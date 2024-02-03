/** @file scc.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ scc modoki (Z8530) ]
*/

#include "scc.h"
//#include "../emu.h"
#include "../../logging.h"
#include "../../fileio.h"
#include "../../utility.h"

//#define SCC_TX_BUFF_2BYTES

#ifdef _DEBUG
#define DEBUG_CH(ch) (ch == 0)
//#define DEBUG_CH(ch) (ch == 1)
//#define DEBUG_CH(ch) (true)
//#define OUT_DEBUG_REGW(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_REGW5(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGW5(...)
//#define OUT_DEBUG_REGW9(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGW9(...)
//#define OUT_DEBUG_REGR(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGR(...)
//#define OUT_DEBUG_RX(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_RX(...)
//#define OUT_DEBUG_TX(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_TX(...)
//#define OUT_DEBUG_INTR logging->out_debugf
#define OUT_DEBUG_INTR(...)
#else
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_REGW5(...)
#define OUT_DEBUG_REGW9(...)
#define OUT_DEBUG_REGR(...)
#define OUT_DEBUG_RX(...)
#define OUT_DEBUG_TX(...)
#define OUT_DEBUG_INTR(...)
#endif

void SCC::initialize()
{
	memset(m_regs, 0, sizeof(m_regs));
	m_cur_cmd = 0;

	m_base_clock = 5000000;	// 5MHz (X68K)
}

void SCC::reset()
{
	warm_reset(true);
}

void SCC::warm_reset(bool por)
{
	m_cur_cmd = 0;

	m_vector = 0;
	m_now_iack = false;

	if (!por) {
		for(int id=0; id<EVENT_SCC_END; id++) {
			cancel_my_event(id);
		}
	} else {
		for(int id=0; id<EVENT_SCC_END; id++) {
			m_register_id[id] = -1;
		}
	}

	m_regs[SCC_CH_COMMON][SCC_WR9] &= 0x03;

	for(int ch=0; ch<2; ch++) {
		channel_reset(ch);

		m_regs[ch][SCC_WR10] = 0;
		m_regs[ch][SCC_WR11] = 8;
		m_regs[ch][SCC_WR14] = 0x30 |  (m_regs[ch][SCC_WR14] & 0xc0);
	}

	update_intr();
}

void SCC::channel_reset(int ch)
{
	m_regs[ch][SCC_WR0] = 0;
	m_regs[ch][SCC_WR1] = 0;
	m_regs[ch][SCC_WR3] &= ~1;
	m_regs[ch][SCC_WR4] |= 0x04;
	m_regs[ch][SCC_WR5] &= ~0x9e;
	if (d_devices[ch]) {
		d_devices[ch]->write_signal(SIG_RTSA + ch, m_regs[ch][SCC_WR5] & WR5_RTS ? 0 : 1, 0xffffffff);
		d_devices[ch]->write_signal(SIG_DTRA + ch, m_regs[ch][SCC_WR5] & WR5_DTR ? 0 : 1, 0xffffffff);
	}
	m_regs[ch][SCC_WR10] &= ~0x9f;
	m_regs[ch][SCC_WR14] = 0x20 | (m_regs[ch][SCC_WR14] & 0xc3);
	m_regs[ch][SCC_WR15] = 0xf8;
	m_regs[ch][SCC_RR0] = 0x44 | (m_regs[ch][SCC_RR0] & 0xfc);
	m_regs[ch][SCC_RR1] = 0x06 | (m_regs[ch][SCC_RR1] & 0x01);
	m_regs[ch][SCC_RR10] &= ~0xbf;

	if (ch == SCC_CH_A) {
		m_regs[SCC_CH_COMMON][SCC_RR3] &= ~0xf8;
		m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~0xf8;
	} else {
		m_regs[SCC_CH_COMMON][SCC_RR3] &= ~0xc7;
		m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~0xc7;
	}

	memset(m_stat[ch].recv_buff, 0, sizeof(m_stat[ch].recv_buff));
	m_stat[ch].recv_wpos = 0;
	memset(m_stat[ch].xmit_buff, 0, sizeof(m_stat[ch].xmit_buff));
	m_stat[ch].xmit_wpos = 0;
	m_stat[ch].flags = 0;
}

// ----------------------------------------------------------------------------

void SCC::register_my_event(int id, double usec)
{
	register_event(this, id, usec, false, &m_register_id[id]);
}

void SCC::cancel_my_event(int id)
{
	if (m_register_id[id] != -1) cancel_event(this, m_register_id[id]);
	m_register_id[id] = -1;
}

