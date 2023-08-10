/** @file adpcm.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@par Origin okim6528.cpp on MAME 0.293 and msm5205.cpp on CSP
	@author Sasaji
	@date   2022.02.22 -

	@brief [ MSM6528 modoki (ADPCM) ]
*/

#ifndef ADPCM_H
#define ADPCM_H

#include "../vm.h"
#include "../../emu.h"
#include "../sound_base.h"

/**
	@brief MSM6528 modoki - ADPCM
*/
class ADPCM : public SOUND_BASE
{
public:
	enum en_signal_ids {
		SIG_SEL_CLOCK = 0,
		SIG_SEL_FREQUENCY,	///< select divider of sampling rate
		SIG_DACK,
	};
	enum en_output_types {
		TYPE_3BITS          = 0,
		TYPE_4BITS          = 1,
	};

private:
	enum en_event_ids {
		EVENT_TIMER = 0
	};
	enum en_reg_nums {
		REG_CMD_STS = 0,
		REG_DATA,
	};
	enum en_cmd_masks {
		COMMAND_STOP	= (1 << 0),
		COMMAND_PLAY	= (1 << 1),
		COMMAND_RECORD	= (1 << 2),
	};
	enum en_status_masks {
		STATUS_IDLING		= 0x80,
		STATUS_PLAYING		= (1 << 0),	// internal use
		STATUS_RECORDING	= (1 << 1),	// internal use
	};
	enum en_buffer_size {
		BUFF_SIZE = 16,
	};

private:
	uint8_t m_reg_cmd;			///< command register
	uint8_t m_reg_sts;			///< status register
	uint8_t m_reg_rdata;		///< read data register
	uint8_t m_reg_wdata;		///< written data register

	int     m_base_clock;		///< clock rate (4MHz)
	int     m_clock_mag;		///< current clock magnification 
	double  m_period;			///< event time
	int m_timer_event_id;
	int     m_vclk;				///< vclk signal
	int     m_req_data;			///< request data 
	int     m_prescaler;		///< prescaler selector SAM1 and SAM2
	int     m_output_bits;		///< D/A precision is 10-bits but 12-bit data can be output serially to an external DAC
//	uint8_t m_data_in;
	int32_t m_signal;			///< current ADPCM signal
	int32_t m_step;				///< current ADPCM step
//	uint8_t m_nibble_shift;		///< nibble select
	int32_t m_signal_max;
	int32_t m_signal_min;

	int m_silent_count;			///< same data continued

	int m_diff_lookup[49*16];

//	int m_select;
	uint32_t m_pan_left;
	uint32_t m_pan_right;

	int     m_rate_on_host;		///< sampling rate on host machine

	int32_t m_signals_buff_l[BUFF_SIZE];	///< current ADPCM signal buffered
	int32_t m_signals_buff_r[BUFF_SIZE];	///< current ADPCM signal buffered
	int32_t m_signals_w;		///< current ADPCM signal buffered
//#define MAX_PREV_SIGNALS 32
//	int32_t m_prev_signals[MAX_PREV_SIGNALS];
	int32_t m_prev_signals[3];
	int32_t m_prev_samples[3];

	int32_t m_prev_sample_l;
	int32_t m_prev_sample_r;

	double m_time_host;			///< time per one sample
	double m_time_host_one;		///< time per one sample
	double m_time_signal;
	double m_time_signal_one;

	uint8_t m_tables_computed;

	int volume_l, volume_r;

	outputs_t outputs_mck;

	static const double c_dividers[4];
	static const int c_index_shift[8];

	// for resume
	struct vm_state_st {
		uint8_t m_reg_cmd;			///< command register
		uint8_t m_reg_sts;			///< status register
		uint8_t m_reg_rdata;		///< read data register
		uint8_t m_reg_wdata;		///< written data register
		int     m_base_clock;		///< clock rate (4MHz)
		int     m_clock_mag;		///< current clock magnification 
		int     m_timer_event_id;
		// 1
		uint64_t m_period;			///< event time
		int     m_vclk;				///< vclk signal
		int     m_req_data;			///< request data 
		// 2
		int     m_prescaler;		///< prescaler selector SAM1 and SAM2
		int     m_output_bits;		///< D/A precision is 10-bits but 12-bit data can be output serially to an external DAC
		int32_t m_signal;			///< current ADPCM signal
		int32_t m_step;				///< current ADPCM step
		// 3
		int32_t m_signal_max;
		int32_t m_signal_min;
		int     m_silent_count;		///< same data continued
		int     m_rate_on_host;		///< sampling rate on host machine
		// 4
		uint32_t m_pan_left;
		uint32_t m_pan_right;
		int volume_l, volume_r;
		// 5
		int32_t m_signals_buff_l[16];	///< current ADPCM signal buffered
		int32_t m_signals_buff_r[16];	///< current ADPCM signal buffered
		// 13
		int32_t m_prev_signals[32];
		// 21
		int32_t m_signals_w;		///< current ADPCM signal buffered
		int32_t m_prev_sample_l;
		int32_t m_prev_sample_r;
		char reserved[4];
		// 22
		uint64_t m_time_host;			///< time per one sample
		uint64_t m_time_host_one;		///< time per one sample
		// 23
		uint64_t m_time_signal;
		uint64_t m_time_signal_one;
	};

	void compute_tables();
	void play(bool onoff);
	void calc_samples(uint8_t data);
	void change_clock();
	inline int32_t clock_adpcm(uint8_t nibble);

public:
	ADPCM(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~ADPCM() {}

	// common functions
	void initialize();
	void reset();
	void warm_reset(bool por);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);

	void event_callback(int event_id, int err);

	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel_l, int decibel_r, bool mute);

	// unique functions
	void initialize_sound(int sample_rate, int clock, int samples, int mode);

	void set_context_mck(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_mck, device, id, mask);
	}

	void set_outbits(int outbit) {
		m_output_bits = outbit;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* ADPCM_H */

