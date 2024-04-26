/** @file fdc.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@par Origin
	upd765a.cpp
	@author Sasaji
	@date   2022.02.22 -

	@brief [ UPD7265 modoki ]
*/

#ifndef FDC_H
#define FDC_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief FDC (such as uPD765A/uPD7265) - Floppy Disk Controller
*/
class FDC : public DEVICE
{
public:
	/// @brief signals of FDC
	enum SIG_FDC_IDS {
		SIG_RESET		= 0,
		SIG_TC,
		SIG_DACK,
		SIG_CLOCKNUM
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_PHASE		= 0,
		EVENT_DRQ,
		EVENT_LOST,
		EVENT_RESULT7,
//		EVENT_INDEX,
		EVENT_SEEK_STEP_0,
		EVENT_SEEK_STEP_1,
		EVENT_SEEK_STEP_2,
		EVENT_SEEK_STEP_3,
		EVENT_SEEK_END_0,
		EVENT_SEEK_END_1,
		EVENT_SEEK_END_2,
		EVENT_SEEK_END_3,
		EVENT_HEAD_UNLOAD,
		FDC_REGISTER_IDS
	};

	/// @brief commands
	enum en_commands {
		CMD_READ_DIAGNOSTIC		= 0x02,
		CMD_READ_A_TRACK		= 0x02,	// same as above
		CMD_SPECIFY				= 0x03,
		CMD_SENSE_DEVICE_STATUS	= 0x04,
		CMD_WRITE_DATA			= 0x05,
		CMD_READ_DATA			= 0x06,
		CMD_RECALIBRATE			= 0x07,
		CMD_SENSE_INTER_STATUS	= 0x08,
		CMD_DELETED_WRITE_DATA	= 0x09,
		CMD_READ_ID				= 0x0a,
		CMD_DELETED_READ_DATA	= 0x0c,
		CMD_WRITE_ID			= 0x0d,
		CMD_FORMAT_A_TRACK		= 0x0d,	// same as above
		CMD_SEEK				= 0x0f,
		CMD_SCAN_EQUAL			= 0x11,
		CMD_SCAN_LOW_OR_EQUAL	= 0x19,
		CMD_SCAN_HIGH_OR_EQUAL	= 0x1d,
		CMD_FLAG_MFM			= 0x40,
	};

	/// @brief main status
	enum en_status {
		S_D0B	= 0x01,
		S_D1B	= 0x02,
		S_D2B	= 0x04,
		S_D3B	= 0x08,
		S_CB	= 0x10,
		S_NDM	= 0x20,
		S_DIO	= 0x40,
		S_RQM	= 0x80,
	};

	/// @brief status 0
	enum en_status_0 {
		ST0_NR	= 0x000008,
		ST0_EC	= 0x000010,
		ST0_SE	= 0x000020,
		ST0_AT	= 0x000040,
		ST0_IC	= 0x000080,
		ST0_AI	= 0x0000c0,
	};

	/// @brief status 1
	enum en_status_1 {
		ST1_MA	= 0x000100,
		ST1_NW	= 0x000200,
		ST1_ND	= 0x000400,
		ST1_OR	= 0x001000,
		ST1_DE	= 0x002000,
		ST1_EN	= 0x008000,
	};

	/// @brief status 2
	enum en_status_2 {
		ST2_MD	= 0x010000,
		ST2_BC	= 0x020000,
		ST2_SN	= 0x040000,
		ST2_SH	= 0x080000,
		ST2_WC	= 0x100000,
		ST2_DD	= 0x200000,
		ST2_CM	= 0x400000,
	};

	/// @brief status 3
	enum en_status_3 {
		ST3_HD	= 0x04,
		ST3_TS	= 0x08,
		ST3_T0	= 0x10,
		ST3_RY	= 0x20,
		ST3_WP	= 0x40,
		ST3_FT	= 0x80,
	};

	enum en_phases {
		PHASE_IDLE		= 0,
		PHASE_CMD,
		PHASE_EXEC,
		PHASE_READ,
		PHASE_WRITE,
		PHASE_SCAN,
		PHASE_TC,
		PHASE_TIMER,
		PHASE_RESULT,
	};

