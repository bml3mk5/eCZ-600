/** @file display.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ display and video controller ]
*/

#include "display.h"
#include <stdlib.h>
#include "../../emu.h"
#include "crtc.h"
#include "sprite_bg.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"

#define CURSOR_MAXPOS 0xffffff

#ifdef _DEBUG
//#define OUT_DEBUG2	logging->out_debugf
#define OUT_DEBUG2(...)
//#define OUT_DEBUG_PAL	logging->out_debugf
#define OUT_DEBUG_PAL(...)
//#define OUT_DEBUG_VC	logging->out_debugf
#define OUT_DEBUG_VC(...)
#define OUT_DEBUG_CONT(...)
#define OUT_DEBUG_BRIGHT(...)
//#define OUT_DEBUG_SHFLGS(vmin, vmax, ...)	if (vline <= vmin || vmax <= vline) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_SHFLGS(...)
#else
#define OUT_DEBUG2(...)
#define OUT_DEBUG_PAL(...)
#define OUT_DEBUG_VC(...)
#define OUT_DEBUG_CONT(...)
#define OUT_DEBUG_BRIGHT(...)
#define OUT_DEBUG_SHFLGS(...)
#endif

static const int c_priority[4] = { 0, 1, 2, 1 };

void DISPLAY::initialize()
{
	vm_pause = emu->get_pause_ptr();

	m_curr_contrast = 0;
	m_tag_contrast = 0;
	m_contrast_count = 0;

	emu->get_rgbaformat(&Rmask, &Gmask, &Bmask, &Amask, &Rshift, &Gshift, &Bshift, &Ashift);

	// create pc palette

	screen_left = 0;
	screen_top = 0;
	screen_width = SCREEN_WIDTH; // + screen_left;
	screen_height = SCREEN_HEIGHT; // + screen_top;

//	update_chr_clocks(0);

	// synchronizable vertical range
//	v_total_min[0] = (262 - 12);
//	v_total_max[0] = (262 + 18);
//	v_total_min[1] = (486 - 12);
//	v_total_max[1] = (486 + 18);
//	v_total_min[2] = (518 - 12);
//	v_total_max[2] = (518 + 18);

//	dws_left_diff = 0;

	// synchronizable horizontal range
//	for(int i=0; i<4; i++) {
//		h_total_min[i] = CHARS_PER_LINE * (i+1) - 3;
//		h_total_max[i] = CHARS_PER_LINE * (i+1) + 2;
//	}

	m_raster_even_odd = 0;
	m_raster_mode = RASTER_NORMAL_NONINTER;

	m_show_screen = 0xffffffff;

	m_draw_even_odd = 0;

	skip_frame_count = 0;

	mv_display_width = 4;
	mv_display_height = 4;
	mv_display_left = 0;
	mv_display_top = 0;
	mv_display_right = 4;
	mv_display_bottom = 4;

	m_crtc_left_base = 0;
	m_crtc_top_base = 0;

	make_bright_table();

	dws_a = Amask;

	update_config();

	register_frame_event(this);
}

void DISPLAY::reset()
{
	m_curr_contrast = 0;
	m_tag_contrast = 0;
	m_contrast_count = 0;

	memset(m_palette , 0xff, sizeof(m_palette));
	memset(m_vc_regs, 0, sizeof(m_vc_regs));
	memset(m_crtc_prev, 0, sizeof(m_crtc_prev));
	memset(m_vc_prev, 0, sizeof(m_vc_prev));
	for(int i=0; i<4; i++) {
		m_vc_priscrn[i] = i;
		m_vc_priority[i] = i;
		m_vc_gr_pripage[i] = i;
		m_vc_gr_prisfts[i] = (i << 2);
	}

#ifdef USE_GRAPHIC_RENDER
	memset(rb_gvram, 0, sizeof(rb_gvram));
#endif
	memset(rb_tvram, 0, sizeof(rb_tvram));

	memset(rb_bgram0, 0, sizeof(rb_bgram0));
	memset(rb_bgram1, 0, sizeof(rb_bgram1));

#ifdef USE_SPRITE_RENDER
	memset(rb_spram, 0, sizeof(rb_spram));
#endif
	memset(rb_pcg, 0, sizeof(rb_pcg));
	memset(mx_txspbg, 0, sizeof(mx_txspbg));
	memset(mx_buf, 0, sizeof(mx_buf));
#ifdef DEBUG_SHOW_FLAGS
	memset(md_show_screen, 0, sizeof(md_show_screen));
	md_show_screen_cnt = 0;
#endif
}

void DISPLAY::set_context_crtc(CRTC* device)
{
	d_crtc = device;
	if (d_crtc) {
		p_crtc_regs = d_crtc->get_regs();
	}
}

void DISPLAY::set_context_sprite_bg(SPRITE_BG* device)
{
	d_sp_bg = device;
	if (d_sp_bg) {
		p_sp_regs = d_sp_bg->get_sp_regs();
		p_bg_regs = d_sp_bg->get_bg_regs();
	}
}

void DISPLAY::update_config()
{
//	update_dws_params();

	set_border_color();
}

//void DISPLAY::update_dws_params()
//{
//}

void DISPLAY::set_border_color()
{
	m_border_color = FLG_ORIG_BORDER_COLOR ? emu->map_rgbcolor(0x24, 0x24, 0x24) : emu->map_rgbcolor(0, 0, 0);
}

//void DISPLAY::update_chr_clocks(int clk)
//{
//}

/// @brief Create the bright table of screen color 
void DISPLAY::make_bright_table()
{
	for(int i=0; i<16; i++) {
		for(int j=0; j<32; j++) {
			for(int k=0; k<2; k++) {
				double bright = (double)i*(j*2+1-k)/4.0;
				if (bright < 0.0) bright = 0.0;
				c_bright_table[i][j][k] = (uint8_t)bright;

				OUT_DEBUG_BRIGHT(_T("bright: cont:%X data:%02X inte:%X %02X"), i, j, k, c_bright_table[i][j][k]);
			}
		}
	}
}

#define STORE_DATA(mem, dat, msk) mem = ((mem & (msk)) | (dat))

void DISPLAY::write_io16(uint32_t addr, uint32_t data)
{
	write_io_m(addr, data, 0xffff0000);
}

void DISPLAY::write_io_m(uint32_t addr, uint32_t data, uint32_t mask)
{
	uint32_t addrh = (addr >> 1);

	switch(addr & 0x0f00) {
	case 0x000:
	case 0x100:
	case 0x200:
	case 0x300:
		// palette
		STORE_DATA(m_palette[addrh & 0x1ff], data, mask);

		OUT_DEBUG_PAL(_T("PALETTE %06X:%04X (data:%04X mask:%04X)"), addr, m_palette[addrh & 0x1ff], data, mask);
		break;
	case 0x400:
		// video control

//		if ((addrh & 0x7f) == 0) {
		{
			STORE_DATA(m_vc_regs[VC_GRAPHIC_SIZE], data, (mask & VC_GS_SZCO_MASK));
			OUT_DEBUG_VC(_T("%06X: VC_GR_SIZE: %04X"), addr, m_vc_regs[VC_GRAPHIC_SIZE]);
		}
		break;
	case 0x500:
		// video control
//		if ((addrh & 0x7f) == 0) {
		{
			// priority
			STORE_DATA(m_vc_regs[VC_PRIORITY], data, mask);

			// set priority
			int n;
			for(n=0; n<4; n++) m_vc_priscrn[n] = -1;
			n = (m_vc_regs[VC_PRIORITY] & VC_PR_TX_MASK) >> VC_PR_TX_SFT;
			n = c_priority[n];
			while (m_vc_priscrn[n] >= 0 && n < 4) n++;
			m_vc_priority[PR_TX] = n;
			m_vc_priscrn[n] = PR_TX;
			n = (m_vc_regs[VC_PRIORITY] & VC_PR_SP_MASK) >> VC_PR_SP_SFT;
			n = c_priority[n];
			while (m_vc_priscrn[n] >= 0 && n < 4) n++;
			m_vc_priority[PR_SP] = n;
			m_vc_priscrn[n] = PR_SP;
			n = (m_vc_regs[VC_PRIORITY] & VC_PR_GR_MASK) >> VC_PR_GR_SFT;
			n = c_priority[n];
			while (m_vc_priscrn[n] >= 0 && n < 4) n++;
			m_vc_priority[PR_GR] = n;
			m_vc_priscrn[n] = PR_GR;

			// set graphics priority
			int pri;
			for(pri=0; pri<4; pri++) {
				m_vc_gr_pripage[pri] = ((m_vc_regs[VC_PRIORITY] >> (pri << 1)) & 3);
			}

			int exists = 0;
			int page;
			for(pri=0; pri<4; pri++) {
				page = m_vc_gr_pripage[pri];
				while(exists & (1 << page)) {
					page++;
					page &= 3;
				}
				m_vc_gr_prisfts[pri] = (page << 2);
				exists |= (1 << page);
			}

			OUT_DEBUG_VC(_T("%06X: VC_PRIORITY: %04X  PR:%d>%d>%d  GR:%d>%d>%d>%d")
				, addr
				, m_vc_regs[VC_PRIORITY]
				, m_vc_priscrn[0] , m_vc_priscrn[1] , m_vc_priscrn[2]
				, m_vc_gr_pripage[0] , m_vc_gr_pripage[1] , m_vc_gr_pripage[2], m_vc_gr_pripage[3]);
		}
		break;
	case 0x600:
		// video control
//		if ((addrh & 0x7f) == 0) {
		{
			STORE_DATA(m_vc_regs[VC_TRANS_ONOFF], data, mask);

			OUT_DEBUG_VC(_T("%06X: VC_TRANS_ONOFF: %04X"), addr, m_vc_regs[VC_TRANS_ONOFF]);
		}
		break;
	}
}

uint32_t DISPLAY::read_io16(uint32_t addr)
{
	uint32_t data = 0xffff;
	uint32_t addrh = (addr >> 1);

	switch(addr & 0x0f00) {
	case 0x000:
	case 0x100:
	case 0x200:
	case 0x300:
		// palette
		data = m_palette[addrh & 0x1ff];
		break;
	case 0x400:
		// video control
		data = m_vc_regs[VC_GRAPHIC_SIZE];
		break;
	case 0x500:
		// video control
		data = m_vc_regs[VC_PRIORITY];
		break;
	case 0x600:
		// video control
		data = m_vc_regs[VC_TRANS_ONOFF];
		break;
	}
	return data;
}

uint32_t DISPLAY::read_io_m(uint32_t addr, uint32_t mask)
{
	return read_io16(addr);
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_DISPLAY_VSYNC:
			if ((data & mask) == 0) {
#ifdef DEBUG_CRTMON
				logging->out_debugf(_T("DISPLAY:SIG_DISPLAY_VSYNC_OFF: crtc:%d nras:%d cmr:%d cmc:%d cmras:%d top:%d btm:%d vtop:%d vbtm:%d")
					,(*crtc_vt_count), crt_mon_now_raster
					,crt_mon_row, crt_mon_col, (*crtc_ra)
					,top_raster, bottom_raster, vsync_top_raster, vsync_bottom_raster
				);
#endif
			} else {
				// synchronize display vertical raster
#if 0
				for(int i=0; i<3; i++) {
					if (v_total_min[i] <= crt_mon_v_count && crt_mon_v_count <= v_total_max[i]) {
						crt_mon_v_start = 0;
						crt_mon_v_count = 0;
						crt_mon_row = CRT_MON_HEIGHT - 2;
						break;
					}
				}
#endif
			}
			break;
//		case SIG_DISPLAY_WRITE_REGS:
//			update_dws_params();
//			break;
		case SIG_CONTRAST:
			if (m_contrast_count) {
				if (m_curr_contrast < m_tag_contrast) {
					m_curr_contrast++;
				} else if (m_curr_contrast > m_tag_contrast) {
					m_curr_contrast--;
				}
				m_curr_contrast &= 0xf;
			}
			m_tag_contrast = (data & mask & 0x0f);
			if (m_tag_contrast != m_curr_contrast) {
				m_contrast_count = CONTRAST_COUNT;
			}
			OUT_DEBUG_CONT(_T("CONTRAST SET: c:%d t:%d n:%d"), m_curr_contrast, m_tag_contrast, m_contrast_count);
			break;
#if 0
		case SIG_DISPLAY_SIZE:
			// set current display size (in vm) $e80028 CRTC:R20
			set_display_mode(data);
			break;
#endif
	}
}

uint32_t DISPLAY::read_signal(int id)
{
	uint32_t data = 0xffff;
	switch(id) {
	case SIG_CONTRAST:
		data = 0xf0 | (m_tag_contrast & 0xf);
		break;
	}
	return data;
}

static const int freq_list[4] = { 31, 15, 24, 0 };

/// set current display size (in vm)
/// @param[in] width  : width of display area
/// @param[in] height : height of display area
/// @param[in] mode   : $e80028 CRTC:R20(b0-b4)
///                   , HRL(b5)
///                   , Horizontal Freq(b6-b7) 0:31kHz 0x40:15kHz 0x80:24kHz
/// @param[in] raster : raster mode
void DISPLAY::set_display_mode(int left, int top, int width, int height, uint32_t mode, en_raster_modes raster)
{
	m_raster_mode = raster;
	mv_display_width = width;
	mv_display_height = height;
#if 0
	if (data & CRTC::CZ_HIRESO) {
		// hireso
		// horizontal
		switch(data & CRTC::CZ_HRES) {
		case CRTC::CZ_H256:
			// 256
			if (data & CRTC::CZ_HRL) {
				// 448 mode
//				mv_display_width = 448;
//				m_crtc_left_base = 8;
				mv_display_width = 384;
				m_crtc_left_base = 10;
			} else if (*p_crtc_hz_total >= 456) {
				// 384 mode (24kHz setting)
				mv_display_width = 384;
				m_crtc_left_base = 8;
			} else {
				// 256 mode
				mv_display_width = 256;
				m_crtc_left_base = 6;
			}
			break;
		case CRTC::CZ_H512:
		case CRTC::CZ_H1024:	// 0x11 is same as 0x01 on X68000 
			// 512
			if (data & CRTC::CZ_HRL) {
				// 384 dot mode
				mv_display_width = 384;
				m_crtc_left_base = 12;
			} else if (*p_crtc_hz_total <= 736) {
				// 512 dot (normal mode)
				mv_display_width = 512;
				m_crtc_left_base = 17;
			} else {
				// 768 dot (24kHz setting)
				mv_display_width = 768;
				m_crtc_left_base = 20;
			}
			break;
		case CRTC::CZ_H768:
//		case CRTC::CZ_H1024:	// 0x11 is same as 0x10 on X68000 Compact or lator
			// 768
			mv_display_width = 768;
			m_crtc_left_base = 28;
			break;
		}
		// vertical
		switch(data & CRTC::CZ_VRES) {
		case CRTC::CZ_V256:
			// 256
//			mv_display_height = 256;
			mv_display_height = 512;	// double raster
			m_crtc_top_base = 40;
			m_raster_mode = RASTER_HIRESO_DOUBLE;
			break;
		case CRTC::CZ_V512:
			// 512
			if (data & CRTC::CZ_HRL) {
				// 256line (Horizontal 15kHz)
				mv_display_height = 256;
				m_crtc_top_base = 8;
			} else {
				// 512line
				mv_display_height = 512;
				m_crtc_top_base = 40;
			}
			m_raster_mode = RASTER_HIRESO_NONINTER;
			break;
		case CRTC::CZ_V768:
			// 512? (invalid)
			mv_display_height = 512;
			m_crtc_top_base = 120;
			m_raster_mode = RASTER_HIRESO_INTERLACE_512;
			break;
		case CRTC::CZ_V1024:
			// 1024 (invalid)
			mv_display_height = 512;
			m_crtc_top_base = 40;
			m_raster_mode = RASTER_HIRESO_INTERLACE_1024;
			break;
		}
	} else {
		// normal

		// horizontal
		switch(data & CRTC::CZ_HRES) {
		case CRTC::CZ_H256:
		case CRTC::CZ_H768:
//		case CRTC::CZ_H1024:	// 0x11 is same as 0x10 on X68000 Compact or lator
			// 256
			mv_display_width = 256;
			m_crtc_left_base = 0;
			break;
		case CRTC::CZ_H512:
		case CRTC::CZ_H1024:	// 0x11 is same as 0x01 on X68000
			// 512
			if (*p_crtc_hz_total <= 560) {
				// 384 dot mode
				mv_display_width = 384;
				m_crtc_left_base = 10;
			} else {
				// 512 dot (normal)
				mv_display_width = 512;
				m_crtc_left_base = 5;
			}
			break;
		}

		// vertical
		switch(data & CRTC::CZ_VRES) {
		case CRTC::CZ_V256:
			// 256
			mv_display_height = 256;
			m_crtc_top_base = 16;
			m_raster_mode = RASTER_NORMAL_NONINTER;
			break;
		case CRTC::CZ_V512:
			// 512
			mv_display_height = 512;
			m_crtc_top_base = 16;
			m_raster_mode = RASTER_NORMAL_INTERLACE_512;
			break;
		case CRTC::CZ_V768:
			// 512? (invalid)
			mv_display_height = 512;
			m_crtc_top_base = 16;
			m_raster_mode = RASTER_NORMAL_INTERLACE_512;
			break;
		case CRTC::CZ_V1024:
			// 1024 (invalid)
			mv_display_height = 512;
			m_crtc_top_base = 16;
			m_raster_mode = RASTER_NORMAL_INTERLACE_512;
			break;
		}
	}
#endif
	//
	m_crtc_left_base = p_crtc_regs[CRTC::CRTC_HORI_START] - p_crtc_regs[CRTC::CRTC_HSYNC_END];
	if (m_crtc_left_base < 0) m_crtc_left_base /= 4;
	m_crtc_left_base -= left;
	m_crtc_left_base *= 8;
	m_crtc_top_base = (p_crtc_regs[CRTC::CRTC_VERT_START] - p_crtc_regs[CRTC::CRTC_VSYNC_END] - top);

	// screen width is base on horizontal total in crtc 
	if (mv_display_width == *p_crtc_hz_disp) {
		m_crtc_left_base = 0;	// fit the left side
	} else {
		if (*p_crtc_hz_disp < 128) {
			mv_display_width = 128;
		} else {
			mv_display_width = *p_crtc_hz_disp;
		}
	}

	// screen height is base on vertical total in crtc
	int dh2, ch2;
	switch(m_raster_mode) {
	case RASTER_NORMAL_NONINTER:
		dh2 = mv_display_height / 2;
		ch2 = *p_crtc_vt_disp;
		if (dh2 <= ch2 && ch2 <= (dh2 * 3)) {
			mv_display_height = ch2;
		}
		break;
	case RASTER_NORMAL_INTERLACE_512:
		m_crtc_top_base *= 2;
		dh2 = mv_display_height / 2;
		ch2 = *p_crtc_vt_disp;
		ch2 *= 2;
		if (dh2 <= ch2 && ch2 <= (dh2 * 3)) {
			mv_display_height = ch2;
		}
		break;
	case RASTER_HIRESO_DOUBLE:
		dh2 = mv_display_height / 2;
		ch2 = *p_crtc_vt_disp;
		if (dh2 <= ch2 && ch2 <= (dh2 * 3)) {
			mv_display_height = ch2;
		}
		break;
	case RASTER_HIRESO_INTERLACE_512:
		dh2 = mv_display_height / 2;
		ch2 = *p_crtc_vt_disp;
		ch2 *= 2;
		if (dh2 <= ch2 && ch2 <= (dh2 * 3)) {
			mv_display_height = ch2;
		}
		break;
	case RASTER_HIRESO_INTERLACE_1024:
		dh2 = mv_display_height / 4;
		ch2 = *p_crtc_vt_disp;
		if (dh2 <= ch2 && ch2 <= (dh2 * 6)) {
			mv_display_height = ch2;
		}
		break;
	default:
		dh2 = mv_display_height / 2;
		ch2 = *p_crtc_vt_disp;
		if (dh2 <= ch2 && ch2 <= (dh2 * 3)) {
			mv_display_height = ch2;
		}
		break;
	}
	//
	mv_display_left = screen_left + m_crtc_left_base;
	mv_display_top = screen_top + m_crtc_top_base;
	mv_display_right = mv_display_left + mv_display_width;
	mv_display_bottom = mv_display_top + mv_display_height;

	if (mv_display_right >= (1 << MX_BUF_WIDTH_SFT)) {
		// adjust horizontal position
		int adj = (mv_display_right - (1 << MX_BUF_WIDTH_SFT));
		if (adj >= mv_display_left) {
			adj = mv_display_left;
		}
		mv_display_left -= adj;
		mv_display_right -= adj;
		if (mv_display_right >= (1 << MX_BUF_WIDTH_SFT)) {
			mv_display_right = (1 << MX_BUF_WIDTH_SFT);
		}
	}
	if (mv_display_bottom >= SCREEN_HEIGHT) {
		// adjust vertical position
		int adj = (mv_display_bottom - SCREEN_HEIGHT);
		if (adj >= mv_display_top) {
			adj = mv_display_top;
		}
		mv_display_top -= adj;
		mv_display_bottom -= adj;
		if (mv_display_bottom >= SCREEN_HEIGHT) {
			mv_display_bottom = SCREEN_HEIGHT;
		}
	}

	logging->out_logf(LOG_DEBUG, _T("%dx%d %dKHz (%dx%d CRTC_R20:0x%02x SYSPORT_HRL:%x)")
		, mv_display_width, mv_display_height, freq_list[(mode >> 6) & 3]
		, width, height
		, (mode & 0x1f), ((mode >> 4) & 2)
	);
#ifdef _DEBUG
	logging->out_debugf(_T("mv_display: l:%d t:%d r:%d b:%d")
		, mv_display_left, mv_display_top, mv_display_right, mv_display_bottom);
#endif
}

void DISPLAY::set_display_size(int left, int top, int right, int bottom)
{
	screen_left = left;
	screen_width = right;
	screen_top = top;
	screen_height = bottom;

	mv_display_left = screen_left + m_crtc_left_base;
	mv_display_top = screen_top + m_crtc_top_base;
	mv_display_right = mv_display_left + mv_display_width;
	mv_display_bottom = mv_display_top + mv_display_height;

	if (mv_display_right >= (1 << MX_BUF_WIDTH_SFT)) {
		// adjust horizontal position
		int adj = (mv_display_right - (1 << MX_BUF_WIDTH_SFT));
		if (adj >= mv_display_left) {
			adj = mv_display_left;
		}
		mv_display_left -= adj;
		mv_display_right -= adj;
		if (mv_display_right >= (1 << MX_BUF_WIDTH_SFT)) {
			mv_display_right = (1 << MX_BUF_WIDTH_SFT);
		}
	}
	if (mv_display_bottom >= SCREEN_HEIGHT) {
		// adjust vertical position
		int adj = (mv_display_bottom - SCREEN_HEIGHT);
		if (adj >= mv_display_top) {
			adj = mv_display_top;
		}
		mv_display_top -= adj;
		mv_display_bottom -= adj;
		if (mv_display_bottom >= SCREEN_HEIGHT) {
			mv_display_bottom = SCREEN_HEIGHT;
		}
	}
#ifdef _DEBUG
	logging->out_debugf(_T("mv_display: l:%d t:%d r:%d b:%d")
		, mv_display_left, mv_display_top, mv_display_right, mv_display_bottom);
#endif
}

// ----------------------------------------------------------------------------
// Palette
// ----------------------------------------------------------------------------

/// @brief Expand RGB color from palette color
/// @param[in] pal : b15-b11:Green b10-b6:Red b5-b1:Blue b0:Intensity
/// @return : RGB color 1 dot (32bit)
uint32_t DISPLAY::expand_rgb16(uint32_t pal)
{
	dws_i = (pal & 1);
	pal >>= 1;
	dws_b = c_bright_table[m_curr_contrast][pal & 0x1f][dws_i];
	pal >>= 5;
	dws_r = c_bright_table[m_curr_contrast][pal & 0x1f][dws_i];
	pal >>= 5;
	dws_g = c_bright_table[m_curr_contrast][pal & 0x1f][dws_i];

	dws_g = (dws_g << Gshift);
	dws_r = (dws_r << Rshift);
	dws_b = (dws_b << Bshift);

	return (dws_r | dws_g | dws_b | dws_a);
}

#ifdef USE_DEBUGGER
/// @brief Expand RGB color from palette color (16 colors)
/// @param[in] pal : b15-b11:Green b10-b6:Red b5-b1:Blue b0:Intensity
/// @return : RGB color 1 dot (32bit)
uint32_t DISPLAY::debug_expand_rgb16(uint32_t pal)
{
	dws_i = (pal & 1);
	pal >>= 1;
	dws_b = c_bright_table[0xf][pal & 0x1f][dws_i];
	pal >>= 5;
	dws_r = c_bright_table[0xf][pal & 0x1f][dws_i];
	pal >>= 5;
	dws_g = c_bright_table[0xf][pal & 0x1f][dws_i];

	dws_g = (dws_g << Gshift);
	dws_r = (dws_r << Rshift);
	dws_b = (dws_b << Bshift);

	return (dws_r | dws_g | dws_b | dws_a);
}

