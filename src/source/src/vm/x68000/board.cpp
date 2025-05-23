/** @file board.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ main board ]
*/

#include "board.h"
//#include "../../emu.h"
#include "../vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../mc68000_consts.h"
#include "mfp.h"
#include "../../logging.h"

#ifdef _DEBUG
//#define OUT_DEBUG_IRQ(p, n, ...) if ((p | n) & 0xe0) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_IRQ(...)
//#define OUT_DEBUG_IACK(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_IACK(...)
//#define OUT_DEBUG_INT1 logging->out_debugf
#else
#define OUT_DEBUG_IRQ(...)
#define OUT_DEBUG_IACK(...)
#endif

void BOARD::initialize()
{
	wreset_register_id = -1;
	preset_register_id = -1;
	m_front_power = pConfig->now_power_off ? 0 : 1;

	register_frame_event(this);
}

void BOARD::reset()
{
	// clear all signals
	now_halt = 1; write_signal(SIG_CPU_HALT, 0, 1);
	now_irq  = 1; write_signal(SIG_CPU_IRQ,  0, 1);
	now_wreset = 0;
	m_now_fc = 0;
//	m_now_iack = false;
	m_now_irq_ack = 0;

	m_int1_mask = 0;
	m_int1_mask_s = 0;
	m_int1_status = 0x20;
	m_int1_vec_num = 0;

	m_int2_irq = 0;
	m_int4_irq = 0;

	m_vector = 0;
	if (pConfig->now_power_off) m_front_power = 0;

	cancel_my_event(wreset_register_id);
	cancel_my_event(preset_register_id);
	d_cpu->write_signal(SIG_CPU_HALT, 1, 1);
	d_cpu->write_signal(SIG_CPU_RESET, 1, 1);
	register_event(this, EVENT_BOARD_PRESET_RELEASE, 40000, false, &preset_register_id);
}

void BOARD::warm_reset(bool por)
{
	// clear all signals
	now_halt = 1; write_signal(SIG_CPU_HALT, 0, 1);
	now_irq  = 1; write_signal(SIG_CPU_IRQ,  0, 1);
	now_wreset = 0;
	m_now_fc = 0;
//	m_now_iack = false;
	m_now_irq_ack = 0;

	m_int1_mask = 0;
	m_int1_mask_s = 0;
	m_int1_status = 0x20;
	m_int1_vec_num = 0;

	m_int2_irq = 0;
	m_int4_irq = 0;

	m_vector = 0;
	if (pConfig->now_power_off) m_front_power = 0;
}

void BOARD::cancel_my_event(int &id)
{
	if (id >= 0) cancel_event(this, id);
	id = -1;
}

void BOARD::write_io8(uint32_t addr, uint32_t data)
{
	// $E9C000
	switch(addr & 1) {
	case 0:
		// interrupt mask
		m_int1_mask = (data & 0xff);

		m_int1_mask_s = ((m_int1_mask & 7) << 5) | ((m_int1_mask & 8) << 1);
		// update int1 interrupt
		now_irq = ((m_int1_status & m_int1_mask_s) ? (now_irq | 0x0002) : (now_irq & ~0x0002));

#ifdef OUT_DEBUG_INT1
		OUT_DEBUG_INT1(_T("clk:%d INT1 MASK CHANGED mask:%02X mask_s:%02X IRQ:%04X")
			, (int)get_current_clock()
			, m_int1_mask, m_int1_mask_s, now_irq);
#endif

		write_signals(&outputs_irq, now_irq);
		break;
	case 1:
		m_int1_vec_num = (data & ~3);
		break;
	}
}

/// now interrupt, send vector number
uint32_t BOARD::read_external_data8(uint32_t addr)
{
	// interrupt vector signal
	uint32_t data = m_vector;
	return data;
}

uint32_t BOARD::read_io8(uint32_t addr)
{
	// $E9C000
	uint32_t data = 0;
//	if (m_now_iack) {
//		// interrupt vector signal
//		data = m_vector;
//	} else {
		switch(addr & 1) {
		case 0:
			// interrupt mask
			data = m_int1_status | (m_int1_mask & 0xf);
			break;
		}
//	}
	return data;
}

void BOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint16_t prev = 0;
	switch (id) {
		case SIG_CPU_RESET:
			// RESET signal
			prev = now_wreset;
			now_wreset = ((data & mask) ? (now_wreset | mask) : (now_wreset & ~mask));
			if (prev == 0 && now_wreset != 0) {
				write_signals(&outputs_reset, 0xffffffff);
			} else if (prev != 0 && now_wreset == 0) {
//				write_signals(&outputs_reset, 0);
				// release RESET after 0.01 sec
				cancel_my_event(wreset_register_id);
				cancel_my_event(preset_register_id);
				register_event(this, EVENT_BOARD_WRESET_RELEASE, 10000, false, &wreset_register_id);
			}
//			logging->out_debugf(_T("BOARD: RESET: %X %X"), data, mask);
			break;
		case SIG_CPU_IRQ:
			prev = now_irq;
			switch (mask & 0xffff) {
			case 0x0002:
				{
				// int1 interrupt
#ifdef OUT_DEBUG_INT1
				uint32_t int1_prev = m_int1_status;
#endif

				uint32_t int1_mask = (mask >> 16);
				m_int1_status = (data & mask & 0xffff) ? (m_int1_status | int1_mask) : (m_int1_status & ~int1_mask);
				mask = (mask & 0xffff);
				now_irq = ((m_int1_status & m_int1_mask_s) ? (now_irq | mask) : (now_irq & ~mask));

#ifdef OUT_DEBUG_INT1
				OUT_DEBUG_INT1(_T("clk:%d INT1 prev:%02X now:%02X mask_s:%02X IRQ:%04X")
					, (int)get_current_clock()
					, int1_prev, m_int1_status, m_int1_mask_s, now_irq);
#endif
				}
				break;
			case 0x0004:
				{
				// int2 (wired or)
				uint32_t int2_mask = (mask >> 16);
				m_int2_irq = ((data & mask) ? (m_int2_irq | int2_mask) : (m_int2_irq & ~int2_mask));
				mask &= 0xffff;
				now_irq = ((m_int2_irq != 0) ? (now_irq | mask) : (now_irq & ~mask));
				}
				break;
			case 0x0010:
				{
				// int4 (wired or)
				uint32_t int4_mask = (mask >> 16);
				m_int4_irq = ((data & mask) ? (m_int4_irq | int4_mask) : (m_int4_irq & ~int4_mask));
				mask &= 0xffff;
				now_irq = ((m_int4_irq != 0) ? (now_irq | mask) : (now_irq & ~mask));
				}
				break;
			default:
				now_irq = ((data & mask) ? (now_irq | mask) : (now_irq & ~mask));
				break;
			}
			if (prev == 0 && now_irq != 0) {
				// off -> on
				write_signals(&outputs_irq, now_irq);
				OUT_DEBUG_IRQ(prev, now_irq, _T("clk:%llu IRQ ON prev:%04X now:%04X"), get_current_clock(), prev, now_irq);
			} else if (prev > now_irq) {
				// priority down
				write_signals(&outputs_irq, now_irq);
				OUT_DEBUG_IRQ(prev, now_irq, _T("clk:%llu IRQ DOWN prev:%04X now:%04X"), get_current_clock(), prev, now_irq);
			} else if (prev < now_irq) {
				// priority up
				write_signals(&outputs_irq, now_irq);
				OUT_DEBUG_IRQ(prev, now_irq, _T("clk:%llu IRQ UP prev:%04X now:%04X"), get_current_clock(), prev, now_irq);
//			} else {
//				// keep interrupt
//				OUT_DEBUG_IRQ(prev, now_irq, _T("clk:%llu IRQ KEEP prev:%04X now:%04X"), get_current_clock(), prev, now_irq);
			}
			break;
		case SIG_CPU_HALT:
			prev = now_halt;
			now_halt = ((data & mask) ? (now_halt | mask) : (now_halt & ~mask));
			if (prev == 0 && now_halt != 0) {
				write_signals(&outputs_halt, now_halt);
			} else if (prev != 0 && now_halt == 0) {
				write_signals(&outputs_halt, now_halt);
			}
//			logging->out_debugf(_T("BOARD: HALT: %X %X"), data, mask);
			break;

		case SIG_BOARD_POWER:
			// front power switch
			BIT_ONOFF(m_front_power, 1, data & mask);
			write_signals(&outputs_power, data & mask ? 0xffffffff : 0);
			break;

		case SIG_M68K_FC:
			if (data == 7) {
				// interrupt
				if (now_irq & 0x80) {
					// NMI
					d_cpu->write_signal(SIG_M68K_VPA_AVEC, 1, 1);
				} else {
//					uint16_t bit_irq = 0x40;
//					for(int n=6; n>=2; n--) {
//						if (now_irq & bit_irq) {
//							write_signals(&outputs_iack[n], 1);
//						}
//						bit_irq >>= 1;
//					}

					if (now_irq & 0x02) {
//						m_now_iack = true;
						for(int i=0; i<4; i++) {
							if (m_int1_status & m_int1_mask_s & c_int1_priority[i]) {
								m_vector = m_int1_vec_num | (i & 3);
								break;
							}
						}
					}

					OUT_DEBUG_IACK(_T("clk:%lld IRQ ACK ON  now:%04X")
						, get_current_clock(), now_irq);
				}
				m_now_irq_ack = now_irq;

			} else if (m_now_fc == 7) {
				if (m_now_irq_ack & 0x80) {
					// NMI
					d_cpu->write_signal(SIG_M68K_VPA_AVEC, 0, 1);
				} else {
//					uint16_t bit_irq = 0x40;
//					for(int n=6; n>=2; n--) {
//						if (m_now_irq_ack & bit_irq) {
//							write_signals(&outputs_iack[n], 0);
//							m_now_irq_ack &= ~bit_irq;
//						}
//						bit_irq >>= 1;
//					}

//					if (m_now_irq_ack & 0x02) {
//						m_now_iack = false;
//						m_now_irq_ack &= ~0x02;
//					}

					OUT_DEBUG_IACK(_T("clk:%lld IRQ ACK OFF now:%04X")
						, get_current_clock(), m_now_irq_ack);
				}
				m_now_irq_ack = now_irq;

			}
			m_now_fc = data;
			break;
	}
}