	enum en_recalib_counts {
//		RECALIB_COUNTS = 77,	///< uPD765A
		RECALIB_COUNTS = 255,	///< uPD7265
	};

private:
	DEVICE *d_fdd;

	// config
	bool m_ignore_crc;

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;
	outputs_t outputs_hdu;

	// fdc
	struct {
		uint8_t tag_track;
		uint8_t cur_track;
		uint8_t result;
	} m_fdc[4];

	union un_id {
		uint8_t b[4];
		struct {
			uint8_t c;
			uint8_t h;
			uint8_t r;
			uint8_t n;
		};
	} m_id;

	en_phases m_phase, m_prevphase;
	uint8_t m_main_status;
	uint8_t m_seekstat;

	struct st_cmd_args {
		uint8_t cmd;
		uint8_t count;
		union {
			uint8_t b[8];
			struct {
				uint8_t hdu;
				uint8_t c;
				uint8_t h;
				uint8_t r;
				uint8_t n;
				uint8_t eot;
				uint8_t gpl;
				uint8_t dtl;
			} rw;
			struct {
				uint8_t hdu;
				uint8_t n;		// sector size
				uint8_t sc;		// sectors/track
				uint8_t gpl;	// GAP length
				uint8_t data;	// filler byte
			} format;
			struct {
				uint8_t hdu;
				uint8_t ncn;	// next track
			} seek;
			struct {
				uint8_t srt_hut;
				uint8_t hlt;
			} specify;
			struct {
				uint8_t hdu;
			};
		};
	} m_command;
	uint8_t *p_cmd_args;

	struct st_result {
		int count;
		union {
			uint8_t b[8];
			struct {
				uint8_t st0;
				uint8_t st1;
				uint8_t st2;
				uint8_t c;
				uint8_t h;
				uint8_t r;
				uint8_t n;
			} rw;
			struct {
				uint8_t st0;
				uint8_t pcn;
			} intstat;
			struct {
				uint8_t st3;
			} devstat;
		};
	} m_result_codes;
	uint8_t *p_result_codes;

	int      m_seek_count;	///< number of seek pulse
	int		 m_data_count;
	int		 m_data_size;	///< sector size
	uint32_t m_result;
	int m_step_rate_time;	///< step rate time (ms)
	int m_head_load_time;	///< head load time (ms)
	int m_head_unload_time;	///< head unload time (ms)
	bool m_no_dma_mode;
#ifdef UPD765A_DMA_MODE
	bool m_dma_data_lost;
#endif

	en_phases m_next_phase;

	int m_write_id_phase;
	int m_sector_count;

	// event
	int m_register_id[FDC_REGISTER_IDS];

	// clock
	int m_clk_num;	// 0:4MHz 1:8MHz
	// specified density on a command
	int m_density;	// 0:single density(FM) 1:double density(MFM)

	int m_channel;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		// fdc
		struct {
			uint8_t tag_track;
			uint8_t cur_track;
			uint8_t result;
			char    reserved;
		} m_fdc[4];

		// 1
		struct {
			uint8_t c;
			uint8_t h;
			uint8_t r;
			uint8_t n;
		} m_id;

		struct {
			uint8_t cmd;
			uint8_t count;
			uint8_t hdu;
			struct {
				uint8_t c;
				uint8_t h;
				uint8_t r;
				uint8_t n;
				uint8_t eot;
				uint8_t gpl;
				uint8_t dtl;
			} rw;
			char reserved[2];
		} m_command;

		// 2
		uint8_t m_phase;
		uint8_t m_prevphase;
		uint8_t m_main_status;
		uint8_t m_seekstat;

		struct {
			int count;
			struct {
				uint8_t st0;
				uint8_t st1;
				uint8_t st2;
				uint8_t c;
				uint8_t h;
				uint8_t r;
				uint8_t n;
			} rw;
			char reserved;
		} m_result_codes;

		// 3
		int      m_seek_count;	///< number of seek pulse
		int		 m_data_count;
		int		 m_data_size;	///< sector size
		uint32_t m_result;

		// 4
		int m_step_rate_time;	///< step rate time (ms)
		int m_head_load_time;	///< head load time (ms)
		int m_head_unload_time;	///< head unload time (ms)

		// config
		uint8_t m_ignore_crc;
		uint8_t m_no_dma_mode;
		uint8_t m_dma_data_lost;
		uint8_t m_next_phase;

