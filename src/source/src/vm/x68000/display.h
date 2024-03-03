/** @file display.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ display and video controller ]
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"

#define DRAW_SCREEN_PER_FRAME	1
#define DRAW_SCREEN_PER_RASTER	2
#define DRAW_SCREEN_METHOD DRAW_SCREEN_PER_RASTER

#ifdef _DEBUG
//#define DEBUG_CRTMON
//#define DEBUG_CRTMON2
#endif

#undef USE_GRAPHIC_RENDER
#undef USE_SPRITE_RENDER

#undef USE_GRAPHIC_1024ALT

class EMU;
class CRTC;
class SPRITE_BG;

/**
	@brief Display Monitor

	* Video Control I/O is here
*/
class DISPLAY : public DEVICE
{
public:
	/// @brief signals on DISPLAY
	enum SIG_DISPLAY_IDS {
		SIG_DISPLAY_LPSTB,
		SIG_DISPLAY_VSYNC,
//		SIG_DISPLAY_WRITE_REGS,
		SIG_DISPLAY_SIZE,
		SIG_CONTRAST
	};

private:
	enum en_sprite_bg_masks {
		SP_BG_BG_COLOR		= 0x0f00,
		SP_BG_BG_COLOR_SFT	= 8,
		SP_BG_H_REVERSE		= 0x4000,
		SP_BG_V_REVERSE		= 0x8000,
		SP_BG_HV_REVERSE	= 0xc000,
		SP_BG_HV_REVERSE_SFT = 14,
	};
	enum en_vc_regs {
		VC_GRAPHIC_SIZE = 0,
		VC_PRIORITY,
		VC_TRANS_ONOFF
	};
	enum en_vc_size_masks {
		VC_GS_SZ_MASK = 0x0004,
		VC_GS_CO_MASK = 0x0003,
		VC_GS_CO65536_MASK = 0x0003,
		VC_GS_CO256_MASK = 0x0001,
		VC_GS_CO16_MASK = 0x0000,
		VC_GS_SZCO_MASK = 0x0007,
	};
	enum en_vc_priority_masks {
		VC_PR_SP_MASK = 0x3000,
		VC_PR_SP_SFT  = 12,
		VC_PR_TX_MASK = 0x0c00,
		VC_PR_TX_SFT  = 10,
		VC_PR_GR_MASK = 0x0300,
		VC_PR_GR_SFT  = 8,
		VC_PR_GRX_MASK = 0x00ff,
	};
	enum en_vc_trans_onoff_masks {
		VC_TO_SPON_MASK = 0x0040,	///< Sprite and BG
		VC_TO_TXON_MASK = 0x0020,
		VC_TO_G4ON_MASK = 0x0010,
		VC_TO_G3ON_MASK = 0x0008,
		VC_TO_G2ON_MASK = 0x0004,
		VC_TO_G1ON_MASK = 0x0002,
		VC_TO_G0ON_MASK = 0x0001,
		VC_TO_G01ON_MASK = 0x0003,
		VC_TO_GRON_MASK = 0x000f,
		VC_TO_ONFF_MASK = 0x007f,

		VC_TO_GT_MASK = 0x0100,
		VC_TO_GG_MASK = 0x0200,
		VC_TO_BP_MASK = 0x0400,
		VC_TO_HP_MASK = 0x0800,
		VC_TO_EO_MASK = 0x1000,
		VC_TO_AH_MASK = 0x4000,
	};

private:
	CRTC *d_crtc;
	SPRITE_BG *d_sp_bg;

	uint16_t *p_crtc_regs;
	uint16_t *p_sp_regs;
	uint16_t *p_bg_regs;

	uint16_t *p_cgrom;
	uint16_t *p_tvram;
	uint8_t *pu_tvram;
	uint16_t *p_gvram;
//	uint8_t *pu_gvram;
	uint16_t *p_spram;			///< Sprite/BG RAM
	uint16_t *pu_pcg;
//	uint16_t *pu_bg[2];

//	uint8_t m_dummy[256];

	int8_t m_curr_contrast;		///< $e8e001
	int8_t m_tag_contrast;		///< $e8e001
	uint16_t m_contrast_count;
	enum en_contrast_count {
		CONTRAST_COUNT = 3
	};