/// @brief Expand RGB color from 3bits data
/// @param[in] pal_num : 3bits data (0 - 7)
/// @return : RGB color 1 dot (32bit)
uint32_t DISPLAY::debug_expand_palcol8(uint32_t pal_num)
{
	dws_b = ((pal_num >> 0) & 1) * 0xc0 + 0x3f;
	dws_r = ((pal_num >> 1) & 1) * 0xc0 + 0x3f;
	dws_g = ((pal_num >> 2) & 1) * 0xc0 + 0x3f;

	dws_g = (dws_g << Gshift);
	dws_r = (dws_r << Rshift);
	dws_b = (dws_b << Bshift);

	return (dws_r | dws_g | dws_b | dws_a);
}

/// @brief Expand RGB color from palette number (16 colors)
/// @param[in] pal_num : palette number
/// @return : RGB color 1 dot (32bit)
uint32_t DISPLAY::debug_expand_palette(uint32_t pal_num)
{
	uint32_t pal = m_palette[pal_num];

	return debug_expand_rgb16(pal);
}

/// @brief Expand RGB color from palette number (65536 colors)
/// @param[in] pal0_num:  b0-b7
/// @param[in] pal1_num: b15-b8
/// @return : RGB color 1 dot (32bit)
uint32_t DISPLAY::debug_expand_palette65536(uint32_t pal0_num, uint32_t pal1_num)
{
	int pal0_num_sft = ((1 - (pal0_num & 1)) << 3);	// x8
	int pal1_num_sft = ((1 - (pal1_num & 1)) << 3);	// x8
	pal0_num &= ~1;	// even
	pal1_num |= 1;	// odd

	uint32_t pal0 = (m_palette[pal0_num] >> pal0_num_sft) & 0xff;
	uint32_t pal1 = (m_palette[pal1_num] >> pal1_num_sft) & 0xff;

	return debug_expand_rgb16(pal1 << 8 | pal0);
}

/// @brief Draw all palette color to screen buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_palette_all(int width, int height, scrntype *buffer)
{
	for(int y=0; y<32; y++) {
		for(int x=0; x<16; x++) {
			scrntype dot = debug_expand_palette(y * 16 + x);
			for(int u=0; u<16; u++) {
				scrntype *p = buffer + (y * 16 + u) * width + x * 32;
				*p = 0xffffffff;
				p++;
				if (u == 0) {
					for(int t=1; t<31; t++) {
						*p = 0xffffffff;
						p++;
					}
				} else if (u == 15) {
					for(int t=1; t<31; t++) {
						*p = 0;
						p++;
					}
				} else {
					for(int t=1; t<31; t++) {
						*p = dot;
						p++;
					}
				}
				*p = 0;
				p++;
			}
		}
	}
	// 65536 colors
	buffer += 32 * 16 * width;
	for(int y=0; y<256; y++) {
		for(int x=0; x<256; x++) {
			scrntype dot = debug_expand_palette65536(x, y);
			for(int u=0; u<2; u++) {
				scrntype *p = buffer + (y * 2 + u) * width + x * 2;
				for(int t=0; t<2; t++) {
					*p = dot;
					p++;
				}
			}
		}
	}
}
/// @brief Dump all palette color
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_palette_dumper(int width, int height, uint16_t *buffer)
{
	// 16color
	for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
			*buffer = m_palette[y * width + x];
			buffer++;
		}
	}
}
#endif /* USE_DEBUGGER */

// ----------------------------------------------------------------------------

/// @brief Mix two colors and return averaged color
/// @param[in] pal_a : palette A
/// @param[in] pal_b : palette B
/// @return : mixed palette
uint32_t DISPLAY::mix_translucent_color16(uint32_t pal_a, uint32_t pal_b)
{
	pal_a =  (((pal_a & 0xf800) << 2) | ((pal_a & 0x07c0) << 1) | (pal_a & 0x003e));
	pal_a += (((pal_b & 0xf800) << 2) | ((pal_b & 0x07c0) << 1) | (pal_b & 0x003f));
	return   (((pal_a >> 3) & 0xf800) | ((pal_a >> 2) & 0x07c0) | ((pal_a >> 1) & 0x003e) | (pal_a & 0x0001));
}

// ----------------------------------------------------------------------------
// Rendering
// ----------------------------------------------------------------------------

/// @brief Draw screen
void DISPLAY::draw_screen()
{
	if (skip_frame_count >= 1) {
		return;
	}

//	if (*vm_pause) return;

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
	int height9 = (mv_display_height << MX_BUF_WIDTH_SFT);

	// draw screen per one frame
	update_show_screen_flags();

//	render_pcg();
	render_sprite_bg();
	render_text();
#ifdef USE_GRAPHIC_RENDER
	render_graphic();
#endif

	clear_mix_buffer(mv_display_width, height9);
	clear_mix_txspbg_render(mv_display_width, mv_display_height << MX_TXSPBG_WIDTH_SFT);

	mix_render_sprite_bg(0);
	mix_render_text_sprite_bg(mv_display_width, mv_display_height);

	mix_buffer_graphic(mv_display_width, height9);
	mix_buffer_graphic_txspbg(mv_display_width, height9);

#endif

	dws_scrn_offset = emu->screen_buffer_offset();
	dws_scrn0 = emu->screen_buffer(0);

	int y_step = 1;
	if (pConfig->scan_line & 1) {
		y_step++;
	}

//	draw_screen_black(m_draw_even_odd, y_step);
	draw_screen_frame(m_draw_even_odd, y_step);
	draw_screen_mixed(m_draw_even_odd, y_step);

	m_draw_even_odd = 1 - m_draw_even_odd;
}

// ----------------------------------------------------------------------------

/// @brief Update a raster data on the screen buffer 
/// @param[in] vline : line number to draw screen
/// @param[in] clock : unused
void DISPLAY::update_display(int vline, int clock)
{
//	bool now_skip_frame = false;

	// now skip frame ?
	if (vline == 0) {
		if (emu->now_skip_frame()) {
			skip_frame_count++;
		} else {
			skip_frame_count = 0;
		}
//		if (skip_frame_count >= 1) {
//			// skip copy vram data
//			now_skip_frame = true;
//		} else {
//			now_skip_frame = false;
//		}

		// interlace mode
		m_raster_even_odd = 1 - m_raster_even_odd;

#ifdef DEBUG_SHOW_FLAGS
		if (md_show_screen_cnt > 0) {
			_TCHAR buf[1024];
			buf[0] = 0;
			for(int cnt=0; cnt<md_show_screen_cnt && cnt<1024; cnt++) {
				UTILITY::sntprintf(buf, sizeof(buf), _T(" %03X:%03X"), md_show_screen[cnt].vline, md_show_screen[cnt].flags);
			}
			logging->out_debugf(buf);
		}
		md_show_screen_cnt = 0;
#endif
	}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_RASTER)

	int vline_sp = vline;
	int vline_sp_step = 0;
//	int vline_spr = vline;
//	int vline_spr_d = vline;
	int vline_cr = vline;
	int vline_cr_d = vline;
	uint8_t draw_flags = 0x03;

	if (pConfig->scan_line >= 2) {
		draw_flags = ((vline & 1) == m_raster_even_odd) ? 0x03 : 0;
	}

	switch(m_raster_mode) {
	case RASTER_HIRESO_DOUBLE:
		// double raster mode (256 x 2)
//		vline_sp >>= 1;
		vline_cr >>= 1;
//		vline_cr_d = vline_cr;
		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & (SPRITE_BG::RESO_VRES | SPRITE_BG::RESO_LHFREQ)) == SPRITE_BG::RESO_LHFREQ) {
			// sprite is hireso 256 mode -> double raster
			vline_sp >>= 1;
			vline_sp_step = 1;
		}
		break;
	case RASTER_HIRESO_NONINTER:
		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & (SPRITE_BG::RESO_VRES | SPRITE_BG::RESO_LHFREQ)) == SPRITE_BG::RESO_LHFREQ) {
			// sprite is hireso 256 mode -> double raster
			vline_sp >>= 1;
			vline_sp_step = 1;
		}
		break;
	case RASTER_HIRESO_INTERLACE_512:
		// double intarlace ??
		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_VRES) != SPRITE_BG::RESO_VRES512) {
			// sprite is 256 mode
			vline_sp >>= 1;
			vline_sp_step = 1;
		}
		break;
	case RASTER_HIRESO_INTERLACE_1024:
		// interlace mode (1024 mode)
		vline_cr <<= 1;
		vline_cr += m_raster_even_odd;
		draw_flags |= 0x02;

		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_VRES) == SPRITE_BG::RESO_VRES512) {
			// sprite is 512 mode
			vline_sp <<= 1;
			vline_sp += m_raster_even_odd;
			draw_flags |= 0x01;
		}
		break;
	case RASTER_NORMAL_INTERLACE_512:
		// interlace mode (512 mode)
		vline_cr <<= 1;
		vline_cr += m_raster_even_odd;
		vline_cr_d = vline_cr;
		draw_flags |= 0x02;

		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_VRES) == SPRITE_BG::RESO_VRES512) {
			// sprite is 512 mode
			vline_sp <<= 1;
			vline_sp += m_raster_even_odd;
			draw_flags |= 0x01;
		} else {
			// sprite is 256 mode
			vline_sp_step = 1;
		}
		break;
	default:
		// normal (256 line)
		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & (SPRITE_BG::RESO_VRES | SPRITE_BG::RESO_LHFREQ)) == SPRITE_BG::RESO_LHFREQ) {
			// sprite is hireso 256 mode -> double raster
			vline_sp >>= 1;
			vline_sp_step = 1;
		}
		break;
	}

	update_show_screen_flags();

	OUT_DEBUG_SHFLGS(1, 566, _T("CRTC: %lld V:%03d flags:%04X")
		, get_current_clock()
		, vline, m_show_screen);

#ifdef DEBUG_SHOW_FLAGS
	if (md_show_screen_cnt == 0 || (md_show_screen[md_show_screen_cnt-1].flags != (m_show_screen & 0xffff))) {
		md_show_screen[md_show_screen_cnt].vline = vline;
		md_show_screen[md_show_screen_cnt].flags = (m_show_screen & 0xffff);
		md_show_screen_cnt++;
	}
#endif

	if (vline_cr_d >= mv_display_height) {
		// buffer over range
		return;
	}

	switch(draw_flags & 3) {
	case 0x01:
		// draw sprite
		render_sprite_bg_one_line(vline_sp, vline_cr);
		render_text_one_line(vline_cr);

		clear_mix_txspbg_render_one_line((1 << MX_TXSPBG_WIDTH_SFT), (vline_cr_d << MX_TXSPBG_WIDTH_SFT));
		mix_render_sprite_bg_one_line(vline_sp_step, vline_cr, vline_cr_d);
		mix_render_text_sprite_bg_one_line(mv_display_width, vline_cr, vline_cr_d);

		clear_mix_buffer_one_line((1 << MX_BUF_WIDTH_SFT), (vline_cr_d << MX_BUF_WIDTH_SFT));

		mix_buffer_graphic_txspbg_one_line(mv_display_width, vline_cr_d, vline_cr_d);
		break;

	case 0x02:
		// draw graphic
		clear_mix_buffer_one_line((1 << MX_BUF_WIDTH_SFT), (vline_cr_d << MX_BUF_WIDTH_SFT));
#ifdef USE_GRAPHIC_RENDER
		render_graphic_one_line(vline_cr);
#endif
		mix_buffer_graphic_one_line(mv_display_width, vline_cr, vline_cr_d);
		break;

	case 0x03:
		// full drawing
		render_sprite_bg_one_line(vline_sp, vline_cr);
		render_text_one_line(vline_cr);

		clear_mix_txspbg_render_one_line((1 << MX_TXSPBG_WIDTH_SFT), (vline_cr_d << MX_TXSPBG_WIDTH_SFT));
		mix_render_sprite_bg_one_line(vline_sp_step, vline_cr, vline_cr_d);
		mix_render_text_sprite_bg_one_line(mv_display_width, vline_cr, vline_cr_d);

		clear_mix_buffer_one_line((1 << MX_BUF_WIDTH_SFT), (vline_cr_d << MX_BUF_WIDTH_SFT));
#ifdef USE_GRAPHIC_RENDER
		render_graphic_one_line(vline_cr);
#endif
		mix_buffer_graphic_one_line(mv_display_width, vline_cr, vline_cr_d);

		mix_buffer_graphic_txspbg_one_line(mv_display_width, vline_cr_d, vline_cr_d);
		break;

	default:
		// skip drawing
		break;
	}
#endif // (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_RASTER)
}

// ----------------------------------------------------------------------------

/// @brief Deside to show screen (Text, Sprite and Graphic)
void DISPLAY::update_show_screen_flags()
{
	m_show_screen = ~emu->get_parami(VM::ParamHideScreen);
	int onoff = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_ONFF_MASK);
	if (onoff & VC_TO_SPON_MASK) {
		onoff |= ((p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG0_ON) << (VM::BG0Sft - SPRITE_BG::CONT_BG0_ON_SFT));
		onoff |= ((p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG1_ON) << (VM::BG1Sft - SPRITE_BG::CONT_BG1_ON_SFT));
		if (!(p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_DISP_CPU)) onoff &= ~(VM::SpriteMask | VM::BG0Mask | VM::BG1Mask);
		if ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HRES) == 0x01) onoff &= ~(VM::BG1Mask); 
	}
	m_show_screen &= onoff;
}

// ----------------------------------------------------------------------------

/// @brief Clear mixed buffer
/// @param[in] width
/// @param[in] y : line(row) * width
void DISPLAY::clear_mix_buffer_one_line(int width, int y)
{
	memset(&mx_buf[y], 0, sizeof(mx_buf[0]) * width);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Clear mixed buffer
/// @param[in] width
/// @param[in] height9
void DISPLAY::clear_mix_buffer(int width, int height9)
{
	for(int y=0; y<height9; y+=(1 << MX_BUF_WIDTH_SFT)) {
		clear_mix_buffer_one_line(width, y);
	}
}
#endif

/// @brief Clear mixed text and sprite render buffer
/// @param[in] width
/// @param[in] y : line(row) * width
void DISPLAY::clear_mix_txspbg_render_one_line(int width, int y)
{
	memset(&mx_txspbg[y], 0, sizeof(mx_txspbg[0]) * width);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Clear mixed text and sprite render buffer
/// @param[in] width
/// @param[in] height9
void DISPLAY::clear_mix_txspbg_render(int width, int height9)
{
	for(int y=0; y<height9; y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
		clear_mix_txspbg_render_one_line(width, y);
	}
}
#endif

// ----------------------------------------------------------------------------

/// @brief Draw screen from mixed buffer
/// @param[in] src_y
void DISPLAY::draw_screen_mixed_one_line(int src_y)
{
	int src_x = 0;
	for(dws_x = mv_display_left; dws_x < mv_display_right; dws_x++) {
		uint32_t pal = mx_buf[src_y + src_x];
		if (pal & MX_BUF_NO_TRANS) *dws_scrn = expand_rgb16(pal);
		else *dws_scrn = Amask;	// black
		dws_scrn++;
		src_x++;
	}
}
/// @brief Draw screen from mixed buffer
/// @param[in] y_even_odd
/// @param[in] y_step
void DISPLAY::draw_screen_mixed(int y_even_odd, int y_step)
{
	int y_top;
	int src_y;
	int src_y_step;

	if (y_step <= 1) {
		y_top = mv_display_top;
		src_y = 0;
		src_y_step = (1 << MX_BUF_WIDTH_SFT);
	} else {
		y_top = (mv_display_top & ~1) | y_even_odd;
		src_y = (y_even_odd << MX_BUF_WIDTH_SFT);
		src_y_step =  (2 << MX_BUF_WIDTH_SFT);
	}

	for(dws_y = y_top; dws_y < mv_display_bottom; dws_y+=y_step) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y;
		dws_scrn += mv_display_left;

		draw_screen_mixed_one_line(src_y);

		src_y+=src_y_step;
	}
}

// ----------------------------------------------------------------------------
// Mix Graphic
// ----------------------------------------------------------------------------

/// @brief Mix Graphic, Text and Sprite per one line (priority TX, SP > GR (special priority mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_sp(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_GR_DATA) == 0) {
			// graphic plane is hide now
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			// text and sprite
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// graphic is transparent
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			if (pal_num & (MX_TXSPBG_TEXT | MX_TXSPBG_PALETTE_L4)) {
				// text or sprite
				pal_num &= MX_TXSPBG_SPBG_PALETTE;
				uint32_t pal = m_palette[pal_num];
				if (pal) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			if (*dst_buf & MX_BUF_SP_AREA) {
				// special area, so prior graphic

			} else {
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					// text or sprite
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					if (pal) {
						*dst_buf = pal;
						*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
					}
				}
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority TX, SP > GR (translucent mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_ht(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_GR_DATA) == 0) {
			// graphic plane is hide now
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			// text and sprite
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// graphic is transparent
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			if (pal_num & (MX_TXSPBG_TEXT | MX_TXSPBG_PALETTE_L4)) {
				// text or sprite
				pal_num &= MX_TXSPBG_SPBG_PALETTE;
				uint32_t pal = m_palette[pal_num];
				if (pal) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			uint32_t pal_num1 = (pal_num & MX_TXSPBG_SPBG_PALETTE) | 0x100;
			uint32_t pal = m_palette[pal_num1];
			if ((*dst_buf & MX_BUF_TR_AREA) != 0 && pal == 0) {
				// translucent area
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else {
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					// text or sprite
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					if (pal) {
						*dst_buf = pal;
						*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
					}
				}
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority TX, SP > GR (normal mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_n(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_GR_DATA) == 0) {
			// graphic plane is hide now
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			// text and sprite
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
				// text or sprite
				pal_num &= MX_TXSPBG_SPBG_PALETTE;
				uint32_t pal = m_palette[pal_num];
				if (pal) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority TX > GR > SP (special priority mode))
///
/// The 1st graphic is the top priority on the screen.
///
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_sp(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			if (*dst_buf & MX_BUF_SP_AREA) {
				// sperical area, so prior graphic

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// if graphic color is all zero
				// then show text, sprite and bg
//				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			} else {
				// if graphic color is not zero
				// then show text only
//				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TEXT) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					if ((pal_num & MX_TXSPBG_PALETTE_L4) != 0 && pal != 0) {
						// text palette and color is not zero
						*dst_buf = pal;
						*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
					}
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority TX > GR > SP (translucent mode))
///
/// The 1st graphic and the sprite are mixed.
///
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_ht(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			uint32_t pal_num1 = (pal_num & MX_TXSPBG_SPBG_PALETTE) | 0x100;
			uint32_t pal = m_palette[pal_num1];
			if ((*dst_buf & MX_BUF_TR_AREA) != 0 && ((pal_num & MX_TXSPBG_SPRITEBG_MASK) != 0 || pal == 0)) {
				// graphic 1st and text are mixed
				// mix color
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// if graphic color is all zero
				// then show text, sprite and bg
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			} else {
				// if graphic color is not zero
				// then show text only
				if (pal_num & MX_TXSPBG_TEXT) {
					if ((pal_num & MX_TXSPBG_PALETTE_L4) != 0 && pal != 0) {
						// text palette and color is not zero
						*dst_buf = pal;
						*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
					}
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority TX > GR > SP (normal mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_n(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// if graphic color is all zero
				// then show text, sprite and bg
				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			} else {
				// if graphic color is not zero
				// then show text only
				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TEXT) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					if ((pal_num & MX_TXSPBG_PALETTE_L4) != 0 && pal != 0) {
						// text palette and color is not zero
						*dst_buf = pal;
						*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
					}
				}
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority SP > GR > TX (special priority mode))
///
/// The 1st graphic is the top priority on the screen.
///
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_sp(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			uint32_t pal = m_palette[pal_num & MX_TXSPBG_SPBG_PALETTE];
			if (*dst_buf & MX_BUF_SP_AREA) {
				// special mode, so prior graphic

			} else if ((pal_num & MX_TXSPBG_SPRITEBG_MASK) != 0 && pal != 0) {
				// sprite and bg
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// text
				if ((pal_num & MX_TXSPBG_TEXT) != 0 && pal != 0) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority SP > GR > TX (translucent mode))
///
/// The 1st graphic and the text are mixed.
///
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_ht(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			uint32_t pal_num1 = (pal_num & MX_TXSPBG_SPBG_PALETTE) | 0x100;
			uint32_t pal = m_palette[pal_num1];
			if ((*dst_buf & MX_BUF_TR_AREA) != 0 && ((pal_num & MX_TXSPBG_TEXT) != 0 || pal == 0)) {
				// graphic 1st and text are mixed
				// mix color
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((pal_num & MX_TXSPBG_SPRITEBG_MASK) != 0 && pal != 0) {
				// sprite and bg
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// text
				if ((pal_num & MX_TXSPBG_TEXT) != 0 && pal != 0) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority SP > GR > TX (normal mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_n(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			uint32_t pal = m_palette[pal_num & MX_TXSPBG_SPBG_PALETTE];

			if ((pal_num & MX_TXSPBG_SPRITEBG_MASK) != 0 && pal != 0) {
				// sprite and bg
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// text
				if ((pal_num & MX_TXSPBG_TEXT) != 0 && pal != 0) {
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority GR > TX, SP (translucent mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_ht(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			// show text, sprite and bg even if palette number is zero
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			if (*dst_buf & MX_BUF_TR_AREA) {
				// translucent
				uint32_t pal_num = mx_txspbg[src_y + src_x];
				pal_num &= MX_TXSPBG_SPBG_PALETTE;
				uint32_t pal = m_palette[pal_num];
				// mix color
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = pal;
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent

			} else if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// text, sprite and bg
				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}

			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (priority GR > TX, SP (normal mode))
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_n(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// no graphic
			uint32_t pal_num = mx_txspbg[src_y + src_x];
			// show text, sprite and bg even if palette number is zero
			pal_num &= MX_TXSPBG_SPBG_PALETTE;
			uint32_t pal = m_palette[pal_num];
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		} else {
			// graphic data already exists
			if ((*dst_buf & MX_BUF_COLOR) == 0) {
				// if graphic color is all zero
				// then show text, sprite and bg
				uint32_t pal_num = mx_txspbg[src_y + src_x];
				if (pal_num & MX_TXSPBG_TXSPBG_MASK) {
					pal_num &= MX_TXSPBG_SPBG_PALETTE;
					uint32_t pal = m_palette[pal_num];
					*dst_buf = pal;
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphic, Text and Sprite per one line (translucent mode with text palette #0)
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line_sub_ah(int width, int src_y, int dst_y)
{
	int src_x = 0;
	uint32_t *dst_buf;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		dst_buf = &mx_buf[dst_y + dst_x];
		// graphics only (no mixed text and sprite)
		// translucent palette #0
		// mix color
		uint32_t pal = mix_translucent_color16(*dst_buf, m_palette[0x100]);
		if (pal) {
			*dst_buf = pal;
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		}
		src_x++;
	}
}

/// @brief Mix Graphic, Text and Sprite per one line
/// @param[in] width
/// @param[in] src_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_txspbg_one_line(int width, int src_y, int dst_y)
{
//	int pri_tx = (m_vc_regs[VC_PRIORITY] & VC_PR_TX_MASK) >> VC_PR_TX_SFT;
//	int pri_sp = (m_vc_regs[VC_PRIORITY] & VC_PR_SP_MASK) >> VC_PR_SP_SFT;
//	int pri_gr = (m_vc_regs[VC_PRIORITY] & VC_PR_GR_MASK) >> VC_PR_GR_SFT;

//	int src_y = dst_y;
	src_y <<= MX_TXSPBG_WIDTH_SFT;
	dst_y <<= MX_BUF_WIDTH_SFT;
	if (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_AH_MASK) {
		// translucent mode with text palette #0
		mix_buffer_graphic_txspbg_one_line_sub_ah(width, src_y, dst_y);

	} else if (m_vc_priority[PR_TX] <= m_vc_priority[PR_GR] && m_vc_priority[PR_SP] <= m_vc_priority[PR_GR]) {
		// TX, SP > GR
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK)) == (VC_TO_EO_MASK)) {
			// special priority mode
			mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_sp(width, src_y, dst_y);
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_ht(width, src_y, dst_y);
		} else {
			// normal mode
			mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_n(width, src_y, dst_y);
		}

	} else if (m_vc_priority[PR_TX] <= m_vc_priority[PR_GR] && m_vc_priority[PR_SP] > m_vc_priority[PR_GR]) {
		// TX > GR > SP
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK)) == (VC_TO_EO_MASK)) {
			// special priority mode
			mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_sp(width, src_y, dst_y);
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_ht(width, src_y, dst_y);
		} else {
			// normal mode
			mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_n(width, src_y, dst_y);
		}

	} else if (m_vc_priority[PR_TX] > m_vc_priority[PR_GR] && m_vc_priority[PR_SP] <= m_vc_priority[PR_GR]) {
		// SP > GR > TX
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK)) == (VC_TO_EO_MASK)) {
			// special priority mode
			mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_sp(width, src_y, dst_y);
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_ht(width, src_y, dst_y);
		} else {
			// normal mode
			mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_n(width, src_y, dst_y);
		}

	} else {
		// GR > TX, SP
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_ht(width, src_y, dst_y);
		} else {
			// normal mode
			mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_n(width, src_y, dst_y);
		}

	}
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphic, Text and Sprite
/// @param[in] width
/// @param[in] height9 : height left shifted 9
void DISPLAY::mix_buffer_graphic_txspbg(int width, int height9)
{
//	int pri_tx = (m_vc_regs[VC_PRIORITY] & VC_PR_TX_MASK) >> VC_PR_TX_SFT;
//	int pri_sp = (m_vc_regs[VC_PRIORITY] & VC_PR_SP_MASK) >> VC_PR_SP_SFT;
//	int pri_gr = (m_vc_regs[VC_PRIORITY] & VC_PR_GR_MASK) >> VC_PR_GR_SFT;

	int src_y = 0;
	if (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_AH_MASK) {
		// translucent mode with text palette #0
		for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
			mix_buffer_graphic_txspbg_one_line_sub_ah(width, src_y, dst_y);
			src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
		}

	} else if (m_vc_priority[PR_TX] <= m_vc_priority[PR_GR] && m_vc_priority[PR_SP] <= m_vc_priority[PR_GR]) {
		// TX, SP > GR
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_GT_MASK)) {
			// special priority mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_sp(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_n(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		}

	} else if (m_vc_priority[PR_TX] <= m_vc_priority[PR_GR] && m_vc_priority[PR_SP] > m_vc_priority[PR_GR]) {
		// TX > GR > SP
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_GT_MASK)) {
			// special priority mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_sp(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// special priority mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_ht(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_n(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		}

	} else if (m_vc_priority[PR_TX] > m_vc_priority[PR_GR] && m_vc_priority[PR_SP] <= m_vc_priority[PR_GR]) {
		// SP > GR > TX
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_GT_MASK)) {
			// special priority mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_sp(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_ht(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_n(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		}

	} else {
		// GR > TX, SP
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GT_MASK)) {
			// translucent mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_ht(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_n(width, src_y, dst_y);
				src_y+=(1 << MX_TXSPBG_WIDTH_SFT);
			}
		}

	}
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
/// @brief Expand mixed buffer to screen buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_mixed_buffer(int width, int height, scrntype *buffer)
{
	uint32_t src = 0;
	scrntype *dst = buffer;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			*dst = debug_expand_rgb16(mx_buf[src]);
			dst++;
			src++;
		}
	}
}

void DISPLAY::debug_expand_mixed_buffer_dumper(int width, int height, uint16_t *buffer)
{
	uint32_t src = 0;
	uint16_t *dst = buffer;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			*dst = mx_buf[src] & 0xffff;
			dst++;
			src++;
		}
	}
}
#endif

