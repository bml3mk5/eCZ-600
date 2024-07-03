/** @file midi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.06 -

	@brief [ MCS modoki (YM3802) ]
*/

#include "midi.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"

#ifdef _DEBUG
//#define OUT_DEBUG_INTR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_INTR(...)
//#define OUT_DEBUG_REGW(n, ...) if (n != 1) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_REGR(n, ...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGR(...)
//#define USE_OUT_DEBUG_REGR
#else
#define OUT_DEBUG_INTR(...)
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_REGR(...)
#endif

#define CLOCK_BASE  4000000.0
#define CLOCKM_BASE 1000000.0
#define CLOCKF_BASE  614400.0


// ----------------------------------------------------------------------------

void MIDI::initialize()
{
	for(int i=0; i<EVENT_MIDI_END; i++) {
		m_register_id[i] = -1;
	}

	m_tx_buffer.allocate(16);
	m_tx_buffer.clear();
	m_itx_buffer.allocate(4);
	m_itx_buffer.clear();

	m_xmit_lamda = 32.0;
	m_xmit_wait_period = 320.0;

	m_rx_buffer.allocate(128);
	m_rx_buffer.clear();
	m_irx_buffer.allocate(4);
	m_irx_buffer.clear();

	m_recv_lamda = 32.0;
	m_recv_wait_period = 320.0;


	m_click_counter = 0;
	m_record_counter = 0;
	m_playback_counter = 0;
	m_general_counter = 0;
	m_midi_counter = 0;

	m_click_counter_base = 0;
	m_playback_counter_base = 0;
	m_general_counter_base = 0;
	m_midi_counter_base = 0;

//	m_req_load_midi_counter = false;
	m_idle_counter = 0;
	m_offline_counter = 0;

	memset(m_ad_buffer, 0, sizeof(m_ad_buffer));
	m_ad_buffer_count = 0;
	m_ad_match_id = 0;

	m_interpolate_counter = 0;
	m_interpolate_deltas_idx = 0;
	m_interpolate_deltas_cnt = 0;
	memset(m_interpolate_deltas, 0, sizeof(m_interpolate_deltas));

	m_sequencer_state = 0;

	m_clkm_period = 0.0;
}

void MIDI::reset()
{
	warm_reset(true);
}

void MIDI::warm_reset(bool por)
{
	for(int i=0; i<REGS_END; i++) {
		m_regs[i] = 0;
	}

	m_tx_buffer.clear();
	m_itx_buffer.clear();

	m_xmit_lamda = 32.0;
	m_xmit_wait_period = 0.0;

	m_rx_buffer.clear();
	m_irx_buffer.clear();

	m_recv_lamda = 32.0;
	m_recv_wait_period = 320.0;


	m_click_counter = 0;
	m_record_counter = 0;
	m_playback_counter = 0;
	m_general_counter = 0;
	m_midi_counter = 0;

	m_click_counter_base = 0;
	m_playback_counter_base = 0;
	m_general_counter_base = 0;
	m_midi_counter_base = 0;

	memset(m_ad_buffer, 0, sizeof(m_ad_buffer));
	m_ad_buffer_count = 0;
	m_ad_match_id = 0;

//	m_req_load_midi_counter = false;
	m_idle_counter = 0;
	m_offline_counter = 0;

	m_interpolate_counter = 0;
	m_interpolate_deltas_idx = 0;
	m_interpolate_deltas_cnt = 0;
	memset(m_interpolate_deltas, 0, sizeof(m_interpolate_deltas));

	m_sequencer_state = 0;

	if (por) {
		m_register_id[EVENT_MIDI_TXD] = -1;
		m_register_id[EVENT_MIDI_RXD] = -1;
	} else {
		cancel_my_event(EVENT_MIDI_TXD);
		cancel_my_event(EVENT_MIDI_RXD);
	}

	// regist loop clock timer
	if (FLG_IOPORT_MIDI != 0 && !pConfig->now_power_off) {
		// enable MIDI board
		calc_clkm_timer();
		calc_clkf_timer();
	} else {
		// disable MIDI board
		cancel_my_event(EVENT_MIDI_CLKM);
		cancel_my_event(EVENT_MIDI_CLKF);
	}

	write_signals(&outputs_irq, 0);
	write_signals(&outputs_exo, 0);
}

// ----------------------------------------------------------------------------

void MIDI::register_my_event(int id, double usec, bool loop)
{
	register_event(this, id, usec, loop, &m_register_id[id]);
}

void MIDI::cancel_my_event(int id)
{
	if (m_register_id[id] != -1) cancel_event(this, m_register_id[id]);
	m_register_id[id] = -1;
}

// ----------------------------------------------------------------------------

