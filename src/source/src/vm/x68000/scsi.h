/** @file scsi.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.23 -

	@brief [ Small Computer System Interface ]
*/

#ifndef SCSI_H
#define SCSI_H

#include "../vm_defs.h"
#include "../../emu.h"
#include "../sound_base.h"
#include "../noise.h"
#include "sxsi.h"
#include "../harddisk.h"
#include "../../cptrlist.h"
#include "../../fifo.h"
#include <vector>

class EMU;
class SCSI_CTRLS;
class SCSI_CTRL;
class SCSI_UNITS;

/**
	@brief MB89352 SCSI Protocol Controller modoki
*/
/**
<pre>
  CPU <---> SCSI <-+-> SCSI_CTRL_RELAY (ID=0) <---> SCSI_CTRL <-+
                   |         |                                  |
                   |   SCSI_HARD_DISK  (UNIT=0) <---------------+
                   |
                   +-> SCSI_CTRL_RELAY (ID=1) <---> SCSI_CTRL <-+
                   :         |                                  |
                       SCSI_MO etc.    (UNIT=0) <---------------+
</pre>
*/
class SCSI : public SXSI_HOST
{
public:
	enum SCSI_SIGNALS {
//		SIG_IACK = 300,
#ifdef USE_SCSI_SIG_OUTPUTS
		SIG_SCSI_DAT = 301,
		SIG_SCSI_BSY = 302,
		SIG_SCSI_CD	 = 303,
		SIG_SCSI_IO	 = 304,
		SIG_SCSI_MSG = 305,
		SIG_SCSI_REQ = 306,
		SIG_SCSI_SEL = 307,
		SIG_SCSI_ATN = 308,
		SIG_SCSI_ACK = 309,
		SIG_SCSI_RST = 310
#endif
	};
	/// @brief communication signals with SCSI controller
	/// (PSNS Register)
	enum SCSI_BUS_STATUS {
		BUSSTS_IO  = 0x01,	///< I/O (from control)
		BUSSTS_CD  = 0x02,	///< Control/Data (from control)
		BUSSTS_MSG = 0x04,	///< Message (from control)
		BUSSTS_BSY = 0x08,	///< Busy (from control)
		BUSSTS_SEL = 0x10,	///< Select (to control)
		BUSSTS_ATN = 0x20,	///< ATN (to control)
		BUSSTS_ACK = 0x40,	///< ACK (to control)
		BUSSTS_REQ = 0x80,	///< Request (from control)
	};

private:
	enum en_reg_names {
		SCSI_REG_BDID = 0,
		SCSI_REG_SCTL,
		SCSI_REG_SCMD,
		SCSI_REG_INTS,
		SCSI_REG_PSNS,
		SCSI_REG_SDGC,
		SCSI_REG_SSTS,
		SCSI_REG_SERR,
		SCSI_REG_PCTL,
		SCSI_REG_MBC,
		SCSI_REG_TEMP_RECV,
		SCSI_REG_TEMP_SEND,
		SCSI_REGS_END,
		SCSI_REGS_RESERVED_MAX = 16
	};
	enum en_sctl_flags {
		SCTL_RESET_AND_DISABLE = 0x80,
		SCTL_CONTROL_RESET = 0x40,
		SCTL_RESET_ALL = 0xc0,
		SCTL_DIAG_MODE = 0x20,
		SCTL_SPC_DISABLE = 0xe0,
		SCTL_ARBITRATION_ENABLE = 0x10,
		SCTL_PARITY_ENABLE = 0x08,
		SCTL_SELECT_ENABLE = 0x04,
		SCTL_RESELECT_ENABLE = 0x02,
		SCTL_INTERRUPT_ENABLE = 0x01,
	};
	enum en_scmd_flags {
		SCMD_COMMAND_MODE = 0xe0,
		SCMD_COMMAND_SFT = 5,
		SCMD_COMMAND_BUSRELEASE = 0x00,
		SCMD_COMMAND_SELECT = 0x20,
		SCMD_COMMAND_RESET_ATN = 0x40,
		SCMD_COMMAND_SET_ATN = 0x60,
		SCMD_COMMAND_TRANSFER = 0x80,
		SCMD_COMMAND_TRANSFER_PAUSE = 0xa0,
		SCMD_COMMAND_RESET_ACK_REQ = 0xc0,
		SCMD_COMMAND_SET_ACK_REQ = 0xe0,
		SCMD_RST_OUT = 0x10,
		SCMD_INTERCEPT_TRANSFER = 0x08,
		SCMD_TRANSFER_MODIFIRE = 0x07,
		SCMD_PROGRAM_TRANSFER = 0x04,
		SCMD_TERMINATION_MODE = 0x01,
	};
	enum en_ints_flags {
		INTS_SELECTED = 0x80,
		INTS_RESELECTED = 0x40,
		INTS_DISCONNECTED = 0x20,
		INTS_COMMAND_COMPLETE = 0x10,
		INTS_SERVICE_REQUIRED = 0x08,
		INTS_TIMEOUT = 0x04,
		INTS_SPC_HARDERROR = 0x02,
		INTS_RESET_CONDITION = 0x01,
	};
	enum en_sdgc_flags {
		SDGC_DIAG_IO = 0x01,
		SDGC_DIAG_CD = 0x02,
		SDGC_DIAG_MSG = 0x04,
		SDGC_DIAG_BSY = 0x08,
		SDGC_XFER_ENABLE = 0x20,
		SDGC_DIAG_ACK = 0x40,
		SDGC_DIAG_REQ = 0x80,
	};
	enum en_ssts_flags {
		SSTS_DREG_STATUS = 0x03,
		SSTS_DREG_EMPTY = 0x01,
		SSTS_DREG_FULL = 0x02,
		SSTS_TC_IS_ZERO = 0x04,
		SSTS_SCSI_RESET_IN = 0x08,
		SSTS_TRANSFER_IN_PROGRESS = 0x10,
		SSTS_SPC_BUSY = 0x20,
		SSTS_CONNECTED = 0xc0,
		SSTS_CONNECTED_TARGET = 0x40,
		SSTS_CONNECTED_INITIATOR = 0x80,
	};
	enum en_serr_flags {
		SERR_SHORT_TRANSFER_PERIOD = 0x02,
		SERR_TC_PARITY_ERROR = 0x08,
		SERR_XFER_OUT = 0x20,
		SERR_DATA_ERROR = 0xc0,
		SERR_DATA_OUT_ERROR = 0x40,
		SERR_DATA_IN_ERROR = 0x80,
		SERR_MASK = 0xea,
	};
	enum en_pctl_flags {
		PCTL_PHASE_MASK = 0x07,
		PCTL_DATAOUT_PHASE = 0x00,
		PCTL_DATAIN_PHASE = 0x01,
		PCTL_COMMAND_PHASE = 0x02,
		PCTL_STATUS_PHASE = 0x03,
		PCTL_MESSAGEOUT_PHASE = 0x06,
		PCTL_MESSAGEIN_PHASE = 0x07,
		PCTL_BUSFREE_INT_ENABLE = 0x80,
		PCTL_MASK = 0x87,
	};
	enum en_event_ids {
		EVENT_DRQ = 0,
		EVENT_NEXT_PHASE,
		EVENT_SELECTED,
		EVENT_STATUS,
		EVENT_IDS_MAX,
		EVENT_IDS_RESERVED_MAX = 8
	};
	enum en_event_args {
		EVENT_ARG_NEXT_PHASE = 0,
		EVENT_ARG_STATUS_TYPE = 0,
		EVENT_ARG_STATUS_CODE = 1,
		EVENT_ARGS_MAX = 2,
		EVENT_ARGS_RESERVED_MAX = 4
	};

private:
//	DEVICE *d_memory;
//	SCSI_CTRLS *d_ctrls;
//	SCSI_CTRL *d_selected_ctrl;

