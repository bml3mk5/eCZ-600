/** @file comm.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ comm ]
*/

#include "comm.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "scc.h"
#include "../../utility.h"

void COMM::initialize()
{
	// RS-232C
//	is_rs = 1; //(cfg_num ? 1 : 0);
//	dipswitch = 1;
	m_through = true;
//	receive_data = 1;
	m_received = false;

	m_register_id = -1;

	m_connect = DISCONNECT;
	m_client_ch = -1;
	m_server_ch = -1;
	m_uart_ch = -1;

	m_send_telcmd_pos = -1;

	m_rts = false;
	m_send_speed = 100.0;

	register_frame_event(this);
}

void COMM::reset()
{
	warm_reset(true);
}

void COMM::warm_reset(bool por)
{
//	is_rs = 1; //(cfg_num ? 1 : 0);
//	dipswitch = 1;
	m_through = true;
//	receive_data = 1;
	m_received = false;

	memset(m_send_buff, 0, sizeof(m_send_buff));
	m_send_buff_w_pos = 0;
	m_send_buff_r_pos = 0;
	memset(m_recv_buff, 0, sizeof(m_recv_buff));
	m_recv_buff_w_pos = 0;
	m_recv_buff_r_pos = 0;

	m_rts = false;
	m_send_speed = 100.0;

	cancel_my_event();
}

void COMM::release()
{
}

void COMM::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t COMM::read_io8(uint32_t addr)
{
	return 0xff;
}

// relay
void COMM::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SCC::SIG_TXDA:
			send_data(data);
			break;
		case SCC::SIG_RTSA:
			if (data & mask) {
				// stop sending to device
				m_rts = false;
			} else if (!m_rts && ((data & mask) == 0)) {
				// accept sending to device
				m_rts = true;
				register_my_event();
			}
			break;
		case SIG_CPU_RESET:
			now_reset = ((data & mask) != 0);
			if (!now_reset) {
				warm_reset(false);
			}
			break;
	}
}

void COMM::send_data(uint32_t data)
{
	if (m_send_buff_w_pos < COMM_MAX_BUFF) {
		if (m_through) {
			m_send_buff[m_send_buff_w_pos++] = (data & 0xff);
		} else {
			m_send_buff[m_send_buff_w_pos++] = (data & 0xff) | 0x30;
		}
	}
	if (m_send_buff_r_pos < m_send_buff_w_pos) {
#ifdef USE_SOCKET
		if (m_client_ch >= 0) {
			emu->send_data_tcp(m_client_ch);
		}
#endif
#ifdef USE_UART
		if (m_uart_ch >= 0) {
			emu->send_uart_data(m_uart_ch);
		}
#endif
	}
}

uint32_t COMM::recv_data()
{
	uint32_t data = 0;
	if (m_recv_buff_r_pos < m_recv_buff_w_pos) {
		if (m_through) {
			data = m_recv_buff[m_recv_buff_r_pos++];
		} else {
			data = (m_recv_buff[m_recv_buff_r_pos++] & 1);
		}
	}
	return data;
}

void COMM::register_my_event()
{
	if(m_register_id == -1) {
		m_send_speed = d_ctrl->get_recv_speed_usec(0, m_through != 0);
		register_event(this, EVENT_TXD, m_send_speed, true, &m_register_id);
	}
}

void COMM::cancel_my_event()
{
	if(m_register_id != -1) {
		cancel_event(this, m_register_id);
		m_register_id = -1;
	}
}


// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void COMM::event_frame()
{
#if 0
	if (m_send_buff_r_pos < m_send_buff_w_pos) {
#ifdef USE_SOCKET
		if (m_client_ch >= 0) {
			emu->send_data_tcp(m_client_ch);
		}
#endif
#ifdef USE_UART
		if (m_uart_ch >= 0) {
			emu->send_uart_data(m_uart_ch);
		}
#endif
	}
#endif
	if (m_recv_buff_r_pos > 0 && m_recv_buff_r_pos < m_recv_buff_w_pos) {
		memcpy(&m_recv_buff[0], &m_recv_buff[m_recv_buff_r_pos], m_recv_buff_w_pos - m_recv_buff_r_pos);
		m_recv_buff_w_pos -= m_recv_buff_r_pos;
		m_recv_buff_r_pos = 0;
	}
}