void MIDI::write_io8(uint32_t addr, uint32_t data)
{
	if (now_reset) return;

	addr &= 0x07;

	if(addr < 4) {
		OUT_DEBUG_REGW(addr, _T("YM3802 REGW R%02d data:%02X"), addr, data);

		switch(addr) {
		case MCS_RGR:
			// group number
			m_regs[REG_RGR] = (data & RGR_GRP);
			if (data & RGR_IC) {
				// now reset
				warm_reset(false);
			}
			break;
		case MCS_ICR:
			// clear interrupt
			clear_irq(data & 0xff);
			break;
		default:
			// cannot write
			m_regs[REG_NUL] = (data & 0xff);
			break;
		}
	} else {
		uint32_t bank = (m_regs[REG_RGR] & 0x0f);
		if (bank > 9) {
			return;
		}
		addr = bank * 10 + addr;

		OUT_DEBUG_REGW(addr, _T("YM3802 REGW R%02d data:%02X"), addr, data);

		switch(addr) {
		case MCS_IOR:
			// IRQ Vector Offset
			m_regs[REG_IOR] = (data & IOR_MASK);
			break;
		case MCS_IMR:
			// IRQ Control
			m_regs[REG_IMR] = (data & IMR_MASK);
			break;
		case MCS_IER:
			// IRQ Enable Control
			m_regs[REG_IER] = (data & IER_MASK);
			update_irq();
			break;
		case MCS_DMR:
			// Real Time Message Control
			m_regs[REG_DMR] = (data & DMR_MASK);
			break;
		case MCS_DCR:
			// Real Time Message Request
			m_regs[REG_DCR] = (data & DCR_MASK);
			midi_dist_send_message(data | 0xf8);
			break;
		case MCS_DNR:
			// FIFO IRx Control
//			m_regs[REG_DNR] = (data & DNR_MASK);
			if (data & DNR_CLK) {
				// the oldest data is lost
				m_irx_buffer.read();
			}
			break;
		case MCS_RRR:
			// FIFO Rx Rate
			m_regs[REG_RRR] = (data & RRR_MASK);
			calc_recv_rate();
			break;
		case MCS_RMR:
			// FIFO Rx Mode
			m_regs[REG_RMR] = (data & RMR_MASK);
			calc_recv_rate();
			break;
		case MCS_AMR:
			// Address-hunter Control 1
			m_regs[REG_AMR] = (data & AMR_MASK);
			break;
		case MCS_ADR:
			// Address-hunter Control 2
			m_regs[REG_ADR] = (data & ADR_MASK);
			break;
		case MCS_RCR:
			// FIFO Rx Control
			m_regs[REG_RCR] = (data & RCR_MASK & ~(RCR_OLC | RCR_BRKC | RCR_OVC | RCR_CLE));
			// clear off-line flag
			// clear break detected flag
			// clear overflow flag
			m_regs[REG_RSR] &= ~(data & (RCR_OLC | RCR_BRKC | RCR_OVC));
			if (
			   ((data & RCR_BRKC) != 0 && (m_regs[REG_IMR] & IMR_OB) != 0)
			|| ((data & RCR_OLC) != 0 && (m_regs[REG_IMR] & IMR_OB) == 0)
			) {
				// clear IRQ
				m_regs[REG_ISR] &= ~ISR_OL;
				update_irq();
			}
			if (m_regs[REG_RCR] & RCR_CLE) {
				// clear buffer
				clear_recv();
 			}
			if (m_regs[REG_RCR] & RCR_ENA) {
				// start
				start_recv();
			} else {
				// stop
				stop_recv();
			}
			break;
		case MCS_TRR:
			// FIFO Tx Rate
			m_regs[REG_TRR] = (data & TRR_MASK);
			calc_xmit_rate();
			break;
		case MCS_TMR:
			// FIFO Tx Mode
			m_regs[REG_TMR] = (data & TMR_MASK);
			calc_xmit_rate();
			break;
		case MCS_TCR:
			// FIFO Tx Control
			m_regs[REG_TCR] = (data & TCR_MASK & ~(TCR_IDLC | TCR_CLE));
			// clear idle flag
			m_regs[REG_TSR] &= ~(m_regs[REG_TCR] & TCR_IDLC);

			if (m_regs[REG_TCR] & TCR_BRKE) {
				// TODO: send break
 			}
			if (m_regs[REG_TCR] & TCR_CLE) {
				// clear buffer
				clear_xmit();
 			}
			if (m_regs[REG_TCR] & TCR_ENA) {
				// start
				start_xmit();
			} else {
				// stop
				stop_xmit();
			}
			break;
		case MCS_TDR:
			// FIFO Tx Data
			push_to_fifo_tx(data & 0xff);
			if (m_regs[REG_TCR] & TCR_ENA) {
				// start
				start_xmit();
			}
			break;
		case MCS_FCR:
			// FSK Control
			m_regs[REG_FCR] = (data & FCR_MASK & ~(FCR_PDFC | FCR_CFC));
			// clear flags
			m_regs[REG_FSR] &= ~(data & FCR_PDFC);
			if (data & FCR_CFC) {
				m_regs[REG_FSR] &= ~(FSR_CFF | FSR_CSF);
			}
			break;
		case MCS_CCR:
			// Click Counter Control
			m_regs[REG_CCR] = (data & CCR_MASK);
			calc_clkm_timer();
			break;
		case MCS_CDR:
			// Click Counter Load Value
			m_regs[REG_CDR] = (data & CDR_DATA);
			m_click_counter_base = m_regs[REG_CDR];
			if (data & CDR_LD) {
				// set value
				m_click_counter = m_click_counter_base;
			}
			break;
		case MCS_SCR:
			// Sequencer Control
			m_regs[REG_SCR] = (data & SCR_MASK & ~(SCR_CLR | SCR_ADD));
			if (data & SCR_CLR) {
				// clear play-back counter
				m_playback_counter = 0;
			}
			if (data & SCR_ADD) {
				// add play-back counter
				m_playback_counter_base = ((uint32_t)m_regs[REG_SPRH] << 8) | m_regs[REG_SPRL];
				m_playback_counter += m_playback_counter_base;
			}
			break;
		case MCS_SPRL:
			// Play-back Counter Value L
			m_regs[REG_SPRL] = (data & SPRL_DATA);
			break;
		case MCS_SPRH:
			// Play-back Counter Value H
			m_regs[REG_SPRH] = (data & SPRH_DATA);
			break;
		case MCS_GTRL:
			// General Timer Value L
			m_regs[REG_GTRL] = (data & GTRL_DATA);
			break;
		case MCS_GTRH:
			// General Timer Value H
			m_regs[REG_GTRH] = (data & GTRH_DATA);
			// for reload value
			m_general_counter_base = ((uint32_t)m_regs[REG_GTRH] << 8) | m_regs[REG_GTRL];
			if (data & GTRH_LD) {
				// set value
				m_general_counter = m_general_counter_base;
			}
			break;
		case MCS_MTRL:
			// MIDI-clock Timer Value L
			m_regs[REG_MTRL] = (data & MTRL_DATA);
			break;
		case MCS_MTRH:
			// MIDI-clock Timer Value H
			m_regs[REG_MTRH] = (data & MTRH_DATA);
			// for reload value
			m_midi_counter_base = ((uint32_t)m_regs[REG_MTRH] << 8) | m_regs[REG_MTRL];
			if (data & MTRH_LD) {
				// set value requset
				// counter will reload the next event
				m_midi_counter = 1;
			}
			break;
		case MCS_EDR:
			// External I/O Direction
			m_regs[REG_EDR] = (data & 0xff);
			break;
		case MCS_EOR:
			// External Oouput Data
			m_regs[REG_EOR] = (data & m_regs[REG_EDR]);
			write_signals(&outputs_exo, m_regs[REG_EOR]);
			break;
		default:
			// cannot write
			m_regs[REG_NUL] = (data & 0xff);
			break;
		}
	}
}

/// now interrupt, send vector number
///
/// @note On X68000, read R00 using RD pin instead VR pin.
uint32_t MIDI::read_external_data8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

#ifdef USE_VECTOR_READ 
	if (!(m_regs[REG_IMR] & IMR_VE)) return data;

	if (!(m_regs[REG_IMR] & IMR_VM) && !(m_regs[REG_ISR] & m_regs[REG_IER])) return data;
#endif

	// interrupt vector
	data = read_irq_vector();

	OUT_DEBUG_INTR(_T("clk:%lld YM3802 Vector Read vec:%02X ISR:%02X IER:%02X => %02X")
		, get_current_clock()
		, data
		, m_regs[REG_ISR], m_regs[REG_IER]
		, m_regs[REG_ISR] & m_regs[REG_IER]);

	return data;
}

