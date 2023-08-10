/** @file dmac.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22-

	[ HD63450 modoki (DMAC) ]
*/

#include "dmac.h"
#include "../../fileio.h"
#if defined(USE_DEBUGGER) || defined(_DEBUG)
#include "../../logging.h"
#include "../debugger.h"
#include "../../utility.h"
#endif

#ifdef _DEBUG
//#define DEBUG_CH(ch) (ch == 0)
//#define DEBUG_CH(ch) (ch == 2)
//#define DEBUG_CH(ch) (ch == 3)
//#define OUT_DEBUG_REQ(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REQ(...)
//#define OUT_DEBUG_TRANS(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_TRANS(...)
//#define OUT_DEBUG_REGW(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_REGR(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REGR(...)
//#define OUT_DEBUG_RES(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_RES(...)
//#define OUT_DEBUG_CNT(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_CNT(...)
//#define OUT_DEBUG_ARR(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_ARR(...)
//#define OUT_DEBUG_LNKARR(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_LNKARR(...)
//#define OUT_DEBUG_IACK(ch, ...) if (DEBUG_CH(ch)) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_IACK(...)
#else
#define OUT_DEBUG_REQ(...)
#define OUT_DEBUG_TRANS(...)
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_REGR(...)
#define OUT_DEBUG_RES(...)
#define OUT_DEBUG_CNT(...)
#define OUT_DEBUG_ARR(...)
#define OUT_DEBUG_LNKARR(...)
#define OUT_DEBUG_IACK(...)
#endif

void DMAC::initialize()
{
	for(int i=0; i<END_OF_EVENT_IDS; i++) {
		m_register_id[i] = -1;
	}
#ifdef USE_DEBUGGER
	if(d_debugger) {
		for(int ch=0; ch<4; ch++) {
			m_dma[ch].d_dbg = new DEBUGGER_BUS(vm, emu, NULL);
			m_dma[ch].d_dbg->set_namef(_T("DMAC CH%d"), ch);
			m_dma[ch].d_dbg->set_parent(d_debugger);
			m_dma[ch].d_dbg->set_context_mem(d_memory);
			m_dma[ch].d_dbg->set_context_io(m_dma[ch].device);
		}
	}
#endif
}

void DMAC::release()
{
	// m_dma[ch].d_dbg are released automatically
//#ifdef USE_DEBUGGER
//	delete d_memory_stored;
//	d_memory_stored = NULL;
//#endif
}

void DMAC::reset()
{
	warm_reset(true);
}

void DMAC::warm_reset(bool por)
{
	for(int i=0; i<4; i++) {
		m_dma[i].csr = 0;
		m_dma[i].cer = 0;
		m_dma[i].dcr = 0;
		m_dma[i].ocr = 0;
		m_dma[i].scr = 0;
		m_dma[i].ccr = 0;
		m_dma[i].mtc = 0;
		m_dma[i].btc = 0;
		m_dma[i].mar = 0;
		m_dma[i].dar = 0;
		m_dma[i].bar = 0;
		m_dma[i].niv = 0;
		m_dma[i].eiv = 0;
		m_dma[i].cpr = 0;
		m_dma[i].mfc = 0;
		m_dma[i].dfc = 0;
		m_dma[i].bfc = 0;

		m_dma[i].pack_data = 0;
		m_dma[i].pack_width = 0;

		m_dma[i].next_pointer = 0;
		m_dma[i].next_clock = 0;
	}
	m_gcr = 0;

	m_busreq = 0;
	m_interrupt = 0;
	m_now_iack = false;

	for(int i=0; i<END_OF_EVENT_IDS; i++) {
		if (por) m_register_id[i] = -1;
		else cancel_my_event(i);
	}
}

void DMAC::write_io8(uint32_t addr, uint32_t data)
{
	write_io_n(addr, data, 1);
}

void DMAC::write_io16(uint32_t addr, uint32_t data)
{
	write_io_n(addr, data, 2);
}