// ----------------------------------------------------------------------------

void SCC::write_io8(uint32_t addr, uint32_t data)
{
	if (now_reset) return;

	int ch = 1 - ((addr & 3) >> 1);

	switch(addr & 1) {
	case 1:
		// set data
		write_to_xmit_data(ch, data);

		break;
	default:
		// set command
		OUT_DEBUG_REGW(ch, _T("%06X SCC %c REGW cmd:%02d data:%02X RR0:%02X"), (addr << 1) | 1, ch + 0x41, m_cur_cmd, data, m_regs[ch][SCC_RR0]);

		switch(m_cur_cmd) {
		case 0:
			// set to WR0
			m_regs[ch][SCC_WR0] = data;
			if ((data & WR0_CLEAR_U) == 0) {
				m_cur_cmd = data & WR0_REG_SEL;
			} else {
				process_wr0_cmd(ch, data);
			}
			return;	// go out current position

		case 1:
			// set to WR1
			m_regs[ch][m_cur_cmd] = data;
			if (!(data & WR1_EX_IE)) {
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_EXT_IP >> (ch * 3));
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_EXT_IP >> (ch * 3));
			}

			if (!(data & WR1_TX_IE)) {
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_TX_IP >> (ch * 3));
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_TX_IP >> (ch * 3));
			} else {
				update_intr_flags_for_tx(ch);
			}

			if (!(data & WR1_RX_IE)) {
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_RX_IP >> (ch * 3));
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_RX_IP >> (ch * 3));
			} else {
				// interrupt
				update_intr_flags_for_rx(ch);
			}

			// update interrupt
			update_intr();
			break;
		case 2:
			// set to WR2 (always use channel A)
			m_regs[SCC_CH_COMMON][m_cur_cmd] = data;
			break;
		case 3:
			// set to WR3
			m_regs[ch][m_cur_cmd] = data;
			if (!(data & WR3_RX_EN)) {
				// disable receive operation
				m_regs[ch][SCC_RR0] &= ~RR0_RXC_AVAIL;
				m_regs[ch][SCC_RR1] &= ~(RR1_PARITYERR | RR1_RX_OVRERR | RR1_CRC_FRERR);
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_RX_IP >> (ch * 3));
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_RX_IP >> (ch * 3));
				// update interrupt
				update_intr();
			}
			break;
		case 5:
			// set to WR5
			m_regs[ch][m_cur_cmd] = data;
			if (!(data & WR5_TX_EN)) {
				// disable transmit operation
				m_regs[ch][SCC_RR0] |= RR0_TXB_EMPTY;
				m_regs[ch][SCC_RR0] &= ~RR0_TX_UN_EOM;
				m_regs[ch][SCC_RR1] |= RR1_ALL_SENT;
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_TX_IP >> (ch * 3));
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_TX_IP >> (ch * 3));
				// update interrupt
				update_intr();
			}
			if (d_devices[ch]) {
				// change signals
				OUT_DEBUG_REGW5(ch, _T("%06X SCC %c REGW5 data:%02X RTS:%d"), (addr << 1) | 1, ch + 0x41, data, data & WR5_RTS ? 0 : 1);

				d_devices[ch]->write_signal(SIG_RTSA + ch, data & WR5_RTS ? 0 : 1, 0xffffffff);
				d_devices[ch]->write_signal(SIG_DTRA + ch, data & WR5_DTR ? 0 : 1, 0xffffffff);
			}
			break;
		case 8:
			// set to WR8 data
			write_to_xmit_data(ch, data);

			break;
		case 9:
			// set to WR9 (always use channel A)
			m_regs[SCC_CH_COMMON][m_cur_cmd] = data;
			OUT_DEBUG_REGW9(ch, _T("%06X SCC %c REGW9 data:%02X"), (addr << 1) | 1, ch + 0x41, data);
			switch (data & 0xc0) {
			case 0xc0:
				// reset
				warm_reset(false);
//				m_regs[SCC_CH_COMMON][SCC_WR9] = (data & 0x1c);
				break;
			case 0x80:
				// reset channel A
				channel_reset(0);
				break;
			case 0x40:
				// reset channel B
				channel_reset(1);
				break;
			}
			// MIE
			if (!(data & WR9_MIE)) {
				// clear interrupt
				m_regs[SCC_CH_COMMON][SCC_RR3] = 0;
				m_regs[SCC_CH_HIDDEN][SCC_IUS] = 0;
			} else {
				update_intr_flags_for_tx(ch);
				update_intr_flags_for_rx(ch);
			}
			// update interrupt
			update_intr();
			break;
		default:
			// set to WRx
			m_regs[ch][m_cur_cmd] = data;
			break;
		}
		m_cur_cmd = 0;
		break;
	}
}