uint32_t MIDI::read_io8(uint32_t addr)
{
	uint32_t data = INVALID_VALUE;

#ifdef USE_OUT_DEBUG_REGR
	static uint32_t prev_addr = 0xffffffff;
	static uint32_t prev_data = 0xffffffff;
#endif

	if (now_reset) return data;

	addr &= 0x07;

	if(addr < 4) {
		switch(addr) {
		case MCS_IVR:
			data = read_irq_vector();
			break;
		case MCS_ISR:
			data = m_regs[REG_ISR];
			break;
		default:
			// cannot read
//			data = m_regs[REG_NUL];
			break;
		}
#ifdef USE_OUT_DEBUG_REGR
		if (addr != prev_addr || data != prev_data) {
			OUT_DEBUG_REGR(addr, _T("YM3802 REGR R%02d data:%02X"), addr, data);
			prev_addr = addr;
			prev_data = data;
		}
#endif

	} else {
		uint32_t bank = (m_regs[REG_RGR] & 0x0f);
		
		addr = bank * 10 + addr;

		switch(addr) {
		case MCS_DSR:
			// FIFO-IRx Data
			data = m_irx_buffer.read_not_remove(0);
			break;
		case MCS_RSR:
			// FIFO-Rx Status
			data = m_regs[REG_RSR];
			data |= ((m_rx_buffer.read_not_remove(0) >> 4) & (RSR_FE | RSR_PE));
			if (!m_rx_buffer.empty()) data |= RSR_RDY;
			break;
		case MCS_RDR:
			// FIFO-Rx Data
			data = m_rx_buffer.read();
			if (m_rx_buffer.empty()) {
				m_regs[REG_ISR] &= ~ISR_RX;
				update_irq();
			}
			break;
		case MCS_TSR:
			// FIFO-Tx Status
			data = m_regs[REG_TSR];
			if (m_tx_buffer.empty()) data |= TSR_EMP;
			if (!m_tx_buffer.full()) data |= TSR_RDY;
			break;
		case MCS_FSR:
			// FSK Status
			data = m_regs[REG_FSR];
			break;
		case MCS_SRR:
			// Recording Counter Current Value
			data = m_record_counter;
			break;
		case MCS_EIR:
			// External Input Data
			data = (m_regs[REG_EIR] & ~m_regs[REG_EDR]);
			break;
		default:
			// cannot read
//			data = m_regs[REG_NUL];
			break;
		}
#ifdef USE_OUT_DEBUG_REGR
		if (addr != prev_addr || data != prev_data) {
			OUT_DEBUG_REGR(addr, _T("YM3802 REGR R%02d data:%02X"), addr, data);
			prev_addr = addr;
			prev_data = data;
		}
#endif

	}

	return data;
}

void MIDI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
	case SIG_EXI:
		m_regs[REG_EIR] = (data & mask & 0xff);
		break;
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		if (!now_reset) {
			warm_reset(false);
		}
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void MIDI::calc_xmit_rate()
{
	// bitrate
	if (m_regs[REG_TRR] & TRR_CLK) {
		// use CLKF
		if (m_regs[REG_TRR] & TRR_RF32) {
			m_xmit_lamda = 1000000.0 * (64 * (1 << (m_regs[REG_TRR] & TRR_RFSL))) / CLOCKF_BASE;
		} else {
			m_xmit_lamda = 1000000.0 * 32 / CLOCKF_BASE;
		}

	} else {
		// use CLKM
		m_xmit_lamda = (m_regs[REG_TRR] & TRR_RM32) ? 1000000.0 * 32 / CLOCKM_BASE : 1000000.0 * 16 / CLOCKM_BASE;
	}

	// bits per a data
	int bit_len = 1; // start bit
	bit_len += (m_regs[REG_TMR] & TMR_CL) ? 7 : 8;	// data bits
	if (m_regs[REG_TMR] & TMR_PE) {
		bit_len += (m_regs[REG_TMR] & TMR_PL) ? 4 : 1;	// parity bit
	}
	bit_len += (m_regs[REG_TMR] & TMR_SL) ? 2 : 1;	// stop bit

	m_xmit_wait_period = m_xmit_lamda * bit_len;
}

#if 0
void MIDI::empty_xmit()
{
	// empty interrupt
	if (m_tx_buffer.empty()) {
		// set empty
		m_regs[REG_ISR] |= ISR_TX;
		// interrupt
		update_irq();
	} else {
		// clear interrupt
		update_irq();
	}
}
#endif

void MIDI::start_xmit()
{
	if (m_register_id[EVENT_MIDI_TXD] >= 0) {
		// now transmitting
		return;
	}

	// send data
	xmit();
}

void MIDI::stop_xmit()
{
// if now transmitting, event will fire and call xmit()
//
//	cancel_my_event(EVENT_MIDI_TXD);
//
//	m_regs[REG_TSR] &= ~TSR_BSY;
}

void MIDI::clear_xmit()
{
	m_tx_buffer.clear();
	m_itx_buffer.clear();
	// IRQ is not generated
//	// set empty
//	m_regs[REG_ISR] |= ISR_TX;
//	// interrupt
//	update_irq();
}

void MIDI::xmit()
{
	if ((m_tx_buffer.empty() && m_itx_buffer.empty()) || (m_regs[REG_TCR] & TCR_ENA) == 0) {
		// no data buffered
		// transmitter is disabled
		m_regs[REG_TSR] &= ~TSR_BSY;
		return;
	}

	m_regs[REG_TSR] |= TSR_BSY;

	// send data
	uint8_t data;
	if (!m_itx_buffer.empty()) {
		data = m_itx_buffer.read();
	} else {
		data = m_tx_buffer.read();
	}
	if (m_regs[REG_TRR] & TRR_D_F) {
		// TODO: TxF not supported
	} else {
		// send TxD
		emu->send_data_to_midiout(data);
	}
	m_idle_counter = 0;

	// empty interrupt
	if (m_tx_buffer.empty()) {
		// set empty
		m_regs[REG_ISR] |= ISR_TX;
		// interrupt
		update_irq();
	}

	// wait a time transmitting process
	register_my_event(EVENT_MIDI_TXD, m_xmit_wait_period, false);
}

/// @brief Push a data to FIFO-Tx
///
/// @param[in] data
void MIDI::push_to_fifo_tx(uint8_t data)
{
	if (!m_tx_buffer.full()) {
		m_tx_buffer.write(data);
		// IRQ
		m_regs[REG_ISR] &= ~ISR_TX; 
		update_irq();
	}
	// if buffer is full then drop the data
}

#if 0
uint8_t MIDI::pop_from_fifo_tx()
{
	uint8_t data = m_tx_buffer.read();
	// IRQ
	if (m_tx_buffer.empty()) {
		m_regs[REG_ISR] |= ISR_TX; 
		update_irq();
	}
}
#endif

// ----------------------------------------------------------------------------