/// @param[in] addr  : address
/// @param[in] data  : data
/// @param[in] width : bus width 1:byte 2:word
///
/// @note The data b15-b8 is always at even address, and data b7-b0 is at odd address.
void DMAC::write_io_n(uint32_t addr, uint32_t data, int width)
{
	int channel = (addr >> 6) & 0x3;
	struct st_dma_regs *dma = &m_dma[channel];

	if ((addr & 0xfe) == 0xfe) {
		// General Control Register
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W GCR ch:%d data:%04X width:%d"), addr, channel, data, width);

		m_gcr = (data & GCR_MASK);
		return;
	}

	switch(addr & 0x3e) {
	case 0x00:
		// Channel Status Register (A0)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W CSR ch:%d data:%04X width:%d"), addr, channel, data, width);

		// set "1" the each bits to clear this.
		// except ACT, PCS
		data >>= 8;
		dma->csr &= ~(data & CSR_MASK);

		if (data & CSR_ERR) {
			dma->cer = 0;
		}

		break;
	case 0x04:
		if (width == 1) {
			if (addr & 1) {
				// Operation Control Register (A5)
				OUT_DEBUG_REGW(channel, _T("DMAC %08X W OCR ch:%d data:%04X width:%d"), addr, channel, data, width);

				dma->ocr = data;
			} else {
				// Device Control Register (A4)
				OUT_DEBUG_REGW(channel, _T("DMAC %08X W DCR ch:%d data:%04X width:%d"), addr, channel, data, width);

				data >>= 8;
				dma->dcr = (data & DCR_MASK);
			}
		} else {
			// Operation Control Register (A5)
			OUT_DEBUG_REGW(channel, _T("DMAC %08X W DCR.OCR ch:%d data:%04X width:%d"), addr, channel, data, width);

			dma->ocr = data;

			// Device Control Register (A4)
			data >>= 8;
			dma->dcr = (data & DCR_MASK);
		}
		break;
	case 0x06:
		{
			uint8_t prev = dma->ccr;
			if (width == 1) {
				if (addr & 1) {
					// Channel Control Register (A7)
					OUT_DEBUG_REGW(channel, _T("DMAC %08X W CCR ch:%d data:%04X width:%d"), addr, channel, data, width);

					dma->ccr |= (data & CCR_OPERATE_MASK);
					dma->ccr = (data & CCR_TOGGLE_MASK) | (dma->ccr & CCR_OPERATE_MASK);
				} else {
					// Sequence Control Register (A6)
					OUT_DEBUG_REGW(channel, _T("DMAC %08X W SCR ch:%d data:%04X width:%d"), addr, channel, data, width);

					data >>= 8;
					dma->scr = (data & SCR_MASK);
				}
			} else {
				// Channel Control Register (A7)
				OUT_DEBUG_REGW(channel, _T("DMAC %08X W SCR.CCR ch:%d data:%04X width:%d"), addr, channel, data, width);

				if (data & CCR_STR) {
					// cannot start transfer when write to it by word access.
					error_transfer(channel, CER_OPERATION_TIMING);
					break;
				}

				dma->ccr |= (data & CCR_OPERATE_MASK);
				dma->ccr = (data & CCR_TOGGLE_MASK) | (dma->ccr & CCR_OPERATE_MASK);

				// Sequence Control Register (A6)
				data >>= 8;
				dma->scr = (data & SCR_MASK);
			}
			if (((prev ^ data) & data & CCR_STR) != 0) {
				// check whether valid operation or not
				if (!prestart_transfer(dma, channel)) {
					break;
				}
			}
			if (((prev ^ data) & data & CCR_CNT) != 0) {
				// check continue mode
				if ((dma->csr & CSR_ACT) != 0 && (dma->ocr & OCR_ARRAY_CHAIN) != 0) {
					// cannot set it while processing on chain mode
					error_transfer(channel, CER_OPERATION_TIMING);
					break;
				}
			}
			if (((prev ^ data) & data & CCR_SAB) != 0) {
				// abort by software
				abort_transfer(channel);
			}
		}
		break;
	case 0x0a:
		// Memory Transfer Counter H,L (A10,11)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W MTC ch:%d data:%04X width:%d"), addr, channel, data, width);
		if ((dma->ccr & CCR_STR) != 0) {
			// count error
			error_transfer(channel, CER_COUNTERR_IN_MTC);
		} else {
			dma->mtc = data;
		}
		break;
	case 0x0c:
		// Memory Address Register 4bytes (A12-A13)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W MAR H ch:%d data:%04X width:%d"), addr, channel, data, width);

		dma->mar = (dma->mar & 0x0000ffff) | (data << 16);
		break;
	case 0x0e:
		// Memory Address Register 4bytes (A14-A15)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W MAR L ch:%d data:%04X width:%d"), addr, channel, data, width);

		dma->mar = (dma->mar & 0xffff0000) | (data & 0xffff);
		break;
	case 0x14:
		// Device Address Register 4bytes (A20-A21)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W DAR H ch:%d data:%04X width:%d"), addr, channel, data, width);

		dma->dar = (dma->dar & 0x0000ffff) | (data << 16);
		break;
	case 0x16:
		// Device Address Register 4bytes (A22-A23)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W DAR L ch:%d data:%04X width:%d"), addr, channel, data, width);

		dma->dar = (dma->dar & 0xffff0000) | (data & 0xffff);
		break;
	case 0x1a:
		// Base Transfer Counter H,L (A26,27)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W BTC ch:%d data:%04X width:%d"), addr, channel, data, width);
		if ((dma->ccr & CCR_STR) != 0 && (dma->ocr & OCR_ARRAY_CHAIN) != 0) {
			// count error
			error_transfer(channel, CER_COUNTERR_IN_BTC);
		} else {
			dma->btc = data;
		}
		break;
	case 0x1c:
		// Base Address Register 4bytes (A28-A29)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W BAR H ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->bar = (dma->bar & 0x0000ffff) | (data << 16);
		break;
	case 0x1e:
		// Base Address Register 4bytes (A30-A31)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W BAR L ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->bar = (dma->bar & 0xffff0000) | (data & 0xffff);
		break;
	case 0x24:
		// Normal Interrupt Vector (A37)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W NIV ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->niv = data;
		break;
	case 0x26:
		// Error Interrupt Vector (A39)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W EIV ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->eiv = data;
		break;
	case 0x2c:
		// Channel Priority Register (A45)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W CPR ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->cpr = (data & CPR_MASK);
		// update priority
		update_priority_channel();
		break;
	case 0x28:
		// Memory Function Codes (A41)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W MFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->mfc = (data & FC_MASK);
		break;
	case 0x30:
		// Device Function Codes (A49)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W DFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->dfc = (data & FC_MASK);
		break;
	case 0x38:
		// Base Function Codes (A57)
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W BFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		dma->bfc = (data & FC_MASK);
		break;
	default:
		OUT_DEBUG_REGW(channel, _T("DMAC %08X W Unknown ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	}
}

uint32_t DMAC::read_io8(uint32_t addr)
{
	return read_io_n(addr, 1);
}

uint32_t DMAC::read_io16(uint32_t addr)
{
	return read_io_n(addr, 2);
}

/// @param[in] addr  : address
/// @param[in] width : access size 1:byte 2:word
/// @return value of specified register
///
/// @note The data b15-b8 is always at even address, and data b7-b0 is at odd address.
uint32_t DMAC::read_io_n(uint32_t addr, int width)
{
	uint32_t data = 0;
	int channel = (addr >> 6) & 0x3;
	struct st_dma_regs *dma = &m_dma[channel];

	if (m_now_iack) {
		// decide channel which is occured interrupt
		channel = -1;
		int pri = 4;
		for(int ch=0; ch<4; ch++) {
			if (m_interrupt & (1 << ch)) {
				// priority
				if (pri > m_dma[ch].cpr) {
					channel = ch;
					pri = m_dma[ch].cpr;
				}
			}
		}
		if (channel < 0) {
			// why?
			return data;
		}

		dma = &m_dma[channel];

		// decide interrupt vector
		if (dma->csr & CSR_ERR) {
			// error occured
			data = dma->eiv;
		} else {
			// success
			data = dma->niv;
		}

		OUT_DEBUG_IACK(channel, _T("clk: %d DMAC ch:%d IACK vector:%02X"), (int)get_current_clock(), channel, data);

		// clear interrupt
		update_irq(channel, false);

		return data;
	}

	if ((addr & 0xfe) == 0xfe) {
		// General Control Register
		data = m_gcr;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R GCR ch:%d data:%04X width:%d"), addr, channel, data, width);
		return data;
	}

	switch(addr & 0x3e) {
	case 0x00:
		if (width == 1) {
			if (addr & 1) {
				// Channel Error Register (A1)
				data = dma->cer;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R CER ch:%d data:%04X width:%d"), addr, channel, data, width);
			} else {
				// Channel Status Register (A0)
				data = dma->csr;
				data <<= 8;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R CSR ch:%d data:%04X width:%d"), addr, channel, data, width);
			}
		} else {
			// Channel Status Register (A0)
			data = dma->csr;
			data <<= 8;
			// Channel Error Register (A1)
			data |= dma->cer;
			OUT_DEBUG_REGR(channel, _T("DMAC %08X R CSR.CER ch:%d data:%04X width:%d"), addr, channel, data, width);
		}
		break;
	case 0x04:
		if (width == 1) {
			if (addr & 1) {
				// Operation Control Register (A5)
				data = dma->ocr;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R OCR ch:%d data:%04X width:%d"), addr, channel, data, width);
			} else {
				// Device Control Register (A4)
				data = dma->dcr;
				data <<= 8;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R DCR ch:%d data:%04X width:%d"), addr, channel, data, width);
			}
		} else {
			// Device Control Register (A4)
			data = dma->dcr;
			data <<= 8;
			// Operation Control Register (A5)
			data |= dma->ocr;
			OUT_DEBUG_REGR(channel, _T("DMAC %08X R DCR.OCR ch:%d data:%04X width:%d"), addr, channel, data, width);
		}
		break;
	case 0x06:
		if (width == 1) {
			if (addr & 1) {
				// Channel Control Register (A7)
				data = dma->ccr;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R CCR ch:%d data:%04X width:%d"), addr, channel, data, width);
			} else {
				// Sequence Control Register (A6)
				data = dma->scr;
				data <<= 8;
				OUT_DEBUG_REGR(channel, _T("DMAC %08X R SCR ch:%d data:%04X width:%d"), addr, channel, data, width);
			}
		} else {
			// Sequence Control Register (A6)
			data = dma->scr;
			data <<= 8;
			// Channel Control Register (A7)
			data |= dma->ccr;
			OUT_DEBUG_REGR(channel, _T("DMAC %08X R SCR.CCR ch:%d data:%04X width:%d"), addr, channel, data, width);
		}
		break;
	case 0x0a:
		// Memory Transfer Counter H,L (A10,11)
		data = dma->mtc;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R MTC ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x0c:
		// Memory Address Register 4bytes (A12-A13)
		data = (dma->mar >> 16);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R MAR H ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x0e:
		// Memory Address Register 4bytes (A14-A15)
		data = (dma->mar & 0xffff);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R MAR L ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x14:
		// Device Address Register 4bytes (A20-A21)
		data = (dma->dar >> 16);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R DAR H ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x16:
		// Device Address Register 4bytes (A22-A23)
		data = (dma->dar & 0xffff);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R DAR L ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x1a:
		// Base Transfer Counter H,L (A26,27)
		data = dma->btc;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R BTC ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x1c:
		// Base Address Register 4bytes (A28-A29)
		data = (dma->bar >> 16);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R BAR H ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x1e:
		// Base Address Register 4bytes (A30-A31)
		data = (dma->bar & 0xffff);
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R BAR L ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x24:
		// Normal Interrupt Vector (A37)
		data = dma->niv;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R NIV ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x26:
		// Error Interrupt Vector (A39)
		data = dma->eiv;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R EIV ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x2c:
		// Channel Priority Register (A45)
		data = dma->cpr;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R CPR ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x28:
		// Memory Function Codes (A41)
		data = dma->mfc;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R MFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x30:
		// Device Function Codes (A49)
		data = dma->dfc;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R DFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	case 0x38:
		// Base Function Codes (A57)
		data = dma->bfc;
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R BFC ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	default:
		OUT_DEBUG_REGR(channel, _T("DMAC %08X R Unknown ch:%d data:%04X width:%d"), addr, channel, data, width);
		break;
	}
	return data;
}

void DMAC::write_via_debugger_data_n(struct st_dma_regs *dma, uint32_t addr, uint32_t data, int width)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		dma->d_dbg->write_dma_data_n(addr, data, width);
	} else
#endif
	d_memory->write_dma_data_n(addr, data, width);
}

uint32_t DMAC::read_via_debugger_data_n(struct st_dma_regs *dma, uint32_t addr, int width)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		return dma->d_dbg->read_dma_data_n(addr, width);
	} else
