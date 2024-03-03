/** @file crtc.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ CRTC ]
*/

#include "crtc.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"

#define STORE_DATA(mem, dat, msk) mem = ((mem & (msk)) | (dat))

#ifdef _DEBUG
//#define OUT_DEBUG_GCLS logging->out_debugf
//#define OUT_DEBUG_RASCPY logging->out_debugf
//#define OUT_DEBUG_ACCS logging->out_debugf
//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_RASINT logging->out_debugf
#define OUT_DEBUG_RASINT(...)
//#define OUT_DEBUG_SCRL logging->out_debugf
#define OUT_DEBUG_SCRL(...)
//#define OUT_DEBUG_CTRL logging->out_debugf
#define OUT_DEBUG_CTRL(...)
#else
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_RASINT(...)
#define OUT_DEBUG_SCRL(...)
#define OUT_DEBUG_CTRL(...)
#endif

#define CRTC_START_DISP 1
#define CRTC_START_HSYNC 2
#define CRTC_START_POSITION CRTC_START_HSYNC

void CRTC::initialize()
{
	// initialize
	m_dot_clocks[0] = 38863630.;	///< use normal resolution
	m_dot_clocks[1] = 69551990.;	///< use high resolution
	m_now_dot_clocks = m_dot_clocks[0]; 

	m_now_raster = false;
	m_now_vdisp = true;
	m_now_vsync = true;
	m_now_hsync = true;

	memset(m_regs, 0, sizeof(m_regs));
	m_gcls_count = 0;

	// initial settings for 1st frame
	m_hz_total = 1;
	m_hz_start = 0;
	m_hz_disp = 0;
	m_hs_start = 0;
	m_hs_end = 0;

//	m_h_count = 0;
//	m_hz_total_per_vline = 1;

	m_vt_start = 0;
	m_vt_total = 1;
	m_vt_total_per_frame = 1;
	m_vt_frames_per_sec = 1;
	m_vt_disp = 0;
	m_vs_start = 0;
	m_vs_end = 0;

	m_v_count = -1;
	m_v_count_delay = -1;

	m_timing_changed = false;
	m_timing_updated = true;

	m_hz_disp_end_clock = 0;
	m_hs_start_clock = 0;
	m_hs_end_clock = 0;

#ifdef USE_CRTC_MARA
	m_memory_address = 0;
	m_raster_address = 0;
	max_raster_address = 1;
#endif

	mv_display_width = 768;
	mv_display_height = 512;

//	m_interlace = 0;
//	m_raster_per_dot = 0;
	m_sysport_hrl = 0;

#ifdef CRTC_HORIZ_FREQ
	horiz_freq = 0;
	next_horiz_freq = CRTC_HORIZ_FREQ;
#endif
	for(int i=0; i<3; i++) {
		register_id[i] = -1;
	}

	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void CRTC::reset()
{
	for(int i=0; i<24; i++) {
		m_regs[i] = 0;
	}
	m_vram_accs = 0;
	m_trig_vram_accs = 0;
	m_intr_vline = 0;
	m_intr_vline_mask = ~0;

	m_timing_changed = true;

//	m_interlace = 0;
//	m_raster_per_dot = 0;
	m_sysport_hrl = 0;

	for(int i=0; i<3; i++) {
		register_id[i] = -1;
	}
}

// ----------------------------------------------------------------------------

const uint16_t CRTC::c_regs_mask[] = {
//  R00     R01     R02     R03     R04     R05     R06     R07
	0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff,
//  R08     R09     R10     R11     R12     R13     R14     R15
	0x00ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x01ff, 0x01ff,
//  R16     R17     R18     R19     R20     R21     R22     R23
	0x01ff, 0x01ff, 0x01ff, 0x01ff, 0x0f1f, 0x03ff, 0xffff, 0xffff
};

void CRTC::write_io16(uint32_t addr, uint32_t data)
{
	write_io_m(addr, data, 0xffff0000);
}

void CRTC::write_io_m(uint32_t addr, uint32_t data, uint32_t mask)
{
	uint32_t addrh = (addr >> 1) & 0x3ff;

	OUT_DEBUG_REGW(_T("CRTC: %06X REGW %04X"), addr, data);

	switch(addrh) {
	case 0:	// HTOTAL
	case 1: // HSYNC_END
	case 2: // HDISP_STA
	case 3: // HDISP_END
	case 5: // VSYNC_END
	case 7: // VDISP_END
	case 8: // EX_H_ADJUST
		if ((m_regs[addrh] ^ data) & ~mask & c_regs_mask[addrh]) {
			m_timing_changed = true;
		}
		STORE_DATA(m_regs[addrh], data & c_regs_mask[addrh], mask);
		break;

	case 4: // VTOTAL
	case 6: // VDISP_STA
		if ((m_regs[addrh] ^ data) & ~mask & c_regs_mask[addrh]) {
			m_timing_changed = true;
		}
		// [[ through ]]

	case 9: // RASTER_INT
		STORE_DATA(m_regs[addrh], data & c_regs_mask[addrh], mask);
		update_raster_intr();
		break;

	case 10: // TX_SCRL_X
	case 11: // TX_SCRL_Y
	case 12: // GR0_SCRL_X
	case 13: // GR0_SCRL_Y
	case 14: // GR1_SCRL_X
	case 15: // GR1_SCRL_Y
	case 16: // GR2_SCRL_X
	case 17: // GR2_SCRL_Y
	case 18: // GR3_SCRL_X
	case 19: // GR3_SCRL_Y
		STORE_DATA(m_regs[addrh], data & c_regs_mask[addrh], mask);
		OUT_DEBUG_SCRL(_T("REGW SCROLL: A:%06X Data:%04X Reg:%04X"), addr, data, m_regs[addrh]);
		break;

	case 20: // CONTROL
		if ((m_regs[addrh] ^ data) & ~mask & (CONTROL0_HIRESO | CONTROL0_RESOLUTION)) {
			m_timing_changed = true;
		}
		STORE_DATA(m_regs[addrh], data & c_regs_mask[addrh], mask);
		OUT_DEBUG_CTRL(_T("REGW CONTROL: A:%06X Data:%04X Mask:%04X Reg:%04X TC:%d"), addr, data, mask, m_regs[addrh], m_timing_changed ? 1 : 0);
		break;

	case 21: // TX_ACCESS
	case 22: // RASTER_COPY
	case 23: // TX_BITMASK
		STORE_DATA(m_regs[addrh], data & c_regs_mask[addrh], mask);
		break;

	case 0x240: // CRTC ACCESS PORT
		STORE_DATA(m_vram_accs, data & 0x000f, mask);
		m_trig_vram_accs |= (m_vram_accs & ~0x5);

#ifdef OUT_DEBUG_ACCS
		OUT_DEBUG_ACCS(_T("CRTC ACCS: W:%02X"), m_vram_accs);
		if (m_vram_accs & OP_TCOPY) {
			OUT_DEBUG_ACCS(_T(" => START RASTER COPY"));
		}
		if (m_vram_accs & OP_GCLS) {
			OUT_DEBUG_ACCS(_T(" => START CLEAR GRAPHIC"));
		}
#endif

		// raster copy immediately
		if (m_now_hsync && (m_trig_vram_accs & OP_TCOPY) != 0) {
			raster_copy_tvram();
			m_vram_accs &= ~OP_TCOPY;
			m_trig_vram_accs &= ~OP_TCOPY;
		}

		break;

	default:
		break;

	}
}

uint32_t CRTC::read_io16(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addrh = (addr >> 1) & 0x3ff;

	// cannot read in CRTC register
	if (addrh == 0x240) {
		data = m_vram_accs;
	} else if (addrh == 0x14 || addrh == 0x15) {
		data = m_regs[addrh];
	}
	return data;
}

uint32_t CRTC::read_io_m(uint32_t addr, uint32_t mask)
{
	return read_io16(addr);
}

// ----------------------------------------------------------------------------

void CRTC::set_max_raster_address(uint16_t data)
{
#ifdef USE_CRTC_MARA
	max_raster_address = data;
	if (max_raster_address <= 0) max_raster_address = 1;
#endif
}

// ----------------------------------------------------------------------------

void CRTC::event_pre_frame()
{
	if(m_timing_changed) {
		m_intr_vline_mask = ~0;

		uint32_t ctrl0 = m_regs[CRTC_CONTROL0];
		if (ctrl0 & CONTROL0_HIRESO) {
			// hireso
			m_now_dot_clocks = m_dot_clocks[1];
			// prescale
			switch(ctrl0 & CONTROL0_HRES) {
//			case CONTROL0_H1024:	// 0x11 is same as 0x10 on X68000 Compact or lator
			case CONTROL0_H768:
				m_now_dot_clocks /= (m_sysport_hrl ? 2. : 2.);
				mv_display_width = 768;
				break;
			case CONTROL0_H1024:	// 0x11 is same as 0x01 on X68000
			case CONTROL0_H512:
				m_now_dot_clocks /= (m_sysport_hrl ? 4. : 3.);
				mv_display_width = 512;
				break;
			default:
				m_now_dot_clocks /= (m_sysport_hrl ? 8. : 6.);
				mv_display_width = 256;
				break;
			}
			// vertical
			switch(ctrl0 & CONTROL0_VRES) {
			case CONTROL0_V1024:
			case CONTROL0_V768:
				m_intr_vline_mask = ~0;
				mv_display_height = 512;
				break;
			case CONTROL0_V512:
				mv_display_height = 512;
				break;
			default:
				mv_display_height = 256;
				break;
			}
		} else {
			// normal
			m_now_dot_clocks = m_dot_clocks[0];
			// prescale
			switch(ctrl0 & CONTROL0_HRES) {
//			case CONTROL0_H1024:	// 0x11 is same as 0x10 on X68000 Compact or lator
//			case CONTROL0_H768:
//				m_now_dot_clocks = m_now_dot_clocks * 3. / 8.;
//				mv_display_width = 768;
//				break;
			case CONTROL0_H1024:	// 0x11 is same as 0x01 on X68000
			case CONTROL0_H512:
				m_now_dot_clocks /= 4.;
				mv_display_width = 512;
				break;
			default:
				m_now_dot_clocks /= 8.;
				mv_display_width = 256;
				break;
			}
			// vertical
			switch(ctrl0 & CONTROL0_VRES) {
			case CONTROL0_V1024:
			case CONTROL0_V768:
			case CONTROL0_V512:
				m_intr_vline_mask = ~0;
				mv_display_height = 512;
				break;
			default:
				mv_display_height = 256;
				break;
			}
		}

#if CRTC_START_POSITION == CRTC_START_DISP
		m_hz_total = (m_regs[CRTC_HORI_TOTAL] + 1) * 8;
		m_hz_start = 0;
		m_hz_disp = (m_regs[CRTC_HORI_END] - m_regs[CRTC_HORI_START]) * 8;
		m_hs_start = m_hz_total - ((m_regs[CRTC_HORI_START] + 1) * 8);
		m_hs_end = m_hs_start + ((m_regs[CRTC_HSYNC_END] + 1) * 8);

#elif CRTC_START_POSITION == CRTC_START_HSYNC
		m_hz_total = (m_regs[CRTC_HORI_TOTAL] + 1) * 8;
		m_hz_start = (m_regs[CRTC_HORI_START] + 1) * 8;
		m_hz_disp = (m_regs[CRTC_HORI_END] - m_regs[CRTC_HORI_START]) * 8;
		m_hs_start = 0;
		m_hs_end = (m_regs[CRTC_HSYNC_END] + 1) * 8;

#endif
		set_vertical_params();

//		if (m_hz_total > 0) {
//			double frames_per_sec = m_now_dot_clocks / (m_hz_total * m_vt_total);
//		}

		int new_vt_total_per_frame;
		double new_vt_frames_per_sec;
		if (m_vt_total <= 10) {
			new_vt_total_per_frame = LINES_PER_FRAME;
		} else {
			new_vt_total_per_frame = m_vt_total;
		}
		if (m_vt_total <= 10 || m_hz_total <= 10) {
			new_vt_frames_per_sec = FRAMES_PER_SEC;
		} else {
			new_vt_frames_per_sec = m_now_dot_clocks / (m_hz_total * m_vt_total);
		}
		if(m_vt_total_per_frame != new_vt_total_per_frame) {
			m_vt_total_per_frame = new_vt_total_per_frame;
			// set parameter to the event manager
			set_lines_per_frame(m_vt_total_per_frame);
		}
		if(m_vt_frames_per_sec != new_vt_frames_per_sec) {
			m_vt_frames_per_sec = new_vt_frames_per_sec;
			// set parameter to the event manager
			set_frames_per_sec(new_vt_frames_per_sec);
		}

		// adjust width
		if (ctrl0 & CONTROL0_HIRESO) {
			// hireso
			// horizontal
			switch(ctrl0 & CONTROL0_HRES) {
			case CONTROL0_H512:
				if (m_hz_total <= 568) {
					// 384 dot mode
					mv_display_width = 384;
				} else if (m_hz_total > 736) {
					// 768 dot mode (invalid)
					mv_display_width = 768;
				}
				break;
			case CONTROL0_H256:
				if (m_hz_total >= 536) {
					// 448 dot mode
					mv_display_width = 448;
				} else if (m_hz_total >= 456) {
					// 384 dot mode
					mv_display_width = 384;
				}
				break;
			}
			// vertical
			switch(ctrl0 & CONTROL0_VRES) {
			case CONTROL0_V512:
				if (m_sysport_hrl && (m_vt_total <= 260 || m_vt_disp <= 256)) {
					// 256 line mode
					mv_display_height = 256;
				}
				break;
			}
		} else {
			// normal
			// horizontal
			switch(ctrl0 & CONTROL0_HRES) {
			case CONTROL0_H512:
				if (m_hz_total <= 560) {
					// 384 dot mode
					mv_display_width = 384;
				}
				break;
			}
		}

		write_signals(&outputs_chsize, ctrl0);

		emu->set_vm_screen_size(mv_display_width, mv_display_height, 0, 0, 1, 1);

		m_timing_changed = false;
		m_timing_updated = true;

#ifdef CRTC_HORIZ_FREQ
		horiz_freq = 0;
#endif
//		if (outputs_wregs.count) {
//			write_signals(&outputs_wregs, 0);
//		}
	}
#ifdef CRTC_HORIZ_FREQ
	if(horiz_freq != next_horiz_freq) {
		uint8_t r8=regs[8]&3;
		horiz_freq = next_horiz_freq;
		frames_per_sec = (double)horiz_freq / (double)vt_total;
		if(regs[8] & 1) {
			frames_per_sec *= 2; // interlace mode
		}
		set_frames_per_sec(frames_per_sec);
	}
#endif
}

// ----------------------------------------------------------------------------

void CRTC::set_vertical_params()
{
	m_vt_start = 0;
	m_vt_total = m_regs[CRTC_VERT_TOTAL] + 1;
	m_vt_disp = MIN(m_regs[CRTC_VERT_END], m_regs[CRTC_VERT_TOTAL]) - m_regs[CRTC_VERT_START];
	m_vs_start = m_vt_total - (m_regs[CRTC_VERT_START] + 1);
	m_vs_end = m_vs_start + (m_regs[CRTC_VSYNC_END] + 1);
}

// ----------------------------------------------------------------------------

void CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	m_cpu_clocks = new_clocks;
#ifndef CRTC_HORIZ_FREQ
	m_frames_per_sec = new_frames_per_sec;
#endif

	// update event clocks
	m_timing_updated = true;
}

