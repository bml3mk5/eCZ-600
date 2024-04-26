/** @file rtc.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@note Original author is Takeda.Toshiya

	@brief [ real time clock (RP5C15) ]
*/

#include "rtc.h"
#include "../vm.h"
#include "../../emumsg.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../logging.h"
#include "../../utility.h"


#ifdef _DEBUG
//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_ALARM logging->out_debugf
#define OUT_DEBUG_ALARM(...)
//#define OUT_DEBUG_CLKOUT logging->out_debugf
#define OUT_DEBUG_CLKOUT(...)
#else
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_ALARM(...)
#define OUT_DEBUG_CLKOUT(...)
#endif

const uint8_t RTC::c_regs_mask[] = {
//	1sec.10sec. 1min.10min. 1hou.10hou. week. 1day 10days 1mon.10mon. 1yea.10yea. mode  test  res
	0x0f, 0x07, 0x0f, 0x07, 0x0f, 0x03, 0x07, 0x0f, 0x03, 0x0f, 0x01, 0x0f, 0x0f, 0x0d, 0x0f, 0x0f,
//  clko. adj.  1min.10min. 1hou.10hou. week. 1day 10days ---  12/24  leap
	0x07, 0x01, 0x0f, 0x07, 0x0f, 0x03, 0x07, 0x0f, 0x03, 0x0f, 0x01, 0x03  
};

#define RTC_FILE_NAME "rtc.dat"

void RTC::initialize()
{
	// initialize rtc
	memset(m_regs, 0, sizeof(m_regs));

	// load data
	m_loaded = 0;
	load_file();

	if (!m_loaded) {
		m_regs[RTC_12_24_SEL] = 1;
		m_regs[RTC_MODE] = 8;
		m_regs[RTC_RESET] = 0xc;
	}
	set_clock_output(m_regs[RTC_CLKOUT]);
	m_alarm = 0;
	m_clkout = 0;
	m_clock_count = 0;

	update_alarm_led();
	update_clkout_led();

	// register events
//	register_event(this, EVENT_RTC_1HZ, 1000000, true, &m_register_id);
	register_event(this, EVENT_RTC_16HZ, 1000000. / 32, true, NULL);
}

void RTC::release()
{
	save_file();
}

void RTC::reset()
{
	m_cur_time.GetHostTime();
	read_from_cur_time();
}

void RTC::load_file()
{
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = vm->initialize_path();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!m_loaded) {
			m_loaded = vm->load_data_from_file(app_path, _T(RTC_FILE_NAME), m_regs, sizeof(m_regs));
		}
	}
	if (!m_loaded) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T(RTC_FILE_NAME));
	}
}

void RTC::save_file()
{
	const _TCHAR *app_path, *rom_path[2];
	_TCHAR file_path[_MAX_PATH];

	rom_path[0] = vm->initialize_path();
	rom_path[1] = vm->application_path();

	bool saved = false;
	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!saved) {
			FILEIO fio;
			UTILITY::tcscpy(file_path, _MAX_PATH, app_path);
			UTILITY::tcscat(file_path, _MAX_PATH, _T(RTC_FILE_NAME));
			if(fio.Fopen(file_path, FILEIO::WRITE_BINARY)) {
				fio.Fwrite(m_regs, sizeof(m_regs[0]), sizeof(m_regs));
				fio.Fclose();
				logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_saved, _T(RTC_FILE_NAME));
				saved = true;
			}
		}
	}
	if (!saved) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_saved, _T(RTC_FILE_NAME));
	}
}

