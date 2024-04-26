/** @file debugger_socket.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger socket ]
*/

#include "debugger_socket.h"

#ifdef USE_DEBUGGER

#include "emu.h"
#include "logging.h"
#include "config.h"
#include "cmutex.h"
#include "utility.h"

DebuggerSocket::DebuggerSocket(EMU *parent_emu, const char *identifier)
	: DEVICE(parent_emu, identifier)
{
	set_class_name("DbgrSock");

	server = false;
	client_ch = -1;
	server_ch = -1;
	connect = false;

	memset(send_buff, 0, sizeof(send_buff));
	send_buff_w_pos = 0;
	send_buff_r_pos = 0;
	memset(recv_buff, 0, sizeof(recv_buff));
	recv_buff_w_pos = 0;
	recv_buff_r_pos = 0;

	init_lock();
}

DebuggerSocket::~DebuggerSocket()
{
	term_lock();
}

void DebuggerSocket::enable_server(bool enable)
{
	if (enable) {
		if (pConfig->debugger_server_port == 0 || _tcslen(pConfig->debugger_server_host.Get()) == 0) {
			enable = false;
		}
	}

	if (!server && enable) {
		// connection active as client
		if (client_ch != -1) {
			// error
			logging->out_log(LOG_ERROR, _T("Already connecting to another debugger server."));
			return;
		}
		// get socket channel
		server_ch = emu->get_socket_channel();
		if (server_ch < 0) {
			return;
		}
		// start server
		if (!emu->init_socket_tcp(server_ch, this, true)) {
			server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return;
		}
		if (!emu->connect_socket(server_ch, pConfig->debugger_server_host.Get(), pConfig->debugger_server_port, true)) {
			server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot start as debugger server."));
			return;
		}
//		logging->out_log(LOG_DEBUG, _T("Started debugger server."));
		server = true;
	} else {
		// stop server
		if (client_ch != -1) {
			// disconnect client
			emu->disconnect_socket(client_ch);
			client_ch = -1;
		}
		if (server_ch >= 0) {
			emu->disconnect_socket(server_ch);
			server_ch = -1;
		}
//		logging->out_log(LOG_DEBUG, _T("Stopped debugger server."));
		server = false;
	}
}

bool DebuggerSocket::connect_socket()
{
	if (!server && client_ch == -1) {
		// get socket channel
		client_ch = emu->get_socket_channel();
		if (client_ch < 0) {
			return false;
		}
		// connect
		if (!emu->init_socket_tcp(client_ch, this)) {
			client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return false;
		}
		if (!emu->connect_socket(client_ch, pConfig->debugger_server_host.Get(), pConfig->debugger_server_port)) {
			client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot connect to debugger server."));
			return false;
		}
//		logging->out_log(LOG_DEBUG, _T("Connect debugger client."));
	}
	return true;
}

void DebuggerSocket::disconnect_socket()
{
	if (client_ch != -1) {
		// disconnect
		emu->disconnect_socket(client_ch);
//		logging->out_log(LOG_DEBUG, _T("Disconnect debugger client."));
	}
}

void DebuggerSocket::enable_connect()
{
	if (!connect) {
		connect_socket();
	} else {
		disconnect_socket();
	}
}


uint8_t* DebuggerSocket::get_sendbuffer(int ch, int* size, int* flags)
{
	if (ch == client_ch) {
		*size = (send_buff_w_pos - send_buff_r_pos);
		return &send_buff[send_buff_r_pos];
	} else {
		*size = 0;
		return NULL;
	}
}
void DebuggerSocket::inc_sendbuffer_ptr(int ch, int size)
{
	if (ch == client_ch) {
		send_buff_r_pos += size;
		if (send_buff_w_pos <= send_buff_r_pos) {
			// all data sent.
			send_buff_w_pos = 0;
			send_buff_r_pos = 0;
		}
		return;
	}
}
uint8_t* DebuggerSocket::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
	if (ch == client_ch) {
		lock();
		*size0 = (DEBUGGER_SOCKET_MAX_BUFF - recv_buff_w_pos);
		*size1 = 0;
		return &recv_buff[recv_buff_w_pos];
	} else {
		*size0 = 0;
		*size1 = 0;
		return NULL;
	}
}

uint8_t* DebuggerSocket::get_recvbuffer1(int ch)
{
	if (ch == client_ch) {
		return recv_buff;
	} else {
		return NULL;
	}
}

void DebuggerSocket::inc_recvbuffer_ptr(int ch, int size)
{
	if (ch == client_ch) {
		recv_buff_w_pos += size;
		unlock();
	}
}

void DebuggerSocket::network_connected(int ch)
{
	if (ch == client_ch) {
		connect = true;
	}
}
void DebuggerSocket::network_disconnected(int ch)
{
	if (ch == client_ch) {
		connect = false;
		client_ch = -1;
	} else if (ch == server_ch) {
		server_ch = -1;
	}
}
void DebuggerSocket::network_writeable(int ch)
{
}
void DebuggerSocket::network_readable(int ch)
{
}
void DebuggerSocket::network_accepted(int ch, int new_ch)
{
	if (ch == server_ch) {
		if (client_ch != -1) {
			// Another client is already connected on my server.
			emu->disconnect_socket(new_ch);
		}
		client_ch = new_ch;
		connect = true;

		emu->debugger_terminal_accepted();
	}
}

/// @brief write data to telnet network client
///
/// @param[in]  data
/// @param[in]  size : data size
/// @note always treat data as 8bit char even if data is widechar
void DebuggerSocket::write_data(const _TCHAR *data, int size)
{
	send_buff_r_pos = 0;
	send_buff_w_pos = 0;
	// always treat data as 8bit char even if data is widechar
	// convert lf to crlf
	for(int i=0; i<size && send_buff_w_pos < DEBUGGER_SOCKET_MAX_BUFF; i++) {
		if (data[i] == 0x0a) {
			send_buff[send_buff_w_pos++] = 0x0d;
		}
		send_buff[send_buff_w_pos++] = (data[i] & 0xff);
	}
	if (client_ch >= 0 && send_buff_r_pos < send_buff_w_pos) {
		emu->send_data_tcp(client_ch);
	}
}

/// @brief read data from telnet network client
///
/// @param[out] data
/// @param[in]  size : buffer size
/// @note always treat data as 8bit char even if data is widechar
int DebuggerSocket::read_data(_TCHAR *data, int size)
{
	int read_size = 0;
	lock();
	if (recv_buff_r_pos < recv_buff_w_pos) {
		read_size = recv_buff_w_pos - recv_buff_r_pos;
		if (size < read_size) read_size = size;
		// always treat data as 8bit char even if data is widechar
		for(int i=0; i<read_size; i++) {
			data[i] = recv_buff[recv_buff_r_pos + i];
		}
		// shift
		memcpy(&recv_buff[0], &recv_buff[recv_buff_r_pos], read_size);
		recv_buff_w_pos -= read_size;
		recv_buff_r_pos = 0;
	}
	unlock();
	return read_size;
}

void DebuggerSocket::init_lock()
{
	mutex = new CMutex();
}

void DebuggerSocket::term_lock()
{
	delete mutex;
	mutex = NULL;
}

void DebuggerSocket::lock()
{
	mutex->lock();
}

void DebuggerSocket::unlock()
{
	mutex->unlock();
}


#endif /* USE_DEBUGGER */