// ----------------------------------------------------------------------------
// Mix Text and Sprite (included BG)
// ----------------------------------------------------------------------------

/// @brief Mix Text and Sprite per one line (Text only (Sprite is off))
/// @param[in] width
/// @param[in] src_left : left position
/// @param[in] src_y : line 0 - 1023 * 1024
/// @param[in] dst_y : line 0 - 1023 * 512/1024
void DISPLAY::mix_render_text_sprite_bg_one_line_sub_tx_only(int width, int src_left, int src_y, int dst_y)
{
	int src = src_y + src_left;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		src &= 0xfffff; // TODO
		uint32_t pal_num = rb_tvram[src];
//		if ((src_y & 0xffc00) == 0x3fc00) {
//			logging->out_debugf(_T("Mix Text: src:%08X dst:%d,%d"), src, dst_x, dst_y);
//		}
		mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100 | pal_num);
		src++;
	}
}

/// @brief Mix Text and Sprite per one line (Sprite only (Text is off))
/// @param[in] width
/// @param[in] src_left : left position
/// @param[in] src_y : line 0 - 1023 * 1024
/// @param[in] dst_y : line 0 - 1023 * 512/1024
void DISPLAY::mix_render_text_sprite_bg_one_line_sub_sp_only(int width, int src_left, int src_y, int dst_y)
{
//	int src = src_y + src_left;
//	uint32_t txtpal = m_palette[0x100];	// color of text palette 0 
//	if (txtpal != 0) {
		for(int dst_x = 0; dst_x < width; dst_x++) {
//			src &= 0xfffff; // TODO
			if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100);
			}
//			src++;
		}
//	}
}

/// @brief Mix Text and Sprite per one line (priority: Sprite > Text)
/// @param[in] width
/// @param[in] src_left : left position
/// @param[in] src_y : line 0 - 1023 * 1024
/// @param[in] dst_y : line 0 - 1023 * 512/1024
void DISPLAY::mix_render_text_sprite_bg_one_line_sub_sp_tx(int width, int src_left, int src_y, int dst_y)
{
	int src = src_y + src_left;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		src &= 0xfffff; // TODO
		if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
			uint32_t pal_num = rb_tvram[src];
			if (pal_num != 0) {
				// if sprite/bg palette is zero and text palette is not zero
				// then set text palette
				mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100 | pal_num);
			} else if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_BG) == 0) {
				// if sprite palette is zero and text palette is zero
				// then set text palette
				// * bg0 and 1 are off
				mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100 | pal_num);
			}
		}
		src++;
	}
}

/// @brief Mix Text and Sprite per one line (priority: Text > Sprite)
/// @param[in] width
/// @param[in] src_left : left position
/// @param[in] src_y : line 0 - 1023 * 1024
/// @param[in] dst_y : line 0 - 1023 * 512/1024
void DISPLAY::mix_render_text_sprite_bg_one_line_sub_tx_sp(int width, int src_left, int src_y, int dst_y)
{
	int src = src_y + src_left;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		src &= 0xfffff; // TODO
		uint32_t pal_num = rb_tvram[src];
		if (pal_num != 0) {
			// if text palette is not zero
			// then set text palette
			mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100 | pal_num);
		} else if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
			if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_BG) == 0) {
				// if sprite palette is zero and text palette is zero
				// then set text palette
				// * bg0 and 1 are off
				mx_txspbg[dst_y + dst_x] = (MX_TXSPBG_TEXT | 0x100 | pal_num);
			}
		}
		src++;
	}
}

/// @brief Mix Text and Sprite per one line (included BG)
/// @param[in] width
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_text_sprite_bg_one_line(int width, int src_y, int dst_y)
{
	int onoff = (m_show_screen & (VM::TextMask | VM::SpriteMask | VM::BG0Mask | VM::BG1Mask));
	if (!onoff) {
		// text, sprite and bg are off
		return;
	}

//	if ((src_y & 0xf) == 0) {
//		logging->out_debugf(_T("Mix Render Text and Sprite: y:%d"), src_y);
//	}

	int src_left = p_crtc_regs[CRTC::CRTC_TSCROLL_X];
	src_y += p_crtc_regs[CRTC::CRTC_TSCROLL_Y];

	src_y <<= 10;
	src_y &= (0x3ff << 10);
	dst_y <<= MX_TXSPBG_WIDTH_SFT;

	if (onoff == VM::TextMask) {
		// text only
		mix_render_text_sprite_bg_one_line_sub_tx_only(width, src_left, src_y, dst_y);

	} else {
//		int tx = ((m_vc_regs[VC_PRIORITY] & VC_PR_TX_MASK) >> VC_PR_TX_SFT);
//		int sp = ((m_vc_regs[VC_PRIORITY] & VC_PR_SP_MASK) >> VC_PR_SP_SFT);

		if (m_vc_priority[PR_TX] > m_vc_priority[PR_SP]) {
			// sprite is heigher than text
			if (!(onoff & VM::TextMask)) {
				// sprite only
				mix_render_text_sprite_bg_one_line_sub_sp_only(width, src_left, src_y, dst_y);
			} else {
				// sprite and text
				mix_render_text_sprite_bg_one_line_sub_sp_tx(width, src_left, src_y, dst_y);
			}
		} else {
			// text is heigher than sprite
			if (!(onoff & VM::TextMask)) {
				// sprite only
				mix_render_text_sprite_bg_one_line_sub_sp_only(width, src_left, src_y, dst_y);
			} else {
				// sprite and text
				mix_render_text_sprite_bg_one_line_sub_tx_sp(width, src_left, src_y, dst_y);
			}
		}
	}
}

