/** @file mfp.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@attention TODO: The pulse-width mode on Timer A and B is not implemented.


	@brief [ mfp modoki (mc68901) ]
*/

#include "mfp.h"
//#include "../emu.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"

#ifdef _DEBUG
//#define OUT_DEBUG_INTR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_INTR(...)
//#define OUT_DEBUG_IERA logging->out_debugf
#define OUT_DEBUG_IERA(...)
//#define OUT_DEBUG_IERB logging->out_debugf
#define OUT_DEBUG_IERB(...)
//#define OUT_DEBUG_TIMERC logging->out_debugf
#define OUT_DEBUG_TIMERC(...)
#else
#define OUT_DEBUG_INTR(...)
#define OUT_DEBUG_IERA(...)
#define OUT_DEBUG_IERB(...)
#define OUT_DEBUG_TIMERC(...)
#endif

// B,A
const uint16_t MFP::c_intr_pri[16] = {
	0x0100, // GPIP0
	0x0200, // GPIP1
	0x0400, // GPIP2
	0x0800, // GPIP3
	0x1000, // Timer D
	0x2000, // Timer C
	0x4000, // GPIP4
	0x8000, // GPIP5
	0x0001, // Timer B
	0x0002, // Transmit Error
	0x0004, // Transmit Buffer Empty
	0x0008, // Receive Error
	0x0010, // Receive Buffer Full
	0x0020, // Timer A
	0x0040, // GPIP6
	0x0080  // GPIP7
};
/// Prescaler for timer
const uint8_t MFP::c_prescaler[] = {
	1, 4, 10, 16, 50, 64, 100, 200
};

// ----------------------------------------------------------------------------

void MFP::initialize()
{
	for(int i=0; i<4; i++) {
		m_timer_register_id[i] = -1;
		m_timer_counter[i] = 0;
		m_timer_period[i] = 1.0;
	}
	for(int i=0; i<2; i++) {
		m_timer_prev_input[i] = 0;
	}
//	m_xmit_register_id = -1;
//	m_xmit_clock = 38400;
	m_xmit_wait_count = 0;
	m_xmit_prev_edge = 0;

	m_irq_register_id = -1;
}

void MFP::reset()
{
	warm_reset(true);
}

void MFP::warm_reset(bool por)
{
	if (por) {
		for(int i=0; i<MFP_REGS_END; i++) {
			m_regs[i] = 0;
		}
		m_regs[MFP_GPDR] = 0xff;	// X68K
	} else {
		for(int i=0; i<MFP_REGS_END; i++) {
			if (i != MFP_GPDR) m_regs[i] = 0;
		}
	}

	m_vector = 0;
//	m_now_iack = false;

	m_timer_clock = 4000000;	// 4MHz(X68K)

	m_regs[MFP_TSR] = TSR_BE;

//	if (por) {
//		for(int i=0; i<4; i++) {
//			m_timer_register_id[i] = -1;
//			m_timer_counter[i] = 0;
//			m_timer_period[i] = 1.0;
//		}
//	} else {
		for(int i=0; i<4; i++) {
			cancel_my_event(m_timer_register_id[i]);
			m_timer_counter[i] = 0;
			m_timer_period[i] = 1.0;
		}
//	}
	m_timer_output = 0;

	for(int i=0; i<2; i++) {
		m_timer_prev_input[i] = 0;
	}

	if (!por) {
//		cancel_my_event(m_xmit_register_id);
		cancel_my_event(m_irq_register_id);
	} else {
//		m_xmit_register_id = -1;
		m_irq_register_id = -1;
	}
	m_xmit_wait_count = 0;
	m_xmit_prev_edge = 0;

	m_send_buf = 0;
	m_recv_buf = 0;
	m_buf_status = 0;

	m_irq_ipr = 0;

	write_signals(&outputs_irq, 0);
	write_signals(&outputs_tmo[0], 0);
	write_signals(&outputs_tmo[1], 0);
	write_signals(&outputs_tmo[2], 0);
	write_signals(&outputs_tmo[3], 0);
}

// ----------------------------------------------------------------------------

