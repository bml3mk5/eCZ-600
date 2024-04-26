/** @file floppy.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ floppy drive ]
*/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "../vm_defs.h"
#include "../sound_base.h"
#include "../noise.h"

//#define _DEBUG_FLOPPY

#include "floppy_defs.h"

//class FDC;
class EMU;
class DISK;

/**
	@brief floppy drive
*/
class FLOPPY : public SOUND_BASE
{
private:
	/// @brief delay time on each modules
	enum DELAY_TIMES {
		/// index hole (2DD) (usually 300rpm -> 200ms)
		DELAY_INDEX_HOLE	  = 200000,	// (us)
		/// index hole (2HD) (usually 360rpm -> 166.666ms)
		DELAY_INDEX_HOLE_H	  = 166667,	// (us)

		/// ready on delay time after motor on (us) (2DD)
		DELAY_READY_ON		 = 200000,
		/// ready on delay time after motor on (us) (2HD) 
		DELAY_READY_ON_H     = 200000,

//		/// motor warmup time (us)
//		DELAY_MOTOR_WARMUP	  = 500000,
		/// motor off delay time (us)
		DELAY_MOTOR_OFF		=  5000000,
#ifdef USE_FLOPPY_TIMEOUT
		/// motor off timeout (us)
		DELAY_MOTOR_TIMEOUT	= 60000000,
#endif
#ifdef USE_FLOPPY_HEAD_LOADING
		/// head loaded time 60ms
		HEAD_LOADED_TIME	= 60000,
		HEAD_LOADED_CLOCK	= (HEAD_LOADED_TIME * (CPU_CLOCKS / 1000000)),
#endif

		DELAY_WRITE_FRAME	= 300,
	};

	/// @brief event ids
	enum EVENT_IDS {
//		EVENT_IRQ			= 0,
//		EVENT_DRQ			= 1,

		EVENT_MOTOR_TIMEOUT	= 0,
		EVENT_INDEXHOLE_ON,
		EVENT_INDEXHOLE_OFF,

		EVENT_READY_ON_0,
		EVENT_READY_ON_1,
		EVENT_READY_ON_2,
		EVENT_READY_ON_3,
		EVENT_READY_ON_4,
		EVENT_READY_ON_5,
		EVENT_READY_ON_6,
		EVENT_READY_ON_7,
		EVENT_MOTOR_OFF,
		EVENT_IDS_MAX
	};

	/// @brief optional signals $E94005
	enum en_e94005_masks {
		OPS_DRIVE_0      = 0x01,
		OPS_DRIVE_1      = 0x02,
		OPS_DRIVE_2      = 0x04,
		OPS_DRIVE_3      = 0x08,
		OPS_EJECT_ON     = 0x20,
		OPS_EJECT_SW_MSK = 0x40,
		OPS_LED_BLINK    = 0x80,
	};
	/// @brief access drive select $E94007
	enum en_e94007_masks {
		ADS_DRIVE_0      = 0x00,
		ADS_DRIVE_1      = 0x01,
		ADS_DRIVE_2      = 0x02,
		ADS_DRIVE_3      = 0x03,
		ADS_FD_TYPE      = 0x10,
		ADS_MOTOR_ON     = 0x80,
	};


private:
	DEVICE *d_fdc, *d_board;

	/// config
	bool m_ignore_crc;

	/// event
	int m_register_id[EVENT_IDS_MAX];

	//
	uint8_t m_drv_num;	///< drive number
//	uint8_t m_drv_sel;	///< drive select + motor

//	uint8_t sidereg;	///< head select

	uint8_t m_index_hole;	///< index hole
	uint64_t m_index_hole_next_clock;
#ifdef USE_FLOPPY_HEAD_LOADING
	uint8_t m_head_load;	///< head loaded
#endif

	uint8_t m_density;	///< density (FM = 0, MFM = 1)
	uint8_t m_motor_on_expand;	///< motor on (expand)

	int m_sectorcnt;
	bool m_sectorcnt_cont;

	uint8_t m_drv_ctrl;
	uint8_t m_accs_drv;

	int m_led_blink;	///< blink led counter

//	uint8_t m_opm_addr;


//	bool irqflg;
//	bool irqflgprev;
//	bool drqflg;
//	bool drqflgprev;

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;

	/// drive info
	typedef struct {
		int side;
		int track;
		int index;
#ifdef USE_SIG_FLOPPY_ACCESS
		bool access;
#endif
		uint8_t ready;			///< ready warmup:01 on:11
		uint8_t motor_warmup;	///< motor warming up:1
#ifdef USE_FLOPPY_HEAD_LOADING
		uint8_t head_loading;
#endif
		int delay_write;

		bool motor_on;
		bool inserted;			///< later than disk->inserted
		bool shown_media_error; 

		uint8_t drv_ctrl;
		uint8_t opened_intr;	///< delay time of asserting interrupt after inserted disk
		uint8_t closed_intr;	///< delay time of asserting interrupt after ejected disk
		uint8_t force_eject;	///< enable eject forcely while over zero 
	} fdd_t;
	fdd_t m_fdd[USE_FLOPPY_DISKS];

	// diskette info
	DISK* p_disk[USE_FLOPPY_DISKS];

	// index hole
	int m_delay_index_hole;	///< spent clocks per round (cpu clocks)
	int m_limit_index_hole; ///< width of asserting index signal (cpu clocks)
	int m_delay_index_mark;	///< clocks in index mark area (cpu clocks)
	// ready on
	int m_delay_ready_on;
	bool m_force_ready;