	uint8_t m_bdid;	// ID 0 - 7

	uint8_t m_regs[SCSI_REGS_END];

	uint32_t m_trans;		///< TC

	uint8_t m_pre_ints;
	bool    m_pre_atn;
	bool    m_pre_drq;
	uint8_t m_retry_selection;
//	bool	m_now_iack;

	FIFOBYTE m_buffer;

	int m_register_id[EVENT_IDS_MAX];
	uint32_t m_event_arg[EVENT_ARGS_MAX];

//	// output signals
//	outputs_t outputs_irq;	// to adaptor
//	outputs_t outputs_drq;

#ifdef USE_SCSI_SIG_OUTPUTS
	outputs_t outputs_bsy;
	outputs_t outputs_cd;
	outputs_t outputs_io;
	outputs_t outputs_msg;
	outputs_t outputs_req;
	
	outputs_t outputs_dat;	// to devices
	outputs_t outputs_sel;
	outputs_t outputs_atn;
	outputs_t outputs_ack;
	outputs_t outputs_rst;
#endif

//	enum WAV_SNDTYPES {
//		SCSI_WAV_SEEK		= 0,
//		SCSI_WAV_SNDTYPES
//	};
//	NOISE m_noises[SCSI_WAV_SNDTYPES];
//	bool m_wav_loaded_at_first;

