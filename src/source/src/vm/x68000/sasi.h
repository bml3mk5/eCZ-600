/** @file sasi.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.03 -

	@brief [ shugart associates system interface ]
*/

#ifndef SASI_H
#define SASI_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"
#include "../sound_base.h"
#include "../noise.h"

class EMU;
class SASI_CTRL;
class HARDDISK;

/**
	@brief SASI (shugart associates system interface)
*/
class SASI : public SOUND_BASE
{
public:
	/// @brief communication signals with SASI controller
	enum SASI_SIGNAL_STATUS {
		SIGSTAT_REQ = 0x01,	///< Request (from control)
		SIGSTAT_BSY = 0x02,	///< Busy (from control)
		SIGSTAT_IO  = 0x04,	///< I/O (from control)
		SIGSTAT_CD  = 0x08,	///< Control/Data (from control)
		SIGSTAT_MSG = 0x10,	///< Message (from control)
		SIGSTAT_MASK = 0x1f,
		// for my emu
		SIGSTAT_SEL  = 0x0080,	///< select (from host)
		SIGSTAT_DRQ0 = 0x0100,	///< request from ctrl0
		SIGSTAT_DRQ1 = 0x0200,	///< request from ctrl1
		SIGSTAT_DRQ2 = 0x0400,	///< request from ctrl2
		SIGSTAT_DRQ3 = 0x0800,	///< request from ctrl3
		SIGSTAT_DRQ4 = 0x1000,	///< request from ctrl4
		SIGSTAT_DRQ5 = 0x2000,	///< request from ctrl5
		SIGSTAT_DRQ6 = 0x4000,	///< request from ctrl6
		SIGSTAT_DRQ7 = 0x8000,	///< request from ctrl7
		SIGSTAT_DRQ_MASK = 0xff00,
	};

private:
	enum en_constants {
		CONTROL_NUMS = 1,
	};

private:
	DEVICE *d_memory;
	SASI_CTRL *d_ctrl[CONTROL_NUMS];
	SASI_CTRL *d_selected_ctrl;

	uint32_t m_signal_status;	///< communication signals and status

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;

	enum WAV_SNDTYPES {
		SASI_WAV_SEEK		= 0,
		SASI_WAV_SNDTYPES
	};
	NOISE m_noises[SASI_WAV_SNDTYPES];
	bool m_wav_loaded_at_first;

	// for save config
#pragma pack(1)
	struct vm_state_st {
		int m_selected_ctrl_id;
		uint32_t m_signal_status;
		char reserved[8];
		NOISE::vm_state_st m_noises[SASI_WAV_SNDTYPES];
	};
#pragma pack()

	//
	void load_wav();
	void select_control(uint32_t data);

public:
	SASI(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~SASI();

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);
//	void cancel_my_event(int &id);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);

