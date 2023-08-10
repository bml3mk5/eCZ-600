/** @file debugger_socket.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger socket ]
*/

#ifndef DEBUGGER_SOCKET_H
#define DEBUGGER_SOCKET_H

#include "common.h"
#include "vm/vm_defs.h"
#include "vm/device.h"

#ifdef USE_DEBUGGER

#define DEBUGGER_SOCKET_MAX_BUFF 1024

class CMutex;

/**
	@brief Accessing debugger console via network
*/
class DebuggerSocket : public DEVICE
{
private:
	bool server;
	int client_ch;
	int server_ch;
	bool connect;

//	bool now_receiving;
	uint8_t send_buff[DEBUGGER_SOCKET_MAX_BUFF];
	int send_buff_w_pos;
	int send_buff_r_pos;
	uint8_t recv_buff[DEBUGGER_SOCKET_MAX_BUFF];
	int recv_buff_w_pos;
	int recv_buff_r_pos;

	CMutex *mutex;

	bool connect_socket();
	void disconnect_socket();

	inline void init_lock();
	inline void term_lock();
	inline void lock();
	inline void unlock();

public:
	DebuggerSocket(EMU *parent_emu, const char *identifier);
	~DebuggerSocket();

	void enable_server(bool enable);
	void enable_connect();
	void network_connected(int ch);
	void network_disconnected(int ch);
	void network_writeable(int ch);
	void network_readable(int ch);
	void network_accepted(int ch, int new_ch);
	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
	uint8_t* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);

	void write_data(const _TCHAR *data, int size);
	int  read_data(_TCHAR *data, int size);
};

#endif /* USE_DEBUGGER */

#endif /* DEBUGGER_SOCKET_H */