void CRTC::update_raster()
{
#ifdef USE_CRTC_MARA
	m_raster_address += (1);
	if (m_raster_address >= max_raster_address) {
		m_raster_address -= max_raster_address;
		m_memory_address = (m_memory_address) & 0x3fff;
	}
#endif
}

void CRTC::update_config()
{
	update_raster_intr();

//	int hdisp = (m_hz_start + m_hz_disp + pConfig->raster_int_hskew + m_hz_total) % m_hz_total;
	int hdisp = (m_hs_start - 16 + m_hz_total + pConfig->raster_int_hskew) % m_hz_total;
	m_hz_disp_end_clock = (int)((double)m_cpu_clocks * (double)hdisp / m_now_dot_clocks);
}

void CRTC::update_raster_intr()
{
	// update raster interrupt
	m_intr_vline = m_regs[CRTC_RASTER_INT] - m_regs[CRTC_VERT_START] + pConfig->raster_int_vskew - 1;
	// recalc the intr_vline only if this become a minus value.
	if (m_intr_vline < 0) m_intr_vline += (m_regs[CRTC_VERT_TOTAL] + 1);


	OUT_DEBUG_RASINT(_T("RASINT: VTOTAL:%03d VSTART:%03d RASINT:%03d -> val:%03d")
		, m_regs[CRTC_VERT_TOTAL] + 1, m_regs[CRTC_VERT_START] + 1
		, m_regs[CRTC_RASTER_INT] + 1, m_intr_vline
	);
	// interrupt
	set_raster((m_intr_vline & m_intr_vline_mask) == (m_v_count_delay & m_intr_vline_mask));
}

