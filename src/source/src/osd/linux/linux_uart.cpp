/** @file linux_uart.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.05.05 -

	@brief [ linux uart ]
*/

#include "linux_uart.h"

#ifdef USE_UART
#include "../../emu_osd.h"
#include "../../main.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"

static const struct {
	int rate;
	int id;
} baud_rates[] = {
	{110, B110}, {300, B300}, {600, B600}, {1200, B1200}, {2400, B2400},
	{4800, B4800}, {9600, B9600}, {19200, B19200}, {38400, B38400},
	{57600, B57600}, {115200, B115200}, {230400, B230400}, {460800, B460800},
	{921600, B921600}, {0, B0}
};


void EMU_OSD::EMU_UART()
{
	coms = NULL;
}

void EMU_OSD::initialize_uart()
{
	coms = new CommPorts();
}

void EMU_OSD::release_uart()
{
	delete coms;
	coms = NULL;
}

int EMU_OSD::enum_uarts()
{
	coms->Enum();
	return coms->Count();
}

/// @note called by main thread
void EMU_OSD::update_uart()
{
	// buffer copy
	for(int i = 0; i < coms->Count(); i++) {
		recv_uart_data(i);
	}
}

void EMU_OSD::get_uart_description(int ch, _TCHAR *buf, size_t size)
{
#ifdef _UNICODE
	char nbuf[256];
	coms->GetNameDescription(ch, nbuf, sizeof(nbuf));
	UTILITY::conv_mbs_to_wcs(nbuf, (int)strlen(nbuf), buf, (int)size);
#else
	coms->GetNameDescription(ch, buf, size);
#endif
}

bool EMU_OSD::init_uart(int ch, DEVICE *dev)
{
	if (ch < 0 || coms->Count() <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU::init_uart: invalid uart channel. ch=%d"), ch);
		return false;
	}
	CommPort *com = coms->Item(ch);
	com->SetDevice(dev);
	return true;
}

bool EMU_OSD::open_uart(int ch)
{
	if (ch < 0 || coms->Count() <= ch) return false;
	return (coms->Item(ch)->Open() != 0); 
}

bool EMU_OSD::is_opened_uart(int ch)
{
	if (ch < 0 || coms->Count() <= ch) return false;
	return coms->Item(ch)->IsOpened();
}

void EMU_OSD::close_uart(int ch)
{
	if (ch < 0 || coms->Count() <= ch) return;
	coms->Item(ch)->Close();
}

int EMU_OSD::send_uart_data(int ch, const uint8_t *data, int size)
{
	if (ch < 0 || coms->Count() <= ch) return -1;

	CommPort *com = coms->Item(ch);
	if (!com->IsOpened()) return -1;

	size = com->SendData((const char *)data, size);

	return size;
}

/// @note Use following functions:
/// get_sendbuffer, inc_sendbuffer_ptr
void EMU_OSD::send_uart_data(int ch)
{
	if (ch < 0 || coms->Count() <= ch) return;

	CommPort *com = coms->Item(ch);
	if (!com->IsOpened()) return;

	com->ReadFromDevice();
}

void EMU_OSD::recv_uart_data(int ch)
{
	if (ch < 0 || coms->Count() <= ch) return;

	CommPort *com = coms->Item(ch);
	if (!com->IsOpened()) return;

	com->WriteToDevice();
}

////////////////////////////////////////
// COM port on linux
////////////////////////////////////////

CommPort::CommPort(int ch)
{
	m_ch = ch;
	device = NULL;
	fd = -1;
	memset(sName, 0, sizeof(sName));
	memset(&ios, 0, sizeof(ios));
	iNeedRetry = 0;
	mux = NULL;
}

CommPort::~CommPort()
{
	Close();
	delete mux;
}

CommPort &CommPort::operator=(const CommPort &src)
{
	Close();
	memcpy(sName, src.sName, sizeof(sName));
	return *this;
}

int CommPort::Open()
{
	if (strlen(sName) <= 0) return 0;

	if (!mux) mux = new CMutex();

	mux->lock();
	fd = open(sName, O_RDWR | /* O_NOTTY | */ O_NONBLOCK);
	if (fd < 0) {
		mux->unlock();
		return 0;
	}

//	iotcl(fd, TIOCEXCL);
//	ioctl(fd, TIOCGETA, &ios);
	tcgetattr(fd, &ios);
	SetBaudRate(pConfig->comm_uart_baudrate);
	SetDataBit(pConfig->comm_uart_databit);
	SetParity(pConfig->comm_uart_parity);
	SetStopBit(pConfig->comm_uart_stopbit);
	SetFlowControl(pConfig->comm_uart_flowctrl);
	ios.c_cflag |= CREAD;
	ios.c_lflag &= ~ICANON;
	tcsetattr(fd, TCSANOW, &ios);

	Clear();

	iNeedRetry = 0;
	mux->unlock();

	return 1;
}