#endif
	return d_memory->read_dma_data_n(addr, width);
}

void DMAC::write_via_debugger_io8(struct st_dma_regs *dma, uint32_t addr, uint32_t data)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		dma->d_dbg->write_dma_io8(addr, data);
	} else
#endif
	dma->device->write_dma_io8(addr, data);
}

uint32_t DMAC::read_via_debugger_io8(struct st_dma_regs *dma, uint32_t addr)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		return dma->d_dbg->read_dma_io8(addr);
	} else
#endif
	return dma->device->read_dma_io8(addr);
}

void DMAC::write_via_debugger_io16(struct st_dma_regs *dma, uint32_t addr, uint32_t data)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		dma->d_dbg->write_dma_io16(addr, data);
	} else
#endif
	dma->device->write_dma_io16(addr, data);
}

uint32_t DMAC::read_via_debugger_io16(struct st_dma_regs *dma, uint32_t addr)
{
#ifdef USE_DEBUGGER
	if (d_debugger->now_debugging()) {
		return dma->d_dbg->read_dma_io16(addr);
	} else
#endif
	return dma->device->read_dma_io16(addr);
}

void DMAC::update_priority_channel()
{
}

void DMAC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_REQ_0:
	case SIG_REQ_1:
	case SIG_REQ_2:
	case SIG_REQ_3:
		request(id - SIG_REQ_0, (data & mask) != 0);
		break;
	case SIG_BG:
		// Bus ground signal return immediately after assert bus request signal to cpu
		if(data & mask) {
			for(int ch = 0; ch<4; ch++) {
				if (m_busreq & (BUSREQ0 << ch)) {
					busack(ch, true);
				}
			}
		}
		break;
	case SIG_IACK:
		// receive interrupt ack signal
		m_now_iack = ((data & mask) != 0);
		break;
	case SIG_CPU_RESET:
		now_reset= ((data & mask) != 0);
		warm_reset(false);
		break;
	}
}

/// request the bus to the bus master
void DMAC::request(int channel, bool onoff)
{
	update_reqline(channel, onoff);

	OUT_DEBUG_REQ(channel, _T("DMAC ch%d REQ:%d BREQ:%02X CCR:%02X"), channel, onoff ? 1 : 0, m_busreq, m_dma[channel].ccr);
	if (m_dma[channel].ccr & CCR_STR) {
		// processing
		if(onoff) {
			if (m_busreq & BUSREQ_MASK) {
				// Already asserting bus request, so bus ground signal does not come in duplicately.
				busack(channel, onoff);
			}
		}
		update_breq(channel, onoff);
	} else {
		// no processing
		update_breq(channel, false);
	}
}

/// received bus ground from the bus master
/// and start transfer after a few clock
void DMAC::busack(int channel, bool onoff)
{
	OUT_DEBUG_REQ(channel, _T("DMAC ch%d REQ:%d BR:%02X Trasfer"), channel, onoff ? 1 : 0, m_busreq);
	register_transfer_event(channel, 4);
}