// ----------------------------------------------------------------------------

void CRTC::event_frame()
{
	// update envet clocks after update_timing() is called
	if(m_timing_updated && m_vt_total != 0) {
//		int hdisp = (m_hz_start + m_hz_disp + pConfig->raster_int_hskew + m_hz_total) % m_hz_total;
		int hdisp = (m_hs_start - 16 + m_hz_total + pConfig->raster_int_hskew) % m_hz_total;
		m_hz_disp_end_clock = (int)((double)m_cpu_clocks * (double)hdisp / m_now_dot_clocks);
		m_hs_start_clock = (int)((double)m_cpu_clocks * m_hs_start / m_now_dot_clocks);
		m_hs_end_clock = (int)((double)m_cpu_clocks * m_hs_end / m_now_dot_clocks);
		m_timing_updated = false;
	}
	// high speed clear gvram
	if (m_gcls_count == 0 && (m_trig_vram_accs & OP_GCLS) != 0) {
		m_gcls_count = 512;
		m_trig_vram_accs &= ~OP_GCLS;
	}
}

void CRTC::event_vline(int v, int clock)
{
//	m_h_count = 0;
	m_v_count = v;

#ifdef USE_CRTC_MARA
	if (m_v_count == 0) {
		// start of display timing area

		m_memory_address = (((m_regs[12] << 8) | m_regs[13])) & 0x3fff;
		m_raster_address = 0;

	} else {
		m_raster_address += 1;
		if (m_raster_address >= max_raster_address) {
			m_raster_address -= max_raster_address;
			m_memory_address = (m_memory_address + m_regs[1]) & 0x3fff;
		}
	}
#endif

#if CRTC_START_POSITION == CRTC_START_DISP
	// hsync
	if(m_hs_start < m_hs_end && m_hs_end < m_hz_total + 9) {
		set_hsync(false);
		if (register_id[EVENT_HSYNC_S] != -1) cancel_event(this, register_id[EVENT_HSYNC_S]);
		if (register_id[EVENT_HSYNC_E] != -1) cancel_event(this, register_id[EVENT_HSYNC_E]);
		register_event_by_clock(this, EVENT_HSYNC_S, m_hs_start_clock, false, &register_id[EVENT_HSYNC_S]);
		register_event_by_clock(this, EVENT_HSYNC_E, m_hs_end_clock, false, &register_id[EVENT_HSYNC_E]);
	}

#elif CRTC_START_POSITION == CRTC_START_HSYNC
	// hsync
	if(m_hs_start < m_hs_end) {
		// hs_start is always 0, so hsync start immediately
		set_hsync(true);
		process_command();
		if (register_id[EVENT_HSYNC_E] != -1) cancel_event(this, register_id[EVENT_HSYNC_E]);
		register_event_by_clock(this, EVENT_HSYNC_E, m_hs_end_clock, false, &register_id[EVENT_HSYNC_E]);
	}
#endif

	// raster
	if (register_id[EVENT_RASTER] != -1) cancel_event(this, register_id[EVENT_RASTER]);
	register_event_by_clock(this, EVENT_RASTER, m_hz_disp_end_clock, false, &register_id[EVENT_RASTER]);

	// vdisp
	int vt_disp_start  = m_vt_start + pConfig->vdisp_skew;
	int vt_disp_end    = vt_disp_start + m_vt_disp;
	int vt_disp_start2 = vt_disp_start + m_vt_total;
	bool new_vdisp = ((vt_disp_start <= m_v_count && m_v_count < vt_disp_end) || (vt_disp_start2 <= m_v_count));
	set_vdisp(new_vdisp);

	// vsync
	bool new_vsync = (m_vs_start <= m_v_count && m_v_count < m_vs_end);
	set_vsync(new_vsync);
}