/// @brief Mix Text and Sprite (included BG)
/// @param[in] width
/// @param[in] height
void DISPLAY::mix_render_text_sprite_bg(int width, int height)
{
	int onoff = (m_show_screen & (VM::TextMask | VM::SpriteMask | VM::BG0Mask | VM::BG1Mask));
	if (!onoff) {
		// text, sprite and bg are off
		return;
	}

	int src_left = p_crtc_regs[CRTC::CRTC_TSCROLL_X];
	int src_y = p_crtc_regs[CRTC::CRTC_TSCROLL_Y];

	int dst_height = (height << MX_TXSPBG_WIDTH_SFT);
	if (onoff == VM::TextMask) {
		// text only
		src_y <<= 10;
		for(int dst_y = 0; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
			src_y &= (0x3ff << 10);
			mix_render_text_sprite_bg_one_line_sub_tx_only(width, src_left, src_y, dst_y);
			src_y+=(1 << 10);
		}

	} else {
//		int tx = ((m_vc_regs[VC_PRIORITY] & VC_PR_TX_MASK) >> VC_PR_TX_SFT);
//		int sp = ((m_vc_regs[VC_PRIORITY] & VC_PR_SP_MASK) >> VC_PR_SP_SFT);

		src_y <<= 10;
		if (m_vc_priority[PR_TX] > m_vc_priority[PR_SP]) {
			// sprite is heigher than text
			if (!(onoff & VM::TextMask)) {
				// sprite only
				for(int dst_y = 0; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
					src_y &= (0x3ff << 10);
					mix_render_text_sprite_bg_one_line_sub_sp_only(width, src_left, src_y, dst_y);
					src_y+=(1 << 10);
				}
			} else {
				// sprite and text
				for(int dst_y = 0; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
					src_y &= (0x3ff << 10);
					mix_render_text_sprite_bg_one_line_sub_sp_tx(width, src_left, src_y, dst_y);
					src_y+=(1 << 10);
				}
			}
		} else {
			// text is heigher than sprite
			if (!(onoff & VM::TextMask)) {
				// sprite only
				for(int dst_y = 0; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
					src_y &= (0x3ff << 10);
					mix_render_text_sprite_bg_one_line_sub_sp_only(width, src_left, src_y, dst_y);
					src_y+=(1 << 10);
				}
			} else {
				// sprite and text
				for(int dst_y = 0; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
					src_y &= (0x3ff << 10);
					mix_render_text_sprite_bg_one_line_sub_tx_sp(width, src_left, src_y, dst_y);
					src_y+=(1 << 10);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Render Text
// ----------------------------------------------------------------------------

/// @brief Expand datas on the text vram and store in the text render buffer.
/// buffer has the palette number.
/// @param[in] y : line 0 - 1023
void DISPLAY::render_text_one_line_sub(int y)
{
	uint8_t *buffer = &rb_tvram[y << 10];
	uint32_t addr = (y << 6);

	// full render in a line
	for(int x = 0; x < 1024; x += 16) {
		if (pu_tvram[addr]) {
//			if ((y & 0xff) == 0xff) {
//				logging->out_debugf(_T("Render Text: (%d,%d) src:%04X dst:%08X"), x, y, addr, buffer - rb_tvram);
//			}
			pu_tvram[addr] >>= 1;

			uint32_t t0 = p_tvram[addr];	// t0
			addr += 0x10000;
			uint32_t t1 = p_tvram[addr];	// t1 
			addr += 0x10000;
			uint32_t t2 = p_tvram[addr];	// t2 
			addr += 0x10000;
			uint32_t t3 = p_tvram[addr];	// t3 
			addr -= 0x30000;
			t1 <<= 1;
			t2 <<= 2;
			t3 <<= 3;
#if 1
			buffer += 4;
			for(int n=0; n<4; n++) {
				buffer += 11;
				uint32_t pal = (t0 & 0x1111);
				pal |= (t1 & 0x2222);
				pal |= (t2 & 0x4444);
				pal |= (t3 & 0x8888);

				*buffer = (uint8_t)(pal & 0xf);
				pal >>= 4;
				buffer -= 4;
				*buffer = (uint8_t)(pal & 0xf);
				pal >>= 4;
				buffer -= 4;
				*buffer = (uint8_t)(pal & 0xf);
				pal >>= 4;
				buffer -= 4;
				*buffer = (uint8_t)(pal & 0xf);

				t0 >>= 1;
				t1 >>= 1;
				t2 >>= 1;
				t3 >>= 1;
			}
#else
			buffer += 16;
			for(int n=0; n<16; n++) {
				uint32_t pal = (t0 & 0x1);
				pal |= (t1 & 0x2);
				pal |= (t2 & 0x4);
				pal |= (t3 & 0x8);

				buffer--;
				*buffer = (uint8_t)pal;

				t0 >>= 1;
				t1 >>= 1;
				t2 >>= 1;
				t3 >>= 1;
			}
#endif
		}
		buffer += 16;
		addr++;
	}
}

/// @brief Expand datas on the text vram and store in the text render buffer.
/// buffer has the palette number.
/// @param[in] y : line 0 - 1023
void DISPLAY::render_text_one_line(int y)
{
//	if (!(m_show_screen & VM::TextMask)) {
//		return;
//	}

	y += p_crtc_regs[CRTC::CRTC_TSCROLL_Y];

	render_text_one_line_sub(y & 0x3ff);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Expand datas on the text vram and store in the text render buffer.
/// buffer has the palette number.
void DISPLAY::render_text()
{
//	if (!(m_show_screen & VM::TextMask)) {
//		return;
//	}

	for(int y = 0; y < 1024; y++) {
		render_text_one_line_sub(y);
	}
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
/// @brief Expand text plane to screen buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_text_plane(int width, int height, scrntype *buffer)
{
	for(int n = 0; n < 4; n++) {
		uint32_t addr = (0x10000 * n);
		for(int y = 0; y < 1024; y++) {
			scrntype *p = buffer + (y * width) + (n & 1) * 1024 + (n & 2) * 1024 * 1024;
			for(int x = 0; x < 1024; x += 16) {
				uint32_t t = p_tvram[addr];
				t <<= 1;
				for(int n=0; n<16; n++) {
					uint32_t pal = (t & 0x10000);
					*p = pal ? 0xffffffff : 0;
					p++;

					t <<= 1;
				}
				addr++;
			}
		}
	}
}

/// @brief Dump text plane
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_text_plane_dumper(int width, int height, uint16_t *buffer)
{
	for(int n = 0; n < 4; n++) {
		uint32_t addr = (0x10000 * n);
		for(int y = 0; y < 1024; y++) {
			uint16_t *p = buffer + (y * width) + (n & 1) * 1024 + (n & 2) * 1024 * 1024;
			for(int x = 0; x < 1024; x += 16) {
				uint32_t t = p_tvram[addr];
				t <<= 1;
				for(int n=0; n<16; n++) {
					uint32_t pal = (t & 0x10000);
					*p = pal ? 0xffff : 0;
					p++;

					t <<= 1;
				}
				addr++;
			}
		}
	}
}
#endif /* USE_DEBUGGER */

// ----------------------------------------------------------------------------
//  Mix Sprite and BG
// ----------------------------------------------------------------------------

/// @brief Mix Sprite and BG data per one line
/// @param[in] step_y : normal:0 / draw even or odd line:1
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_sprite_bg_one_line(int step_y, int src_y, int dst_y)
{
	if (!(m_show_screen & (VM::SpriteMask | VM::BG0Mask | VM::BG1Mask))) {
		// sprite and bg are off
		return;
	}
	if (p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HVRES_INVALID) {
		// invalid mode
		return;
	}

	// sprite is 16x16 ?
	int size512 = ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HRES) == 0x01 ? 1 : 0);

	int width = 256;	// disp area is 256x256
	int crtc_width = (p_crtc_regs[CRTC::CRTC_HORI_END]-p_crtc_regs[CRTC::CRTC_HORI_START]);
	if (crtc_width > 32 && p_bg_regs[SPRITE_BG::BG_HORI_TOTAL] >= 0x40) {
		// draw 512 when disp area is not 256 and htotal is 0xff
		width <<= 1;
	}
	int height;
	if (m_raster_mode != RASTER_NORMAL_NONINTER) {
		height = 640;
	} else {
		height = ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & (SPRITE_BG::RESO_VRES | SPRITE_BG::RESO_LHFREQ)) == 0x00 ? 320 : 640);
	}

	int disp_left = (p_bg_regs[SPRITE_BG::BG_HORI_DISP] - p_crtc_regs[CRTC::CRTC_HORI_START] - 4) * 8;
	int disp_top = (p_bg_regs[SPRITE_BG::BG_VERT_DISP] - p_crtc_regs[CRTC::CRTC_VERT_START]);

	disp_top >>= step_y;

	// mix sprite cells
	mix_render_sprite_one_line(width, height, disp_left, disp_top, step_y, dst_y);

	if (src_y < disp_top) {
		// out of display area
		return;
	}
	src_y -= disp_top;

	// mix bg and sprite
	mix_render_bg0_one_line(size512, width, disp_left, disp_top, src_y, dst_y);
	mix_render_bg1_one_line(size512, width, disp_left, disp_top, src_y, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Sprite and BG data
/// @param[in] step_y : normal:0 / draw even or odd line:1
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_sprite_bg(int step_y)
{
	if (!(m_show_screen & (VM::SpriteMask | VM::BG0Mask | VM::BG1Mask))) {
		// sprite and bg are off
		return;
	}
	if (p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HVRES_INVALID) {
		// invalid mode
		return;
	}

	int size512 = ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HRES) == 0x01 ? 1 : 0);

	int width = 256;	// disp area is 256x256
//	width <<= size512;	// is 512x512 if hireso
	if ((p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_HRES) != 0 && p_bg_regs[SPRITE_BG::BG_HORI_TOTAL] >= 0x4b) {
		// draw 512 when disp area is not 256 and htotal is 0xff
		width <<= 1;
	}

	int disp_left = (p_bg_regs[SPRITE_BG::BG_HORI_DISP] - p_crtc_regs[CRTC::CRTC_HORI_START] - 4) * 8;
	int disp_top = (p_bg_regs[SPRITE_BG::BG_VERT_DISP] - p_crtc_regs[CRTC::CRTC_VERT_START]);
	// 256line?
	if ((p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRES) == 0) disp_top /= 2;

	// mix sprite cells
	mix_render_sprite(size512, width, disp_left, disp_top, step_y);

	// mix bg and sprite
	mix_render_bg0(size512, width, disp_left, disp_top);
	mix_render_bg1(size512, width, disp_left, disp_top);
}
#endif

/// @brief Mix BG0 data and Sprite per one line
/// @param[in] width : 256 / 512
/// @param[in] src_left : left position
/// @param[in] src_x_limit : limit mask (255 / 511)
/// @param[in] disp_left : left position of display area
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_bg0_one_line_sub(int width, int src_left, int src_x_limit, int disp_left, int src_y, int dst_y)
{
	int src_x = src_left;
	for(int dst_x = disp_left; dst_x < width; dst_x++) {
		src_x &= src_x_limit;

		uint16_t priority = mx_txspbg[dst_y + dst_x] & MX_TXSPBG_SPRITE_PRIORITY;
		switch(priority) {
		case (0 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// no data (sprite is off)
			mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram0[src_y + src_x];
			break;
		case (1 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
		case (2 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// BG0 > SP
			// if bg0 palette is not zero
			// or bg0 palette is zero and sprite palette is zero
			// then show bg0
			if ((rb_bgram0[src_y + src_x] & 0xf) != 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram0[src_y + src_x];
			}
			else if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram0[src_y + src_x];
			}
			break;
		case (3 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// SP > BG0
			// if sprite palette is zero
			// then show bg0
			if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram0[src_y + src_x];
			}
			break;
		}

		src_x++;
	}
}

/// @brief Mix BG1 data, BG0 and Sprite per one line
/// @param[in] width : 256 only
/// @param[in] src_left : left position
/// @param[in] src_x_limit : limit mask (255)
/// @param[in] disp_left : left position of display area
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_bg1_one_line_sub(int width, int src_left, int src_x_limit, int disp_left, int src_y, int dst_y)
{
	int src_x = src_left;
	for(int dst_x = disp_left; dst_x < width; dst_x++) {
		src_x &= src_x_limit;

		uint16_t priority = mx_txspbg[dst_y + dst_x] & MX_TXSPBG_SPRITE_PRIORITY;
		switch(priority) {
		case (0 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// BG0
			// if bg0 palette is zero
			// then show bg1
			if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram1[src_y + src_x];
			}
			break;
		case (1 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// SP (BG1 > SP)
			// if bg1 palette is not zero
			// or sprite palette is zero
			// then show bg1
			if ((rb_bgram1[src_y + src_x] & 0xf) != 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram1[src_y + src_x];
			}
			else if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram1[src_y + src_x];
			}
			break;
		case (2 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
		case (3 << MX_TXSPBG_SPRITE_PRIORITY_SFT):
			// SP (SP > BG1)
			// if sprite palette is zero
			// then show bg1
			if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
				mx_txspbg[dst_y + dst_x] = MX_TXSPBG_BG | rb_bgram1[src_y + src_x];
			}
			break;
		}

		src_x++;
	}
}

/// @brief Mix BG0 data and Sprite per one line
/// @param[in] size512 : 256x256:0 512x512:1
/// @param[in] width : 256 / 512
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_bg0_one_line(int size512, int width, int disp_left, int disp_top, int src_y, int dst_y)
{
	if (!(m_show_screen & VM::BG0Mask)) {
		// bg0 is off
		return;
	}

	uint32_t src_x_limit = 512;
	src_x_limit <<= size512;
	src_x_limit--;
	uint32_t src_y_limit = (src_x_limit << 10);

	// start x axis from $eb0800
	int src_left = p_bg_regs[SPRITE_BG::BG0_SCROLL_X];

	// start y axis $eb0802
	int src_top = p_bg_regs[SPRITE_BG::BG0_SCROLL_Y];

	src_y = ((src_top + src_y) << 10);
	dst_y <<= MX_TXSPBG_WIDTH_SFT;
	src_y &= src_y_limit;
	mix_render_bg0_one_line_sub(width, src_left, src_x_limit, disp_left, src_y, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix BG0 data and Sprite
/// @param[in] size512 : 256x256:0 512x512:1
/// @param[in] width : 256 / 512
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
void DISPLAY::mix_render_bg0(int size512, int width, int disp_left, int disp_top)
{
	if (!(m_show_screen & VM::BG0Mask)) {
		// bg0 is off
		return;
	}

	uint32_t src_x_limit = 512;
	src_x_limit <<= size512;
	src_x_limit--;
	uint32_t src_y_limit = (src_x_limit << 10);

	// start x axis from $eb0800
	int src_left = p_bg_regs[SPRITE_BG::BG0_SCROLL_X];

	// start y axis $eb0802
	int src_top = p_bg_regs[SPRITE_BG::BG0_SCROLL_Y];

	int src_y = (src_top << 10);
	int dst_height = (width << MX_TXSPBG_WIDTH_SFT);
	for(int dst_y = disp_top; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
		src_y &= src_y_limit;
		mix_render_bg0_one_line_sub(width, src_left, src_x_limit, disp_left, src_y, dst_y);
		src_y += (1 << 10);	// 1024
	}
}
#endif

/// @brief Mix BG1 data, BG0 and Sprite per one line
/// @note bg1 uses normal resolution only
/// @param[in] size512 : 256x256:0
/// @param[in] width : 256
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_bg1_one_line(int size512, int width, int disp_left, int disp_top, int src_y, int dst_y)
{
	if (size512) {
		// size512 is not supported
		return;
	}
	if (!(m_show_screen & VM::BG1Mask)) {
		// bg1 is off
		return;
	}

	uint32_t src_x_limit = 512;
	src_x_limit--;
	uint32_t src_y_limit = (src_x_limit << 9);	// x512

	// start x axis from $eb0804
	int src_left = p_bg_regs[SPRITE_BG::BG1_SCROLL_X];

	// start y axis $eb0806
	int src_top = p_bg_regs[SPRITE_BG::BG1_SCROLL_Y];

	src_y = ((src_top + src_y) << 9);	// x512
	dst_y <<= MX_TXSPBG_WIDTH_SFT;
	src_y &= src_y_limit;
	mix_render_bg1_one_line_sub(width, src_left, src_x_limit, disp_left, src_y, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix BG1 data, BG0 and Sprite
/// @note bg1 uses normal resolution only
/// @param[in] size512 : 256x256:0
/// @param[in] width : 256
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
void DISPLAY::mix_render_bg1(int size512, int width, int disp_left, int disp_top)
{
	if (size512) {
		// size512 is not supported
		return;
	}
	if (!(m_show_screen & VM::BG1Mask)) {
		// bg1 is off
		return;
	}

	uint32_t src_x_limit = 512;
	src_x_limit--;
	uint32_t src_y_limit = (src_x_limit << 9);	// x512

	// start x axis from $eb0804
	int src_left = p_bg_regs[SPRITE_BG::BG1_SCROLL_X];

	// start y axis $eb0806
	int src_top = p_bg_regs[SPRITE_BG::BG1_SCROLL_Y];

	int src_y = (src_top << 9);	// x512
	int dst_height = (width << MX_TXSPBG_WIDTH_SFT);
	for(int dst_y = disp_top; dst_y < dst_height; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
		src_y &= src_y_limit;
		mix_render_bg1_one_line_sub(width, src_left, src_x_limit, disp_left, src_y, dst_y);
		src_y += (1 << 9);	// 512
	}
}
#endif

#ifdef USE_SPRITE_RENDER
void DISPLAY::mix_render_sprite_cell_one_line_sub(int sp_num, uint32_t bg_priority, int src_left, int src_y, int dst_left, int dst_right, int dst_y)
{
	int src_x = src_left;
	for(int dst_x = dst_left; dst_x < dst_right; dst_x++) {
		if ((mx_txspbg[dst_y + dst_x] & 0xf) == 0) {
			mx_txspbg[dst_y + dst_x] = MX_TXSPBG_SPBG | bg_priority | rb_spram[sp_num][src_y + src_x];
		}
		src_x++;
	}
}
#else
/// @brief Copy PCG data to mixed buffer for one sprite 
/// @param[in] ptn_num : pattern number of PCG data
/// @param[in] hv : normal:0 reverse horizontal:1 reverse vertical:2 all reverse:3
/// @param[in] bg_priority : palette number (b8-b0) and priority sprite and bg (b11-b10)
/// @param[in] src_left : left position of source
/// @param[in] src_y : source line
/// @param[in] dst_left : left position of display area
/// @param[in] dst_right : right position of display area
/// @param[in] dst_y : dst line
void DISPLAY::mix_render_sprite_cell_one_line_sub(int ptn_num, int hv, uint32_t bg_priority, int src_left, int src_y, int dst_left, int dst_right, int dst_y)
{
	int src_x = src_left;
	for(int dst_x = dst_left; dst_x < dst_right; dst_x++) {
		// mixed buffer is empty or palette 0 ?
		if ((mx_txspbg[dst_y + dst_x] & MX_TXSPBG_PALETTE_L4) == 0) {
			mx_txspbg[dst_y + dst_x] = MX_TXSPBG_SPRITE | bg_priority | rb_pcg[ptn_num][hv][src_y + src_x];
		}
		src_x++;
	}
}
#endif

/// @brief Expand one Sprite data per one line
/// @param[in] width
/// @param[in] height
/// @param[in] sp_num : sprite number 
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] step_y : normal:0 / draw even or odd line:1 
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_sprite_cell_one_line(int width, int height, int sp_num, int disp_left, int disp_top, int step_y, int dst_y)
{
	int sp_num4 = (sp_num << 2);

	uint32_t bg_priority = (p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK);
	if (!bg_priority) {
		// hide
		return;
	}

	int dst_top = (p_sp_regs[sp_num4 | SPRITE_BG::SP_YPOS] & 0x3ff) + disp_top - 16;
	int dst_bottom = dst_top + 16;

	int src_top = 0;
	if (dst_top < 0) {
		src_top = -dst_top;
		dst_top = 0;
	}
	dst_top <<= step_y;
	dst_bottom <<= step_y;

	if (dst_bottom >= height) {
		dst_bottom = height;
	}
	if (dst_y < dst_top || dst_bottom <= dst_y) {
		// out of range
		return;
	}

	int raster = dst_y - dst_top;
	raster >>= step_y;

	bg_priority <<= MX_TXSPBG_SPRITE_PRIORITY_SFT;

	int dst_left = (p_sp_regs[sp_num4 | SPRITE_BG::SP_XPOS] & 0x3ff) + disp_left - 16;
	int dst_right = dst_left + 16;
	int src_left = 0;
	if (dst_left < 0) {
		src_left = -dst_left;
		dst_left = 0;
	}
	if (dst_right >= width) {
		dst_right = width;
	}

#ifndef USE_SPRITE_RENDER
	// pattern number and color
	int ptn_num = p_sp_regs[sp_num4 | SPRITE_BG::SP_PTN];
	int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
	bg_priority |= (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
	ptn_num &= 0xff;

	render_pcg_one_16x16(ptn_num);
#endif

//	dst_top <<= MX_TXSPBG_WIDTH_SFT;
//	dst_bottom <<= MX_TXSPBG_WIDTH_SFT;
	int src_y = ((src_top + raster) << 4);
	dst_y <<= MX_TXSPBG_WIDTH_SFT;
#ifdef USE_SPRITE_RENDER
	mix_render_sprite_cell_one_line_sub(sp_num, bg_priority, src_left, src_y, dst_left, dst_right, dst_y);
#else
	mix_render_sprite_cell_one_line_sub(ptn_num, hv, bg_priority, src_left, src_y, dst_left, dst_right, dst_y);
#endif
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Expand one Sprite data
/// @param[in] size512 : 256x256:0 512x512:1
/// @param[in] width
/// @param[in] sp_num : sprite number 
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] step_y : normal:0 / draw even or odd line:1 
void DISPLAY::mix_render_sprite_cell(int size512, int width, int sp_num, int disp_left, int disp_top, int step_y)
{
	int sp_num4 = (sp_num << 2);

	uint32_t bg_priority = (p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK);
	if (!bg_priority) {
		// hide
		return;
	}

	int dst_top = (p_sp_regs[sp_num4 | SPRITE_BG::SP_YPOS] & 0x3ff) + disp_top - 16;
	int dst_bottom = dst_top + 16;
	dst_top <<= step_y;
	dst_bottom <<= step_y;

	int src_top = 0;
	if (dst_top < 0) {
		src_top = -dst_top;
		dst_top = 0;
	}
	if (dst_bottom >= width) {
		dst_bottom = width;
	}

	bg_priority <<= MX_TXSPBG_SPRITE_PRIORITY_SFT;

	int dst_left = (p_sp_regs[sp_num4 | SPRITE_BG::SP_XPOS] & 0x3ff) + disp_left - 16;
	int dst_right = dst_left + 16;
	int src_left = 0;
	if (dst_left < 0) {
		src_left = -dst_left;
		dst_left = 0;
	}
	if (dst_right >= width) {
		dst_right = width;
	}

#ifndef USE_SPRITE_RENDER
	// pattern number and color
	int ptn_num = p_sp_regs[sp_num4 | SPRITE_BG::SP_PTN];
	int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
	bg_priority |= (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
	ptn_num &= 0xff;

	render_pcg_one_16x16(ptn_num);
#endif

	dst_top <<= MX_TXSPBG_WIDTH_SFT;
	dst_bottom <<= MX_TXSPBG_WIDTH_SFT;
#ifdef USE_SPRITE_RENDER
	int src_y = (src_top << 4);
	for(int dst_y = dst_top; dst_y < dst_bottom; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
		for(int step=0; step<=step_y; step++) {
			mix_render_sprite_cell_one_line_sub(sp_num, bg_priority, src_left, src_y, dst_left, dst_right, dst_y);
		}
		src_y+=(1 << 4);
	}
#else
	int src_y = (src_top << 4);
	for(int dst_y = dst_top; dst_y < dst_bottom; dst_y+=(1 << MX_TXSPBG_WIDTH_SFT)) {
		for(int step=0; step<=step_y; step++) {
			mix_render_sprite_cell_one_line_sub(ptn_num, hv, bg_priority, src_left, src_y, dst_left, dst_right, dst_y);
		}
		src_y+=(1 << 4);
	}
#endif
}
#endif

/// @brief Expand Sprite data per one line
/// @param[in] width
/// @param[in] height
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] step_y : normal:0 / draw even or odd line:1
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::mix_render_sprite_one_line(int width, int height, int disp_left, int disp_top, int step_y, int dst_y)
{
	if (!(m_show_screen & VM::SpriteMask)) {
		// sprite is off
		return;
	}

	// num 0 is the highest priority in sprite cells
	for(int sp_num = 0; sp_num < 128; sp_num++) {
		mix_render_sprite_cell_one_line(width, height, sp_num, disp_left, disp_top, step_y, dst_y);
	}
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Expand Sprite data
/// @param[in] size512 : width is 512 mode? 
/// @param[in] width
/// @param[in] disp_left : left position of display area
/// @param[in] disp_top : top position of display area
/// @param[in] step_y : normal:0 / draw even or odd line:1
void DISPLAY::mix_render_sprite(int size512, int width, int disp_left, int disp_top, int step_y)
{
	if (!(m_show_screen & VM::SpriteMask)) {
		// sprite is off
		return;
	}

	disp_top >>= step_y;

	// num 0 is the highest priority in sprite cells
	for(int sp_num = 0; sp_num < 128; sp_num++) {
		mix_render_sprite_cell(size512, width, sp_num, disp_left, disp_top, step_y);
	}
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
/// @brief Expand text render buffer to screen buffer
/// @param[in] render : render buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_text_render(const uint8_t *render, int width, int height, scrntype *buffer)
{
	for(int y = 0; y < height; y++) {
		uint32_t addr = width * y;
		for(int x = 0; x < width; x++) {
			uint32_t pal = render[addr];
			*buffer = debug_expand_palette(0x100 | pal);
			buffer++;
			addr++;
		}
	}
}

/// @brief Dump text render buffer
/// @param[in] render : render buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_text_dumper(const uint8_t *render, int width, int height, uint16_t *buffer)
{
	for(int y = 0; y < height; y++) {
		uint32_t addr = width * y;
		for(int x = 0; x < width; x++) {
			uint32_t pal = render[addr];
			*buffer = (uint16_t)(0x100 | pal);
			buffer++;
			addr++;
		}
	}
}

/// @brief Expand render buffer to screen buffer
/// @param[in] render : render buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_render(const uint16_t *render, int width, int height, scrntype *buffer)
{
	for(int y = 0; y < height; y++) {
		uint32_t addr = width * y;
		for(int x = 0; x < width; x++) {
			uint32_t pal = render[addr];
			*buffer = debug_expand_palette(pal);
			buffer++;
			addr++;
		}
	}
}

/// @brief Dump render buffer
/// @param[in] render : render buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_dumper(const uint16_t *render, int width, int height, uint16_t *buffer)
{
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			*buffer = *render;
			buffer++;
			render++;
		}
	}
}

/// @brief Expand Sprite data to screen buffer
/// @param[in] width 256
/// @param[in] height 128
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_sprite_ptn_render(int width, int height, scrntype *buffer)
{
	int sp_num = 0; 
	for(int dy = 0; dy < height; dy += 16) {
		for(int dx = 0; dx < width; dx += 16) {
			scrntype *p = buffer + dy * width + dx;
			uint32_t addr = 0;
#ifndef USE_SPRITE_RENDER
			// pattern number and color
			int ptn_num = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PTN];
			int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
			uint32_t bg_color = (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
			ptn_num &= 0xff;
#endif
			for(int y = 0; y < 16; y++) {
				for(int x = 0; x < 16; x++) {
#ifdef USE_SPRITE_RENDER
					uint32_t pal = rb_spram[sp_num][addr];
#else
					uint32_t pal = rb_pcg[ptn_num][hv][addr] | bg_color;
#endif
					*p = debug_expand_palette(pal);
					p++;
					addr++;
				}
				p += (width - 16);
			}
			sp_num++;
		}
	}
}

/// @brief Dump Sprite data
/// @param[in] width 256
/// @param[in] height 128
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_sprite_ptn_dumper(int width, int height, uint16_t *buffer)
{
	int sp_num = 0; 
	for(int dy = 0; dy < height; dy += 16) {
		for(int dx = 0; dx < width; dx += 16) {
			uint16_t *p = buffer + dy * width + dx;
			uint32_t addr = 0;
#ifndef USE_SPRITE_RENDER
			// pattern number and color
			int ptn_num = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PTN];
			int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
			uint32_t bg_color = (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
			ptn_num &= 0xff;
#endif
			for(int y = 0; y < 16; y++) {
				for(int x = 0; x < 16; x++) {
#ifdef USE_SPRITE_RENDER
					uint32_t pal = rb_spram[sp_num][addr];
#else
					uint32_t pal = rb_pcg[ptn_num][hv][addr] | bg_color;
#endif
					*p = (uint16_t)(pal);
					p++;
					addr++;
				}
				p += (width - 16);
			}
			sp_num++;
		}
	}
}

/// @brief Expand Sprite data to screen buffer
/// @param[in] width 512 or 1024
/// @param[in] height 512 or 1024
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_sprite_render(int width, int height, scrntype *buffer)
{
	for(int sp_num = 127; sp_num >= 0; sp_num--) {
		if (!(p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK)) {
			continue;
		}
		int dx = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_XPOS];
		int dy = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_YPOS];
		scrntype *p = buffer + dy * width + dx;
		uint32_t addr = 0;
#ifndef USE_SPRITE_RENDER
		// pattern number and color
		int ptn_num = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PTN];
		int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
		uint32_t bg_color = (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
		ptn_num &= 0xff;
#endif
		for(int y = 0; y < 16 && (dy + y) < height; y++) {
			for(int x = 0; x < 16; x++) {
#ifdef USE_SPRITE_RENDER
				uint32_t pal = rb_spram[sp_num][addr];
#else
				uint32_t pal = rb_pcg[ptn_num][hv][addr] | bg_color;
#endif
				if ((dx + x) < width) {
					*p = debug_expand_palette(pal);
				}
				p++;
				addr++;
			}
			p += (width - 16);
		}
	}
}

/// @brief Dump Sprite data
/// @param[in] width 512 or 1024
/// @param[in] height 512 or 1024
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_sprite_dumper(int width, int height, uint16_t *buffer)
{
	memset(buffer, 0xff, width * height * sizeof(uint16_t));

	for(int sp_num = 127; sp_num >= 0; sp_num--) {
		if (!(p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK)) {
			continue;
		}
		int dx = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_XPOS];
		int dy = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_YPOS];
		uint16_t *p = buffer + dy * width + dx;
		uint32_t addr = 0;
#ifndef USE_SPRITE_RENDER
		// pattern number and color
		int ptn_num = p_sp_regs[(sp_num << 2) | SPRITE_BG::SP_PTN];
		int hv = ((ptn_num & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);
		uint32_t bg_color = (0x100 | ((ptn_num & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4))); 
		ptn_num &= 0xff;
#endif
		for(int y = 0; y < 16 && (dy + y) < height; y++) {
			for(int x = 0; x < 16; x++) {
#ifdef USE_SPRITE_RENDER
				uint32_t pal = rb_spram[sp_num][addr];
#else
				uint32_t pal = rb_pcg[ptn_num][hv][addr] | bg_color;
#endif
				if ((dx + x) < width) {
					*p = (uint16_t)(pal);
				}
				p++;
				addr++;
			}
			p += (width - 16);
		}
	}
}
#endif /* USE_DEBUGGER */

// ----------------------------------------------------------------------------
// Expand PCG data
// ----------------------------------------------------------------------------

/// @brief Expand PCG data per one line (8dot width)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[out] buffer_n : normal data
/// @param[out] buffer_h : reversed horizontal data
void DISPLAY::expand_pcg_one_line_sub_ad(uint32_t src_addr, uint8_t *buffer_n, uint8_t *buffer_h)
{
	uint32_t dot4;
	uint32_t pal;

	buffer_n+=8;
	src_addr++;
	for(int j=0; j<2; j++) {
		dot4 = p_spram[src_addr];
		for(int i=0; i<4; i++) {
			pal = dot4 & RB_PCG_PALETTE;

			// h reverse
			*buffer_h = pal;
			buffer_h++;

			// normal direction
			buffer_n--;
			*buffer_n = pal;

			dot4 >>= 4;
		}
		src_addr--;
	}
}

/// @brief Expand PCG data per one line (8dot width)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[out] buffer : expanded data
void DISPLAY::expand_pcg_one_line_sub(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, uint16_t *buffer)
{
	uint32_t dot4;
	uint32_t pal;

	if (h_reverse) {
		src_addr++;
		for(int j=0; j<2; j++) {
			dot4 = p_spram[src_addr];
			for(int i=0; i<4; i++) {
				pal = dot4 & 0xf;
				pal |= bg_palette;

				*buffer = 0x100 | pal;

				buffer++;

				dot4 >>= 4;
			}
			src_addr--;
		}
	} else {
		src_addr++;
		buffer+=8;
		for(int j=0; j<2; j++) {
			dot4 = p_spram[src_addr];
			for(int i=0; i<4; i++) {
				pal = dot4 & 0xf;
				pal |= bg_palette;

				buffer--;

				*buffer = 0x100 | pal;

				dot4 >>= 4;
			}
			src_addr--;
		}
	}
}

/// @brief Expand PCG data (8x8dot)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[in] width : 8
/// @param[out] buffer_n : expanded data (normal)
/// @param[out] buffer_h : expanded data (reversed horizontal)
/// @param[out] buffer_v : expanded data (reversed vertical)
/// @param[out] buffer_hv : expanded data (reversed horizontal and vertical)
void DISPLAY::expand_pcg_one_data_ad(uint32_t src_addr, int width, uint8_t *buffer_n, uint8_t *buffer_h, uint8_t *buffer_v, uint8_t *buffer_hv)
{
	// normal and h reverse
	for(int y=0; y<8; y++) {
		// expand dot pattern
		expand_pcg_one_line_sub_ad(src_addr, buffer_n, buffer_h);
		buffer_n += width;
		buffer_h += width;
		src_addr+=2;
	}
	// v reverse and hv reverse
	for(int y=0; y<8; y++) {
		// expand dot pattern
		src_addr-=2;
		expand_pcg_one_line_sub_ad(src_addr, buffer_v, buffer_hv);
		buffer_v += width;
		buffer_hv += width;
	}
}

/// @brief Expand PCG data (8x8dot)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 8
/// @param[out] buffer : expanded data
void DISPLAY::expand_pcg_one_data(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer)
{
	if (v_reverse) {
		src_addr += 14;
		for(int y=0; y<8; y++) {
			// expand dot pattern
			expand_pcg_one_line_sub(src_addr, bg_palette, h_reverse, buffer);
			buffer += width;
			src_addr-=2;
		}
	} else {
		for(int y=0; y<8; y++) {
			// expand dot pattern
			expand_pcg_one_line_sub(src_addr, bg_palette, h_reverse, buffer);
			buffer += width;
			src_addr+=2;
		}
	}
}

/// @brief Expand PCG data (16x16dot)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[in] width : 16
/// @param[out] buffer_n : expanded data (normal)
/// @param[out] buffer_h : expanded data (reversed horizontal)
/// @param[out] buffer_v : expanded data (reversed vertical)
/// @param[out] buffer_hv : expanded data (reversed horizontal and vertical)
void DISPLAY::expand_pcg_one_data16x16_ad(uint32_t src_addr, int width, uint8_t *buffer_n, uint8_t *buffer_h, uint8_t *buffer_v, uint8_t *buffer_hv)
{
	// 0 (upper left on normal direction)
	expand_pcg_one_data_ad(src_addr     , width, buffer_n, buffer_h + 8, buffer_v + width * 8, buffer_hv + width * 8 + 8);
	// 2 (upper right on normal direction)
	expand_pcg_one_data_ad(src_addr + 32, width, buffer_n + 8, buffer_h, buffer_v + width * 8 + 8, buffer_hv + width * 8);
	// 1 (lower left)
	expand_pcg_one_data_ad(src_addr + 16, width, buffer_n + width * 8, buffer_h + width * 8 + 8, buffer_v, buffer_hv + 8);
	// 3 (lower right)
	expand_pcg_one_data_ad(src_addr + 48, width, buffer_n + width * 8 + 8, buffer_h + width * 8, buffer_v + 8, buffer_hv);
}

/// @brief Expand PCG data (16x16dot)
/// @param[in] src_addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 16
/// @param[out] buffer : expanded data
void DISPLAY::expand_pcg_one_data16x16(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer)
{
	if (v_reverse) {
		if (h_reverse) {
			expand_pcg_one_data(src_addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			expand_pcg_one_data(src_addr + 48, bg_palette, h_reverse, v_reverse, width, buffer);
			expand_pcg_one_data(src_addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			expand_pcg_one_data(src_addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			expand_pcg_one_data(src_addr + 16, bg_palette, h_reverse, v_reverse, width, buffer);
			expand_pcg_one_data(src_addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			expand_pcg_one_data(src_addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			expand_pcg_one_data(src_addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	} else {
		if (h_reverse) {
			expand_pcg_one_data(src_addr     , bg_palette, h_reverse, v_reverse, width, buffer + 8);
			expand_pcg_one_data(src_addr + 32, bg_palette, h_reverse, v_reverse, width, buffer);
			expand_pcg_one_data(src_addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			expand_pcg_one_data(src_addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			expand_pcg_one_data(src_addr     , bg_palette, h_reverse, v_reverse, width, buffer);
			expand_pcg_one_data(src_addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			expand_pcg_one_data(src_addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			expand_pcg_one_data(src_addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	}
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
#if 1
/// @brief Expand PCG data to screen buffer per one line (8dot width)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_line(uint32_t addr, uint32_t bg_palette, bool h_reverse, scrntype *buffer)
{
	uint32_t dot4;
	uint32_t pal;

	if (h_reverse) {
		addr++;
		for(int j=0; j<2; j++) {
			dot4 = p_spram[addr];
			for(int i=0; i<4; i++) {
				pal = dot4 & 0xf;
				pal |= bg_palette;

				*buffer = debug_expand_palette(0x100 | pal);

				buffer++;

				dot4 >>= 4;
			}
			addr--;
		}
	} else {
		for(int j=0; j<2; j++) {
			dot4 = p_spram[addr];
			for(int i=0; i<4; i++) {
				pal = (dot4 >> 12) & 0xf;
				pal |= bg_palette;

				*buffer = debug_expand_palette(0x100 | pal);

				buffer++;

				dot4 <<= 4;
			}
			addr++;
		}
	}
}

/// @brief Dump PCG data at one line (8dot width)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_line_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, uint16_t *buffer)
{
	uint32_t dot4;
	uint32_t pal;

	if (h_reverse) {
		addr++;
		for(int j=0; j<2; j++) {
			dot4 = p_spram[addr];
			for(int i=0; i<4; i++) {
				pal = dot4 & 0xf;
				pal |= bg_palette;

				*buffer = (0x100 | pal);

				buffer++;

				dot4 >>= 4;
			}
			addr--;
		}
	} else {
		for(int j=0; j<2; j++) {
			dot4 = p_spram[addr];
			for(int i=0; i<4; i++) {
				pal = (dot4 >> 12) & 0xf;
				pal |= bg_palette;

				*buffer = (0x100 | pal);

				buffer++;

				dot4 <<= 4;
			}
			addr++;
		}
	}
}

/// @brief Expand PCG data to screen buffer (8x8dot)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 8 / 16
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_data(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, scrntype *buffer)
{
	if (v_reverse) {
		addr += 14;
		for(int y=0; y<8; y++) {
			// expand dot pattern
			debug_expand_pcg_one_line(addr, bg_palette, h_reverse, buffer);
			buffer += width;
			addr-=2;
		}
	} else {
		for(int y=0; y<8; y++) {
			// expand dot pattern
			debug_expand_pcg_one_line(addr, bg_palette, h_reverse, buffer);
			buffer += width;
			addr+=2;
		}
	}
}

/// @brief Dump PCG data (8x8dot)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 8 / 16
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_data_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer)
{
	if (v_reverse) {
		addr += 14;
		for(int y=0; y<8; y++) {
			// expand dot pattern
			debug_expand_pcg_one_line_dumper(addr, bg_palette, h_reverse, buffer);
			buffer += width;
			addr-=2;
		}
	} else {
		for(int y=0; y<8; y++) {
			// expand dot pattern
			debug_expand_pcg_one_line_dumper(addr, bg_palette, h_reverse, buffer);
			buffer += width;
			addr+=2;
		}
	}
}

/// @brief Expand PCG data to screen buffer (16x16dot)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 16
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_data16x16(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, scrntype *buffer)
{
	if (v_reverse) {
		if (h_reverse) {
			debug_expand_pcg_one_data(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data(addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			debug_expand_pcg_one_data(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			debug_expand_pcg_one_data(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data(addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			debug_expand_pcg_one_data(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	} else {
		if (h_reverse) {
			debug_expand_pcg_one_data(addr     , bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			debug_expand_pcg_one_data(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			debug_expand_pcg_one_data(addr     , bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			debug_expand_pcg_one_data(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	}
}

/// @brief Dump PCG data (16x16dot)
/// @param[in] addr : start address of PCG data on sprite RAM
/// @param[in] bg_palette : palette number (b7-b4)
/// @param[in] h_reverse : true if data is reversed horizontal
/// @param[in] v_reverse : true if data is reversed vertical
/// @param[in] width : 16
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_one_data16x16_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer)
{
	if (v_reverse) {
		if (h_reverse) {
			debug_expand_pcg_one_data_dumper(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data_dumper(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data_dumper(addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			debug_expand_pcg_one_data_dumper(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			debug_expand_pcg_one_data_dumper(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data_dumper(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data_dumper(addr     , bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			debug_expand_pcg_one_data_dumper(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	} else {
		if (h_reverse) {
			debug_expand_pcg_one_data_dumper(addr     , bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data_dumper(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data_dumper(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
			debug_expand_pcg_one_data_dumper(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
		} else {
			debug_expand_pcg_one_data_dumper(addr     , bg_palette, h_reverse, v_reverse, width, buffer);
			debug_expand_pcg_one_data_dumper(addr + 32, bg_palette, h_reverse, v_reverse, width, buffer + 8);
			debug_expand_pcg_one_data_dumper(addr + 16, bg_palette, h_reverse, v_reverse, width, buffer + width * 8);
			debug_expand_pcg_one_data_dumper(addr + 48, bg_palette, h_reverse, v_reverse, width, buffer + width * 8 + 8);
		}
	}
}

/// @brief Expand PCG data to screen buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_data(int width, int height, scrntype *buffer)
{
	for(int y=0; y<height; y++) {
		uint32_t ptn_addr = (y & 7) | ((y & ~7) * width / 8);
		ptn_addr <<= 1;
		for(int x=0; x<width; x+=8) {
			debug_expand_pcg_one_line(ptn_addr, 0, false, buffer + x);
			ptn_addr += 16;
		}
		buffer += width;
	}
}
/// @brief Dump PCG data
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_data_dumper(int width, int height, uint16_t *buffer)
{
	for(int y=0; y<height; y++) {
		uint32_t ptn_addr = (y & 7) | ((y & ~7) * width / 8);
		ptn_addr <<= 1;
		for(int x=0; x<width; x+=8) {
			debug_expand_pcg_one_line_dumper(ptn_addr, 0, false, buffer + x);
			ptn_addr += 16;
		}
		buffer += width;
	}
}
#endif

/// @brief Expand PCG render buffer to screen buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_render(int width, int height, scrntype *buffer)
{
	for(int ptn_num=0; ptn_num<256; ptn_num+=8) {
		for(int y=0; y<16; y++) {
			for(int n=0; n<8; n++) {
				for(int hv=0; hv<4; hv++) {
					for(int x=0; x<16; x++) {
						uint32_t pal = rb_pcg[ptn_num + n][hv][y * 16 + x];
						*buffer = debug_expand_palcol8(0x100 | pal);
						buffer++;
					}
				}
			}
		}
	}
}

/// @brief Dump PCG render buffer
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_pcg_dumper(int width, int height, uint16_t *buffer)
{
	for(int ptn_num=0; ptn_num<256; ptn_num+=8) {
		for(int y=0; y<16; y++) {
			for(int n=0; n<8; n++) {
				for(int hv=0; hv<4; hv++) {
					for(int x=0; x<16; x++) {
						uint16_t pal = rb_pcg[ptn_num + n][hv][y * 16 + x];
						*buffer = (0x100 | pal);
						buffer++;
					}
				}
			}
		}
	}
}

/// @brief Expand BG data to screen buffer
/// @param[in] area : BG area 0 or 1
/// @param[in] hireso is 1
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_bg_data(int area, int hireso, int width, int height, scrntype *buffer)
{
	uint32_t addr_offset = (area == 0 ? 0x2000 : 0x3000);	// BG 0 or 1

	hireso &= 1;

	for(int by=0; by<64; by++) {
		uint32_t addr = addr_offset + (by << 6);

		for(int bx=0; bx<64; bx++) {
			// get pattern number
			uint32_t pattern = p_spram[addr];
			uint32_t ptn_addr = (pattern & 0xff);
			uint32_t bg_palette = (pattern & SP_BG_BG_COLOR) >> 4;
			bool h_reverse = (pattern & SP_BG_H_REVERSE) != 0;
			bool v_reverse = (pattern & SP_BG_V_REVERSE) != 0;

			if (hireso) {
				ptn_addr <<= 6; // x64
			} else {
				ptn_addr <<= 4;	// x16
			}

			if (hireso) {
				debug_expand_pcg_one_data16x16(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
				buffer += 16;
			} else {
				debug_expand_pcg_one_data(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
				buffer += 8;
			}
			addr++;
		}
		buffer += ((8 << hireso) - 1) * width;
	}
}

/// @brief Dump BG data on sepcified area
/// @param[in] area : BG area 0 or 1
/// @param[in] hireso is 1
/// @param[in] width
/// @param[in] height
/// @param[out] buffer : screen buffer
void DISPLAY::debug_expand_bg_data_dumper(int area, int hireso, int width, int height, uint16_t *buffer)
{
	uint32_t addr_offset = (area == 0 ? 0x2000 : 0x3000);	// BG 0 or 1

	hireso &= 1;

	for(int by=0; by<64; by++) {
		uint32_t addr = addr_offset + (by << 6);

		for(int bx=0; bx<64; bx++) {
			// get pattern number
			uint32_t pattern = p_spram[addr];
			uint32_t ptn_addr = (pattern & 0xff);
			uint32_t bg_palette = (pattern & SP_BG_BG_COLOR) >> 4;
			bool h_reverse = (pattern & SP_BG_H_REVERSE) != 0;
			bool v_reverse = (pattern & SP_BG_V_REVERSE) != 0;

			if (hireso) {
				ptn_addr <<= 6; // x64
			} else {
				ptn_addr <<= 4;	// x16
			}

			if (hireso) {
				debug_expand_pcg_one_data16x16_dumper(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
				buffer += 16;
			} else {
				debug_expand_pcg_one_data_dumper(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
				buffer += 8;
			}
			addr++;
		}
		buffer += ((8 << hireso) - 1) * width;
	}
}
#endif /* USE_DEBUGGER */

// ----------------------------------------------------------------------------
// Sprite and BG
// ----------------------------------------------------------------------------

/// @brief Render BG data per one line
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::render_sprite_bg_one_line(int src_y, int dst_y)
{
	render_bg0_one_line(src_y, dst_y);
	render_bg1_one_line(src_y, dst_y);
#ifdef USE_SPRITE_RENDER
	render_sprite_one_line(y);
#endif
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Render BG data
void DISPLAY::render_sprite_bg()
{
	render_bg0();
	render_bg1();
#ifdef USE_SPRITE_RENDER
	render_sprite();
#endif
}
#endif

// ----------------------------------------------------------------------------
// Render BG
// ----------------------------------------------------------------------------

/// @brief Render BG0 data per one line
/// @param[in] size512 : set 1 on 512x512mode
/// @param[in] addr_offset : offset of sprite ram address (BG page 0 or 1)
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::render_bg0_one_line_sub(int size512, uint32_t addr_offset, int src_y, int dst_y)
{
	uint16_t *dst = rb_bgram0 + (dst_y << 10);	// 1024

	int step = (8 << size512) - 1;	// 7 or 15
	int src_l = (src_y & step);
	uint32_t src_y_step = (src_y & ~step) << (3 - size512);
	uint32_t src_addr = addr_offset + src_y_step;

	for(int bx=0; bx<64; bx++) {
		// get pattern number
		uint32_t pattern = p_spram[src_addr];
//		uint32_t src_uaddr = src_addr & 0x1fff;
		uint32_t ptn_num = (pattern & 0xff);
		uint32_t ptn_addr = ptn_num;
//		uint32_t u_mask = (1 << src_l);

		// pattern number updated?
//		if (pu_pcg[ptn_num] | (pu_bg[0][src_uaddr] & u_mask)) {
			// check pcg data was updated.
			if (size512) {
				ptn_addr <<= 6; // x64
				render_pcg_one_16x16(ptn_num);
			} else {
				ptn_addr <<= 4;	// x16
				render_pcg_one_16x16(ptn_num >> 2);
			}

			uint32_t bg_palette = 0x100 | ((pattern & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4));
			int hv = ((pattern & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);

			if (size512) {
				copy_pcg_one_16x1(ptn_num, hv, bg_palette, src_l, dst);
			} else {
				copy_pcg_one_8x1(ptn_num, hv, bg_palette, src_l, dst);
			}
//			pu_bg[0][src_uaddr] &= ~u_mask;
//		}
		src_addr++;
		dst += step + 1;	// 8 or 16
	}
}

/// @brief Render BG1 data per one line
/// @param[in] size512 : set 1 on 512x512mode
/// @param[in] addr_offset : offset of sprite ram address (BG page 0 or 1)
/// @param[in] src_y : 0 - 511
/// @param[in] dst_y : 0 - 511
void DISPLAY::render_bg1_one_line_sub(int size512, uint32_t addr_offset, int src_y, int dst_y)
{
	uint16_t *dst = rb_bgram1 + (dst_y << 9);	// 512

	int step = (8 << size512) - 1;	// 7 or 15
	int src_l = (src_y & step);
	uint32_t src_y_step = (src_y & ~step) << (3 - size512);
	uint32_t src_addr = addr_offset + src_y_step;

	for(int bx=0; bx<64; bx++) {
		// get pattern number
		uint32_t pattern = p_spram[src_addr];
//		uint32_t src_uaddr = src_addr & 0x1fff;
		uint32_t ptn_num = (pattern & 0xff);
		uint32_t ptn_addr = ptn_num;
//		uint32_t u_mask = (1 << src_l);

		// pattern number updated?
//		if (pu_pcg[ptn_num] | (pu_bg[1][src_uaddr] & u_mask)) {
			// check pcg data was updated.
			if (size512) {
				ptn_addr <<= 6; // x64
				render_pcg_one_16x16(ptn_num);
			} else {
				ptn_addr <<= 4;	// x16
				render_pcg_one_16x16(ptn_num >> 2);
			}

			uint32_t bg_palette = 0x100 | ((pattern & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4));
			int hv = ((pattern & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);

			if (size512) {
				copy_pcg_one_16x1(ptn_num, hv, bg_palette, src_l, dst);
			} else {
				copy_pcg_one_8x1(ptn_num, hv, bg_palette, src_l, dst);
			}
//			pu_bg[1][src_uaddr] &= ~u_mask;
//		}
		src_addr++;
		dst += step + 1;	// 8 or 16
	}
}

/// @brief Render BG0 data per one block (8x8 / 16x16)
/// @param[in] size512 : set 1 on 512x512mode
/// @param[in] width : real size 512 or 1024
/// @param[in] addr_offset : offset of sprite ram address (BG page 0 or 1)
/// @param[in] by : pattern block position y (0 - 63)
void DISPLAY::render_bg0_one_block_sub(int size512, int width, uint32_t addr_offset, int by)
{
	uint16_t *dst = rb_bgram0 + by * width * (8 << size512);

	uint32_t src_addr = addr_offset + (by << 6);

	for(int bx=0; bx<64; bx++) {
		// get pattern number
		uint32_t pattern = p_spram[src_addr];
//		uint32_t src_uaddr = src_addr & 0x1fff;
		uint32_t ptn_num = (pattern & 0xff);
		uint32_t ptn_addr = ptn_num;

//		if (pu_pcg[ptn_num] | pu_bg[0][src_uaddr]) {

			if (size512) {
				ptn_addr <<= 6; // x64
				render_pcg_one_16x16(ptn_num);
			} else {
				ptn_addr <<= 4;	// x16
				render_pcg_one_16x16(ptn_num >> 2);
			}
//			pu_bg[0][src_uaddr] = 0;

			uint32_t bg_palette = 0x100 | (pattern & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4);
			int hv = ((pattern & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);

			if (size512) {
				copy_pcg_one_16x16(ptn_num, hv, bg_palette, dst, width);
			} else {
				copy_pcg_one_8x8(ptn_num, hv, bg_palette, dst, width);
			}
//		}
		src_addr++;
		dst += (8 << size512);
	}
}

/// @brief Render BG1 data per one block (8x8 / 16x16)
/// @param[in] size512 : set 1 on 512x512mode
/// @param[in] width : real size 512 or 1024
/// @param[in] addr_offset : offset of sprite ram address (BG page 0 or 1)
/// @param[in] by : pattern block position y (0 - 63)
void DISPLAY::render_bg1_one_block_sub(int size512, int width, uint32_t addr_offset, int by)
{
	uint16_t *dst = rb_bgram1 + by * width * (8 << size512);

	uint32_t src_addr = addr_offset + (by << 6);

	for(int bx=0; bx<64; bx++) {
		// get pattern number
		uint32_t pattern = p_spram[src_addr];
//		uint32_t src_uaddr = src_addr & 0x1fff;
		uint32_t ptn_num = (pattern & 0xff);
		uint32_t ptn_addr = ptn_num;

//		if (pu_pcg[ptn_num] | pu_bg[1][src_uaddr]) {
			if (size512) {
				ptn_addr <<= 6; // x64
				render_pcg_one_16x16(ptn_num);
			} else {
				ptn_addr <<= 4;	// x16
				render_pcg_one_16x16(ptn_num >> 2);
			}
//			pu_bg[1][src_uaddr] = 0;

			uint32_t bg_palette = 0x100 | (pattern & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4);
			int hv = ((pattern & SP_BG_HV_REVERSE) >> SP_BG_HV_REVERSE_SFT);

			if (size512) {
				copy_pcg_one_16x16(ptn_num, hv, bg_palette, dst, width);
			} else {
				copy_pcg_one_8x8(ptn_num, hv, bg_palette, dst, width);
			}
//		}
		src_addr++;
		dst += (8 << size512);
	}
}

/// @brief Render BG0 data per one line
/// @param[in] src_y : line 0 - 1023
/// @param[in] dst_y : line 0 - 1023
void DISPLAY::render_bg0_one_line(int src_y, int dst_y)
{
	if (!(m_show_screen & VM::BG0Mask)) {
		// bg0 is off
		return;
	}

	// select BG area 0 or 1
	uint32_t addr_offset = (p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG0_AREA_SEL) ? 0x3000 : 0x2000;

	int size512 = ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HRES) == 0x01 ? 1 : 0);

//	int width = 1024;

	int limit = ((512 << size512) - 1);	// 511 or 1023

	int top = p_bg_regs[SPRITE_BG::BG0_SCROLL_Y]; 

	src_y += top;
	src_y &= limit;
	dst_y += top;
	dst_y &= limit;

	render_bg0_one_line_sub(size512, addr_offset, src_y, dst_y);
}

/// @brief Render BG1 data per one line
/// @param[in] src_y : line 0 - 511
/// @param[in] dst_y : line 0 - 511
void DISPLAY::render_bg1_one_line(int src_y, int dst_y)
{
	if (!(m_show_screen & VM::BG1Mask)) {
		// bg1 is off
		return;
	}

	// select BG area 0 or 1
	uint32_t addr_offset = (p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG1_AREA_SEL) ? 0x3000 : 0x2000;

	int size512 = 0;

//	int width = 512;

	int limit = ((512 << size512) - 1);	// 511 or 1023

	int top = p_bg_regs[SPRITE_BG::BG1_SCROLL_Y]; 

	src_y += top;
	src_y &= limit;
	dst_y += top;
	dst_y &= limit;

	render_bg1_one_line_sub(size512, addr_offset, src_y, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Render BG0 data
void DISPLAY::render_bg0()
{
	if (!(m_show_screen & VM::BG0Mask)) {
		// bg0 is off
		return;
	}

	// select BG area 0 or 1
	uint32_t addr_offset = (p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG0_AREA_SEL) ? 0x3000 : 0x2000;

	int size512 = ((p_bg_regs[SPRITE_BG::BG_RESOLUTION] & SPRITE_BG::RESO_HRES) == 0x01 ? 1 : 0);

	int width = 1024;

	for(int by=0; by<64; by++) {
		render_bg0_one_block_sub(size512, width, addr_offset, by);
	}
}

/// @brief Render BG1 data
void DISPLAY::render_bg1()
{
	if (!(m_show_screen & VM::BG1Mask)) {
		// bg1 is off
		return;
	}

	// select BG area 0 or 1
	uint32_t addr_offset = (p_bg_regs[SPRITE_BG::BG_CONTROL] & SPRITE_BG::CONT_BG1_AREA_SEL) ? 0x3000 : 0x2000;

	int size512 = 0;

	int width = 512;

	for(int by=0; by<64; by++) {
		render_bg1_one_block_sub(size512, width, addr_offset, by);
	}
}
#endif

// ----------------------------------------------------------------------------
// Render Sprite
// ----------------------------------------------------------------------------

#ifdef USE_SPRITE_RENDER
/// @brief Render Sprite cell per one line
/// @param[in] sp_num : sprite number
/// @param[in] y : line 0 - 1023
void DISPLAY::render_sprite_cell_one_line(int sp_num, int y)
{
	int sp_num4 = (sp_num << 2);

	if (!(p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK)) {
		// sprite is off
		return;
	}

	int width = 16;

	if (y & 0xf) return;

	if (p_sp_regs[sp_num4 | SPRITE_BG::SP_YPOS] < y || (y + width) <= p_sp_regs[sp_num4 | SPRITE_BG::SP_YPOS]) {
		// sprite is out of range
		return;
	}

	// get pattern number
	uint32_t pattern = p_sp_regs[sp_num4 | SPRITE_BG::SP_PTN];
	uint32_t updated = p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_UPD_MASK;
	uint32_t ptn_num = (pattern & 0xff);
	uint32_t ptn_addr = ptn_num;

	ptn_addr <<= 6; // x64

//	updated |= (pu_spram[ptn_addr] | pu_spram[ptn_addr + 16] | pu_spram[ptn_addr + 32] | pu_spram[ptn_addr + 48]);
	updated |= (pu_pcg[ptn_num] ? SPRITE_BG::SP_PRW_UPD_MASK : 0);

//	updated &= SPRITE_BG::SP_PRW_UPD_MASK;
	if (updated) {
		p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] &= ~SPRITE_BG::SP_PRW_UPD_MASK;

//		update_pcg_read_flags(ptn_num);

		uint32_t bg_palette = (pattern & SP_BG_BG_COLOR) >> 4;
		bool h_reverse = (pattern & SP_BG_H_REVERSE) != 0;
		bool v_reverse = (pattern & SP_BG_V_REVERSE) != 0;

		uint16_t *buffer = rb_spram[sp_num];

		expand_pcg_one_data16x16(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
	}
}
#endif

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
#ifdef USE_SPRITE_RENDER
/// @brief Render Sprite cell
/// @param[in] sp_num : sprite number
void DISPLAY::render_sprite_cell(int sp_num)
{
	int sp_num4 = (sp_num << 2);

	if (!(p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_MASK)) {
		// sprite is off
		return;
	}

	int width = 16;

	// get pattern number
	uint32_t pattern = p_sp_regs[sp_num4 | SPRITE_BG::SP_PTN];
	uint32_t updated = p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] & SPRITE_BG::SP_PRW_UPD_MASK;
	uint32_t ptn_num = (pattern & 0xff);
	uint32_t ptn_addr = ptn_num;

	ptn_addr <<= 6; // x64

//	updated |= (pu_spram[ptn_addr] | pu_spram[ptn_addr + 16] | pu_spram[ptn_addr + 32] | pu_spram[ptn_addr + 48]);
	updated |= (pu_pcg[ptn_num] ? SPRITE_BG::SP_PRW_UPD_MASK : 0);

//	updated &= SPRITE_BG::SP_PRW_UPD_MASK;
	if (updated) {
		p_sp_regs[sp_num4 | SPRITE_BG::SP_PRW] &= ~SPRITE_BG::SP_PRW_UPD_MASK;

//		update_pcg_read_flags(ptn_num);

		uint32_t bg_palette = (pattern & SP_BG_BG_COLOR) >> (SP_BG_BG_COLOR_SFT - 4);
		bool h_reverse = (pattern & SP_BG_H_REVERSE) != 0;
		bool v_reverse = (pattern & SP_BG_V_REVERSE) != 0;

		uint16_t *buffer = rb_spram[sp_num];

		expand_pcg_one_data16x16(ptn_addr, bg_palette, h_reverse, v_reverse, width, buffer);
	}
}
#endif
#endif

#ifdef USE_SPRITE_RENDER
/// @brief Render all Sprite per one line
/// @param[in] y : line 0 - 1023
void DISPLAY::render_sprite_one_line(int y)
{
//	init_pcg_read_flags();

	int disp_top = (p_bg_regs[SPRITE_BG::BG_VERT_DISP] - p_crtc_regs[CRTC::CRTC_VERT_START]);
	// 256line?
	if ((p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRES) == 0) disp_top /= 2;

	for(int sp_num = 0; sp_num < 128; sp_num++) {
		render_sprite_cell_one_line(sp_num, y + 16 + disp_top);
	}

	// clear update flag on pcg area
//	commit_pcg_read_flags_16x16(~SPRITE_BG::SP_PRW_UPD_MASK);
}
#endif

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
#ifdef USE_SPRITE_RENDER
/// @brief Render all Sprite
void DISPLAY::render_sprite()
{
//	init_pcg_read_flags();

	for(int sp_num = 0; sp_num < 128; sp_num++) {
		render_sprite_cell(sp_num);
	}

	// clear update flag on pcg area
//	commit_pcg_read_flags_16x16(~SPRITE_BG::SP_PRW_UPD_MASK);
}
#endif
#endif

// ----------------------------------------------------------------------------

/// @brief Expand one PCG data to PCG render buffer
/// @param[in] ptn_num : pattern number of PCG
void DISPLAY::render_pcg_one_16x16(int ptn_num)
{
	if (pu_pcg[ptn_num]) {
		uint32_t ptn_addr = (ptn_num << 6); // * 16 * 4

		expand_pcg_one_data16x16_ad(ptn_addr, 16
			, rb_pcg[ptn_num][0]
			, rb_pcg[ptn_num][1]
			, rb_pcg[ptn_num][2]
			, rb_pcg[ptn_num][3]
		);

		pu_pcg[ptn_num] = 0;
	}
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Expand one PCG data to PCG render buffer
void DISPLAY::render_pcg()
{
	for(int ptn_num = 0; ptn_num < 256; ptn_num++) {
		render_pcg_one_16x16(ptn_num);
	}
}
#endif

// ----------------------------------------------------------------------------

/// @brief Copy one PCG buffer to mixed buffer (16dot width)
/// @param[in] ptn_num : pattern number of PCG
/// @param[in] hv : normal:0 reversed horizontal:1 reversed vertical:2 reversed all:3
/// @param[in] bg_ptn : palette number and priority 
/// @param[in] l : line of PCG data
/// @param[out] dst : buffer
void DISPLAY::copy_pcg_one_16x1(int ptn_num, int hv, uint32_t bg_ptn, int l, uint16_t *dst)
{
	uint8_t *src = rb_pcg[ptn_num][hv];

	src += (l << 4);
	for(int x = 0; x < 16; x++) {
		*dst = *src | bg_ptn;
		src++;
		dst++;
	}
}

/// @brief Copy one PCG buffer to mixed buffer (8dot width)
/// @param[in] ptn_num : pattern number of PCG
/// @param[in] hv : normal:0 reversed horizontal:1 reversed vertical:2 reversed all:3
/// @param[in] bg_ptn : palette number and priority 
/// @param[in] l : line of PCG data
/// @param[out] dst : buffer
void DISPLAY::copy_pcg_one_8x1(int ptn_num, int hv, uint32_t bg_ptn, int l, uint16_t *dst)
{
	uint8_t *src = rb_pcg[ptn_num >> 2][hv];

	src += ((((((ptn_num & 1) << 1) ^ (hv & 2)) << 3) + (((ptn_num & 2) >> 1) ^ (hv & 1))) << 3);

	src += (l << 4);
	for(int x = 0; x < 8; x++) {
		*dst = *src | bg_ptn;
		src++;
		dst++;
	}
}

/// @brief Copy one PCG buffer to mixed buffer (16x16dot)
/// @param[in] ptn_num : pattern number of PCG
/// @param[in] hv : normal:0 reversed horizontal:1 reversed vertical:2 reversed all:3
/// @param[in] bg_ptn : palette number and priority 
/// @param[out] dst : buffer
/// @param[in] width : width of buffer
void DISPLAY::copy_pcg_one_16x16(int ptn_num, int hv, uint32_t bg_ptn, uint16_t *dst, int width)
{
	uint8_t *src = rb_pcg[ptn_num][hv];
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
			*dst = *src | bg_ptn;
			src++;
			dst++;
		}
		dst += width;
		dst -= 16;
	}
}

/// @brief Copy one PCG buffer to mixed buffer (8x8dot)
/// @param[in] ptn_num : pattern number of PCG
/// @param[in] hv : normal:0 reversed horizontal:1 reversed vertical:2 reversed all:3
/// @param[in] bg_ptn : palette number and priority 
/// @param[out] dst : buffer
/// @param[in] width : width of buffer
void DISPLAY::copy_pcg_one_8x8(int ptn_num, int hv, uint32_t bg_ptn, uint16_t *dst, int width)
{
	uint8_t *src = rb_pcg[ptn_num >> 2][hv];

	src += ((((((ptn_num & 1) << 1) ^ (hv & 2)) << 3) + (((ptn_num & 2) >> 1) ^ (hv & 1))) << 3);

	for(int y = 0; y < 8; y++) {
		for(int x = 0; x < 8; x++) {
			*dst = *src | bg_ptn;
			src++;
			dst++;
		}
		src += 8;
		dst += width;
		dst -= 8;
	}
}

// ----------------------------------------------------------------------------

#if 0
void DISPLAY::init_pcg_read_flags()
{
	for(int i=0; i<8; i++) m_pcg_read[i] = 0;
}

void DISPLAY::update_pcg_read_flags(int num)
{
	m_pcg_read[num >> 5] |= (1 << (num & 0x1f));
}

void DISPLAY::commit_pcg_read_flags_8x8(uint32_t mask)
{
	uint32_t ptn_addr = 0;
	for(int ptn_num=0; ptn_num<256; ptn_num++) {
		if (m_pcg_read[ptn_num >> 5] & (1 << ((ptn_num) & 0x1f))) {
			pu_spram[ptn_addr] &= mask;
		}
		ptn_addr += (1 << 4);	// x16
	}
}

void DISPLAY::commit_pcg_read_flags_16x16(uint32_t mask)
{
	uint32_t ptn_addr = 0;
	for(int ptn_num=0; ptn_num<(256 << 2); ptn_num++) {
		if (m_pcg_read[ptn_num >> 7] & (1 << ((ptn_num >> 2) & 0x1f))) {
			pu_spram[ptn_addr] &= mask;
		}
		ptn_addr += (1 << 4);	// x16
	}
}
#endif

// ----------------------------------------------------------------------------
// Mix Graphics
// ----------------------------------------------------------------------------

/// @brief Mix Graphics per one line
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic_one_line(int width, int src_y_base, int dst_y)
{
	if (m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_SZ_MASK) {
		// 1024 x 1024 mode
		mix_buffer_graphic1024_one_line(width, src_y_base, dst_y);
	} else {
		// 512 x 512 mode
		mix_buffer_graphic512_one_line(width, src_y_base, dst_y);
	}
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic(int width, int height9)
{
	if (m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_SZ_MASK) {
		// 1024 x 1024 mode
		mix_buffer_graphic1024(width, height9);
	} else {
		// 512 x 512 mode
		mix_buffer_graphic512(width, height9);
	}
}
#endif

/// @brief Set graphics flags on mixed buffer
/// @param[in] width
/// @param[in] dst_y
void DISPLAY::set_mix_buffer_graphic_flags(int width, int dst_y)
{
	uint32_t *dst_buf;
	dst_buf = &mx_buf[dst_y];
	for(int dst_x = 0; dst_x < width; dst_x++) {
		*dst_buf |= MX_BUF_GR_DATA;
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			// palette number on all graphics page is 0
			*dst_buf = ((*dst_buf) & ~MX_BUF_COLOR) | m_palette[0];
		}
		if (*dst_buf & MX_BUF_COLOR) {
			*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
		}
		dst_buf++;
	}
}

// ----------------------------------------------------------------------------

/// @brief Mix Graphics per one line (1024 x 1024 mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic1024_one_line(int width, int src_y_base, int dst_y)
{
	// 1024 x 1024 mode
	if (!(m_show_screen & VM::Graphic4Mask)) {
		// off
		return;
	}
	mix_buffer_graphic1024_c16_one_line(width, src_y_base, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics per one line (1024 x 1024 mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic1024(int width, int height9)
{
	// 1024 x 1024 mode
	if (!(m_show_screen & VM::Graphic4Mask)) {
		// off
		return;
	}
#ifdef USE_GRAPHIC_RENDER
	render_graphic1024();
#endif
	mix_buffer_graphic1024_c16(width, height9);
}
#endif

#ifdef USE_GRAPHIC_RENDER
/// @brief Mix Graphics per one line (1024 x 1024, 16 colors, normal mode)
/// @param[in] width
/// @param[in] src0_left
/// @param[in] src0_y
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int dst_y)
{
	int src0_x = src0_left;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		src0_x &= 0x3ff;
		uint32_t pal_num = rb_gvram[src0_y + src0_x] & 0xf;
		uint32_t pal = m_palette[pal_num];
		if ((mx_buf[dst_y + dst_x] & MX_BUF_NO_TRANS) == 0) {
			mx_buf[dst_y + dst_x] = pal;
			if (pal_num) mx_buf[dst_y + dst_x] |= MX_BUF_NO_TRANS;	// no transparent
		}
		src0_x++;
	}
}
#else
#ifndef USE_GRAPHIC_1024ALT
/// @brief Mix Graphics per one line (1024 x 1024, 16 colors, normal mode)
/// @param[in] width
/// @param[in] src0_left
/// @param[in] src0_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int sft[2], int dst_y)
{
	int src0_x = src0_left;
	int n = 0;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		if (src0_x >= 512) n = 1 - n;
		src0_x &= 0x1ff;
		uint32_t pal_num = (p_gvram[src0_y + src0_x] >> sft[n]) & 0xf;
		uint32_t pal = m_palette[pal_num];
		if ((mx_buf[dst_y + dst_x] & MX_BUF_NO_TRANS) == 0) {
			mx_buf[dst_y + dst_x] = pal;
			if (pal_num) mx_buf[dst_y + dst_x] |= MX_BUF_NO_TRANS;	// no transparent
		}
		src0_x++;
	}
}
#else
/// @brief Mix Graphics per one line (1024 x 1024, 16 colors, normal mode)
/// @param[in] width
/// @param[in] src0_left
/// @param[in] src0_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int sft, int dst_y)
{
	int src0_x = src0_left;
//	int n = 0;
	for(int dst_x = 0; dst_x < width; dst_x++) {
		src0_x &= 0x3ff;
		uint32_t pal_num = (p_gvram[src0_y + src0_x] >> sft) & 0xf;
		uint32_t pal = m_palette[pal_num];
		if ((mx_buf[dst_y + dst_x] & MX_BUF_NO_TRANS) == 0) {
			mx_buf[dst_y + dst_x] = pal;
			if (pal_num) mx_buf[dst_y + dst_x] |= MX_BUF_NO_TRANS;	// no transparent
		}
		src0_x++;
	}
}
#endif
#endif

/// @brief Mix Graphics per one line (1024 x 1024, 16 colors mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic1024_c16_one_line(int width, int src_y_base, int dst_y)
{
	int src0_left = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0];
	int src0_y = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0];

#ifdef USE_GRAPHIC_RENDER
	src0_y += dst_y;
	src0_y <<= 10;
	src0_y &= (0x3ff << 10);
	dst_y <<= MX_BUF_WIDTH_SFT;

	mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, dst_y);
#else
#ifndef USE_GRAPHIC_1024ALT
	int sft[2];

	src0_y += src_y_base;
	if (src0_y & 0x200) {
		sft[0] = m_vc_gr_prisfts[2];
		sft[1] = m_vc_gr_prisfts[3];
	} else {
		sft[0] = m_vc_gr_prisfts[0];
		sft[1] = m_vc_gr_prisfts[1];
	}

	src0_y <<= 9;
	src0_y &= (0x1ff << 9);
	dst_y <<= MX_BUF_WIDTH_SFT;

	mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, sft, dst_y);
#else
	src0_y += src_y_base;

	int sft = m_vc_gr_prisfts[(src0_y >> 8) & 0x3];

	src0_y <<= 10;
	src0_y &= (0xff << 10);
	dst_y <<= MX_BUF_WIDTH_SFT;

	mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, sft, dst_y);
#endif
#endif
	set_mix_buffer_graphic_flags(width, dst_y);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics per one line (1024 x 1024, 16 colors mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic1024_c16(int width, int height9)
{
	int src0_left = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0];
	int src0_y = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0];

#ifdef USE_GRAPHIC_RENDER
	src0_y <<= 10;
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		src0_y &= (0x3ff << 10);
		mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, dst_y);
		src0_y+=(1 << 10);
	}
#else
#if 0
	int sft[2];
	sft[0] = m_vc_gr_prisfts[0];
	sft[1] = m_vc_gr_prisfts[1];
	src0_y <<= 10;
	int pri = 0;
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		if (src0_y >= (512 << 10)) {
			pri = 2 - pri;
			sft[0] = m_vc_gr_prisfts[pri + 0];
			sft[1] = m_vc_gr_prisfts[pri + 1];
		}
		src0_y &= (0x1ff << 10);
		mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, sft, dst_y);
		src0_y+=(1 << 10);
	}
#else
	int src1_y = src0_y;
	src0_y <<= 10;
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		src0_y &= (0xff << 10);
		int sft = m_vc_gr_prisfts[(src1_y >> 8) & 0x3];
		mix_buffer_graphic1024_c16_one_line_sub(width, src0_left, src0_y, sft, dst_y);
		src0_y+=(1 << 10);
		src1_y++;
	}
#endif
#endif
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		set_mix_buffer_graphic_flags(width, dst_y);
	}
}
#endif

// ----------------------------------------------------------------------------

/// @brief Mix Graphics per one line (512 x 512 mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_one_line(int width, int src_y_base, int dst_y)
{
	// 512 x 512 mode
	if (!(m_show_screen & (VM::Graphic3Mask | VM::Graphic2Mask | VM::Graphic1Mask | VM::Graphic0Mask))) {
		// off
		return;
	}

	switch(m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_CO_MASK) {
	case VC_GS_CO65536_MASK:
		// 65536 colors
		mix_buffer_graphic512_c65536_one_line(width, src_y_base, dst_y);
		break;
	case VC_GS_CO256_MASK:
		// 256 colors
		mix_buffer_graphic512_c256_one_line(width, src_y_base, dst_y);
		break;
	default:
		// 16 colors
		mix_buffer_graphic512_c16_one_line(width, src_y_base, dst_y);
		break;
	}
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics (512 x 512 mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic512(int width, int height9)
{
	// 512 x 512 mode
	if (!(m_show_screen & (VM::Graphic3Mask | VM::Graphic2Mask | VM::Graphic1Mask | VM::Graphic0Mask))) {
		// off
		return;
	}

	switch(m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_CO_MASK) {
	case VC_GS_CO65536_MASK:
		// 65536 colors
		mix_buffer_graphic512_c65536(width, height9);
		break;
	case VC_GS_CO256_MASK:
		// 256 colors
		mix_buffer_graphic512_c256(width, height9);
		break;
	default:
		// 16 colors
		mix_buffer_graphic512_c16(width, height9);
		break;
	}
}
#endif

/// @brief Mix Graphics per one line (512 x 512, 65536 colors, translucent or special mode)
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c65536_one_line_sub_p(int width, int src_left[4], int src_top[4], int src_y[4], int sft[4], uint32_t onoff[4], int dst_y)
{
	int src_x[4];
	src_x[0] = src_left[0];
	src_x[1] = src_left[1];
	src_x[2] = src_left[2];
	src_x[3] = src_left[3];

	uint32_t pal0_num1;
	uint32_t ht_area = 0;
	uint32_t hp_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_HP_MASK) ? 1 : 0;
	uint32_t bp1_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_BP_MASK) ? 1 : 0;
	uint32_t bp0_mask = (~bp1_mask & hp_mask);
	bp1_mask &= hp_mask;
	uint32_t *dst_buf;

	for(int dst_x = 0; dst_x < width; dst_x++) {
		src_x[0] &= 0x1ff;
		src_x[1] &= 0x1ff;
		src_x[2] &= 0x1ff;
		src_x[3] &= 0x1ff;
		dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
		uint32_t pal1_num = (rb_gvram[src_top[3] + src_y[3] + src_x[3]] & onoff[1]);
		pal1_num <<= 4;
		pal1_num |= (rb_gvram[src_top[2] + src_y[2] + src_x[2]] & onoff[0]);
		uint32_t pal0_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
		pal0_num <<= 4;
		pal0_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
		uint32_t pal1_num = ((p_gvram[src_top[3] + src_y[3] + src_x[3]] >> sft[3]) & onoff[1]);
		pal1_num <<= 4;
		pal1_num |= ((p_gvram[src_top[2] + src_y[2] + src_x[2]] >> sft[2]) & onoff[0]);
		uint32_t pal0_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
		pal0_num <<= 4;
		pal0_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif

		int pal0_num_sft = ((1 - (pal0_num & 1)) << 3);	// x8
		int pal1_num_sft = ((1 - (pal1_num & 1)) << 3);	// x8
		pal0_num1 = (pal0_num & 1); 
		pal0_num &= ~1;	// even
		pal1_num |= 1;	// odd

		uint32_t pal0 = (m_palette[pal0_num] >> pal0_num_sft) & 0xff;
		uint32_t pal1 = (m_palette[pal1_num] >> pal1_num_sft) & 0xff;

		ht_area = 0;
		if (pal0_num != 0 && pal0_num1 != 0) {
			// special priority area
			ht_area |= MX_BUF_SP_AREA;
		}
		if (pal0_num != 0 && ((pal0_num1 & bp1_mask) | (pal0 & bp0_mask)) != 0) {
			// translucent area
			ht_area |= MX_BUF_TR_AREA;
		}
//		pal0 &= ~1; // ignore intensity

		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			*dst_buf = (pal1 << 8) | pal0 | ht_area;
			if (pal0_num | pal1_num) {
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
			}
		}

		src_x[0]++;
		src_x[1]++;
		src_x[2]++;
		src_x[3]++;
	}
}
/// @brief Mix Graphics per one line (512 x 512, 65536 colors, normal mode)
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c65536_one_line_sub_n(int width, int src_left[4], int src_top[4], int src_y[4], int sft[4], uint32_t onoff[4], int dst_y)
{
	int src_x[4];
	src_x[0] = src_left[0];
	src_x[1] = src_left[1];
	src_x[2] = src_left[2];
	src_x[3] = src_left[3];
	uint32_t *dst_buf;

	for(int dst_x = 0; dst_x < width; dst_x++) {
		src_x[0] &= 0x1ff;
		src_x[1] &= 0x1ff;
		src_x[2] &= 0x1ff;
		src_x[3] &= 0x1ff;
		dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
		uint32_t pal1_num = (rb_gvram[src_top[3] + src_y[3] + src_x[3]] & onoff[1]);
		pal1_num <<= 4;
		pal1_num |= (rb_gvram[src_top[2] + src_y[2] + src_x[2]] & onoff[0]);
		uint32_t pal0_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
		pal0_num <<= 4;
		pal0_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
		uint32_t pal1_num = ((p_gvram[src_top[3] + src_y[3] + src_x[3]] >> sft[3]) & onoff[1]);
		pal1_num <<= 4;
		pal1_num |= ((p_gvram[src_top[2] + src_y[2] + src_x[2]] >> sft[2]) & onoff[0]);
		uint32_t pal0_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
		pal0_num <<= 4;
		pal0_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif

		int pal0_num_sft = ((1 - (pal0_num & 1)) << 3);	// x8
		int pal1_num_sft = ((1 - (pal1_num & 1)) << 3);	// x8
		pal0_num &= ~1;	// even
		pal1_num |= 1;	// odd

		uint32_t pal0 = (m_palette[pal0_num] >> pal0_num_sft) & 0xff;
		uint32_t pal1 = (m_palette[pal1_num] >> pal1_num_sft) & 0xff;

		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			*dst_buf = (pal1 << 8) | pal0;
			if (pal0_num | pal1_num) {
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
			}
		}

		src_x[0]++;
		src_x[1]++;
		src_x[2]++;
		src_x[3]++;
	}
}
/// @brief Mix Graphics per one line (512 x 512, 65536 colors mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c65536_one_line(int width, int src_y_base, int dst_y)
{
	int dst_y9 = (dst_y << MX_BUF_WIDTH_SFT);
	for(int pri = 0; pri < 4; pri+=4) {
		uint32_t onoff[4];
//		onoff[0] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 0))) ? 0xf : 0;
//		onoff[1] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 1))) ? 0xf : 0;
//		onoff[2] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 2))) ? 0xf : 0;
//		onoff[3] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 3))) ? 0xf : 0;
		onoff[0] = (m_show_screen & (VM::Graphic0Mask << (pri + 0))) ? 0xf : 0;
		onoff[1] = (m_show_screen & (VM::Graphic0Mask << (pri + 1))) ? 0xf : 0;
		onoff[2] = (m_show_screen & (VM::Graphic0Mask << (pri + 2))) ? 0xf : 0;
		onoff[3] = (m_show_screen & (VM::Graphic0Mask << (pri + 3))) ? 0xf : 0;
		if (!(onoff[0] | onoff[1] | onoff[2] | onoff[3])) {
			// off
			continue;
		}
		int page[4];
		page[0] = m_vc_gr_pripage[pri + 0];
		page[1] = m_vc_gr_pripage[pri + 1];
		page[2] = m_vc_gr_pripage[pri + 2];
		page[3] = m_vc_gr_pripage[pri + 3];

		int src_left[4];
		int src_y[4];
		src_left[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[0] * 2];
		src_y[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[0] * 2];
		src_left[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[1] * 2];
		src_y[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[1] * 2];
		src_left[2] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[2] * 2];
		src_y[2] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[2] * 2];
		src_left[3] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[3] * 2];
		src_y[3] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[3] * 2];

		int src_top[4];
		int sft[4];
#ifdef USE_GRAPHIC_RENDER
		src_top[0] = ((pri + 0) << 18);
		src_top[1] = ((pri + 1) << 18);
		src_top[2] = ((pri + 2) << 18);
		src_top[3] = ((pri + 3) << 18);
		sft[0] = 0;
		sft[1] = 0;
		sft[2] = 0;
		sft[3] = 0;
#else
		src_top[0] = 0;
		src_top[1] = 0;
		src_top[2] = 0;
		src_top[3] = 0;
		sft[0] = m_vc_gr_prisfts[pri + 0];
		sft[1] = m_vc_gr_prisfts[pri + 1];
		sft[2] = m_vc_gr_prisfts[pri + 2];
		sft[3] = m_vc_gr_prisfts[pri + 3];
#endif

		src_y[0] += src_y_base;
		src_y[1] += src_y_base;
		src_y[2] += src_y_base;
		src_y[3] += src_y_base;
		src_y[0] <<= 9;
		src_y[1] <<= 9;
		src_y[2] <<= 9;
		src_y[3] <<= 9;
		src_y[0] &= (0x1ff << 9);
		src_y[1] &= (0x1ff << 9);
		src_y[2] &= (0x1ff << 9);
		src_y[3] &= (0x1ff << 9);

		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			mix_buffer_graphic512_c65536_one_line_sub_p(width, src_left, src_top, src_y, sft, onoff, dst_y9);
		} else {
			mix_buffer_graphic512_c65536_one_line_sub_n(width, src_left, src_top, src_y, sft, onoff, dst_y9);
		}
	}
	set_mix_buffer_graphic_flags(width, dst_y9);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics (512 x 512, 65536 colors mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic512_c65536(int width, int height9)
{
	for(int pri = 0; pri < 4; pri+=4) {
		uint32_t onoff[4];
//		onoff[0] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 0))) ? 0xf : 0;
//		onoff[1] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 1))) ? 0xf : 0;
//		onoff[2] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 2))) ? 0xf : 0;
//		onoff[3] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 3))) ? 0xf : 0;
		onoff[0] = (m_show_screen & (VM::Graphic0Mask << (pri + 0))) ? 0xf : 0;
		onoff[1] = (m_show_screen & (VM::Graphic0Mask << (pri + 1))) ? 0xf : 0;
		onoff[2] = (m_show_screen & (VM::Graphic0Mask << (pri + 2))) ? 0xf : 0;
		onoff[3] = (m_show_screen & (VM::Graphic0Mask << (pri + 3))) ? 0xf : 0;
		if (!(onoff[0] | onoff[1] | onoff[2] | onoff[3])) {
			// off
			continue;
		}
		int page[4];
		page[0] = m_vc_gr_pripage[pri + 0];
		page[1] = m_vc_gr_pripage[pri + 1];
		page[2] = m_vc_gr_pripage[pri + 2];
		page[3] = m_vc_gr_pripage[pri + 3];

		int src_left[4];
		int src_y[4];
		src_left[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[0] * 2];
		src_y[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[0] * 2];
		src_left[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[1] * 2];
		src_y[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[1] * 2];
		src_left[2] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[2] * 2];
		src_y[2] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[2] * 2];
		src_left[3] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[3] * 2];
		src_y[3] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[3] * 2];

		int src_top[4];
		int sft[4];
#ifdef USE_GRAPHIC_RENDER
		src_top[0] = ((pri + 0) << 18);
		src_top[1] = ((pri + 1) << 18);
		src_top[2] = ((pri + 2) << 18);
		src_top[3] = ((pri + 3) << 18);
		sft[0] = 0;
		sft[1] = 0;
		sft[2] = 0;
		sft[3] = 0;
#else
		src_top[0] = 0;
		src_top[1] = 0;
		src_top[2] = 0;
		src_top[3] = 0;
		sft[0] = m_vc_gr_prisfts[pri + 0];
		sft[1] = m_vc_gr_prisfts[pri + 1];
		sft[2] = m_vc_gr_prisfts[pri + 2];
		sft[3] = m_vc_gr_prisfts[pri + 3];
#endif

		src_y[0] <<= 9;
		src_y[1] <<= 9;
		src_y[2] <<= 9;
		src_y[3] <<= 9;
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y[0] &= (0x1ff << 9);
				src_y[1] &= (0x1ff << 9);
				src_y[2] &= (0x1ff << 9);
				src_y[3] &= (0x1ff << 9);

				mix_buffer_graphic512_c65536_one_line_sub_p(width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y[0]+=(1 << 9);	// x512
				src_y[1]+=(1 << 9);	// x512
				src_y[2]+=(1 << 9);	// x512
				src_y[3]+=(1 << 9);	// x512
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y[0] &= (0x1ff << 9);
				src_y[1] &= (0x1ff << 9);
				src_y[2] &= (0x1ff << 9);
				src_y[3] &= (0x1ff << 9);

				mix_buffer_graphic512_c65536_one_line_sub_n(width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y[0]+=(1 << 9);	// x512
				src_y[1]+=(1 << 9);	// x512
				src_y[2]+=(1 << 9);	// x512
				src_y[3]+=(1 << 9);	// x512
			}
		}
	}
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		set_mix_buffer_graphic_flags(width, dst_y);
	}
}
#endif

#define IS_NOT_TRANS(x) (x & 0xff)

/// @brief Mix Graphics per one line (512 x 512, 256 colors, translucent or special (graphic x graphic) mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c256_one_line_sub_s(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y)
{
	int src_x[2];
	src_x[0] = src_left[0];
	src_x[1] = src_left[1];

	uint32_t pal_num1;
	uint32_t pal = 0;
	uint32_t ht_area = 0;
	uint32_t hp_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_HP_MASK) ? 1 : 0;
	uint32_t bp1_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_BP_MASK) ? 1 : 0;
	uint32_t bp0_mask = (~bp1_mask & hp_mask);
	bp1_mask &= hp_mask;
	uint32_t *dst_buf;

	switch (pri_num) {
	case 0:
		// 1st (base) page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x[0] &= 0x1ff;
			src_x[1] &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
			pal_num <<= 4;
			pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
			uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
			pal_num <<= 4;
			pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif

			pal_num1 = (pal_num & 1); 
			pal_num &= ~1;	// even palette only
			pal = m_palette[pal_num];
			ht_area = 0;
			if (pal_num != 0 && pal_num1 != 0) {
				// special priority area
				ht_area |= MX_BUF_SP_AREA;
			}
			if (pal_num != 0 && ((pal_num1 & bp1_mask) | (pal & bp0_mask)) != 0) {
				// translucent area
				ht_area |= MX_BUF_TR_AREA;
			}
			pal &= ~1; // ignore intensity

			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal | ht_area;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x[0]++;
			src_x[1]++;
		}
		break;

	case 1:
		// 2nd page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x[0] &= 0x1ff;
			src_x[1] &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
			pal_num <<= 4;
			pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
			uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
			pal_num <<= 4;
			pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif
			// add each color
			if (*dst_buf & MX_BUF_TR_AREA) {
				// translucent area, so mix color
				pal = m_palette[pal_num | hp_mask];	// odd palette number
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = (*dst_buf & MX_BUF_NO_TRANS) | pal | MX_BUF_TR_AREA;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			} else if (*dst_buf & MX_BUF_SP_AREA) {
				// special priority area, so hide first priority
				pal = m_palette[pal_num | hp_mask];	// odd palette number
				*dst_buf = (*dst_buf & MX_BUF_NO_TRANS) | pal | MX_BUF_SP_AREA;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			} else if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				// transparent area
				pal = m_palette[pal_num & ~hp_mask];	// even palette number
				*dst_buf = pal;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x[0]++;
			src_x[1]++;
		}
		break;

	default:
		// other pages
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x[0] &= 0x1ff;
			src_x[1] &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
			pal_num <<= 4;
			pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
			uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
			pal_num <<= 4;
			pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif
			// expand palette
			pal = m_palette[pal_num & ~1]; // even palette only
			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x[0]++;
			src_x[1]++;
		}
		break;

	}
}
/// @brief Mix Graphics per one line (512 x 512, 256 colors, translucent or special (graphic x other) mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c256_one_line_sub_p(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y)
{
	int src_x[2];
	src_x[0] = src_left[0];
	src_x[1] = src_left[1];

	uint32_t pal_num1;
	uint32_t pal = 0;
	uint32_t ht_area = 0;
	uint32_t hp_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_HP_MASK) ? 1 : 0;
	uint32_t bp1_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_BP_MASK) ? 1 : 0;
	uint32_t bp0_mask = (~bp1_mask & hp_mask);
	bp1_mask &= hp_mask;
	uint32_t *dst_buf;

	switch (pri_num) {
	case 0:
		// 1st (base) page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x[0] &= 0x1ff;
			src_x[1] &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
			pal_num <<= 4;
			pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
			uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
			pal_num <<= 4;
			pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif

			pal_num1 = (pal_num & 1); 
			pal_num &= ~1;	// even palette only
			pal = m_palette[pal_num];
			ht_area = 0;
			if (pal_num != 0 && pal_num1 != 0) {
				// special priority area
				ht_area |= MX_BUF_SP_AREA;
			}
			if (pal_num != 0 && ((pal_num1 & bp1_mask) | (pal & bp0_mask)) != 0) {
				// translucent area
				ht_area |= MX_BUF_TR_AREA;
			}
			pal &= ~1; // ignore intensity

			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal | ht_area;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x[0]++;
			src_x[1]++;
		}
		break;

	default:
		// other pages
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x[0] &= 0x1ff;
			src_x[1] &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
			pal_num <<= 4;
			pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
			uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
			pal_num <<= 4;
			pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif
			// expand palette
			pal = m_palette[pal_num & ~1]; // even palette only
			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal;
				if (IS_NOT_TRANS(pal_num)) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x[0]++;
			src_x[1]++;
		}
		break;

	}
}
/// @brief Mix Graphics per one line (512 x 512, 256 colors, normal mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c256_one_line_sub_n(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y)
{
	int src_x[2];
	src_x[0] = src_left[0];
	src_x[1] = src_left[1];
	uint32_t *dst_buf;

	for(int dst_x = 0; dst_x < width; dst_x++) {
		src_x[0] &= 0x1ff;
		src_x[1] &= 0x1ff;
		dst_buf = &mx_buf[dst_y + dst_x];

#ifdef USE_GRAPHIC_RENDER
		uint32_t pal_num = (rb_gvram[src_top[1] + src_y[1] + src_x[1]] & onoff[1]);
		pal_num <<= 4;
		pal_num |= (rb_gvram[src_top[0] + src_y[0] + src_x[0]] & onoff[0]);
#else
		uint32_t pal_num = ((p_gvram[src_top[1] + src_y[1] + src_x[1]] >> sft[1]) & onoff[1]);
		pal_num <<= 4;
		pal_num |= ((p_gvram[src_top[0] + src_y[0] + src_x[0]] >> sft[0]) & onoff[0]);
#endif

		uint32_t pal = m_palette[pal_num];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			*dst_buf = pal;
			if (IS_NOT_TRANS(pal_num)) {
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
			}
		}
		src_x[0]++;
		src_x[1]++;
	}
}
/// @brief Mix Graphics per one line (512 x 512, 256 colors mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c256_one_line(int width, int src_y_base, int dst_y)
{
	int dst_y9 = (dst_y << MX_BUF_WIDTH_SFT);
	for(int pri = 0, pri_num = 0; pri < 4; pri+=2, pri_num++) {
		uint32_t onoff[2];
//		onoff[0] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 0))) ? 0xf : 0;
//		onoff[1] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 1))) ? 0xf : 0;
		onoff[0] = (m_show_screen & (VM::Graphic0Mask << (pri + 0))) ? 0xf : 0;
		onoff[1] = (m_show_screen & (VM::Graphic0Mask << (pri + 1))) ? 0xf : 0;
		if (!(onoff[0] | onoff[1])) {
			// off
			continue;
		}
		int page[2];
		page[0] = m_vc_gr_pripage[pri + 0];
		page[1] = m_vc_gr_pripage[pri + 1];

		int src_left[2];
		int src_y[2];
		src_left[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[0] * 2];
		src_y[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[0] * 2];
		src_left[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[1] * 2];
		src_y[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[1] * 2];

		int src_top[2];
		int sft[2];
#ifdef USE_GRAPHIC_RENDER
		src_top[0] = ((pri + 0) << 18);
		src_top[1] = ((pri + 1) << 18);
		sft[0] = 0;
		sft[1] = 0;
#else
		src_top[0] = 0;
		src_top[1] = 0;
		sft[0] = m_vc_gr_prisfts[pri + 0];
		sft[1] = m_vc_gr_prisfts[pri + 1];
#endif

		src_y[0] += src_y_base;
		src_y[1] += src_y_base;
		src_y[0] <<= 9;
		src_y[1] <<= 9;
		src_y[0] &= (0x1ff << 9);
		src_y[1] &= (0x1ff << 9);
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GG_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GG_MASK)) {
			// translucent mode
			// graphic x graphic
			mix_buffer_graphic512_c256_one_line_sub_s(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			// graphic x other
			mix_buffer_graphic512_c256_one_line_sub_p(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		} else {
			// normal mode
			mix_buffer_graphic512_c256_one_line_sub_n(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		}
	}
	set_mix_buffer_graphic_flags(width, dst_y9);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics (512 x 512, 256 colors mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic512_c256(int width, int height9)
{
	for(int pri = 0, pri_num = 0; pri < 4; pri+=2, pri_num++) {
		uint32_t onoff[2];
//		onoff[0] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 0))) ? 0xf : 0;
//		onoff[1] = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << (pri + 1))) ? 0xf : 0;
		onoff[0] = (m_show_screen & (VM::Graphic0Mask << (pri + 0))) ? 0xf : 0;
		onoff[1] = (m_show_screen & (VM::Graphic0Mask << (pri + 1))) ? 0xf : 0;
		if (!(onoff[0] | onoff[1])) {
			// off
			continue;
		}
		int page[2];
		page[0] = m_vc_gr_pripage[pri + 0];
		page[1] = m_vc_gr_pripage[pri + 1];

		int src_left[2];
		int src_y[2];
		src_left[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[0] * 2];
		src_y[0] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[0] * 2];
		src_left[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page[1] * 2];
		src_y[1] = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page[1] * 2];

		int src_top[2];
		int sft[2];
#ifdef USE_GRAPHIC_RENDER
		src_top[0] = ((pri + 0) << 18);
		src_top[1] = ((pri + 1) << 18);
		sft[0] = 0;
		sft[1] = 0;
#else
		src_top[0] = 0;
		src_top[1] = 0;
		sft[0] = m_vc_gr_prisfts[pri + 0];
		sft[1] = m_vc_gr_prisfts[pri + 1];
#endif

		src_y[0] <<= 9;
		src_y[1] <<= 9;
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_GG_MASK)) == (VC_TO_EO_MASK | VC_TO_GG_MASK)) {
			// translucent / special mode
			// graphic x graphic
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y[0] &= (0x1ff << 9);
				src_y[1] &= (0x1ff << 9);
				mix_buffer_graphic512_c256_one_line_sub_s(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y[0]+=(1 << 9);	// x512
				src_y[1]+=(1 << 9);	// x512
			}
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			// graphic x other
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y[0] &= (0x1ff << 9);
				src_y[1] &= (0x1ff << 9);
				mix_buffer_graphic512_c256_one_line_sub_p(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y[0]+=(1 << 9);	// x512
				src_y[1]+=(1 << 9);	// x512
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y[0] &= (0x1ff << 9);
				src_y[1] &= (0x1ff << 9);
				mix_buffer_graphic512_c256_one_line_sub_n(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y[0]+=(1 << 9);	// x512
				src_y[1]+=(1 << 9);	// x512
			}
		}
	}
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		set_mix_buffer_graphic_flags(width, dst_y);
	}
}
#endif

/// @brief Mix Graphics per one line (512 x 512, 16 colors, translucent or special (graphic x graphic) mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c16_one_line_sub_s(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y)
{
	int src_x = src_left;

	uint32_t pal_num1;
	uint32_t pal = 0;
	uint32_t ht_area = 0;
	uint32_t hp_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_HP_MASK) ? 1 : 0;
	uint32_t bp1_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_BP_MASK) ? 1 : 0;
	uint32_t bp0_mask = (~bp1_mask & hp_mask);
	bp1_mask &= hp_mask;
	uint32_t *dst_buf;

	switch (pri_num) {
	case 0:
		// 1st (base) page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
			uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif

			pal_num1 = (pal_num & 1); 
			pal_num &= ~1;	// even palette only
			pal = m_palette[pal_num];
			ht_area = 0;
			if (pal_num != 0 && pal_num1 != 0) {
				// special priority area
				ht_area |= MX_BUF_SP_AREA;
			}
			if (pal_num != 0 && ((pal_num1 & bp1_mask) | (pal & bp0_mask)) != 0) {
				// translucent area
				ht_area |= MX_BUF_TR_AREA;
			}
			pal &= ~1; // ignore intensity

			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal | ht_area;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}

			src_x++;
		}
		break;

	case 1:
		// 2nd page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
			uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif
			// add each color
			if (*dst_buf & MX_BUF_TR_AREA) {
				// translucent area, so mix color
				pal = m_palette[pal_num | hp_mask];	// odd palette number
				pal = mix_translucent_color16(*dst_buf, pal);
				*dst_buf = (*dst_buf & MX_BUF_NO_TRANS) | pal | MX_BUF_TR_AREA;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			} else if (*dst_buf & MX_BUF_SP_AREA) {
				// special priority area, so hide first priority
				pal = m_palette[pal_num | hp_mask];	// odd palette number
				*dst_buf = (*dst_buf & MX_BUF_NO_TRANS) | pal | MX_BUF_SP_AREA;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			} else if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				// transparent area
				pal = m_palette[pal_num & ~hp_mask]; // even palette number
				*dst_buf = pal;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
			src_x++;
		}
		break;

	default:
		// other pages
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
			uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif
			// expand palette
			pal = m_palette[pal_num & ~1]; // even palette only
			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
			src_x++;
		}
		break;

	}
}
/// @brief Mix Graphics per one line (512 x 512, 16 colors, translucent or special (graphic x other) mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c16_one_line_sub_p(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y)
{
	int src_x = src_left;

	uint32_t pal_num1;
	uint32_t pal = 0;
	uint32_t ht_area = 0;
	uint32_t hp_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_HP_MASK) ? 1 : 0;
	uint32_t bp1_mask = (m_vc_regs[VC_TRANS_ONOFF] & VC_TO_BP_MASK) ? 1 : 0;
	uint32_t bp0_mask = (~bp1_mask & hp_mask);
	bp1_mask &= hp_mask;
	uint32_t *dst_buf;

	switch (pri_num) {
	case 0:
		// 1st (base) page
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
			uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif

			pal_num1 = (pal_num & 1); 
			pal_num &= ~1;	// even palette only
			pal = m_palette[pal_num];
			ht_area = 0;
			if (pal_num != 0 && pal_num1 != 0) {
				// special priority area
				ht_area |= MX_BUF_SP_AREA;
			}
			if (pal_num != 0 && ((pal_num1 & bp1_mask) | (pal & bp0_mask)) != 0) {
				// translucent area
				ht_area |= MX_BUF_TR_AREA;
			}
			pal &= ~1; // ignore intensity

			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal | ht_area;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
			src_x++;
		}
		break;

	default:
		// other pages
		for(int dst_x = 0; dst_x < width; dst_x++) {
			src_x &= 0x1ff;
			dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
			uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
			uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif
			// expand palette
			pal = m_palette[pal_num & ~1]; // even palette only
			if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
				*dst_buf = pal;
				if (pal_num) {
					*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
				}
			}
			src_x++;
		}
		break;

	}
}
/// @brief Mix Graphics per one line (512 x 512, 16 colors, normal mode)
/// @param[in] pri_num
/// @param[in] width
/// @param[in] src_left
/// @param[in] src_top
/// @param[in] src_y
/// @param[in] sft : 0:GR0 4:GR1 8:GR2 12:GR3
/// @param[in] onoff : 0xf:on 0x0:off
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c16_one_line_sub_n(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y)
{
	int src_x = src_left;
	uint32_t *dst_buf;

	for(int dst_x = 0; dst_x < width; dst_x++) {
		src_x &= 0x1ff;
		dst_buf = &mx_buf[dst_y + dst_x];
#ifdef USE_GRAPHIC_RENDER
		uint32_t pal_num = rb_gvram[src_top + src_y + src_x] & onoff;
#else
		uint32_t pal_num = (p_gvram[src_top + src_y + src_x] >> sft) & onoff;
#endif
		// expand palette
		uint32_t pal = m_palette[pal_num];
		if ((*dst_buf & MX_BUF_NO_TRANS) == 0) {
			*dst_buf = pal;
			if (pal_num) {
				*dst_buf |= MX_BUF_NO_TRANS;	// no transparent
			}
		}
		src_x++;
	}
}
/// @brief Mix Graphics per one line (512 x 512, 16 colors mode)
/// @param[in] width
/// @param[in] src_y_base
/// @param[in] dst_y
void DISPLAY::mix_buffer_graphic512_c16_one_line(int width, int src_y_base, int dst_y)
{
	int dst_y9 = (dst_y << MX_BUF_WIDTH_SFT);
	for(int pri = 0, pri_num = 0; pri < 4; pri++, pri_num++) {
//		uint32_t onoff = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << pri)) ? 0xf : 0;
		uint32_t onoff = (m_show_screen & (VM::Graphic0Mask << pri)) ? 0xf : 0;
		if (!(onoff)) {
			// off
			continue;
		}
		int page = m_vc_gr_pripage[pri];

		int src_left = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page * 2];
		int src_y = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page * 2];

#ifdef USE_GRAPHIC_RENDER
		int src_top = (pri << 18);
		int sft = 0;
#else
		int src_top = 0;
		int sft = m_vc_gr_prisfts[pri];
#endif
		
		src_y += src_y_base;
		src_y <<= 9;
		src_y &= (0x1ff << 9);
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GG_MASK)) == (VC_TO_EO_MASK | VC_TO_HP_MASK | VC_TO_GG_MASK)) {
			// translucent mode
			// graphic x graphic
			mix_buffer_graphic512_c16_one_line_sub_s(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			// graphic x other
			mix_buffer_graphic512_c16_one_line_sub_p(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		} else {
			// normal mode
			mix_buffer_graphic512_c16_one_line_sub_n(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y9);
		}
	}
	set_mix_buffer_graphic_flags(width, dst_y9);
}

#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
/// @brief Mix Graphics (512 x 512, 16 colors mode)
/// @param[in] width
/// @param[in] height9
void DISPLAY::mix_buffer_graphic512_c16(int width, int height9)
{
	for(int pri = 0, pri_num = 0; pri < 4; pri++, pri_num++) {
//		uint32_t onoff = (m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_G0ON_MASK << pri)) ? 0xf : 0;
		uint32_t onoff = (m_show_screen & (VM::Graphic0Mask << pri)) ? 0xf : 0;
		if (!(onoff)) {
			// off
			continue;
		}
		int page = m_vc_gr_pripage[pri];

		int src_left = p_crtc_regs[CRTC::CRTC_GSCROLL_X_0 + page * 2];
		int src_y = p_crtc_regs[CRTC::CRTC_GSCROLL_Y_0 + page * 2];

#ifdef USE_GRAPHIC_RENDER
		int src_top = (pri << 18);
		int sft = 0;
#else
		int src_top = 0;
		int sft = m_vc_gr_prisfts[pri];
#endif

		src_y <<= 9;
		if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK | VC_TO_GG_MASK)) == (VC_TO_EO_MASK | VC_TO_GG_MASK)) {
			// translucent / special mode
			// graphic x graphic
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y &= (0x1ff << 9);
				mix_buffer_graphic512_c16_one_line_sub_s(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y+=(1 << 9);	// x512
			}
		} else if ((m_vc_regs[VC_TRANS_ONOFF] & (VC_TO_AH_MASK | VC_TO_EO_MASK)) == VC_TO_EO_MASK) {
			// translucent / special mode
			// graphic x other
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y &= (0x1ff << 9);
				mix_buffer_graphic512_c16_one_line_sub_p(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y+=(1 << 9);	// x512
			}
		} else {
			// normal mode
			for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
				src_y &= (0x1ff << 9);
				mix_buffer_graphic512_c16_one_line_sub_n(pri_num, width, src_left, src_top, src_y, sft, onoff, dst_y);
				src_y+=(1 << 9);	// x512
			}
		}
	}
	for(int dst_y = 0; dst_y < height9; dst_y+=(1 << MX_BUF_WIDTH_SFT)) {
		set_mix_buffer_graphic_flags(width, dst_y);
	}
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void DISPLAY::debug_expand_graphic_render(int type, int width, int height, scrntype *buffer)
{
	uint32_t src = 0;
	scrntype *dst = buffer;

#ifdef USE_GRAPHIC_RENDER

	switch(type) {
	case 0:
		// 512x512 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				*dst = debug_expand_palette(pal0);
				dst++;
				src++;
			}
		}
		break;
	case 1:
		// 512x512 256 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				uint32_t pal1 = rb_gvram[src + 0x40000];
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = debug_expand_palette(pal0);
				dst++;
				src++;
			}
			if (src == 0x40000) src += 0x40000;
		}
		break;
	case 2:
		// 512x512 65536 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				uint32_t pal1 = rb_gvram[src + 0x40000];
				uint32_t pal2 = rb_gvram[src + 0x80000];
				uint32_t pal3 = rb_gvram[src + 0xc0000];
				pal3 <<= 4;
				pal2 |= pal3;
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = debug_expand_palette65536(pal0, pal2);
				dst++;
				src++;
			}
		}
		break;
	case 3:
		// 1024x1024 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal = rb_gvram[src];
				*dst = debug_expand_palette(pal);
				dst++;
				src++;
			}
		}
		break;
	}