void MIDI::calc_recv_rate()
{
	// bitrate
	if (m_regs[REG_RRR] & RRR_CLK) {
		// use CLKF
		if (m_regs[REG_RRR] & RRR_RF32) {
			m_recv_lamda = 1000000.0 * (64 * (1 << (m_regs[REG_RRR] & RRR_RFSL))) / CLOCKF_BASE;
		} else {
			m_recv_lamda = 1000000.0 * 32 / CLOCKF_BASE;
		}

	} else {
		// use CLKM
		m_recv_lamda = (m_regs[REG_RRR] & RRR_RM32) ? 1000000.0 * 32 / CLOCKM_BASE : 1000000.0 * 16 / CLOCKM_BASE;
	}

	// bits per a data
	int bit_len = 1; // start bit
	bit_len += (m_regs[REG_RMR] & RMR_CL) ? 7 : 8;	// data bits
	if (m_regs[REG_RMR] & RMR_PE) {
		bit_len += (m_regs[REG_RMR] & RMR_PL) ? 4 : 1;	// parity bit
	}
	bit_len += (m_regs[REG_RMR] & RMR_SL) ? 2 : 1;	// stop bit

	m_recv_wait_period = m_recv_lamda * bit_len;
}

void MIDI::start_recv()
{
	m_regs[REG_RSR] |= RSR_BSY;
}

void MIDI::stop_recv()
{
	m_regs[REG_RSR] &= ~RSR_BSY;
}

void MIDI::clear_recv()
{
	m_rx_buffer.clear();
	m_regs[REG_ISR] &= ~ISR_RX;
	update_irq();
}

/// @brief Receiver
///
/// @param[in] data :
///  b7 - b0 : received data
///  b8      : parity error flag (bit position is diffrent from YM3802 application manual!)
///  b9      : flaming error flag (bit position is diffrent from YM3802 application manual!)
///  b13 - b10 : parity error on 4-bit parity mode (unsupported)
void MIDI::recv(uint16_t data)
{
	// receiver enable ?
	if (!(m_regs[REG_RCR] & RCR_ENA)) return;

	// offline detector
	m_offline_counter = 0;

	if (data >= 0x100) {
		// error data
		push_to_fifo_rx(data);

	} else if (data == MIDI_MSG_CLOCK) {
		// MIDI-clock  real-time message
		// store to FIFO-Rx if need
		if (m_regs[REG_RCR] & RCR_FLTE) {
			push_to_fifo_rx(data);
		}
		// store to FIFO-IRx
		if ((m_regs[REG_DMR] & DMR_MCFS) == DMR_MCFS_MESSAGE) {
			m_irx_buffer.write(data & 0xff);
			// check data
			midi_dist_check();
		}

	} else if (data > MIDI_MSG_CLOCK) {
		// MIDI real-time message
		// store to FIFO-Rx
		push_to_fifo_rx(data);
		// store to FIFO-IRx if need
		if (m_regs[REG_DMR] & DMR_CDE) {
			m_irx_buffer.write(data & 0xff);
			// check data
			midi_dist_check();
		}

	} else if (m_regs[REG_RSR] & RSR_ABSY) {
		// address hunter check mode
		if (data & 0x80) {
			if (data != MIDI_EXCLUSIVE_START) {
				// stop address hunter mode
				m_regs[REG_RSR] &= ~RSR_ABSY;
				// next status
				push_to_fifo_rx(data);
			} else {
				// next status is also exclusive message
				m_ad_buffer_count = 0;
				m_ad_buffer[m_ad_buffer_count++]=(data & 0xff);
				m_ad_match_id = 0;
			}

		} else {
			// check ID
			switch (m_ad_buffer_count) {
			case 1:
				// check make ID
				if (data == (m_regs[REG_AMR] & AMR_MAKER_ID)) {
					m_ad_match_id |= 1;
				}
				m_ad_buffer[m_ad_buffer_count++]=(data & 0xff);
				break;
			case 2:
				// check device ID if need
				if (m_regs[REG_AMR] & AMR_IDCL) {
					if ((data == (m_regs[REG_ADR] & ADR_DEVICE_ID))
					|| ((m_regs[REG_ADR] & ADR_BDRE) != 0 && data == 0x7f)) {
						m_ad_match_id |= 2;
					} else {
						m_ad_match_id = 0;
					}
				}
				m_ad_buffer[m_ad_buffer_count++]=(data & 0xff);
				if (m_ad_match_id) {
					// send to FIFO-Rx
					for(int i=0; i<m_ad_buffer_count; i++) {
						push_to_fifo_rx(m_ad_buffer[i]);
					}
				}
				break;
			default:
				if (m_ad_match_id) {
					push_to_fifo_rx(data);
				}
				break;
			}
		}

	} else {
		// address hunter is enable and system exclusive message received 
		if ((m_regs[REG_RCR] & RCR_AHE) != 0 && data == MIDI_EXCLUSIVE_START) {
			// go address hunter mode
			m_regs[REG_RSR] |= RSR_ABSY;
			m_ad_buffer_count = 0;
			m_ad_buffer[m_ad_buffer_count++]=(data & 0xff);
			m_ad_match_id = 0;
		}
	}
	// IRQ
	if (!m_rx_buffer.empty()) {
		m_regs[REG_ISR] |= ISR_RX;
		update_irq();
	}
}

/// @brief Push a data to FIFO-Rx
///
/// @param[in] data :
///  b7 - b0 : received data
///  b8      : parity error flag (bit position is diffrent from YM3802 application manual!)
///  b9      : flaming error flag (bit position is diffrent from YM3802 application manual!)
///  b13 - b10 : parity error on 4-bit parity mode (unsupported)
void MIDI::push_to_fifo_rx(uint16_t data)
{
	if (!m_rx_buffer.full()) {
		m_rx_buffer.write(data);
		// IRQ
		m_regs[REG_ISR] |= ISR_RX;
		update_irq();
	} else {
		// full
		// drop 2nd older item
		// and push the data
		uint16_t oldest = m_rx_buffer.read();
		int rpos = m_rx_buffer.read_pos();
		*m_rx_buffer.data(rpos) = oldest;
		m_rx_buffer.write(data);
		// overflow
		m_regs[REG_RSR] |= RSR_OV;
	}
}

// ----------------------------------------------------------------------------

void MIDI::clear_irq(uint8_t val)
{
	m_regs[REG_ISR] &= ~val;

	// clear offline or break flag on FIFO-Rx
	if (m_regs[REG_IMR] & IMR_OB) {
		m_regs[REG_RSR] &= ~RSR_BRK;
	} else {
		m_regs[REG_RSR] &= ~RSR_OL;
	}

	update_irq();
}