void DMAC::cancel_my_event(int id)
{
	cancel_event(this, m_register_id[id]);
	m_register_id[id] = -1;
}

void DMAC::register_transfer_event(int channel, int clock)
{
	cancel_my_event(EVENT_TRANSFER_0 + channel);
	register_event_by_clock(this, EVENT_TRANSFER_0 + channel, clock, false, &m_register_id[EVENT_TRANSFER_0 + channel]);
}

void DMAC::register_processed_event(int channel, int clock)
{
	cancel_my_event(EVENT_PROCESSED_0 + channel);
	register_event_by_clock(this, EVENT_PROCESSED_0 + channel, clock, false, &m_register_id[EVENT_PROCESSED_0 + channel]);
}

void DMAC::register_request_event(int channel, int clock)
{
	cancel_my_event(EVENT_REQUEST_0 + channel);
	register_event_by_clock(this, EVENT_REQUEST_0 + channel, clock, false, &m_register_id[EVENT_REQUEST_0 + channel]);
}

/// check configuration in registers before start transfer
bool DMAC::prestart_transfer(struct st_dma_regs *dma, int channel)
{
	int error = 0;
	do {
		if ((dma->ocr & OCR_ARRAY_CHAIN) != 0 && (dma->ccr & CCR_CNT) != 0) {
			// cannot continue mode when set chain mode
			error = CER_CONFIGURATION;
			break;
		}
		if (dma->dcr & DCR_DTYP_SINGLE) {
			// single mode
			if ((dma->dcr & DCR_DPS) == 0 && ((dma->ocr & OCR_SIZE) == OCR_SIZE_16BITS || (dma->ocr & OCR_SIZE) == OCR_SIZE_32BITS)) {
				error = CER_CONFIGURATION;
				break;
			} else if ((dma->dcr & DCR_DPS) != 0 && ((dma->ocr & OCR_SIZE) != OCR_SIZE_16BITS && (dma->ocr & OCR_SIZE) != OCR_SIZE_32BITS)) {
				error = CER_CONFIGURATION;
				break;
			}
		} else {
			// dual mode
			if ((dma->dcr & DCR_DPS) != 0
			 && ((dma->ocr & OCR_SIZE) != OCR_SIZE_16BITS && (dma->ocr & OCR_SIZE) != OCR_SIZE_32BITS)
			 && (dma->ocr & OCR_REQG_EXT) != 0) {
				error = CER_CONFIGURATION;
				break;
			}
		}
		if ((dma->dcr & DCR_XRM) == DCR_XRM_UNDEF
			|| (dma->scr & SCR_MAC) == SCR_MAC_UNDEF
			|| (dma->scr & SCR_DAC) == SCR_DAC_UNDEF
			|| (dma->ocr & OCR_CHAIN) == OCR_CHAIN_UNDEF
		) {
			error = CER_CONFIGURATION;
			break;
		}
		if (dma->csr & (CSR_COC | CSR_BTC | CSR_NDT | CSR_ERR | CSR_ACT)) {
			error = CER_OPERATION_TIMING;
		}
	} while(0);

	if (error) {
		// error occurring
		error_transfer(channel, error & 0xff);
		return false;
	}

	// start transfer
	start_transfer(channel);
	return true;
}

void DMAC::start_transfer(int channel)
{
	struct st_dma_regs *dma = &m_dma[channel];

	dma->csr &= ~(CSR_COC | CSR_BTC | CSR_NDT | CSR_ERR | CSR_DIT);
	dma->csr |= CSR_ACT;
//	dma->ccr &= ~(CCR_SAB | CCR_CNT | CCR_HLT);
	dma->ccr &= ~(CCR_SAB | CCR_HLT);

	dma->pack_data = 0;
	dma->pack_width = 0;

	// get first chain parameters
	// TODO: need bus arbitration to access memory...
	switch(dma->ocr & OCR_CHAIN) {
	case 0x8:
		// array chaining mode
		dma->mar = d_memory->read_data16(dma->bar);
		dma->mar <<= 16;
		dma->mar |= d_memory->read_data16(dma->bar + 2);
		dma->mtc = d_memory->read_data16(dma->bar + 4);
		dma->bar += 6;
		dma->next_pointer = dma->bar;

		OUT_DEBUG_ARR(channel, _T("DMAC ch%d START ARR CCR:%02X BAR:%06X BTC:%04X MAR:%06X MTC:%04X next:%06X")
			, channel, dma->ccr, dma->bar, dma->btc, dma->mar, dma->mtc, dma->next_pointer);
		break;
	case 0xc:
		// linked array chaining mode
		dma->mar = d_memory->read_data16(dma->bar);
		dma->mar <<= 16;
		dma->mar |= d_memory->read_data16(dma->bar + 2);
		dma->mtc = d_memory->read_data16(dma->bar + 4);
		dma->next_pointer = d_memory->read_data16(dma->bar + 6);
		dma->next_pointer <<= 16;
		dma->next_pointer |= d_memory->read_data16(dma->bar + 8);

		OUT_DEBUG_LNKARR(channel, _T("DMAC ch%d START LINK ARR CCR:%02X BAR:%06X MAR:%06X MTC:%04X next:%06X")
			, channel, dma->ccr, dma->bar, dma->mar, dma->mtc, dma->next_pointer);
		break;
	default:
		break;
	}

	OUT_DEBUG_TRANS(channel, _T("clk:%d DMAC ch%d START TRANSFER CCR:%02X OCR:%02X MAR:%08X MTC:%04X DAR:%08X")
		, (int)get_current_clock()
		, channel
		, dma->ccr, dma->ocr, dma->mar, dma->mtc, dma->dar);

	switch (dma->ocr & OCR_REQG) {
	case 0:
	case 1:
	case 3:
		// Auto-requset mode
		update_breq(channel, true);
		break;
	case 2:
		// if already asserted requesting, start process immediately
		request(channel, (m_busreq & (REQLINE0 << channel)) != 0);
		break;
	}
}