#else
	int pri = 0;
	switch(type) {
	case 0:
		// 512x512 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = debug_expand_palette(pal0);
				dst++;
				src++;
			}
			if (src >= 0x40000) {
				pri++;
				src = 0;
			}
		}
		break;
	case 1:
		// 512x512 256 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[pri + 0]) & 0xf;
				uint32_t pal1 = (p_gvram[src] >> m_vc_gr_prisfts[pri + 1]) & 0xf;
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = debug_expand_palette(pal0);
				dst++;
				src++;
			}
			if (src >= 0x40000) {
				pri += 2;
				src = 0;
			}
		}
		break;
	case 2:
		// 512x512 65536 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[0]) & 0xf;
				uint32_t pal1 = (p_gvram[src] >> m_vc_gr_prisfts[1]) & 0xf;
				uint32_t pal2 = (p_gvram[src] >> m_vc_gr_prisfts[2]) & 0xf;
				uint32_t pal3 = (p_gvram[src] >> m_vc_gr_prisfts[3]) & 0xf;
				pal3 <<= 4;
				pal2 |= pal3;
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = debug_expand_palette65536(pal0, pal2);
				dst++;
				src++;
			}
		}
		break;
	case 3:
		// 1024x1024 16 colors
#ifndef USE_GRAPHIC_1024ALT
		for(int y = 0; y < height; y++) {
			src = (y << 9);
			pri = ((y & 0x200) >> 8);
			for(int x = 0; x < width; x++) {
				if (x == 512) {
					src -= 512;
					pri++;
				}
				uint32_t pal = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = debug_expand_palette(pal);
				dst++;
				src++;
			}
		}
