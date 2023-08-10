/** @file sdl_socket.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2013.12.06

	@brief [ sdl socket ]

	@note
	This code is based on win32_socket.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
	Date   : 2006.08.27 -
*/

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "../../emu_osd.h"
#include "sdl_socket.h"
#include "../../vm/vm.h"
#include "../../vm/device.h"
#include "../../fifo.h"
#include "../../cmutex.h"

#ifndef USE_SDL_NET
#ifdef _WIN32
/* windows */
typedef int socklen_t;
#else
/* linux , macosx */
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define closesocket close
#endif
#endif

#define USE_SDL_EVENT

#ifndef USE_SDL_EVENT
static EMU *emu;
// for callback timer
typedef struct {
	int ch;
	SDL_TimerID id;
} st_timer;
// callback
static Uint32 on_connected(Uint32 interval, void *param);
static Uint32 on_disconnected(Uint32 interval, void *param);
static Uint32 on_writeable(Uint32 interval, void *param);
static Uint32 on_readable(Uint32 interval, void *param);
#endif

#ifdef USE_SOCKET
#ifdef USE_GTK
static void OnUserSocket(GtkWidget *widget, gint data, gpointer user_data);
#endif

void EMU_OSD::EMU_SOCKET()
{
	// init sockets
	conn = new Connection();
}

void EMU_OSD::initialize_socket()
{
	conn->Initialize();
}

void EMU_OSD::release_socket()
{
	// release sockets
	delete conn;
	conn = NULL;
}