	// for save config
#pragma pack(1)
	struct vm_state_st {
		SXSI_HOST::vm_state_st m_sxsi_host;

		uint8_t m_regs[SCSI_REGS_RESERVED_MAX];

		int m_selected_ctrl_id;
		uint8_t m_bdid;	// ID 0 - 7
		uint8_t m_pre_ints;
		uint8_t m_flags;
		uint8_t m_retry_selection;
		uint32_t m_trans;		///< TC
		uint32_t reserved;

		uint8_t m_buffer_count;
		uint8_t m_buffer[8];
		char reserved0[7];

		int m_register_id[EVENT_IDS_RESERVED_MAX];
		uint32_t m_event_arg[EVENT_ARGS_RESERVED_MAX];
	};
#pragma pack()

	void cancel_my_event(int event_id);
	void cancel_my_events();
	void register_my_event(int event_id, double usec);
	void register_my_event_by_clock(int event_id, int clock);

	inline void process_spc_control(uint32_t data);
	inline void reset_interrupt_status(uint32_t data);
	inline void write_pctl_register(uint32_t data);
	inline void write_data_register(uint32_t data);
	inline void write_temp_register(uint32_t data);
	inline uint32_t read_temp_register();
	inline uint32_t read_data_register();

	inline void process_spc_command(uint32_t data);
	inline void process_spc_command_select();
	inline void process_spc_command_atn(bool is_set);
	inline void process_spc_command_transfer();
	inline void process_spc_command_ack_req(bool is_set);

	inline void arbitration();
	inline void selection();
	inline void selected_device();

//	inline void assert_ack();
//	inline void negate_ack();

	inline void data_in_and_update_drq();
//	inline void data_out_and_update_drq();

//	inline void transfer_data();
//	inline void receive_data();

	void update_irq();
	void set_irq(bool onoff);
//	void update_drq();
	void set_drq(bool onoff);
	inline void set_drq_delay(bool onoff);

	void update_ssts(uint32_t on, uint32_t off);

	//
//	void load_wav();
	
public:
	SCSI(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~SCSI();

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_external_data8(uint32_t addr);
	uint32_t read_io8(uint32_t addr);
	uint32_t get_bus_signal() const;

#ifdef USE_SCSI_SIG_OUTPUTS
#ifdef SCSI_HOST_WIDE
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io16(uint32_t addr);
#else
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
#endif
	uint32_t read_signal(int id);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);