void CRTC::process_command()
{
	// raster copy
	if (m_trig_vram_accs & OP_TCOPY) {
		raster_copy_tvram();
		m_vram_accs &= ~OP_TCOPY;
		m_trig_vram_accs &= ~OP_TCOPY;
	}
	// high speed clear
	if (m_gcls_count) {
		m_gcls_count--;
		if (m_gcls_count == 0) {
			clear_gvram();
			m_vram_accs &= ~OP_GCLS;
			m_trig_vram_accs &= ~OP_GCLS;
		}
	}
}

void CRTC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_RASTER) {
		m_v_count_delay = m_v_count;
		set_raster((m_intr_vline & m_intr_vline_mask) == (m_v_count_delay & m_intr_vline_mask));
	}
	else if(event_id == EVENT_HSYNC_S) {
		set_hsync(true);
		process_command();
	}
	else if(event_id == EVENT_HSYNC_E) {
		set_hsync(false);
	}
	register_id[event_id] = -1;
}

// ----------------------------------------------------------------------------
// special implemented functions in CRTC
// ----------------------------------------------------------------------------

#ifdef OUT_DEBUG_RASCPY
static uint32_t prev_src = 0;
#endif

/// copy raster to raster on the text vram
void CRTC::raster_copy_tvram()
{
	uint32_t src_base = (m_regs[CRTC_RASTER_ADDR] & 0xff00) >> 8;
	uint32_t dst_base = m_regs[CRTC_RASTER_ADDR] & 0xff;

	if (src_base == dst_base) {
		// no copy
		return;
	}

	uint32_t mask = m_regs[CRTC_CONTROL1] & CONTROL1_CP;

#ifdef OUT_DEBUG_RASCPY
	if (prev_src + 1 != src_base) {
		OUT_DEBUG_RASCPY(_T("RASTER COPY DIFF: prev:%02X src:%02X dst:%02X"), prev_src, src_base, dst_base);
	}
	prev_src = src_base;
	OUT_DEBUG_RASCPY(_T("RASTER COPY: src:%02X dst:%02X"), src_base, dst_base);
#endif

	src_base <<= 8;	// 256
	dst_base <<= 8;	// 256

	for(int t = 0; t < 4; t++) {
		if (!(mask & (1 << t))) continue;
		uint32_t src_addr = src_base + (t << 16);
		uint32_t dst_addr = dst_base + (t << 16);
		for(int n = 0; n < 256; n++) {
			pu_tvram[dst_addr & 0xffff] |= 3;
			p_tvram[dst_addr] = p_tvram[src_addr];
			src_addr++;
			dst_addr++;
		}
	}
}

