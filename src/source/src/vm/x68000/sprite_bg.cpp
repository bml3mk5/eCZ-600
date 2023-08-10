/** @file sprite_bg.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ SPRITE_BG ]
*/

#include "sprite_bg.h"
//#include "../../emu.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"

//#define OUT_DEBUG1 logging->out_debugf
#define OUT_DEBUG1(...)
//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)

#define STORE_DATA(mem, dat, msk) mem = ((mem & (msk)) | (dat))

const uint16_t SPRITE_BG::c_sp_reg_masks[4] = {
	0x03ff, 0x03ff, 0xcfff, 0x0007
};

const uint16_t SPRITE_BG::c_bg_reg_masks[9] = {
	0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x063f, 0x00ff, 0x003f, 0x00ff, 0x001f
};

void SPRITE_BG::initialize()
{
	memset(m_sp_regs, 0, sizeof(m_sp_regs));
	memset(m_bg_regs, 0, sizeof(m_bg_regs));
}

void SPRITE_BG::reset()
{
	memset(m_sp_regs, 0, sizeof(m_sp_regs));
	memset(m_bg_regs, 0, sizeof(m_bg_regs));
}

void SPRITE_BG::write_io16(uint32_t addr, uint32_t data)
{
	write_io_m(addr, data, 0xffff0000);
}

void SPRITE_BG::write_io_m(uint32_t addr, uint32_t data, uint32_t mask)
{
	uint32_t addrh = (addr >> 1) & 0x7ff;
	int pos;

	switch(addrh & 0x700) {
	case 0x000:
	case 0x100:
		// sprite register
		pos = (addrh & 0x1ff);
		STORE_DATA(m_sp_regs[pos], data & c_sp_reg_masks[pos & 3], mask);
#ifdef USE_SPRITE_RENDER
		m_sp_regs[pos | 3] |= SP_PRW_UPD_MASK;	// update flag
#endif
		OUT_DEBUG_REGW(_T("%06X: SP%03d-%d: data:%04X mask:%04X result:%04X"), addr, pos >> 2, pos & 3, data, mask, m_sp_regs[pos]);
		break;
	case 0x400:
		// bg and control register
		pos = (addrh & 0xf);
		if (pos < 9) {
			STORE_DATA(m_bg_regs[pos], data & c_bg_reg_masks[pos], mask);
			OUT_DEBUG_REGW(_T("%06X: BG%d: %04X"), addr, pos, m_bg_regs[pos]);
		}
		break;
	}
}

uint32_t SPRITE_BG::read_io16(uint32_t addr)
{
	uint32_t data = 0xffff;
	uint32_t addrh = (addr >> 1) & 0x7ff;

	int pos;

	switch(addrh & 0x700) {
	case 0x000:
	case 0x100:
		// sprite register
		pos = (addrh & 0x1ff);
		data = m_sp_regs[pos];
		break;
	case 0x400:
		// bg and control register
		pos = (addrh & 0xf);
		if (pos < 9) data = m_bg_regs[pos];
		break;
	}
	return data;
}

uint32_t SPRITE_BG::read_io_m(uint32_t addr, uint32_t mask)
{
	return read_io16(addr);
}

void SPRITE_BG::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		reset();
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

void SPRITE_BG::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<128*4; i++) {
		SET_Uint16_LE(m_sp_regs[i]);	// sprite register 128pattern x 4
	}
	for(int i=0; i<9; i++) {
		SET_Uint16_LE(m_bg_regs[i]);		// bg register / screen mode register
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

bool SPRITE_BG::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	for(int i=0; i<128*4; i++) {
		GET_Uint16_LE(m_sp_regs[i]);	// sprite register 128pattern x 4
	}
	for(int i=0; i<9; i++) {
		GET_Uint16_LE(m_bg_regs[i]);		// bg register / screen mode register
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t SPRITE_BG::debug_read_io16(uint32_t addr)
{
	uint32_t data = 0xffff;
	uint32_t addrh = (addr >> 1) & 0x7ff;

	int pos;

	switch(addrh & 0x700) {
	case 0x000:
	case 0x100:
		// sprite register
		pos = (addrh & 0x1ff);
		data = m_sp_regs[pos];
		break;
	case 0x400:
		pos = (addrh & 0xf);
		if (pos < 9) data = m_bg_regs[pos];
		break;
	}
	return data;
}

bool SPRITE_BG::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool SPRITE_BG::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

static const _TCHAR *c_reg_bg_names[] = {
	_T("BG0_SCRL_X"),
	_T("BG0_SCRL_Y"),
	_T("BG1_SCRL_X"),
	_T("BG1_SCRL_Y"),
	_T("BG_CONTROL"),
	_T("HTOTAL"),
	_T("HDISP"),
	_T("VDISP"),
	_T("FREQ"),
	NULL
};

static const _TCHAR *c_r8_reso[] = {
	_T("Normal"),
	_T("Hireso"),
	NULL
};

void SPRITE_BG::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR str[40];

	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("SP:\n"));
	for(int num=0; num<128; num++) {
		UTILITY::stprintf(str, sizeof(str) / sizeof(_TCHAR), _T(" %02X: X:%04X Y:%04X COL:%04X PRI:%04X")
			, num
			, m_sp_regs[(num << 2)]
			, m_sp_regs[(num << 2) + 1]
			, m_sp_regs[(num << 2) + 2]
			, m_sp_regs[(num << 2) + 3] & 0xff
		);
		if ((num % 2) == 1) {
			UTILITY::tcscat(str, sizeof(str) / sizeof(_TCHAR), _T("\n"));
		}
		UTILITY::tcscat(buffer, buffer_len, str);
	}
	UTILITY::tcscat(buffer, buffer_len, _T("BG:\n"));

	for(int pos=0; pos<9; pos++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" %d(%-10s):%04X")
			, pos, c_reg_bg_names[pos], m_bg_regs[pos]
		);
		switch(pos) {
		case 4:
			UTILITY::sntprintf(buffer, buffer_len, _T(" DISP/CPU:%d BG0:%d(Page:%d) BG1:%d(Page:%d)\n")
				, (m_bg_regs[pos] & CONT_DISP_CPU) >> CONT_DISP_CPU_SFT
				, (m_bg_regs[pos] & CONT_BG0_ON) >> CONT_BG0_ON_SFT
				, (m_bg_regs[pos] & CONT_BG0_AREA_SEL) >> CONT_BG0_AREA_SFT
				, (m_bg_regs[pos] & CONT_BG1_ON) >> CONT_BG1_ON_SFT
				, (m_bg_regs[pos] & CONT_BG1_AREA_SEL) >> CONT_BG1_AREA_SFT
			);
			break;
		case 8:
			{
				int f = (m_bg_regs[pos] & RESO_LHFREQ) >> RESO_LHFREQ_SFT;
				int h = (((m_bg_regs[pos] & RESO_HRES) >> RESO_HRES_SFT) + 1) * 256;
				int v = (((m_bg_regs[pos] & RESO_VRES) >> RESO_VRES_SFT) + 1) * 256;
				UTILITY::sntprintf(buffer, buffer_len, _T(" %s %dx%d %s\n")
					, c_r8_reso[f]
					, h
					, v
					, ((h + v) > 1024) ? _T("Invalid") : _T("")
				);
			}
			break;
		default:
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
			break;
		}
	}
}
#endif