uint32_t SCC::read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	if (m_now_iack) {
		// set vector number
		update_intr_vector();
		// vector number
		data = m_vector;
		// clear interrupt
		update_intr();
		return data;
	}

	int ch = 1 - ((addr & 3) >> 1);

	switch(addr & 1) {
	case 1:
		// data
		data = read_received_data(ch);

		break;
	default:
		switch(m_cur_cmd) {
		case 0:
			// status
			data = m_regs[ch][SCC_RR0];
			break;
		case 1:
			// status 1
			data = m_regs[ch][SCC_RR1];
			break;
		case 2:
			// interrupt
			if (ch == 0) {
				// channel A
				data = m_regs[ch][SCC_WR2];
			} else {
				// channel B
				data = m_regs[ch][SCC_RR2];
			}
			break;
		case 3:
			// interrupt reason
			if (ch == 0) {
				// channel A
				data = m_regs[ch][SCC_RR3];
			} else {
				// channel B
				data = 0;
			}
			break;
		case 8:
			// data
			data = read_received_data(ch);

			break;
		case 10:
			// status on FM mode
			data = m_regs[ch][SCC_RR10];
			break;
		default:
			// return the data of WRx
			data = m_regs[ch][m_cur_cmd];
			break;
		}

		OUT_DEBUG_REGR(ch, _T("%06X SCC %c REGR cmd:%02d data:%02X"), (addr << 1) | 1, ch + 0x41, m_cur_cmd, data);

		m_cur_cmd = 0;
		break;
	}
	return data;
}