void EMU_OSD::socket_connected(int ch)
{
	// network is connected
	logging->out_debugf(_T("connected. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_connected(ch);
//	else vm->network_connected(ch);
}

void EMU_OSD::socket_disconnected(int ch)
{
	// network is disconnected
//	conn->SetDisconnectDelay(ch, 1 /*56*/);
	logging->out_debugf(_T("disconnected. ch=%d"), ch);
}

/// @note called by main thread
void EMU_OSD::socket_writeable(int ch)
{
	// network is writeable
	logging->out_debugf(_T("writeable. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_writeable(ch);
//	else vm->network_writeable(ch);
}

/// @note called by main thread
void EMU_OSD::socket_readable(int ch)
{
	// network is readable
	logging->out_debugf(_T("readable. ch=%d"), ch);
	DEVICE *dev = conn->GetDevice(ch);
	if (dev) dev->network_readable(ch);
//	else vm->network_readable(ch);
}

/// @note called by main thread
void EMU_OSD::check_socket()
{
	// check arrived packets
	conn->CheckArrived();
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

#if 0
const void *EMU_OSD::get_socket(int ch) const
{
	if (socs[ch].is_tcp) {
		return socs[ch].tcp_socket;
	} else {
		return socs[ch].udp_socket;
	}
}
#endif

#endif /* USE_SOCKET */

#ifndef USE_SDL_EVENT
//
// callback on timer
//
static Uint32 on_connected(Uint32 interval, void *param)
{
	st_timer *timer = (st_timer *)param;
	emu->socket_connected(timer->ch);
	SDL_RemoveTimer(timer->id);
	delete timer;
	return interval;
}
static Uint32 on_disconnected(Uint32 interval, void *param)
{
	st_timer *timer = (st_timer *)param;
	emu->socket_disconnected(timer->ch);
	SDL_RemoveTimer(timer->id);
	delete timer;
	return interval;
}
static Uint32 on_writeable(Uint32 interval, void *param)
{
	st_timer *timer = (st_timer *)param;
	emu->socket_writeable(timer->ch);
	emu->send_data(timer->ch);
	SDL_RemoveTimer(timer->id);
	delete timer;
	return interval;
}
static Uint32 on_readable(Uint32 interval, void *param)
{
	st_timer *timer = (st_timer *)param;
	emu->recv_data(timer->ch);
	emu->socket_readable(timer->ch);
	SDL_RemoveTimer(timer->id);
	delete timer;
	return interval;
}
#endif // USE_SDL_EVENT

#ifdef USE_GTK
void OnUserSocket(GtkWidget *widget, gint data, gpointer user_data)
{
	EMU *emu = (EMU *)user_data;
	int no = (data & 0xffff0000) >> 16;
	int code = (data & 0xffff);
	switch(code) {
		case SDL_USEREVENT_SOCKET_CONNECTED:
			emu->socket_connected(no);
			break;
		case SDL_USEREVENT_SOCKET_DISCONNECTED:
			emu->socket_disconnected(no);
			break;
		case SDL_USEREVENT_SOCKET_WRITEABLE:
			emu->socket_writeable(no);
			emu->send_data(no);
			break;
//		case SDL_USEREVENT_SOCKET_READABLE:
//			emu->recv_data(no);
//			emu->socket_readable(no);
//			break;
//		case SDL_USEREVENT_SOCKET_ACCEPT:
//			emu->socket_accept(no);
//			break;
	}
}
#endif

/**********************************************************************/

CSocket::CSocket(Connection *conn, int ch)
{
	m_conn = conn;
	m_ch = ch;
	device = NULL;
#ifdef USE_SDL_NET
	tcp_socket = NULL;
	udp_socket = NULL;
#else
	soc = INVALID_SOCKET;
	udp_address = 0;
	udp_port = 0;
#endif
	is_tcp = true;
	is_server = false;
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

void CSocket::CheckArrived()
{
#ifdef USE_SDL_NET
	if (is_tcp) {
		if (SDLNet_SocketReady(tcp_socket)) {
			if (is_server) {
				emu->socket_accept(m_ch);
			} else {
				emu->recv_data(m_ch);
				emu->socket_readable(m_ch);
			}
		}
	} else {
		if (SDLNet_SocketReady(udp_socket)) {
			emu->recv_data(m_ch);
			emu->socket_readable(m_ch);
		}
	}
#else
	if (is_tcp) {
		if (is_server) {
			emu->socket_accept(m_ch);
		} else {
			emu->recv_data(m_ch);
			emu->socket_readable(m_ch);
		}
	} else {
		emu->recv_data(m_ch);
		emu->socket_readable(m_ch);
	}
#endif
}

#ifdef USE_SDL_NET
TCPsocket CSocket::Accept()
{
	return SDLNet_TCP_Accept(tcp_socket);
}
#else
SOCKET CSocket::Accept()
{
	SOCKET new_soc;
	struct sockaddr_in sock_addr;
	socklen_t sock_alen;

	if (!is_server) {
		logging->out_logf(LOG_ERROR, _T("CSocket::Accept: not server. ch=%d"), m_ch);
		return INVALID_SOCKET;
	}

	/* Accept a new TCP connection on a server socket */
	sock_alen = (socklen_t)sizeof(sock_addr);
	new_soc = accept(soc, (struct sockaddr *)&sock_addr, &sock_alen);
	if (new_soc == INVALID_SOCKET) {
		logging->out_logf(LOG_ERROR, _T("CSocket::Accept: accept() failed. ch=%d"), m_ch);
		return new_soc;
	}
#ifdef _WIN32
	{
		// passing a zero value, socket mode set to block on
		unsigned long mode = 0;
		ioctlsocket(new_soc, FIONBIO, &mode);
	}
#elif defined(O_NONBLOCK)
	{
		int flags = fcntl(new_soc, F_GETFL, 0);
		fcntl(new_soc, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif /* WIN32 */

	return new_soc;
}
#endif /* USE_SDL_NET */

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
	if (!recv_mux) return;

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

	if (!send_mux) return;

	// loop while send buffer is not empty
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

	// reserve channel for tcp connection
	is_tcp = true;
	device = NULL;

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

	// reserve channel for udp connection
	is_tcp = false;
	device = NULL;

	if (!server) {
		// client
#ifndef USE_SDL_NET
		int rc;
		struct sockaddr_in sock_addr;
		socklen_t sock_len;

		memset(&sock_addr, 0, sizeof(sock_addr));

		soc = socket(AF_INET, SOCK_DGRAM, 0);
		if (soc == INVALID_SOCKET) {
			logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: socket() failed ch=%d"), m_ch);
			return false;
		}

		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = INADDR_ANY;
		sock_addr.sin_port = 0;

		rc = bind(soc, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if (rc == SOCKET_ERROR) {
			logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: bind() failed ch=%d"), m_ch);
			Disconnect();
			return false;
		}

		sock_len = sizeof(sock_addr);
		rc = getsockname(soc, (struct sockaddr *)&sock_addr, &sock_len);
		if (rc < 0) {
			logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: getsockname() failed ch=%d"), m_ch);
			Disconnect();
			return false;
		}

#ifdef SO_BROADCAST
		// Allow LAN broadcasts with the socket
		{ int yes = 1;
			setsockopt(soc, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
		}
#endif
#ifdef IP_ADD_MEMBERSHIP
		/* Receive LAN multicast packets on 224.0.0.1
			This automatically works on Mac OS X, Linux and BSD, but needs
			this code on Windows.
		*/
		{
			struct ip_mreq  g;

			g.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
			g.imr_interface.s_addr = INADDR_ANY;
			setsockopt(soc, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&g, sizeof(g));
		}
#endif
#else
		UDPsocket sock;

		sock = SDLNet_UDP_Open(0);
		if(!sock) {
			logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: SDLNet_UDP_Open: ch=%d %s"), m_ch, SDLNet_GetError());
			return false;
		}
		int numused = SDLNet_UDP_AddSocket(m_conn->GetSocketSet(), sock);
		if(numused == -1) {
			SDLNet_UDP_Close(sock);
			logging->out_logf(LOG_ERROR, _T("CSocket::InitUDP: SDLNet_UDP_AddSocket: ch=%d %s"), m_ch, SDLNet_GetError());
			return false;
		}
		udp_socket = sock;
		is_server = server;
#endif
	}

	if (recv_buffer) recv_buffer->clear();
	if (send_buffer) send_buffer->clear();

	device = dev;

	return true;
}

bool CSocket::get_ipaddr(const _TCHAR *hostname, int port, uint32_t &ipaddr)
{
#ifndef USE_SDL_NET
	if (hostname == NULL) {
		ipaddr = INADDR_ANY;
		return true;
	}
#endif /* USE_SDL_NET */

#ifdef _UNICODE
	char host[_MAX_PATH];
	wcstombs(host, hostname, _MAX_PATH);
#else
	const char *host = hostname;
#endif

#ifndef USE_SDL_NET
	ipaddr = inet_addr(host);

	if (ipaddr == INADDR_NONE) {
		struct hostent *hp = gethostbyname(host);
		if (hp) {
			if (hp->h_length <= 4) {
				memcpy(&ipaddr, hp->h_addr, hp->h_length);
			} else {
				// IPv6?
				return false;
			}
		} else {
			return false;
		}
	}
#else
	IPaddress ip;
	if(SDLNet_ResolveHost(&ip, (const char *)host, (Uint16)port) == -1) {
		// unresolved hostname to ipaddr
		logging->out_logf(LOG_ERROR, _T("CSocket::get_ipaddr: SDLNet_ResolveHost: %s"), SDLNet_GetError());
		return false;
	}
	ipaddr = ip.host;
#endif /* USE_SDL_NET */

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
#ifdef USE_SDL_NET
	IPaddress ip;

	ip.host = (Uint32)ipaddr;
	ip.port = (Uint16)port;
#else
	int rc;
    struct sockaddr_in sock_addr;
#endif /* USE_SDL_NET */

	if (is_tcp) {
		if (server == false && (ipaddr == INADDR_NONE || ipaddr == INADDR_ANY)) {
			logging->out_log(LOG_ERROR, _T("Cannot create socket as the server."));
			return false;
		}
#ifndef USE_SDL_NET
		soc = socket(AF_INET, SOCK_STREAM, 0);
		if (soc == INVALID_SOCKET) {
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: socket() failed ch=%d"), m_ch);
			return false;
		}
		if (!server && ipaddr != INADDR_NONE && ipaddr != INADDR_ANY) {
			// client
			memset(&sock_addr, 0, sizeof(sock_addr));
			sock_addr.sin_family = AF_INET;
			sock_addr.sin_addr.s_addr = ipaddr;
			sock_addr.sin_port = htons(port);

			// Connect to the remote host
			rc = connect(soc, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
			if (rc == SOCKET_ERROR) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: connect() failed ch=%d"), m_ch);
				Disconnect();
				return false;
			}
			is_server = false;
		} else {
			// server
			memset(&sock_addr, 0, sizeof(sock_addr));
			sock_addr.sin_family = AF_INET;
			sock_addr.sin_addr.s_addr = ipaddr;
			sock_addr.sin_port = htons(port);

#ifndef WIN32
			// allow local address reuse
			{ int yes = 1;
				setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
			}
#endif

			// Bind the socket for listening
			rc = bind(soc, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
			if (rc == SOCKET_ERROR) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: bind() failed ch=%d"), m_ch);
				Disconnect();
				return false;
			}
			rc = listen(soc, 5);
			if (rc == SOCKET_ERROR) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: listen() failed ch=%d"), m_ch);
				Disconnect();
				return false;
			}

			// Set the socket to non-blocking mode for accept()
			// from SDL_Net
#if defined(__BEOS__) && defined(SO_NONBLOCK)
			/* On BeOS r5 there is O_NONBLOCK but it's for files only */
			{
				long b = 1;
				setsockopt(soc, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));
			}
#elif defined(O_NONBLOCK)
			{
				fcntl(soc, F_SETFL, O_NONBLOCK);
			}
#elif defined(_WIN32)
			{
				unsigned long mode = 1;
				ioctlsocket(soc, FIONBIO, &mode);
			}
#elif defined(__OS2__)
			{
				int dontblock = 1;
				ioctl(soc, FIONBIO, &dontblock);
			}
#else
#warning How do we set non-blocking mode on other operating systems?
#endif
			is_server = true;
		}
#else /* !USE_SDL_NET */
		TCPsocket sock;
		sock = SDLNet_TCP_Open(&ip, server ? SDL_TRUE : SDL_FALSE);
		if(!sock) {
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: SDLNet_TCP_Open: ch=%d %s"), m_ch, SDLNet_GetError());
			return false;
		}
		int numused = SDLNet_TCP_AddSocket(m_conn->GetSocketSet(), sock);
		if(numused == -1) {
			SDLNet_TCP_Close(sock);
			logging->out_logf(LOG_ERROR, _T("CSocket::Connect: SDLNet_TCP_AddSocket: ch=%d %s"), m_ch, SDLNet_GetError());
			return false;
		}
		tcp_socket = sock;
#endif /* USE_SDL_NET */
	} else {
		if (server) {
#ifndef USE_SDL_NET
			struct sockaddr_in sock_addr;
			socklen_t sock_len;

			memset(&sock_addr, 0, sizeof(sock_addr));

			soc = socket(AF_INET, SOCK_DGRAM, 0);
			if (soc == INVALID_SOCKET) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: socket() failed ch=%d"), m_ch);
				return false;
			}

			sock_addr.sin_family = AF_INET;
			sock_addr.sin_addr.s_addr = INADDR_ANY;
			sock_addr.sin_port = htons(port);

			rc = bind(soc, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
			if (rc == SOCKET_ERROR) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: bind() failed ch=%d"), m_ch);
				Disconnect();
				return false;
			}

			sock_len = sizeof(sock_addr);
			rc = getsockname(soc, (struct sockaddr *)&sock_addr, &sock_len);
			if (rc < 0) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: getsockname() failed ch=%d"), m_ch);
				Disconnect();
				return false;
			}

//			address.host = sock_addr.sin_addr.s_addr;
//			address.port = sock_addr.sin_port;

#ifdef SO_BROADCAST
			// Allow LAN broadcasts with the socket
			{ int yes = 1;
				setsockopt(soc, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
			}
#endif
#ifdef IP_ADD_MEMBERSHIP
			/* Receive LAN multicast packets on 224.0.0.1
				This automatically works on Mac OS X, Linux and BSD, but needs
				this code on Windows.
			*/
			{
				struct ip_mreq  g;

				g.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
				g.imr_interface.s_addr = INADDR_ANY;
				setsockopt(soc, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&g, sizeof(g));
			}
#endif
#else /* !USE_SDL_NET */
			UDPsocket sock;

			sock = SDLNet_UDP_Open(ip.port);
			if(!sock) {
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: SDLNet_UDP_Open: ch=%d %s"), m_ch, SDLNet_GetError());
				return false;
			}
			int channel = SDLNet_UDP_Bind(sock, -1, &ip);
			if (channel == -1) {
				SDLNet_UDP_Close(sock);
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: SDLNet_UDP_Bind: ch=%d %s"), m_ch, SDLNet_GetError());
				return false;
			}
			int numused = SDLNet_UDP_AddSocket(m_conn->GetSocketSet(), sock);
			if(numused == -1) {
				SDLNet_UDP_Close(sock);
				logging->out_logf(LOG_ERROR, _T("CSocket::Connect: SDLNet_UDP_AddSocket: ch=%d %s"), m_ch, SDLNet_GetError());
				return false;
			}
			udp_socket = sock;
#endif /* USE_SDL_NET */
		}
	}

	m_conn->IncreaseSocketNums();
	is_server = server;

	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX);
	if (!server && !recv_mux) recv_mux = new CMutex();
	if (!server && !send_mux) send_mux = new CMutex();

	// TODO: post SDL event
	if (!server) {
#ifndef USE_SDL_EVENT
		st_timer *timer1 = new st_timer;
		timer1->ch = m_ch;
		timer1->id = SDL_AddTimer(16, on_connected, (void *)timer1);
		st_timer *timer2 = new st_timer;
		timer2->ch = m_ch;
		timer2->id = SDL_AddTimer(32, on_writeable, (void *)timer2);
#else
#ifdef USE_GTK
		GtkWidget *window = ((EMU_OSD *)emu)->get_window();
		g_signal_emit_by_name(G_OBJECT(window), "user-socket"
			, m_ch << 16 | SDL_USEREVENT_SOCKET_CONNECTED);

		g_signal_emit_by_name(G_OBJECT(window), "user-socket"
			, m_ch << 16 | SDL_USEREVENT_SOCKET_WRITEABLE);
#else
		SDL_Event evt;
		evt.type = SDL_USEREVENT_SOCKET;
		evt.user.code = SDL_USEREVENT_SOCKET_CONNECTED;
		evt.user.data1 = (void *)(intptr_t)m_ch;
		evt.user.data2 = NULL;
		SDL_PushEvent(&evt);

		evt.type = SDL_USEREVENT_SOCKET;
		evt.user.code = SDL_USEREVENT_SOCKET_WRITEABLE;
		evt.user.data1 = (void *)(intptr_t)m_ch;
		evt.user.data2 = NULL;
		SDL_PushEvent(&evt);
#endif
#endif
	}
	return true;
}

void CSocket::Disconnect()
{
#ifndef USE_SDL_NET
	if (soc != INVALID_SOCKET) {
		closesocket(soc);
		soc = INVALID_SOCKET;
	}
#else
	if (is_tcp) {
		if (tcp_socket) {
			SDLNet_TCP_Close(tcp_socket);
			SDLNet_TCP_DelSocket(m_conn->GetSocketSet(), tcp_socket);
			tcp_socket = NULL;
		}
	} else {
		if (udp_socket) {
			SDLNet_UDP_Close(udp_socket);
			SDLNet_UDP_DelSocket(m_conn->GetSocketSet(), udp_socket);
			udp_socket = NULL;
		}
	}
#endif /* USE_SDL_NET */

	socket_delay = 0;
	m_conn->DecreaseSocketNums();
	if (device) device->network_disconnected(m_ch);
//	else vm->network_disconnected(ch);

	device = NULL;

	// TODO: post SDL event
#ifndef USE_SDL_EVENT
	st_timer *timer = new st_timer;
	timer->ch = m_ch;
	timer->id = SDL_AddTimer(16, on_disconnected, (void *)timer);
#else
#ifdef USE_GTK
	GtkWidget *window = ((EMU_OSD *)emu)->get_window();
	g_signal_emit_by_name(G_OBJECT(window), "user-socket"
		, m_ch << 16 | SDL_USEREVENT_SOCKET_DISCONNECTED);
#else
	SDL_Event evt;
	evt.type = SDL_USEREVENT_SOCKET;
	evt.user.code = SDL_USEREVENT_SOCKET_DISCONNECTED;
	evt.user.data1 = (void *)(intptr_t)m_ch;
	evt.user.data2 = NULL;
	SDL_PushEvent(&evt);
#endif
#endif
}

void CSocket::Shutdown()
{
#ifndef USE_SDL_NET
	if (soc != INVALID_SOCKET) {
		shutdown(soc, 2);
		closesocket(soc);
		soc = INVALID_SOCKET;
	}
#else
	SDLNet_TCP_Close(tcp_socket);
	SDLNet_UDP_Close(udp_socket);
	tcp_socket = NULL;
	udp_socket = NULL;
#endif /* USE_SDL_NET */
}

void CSocket::SendData()
{
	char buf[1024];

	if (!send_mux) return;

	send_mux->lock();
	int size = send_buffer->read(buf, sizeof(buf));
	send_mux->unlock();

	if(is_tcp) {
#ifndef USE_SDL_NET
		int sent_size = (int)send(soc, buf, size, 0);
#else
		int sent_size = SDLNet_TCP_Send(tcp_socket, (const void *)buf, size);
#endif /* USE_SDL_NET */
		if(sent_size < size) {
			// error
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
		}
	} else {
#ifndef USE_SDL_NET
		struct sockaddr_in sock_addr;
		sock_addr.sin_addr.s_addr = udp_address;
		sock_addr.sin_port = htons(udp_port);
		sock_addr.sin_family = AF_INET;

		int sent_size = (int)sendto(soc, buf, size, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if (sent_size < size) {
			// error
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
		}
#else
		udp_packet.data = (Uint8 *)buf;
		udp_packet.maxlen = size;
		udp_packet.len = size;
		int sent_packet = SDLNet_UDP_Send(udp_socket, udp_packet.channel, &udp_packet);
		if(sent_packet <= 0) {
			// error
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
		}
#endif /* USE_SDL_NET */
	}
}

void CSocket::RecvData()
{
	char buf[1024];
	int recv_size = 0;

	if (!recv_mux) return;

	if(is_tcp) {
#ifndef USE_SDL_NET
		recv_size = (int)recv(soc, (char *)buf, sizeof(buf), 0);
#else
		recv_size = SDLNet_TCP_Recv(tcp_socket, buf, sizeof(buf));
#endif /* USE_SDL_NET */
		if(recv_size <= 0) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		}
		recv_mux->lock();
		recv_size = recv_buffer->write(buf, recv_size);
		recv_mux->unlock();
	}
	else {
#ifndef USE_SDL_NET
		socklen_t sock_len;
		struct sockaddr_in sock_addr;

		sock_len = sizeof(sock_addr);
		recv_size = (int)recvfrom(soc, (buf + 8), sizeof(buf) - 8, 0, (struct sockaddr *)&sock_addr, &sock_len);
		if (recv_size <= 0) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		} else if (recv_size > 0) {
			buf[0] = sock_len >> 8;
			buf[1] = sock_len & 0xff;
			buf[2] = (char)sock_addr.sin_addr.s_addr;
			buf[3] = (char)(sock_addr.sin_addr.s_addr >> 8);
			buf[4] = (char)(sock_addr.sin_addr.s_addr >> 16);
			buf[5] = (char)(sock_addr.sin_addr.s_addr >> 24);
			buf[6] = (char)sock_addr.sin_port;
			buf[7] = (char)(sock_addr.sin_port >> 8);
			recv_mux->lock();
			recv_size = recv_buffer->write(buf, recv_size + 8);
			recv_mux->unlock();
		}
#else
		UDPpacket packet;

		packet.address.host = 0;
		packet.address.port = 0;
		packet.channel = 0;
		packet.maxlen = 0; // sizeof(buf) - 8;
		packet.len = 0;
		packet.data = NULL; // (Uint8 *)(buf + 8);

		int recv_packet = SDLNet_UDP_Recv(udp_socket, &packet);
		if(recv_packet < 0) {
			emu->disconnect_socket(m_ch);
			emu->socket_disconnected(m_ch);
			return;
		} else if (recv_packet > 0) {
			buf[0] = packet.len >> 8;
			buf[1] = packet.len & 0xff;
			buf[2] = (char)packet.address.host;
			buf[3] = (char)(packet.address.host >> 8);
			buf[4] = (char)(packet.address.host >> 16);
			buf[5] = (char)(packet.address.host >> 24);
			buf[6] = (char)packet.address.port;
			buf[7] = (char)(packet.address.port >> 8);
			recv_mux->lock();
			recv_size += recv_buffer->write(buf, 8);
			recv_size += recv_buffer->write((const char *)packet.data, packet.len);
			recv_mux->unlock();
		}
#endif /* USE_SDL_NET */
	}
}

#ifdef USE_SDL_NET
void CSocket::Set(TCPsocket sock, bool tcp, DEVICE *dev)
{
	tcp_socket = sock;
	is_tcp = tcp;
	device = dev;
	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!recv_mux) recv_mux = new CMutex();
	if (!send_mux) send_mux = new CMutex();
}
#else
void CSocket::Set(SOCKET sock, bool tcp, DEVICE *dev)
{
	soc = sock;
	is_tcp = tcp;
	device = dev;
	if (!recv_buffer) recv_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!send_buffer) send_buffer = new FIFOCHAR(SOCKET_BUFFER_MAX); 
	if (!recv_mux) recv_mux = new CMutex();
	if (!send_mux) send_mux = new CMutex();
}
#endif /* USE_SDL_NET */

void CSocket::SetUDPAddress(uint32_t ipaddr, int port)
{
#ifndef USE_SDL_NET
	udp_address = ipaddr;
	udp_port = (uint16_t)port;
#else
	udp_packet.address.host = (Uint32)ipaddr;
	udp_packet.address.port = (Uint16)port;
	udp_packet.channel = -1;
#endif /* USE_SDL_NET */
}

bool CSocket::IsConnecting() const
{
#ifdef USE_SDL_NET
	return (tcp_socket != NULL || udp_socket != NULL);
#else
	return (soc != INVALID_SOCKET);
#endif /* USE_SDL_NET */
}

/**********************************************************************/

Connection::Connection()
{
	// init sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		socs.Add(new CSocket(this, i));
	}
#ifdef USE_SDL_NET
	socket_set = NULL;
#endif
	socket_nums = 0;

#ifdef USE_GTK
	// create new user defined signal
	g_signal_new("user-socket",
		GTK_TYPE_WIDGET,
		G_SIGNAL_RUN_LAST,
		0,
		NULL, NULL,
		g_cclosure_marshal_VOID__INT,
		G_TYPE_NONE, 1, G_TYPE_INT);
#endif
}

Connection::~Connection()
{
	// release sockets
	for(int i = 0; i < SOCKET_MAX; i++) {
		socs.Item(i)->Shutdown();
	}

#ifdef USE_SDL_NET
	SDLNet_FreeSocketSet(socket_set);
	socket_set = NULL;

	// release sdl net
	SDLNet_Quit();
#else
#ifdef _WIN32
	WSACleanup();
#else
    // Restore the SIGPIPE handler
    void (*handler)(int);
    handler = signal(SIGPIPE, SIG_DFL);
    if ( handler != SIG_IGN ) {
        signal(SIGPIPE, handler);
    }
#endif
#endif /* USE_SDL_NET */
}

void Connection::Initialize()
{
#ifndef USE_SDL_NET
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(0x0101, &wsaData) != 0) {
		logging->out_log(LOG_ERROR, _T("Connection::Initialize: WSAStartup failed."));
	}
#else
	// SIGPIPE is generated when a remote socket is closed
	void (*handler)(int);
	handler = signal(SIGPIPE, SIG_IGN);
	if ( handler != SIG_DFL ) {
		signal(SIGPIPE, handler);
	}
#endif
#else
	// init sdl net
	if(SDLNet_Init()==-1) {
		logging->out_logf(LOG_ERROR, _T("Connection::Initialize: SDLNet_Init: %s"), SDLNet_GetError());
	}
	socket_set = SDLNet_AllocSocketSet(SOCKET_MAX);
	if(!socket_set) {
		logging->out_logf(LOG_ERROR, _T("Connection::Initialize: SDLNet_AllocSocketSet: %s"), SDLNet_GetError());
	}
#endif /* USE_SDL_NET */
#ifdef USE_GTK
	// connect signal
	GtkWidget *window = ((EMU_OSD *)emu)->get_window();
	g_signal_connect(G_OBJECT(window), "user-socket",
		G_CALLBACK(OnUserSocket), (gpointer)emu);
#endif
}

