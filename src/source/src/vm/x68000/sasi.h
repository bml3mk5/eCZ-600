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
#include "sxsi.h"
#include "../harddisk.h"
#include "../../cptrlist.h"
#include <vector>

class EMU;
class SASI_CTRLS;
class SASI_CTRL;
class SASI_UNITS;

/**
	@brief SASI (shugart associates system interface)
*/
/**
<pre>
  CPU <---> SASI <-+-> SASI_CTRL_RELAY (ID=0) <---> SASI_CTRL <-+
                   |         |                                  |
                   |   SASI_HARD_DISKs (UNIT=0,1) <-------------+
                   |
                   +-> SASI_CTRL_RELAY (ID=1) <---> SCSI_CTRL <-+
                   :         |                                  |
                       SCSI_HARD_DISK  (UNIT=0) <---------------+
</pre>
*/
class SASI : public SXSI_HOST
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
		SIGSTAT_ACK  = 0x0040,	///< Acknowledge (from host)
		SIGSTAT_SEL  = 0x0080,	///< select (from host)
		SIGSTAT_DRQ	 = 0x0100,	///< request from ctrl
		SIGSTAT_DRQ_MASK = 0xff00,
	};
	enum en_host_id {
		HOST_ID = 8
	};

private:
	uint32_t m_signal_status;	///< communication signals and status

	// for save config
#pragma pack(1)
	struct vm_state_st {
		SXSI_HOST::vm_state_st m_sxsi_host;

		int m_selected_ctrl_id;
		uint32_t m_signal_status;
		int reserved[2];
	};
#pragma pack()

	//
//	void load_wav();
	void select_control(uint32_t data);
	void set_irq(bool onoff);
	void set_drq(bool onoff);

public:
	SASI(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~SASI();

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t get_bus_signal() const;

	void write_signal(int id, uint32_t data, uint32_t mask);

//	void initialize_sound(int rate, int decibel);
//	void mix(int32_t* buffer, int cnt);
//	void set_volume(int decibel, bool vol_mute);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
//	void update_signal_status(uint32_t on, uint32_t off);
	static const uint8_t bus_status_2_sig_map[BUSTYPE_MAX];
//	void set_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3);
//	void clr_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3);
	void update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff);
	void accept_req(uint32_t flags);
	void go_idle();

	// user interface
	bool open_disk(int drv, const _TCHAR *path, uint32_t flags);
	bool close_disk(int drv, uint32_t flags);
	bool disk_mounted(int drv);
	void toggle_disk_write_protect(int drv);
	bool disk_write_protected(int drv) const;
	bool is_same_disk(int drv, const _TCHAR *path);
	int  mounted_disk_another_drive(int drv, const _TCHAR *path);

	void change_device_type(int drv, int num);

	uint32_t get_led_status() const;

//	//
//	void play_seek_sound();

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
class SASI_CTRL : public SXSI_CTRL
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
	static const _TCHAR *c_phase_labels[];

	/// @brief events on SASI_CTRL
	enum en_event_ids {
		EVENT_REQ = 0,
		EVENT_STATUS,
		EVENT_SEEK,
		EVENT_IDS_MAX,
		EVENT_IDS_RESERVED_MAX = 6
	};
	enum en_event_args {
//		EVENT_ARG_SIGNAL_ON = 0,
//		EVENT_ARG_SIGNAL_OFF = 1,
		EVENT_ARG_STATUS_TYPE = 0,
		EVENT_ARG_STATUS_CODE = 1,
		EVENT_ARG_SEEK_TIME = 2,
		EVENT_ARG_NEXT_PHASE = 3,
		EVENT_ARGS_MAX = 4,
		EVENT_ARGS_RESERVED_MAX = 4
	};
	enum en_buffer_args {
		BUFFER_MAX = 2048
	};

