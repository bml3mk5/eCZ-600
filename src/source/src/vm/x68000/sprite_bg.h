/** @file sprite_bg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ SPRITE_BG ]
*/

#ifndef SPRITE_BG_H
#define SPRITE_BG_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief SPRITE_BG - CRT Controller
*/
class SPRITE_BG : public DEVICE
{
public:
	enum en_sp_reg_names {
		SP_XPOS = 0,
		SP_YPOS,
		SP_PTN,
		SP_PRW
	};

	enum en_sp_masks {
//		SP_PTN_HV_MASK = 0xc000,
//		SP_PTN_HV_SFT = 14,
//		SP_PTN_COLOR_MASK = 0x0f00,
//		SP_PTN_COLOR_SFT = 8,
		SP_PRW_MASK = 0x0003,
		SP_PRW_UPD_MASK = 0x0080,	// emu original
	};
	enum en_bg_reg_names {
		BG0_SCROLL_X = 0,
		BG0_SCROLL_Y,
		BG1_SCROLL_X,
		BG1_SCROLL_Y,
		BG_CONTROL,
		BG_HORI_TOTAL,
		BG_HORI_DISP,
		BG_VERT_DISP,
		BG_RESOLUTION,
	};
	enum en_bg_control_names {
		CONT_DISP_CPU	  = 0x0200,
		CONT_DISP_CPU_SFT = 9,
		CONT_BG1_AREA_SEL = 0x0010,
		CONT_BG1_AREA_SFT = 4,
		CONT_BG1_ON		  = 0x0008,
		CONT_BG1_ON_SFT	  = 3,
		CONT_BG0_AREA_SEL = 0x0002,
		CONT_BG0_AREA_SFT = 1,
		CONT_BG0_ON		  = 0x0001,
		CONT_BG0_ON_SFT   = 0,
	};
	enum en_bg_resolution_names {
		RESO_HRES = 0x0003,
		RESO_HRES512 = 0x0001,
		RESO_HRES_SFT = 0,
		RESO_VRES = 0x000c,
		RESO_VRES512 = 0x0004,
		RESO_VRES_SFT = 2,
		RESO_HVRES_INVALID = 0x000a,
		RESO_LHFREQ = 0x0010,
		RESO_LHFREQ_SFT = 4,
	};

private:
//	// output signals
//	outputs_t outputs_disp;

	uint16_t m_sp_regs[128 * 4];	// sprite register 128pattern x 4
	uint16_t m_bg_regs[9];		// bg register / screen mode register

	static const uint16_t c_sp_reg_masks[4];
	static const uint16_t c_bg_reg_masks[9];

#pragma pack(1)
	struct vm_state_st {
		uint16_t m_sp_regs[128 * 4];	// sprite register 128pattern x 4
		uint16_t m_bg_regs[9];		// bg register / screen mode register
		uint16_t reserved[7];
	};
#pragma pack()

public:
	SPRITE_BG(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("SPRITE_BG");
//		init_output_signals(&outputs_disp);
	}
	~SPRITE_BG() {}

	// common functions
	void initialize();
	void reset();
	void write_io16(uint32_t addr, uint32_t data);
	void write_io_m(uint32_t addr, uint32_t data, uint32_t mask);
	uint32_t read_io16(uint32_t addr);
	uint32_t read_io_m(uint32_t addr, uint32_t mask);
	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique function
	uint16_t* get_sp_regs() {
		return m_sp_regs;
	}
	uint16_t* get_bg_regs() {
		return m_bg_regs;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io16(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* SPRITE_BG_H */