#else
		for(int y = 0; y < height; y++) {
			pri = ((y & 0x300) >> 8);
			src = ((y & 0xff) * 1024);
			for(int x = 0; x < width; x++) {
				uint32_t pal = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = debug_expand_palette(pal);
				dst++;
				src++;
			}
		}
#endif
		break;
	}

#endif
}

void DISPLAY::debug_expand_graphic_dumper(int type, int width, int height, uint16_t *buffer)
{
	uint32_t src = 0;
	uint16_t *dst = buffer;

#ifdef USE_GRAPHIC_RENDER

	switch(type) {
	case 0:
		// 512x512 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
		}
		break;
	case 1:
		// 512x512 256 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				uint32_t pal1 = rb_gvram[src + 0x40000];
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
			if (src == 0x40000) src += 0x40000;
		}
		break;
	case 2:
		// 512x512 65536 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = rb_gvram[src];
				uint32_t pal1 = rb_gvram[src + 0x40000];
				uint32_t pal2 = rb_gvram[src + 0x80000];
				uint32_t pal3 = rb_gvram[src + 0xc0000];
				pal3 <<= 4;
				pal2 |= pal3;
				pal1 <<= 4;
				pal0 |= pal1;
				pal0 |= (pal2 << 8);
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
		}
		break;
	case 3:
		// 1024x1024 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal = rb_gvram[src];
				*dst = (uint16_t)(pal);
				dst++;
				src++;
			}
		}
		break;
	}

