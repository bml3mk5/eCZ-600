/** @file crtc.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ CRTC ]
*/

#ifndef CRTC_H
#define CRTC_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief CRTC - CRT Controller
*/
class CRTC : public DEVICE
{
public:
	/// @brief signals on CRTC(CRTC)
	enum SIG_CRTC_IDS {
		SIG_CRTC_CPU_POWER	= 1,
		SIG_HRL
	};

	enum en_crtc_reg_names {
		CRTC_HORI_TOTAL = 0,	// R00
		CRTC_HSYNC_END,
		CRTC_HORI_START,
		CRTC_HORI_END,
		CRTC_VERT_TOTAL,
		CRTC_VSYNC_END,
		CRTC_VERT_START,
		CRTC_VERT_END,
		CRTC_HORI_ADJUST,		// R08
		CRTC_RASTER_INT,
		CRTC_TSCROLL_X,
		CRTC_TSCROLL_Y,
		CRTC_GSCROLL_X_0,
		CRTC_GSCROLL_Y_0,
		CRTC_GSCROLL_X_1,
		CRTC_GSCROLL_Y_1,
		CRTC_GSCROLL_X_2,
		CRTC_GSCROLL_Y_2,
		CRTC_GSCROLL_X_3,
		CRTC_GSCROLL_Y_3,
		CRTC_CONTROL0,			// R20
		CRTC_CONTROL1,			// R21
		CRTC_RASTER_ADDR,
		CRTC_BIT_MASK,
		CRTC_PORT,
	};
	enum en_crtc_r20_masks {
		CONTROL0_VRAM_ACCS = 0x0800,	// graphic memory access method
		CONTROL0_SIZE = 0x0400,		// graphic size on real screen
		CONTROL0_SIZE1024 = 0x0400,		// graphic 1024 dots on real screen
		CONTROL0_COLOR = 0x0300,	// graphic color
		CONTROL0_COLOR16 = 0x0000,	// graphic 16 colors mode
		CONTROL0_COLOR256 = 0x0100,	// graphic 256 colors mode
		CONTROL0_COLOR65536 = 0x0300,	// graphic 65536 colors mode
		CONTROL0_COLOR_SFT = 8,
		CONTROL0_RESOLUTION = 0x000f,	// resolution
		CONTROL0_HRES = 0x0003,	// horizontal
		CONTROL0_H256 = 0x0000,	// horizontal 256dot
		CONTROL0_H512 = 0x0001,	// horizontal 512dot
		CONTROL0_H768 = 0x0002,	// horizontal 768dot
		CONTROL0_H1024 = 0x0003,	// horizontal 1024dot?
		CONTROL0_VRES = 0x000c,	// vertical
		CONTROL0_VHRES = 0x000f,	// horizontal and vertical
		CONTROL0_V512 = 0x0004,	// vertical 512line
		CONTROL0_V768 = 0x0008,	// vertical 768line?
		CONTROL0_V1024 = 0x000c,	// vertical 1024line?
		CONTROL0_HIRESO = 0x0010,	// high resolution (Hsync: 31kHz)
		CONTROL0_HIRESO_SFT = 4,
	};
	enum en_crtc_r21_masks {
		CONTROL1_MEN = 0x0200,	// access mask text
		CONTROL1_SA = 0x0100,	// single / multi access text
		CONTROL1_AP = 0x00f0,	// multi access text plane
		CONTROL1_AP0 = 0x0010,	// multi access text0
		CONTROL1_AP1 = 0x0020,	// multi access text1
		CONTROL1_AP2 = 0x0040,	// multi access text2
		CONTROL1_AP3 = 0x0080,	// multi access text3
		CONTROL1_AP_SFT = 4,
		CONTROL1_CP = 0x000f,	// clear graphic or copy raster text
		CONTROL1_CP3 = 0x0008,	// clear graphic3 or copy raster text3
		CONTROL1_CP2 = 0x0004,	// clear graphic2 or copy raster text2
		CONTROL1_CP1 = 0x0002,	// clear graphic1 or copy raster text1
		CONTROL1_CP0 = 0x0001,	// clear graphic0 or copy raster text0
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_RASTER	= 0,
		EVENT_HSYNC_S	= 1,
		EVENT_HSYNC_E	= 2,
	};
	enum en_crtc_op_masks {
		OP_IMPORT = 0x01,
		OP_GCLS = 0x02,
		OP_TCOPY = 0x08,
	};


private:
	// output signals
	outputs_t outputs_raster;
	outputs_t outputs_vdisp;
	outputs_t outputs_vsync;
	outputs_t outputs_hsync;
	outputs_t outputs_chsize;

	static const uint16_t c_regs_mask[24];

	uint16_t m_regs[24];
	uint16_t m_vram_accs;	///< CRTC operation port
	uint16_t m_trig_vram_accs;
	int      m_intr_vline;	///< rater interrupt
	int      m_intr_vline_mask;

	int m_gcls_count;		///< high speed clear count

	uint16_t *p_tvram;		///< text vram
	uint8_t *pu_tvram;		///< text vram update flag
	uint16_t *p_gvram;		///< graphic vram
//	uint8_t *pu_gvram;		///< graphic vram update flag

	bool m_timing_changed;
	bool m_timing_updated;

	int m_cpu_clocks;
	double m_dot_clocks[2];
	double m_now_dot_clocks;

#ifdef CRTC_HORIZ_FREQ
	int horiz_freq, next_horiz_freq;
#endif

	double m_frames_per_sec;

	int m_hz_total, m_hz_start, m_hz_disp;
	int m_hs_start, m_hs_end;	///< H-SYNC start and end
//	int m_h_count;