	// -- video control
	uint16_t m_palette[0x200];	///< palette (graphics, text, sprite) 1KB

	uint16_t m_vc_regs[3];	///< video controller $e82400-$e82600
	uint32_t m_vc_prev[512];
	uint16_t m_crtc_prev[512];

	int m_vc_priscrn[4];		///< Sprite, Text and Graphic priority on video controller
	enum en_priscrn_nums {
		PR_SP = 0,
		PR_TX,
		PR_GR,
		PR_RESERVED,
	};
	int m_vc_priority[4];		///< Priority number on each screen (item0:SP item1:TX item2:GR)
	int m_vc_gr_pripage[4];		///< Graphics page priority on video controller
	int m_vc_gr_prisfts[4];		///< Graphics page priority on video controller

//	uint32_t m_pcg_read[8];		///< PCG vram read and render updated flag

	int mv_display_width;
	int mv_display_height;
	int mv_display_left;
	int mv_display_top;
	int mv_display_right;
	int mv_display_bottom;

	int m_crtc_left_base;
	int m_crtc_top_base;

//	bool m_transparent;	///< transparent when palette0

#ifdef USE_GRAPHIC_RENDER
	uint8_t  rb_gvram[1024 * 1024];		///< Graphic Rendef buffer
#endif
	uint8_t  rb_tvram[1024 * 1024];		///< Text Rendef buffer
	uint16_t rb_bgram0[1024 * 1024];	///< BG0 Render buffer
	uint16_t rb_bgram1[512 * 512];		///< BG1 Render buffer
#ifdef USE_SPRITE_RENDER
	uint16_t rb_spram[128][16 * 16];	///< SPRITE Render buffer 16x16 x 128
#endif
	enum en_rb_pcg_masks {
		RB_PCG_PALETTE = 0x0f
	};
	uint8_t  rb_pcg[256][4][16 * 16];	///< PCG Render buffer 16x16 Normal HReverse VReverse HVRev.

	enum en_mx_txspbg_masks {
		MX_TXSPBG_TEXT			 = 0x8000,
		MX_TXSPBG_SPBG			 = 0x4000,
		MX_TXSPBG_SPRITE_PRIORITY	= 0x3000,	///< SPRITE and BG priority
		MX_TXSPBG_SPRITE_PRIORITY_SFT = 12,
		MX_TXSPBG_SPBG_PALETTE	 = 0x01ff,
		MX_TXSPBG_PALETTE_MASK	 = 0x00ff,
		MX_TXSPBG_PALETTE_L4	 = 0x000f,
//		MX_TXSPBG_WIDTH = 768,
		MX_TXSPBG_WIDTH_SFT = 10
	};
	uint16_t mx_txspbg[(1 << MX_TXSPBG_WIDTH_SFT) * 512];		// TEXT, SPRITE and BG mixed buffer
	enum en_mx_buf_masks {
		MX_BUF_COLOR		 = 0x0000ffff,	// color grbi
		MX_BUF_GR_DATA		 = 0x01000000,	// graphic data is stored
		MX_BUF_NO_TRANS		 = 0x02000000,	// no transparent
		MX_BUF_SP_AREA	     = 0x04000000,	// special priority area
		MX_BUF_TR_AREA	     = 0x08000000,	// translucent area
//		MX_BUF_WIDTH = 768,
		MX_BUF_WIDTH_SFT = 10
	};
	uint32_t mx_buf[(1 << MX_BUF_WIDTH_SFT) * 512];	// Mixed buffer

	// synchronizable vertical range
	int *p_crtc_vt_total;
	int *p_crtc_vt_disp;

	// synchronizable horizontal range
	int *p_crtc_hz_total;
	int *p_crtc_hz_disp;

	int m_raster_even_odd;
	enum en_raster_modes {
		RASTER_NORMAL_NONINTER = 0,
		RASTER_NORMAL_INTERLACE_512,
		RASTER_HIRESO_DOUBLE,
		RASTER_HIRESO_NONINTER,
		RASTER_HIRESO_INTERLACE_512,
		RASTER_HIRESO_INTERLACE_1024,
	} m_raster_mode;