void COMM::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_TXD:
		// send to device (SCC)
		if (m_received) {
			// data exists in buffer
			uint32_t data = recv_data();
			d_ctrl->write_signal(SCC::SIG_RXDA, data, 0xff);
			if (m_recv_buff_r_pos >= m_recv_buff_w_pos) {
				m_received = false;
			}
		}
		if (!m_rts) {
			// stop requset to send
			cancel_my_event();
		}
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------
/// @note called by main thread
void COMM::enable_server()
{
#ifdef USE_SOCKET
	if (!pConfig->comm_server[m_cfg_num]) {
		// connection active as client
		if (m_client_ch != -1) {
			// error
			logging->out_log(LOG_ERROR, _T("Already connecting to another comm server."));
			return;
		}
		// get socket channel
		m_server_ch = emu->get_socket_channel();
		if (m_server_ch < 0) {
			return;
		}
		// start server
		if (!emu->init_socket_tcp(m_server_ch, this, true)) {
			m_server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return;
		}
		if (!emu->connect_socket(m_server_ch, pConfig->comm_server_host[m_cfg_num], pConfig->comm_server_port[m_cfg_num], true)) {
			m_server_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot start as comm server."));
			return;
		}
//		logging->out_log(LOG_DEBUG, _T("Started comm server."));
		pConfig->comm_server[m_cfg_num] = true;
	} else {
		// stop server
		if (m_client_ch != -1) {
			// disconnect client
			emu->disconnect_socket(m_client_ch);
		}
		emu->disconnect_socket(m_server_ch);
//		logging->out_log(LOG_DEBUG, _T("Stopped comm server."));
		pConfig->comm_server[m_cfg_num] = false;
	}
#endif
}

/// @note called by main thread
void COMM::enable_connect(int num)
{
	if (num == 0) {
		// Ethernet
		if (!emu->is_connecting_socket(m_client_ch)) {
			disconnect_all();
			connect_socket();
		} else {
			disconnect_all();
		}
#ifdef USE_UART
	} else if(num > 0) {
		// COM port on host
		if (!emu->is_opened_uart(num - 1)) {
			disconnect_all();
			connect_uart(num - 1);
		} else {
			disconnect_all();
		}
#endif
	}
}

/// @note called by main thread
bool COMM::now_connecting(int num)
{
	if (num == 0) {
		// Ethernet
		return emu->is_connecting_socket(m_client_ch);
#ifdef USE_UART
	} else if(num > 0) {
		// COM port on host
		return emu->is_opened_uart(num - 1);
#endif
	} else {
		return false;
	}
}

/// @note called by main thread
void COMM::send_telnet_command(int num)
{
	char buf[8];
	int len = 0;
	buf[0]=0;
	if (m_server_ch < 0 || m_client_ch < 0) {
		return;
	}
	switch(num) {
	case 0:
		// WILL/DO BINARY
		len = 6;
		memcpy(buf, "\xff\xfb\x00\xff\xfd\x00", len);
		pConfig->comm_binary[m_cfg_num] = true;
		break;
	case 0x10:
		// WON'T/DON'T BINARY
		len = 6;
		memcpy(buf, "\xff\xfc\x00\xff\xfe\x00", len);
		pConfig->comm_binary[m_cfg_num] = false;
		break;
	case 1:
		// WILL Suppress Go Ahead and WILL ECHO 
		len = 6;
		memcpy(buf, "\xff\xfb\x03\xff\xfb\x01", len);
		break;
	}
	if (len > 0 && m_send_buff_w_pos < (COMM_MAX_BUFF - len)) {
		memcpy(&m_send_buff[m_send_buff_w_pos], buf, len);
		m_send_buff_w_pos += len;
		m_send_telcmd_pos = m_send_buff_w_pos;
	}
}