	void initialize_sound(int rate, int decibel);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel, bool vol_mute);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
	void set_context_memory(DEVICE *device) {
		d_memory = device;
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void update_signal_status(uint32_t on, uint32_t off);
	void set_irq(bool onoff);
	void clr_drq(int ctrl_num);
	void set_drq(int ctrl_num);

	// user interface
	bool open_disk(int drv, const _TCHAR *path, uint32_t flags);
	bool close_disk(int drv, uint32_t flags);
	bool disk_mounted(int drv);
	bool is_same_disk(int drv, const _TCHAR *path);

	uint32_t get_led_status() const;

	//
	void play_seek_sound();

#ifdef USE_DEBUGGER
	void debug_write_io8(uint32_t addr, uint32_t data);
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

/**
	@brief SASI (shugart associates system interface) controller
*/
class SASI_CTRL : public DEVICE
{
public:
	/// @brief phase in SASI
	enum en_phases {
		PHASE_BUSFREE = 0,
		PHASE_SELECTION,
		PHASE_COMMAND,
		PHASE_TRANSFER,
		PHASE_STATUS,
		PHASE_MESSAGE,
		PHASE_UNKNOWN
	};

private:
	/// @brief events on SASI_CTRL
	enum en_event_ids {
		EVENT_SIGNAL = 0,
		EVENT_DRQ,
		EVENT_STATUS,
		EVENT_SEEK,
		EVENT_IDS_MAX
	};
	enum en_event_args {
		EVENT_ARG_SIGNAL_ON = 0,
		EVENT_ARG_SIGNAL_OFF = 1,
		EVENT_ARG_STATUS_TYPE = 0,
		EVENT_ARG_STATUS_CODE = 1,
		EVENT_ARG_SEEK_TIME = 2,
		EVENT_ARGS_MAX = 3,
	};

private:
	SASI *d_host;
	HARDDISK *d_disk;
	int m_ctrl_id;

	// event
	int m_register_id[EVENT_IDS_MAX];
	uint32_t m_event_arg[EVENT_ARGS_MAX];

	int m_phase;

	int m_command_pos;

	enum en_command_codes {
		CMD_NONE = 0,
		CMD_TEST_DRIVE_READY,
		CMD_RECALIBRATE,
		CMD_REQUEST_SENSE,
		CMD_FORMAT_DRIVE,
		CMD_FORMAT_TRACK,
		CMD_READ,
		CMD_WRITE,
		CMD_SEEK,
		CMD_EXTEND_ARGS,	///< unknown command 0xc2
		CMD_DIAGNOSTIC,		///< unknown command 0xe0
		CMD_UNKNOWN,
		CMD_UNSUPPORTED = 0xff,
	};
	uint8_t m_command_class;
	uint8_t m_command_code;
	static const uint8_t c_command_table[32];
	uint8_t m_command_len;

	uint8_t m_unit_number;	///< b5-b7
	int m_curr_block;
	int m_num_blocks;

	int m_send_data_pos;
	int m_send_data_len;
	uint8_t m_send_data[256];

	int m_recv_data_pos;
	int m_recv_data_len;
	uint8_t m_recv_data[256];

	enum en_status {
		STA_COMPLETE = 0x00,
		STA_PARITY_ERROR = 0x01,
		STA_ERROR = 0x02,
	};
	enum en_messages {
		MSG_COMPLETE = 0x00,
//		MSG_EXTEND_MESSAGE_FOLLOWS = 0x01,
//		MSG_SAVE_STATE = 0x02,
//		MSG_RESTORE_STATE = 0x03,
//		MSG_DISCONNECT = 0x04,
//		MSG_RETRY = 0x05,
//		MSG_ABORT = 0x06,
//		MSG_REJECT = 0x07,
//		MSG_NO_OPERATOIN = 0x08,
//		MSG_PARITY_ERROR = 0x09,
//		MSG_LINKED_COMMAND_COMPLETE = 0x0a,
//		MSG_UNKNOWN = 0x7f,
	};
	uint8_t m_send_message;
	enum en_error_class0 {
		ERR_NO_STATUS = 0x00,
		ERR_NO_INDEX_SIGNAL = 0x01,
		ERR_NO_SEEK_COMPLETE = 0x02,
		ERR_WRITE_FAULT = 0x03,
		ERR_DRIVE_NOT_READY = 0x04,
		ERR_DRIVE_NOT_SELECTED = 0x05,
		ERR_NO_TRACK0 = 0x06,
		ERR_MULTIPLE_DRIVES_SELECTED = 0x07,
		ERR_DRIVE_WRITE_PROTECTED = 0x08,
		ERR_MEDIA_NOT_LOADED = 0x09,
		ERR_INSUFFICIENT_CAPACITY = 0x0a,
		ERR_UNKNOWN = 0x0f,
	};
	uint8_t m_send_error;

	/// request signal delay (usec)
	enum en_constants {
//		DELAY_COMMAND_PHASE = 100,
		DELAY_TRANSFER_PHASE = 50,
		DELAY_TRANSFER_DATA = 2,
	};

	// for save config
#pragma pack(1)
	struct vm_state_st {
		int m_phase;
		int m_command_pos;
		int m_curr_block;
		int m_num_blocks;

		uint8_t m_command_class;
		uint8_t m_command_code;
		uint8_t m_command_len;
		uint8_t m_unit_number;	///< b5-b7
		uint8_t m_send_message;
		uint8_t m_send_error;
		char reserved0[2];
		uint32_t m_event_arg[EVENT_ARGS_MAX];

		int m_register_id[EVENT_IDS_MAX];

		char reserved1[4];
		int m_send_data_pos;
		int m_send_data_len;
		uint8_t m_send_data[256];

		char reserved3[8];
		int m_recv_data_pos;
		int m_recv_data_len;
		uint8_t m_recv_data[256];
	};
#pragma pack()

	void clear();
	inline void write_control(uint32_t data);
	inline void parse_command_code(uint8_t data); 
	inline void process_command();

	inline void test_drive_ready();
	inline void recalibrate();
	inline void send_sense();
	inline void send_disk_data(int cylinder_sum);
	inline void send_first_disk_data();
	inline void send_next_disk_data();
	inline void recv_disk_data(int cylinder_sum);
	inline void recv_first_disk_data();
	inline void recv_next_disk_data();
	inline void seek();
	inline void format_disk();
	inline void format_track();
	inline void recv_extended_args();
	inline void finish_extended_args();
	inline void diagnostic();
	inline void send_status(uint8_t error_occured, uint8_t error_code);
	inline void send_status_delay(int delay, uint8_t error_occured, uint8_t error_code);
	inline void send_message();
	inline void go_idle();

	inline void play_seek_sound(int sum);
	inline void register_seek_sound(int sum, int delay);

	inline void update_signal(uint32_t on, uint32_t off);
	inline void update_signal_delay(int delay, uint32_t on, uint32_t off);
	inline void clr_drq();
	inline void set_drq_delay(int delay);

	inline void cancel_my_event(int event_id);
	inline void register_my_event(int event_id, int wait);

public:
	SASI_CTRL(VM* parent_vm, EMU* parent_emu, SASI *host, int ctrl_id);
	~SASI_CTRL();

	void initialize();
	void reset();
	void warm_reset(bool por);
	void event_callback(int event_id, int err);
	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique function
	bool select();
	void selected(uint32_t data);
	void write_data(uint32_t data);
	uint32_t read_data();
	int get_ctrl_id() const { return m_ctrl_id; }

	// user interface
	bool open_disk(const _TCHAR *path, uint32_t flags);
	bool close_disk(uint32_t flags);
	bool disk_mounted();
	bool is_same_disk(const _TCHAR *path);

#ifdef USE_DEBUGGER
	void debug_write_data(uint32_t data);
	uint32_t debug_read_data();
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* SASI_H */