/// clear on the graphic vram
void CRTC::clear_gvram()
{
	if (m_regs[CRTC_CONTROL0] & CONTROL0_SIZE) {
		// 1024 x 1024 mode
		int width = 256;
		if ((m_regs[CRTC_CONTROL0] & CONTROL0_HRES) != 0) {
			// 512line
			width <<= 1;
		}
		int top;
		int bottom;
		top = m_regs[CRTC_GSCROLL_Y_0];
		bottom = top + width;

		int left = m_regs[CRTC_GSCROLL_X_0];

		int by = top;
		by <<= 9;
#if 1
		uint32_t data_mask_base = (((m_regs[CRTC_CONTROL1] & 8) << 9)
							| ((m_regs[CRTC_CONTROL1] & 4) << 6)
							| ((m_regs[CRTC_CONTROL1] & 2) << 3)
							| ((m_regs[CRTC_CONTROL1] & 1))
							) * 15;

		for(int y = top; y < bottom; y++) {
			by &= (0x1ff << 9);

			uint32_t page2 = ((y & 0x200) >> 6);
			uint32_t data_mask = data_mask_base & (0xff << page2);
#ifdef OUT_DEBUG_GCLS
			OUT_DEBUG_GCLS(_T("GCLS 1024x1024: bit:%X mask:%04X"), page_bit, data_mask);
#endif
			data_mask = ~data_mask;

			int bx = left;
			for(int x = 0; x < width; x++) {
				bx &= 0x1ff;
//				pu_gvram[by + bx] |= page_bit;
				p_gvram[by + bx] &= data_mask;
				bx++;
			}
			by += (1 << 9);	//x512
		}
#else
		for(int y = top; y < bottom; y++) {
			by &= (0xff << 10);

			uint32_t page = ((y >> 8) & 3);
			uint32_t page_bit = (1 << page);
			uint32_t data_mask = (0xf << (page * 4));
#ifdef OUT_DEBUG_GCLS
			OUT_DEBUG_GCLS(_T("GCLS 1024x1024: bit:%X mask:%04X"), page_bit, data_mask);
#endif
			if (m_regs[CRTC_CONTROL1] & page_bit) {
				data_mask = ~data_mask;
				int bx = left;
				for(int x = 0; x < width; x++) {
					bx &= 0x1ff;
//					pu_gvram[by + bx] |= page_bit;
					p_gvram[by + bx] &= data_mask;
					p_gvram[(by + bx) | 0x200] &= data_mask;
					bx++;
				}
			}
			by += (1 << 10);	//x1024
		}
#endif
	} else {
		// 512 x 512 mode
		int width = 256;
		if ((m_regs[CRTC_CONTROL0] & CONTROL0_HRES) != 0) {
			// 512line
			width <<= 1;
		}
		uint32_t page_bit;
		uint32_t data_mask;

		data_mask = 0x000f;
		for(int page = 0; page < 4; page++) {
			page_bit = (1 << page);
			if (m_regs[CRTC_CONTROL1] & page_bit) {
#ifdef OUT_DEBUG_GCLS
				OUT_DEBUG_GCLS(_T("GCLS 512x512: bit:%X mask:%04X"), page_bit, data_mask);
#endif
				data_mask = ~data_mask;
				int top = m_regs[CRTC_GSCROLL_Y_0 + page * 2];
				int left = m_regs[CRTC_GSCROLL_X_0 + page * 2];
				int by = top;
				by <<= 9;
				for(int y = 0; y < width; y++) {
					by &= (0x1ff << 9);
					int bx = left;
					for(int x = 0; x < width; x++) {
						bx &= 0x1ff;
//						pu_gvram[by + bx] |= page_bit;
						p_gvram[by + bx] &= data_mask;
						bx++;
					}
					by += (1 << 9);	//x512
				}
				data_mask = ~data_mask;
			}
			data_mask <<= 4;
		}
	}
}