void RTC::write_io8(uint32_t addr, uint32_t data)
{
	OUT_DEBUG_REGW(_T("RTC: REGW Addr:%08X Bank:%d Data:%02X"), (addr << 1) | 1, m_regs[RTC_MODE] & 1, data);
#ifdef _DEBUG
	if (data & 0x10) {
		logging->out_debugf(_T("RTC REGW: Addr:%08X Data:%02X bit4 is high??"), (addr << 1) | 1, data);
	}
#endif

	addr &= 0x0f;
	if(addr <= 0x0c) {
		switch(m_regs[RTC_MODE] & 1) {
		case 0:
			// bank0
			if(m_regs[addr] != data) {
				m_regs[addr] = data & c_regs_mask[addr];
				write_to_cur_time();
			}
			break;
		case 1:
			// bank1
			addr += 16;
			if (addr == RTC_12_24_SEL) {
				if (m_regs[addr] ^ data) {
					m_regs[addr] = data & c_regs_mask[addr];
					// am/pm is changed
					write_to_cur_time();
				}
			} else if (addr == RTC_CLKOUT) {
				// setting of clock output
				m_regs[addr] = data & c_regs_mask[addr];
				set_clock_output(data);

			} else if (RTC_ALARM_1MIN <= addr && addr <= RTC_ALARM_10DAY) {
				// alarm
				if(m_regs[addr] != data) {
					m_regs[addr] = data & c_regs_mask[addr];
				}
				// clear no compare flag
				m_regs[RTC_ALARM_MASK] &= ~(1 << (addr - RTC_ALARM_1MIN));
			} else {
				if(m_regs[addr] != data) {
					m_regs[addr] = data & c_regs_mask[addr];
				}
			}
			break;
		}
	} else {
		if (addr == RTC_RESET) {
			m_regs[addr] = data & c_regs_mask[addr];
			uint32_t prev = m_alarm;
			if (data & RESET_ALARM) {
				m_alarm &= ~ALARM_MATCH;
				// set no compare flag
				m_regs[RTC_ALARM_MASK] = 0x7f;
			}
			if (data & RESET_16HZ_ALARM_EN) {
				m_alarm &= ~CLOCK_16HZ;
			}
			if (data & RESET_1HZ_ALARM_EN) {
				m_alarm &= ~CLOCK_1HZ;
			}
			if (prev != m_alarm) {
				update_alarm();
			}
			OUT_DEBUG_ALARM(_T("RTC: RESET REGW Addr:%08X Data:%02X"), (addr << 1) | 1, data);
		}
#ifdef _DEBUG
		else if (addr == RTC_MODE) {
			uint8_t prev = m_regs[addr]; 
			m_regs[addr] = data & c_regs_mask[addr];
			if ((prev ^ data) & 0x4) {
				OUT_DEBUG_ALARM(_T("RTC: MODE ALARM %s"), data & 0x4 ? _T("ON"): _T("OFF"));
			}
		}
#endif
		else {
			m_regs[addr] = data & c_regs_mask[addr];
		}
	}

}

void RTC::set_clock_output(uint32_t data)
{
	switch(data & 7) {
	case 0:
		// HiZ
		m_clkout_mask = CLOCK_ALWAYS;
		break;
	case 1:
		// 16384Hz (TODO:)
		m_clkout_mask = CLOCK_16HZ;
		break;
	case 2:
		// 1024Hz (TODO:)
		m_clkout_mask = CLOCK_16HZ;
		break;
	case 3:
		// 128Hz (TODO:)
		m_clkout_mask = CLOCK_16HZ;
		break;
	case 4:
		// 16Hz
		m_clkout_mask = CLOCK_16HZ;
		break;
	case 5:
		// 1Hz (1sec.)
		m_clkout_mask = CLOCK_1HZ;
		break;
	case 6:
		// 1/60Hz (1min.)
		m_clkout_mask = CLOCK_1_60HZ;
		break;
	case 7:
		// Low
		m_clkout_mask = 0;
		break;
	}
	OUT_DEBUG_CLKOUT(_T("RTC: SET CLKOUT Data:%02X Clk:%08X"), data, m_clkout_mask);
}

#ifndef LEAP_YEAR
#define LEAP_YEAR(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#endif

uint32_t RTC::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	addr &= 0x0f;
	if(addr <= 0x0c) {
		switch(m_regs[RTC_MODE] & 1) {
		case 0:
			data = m_regs[addr];
			break;
		case 1:
			addr += 16;
			data = m_regs[addr];
			break;
		}
	} else {
		data = m_regs[addr];
	}
	if(addr == RTC_LEAP_YEAR) {
		bool match = false;
		for(int i = 0; i < 3; i++) {
			if(LEAP_YEAR(m_cur_time.GetYear() - i)) {
				data = i;
				match = true;
				break;
			}
		}
		if (!match) data = 3;
	}

	return data;
}

void RTC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_UPDATE_ALARM:
		update_alarm();
		break;
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
//		if (!now_reset) {
//			reset();
//		}
		break;
	}
}