void MIDI::update_irq_vector()
{
	int i = 0;
	uint8_t msk = 1;

	for(;i<8;i++) {
		if (m_regs[REG_ISR] & m_regs[REG_IER] & msk) {
			break;
		}
		msk <<= 1;
	}
	m_regs[REG_IVR] = (i << 1);
}

void MIDI::update_irq()
{
	update_irq_vector();

	if (m_regs[REG_ISR] & m_regs[REG_IER]) {
		write_signals(&outputs_irq, 0xffffffff);
	} else {
		write_signals(&outputs_irq, 0);
	}
}

uint8_t MIDI::read_irq_vector()
{
	return (m_regs[REG_IVR] & IVR_VEC) | (m_regs[REG_IOR] & IOR_MASK);
}

// ----------------------------------------------------------------------------

void MIDI::calc_clkf_timer()
{
	// usually 2^13Tclkf = 13333.33.. usec
	double period = 8192.0 * 1000000.0 / CLOCKF_BASE;

	cancel_my_event(EVENT_MIDI_CLKF);
	register_my_event(EVENT_MIDI_CLKF, period, true);
}

void MIDI::calc_clkm_timer()
{
	// usually 8Tclkm = 8usec
	double periodm = ((m_regs[REG_CCR] & CCR_CLKM) ? 8.0 : 4.0) * 1000000.0 / CLOCKM_BASE;
	double periodt = 16.0 * 1000000.0 / CLOCK_BASE;

	m_clkm_period = periodm > periodt ? periodm : periodt;

	cancel_my_event(EVENT_MIDI_CLKM);
	register_my_event(EVENT_MIDI_CLKM, m_clkm_period, true);
}

void MIDI::count_clkf_timer()
{
	// idle detector
	if (m_idle_counter < 0xff) {
		m_idle_counter++;
	}
	if (m_idle_counter >= 6) {
		// set idle flag
		m_regs[REG_TSR] |= TSR_IDL;
		if (m_regs[REG_DMR] & DMR_ASE) {
			// send active sense
			if (!(pConfig->midi_flags & MIDI_FLAG_NO_REALTIME_MSG)) {
				m_itx_buffer.write(MIDI_MSG_SENSE);
			}
		}
		m_idle_counter = 0;
	}

	// offline detector
	if (m_offline_counter < 0xff) {
		m_offline_counter++;
	}
	if (m_offline_counter >= 24) {
		// set offline flag
		m_regs[REG_RSR] |= RSR_OL;
		if (!(m_regs[REG_IMR] & IMR_OB)) {
			// IRQ
			m_regs[REG_ISR] |= ISR_OL;
			update_irq();
		}
		m_offline_counter = 0;
	}
}

/// @brief Count MIDI counter and General-purpose counter
///
/// @note usually is called every 8usec. 
void MIDI::count_clkm_timer()
{
	// MIDI counter
	countdown_midi_timer();

	// General-purpose counter
	countdown_general_timer();

	// interpolate
	countup_interpolate_counter();
}

// ----------------------------------------------------------------------------

void MIDI::countdown_midi_timer()
{
	if (m_midi_counter) {
		m_midi_counter--;
	}
	if (m_midi_counter == 0) {
		// reload count value
		m_midi_counter = m_midi_counter_base;
		// send real-time message $F8 to FIFO-IRx
		if ((m_regs[REG_DMR] & DMR_MCFS) == DMR_MCFS_TIMER) {
			m_irx_buffer.write(MIDI_MSG_CLOCK);
			// midi-clock check
			midi_dist_check();
		}
//		m_req_load_midi_counter = false;
	}
}

void MIDI::countdown_general_timer()
{
	if (m_general_counter) {
		m_general_counter--;
	}
	if (m_general_counter == 0) {
		// reload count value
		m_general_counter = m_general_counter_base;
		// IRQ
		m_regs[REG_ISR] |= ISR_GT;
		update_irq();
	}
}

// ----------------------------------------------------------------------------

void MIDI::midi_dist_check()
{
	// MIDI message
	switch(m_irx_buffer.read_not_remove(0)) {
	case MIDI_MSG_CLOCK:
		// MIDI-clock

		// shift
		do {
			m_irx_buffer.read();
		} while(m_irx_buffer.read_not_remove(0) == MIDI_MSG_CLOCK);
		// IRQ
		if (m_regs[REG_IMR] & IMR_CT) {
			m_regs[REG_ISR] |= ISR_CC;
			update_irq();
		}
		// send MIDI-clock
		if (!(m_regs[REG_DMR] & DMR_MCDS)) {
			midi_dist_send_midi_clock();
		}

		break;
	case MIDI_MSG_SENSE:
	case MIDI_MSG_RESET:
		break;
	default:
		// MIDI start, stop, continue
		// IRQ
		m_regs[REG_ISR] |= ISR_MM;
		update_irq();
		break;
	}
}

void MIDI::midi_dist_send_midi_clock()
{
	// send to FIFO-ITx
	if ((m_regs[REG_DMR] & DMR_MCE) != 0 && (m_regs[REG_TCR] & TCR_ENA) != 0) {
		if (!(pConfig->midi_flags & MIDI_FLAG_NO_REALTIME_MSG)) {
			m_itx_buffer.write(MIDI_MSG_CLOCK);
		}
	}
	// interpolator
	interpolator_recv_midi_clock();

	// send to SYNC controller

	// send to CLICK counter
	countdown_click_counter();

	// send to playback counter
	countdown_playback_counter();

	// send to recording counter
	countup_recording_counter();
}

void MIDI::midi_dist_send_message(uint8_t msg)
{
	if (msg == MIDI_MSG_CLOCK) {
		// MIDI-clock message
		midi_dist_send_midi_clock();

	} else {
		// send to FIFO-ITx
		if ((m_regs[REG_DCR] & DCR_ITX) != 0 && (m_regs[REG_TCR] & TCR_ENA) != 0) {
			if (!(pConfig->midi_flags & MIDI_FLAG_NO_REALTIME_MSG)) {
				m_itx_buffer.write(msg);
			}
		}
		// send to SYNC controller
		if (m_regs[REG_DCR] & DCR_SYNC) {
		}
		// send to CLICK counter
		if (m_regs[REG_DCR] & DCR_CC) {
			process_click_counter(msg);
		}
		// send to playback counter
		if (m_regs[REG_DCR] & DCR_PC) {
			process_playback_counter(msg);
		}
		// send to recording counter
		if (m_regs[REG_DCR] & DCR_RC) {
			process_recording_counter(msg);
		}
	}
}

// ----------------------------------------------------------------------------

void MIDI::countup_interpolate_counter()
{
	if (m_interpolate_counter < 0xffff) {
		m_interpolate_counter++;
	}
	if (m_interpolate_deltas_idx < m_interpolate_deltas_cnt) {
		if (m_interpolate_counter == m_interpolate_deltas[m_interpolate_deltas_idx]) {
			// interpolate clock generate and count up/down recording or playback counter
			interpolator_count_clock();
			m_interpolate_deltas_idx++;
		}
	}
}