// ----------------------------------------------------------------------------

#if 0
void CRTC::set_display(bool val)
{
	if(m_display != val) {
		write_signals(&outputs_disp, val ? 0xffffffff : 0);
		m_display = val;
	}
}
#endif

void CRTC::set_raster(bool val)
{
	if(m_now_raster != val) {
		write_signals(&outputs_raster, val ? 0xffffffff : 0);
		m_now_raster = val;
	}
}

void CRTC::set_vdisp(bool val)
{
	if(m_now_vdisp != val) {
		write_signals(&outputs_vdisp, val ? 0xffffffff : 0);
		m_now_vdisp = val;
	}
}

void CRTC::set_vsync(bool val)
{
	if(m_now_vsync != val) {
#ifdef USE_CRTC_MARA
		if (val) {
			// reset start raster
			m_raster_address = 0;
		}
#endif
		write_signals(&outputs_vsync, val ? 0xffffffff : 0);
		m_now_vsync = val;
	}
}

void CRTC::set_hsync(bool val)
{
	if(m_now_hsync != val) {
		write_signals(&outputs_hsync, val ? 0xffffffff : 0);
		m_now_hsync = val;
	}
}

void CRTC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_HRL:
		// select prescaler of the hireso clock $e8e007 SYSPORT:R4 bit1
		if (m_sysport_hrl != (data & 2)) {
			m_timing_changed = true;
		}
		m_sysport_hrl = (data & 2);
		break;
	}
}

uint32_t CRTC::read_signal(int id)
{
	uint32_t data = 0;
	switch(id) {
	case SIG_HRL:
		data = (m_sysport_hrl & 2);
		break;
	}
	return data;
}

/// b13: hireso
uint32_t CRTC::get_led_status()
{
	return ((m_regs[CRTC_CONTROL0] & CONTROL0_HIRESO) << (13 - CONTROL0_HIRESO_SFT));
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void CRTC::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<24; i++) {
		SET_Uint16_LE(m_regs[i]);
	}
	// 3
	SET_Uint16_LE(m_vram_accs);		///< CRTC operation port
	SET_Uint16_LE(m_trig_vram_accs);
	SET_Int32_LE(m_intr_vline);		///< rater interrupt
	SET_Int32_LE(m_gcls_count);		///< high speed clear count
	// 4
	SET_Double_LE(m_now_dot_clocks);
	SET_Double_LE(m_frames_per_sec);
	// 5
#ifdef CRTC_HORIZ_FREQ
	SET_Int32_LE(horiz_freq);
	SET_Int32_LE(next_horiz_freq);
#endif
	SET_Int32_LE(m_cpu_clocks);
	SET_Int32_LE(m_hz_total);
	// 6
	SET_Int32_LE(m_hz_start);
	SET_Int32_LE(m_hz_disp);
	SET_Int32_LE(m_hs_start);
	SET_Int32_LE(m_hs_end);
	// 7
	SET_Int32_LE(m_vt_start);
	SET_Int32_LE(m_vt_total);
	SET_Int32_LE(m_vt_disp);
	SET_Int32_LE(m_vt_total_per_frame);
	// 8
	SET_Int32_LE(m_vs_start);
	SET_Int32_LE(m_vs_end);
	SET_Int32_LE(m_v_count);
	SET_Int32_LE(m_hz_disp_end_clock);
	// 9
	SET_Int32_LE(m_hs_start_clock);
	SET_Int32_LE(m_hs_end_clock);
	SET_Double_LE(m_vt_frames_per_sec);
	// 10
	SET_Bool(m_now_raster);
	SET_Bool(m_now_vdisp);
	SET_Bool(m_now_vsync);
	SET_Bool(m_now_hsync);
	SET_Byte(m_sysport_hrl);