void RTC::event_callback(int event_id, int err)
{
	switch(event_id) {
#if 0
	case EVENT_RTC_1HZ:
		m_cur_time.Increment();
		if(m_regs[RTC_MODE] & MODE_TIMER_EN) {
			read_from_cur_time();
			if(m_regs[RTC_MODE] & MODE_ALARM_EN) {
				update_alarm();
			}
		}
		break;
#endif
	case EVENT_RTC_16HZ:
		{
			m_clock_count++;
			m_clock_count &= CLOCK_LIMIT;

			int prev_alarm = m_alarm;

			m_clkout = 0;
			m_clkout |= ((m_clock_count | CLOCK_ALWAYS) & m_clkout_mask);
			update_clkout_led();

			// 1Hz
			if((m_clock_count & 0xf) == 0) {
				if((m_clock_count & 0x1f) == 0) {
					// increment 1sec.
					if(m_regs[RTC_MODE] & MODE_TIMER_EN) {
						m_cur_time.Increment();
						read_from_cur_time();
						if ((m_clkout_mask & CLOCK_1_60HZ) != 0 && m_cur_time.GetSec() < 30) {
							m_clkout |= CLOCK_1_60HZ;
						}
					}
				}
				m_alarm &= ~CLOCK_1HZ;
				if(!(m_regs[RTC_RESET] & RESET_1HZ_ALARM_EN)) {
					m_alarm |= (m_clock_count & CLOCK_1HZ);
				}
			}
			// 16Hz
			if (true) {
				m_alarm &= ~CLOCK_16HZ;
				if(!(m_regs[RTC_RESET] & RESET_16HZ_ALARM_EN)) {
					m_alarm |= (m_clock_count & CLOCK_16HZ);
				}
			}
			if(prev_alarm ^ m_alarm) {
				update_alarm();
			}
		}
		break;
	}
}

void RTC::update_alarm()
{
	update_alarm_led();

	// negative
	if ((m_regs[RTC_MODE] & MODE_ALARM_EN) != 0) {
		if (m_alarm) {
			// alarm
			write_signals(&outputs_alarm, 0);
			// power on
			if (pConfig->now_power_off) {
				emumsg.Send(EMUMSG_ID_POWER_ON);
			}
		} else {
			write_signals(&outputs_alarm, 0xffffffff);
		}
	} else {
		write_signals(&outputs_alarm, 0xffffffff);
	}
}

void RTC::update_clkout()
{
	// negative
	write_signals(&outputs_clkout, m_clkout ? 0 : 0xffffffff);
}

// b9: power led green (alarm or power switch) 
// b11: timer
uint32_t RTC::get_led_status()
{
	return m_alarm_led | m_clkout_led;
}

void RTC::update_alarm_led()
{
	m_alarm_led = ((((m_alarm & CLOCK_1HZ) >> CLOCK_1HZ_SFT) ^ (m_alarm & (CLOCK_ALWAYS | CLOCK_16HZ))) ? 0x200 : 0);
}

void RTC::update_clkout_led()
{
	m_clkout_led = ((((m_clkout & CLOCK_1HZ) >> CLOCK_1HZ_SFT) ^ (m_clkout & (CLOCK_ALWAYS | CLOCK_16HZ))) ? 0x800 : 0);
}

void RTC::read_from_cur_time()
{
	// update clock
	m_cur_time.GetCurrTime();
	int sec = m_cur_time.GetSec();
	int min = m_cur_time.GetMin();
	int hour = m_cur_time.GetHour();
	int day = m_cur_time.GetDay();
	int month = m_cur_time.GetMonth();
	int year = m_cur_time.GetYear();
#ifdef _X68000
	year -= 1980;
#endif
	int dow = m_cur_time.GetDayOfWeek();
	int ampm = (((m_regs[RTC_12_24_SEL] & 1) == 0) && (hour > 11)) ? 0x02 : 0x00;

	hour = (m_regs[RTC_12_24_SEL] & 1) ? hour : (hour % 12);

	m_regs[RTC_1SEC]  = TO_BCD_LO(sec) & c_regs_mask[RTC_1SEC];
	m_regs[RTC_10SEC] = TO_BCD_HI(sec) & c_regs_mask[RTC_10SEC];
	m_regs[RTC_1MIN]  = TO_BCD_LO(min) & c_regs_mask[RTC_1MIN];
	m_regs[RTC_10MIN] = TO_BCD_HI(min) & c_regs_mask[RTC_10MIN];
	m_regs[RTC_1HOUR] = TO_BCD_LO(hour) & c_regs_mask[RTC_1HOUR];
	m_regs[RTC_10HOUR] = (TO_BCD_HI(hour) | ampm) & c_regs_mask[RTC_10HOUR];
	m_regs[RTC_WEEK] = dow & c_regs_mask[RTC_WEEK];
	m_regs[RTC_1DAY] = TO_BCD_LO(day) & c_regs_mask[RTC_1DAY];
	m_regs[RTC_10DAY] = TO_BCD_HI(day) & c_regs_mask[RTC_10DAY];
	m_regs[RTC_1MON] = TO_BCD_LO(month) & c_regs_mask[RTC_1MON];
	m_regs[RTC_10MON] = TO_BCD_HI(month) & c_regs_mask[RTC_10MON];
	m_regs[RTC_1YEAR] = TO_BCD_LO(year) & c_regs_mask[RTC_1YEAR];
	m_regs[RTC_10YEAR] = TO_BCD_HI(year) & c_regs_mask[RTC_10YEAR];

	// check alarm
	int prev_alarm = m_alarm;
	uint8_t mask = 1;
	m_alarm |= ALARM_MATCH;
	for(int i = RTC_1MIN; i <= RTC_10DAY; i++) {
		if (!(m_regs[RTC_ALARM_MASK] & mask)) {
			if (m_regs[i + 16] != m_regs[i]) {
				m_alarm &= ~ALARM_MATCH;
				break;
			}
		}
		mask <<= 1;
	}
	if (prev_alarm ^ m_alarm) {
		update_alarm();
	}
}