void DMAC::transfer(int channel)
{
	struct st_dma_regs *dma = &m_dma[channel];

	int mem_unit;
	int dev_unit;
	int spent_clock = 0;

	if(true) {
		// assert DACK signal to device
		write_signals(&dma->outputs_ack, 1);

		if(dma->ocr & OCR_DIR) {
			// device to memory
			uint32_t data;
			// operand size
			switch(dma->ocr & OCR_SIZE) {
			case 0x30:
				// non packing mode and transfer per 1byte
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2;
					data = read_via_debugger_io16(dma, dma->dar);
				} else {
					// 1byte
					dev_unit = 1;
					data = read_via_debugger_io8(dma, dma->dar);
				}
				spent_clock += (4 * 1);
				if (!(dma->dar & 1)) data >>= 8;
				if (!(dma->mar & 1)) data <<= 8;
				mem_unit = 1;
				this->write_via_debugger_data_n(dma, dma->mar, data, 1);
				spent_clock += (4 * 1);
				break;
			case 0x20:
				// packing mode and transfer per 4bytes
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2 * 2;
					if (dma->dar & 1) {
						// odd address
						data = (read_via_debugger_io8(dma, dma->dar) & 0xff);
						data <<= 16;
						data |= (read_via_debugger_io16(dma, dma->dar + 1) & 0xffff);
						data <<= 8;
						data |= ((read_via_debugger_io16(dma, dma->dar + 3) >> 8) & 0xff);
						spent_clock += (4 * 3);
					} else {
						data = (read_via_debugger_io16(dma, dma->dar) & 0xffff);
						data <<= 16;
						data |= (read_via_debugger_io16(dma, dma->dar + 2) & 0xffff);
						spent_clock += (4 * 2);
					}
				} else {
					// 1byte
					dev_unit = 1 * 4;
					data = (read_via_debugger_io8(dma, dma->dar) & 0xff);
					data <<= 8;
					data |= (read_via_debugger_io8(dma, dma->dar + 2) & 0xff);
					data <<= 8;
					data |= (read_via_debugger_io8(dma, dma->dar + 4) & 0xff);
					data <<= 8;
					data |= (read_via_debugger_io8(dma, dma->dar + 6) & 0xff);
					spent_clock += (4 * 4);
				}
				mem_unit = 4;
				if (dma->mar & 1) {
					// odd address
					this->write_via_debugger_data_n(dma, dma->mar + 3, data & 0xff, 1);
					data >>= 8;
					this->write_via_debugger_data_n(dma, dma->mar + 1, data & 0xffff, 2);
					data >>= 16;
					this->write_via_debugger_data_n(dma, dma->mar, data & 0xff, 1);
					spent_clock += (4 * 3);
				} else {
					this->write_via_debugger_data_n(dma, dma->mar + 2, data & 0xffff, 2);
					data >>= 16;
					this->write_via_debugger_data_n(dma, dma->mar, data & 0xffff, 2);
					spent_clock += (4 * 2);
				}
				break;
			case 0x10:
				// packing mode and transfer per 2bytes
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2 * 1;
					if (dma->dar & 1) {
						// odd address
						data = (read_via_debugger_io8(dma, dma->dar) & 0xff);
						data <<= 8;
						data |= ((read_via_debugger_io16(dma, dma->dar + 1) >> 8) & 0xff);
						spent_clock += (4 * 2);
					} else {
						data = read_via_debugger_io16(dma, dma->dar);
						spent_clock += (4 * 1);
					}
				} else {
					// 1byte
					dev_unit = 1 * 2;
					data = (read_via_debugger_io8(dma, dma->dar) & 0xff);
					data <<= 8;
					data |= (read_via_debugger_io8(dma, dma->dar + 2) & 0xff);
					spent_clock += (4 * 2);
				}
				mem_unit = 2;
				if (dma->mar & 1) {
					// odd address
					this->write_via_debugger_data_n(dma, dma->mar + 1, data & 0xff, 1);
					data >>= 8;
					this->write_via_debugger_data_n(dma, dma->mar, data & 0xff, 1);
					spent_clock += (4 * 2);
				} else {
					this->write_via_debugger_data_n(dma, dma->mar, data & 0xffff, 2);
					spent_clock += (4 * 1);
				}
				break;
			default:
				// packing mode and transfer per 1byte
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2;
					if (dma->dar & 1) {
						// odd address
						data = (read_via_debugger_io8(dma, dma->dar) & 0xff);
						data <<= 8;
						data |= ((read_via_debugger_io16(dma, dma->dar + 1) >> 8) & 0xff);
						spent_clock += (4 * 2);
					} else {
						data = read_via_debugger_io16(dma, dma->dar);
						spent_clock += (4 * 1);
					}
					mem_unit = 1 * 2;
					if (dma->mtc == 1) {
						// last 1byte
						if (dma->mar & 1) data >>= 8;
						this->write_via_debugger_data_n(dma, dma->mar, data, 1);
						spent_clock += (4 * 1);
					} else if (dma->mar & 1) {
						// odd address
						this->write_via_debugger_data_n(dma, dma->mar, (data >> 8) & 0xff, 1);
						this->write_via_debugger_data_n(dma, dma->mar, (data & 0xff) << 8, 1);
						spent_clock += (4 * 2);
					} else {
						this->write_via_debugger_data_n(dma, dma->mar, data & 0xffff, 2);
						spent_clock += (4 * 1);
					}
					// 2bytes transfered
					if (dma->mtc > 1) dma->mtc--;
				} else {
					// 1byte
					dev_unit = 2;
					data = read_via_debugger_io8(dma, dma->dar);
					spent_clock += (4 * 1);
					if (!(dma->dar & 1)) data >>= 8;

					if (dma->pack_width == 0) {
						// no packing data
						if (dma->mar & 1) {
							// store to memory immediately
							mem_unit = 1;
							this->write_via_debugger_data_n(dma, dma->mar, data, 1);
							spent_clock += (4 * 1);
						} else {
							// packing
							mem_unit = 0;
							dma->pack_data = data & 0xff;
							dma->pack_width = 1;
						}
					} else {
						// store a word to memory with packing data
						data &= 0xff;
						data |= (dma->pack_data << 8);
						mem_unit = 2;
						this->write_via_debugger_data_n(dma, dma->mar, data, 2);
						spent_clock += (4 * 1);
						dma->pack_width = 0;
					}
				}
				break;
			}
		} else {
			// memory to device
			uint32_t data;
			// operand size
			switch(dma->ocr & OCR_SIZE) {
			case 0x30:
				// non packing mode and transfer per 1byte
				mem_unit = 1;
				data = this->read_via_debugger_data_n(dma, dma->mar, 1);
				spent_clock += (4 * 1);
				if (!(dma->mar & 1)) data >>= 8;
				if (!(dma->dar & 1)) data <<= 8;
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2;
					write_via_debugger_io16(dma, dma->dar, data);
					spent_clock += (4 * 1);
				} else {
					// 1byte
					dev_unit = 1;
					write_via_debugger_io8(dma, dma->dar, data);
					spent_clock += (4 * 1);
				}
				break;
			case 0x20:
				// packing mode and transfer per 4bytes
				mem_unit = 4;
				if (dma->mar & 1) {
					// odd address
					data = (this->read_via_debugger_data_n(dma, dma->mar, 1) << 24);
					data |= ((this->read_via_debugger_data_n(dma, dma->mar + 1, 2) & 0xffff) << 8);
					data |= ((this->read_via_debugger_data_n(dma, dma->mar + 3, 1) >> 8) & 0xff);
					spent_clock += (4 * 3);
				} else {
					data = (this->read_via_debugger_data_n(dma, dma->mar, 2) << 16);
					data |= (this->read_via_debugger_data_n(dma, dma->mar + 2, 2) & 0xffff);
					spent_clock += (4 * 2);
				}
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2 * 2;
					if (dma->dar & 1) {
						// odd address
						write_via_debugger_io8(dma, dma->dar + 3, data & 0xff);
						data >>= 8;
						write_via_debugger_io16(dma, dma->dar + 1, data & 0xffff);
						data >>= 16;
						write_via_debugger_io8(dma, dma->dar, data & 0xff);
						spent_clock += (4 * 3);
					} else {
						write_via_debugger_io16(dma, dma->dar + 2, data);
						data >>= 16;
						write_via_debugger_io16(dma, dma->dar, data);
						spent_clock += (4 * 2);
					}
				} else {
					// 1byte
					dev_unit = 1 * 4;
					write_via_debugger_io8(dma, dma->dar + 6, data & 0xff);
					data >>= 8;
					write_via_debugger_io8(dma, dma->dar + 4, data & 0xff);
					data >>= 8;
					write_via_debugger_io8(dma, dma->dar + 2, data & 0xff);
					data >>= 8;
					write_via_debugger_io8(dma, dma->dar + 0, data & 0xff);
					spent_clock += (4 * 4);
				}
				break;
			case 0x10:
				// packing mode and transfer per 2bytes
				mem_unit = 2;
				if (dma->mar & 1) {
					// odd address
					data = (this->read_via_debugger_data_n(dma, dma->mar, 1) << 8);
					data |= ((this->read_via_debugger_data_n(dma, dma->mar + 1, 1) >> 8) & 0xff);
					spent_clock += (4 * 2);
				} else {
					data = this->read_via_debugger_data_n(dma, dma->mar, 2);
					spent_clock += (4 * 1);
				}
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					dev_unit = 2 * 1;
					if (dma->dar & 1) {
						// odd address
						write_via_debugger_io8(dma, dma->dar + 1, data & 0xff);
						data >>= 8;
						write_via_debugger_io8(dma, dma->dar, data & 0xff);
						spent_clock += (4 * 2);
					} else {
						write_via_debugger_io16(dma, dma->dar, data);
						spent_clock += (4 * 1);
					}
				} else {
					// 1byte
					dev_unit = 1 * 2;
					write_via_debugger_io8(dma, dma->dar + 2, data & 0xff);
					data >>= 8;
					write_via_debugger_io8(dma, dma->dar, data & 0xff);
					spent_clock += (4 * 2);
				}
				break;
			default:
				// packing mode and transfer per 1byte
				if (dma->dcr & DCR_DPS) {
					// 2bytes
					mem_unit = 1 * 2;
					if (dma->mar & 1) {
						// odd address
						data = (this->read_via_debugger_data_n(dma, dma->mar, 1) << 8);
						data |= ((this->read_via_debugger_data_n(dma, dma->mar + 1, 1) >> 8) & 0xff);
						spent_clock += (4 * 2);
					} else {
						data = this->read_via_debugger_data_n(dma, dma->mar, 2);
						spent_clock += (4 * 1);
					}
					dev_unit = 2;
					if (dma->mtc == 1) {
						// last 1byte
						if (dma->dar & 1) data >>= 8;
						write_via_debugger_io8(dma, dma->dar, data);
						spent_clock += (4 * 1);
					} else if (dma->dar & 1) {
						// odd address
						write_via_debugger_io8(dma, dma->dar, (data >> 8) & 0xff);
						write_via_debugger_io8(dma, dma->dar + 1, (data & 0xff) << 8);
						spent_clock += (4 * 2);
					} else {
						write_via_debugger_io16(dma, dma->dar, data);
						spent_clock += (4 * 1);
					}
					// 2bytes transfered
					if (dma->mtc > 1) dma->mtc--;
				} else {
					// 1byte
					if (dma->pack_width == 0) {
						// no packing data
						if (dma->mar & 1) {
							// read a byte when odd address
							mem_unit = 1;
							data = this->read_via_debugger_data_n(dma, dma->mar, 1);
							spent_clock += (4 * 1);
						} else {
							// read a word when even address
							mem_unit = 2;
							data = this->read_via_debugger_data_n(dma, dma->mar, 2);
							spent_clock += (4 * 1);
							// store a byte at odd address to use next transfer
							dma->pack_data = data & 0xff;
							dma->pack_width = 1;
							data >>= 8;
						}
					} else {
						// load from packing data
						mem_unit = 0;
						data = dma->pack_data;
						dma->pack_width = 0;
					}

					if (!(dma->dar & 1)) data <<= 8;
					dev_unit = 2;
					write_via_debugger_io8(dma, dma->dar, data);
					spent_clock += (4 * 1);
				}
				break;
			}
		}

		// count up/down device address register
		switch(dma->scr & SCR_DAC) {
		case 1:
			dma->dar+=dev_unit;
			break;
		case 2:
			dma->dar-=dev_unit;
			break;
		default:
			break;
		}

		// count up/down memory address register
		switch(dma->scr & SCR_MAC) {
		case 4:
			dma->mar+=mem_unit;
			break;
		case 8:
			dma->mar-=mem_unit;
			break;
		default:
			break;
		}

		// transfer complete?
		if (dma->mtc) dma->mtc--;
		if (dma->mtc == 0) {
			// chain control
			switch(dma->ocr & OCR_CHAIN) {
			case 0x8:
				// array chaining mode
				if(dma->btc) dma->btc--;
//				dma->csr |= CSR_BTC;
				if (dma->btc) {
					// read next table on memory
					uint32_t next_ptr = dma->next_pointer;
					dma->mar = d_memory->read_data16(next_ptr);
					dma->mar <<= 16;
					dma->mar |= d_memory->read_data16(next_ptr + 2);
					dma->mtc = d_memory->read_data16(next_ptr + 4);
					dma->bar += 6;
					next_ptr += 6;
					spent_clock += (4 * 3);

					OUT_DEBUG_ARR(channel, _T("DMAC ch%d NEXT ARR MAR:%06X MTC:%04X BTC:%04X BAR:%06X next:%06X")
						, channel, dma->mar, dma->mtc, dma->btc, dma->bar, next_ptr);

					dma->next_pointer = next_ptr;

					// next transfer
					next_transfer(dma, channel, spent_clock);
				} else {
					// end of transfer

					OUT_DEBUG_ARR(channel, _T("DMAC ch%d FINISH ARR MAR:%06X MTC:%04X BTC:%04X BAR:%06X next:%06X")
						, channel, dma->mar, dma->mtc, dma->btc, dma->bar, dma->next_pointer);

					finish_transfer(dma, channel);
				}
				break;
			case 0xc:
				// linked array chaining mode
//				dma->csr |= CSR_BTC;
				if (dma->next_pointer != 0) {
					// read next table on memory
					uint32_t curr_ptr = dma->next_pointer;
					dma->mar = d_memory->read_data16(curr_ptr);
					dma->mar <<= 16;
					dma->mar |= d_memory->read_data16(curr_ptr + 2);
					dma->mtc = d_memory->read_data16(curr_ptr + 4);
					uint32_t next_ptr = d_memory->read_data16(curr_ptr + 6);
					next_ptr <<= 16;
					next_ptr |= d_memory->read_data16(curr_ptr + 8);
					spent_clock += (4 * 5);

					OUT_DEBUG_LNKARR(channel, _T("DMAC ch%d NEXT LINK ARR MAR:%06X MTC:%04X next:%06X->%06X")
						, channel, dma->mar, dma->mtc, curr_ptr, next_ptr);

					dma->next_pointer = next_ptr;

					// next transfer
					next_transfer(dma, channel, spent_clock);
				} else {
					// end of transfer

					OUT_DEBUG_LNKARR(channel, _T("DMAC ch%d FINISH LINK ARR next:%06X")
						, channel, dma->next_pointer);

					finish_transfer(dma, channel);
				}
				break;
			default:
				if (dma->ccr & CCR_CNT) {
					// continuous mode
					dma->mar = dma->bar;
					dma->mtc = dma->btc;
					dma->mfc = dma->bfc;
					spent_clock = 4;

					// continue transfer
					continue_transfer(dma, channel, spent_clock);
				} else {
					// end of transfer
					finish_transfer(dma, channel);
				}
				break;
			}
		} else {
			// next transfer
			next_transfer(dma, channel, spent_clock);
		}

		// negate DACK signal
		write_signals(&dma->outputs_ack, 0);
	}
}