#ifdef USE_CRTC_MARA
	SET_Int32_LE(m_memory_address);			///< refresh memory address(MA)
	SET_Int32_LE(m_raster_address);			///< raster address (RA)
	// 11
	SET_Int32_LE(max_raster_address);		///< max raster address
#endif
	SET_Int32_LE(mv_display_width);
	SET_Int32_LE(mv_display_height);
//	SET_Int32_LE(m_interlace);				///< 0:noninterlace 1:interlace
	// 12
//	SET_Int32_LE(m_raster_per_dot);			///< raster per dot (set 1 on hireso 256line)
	for(int i=0; i<3; i++) {
		SET_Int32_LE(register_id[i]);
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

bool CRTC::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	for(int i=0; i<24; i++) {
		GET_Uint16_LE(m_regs[i]);
	}
	// 3
	GET_Uint16_LE(m_vram_accs);		///< CRTC operation port
	GET_Uint16_LE(m_trig_vram_accs);
	GET_Int32_LE(m_intr_vline);		///< rater interrupt
	GET_Int32_LE(m_gcls_count);		///< high speed clear count
	// 4
	GET_Double_LE(m_now_dot_clocks);
	GET_Double_LE(m_frames_per_sec);
	// 5
#ifdef CRTC_HORIZ_FREQ
	GET_Int32_LE(horiz_freq);
	GET_Int32_LE(next_horiz_freq);
#endif
	GET_Int32_LE(m_cpu_clocks);
	GET_Int32_LE(m_hz_total);
	// 6
	GET_Int32_LE(m_hz_start);
	GET_Int32_LE(m_hz_disp);
	GET_Int32_LE(m_hs_start);
	GET_Int32_LE(m_hs_end);
	// 7
	GET_Int32_LE(m_vt_start);
	GET_Int32_LE(m_vt_total);
	GET_Int32_LE(m_vt_disp);
	GET_Int32_LE(m_vt_total_per_frame);
	// 8
	GET_Int32_LE(m_vs_start);
	GET_Int32_LE(m_vs_end);
	GET_Int32_LE(m_v_count);
	GET_Int32_LE(m_hz_disp_end_clock);
	// 9
	GET_Int32_LE(m_hs_start_clock);
	GET_Int32_LE(m_hs_end_clock);
	GET_Double_LE(m_vt_frames_per_sec);
	// 10
	GET_Bool(m_now_raster);
	GET_Bool(m_now_vdisp);
	GET_Bool(m_now_vsync);
	GET_Bool(m_now_hsync);
	GET_Byte(m_sysport_hrl);
#ifdef USE_CRTC_MARA
	GET_Int32_LE(m_memory_address);			///< refresh memory address(MA)
	GET_Int32_LE(m_raster_address);			///< raster address (RA)
	// 11
	GET_Int32_LE(max_raster_address);		///< max raster address
#endif
	GET_Int32_LE(mv_display_width);
	GET_Int32_LE(mv_display_height);
//	GET_Int32_LE(m_interlace);				///< 0:noninterlace 1:interlace
	// 12
//	GET_Int32_LE(m_raster_per_dot);			///< raster per dot (set 1 on hireso 256line)
	for(int i=0; i<3; i++) {
		GET_Int32_LE(register_id[i]);
	}

	m_v_count_delay = m_v_count;

	m_timing_changed = true;

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t CRTC::debug_read_io16(uint32_t addr)
{
	uint32_t data = 0;
	uint32_t addrh = (addr >> 1) & 0x1f;

	if (addrh < 24) {
		data = m_regs[addrh];
	} else if (addrh == 0x240) {
		data = m_vram_accs;
	}
	return data;
}

static const _TCHAR *c_reg_names[] = {
	_T("HTOTAL"),
	_T("HSYNC_END"),
	_T("HDISP_STA"),
	_T("HDISP_END"),
	_T("VTOTAL"),
	_T("VSYNC_END"),
	_T("VDISP_STA"),
	_T("VDISP_END"),
	_T("EX_H_ADJUST"),
	_T("RASTER_INT"),
	_T("TX_SCRL_X"),
	_T("TX_SCRL_Y"),
	_T("GR0_SCRL_X"),
	_T("GR0_SCRL_Y"),
	_T("GR1_SCRL_X"),
	_T("GR1_SCRL_Y"),
	_T("GR2_SCRL_X"),
	_T("GR2_SCRL_Y"),
	_T("GR3_SCRL_X"),
	_T("GR3_SCRL_Y"),
	_T("CONTROL"),
	_T("TX_ACCESS"),
	_T("RASTER_COPY"),
	_T("TX_BITMASK"),
	NULL
};

bool CRTC::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool CRTC::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < 24) {
		m_regs[reg_num] = (data & c_regs_mask[reg_num]);
	} else if (reg_num == 25) {
		m_vram_accs = (data & 0xf);
	} else {
		return false;
	}
	return true;
}