//
uint32_t BOARD::get_intr_ack()
{
	return m_now_fc;
}

//                                        FDC   FDD   HDD   Printer 
const uint8_t BOARD::c_int1_priority[] = {0x80, 0x40, 0x10, 0x20};

uint32_t BOARD::read_signal(int id)
{
	switch(id) {
	case SIG_BOARD_POWER:
		// front power switch
		return get_front_power_on();
		break;
	}
	return 0xffffff;
}

void BOARD::event_callback(int event_id, int err)
{
	if (event_id == EVENT_BOARD_WRESET_RELEASE) {
		write_signals(&outputs_reset, 0);
		warm_reset(false);
		wreset_register_id = -1;
	}
	else if (event_id == EVENT_BOARD_PRESET_RELEASE) {
		d_cpu->write_signal(SIG_CPU_RESET, 0, 1);
		d_cpu->write_signal(SIG_CPU_HALT, 0, 1);
		preset_register_id = -1;
	}
}

void BOARD::event_frame()
{
//	m_front_power += 2;
//	m_front_power &= 0x3f;
}

uint32_t BOARD::get_front_power_on() const
{
	return (m_front_power & 1);
}

/// b09: power led green
/// b10: power led red
uint32_t BOARD::get_led_status()
{
	uint32_t status = 0;
	status = (pConfig->now_power_off ? 0x400 : (m_front_power & 1 ? 0x200 : 0));
	return status;
}

// ----------------------------------------------------------------------------

uint32_t BOARD::asserted_int2_devices() const
{
	return (uint32_t)m_int2_irq << 16;
}

uint32_t BOARD::asserted_int4_devices() const
{
	return (uint32_t)m_int4_irq << 16;
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)

void BOARD::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	memset(&vm_state, 0, sizeof(vm_state));

	// save config flags
	BIT_ONOFF(vm_state.flags, 1, pConfig->sync_irq);
	BIT_ONOFF(vm_state.flags, 4, pConfig->use_power_off);
	BIT_ONOFF(vm_state.flags, 8, pConfig->now_power_off);
	vm_state.flags = Uint32_LE(vm_state.flags);

	SET_Uint16_LE(now_halt);
	SET_Uint16_LE(now_irq);
	SET_Uint16_LE(now_wreset);
	SET_Uint16_LE(m_now_irq_ack);

	SET_Byte(m_now_fc);			///< function code on CPU
//	SET_Bool(m_now_iack);		///< receiving IACK
	SET_Byte(m_int1_mask);		///< int1 mask
	SET_Byte(m_int1_mask_s);	///< int1 mask swap flags 0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	SET_Byte(m_int1_status);	///< int1 flags  0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	SET_Byte(m_int1_vec_num);	///< int1 vector bit0&1 are 0:FDC 1:FDD 2:HDD 3:Printer
	SET_Byte(m_vector);			///< int1 vector number occuring interrupt
	SET_Byte(m_front_power);	///< front power button and power led (b0: power sw on = 1)

	SET_Int32_LE(wreset_register_id);	// normal reset
	SET_Int32_LE(preset_register_id);	// power on reset

	SET_Uint16_LE(m_int2_irq);
	SET_Uint16_LE(m_int4_irq);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)

bool BOARD::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// load config flags
	vm_state.flags = Uint32_LE(vm_state.flags);
	pConfig->sync_irq = ((vm_state.flags & 1) != 0);
	pConfig->use_power_off = ((vm_state.flags & 4) != 0);
	pConfig->now_power_off = ((vm_state.flags & 8) != 0);

	GET_Uint16_LE(now_halt);
	GET_Uint16_LE(now_irq);
	GET_Uint16_LE(now_wreset);
	GET_Uint16_LE(m_now_irq_ack);

	GET_Byte(m_now_fc);			///< function code on CPU