void MIDI::interpolator_recv_midi_clock()
{
	int rate = (m_regs[REG_SCR] & SCR_RATE);
	int missing = 0;

	if (m_interpolate_deltas_idx < m_interpolate_deltas_cnt) {
		// Interpolated clock was not enough generated.
		missing += m_interpolate_deltas_cnt - m_interpolate_deltas_idx;
	}
//	if (m_interpolate_deltas_cnt < rate) {
//		// Rate is more than before one.
//		missing += m_interpolate_deltas_cnt - rate;
//	}
	// Add missing clock events to list at first.
	for(int i=0; i<missing; i++) {
		m_interpolate_deltas[i] = (i + 1);
	}

	if (rate > 0) {
		rate--;
	}
	if (INTERPOLATE_BUFFER_MAX <= (rate + missing)) {
		rate = INTERPOLATE_BUFFER_MAX - missing;
	}
	m_interpolate_deltas_cnt = rate + missing;
	m_interpolate_deltas_idx = 0;
	for(int i=0; i<rate; i++) {
		int val = (m_interpolate_counter * (i + 1) / rate);
		if (val <= missing) {
			val = missing + i + 1;
		}
		m_interpolate_deltas[i+missing] = val;
	}

	m_interpolate_counter = 0;
}

void MIDI::interpolator_count_clock()
{
	// count up recording counter
	countup_recording_counter();

	// count down playback counter
	countdown_playback_counter();
}

// ----------------------------------------------------------------------------

void MIDI::countup_recording_counter()
{
	if (!(m_sequencer_state & SEQ_STATE_RECORD_WORKING)) return;

	if (m_record_counter == 0xff) {
		// IRQ
		m_regs[REG_ISR] |= ISR_RC;
		update_irq();
	}
	m_record_counter++;
}

