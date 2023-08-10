/** @file comm.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ comm ]
*/

#ifndef COMM_H
#define COMM_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "acia.h"

#define COMM_MAX_BUFF 320

class EMU;

/**
	@brief Communication (RS-232C)
*/
class COMM : public DEVICE
{
public:
	/// @brief signals on COMM
//	enum SIG_COMM_IDS {
//		SIG_COMM_RS	= 100,
//	};

private:
	enum EVENT_IDS {
		EVENT_TXD = 0
	};

	DEVICE *d_ctrl;

	int m_cfg_num;

	bool m_through;	// send/receive 8bit on through mode 

	bool m_received;
	uint8_t m_send_buff[COMM_MAX_BUFF];
	int m_send_buff_w_pos;
	int m_send_buff_r_pos;
	uint8_t m_recv_buff[COMM_MAX_BUFF];
	int m_recv_buff_w_pos;
	int m_recv_buff_r_pos;

	bool m_rts;
	double m_send_speed;	// send speed

	int m_register_id;

	enum en_connect {
		DISCONNECT = 0,
		CONNECTING,
		CONNECTED,
		CONNWRITEABLE,
	};
	int m_connect;
	int m_client_ch;
	int m_server_ch;
	int m_uart_ch;

	int m_send_telcmd_pos;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int m_cfg_num;
		int m_register_id;
		uint64_t m_send_speed;	// send speed
		uint8_t m_through;
		uint8_t m_rts;
		char reserved[14];
	};
#pragma pack()

	void register_my_event();
	void cancel_my_event();

	bool connect_socket();
	void disconnect_socket();

	bool connect_uart(int ch);
	void disconnect_uart();

	void disconnect_all();

public:
	COMM(VM* parent_vm, EMU* parent_emu, const char* identifier, int config_num) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("COMM");
		d_ctrl = NULL;
		m_cfg_num = config_num;
	}
	~COMM() {}

	// common functions
	void initialize();
	void reset();
	void warm_reset(bool por);
	void release();

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_ctrl(DEVICE* device) {
		d_ctrl = device;
	}

	void send_data(uint32_t data);
	uint32_t recv_data();

	void event_frame();
	void event_callback(int , int);

	void enable_server();
	void enable_connect(int num);
	bool now_connecting(int num);
	void send_telnet_command(int num);
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

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* COMM_H */
