/** @file win_socket.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.05.08 -

	@brief [ win32 socket ]
*/

#ifndef WIN_SOCKET_H
#define WIN_SOCKET_H

#include <winsock.h>
#include "../../cptrlist.h"

class Connection;
class FIFOCHAR;

/**
	@brief Manage a network socket 
*/
class CSocket
{
private:
	Connection *m_conn;
	int m_ch;

	SOCKET soc;
	DEVICE *device;
	bool is_tcp;
	struct sockaddr_in udpaddr;
	int socket_delay;

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

	void SendToDevice();
	void RecvFromDevice();
	void RecvFromDevice(const _TCHAR *hostname, int port);
	void RecvFromDevice(uint32_t ipaddr, int port);

	bool InitTCP(DEVICE *dev, bool server);
	bool InitUDP(DEVICE *dev, bool server);

	bool Connect(const _TCHAR *hostname, int port, bool server);
	bool Connect(uint32_t ipaddr, int port, bool server);
	void Disconnect();
	void Shutdown();

	void SendData();
	void RecvData();

	bool IsConnecting() const;
	void Set(SOCKET sock, bool tcp, DEVICE *dev);

	void SetSocket(SOCKET nsock) { soc = nsock; } 
	SOCKET GetSocket() { return soc; } 
	const SOCKET GetSocket() const { return soc; } 
	void SetTCP(bool val) { is_tcp = val; }
	bool IsTCP() const { return is_tcp; }
	void SetDevice(DEVICE *dev) { device = dev; }
	DEVICE *GetDevice() { return device; }

	void SetDisconnectDelay(int val);

	FIFOCHAR *GetRecvBuffer() { return recv_buffer; }
};

/**
	@brief Manage CSocket list
*/
class Connection
{
private:
	CPtrList<CSocket> socs;

public:
	Connection();
	~Connection();

	void Initialize();

	int GetEmptySocket() const;
	void Accept(int ch);

	CSocket *Item(int ch);

	void SetDisconnectDelay(int ch, int val);
	const SOCKET GetSocket(int ch) const;

	void SetDevice(int ch, DEVICE *dev);
	DEVICE *GetDevice(int ch);
};

#endif /* WIN_SOCKET_H */