	uint32_t m_now_irq;	///< current interrupt status

	bool m_wav_loaded_at_first;

	/// sound data 1st dimension [0]:5inch
	enum WAV_FDDTYPES {
		FLOPPY_WAV_TYPE2HD = 0,
		FLOPPY_WAV_FDDTYPES,
	};
	/// 2nd [0]:seek sound [1]:motor sound [2]:head on [3]:head off
	enum WAV_SNDTYPES {
		FLOPPY_WAV_SEEK		= 0,
		FLOPPY_WAV_MOTOR,
		FLOPPY_WAV_EJECT,
		FLOPPY_WAV_SNDTYPES,
	};
	NOISE m_noises[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];

	//for resume
#pragma pack(1)
	struct vm_state_st {
		// 0
		uint8_t m_ignore_crc;
		uint8_t m_drv_num;	///< drive number
		uint8_t m_index_hole;	///< index hole
		uint8_t m_head_load;	///< head loaded
		uint8_t m_density;	///< density (FM = 0, MFM = 1)
		uint8_t m_motor_on_expand;	///< motor on (expand)
		uint8_t m_drv_ctrl;
		uint8_t m_accs_drv;
		uint64_t m_index_hole_next_clock;

		// 1
		struct {
			int side;
			int track;
			int index;

			uint8_t access;

			uint8_t ready;			///< ready warmup:01 on:11
			uint8_t motor_warmup;	///< motor warming up:1
			uint8_t head_loading;

			int delay_write;

			uint8_t motor_on;
			uint8_t inserted;			///< later than disk->inserted
			uint8_t shown_media_error; 

			uint8_t drv_ctrl;
			uint8_t opened_intr;	///< delay time of asserting interrupt after inserted disk
			uint8_t closed_intr;	///< delay time of asserting interrupt after ejected disk
			uint8_t force_eject;	///< enable eject forcely while over zero

			char reserved[5];
		} m_fdd[4];

		int m_led_blink;	///< blink led counter

		int m_delay_ready_on;

		uint8_t m_force_ready;
		char reserved1[2];
		uint8_t m_sectorcnt_cont;

		int m_sectorcnt;

		uint32_t m_now_irq;	///< current interrupt status

		/// event
		int m_register_id[15];

		/// noise
		struct NOISE::vm_state_st m_noises[FLOPPY_WAV_FDDTYPES][FLOPPY_WAV_SNDTYPES];
	};
#pragma pack()

	// cannot write when load state from file
	bool m_ignore_write;

	void warm_reset(bool);

	inline void cancel_my_event(int);
	inline void register_my_event(int, int);
	void register_index_hole_event(int, int);
	void cancel_my_events();

	void set_drive_control(uint8_t data);
	uint8_t get_drive_status(bool negate_intr);
	void set_access_drive(uint8_t data);

	// irq/dma
	void set_irq(uint32_t data, uint32_t mask);
//	void set_drq(bool val);

	int  search_sector_main(int fdcnum, int drvnum, int index);

	void load_wav();

	void motor(int drv, bool val);

	void set_drive_speed();

public:
	FLOPPY(VM* parent_vm, EMU* parent_emu, const char* identifier) : SOUND_BASE(parent_vm, parent_emu, identifier) {
		set_class_name("FLOPPY");
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_drq);
		d_fdc = NULL;
	}
	~FLOPPY() {}

	// common functions
	void initialize();
	void reset();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
//	void write_dma_io8(uint32_t addr, uint32_t data);
//	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	// unique functions
	void set_context_fdc(DEVICE* device) {
		d_fdc = device;
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
//	void set_context_drq(DEVICE* device, int id, uint32_t mask) {
//		register_output_signal(&outputs_drq, device, id, mask);
//	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	bool search_track(int channel);
	bool verify_track(int channel, int track);
	int  get_current_track_number(int channel);
	int  search_sector(int channel);
	int  search_sector(int channel, int track, int sect, bool compare_side, int side);
	int  search_sector_by_index(int channel, int track, int index, bool compare_sect, int sect, bool compare_side, int side);
	bool make_track(int channel);
	bool parse_track(int channel);

	int get_a_round_clock(int channel);
	int get_head_loading_clock(int channel);
	int get_index_hole_remain_clock();
	int calc_index_hole_search_clock(int channel);
	int get_clock_arrival_sector(int channel, int sect, int delay);
	int get_clock_next_sector(int channel, int delay);
	int calc_sector_search_clock(int channel, int sect);
	int calc_next_sector_clock(int channel);

	// user interface
	bool open_disk(int drv, const _TCHAR *path, int offset, uint32_t flags);
	bool close_disk(int drv, uint32_t flags);
	int  change_disk(int drv);
	void set_disk_side(int drv, int side);
	int  get_disk_side(int drv);
	bool disk_inserted(int drv);
	void set_drive_type(int drv, uint8_t type);
	uint8_t get_drive_type(int drv);
	uint8_t fdc_status();
	void toggle_disk_write_protect(int drv);
	bool disk_write_protected(int drv);
	bool is_same_disk(int drv, const _TCHAR *file_path, int offset);
	int  inserted_disk_another_drive(int drv, const _TCHAR *file_path, int offset);

//	uint16_t get_drive_select();
	uint32_t get_led_status() const;

	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel, bool vol_mute);
	void initialize_sound(int rate, int decibel);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* FLOPPY_H */