void SCC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
	case SIG_RXDA:
	case SIG_RXDB:
		// receive serial input
		receive_data(id - SIG_RXDA, data, mask);
		break;
	case SIG_IACK:
		// receive interrupt ack signal
		m_now_iack = ((data & mask) != 0);
		OUT_DEBUG_INTR(_T("SCC IACK %d"), m_now_iack ? 1 : 0);
		break;
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		if (!(data & mask)) {
			warm_reset(false);
		}
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void SCC::process_wr0_cmd(int ch, uint8_t cmd)
{
	int imask;

	switch(cmd & 0x38) {
	case 0x10:
		// clear ex interrupt
		m_regs[SCC_CH_COMMON][SCC_RR3] &= ~(RR3_CA_EXT_IP >> (ch * 3));
		m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~(RR3_CA_EXT_IP >> (ch * 3));
		// update interrupt
		update_intr();
		break;
	case 0x18:
		// send abort
		m_regs[ch][SCC_RR0] = RR0_TXB_EMPTY | RR0_TX_UN_EOM;
		break;
	case 0x20:
		// next Rx interrupt enable
		m_stat[ch].flags &= ~FIRST_RECEIVED;
		break;
	case 0x28:
		// clear Tx empty flag
		imask = 0x10 >> (ch * 3);
		m_regs[SCC_CH_COMMON][SCC_RR3] &= ~imask;
		// update interrupt
		update_intr();
		break;
	case 0x30:
		// clear Rx error flag
		m_regs[ch][SCC_RR1] &= ~RR1_RX_ERRALL;
		// clear interrupt flag
		imask = 0x20 >> (ch * 3);
		m_regs[SCC_CH_COMMON][SCC_RR3] &= ~imask;
		// update rx interrupt flag
		update_intr_flags_for_rx(ch);
		// update interrupt
		update_intr();
		break;
	case 0x38:
		// clear first interrupt flag on IUS
		imask = 0x20;
		for(int i=5; i>=0; --i) {
			if (m_regs[SCC_CH_HIDDEN][SCC_IUS] & imask) {
				m_regs[SCC_CH_HIDDEN][SCC_IUS] &= ~imask;
				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~imask;
				break;
			}
			imask >>= 1;
		}
		// update rx interrupt flag
		update_intr_flags_for_rx(ch);
		// update interrupt
		update_intr();
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

double SCC::get_send_speed_usec(int ch, bool is_byte)
{
	double usec;
	int baud = 1 << ((m_regs[ch][SCC_WR4] & WR4_CLK_MODE) >> 4);	// 00:x1 01:x16 10:x32 11:x64
	if ((m_regs[ch][SCC_WR11] & WR11_TX_CLK) == WR11_TX_CLK_BAUDG) {
		// use baud generator
		baud *= (((int)m_regs[ch][SCC_WR13] << 8 | m_regs[ch][SCC_WR12]) + 2) * 2; 
		usec = (double)baud * 1000000 / m_base_clock;
	} else {
		// TODO: DPLL, /TRxC, /RTxC are not supported
		usec = (double)1000000.0 / 9600.0;
	}
	if (is_byte) {
		// to transmit bits
		int bits = 1;	// start bit
		switch (m_regs[ch][SCC_WR5] & WR5_TX_CHAR_BITS) {
		case 0x00:
			bits += 5;
			break;
		case 0x20:
			bits += 7;
			break;
		case 0x40:
			bits += 6;
			break;
		default:
			bits += 8;
			break;
		}
		// parity bit
		if (m_regs[ch][SCC_WR4] & WR4_PARITY_EN) bits++;
		// stop bits
		switch (m_regs[ch][SCC_WR4] & WR4_STOP_BITS) {
		case 0x00:
			bits += 0;
			break;
		case 0x40:
			bits += 1;
			break;
		case 0x80:
			bits += 2;	// 1.5 stop bits
			break;
		default:
			bits += 2;
			break;
		}
		usec *= (double)bits;
	}
	return usec;
}

double SCC::get_recv_speed_usec(int ch, bool is_byte)
{
	double usec;
	int baud = 1 << ((m_regs[ch][SCC_WR4] & WR4_CLK_MODE) >> 4);	// 00:x1 01:x16 10:x32 11:x64
	if ((m_regs[ch][SCC_WR11] & WR11_RX_CLK) == WR11_RX_CLK_BAUDG) {
		// use baud generator
		baud *= (((int)m_regs[ch][SCC_WR13] << 8 | m_regs[ch][SCC_WR12]) + 2) * 2; 
		usec = (double)baud * 1000000 / m_base_clock;
	} else {
		// TODO: DPLL, /TRxC, /RTxC are not supported
		usec = (double)1000000.0 / 9600.0;
	}
	if (is_byte) {
		// to transmit bits
		int bits = 1;	// start bit
		switch (m_regs[ch][SCC_WR3] & WR3_RX_CHAR_BITS) {
		case 0x00:
			bits += 5;
			break;
		case 0x40:
			bits += 7;
			break;
		case 0x80:
			bits += 6;
			break;
		default:
			bits += 8;
			break;
		}
		// parity bit
		if (m_regs[ch][SCC_WR4] & WR4_PARITY_EN) bits++;
		// stop bits
		switch (m_regs[ch][SCC_WR4] & WR4_STOP_BITS) {
		case 0x00:
			bits += 0;
			break;
		case 0x40:
			bits += 1;
			break;
		case 0x80:
			bits += 2;	// 1.5 stop bits
			break;
		default:
			bits += 2;
			break;
		}
		usec *= (double)bits;
	}
	return usec;
}

void SCC::start_xmit(int ch)
{
	// data filled so start transmit process
	if (m_stat[ch].xmit_wpos) {
		cancel_my_event(EVENT_SCC_TXDA + ch);
		register_my_event(EVENT_SCC_TXDA + ch, get_send_speed_usec(ch, true));
	}
}

void SCC::write_to_xmit_data(int ch, uint8_t data)
{
	OUT_DEBUG_TX(ch, _T("SCC %c WRITE DATA:%02X RR0:%02X"), ch + 0x41, data, m_regs[ch][SCC_RR0]);

	if (m_regs[ch][SCC_RR0] & RR0_TXB_EMPTY) {
		m_regs[ch][SCC_WR8] = data;
		m_regs[ch][SCC_RR0] &= ~RR0_TXB_EMPTY;

		load_xmit_data_from_reg(ch);

	} else {
		// buffer is full
		m_regs[ch][SCC_RR0] |= RR0_TX_UN_EOM;

		OUT_DEBUG_TX(ch, _T("SCC %c TX underrun data:%02X RR0:%02X"), ch + 0x41, m_regs[ch][SCC_WR8], m_regs[ch][SCC_RR0]);

		// interrupt
		update_intr_for_tx(ch);

	}
}

void SCC::load_xmit_data_from_reg(int ch)
{
	if (m_stat[ch].xmit_wpos < XMIT_BUFF_SIZE) {
		// push to buffer
		push_xmit_buff(ch, m_regs[ch][SCC_WR8]);

		start_xmit(ch);

#ifdef SCC_TX_BUFF_2BYTES
		m_regs[ch][SCC_RR0] |= RR0_TXB_EMPTY;

		// interrupt
		update_intr_for_tx(ch);
#endif
	}
}

// ----------------------------------------------------------------------------

void SCC::receive_data(int ch, uint32_t data, uint32_t mask)
{
	if (m_stat[ch].recv_wpos < RECV_BUFF_SIZE) {
		// push to buffer
		push_recv_buff(ch, data & mask);

		OUT_DEBUG_RX(ch, _T("clk:%lld SCC %c RX push buffer data:%02X mask:%02X RR0:%02X")
			, get_current_clock()
			, ch + 0x41, data, mask, m_regs[ch][SCC_RR0]);

		// store to register RR8
		if ((m_regs[ch][SCC_RR0] & RR0_RXC_AVAIL) == 0) {
			store_received_data_to_reg(ch);
		}

	} else {
		// buffer overrun
		m_regs[ch][SCC_RR1] |= RR1_RX_OVRERR;

		OUT_DEBUG_RX(ch, _T("clk:%lld SCC %c RX overrun data:%02X mask:%02X RR0:%02X")
			, get_current_clock()
			, ch + 0x41, data, mask, m_regs[ch][SCC_RR0]);

		// interrupt
		if ((m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE) != 0) {
			switch(m_regs[ch][SCC_WR1] & WR1_RX_IE) {
			case WR1_RX_IE_AC:
				if (m_stat[ch].flags & FIRST_RECEIVED) {
					// already received the first data 
					break;
				}
				// [[ fall through ]];
			case WR1_RX_IE_SP:
			case WR1_RX_IE_ALL:
				// set pending register
				m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_RX_IP >> (ch * 3));
				// first data
				m_stat[ch].flags |= FIRST_RECEIVED;
				// interrupt
				assert_intr();

				break;
			}
		}
	}
}

uint8_t SCC::read_received_data(int ch)
{
	uint8_t data = m_regs[ch][SCC_RR8];
	m_regs[ch][SCC_RR0] &= ~RR0_RXC_AVAIL;

	OUT_DEBUG_REGR(ch, _T("clk:%lld SCC %c READ DATA:%02X")
		, get_current_clock()
		, ch + 0x41, data);

	if (m_stat[ch].recv_wpos) {
		// data exists in buffer
		store_received_data_to_reg(ch);
	}

	return data;
}

void SCC::store_received_data_to_reg(int ch)
{
	// receive data
	m_regs[ch][SCC_RR8] = shift_recv_buff(ch);
	m_regs[ch][SCC_RR0] |= RR0_RXC_AVAIL;
	m_regs[ch][SCC_RR1] &= ~(RR1_PARITYERR | RR1_RX_OVRERR | RR1_CRC_FRERR);

	OUT_DEBUG_RX(ch, _T("clk:%lld SCC %c RX next data:%02X RR0:%02X MIE:%02X RXIE:%02X")
		, get_current_clock()
		, ch + 0x41, m_regs[ch][SCC_RR8], m_regs[ch][SCC_RR0]
		, (m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE)
		, m_regs[ch][SCC_WR1] & WR1_RX_IE
	);

	// interrupt
	if ((m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE) != 0) {
		switch(m_regs[ch][SCC_WR1] & WR1_RX_IE) {
		case WR1_RX_IE_AC:
			if (m_stat[ch].flags & FIRST_RECEIVED) {
				// already received the first data 
				break;
			}
			// [[ fall through ]];
		case WR1_RX_IE_ALL:
			// set pending register
			m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_RX_IP >> (ch * 3));
			// first data
			m_stat[ch].flags |= FIRST_RECEIVED;
			// interrupt
			assert_intr();

			break;
		}
	}
}

