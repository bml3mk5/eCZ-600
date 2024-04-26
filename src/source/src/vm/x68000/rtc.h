/** @file rtc.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@note Original author is Takeda.Toshiya

	@brief [ real time clock (RP5C15) ]
*/

#ifndef RTC_H
#define RTC_H

#include "../vm_defs.h"
#include "../device.h"
#include "../../curtime.h"

class EMU;
class VM;

/**
	@brief real time clock control

	@note clkout is only supported on 1Hz or 16Hz 
*/
class RTC : public DEVICE
{
public:
	enum SIGNAL_IDS {
		SIG_UPDATE_ALARM = 0,
	};
private:
	/// @brief event ids
	enum EVENT_IDS {
//		EVENT_RTC_1HZ	= 0,
		EVENT_RTC_16HZ	= 1,
	};
	enum en_rtc_names {
		RTC_1SEC = 0,
		RTC_10SEC,
		RTC_1MIN,
		RTC_10MIN,
		RTC_1HOUR,
		RTC_10HOUR,
		RTC_WEEK,
		RTC_1DAY,
		RTC_10DAY,
		RTC_1MON,
		RTC_10MON,
		RTC_1YEAR,
		RTC_10YEAR,
		RTC_MODE,
		RTC_TEST,
		RTC_RESET,
		// BANK1
		RTC_CLKOUT,
		RTC_ADJUST,
		RTC_ALARM_1MIN,
		RTC_ALARM_10MIN,
		RTC_ALARM_1HOUR,
		RTC_ALARM_10HOUR,
		RTC_ALARM_WEEK,
		RTC_ALARM_1DAY,
		RTC_ALARM_10DAY,
		RTC_RESERVED1,
		RTC_12_24_SEL,
		RTC_LEAP_YEAR,
		RTC_ALARM_MASK,	// no compare flag (emu original)
		RTC_REGS_END
	};
	enum en_rtc_mode_masks {
		MODE_ALARM_EN = 0x04,
		MODE_TIMER_EN = 0x08,
	};
	enum en_rtc_reset_masks {
		RESET_ALARM = 0x01,
		RESET_16HZ_ALARM_EN = 0x04,
		RESET_1HZ_ALARM_EN = 0x08,
	};

private:
	// output signals
	outputs_t outputs_alarm;
	outputs_t outputs_clkout;

	CurTime m_cur_time;
	int m_register_id;

	static const uint8_t c_regs_mask[RTC_REGS_END];

	uint8_t m_regs[RTC_REGS_END];

	enum en_clock_masks {
		ALARM_MATCH  = 0x10000000,
		CLOCK_LIMIT  = 0x0fffffff,
		CLOCK_1HZ    = 0x00000010,
		CLOCK_16HZ   = 0x00000001,
		CLOCK_1_60HZ = 0x20000000,
		CLOCK_ALWAYS = 0x40000000,
		CLOCK_1HZ_SFT = 4,
	};
	uint32_t m_alarm;
	uint32_t m_clkout;
	uint32_t m_clkout_mask;
	uint32_t m_clock_count;

	uint32_t m_alarm_led;	// for X68K
	uint32_t m_clkout_led;	// for X68K

	int  m_loaded;

	void load_file();
	void save_file();

	void update_alarm();
	void update_clkout();
	void read_from_cur_time();
	void write_to_cur_time();

	void set_clock_output(uint32_t data);
	inline void update_alarm_led();
	inline void update_clkout_led();

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_regs[RTC_REGS_END];
		char reserved[3];

		int m_register_id;
		uint32_t m_alarm;
		uint32_t m_clkout;
		uint32_t m_clkout_mask;

		uint32_t m_clock_count;
		uint32_t reserved2[3];
	};
#pragma pack()

public:
	RTC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		init_output_signals(&outputs_alarm);
		init_output_signals(&outputs_clkout);

		set_class_name("RTC");
	}
	~RTC() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);

	// unique functions
	void set_context_alarm(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_alarm, device, id, mask);
	}
	void set_context_clkout(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_clkout, device, id, mask);
	}
	uint32_t get_led_status();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* RTC_H */