void CommPort::Close()
{
	if (fd >= 0) {
		mux->lock();
		close(fd);
		fd = -1;
		mux->unlock();
	}
}

bool CommPort::IsOpened() const
{
	return (fd >= 0 || iNeedRetry > 0);
}

void CommPort::Clear()
{
	if (fd < 0) return;

	// clear buffer
	tcflush(fd, TCIOFLUSH);
}

int CommPort::ReadFromDevice()
{
	// get send buffer and data size
	int size;
	int flags = 0;
	uint8_t* buf;

	if (!device) {
		return 0;
	}

	// loop until send buffer becomes empty
	while(1) {
		buf = device->get_sendbuffer(m_ch, &size, &flags);
		if(!buf || !size) {
			break;
		}

		if((size = SendData((const char *)buf, size)) <= 0) {
			break;
		}

		device->inc_sendbuffer_ptr(m_ch, size);
	}

	return size;
}

int CommPort::SendData(const char *buf, int size)
{
	if (fd < 0) {
		if (iNeedRetry > 0) {
			// try re-open port
			if (Open() <= 0) {
				return -1;
			}
			iNeedRetry = 0;
		} else {
			return -1;
		}
	}

	mux->lock();
	ssize_t len = write(fd, buf, size);
	mux->unlock();
	if (len < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			len = 0;
		} else if (errno == ENXIO) {
			// device not configured
			// detached device?
			iNeedRetry++;
			Close();
			len = 0;
		} else {
			logging->out_logf(LOG_ERROR, _T("Sending error on serial port %d. errno:%d"), m_ch, errno);
			Close();
			return -1;
		}
	}
	return (int)len;
}

int CommPort::WriteToDevice()
{
	if (!device) {
		return 0;
	}

	// get buffer
	int size0 = 0;
	int size1 = 0;
	int flags = 0;
	uint8_t* buf0 = device->get_recvbuffer0(m_ch, &size0, &size1, &flags);
	uint8_t* buf1 = device->get_recvbuffer1(m_ch);

	int size = 0;
	size = RecvData((char *)buf0, size0);
	if (size1 > 0) {
		size += RecvData((char *)buf1, size1);
	}
	device->inc_recvbuffer_ptr(m_ch, size);

	return size;
}

/// @return written size (bytes)
int CommPort::RecvData(char *buf, int size)
{
	if (fd < 0) {
		if (iNeedRetry > 0) {
			// try re-open port
			if (Open() <= 0) {
				return -1;
			}
			iNeedRetry = 0;
		} else {
			return -1;
		}
	}

	mux->lock();
	ssize_t len = read(fd, buf, size);
	mux->unlock();
	if (len < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			len = 0;
		} else if (errno == ENXIO) {
			// device not configured
			// detached device?
			iNeedRetry++;
			Close();
			len = 0;
		} else {
			logging->out_logf(LOG_ERROR, _T("Receiving error on serial port %d. errno:%d"), m_ch, errno);
			Close();
			return -1;
		}
	}

//	buf[len] = 0;
	return (int)len;
}

void CommPort::SetName(const char *name)
{
	memset(sName, 0, sizeof(sName));
	UTILITY::strcpy(sName, sizeof(sName), name);
}

void CommPort::SetDescription(const char *desc)
{
}

void CommPort::SetDevice(DEVICE *dev)
{
	device = dev;
}

void CommPort::SetBaudRate(int rate)
{
	int match = -1;
	for(int i=0; baud_rates[i].id != 0; i++) {
		if (baud_rates[i].rate == rate) {
			match = i;
			break;
		}
	}
	if (match < 0) return;

	cfsetispeed(&ios, baud_rates[match].id);
	cfsetospeed(&ios, baud_rates[match].id);
}

void CommPort::SetDataBit(int bit)
{
	ios.c_cflag &= ~CSIZE;
	if (bit == 7) ios.c_cflag |= CS7;
	else ios.c_cflag |= CS8;
}

void CommPort::SetParity(int parity)
{
	ios.c_cflag &= ~(PARENB | PARODD);
	if (parity > 0) {
		ios.c_cflag |= PARENB;
		if (parity == 1) {
			ios.c_cflag |= PARODD;
		}
	}
}

void CommPort::SetStopBit(int bit)
{
	ios.c_cflag &= ~CSTOPB;
	if (bit == 2) ios.c_cflag |= CSTOPB;
}

