/** @file sdl_socket.h

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2021.05.06

	@brief [ sdl socket ]
*/

#ifndef SDL_SOCKET_H
#define SDL_SOCKET_H

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#ifdef USE_SDL_NET
#include <SDL_net.h>
#endif
#endif
#ifdef _WIN32
#include <WinSock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include "../../cptrlist.h"

#ifndef _WIN32
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

class DEVICE;
class Connection;
class FIFOCHAR;
class CMutex;

/**
	@brief Manage a network socket
*/
class CSocket
{
private:
	Connection *m_conn;
	int m_ch;
	DEVICE *device;
	bool is_tcp;
	int socket_delay;
#ifdef USE_SDL_NET
	TCPsocket tcp_socket;
	UDPsocket udp_socket;
	UDPpacket udp_packet;
#else
	SOCKET soc;
	uint32_t udp_address;
	uint16_t udp_port;
#endif
	bool is_server;

	FIFOCHAR *recv_buffer;
	CMutex *recv_mux;
	FIFOCHAR *send_buffer;
	CMutex *send_mux;

	static int read_and_remove_dup_iac(char *dst, int size, FIFOCHAR *buffer);
	static int write_and_escape_iac(const char *src, int size, FIFOCHAR *buffer);

	static bool get_ipaddr(const _TCHAR *hostname, int port, uint32_t &ipaddr);
	void SetUDPAddress(uint32_t ipaddr, int port);

public:
	CSocket(Connection *conn, int ch);
	~CSocket();

	void CheckArrived();
#ifdef USE_SDL_NET
	TCPsocket Accept();
#else
	SOCKET Accept();
#endif
	void SendToDevice();
	void RecvFromDevice();
	void RecvFromDevice(const _TCHAR *hostname, int port);
	void RecvFromDevice(uint32_t ipaddr, int port);

	bool InitTCP(DEVICE *dev, bool server);
	bool InitUDP(DEVICE *dev, bool server);

	bool Connect(const _TCHAR *hostname, int port, bool server);
	bool Connect(uint32_t ipaddr, int port, bool server);
//	bool Connect(IPaddress &ip, bool server);
	void Disconnect();
	void Shutdown();

	void SendData();
	void RecvData();

	bool IsConnecting() const;
#ifdef USE_SDL_NET
	void Set(TCPsocket sock, bool tcp, DEVICE *dev);
	void SetTCPsocket(TCPsocket sock) { tcp_socket = sock; } 
	TCPsocket GetTCPsocket() { return tcp_socket; } 
#else
	void Set(SOCKET sock, bool tcp, DEVICE *dev);
	SOCKET GetSocket() { return soc; }
#endif

	void SetTCP(bool val) { is_tcp = val; }
	bool IsTCP() const { return is_tcp; }
	void SetDevice(DEVICE *dev) { device = dev; }
	DEVICE *GetDevice() { return device; }

	FIFOCHAR *GetRecvBuffer() { return recv_buffer; }
	FIFOCHAR *GetSendBuffer() { return send_buffer; }
};

/**
	@brief Manage CSocket list
*/
class Connection
{
private:
	CPtrList<CSocket> socs;

#ifdef USE_SDL_NET
	SDLNet_SocketSet socket_set;
#endif
	int socket_nums;

public:
	Connection();
	~Connection();

	void Initialize();

	void CheckArrived();
	int GetEmptySocket() const;
	void Accept(int ch);

	CSocket *Item(int ch);

#ifdef USE_SDL_NET
	SDLNet_SocketSet GetSocketSet() { return socket_set; }
#endif
	void IncreaseSocketNums();
	void DecreaseSocketNums();
	void SetDevice(int ch, DEVICE *dev);
	DEVICE *GetDevice(int ch);
};

#endif /* SDL_SOCKET_H */