private:
	// event
	int m_register_id[EVENT_IDS_MAX];
	uint32_t m_event_arg[EVENT_ARGS_MAX];

	enum en_command_codes {
		CMD_NONE = 0,
		CMD_TEST_DRIVE_READY,
		CMD_RECALIBRATE,
		CMD_REQUEST_SENSE,
		CMD_FORMAT_DRIVE,
		CMD_FORMAT_TRACK,
		CMD_FORMAT_BAD_TRACK,
		CMD_READ,
		CMD_WRITE,
		CMD_SEEK,
		CMD_CONFIG_DRIVE,	///< unknown command 0xc2
		CMD_DIAGNOSTIC,		///< unknown command 0xe0
		CMD_UNKNOWN,
		CMD_UNSUPPORTED = 0xff,
	};
	static const uint8_t c_command_table[32];
	static const _TCHAR *c_command_labels[CMD_UNKNOWN+1];

	int m_send_data_pos;
	int m_send_data_len;
	uint8_t m_send_data[BUFFER_MAX];

	int m_recv_data_pos;
	int m_recv_data_len;
	uint8_t m_recv_data[BUFFER_MAX];

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
		SXSI_CTRL::vm_state_st m_sxsi_ctrl;

		uint8_t m_send_message;
		uint8_t m_send_error;
		char	reserved1[2];
		int		reserved2[1];

		uint32_t m_event_arg[EVENT_ARGS_RESERVED_MAX];
		int m_register_id[EVENT_IDS_RESERVED_MAX];

		int reserved3[2];
		int m_send_data_pos;
		int m_send_data_len;
		uint8_t m_send_data[BUFFER_MAX];

		int reserved4[2];
		int m_recv_data_pos;
		int m_recv_data_len;
		uint8_t m_recv_data[BUFFER_MAX];
	};
#pragma pack()

	inline void write_control(uint32_t data);
	inline void parse_command_code(uint8_t data);
	inline void parse_command_param_group_0();
	inline void parse_command_param_group_1();
	inline void parse_command_param_group_2();
	inline void process_command();

	inline void test_drive_ready();
	inline void recalibrate();
	inline void send_sense();
	inline void init_send_buffer(HARDDISK *disk);
	inline void send_disk_data(HARDDISK *disk);
	inline void send_first_disk_data();
	inline void send_next_disk_data();
	inline void init_recv_buffer(HARDDISK *disk);
	inline void recv_disk_data(HARDDISK *disk);
	inline void recv_first_disk_data();
	inline void recv_next_disk_data();
	inline void seek();
	inline void format_disk();
	inline void format_track();
	inline void format_bad_track();
	inline void recv_config_drive();
	inline void finish_config_drive();
	inline void diagnostic();
	inline void send_status(uint8_t error_occured, uint8_t error_code, bool from_event = false);
	inline void send_status_delay(int delay, uint8_t error_occured, uint8_t error_code);
	inline void send_message();
	inline void go_idle();

	inline void play_seek_sound(int sum);
	inline void register_seek_sound(int sum, int delay);

//	inline void update_signal(uint32_t on, uint32_t off);
//	inline void update_bus_status(SXSI_HOST::SXSI_SIGNAL_STATUS_TYPE bus_type, bool onoff);
//	inline void update_signal_delay(int delay, uint32_t on, uint32_t off);
//	inline void clr_drq();
//	inline void set_drq();
	inline void set_request_delay(int delay, int next_phase);
	inline void send_request();

	inline void cancel_my_event(int event_id);
	inline void register_my_event(int event_id, int usec);

//	inline HARDDISK *get_disk_unit(int unit_num) const;
//	inline bool unit_mounted(int unit_num) const;

public:
	SASI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id);
	~SASI_CTRL();

	void clear();
	void initialize();
	void reset();
	void warm_reset(bool por);
	void event_callback(int event_id, int err);
	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique function
//	bool select();
	void go_command_phase();
	void accept_ack();
	void request_next();

	void write_data(uint32_t data);
	uint32_t read_data();
//	bool unit_mounted_at_least() const;
	int get_ctrl_id() const { return m_ctrl_id; }

	// user interface
//	bool open_disk(int unit_num, const _TCHAR *path, uint32_t flags);
//	bool close_disk(int unit_num, uint32_t flags);
//	bool disk_mounted(int unit_num) const;
//	void set_write_protect(int unit_num, bool val);
//	bool write_protected(int unit_num) const;
//	bool is_same_disk(int unit_num, const _TCHAR *path);

#ifdef USE_DEBUGGER
	void debug_write_data(uint32_t data);
	uint32_t debug_read_data();
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

/**
	@brief SASI (shugart associates system interface) relay class
*/
class SASI_CTRL_RELAY : public SXSI_CTRL_RELAY
{
public:
	SASI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id);

	// unique functions
	void change_device_type(int num);
	bool unit_mounted_at_least() const;
};

#if 0
/**
	@brief SASI (shugart associates system interface) controll list
*/
class SASI_CTRLS : public std::vector<SASI_CTRL *>
{
public:
	SASI_CTRLS();
};

/**
	@brief SASI (shugart associates system interface) unit list
*/
class SASI_UNITS : public CPtrList<HARDDISK>
{
public:
	SASI_UNITS(int alloc_size);
};
#endif

#endif /* SASI_H */