void DMAC::finish_transfer(struct st_dma_regs *dma, int channel)
{
	dma->ccr &= ~(CCR_OPE_ALL);
	dma->csr &= ~(CSR_ACT);
	dma->csr |= CSR_COC; //| CSR_BTC); do not set BTC on finished transfer

	OUT_DEBUG_RES(channel, _T("clk:%d DMAC ch%d FINISH TRANSFER CSR:%02X CER:%02X OCR:%02X MAR:%08X MTC:%04X DAR:%08X")
		, (int)get_current_clock()
		, channel
		, dma->csr, dma->cer
		, dma->ocr, dma->mar, dma->mtc, dma->dar);

	// Auto-requset mode
	switch (dma->ocr & OCR_REQG) {
	case 0:
	case 1:
	case 3:
		update_breq(channel, false);
		break;
	}

	// interrupt
	if (dma->ccr & CCR_INT) {
		update_irq(channel, true);
	}
}

void DMAC::continue_transfer(struct st_dma_regs *dma, int channel, int clock)
{
	dma->ccr &= ~CCR_CNT;	// reset continue flag
	dma->csr |= CSR_BTC;

	OUT_DEBUG_CNT(channel, _T("clk:%d DMAC ch%d CONTINUE TRANSFER CSR:%02X CER:%02X OCR:%02X MAR:%06X MTC:%04X MFC:%02X")
		,(int)get_current_clock()
		, channel
		, dma->csr, dma->cer
		, dma->ocr, dma->mar, dma->mtc, dma->mfc);

	// Auto-requset mode
	switch (dma->ocr & OCR_REQG) {
	case 0:
	case 1:
	case 3:
		update_breq(channel, false);
		break;
	}

	// interrupt
	if (dma->ccr & CCR_INT) {
		update_irq(channel, true);
	}

	// start next transfer
	next_transfer(dma, channel, clock);
}