/// @note called by emu thread
uint8_t* COMM::get_sendbuffer(int ch, int* size, int* flags)
{
	if (ch == m_client_ch || ch == m_uart_ch) {
		*size = (m_send_buff_w_pos - m_send_buff_r_pos);
		*flags = (pConfig->comm_binary[m_cfg_num] && m_send_telcmd_pos < 0) ? 1 : 0;
		return &m_send_buff[m_send_buff_r_pos];
	} else {
		*size = 0;
		return NULL;
	}
}

/// @note called by emu thread
void COMM::inc_sendbuffer_ptr(int ch, int size)
{
	if (ch == m_client_ch || ch == m_uart_ch) {
		m_send_buff_r_pos += size;
		if (m_send_buff_r_pos >= m_send_telcmd_pos) {
			m_send_telcmd_pos = -1;
		}
		if (m_send_buff_w_pos <= m_send_buff_r_pos) {
			// all data sent.
			m_send_buff_w_pos = 0;
			m_send_buff_r_pos = 0;
		}
	}
}

/// @note called by emu thread
uint8_t* COMM::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
//	logging->out_logf(LOG_DEBUG, _T("get_recvbuffer0: ch:%d client_ch:%d"),ch,client_ch);
	if (ch == m_client_ch || ch == m_uart_ch) {
		*size0 = (COMM_MAX_BUFF - m_recv_buff_w_pos);
		*size1 = 0;
		*flags = pConfig->comm_binary[m_cfg_num] ? 1 : 0;
//		logging->out_logf(LOG_DEBUG, _T("Recve buffer. w:%d size:%d"),recv_buff_w_pos,*size0);
//		if (*size0 == 0) {
//			if (d_ctrl) d_ctrl->write_signal(ACIA::SIG_ACIA_ERR_OVRN, 1, 1);
//		}
		return &m_recv_buff[m_recv_buff_w_pos];
	} else {
		*size0 = 0;
		*size1 = 0;
		return NULL;
	}
}

/// @note called by emu thread
uint8_t* COMM::get_recvbuffer1(int ch)
{
	if (ch == m_client_ch || ch == m_uart_ch) {
		return m_recv_buff;
	} else {
		return NULL;
	}
}

/// @note called by emu thread
void COMM::inc_recvbuffer_ptr(int ch, int size)
{
//	logging->out_logf(LOG_DEBUG, _T("inc_recvbuffer_ptr: ch:%d client_ch:%d"),ch,client_ch);
	if (ch == m_client_ch || ch == m_uart_ch) {
//		logging->out_logf(LOG_DEBUG, _T("Recvd buffer. w:%d size:%d"),recv_buff_w_pos,size);
		if (size > 0) {
			m_recv_buff_w_pos += size;
			m_received = true;
		}
	}
}

/// @note called by main thread
bool COMM::connect_socket()
{
#ifdef USE_SOCKET
	if (!pConfig->comm_server[m_cfg_num] && m_client_ch == -1) {
		// get socket channel
		m_client_ch = emu->get_socket_channel();
		if (m_client_ch < 0) {
			return false;
		}
		// connect
		if (!emu->init_socket_tcp(m_client_ch, this)) {
			m_client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return false;
		}
		if (!emu->connect_socket(m_client_ch, pConfig->comm_server_host[m_cfg_num], pConfig->comm_server_port[m_cfg_num])) {
			m_client_ch = -1;
			logging->out_log(LOG_ERROR, _T("Cannot connect to comm server."));
			return false;
		}
		m_connect = CONNECTING;
//		logging->out_log(LOG_DEBUG, _T("Connect comm client."));
	}
	return true;
#else
	return false;
#endif
}