void SCC::push_recv_buff(int ch, uint8_t data)
{
	m_stat[ch].recv_buff[m_stat[ch].recv_wpos++] = data;
}

void SCC::push_xmit_buff(int ch, uint8_t data)
{
	m_stat[ch].xmit_buff[m_stat[ch].xmit_wpos++] = data;
}

uint8_t SCC::shift_recv_buff(int ch)
{
	uint8_t data = m_stat[ch].recv_buff[0];
	for(int i=0; i<RECV_BUFF_SIZE; i++) {
		m_stat[ch].recv_buff[i] = m_stat[ch].recv_buff[i + 1];
	}
	if (m_stat[ch].recv_wpos) m_stat[ch].recv_wpos--;
	return data;
}

uint8_t SCC::shift_xmit_buff(int ch)
{
	uint8_t data = m_stat[ch].xmit_buff[0];
	for(int i=0; i<XMIT_BUFF_SIZE; i++) {
		m_stat[ch].xmit_buff[i] = m_stat[ch].xmit_buff[i + 1];
	}
	if (m_stat[ch].xmit_wpos) m_stat[ch].xmit_wpos--;
	return data;
}

void SCC::update_intr_flags_for_rx(int ch)
{
	if ((m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE) == 0) return;

	switch(m_regs[ch][SCC_WR1] & WR1_RX_IE) {
	case WR1_RX_IE_SP:
		if ((m_regs[ch][SCC_RR1] & RR1_RX_OVRERR) != 0
		|| ((m_regs[ch][SCC_WR1] & WR1_PE_IE) != 0 && (m_regs[ch][SCC_RR1] & RR1_PARITYERR) != 0)) {
			// set pending register
			m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_RX_IP >> (ch * 3));
		}
		break;
	case WR1_RX_IE_ALL:
	case WR1_RX_IE_AC:
		if ((m_regs[ch][SCC_RR0] & RR0_RXC_AVAIL) != 0
		||  (m_regs[ch][SCC_RR1] & RR1_RX_OVRERR) != 0
		|| ((m_regs[ch][SCC_WR1] & WR1_PE_IE) != 0 && (m_regs[ch][SCC_RR1] & RR1_PARITYERR) != 0)) {
			// set pending register
			m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_RX_IP >> (ch * 3));
		}
		break;
	}
}

