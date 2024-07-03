/** @file win_uart.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.29 -

	@brief [ win32 uart ]
*/

#include "win_uart.h"

#ifdef USE_UART
#include "../../emu_osd.h"
#include "../../main.h"
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"

#include <WinIoCtl.h>
#include <SetupAPI.h>

#ifdef _MSC_VER
#pragma comment(lib,"setupapi.lib")
#endif

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
	coms->GetNameDescription(ch, buf, size);
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
// COM port on windows
////////////////////////////////////////

CommPort::CommPort(int ch)
{
	m_ch = ch;
	device = NULL;
	hComm = INVALID_HANDLE_VALUE;
	ZeroMemory(&sDcb, sizeof(sDcb));
	sDcb.DCBlength = sizeof(sDcb);
	sDcb.fBinary = 1;
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
	UTILITY::tcsncpy(sName, sizeof(sName)/sizeof(sName[0]), src.sName, sizeof(sName)/sizeof(sName[0]));
	UTILITY::tcsncpy(sDesc, sizeof(sDesc)/sizeof(sDesc[0]), src.sDesc, sizeof(sDesc)/sizeof(sDesc[0]));
	sDcb = src.sDcb;
	return *this;
}

int CommPort::Open()
{
	if (_tcslen(sName) <= 0) return 0;

	if (!mux) mux = new CMutex();

	TCHAR comfile[16];
	DWORD attr = FILE_ATTRIBUTE_NORMAL;
	UTILITY::stprintf(comfile, sizeof(comfile)/sizeof(comfile[0]), _T("\\\\.\\%s"), sName);

	mux->lock();
	hComm=CreateFile(comfile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, attr, NULL);
	if(hComm == INVALID_HANDLE_VALUE) {
		mux->unlock();
		return 0;
	}

	GetCommState(hComm, &sDcb);
	SetBaudRate(pConfig->comm_uart_baudrate);
	SetDataBit(pConfig->comm_uart_databit);
	SetParity(pConfig->comm_uart_parity);
	SetStopBit(pConfig->comm_uart_stopbit);
	SetFlowControl(pConfig->comm_uart_flowctrl);
	if (!SetCommState(hComm, &sDcb)) {
		mux->unlock();
		return 0;
	}

	Clear();

	iNeedRetry = 0;
	mux->unlock();

	return 1;
}

void CommPort::Close()
{
	if (hComm != INVALID_HANDLE_VALUE) {
		mux->lock();
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		mux->unlock();
	}
}

bool CommPort::IsOpened() const
{
	return (hComm != INVALID_HANDLE_VALUE || iNeedRetry > 0);
}

void CommPort::Clear()
{
	if (hComm == INVALID_HANDLE_VALUE) return;

	SetupComm(hComm, UART_BUFFER_MAX, UART_BUFFER_MAX);

	PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);

	// Timeout
	COMMTIMEOUTS timeouts;
	GetCommTimeouts(hComm, &timeouts);
	// non block when read sequence
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	// non block when write sequence
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 1;
	SetCommTimeouts(hComm, &timeouts);
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
	if (hComm == INVALID_HANDLE_VALUE) {
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

	DWORD wsize = 0;

	mux->lock();
	BOOL rc = WriteFile(hComm, buf, size, &wsize, NULL);
	mux->unlock();
	if (!rc) {
		// error
		DWORD err = GetLastError();
		switch(err){
		case ERROR_ACCESS_DENIED:
			// device detached ?
			iNeedRetry++;
			wsize = 0;
			Close();
			break;
		default:
			logging->out_logf(LOG_ERROR, _T("Sending error on serial port %d. errno:%d"), m_ch, err);
			Close();
			return -1;
		}
	}
#ifdef _DEBUG_LOG
	if (wsize > 0) {
		char str[1024];
		str[0] = '<'; str[1] = '<';
		for(int i=0, pos = 2; i<(int)wsize && pos < 1021; i++) {
			UTILITY::sprintf(&str[pos], sizeof(str), " %02X", buf[i] & 0xff);
			pos += 3;
		}
		logging->out_debug(str);
	}
#endif
	return wsize;
}