	void event_callback(int event_id, int err);

//	void initialize_sound(int rate, int decibel);
//	void mix(int32_t* buffer, int cnt);
//	void set_volume(int decibel, bool vol_mute);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
#ifdef USE_SCSI_SIG_OUTPUTS
	void set_context_bsy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_bsy, device, id, mask);
	}
	void set_context_cd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_cd, device, id, mask);
	}
	void set_context_io(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_io, device, id, mask);
	}
	void set_context_msg(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_msg, device, id, mask);
	}
	void set_context_req(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_req, device, id, mask);
	}
	void set_context_ack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ack, device, id, mask);
	}
	void set_context_target(DEVICE* device)
	{
#ifdef SCSI_HOST_WIDE
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xffff);
#else
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xff);
#endif
		register_output_signal(&outputs_sel, device, SIG_SCSI_SEL, 1);
		register_output_signal(&outputs_atn, device, SIG_SCSI_ATN, 1);
		register_output_signal(&outputs_ack, device, SIG_SCSI_ACK, 1);
		register_output_signal(&outputs_rst, device, SIG_SCSI_RST, 1);
	}
#endif

	static const uint8_t bus_status_2_sig_map[BUSTYPE_MAX];
	void update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff);

	void reselection(int id);
	void accept_req(uint32_t flags);
	void command_complete();
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
	@brief SCSI controller
*/
class SCSI_CTRL : public SXSI_CTRL
{
public:
	/// @brief phase in SCSI
	enum en_phases {
		PHASE_BUSFREE = 0,
		PHASE_ARBITRATION,
		PHASE_SELECTION,
		PHASE_COMMAND,
		PHASE_DATA_IN,
		PHASE_DATA_OUT,
		PHASE_STATUS,
		PHASE_MESSAGE_IN,
		PHASE_MESSAGE_OUT,
		PHASE_COMMAND_NEXT,	// next phase
		PHASE_UNKNOWN
	};

private:
	static const _TCHAR *c_phase_labels[];

	/// @brief events on SCSI_CTRL
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

	int m_next_phase;

	enum en_command_codes {
		CMD_NONE = 0,
		CMD_TEST_UNIT_READY,
		CMD_REZERO_UNIT,
		CMD_REQUEST_SENSE,
		CMD_FORMAT_UNIT,
		CMD_READ_CAPACITY,
		CMD_READ,
		CMD_WRITE,
		CMD_SEEK,
		CMD_INQUIRY,
		CMD_MODE_SENSE,
		CMD_MODE_SELECT,
		CMD_START_STOP_UNIT,
		CMD_MEDIA_REMOVAL,
		CMD_VERIFY,
		CMD_UNKNOWN,
		CMD_UNSUPPORTED = 0xff,
	};
	static const uint8_t c_command_table[64];
	static const _TCHAR *c_command_labels[CMD_UNKNOWN+1];

	uint8_t m_is_relative;

	int m_send_data_pos;
	int m_send_data_len;
	uint8_t m_send_data[BUFFER_MAX];

	int m_recv_data_pos;
	int m_recv_data_len;
	uint8_t m_recv_data[BUFFER_MAX];

	enum en_identify {
		IDT_ENABLE = 0x80,
		IDT_DISCONNECT = 0x40,
		IDT_LUN = 0x07,
	};
	uint8_t m_identify;

	enum en_status {
		STA_GOOD = 0x00,
		STA_CHECK_CONDITION = 0x1,
		STA_CONDITION_MET_GOOD = 0x2,
		STA_BUSY = 0x4,
		STA_INTERMEDIATE = 0x8,
		STA_RESERVATION_CONFLICT = 0xc,
		STA_UNKNOWN = 0xf,
	};

	enum en_messages {
		MSG_COMPLETE = 0x00,
	};
	uint8_t m_send_message;