	int m_vt_total, m_vt_start, m_vt_disp;
	int m_vt_total_per_frame;
	double m_vt_frames_per_sec;
	int m_vs_start, m_vs_end;	///< V-SYNC start and end
	int m_v_count;
	int m_v_count_delay;

	int m_hz_disp_end_clock;
	int m_hs_start_clock, m_hs_end_clock;

	bool m_now_raster, m_now_vdisp, m_now_vsync, m_now_hsync;

#ifdef USE_CRTC_MARA
	int m_memory_address;			///< refresh memory address(MA)
	int m_raster_address;			///< raster address (RA)
	int max_raster_address;			///< max raster address
#endif

	int mv_display_width;	
	int mv_display_height;	

//	int m_interlace;				///< 0:noninterlace 1:interlace
//	int m_raster_per_dot;			///< raster per dot (set 1 on hireso 256line)

	uint8_t m_sysport_hrl;		///< Hireso clock select on systemport R4 bit1 

	int register_id[3];

#pragma pack(1)
	struct vm_state_st {
		uint16_t m_regs[24];
		// 3
		uint16_t m_vram_accs;	///< CRTC operation port
		uint16_t m_trig_vram_accs;
		int m_intr_vline;		///< rater interrupt
		int m_gcls_count;		///< high speed clear count
		uint32_t reserved1;
		// 4
		uint64_t m_now_dot_clocks;
		uint64_t m_frames_per_sec;
		// 5
		int horiz_freq;
		int next_horiz_freq;
		int m_cpu_clocks;
		int m_hz_total;
		// 6
		int m_hz_start;
		int m_hz_disp;
		int m_hs_start;
		int m_hs_end;
		// 7
		int m_vt_start;
		int m_vt_total;
		int m_vt_disp;
		int m_vt_total_per_frame;
		// 8
		int m_vs_start;
		int m_vs_end;
		int m_v_count;
		int m_hz_disp_end_clock;
		// 9
		int m_hs_start_clock;
		int m_hs_end_clock;
		uint64_t m_vt_frames_per_sec;
		// 10
		uint8_t m_now_raster;
		uint8_t m_now_vdisp;
		uint8_t m_now_vsync;
		uint8_t m_now_hsync;
		uint8_t m_sysport_hrl;
		char reserved2[3];
		int m_memory_address;			///< refresh memory address(MA)
		int m_raster_address;			///< raster address (RA)
		// 11
		int max_raster_address;			///< max raster address
		int mv_display_width;
		int mv_display_height;	
		int m_interlace;				///< 0:noninterlace 1:interlace
		// 12
		int m_raster_per_dot;			///< raster per dot (set 1 on hireso 256line)
		int register_id[3];
	};
#pragma pack()

//	void set_display(bool val);
	void set_raster(bool val);
	void set_vdisp(bool val);
	void set_vsync(bool val);
	void set_hsync(bool val);

//	void set_interlace_and_skew(uint8_t data);
	void set_max_raster_address(uint16_t data);
//	inline void set_display_cursor();
	inline void process_command();
	void raster_copy_tvram();
	void clear_gvram();
	void update_raster_intr();

	inline void set_vertical_params();

public:
	CRTC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("CRTC");
		init_output_signals(&outputs_raster);
		init_output_signals(&outputs_vdisp);
		init_output_signals(&outputs_vsync);
		init_output_signals(&outputs_hsync);
		init_output_signals(&outputs_chsize);
	}
	~CRTC() {}

	// common functions
	void initialize();
	void reset();
	void write_io16(uint32_t addr, uint32_t data);
	void write_io_m(uint32_t addr, uint32_t data, uint32_t mask);
	uint32_t read_io16(uint32_t addr);
	uint32_t read_io_m(uint32_t addr, uint32_t mask);
	void event_pre_frame();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void update_config();
	void update_raster();

	// unique function
	void set_context_raster(DEVICE* device, int id, uint32_t mask, uint32_t negative = 0) {
		register_output_signal(&outputs_raster, device, id, mask, negative);
	}
	void set_context_vdisp(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_vdisp, device, id, mask);
	}
	void set_context_vsync(DEVICE* device, int id, uint32_t mask, uint32_t negative = 0) {
		register_output_signal(&outputs_vsync, device, id, mask, negative);
	}
	void set_context_hsync(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_hsync, device, id, mask);
	}
	void set_context_change_size(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_chsize, device, id, mask);
	}
#ifdef CRTC_HORIZ_FREQ
	void set_horiz_freq(int freq) {
		next_horiz_freq = freq;
	}
#endif
	uint16_t* get_regs() {
		return m_regs;
	}
	int* get_vt_total_ptr() {
		return &m_vt_total;
	}
	int* get_vt_count_ptr() {
		return &m_v_count;
	}
	int* get_vt_disp_ptr() {
		return &m_vt_disp;
	}
	int* get_hz_total_ptr() {
		return &m_hz_total;
	}
//	int* get_hz_count_ptr() {
//		return &m_h_count;
//	}
	int* get_hz_disp_ptr() {
		return &m_hz_disp;
	}
#ifdef USE_CRTC_MARA
	int* get_ma_ptr() {
		return &m_memory_address;
	}
	int* get_ra_ptr() {
		return &m_raster_address;
	}
	int* get_max_ra_ptr() {
		return &max_raster_address;
	}
#endif
	int* get_vs_start_ptr() {
		return &m_vs_start;
	}
	int* get_vs_end_ptr() {
		return &m_vs_end;
	}
	void set_vtram_ptr(uint16_t *ptr, uint8_t *ptru) {
		p_tvram = ptr;
		pu_tvram = ptru;
	}
	void set_gtram_ptr(uint16_t *ptr /*, uint8_t *ptru */) {
		p_gvram = ptr;
//		pu_gvram = ptru;
	}
	uint32_t get_led_status();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io16(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* CRTC_H */