void CommPort::SetFlowControl(int flow)
{
	ios.c_iflag &= ~(IXON | IXOFF);
	ios.c_cflag &= ~CRTSCTS;

	switch(flow) {
	case 1:
		// Xon/off
		ios.c_iflag |= (IXON | IXOFF);
		break;
	case 2:
		// Hardware
		ios.c_cflag |= CRTSCTS;
		break;
	default:
		// none
		break;
	}
}

int CommPort::GetName(char *name, size_t size) const
{
	memset(name, 0, size);
	UTILITY::strncpy(name, size, sName, strlen(sName));
	return (int)strlen(name);
}

int CommPort::GetDescription(char *desc, size_t size) const
{
	return 0;
}

int CommPort::GetNameDescription(char *str, size_t size) const
{
	memset(str, 0, size);
	UTILITY::strncpy(str, size, sName, strlen(sName));
//	UTILITY::strncat(str, size, _T(" ("), 2);
//	UTILITY::strncat(str, size, sDesc, strlen(sDesc));
//	UTILITY::strncat(str, size, _T(")"), 2);
	return (int)strlen(str);
}

DEVICE *CommPort::GetDevice() const
{
	return device;
}

bool CommPort::CompareName(const char *name) const
{
	return (strcmp(sName, name) == 0);
}

void CommPort::Lock()
{
	if (mux) mux->lock();
}

void CommPort::Unlock()
{
	if (mux) mux->unlock();
}

//
// search in /proc/tty/drivers
//

CommPorts::CommPorts()
	: CPtrList<CommPort>()
{
}

CommPorts::~CommPorts()
{
}

//CommPort &CommPorts::operator[](int idx)
//{
//	return cPorts[idx];
//}


/// Enumrate serial ports
///
/// @return number of serial port
int CommPorts::Enum()
{
	char buf[512];

	// read from procfs
	FILE *fp = fopen("/proc/tty/drivers", "r");
	if (!fp) return 0;

	char *prms[6];
	int prm_count;
	CPtrList<CNchar> categories;
	// <driver name> <path> <major> <minor range> <device type name>
	while(fgets(buf, sizeof(buf)-1, fp) != NULL) {
		prm_count = UTILITY::get_parameters(buf, sizeof(buf)-1, prms, 6);
		if (prm_count == 5 && strstr(prms[4], "serial") != NULL) {
//			logging->out_debugf("Enum: %d [%s][%s]", prm_count, prms[1], prms[4]);
			// match serial device
			CNchar *s = new CNchar(prms[1]);
			categories.Add(s);
		}
	}
	fclose(fp);


	int ch = 0;
#if 0
	for(int idx = 0; idx < categories.Count(); idx++) {
		DIR *dir = opendir("/dev/");
		if (!dir) break;

		int match = 0;
		struct dirent *item = NULL;
		while((item = readdir(dir)) != NULL) {
			strcpy(buf, "/dev/");
			strcat(buf, item->d_name);
			if (strstr(buf, categories.Item(idx).Get()) != NULL) {
				// match device file
				if (!Find(buf)) {
					CommPort *com = new CommPort(ch);
					com->SetName(buf);
					Add(com);
					match++;
					ch++;
				}
			}
			if (match >= 4) break;
		}
		closedir(dir);
	}
#else
	struct stat st;
	for(int idx = 0; idx < categories.Count(); idx++) {
		for(int n = 0; n < 4; n++) {
			sprintf(buf, "%s%d", categories.Item(idx)->Get(), n);
			logging->out_debugf("Enum: %d-%d [%s]", idx, n, buf);
			if (!stat(buf, &st)) {
				// access ok
				if ((st.st_mode & S_IFMT) == S_IFCHR) {
					logging->out_debugf("Enum: permission: %o", (int)st.st_mode);
					// is character device
					if (Find(buf) < 0) {
						CommPort *com = new CommPort(ch);
						com->SetName(buf);
						Add(com);
						ch++;
					}
				}
			}
		}
	}
#endif
	logging->out_debugf("Enum: count: %d", (int)Count());
	return Count();
}

/// Search serial port by name
///
/// @param[in] name: port name
/// @return >=0: index number / -1: not found
int CommPorts::Find(const char *name)
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->CompareName(name)) {
			match = i;
			break;
		}
	}
	return match;
}

int CommPorts::GetName(int idx, char *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetName(name, size);
	else return 0;
}

int CommPorts::GetDescription(int idx, char *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetDescription(name, size);
	else return 0;
}

int CommPorts::GetNameDescription(int idx, char *str, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetNameDescription(str, size);
	else return 0;
}

#endif /* USE_UART */