	enum en_sense_keys {
		SENSE_NO_SENSE = 0x0,
		SENSE_RECOVERED_ERROR = 0x1,
		SENSE_NOT_READY = 0x2,
		SENSE_MEDIUM_ERROR = 0x3,
		SENSE_HARDWARE_ERROR = 0x4,
		SENSE_ILLEGAL_REQUEST = 0x5,
		SENSE_UNIT_ATTENTION = 0x6,
		SENSE_DATA_PROTECT = 0x7,
		SENSE_BLANK_CHECK = 0x8,
		SENSE_VENDOR_UNIQUE = 0x9,
		SENSE_COPY_ABORTED = 0xa,
		SENSE_ABORTED_COMMAND = 0xb,
		SENSE_EQUAL = 0xc,
		SENSE_VOLUME_OVERFLOW = 0xd,
		SENSE_MISCOMPARE = 0xe,
		SENSE_RESERVED = 0xf,
	};
	uint8_t m_sense_code;

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
		uint8_t m_sense_code;
		uint8_t m_is_relative;
		uint8_t m_identify;
		int		m_next_phase;
		int		reserved2[2];

		uint32_t m_event_arg[EVENT_ARGS_RESERVED_MAX];
		int m_register_id[EVENT_IDS_RESERVED_MAX];

		int m_send_data_pos;
		int m_send_data_len;
		uint8_t m_send_data[BUFFER_MAX];

		int reserved3[2];
		int m_recv_data_pos;
		int m_recv_data_len;
		uint8_t m_recv_data[BUFFER_MAX];
	};
#pragma pack()

	inline void write_control(uint32_t data);
	inline void parse_command_code(uint8_t data);
	inline void parse_command_param_group_0();
	inline void parse_command_param_group_1();
	inline void parse_command_param_group_5();
	inline void process_command();

	inline void test_unit_ready();
	inline void rezero_unit();
	inline void request_sense();
	inline void init_send_buffer(HARDDISK *disk);
	inline void send_disk_data(HARDDISK *disk);
	inline void send_first_disk_data();
	inline void send_next_disk_data();
	inline void init_recv_buffer(HARDDISK *disk);
	inline void recv_disk_data(HARDDISK *disk);
	inline void recv_first_disk_data();
	inline void recv_next_disk_data();
	inline void verify_disk_data(HARDDISK *disk);
	inline void verify_first_disk_data();
	inline void verify_next_disk_data();
	inline void seek();
	inline void format_unit();
	inline void read_capacity();
	inline void set_identify(uint32_t data);
	inline void inquiry();
	inline void mode_sense();
	inline void mode_select();
	inline void start_stop_unit();
	inline void media_removal();
	inline void send_status(uint8_t error_occured, uint8_t sense_code, bool from_event = false);
	inline void send_status_delay(int delay, uint8_t error_occured, uint8_t sense_code);
	inline void send_message_in();
	inline void go_disconnect();
	inline void go_idle();
	inline void reselection(int next_phase);

	inline void play_seek_sound(int sum);
	inline void register_seek_sound(int sum, int delay);

//	inline void update_bus_status(SXSI_HOST::SXSI_SIGNAL_STATUS_TYPE bus_type, bool onoff);
	inline void set_request_delay(int delay, int next_phase);
	inline void send_request();

	inline void cancel_my_event(int event_id);
	inline void register_my_event(int event_id, int usec);

//	inline HARDDISK *get_disk_unit(int unit_num) const;
//	inline bool unit_mounted(int unit_num) const;

public:
	SCSI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id);
	~SCSI_CTRL();

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
	void go_message_out_phase();
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

//	void change_device_type(int num);

#ifdef USE_DEBUGGER
	void debug_write_data(uint32_t data);
	uint32_t debug_read_data();
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

/**
	@brief SCSI relay class
*/
class SCSI_CTRL_RELAY : public SXSI_CTRL_RELAY
{
public:
	SCSI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id);

	// unique functions
	void change_device_type(int num);
	bool unit_mounted_at_least() const;
};

#if 0
/**
	@brief SCSI device list
*/
class SCSI_CTRLS : public std::vector<SCSI_CTRL *>
{
public:
	SCSI_CTRLS();
};

/**
	@brief SCSI unit list
*/
class SCSI_UNITS : public CPtrList<HARDDISK>
{
public:
	SCSI_UNITS(int alloc_size);
};
#endif

#endif /* SCSI_H */
