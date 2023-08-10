/** @file printer.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ printer ]
*/

#ifndef PRINTER_H
#define PRINTER_H

#include "../vm_defs.h"
#include "../device.h"

#undef USE_PRINTER_PENDSIZE
#define PRNDATA_BUFFER_SIZE	0x10000
#define PRNDATA_BUFFER_SIZE_MAX	(PRNDATA_BUFFER_SIZE * 16)

class EMU;
class FILEIO;
class FIFOBYTE;

/**
	@brief printer
*/
class PRINTER : public DEVICE
{
public:
	/// @brief signals on PRINTER
	enum SIG_PRINTER_IDS {
		SIG_PRINTER_CLOCK	= 1,
		SIG_PRINTER_DISCONNECT	= 11
	};

private:
	outputs_t outputs_busy;

	int m_cfg_num;

	FIFOBYTE *p_data_buffer;
	bool   m_buffer_overflow;

	FILEIO* h_fio;
	bool  m_send;
	bool m_strobe;
	uint8_t m_busdata;

	int   m_register_id;
	int   m_register_id_2;

	enum en_connect {
		DISCONNECT = 0,
		CONNECTABLE = 1,
		CONNWRITEABLE = 2,
		CONNECTED = 3,
	};
	int   m_connect;
	int   m_client_ch;
	enum en_send_type {
		SEND_TYPE_FILEONLY = 0,
		SEND_TYPE_INDIRECT,
		SEND_TYPE_DIRECT,
	};
	en_send_type m_send_type;
	int   m_send_size;
#ifdef USE_PRINTER_PENDSIZE
	int   m_pend_size;
#endif
	uint8_t *p_send_buff;
	int   m_send_retry;
	uint8_t m_recv_buff[4];

	//for resume
#pragma pack(1)
	struct vm_state_st {
		int m_register_id;
		int m_register_id_2;

		char reserved[8];
	};
#pragma pack()

	void  cancel_my_event(int &id);
	void  register_my_event(double wait, int &id);

	void  set_data(uint32_t data);
	void  save_image();
	bool  connect_socket();
	void  disconnect_socket();

public:
	PRINTER(VM* parent_vm, EMU* parent_emu, const char* identifier, int config_num) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("PRINTER");
		init_output_signals(&outputs_busy);
		m_cfg_num = config_num;
	}

	~PRINTER() {}

	// common functions
	void initialize();
	void reset();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique functions
	void set_context_busy(DEVICE* device, int id, uint32_t mask, uint32_t negative) {
		register_output_signal(&outputs_busy, device, id, mask, negative);
	}

	int  get_buffer_size() const;
	uint8_t* get_buffer() const;
	bool set_direct_mode();
	bool save_printer(const _TCHAR* filename);
	void close_printer();
	bool print_printer();
	void toggle_printer_online();
	void network_connected(int ch);
	void network_disconnected(int ch);
	void network_writeable(int ch);
	void network_readable(int ch);
	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
	uint8_t* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* PRINTER_H */