#else
	int pri = 0;
	switch(type) {
	case 0:
		// 512x512 16 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
			if (src >= 0x40000) {
				pri++;
				src = 0;
			}
		}
		break;
	case 1:
		// 512x512 256 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[pri + 0]) & 0xf;
				uint32_t pal1 = (p_gvram[src] >> m_vc_gr_prisfts[pri + 1]) & 0xf;
				pal1 <<= 4;
				pal0 |= pal1;
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
			if (src >= 0x40000) {
				pri += 2;
				src = 0;
			}
		}
		break;
	case 2:
		// 512x512 65536 colors
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				uint32_t pal0 = (p_gvram[src] >> m_vc_gr_prisfts[0]) & 0xf;
				uint32_t pal1 = (p_gvram[src] >> m_vc_gr_prisfts[1]) & 0xf;
				uint32_t pal2 = (p_gvram[src] >> m_vc_gr_prisfts[2]) & 0xf;
				uint32_t pal3 = (p_gvram[src] >> m_vc_gr_prisfts[3]) & 0xf;
				pal3 <<= 4;
				pal2 |= pal3;
				pal1 <<= 4;
				pal0 |= pal1;
				pal0 |= (pal2 << 8);
				*dst = (uint16_t)(pal0);
				dst++;
				src++;
			}
		}
		break;
	case 3:
		// 1024x1024 16 colors
#ifndef USE_GRAPHIC_1024ALT
		for(int y = 0; y < height; y++) {
			src = (y << 9);
			pri = ((y & 0x200) >> 8);
			for(int x = 0; x < width; x++) {
				if (x == 512) {
					src -= 512;
					pri++;
				}
				uint32_t pal = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = (uint16_t)(pal);
				dst++;
				src++;
			}
		}
#else
		for(int y = 0; y < height; y++) {
			pri = ((y & 0x300) >> 8);
			src = ((y & 0xff) * 1024);
			for(int x = 0; x < width; x++) {
				uint32_t pal = (p_gvram[src] >> m_vc_gr_prisfts[pri]) & 0xf;
				*dst = (uint16_t)(pal);
				dst++;
				src++;
			}
		}
#endif
		break;
	}

#endif
}
#endif /* USE_DEBUGGER */

// ----------------------------------------------------------------------------

#ifdef USE_GRAPHIC_RENDER
/// render to graphic buffer from graphic VRAM
void DISPLAY::render_graphic_one_line(int y)
{
	if (m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_SZ_MASK) {
		// 1024 x 1024 mode
		if (!(m_vc_regs[VC_TRANS_ONOFF] & VC_TO_G4ON_MASK)) {
			// off
			return;
		}
		render_graphic1024_one_line(y);

	} else {
		// 512 x 512 mode
		if (!(m_vc_regs[VC_TRANS_ONOFF] & VC_TO_GRON_MASK)) {
			// off
			return;
		}
		render_graphic512_one_line(y);

	}
}