	int m_show_screen;

	int m_draw_even_odd;

	scrntype m_border_color;

#ifdef DEBUG_SHOW_FLAGS
	struct {
		uint16_t vline;
		uint16_t flags;
	} md_show_screen[1024];
	int md_show_screen_cnt;
#endif

	uint32_t Rmask;
	uint32_t Gmask;
	uint32_t Bmask;
	uint32_t Amask;
	uint8_t  Rshift;
	uint8_t  Gshift;
	uint8_t  Bshift;
	uint8_t  Ashift;


	uint8_t c_bright_table[16][32][2];	// palette -> rgb table


	int screen_left;
	int screen_right;
	int screen_top;
	int screen_bottom;

	int skip_frame_count;

	int *vm_pause;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint16_t m_palette[0x200];	///< palette (graphics, text, sprite) 1KB

		uint32_t m_vc_prev[512];
		uint16_t m_crtc_prev[512];

		int8_t m_curr_contrast;		///< $e8e001
		int8_t m_tag_contrast;		///< $e8e001
		uint16_t m_contrast_count;
		uint16_t m_vc_regs[3];		///< video controller $e82400-$e82600
		uint8_t m_border_color;
		char reserved1[5];

		int m_vc_priscrn[4];		///< Sprite, Text and Graphic priority on video controller
		int m_vc_priority[4];		///< Priority number on each screen (item0:SP item1:TX item2:GR)
		int m_vc_gr_pripage[4];		///< Graphics page priority on video controller
		int m_vc_gr_prisfts[4];		///< Graphics page priority on video controller

		int mv_display_width;
		int mv_display_height;
		int mv_display_left;
		int mv_display_top;

		int mv_display_right;
		int mv_display_bottom;
		int m_crtc_left_base;
		int m_crtc_top_base;

		int m_raster_even_odd;
		int m_raster_mode;
		int m_show_screen;
		int m_draw_even_odd;

		int screen_left;
		int screen_right;
		int screen_top;
		int screen_bottom;

		int skip_frame_count;

		uint32_t reserved2[3];
	};
#pragma pack()

	int dws_x;
	int dws_y;

	int dws_scrn_offset;
	scrntype *dws_scrn0;
	scrntype *dws_scrn;
//	scrntype dws_dot;
//	scrntype *dws_scrnline1p;
	uint32_t dws_r,dws_g,dws_b,dws_a,dws_i;