/// @note called by main thread
void COMM::disconnect_socket()
{
#ifdef USE_SOCKET
	if (m_client_ch != -1) {
		// disconnect
		emu->disconnect_socket(m_client_ch);
//		logging->out_log(LOG_DEBUG, _T("Disconnect comm client."));
	}
#endif
}
/// callback
/// called by EMU::socket_connected()
void COMM::network_connected(int ch)
{
	if (ch == m_client_ch) {
		m_connect = CONNECTED;
//		pConfig->comm_connect[m_cfg_num] = true;
//		logging->out_logf(LOG_DEBUG, _T("Connected comm. ch:%d"),ch);
	}
}
/// callback
/// called by EMU::disconnect_socket()
void COMM::network_disconnected(int ch)
{
	if (ch == m_client_ch) {
		m_connect = DISCONNECT;
//		pConfig->comm_connect[m_cfg_num] = false;
//		logging->out_logf(LOG_DEBUG, _T("Disonnected comm. ch:%d"),ch);
		m_client_ch = -1;
	} else if (ch == m_server_ch) {
//		logging->out_logf(LOG_DEBUG, _T("Disonnected comm. ch:%d"),ch);
		m_server_ch = -1;
	}
	pConfig->comm_binary[m_cfg_num] = false;
}
void COMM::network_writeable(int ch)
{
	if (ch == m_client_ch) {
//		logging->out_logf(LOG_DEBUG, _T("Writeable comm. ch:%d"),ch);
		m_connect = CONNWRITEABLE;
	}
}
void COMM::network_readable(int ch)
{
}
void COMM::network_accepted(int ch, int new_ch)
{
#ifdef USE_SOCKET
	if (ch == m_server_ch) {
		// close serial port 
		disconnect_uart();

		if (m_client_ch != -1) {
			// Another client is already connected on my server.
			emu->disconnect_socket(new_ch);
		}
		m_client_ch = new_ch;
//		pConfig->comm_connect[m_cfg_num] = true;
	}
#endif
}

bool COMM::connect_uart(int ch)
{
#ifdef USE_UART
	if (!emu->open_uart(ch)) {
		logging->out_log(LOG_ERROR, _T("Cannot open serial port on host."));
		return false;
	}
	emu->init_uart(ch, this);
	m_uart_ch = ch;
	return true;
#else
	return false;
#endif
}

void COMM::disconnect_uart()
{
#ifdef USE_UART
	if (m_uart_ch != -1) {
		emu->close_uart(m_uart_ch);
		m_uart_ch = -1;
	}
#endif
}

void COMM::disconnect_all()
{
	disconnect_socket();
	disconnect_uart();
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void COMM::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	SET_Int32_LE(m_cfg_num);
	SET_Int32_LE(m_register_id);
	SET_Double_LE(m_send_speed);	// send speed
	SET_Bool(m_through);
	SET_Bool(m_rts);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool COMM::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	GET_Int32_LE(m_cfg_num);
	GET_Int32_LE(m_register_id);
	GET_Double_LE(m_send_speed);	// send speed
	GET_Bool(m_through);
	GET_Bool(m_rts);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t COMM::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	switch(addr & 1) {
	case 0:
		// Data
		data = m_recv_buff[m_recv_buff_r_pos];
		break;
	case 1:
		// Status
		data = ((m_send_buff_w_pos < COMM_MAX_BUFF ? 0x01 : 0x00)
			| (m_recv_buff_r_pos < m_recv_buff_w_pos ? 0x02 : 0x00));
		break;
	default:
		break;
	}
	return data;
}

bool COMM::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch(reg_num) {
	case 0:
		write_io8(0, data);
		return true;
	case 1:
		return true;
	}
	return false;
}

void COMM::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
//	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 0, _T("CR"), cr);
//	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%02X"), 1, _T("SR"), sr);
}
#endif