void RTC::write_to_cur_time()
{
	m_cur_time.SetSec(m_regs[RTC_1SEC] + (m_regs[RTC_10SEC] & 7) * 10);
	m_cur_time.SetMin(m_regs[RTC_1MIN] + (m_regs[RTC_10MIN] & 7) * 10);
	int hour;
	if(m_regs[RTC_12_24_SEL] & 1) {
		// 24hours
		hour = m_regs[RTC_1HOUR] + (m_regs[RTC_10HOUR] & 3) * 10;
	} else {
		// 12hours
		hour = m_regs[RTC_1HOUR] + (m_regs[RTC_10HOUR] & 1) * 10;
		if ((m_regs[RTC_10HOUR] & 0x02) != 0) hour += 12;
	}
	m_cur_time.SetHour(hour);
//	m_cur_time.SetDayOfWeek(regs[6] & 7);
	m_cur_time.SetDay(m_regs[RTC_1DAY] + (m_regs[RTC_10DAY] & 3) * 10);
	m_cur_time.SetMonth(m_regs[RTC_1MON] + (m_regs[RTC_10MON] & 1) * 10);
	int year = m_regs[RTC_1YEAR] + m_regs[RTC_10YEAR] * 10;
#ifdef _X68000
	year += 1980;
#endif
	m_cur_time.SetYear(year);
	m_cur_time.CommitTime();

	// restart event
//	cancel_event(this, m_register_id);
//	register_event(this, EVENT_RTC_1HZ, 1000000.0, true, &m_register_id);
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void RTC::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<RTC_REGS_END; i++) {
		SET_Byte(m_regs[i]);
	}

	SET_Int32_LE(m_register_id);
	SET_Uint32_LE(m_alarm);
	SET_Uint32_LE(m_clkout);
	SET_Uint32_LE(m_clkout_mask);

	SET_Uint32_LE(m_clock_count);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool RTC::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	for(int i=0; i<RTC_REGS_END; i++) {
		GET_Byte(m_regs[i]);
	}

	GET_Int32_LE(m_register_id);
	GET_Uint32_LE(m_alarm);
	GET_Uint32_LE(m_clkout);
	GET_Uint32_LE(m_clkout_mask);

	GET_Uint32_LE(m_clock_count);

	update_alarm_led();
	update_clkout_led();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t RTC::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

static const _TCHAR *c_reg_names[] = {
	_T("1SEC"),
	_T("10SEC"),
	_T("1MIN"),
	_T("10MIN"),
	_T("1HOUR"),
	_T("10HOUR"),
	_T("WEEK"),
	_T("1DAY"),
	_T("10DAY"),
	_T("1MON"),
	_T("10MON"),
	_T("1YEAR"),
	_T("10YEAR"),
	_T("MODE"),
	_T("TEST"),
	_T("RESET"),
		// BANK1
	_T("CLKOUT"),
	_T("ADJUST"),
	_T("ALARM_1MIN"),
	_T("ALARM_10MIN"),
	_T("ALARM_1HOUR"),
	_T("ALARM_10HOUR"),
	_T("ALARM_WEEK"),
	_T("ALARM_1DAY"),
	_T("ALARM_10DAY"),
	_T("RESERVED1"),
	_T("12_24_SEL"),
	_T("LEAP_YEAR"),
	_T("ALARM_MASK"),
	//
	_T("ALARM_ON_FORCE"),
	NULL
};

bool RTC::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool RTC::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < RTC_REGS_END) {
		m_regs[reg_num] = data & 0xf;
		return true;
	} else if (reg_num == 29) {
		// alarm on forcely
		m_alarm |= CLOCK_1HZ;
		update_alarm();
		return true;
	}
	return false;
}

void RTC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');

	UTILITY::tcscat(buffer, buffer_len, _T("RTC:\n"));
	for(int reg_num=0; reg_num<RTC_REGS_END; reg_num++) {
		if (reg_num == RTC_RESERVED1) continue;
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X(%-12s):%X"), reg_num, c_reg_names[reg_num], m_regs[reg_num]);
		if ((reg_num % 4) == 3) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
	UTILITY::tcscat(buffer, buffer_len, _T("\n * reg 1D(ALARM_ON_FORCE) is a command to alarm on forcely."));
}
#endif