//	GET_Bool(m_now_iack);		///< receiving IACK
	GET_Byte(m_int1_mask);		///< int1 mask
	GET_Byte(m_int1_mask_s);	///< int1 mask swap flags 0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	GET_Byte(m_int1_status);	///< int1 flags  0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	GET_Byte(m_int1_vec_num);	///< int1 vector bit0&1 are 0:FDC 1:FDD 2:HDD 3:Printer
	GET_Byte(m_vector);			///< int1 vector number occuring interrupt
	GET_Byte(m_front_power);	///< front power button and power led (b0: power sw on = 1)

	GET_Int32_LE(wreset_register_id);	// normal reset
	GET_Int32_LE(preset_register_id);	// power on reset

	GET_Uint16_LE(m_int2_irq);
	GET_Uint16_LE(m_int4_irq);

	if (vm_state_i.version ==  Uint16_LE(1)) {
		if (m_now_fc == 7) {
			m_now_irq_ack = now_irq;
		}
	}

	vm->set_pause(3, pConfig->now_power_off);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
bool BOARD::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool BOARD::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	uint32_t prev = 0;
	switch(reg_num) {
		case WREG_RESET:
			prev = now_wreset;
			now_wreset = data;
			if (prev == 0 && now_wreset != 0) {
				write_signals(&outputs_reset, 0xffffffff);
			} else if (prev != 0 && now_wreset == 0) {
				// release RESET after 0.01 sec
				cancel_my_event(wreset_register_id);
				cancel_my_event(preset_register_id);
				register_event(this, EVENT_BOARD_WRESET_RELEASE, 10000, false, &wreset_register_id);
			}
			return true;
		case WREG_IRQ:
			prev = now_irq;
			now_irq = data;
			if (prev == 0 && now_irq != 0) {
				write_signals(&outputs_irq, now_irq);
			} else if (prev != 0 && now_irq == 0) {
				write_signals(&outputs_irq, now_irq);
			}
			return true;
		case WREG_HALT:
			prev = now_halt;
			now_halt = data;
			if (prev == 0 && now_halt != 0) {
				write_signals(&outputs_halt, now_halt);
			} else if (prev != 0 && now_halt == 0) {
				write_signals(&outputs_halt, now_halt);
			}
			return true;
		case WREG_INT1MASK:
			m_int1_mask = data;
			return true;
		case WREG_INT1STAT:
			m_int1_status = data;
			return true;
		case WREG_INT1VECNUM:
			m_int1_vec_num = data;
			return true;
	}
	return false;
}

static const struct st_int_flags {
	uint32_t val;
	const _TCHAR *name;
} c_int_flags[] = {
	{ INTDEV_HDD,		_T("HDD")	},
	{ INTDEV_PRINTER,	_T("PRN")	},
	{ INTDEV_FDD,		_T("FDD")	},
	{ INTDEV_FDC,		_T("FDC")	},
	{ INTDEV_MIDI,		_T("MIDI")	},
	{ 0, NULL }
};

static void print_int_flags(uint32_t flags, _TCHAR *buffer, size_t buffer_len)
{
	int cnt = 0;
	UTILITY::tcscat(buffer, buffer_len, _T("("));
	for(int i=0; c_int_flags[i].val != 0; i++) {
		if (flags & c_int_flags[i].val) {
			if (cnt) {
				UTILITY::tcscat(buffer, buffer_len, _T("|"));
			}
			UTILITY::sntprintf(buffer, buffer_len, c_int_flags[i].name);
			cnt++;
		}
	}
	UTILITY::tcscat(buffer, buffer_len, _T(")"));
}

void BOARD::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("INTERRUPT CONTROL:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), WREG_RESET, _T("RESET"), now_wreset);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X"), WREG_IRQ, _T("IRQ  "), now_irq);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%s):%04X\n"), WREG_HALT, _T("HALT "), now_halt);

	UTILITY::sntprintf(buffer, buffer_len, _T(" %X($E9C001 W:INT1MASK):%02X\n"), WREG_INT1MASK, m_int1_mask);
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X($E9C001 R:INT1STAT):%02X "), WREG_INT1STAT, m_int1_status);
	print_int_flags((uint32_t)m_int1_status << 16, buffer, buffer_len);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" %X($E9C003 W:INT1VECNUM):%02X\n"), WREG_INT1VECNUM, m_int1_vec_num);

	UTILITY::sntprintf(buffer, buffer_len, _T(" INT2IRQ:%04X "), m_int2_irq);
	print_int_flags(m_int2_irq << 16, buffer, buffer_len);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" INT4IRQ:%04X "), m_int4_irq);
	print_int_flags(m_int4_irq << 16, buffer, buffer_len);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
}
#endif