static const _TCHAR *c_r20_sizcolors[] = {
	_T("Real 512x 512,    16colors x4pages "),
	_T("Real 512x 512,   256colors x2pages "),
	_T("Real 512x 512, Invalid( 4096colors)"),
	_T("Real 512x 512, 65536colors x1page  "),
	_T("Real1024x1024,    16colors x1page  "),
	_T("Real1024x1024, Invalid(  256colors)"),
	_T("Real1024x1024, Invalid( 4096colors)"),
	_T("Real1024x1024, Invalid(65536colors)"),
};

static const _TCHAR *c_r20_reso[] = {
	_T("Normal 0x00  256x256"),
	_T("Normal 0x01  512x256"),
	_T("Normal 0x02 (768x256)"),
	_T("Normal 0x03 (768?x256)"),

	_T("Normal 0x04 (256x512 I)"),
	_T("Normal 0x05  512x512 I"),
	_T("Normal 0x06 (768x512 I)"),
	_T("Normal 0x07 (768?x512 I)"),

	_T("Normal 0x08 (256x512 I?)"),
	_T("Normal 0x09 (512x512 I?)"),
	_T("Normal 0x0a (768x512 I?)"),
	_T("Normal 0x0b (768?x512 I?)"),

	_T("Normal 0x0c (256x1024 I?)"),
	_T("Normal 0x0d (512x1024 I?)"),
	_T("Normal 0x0e (768x1024 I?)"),
	_T("Normal 0x0f (768?x1024 I?)"),

	_T("Hireso 0x10  256x256"),
	_T("Hireso 0x11  512x256"),
	_T("Hireso 0x12 (768x256)"),
	_T("Hireso 0x13 (768?x256)"),

	_T("Hireso 0x14 (256x512)"),
	_T("Hireso 0x15  512x512"),
	_T("Hireso 0x16  768x512"),
	_T("Hireso 0x17  768?x512"),

	_T("Hireso 0x18 (256x512 I?)"),
	_T("Hireso 0x19 (512x512 I?)"),
	_T("Hireso 0x1a (768x512 I?)"),
	_T("Hireso 0x1b (768?x512 I?)"),

	_T("Hireso 0x1c (256x1024 I?)"),
	_T("Hireso 0x1d (512x1024 I?)"),
	_T("Hireso 0x1e (768x1024 I?)"),
	_T("Hireso 0x1f (768?x1024 I?)"),
};

void CRTC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("CRTC:\n"));
	for(int i=0; i<20; i++) {
		UTILITY::sntprintf(buffer, buffer_len
			,_T(" %02X(%-12s):%04X")
			, i, c_reg_names[i], m_regs[i]);
		if ((i & 3) == 3) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
	for(int i=20; i<24; i++) {
		int n0,n1;
		UTILITY::sntprintf(buffer, buffer_len
			,_T(" %02X(%-12s):%04X ")
			, i, c_reg_names[i], m_regs[i]);
		switch(i) {
		case 20:
			n0 = (m_regs[i] & (CONTROL0_SIZE | CONTROL0_COLOR)) >> CONTROL0_COLOR_SFT;
			n1 = (m_regs[i] & (CONTROL0_HIRESO | CONTROL0_VHRES));
			UTILITY::sntprintf(buffer, buffer_len, _T("Gr:%s Reso:%s\n")
				, c_r20_sizcolors[n0]
				, c_r20_reso[n1]);
			break;
		case 21:
			UTILITY::sntprintf(buffer, buffer_len, _T("Tx BitMask:%d ParallelAccs:%d T%c%c%c%c Copy/Cls:T%c%c%c%c\n")
				, (m_regs[i] & CONTROL1_MEN) ? 1 : 0
				, (m_regs[i] & CONTROL1_SA) ? 1 : 0
				, (m_regs[i] & CONTROL1_AP3) ? _T('3') : _T('-')
				, (m_regs[i] & CONTROL1_AP2) ? _T('2') : _T('-')
				, (m_regs[i] & CONTROL1_AP1) ? _T('1') : _T('-')
				, (m_regs[i] & CONTROL1_AP0) ? _T('0') : _T('-')
				, (m_regs[i] & CONTROL1_CP3) ? _T('3') : _T('-')
				, (m_regs[i] & CONTROL1_CP2) ? _T('2') : _T('-')
				, (m_regs[i] & CONTROL1_CP1) ? _T('1') : _T('-')
				, (m_regs[i] & CONTROL1_CP0) ? _T('0') : _T('-')
			);
			break;
		case 22:
			UTILITY::sntprintf(buffer, buffer_len, _T("RasCopy src:$%02X -> dst:$%02X\n")
				, (m_regs[i] & 0xff00) >> 8, m_regs[i] & 0xff);
			break;
		case 23:
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
			break;
		}
	}
	UTILITY::sntprintf(buffer, buffer_len
		, _T(" %02X(VRAM_ACCESS ):%02X\n"), 25, m_vram_accs);

	int vcnt = m_v_count + m_regs[CRTC_VERT_START] + 1;
	if (vcnt >= m_vt_total) vcnt -= m_vt_total;
	UTILITY::sntprintf(buffer, buffer_len
		, _T(" CURRENT LINE:%04X (inner:%04X)\n"), vcnt, m_v_count);
}
#endif
