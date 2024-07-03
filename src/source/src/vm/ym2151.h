/** @file ym2151.h

	Skelton for retropc emulator

	@author : Takeda.Toshiya
	@date   : 2009.03.08-

	@note
	Modified by Sasaji at 2022.02.22

	@brief [ YM2151 ]
*/

#ifndef YM2151_H
#define YM2151_H

#include "vm_defs.h"
#include "device.h"
#include "fmgen/opm.h"

#define HAS_YM_SERIES

#ifdef SUPPORT_WIN32_DLL
#define SUPPORT_MAME_FM_DLL
//#include "fmdll/fmdll.h"
#endif

class EMU;

/**
	@brief YM2151 - FM Sound Operator
*/
class YM2151 : public DEVICE
{
public:
	/// @brief signals of YM2151
	enum SIG_YM2203_IDS {
		SIG_YM2151_MUTE		= 0,
	};

private:
	enum EVENT_IDS {
		EVENT_FM_TIMER	= 0,
	};

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_ct1;
	outputs_t outputs_ct2;

//#ifdef USE_DEBUGGER
//	DEBUGGER *d_debugger;
//#endif
	FM::OPM* opm;

#ifdef SUPPORT_MAME_FM_DLL
//	CFMDLL* fmdll;
	LPVOID* dllchip;
#endif
	struct {
//		bool written;
		uint8_t data;
	} port_log[0x100];
	int base_decibel;

	int chip_clock;
	uint8_t ch;
	bool irq_prev, mute;

	uint64_t clock_prev;
	uint64_t clock_accum;
	uint64_t clock_const;
	int timer_event_id;

	uint64_t clock_busy;
	bool busy;

	// for resume
#pragma pack(1)
	struct vm_state_st {
		struct {
			uint8_t data;
			char reserved;
		} port_log[0x100];

		int base_decibel;
		int chip_clock;

		uint64_t clock_prev;

		uint64_t clock_accum;
		uint64_t clock_const;

		uint64_t clock_busy;

		uint8_t ch;
		uint8_t irq_prev;
		uint8_t mute;
		uint8_t busy;

		int timer_event_id;

		int reserved[3];
	};
#pragma pack()

	void update_count();
	void update_event();
	void update_interrupt();

public:
	YM2151(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~YM2151() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int error);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel_l, int decibel_r, bool mute);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask, uint32_t negative = 0) {
		register_output_signal(&outputs_irq, device, id, mask, negative);
	}
	void set_context_ct1(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_ct1, device, id, mask);
	}
	void set_context_ct2(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_ct2, device, id, mask);
	}
	void initialize_sound(int rate, int clock, int samples, int decibel);
	void set_reg(uint32_t addr, uint32_t data); // for patch
	uint32_t read_status();

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* YM2151_H */