void DMAC::next_transfer(struct st_dma_regs *dma, int channel, int clock)
{
	switch (dma->ocr & OCR_REQG) {
	case 0:
		// Auto-request limited rate speed
		dma->next_clock = (1 << ((m_gcr & GCR_BR) + 4 + ((m_gcr & GCR_BT) >> GCR_BT_SFT) + 1));
		// delay processed clock 
		register_processed_event(channel, clock);
		break;
	case 1:
		// Auto-request max rate speed
		register_transfer_event(channel, clock);
		break;
	}
}

void DMAC::abort_transfer(int channel)
{
	struct st_dma_regs *dma = &m_dma[channel];

	dma->csr &= ~(CSR_ACT);
	if (!(dma->ccr & CCR_STR)) {
		// no operating
		dma->ccr &= ~(CCR_OPE_ALL);
		return;
	}
	dma->csr |= (CSR_ERR);
	dma->cer |= CER_SOFTWARE_ABORT;
	dma->ccr &= ~(CCR_OPE_ALL);

	OUT_DEBUG_RES(channel, _T("clk:%d DMAC ch%d ABORT TRANSFER CSR:%02X CER:%02X OCR:%02X MAR:%08X MTC:%04X DAR:%08X")
		, (int)get_current_clock()
		, channel
		, dma->csr, dma->cer
		, dma->ocr, dma->mar, dma->mtc, dma->dar);

	// Auto-requset mode
	switch (dma->ocr & OCR_REQG) {
	case 0:
	case 1:
	case 3:
		update_breq(channel, false);
		break;
	}

	// interrupt
	if (dma->ccr & CCR_INT) {
		update_irq(channel, true);
	}
}

void DMAC::error_transfer(int channel, uint8_t reason)
{
	struct st_dma_regs *dma = &m_dma[channel];

	dma->csr &= ~(CSR_ACT);
	dma->csr |= (CSR_ERR);
	dma->cer = reason;
	dma->ccr &= ~(CCR_OPE_ALL);

	OUT_DEBUG_RES(channel, _T("clk:%d DMAC ch%d ERROR TRANSFER CSR:%02X CER:%02X OCR:%02X MAR:%08X MTC:%04X DAR:%08X")
		, (int)get_current_clock()
		, channel
		, dma->csr, dma->cer
		, dma->ocr, dma->mar, dma->mtc, dma->dar);

	// Auto-requset mode
	switch (dma->ocr & OCR_REQG) {
	case 0:
	case 1:
	case 3:
		update_breq(channel, false);
		break;
	}

	// interrupt
	if (dma->ccr & CCR_INT) {
		update_irq(channel, true);
	}
}