int CommPort::SendData(const wchar_t *buf, int size)
{
	if (hComm == INVALID_HANDLE_VALUE) {
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

	DWORD wsize = 0;
	char nbuf[64];
	int nsize = (int)sizeof(nbuf);
	if (size < nsize) nsize = size;
	wcstombs(nbuf, buf, nsize);

	mux->lock();
	BOOL rc = WriteFile(hComm, nbuf, nsize, &wsize, NULL);
	mux->unlock();
	if (!rc) {
		// error
		DWORD err = GetLastError();
		switch(err){
		case ERROR_ACCESS_DENIED:
			// device detached ?
			iNeedRetry++;
			Close();
			wsize = 0;
			break;
		default:
			logging->out_logf(LOG_ERROR, _T("Sending error on serial port %d. errno:%d"), m_ch, err);
			Close();
			return -1;
		}
	}

	return wsize;
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
	if (hComm == INVALID_HANDLE_VALUE) {
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

//	DWORD serr = CE_RXOVER;
//	COMSTAT sts;
//	ClearCommError(hComm, &serr, &sts);
//	if ((serr & CE_RXOVER) != 0) {
//		int a = 0;
//	}

	DWORD rsize = 0;

	mux->lock();
	BOOL rc = ReadFile(hComm, buf, size, &rsize, NULL);
	mux->unlock();
	if (!rc) {
		// error
		DWORD err = GetLastError();
		switch(err){
		case ERROR_IO_PENDING:
		case ERROR_INVALID_USER_BUFFER:
			break;
		case ERROR_ACCESS_DENIED:
			// device detached ?
			iNeedRetry++;
			Close();
			rsize = 0;
			break;
		default:
			logging->out_logf(LOG_ERROR, _T("Received error on serial port %d. errno:%d"), m_ch, err);
			Close();
			return -1;
		}
	}
#ifdef _DEBUG_LOG
	if (rsize > 0) {
		char str[1024];
		str[0] = '>'; str[1] = '>';
		for(int i=0, pos = 2; i<(int)rsize && pos < 1021; i++) {
			UTILITY::sprintf(&str[pos], sizeof(str), " %02X", buf[i] & 0xff);
			pos += 3;
		}
		logging->out_debug(str);
	}
#endif
//	buf[rsize] = 0;
	return rsize;
}

/// @return written size (bytes)
int CommPort::RecvData(wchar_t *buf, int size)
{
	if (hComm == INVALID_HANDLE_VALUE) {
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

	DWORD rsize = 0;
	char nbuf[64];
	int nsize = sizeof(nbuf) - 1;

	mux->lock();
	BOOL rc = ReadFile(hComm, nbuf, nsize, &rsize, NULL);
	mux->unlock();
	if (!rc) {
		// error
		DWORD err = GetLastError();
		switch(err){
		case ERROR_IO_PENDING:
		case ERROR_INVALID_USER_BUFFER:
			break;
		case ERROR_ACCESS_DENIED:
			// device detached ?
			iNeedRetry++;
			Close();
			rsize = 0;
			break;
		default:
			logging->out_logf(LOG_ERROR, _T("Received error on serial port %d. errno:%d"), m_ch, err);
			Close();
			return -1;
		}
	}
	mbstowcs(buf, nbuf, rsize);
//	buf[rsize] = 0;
	return rsize;
}

void CommPort::SetName(const TCHAR *name)
{
	ZeroMemory(sName, sizeof(sName));
	UTILITY::tcscpy(sName, sizeof(sName) / sizeof(TCHAR), name);
}

void CommPort::SetDescription(const TCHAR *desc)
{
	ZeroMemory(sDesc, sizeof(sDesc));
	UTILITY::tcscpy(sDesc, sizeof(sDesc) / sizeof(TCHAR), desc);
}

#if 0
int CommPort::SetProperty(const DCB &dcb)
{
	if (Open() <= 0) {
		return -1;
	}
	sDcb = dcb;
	SetCommState(hComm, &sDcb);
	Close();
	return 0;
}
#endif

void CommPort::SetDevice(DEVICE *dev)
{
	device = dev;
}

void CommPort::SetBaudRate(int rate)
{
	sDcb.BaudRate = rate;
}

void CommPort::SetDataBit(int bit)
{
	sDcb.ByteSize = bit;
}

void CommPort::SetParity(int parity)
{
	sDcb.fParity = (parity == 0 ? 0 : 1);
	sDcb.Parity = parity;
}

void CommPort::SetStopBit(int bit)
{
	if (bit == 2) sDcb.StopBits = bit;
	else  sDcb.StopBits = 0;
}

void CommPort::SetFlowControl(int flow)
{
	sDcb.fRtsControl = RTS_CONTROL_ENABLE;
	sDcb.fDtrControl = DTR_CONTROL_ENABLE;

	switch(flow) {
	case 1:
		// Xon/off
		sDcb.fInX = TRUE;
		sDcb.fOutX = TRUE;
		sDcb.fTXContinueOnXoff = FALSE;
		sDcb.fOutxCtsFlow = FALSE;
		sDcb.fOutxDsrFlow = FALSE;
		break;
	case 2:
		// Hardware
		sDcb.fInX = FALSE;
		sDcb.fOutX = FALSE;
		sDcb.fTXContinueOnXoff = TRUE;
		sDcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		sDcb.fOutxCtsFlow = TRUE;
		sDcb.fOutxDsrFlow = FALSE;
		break;
	default:
		// none
		sDcb.fInX = FALSE;
		sDcb.fOutX = FALSE;
		sDcb.fTXContinueOnXoff = TRUE;
		sDcb.fOutxCtsFlow = FALSE;
		sDcb.fOutxDsrFlow = FALSE;
		break;
	}
}

int CommPort::GetName(TCHAR *name, size_t size) const
{
	ZeroMemory(name, size);
	UTILITY::tcsncpy(name, size, sName, _tcslen(sName));
	return (int)_tcslen(name);
}

int CommPort::GetDescription(TCHAR *desc, size_t size) const
{
	ZeroMemory(desc, size);
	UTILITY::tcsncpy(desc, size, sDesc, _tcslen(sDesc));
	return (int)_tcslen(desc);
}

int CommPort::GetNameDescription(TCHAR *str, size_t size) const
{
	ZeroMemory(str, size);
	UTILITY::tcsncpy(str, size, sName, _tcslen(sName));
	UTILITY::tcsncat(str, size, _T(" ("), 2);
	UTILITY::tcsncat(str, size, sDesc, _tcslen(sDesc));
	UTILITY::tcsncat(str, size, _T(")"), 2);
	return (int)_tcslen(str);
}

#if 0
int CommPort::GetProperty(DCB &dcb)
{
	if (Open() <= 0) {
		return -1;
	}
	dcb = sDcb;
	Close();
	return 0;
}
#endif

DEVICE *CommPort::GetDevice() const
{
	return device;
}

bool CommPort::CompareName(const TCHAR *name) const
{
	return (_tcscmp(sName, name) == 0);
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
//
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
	HDEVINFO hDevInfo;
//	DWORD MemberIndex=0;
	SP_DEVINFO_DATA dev_data;

	// Get device information
	hDevInfo=SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(hDevInfo == 0) {
		return 0;
	}
	ZeroMemory(&dev_data, sizeof(dev_data));
	dev_data.cbSize=sizeof(dev_data);

	TCHAR buf[256];
	int idx;
	int ch = 0;
	for(idx = 0; SetupDiEnumDeviceInfo(hDevInfo, idx, &dev_data); idx++) {
		// device interface
		DWORD dataT;
		DWORD size;

		// Get COM port name
        HKEY key = SetupDiOpenDevRegKey(hDevInfo, &dev_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
        if(!key) {
			continue;
		}

		size = sizeof(buf);
		DWORD type=0;
		RegQueryValueEx(key, _T("PortName"), NULL, &type, (LPBYTE)buf, &size);

		size_t len = _tcslen(buf);
		if (len == 0) {
			continue;
		}

		if (Find(buf) >= 0) {
			// already exists
			continue;
		}

		CommPort *comm = new CommPort(ch);
		Add(comm);
		ch++;

		comm->SetName(buf);

		// Get device description
		size = sizeof(buf);
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &dev_data, SPDRP_DEVICEDESC, &dataT, (PBYTE)buf, size, &size)) {
			// ignore description
			buf[0] = _T('\0');
		}
		comm->SetDescription(buf);

		if (idx >= UART_MAX_PORTS) break;
	}
	// Release device information
    SetupDiDestroyDeviceInfoList(hDevInfo);

	return idx;
}

/// Search serial port by name
///
/// @param[in] name: port name
/// @return >=0: index number / -1: not found
int CommPorts::Find(const TCHAR *name)
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

int CommPorts::GetName(int idx, TCHAR *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetName(name, size);
	else return 0;
}

int CommPorts::GetDescription(int idx, TCHAR *name, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetDescription(name, size);
	else return 0;
}

int CommPorts::GetNameDescription(int idx, TCHAR *str, size_t size) const
{
	if (idx < Count()) return Item(idx)->GetNameDescription(str, size);
	else return 0;
}

#if 0
int CommPorts::GetProperty(int idx, DCB &dcb)
{
	if (idx < Count()) return Item(idx)->GetProperty(dcb);
	else return -1;
}

int CommPorts::SetProperty(int idx, const DCB &dcb)
{
	if (idx < Count()) return Item(idx)->SetProperty(dcb);
	else return -1;
}
#endif

#endif /* USE_UART */