/// render to graphic buffer from graphic VRAM
void DISPLAY::render_graphic()
{
	if (m_vc_regs[VC_GRAPHIC_SIZE] & VC_GS_SZ_MASK) {
		// 1024 x 1024 mode
		if (!(m_vc_regs[VC_TRANS_ONOFF] & VC_TO_G4ON_MASK)) {
			// off
			return;
		}
		render_graphic1024();

	} else {
		// 512 x 512 mode
		if (!(m_vc_regs[VC_TRANS_ONOFF] & VC_TO_GRON_MASK)) {
			// off
			return;
		}
		render_graphic512();

	}
}

/// @param[in] need_update : need update render buffer?
/// @param[in] y: 0 - 511
void DISPLAY::render_graphic1024_one_line_sub(uint32_t need_update, int y)
{
	uint32_t src = (y << 9);
	uint32_t dst = (y << 10);
	for(int x = 0; x < 512; x++) {
		if (need_update | pu_gvram[src]) {
			pu_gvram[src] = 0;
			uint32_t data = p_gvram[src];
			rb_gvram[dst | 0x00000] = ((data >> m_vc_gr_prisfts[0]) & 0xf); // priority0
			rb_gvram[dst | 0x00200] = ((data >> m_vc_gr_prisfts[1]) & 0xf); // priority1
			rb_gvram[dst | 0x80000] = ((data >> m_vc_gr_prisfts[2]) & 0xf); // priority2
			rb_gvram[dst | 0x80200] = ((data >> m_vc_gr_prisfts[3]) & 0xf); // priority3
		}
		dst++;
	}
}

/// @param[in] y: 0 - 511
void DISPLAY::render_graphic1024_one_line(int y)
{
	// 1024 x 1024 mode
	uint32_t vc_regs = (m_vc_regs[VC_GRAPHIC_SIZE] | (m_vc_regs[VC_PRIORITY] << 3) | (m_vc_regs[VC_TRANS_ONOFF] << 13));
	uint32_t need_update = (m_vc_prev[y] ^ vc_regs);
	m_vc_prev[y] = vc_regs;

	uint16_t crtc_regs = p_crtc_regs[CRTC::CRTC_CONTROL0];
	need_update |= (m_crtc_prev[y] ^ crtc_regs);
	m_crtc_prev[y] = crtc_regs;

	render_graphic1024_one_line_sub(need_update, y);
}

/// render buffer has 4 pages.
/// priority0 is 0x00000 - 0x001ff, 0x00400-0x005ff, ...
/// priority1 is 0x00200 - 0x003ff, 0x00600-0x007ff, ...
/// priority2 is 0x80000 - 0x801ff, 0x80400-0x805ff, ...
/// priority3 is 0x80200 - 0x803ff, 0x80600-0x007ff, ...
void DISPLAY::render_graphic1024()
{
	// 1024 x 1024 mode
	uint32_t vc_regs = (m_vc_regs[VC_GRAPHIC_SIZE] | (m_vc_regs[VC_PRIORITY] << 3) | (m_vc_regs[VC_TRANS_ONOFF] << 13));
	uint32_t need_update = (m_vc_prev[0] ^ vc_regs);
	m_vc_prev[0] = vc_regs;

	uint16_t crtc_regs = p_crtc_regs[CRTC::CRTC_CONTROL0];
	need_update |= (m_crtc_prev[0] ^ crtc_regs);
	m_crtc_prev[0] = crtc_regs;

	for(int y = 0; y < 512; y++) {
		render_graphic1024_one_line_sub(need_update, y);
	}
}

/// @param[in] need_update : need update render buffer?
/// @param[in] y: 0 - 511
void DISPLAY::render_graphic512_one_line_sub(uint32_t need_update, int y)
{
	uint32_t src = (y << 9);
	uint32_t dst = (y << 9);
	for(int x = 0; x < 512; x++) {
		if (need_update | pu_gvram[src]) {
			pu_gvram[src] = 0;
			uint32_t data = p_gvram[src];
			rb_gvram[dst | 0x00000] = ((data >> m_vc_gr_prisfts[0]) & 0xf); // priority0
			rb_gvram[dst | 0x40000] = ((data >> m_vc_gr_prisfts[1]) & 0xf); // priority1
			rb_gvram[dst | 0x80000] = ((data >> m_vc_gr_prisfts[2]) & 0xf); // priority2
			rb_gvram[dst | 0xc0000] = ((data >> m_vc_gr_prisfts[3]) & 0xf); // priority3
		}
		src++;
		dst++;
	}
}

/// @param[in] y: 0 - 511
void DISPLAY::render_graphic512_one_line(int y)
{
	// 512 x 512 mode
	uint32_t vc_regs = (m_vc_regs[VC_GRAPHIC_SIZE] | (m_vc_regs[VC_PRIORITY] << 3) | (m_vc_regs[VC_TRANS_ONOFF] << 13));
	uint32_t need_update = (m_vc_prev[y] ^ vc_regs);
	m_vc_prev[y] = vc_regs;

	uint16_t crtc_regs = p_crtc_regs[CRTC::CRTC_CONTROL0];
	need_update |= (m_crtc_prev[y] ^ crtc_regs);
	m_crtc_prev[y] = crtc_regs;

	render_graphic512_one_line_sub(need_update, y);
}

/// render buffer has 4 pages.
/// priority0 starts at 0x00000
/// priority1 starts at 0x40000
/// priority2 starts at 0x80000
/// priority3 starts at 0xc0000
void DISPLAY::render_graphic512()
{
	// 512 x 512 mode
	uint32_t vc_regs = (m_vc_regs[VC_GRAPHIC_SIZE] | (m_vc_regs[VC_PRIORITY] << 3) | (m_vc_regs[VC_TRANS_ONOFF] << 13));
	uint32_t need_update = (m_vc_prev[0] ^ vc_regs);
	m_vc_prev[0] = vc_regs;

	uint16_t crtc_regs = p_crtc_regs[CRTC::CRTC_CONTROL0];
	need_update |= (m_crtc_prev[0] ^ crtc_regs);
	m_crtc_prev[0] = crtc_regs;

	for(int y = 0; y < 512; y++) {
		render_graphic512_one_line_sub(need_update, y);
	}
}
#endif

// ----------------------------------------------------------------------------

/// @brief Draw border area in screen
/// @param[in] y_even_odd
/// @param[in] y_step
void DISPLAY::draw_screen_frame(int y_even_odd, int y_step)
{
	uint32_t dot = m_border_color;

	// if scanline mode
	int y_top;
	int y_bottom;
	int y_disp_top;
	if (y_step <= 1) {
		y_top = 0;
		y_bottom = mv_display_bottom;
		y_disp_top = mv_display_top;
	} else {
		y_top = y_even_odd;
		y_bottom = (mv_display_bottom & ~1) | y_even_odd;
		y_disp_top = (mv_display_top & ~1) | y_even_odd;
	}

	// around disp area (front and back porch)
	for(dws_y = y_top; dws_y < mv_display_top; dws_y+=y_step) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y;
		for(dws_x = 0; dws_x < SCREEN_WIDTH; dws_x++) {
			*dws_scrn = dot;
			dws_scrn++;
		}
	}
	for(dws_y = y_bottom; dws_y < SCREEN_HEIGHT; dws_y+=y_step) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y;
		for(dws_x = 0; dws_x < SCREEN_WIDTH; dws_x++) {
			*dws_scrn = dot;
			dws_scrn++;
		}
	}
	for(dws_y = y_disp_top; dws_y < mv_display_bottom; dws_y+=y_step) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y;
		for(dws_x = 0; dws_x < mv_display_left; dws_x++) {
			*dws_scrn = dot;
			dws_scrn++;
		}
	}
	for(dws_y = y_disp_top; dws_y < mv_display_bottom; dws_y+=y_step) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y + mv_display_right;
		for(dws_x = mv_display_right; dws_x < SCREEN_WIDTH; dws_x++) {
			*dws_scrn = dot;
			dws_scrn++;
		}
	}
}

/// @brief Fill black in screen
/// @param[in] y_top
/// @param[in] y_step
void DISPLAY::draw_screen_black(int y_top, int y_step)
{
	// fill black
	uint32_t dot = m_border_color;

	for(dws_y = 0; dws_y < SCREEN_HEIGHT; dws_y++) {
		dws_scrn = dws_scrn0 + dws_scrn_offset * dws_y;
		for(dws_x = 0; dws_x < SCREEN_WIDTH; dws_x++) {
			*dws_scrn = dot;
			dws_scrn++;
		}
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void DISPLAY::event_frame()
{
	// contrast
	if (m_contrast_count) {
		m_contrast_count--;
		if (m_contrast_count == 0) {
			if (m_curr_contrast < m_tag_contrast) {
				m_curr_contrast++;
			} else if (m_curr_contrast > m_tag_contrast) {
				m_curr_contrast--;
			}
			m_curr_contrast &= 0xf;
			if (m_curr_contrast != m_tag_contrast) {
				m_contrast_count = CONTRAST_COUNT;
			}
			OUT_DEBUG2(_T("CONTRAST UPDATE: c:%d t:%d n:%d"), m_curr_contrast, m_tag_contrast, m_contrast_count);
		}
	}
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Char(v) vm_state.v = v
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint16_BE(v) vm_state.v = Uint16_BE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void DISPLAY::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<0x200; i++) {
		// BIG endien
		SET_Uint16_BE(m_palette[i]);
	}

	for(int i=0; i<512; i++) {
		SET_Uint32_LE(m_vc_prev[i]);
		SET_Uint16_LE(m_crtc_prev[i]);
	}


	SET_Char(m_curr_contrast);		///< $e8e001
	SET_Char(m_tag_contrast);		///< $e8e001
	SET_Uint16_LE(m_contrast_count);
	for(int i=0; i<3; i++) {
		SET_Uint16_LE(m_vc_regs[i]);		///< video controller $e82400-$e82600
	}
	vm_state.m_border_color = (FLG_ORIG_BORDER_COLOR ? 1 : 0);

	for(int i=0; i<4; i++) {
		SET_Int32_LE(m_vc_priscrn[i]);		///< Sprite, Text and Graphic priority on video controller
		SET_Int32_LE(m_vc_priority[i]);		///< Priority number on each screen (item0:SP item1:TX item2:GR)
		SET_Int32_LE(m_vc_gr_pripage[i]);	///< Graphics page priority on video controller
		SET_Int32_LE(m_vc_gr_prisfts[i]);	///< Graphics page priority on video controller
	}

	SET_Int32_LE(mv_display_width);
	SET_Int32_LE(mv_display_height);
	SET_Int32_LE(mv_display_left);
	SET_Int32_LE(mv_display_top);

	SET_Int32_LE(mv_display_right);
	SET_Int32_LE(mv_display_bottom);
	SET_Int32_LE(m_crtc_left_base);
	SET_Int32_LE(m_crtc_top_base);

	SET_Int32_LE(m_raster_even_odd);
	SET_Int32_LE(m_raster_mode);
	SET_Int32_LE(m_show_screen);
	SET_Int32_LE(m_draw_even_odd);

	SET_Int32_LE(screen_left);
	SET_Int32_LE(screen_width);
	SET_Int32_LE(screen_top);
	SET_Int32_LE(screen_height);

	SET_Int32_LE(skip_frame_count);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Char(v) v = vm_state.v
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint16_BE(v) v = Uint16_BE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool DISPLAY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	for(int i=0; i<0x200; i++) {
		// BIG endien
		GET_Uint16_BE(m_palette[i]);
	}

	for(int i=0; i<512; i++) {
		GET_Uint32_LE(m_vc_prev[i]);
		GET_Uint16_LE(m_crtc_prev[i]);
	}

	GET_Char(m_curr_contrast);		///< $e8e001
	GET_Char(m_tag_contrast);		///< $e8e001
	GET_Uint16_LE(m_contrast_count);
	for(int i=0; i<3; i++) {
		GET_Uint16_LE(m_vc_regs[i]);		///< video controller $e82400-$e82600
	}
//	BIT_ONOFF(pConfig->original, MSK_ORIG_BORDER_COLOR, vm_state.m_border_color != 0);

	for(int i=0; i<4; i++) {
		GET_Int32_LE(m_vc_priscrn[i]);		///< Sprite, Text and Graphic priority on video controller
		GET_Int32_LE(m_vc_priority[i]);		///< Priority number on each screen (item0:SP item1:TX item2:GR)
		GET_Int32_LE(m_vc_gr_pripage[i]);	///< Graphics page priority on video controller
		GET_Int32_LE(m_vc_gr_prisfts[i]);	///< Graphics page priority on video controller
	}

	GET_Int32_LE(mv_display_width);
	GET_Int32_LE(mv_display_height);
	GET_Int32_LE(mv_display_left);
	GET_Int32_LE(mv_display_top);

	GET_Int32_LE(mv_display_right);
	GET_Int32_LE(mv_display_bottom);
	GET_Int32_LE(m_crtc_left_base);
	GET_Int32_LE(m_crtc_top_base);

	GET_Int32_LE(m_raster_even_odd);
	m_raster_mode = (en_raster_modes)Int32_LE(vm_state.m_raster_mode);
	GET_Int32_LE(m_show_screen);
	GET_Int32_LE(m_draw_even_odd);

//	GET_Int32_LE(screen_left);
//	GET_Int32_LE(screen_width);
//	GET_Int32_LE(screen_top);
//	GET_Int32_LE(screen_height);

	GET_Int32_LE(skip_frame_count);

	set_border_color();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t DISPLAY::debug_read_io16(uint32_t addr)
{
	uint32_t data = 0xffff;
	uint32_t addrh = (addr >> 1);

	switch(addr & 0x0f00) {
	case 0x000:
	case 0x100:
	case 0x200:
	case 0x300:
		// palette
		data = m_palette[addrh & 0x1ff];
		break;
	case 0x400:
	case 0x500:
	case 0x600:
		// video control
		if ((addrh & 0x7f) == 0) {
			uint32_t num = (addr >> 8) & 0x7;
			num -= 4;
			data = m_vc_regs[num];
		}
		break;
	}
	return data;
}

static const _TCHAR *c_vc_reg_names[] = {
	_T("VC_MODE"),
	_T("VC_PRIORITY"),
	_T("VC_ONOFF"),
	NULL
};

bool DISPLAY::debug_write_reg(int type, const _TCHAR *reg, uint32_t data)
{
	if (type == 1) {
		// video control
		uint32_t num = find_debug_reg_name(c_vc_reg_names, reg);
		return debug_write_reg(type, num, data);
	}
	return false;
}

bool DISPLAY::debug_write_reg(int type, uint32_t reg_num, uint32_t data)
{
	switch(type) {
	case 1:
		// video control
		if (reg_num < 3) {
			m_vc_regs[reg_num] = data;
			return true;
		}
		break;
	default:
		// palette
		if (reg_num < 0x200) {
			m_palette[reg_num] = data;
			return true;
		}
		break;
	}
	return false;
}

static const _TCHAR *c_vc_r0_sz_co[] = {
	_T("Graphic real 512x 512    16colors x4pages"),	// 000
	_T("Graphic real 512x 512   256colors x2pages"),	// 001
	_T("Graphic real 512x 512 Invalid 2"),			// 010
	_T("Graphic real 512x 512 65536colors x1page"),		// 011
	_T("Graphic real1024x1024    16colors x1page"),		// 100
	_T("Graphic real1024x1024 Invalid 5"),		// 101
	_T("Graphic real1024x1024 Invalid 6"),		// 110
	_T("Graphic real1024x1024 Invalid 7"),		// 111
};

static const _TCHAR *c_vc_r1_pr[] = {
	_T("SP"),
	_T("TX"),
	_T("GR"),
	_T("??")
};

static const _TCHAR *c_vc_r2_onoff[] = {
	_T("G0"),
	_T("G1"),
	_T("G2"),
	_T("G3"),
	_T("G4"),
	_T("TX"),
	_T("SP")
};

static const _TCHAR *c_vc_r2_sp_pri[] = {
	_T("GT"),
	_T("GG"),
	_T("BP"),
	_T("HP"),
	_T("EO"),
	_T("VH"),
	_T("AH"),
	_T("YS"),
};

void DISPLAY::debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	switch(type) {
	case 1:
		UTILITY::tcscat(buffer, buffer_len, _T("VIDEO CONTROL:\n"));
		for(int num=0; num<3; num++) {
			UTILITY::sntprintf(buffer, buffer_len, _T(" %02X(%-12s): %04X  ")
				, num
				, c_vc_reg_names[num]
				, m_vc_regs[num]
			);
			switch(num) {
			case 0:
				UTILITY::tcscat(buffer, buffer_len, c_vc_r0_sz_co[m_vc_regs[num] & (VC_GS_CO_MASK | VC_GS_SZ_MASK)]);
				break;
			case 1:
				// priority
				UTILITY::tcscat(buffer, buffer_len, _T("Priority: "));
				for(int i=0; i<3; i++) {
					if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(" > "));
					UTILITY::tcscat(buffer, buffer_len, c_vc_r1_pr[m_vc_priscrn[i] & 3]);
				}
				UTILITY::tcscat(buffer, buffer_len, _T("  GR page: "));
				// graphic
				for(int i=0; i<4; i++) {
					if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(" > "));
					UTILITY::sntprintf(buffer, buffer_len, _T("%d"), m_vc_gr_pripage[i]);
				}
				break;
			case 2:
				// on/off
				UTILITY::tcscat(buffer, buffer_len, _T("\n  On:"));
				for(int i=6; i>=0; --i) {
					UTILITY::sntprintf(buffer, buffer_len, _T(" %s:%c"), c_vc_r2_onoff[i], (m_vc_regs[num] & (1 << i)) ? _T('1') : _T('-'));
				}
				// translucent and special priority
				UTILITY::tcscat(buffer, buffer_len, _T("\n  Sp:"));
				for(int i=7; i>=0; --i) {
					UTILITY::sntprintf(buffer, buffer_len, _T(" %s:%c"), c_vc_r2_sp_pri[i], (m_vc_regs[num] & (0x100 << i)) ? _T('1') : _T('-'));
				}
			}
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		}
		break;
	default:
		UTILITY::tcscat(buffer, buffer_len, _T("PALETTE:\n"));
		for(int y=0; y<32; y++) {
			UTILITY::sntprintf(buffer, buffer_len, _T("%03X:"), y * 16);
			for(int x=0; x<16; x++) {
				UTILITY::sntprintf(buffer, buffer_len, _T(" %04X"), m_palette[y * 16 + x]);
			}
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		}
		break;
	}
}

enum en_gnames {
	GN_PAL = 0,
	GN_PCG,
	GN_BGN0,
	GN_BGN1,
	GN_BGH0,
	GN_BGH1,
	GN_BGR0,
	GN_BGR1,
	GN_TXR,
	GN_SPR0,
	GN_SPR1,
	GN_SPR2,
	GN_GRR0,
	GN_GRR1,
	GN_GRR2,
	GN_GRR3,
	GN_TXP,
	GN_MIXD,
};

const struct st_gnames {
	int type;
	const _TCHAR *name;
	const struct {
		int width;
		int height;
	} size[2];
} c_gnames[] = {
	{ GN_PAL,  _T("Palette"), 16*32, 32*16+256*2, 16, 32 },
	{ GN_PCG,  _T("PCG Render"), 64*8, 16*32, 0, 0 },
	{ GN_BGN0, _T("BG Area 0 (Normal)"),  512,  512, 0, 0 },
	{ GN_BGN1, _T("BG Area 1 (Normal)"),  512,  512, 0, 0 },
	{ GN_BGH0, _T("BG Area 0 (Hireso)"), 1024, 1024, 0, 0 },
	{ GN_BGH1, _T("BG Area 1 (Hireso)"), 1024, 1024, 0, 0 },
	{ GN_BGR0, _T("BG0 Render"), 1024, 1024, 0, 0 },
	{ GN_BGR1, _T("BG1 Render"),  512,  512, 0, 0 },
	{ GN_TXR,  _T("Text Render"), 1024, 1024, 0, 0 },
	{ GN_SPR0, _T("Sprite Pattern Render"), 16*16, 16*8, 0, 0 },
	{ GN_SPR1, _T("Sprite Render (512x512)"), 512, 512, 0, 0 },
	{ GN_SPR2, _T("Sprite Render (1024x1024)"), 1024, 1024, 0, 0 },
	{ GN_GRR0, _T("Graphic Render (512x512, 16 colors)"), 512, 2048, 0, 0 },
	{ GN_GRR1, _T("Graphic Render (512x512, 256 colors)"), 512, 1024, 0, 0 },
	{ GN_GRR2, _T("Graphic Render (512x512, 65536 colors)"), 512, 512, 0, 0 },
	{ GN_GRR3, _T("Graphic Render (1024x1024, 16 colors)"), 1024, 1024, 0, 0 },
	{ GN_TXP,  _T("Text Plane 0-3"), 2048, 2048, 0, 0 },
	{ GN_MIXD, _T("Mixed Buffer"), 1024, SCREEN_HEIGHT, 0, 0 },
	{ -1, NULL, 0, 0, 0, 0 }
};

int DISPLAY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	for(int i=0; c_gnames[i].type >= 0; i++) {
		if (type == c_gnames[i].type) {
			*width = c_gnames[i].size[num].width;
			*height = c_gnames[i].size[num].height;
			if (*width <= 0 && *height <= 0) {
				*width = c_gnames[i].size[0].width;
				*height = c_gnames[i].size[0].height;
			}
			return 0;
		}
	}
	return -1;
}

bool DISPLAY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	for(int i=0; c_gnames[i].type >= 0; i++) {
		if (type == c_gnames[i].type) {
			UTILITY::tcscpy(buffer, buffer_len, c_gnames[i].name);
			return true;
		}
	}
	return false;
}

bool DISPLAY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{
	switch(type) {
	case GN_PAL:
		debug_expand_palette_all(width, height, buffer);
		break;
	case GN_PCG:
		debug_expand_pcg_render(width, height, buffer);
		break;
	case GN_BGN0:
		debug_expand_bg_data(0, 0, width, height, buffer);
		break;
	case GN_BGN1:
		debug_expand_bg_data(1, 0, width, height, buffer);
		break;
	case GN_BGH0:
		debug_expand_bg_data(0, 1, width, height, buffer);
		break;
	case GN_BGH1:
		debug_expand_bg_data(1, 1, width, height, buffer);
		break;
	case GN_BGR0:
		debug_expand_render(rb_bgram0, width, height, buffer);
		break;
	case GN_BGR1:
		debug_expand_render(rb_bgram1, width, height, buffer);
		break;
	case GN_TXR:
		debug_expand_text_render(rb_tvram, width, height, buffer);
		break;
	case GN_SPR0:
		debug_expand_sprite_ptn_render(width, height, buffer);
		break;
	case GN_SPR1:
	case GN_SPR2:
		debug_expand_sprite_render(width, height, buffer);
		break;
	case GN_GRR0:
	case GN_GRR1:
	case GN_GRR2:
	case GN_GRR3:
		debug_expand_graphic_render(type - GN_GRR0, width, height, buffer);
		break;
	case GN_TXP:
		debug_expand_text_plane(width, height, buffer);
		break;
	case GN_MIXD:
		debug_expand_mixed_buffer(width, height, buffer);
		break;
	default:
		return false;
	}
	return true;
}

bool DISPLAY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{
	switch(type) {
	case GN_PAL:
		debug_expand_palette_dumper(width, height, buffer);
		break;
	case GN_PCG:
		debug_expand_pcg_dumper(width, height, buffer);
		break;
	case GN_BGN0:
		debug_expand_bg_data_dumper(0, 0, width, height, buffer);
		break;
	case GN_BGN1:
		debug_expand_bg_data_dumper(1, 0, width, height, buffer);
		break;
	case GN_BGH0:
		debug_expand_bg_data_dumper(0, 1, width, height, buffer);
		break;
	case GN_BGH1:
		debug_expand_bg_data_dumper(1, 1, width, height, buffer);
		break;
	case GN_BGR0:
		debug_expand_dumper(rb_bgram0, width, height, buffer);
		break;
	case GN_BGR1:
		debug_expand_dumper(rb_bgram1, width, height, buffer);
		break;
	case GN_TXR:
		debug_expand_text_dumper(rb_tvram, width, height, buffer);
		break;
	case GN_SPR0:
		debug_expand_sprite_ptn_dumper(width, height, buffer);
		break;
	case GN_SPR1:
	case GN_SPR2:
		debug_expand_sprite_dumper(width, height, buffer);
		break;
	case GN_GRR0:
	case GN_GRR1:
	case GN_GRR2:
	case GN_GRR3:
		debug_expand_graphic_dumper(type - GN_GRR0, width, height, buffer);
		break;
	case GN_TXP:
		debug_expand_text_plane_dumper(width, height, buffer);
		break;
	case GN_MIXD:
		debug_expand_mixed_buffer_dumper(width, height, buffer);
		break;
	default:
		return false;
	}
	return true;
}

#endif