void SCC::update_intr_flags_for_tx(int ch)
{
	if ((m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE) == 0) return;

	if ((m_regs[ch][SCC_WR1] & WR1_TX_IE) != 0 && (m_regs[ch][SCC_RR0] & RR0_TXB_EMPTY) != 0) {
		// tramsmit set pending register
		m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_TX_IP >> (ch * 3));
	}
}

void SCC::update_intr_for_tx(int ch)
{
	if ((m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_MIE) != 0	// master interrupt enable
		&& (m_regs[ch][SCC_WR1] & WR1_TX_IE) != 0)	// transmit interrupt enable
	{
		// set pending register
		m_regs[SCC_CH_COMMON][SCC_RR3] |= (RR3_CA_TX_IP >> (ch * 3));
		// interrupt
		assert_intr();
	}
}

void SCC::assert_intr()
{
	uint32_t flag = (m_regs[SCC_CH_COMMON][SCC_RR3] ^ m_regs[SCC_CH_HIDDEN][SCC_IUS]) ? 0xffffffff : 0;

	OUT_DEBUG_INTR(_T("SCC ASSERT INTR RR3:%02X IUS:%02X flag:%d"), m_regs[SCC_CH_COMMON][SCC_RR3], m_regs[SCC_CH_HIDDEN][SCC_IUS], -(int)flag);

	write_signals(&outputs_irq, flag);
}

void SCC::update_intr()
{
	uint32_t flag = (m_regs[SCC_CH_COMMON][SCC_RR3] ^ m_regs[SCC_CH_HIDDEN][SCC_IUS]) ? 0xffffffff : 0;

	write_signals(&outputs_irq, flag);

	OUT_DEBUG_INTR(_T("SCC UPDATE INTR RR3:%02X IUS:%02X flag:%d"), m_regs[SCC_CH_COMMON][SCC_RR3], m_regs[SCC_CH_HIDDEN][SCC_IUS], -(int)flag);
}

const uint8_t SCC::c_intr_causes[] = {
	0x01,	// CB ex 
	0x00,	// CB tx buffer is empty
	0x02,	// CB rx char active
	0x05,	// CA ex
	0x04,	// CA tx buffer is empty
	0x06,	// CA rx char active

	0x07,	// CA error 
	0x03,	// CB error 
};