void MIDI::process_recording_counter(uint8_t msg)
{
	switch(msg) {
	case MIDI_MSG_START:
		m_sequencer_state |= SEQ_STATE_RECORD_WORKING;
		m_record_counter = 0;
		break;
	case MIDI_MSG_CONT:
		m_sequencer_state |= SEQ_STATE_RECORD_WORKING;
		break;
	case MIDI_MSG_STOP:
		m_sequencer_state &= ~SEQ_STATE_RECORD_WORKING;
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void MIDI::countdown_playback_counter()
{
	if (!(m_sequencer_state & SEQ_STATE_PLAY_WORKING)) return;

	if (-32768 < m_playback_counter) {
		m_playback_counter--;
	}
	if (m_playback_counter <= 0) {
		// IRQ
		m_regs[REG_ISR] |= ISR_PC;
		update_irq();
	}
}

void MIDI::process_playback_counter(uint8_t msg)
{
	switch(msg) {
	case MIDI_MSG_START:
		m_sequencer_state |= SEQ_STATE_PLAY_WORKING;
		// will reload the next event 
		m_playback_counter = 0;
		break;
	case MIDI_MSG_CONT:
		m_sequencer_state |= SEQ_STATE_PLAY_WORKING;
		break;
	case MIDI_MSG_STOP:
		m_sequencer_state &= ~SEQ_STATE_PLAY_WORKING;
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void MIDI::countdown_click_counter()
{
	if (!(m_sequencer_state & SEQ_STATE_CLICK_WORKING)) return;

	if (m_click_counter) {
		m_click_counter--;
	}
	if (m_click_counter == 0) {
		// reload count value
		m_click_counter = m_click_counter_base;
		// IRQ
		if (!(m_regs[REG_IMR] & IMR_CT)) {
			m_regs[REG_ISR] |= ISR_CC;
			update_irq();
		}
	}
}

void MIDI::process_click_counter(uint8_t msg)
{
	switch(msg) {
	case MIDI_MSG_START:
		m_sequencer_state |= SEQ_STATE_CLICK_WORKING;
		// will reload the next event 
		m_click_counter = 1;
		break;
	case MIDI_MSG_CONT:
		m_sequencer_state |= SEQ_STATE_CLICK_WORKING;
		break;
	case MIDI_MSG_STOP:
		m_sequencer_state &= ~SEQ_STATE_CLICK_WORKING;
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MIDI::event_frame()
{
}

void MIDI::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MIDI_TXD:
		// need tramsmit next data
		m_register_id[event_id] = -1;
		xmit();
		break;
	case EVENT_MIDI_CLKM:
		// clockm timer
		count_clkm_timer();
		break;
	case EVENT_MIDI_CLKF:
		// clockf timer
		count_clkf_timer();
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int16_LE(v) vm_state.v = Int16_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void MIDI::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<REGS_END; i++) {
		SET_Byte(m_regs[i]);
	}
	for(int i=0; i<EVENT_MIDI_END; i++) {
		SET_Int32_LE(m_register_id[i]);
	}
	m_tx_buffer.save_to(vm_state.m_tx_buffer, vm_state.m_tx_buffer_wpt, vm_state.m_tx_buffer_rpt, vm_state.m_tx_buffer_cnt);
	m_itx_buffer.save_to(vm_state.m_itx_buffer, vm_state.m_itx_buffer_wpt, vm_state.m_itx_buffer_rpt, vm_state.m_itx_buffer_cnt);

	SET_Double_LE(m_xmit_lamda);
	SET_Double_LE(m_xmit_wait_period);

	m_rx_buffer.save_to(vm_state.m_rx_buffer, vm_state.m_rx_buffer_wpt, vm_state.m_rx_buffer_rpt, vm_state.m_rx_buffer_cnt);
	m_irx_buffer.save_to(vm_state.m_irx_buffer, vm_state.m_irx_buffer_wpt, vm_state.m_irx_buffer_rpt, vm_state.m_irx_buffer_cnt);

	SET_Double_LE(m_recv_lamda);
	SET_Double_LE(m_recv_wait_period);

	SET_Double_LE(m_clkm_period);
	SET_Byte(m_click_counter);
	SET_Byte(m_record_counter);
	SET_Int16_LE(m_playback_counter);
	SET_Uint16_LE(m_general_counter);
	SET_Uint16_LE(m_midi_counter);

	SET_Byte(m_click_counter_base);
	SET_Int16_LE(m_playback_counter_base);
	SET_Uint16_LE(m_general_counter_base);
	SET_Uint16_LE(m_midi_counter_base);
	SET_Byte(m_idle_counter);
	SET_Byte(m_offline_counter);
	for(int i=0; i<4; i++) {
		SET_Byte(m_ad_buffer[i]);
	}
	SET_Byte(m_ad_buffer_count);
	SET_Byte(m_ad_match_id);

	for(int i=0; i<16; i++) {
		SET_Uint16_LE(m_interpolate_deltas[i]);
	}
	SET_Uint16_LE(m_interpolate_counter);
	SET_Uint16_LE(m_interpolate_deltas_idx);
	SET_Uint16_LE(m_interpolate_deltas_cnt);
	SET_Byte(m_sequencer_state);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int16_LE(v) v = Int16_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool MIDI::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	if (Uint16_LE(vm_state_i.version) != 1) {
		return false;
	}

	// copy values
	for(int i=0; i<REGS_END; i++) {
		GET_Byte(m_regs[i]);
	}
	for(int i=0; i<EVENT_MIDI_END; i++) {
		GET_Int32_LE(m_register_id[i]);
	}
	m_tx_buffer.load_from(vm_state.m_tx_buffer, vm_state.m_tx_buffer_wpt, vm_state.m_tx_buffer_rpt, vm_state.m_tx_buffer_cnt);
	m_itx_buffer.load_from(vm_state.m_itx_buffer, vm_state.m_itx_buffer_wpt, vm_state.m_itx_buffer_rpt, vm_state.m_itx_buffer_cnt);

	GET_Double_LE(m_xmit_lamda);
	GET_Double_LE(m_xmit_wait_period);

	m_rx_buffer.load_from(vm_state.m_rx_buffer, vm_state.m_rx_buffer_wpt, vm_state.m_rx_buffer_rpt, vm_state.m_rx_buffer_cnt);
	m_irx_buffer.load_from(vm_state.m_irx_buffer, vm_state.m_irx_buffer_wpt, vm_state.m_irx_buffer_rpt, vm_state.m_irx_buffer_cnt);

	GET_Double_LE(m_recv_lamda);
	GET_Double_LE(m_recv_wait_period);

	GET_Double_LE(m_clkm_period);
	GET_Byte(m_click_counter);
	GET_Byte(m_record_counter);
	GET_Int16_LE(m_playback_counter);
	GET_Uint16_LE(m_general_counter);
	GET_Uint16_LE(m_midi_counter);

	GET_Byte(m_click_counter_base);
	GET_Int16_LE(m_playback_counter_base);
	GET_Uint16_LE(m_general_counter_base);
	GET_Uint16_LE(m_midi_counter_base);
	GET_Byte(m_idle_counter);
	GET_Byte(m_offline_counter);
	for(int i=0; i<4; i++) {
		GET_Byte(m_ad_buffer[i]);
	}
	GET_Byte(m_ad_buffer_count);
	GET_Byte(m_ad_match_id);

	for(int i=0; i<16; i++) {
		GET_Uint16_LE(m_interpolate_deltas[i]);
	}
	GET_Uint16_LE(m_interpolate_counter);
	GET_Uint16_LE(m_interpolate_deltas_idx);
	GET_Uint16_LE(m_interpolate_deltas_cnt);
	GET_Byte(m_sequencer_state);

	// timer

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
const uint8_t MIDI::c_reg_map[] = {
	// 00
	REG_IVR,      // Interrupt Vector (read only)
	REG_RGR,      // Register Group / System Control
	REG_ISR,      // Interrupt Service (read only)
	REG_ICR,      // Interrupt Clear (write only)
	REG_IOR,      // Interrupt Vector Offset Request
	REG_IMR,      // Interrupt Mode Control
	REG_IER,      // Interrupt Enable Request
	REG_NUL,
	REG_NUL,
	REG_NUL,
	// 10
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_DMR,      // Real Time Message Control
	REG_DCR,      // Real Time Message Request
	REG_NUL, //		REG_DSR,      // FIFO IRx Data
	REG_NUL, //		REG_DNR,      // FIFO IRx Control
	REG_NUL,
	REG_NUL,
	// 20
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_RRR,      // Rx Rate
	REG_RMR,      // Rx Mode
	REG_AMR,      // Address Hunter Maker
	REG_ADR,      // Address Hunter Device
	REG_NUL,
	REG_NUL,
	// 30
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_RSR,      // FIFO Rx Buffer Status
	REG_RCR,      // FIFO Rx Buffer Control
	REG_NUL, //		REG_RDR,      // FIFO Rx Data
	REG_NUL,
	REG_NUL,
	REG_NUL,
	// 40
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_TRR,      // Tx Rate
	REG_TMR,      // Tx Mode
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	// 50
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_TSR,      // FIFO Tx Status
	REG_TCR,      // FIFO Tx Control
	REG_NUL, //		REG_TDR,      // FIFO Tx Data
	REG_NUL,
	REG_NUL,
	REG_NUL,
	// 60
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_FSR,      // FSK status
	REG_FCR,      // FSK control
	REG_CCR,      // Click Counter Control
	REG_CDR,      // Click Counter Data (7-bit)
	REG_NUL,
	REG_NUL,
	// 70
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL, //		REG_SRR,      // Recording Counter current value
	REG_SCR,      // Sequencer Control
	REG_SPRL,     // Playback Counter (low 8-bits)
	REG_SPRH,     // Playback Counter (high 7-bits)
	REG_NUL,
	REG_NUL,
	// 80
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_GTRL,     // General Timer (low 8-bits)
	REG_GTRH,     // General Timer (high 6-bits)
	REG_MTRL,     // MIDI Clock Timer (low 8-bits)
	REG_MTRH,     // MIDI Clock Timer (high 6-bits)
	REG_NUL,
	REG_NUL,
	// 90
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_NUL,
	REG_EDR,      // External I/O Direction
	REG_EOR,      // External I/O Output Data
	REG_EIR,      // External I/O Input Data
	REG_NUL,
	REG_NUL,
	REG_NUL
};

const _TCHAR *MIDI::c_reg_names[] = {
	NULL,			// Dummy

	_T("IVR "),     // Interrupt Vector (read only)
	_T("RGR "),     // Register Group / System Control
	_T("ISR "),     // Interrupt Service (read only)
	_T("ICR "),     // Interrupt Clear (write only)

	_T("IOR "),     // Interrupt Vector Offset Request
	_T("IMR "),     // Interrupt Mode Control
	_T("IER "),     // Interrupt Enable Request

	_T("DMR "),     // Real Time Message Control
	_T("DCR "),     // Real Time Message Request
//	_T("DSR "),     // FIFO IRx Data
//	_T("DNR "),     // FIFO IRx Control

	_T("RRR "),     // Rx Rate
	_T("RMR "),     // Rx Mode
	_T("AMR "),     // Address Hunter Maker
	_T("ADR "),     // Address Hunter Device

	_T("RSR "),     // FIFO Rx Buffer Status
	_T("RCR "),     // FIFO Rx Buffer Control
//	_T("RDR "),     // FIFO Rx Data

	_T("TRR "),     // Tx Rate
	_T("TMR "),     // Tx Mode

	_T("TSR "),     // FIFO Tx Status
	_T("TCR "),     // FIFO Tx Control
//	_T("TDR "),     // FIFO Tx Data

	_T("FSR "),     // FSK status
	_T("FCR "),     // FSK control
	_T("CCR "),     // Click Counter Control
	_T("CDR "),     // Click Counter Data (7-bit)

//	_T("SRR "),     // Recording Counter current value
	_T("SCR "),     // Sequencer Control
	_T("SPRL"),     // Playback Counter (low 8-bits)
	_T("SPRH"),     // Playback Counter (high 7-bits)

	_T("GTRL"),     // General Timer (low 8-bits)
	_T("GTRH"),     // General Timer (high 6-bits)
	_T("MTRL"),     // MIDI Clock Timer (low 8-bits)
	_T("MTRH"),     // MIDI Clock Timer (high 6-bits)

	_T("EDR "),     // External I/O Direction
	_T("EOR "),     // External I/O Output Data
	_T("EIR "),     // External I/O Input Data

	NULL,			// Dummy
	NULL
};

uint32_t MIDI::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;

	addr &= 0x07;

	if(addr < 4) {
		data = debug_read_reg(addr);
	} else {
		uint32_t bank = (m_regs[REG_RGR] & 0x0f);
		addr = bank * 10 + addr;
		data = debug_read_reg(addr);
	}

	return data;
}

uint32_t MIDI::debug_read_reg(uint32_t reg_num)
{
	uint32_t data = 0;

	if (reg_num >= MCS_END) {
		return data;
	}

	int reg_idx = c_reg_map[reg_num];

	switch(reg_num) {
	case MCS_IVR:
		// IRQ Vector
		data = read_irq_vector();
		break;
	case MCS_DSR:
		// FIFO-IRx Data
		data = m_irx_buffer.read_not_remove(0);
		break;
	case MCS_RSR:
		// FIFO-Rx Status
		data = m_regs[REG_RSR];
		data |= ((m_rx_buffer.read_not_remove(0) >> 4) & (RSR_FE | RSR_PE));
		if (!m_rx_buffer.empty()) data |= RSR_RDY;
		break;
	case MCS_RDR:
		// FIFO-Rx Data
		data = m_rx_buffer.read_not_remove(0);
		break;
	case MCS_TSR:
		// FIFO-Tx Status
		data = m_regs[REG_TSR];
		if (m_tx_buffer.empty()) data |= TSR_EMP;
		if (!m_tx_buffer.full()) data |= TSR_RDY;
		break;
	case MCS_SRR:
		// Recording Counter Current Value
		data = m_record_counter;
		break;
	case MCS_EIR:
		// External Input Data
		data = (m_regs[REG_EIR] & ~m_regs[REG_EDR]);
		break;
	default:
		if (reg_idx > REG_NUL) {
			data = m_regs[reg_idx];
		}
		break;
	}
	return data;
}

bool MIDI::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool MIDI::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < MCS_END) {
		int reg_idx = c_reg_map[reg_num];
		switch(reg_idx) {
		case REG_NUL:
			break;
		default:
			m_regs[reg_idx] = data;
			break;
		}
		return true;
	} else {
		return false;
	}
}

void MIDI::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	uint32_t data;
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("MIDI\n"));
	//  register value
	for(int reg_num = MCS_IVR; reg_num < MCS_END; reg_num++) {
		if ((reg_num % 10) == 4) {
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		}
		data = debug_read_reg(reg_num);
		int reg_idx = c_reg_map[reg_num];
		const _TCHAR *name;
		switch(reg_num) {
		case MCS_DSR:
			name = _T("DSR ");     // FIFO IRx Data
			break;
		case MCS_DNR:
			name = _T("DNR ");     // FIFO IRx Control
			break;
		case MCS_RDR:
			name = _T("RDR ");     // FIFO Rx Data
			break;
		case MCS_TDR:
			name = _T("TDR ");     // FIFO Tx Data
			break;
		case MCS_SRR:
			name = _T("SRR ");     // Recording Counter current value
			break;
		default:
			name = c_reg_names[reg_idx];
			break;
		}
		if (name) {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %02X(R%02d:%s):%02X")
				, reg_num, reg_num
				, name
				, data
			);
		}
	}
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	// buffer value
	UTILITY::tcscat(buffer, buffer_len, _T("Buffers:"));
	int cnt = 0;
	cnt = m_rx_buffer.count();
	UTILITY::sntprintf(buffer, buffer_len, _T("\n FIFO-Rx (% 4d):"), cnt);
	for(int i=0; i<cnt; i++) {
		data = m_rx_buffer.read_not_remove(i);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %03X"), data);
	}
	cnt = m_irx_buffer.count();
	UTILITY::sntprintf(buffer, buffer_len, _T("\n FIFO-IRx(% 4d):"), cnt);
	for(int i=0; i<cnt; i++) {
		data = m_irx_buffer.read_not_remove(i);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), data);
	}
	cnt = m_tx_buffer.count();
	UTILITY::sntprintf(buffer, buffer_len, _T("\n FIFO-Tx (% 4d):"), cnt);
	for(int i=0; i<cnt; i++) {
		data = m_tx_buffer.read_not_remove(i);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), data);
	}
	cnt = m_itx_buffer.count();
	UTILITY::sntprintf(buffer, buffer_len, _T("\n FIFO-ITx(% 4d):"), cnt);
	for(int i=0; i<cnt; i++) {
		data = m_itx_buffer.read_not_remove(i);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), data);
	}
	cnt = m_ad_buffer_count;
	UTILITY::sntprintf(buffer, buffer_len, _T("\n Address Hunter(% 4d):"), cnt);
	for(int i=0; i<cnt; i++) {
		data = m_ad_buffer[i];
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), data);
	}
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	// counter value
	UTILITY::tcscat(buffer, buffer_len, _T("Counters:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" MIDI clock: %d(%d)"), m_midi_counter, m_midi_counter_base);
	UTILITY::sntprintf(buffer, buffer_len, _T(" General: %d(%d)\n"), m_general_counter, m_general_counter_base);
	UTILITY::sntprintf(buffer, buffer_len, _T(" Playback: %d(%d)"), m_playback_counter, m_playback_counter_base);
	UTILITY::sntprintf(buffer, buffer_len, _T(" Recording: %d\n"), m_record_counter);
	UTILITY::sntprintf(buffer, buffer_len, _T(" Click: %d"), m_click_counter, m_click_counter_base);
	UTILITY::sntprintf(buffer, buffer_len, _T(" Idle: %d"), m_idle_counter);
	UTILITY::sntprintf(buffer, buffer_len, _T(" Offline: %d\n"), m_offline_counter);

}
#endif