		// 5
		int m_write_id_phase;
		int m_sector_count;

		// clock
		int m_clk_num;	// 0:4MHz 1:8MHz
		// specified density on a command
		int m_density;	// 0:single density(FM) 1:double density(MFM)

		// 6
		int m_channel;

		// event
		int m_register_id[15];
	};
#pragma pack()

	void cancel_my_event(int event_id);
	void cancel_my_events();
	void register_my_event(int event_id, double usec);
	void register_my_event_by_clock(int event_id, int clock);
	void register_phase_event_old(en_phases next_phase, double usec);
	void register_phase_event_new(en_phases next_phase, double usec);
	void register_phase_event_by_clock(en_phases next_phase, int clock);
	void register_seek_step_event(int drv);
	void register_seek_end_event(int drv);
	void register_search_event(int wait);
	void register_drq_event();
	void register_lost_event(int bytes);
//	void register_restore_event();
	void register_head_unload_event();

	// image handler
	uint32_t verify_track();
	uint32_t search_sector(int side, int sector, bool compare_side);
	uint32_t search_sector_by_index(int side, int index, int sector, bool compare_side);
	uint32_t search_addr();
	bool make_track();
	bool parse_track();

	// timing
	uint32_t m_prev_drq_clock;

	int get_cur_position(int drv);
	double get_usec_to_exec_phase();

	// update status
	void set_irq(bool val);
	void set_drq(bool val);
	void set_hdu(uint8_t val);

	// phase shift
	void shift_to_idle();
	void shift_to_cmd(int length);
	void shift_to_exec();
	void shift_to_read();
	void shift_to_write();
	void shift_to_write_id();
	void shift_to_scan();
	void shift_to_result(int length);
	void set_to_result7();
	void set_to_result7_event();
	void set_to_result7_event(uint8_t c, uint8_t h, uint8_t r, uint8_t n);
	void start_transfer_old();
	void start_transfer_data();
	void start_transfer_diagnostic();
	void start_transfer_id();
	void start_transfer_index();
	void finish_transfer();

	// command
	void accept_cmd();
	void process_cmd();
	void execute_cmd();
	void accept_cmd_sense_devstat();
	void cmd_sense_devstat();
	void accept_cmd_sense_intstat();
	uint8_t get_devstat(int drv);
	void accept_cmd_seek();
	void cmd_seek();
	void accept_cmd_recalib();
	void cmd_recalib();
	void seek(int drv, int trk);
	void seek_event(int drv);
	void seek_end(int drv);
	void accept_cmd_read_data();
	void cmd_read_data();
	void execute_cmd_read_data();
	void accept_cmd_write_data();
	void cmd_write_data();
	void execute_cmd_write_data();
	void accept_cmd_scan();
	void cmd_scan();
	void execute_cmd_scan();
	void accept_cmd_read_diagnostic();
	void cmd_read_diagnostic();
	void execute_cmd_read_diagnostic();
	void read_data(bool deleted, bool scan);
	void write_data(bool deleted);
	void read_diagnostic();
//	uint32_t read_sector();
//	uint32_t write_sector(bool deleted);
	uint32_t find_id();
	uint32_t check_cond();
	void get_sector_params();
	bool id_increment();
	inline void set_chrn();
	void accept_cmd_read_id();
	void cmd_read_id();
	void execute_cmd_read_id();
	void accept_cmd_write_id();
	void cmd_write_id();
	void execute_cmd_write_id();
	void read_id();
	void write_id();
	void accept_cmd_specify();
	void cmd_specify();
	void accept_cmd_invalid();
	void update_head_flag(int drv, bool head_load);

public:
	FDC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("FDC");
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_drq);
		init_output_signals(&outputs_hdu);
		d_fdd = NULL;
		m_clk_num = 0;
		m_channel = 0;
	}
	~FDC() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void update_config();

	// unique function
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_hdu(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_hdu, device, id, mask);
	}
	void set_context_fdd(DEVICE* device) {
		d_fdd = device;
	}
	void set_context_clock_num(int clock_num) {
		m_clk_num = clock_num;
	}
	void set_channel(int ch) {
		m_channel = (uint32_t)(ch << 16);
	}

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* FDC_H */
