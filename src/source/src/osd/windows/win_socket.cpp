/** @file win_socket.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.05.08 -

	@brief [ win32 socket ]

	@note
	This code is based on win32_socket.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
	Date   : 2006.08.27 -
*/

#include "win_emu.h"
#include "win_main.h"
#include "win_socket.h"
#include "../../vm/vm.h"
#include "../../vm/device.h"
#include "../../fifo.h"
#include "../../cmutex.h"

#ifdef USE_SOCKET

//#define OUT_DEBUG logging->out_debugf
#define OUT_DEBUG logging->dummyf

void EMU_OSD::EMU_SOCKET()
{
	// init sockets
	conn = new Connection();
}

void EMU_OSD::initialize_socket()
{
	// init winsock
	conn->Initialize();
}

void EMU_OSD::release_socket()
{
	// release sockets
	delete conn;
	conn = NULL;
}

const void *EMU_OSD::get_socket(int ch) const
{
	return (void *)conn->GetSocket(ch);
}

void EMU_OSD::socket_connected(int ch)
{
	// winmain notify that network is connected
	OUT_DEBUG(_T("connected. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_connected(ch);
//	else vm->network_connected(ch);
}

void EMU_OSD::socket_disconnected(int ch)
{
	// winmain notify that network is disconnected
	conn->SetDisconnectDelay(ch, 1 /*56*/);
	OUT_DEBUG(_T("disconnected. ch=%d"), ch);
}

/// @note called by main thread
void EMU_OSD::socket_writeable(int ch)
{
	// winmain notify that network is writeable
	OUT_DEBUG(_T("writeable. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_writeable(ch);
//	else vm->network_writeable(ch);
}

/// @note called by main thread
void EMU_OSD::socket_readable(int ch)
{
	// winmain notify that network is readable
	OUT_DEBUG(_T("readable. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_readable(ch);
//	else vm->network_readable(ch);
}

/// @note called by emu thread
void EMU_OSD::update_socket()
{
	// buffer copy
	for(int i = 0; i < SOCKET_MAX; i++) {
		CSocket *soc = conn->Item(i);
		soc->SendToDevice();
	}
}

/// @note called by main thread
void EMU_OSD::socket_accept(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	conn->Accept(ch);
}

/// @note called by main thread
bool EMU_OSD::init_socket_tcp(int ch, DEVICE *dev, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::init_socket_tcp: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->InitTCP(dev, server);
}

/// @note called by main thread
bool EMU_OSD::init_socket_udp(int ch, DEVICE *dev, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::init_socket_udp: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->InitUDP(dev, server);
}

bool EMU_OSD::connect_socket(int ch, uint32_t ipaddr, int port, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::connect_socket: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->Connect(ipaddr, port, server);
}

bool EMU_OSD::connect_socket(int ch, const _TCHAR *hostname, int port, bool server)
{
	if (ch < 0 || SOCKET_MAX <= ch) {
		logging->out_logf(LOG_ERROR, _T("EMU_OSD::connect_socket: invalid socket channel. ch=%d"), ch);
		return false;
	}

	CSocket *soc = conn->Item(ch);

	return soc->Connect(hostname, port, server);
}

bool EMU_OSD::is_connecting_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return false;

	CSocket *soc = conn->Item(ch);

	return soc->IsConnecting();
}

void EMU_OSD::disconnect_socket(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->Disconnect();
}

int EMU_OSD::get_socket_channel()
{
	return conn->GetEmptySocket();
}

bool EMU_OSD::listen_socket(int ch)
{
	return false;
}

/// @note called by emu thread
void EMU_OSD::send_data_tcp(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice();
}

/// @note called by emu thread
void EMU_OSD::send_data_udp(int ch, uint32_t ipaddr, int port)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice(ipaddr, port);
}

/// @note called by emu thread
void EMU_OSD::send_data_udp(int ch, const _TCHAR *hostname, int port)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvFromDevice(hostname, port);
}

/// @note usually called by emu thread
/// @note called by main thread at first when connection succeeded.
void EMU_OSD::send_data(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->SendData();
}

/// Receive data from socket, and store to first buffer to be able to read from emu devices.
/// @note called by main thread
void EMU_OSD::recv_data(int ch)
{
	if (ch < 0 || SOCKET_MAX <= ch) return;

	CSocket *soc = conn->Item(ch);

	soc->RecvData();
}


/**********************************************************************/

CSocket::CSocket(Connection *conn, int ch)
{
	m_conn = conn;
	m_ch = ch;

	soc = INVALID_SOCKET;
	device = NULL;
	is_tcp = true;
	socket_delay = 0;

	recv_buffer = NULL;
	recv_mux = NULL;
	send_buffer = NULL;
	send_mux = NULL;
}

CSocket::~CSocket()
{
	delete recv_buffer;
	delete recv_mux;
	delete send_buffer;
	delete send_mux;
}

int CSocket::read_and_remove_dup_iac(char *dst, int size, FIFOCHAR *buffer)
{
	int pos = 0;
	int rep = 0;
	char cp = 0;
	for(int spos = 0; spos < size; ) {
		char c = buffer->read();
		spos++;
		if (c == (char)0xff && spos == size) {
			if (rep != 1) buffer->rollback();
			break;
		}
		if (cp != (char)0xff || c != (char)0xff) {
			dst[pos++] = c;
			rep = 0;
		}
		if (c == (char)0xff) {
			rep++;
			if (rep >= 2) c = 0;
		}
		cp = c;
	}
	return pos;
}

void CSocket::SendToDevice()
{
	int size = recv_buffer ? recv_buffer->count() : 0;
	if(size > 0) {
		if (device) {
			// get buffer
			int size0 = 0;
			int size1 = 0;
			int flags = 0;
			uint8_t* buf0 = device->get_recvbuffer0(m_ch, &size0, &size1, &flags);
			//	buf0 = vm->get_recvbuffer0(m_ch, &size0, &size1, &flags);
			uint8_t* buf1 = device->get_recvbuffer1(m_ch);
			//	buf1 = vm->get_recvbuffer1(m_ch);

			if(size > size0 + size1) {
				size = size0 + size1;
			}

			if(size <= size0) {
				recv_mux->lock();
				if (!(flags & 1)) {
					recv_buffer->read((char *)buf0, size);
				} else {
					// escape telnet iac
					size = read_and_remove_dup_iac((char *)buf0, size, recv_buffer);
				}
				recv_mux->unlock();
			}
			else {
				recv_mux->lock();
				if (!(flags & 1)) {
					recv_buffer->read((char *)buf0, size0);
					recv_buffer->read((char *)buf1, size - size0);
				} else {
					// escape telnet iac
					size0 = read_and_remove_dup_iac((char *)buf0, size0, recv_buffer);
					size = read_and_remove_dup_iac((char *)buf1, size - size0, recv_buffer);
				}
				recv_mux->unlock();
			}
			device->inc_recvbuffer_ptr(m_ch, size);
			//	vm->inc_recvbuffer_ptr(m_ch, size);
		}
	}
	else if(socket_delay > 0) {
		if(--socket_delay == 0) {
			Disconnect();
//			if (device) device->network_disconnected(m_ch);
//			else vm->network_disconnected(m_ch);
		}
	}
}

int CSocket::write_and_escape_iac(const char *src, int size, FIFOCHAR *buffer)
{
	int pos = 0;
	for(int i=0; i<size; i++) {
		buffer->write(src[i]); pos++;
		if (src[i] == (char)0xff) {
			buffer->write(src[i]); pos++;
		}
	}
	return pos;
}

void CSocket::RecvFromDevice()
{
	// get send buffer and data size
	int size;
	int flags = 0;
	uint8_t* buf;

	// loop while send buffer is not empty or not WSAEWOULDBLOCK
	while(device) {
		buf = device->get_sendbuffer(m_ch, &size, &flags);
		//	buf = vm->get_sendbuffer(m_ch, &size);

		if(!buf || !size) {
			break;
		}

		send_mux->lock();
		if (flags & 1) {
			// escape telnet iac
			size = write_and_escape_iac((const char *)buf, size, send_buffer);
		} else {
			size = send_buffer->write((const char *)buf, size);
		}
		send_mux->unlock();

		device->inc_sendbuffer_ptr(m_ch, size);
		//	vm->inc_sendbuffer_ptr(m_ch, size);
	}

	SendData();
}

void CSocket::RecvFromDevice(const _TCHAR *hostname, int port)
{
	uint32_t ipaddr;

	if(!get_ipaddr(hostname, port, ipaddr)) {
		return;
	}

	SetUDPAddress(ipaddr, port);
	RecvFromDevice();
}

void CSocket::RecvFromDevice(uint32_t ipaddr, int port)
{
	SetUDPAddress(ipaddr, port);
	RecvFromDevice();
}

bool CSocket::InitTCP(DEVICE *dev, bool server)
{
	if (IsConnecting()) {
		Disconnect();
	}

	is_tcp = true;
	device = NULL;

	if((soc = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		int ecode = WSAGetLastError();
		logging->out_logf(LOG_ERROR, _T("CSocket::InitTCP: socket error. ch=%d code=%d"), m_ch, ecode);
		return false;
	}
	if (!server) {
		if(WSAAsyncSelect(soc, hMainWindow, WM_SOCKET0 + m_ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
			closesocket(soc);
			int ecode = WSAGetLastError();
			logging->out_logf(LOG_ERROR, _T("CSocket::InitTCP: WSAAsyncSelect error. ch=%d code=%d"), m_ch, ecode);
			soc = INVALID_SOCKET;
			return false;
		}
	} else {
		if(WSAAsyncSelect(soc, hMainWindow, WM_SOCKET0 + m_ch, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) {
			closesocket(soc);
			int ecode = WSAGetLastError();
			logging->out_logf(LOG_ERROR, _T("CSocket::InitTCP: WSAAsyncSelect error. ch=%d code=%d"), m_ch, ecode);
			soc = INVALID_SOCKET;
			return false;
		}
	}
	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();

	device = dev;

	return true;
}

bool CSocket::InitUDP(DEVICE *dev, bool server)
{
	if (IsConnecting()) {
		Disconnect();
	}

	is_tcp = false;
	device = NULL;

	if((soc = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		int ecode = WSAGetLastError();
		logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: socket error. ch=%d code=%d"), m_ch, ecode);
		return false;
	}
	if(WSAAsyncSelect(soc, hMainWindow, WM_SOCKET0 + m_ch, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(soc);
		int ecode = WSAGetLastError();
		logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: WSAAsyncSelect error. ch=%d code=%d"), m_ch, ecode);
		soc = INVALID_SOCKET;
		return false;
	}
	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();

	device = dev;

	return true;
}

bool CSocket::get_ipaddr(const _TCHAR *hostname, int port, uint32_t &ipaddr)
{
	HOSTENT *ent;
#ifdef _UNICODE
	char host[_MAX_PATH];
	wcstombs(host, hostname, _MAX_PATH);
#else
	const char *host = hostname;
#endif
	ent = gethostbyname((const char *)host);
	if (ent == NULL || ent->h_addr_list[0] == NULL) {
		// unresolved hostname to ipaddr
		logging->out_logf(LOG_ERROR, _T("resolve error: hostname=%s"), host);
		return false;
	}
	// IPv4 only
	ipaddr = *(uint32_t *)ent->h_addr_list[0];

	return true;
}

bool CSocket::Connect(const _TCHAR *hostname, int port, bool server)
{
	uint32_t ipaddr;

	if(!get_ipaddr(hostname, port, ipaddr)) {
		return false;
	}

	return Connect(ipaddr, port, server);
}

bool CSocket::Connect(uint32_t ipaddr, int port, bool server)
{
	struct sockaddr_in tcpaddr;
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_addr.s_addr = ipaddr;
	tcpaddr.sin_port = htons((unsigned short)port);
	memset(tcpaddr.sin_zero, (int)0, sizeof(tcpaddr.sin_zero));

	if (!server) {
		if (ipaddr == INADDR_ANY || ipaddr == INADDR_NONE) {
			// client cannot use
			logging->out_log(LOG_ERROR, _T("Cannot create socket."));
			return false;
		}
		if(connect(soc, (struct sockaddr *)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {
			int ecode = WSAGetLastError();
			if(ecode != WSAEWOULDBLOCK) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: connect error. ch=%d code=%d"), m_ch, ecode);
				return false;
			}
			OUT_DEBUG(_T("CSocket::Connect: connect would block. ch=%d code=%d"), m_ch, ecode);
		}
		if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
		if (!recv_mux) recv_mux = new CMutex();
		if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
		if (!send_mux) send_mux = new CMutex();
	} else {
		if (bind(soc, (struct sockaddr *)&tcpaddr, sizeof(tcpaddr)) == SOCKET_ERROR) {
			closesocket(soc);
			int ecode = WSAGetLastError();
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: bind error. ch=%d code=%d"), m_ch, ecode);
			soc = INVALID_SOCKET;
			return false;
		}
		if (is_tcp) {
			if (listen(soc, 1) == SOCKET_ERROR) {
				closesocket(soc);
				int ecode = WSAGetLastError();
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: listen error. ch=%d code=%d"), m_ch, ecode);
				soc = INVALID_SOCKET;
				return false;
			}
		}
	}

	return true;
}

void CSocket::Disconnect()
{
	if(soc != INVALID_SOCKET) {
		shutdown(soc, 2);
		closesocket(soc);
		soc = INVALID_SOCKET;
		socket_delay = 0;

		if (device) device->network_disconnected(m_ch);
//		else vm->network_disconnected(m_ch);
	}

	device = NULL;

//	delete mux;
//	mux = NULL;
}

void CSocket::Shutdown()
{
	if(soc != INVALID_SOCKET) {
		shutdown(soc, 2);
		closesocket(soc);
	}
}

void CSocket::SendData()
{
	char buf[1024];

	send_mux->lock();
	int size = send_buffer->read(buf, sizeof(buf));
	send_mux->unlock();

	if(is_tcp) {
		int sent_size = send(soc, (const char *)buf, size, 0);
		if(sent_size == SOCKET_ERROR) {
			// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
			if(WSAGetLastError() != WSAEWOULDBLOCK) {
				emu->disconnect_socket(m_ch);
				emu->socket_disconnected(m_ch);
			}
		}
	} else {
		int sent_size = sendto(soc, (char *)buf, size, 0, (struct sockaddr *)&udpaddr, sizeof(udpaddr));
		if(sent_size == SOCKET_ERROR) {
			// if WSAEWOULDBLOCK, WM_SOCKET* and FD_WRITE will come later
			if(WSAGetLastError() != WSAEWOULDBLOCK) {
				emu->disconnect_socket(m_ch);
				emu->socket_disconnected(m_ch);
			}
		}
	}
}

void CSocket::RecvData()
{
	char buf[1024];
	int recv_size = 0;
	if(is_tcp) {
		recv_size = recv(soc, buf, sizeof(buf), 0);
		if(recv_size == SOCKET_ERROR) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		}
		recv_mux->lock();
		recv_size = recv_buffer->write(buf, recv_size);
		recv_mux->unlock();
	}
	else {
		SOCKADDR_IN addr;

		int len = sizeof(addr);


		recv_size = recvfrom(soc, buf + 8, sizeof(buf) - 8, 0, (struct sockaddr *)&addr, &len);
		if(recv_size == SOCKET_ERROR) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		}
		buf[0] = recv_size >> 8;
		buf[1] = recv_size & 0xff;
		buf[2] = (char)addr.sin_addr.s_addr;
		buf[3] = (char)(addr.sin_addr.s_addr >> 8);
		buf[4] = (char)(addr.sin_addr.s_addr >> 16);
		buf[5] = (char)(addr.sin_addr.s_addr >> 24);
		buf[6] = (char)addr.sin_port;
		buf[7] = (char)(addr.sin_port >> 8);
		recv_mux->lock();
		recv_size += recv_buffer->write(buf, recv_size + 8);
		recv_mux->unlock();
	}
}

void CSocket::Set(SOCKET sock, bool tcp, DEVICE *dev)
{
	soc = sock;
	is_tcp = tcp;
	device = dev;
	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!recv_mux) recv_mux = new CMutex();
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!send_mux) send_mux = new CMutex();
}

void CSocket::SetUDPAddress(uint32_t ipaddr, int port)
{
	udpaddr.sin_family = AF_INET;
	udpaddr.sin_addr.s_addr = ipaddr;
	udpaddr.sin_port = htons((unsigned short)port);
	memset(udpaddr.sin_zero, 0, sizeof(udpaddr.sin_zero));
}

bool CSocket::IsConnecting() const
{
	return (soc != INVALID_SOCKET);
}

void CSocket::SetDisconnectDelay(int val)
{
	if (!socket_delay) socket_delay = val;
}

/**********************************************************************/

Connection::Connection()
{
	// init sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		socs.Add(new CSocket(this, i));
	}
}

Connection::~Connection()
{
	// release sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		socs.Item(i)->Shutdown();
	}

	// release winsock
	WSACleanup();
}

void Connection::Initialize()
{
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);
}

int Connection::GetEmptySocket() const
{
	int new_ch = -1;
	for(int i = 0; i < SOCKET_MAX; i++) {
		if (!socs.Item(i)->IsConnecting()) {
			new_ch = i;
			break;
		}
	}
	if (new_ch < 0) {
		logging->out_log(LOG_ERROR, _T("Socket is full."));
	}
	return new_ch;
}

void Connection::Accept(int ch)
{
	int new_ch = -1;

	CSocket *soc = socs.Item(ch);

	SOCKET new_socket;
	struct sockaddr_in new_sockaddr;
	int new_socklen;

	new_socklen = sizeof(new_sockaddr);
	memset(&new_sockaddr, 0, sizeof(new_sockaddr));

	if ((new_socket = accept(soc->GetSocket(), (struct sockaddr *)&new_sockaddr, &new_socklen)) == INVALID_SOCKET) {
		int ecode = WSAGetLastError();
		logging->out_logf(LOG_ERROR, _T("Connection::Accept: accept error. ch=%d code=%d"), ch, ecode);
		return;
	}

	// find empty socket
	new_ch = GetEmptySocket();
	if (new_ch < 0) {
		closesocket(new_socket);
		return;
	}

	// note that i'll fire the writeable event only, won't fire the connected event.
	if(WSAAsyncSelect(new_socket, hMainWindow, WM_SOCKET0 + new_ch, FD_WRITE | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		closesocket(new_socket);
		int ecode = WSAGetLastError();
		logging->out_logf(LOG_ERROR, _T("Connection::Accept: WSAAsyncSelect error. new_ch=%d code=%d"), new_ch, ecode);
		return;
	}
	OUT_DEBUG(_T("Connection::Accept: accepted from 0x%08x"), new_sockaddr.sin_addr.s_addr);

	DEVICE *dev = soc->GetDevice();

	CSocket *new_soc = socs.Item(new_ch);

	new_soc->Set(new_socket, soc->IsTCP(), dev);

	if (dev) dev->network_accepted(ch, new_ch);
//	else vm->network_accepted(ch, new_ch);

	OUT_DEBUG(_T("Connection::Accept: ok. ch=%d is_tcp=%s new_ch=%d is_tcp=%s")
		, ch, soc->IsTCP() ? _T("true") : _T("false"), new_ch, new_soc->IsTCP() ? _T("true") : _T("false"));
}

CSocket *Connection::Item(int ch)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return NULL;
	return socs.Item(ch);
}

void Connection::SetDisconnectDelay(int ch, int val)
{
	socs.Item(ch)->SetDisconnectDelay(val);
}

const SOCKET Connection::GetSocket(int ch) const
{
	return socs.Item(ch)->GetSocket();
}

void Connection::SetDevice(int ch, DEVICE *dev)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return;
	socs.Item(ch)->SetDevice(dev);
}

DEVICE *Connection::GetDevice(int ch)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return NULL;
	return socs.Item(ch)->GetDevice();
}

#endif /* USE_SOCKET */