void SCC::update_intr_vector()
{
	// now processing interrupt acknowledge
	if (m_regs[SCC_CH_HIDDEN][SCC_IUS]) {
		return;
	}

	// select current interrupt
	uint8_t ipr = m_regs[SCC_CH_COMMON][SCC_RR3] & 0x3f; 
	if (ipr) {
		// interrupt vector signal
		uint8_t imask = RR3_CA_RX_IP;
		for(int i=5; i>=0; --i) {
			if (ipr & imask) {
				int ch = (i < 3 ? SCC_CH_B : SCC_CH_A);
				if (m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_VIS) {
					// calcrate vector number
					int num = i;
					if ((imask & (RR3_CA_RX_IP | RR3_CB_RX_IP)) != 0 && (m_regs[ch][SCC_RR1] & (RR1_PARITYERR | RR1_RX_OVRERR | RR1_CRC_FRERR)) != 0) {
						num = 6 + ch;
					}
					if (m_regs[SCC_CH_COMMON][SCC_WR9] & WR9_STS) {
						// bit4-6
						m_vector = (m_regs[SCC_CH_COMMON][SCC_WR2] & 0x8f) | (c_intr_causes[num] << 4);
					} else {
						// bit1-3
						m_vector = (m_regs[SCC_CH_COMMON][SCC_WR2] & 0xf1) | (c_intr_causes[num] << 1);
					}
				} else {
					m_vector = m_regs[SCC_CH_COMMON][SCC_WR2];
				}
				// IUS set
				m_regs[SCC_CH_HIDDEN][SCC_IUS] |= imask;
//				// IP reset
//				m_regs[SCC_CH_COMMON][SCC_RR3] &= ~imask;
				break;
			}
			imask >>= 1;
		}
	}

	OUT_DEBUG_INTR(_T("SCC UPDATE VECT RR3:%02X IUS:%02X vector:%02X"), m_regs[SCC_CH_COMMON][SCC_RR3], m_regs[SCC_CH_HIDDEN][SCC_IUS], m_vector);
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void SCC::event_frame()
{
}

void SCC::event_callback(int event_id, int err)
{
	int ch;
	uint8_t data;
//	logging->out_debugf("scc event_callback %d",event_id);

	switch(event_id) {
	case EVENT_SCC_TXDA:
	case EVENT_SCC_TXDB:
		// transmit finished
		m_register_id[event_id] = -1;
		ch = event_id - EVENT_SCC_TXDA;
		// send data
		data = shift_xmit_buff(ch);
#ifdef SCC_TX_BUFF_2BYTES
		if (!(m_regs[ch][SCC_RR0] & RR0_TXB_EMPTY)) {
			load_xmit_data_from_reg(ch);
		}

		if (d_devices[ch]) {
			d_devices[ch]->write_signal(SCC::SIG_TXDA + ch, data, 0xff);
		}
#else
		if (d_devices[ch]) {
			d_devices[ch]->write_signal(SCC::SIG_TXDA + ch, data, 0xff);
		}
		m_regs[ch][SCC_RR0] |= RR0_TXB_EMPTY;

		// interrupt
		update_intr_for_tx(ch);
#endif
		OUT_DEBUG_TX(ch, _T("SCC %c TX sent data:%02X RR0:%02X"), ch + 0x41, data, m_regs[ch][SCC_RR0]);
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

void SCC::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);	// version
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int ch=0; ch<2; ch++) {
		for(int i=0; i<SCC_REGS_END; i++) {
			SET_Byte(m_regs[ch][i]);
		}
	}
	SET_Int32_LE(m_cur_cmd);		///< set command number to WR0

	SET_Int32_LE(m_base_clock);	///< base clock
	SET_Byte(m_vector);	///< interrupt vector
	SET_Bool(m_now_iack);	///< receiving IACK

	for(int i=0; i<2; i++) {
		vm_state.v2.m_stat[i].flags = m_stat[i].flags;
		memcpy(vm_state.v2.m_stat[i].recv_buff, m_stat[i].recv_buff, sizeof(vm_state.v2.m_stat[i].recv_buff));
		vm_state.v2.m_stat[i].recv_wpos = m_stat[i].recv_wpos;
		memcpy(vm_state.v2.m_stat[i].xmit_buff, m_stat[i].xmit_buff, sizeof(vm_state.v2.m_stat[i].xmit_buff));
		vm_state.v2.m_stat[i].xmit_wpos = m_stat[i].xmit_wpos;
	}
	for(int i=0; i<EVENT_SCC_END; i++) {
		vm_state.v2.m_register_id[i] = Int32_LE(m_register_id[i]);	///< interrupt after sent data
	}

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

bool SCC::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	for(int ch=0; ch<2; ch++) {
		for(int i=0; i<SCC_REGS_END; i++) {
			GET_Byte(m_regs[ch][i]);
		}
	}
	GET_Int32_LE(m_cur_cmd);		///< set command number to WR0

	GET_Int32_LE(m_base_clock);	///< base clock
	GET_Byte(m_vector);	///< interrupt vector
	GET_Bool(m_now_iack);	///< receiving IACK

	switch(Uint16_LE(vm_state_i.version)) {
	case 1: // version 1
		for(int i=0; i<2; i++) {
			m_stat[i].flags = (vm_state.v1.m_stat[i].first_received) ? FIRST_RECEIVED : 0;	///< first received data
			memset(m_stat[i].recv_buff, 0, sizeof(m_stat[i].recv_buff));
			m_stat[i].recv_wpos = 0;
			memset(m_stat[i].xmit_buff, 0, sizeof(m_stat[i].xmit_buff));
			m_stat[i].xmit_wpos = 0;
		}
		for(int i=0; i<EVENT_SCC_END; i++) {
			m_register_id[i] = Int32_LE(vm_state.v1.m_register_id[i]);	///< interrupt after sent data
		}
		break;
	default: // version 2
		for(int i=0; i<2; i++) {
			m_stat[i].flags = vm_state.v2.m_stat[i].flags;
			memcpy(m_stat[i].recv_buff, vm_state.v2.m_stat[i].recv_buff, sizeof(m_stat[i].recv_buff));
			m_stat[i].recv_wpos = vm_state.v2.m_stat[i].recv_wpos;
			memcpy(m_stat[i].xmit_buff, vm_state.v2.m_stat[i].xmit_buff, sizeof(m_stat[i].xmit_buff));
			m_stat[i].xmit_wpos = vm_state.v2.m_stat[i].xmit_wpos;
		}
		for(int i=0; i<EVENT_SCC_END; i++) {
			m_register_id[i] = Int32_LE(vm_state.v2.m_register_id[i]);	///< interrupt after sent data
		}
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t SCC::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;

	if (now_reset) return data;

	int port = 1 - ((addr & 3) >> 1);

	switch(addr & 1) {
	case 1:
		data = m_regs[port][SCC_RR8];
		break;
	default:
		switch(m_cur_cmd) {
		case 0:
			// status
			data = m_regs[port][SCC_RR0];
			break;
		case 1:
			// status 1
			data = m_regs[port][SCC_RR1];
			break;
		case 2:
			// interrupt
			if (port == 0) {
				// channel A
				data = m_regs[port][SCC_WR2];
			} else {
				// channel B
				data = m_regs[port][SCC_RR2];
			}
			break;
		case 3:
			// interrupt reason
			if (port == 0) {
				// channel A
				data = m_regs[port][SCC_RR3];
			} else {
				// channel B
				data = 0;
			}
			break;
		case 8:
			// data
			data = m_regs[port][SCC_RR8];
			break;
		case 10:
			// status on FM mode
			data = m_regs[port][SCC_RR10];
			break;
		default:
			// return the data of WRx
			data = m_regs[port][m_cur_cmd];
			break;
		}
		break;
	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("WR0"),
	_T("WR1"),
	_T("WR2"),	// A only
	_T("WR3"),
	_T("WR4"),
	_T("WR5"),
	_T("WR6"),
	_T("WR7"),
	_T("WR8"),
	_T("WR9"),	// A only
	_T("WR10"),
	_T("WR11"),
	_T("WR12"),
	_T("WR13"),
	_T("WR14"),
	_T("WR15"),
	_T("RR0"),
	_T("RR1"),
	_T("RR2"),
	_T("RR3"),	// A only
	_T("RR8"),
	_T("RR10"),
	NULL
};

bool SCC::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool SCC::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num == 512) {
		// IUS
		m_regs[SCC_CH_HIDDEN][SCC_IUS] = data;
		return true;
	} else if (reg_num >= 256 && reg_num < (256 + SCC_REGS_END)) {
		// port B
		reg_num -= 256;
		if (!(reg_num == 2 || reg_num == 9 || reg_num == 19)) {
			m_regs[SCC_CH_B][reg_num] = data;
			return true;
		}
	} else if (reg_num < SCC_REGS_END) {
		// port A
		m_regs[SCC_CH_A][reg_num] = data;
		return true;
	}
	return false;
}

void SCC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	for(int port=0; port<2; port++) {
		UTILITY::sntprintf(buffer, buffer_len,_T("Port %c\n"), port + 0x41);
		for(int i=0; i<SCC_REGS_END; i++) {
			if (port == 1 && (i == 2 || i == 9 || i == 19)) {
				UTILITY::tcscat(buffer, buffer_len,
					_T("             "));
			} else {
				UTILITY::sntprintf(buffer, buffer_len,
					_T(" %03X(%-4s):%02X"), i + port * 256, c_reg_names[i], m_regs[port][i]);
			}
			if ((i & 3) == 3) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		}
		UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
	UTILITY::sntprintf(buffer, buffer_len,
		_T(" %03X(IUS ):%02X\n"), 512, m_regs[SCC_CH_HIDDEN][SCC_IUS]);
}
#endif