void Connection::CheckArrived()
{
	if (socket_nums > 0) {
#ifndef USE_SDL_NET
		int retval;
		struct timeval tv;
		int maxfd = -1;
		fd_set mask;

		FD_ZERO(&mask);

		// copy
		for(int i=0; i<SOCKET_MAX; i++) {
			SOCKET s = socs.Item(i)->GetSocket();
			if (s != INVALID_SOCKET) {
				FD_SET(s, &mask);
				if (maxfd < (int)s) {
					maxfd = (int)s;
				}
			}
		}

		// immediately
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		// arrived?
		retval = select(maxfd+1, &mask, NULL, NULL, &tv);
		if (retval < 0) {
			// error
			logging->out_log(LOG_ERROR, _T("Connection::CheckArrived: select() failed."));
		} else if (retval > 0) {
			// data arrived
			int hit = 0;
			for(int i = 0, c = 0; i < SOCKET_MAX && c < retval; i++) {
				CSocket *soc = socs.Item(i);
				SOCKET s = soc->GetSocket();
				if (FD_ISSET(s, &mask)) {
					hit |= (1 << i);
					c++;
				}
			}
			for(int i = 0; i < SOCKET_MAX; i++) {
				if (hit & 1) {
					CSocket *soc = socs.Item(i);
					soc->CheckArrived();
				}
				hit >>= 1;
			}
		}
#else
		int retval = SDLNet_CheckSockets(socket_set, 0);
		if (retval < 0) {
			// error
			logging->out_logf(LOG_ERROR, _T("Connection::CheckArrived: SDLNet_CheckSockets: %s"), SDLNet_GetError());
		} else if (retval > 0) {
			// data arrived
			for(int i = 0; i < SOCKET_MAX; i++) {
				CSocket *soc = socs.Item(i);
				soc->CheckArrived();
			}
		}
#endif /* USE_SDL_NET */
	}
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

#ifndef USE_SDL_NET
	SOCKET new_tcpsock = soc->Accept();
	if (new_tcpsock == INVALID_SOCKET) {
		return;
	}
#else
	TCPsocket new_tcpsock = soc->Accept();
	if (new_tcpsock == NULL) {
		logging->out_logf(LOG_ERROR, _T("Connection::Accept: CSocket::Accept: ch=%d %s"), ch, SDLNet_GetError());
		return;
	}
#endif

	// find empty socket
	new_ch = GetEmptySocket();
	if (new_ch < 0) {
#ifndef USE_SDL_NET
		closesocket(new_tcpsock);
#else
		SDLNet_TCP_Close(new_tcpsock);
#endif
		return;
	}

#ifdef USE_SDL_NET
	int numused = SDLNet_TCP_AddSocket(socket_set, new_tcpsock);
	if(numused == -1) {
		SDLNet_TCP_Close(new_tcpsock);
		logging->out_logf(LOG_ERROR, _T("Connection::Accept: SDLNet_TCP_AddSocket: new_ch=%d %s"), new_ch, SDLNet_GetError());
		return;
	}
	IPaddress *remote_ip = SDLNet_TCP_GetPeerAddress(new_tcpsock);
	if (remote_ip) {
		logging->out_debugf(_T("accepted from 0x%08x"), remote_ip->host);
	}
#endif

	DEVICE *dev = soc->GetDevice();

	CSocket *new_soc = socs.Item(new_ch);

	new_soc->Set(new_tcpsock, soc->IsTCP(), dev);

	IncreaseSocketNums();

	if (dev) dev->network_accepted(ch, new_ch);
//	else vm->network_accepted(ch, new_ch);

	logging->out_debugf(_T("Connection::Accept: ok. ch=%d is_tcp=%s new_ch=%d is_tcp=%s")
		, ch, soc->IsTCP() ? _T("true") : _T("false"), new_ch, new_soc->IsTCP() ? _T("true") : _T("false"));

	// TODO: post SDL event
	// note that i'll fire the writeable event only, won't fire the connected event.
#ifndef USE_SDL_EVENT
	st_timer *timer2 = new st_timer;
	timer2->ch = ch;
	timer2->id = SDL_AddTimer(32, on_writeable, (void *)timer2);
#else
#ifdef USE_GTK
	GtkWidget *window = ((EMU_OSD *)emu)->get_window();
	g_signal_emit_by_name(G_OBJECT(window), "user-socket"
		, new_ch << 16 | SDL_USEREVENT_SOCKET_WRITEABLE);
#else
	SDL_Event evt;
	evt.type = SDL_USEREVENT_SOCKET;
	evt.user.code = SDL_USEREVENT_SOCKET_WRITEABLE;
	evt.user.data1 = (void *)(intptr_t)new_ch;
	evt.user.data2 = NULL;
	SDL_PushEvent(&evt);
#endif
#endif
}

CSocket *Connection::Item(int ch)
{
//	if (ch < 0 || SOCKET_MAX <= ch) return NULL;
	return socs.Item(ch);
}

void Connection::IncreaseSocketNums()
{
	if (socket_nums < SOCKET_MAX) socket_nums++;
}

void Connection::DecreaseSocketNums()
{
	if (socket_nums > 0) socket_nums--;
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