void DMAC::processed(int channel)
{
	// next request
	register_request_event(channel, m_dma[channel].next_clock);
	// release bus
	update_breq(channel, false);
}

void DMAC::update_irq(int channel, bool onoff)
{
	BIT_ONOFF(m_interrupt, (1 << channel), onoff);
	write_signals(&outputs_irq, m_interrupt ? 0xffffffff : 0);
}

void DMAC::update_breq(int channel, bool onoff)
{
	BIT_ONOFF(m_busreq, (BUSREQ0 << channel), onoff);
	write_signals(&outputs_busreq, (m_busreq & BUSREQ_MASK) ? 0xffffffff : 0); 
}

void DMAC::update_reqline(int channel, bool onoff)
{
	BIT_ONOFF(m_busreq, (REQLINE0 << channel), onoff);
}

void DMAC::event_callback(int event_id, int err)
{
	m_register_id[event_id] = -1;
	switch(event_id) {
	case EVENT_TRANSFER_0:
	case EVENT_TRANSFER_1:
	case EVENT_TRANSFER_2:
	case EVENT_TRANSFER_3:
		transfer(event_id - EVENT_TRANSFER_0);
		break;
	case EVENT_PROCESSED_0:
	case EVENT_PROCESSED_1:
	case EVENT_PROCESSED_2:
	case EVENT_PROCESSED_3:
		processed(event_id - EVENT_PROCESSED_0);
		break;
	case EVENT_REQUEST_0:
	case EVENT_REQUEST_1:
	case EVENT_REQUEST_2:
	case EVENT_REQUEST_3:
		request(event_id - EVENT_REQUEST_0, true);
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
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void DMAC::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<4; i++) {
		SET_Byte(m_dma[i].csr);
		SET_Byte(m_dma[i].cer);
		SET_Byte(m_dma[i].dcr);
		SET_Byte(m_dma[i].ocr);
		SET_Byte(m_dma[i].scr);
		SET_Byte(m_dma[i].ccr);
		SET_Byte(m_dma[i].niv);
		SET_Byte(m_dma[i].eiv);
		SET_Uint16_LE(m_dma[i].mtc);
		SET_Uint16_LE(m_dma[i].btc);
		SET_Uint32_LE(m_dma[i].mar);

		SET_Uint32_LE(m_dma[i].dar);
		SET_Uint32_LE(m_dma[i].bar);
		SET_Byte(m_dma[i].cpr);
		SET_Byte(m_dma[i].mfc);
		SET_Byte(m_dma[i].dfc);
		SET_Byte(m_dma[i].bfc);

		SET_Uint16_LE(m_dma[i].pack_data);
		SET_Uint16_LE(m_dma[i].pack_width);

		vm_state.m_param[i].next_pointer = Uint32_LE(m_dma[i].next_pointer);
		vm_state.m_param[i].next_clock = Int32_LE(m_dma[i].next_clock);
	}

	SET_Byte(m_gcr);
	SET_Byte(m_busreq);	   ///< asserting bus request (bit3-0: each asserting channel, b7-b4:request signal)
	SET_Byte(m_interrupt); ///< asserting interrupt 
	SET_Bool(m_now_iack);  ///< receiving IACK

	for(int i=0; i<15 && i<END_OF_EVENT_IDS; i++) {
		SET_Int32_LE(m_register_id[i]);
	}

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

bool DMAC::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	for(int i=0; i<4; i++) {
		GET_Byte(m_dma[i].csr);
		GET_Byte(m_dma[i].cer);
		GET_Byte(m_dma[i].dcr);
		GET_Byte(m_dma[i].ocr);
		GET_Byte(m_dma[i].scr);
		GET_Byte(m_dma[i].ccr);
		GET_Byte(m_dma[i].niv);
		GET_Byte(m_dma[i].eiv);
		GET_Uint16_LE(m_dma[i].mtc);
		GET_Uint16_LE(m_dma[i].btc);
		GET_Uint32_LE(m_dma[i].mar);

		GET_Uint32_LE(m_dma[i].dar);
		GET_Uint32_LE(m_dma[i].bar);
		GET_Byte(m_dma[i].cpr);
		GET_Byte(m_dma[i].mfc);
		GET_Byte(m_dma[i].dfc);
		GET_Byte(m_dma[i].bfc);

		GET_Uint16_LE(m_dma[i].pack_data);
		GET_Uint16_LE(m_dma[i].pack_width);

		m_dma[i].next_pointer = Uint32_LE(vm_state.m_param[i].next_pointer);
		m_dma[i].next_clock = Int32_LE(vm_state.m_param[i].next_clock);
	}

	GET_Byte(m_gcr);
	GET_Byte(m_busreq);	   ///< asserting bus request (bit3-0: each asserting channel, b7-b4:request signal)
	GET_Byte(m_interrupt); ///< asserting interrupt 
	GET_Bool(m_now_iack);  ///< receiving IACK

	for(int i=0; i<15 && i<END_OF_EVENT_IDS; i++) {
		GET_Int32_LE(m_register_id[i]);
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER

#include "../../utility.h"

void DMAC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
//	static const _TCHAR *dir[2] = {
//		_T("I/O->MEM"), _T("MEM->I/O")
//	};

	buffer[0] = 0;
	UTILITY::sntprintf(buffer, buffer_len, _T("GCR:%02X\n"), m_gcr);
	for(int i=0; i<4; i++) {
		struct st_dma_regs *dma = &m_dma[i];
		UTILITY::sntprintf(buffer, buffer_len,
			_T("CH%d: CSR:%02X CER:%02X DCR:%02X OCR:%02X SCR:%02X CCR:%02X NIV:%02X EIV:%02X CPR:%X MFC:%d DFC:%d BFC:%d\n"),
			i, dma->csr, dma->cer, dma->dcr, dma->ocr, dma->scr, dma->ccr, dma->niv, dma->eiv, dma->cpr, dma->mfc, dma->dfc, dma->bfc);
		UTILITY::sntprintf(buffer, buffer_len,
			_T("     MTC:%04X MAR:%06X DAR:%06X BTC:%04X BAR:%06X\n"),
			dma->mtc, dma->mar, dma->dar, dma->btc, dma->bar);
	}
}
#endif /* USE_DEBUGGER */