void MFP::cancel_my_event(int &register_id)
{
	if (register_id != -1) cancel_event(this, register_id);
	register_id = -1;
}

// ----------------------------------------------------------------------------

void MFP::write_io8(uint32_t addr, uint32_t data)
{
	if (now_reset) return;

	addr &= 0x1f;

	switch(addr & 0x1f) {
	case MFP_IERA:
		m_regs[addr] = data;
		// update IPRA and ISRA
		// cancel asserting interrupt
		m_regs[MFP_IPRA] &= m_regs[MFP_IERA];
		m_regs[MFP_ISRA] &= m_regs[MFP_IERA];
		OUT_DEBUG_IERA(_T("clk:%lld MFP IERA W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_IMRA:
		m_regs[addr] = data;
		// update IPRA and ISRA
		// cancel asserting interrupt and back to pending
		m_regs[MFP_IPRA] |= (m_regs[MFP_ISRA] ^ m_regs[MFP_IMRA]) & m_regs[MFP_ISRA];
		m_regs[MFP_ISRA] &= m_regs[MFP_IMRA];
		OUT_DEBUG_IERA(_T("clk:%lld MFP IMRA W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_IPRA:
	case MFP_ISRA:
		// all bits can be cleared only.
		m_regs[addr] &= data;
		OUT_DEBUG_IERA(_T("clk:%lld MFP I%cRA W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, (addr & 0x1f) == MFP_IPRA ? _T('P') : _T('S')
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_IERB:
		m_regs[addr] = data;
		// update IPRB and ISRB
		// cancel asserting interrupt
		m_regs[MFP_IPRB] &= m_regs[MFP_IERB];
		m_regs[MFP_ISRB] &= m_regs[MFP_IERB];
		OUT_DEBUG_IERB(_T("clk:%lld MFP IERB W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_IMRB:
		m_regs[addr] = data;
		// update IPRB and ISRB
		// cancel asserting interrupt and back to pending
		m_regs[MFP_IPRB] |= (m_regs[MFP_ISRB] ^ m_regs[MFP_IMRB]) & m_regs[MFP_ISRB];
		m_regs[MFP_ISRB] &= m_regs[MFP_IMRB];
		OUT_DEBUG_IERB(_T("clk:%lld MFP IMRB W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_IPRB:
	case MFP_ISRB:
		// all bits can be cleared only.
		m_regs[addr] &= data;
		OUT_DEBUG_IERB(_T("clk:%lld MFP I%cRB W ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
			, (addr & 0x1f) == MFP_IPRA ? _T('P') : _T('S')
			, get_current_clock()
			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
		// interrupt
		update_irq();
		break;
	case MFP_TACR:
	case MFP_TBCR:
		{
			// timer A,B controller
			int n = addr - MFP_TACR;

			m_regs[addr] = data & 0xff;

			if (data & 0x10) {
				// Reset forced low output
				m_timer_output &= ~(0x01 << n);
				write_signals(&outputs_tmo[n], m_timer_output & (0x01 << n));
			}
			if (data & 0x7) {
				// Countdown start
				m_timer_counter[n] = m_regs[MFP_TADR + n];
				m_timer_period[n] = (double)c_prescaler[data & 0x7] * 1000000.0 / m_timer_clock; 
				cancel_my_event(m_timer_register_id[n]);
				register_event(this, EVENT_MFP_TIMER_A + n, m_timer_period[n], true, &m_timer_register_id[n]);
			} else {
				// Stop Timer
				cancel_my_event(m_timer_register_id[n]);
			}
		}
		break;
	case MFP_TCDCR:
		{
			// timer C,D controller
			m_regs[addr] = data & 0xff;

			for(int n=1; n>=0; n--) {
				if (data & 0x7) {
					// Countdown start
					m_timer_counter[n + 2] = m_regs[MFP_TCDR + n];
					m_timer_period[n + 2] = (double)c_prescaler[data & 0x7] * 1000000.0 / m_timer_clock; 
					cancel_my_event(m_timer_register_id[n + 2]);
					register_event(this, EVENT_MFP_TIMER_C + n, m_timer_period[n + 2], true, &m_timer_register_id[n + 2]);

					OUT_DEBUG_TIMERC(_T("clk:%lld MFP TIMER%c START: prescale:%d -> %fus.")
						, get_current_clock()
						, n + 0x43, c_prescaler[data & 0x7], m_timer_period[n + 2]);
				} else {
					// Stop Timer
					cancel_my_event(m_timer_register_id[n + 2]);
				}
				data >>= 4;
			}
		}
		break;
	case MFP_GPDR:
		break;
	case MFP_TSR:
		// transmit status
		m_regs[addr] = data & ~TSR_BE;	// buffer empty flag is not care.
		empty_xmit();
		start_xmit();
		break;
	case MFP_UDRI:
		{
			// send serial data
			if ((m_regs[MFP_TSR] & TSR_BE) != 0) {
				m_regs[MFP_UDRO] = (data & 0xff);
				//
				m_regs[MFP_TSR] &= ~(TSR_BE | TSR_UE);

				// regist the event of sent data
				start_xmit();
			} else {
				// buffer full
				m_regs[MFP_TSR] |= TSR_UE;

				// interrupt
				if ((m_regs[MFP_IERA] & IRA_XMTERR) != 0) {
					// set pending register
					m_regs[MFP_IPRA] |= IRA_XMTERR;
					// interrupt
					update_irq();
				}
			}
		}
		break;
	case MFP_RSR:
		m_regs[MFP_RSR] = data & 0xff;
		// send enable signal to sender
		d_serial->write_signal(SIG_RR, (data & RSR_RE) && !(m_buf_status & RX_BUF_FULL) ? 1 : 0, 1);
		break;
	default:
		m_regs[addr] = data & 0xff;
		break;
	}
}

/// now interrupt, send vector number
uint32_t MFP::read_external_data8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	addr &= 0x1f;

	// interrupt vector
	data = m_vector;

	OUT_DEBUG_INTR(_T("clk:%lld MFP IACK vec:%02X ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
		, get_current_clock()
		, m_vector
		, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
		, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
		, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);

	// clear interrupt process
	if (!(m_regs[MFP_VR] & VR_S)) {
		// automatic
		m_regs[MFP_ISRA] = 0;
		m_regs[MFP_ISRB] = 0;
		// clear interrupt
		update_irq();
	}
	return data;
}

uint32_t MFP::read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	addr &= 0x1f;

//	if (m_now_iack) {
//		// interrupt vector
//		data = m_vector;
//
//		OUT_DEBUG_INTR(_T("clk:%lld MFP IACK vec:%02X ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
//			, get_current_clock()
//			, m_vector
//			, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
//			, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
//			, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
//
//		// clear interrupt process
//		if (!(m_regs[MFP_VR] & VR_S)) {
//			// automatic
//			m_regs[MFP_ISRA] = 0;
//			m_regs[MFP_ISRB] = 0;
//			// clear interrupt
//			update_irq();
//		}
//	} else {
		switch(addr & 0x1f) {
		case MFP_IERA:
		case MFP_IPRA:
		case MFP_ISRA:
		case MFP_IMRA:
			data = m_regs[addr];
			break;
		case MFP_IERB:
		case MFP_IPRB:
		case MFP_ISRB:
			data = m_regs[addr];
			break;
		case MFP_IMRB:
			data = m_regs[addr];
			OUT_DEBUG_IERB(_T("clk:%lld MFP IMRB R ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X")
				, get_current_clock()
				, m_regs[MFP_ISRA], m_regs[MFP_ISRB]
				, m_regs[MFP_IPRA], m_regs[MFP_IPRB]
				, m_regs[MFP_IMRA], m_regs[MFP_IMRB]);
			break;
		case MFP_UDRI:
			// received serial data
			data = m_regs[MFP_UDRI];
			// clear status
			m_regs[MFP_RSR] &= ~(RSR_BF | RSR_OE | RSR_PE | RSR_FE | RSR_FSB);
			// next data
			store_received_data();
			break;
		case MFP_TADR:
		case MFP_TBDR:
		case MFP_TCDR:
		case MFP_TDDR:
			// current value on timer counter
			data = m_timer_counter[addr - MFP_TADR];
			break;
		default:
			data = m_regs[addr];
			break;
		}
//	}
	return data;
}

void MFP::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
	case SIG_GPIP:
		{
			uint8_t prev = m_regs[MFP_GPDR];

			// DDR 0 as input.
			if (data & mask) {
				m_regs[MFP_GPDR] |= (mask & ~m_regs[MFP_DDR]);
			} else {
				m_regs[MFP_GPDR] &= ~(mask & ~m_regs[MFP_DDR]);
			}
			// interrupt
			uint8_t ier = (m_regs[MFP_IERB] & 0x0f) | ((m_regs[MFP_IERB] & 0xc0) >> 2) | (m_regs[MFP_IERA] & 0xc0);
			uint8_t edge;
			if (ier & ~m_regs[MFP_DDR]) {
				// catch riging edge
				edge = ((prev ^ m_regs[MFP_GPDR]) & m_regs[MFP_GPDR] & ier & m_regs[MFP_AER]);
				if (edge != 0) {
					// set pending register
					m_regs[MFP_IPRA] |= (edge & 0xc0);
					m_regs[MFP_IPRB] |= (edge & 0x0f);
					m_regs[MFP_IPRB] |= ((edge & 0x30) << 2);
					// interrupt
					OUT_DEBUG_INTR(_T("clk:%lld MFP INTR RIGING EDGE IER:%02X AER:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X ISRA:%02X ISRB:%02X")
						, get_current_clock()
						, ier, m_regs[MFP_AER], m_regs[MFP_IPRA], m_regs[MFP_IPRB]
						, m_regs[MFP_IMRA], m_regs[MFP_IMRB]
						, m_regs[MFP_ISRA], m_regs[MFP_ISRB]);
					update_irq();
				}
				// catch falling edge
				edge = ((prev ^ m_regs[MFP_GPDR]) & prev & ier & ~m_regs[MFP_AER]);
				if (edge != 0) {
					// set pending register
					m_regs[MFP_IPRA] |= (edge & 0xc0);
					m_regs[MFP_IPRB] |= (edge & 0x0f);
					m_regs[MFP_IPRB] |= ((edge & 0x30) << 2);
					// interrupt
					OUT_DEBUG_INTR(_T("clk:%lld MFP INTR FALLING EDGE IER:%02X AER:%02X IPRA:%02X IPRB:%02X IMRA:%02X IMRB:%02X ISRA:%02X ISRB:%02X")
						, get_current_clock()
						, ier, m_regs[MFP_AER], m_regs[MFP_IPRA], m_regs[MFP_IPRB]
						, m_regs[MFP_IMRA], m_regs[MFP_IMRB]
						, m_regs[MFP_ISRA], m_regs[MFP_ISRB]);
					update_irq();
				}
			}
		}
		break;
	case SIG_TAI:
	case SIG_TBI:
		{
			int n = (id - SIG_TAI);
			if ((m_regs[MFP_TACR + n] & 0xf) == 0x8) {
				// enable when event count mode
				uint8_t aer = m_regs[MFP_AER] & (0x10 >> n);
				if (aer) {
					// rising edge
					if (m_timer_prev_input[n] < (data & mask)) {
							// count down
							count_down_timer_a_b(n);
					}
				} else {
					// falling edge
					if (m_timer_prev_input[n] > (data & mask)) {
							// count down
							count_down_timer_a_b(n);
					}
				}
			}
			m_timer_prev_input[n] = (data & mask);
		}
		break;
	case SIG_SI:
		// receive serial input
		receive_data(data & mask);
		break;
	case SIG_TXC:
		if ((m_xmit_prev_edge ^ data) & data & mask) {
			// rise up edge
			if (m_xmit_wait_count) {
				m_xmit_wait_count--;
				if (m_xmit_wait_count == 0) {
					// transmit
//					logging->out_debugf(_T("MFP End Xmit Clk:%lld"), get_current_clock());

					xmit();
				}
			}
		}
		m_xmit_prev_edge = (data & mask);
		break;
//	case SIG_IACK:
//		// receive interrupt ack signal
//		m_now_iack = ((data & mask) != 0);
//		break;
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

void MFP::empty_xmit()
{
	// empty interrupt
	if ((m_regs[MFP_TSR] & (TSR_BE | TSR_TE)) == (TSR_BE | TSR_TE) && (m_regs[MFP_IERA] & IRA_XMTEMP) != 0) {
		// set pending register
		m_regs[MFP_IPRA] |= IRA_XMTEMP;
		// interrupt
		update_irq();
	} else {
		// clear interrupt
		update_irq();
	}
}

void MFP::start_xmit()
{
	// data filled so start transmit process
//	if ((m_regs[MFP_TSR] & (TSR_BE | TSR_TE)) == TSR_TE) {
		if (!(m_buf_status & TX_BUF_FULL)) {
			// buffer empty
			m_send_buf = m_regs[MFP_UDRO];
			m_buf_status |= TX_BUF_FULL;

//			cancel_my_event(m_xmit_register_id);
			int bit_len = 8 - ((m_regs[MFP_UCR] >> 5) & 3);
			switch ((m_regs[MFP_UCR] >> 3) & 3) {
			case 1:
				bit_len += 2;
				break;
			case 2:
			case 3:
				bit_len += 3;
				break;
			default:
				break;
			}

			m_xmit_wait_count = bit_len * (m_regs[MFP_UCR] & 0x80 ? 16 : 1);

//			logging->out_debugf(_T("MFP Start Xmit Clk:%lld"), get_current_clock());

//			double usec = (double)bit_len * (m_regs[MFP_UCR] & 0x80 ? 16.0 : 1.0) * 1000000.0 / m_xmit_clock;
//			register_event(this, EVENT_MFP_XMIT, usec, false, &m_xmit_register_id);

			m_regs[MFP_TSR] |= TSR_BE;
		}
//	}
}

void MFP::xmit()
{
	// send data
	if (d_serial) d_serial->write_signal(MFP::SIG_SO, m_send_buf, 0xff);
	m_buf_status &= ~TX_BUF_FULL;
	//
	if (m_regs[MFP_TSR] & TSR_END) {
		// clear enable bit automatically
		m_regs[MFP_TSR] &= ~TSR_TE;
	}
	uint8_t prev_tsr = m_regs[MFP_TSR];
	// interrupt
	m_regs[MFP_TSR] &= ~(TSR_UE | TSR_B);
	m_regs[MFP_TSR] |= TSR_BE;
	empty_xmit();
	// alreay stored next data
	if (!(prev_tsr & TSR_BE)) {
		start_xmit();
	}
}

void MFP::set_full_receive_buffer()
{
	m_buf_status |= RX_BUF_FULL;
	// send enable signal to sender
	d_serial->write_signal(SIG_RR, 0, 1);
}

void MFP::clear_full_receive_buffer()
{
	m_buf_status &= ~RX_BUF_FULL;
	// send enable signal to sender
	d_serial->write_signal(SIG_RR, m_regs[MFP_RSR] & RSR_RE, 1);
}

void MFP::receive_data(uint8_t data)
{
	if ((m_regs[MFP_RSR] & RSR_RE) != 0) {
		// receive serial input
		if (m_buf_status & RX_BUF_FULL) {
			// buffer overrun
			m_regs[MFP_RSR] |= RSR_OE;

			update_irq_for_rx(IRA_RCVERR);
		} else {
			// receive data
			m_recv_buf = data;
			set_full_receive_buffer();

			m_regs[MFP_RSR] &= ~(RSR_OE | RSR_PE | RSR_FE | RSR_FSB | RSR_MCIP);

			store_received_data();
		}
	}
}

void MFP::store_received_data()
{
	if ((m_buf_status & RX_BUF_FULL) && !(m_regs[MFP_RSR] & RSR_BF)) {
		m_regs[MFP_RSR] |= RSR_BF;
		m_regs[MFP_UDRI] = m_recv_buf;
		clear_full_receive_buffer();

		update_irq_for_rx(IRA_RCVFUL);
	}
}

void MFP::update_irq_for_rx(uint32_t maska)
{
	// interrupt
	if ((m_regs[MFP_IERA] & maska) != 0) {
		// set pending register
		m_regs[MFP_IPRA] |= maska;
		// interrupt
		update_irq();
	}
}

void MFP::update_irq()
{
	if ((m_regs[MFP_ISRA] | m_regs[MFP_ISRB]) == 0) {
		uint16_t ipr = ((((uint16_t)m_regs[MFP_IPRB] & m_regs[MFP_IMRB]) << 8) | (m_regs[MFP_IPRA] & m_regs[MFP_IMRA]));
		if (ipr) {
			m_irq_ipr = ipr;
			assert_irq();
//			if (m_irq_register_id == -1) register_event_by_clock(this, EVENT_MFP_IRQ, 4, false, &m_irq_register_id);
		} else {
			write_signals(&outputs_irq, 0);
//			OUT_DEBUG_INTR(_T("MFP INTR Reset"));
		}
	}
}

void MFP::assert_irq()
{
	if (m_irq_ipr) {
		write_signals(&outputs_irq, 0xffffffff);
		// interrupt vector signal
		for(int i=15; i>=0; --i) {
			if (c_intr_pri[i] & m_irq_ipr) {
				m_vector = ((m_regs[MFP_VR] & VR_VEC) | i);
				m_regs[MFP_ISRA] = (c_intr_pri[i] & 0xff);
				m_regs[MFP_ISRB] = (c_intr_pri[i] >> 8);
				m_regs[MFP_IPRA] &= ~(c_intr_pri[i] & 0xff);
				m_regs[MFP_IPRB] &= ~(c_intr_pri[i] >> 8);
				OUT_DEBUG_INTR(_T("clk:%lld MFP ASSERT INTR VECNUM:%d VEC:%02X ISRA:%02X ISRB:%02X IPRA:%02X IPRB:%02X")
					, get_current_clock()
					, i, m_vector
					, m_regs[MFP_ISRA], m_regs[MFP_ISRB], m_regs[MFP_IPRA], m_regs[MFP_IPRB]);
				break;
			}
		}
		m_irq_ipr = 0;
	}
}

/// @param[in] n : 0:A 1:B
void MFP::count_down_timer_a_b(int n)
{
	// count down
//	if (m_timer_counter[n]) {
		m_timer_counter[n]--;
		if (m_timer_counter[n] == 0) {
			// set pending register
			m_regs[MFP_IPRA] |= ((0x20 >> (n * 5)) & m_regs[MFP_IERA]);
			// interrupt
			update_irq();
			// and toggle output signal
			m_timer_output ^= (0x01 << n);
			write_signals(&outputs_tmo[n], m_timer_output & (0x01 << n));

			// continue count down
			m_timer_counter[n] = m_regs[MFP_TADR + n];
		}
//	}
//	register_event(this, EVENT_MFP_TIMER_A + n, m_timer_period[n], false, &m_timer_register_id[n]);
}

/// @param[in] n : 0:C 1:D
void MFP::count_down_timer_c_d(int n)
{
	// count down
//	if (m_timer_counter[n + 2]) {
		m_timer_counter[n + 2]--;
		if (m_timer_counter[n + 2] == 0) {
			// set pending register
			m_regs[MFP_IPRB] |= ((0x20 >> n) & m_regs[MFP_IERB]);
			// fire interrupt
			update_irq();
			// and toggle output signal
			m_timer_output ^= (0x04 << n);
			write_signals(&outputs_tmo[n + 2], m_timer_output & (0x04 << n));
			// continue count down
			m_timer_counter[n + 2] = m_regs[MFP_TCDR + n];

			OUT_DEBUG_TIMERC(_T("clk:%lld TIMER%c ZERO")
				, get_current_clock()
				, n + 0x43);
		}
//	}
//	register_event(this, EVENT_MFP_TIMER_C + n, m_timer_period[n + 2], false, &m_timer_register_id[n + 2]);
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MFP::event_frame()
{
}

void MFP::event_callback(int event_id, int err)
{
//	logging->out_debugf("mfp event_callback %d",event_id);
	switch(event_id) {
	case EVENT_MFP_TIMER_A:
	case EVENT_MFP_TIMER_B:
		// count down
		count_down_timer_a_b(event_id - EVENT_MFP_TIMER_A);
		break;
	case EVENT_MFP_TIMER_C:
	case EVENT_MFP_TIMER_D:
		// count down
		count_down_timer_c_d(event_id - EVENT_MFP_TIMER_C);
		break;
//	case EVENT_MFP_XMIT:
//		// transmit finished
//		m_xmit_register_id = -1;
//		xmit();
//		break;
	case EVENT_MFP_IRQ:
		m_irq_register_id = -1;
		assert_irq();
		break;
	}
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void MFP::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<MFP_REGS_END; i++) {
		SET_Byte(m_regs[i]);
	}
	SET_Byte(m_vector);	///< vector number on interrupt
//	SET_Bool(m_now_iack);	///< receiving IACK
	SET_Byte(m_timer_output);		///< bit0 timer A, bit 1 timer B

	for(int i=0; i<4; i++) {
		SET_Byte(m_timer_counter[i]);			///< count down timer
		SET_Double_LE(m_timer_period[i]);		///< timer event clock (us)
		SET_Int32_LE(m_timer_register_id[i]);	///< for timer event
	}

	SET_Int32_LE(m_timer_clock);		///< Timer clock
//	SET_Int32_LE(m_xmit_clock);		///< clock for transmit

	for(int i=0; i<2; i++) {
		SET_Uint32_LE(m_timer_prev_input[i]);	///< TAI and TBI diff 
	}
//	SET_Int32_LE(m_xmit_register_id);	///< interrupt after sent data

	SET_Byte(m_send_buf);	///< xmit buffer
	SET_Byte(m_recv_buf); ///< recv buffer
	SET_Byte(m_buf_status);

	SET_Int32_LE(m_irq_register_id);	///< delay interrupt
	SET_Int32_LE(m_irq_ipr);			///< interrupt priority

	SET_Int32_LE(m_xmit_wait_count);	///< transmit wait
	SET_Uint32_LE(m_xmit_prev_edge);	///< transmit clock edge on/off

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool MFP::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	if (Uint16_LE(vm_state_i.version) != 2) {
		return false;
	}

	// copy values
	for(int i=0; i<MFP_REGS_END; i++) {
		GET_Byte(m_regs[i]);
	}
	GET_Byte(m_vector);	///< vector number on interrupt
//	GET_Bool(m_now_iack);	///< receiving IACK
	GET_Byte(m_timer_output);		///< bit0 timer A, bit 1 timer B

	for(int i=0; i<4; i++) {
		GET_Byte(m_timer_counter[i]);			///< count down timer
		GET_Double_LE(m_timer_period[i]);		///< timer event clock (us)
		GET_Int32_LE(m_timer_register_id[i]);	///< for timer event
	}

	GET_Int32_LE(m_timer_clock);		///< Timer clock
//	GET_Int32_LE(m_xmit_clock);		///< clock for transmit

	for(int i=0; i<2; i++) {
		GET_Uint32_LE(m_timer_prev_input[i]);	///< TAI and TBI diff 
	}
//	GET_Int32_LE(m_xmit_register_id);	///< interrupt after sent data

	GET_Byte(m_send_buf);	///< xmit buffer
	GET_Byte(m_recv_buf); ///< recv buffer
	GET_Byte(m_buf_status);

	GET_Int32_LE(m_irq_register_id);	///< delay interrupt
	GET_Int32_LE(m_irq_ipr);			///< interrupt priority

	GET_Int32_LE(m_xmit_wait_count);	///< transmit wait
	GET_Uint32_LE(m_xmit_prev_edge);	///< transmit clock edge on/off

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t MFP::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;

	addr &= 0x1f;

	switch(addr & 0x1f) {
	case MFP_TADR:
	case MFP_TBDR:
	case MFP_TCDR:
	case MFP_TDDR:
		// current value on timer counter
		data = m_timer_counter[addr - MFP_TADR];
		break;
	default:
		data = m_regs[addr];
		break;
	}

	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("GPDR"),
	_T("AER"),
	_T("DDR"),
	_T("IERA"),
	_T("IERB"),
	_T("IPRA"),
	_T("IPRB"),
	_T("ISRA"),
	_T("ISRB"),
	_T("IMRA"),
	_T("IMRB"),
	_T("VR"),
	_T("TACR"),
	_T("TBCR"),
	_T("TCDCR"),
	_T("TADR"),
	_T("TBDR"),
	_T("TCDR"),
	_T("TDDR"),
	_T("SCR"),
	_T("UCR"),
	_T("RSR"),
	_T("TSR"),
	_T("UDRI"),
	_T("UDRO"),
	_T("TIMERA"),
	_T("TIMERB"),
	_T("TIMERC"),
	_T("TIMERD"),
	NULL
};

bool MFP::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool MFP::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < MFP_REGS_END) {
		m_regs[reg_num] = data;
	} else if (reg_num < (MFP_REGS_END + 4)) {
		m_timer_counter[reg_num - MFP_REGS_END] = data;
	} else {
		return false;
	}
	return true;
}

static const _TCHAR *inames[] = {
	_T("IER"),_T("IPR"),_T("ISR"),_T("IMR"),NULL
};

void MFP::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("MFP\n"));
	UTILITY::sntprintf(buffer, buffer_len,
		_T(" %02X(GPDR):%02X %02X(AER):%02X %02X(DDR):%02X\n"),
		MFP_GPDR, m_regs[MFP_GPDR],MFP_AER,m_regs[MFP_AER],MFP_DDR,m_regs[MFP_DDR]);
	for(int i=0; i<4; i++) {
		int addr = MFP_IERA + i * 2;
		UTILITY::sntprintf(buffer, buffer_len,
			_T(" %02X(%sA):%02X %02X(%sB):%02X"),
			addr, inames[i], m_regs[addr], addr + 1, inames[i], m_regs[addr + 1]);
	}
	UTILITY::sntprintf(buffer, buffer_len,
		_T("\n %02X(VR):%02X %02X(TACR):%02X %02X(TBCR):%02X %02X(TCDCR):%02X %02X(TADR):%02X %02X(TBDR):%02X %02X(TCDR):%02X %02X(TDDR):%02X\n"),
		MFP_VR,m_regs[MFP_VR],MFP_TACR,m_regs[MFP_TACR],MFP_TBCR,m_regs[MFP_TBCR],MFP_TCDCR,m_regs[MFP_TCDCR],
		MFP_TADR,m_regs[MFP_TADR],MFP_TBDR,m_regs[MFP_TBDR],MFP_TCDR,m_regs[MFP_TCDR],MFP_TDDR,m_regs[MFP_TDDR]);
	UTILITY::sntprintf(buffer, buffer_len,
		_T(" %02X(SCR):%02X %02X(UCR):%02X %02X(RSR):%02X %02X(TSR):%02X %02X(UDR:in):%02X %02X(UDR:out):%02X\n"),
		MFP_SCR,m_regs[MFP_SCR],MFP_UCR,m_regs[MFP_UCR],MFP_RSR,m_regs[MFP_RSR],MFP_TSR,m_regs[MFP_TSR],MFP_UDRI,m_regs[MFP_UDRI],MFP_UDRO,m_regs[MFP_UDRO]);
	int n = MFP_REGS_END;
	UTILITY::sntprintf(buffer, buffer_len,
		_T("   Current Timer Value: %02X(TIMERA):%02X %02X(TIMERB):%02X %02X(TIMERC):%02X %02X(TIMERD):%02X\n"),
		n, m_timer_counter[0], n + 1, m_timer_counter[1], n + 2, m_timer_counter[2], n + 3, m_timer_counter[3]);
}
#endif