//	void update_dws_params();
//	void update_chr_clocks(int clk);
	inline void set_display_mode(uint32_t data);

	inline void update_show_screen_flags();
	inline void draw_screen_black(int y_even_odd, int y_step);
	inline void draw_screen_frame(int y_even_odd, int y_step);
	inline void draw_screen_mixed_one_line(int src_y);
	inline void draw_screen_mixed(int y_even_odd, int y_step);
	inline void clear_mix_buffer_one_line(int width, int y);
	inline void clear_mix_txspbg_render_one_line(int width, int y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_sp(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_txsp_gr_n(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_sp(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_ht(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_tx_gr_sp_n(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_sp(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_ht(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_sp_gr_tx_n(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_ht(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_gr_txsp_n(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line_sub_ah(int width, int src_y, int dst_y);
	inline void mix_buffer_graphic_txspbg_one_line(int width, int src_y, int dst_y);
	inline void render_text_one_line_sub(int y);
	inline void render_text_one_line(int y);
	inline void mix_render_text_sprite_bg_one_line_sub_tx_only(int width, int src_left, int src_y, int dst_y);
	inline void mix_render_text_sprite_bg_one_line_sub_sp_tx(int width, int src_left, int src_y, int dst_y);
	inline void mix_render_text_sprite_bg_one_line_sub_tx_sp(int width, int src_left, int src_y, int dst_y);
	inline void mix_render_text_sprite_bg_one_line(int width, int src_y, int dst_y);
	inline void mix_render_text_sprite_bg(int width, int height);
	inline void mix_render_bg0_one_line_sub(int width, int src_left, int src_x_limit, int disp_left, int src_y, int dst_y);
	inline void mix_render_bg1_one_line_sub(int width, int src_left, int src_x_limit, int disp_left, int src_y, int dst_y);
	inline void mix_render_bg0_one_line(int size512, int width, int disp_left, int disp_top, int src_y, int dst_y);
	inline void mix_render_bg1_one_line(int size512, int width, int disp_left, int disp_top, int src_y, int dst_y);
	inline void mix_render_sprite_bg_one_line(int step_y, int src_y, int dst_y);
#ifdef USE_SPRITE_RENDER
	inline void mix_render_sprite_cell_one_line_sub(int sp_num, uint32_t bg_priority, int src_left, int src_y, int dst_left, int dst_right, int dst_y);
#else
	inline void mix_render_sprite_cell_one_line_sub(int ptn_num, int hv, uint32_t bg_priority, int src_left, int src_y, int dst_left, int dst_right, int dst_y);
#endif
	inline void mix_render_sprite_cell_one_line(int size512, int width, int sp_num, int disp_left, int disp_top, int step_y, int dst_y);
	inline void mix_render_sprite_one_line(int size512, int width, int disp_left, int disp_top, int step_y, int dst_y);
	inline void expand_pcg_one_line_sub_ad(uint32_t src_addr, uint8_t *buffer_n, uint8_t *buffer_h);
	inline void expand_pcg_one_line_sub(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, uint16_t *buffer);
	inline void expand_pcg_one_data_ad(uint32_t src_addr, int width, uint8_t *buffer_n, uint8_t *buffer_h, uint8_t *buffer_v, uint8_t *buffer_hv);
	inline void expand_pcg_one_data(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer);
	inline void expand_pcg_one_data16x16_ad(uint32_t src_addr, int width, uint8_t *buffer_n, uint8_t *buffer_h, uint8_t *buffer_v, uint8_t *buffer_hv);
	inline void expand_pcg_one_data16x16(uint32_t src_addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer);
	inline void render_sprite_bg_one_line(int src_y, int dst_y);
	inline void render_bg0_one_line_sub(int size512, uint32_t addr_offset, int src_y, int dst_y);
	inline void render_bg1_one_line_sub(int size512, uint32_t addr_offset, int src_y, int dst_y);
	inline void render_bg0_one_block_sub(int size512, int width, uint32_t addr_offset, int by);
	inline void render_bg1_one_block_sub(int size512, int width, uint32_t addr_offset, int by);
	inline void render_bg0_one_line(int src_y, int dst_y);
	inline void render_bg1_one_line(int src_y, int dst_y);
	inline void render_pcg_one_16x16(int ptn_num);
	inline void copy_pcg_one_16x1(int ptn_num, int hv, uint32_t bg_ptn, int l, uint16_t *dst);
	inline void copy_pcg_one_8x1(int ptn_num, int hv, uint32_t bg_ptn, int l, uint16_t *dst);
	inline void copy_pcg_one_16x16(int ptn_num, int hv, uint32_t bg_ptn, uint16_t *dst, int width);
	inline void copy_pcg_one_8x8(int ptn_num, int hv, uint32_t bg_ptn, uint16_t *dst, int width);
//	inline void init_pcg_read_flags();
//	inline void update_pcg_read_flags(int num);
//	inline void commit_pcg_read_flags_8x8(uint32_t mask);
//	inline void commit_pcg_read_flags_16x16(uint32_t mask);
	inline uint32_t expand_rgb16(uint32_t pal);
	inline uint32_t mix_translucent_color16(uint32_t pal_a, uint32_t pal_b);
	inline void mix_buffer_graphic_one_line(int width, int src_y_base, int dst_y);
	inline void set_mix_buffer_graphic_flags(int width, int dst_y);
	inline void mix_buffer_graphic1024_one_line(int width, int src_y_base, int dst_y);
#ifdef USE_GRAPHIC_RENDER
	inline void mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int dst_y);
#else
#ifndef USE_GRAPHIC_1024ALT
	inline void mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int sft[2], int dst_y);
#else
	inline void mix_buffer_graphic1024_c16_one_line_sub(int width, int src0_left, int src0_y, int sft, int dst_y);
#endif
#endif
	inline void mix_buffer_graphic1024_c16_one_line(int width, int src_y_base, int dst_y);
	inline void mix_buffer_graphic512_one_line(int width, int src_y_base, int dst_y);
	inline void mix_buffer_graphic512_c65536_one_line_sub_p(int width, int src_left[4], int src_top[4], int src_y[4], int sft[4], uint32_t onoff[4], int dst_y);
	inline void mix_buffer_graphic512_c65536_one_line_sub_n(int width, int src_left[4], int src_top[4], int src_y[4], int sft[4], uint32_t onoff[4], int dst_y);
	inline void mix_buffer_graphic512_c65536_one_line(int width, int src_y_base, int dst_y);
	inline void mix_buffer_graphic512_c256_one_line_sub_s(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y);
	inline void mix_buffer_graphic512_c256_one_line_sub_p(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y);
	inline void mix_buffer_graphic512_c256_one_line_sub_n(int pri_num, int width, int src_left[2], int src_top[2], int src_y[2], int sft[2], uint32_t onoff[2], int dst_y);
	inline void mix_buffer_graphic512_c256_one_line(int width, int src_y_base, int dst_y);
	inline void mix_buffer_graphic512_c16_one_line_sub_s(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y);
	inline void mix_buffer_graphic512_c16_one_line_sub_p(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y);
	inline void mix_buffer_graphic512_c16_one_line_sub_n(int pri_num, int width, int src_left, int src_top, int src_y, int sft, uint32_t onoff, int dst_y);
	inline void mix_buffer_graphic512_c16_one_line(int width, int src_y_base, int dst_y);
#ifdef USE_SPRITE_RENDER
	inline void render_sprite_cell_one_line(int sp_num, int y);
	inline void render_sprite_one_line(int y);
#endif
#ifdef USE_GRAPHIC_RENDER
	inline void render_graphic_one_line(int y);
	inline void render_graphic();
	inline void render_graphic1024_one_line_sub(uint32_t need_update, int y);
	inline void render_graphic1024_one_line(int y);
	inline void render_graphic1024();
	inline void render_graphic512_one_line_sub(uint32_t need_update, int y);
	inline void render_graphic512_one_line(int y);
	inline void render_graphic512();
#endif
#if (DRAW_SCREEN_METHOD == DRAW_SCREEN_PER_FRAME)
	inline void clear_mix_buffer(int width, int height9);
	inline void clear_mix_txspbg_render(int width, int height9);
	inline void render_text();
	inline void mix_buffer_graphic_txspbg(int width, int height);
	inline void mix_render_bg0(int size512, int width);
	inline void mix_render_bg1(int size512, int width);
	inline void mix_render_sprite_bg(int step_y);
	inline void mix_render_sprite_cell(int size512, int width, int sp_num, int disp_left, int disp_top, int step_y);
	inline void mix_render_sprite(int size512, int width, int step_y);
	inline void render_sprite_bg();
	inline void render_bg0();
	inline void render_bg1();
	inline void render_pcg();
	inline void mix_buffer_graphic(int width, int height9);
	inline void mix_buffer_graphic1024(int width, int height9);
	inline void mix_buffer_graphic1024_c16(int width, int height9);
	inline void mix_buffer_graphic512(int width, int height9);
	inline void mix_buffer_graphic512_c65536(int width, int height9);
	inline void mix_buffer_graphic512_c256(int width, int height9);
	inline void mix_buffer_graphic512_c16(int width, int height9);
#ifdef USE_SPRITE_RENDER
	inline void render_sprite_cell(int sp_num);
	inline void render_sprite();
#endif
#endif

#ifdef USE_DEBUGGER
	uint32_t debug_expand_rgb16(uint32_t pal);
	uint32_t debug_expand_palcol8(uint32_t pal_num);
	uint32_t debug_expand_palette(uint32_t pal_num);
	uint32_t debug_expand_palette65536(uint32_t pal0_num, uint32_t pal1_num);
	void debug_expand_palette_all(int width, int height, scrntype *buffer);
	void debug_expand_palette_dumper(int width, int height, uint16_t *buffer);
	void debug_expand_pcg_one_line(uint32_t addr, uint32_t bg_palette, bool h_reverse, scrntype *buffer);
	void debug_expand_pcg_one_line_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, uint16_t *buffer);
	void debug_expand_pcg_one_data(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, scrntype *buffer);
	void debug_expand_pcg_one_data_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer);
	void debug_expand_pcg_one_data16x16(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, scrntype *buffer);
	void debug_expand_pcg_one_data16x16_dumper(uint32_t addr, uint32_t bg_palette, bool h_reverse, bool v_reverse, int width, uint16_t *buffer);
	void debug_expand_pcg_data(int width, int height, scrntype *buffer);
	void debug_expand_pcg_data_dumper(int width, int height, uint16_t *buffer);
	void debug_expand_pcg_render(int width, int height, scrntype *buffer);
	void debug_expand_pcg_dumper(int width, int height, uint16_t *buffer);
	void debug_expand_bg_data(int area, int hireso, int width, int height, scrntype *buffer);
	void debug_expand_bg_data_dumper(int area, int hireso, int width, int height, uint16_t *buffer);
	void debug_expand_text_render(const uint8_t *render, int width, int height, scrntype *buffer);
	void debug_expand_text_dumper(const uint8_t *render, int width, int height, uint16_t *buffer);
	void debug_expand_render(const uint16_t *render, int width, int height, scrntype *buffer);
	void debug_expand_dumper(const uint16_t *render, int width, int height, uint16_t *buffer);
	void debug_expand_sprite_ptn_render(int width, int height, scrntype *buffer);
	void debug_expand_sprite_ptn_dumper(int width, int height, uint16_t *buffer);
	void debug_expand_sprite_render(int width, int height, scrntype *buffer);
	void debug_expand_sprite_dumper(int width, int height, uint16_t *buffer);
	void debug_expand_graphic_render(int type, int width, int height, scrntype *buffer);
	void debug_expand_graphic_dumper(int type, int width, int height, uint16_t *buffer);
	void debug_expand_text_plane(int width, int height, scrntype *buffer);
	void debug_expand_text_plane_dumper(int width, int height, uint16_t *buffer);
#endif

//	void draw_screen_sub_afterimage1();
//	void draw_screen_sub_afterimage2();

	void make_bright_table();
	void set_border_color();

public:
	DISPLAY(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("DISPLAY");
	}
	~DISPLAY() {}

	// common functions
	void initialize();
	void reset();
	void update_config();
	void write_io16(uint32_t addr, uint32_t data);
	void write_io_m(uint32_t addr, uint32_t data, uint32_t mask);
	uint32_t read_io16(uint32_t addr);
	uint32_t read_io_m(uint32_t addr, uint32_t mask);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void event_frame();

	// unique function
	void set_context_crtc(CRTC* device);
	void set_context_sprite_bg(SPRITE_BG* device);
	void set_cgrom_ptr(uint16_t* ptr) {
		p_cgrom = ptr;
	}
	void set_tvram_ptr(uint16_t* ptr, uint8_t *ptru) {
		p_tvram = ptr;
		pu_tvram = ptru;
	}
	void set_gvram_ptr(uint16_t* ptr /*, uint8_t *ptru */) {
		p_gvram = ptr;
//		pu_gvram = ptru;
	}
	void set_spram_ptr(uint16_t* ptr, uint16_t *ptrupcg /*, uint16_t *ptrubg0, uint16_t *ptrubg1*/) {
		p_spram = ptr;
		pu_pcg = ptrupcg;
//		pu_bg[0] = ptrubg0;
//		pu_bg[1] = ptrubg1;
	}
	void set_crtc_hz_ptr(int *p_hz_total, int *p_hz_disp) {
		p_crtc_hz_total = p_hz_total;
		p_crtc_hz_disp = p_hz_disp;
	}
	void set_crtc_vt_ptr(int *p_vt_total, int *p_vt_disp) {
		p_crtc_vt_total = p_vt_total;
		p_crtc_vt_disp = p_vt_disp;
	}
	void set_display_size(int left, int top, int right, int bottom);

	void draw_screen();
	void update_display(int vline, int clock);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io16(uint32_t addr);
	bool debug_write_reg(int type, const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(int type, uint32_t reg_num, uint32_t data);
	void debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len);
	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);
#endif
};

#endif /* DISPLAY_H */

