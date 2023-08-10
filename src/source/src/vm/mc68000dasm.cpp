/** @file mc68000dasm.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.01.01 -

	@brief [ mc68000 disassembler ]
*/

#include "mc68000dasm.h"
#include "device.h"
#include "mc68000_consts.h"

#ifdef USE_DEBUGGER

#include "debugger.h"
#include "../utility.h"

#ifdef USE_MC68000DASM_PHYADDR
#define GET_PHYADDR(mem, pc) (mem ? mem->debug_latch_address(pc) : 0)
#else
#define GET_PHYADDR(mem, pc) (pc)
#endif

//#ifndef ADDR_FORMAT
//#define ADDR_FORMAT "%06X"
//#endif
//#ifndef ADDR_FORMAT_S
//#define ADDR_FORMAT_S "$%06X"
//#endif
//#ifndef ADDR_FORMAT_MASK
//#define ADDR_FORMAT_MASK(x) ((x) & 0xffffff)
//#endif

//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM_DATA::MC68000DASM_DATA()
{
	clear();
}
MC68000DASM_DATA::MC68000DASM_DATA(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	set(phyaddr_, addr_, fc_, data_, write_, addflags_);
}
void MC68000DASM_DATA::clear()
{
	m_phyaddr = 0;
	m_addr = 0;
	m_data = 0;
	m_fc = 0;
	m_flags = 0;
}
void MC68000DASM_DATA::set(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	m_addr = addr_;
	m_data = data_;
	m_fc = fc_;
	m_flags &= (~FLAG_MEMALL);
	m_flags |= (FLAG_STORE);
	m_flags |= addflags_;
	if (write_) m_flags |= FLAG_WRITE;
}

//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM_DATAS::MC68000DASM_DATAS()
{
	m_len = 0;
}
void MC68000DASM_DATAS::clear()
{
	m_len = 0;
}
void MC68000DASM_DATAS::set_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	m_datas[0].set(phyaddr_, addr_, fc_, data_, write_, addflags_);
	if (m_len <= 0) m_len = 1;
}
void MC68000DASM_DATAS::add_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	if (m_len >= 2) return;
	m_datas[m_len].set(phyaddr_, addr_, fc_, data_, write_, addflags_);
	m_len++;
}

//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM_REGPTR::MC68000DASM_REGPTR()
{
	p_pc = 0;
	p_dar = 0;
	p_sp = 0;
#ifdef USE_MC68000VBR
	p_vbr = 0;
	p_sfc = 0;
	p_dfc = 0;
#endif
#ifdef USE_MC68000CACHE
	p_cacr = 0;
	p_caar = 0;
#endif
#ifdef USE_MC68000FPU
	p_fpiar = 0;
	p_fpsr = 0;
	p_fpcr = 0;
#endif
	p_signals = 0;
}
void MC68000DASM_REGPTR::set_regptrs(
	uint32_t *n_pc,
	uint32_t *n_dar,
	uint32_t *n_sp,
#ifdef USE_MC68000VBR
	uint32_t *n_vbr,
	uint32_t *n_sfc,
	uint32_t *n_dfc,
#endif
#ifdef USE_MC68000CACHE
	uint32_t *n_cacr,
	uint32_t *n_caar,
#endif
#ifdef USE_MC68000FPU
	uint32_t *n_fpiar,
	uint32_t *n_fpsr,
	uint32_t *n_fpcr,
#endif
	uint32_t *n_signals
)
{
	p_pc = n_pc;
	p_dar = n_dar;
	p_sp = n_sp;
#ifdef USE_MC68000VBR
	p_vbr = n_vbr;
	p_sfc = n_sfc;
	p_dfc = n_dfc;
#endif
#ifdef USE_MC68000CACHE
	p_cacr = n_cacr;
	p_caar = n_caar;
#endif
#ifdef USE_MC68000FPU
	p_fpiar = n_fpiar;
	p_fpsr = n_fpsr;
	p_fpcr = n_fpcr;
#endif
	p_signals = n_signals;
}

//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM_REG::MC68000DASM_REG()
{
	clear();
}
MC68000DASM_REG::MC68000DASM_REG(uint32_t phyaddr_, uint32_t addr_, uint32_t pc_, const uint32_t *dar_, const uint32_t *sp_, uint16_t sr_, uint32_t signals_)
{
	clear();
	set(phyaddr_, addr_, pc_, dar_, sp_, sr_, signals_);
}
void MC68000DASM_REG::clear()
{
	phyaddr = 0;
	addr = 0;

	for(int i=0; i<CODELEN_MAX; i++) {
		ops[i] = 0;
	}
	opslen = 0;

	pc = 0;
	for(int i=0; i<16; i++) {
		dar[i] = 0;
	}
	for(int i=0; i<4; i++) {
		sp[i] = 0;
	}
	sr = 0;

	cycles = 0;

#ifdef USE_MC68000VBR
	vbr = 0;
	sfc = 0;
	dfc = 0;
#endif

#ifdef USE_MC68000CACHE
	cacr = 0;
	caar = 0;
#endif

	isignals = 0;

	category = CATEGORY_INVALID;
	vec_num = 0;

}
void MC68000DASM_REG::set(uint32_t phyaddr_, uint32_t addr_)
{
	phyaddr = phyaddr_;
	addr    = addr_;

	opslen  = 0;
	isignals = 0;
	cycles  = 0;
	rw.clear();
	category = CATEGORY_NORMAL;	
	vec_num = 0;
}
void MC68000DASM_REG::set(uint32_t phyaddr_, uint32_t addr_, uint32_t pc_, const uint32_t *dar_, const uint32_t *sp_, uint16_t sr_, uint32_t signals_)
{
	phyaddr = phyaddr_;
	addr    = addr_;

	pc = pc_;
	for(int i=0; i<16; i++) {
		dar[i] = dar_[i];
	}
	for(int i=0; i<4; i++) {
		sp[i] = sp_[i];
	}
	sr = sr_;
	isignals = signals_;
}
#if 0
void MC68000DASM_REG::set_regs(int accum_, int cycles_, uint32_t pc_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	cycles = cycles_;

	pc      = pc_;
	for(int i=0; i<16; i++) {
		dar[i] = ptr_.p_dar[i];
	}
	for(int i=0; i<4; i++) {
		sp[i] = ptr_.p_sp[i];
	}
	sr = sr_;
	signals |= *ptr_.p_signals;	// add signal
}
#endif
void MC68000DASM_REG::set_regs(int accum_, int cycles_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	cycles = cycles_;
	pc = *ptr_.p_pc;
	for(int i=0; i<16; i++) {
		dar[i] = ptr_.p_dar[i];
	}
	for(int i=0; i<4; i++) {
		sp[i] = ptr_.p_sp[i];
	}
	sr = sr_;
	isignals |= *ptr_.p_signals;	// add signal
}
void MC68000DASM_REG::set_regs(uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	for(int i=0; i<16; i++) {
		dar[i] = ptr_.p_dar[i];
	}
	for(int i=0; i<4; i++) {
		sp[i] = ptr_.p_sp[i];
	}
	sr = sr_;
	isignals = *ptr_.p_signals;
}
bool MC68000DASM_REG::add_code(uint32_t code_)
{
	if (opslen >= CODELEN_MAX) return false;
	ops[opslen] = code_;
	opslen++;
	return true;
}
void MC68000DASM_REG::set_cycles(int accum_, int cycles_)
{
	cycles = cycles_;
}
void MC68000DASM_REG::set_reset(uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	category = CATEGORY_RESET;
	set_regs(sr_, ptr_);
}
void MC68000DASM_REG::set_busreq(uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	category = CATEGORY_BUSREQ;
	set_regs(sr_, ptr_);
}
void MC68000DASM_REG::set_halt(uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	category = CATEGORY_HALT;
	set_regs(sr_, ptr_);
}
void MC68000DASM_REG::set_pref(uint32_t code_, int cycles_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	category = CATEGORY_PREF;
	add_code(code_);
	set_regs(0, cycles_, sr_, ptr_);
}
void MC68000DASM_REG::set_aerr()
{
	category = CATEGORY_AERROR;
}
void MC68000DASM_REG::set_berr(uint32_t code_)
{
	category = CATEGORY_BERROR;

	if (opslen == 0) opslen++;
	ops[opslen-1] =code_;
}
void MC68000DASM_REG::set_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	rw.set_data(phyaddr_, addr_, fc_, data_, write_, addflags_);
}
void MC68000DASM_REG::add_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_)
{
	rw.add_data(phyaddr_, addr_, fc_, data_, write_, addflags_);
}
void MC68000DASM_REG::set_flag_vector(uint16_t category_, uint16_t vec_num_)
{
	category = CATEGORY_EXCEPTION;
	vec_num = (category_ << 8) | vec_num_;
}
void MC68000DASM_REG::set_flag_intvec(uint16_t sr_, const MC68000DASM_REGPTR &ptr_)
{
	category = CATEGORY_INTR_VECNUM;
	set_regs(sr_, ptr_);
}
#if 0
void MC68000DASM_REG::set_flag_resvec()
{
	category = CATEGORY_RESVEC;
}
#endif

//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM_REGS::MC68000DASM_REGS()
{
	// register stack
	current_idx = 0;
	current_reg = &regs[current_idx];
}
void MC68000DASM_REGS::push_stack(uint32_t phyaddr, uint32_t addr)
{
//	MC68000DASM_REG *prev_reg = current_reg;

	current_idx = (current_idx + 1) % MC68000DASM_PCSTACK_NUM;

	current_reg = &regs[current_idx];

	current_reg->set(phyaddr, addr);
}
int MC68000DASM_REGS::get_stack(int index, MC68000DASM_REG &stack)
{
	if (index < 0) return -2;
	if (index >= MC68000DASM_PCSTACK_NUM) index = MC68000DASM_PCSTACK_NUM - 1;

	int reg_index = (current_idx - index + MC68000DASM_PCSTACK_NUM) % MC68000DASM_PCSTACK_NUM;
	bool exist = false;
	do {
		if (regs[reg_index].is_valid()) {
			exist = true;
			break;
		} 
		reg_index++;
		index--;
		if (reg_index >= MC68000DASM_PCSTACK_NUM) reg_index = 0;
	} while(reg_index != current_idx);
	if (!exist) return -2;

	stack = regs[reg_index];

	index--;
	return index;
}


//////////////////////////////////////////////////////////////////////
//
//

MC68000DASM::MC68000DASM()
{
	d_cpu = NULL;
	m_lower_case = true;

	UTILITY::tcscpy(m_addr_format, 8, _T("%06X"));
	UTILITY::tcscpy(m_addr_format_s, 8, _T("$%06X"));
	m_addr_mask = 0xffffff;

	build_opcode_table();
}

MC68000DASM::~MC68000DASM()
{
}

void MC68000DASM::set_context_cpu(DEVICE* device)
{
	d_cpu = device;
	if (d_cpu) {
		m_addr_mask = d_cpu->debug_prog_addr_mask();
		uint32_t mask = m_addr_mask;
		int n;
		for(n=0; n<8; n++) {
			if (!(mask & 0xf)) {
				break;
			}
			mask >>= 4;
		}
		if (n < 4) n = 4;
		UTILITY::stprintf(m_addr_format, 8, _T("%%0%dX"), n);
		UTILITY::stprintf(m_addr_format_s, 8, _T("$%%0%dX"), n);
	}
}

void MC68000DASM::push_addr(uint32_t addr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
}

//void MC68000DASM::set_pc(uint32_t pc)
//{
//}

void MC68000DASM::add_code(uint16_t code)
{
	current_reg->add_code(code);
}

void MC68000DASM::push_vector(uint16_t category, uint16_t vec_num, uint32_t addr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_flag_vector(category, vec_num);
}

void MC68000DASM::push_intvec(uint32_t addr, uint16_t sr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_flag_intvec(sr, *this);
}

#if 0
void MC68000DASM::push_resvec(uint32_t addr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_flag_resvec();
}
#endif

void MC68000DASM::push_reset(uint16_t sr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, 0);
	push_stack(inphyaddr, 0);
	current_reg->set_reset(sr, *this);
}

void MC68000DASM::push_busreq(uint32_t addr, uint16_t sr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_busreq(sr, *this);
}

void MC68000DASM::push_halt(uint16_t sr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, 0);
	push_stack(inphyaddr, 0);
	current_reg->set_halt(sr, *this);
}

void MC68000DASM::push_pref(uint32_t addr, uint32_t code, int cycles, uint16_t sr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_pref(code, cycles, sr, *this);
}

void MC68000DASM::push_aerr(uint32_t addr)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_aerr();
}

void MC68000DASM::push_aerr(uint32_t addr, uint16_t fc, uint8_t data, bool write)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_aerr();
	current_reg->set_data(inphyaddr, addr, fc, data, write, FLAG_2BYTES);
}

void MC68000DASM::set_aerr()
{
	current_reg->set_aerr();
}
void MC68000DASM::push_berr(uint32_t addr, uint32_t code)
{
	uint32_t inphyaddr = GET_PHYADDR(d_mem, addr);
	push_stack(inphyaddr, addr);
	current_reg->set_berr(code);
}
void MC68000DASM::set_signals(uint32_t sig)
{
	current_reg->set_signals(sig);
}
void MC68000DASM::add_signals(uint32_t sig)
{
	current_reg->add_signals(sig);
}

void MC68000DASM::set_mem(uint32_t addr, uint16_t fc, uint8_t data, bool write)
{
	current_reg->set_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_1BYTE);
}

void MC68000DASM::set_mem(uint32_t addr, uint16_t fc, uint16_t data, bool write)
{
	current_reg->set_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_2BYTES);
}

void MC68000DASM::set_mem(uint32_t addr, uint16_t fc, uint32_t data, bool write)
{
	current_reg->set_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_4BYTES);
}

void MC68000DASM::add_mem(uint32_t addr, uint16_t fc, uint8_t data, bool write)
{
	current_reg->add_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_1BYTE);
}

void MC68000DASM::add_mem(uint32_t addr, uint16_t fc, uint16_t data, bool write)
{
	current_reg->add_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_2BYTES);
}

void MC68000DASM::add_mem(uint32_t addr, uint16_t fc, uint32_t data, bool write)
{
	current_reg->add_data(GET_PHYADDR(d_mem, addr), addr, fc, data, write, FLAG_4BYTES);
}

//void MC68000DASM::set_phymem(uint32_t phyaddr)
//{
//	current_reg->set_rw_phyaddr(phyaddr);
//}

void MC68000DASM::set_reg(int accum, int cycles, uint16_t sr)
{
	current_reg->set_regs(accum, cycles, sr, *this);
}

void MC68000DASM::update_reg(uint16_t sr)
{
	current_reg->set_regs(sr, *this);
}

void MC68000DASM::set_cycles(int accum, int cycles)
{
	current_reg->set_cycles(accum, cycles);
}

const _TCHAR *MC68000DASM::m_except[] = {
	_T("Stack Pointer"),
	_T("Program Counter"),
	_T("Bus Error"),
	_T("Address Error"),
	_T("Illegal Instruction"),
	_T("Divide By Zero"),
	_T("CHK Instruction"),
	_T("TRAPV Instruction"),
	_T("Privilege Violation"),
	_T("Trace Exception"),
	_T("1010 Exception"),
	_T("1111 Exception"),
	_T("Format Error"),
	_T("Interrupt"),
	_T("TRAP Instruction"),
	_T("Exception"),
	0
};

int MC68000DASM::print_dasm(const MC68000DASM_REG &reg)
{
	return print_dasm(reg.get_phyaddr(), reg.get_addr(), reg.get_ops(), reg.get_opslen(), reg.get_category(), reg.get_vec_num());
}
/// @param [in] phyaddr  : physical address converted from start address
/// @param [in] addr     : start address of op code
/// @param [in] ops      : op code
/// @param [in] opslen   : op code length
/// @param [in] category : category
int MC68000DASM::print_dasm(uint32_t phyaddr, uint32_t addr, const uint16_t *ops, int opslen, uint16_t category, uint16_t vec_num)
{
	int opspos = MC68000DASM_REG::CODELEN_SHOW;

	cmd[0] = 0;
	switch(category) {
	case CATEGORY_EXCEPTION:
		// read vector address
		UTILITY::sntprintf(cmd, CMDLINE_LEN, _T("; %s: Read vector%d")
			, m_except[(vec_num >> 8) & 0xff], vec_num & 0xff);
		break;
	case CATEGORY_INTR_VECNUM:
		// read vector number
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Interrput: Read vector number"));
		break;
	case CATEGORY_AERROR:
		// address error
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Address error"));
		break;
	case CATEGORY_BERROR:
		// bus error
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Bus error"));
		break;
	case CATEGORY_RESET:
		// reset
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Reset"));
		break;
	case CATEGORY_BUSREQ:
		// bus request
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Bus request"));
		break;
	case CATEGORY_HALT:
		// halt
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Halt"));
		break;
	case CATEGORY_PREF:
		// prefetch
		UTILITY::tcscat(cmd, CMDLINE_LEN, _T("; Prefetch"));
		break;
	default:
		// disassemble
		opslen = make_cmd_str(addr, ops, opslen, cmd, CMDLINE_LEN);
		break;
	}

	// address
	UTILITY::stprintf(line, sizeof(line) / sizeof(line[0]), m_addr_format, addr & m_addr_mask);

	// operands
	if (opspos < opslen) opspos = opslen;
	for(int i=0; i < opspos; i++) {
		if (i < opslen) {
			UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T(" %04X"), ops[i]);
		} else {
			UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T("     "));
		}
	}

	// mnemonic
	UTILITY::sntprintf(line, sizeof(line) / sizeof(line[0]), _T(" %-24s"), cmd);

	return opslen;
}

int MC68000DASM::print_dasm_label(int type, uint32_t addr)
{
	const _TCHAR *label = debugger->find_symbol_name(addr);
	if (label) {
		UTILITY::tcscpy(line, sizeof(line) / sizeof(line[0]), _T("              "));
		UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), label);
		UTILITY::tcscat(line, sizeof(line) / sizeof(line[0]), _T(":"));
		return 1;
	}
	return 0;
}

int MC68000DASM::print_dasm_preprocess(int type, uint32_t addr, int flags)
{
	// next opcode
	uint16_t ops[MC68000DASM_REG::CODELEN_MAX];
	uint32_t phyaddr = 0;

	phyaddr = addr;
	for(int i = 0; i < MC68000DASM_REG::CODELEN_MAX; i++) {
		ops[i] = d_cpu->debug_read_data16(type, addr + i * 2);
	}

	line[0]=0;
	int opspos = print_dasm(phyaddr, addr, ops, MC68000DASM_REG::CODELEN_MAX, CATEGORY_NORMAL, 0);
	return opspos;
}

int MC68000DASM::print_dasm_processed()
{
	line[0]=0;
	int opspos = print_dasm(*current_reg);
	print_cycles(current_reg->get_cycles());
	print_memory_datas(current_reg->get_rwdata());
	return opspos;
}

int MC68000DASM::print_dasm_traceback(int index)
{
	MC68000DASM_REG stack;
	int next = get_stack(index, stack);
	if (next >= -1) {
		line[0]=0;
		print_dasm(stack);
		print_cycles(stack.get_cycles());
		print_memory_datas(stack.get_rwdata());
	}
	return next;
}

void MC68000DASM::print_cycles(int cycles)
{
	// expended machine cycles 
	UTILITY::sntprintf(line, LINE_LEN, _T(" ; (%2d)")
		,cycles
	);
}

const _TCHAR MC68000DASM::m_tr_table[] = _T("Tt");
const _TCHAR MC68000DASM::m_sr_table[] = _T("XNZVC");
const _TCHAR MC68000DASM::m_su_table[] = _T("U-S-U-IM");
const _TCHAR MC68000DASM::m_si_table[] = _T("b210EsAQVBhH");

void MC68000DASM::print_regs(const MC68000DASM_REG &reg)
{
	// register
	line[0] = 0;
	for(int i = 0; i < 8; i++) {
		if (i > 0) UTILITY::tcscat(line, LINE_LEN, _T(" "));
		UTILITY::sntprintf(line, LINE_LEN, _T("D%d:%08X"), i, reg.dar[i]);
	}
	UTILITY::tcscat(line, LINE_LEN, _T("\n"));
	for(int i = 0; i < 8; i++) {
		if (i > 0) UTILITY::tcscat(line, LINE_LEN, _T(" "));
		UTILITY::sntprintf(line, LINE_LEN, _T("A%d:%08X"), i, reg.dar[i + 8]);
	}
	UTILITY::tcscat(line, LINE_LEN, _T("\n"));
	UTILITY::sntprintf(line, LINE_LEN, _T("PC:%08X USP:%08X"), reg.pc, reg.sp[0]);	// USP
	if (CPU_TYPE_IS_020_PLUS()) {
		UTILITY::sntprintf(line, LINE_LEN, _T(" ISP:%08X MSP:%08X"), reg.sp[2], reg.sp[3]);	// ISP, MSP
	} else {
		UTILITY::sntprintf(line, LINE_LEN, _T(" SSP:%08X"), reg.sp[2]);	// SSP/ISP
	}
	UTILITY::sntprintf(line, LINE_LEN, _T(" SR:%04X"), reg.sr);

	int pos = (int)_tcslen(line);
	line[pos++] = _T(' '); line[pos++] = _T('[');

	// trace
	line[pos++] = (reg.sr & 0x8000) ? m_tr_table[0] : _T('-'); 
	line[pos++] = (reg.sr & 0x4000) ? m_tr_table[1] : _T('-'); 

	// status
	for(int i = 4; i >= 0; i--) {
		line[pos++] = (reg.sr & (1 << i)) ? m_sr_table[4 - i] : _T('-'); 
	}

	line[pos++] = _T(':');

	// super visor / user mode
	if (CPU_TYPE_IS_020_PLUS()) {
		line[pos++] = m_su_table[((reg.sr >> 12) & 3) + 4];
	} else {
		line[pos++] = m_su_table[(reg.sr >> 12) & 2]; 
	}

	line[pos++] = _T(':');

	// interrupt mask
	line[pos++] = _T('M');
	line[pos++] = (((reg.sr >> 8) & 7) + 0x30);

	line[pos++] = _T(':');

	// current signals
	for(int i = 11; i >= 0; i--) {
		line[pos++] = (reg.isignals & (1 << i)) ? m_si_table[11 - i] : _T('-');
	}
	line[pos++] = _T(']');
	line[pos++] = 0;

#ifdef USE_MC68000VBR
	UTILITY::tcscat(line, LINE_LEN, _T("\n"));
	UTILITY::sntprintf(line, LINE_LEN, _T("VBR:%08X"), reg.vbr);
	UTILITY::sntprintf(line, LINE_LEN, _T(" SFC:%d"), reg.sfc);
	UTILITY::sntprintf(line, LINE_LEN, _T(" DFC:%d"), reg.dfc);
#endif

#ifdef USE_MC68000CACHE
	UTILITY::sntprintf(line, LINE_LEN, _T(" CACR:%04X"), reg.cacr);
	UTILITY::sntprintf(line, LINE_LEN, _T(" CAAR:%04X"), reg.caar);
#endif
}

void MC68000DASM::print_regs_current()
{
	print_regs(*current_reg);
}

#if 0
void MC68000DASM::print_regs_current(uint32_t pc, const uint32_t *dar, const uint32_t *sp, uint16_t sr, uint32_t signals)
{
	print_regs(MC68000DASM_REG(pc, dar, sp, sr, signals));
}
#endif

int MC68000DASM::print_regs_traceback(int index)
{
	MC68000DASM_REG stack;
	int next = get_stack(index, stack);
	if (next >= -1) {
		print_regs(stack);
	}
	return next;
}

void MC68000DASM::print_memory_datas(const MC68000DASM_DATAS &rws)
{
	// memory data
	int count = rws.size();

	for(int i=0; i<count; i++) {
		const MC68000DASM_DATA *itm = &rws.at(i);

		if ((itm->m_flags & FLAG_STORE) == 0) continue;

		_TCHAR rw = (itm->m_flags & FLAG_WRITE) ? _T('W') : _T('R');
		_TCHAR az[12], dz[12];

		UTILITY::stprintf(az, sizeof(az) / sizeof(_TCHAR),m_addr_format, (itm->m_addr) & m_addr_mask);

		if ((itm->m_flags & FLAG_4BYTES) != 0) {
			UTILITY::stprintf(dz, sizeof(dz) / sizeof(_TCHAR), _T("%08X"), itm->m_data);
		} else if ((itm->m_flags & FLAG_2BYTES) != 0) {
			UTILITY::stprintf(dz, sizeof(dz) / sizeof(_TCHAR), _T("%04X"), itm->m_data & 0xffff);
		} else {
			UTILITY::stprintf(dz, sizeof(dz) / sizeof(_TCHAR), _T("%02X"), itm->m_data & 0xff);
		}

		UTILITY::sntprintf(line, LINE_LEN, _T(" (%c %d:%s:%s)"), rw, itm->m_fc & 7, az, dz);
	}
}

/**********************************************************************/

/* used by ops like asr, ror, addq, etc */
const uint32_t MC68000DASM::m_3bit_qdata_table[8] = {8, 1, 2, 3, 4, 5, 6, 7};

const uint32_t MC68000DASM::m_5bit_data_table[32] =
{
	32,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

const _TCHAR *MC68000DASM::m_bcc[16] = {
	_T("T"), _T("F"), _T("HI"), _T("LS"), _T("CC"), _T("CS"), _T("NE"), _T("EQ"),
	_T("VC"), _T("VS"), _T("PL"), _T("MI"), _T("GE"), _T("LT"), _T("GT"), _T("LE")
};
const _TCHAR *MC68000DASM::m_cpcc[64] =
{/* 000         001        010        011        100        101        110        111 */
	_T("F"),    _T("EQ"),  _T("OGT"), _T("OGE"), _T("OLT"), _T("OLE"), _T("OGL"), _T("OR"),  /* 000 */
	_T("UN"),   _T("UEQ"), _T("UGT"), _T("UGE"), _T("ULT"), _T("ULE"), _T("NE"),  _T("T"),   /* 001 */
	_T("SF"),   _T("SEQ"), _T("GT"),  _T("GE"),  _T("LT"),  _T("LE"),  _T("GL"),  _T("GLE"), /* 010 */
	_T("NGLE"), _T("NGL"), _T("NLE"), _T("NLT"), _T("NGE"), _T("NGT"), _T("SNE"), _T("ST"),  /* 011 */
	_T("???"),  _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), /* 100 */
	_T("???"),  _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), /* 101 */
	_T("???"),  _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), /* 110 */
	_T("???"),  _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???"), _T("???")  /* 111 */
};
const _TCHAR *MC68000DASM::btst_str[4] = {
	_T("BTST"), _T("BCHG"), _T("BCLR"), _T("BSET")
};

const _TCHAR *MC68000DASM::m_mmuregs[8] =
{
	_T("TC"), _T("DRP"), _T("SRP"), _T("CRP"), _T("CAL"), _T("VAL"), _T("SCCR"), _T("ACR")
};

const _TCHAR *MC68000DASM::m_mmucond[16] =
{
	_T("BS"), _T("BC"), _T("LS"), _T("LC"), _T("SS"), _T("SC"), _T("AS"), _T("AC"),
	_T("WS"), _T("WC"), _T("IS"), _T("IC"), _T("GS"), _T("GC"), _T("CS"), _T("CC")
};

/**********************************************************************/

int MC68000DASM::make_cmd_str(uint32_t addr, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 0;
	int insnum = instruction_table[ops[0]];
	m_bag.pc = addr; 
	opscnt = (this->*m_opcode_info[insnum].handler)(ops, opslen, str, len);

	return opscnt;
}

void MC68000DASM::make_comment_str(const _TCHAR *comment, _TCHAR *str, int len)
{
	UTILITY::tcscat(str, len, _T(" ; "));
	UTILITY::tcscat(str, len, comment);
}

void MC68000DASM::make_reg_direct_str(_TCHAR reg, uint8_t data, _TCHAR *str, int len)
{
	UTILITY::sntprintf(str, len, _T("%c%d"), reg, data);
}

void MC68000DASM::make_data_reg_direct_str(uint8_t data, _TCHAR *str, int len)
{
	make_reg_direct_str(_T('D'), data, str, len);
}

void MC68000DASM::make_address_reg_direct_str(uint8_t data, _TCHAR *str, int len)
{
	make_reg_direct_str(_T('A'), data, str, len);
}

void MC68000DASM::make_address_reg_indirect_str(uint8_t data, _TCHAR *str, int len)
{
	UTILITY::sntprintf(str, len, _T("(A%d)"), data);
}

void MC68000DASM::make_address_reg_indirect_postinc_str(uint8_t data, _TCHAR *str, int len)
{
	UTILITY::sntprintf(str, len, _T("(A%d)+"), data);
}

void MC68000DASM::make_address_reg_indirect_predec_str(uint8_t data, _TCHAR *str, int len)
{
	UTILITY::sntprintf(str, len, _T("-(A%d)"), data);
}

/// Rn, Rn parts
void MC68000DASM::make_reg_reg_direct_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	UTILITY::stprintf(wk[0], WKWORD_LEN, _T("%c%d"), reg1, d1);
	UTILITY::stprintf(wk[1], WKWORD_LEN, _T("%c%d"), reg2, d2);
	wk[2][0] = 0;
	join_words(str, len, wk);
}

/// (Rn), (Rn) parts
void MC68000DASM::make_reg_reg_indirect_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	UTILITY::stprintf(wk[0], WKWORD_LEN, _T("(%c%d)"), reg1, d1);
	UTILITY::stprintf(wk[1], WKWORD_LEN, _T("(%c%d)"), reg2, d2);
	wk[2][0] = 0;
	join_words(str, len, wk);
}

/// (Rn)+, (Rn)+ parts
void MC68000DASM::make_reg_reg_indirect_postinc_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	UTILITY::stprintf(wk[0], WKWORD_LEN, _T("(%c%d)+"), reg1, d1);
	UTILITY::stprintf(wk[1], WKWORD_LEN, _T("(%c%d)+"), reg2, d2);
	wk[2][0] = 0;
	join_words(str, len, wk);
}

/// -(Rn), -(Rn) parts
void MC68000DASM::make_reg_reg_indirect_predec_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	UTILITY::stprintf(wk[0], WKWORD_LEN, _T("-(%c%d)"), reg1, d1);
	UTILITY::stprintf(wk[1], WKWORD_LEN, _T("-(%c%d)"), reg2, d2);
	wk[2][0] = 0;
	join_words(str, len, wk);
}

int MC68000DASM::make_address_reg_indirect_displace_str(uint8_t data, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 0;
	_TCHAR wk[3][WKWORD_LEN];
	wk[0][0] = 0; UTILITY::tcscat(wk[0], WKWORD_LEN, _T("("));
	make_signed_str_16(ops[opscnt++], wk[0], WKWORD_LEN);
	wk[1][0] = 0; make_address_reg_direct_str(data,  wk[1], WKWORD_LEN);
	UTILITY::tcscat(wk[1], WKWORD_LEN, _T(")"));
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

uint32_t MC68000DASM::read_op32(const uint16_t *ops, int &opscnt)
{
	uint32_t value = (uint32_t)ops[opscnt++] << 16;
	value |= ops[opscnt++];
	return value;
}

int MC68000DASM::make_absolute_address_str(uint32_t pc, uint8_t data, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 0;
	uint32_t temp_value;

	switch(data & 7) {
	case 0:
		// short
		UTILITY::sntprintf(str, len, _T("$%04X"), ops[opscnt++]);
		make_l_w_str(false, str, len);
		break;
	case 1:
		// long
		UTILITY::sntprintf(str, len, _T("$%08X"), read_op32(ops, opscnt));
		make_l_w_str(true, str, len);
		break;
	case 2:
		// program counter with displacement
		temp_value = ops[opscnt++];
		wk[0][0] = 0; 
//		UTILITY::tcscat(wk[0], WKWORD_LEN, _T("($"));
		debugger->cat_value_or_symbol(wk[0], WKWORD_LEN, _T("("), m_addr_format_s, (make_int_16(temp_value) + pc + 2) & m_addr_mask);
//		UTILITY::sntprintf(wk[0], WKWORD_LEN, _T(ADDR_FORMAT), ADDR_FORMAT_MASK(make_int_16(temp_value) + pc + 2));
		wk[1][0] = 0; UTILITY::tcscat(wk[1], WKWORD_LEN, _T("PC)"));
		wk[2][0] = 0;
		join_words(str, len, wk);
		break;
	case 3:
		// program counter with index
		opscnt = make_ea_index_str(0xff, ops, opslen, str, len);
		break;
	case 4:
		// Immediate
		opscnt = make_immediage_str_u(immsize, ops, opslen, str, len);
		break;
	default:
		break;
	}
	return opscnt;
}

int MC68000DASM::make_immediage_str_u(en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10)
{
	int opscnt = 0;

	_TCHAR format[8];
	if (base10) {
		UTILITY::tcscpy(format, sizeof(format) / sizeof(_TCHAR), _T("#%u"));
	} else {
		UTILITY::tcscpy(format, sizeof(format) / sizeof(_TCHAR), _T("#$%X"));
	}

	uint32_t value;
	switch(immsize)
	{
	case SIZE_8BITS:
		value = (ops[opscnt++] & 0xff);
		break;
	case SIZE_32BITS:
		value = read_op32(ops, opscnt);
		break;
	default:
		// 16bits
		value = ops[opscnt++];
		break;
	}
	UTILITY::sntprintf(str, len, format, value);
	return opscnt;
}

int MC68000DASM::make_immediage_str_s(en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10)
{
	int opscnt = 0;

	UTILITY::tcscat(str, len, _T("#"));
	switch(immsize)
	{
	case SIZE_8BITS:
		make_signed_str_8(ops[opscnt++] & 0xff, str, len, base10);
		break;
	case SIZE_32BITS:
		make_signed_str_32(read_op32(ops, opscnt), str, len, base10);
		break;
	default:
		// 16bits
		make_signed_str_16(ops[opscnt++], str, len, base10);
		break;
	}
	return opscnt;
}

void MC68000DASM::make_signed_str_8(uint8_t val, _TCHAR *str, int len, bool base10)
{
//	if(val == 0x80)
//		UTILITY::tcscat(str, len, _T("-$80"));
//	else if(val & 0x80)
//		UTILITY::sntprintf(str, len, _T("-$%x"), (-val) & 0x7f);
//	else
//		UTILITY::sntprintf(str, len, _T("$%x"), val & 0x7f);
	if (base10) {
		UTILITY::sntprintf(str, len, _T("%d"), (int)(int8_t)val);
	} else {
		if ((int8_t)val < 0) {
			UTILITY::sntprintf(str, len, _T("-$%X"), -(int)(int8_t)val);
		} else {
			UTILITY::sntprintf(str, len, _T("$%X"), (int)(int8_t)val);
		}
	}
}

void MC68000DASM::make_signed_str_16(uint16_t val, _TCHAR *str, int len, bool base10)
{
//	if(val == 0x8000)
//		UTILITY::tcscat(str, len, _T("-$8000"));
//	else if(val & 0x8000)
//		UTILITY::nstprintf(str, len, _T("-$%x"), (-val) & 0x7fff);
//	else
//		UTILITY::sntprintf(str, len, _T("$%x"), val & 0x7fff);
	if (base10) {
		UTILITY::sntprintf(str, len, _T("%d"), (int)(int16_t)val);
	} else {
		if ((int16_t)val < 0) {
			UTILITY::sntprintf(str, len, _T("-$%X"), -(int)(int16_t)val);
		} else {
			UTILITY::sntprintf(str, len, _T("$%X"), (int)(int16_t)val);
		}
	}
}

void MC68000DASM::make_signed_str_32(uint32_t val, _TCHAR *str, int len, bool base10)
{
//	if(val == 0x80000000)
//		UTILITY::tcscat(str, len, _T("-$80000000"));
//	else if(val & 0x80000000)
//		UTILITY::sntprintf(str, len, _T("-$%x"), (-val) & 0x7fffffff);
//	else
//		UTILITY::sntprintf(str, len, _T("$%x"), val & 0x7fffffff);
	if (base10) {
		UTILITY::sntprintf(str, len, _T("%d"), (int)val);
	} else {
		if ((int32_t)val < 0) {
			UTILITY::sntprintf(str, len, _T("-$%X"), -(int)(int32_t)val);
		} else {
			UTILITY::sntprintf(str, len, _T("$%X"), (int)(int32_t)val);
		}
	}
}

void MC68000DASM::make_l_w_str(bool long_data, _TCHAR *str, int len)
{
	_TCHAR wk[4];
	UTILITY::tcscpy(wk, 4, long_data ? _T(".L") : _T(".W"));
	if (m_lower_case) strlower(wk);
	UTILITY::tcscat(str, len, wk);
}

/// @param[in] reg: 0-7 register number, otherwise "PC"
/// @param[in] ops: operands
/// @param[in] opslen: num of operands
/// @param[out] str: formatted string
/// @param[in] len: size of str
/// @return number of read operands
int MC68000DASM::make_ea_index_str(uint8_t reg, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[5][WKWORD_LEN];
	int pos = 0;

	int opscnt = 0;

	uint16_t extension = ops[opscnt++];

	if(CPU_TYPE_IS_010_LESS() && ext_index_scale(extension)) {
		// invalid operand
		UTILITY::tcscat(str, len, _T("???"));
		return opscnt;
	}

	if(ext_full(extension))
	{
		/* 68020 or lator */
		if(CPU_TYPE_IS_010_LESS()) {
			// unsupported mode
			UTILITY::tcscat(str, len, _T("???"));
			return opscnt;
		}

		if(ext_effective_zero(extension)) {
			UTILITY::tcscat(str, len, _T("0"));
			return opscnt;
		}

		uint32_t base = 0;
		if (ext_base_displacement_present(extension)) {
			if (ext_base_displacement_long(extension)) {
				base = ops[opscnt++];
				base <<= 16;
				base |= ops[opscnt++];
			} else {
				base = ops[opscnt++];
			}
		}
		uint32_t outer = 0;
		if (ext_outer_displacement_present(extension)) {
			if (ext_outer_displacement_long(extension)) {
				outer = ops[opscnt++];
				outer <<= 16;
				outer |= ops[opscnt++];
			} else {
				outer = ops[opscnt++];
			}
		}
		_TCHAR base_reg[4];
		base_reg[0] = 0;
		if (ext_base_register_present(extension)) {
			if (reg < 8) {
				make_address_reg_direct_str(reg & 7, base_reg, sizeof(base_reg) / sizeof(_TCHAR));
			} else {
				UTILITY::tcscpy(base_reg, sizeof(base_reg) / sizeof(_TCHAR), _T("PC"));
			}
		}
		_TCHAR index_reg[32];
		index_reg[0] = 0;
		if (ext_index_register_present(extension)) {
			make_reg_direct_str(ext_index_ar(extension) ? _T('A') : _T('D')
				, ext_index_register(extension)
				, index_reg, sizeof(index_reg) / sizeof(_TCHAR)
			);
			make_l_w_str(ext_index_long(extension) != 0, index_reg, sizeof(index_reg) / sizeof(_TCHAR)); 
			if (ext_index_scale(extension)) {
				UTILITY::sntprintf(index_reg, sizeof(index_reg) / sizeof(_TCHAR)
				 ,_T("*%d"), 1 << ext_index_scale(extension));
			}
		}

		bool preindex = ((extension & 7) > 0 && (extension & 7) < 4);
		bool postindex = ((extension & 7) > 4);

		wk[pos][0] = 0;
		UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("("));
		if(preindex || postindex)
			UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("["));

		bool comma = false;
		if(base) {
			if(reg < 8 && ext_base_displacement_long(extension))
				make_signed_str_32(base, wk[pos], WKWORD_LEN);
			else
				make_signed_str_16(base, wk[pos], WKWORD_LEN);
			comma = true;
		}
		if(base_reg[0])
		{
			if(comma) wk[++pos][0] = 0;
			UTILITY::tcscat(wk[pos], WKWORD_LEN, base_reg);
			comma = true;
		}
		if(postindex)
		{
			UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("]"));
			comma = true;
		}
		if(index_reg[0])
		{
			if(comma) wk[++pos][0] = 0;
			UTILITY::tcscat(wk[pos], WKWORD_LEN, index_reg);
			comma = true;
		}
		if(preindex)
		{
			UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("]"));
			comma = true;
		}
		if(outer)
		{
			if(comma) wk[++pos][0] = 0;
			make_signed_str_16(outer, wk[pos], WKWORD_LEN);
		}
		UTILITY::tcscat(wk[pos], WKWORD_LEN, _T(")"));
		wk[++pos][0] = 0;
		join_words(str, len, wk);
	}
	else
	{
		/* 68000 - 68010 */
		wk[pos][0] = 0; UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("("));
		if (ext_8bit_displacement(extension)) {
			make_signed_str_8((uint8_t)extension, wk[pos], WKWORD_LEN);
			wk[++pos][0] = 0;
		}
		if (reg < 8) {
			make_address_reg_direct_str(reg & 7, wk[pos], WKWORD_LEN);
		} else {
			UTILITY::tcscat(wk[pos], WKWORD_LEN, _T("PC"));
		}
		wk[++pos][0] = 0;
		UTILITY::sntprintf(wk[pos], WKWORD_LEN, _T("%c%d"), ext_index_ar(extension) ? _T('A') : _T('D'), ext_index_register(extension));
		make_l_w_str(ext_index_long(extension) != 0, wk[pos], WKWORD_LEN);

		if(ext_index_scale(extension)) {
			UTILITY::sntprintf(wk[pos], WKWORD_LEN, _T("*%d"), 1 << ext_index_scale(extension));
		}
		UTILITY::tcscat(wk[pos], WKWORD_LEN, _T(")"));
		wk[++pos][0] = 0;
		join_words(str, len, wk);
	}
	return opscnt;
}

#if 0
int MC68000DASM::make_pc_index_str(uint32_t pc, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	/* program counter with index */
	int opscnt = 0;

	uint16_t extension = ops[opscnt++];

	if(CPU_TYPE_IS_010_LESS() && ext_index_scale(extension)) {
		// invalid operand
		UTILITY::tcscat(str, len, _T("???"));
		return opscnt;
	}

	if(ext_full(extension))
	{
		/* 68020 or lator */
		if(CPU_TYPE_IS_010_LESS()) {
			// unsupported mode
			UTILITY::tcscat(str, len, _T("???"));
			return opscnt;
		}

		if(ext_effective_zero(extension)) {
			UTILITY::tcscat(str, len, _T("0"));
			return opscnt;
		}

		uint32_t base = 0;
		if (ext_base_displacement_present(extension)) {
			if (ext_base_displacement_long(extension)) {
				base = ops[opscnt++];
				base <<= 16;
				base |= ops[opscnt++];
			} else {
				base = ops[opscnt++];
			}
		}
		uint32_t outer = 0;
		if (ext_outer_displacement_present(extension)) {
			if (ext_outer_displacement_long(extension)) {
				outer = ops[opscnt++];
				outer <<= 16;
				outer |= ops[opscnt++];
			} else {
				outer = ops[opscnt++];
			}
		}

		_TCHAR base_reg[4];
		base_reg[0] = 0;
		if (ext_base_register_present(extension)) {
			UTILITY::tcscpy(base_reg, sizeof(base_reg) / sizeof(_TCHAR), _T("PC"));
		}
		_TCHAR index_reg[32];
		index_reg[0] = 0;
		_TCHAR index_scale[16];
		index_scale[0] = 0;

		if (ext_index_register_present(extension)) {
			if (ext_index_scale(extension)) {
				UTILITY::stprintf(index_scale, sizeof(index_scale) / sizeof(_TCHAR)
				 ,_T("*%d"), 1 << ext_index_scale(extension));
			}
			UTILITY::stprintf(index_reg, sizeof(index_reg) / sizeof(_TCHAR)
				, _T("%c%d.%c%s")
				, ext_index_ar(extension) ? _T('A') : _T('D')
				, ext_index_register(extension)
				, ext_index_long(extension) ? _T('l') : _T('w')
				, index_scale);
		}

		bool preindex = ((extension & 7) > 0 && (extension & 7) < 4);
		bool postindex = ((extension & 7) > 4);

		UTILITY::tcscat(str, len, _T("("));
		if(preindex || postindex)
			UTILITY::tcscat(str, len, _T("["));

		bool comma = false;
		if(base)
		{
			make_signed_hex_str_16(base, str, len);
			comma = true;
		}
		if(base_reg[0])
		{
			if(comma)
				UTILITY::tcscat(str, len, _T(","));
			UTILITY::tcscat(str, len, base_reg);
			comma = true;
		}
		if(postindex)
		{
			UTILITY::tcscat(str, len, _T("]"));
			comma = true;
		}
		if(index_reg[0])
		{
			if(comma)
				UTILITY::tcscat(str, len, _T(","));
			UTILITY::tcscat(str, len, index_reg);
			comma = true;
		}
		if(preindex)
		{
			UTILITY::tcscat(str, len, _T("]"));
			comma = true;
		}
		if(outer)
		{
			if(comma)
				UTILITY::tcscat(str, len, _T(","));
			make_signed_hex_str_16(outer, str, len);
		}
		UTILITY::tcscat(str, len, _T(")"));
	}
	else
	{
		/* 68000 - 68010 */
		if (ext_8bit_displacement(extension)) {
			UTILITY::tcscat(str, len, _T("("));
			make_signed_hex_str_8((uint8_t)extension, str, len);
			UTILITY::tcscat(str, len, _T(",PC"));
			UTILITY::sntprintf(str, len, _T(",%c%d"), ext_index_ar(extension) ? _T('A') : _T('D'), ext_index_register(extension));
			UTILITY::sntprintf(str, len, _T(".%c"), ext_index_long(extension) ? _T('l') : _T('w'));
		} else {
			UTILITY::tcscat(str, len, _T("("));
			UTILITY::tcscat(str, len, _T("PC"));
			UTILITY::sntprintf(str, len, _T(",%c%d"), ext_index_ar(extension) ? _T('A') : _T('D'), ext_index_register(extension));
			UTILITY::sntprintf(str, len, _T(".%c"), ext_index_long(extension) ? _T('l') : _T('w'));
		}

		if(ext_index_scale(extension)) {
			UTILITY::sntprintf(str, len, _T("*%d"), 1 << ext_index_scale(extension));
		}
		UTILITY::tcscat(str, len, _T(")"));
	}
	return opscnt;
}
#endif


int MC68000DASM::make_ea_mode_str(uint8_t mode, uint8_t reg, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 0;

	switch(mode & 7) {
	case 0x00:
		/* data register direct */
		make_data_reg_direct_str(reg, str, len);
		break;
	case 0x01:
		/* address register direct */
		make_address_reg_direct_str(reg, str, len);
		break;
	case 0x02:
		/* address register indirect */
		make_address_reg_indirect_str(reg, str, len);
		break;
	case 0x03:
		/* address register indirect with postincrement */
		make_address_reg_indirect_postinc_str(reg, str, len);
		break;
	case 0x04:
		/* address register indirect with predecrement */
		make_address_reg_indirect_predec_str(reg, str, len);
		break;
	case 0x05:
		/* address register indirect with displacement*/
		opscnt = make_address_reg_indirect_displace_str(reg, ops, opslen, str, len);
		break;
	case 0x06:
		/* address register indirect with index */
		opscnt = make_ea_index_str(reg, ops, opscnt, str, len);
		break;
	case 0x07:
		/* absolute address */
		opscnt = make_absolute_address_str(m_bag.pc, reg, immsize, ops, opslen, str, len);
		break;
	default:
		break;
	}
	return opscnt;
}

void MC68000DASM::join_words(_TCHAR *str, int len, const _TCHAR words[][WKWORD_LEN])
{
//	int pos = (int)_tcslen(str);
	for(int i=0; i<16; i++) {
		if (words[i][0] == 0) break;
		if (i > 0) {
			UTILITY::tcscat(str, len, _T(","));
		}
		UTILITY::tcscat(str, len, words[i]);
	}
}

void MC68000DASM::strlower(_TCHAR *str)
{
	int len = (int)_tcslen(str);
	for(int i=0; i<len; i++) {
		if (str[i] >= 0x40 && str[i] <= 0x5a) {
			str[i] += 0x20;
		}
	}
}

void MC68000DASM::make_name_str(const _TCHAR *name, bool sup, _TCHAR *str, int len, const _TCHAR *pre, const _TCHAR *post)
{
	_TCHAR nname[32]; nname[0] = 0;
	if (!sup) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), _T("; "));
	if (pre) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), pre);
	UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), name);
	if (post) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), post);
	if (m_lower_case) {
		strlower(nname);
	}
	UTILITY::sntprintf(str, len, _T("%-7s "), nname);
}

void MC68000DASM::make_cpname_str(uint16_t cpnum, const _TCHAR *name, bool sup, _TCHAR *str, int len, const _TCHAR *pre, const _TCHAR *post)
{
	_TCHAR nname[32]; nname[0] = 0;
	if (!sup) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), _T("; "));
	UTILITY::sntprintf(nname, sizeof(nname) / sizeof(_TCHAR), _T("%d"), cpnum);
	if (pre) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), pre);
	UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), name);
	if (post) UTILITY::tcscat(nname, sizeof(nname) / sizeof(_TCHAR), post);
	if (m_lower_case) {
		strlower(nname);
	}
	UTILITY::sntprintf(str, len, _T("%-7s "), nname);
}

/// Rn base
int MC68000DASM::make_r_base_str(const _TCHAR *name, bool sup, _TCHAR reg, uint32_t d, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(name, sup, str, len);
	make_reg_direct_str(reg, d, str, len);
	return 1;
}

/// Dn
int MC68000DASM::make_dr_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_base_str(name, sup, _T('D'), ops[0] & 7, ops, opslen, str, len);
}

/// Rn, Rn base
int MC68000DASM::make_r_r_base_str(const _TCHAR *name, bool sup, _TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(name, sup, str, len);
	make_reg_reg_direct_str(reg1, d1, reg2, d2, str, len);
	return 1;
}

/// Dn, Dn
int MC68000DASM::make_dr_dr_d_s_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_r_base_str(name, sup, _T('D'), ops[0] & 7, _T('D'), (ops[0] >> 9) & 7, ops, opslen, str, len);
}

/// Dn, Dn
int MC68000DASM::make_dr_dr_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_r_base_str(name, sup, _T('D'), (ops[0] >> 9) & 7, _T('D'), ops[0] & 7, ops, opslen, str, len);
}

/// An, An
int MC68000DASM::make_ar_ar_d_s_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_r_base_str(name, sup, _T('A'), ops[0] & 7, _T('A'), (ops[0] >> 9) & 7, ops, opslen, str, len);
}

/// An, An
int MC68000DASM::make_ar_ar_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_r_base_str(name, sup, _T('A'), (ops[0] >> 9) & 7, _T('A'), ops[0] & 7, ops, opslen, str, len);
}

/// Dn, An
int MC68000DASM::make_dr_ar_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_r_r_base_str(name, sup, _T('D'), (ops[0] >> 9) & 7, _T('A'), ops[0] & 7, ops, opslen, str, len);
}

/// (An)+, (An)+
int MC68000DASM::make_arpl_arpl_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(name, sup, str, len);
	make_reg_reg_indirect_postinc_str(_T('A'), ops[0] & 7, _T('A'), (ops[0] >> 9) & 7, str, len);
	return 1;
}

/// -(An), -(An)
int MC68000DASM::make_miar_miar_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(name, sup, str, len);
	make_reg_reg_indirect_predec_str(_T('A'), ops[0] & 7, _T('A'), (ops[0] >> 9) & 7, str, len);
	return 1;
}

/// An, {CCR | SR}
int MC68000DASM::make_ar_ccrsr_str(const _TCHAR *name, bool sup, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	make_address_reg_direct_str(ops[0] & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_ccrsr_str(regn, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return 1;
}

/// {CCR | SR}, An
int MC68000DASM::make_ccrsr_ar_str(const _TCHAR *name, bool sup, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	make_ccrsr_str(regn, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_address_reg_direct_str(ops[0] & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return 1;
}

/// <ea>
int MC68000DASM::make_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, const _TCHAR *pre)
{
	int opscnt = 1;
	make_name_str(name, sup, str, len, pre);
	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, str, len);
	return opscnt;
}

/// <ea>, <ea>
int MC68000DASM::make_ea_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, const _TCHAR *pre)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len, pre);
	wk[0][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 6) & 7, (ops[0] >> 9) & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// <ea>, Dn
int MC68000DASM::make_ea_dr_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_data_reg_direct_str((ops[0] >> 9) & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// Dn, <ea>
int MC68000DASM::make_dr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	make_data_reg_direct_str((ops[0] >> 9) & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// Dn, Dn, <ea>
int MC68000DASM::make_dr_dr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[4][WKWORD_LEN];
	int opscnt = 1;
	uint16_t extension = ops[opscnt++];
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	make_data_reg_direct_str(extension & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_data_reg_direct_str((extension >> 8) & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[2], WKWORD_LEN);
	wk[3][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// <ea>, An
int MC68000DASM::make_ea_ar_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_address_reg_direct_str((ops[0] >> 9) & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

void MC68000DASM::make_ccrsr_str(en_regname regn, _TCHAR *str, int len)
{
	const _TCHAR *regname[] = { _T("CCR"), _T("SR"), _T("USP"), _T("???"), NULL };
	if (regn < REG_UNKNOWN) {
		UTILITY::tcscat(str, len, regname[regn]);
	} else {
		UTILITY::tcscat(str, len, regname[REG_UNKNOWN]);
	}
}

/// <ea>, {CCR | SR}
int MC68000DASM::make_ea_ccrsr_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_ccrsr_str(regn, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// {CCR | SR}, <ea>
int MC68000DASM::make_ccrsr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0;	make_ccrsr_str(regn, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// #<unsigned imm>
int MC68000DASM::make_uimm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10)
{
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	opscnt += make_immediage_str_u(immsize, &ops[opscnt], opslen - opscnt, str, len, base10);
	return opscnt;
}

/// #<signed imm>
int MC68000DASM::make_simm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10)
{
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	opscnt += make_immediage_str_s(immsize, &ops[opscnt], opslen - opscnt, str, len, base10);
	return opscnt;
}

/// #<signed imm>, <ea>
int MC68000DASM::make_simm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; opscnt += make_immediage_str_s(immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0; opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// #<unsigned imm>, <ea>
int MC68000DASM::make_uimm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; opscnt += make_immediage_str_u(immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0; opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// #<quick imm>, <ea>
int MC68000DASM::make_qimm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; UTILITY::sntprintf(wk[0], WKWORD_LEN, _T("#%d"), m_3bit_qdata_table[(ops[0] >> 9) & 7]);
	wk[1][0] = 0; opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// #<quick imm>, Dn
int MC68000DASM::make_qimm_dr_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; UTILITY::sntprintf(wk[0], WKWORD_LEN, _T("#%d"), m_3bit_qdata_table[(ops[0] >> 9) & 7]);
	wk[1][0] = 0; make_data_reg_direct_str(ops[0] & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return 1;
}

/// #<unsigned imm>, {CCR | SR}
int MC68000DASM::make_uimm_ccrsr_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; opscnt += make_immediage_str_u(immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0; make_ccrsr_str(regn, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// An, <signed imm>
int MC68000DASM::make_ar_simm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, sup, str, len);
	wk[0][0] = 0; make_address_reg_direct_str(ops[0] & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0; opscnt += make_immediage_str_s(immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

/// bcc
int MC68000DASM::make_bcc_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;
	make_name_str(name, sup, str, len, _T("B"));
	uint32_t offset;
	switch(immsize) {
	case SIZE_16BITS:
		offset = make_int_16(ops[opscnt++]);
		break;
	case SIZE_32BITS:
		offset = make_int_32(read_op32(ops, opscnt));
		break;
	default:
		offset = make_int_8((uint8_t)ops[0]);
		break;
	}
//	UTILITY::tcscat(str, len, _T("$"));
//	UTILITY::sntprintf(str, len, _T(ADDR_FORMAT), ADDR_FORMAT_MASK(temp_pc + 2 + offset));
	debugger->cat_value_or_symbol(str, len, NULL, m_addr_format_s, (temp_pc + 2 + offset) & m_addr_mask); 
	return opscnt;
}

/// dbcc
int MC68000DASM::make_dbcc_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;
	make_name_str(name, sup, str, len, _T("DB"));
	uint32_t offset;
	switch(immsize) {
	case SIZE_32BITS:
		offset = make_int_32(read_op32(ops, opscnt));
		break;
	default:
		offset = make_int_16(ops[opscnt++]);
		break;
	}
	wk[0][0] = 0; make_data_reg_direct_str(ops[0] & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;
	debugger->cat_value_or_symbol(wk[1], WKWORD_LEN, NULL, m_addr_format_s, (temp_pc + 2 + offset) & m_addr_mask); 
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

#if 0
/* Check if opcode is using a valid ea mode */
bool MC68000DASM::valid_ea(uint32_t opcode, uint32_t mask)
{
	if(mask == 0)
		return true;

	switch(opcode & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			return (mask & 0x800) != 0;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			return (mask & 0x400) != 0;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			return (mask & 0x200) != 0;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return (mask & 0x100) != 0;
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			return (mask & 0x080) != 0;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			return (mask & 0x040) != 0;
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			return (mask & 0x020) != 0;
		case 0x38:
			return (mask & 0x010) != 0;
		case 0x39:
			return (mask & 0x008) != 0;
		case 0x3a:
			return (mask & 0x002) != 0;
		case 0x3b:
			return (mask & 0x001) != 0;
		case 0x3c:
			return (mask & 0x004) != 0;
	}
	return false;
}
#endif

#if 0
/* Used by qsort */
bool MC68000DASM::compare_nof_true_bits(struct opcode_index *aidx, struct opcode_index *bidx)
{
//	uint32_t a = aidx->mask;
//	uint32_t b = bidx->mask;

//	a = ((a & 0xAAAA) >> 1) + (a & 0x5555);
//	a = ((a & 0xCCCC) >> 2) + (a & 0x3333);
//	a = ((a & 0xF0F0) >> 4) + (a & 0x0F0F);
//	a = ((a & 0xFF00) >> 8) + (a & 0x00FF);

//	b = ((b & 0xAAAA) >> 1) + (b & 0x5555);
//	b = ((b & 0xCCCC) >> 2) + (b & 0x3333);
//	b = ((b & 0xF0F0) >> 4) + (b & 0x0F0F);
//	b = ((b & 0xFF00) >> 8) + (b & 0x00FF);

//	return b < a; /* reversed to get greatest to least sorting */

	return ((aidx->match < bidx->match) || (bidx->mask < aidx->mask));
}
#endif

#include <algorithm>

//void MC68000DASM::build_opcode_table_sub()
//{
//}

// compare function
bool MC68000DASM::compare_mask_bits(uint16_t a, uint16_t b)
{
	return (m_opcode_info[b].mask < m_opcode_info[a].mask);
}

/* build the opcode handler jump table */
void MC68000DASM::build_opcode_table()
{
	std::vector<uint16_t> opcode_info[16];
	// i=0 is illegal function
	for(uint16_t i=1; m_opcode_info[i].handler; i++) {
//		uint32_t mask = m_opcode_info[i].mask;
		uint32_t match = m_opcode_info[i].match;
		int sect = (match >> 12) & 0xf;
		opcode_info[sect].push_back(i);
	}
	for(int i=0; i<16; i++) {
//		int cnt = (int)opcode_info[i].size();
		std::sort(opcode_info[i].begin(), opcode_info[i].end(), compare_mask_bits
// lambda expression is supported on c++11 or lator
//			[](uint16_t a, uint16_t b) {
//				return (m_opcode_info[b].mask < m_opcode_info[a].mask);
//			}
		);
	}
	std::vector<uint16_t>::iterator it;

	for(uint32_t opcode = 0; opcode != 0x10000; opcode++)
	{
		instruction_table[opcode] = 0; /* default to illegal */
		int sect = (opcode >> 12) & 0xf;

		for(it = opcode_info[sect].begin(); it != opcode_info[sect].end(); it++) {
			const opcode_struct *info = &m_opcode_info[*it];
			if((opcode & (info->mask)) == info->match) {
				instruction_table[opcode] = (*it);
				break;
			}
		}
	}
}

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */
/* Instruction handler function names follow this convention:
 *
 * d68000_NAME_EXTENSIONS(void)
 * where NAME is the name of the opcode it handles and EXTENSIONS are any
 * extensions for special instances of that opcode.
 *
 * Examples:
 *   d68000_add_er_8(): add opcode, from effective address to register,
 *                      size = byte
 *
 *   d68000_asr_s_8(): arithmetic shift right, static count, size = byte
 *
 *
 * Common extensions:
 * 8   : size = byte
 * 16  : size = word
 * 32  : size = long
 * rr  : register to register
 * mm  : memory to memory
 * r   : register
 * s   : static
 * er  : effective address -> register
 * re  : register -> effective address
 * ea  : using effective address mode of operation
 * d   : data register direct
 * a   : address register direct
 * ai  : address register indirect
 * pi  : address register indirect with postincrement
 * pd  : address register indirect with predecrement
 * di  : address register indirect with displacement
 * ix  : address register indirect with index
 * aw  : absolute word
 * al  : absolute long
 */

int MC68000DASM::d68000_illegal(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("DC.W"), true, str, len);
	UTILITY::sntprintf(str, len, _T("$%04X ; ILLEGAL"), ops[0]);
	return 1;
}

int MC68000DASM::d68000_1010(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("DC.W"), true, str, len);
	UTILITY::sntprintf(str, len, _T("$%04X ; opcode 1010"), ops[0]);
	return 1;
}

int MC68000DASM::d68000_1111(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("DC.W"), true, str, len);
	UTILITY::sntprintf(str, len, _T("$%04X ; opcode 1111"), ops[0]);
	return 1;
}

int MC68000DASM::d68000_abcd_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("ABCD"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_abcd_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("ABCD"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("ADD.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("ADD.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("ADD.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("ADD.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("ADD.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_add_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("ADD.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_adda_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("ADDA.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_adda_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("ADDA.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("ADDI.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("ADDI.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("ADDI.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addq_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("ADDQ.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addq_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("ADDQ.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addq_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("ADDQ.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_rr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("ADDX.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_rr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("ADDX.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_rr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("ADDX.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_mm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("ADDX.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_mm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("ADDX.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_addx_mm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("ADDX.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("AND.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("AND.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("AND.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("AND.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("AND.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_and_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("AND.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_andi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ANDI.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_andi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ANDI.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_andi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ANDI.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_andi_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("ANDI"), true, SIZE_8BITS, REG_CCR, ops, opslen, str, len);
}

int MC68000DASM::d68000_andi_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("ANDI"), true, SIZE_16BITS, REG_SR, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ASR.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ASL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ASL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_asl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ASL.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bcc_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(m_bcc[(ops[0] >> 8) & 0xf], true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(m_bcc[(ops[0] >> 8) & 0xf], true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_bcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_bcc_str(m_bcc[(ops[0] >> 8) & 0xf], CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

#if 0
int MC68000DASM::d68000_bchg_clr_set_tst_r_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	UTILITY::tcscat(str, len, name);
	UTILITY::tcscat(str, len, _T("    "));
	UTILITY::sntprintf(str, len, _T("D%d,"), (ops[0] >> 9) & 7);
	opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	return opscnt;
}

int MC68000DASM::d68000_bchg_clr_set_tst_s_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	UTILITY::tcscat(str, len, name);
	UTILITY::tcscat(str, len, _T("    "));
	opscnt += make_immediage_str_u8(&ops[opscnt], opslen - opscnt, str, len);
	UTILITY::tcscat(str, len, _T(", "));
	opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	return opscnt;
}
#endif

int MC68000DASM::d68000_bchg_r(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("BCHG"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bchg_s(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("BCHG"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bclr_r(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("BCLR"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bclr_s(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("BCLR"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68010_bkpt(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_uimm_str(_T("BKPT"), CPU_TYPE_IS_010_PLUS() != 0, SIZE_16BITS, ops, opslen, str, len);
	make_comment_str(_T("V1+"), str, len);
	return opscnt;
}

void MC68000DASM::d68020_bfxxx_offset_width(uint16_t extension, _TCHAR *str, int len)
{
	// offset
	if (BIT(extension, 11))
		UTILITY::sntprintf(str, len, _T("{D%d:"),(extension >> 6) & 7);
	else
		UTILITY::sntprintf(str, len, _T("{%d:"),(extension >> 6) & 31);
	// width
	if (BIT(extension, 5))
		UTILITY::sntprintf(str, len, _T("D%d}"), extension & 7);
	else
		UTILITY::sntprintf(str, len, _T("%d}"), m_5bit_data_table[extension & 31]);
}

int MC68000DASM::d68020_bfxxx_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;

	make_name_str(name, CPU_TYPE_IS_020_PLUS() != 0, str, len);

	uint16_t extension = ops[opscnt++];
	opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	d68020_bfxxx_offset_width(extension, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_bfchg(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_temp(_T("BFCHG"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfclr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_temp(_T("BFCLR"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfxxx_di_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;

	make_name_str(name, CPU_TYPE_IS_020_PLUS() != 0, str, len);

	uint16_t extension = ops[opscnt++];

	wk[0][0] = 0;	make_data_reg_direct_str((extension >> 12) & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	d68020_bfxxx_offset_width(extension, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_bfexts(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_di_temp(_T("BFEXTS"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfextu(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_di_temp(_T("BFEXTU"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfffo(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_di_temp(_T("BFFFO"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfins(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_di_temp(_T("BFINS"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bfset(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_temp(_T("BFSET"), ops, opslen, str, len);
}

int MC68000DASM::d68020_bftst(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_bfxxx_temp(_T("BFTST"), ops, opslen, str, len);
}

int MC68000DASM::d68000_bra_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(_T("RA"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bra_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(_T("RA"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_bra_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_bcc_str(_T("RA"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_bset_r(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("BSET"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bset_s(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("BSET"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bsr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(_T("SR"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_bsr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_bcc_str(_T("SR"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_bsr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_bcc_str(_T("SR"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_btst_r(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("BTST"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_btst_s(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("BTST"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_callm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_uimm_ea_str(_T("CALLM"), CPU_TYPE_IS_020_VARIANT() != 0, SIZE_8BITS, ops, opslen, str, len);
	make_comment_str(_T("V2"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_dr_dr_ea_str(_T("CAS.B"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_8BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_dr_dr_ea_str(_T("CAS.W"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_16BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_dr_dr_ea_str(_T("CAS.L"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas2_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
/* CAS2 Dc1:Dc2,Du1:Dc2:(Rn1):(Rn2)
f e d c b a 9 8 7 6 5 4 3 2 1 0
 DARn1  0 0 0  Du1  0 0 0  Dc1
 DARn2  0 0 0  Du2  0 0 0  Dc2
*/
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	uint32_t extension = read_op32(ops, opscnt);
	make_name_str(name, CPU_TYPE_IS_020_PLUS() != 0, str, len);
	wk[0][0] = 0;	UTILITY::sntprintf(wk[0], WKWORD_LEN, _T("D%d:D%d:D%d:D%d"),
		(extension >> 16) & 7, extension & 7, (extension >> 22) & 7, (extension >> 6) & 7);
	wk[1][0] = 0;	UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("(%c%d):(%c%d)"),
		BIT(extension, 31) ? _T('A') : _T('D'), (extension >> 28) & 7,
		BIT(extension, 15) ? _T('A') : _T('D'), (extension >> 12) & 7);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas2_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = d68020_cas2_temp(_T("CAS2.W"), ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cas2_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = d68020_cas2_temp(_T("CAS2.L"), ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_chk_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
//	uint16_t extension = ops[opscnt];
	opscnt++;
	opscnt += make_ea_dr_str(_T("CHK.W"), true, SIZE_16BITS, ops, opslen, str, len);
	return opscnt;
}

int MC68000DASM::d68020_chk_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
//	uint16_t extension = ops[opscnt];
	opscnt++;
	opscnt += make_ea_dr_str(_T("CHK.L"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_chk2_cmp2_temp(const _TCHAR *post, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	uint16_t extension = ops[opscnt++];
	make_name_str(BIT(extension, 11) ? _T("CHK2") : _T("CMP2"), CPU_TYPE_IS_020_PLUS() != 0, str, len, NULL, post);
	wk[0][0] = 0;	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_reg_direct_str(BIT(extension, 15) ? _T('A') : _T('D'), (extension >> 12) & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_chk2_cmp2_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_chk2_cmp2_temp(_T(".B"), SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_chk2_cmp2_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_chk2_cmp2_temp(_T(".W"), SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_chk2_cmp2_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_chk2_cmp2_temp(_T(".L"), SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68040_cache_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_040_PLUS() != 0;

	static const _TCHAR *cachetype[4] = { _T("NOP"), _T("DATA"), _T("INST"), _T("BOTH") };

	switch((ops[0] >> 3) & 3)
	{
		case 0:
			make_name_str(name, c, str, len);
			UTILITY::tcscat(str, len, _T("???"));
			break;
		case 1:
			make_name_str(name, c, str, len, NULL, _T("L"));
			UTILITY::tcscat(str, len, cachetype[(ops[0] >> 6) & 3]);
			UTILITY::sntprintf(str, len, _T(", (A%d)"), ops[0] & 7);
			break;
		case 2:
			make_name_str(name, c, str, len, NULL, _T("P"));
			UTILITY::tcscat(str, len, cachetype[(ops[0] >> 6) & 3]);
			UTILITY::sntprintf(str, len, _T(", (A%d)"), ops[0] & 7);
			break;
		default:
			make_name_str(name, c, str, len, NULL, _T("A"));
			UTILITY::tcscat(str, len, cachetype[(ops[0] >> 6) & 3]);
			break;
	}
	make_comment_str(_T("V4"), str, len);
	return 1;
}

int MC68000DASM::d68040_cinv(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68040_cache_temp(_T("CINV"), ops, opslen, str, len);
}

int MC68000DASM::d68000_clr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("CLR.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_clr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("CLR.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_clr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("CLR.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmp_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("CMP.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmp_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("CMP.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmp_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("CMP.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpa_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("CMPA.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpa_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("CMPA.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("CMPI.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_cmpi_pcdi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_simm_ea_str(_T("CMPI.B"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_8BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cmpi_pcix_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_cmpi_pcdi_8(ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("CMPI.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_cmpi_pcdi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_simm_ea_str(_T("CMPI.W"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_16BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cmpi_pcix_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_cmpi_pcdi_16(ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("CMPI.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_cmpi_pcdi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_simm_ea_str(_T("CMPI.L"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cmpi_pcix_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_cmpi_pcdi_32(ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_arpl_arpl_str(_T("CMPM.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_arpl_arpl_str(_T("CMPM.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_cmpm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_arpl_arpl_str(_T("CMPM.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68020_cpbcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint32_t new_pc = m_bag.pc;
//	uint16_t extension = ops[opscnt];
	opscnt++;
	new_pc += make_int_16(ops[opscnt]);
	opscnt++;

	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[ops[0] & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("B"));
	opscnt += make_immediage_str_s16(&ops[opscnt], opslen - opscnt, str, len);
//	UTILITY::sntprintf(str, len, _T(";%X (ext = %X) (2-3)"), new_pc + 2, extension);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cpbcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint32_t new_pc = m_bag.pc;
//	uint16_t extension = ops[opscnt];
	opscnt++;
	new_pc += read_op32(ops, opscnt);

	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[ops[0] & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("B"));
	opscnt += make_immediage_str_s16(&ops[opscnt], opslen - opscnt, str, len);
//	UTILITY::sntprintf(str, len, _T(";%X (ext = %X) (2-3)"), new_pc + 2, extension);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cpdbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;
	uint32_t new_pc = m_bag.pc;
	uint16_t extension1 = ops[opscnt];
	opscnt++;
//	uint16_t extension2 = ops[opscnt];
	opscnt++;
	new_pc += make_int_16(ops[opscnt]);
	opscnt++;

	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[extension1 & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("DB"));
	wk[0][0] = 0;	make_data_reg_direct_str(ops[0] & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_immediage_str_s16(&ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
//	UTILITY::sntprintf(str, len, _T("; %X (ext = %X) (2-3)"), new_pc + 2, extension2);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cpgen(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	make_cpname_str((ops[0] >> 9) & 7, _T("GEN"), CPU_TYPE_IS_020_PLUS() != 0, str, len);
	opscnt += make_immediage_str_u32(&ops[opscnt], opslen - opscnt, str, len);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cprestore(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_020_PLUS() != 0;
	int opscnt = 1;
	if (((ops[0] >> 9) & 7) == 1)
	{
		make_name_str(_T("FRESTORE"), c, str, len);
		opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	}
	else
	{
		make_cpname_str((ops[0] >> 9) & 7, _T("RESTORE"), c, str, len);
		opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	}
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cpsave(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_020_PLUS() != 0;
	int opscnt = 1;
	if (((ops[0] >> 9) & 7) == 1)
	{
		make_name_str(_T("FSAVE"), c, str, len);
		opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	}
	else
	{
		make_cpname_str((ops[0] >> 9) & 7, _T("SAVE"), c, str, len);
		opscnt += make_ea_mode_str_8((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
	}
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cpscc(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint16_t extension1 = ops[opscnt];
	opscnt++;
//	uint16_t extension2 = ops[opscnt];
	opscnt++;
	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[extension1 & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("S"));
	opscnt += make_immediage_str_s8(&ops[opscnt], opslen - opscnt, str, len);
//	UTILITY::sntprintf(str, len, _T("; (ext = %X) (2-3)"), extension2);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cptrapcc_0(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint16_t extension1 = ops[opscnt];
	opscnt++;
//	uint16_t extension2 = ops[opscnt];
	opscnt++;
	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[extension1 & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("TRAP"));
//	UTILITY::sntprintf(str, len, _T("; (ext = %X) (2-3)"), extension2);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cptrapcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint16_t extension1 = ops[opscnt];
	opscnt++;
//	uint16_t extension2 = ops[opscnt];
	opscnt++;
	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[extension1 & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("TRAP"));
	opscnt += make_immediage_str_u16(&ops[opscnt], opslen - opscnt, str, len);
//	UTILITY::sntprintf(str, len, _T("; (ext = %X) (2-3)"), extension2);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_cptrapcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint16_t extension1 = ops[opscnt];
	opscnt++;
//	uint16_t extension2 = ops[opscnt];
	opscnt++;
	make_cpname_str((ops[0] >> 9) & 7, m_cpcc[extension1 & 0x3f], CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("TRAP"));
	opscnt += make_immediage_str_u32(&ops[opscnt], opslen - opscnt, str, len);
//	UTILITY::sntprintf(str, len, _T("; (ext = %X) (2-3)"), extension2);
	make_comment_str(_T("V2-3"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_cpush(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68040_cache_temp(_T("CPUSH"), ops, opslen, str, len);
}

int MC68000DASM::d68000_dbra(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dbcc_str(_T("RA"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_dbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dbcc_str(m_bcc[(ops[0] >> 8) & 0xf], true, SIZE_16BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_divs(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("DIVS.W"), true, SIZE_16BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_divu(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("DIVU.W"), true, SIZE_16BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68020_divl(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	bool c = CPU_TYPE_IS_020_PLUS() != 0;
	int opscnt = 1;

	uint16_t extension = ops[opscnt++];
	_TCHAR u[8];

	u[0] = BIT(extension, 11) ? _T('S') : _T('U');
	u[1] = 0;

	if(BIT(extension, 10)) {
		UTILITY::tcscat(u, 8, _T(".L"));
		make_name_str(_T("DIV"), c, str, len, NULL, u);
		wk[0][0] = 0;	opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
		wk[1][0] = 0;	UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("D%d:D%d"), extension&7, (extension>>12)&7);
	} else if((extension&7) == ((extension>>12)&7)) {
		UTILITY::tcscat(u, 8, _T(".L"));
		make_name_str(_T("DIV"), c, str, len, NULL, u);
		wk[0][0] = 0;	opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
		wk[1][0] = 0;	make_data_reg_direct_str((extension>>12)&7, wk[1], WKWORD_LEN);
	} else {
		UTILITY::tcscat(u, 8, _T("L.L"));
		make_name_str(_T("DIV"), c, str, len, NULL, u);
		wk[0][0] = 0;	opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
		wk[1][0] = 0;	UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("D%d:D%d"), extension&7, (extension>>12)&7);
	}
	wk[2][0] = 0;
	join_words(str, len ,wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_eor_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("EOR.B"), true, SIZE_8BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eor_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("EOR.W"), true, SIZE_16BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eor_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("EOR.L"), true, SIZE_32BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eori_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("EORI.B"), true, SIZE_8BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eori_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("EORI.W"), true, SIZE_16BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eori_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("EORI.L"), true, SIZE_32BITS, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eori_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("EORI"), true, SIZE_8BITS, REG_CCR, ops, opslen ,str, len);
}

int MC68000DASM::d68000_eori_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("EORI"), true, SIZE_16BITS, REG_SR, ops, opslen ,str, len);
}

int MC68000DASM::d68000_exg_dd(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("EXG"), true, ops, opslen ,str, len);
}

int MC68000DASM::d68000_exg_aa(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ar_ar_s_d_str(_T("EXG"), true, ops, opslen ,str, len);
}

int MC68000DASM::d68000_exg_da(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ar_s_d_str(_T("EXG"), true, ops, opslen ,str, len);
}

int MC68000DASM::d68000_ext_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_str(_T("EXT.W"), true, ops, opslen ,str, len);
}

int MC68000DASM::d68000_ext_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_str(_T("EXT.L"), true, ops, opslen ,str, len);
}

int MC68000DASM::d68020_extb_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_dr_str(_T("EXTB.L"), CPU_TYPE_IS_020_PLUS() != 0, ops, opslen ,str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68881_ftrap(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_020_PLUS() != 0;
	int opscnt = 1;

	uint16_t w2 = ops[opscnt++];

	switch (ops[0] & 0x7)
	{
		case 2: // word operand
			make_name_str(m_cpcc[w2 & 0x3f], c, str, len, _T("FTRAP"), _T(".W"));
			UTILITY::sntprintf(str, len, _T("$%04x"), ops[opscnt++]);
			break;
		case 3: // long word operand
			make_name_str(m_cpcc[w2 & 0x3f], c, str, len, _T("FTRAP"), _T(".L"));
			UTILITY::sntprintf(str, len, _T("$%08x"), read_op32(ops, opscnt));
			break;
		case 4: // no operand
			make_name_str(m_cpcc[w2 & 0x3f], c, str, len, _T("FTRAP"));
			break;
		default:
			{
				_TCHAR val[4];
				UTILITY::stprintf(val, 4, _T("<%d>"), ops[0] & 7);
				make_name_str(m_cpcc[w2 & 0x3f], c, str, len, _T("FTRAP"), val);
			}
			break;
	}
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_fpu(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_020_PLUS() != 0;
	int opscnt = 1;

	const _TCHAR float_data_format[8][3] = {
		_T(".L"), _T(".S"), _T(".X"), _T(".P"), _T(".W"), _T(".D"), _T(".B"), _T(".P")
	};

	uint16_t w2 = ops[opscnt++];

	uint16_t src = (w2 >> 10) & 0x7;
	uint16_t dst_reg = (w2 >> 7) & 0x7;

	// special override for FMOVECR
	if ((((w2 >> 13) & 0x7) == 2) && (((w2>>10)&0x7) == 7))
	{
		make_name_str(_T("FMOVECR"), c, str, len);
		UTILITY::sntprintf(str, len, _T("#$%0x,fp%d"), (w2&0x7f), dst_reg);
		return opscnt;
	}

	const _TCHAR *mnemonic_list[] = {
		_T("FMOVE"), _T("FINT"), _T("FSINH"), _T("FINTRZ"), _T("FSQRT"), NULL /*_T("?5?")*/, _T("FLOGNP1"), NULL /*_T("?7?")*/,
		_T("FETOXM1"), _T("FTANH1"), _T("FATAN"), NULL /*_T("?B?")*/, _T("FASIN"), _T("FATANH"), _T("FSIN"), _T("FTAN"),
		_T("FETOX"), _T("FTWOTOX"), _T("FTENTOX"), NULL /*_T("?13")*/, _T("FLOGN"), _T("FLOG10"), _T("FLOG2"), NULL /*_T("?17")*/,
		_T("FABS"), _T("FCOSH"), _T("FNEG"), NULL /*_T("?1B")*/, _T("FACOS"), _T("FCOS"), _T("FGETEXP"), _T("FGETMAN"),
		_T("FDIV"), _T("FMOD"), _T("FADD"), _T("FMUL"), _T("FSGLDIV"), _T("FREM"), _T("FSCALE"), _T("FSGLMUL"),
		_T("FSUB"), NULL /*_T("?29")*/, NULL /*_T("?2A")*/, NULL /*_T("?2B")*/, NULL /*_T("?2C")*/, NULL /*_T("?2D")*/, NULL /*_T("?2E")*/, NULL /*_T("?2F")*/,
		_T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"), _T("FSINCOS"),
		_T("FCMP"), NULL /*_T("?39")*/, _T("FTST"), NULL /*_T("?3B")*/, NULL /*_T("?3C")*/, NULL /*_T("?3D")*/, NULL /*_T("?3E")*/, NULL /*_T("?3F")*/,
		NULL /*_T("?40")*/, _T("FSSQRT"), NULL /*_T("?42")*/, NULL /*_T("?43")*/, NULL /*_T("?44")*/, _T("FDSQRT"), NULL /*_T("?46")*/, NULL /*_T("?47")*/,
		NULL /*_T("?48")*/, NULL /*_T("?49")*/, NULL /*_T("?4A")*/, NULL /*_T("?4B")*/, NULL /*_T("?4C")*/, NULL /*_T("?4D")*/, NULL /*_T("?4E")*/, NULL /*_T("?4F")*/,
		NULL /*_T("?50")*/, NULL /*_T("?51")*/, NULL /*_T("?52")*/, NULL /*_T("?53")*/, NULL /*_T("?54")*/, NULL /*_T("?55")*/, NULL /*_T("?56")*/, NULL /*_T("?57")*/,
		_T("FSABS"), NULL /*_T("?59")*/, _T("FSNEG"), NULL /*_T("?5B")*/, _T("FDABS"), NULL /*_T("?5D")*/, _T("FDNEG"), NULL /*_T("?5F")*/,
		_T("FSDIV"), NULL /*_T("?61")*/, _T("FSADD"), _T("FSMUL"), _T("FDDIV"), NULL /*_T("?65")*/, _T("FDADD"), _T("FDMUL"),
		_T("FSSUB"), NULL /*_T("?69")*/, NULL /*_T("?6A")*/, NULL /*_T("?6B")*/, _T("FDSUB"), NULL /*_T("?6D")*/, NULL /*_T("?6E")*/, NULL /*_T("?6F")*/,
	};

	_TCHAR mnemonic[16];
	int mnemolen = 16;
	switch ((w2 >> 13) & 0x7)
	{
		case 0x0:
		case 0x2:
		{
			int ww2 = (w2 & 0x7f);
			if (ww2 < 0x70 && mnemonic_list[ww2] != NULL) {
				UTILITY::tcscpy(mnemonic, mnemolen, mnemonic_list[ww2]);
			} else {
				UTILITY::stprintf(mnemonic, mnemolen, _T("F%02X?"), ww2);
			}

			if (w2 & 0x4000)
			{
				make_name_str(mnemonic, c, str, len, NULL, float_data_format[src]);
				opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
				UTILITY::sntprintf(str, len, _T(",FP%d"), dst_reg);
			}
			else
			{
				UTILITY::sntprintf(str, len, _T("FP%d,FP%d"), src, dst_reg);
			}
			break;
		}

		case 0x3:
		{
			switch ((w2>>10)&7)
			{
				case 3:     // packed decimal w/fixed k-factor
					make_name_str(_T("FMOVE"), c, str, len, NULL, float_data_format[(w2>>10)&7]);
					UTILITY::sntprintf(str, len, _T("FP%d,"), dst_reg);
					opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
					UTILITY::sntprintf(str, len, _T(" {#%d}"), sext_7bit_int(w2&0x7f));
					break;

				case 7:     // packed decimal w/dynamic k-factor (register)
					make_name_str(_T("FMOVE"), c, str, len, NULL, float_data_format[(w2>>10)&7]);
					UTILITY::sntprintf(str, len, _T("FP%d,"), dst_reg);
					opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
					UTILITY::sntprintf(str, len, _T(" {D%d}"), (w2>>4)&7);
					break;

				default:
					make_name_str(_T("FMOVE"), c, str, len, NULL, float_data_format[(w2>>10)&7]);
					UTILITY::sntprintf(str, len, _T("FP%d,"), float_data_format[(w2>>10)&7], dst_reg);
					opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
					break;
			}
			break;
		}

		case 0x4:   // ea to control
		{
			make_name_str(_T("FMOVEM.L"), c, str, len);
			opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
			UTILITY::tcscat(str, len, _T(","));
			if (w2 & 0x1000) UTILITY::tcscat(str, len, _T("fpcr"));
			if (w2 & 0x0800) UTILITY::tcscat(str, len, _T("/fpsr"));
			if (w2 & 0x0400) UTILITY::tcscat(str, len, _T("/fpiar"));
			break;
		}

		case 0x5:   // control to ea
		{
			make_name_str(_T("FMOVEM.L"), c, str, len);
			if (w2 & 0x1000) UTILITY::tcscat(str, len, _T("fpcr"));
			if (w2 & 0x0800) UTILITY::tcscat(str, len, _T("/fpsr"));
			if (w2 & 0x0400) UTILITY::tcscat(str, len, _T("/fpiar"));
			UTILITY::tcscat(str, len, _T(","));
			opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
			break;
		}

		case 0x6:   // memory to FPU, list
		{
			if ((w2>>11) & 1)   // dynamic register list
			{
				make_name_str(_T("FMOVEM.X"), c, str, len);
				opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
				UTILITY::sntprintf(str, len, _T(",D%d"), (w2>>4)&7);
			}
			else    // static register list
			{
				make_name_str(_T("FMOVEM.X"), c, str, len);
				opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
				UTILITY::tcscat(str, len, _T(","));

				for (int i = 0; i < 8; i++)
				{
					if (w2 & (1<<i))
					{
						if ((w2>>12) & 1)   // postincrement or control
						{
							UTILITY::sntprintf(str, len, _T("FP%d "), 7-i);
						}
						else            // predecrement
						{
							UTILITY::sntprintf(str, len, _T("FP%d "), i);
						}
					}
				}
			}
			break;
		}

		case 0x7:   // FPU to memory, list
		{
			if ((w2>>11) & 1)   // dynamic register list
			{
				make_name_str(_T("FMOVEM.X"), c, str, len);
				UTILITY::sntprintf(str, len, _T("D%d,"), (w2>>4)&7);
				opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
			}
			else    // static register list
			{
				make_name_str(_T("FMOVEM.X"), c, str, len);

				for (int i = 0; i < 8; i++)
				{
					if (w2 & (1<<i))
					{
						if ((w2>>12) & 1)   // postincrement or control
						{
							UTILITY::sntprintf(str, len, _T("FP%d "), 7-i);
						}
						else            // predecrement
						{
							UTILITY::sntprintf(str, len, _T("FP%d "), i);
						}
					}
				}
				UTILITY::tcscat(str, len, _T(","));
				opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, str, len);
			}
			break;
		}
		default:
			UTILITY::sntprintf(str, len, _T("FPU (?) "));
			break;
	}
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_jmp(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("JMP"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_jsr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("JSR"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_lea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("LEA"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_link_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ar_simm_str(_T("LINK"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_link_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_ar_simm_str(_T("LINK"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_lsr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("LSR.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("LSL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("LSL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_lsl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("LSL.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ea_str(_T("MOVE.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ea_str(_T("MOVE.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ea_str(_T("MOVE.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movea_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("MOVEA.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movea_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("MOVEA.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ccrsr_str(_T("MOVE"), true, SIZE_8BITS, REG_CCR, ops, opslen, str, len);
}

int MC68000DASM::d68010_move_fr_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_ccrsr_ea_str(_T("MOVE"), CPU_TYPE_IS_010_PLUS() != 0, SIZE_8BITS, REG_CCR, ops, opslen, str, len);
	make_comment_str(_T("V1+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_move_fr_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ccrsr_ea_str(_T("MOVE"), true, SIZE_16BITS, REG_SR, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ccrsr_str(_T("MOVE"), true, SIZE_16BITS, REG_SR, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_fr_usp(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ccrsr_ar_str(_T("MOVE"), true, REG_USP, ops, opslen, str, len);
}

int MC68000DASM::d68000_move_to_usp(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ar_ccrsr_str(_T("MOVE"), true, REG_USP, ops, opslen, str, len);
}

int MC68000DASM::d68010_movec(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	bool c = false;
	int opscnt = 1;

	_TCHAR reg_name[8]; int reg_nlen = 8;
	_TCHAR processor[8]; int proc_len = 8;

	uint16_t extension = ops[opscnt++];

	switch(extension & 0xfff)
	{
		case 0x000:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("SFC"));
			UTILITY::tcscpy(processor, proc_len, _T("1+"));
			c = CPU_TYPE_IS_010_PLUS() != 0;
			break;
		case 0x001:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("DFC"));
			UTILITY::tcscpy(processor, proc_len, _T("1+"));
			c = CPU_TYPE_IS_010_PLUS() != 0;
			break;
		case 0x800:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("USP"));
			UTILITY::tcscpy(processor, proc_len, _T("1+"));
			c = CPU_TYPE_IS_010_PLUS() != 0;
			break;
		case 0x801:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("VBR"));
			UTILITY::tcscpy(processor, proc_len, _T("1+"));
			c = CPU_TYPE_IS_010_PLUS() != 0;
			break;
		case 0x002:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("CACR"));
			UTILITY::tcscpy(processor, proc_len, _T("2+"));
			c = CPU_TYPE_IS_020_PLUS() != 0;
			break;
		case 0x802:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("CAAR"));
			UTILITY::tcscpy(processor, proc_len, _T("2,3"));
			c = CPU_TYPE_IS_020_PLUS() != 0;
			break;
		case 0x803:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("MSP"));
			UTILITY::tcscpy(processor, proc_len, _T("2+"));
			c = CPU_TYPE_IS_020_PLUS() != 0;
			break;
		case 0x804:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("ISP"));
			UTILITY::tcscpy(processor, proc_len, _T("2+"));
			c = CPU_TYPE_IS_020_PLUS() != 0;
			break;
		case 0x003:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("TC"));
			UTILITY::tcscpy(processor, proc_len, _T("4+"));
			c = CPU_TYPE_IS_040_PLUS() != 0;
			break;
		case 0x004:
			if(m_cpu_type & CPU_TYPE_COLDFIRE)
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ACR0"));
				UTILITY::tcscpy(processor, proc_len, _T("CF"));
			}
			else
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ITT0"));
				UTILITY::tcscpy(processor, proc_len, _T("4+"));
				c = CPU_TYPE_IS_040_PLUS() != 0;
			}
			break;
		case 0x005:
			if(m_cpu_type & CPU_TYPE_COLDFIRE)
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ACR1"));
				UTILITY::tcscpy(processor, proc_len, _T("CF"));
			}
			else
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ITT1"));
				UTILITY::tcscpy(processor, proc_len, _T("4+"));
				c = CPU_TYPE_IS_040_PLUS() != 0;
			}
			break;
		case 0x006:
			if(m_cpu_type & CPU_TYPE_COLDFIRE)
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ACR2"));
				UTILITY::tcscpy(processor, proc_len, _T("CF"));
			}
			else
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("DTT0"));
				UTILITY::tcscpy(processor, proc_len, _T("4+"));
				c = CPU_TYPE_IS_040_PLUS() != 0;
			}
			break;
		case 0x007:
			if(m_cpu_type & CPU_TYPE_COLDFIRE)
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("ACR3"));
				UTILITY::tcscpy(processor, proc_len, _T("CF"));
			}
			else
			{
				UTILITY::tcscpy(reg_name, reg_nlen, _T("DTT1"));
				UTILITY::tcscpy(processor, proc_len, _T("4+"));
				c = CPU_TYPE_IS_040_PLUS() != 0;
			}
			break;
		case 0x805:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("MMUSR"));
			UTILITY::tcscpy(processor, proc_len, _T("4+"));
			c = CPU_TYPE_IS_040_PLUS() != 0;
			break;
		case 0x806:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("URP"));
			UTILITY::tcscpy(processor, proc_len, _T("4+"));
			c = CPU_TYPE_IS_040_PLUS() != 0;
			break;
		case 0x807:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("SRP"));
			UTILITY::tcscpy(processor, proc_len, _T("4+"));
			c = CPU_TYPE_IS_040_PLUS() != 0;
			break;
		case 0xc00:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("ROMBAR0"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc01:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("ROMBAR1"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc04:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("RAMBAR0"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc05:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("RAMBAR1"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc0c:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("MPCR"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc0d:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("EDRAMBAR"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc0e:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("SECMBAR"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		case 0xc0f:
			UTILITY::tcscpy(reg_name, reg_nlen, _T("MBAR"));
			UTILITY::tcscpy(processor, proc_len, _T("CF"));
			break;
		default:
			make_signed_str_16(extension & 0xfff, reg_name, reg_nlen);
			UTILITY::tcscpy(processor, proc_len, _T("?"));
	}

	make_name_str(_T("MOVEC"), c, str, len);

	if(BIT(ops[0], 0)) {
		wk[0][0] = 0; make_reg_direct_str(BIT(extension, 15) ? _T('A') : _T('D'), (extension>>12)&7, wk[0], WKWORD_LEN);
		wk[1][0] = 0; UTILITY::tcscpy(wk[1], WKWORD_LEN, reg_name);
	} else {
		wk[0][0] = 0; UTILITY::tcscpy(wk[0], WKWORD_LEN, reg_name);
		wk[1][0] = 0; make_reg_direct_str(BIT(extension, 15) ? _T('A') : _T('D'), (extension>>12)&7, wk[1], WKWORD_LEN);
	}
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(processor, str, len);
	return opscnt;
}

/// @param[in] reg : 'A' or 'D'
/// @param[in] data: bit mask data
/// @param[in] msb : if true, msb in data is D0 and next D1,D2... else A7,A6,A5...
/// @param[in] centre: if true set '/' at the first of string
/// @param[in,out] str: append formatted string
/// @param[in] len: str buffer length
bool MC68000DASM::d68000_movem_regs_temp(_TCHAR reg, uint8_t data, bool msb, bool centre, _TCHAR *str, int len)
{
	for(int i = 0; i < 8; i++) {
		uint8_t mask = (msb ? (1 << (7 - i)) : (1 << i));
		if(data & mask) {
			int first = i;
			int run_length = 0;
			i++;
			for(; i < 8; i++) { 
				mask = (msb ? (1 << (7 - i)) : (1 << i));
				if (!(data & mask)) break;
				run_length++;
			}
			if(centre) UTILITY::tcscat(str, len, _T("/"));
			UTILITY::sntprintf(str, len, _T("%c%d"), reg, first);
			centre = true;
			if(run_length > 0)
				UTILITY::sntprintf(str, len, _T("-%c%d"), reg, first + run_length);
		}
	}
	return centre;
}

int MC68000DASM::d68000_movem_pd_regs_temp(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint32_t data = ops[opscnt++];

	bool centre = false;
	// D0/D1/.../A6/A7
	centre = d68000_movem_regs_temp(_T('D'), (data >> 8) & 0xff, true, centre, str, len);
	centre = d68000_movem_regs_temp(_T('A'), data & 0xff, true, centre, str, len);

	return opscnt;
}

int MC68000DASM::d68000_movem_pd_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	wk[0][0] = 0;
	wk[1][0] = 0;
	int opscnt = d68000_movem_pd_regs_temp(ops, opslen, wk[0], WKWORD_LEN);
	make_name_str(name, true, str, len);
	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

int MC68000DASM::d68000_movem_pd_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_pd_temp(_T("MOVEM.W"), SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movem_pd_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_pd_temp(_T("MOVEM.L"), SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movem_erre_regs_temp(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	uint32_t data = ops[opscnt++];

	bool centre = false;
	// A7/A6/.../D1/D0
	centre = d68000_movem_regs_temp(_T('D'), data & 0xff, false, centre, str, len);
	centre = d68000_movem_regs_temp(_T('A'), (data >> 8) & 0xff, false, centre, str, len);

	return opscnt;
}

int MC68000DASM::d68000_movem_er_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	wk[0][0] = 0;
	wk[1][0] = 0;
	int opscnt = d68000_movem_erre_regs_temp(ops, opslen, wk[1], WKWORD_LEN);
	make_name_str(name, true, str, len);
	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

int MC68000DASM::d68000_movem_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_er_temp(_T("MOVEM.W"), SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movem_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_er_temp(_T("MOVEM.L"), SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movem_re_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	wk[0][0] = 0;
	wk[1][0] = 0;
	int opscnt = d68000_movem_erre_regs_temp(ops, opslen, wk[0], WKWORD_LEN);
	make_name_str(name, true, str, len);
	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

int MC68000DASM::d68000_movem_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_re_temp(_T("MOVEM.W"), SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movem_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movem_re_temp(_T("MOVEM.L"), SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_movep_re_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[4][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, true, str, len);
	wk[0][0] = 0;	make_data_reg_direct_str((ops[0] >> 9) & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("($%X"), ops[opscnt++]);
	wk[2][0] = 0;	make_address_reg_direct_str(ops[0] & 7, wk[2], WKWORD_LEN);
					UTILITY::tcscat(wk[2], WKWORD_LEN, _T(")"));
	wk[3][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

int MC68000DASM::d68000_movep_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movep_re_temp(_T("MOVEP.W"), ops, opslen, str, len);
}

int MC68000DASM::d68000_movep_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movep_re_temp(_T("MOVEP.L"), ops, opslen, str, len);
}

int MC68000DASM::d68000_movep_er_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[4][WKWORD_LEN];
	int opscnt = 1;
	make_name_str(name, true, str, len);
	wk[0][0] = 0;	UTILITY::sntprintf(wk[0], WKWORD_LEN, _T("($%X"), ops[opscnt++]);
	wk[1][0] = 0;	make_address_reg_direct_str(ops[0] & 7, wk[1], WKWORD_LEN);
					UTILITY::tcscat(wk[1], WKWORD_LEN, _T(")"));
	wk[2][0] = 0;	make_data_reg_direct_str((ops[0] >> 9) & 7, wk[2], WKWORD_LEN);
	wk[3][0] = 0;
	join_words(str, len, wk);
	return opscnt;
}

int MC68000DASM::d68000_movep_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movep_er_temp(_T("MOVEP.W"), ops, opslen, str, len);
}

int MC68000DASM::d68000_movep_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68000_movep_er_temp(_T("MOVEP.L"), ops, opslen, str, len);
}

int MC68000DASM::d68010_moves_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;

	uint16_t extension = ops[opscnt++];
	make_name_str(name, CPU_TYPE_IS_010_PLUS() != 0, str, len);
	int pos = BIT(extension, 11);
	wk[0][0] = 0;
	wk[1][0] = 0;
	make_reg_direct_str(BIT(extension, 15) ? _T('A') : _T('D'), (extension >> 12) & 7, wk[1-pos], WKWORD_LEN);
	opscnt += make_ea_mode_str((ops[0] >> 3) & 7, ops[0] & 7, immsize, &ops[opscnt], opslen - opscnt, wk[pos], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V1+"), str, len);
	return opscnt;
}

int MC68000DASM::d68010_moves_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68010_moves_temp(_T("MOVES.B"), SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68010_moves_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68010_moves_temp(_T("MOVES.W"), SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68010_moves_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68010_moves_temp(_T("MOVES.L"), SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_moveq(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];

	make_name_str(_T("MOVEQ"), true, str, len);
	wk[0][0] = 0;	UTILITY::tcscat(wk[0], WKWORD_LEN, _T("#"));
					make_signed_str_8((uint8_t)ops[0], wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_data_reg_direct_str((ops[0] >> 9) & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	return 1;
}

int MC68000DASM::d68040_move16_pi_pi(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	make_name_str(_T("MOVE16"), CPU_TYPE_IS_040_PLUS() != 0, str, len);
	make_reg_reg_indirect_postinc_str(_T('A'), ops[0] & 7, _T('A'), (ops[opscnt++] >> 12) & 7, str, len);
	make_comment_str(_T("V4"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_move16_pi_al(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];

	int opscnt = 1;
	make_name_str(_T("MOVE16"), CPU_TYPE_IS_040_PLUS() != 0, str, len);
	wk[0][0] = 0;	make_address_reg_indirect_postinc_str(ops[0] & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_immediage_str_u32(&ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V4"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_move16_al_pi(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];

	int opscnt = 1;
	make_name_str(_T("MOVE16"), CPU_TYPE_IS_040_PLUS() != 0, str, len);
	wk[0][0] = 0;	opscnt += make_immediage_str_u32(&ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	make_address_reg_indirect_postinc_str(ops[0] & 7, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V4"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_move16_ai_al(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68040_move16_pi_al(ops, opslen, str, len);
}

int MC68000DASM::d68040_move16_al_ai(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68040_move16_al_pi(ops, opslen, str, len);
}

int MC68000DASM::d68000_muls(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("MULS.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_mulu(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("MULU.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_mull(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];

	int opscnt = 1;

	uint16_t extension = ops[opscnt++];

	make_name_str(BIT(extension, 11) ? _T("S") : _T("U"), CPU_TYPE_IS_020_PLUS() != 0, str, len, _T("MUL"), _T(".L"));
	wk[0][0] = 0;	opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, wk[0], WKWORD_LEN);
	wk[1][0] = 0;
	if(BIT(extension, 10)) {
		UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("D%d-D%d"), extension&7, (extension>>12)&7);
	} else {
		UTILITY::sntprintf(wk[1], WKWORD_LEN, _T("D%d"), (extension>>12)&7);
	}
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_nbcd(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NBCD"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_neg_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEG.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_neg_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEG.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_neg_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEG.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_negx_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEGX.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_negx_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEGX.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_negx_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NEGX.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_nop(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("NOP"), true, str, len);
	return 1;
}

int MC68000DASM::d68000_not_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NOT.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_not_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NOT.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_not_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("NOT.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("OR.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("OR.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("OR.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("OR.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("OR.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_or_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("OR.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_ori_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ORI.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_ori_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ORI.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_ori_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ea_str(_T("ORI.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_ori_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("ORI"), true, SIZE_8BITS, REG_CCR, ops, opslen, str, len);
}

int MC68000DASM::d68000_ori_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_uimm_ccrsr_str(_T("ORI"), true, SIZE_16BITS, REG_SR, ops, opslen, str, len);
}

int MC68000DASM::d68020_pack_unpk_rr(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;

	make_name_str(name, CPU_TYPE_IS_020_PLUS() != 0, str, len);
	wk[0][0] = 0;	make_reg_reg_direct_str(_T('D'), ops[0] & 7, _T('D'), (ops[0] >> 9) & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_immediage_str_u16(&ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_pack_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_pack_unpk_rr(_T("PACK"), ops, opslen, str, len);
}

int MC68000DASM::d68020_pack_unpk_mm(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	_TCHAR wk[3][WKWORD_LEN];
	int opscnt = 1;

	make_name_str(name, CPU_TYPE_IS_020_PLUS() != 0, str, len);
	wk[0][0] = 0;	make_reg_reg_indirect_predec_str(_T('A'), ops[0] & 7, _T('A'), (ops[0] >> 9) & 7, wk[0], WKWORD_LEN);
	wk[1][0] = 0;	opscnt += make_immediage_str_u16(&ops[opscnt], opslen - opscnt, wk[1], WKWORD_LEN);
	wk[2][0] = 0;
	join_words(str, len, wk);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_pack_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_pack_unpk_mm(_T("PACK"), ops, opslen, str, len);
}

int MC68000DASM::d68000_pea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("PEA"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_reset(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("RESET"), true, str, len);
	return 1;
}

int MC68000DASM::d68000_ror_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_ror_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ROR.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_rol_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ROL.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXR.W"), true, ops, opslen, str, len);
}


int MC68000DASM::d68000_roxr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXR.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXR.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXR.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ROXR.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_dr_str(_T("ROXL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXL.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXL.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_s_d_str(_T("ROXL.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_roxl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("ROXL.W"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68010_rtd(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_simm_str(_T("RTD"), CPU_TYPE_IS_010_PLUS() != 0, SIZE_16BITS, ops, opslen, str, len);
	make_comment_str(_T("V1+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_rte(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("RTE"), true, str, len);
	return 1;
}

int MC68000DASM::d68020_rtm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("RTM"), CPU_TYPE_IS_020_PLUS() != 0, str, len);
	make_reg_direct_str(BIT(ops[0], 3) ? _T('A') : _T('D'), ops[0] & 7, str, len);
	make_comment_str(_T("V2+"), str, len);
	return 1;
}

int MC68000DASM::d68000_rtr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("RTR"), true, str, len);
	return 1;
}

int MC68000DASM::d68000_rts(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("RTS"), true, str, len);
	return 1;
}

int MC68000DASM::d68000_sbcd_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("SBCD"), true, ops, opslen, str, len); 
}

int MC68000DASM::d68000_sbcd_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("SBCD"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_scc(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(m_bcc[(ops[0] >> 8) & 0xf], true, SIZE_8BITS, ops, opslen, str, len, _T("S"));
}

int MC68000DASM::d68000_stop(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_str(_T("STOP"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("SUB.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("SUB.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_dr_str(_T("SUB.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("SUB.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("SUB.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_sub_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_ea_str(_T("SUB.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_suba_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("SUB.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_suba_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_ar_str(_T("SUB.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("SUBI.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("SUBI.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_simm_ea_str(_T("SUBI.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subq_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("SUBQ.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subq_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("SUBQ.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subq_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_qimm_ea_str(_T("SUBQ.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_rr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("SUBX.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_rr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("SUBX.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_rr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_dr_d_s_str(_T("SUBX.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_mm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("SUBX.B"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_mm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("SUBX.W"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_subx_mm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_miar_miar_str(_T("SUBX.L"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_swap(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_dr_str(_T("SWAP"), true, ops, opslen, str, len);
}

int MC68000DASM::d68000_tas(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("TAS"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68000_trap(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("TRAP"), true, str, len);
	UTILITY::sntprintf(str, len, _T("#$%x"), ops[0] & 0xf);
	return 1;
}

int MC68000DASM::d68020_trapcc_0(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("TRAP"), CPU_TYPE_IS_020_PLUS() != 0, str, len, NULL, m_bcc[(ops[0] >> 8) & 0xf]);
	make_comment_str(_T("V2+"), str, len);
	return 1;
}

int MC68000DASM::d68020_trapcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	make_name_str(_T("TRAP"), CPU_TYPE_IS_020_PLUS() != 0, str, len, NULL, m_bcc[(ops[0] >> 8) & 0xf]);
	opscnt += make_immediage_str_u16(&ops[opscnt], opslen - opscnt, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_trapcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = 1;
	make_name_str(_T("TRAP"), CPU_TYPE_IS_020_PLUS() != 0, str, len, NULL, m_bcc[(ops[0] >> 8) & 0xf]);
	opscnt += make_immediage_str_u32(&ops[opscnt], opslen - opscnt, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68000_trapv(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("TRAPV"), true, str, len);
	return 1;
}

int MC68000DASM::d68000_tst_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("TST.B"), true, SIZE_8BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_pcdi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_ea_str(_T("TST.B"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_8BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_tst_pcix_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_pcdi_8(ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_i_8(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_pcdi_8(ops, opslen, str, len);
}

int MC68000DASM::d68000_tst_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("TST.W"), true, SIZE_16BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_a_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_ea_str(_T("TST.W"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_16BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_tst_pcdi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_16(ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_pcix_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_16(ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_i_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_16(ops, opslen, str, len);
}

int MC68000DASM::d68000_tst_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return make_ea_str(_T("TST.L"), true, SIZE_32BITS, ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_a_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	int opscnt = make_ea_str(_T("TST.L"), CPU_TYPE_IS_020_PLUS() != 0, SIZE_32BITS, ops, opslen, str, len);
	make_comment_str(_T("V2+"), str, len);
	return opscnt;
}

int MC68000DASM::d68020_tst_pcdi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_32(ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_pcix_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_32(ops, opslen, str, len);
}

int MC68000DASM::d68020_tst_i_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_tst_a_32(ops, opslen, str, len);
}

int MC68000DASM::d68000_unlk(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	make_name_str(_T("UNLK"), true, str, len);
	UTILITY::sntprintf(str, len, _T("A%d"), ops[0] & 7);
	return 1;
}

int MC68000DASM::d68020_unpk_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_pack_unpk_rr(_T("UNPK"), ops, opslen, str, len);
}

int MC68000DASM::d68020_unpk_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	return d68020_pack_unpk_mm(_T("UNPK"), ops, opslen, str, len);
}

void MC68000DASM::fc_to_string(uint16_t modes, _TCHAR *str, int len)
{
	uint16_t fc = modes & 0x1f;

	if (fc == 0)
	{
		UTILITY::tcscat(str, len, _T("%sFC"));
	}
	else if (fc == 1)
	{
		UTILITY::tcscat(str, len, _T("%dFC"));
	}
	else if ((fc >> 3) == 1)
	{
		UTILITY::sntprintf(str, len, _T("D%d"), fc & 7);
	}
	else if ((fc >> 3) == 2)
	{
		UTILITY::sntprintf(str, len, _T("#%d"), fc & 7);
	}
	else {
		UTILITY::sntprintf(str, len, _T("unknown fc %X"), fc);
	}
}

int MC68000DASM::d68040_p000(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_040_PLUS() != 0;

	if ((ops[0] & 0xffd8) == 0xf548) // 68040 PTEST
	{
		make_name_str(_T("PTEST"), c, str, len, NULL, (ops[0] & 0x20) ? _T("R") : _T("W"));
		UTILITY::sntprintf(str, len, _T("(A%d)"), ops[0] & 7);
	}
	else if ((ops[0] & 0xffe0) == 0xf500) // 68040 PFLUSH
	{
		switch((ops[0] >> 3) & 3)
		{
		case 0:
			make_name_str(_T("PFLUSHN"), c, str, len);
			UTILITY::sntprintf(str, len, _T("(A%d)"), ops[0] & 7);
			break;
		case 1:
			make_name_str(_T("PFLUSH"), c, str, len);
			UTILITY::sntprintf(str, len, _T("(A%d)"), ops[0] & 7);
			break;
		case 2:
			make_name_str(_T("PFLUSHAN"), c, str, len);
			break;
		case 3:
			make_name_str(_T("PFLUSHA"), c, str, len);
			break;
		default:
			make_name_str(_T("PF?"), c, str, len);
			UTILITY::sntprintf(str, len, _T("%04X"), ops[0]);
			break;
		}
	}
	make_comment_str(_T("V4"), str, len);
	return 1;
}
// PFLUSH:  001xxx0xxxxxxxxx
// PLOAD:   001000x0000xxxxx
// PVALID1: 0010100000000000
// PVALID2: 0010110000000xxx
// PMOVE 1: 010xxxx000000000
// PMOVE 2: 011xxxx0000xxx00
// PMOVE 3: 011xxxx000000000
// PTEST:   100xxxxxxxxxxxxx
// PFLUSHR:  1010000000000000
int MC68000DASM::d68851_p000(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_030_PLUS() != 0;
	int opscnt = 1;

	uint16_t modes = ops[opscnt++];

	// do this after fetching the second PMOVE word so we properly get the 3rd if necessary
	_TCHAR reg[32]; int reglen = 32; reg[0] = 0;
	opscnt += make_ea_mode_str_32((ops[0] >> 3) & 7, ops[0] & 7, &ops[opscnt], opslen - opscnt, reg, reglen);

	if ((modes & 0xfde0) == 0x2000) // PLOAD
	{
		make_name_str(_T("PLOAD"), c, str, len);
		if (modes & 0x0200)
		{
			UTILITY::sntprintf(str, len, _T("#%d,%s"), (modes>>10)&7, reg);
		}
		else
		{
			UTILITY::sntprintf(str, len, _T("%s,#%d"), reg, (modes>>10)&7);
		}
	}
	else if ((modes & 0xe200) == 0x2000) // PFLUSH
	{
		make_name_str(_T("PFLUSH"), c, str, len);
		UTILITY::sntprintf(str, len, _T("%X,%X,%s"), modes & 0x1f, (modes>>5)&0xf, reg);
	}
	else if (modes == 0xa000)    // PFLUSHR
	{
		make_name_str(_T("PFLUSHR"), c, str, len);
		UTILITY::sntprintf(str, len, _T("%s"), reg);
	}
	else if (modes == 0x2800)    // PVALID (FORMAT 1)
	{
		make_name_str(_T("PVALID"), c, str, len);
		UTILITY::sntprintf(str, len, _T("VAL,%s"), reg);
	}
	else if ((modes & 0xfff8) == 0x2c00) // PVALID (FORMAT 2)
	{
		make_name_str(_T("PVALID"), c, str, len);
		UTILITY::sntprintf(str, len, _T("A%d,%s"), modes & 0xf, reg);
	}
	else if ((modes & 0xe000) == 0x8000) // PTEST
	{
		make_name_str(_T("PTEST"), c, str, len, NULL, (modes & 0x200) ? _T("R") : _T("W"));
		if (modes & 0x100)
		{
			UTILITY::sntprintf(str, len, _T("%s,"), reg);
			fc_to_string(modes, str, len);
			UTILITY::sntprintf(str, len, _T(",%s,%d,@A%d"),
							(modes >> 10) & 7,
							(modes >> 5) & 7);
		}
		else
		{
			fc_to_string(modes, str, len);
			UTILITY::sntprintf(str, len, _T(",%s,%d"),
							reg,
							(modes >> 10) & 7);
		}
	}
	else
	{
		switch ((modes>>13) & 0x7)
		{
		case 0: // MC68030/040 form with FD bit
		case 2: // MC68881 form, FD never set
			if (modes & 0x0100)
			{
				make_name_str(_T("PMOVEFD"), c, str, len);
				if (modes & 0x0200)
				{
					UTILITY::sntprintf(str, len, _T("%s,%s"), m_mmuregs[(modes>>10)&7], reg);
				}
				else
				{
					UTILITY::sntprintf(str, len, _T("%s,%s"), reg, m_mmuregs[(modes>>10)&7]);
				}
			}
			else
			{
				make_name_str(_T("PMOVE"), c, str, len);
				if (modes & 0x0200)
				{
					UTILITY::sntprintf(str, len, _T("%s,%s"), m_mmuregs[(modes>>10)&7], reg);
				}
				else
				{
					UTILITY::sntprintf(str, len, _T("%s,%s"), reg, m_mmuregs[(modes>>10)&7]);
				}
			}
			break;

		case 3: // MC68030 to/from status reg
			make_name_str(_T("PMOVE"), c, str, len);
			if (modes & 0x0200)
			{
				UTILITY::sntprintf(str, len, _T("MMUSR,%s"), reg);
			}
			else
			{
				UTILITY::sntprintf(str, len, _T("%s,MMUSR"), reg);
			}
			break;
		default:
			make_name_str(_T("PMOVE?"), c, str, len);
			UTILITY::sntprintf(str, len, _T("[unknown form] %s"), reg);
			break;
		}
	}
	make_comment_str(_T("V3+"), str, len);
	return opscnt;
}

int MC68000DASM::d68851_pbcc16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_030_PLUS() != 0;
	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;

	make_name_str(_T("PB"), c, str, len, NULL,  m_mmucond[ops[0] & 0xf]);
	UTILITY::sntprintf(str, len, _T("%X"), temp_pc + 2 + make_int_16(ops[opscnt++]));
	make_comment_str(_T("V3+"), str, len);
	return opscnt;
}

int MC68000DASM::d68851_pbcc32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_030_PLUS() != 0;
	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;

	make_name_str(_T("PB"), c, str, len, NULL,  m_mmucond[ops[0] & 0xf]);
	UTILITY::sntprintf(str, len, _T("%X"), temp_pc + 2 + make_int_32(read_op32(ops, opscnt)));
	make_comment_str(_T("V3+"), str, len);
	return opscnt;
}

int MC68000DASM::d68851_pdbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_030_PLUS() != 0;
	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;
//	uint16_t modes = ops[opscnt];
	opscnt++;

	make_name_str(_T("PB"), c, str, len, NULL,  m_mmucond[ops[0] & 0xf]);
	UTILITY::sntprintf(str, len, _T("%X"), temp_pc + 2 + make_int_16(ops[opscnt++]));
	make_comment_str(_T("V3+"), str, len);
	return opscnt;
}

// PScc:  0000000000xxxxxx
int MC68000DASM::d68851_p001(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_030_PLUS() != 0;
	make_name_str(_T("MMU 001 group"), c, str, len);
	make_comment_str(_T("V3+"), str, len);
	return 1;
}

// fbcc is 68040 and 68881
int MC68000DASM::d68040_fbcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_040_PLUS() != 0;

	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;
	int16_t disp = make_int_16(ops[opscnt++]);
	make_name_str(_T("FB"), c, str, len, NULL, m_cpcc[ops[0] & 0x3f]);
	UTILITY::sntprintf(str, len, _T("$%X"),temp_pc + 2 + disp);
	make_comment_str(_T("V4+"), str, len);
	return opscnt;
}

int MC68000DASM::d68040_fbcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len)
{
	bool c = CPU_TYPE_IS_040_PLUS() != 0;

	int opscnt = 1;
	uint32_t temp_pc = m_bag.pc;
	uint32_t disp = read_op32(ops, opscnt);
	make_name_str(_T("FB"), c, str, len, NULL, m_cpcc[ops[0] & 0x3f]);
	UTILITY::sntprintf(str, len, _T("$%X"), temp_pc + 2 + disp);
	make_comment_str(_T("V4+"), str, len);
	return opscnt;
}

/* ======================================================================== */
/* ======================= INSTRUCTION TABLE BUILDER ====================== */
/* ======================================================================== */

/* EA Masks:
800 = data register direct
400 = address register direct
200 = address register indirect
100 = ARI postincrement
 80 = ARI pre-decrement
 40 = ARI displacement
 20 = ARI index
 10 = absolute short
  8 = absolute long
  4 = immediate / sr
  2 = pc displacement
  1 = pc idx
*/

const MC68000DASM::opcode_struct MC68000DASM::m_opcode_info[] =
{
/*  opcode handler             mask    match   ea mask */
	{&MC68000DASM::d68000_illegal      , 0xffff, 0xffff, 0xfff},		
	{&MC68000DASM::d68000_1010         , 0xf000, 0xa000, 0x000},
	{&MC68000DASM::d68000_1111         , 0xf000, 0xf000, 0x000},
	{&MC68000DASM::d68000_abcd_rr      , 0xf1f8, 0xc100, 0x000},
	{&MC68000DASM::d68000_abcd_mm      , 0xf1f8, 0xc108, 0x000},
	{&MC68000DASM::d68000_add_er_8     , 0xf1c0, 0xd000, 0xbff},
	{&MC68000DASM::d68000_add_er_16    , 0xf1c0, 0xd040, 0xfff},
	{&MC68000DASM::d68000_add_er_32    , 0xf1c0, 0xd080, 0xfff},
	{&MC68000DASM::d68000_add_re_8     , 0xf1c0, 0xd100, 0x3f8},
	{&MC68000DASM::d68000_add_re_16    , 0xf1c0, 0xd140, 0x3f8},
	{&MC68000DASM::d68000_add_re_32    , 0xf1c0, 0xd180, 0x3f8},
	{&MC68000DASM::d68000_adda_16      , 0xf1c0, 0xd0c0, 0xfff},
	{&MC68000DASM::d68000_adda_32      , 0xf1c0, 0xd1c0, 0xfff},
	{&MC68000DASM::d68000_addi_8       , 0xffc0, 0x0600, 0xbf8},
	{&MC68000DASM::d68000_addi_16      , 0xffc0, 0x0640, 0xbf8},
	{&MC68000DASM::d68000_addi_32      , 0xffc0, 0x0680, 0xbf8},
	{&MC68000DASM::d68000_addq_8       , 0xf1c0, 0x5000, 0xbf8},
	{&MC68000DASM::d68000_addq_16      , 0xf1c0, 0x5040, 0xff8},
	{&MC68000DASM::d68000_addq_32      , 0xf1c0, 0x5080, 0xff8},
	{&MC68000DASM::d68000_addx_rr_8    , 0xf1f8, 0xd100, 0x000},
	{&MC68000DASM::d68000_addx_rr_16   , 0xf1f8, 0xd140, 0x000},
	{&MC68000DASM::d68000_addx_rr_32   , 0xf1f8, 0xd180, 0x000},
	{&MC68000DASM::d68000_addx_mm_8    , 0xf1f8, 0xd108, 0x000},
	{&MC68000DASM::d68000_addx_mm_16   , 0xf1f8, 0xd148, 0x000},
	{&MC68000DASM::d68000_addx_mm_32   , 0xf1f8, 0xd188, 0x000},
	{&MC68000DASM::d68000_and_er_8     , 0xf1c0, 0xc000, 0xbff},
	{&MC68000DASM::d68000_and_er_16    , 0xf1c0, 0xc040, 0xbff},
	{&MC68000DASM::d68000_and_er_32    , 0xf1c0, 0xc080, 0xbff},
	{&MC68000DASM::d68000_and_re_8     , 0xf1c0, 0xc100, 0x3f8},
	{&MC68000DASM::d68000_and_re_16    , 0xf1c0, 0xc140, 0x3f8},
	{&MC68000DASM::d68000_and_re_32    , 0xf1c0, 0xc180, 0x3f8},
	{&MC68000DASM::d68000_andi_to_ccr  , 0xffff, 0x023c, 0x000},
	{&MC68000DASM::d68000_andi_to_sr   , 0xffff, 0x027c, 0x000},
	{&MC68000DASM::d68000_andi_8       , 0xffc0, 0x0200, 0xbf8},
	{&MC68000DASM::d68000_andi_16      , 0xffc0, 0x0240, 0xbf8},
	{&MC68000DASM::d68000_andi_32      , 0xffc0, 0x0280, 0xbf8},
	{&MC68000DASM::d68000_asr_s_8      , 0xf1f8, 0xe000, 0x000},
	{&MC68000DASM::d68000_asr_s_16     , 0xf1f8, 0xe040, 0x000},
	{&MC68000DASM::d68000_asr_s_32     , 0xf1f8, 0xe080, 0x000},
	{&MC68000DASM::d68000_asr_r_8      , 0xf1f8, 0xe020, 0x000},
	{&MC68000DASM::d68000_asr_r_16     , 0xf1f8, 0xe060, 0x000},
	{&MC68000DASM::d68000_asr_r_32     , 0xf1f8, 0xe0a0, 0x000},
	{&MC68000DASM::d68000_asr_ea       , 0xffc0, 0xe0c0, 0x3f8},
	{&MC68000DASM::d68000_asl_s_8      , 0xf1f8, 0xe100, 0x000},
	{&MC68000DASM::d68000_asl_s_16     , 0xf1f8, 0xe140, 0x000},
	{&MC68000DASM::d68000_asl_s_32     , 0xf1f8, 0xe180, 0x000},
	{&MC68000DASM::d68000_asl_r_8      , 0xf1f8, 0xe120, 0x000},
	{&MC68000DASM::d68000_asl_r_16     , 0xf1f8, 0xe160, 0x000},
	{&MC68000DASM::d68000_asl_r_32     , 0xf1f8, 0xe1a0, 0x000},
	{&MC68000DASM::d68000_asl_ea       , 0xffc0, 0xe1c0, 0x3f8},
	{&MC68000DASM::d68000_bcc_8        , 0xf000, 0x6000, 0x000},
	{&MC68000DASM::d68000_bcc_16       , 0xf0ff, 0x6000, 0x000},
	{&MC68000DASM::d68020_bcc_32       , 0xf0ff, 0x60ff, 0x000},
	{&MC68000DASM::d68000_bchg_r       , 0xf1c0, 0x0140, 0xbf8},
	{&MC68000DASM::d68000_bchg_s       , 0xffc0, 0x0840, 0xbf8},
	{&MC68000DASM::d68000_bclr_r       , 0xf1c0, 0x0180, 0xbf8},
	{&MC68000DASM::d68000_bclr_s       , 0xffc0, 0x0880, 0xbf8},
	{&MC68000DASM::d68020_bfchg        , 0xffc0, 0xeac0, 0xa78},
	{&MC68000DASM::d68020_bfclr        , 0xffc0, 0xecc0, 0xa78},
	{&MC68000DASM::d68020_bfexts       , 0xffc0, 0xebc0, 0xa7b},
	{&MC68000DASM::d68020_bfextu       , 0xffc0, 0xe9c0, 0xa7b},
	{&MC68000DASM::d68020_bfffo        , 0xffc0, 0xedc0, 0xa7b},
	{&MC68000DASM::d68020_bfins        , 0xffc0, 0xefc0, 0xa78},
	{&MC68000DASM::d68020_bfset        , 0xffc0, 0xeec0, 0xa78},
	{&MC68000DASM::d68020_bftst        , 0xffc0, 0xe8c0, 0xa7b},
	{&MC68000DASM::d68881_ftrap        , 0xfff8, 0xf278, 0x000},
	{&MC68000DASM::d68010_bkpt         , 0xfff8, 0x4848, 0x000},
	{&MC68000DASM::d68000_bra_8        , 0xff00, 0x6000, 0x000},
	{&MC68000DASM::d68000_bra_16       , 0xffff, 0x6000, 0x000},
	{&MC68000DASM::d68020_bra_32       , 0xffff, 0x60ff, 0x000},
	{&MC68000DASM::d68000_bset_r       , 0xf1c0, 0x01c0, 0xbf8},
	{&MC68000DASM::d68000_bset_s       , 0xffc0, 0x08c0, 0xbf8},
	{&MC68000DASM::d68000_bsr_8        , 0xff00, 0x6100, 0x000},
	{&MC68000DASM::d68000_bsr_16       , 0xffff, 0x6100, 0x000},
	{&MC68000DASM::d68020_bsr_32       , 0xffff, 0x61ff, 0x000},
	{&MC68000DASM::d68000_btst_r       , 0xf1c0, 0x0100, 0xbff},
	{&MC68000DASM::d68000_btst_s       , 0xffc0, 0x0800, 0xbfb},
	{&MC68000DASM::d68020_callm        , 0xffc0, 0x06c0, 0x27b},
	{&MC68000DASM::d68020_cas_8        , 0xffc0, 0x0ac0, 0x3f8},
	{&MC68000DASM::d68020_cas_16       , 0xffc0, 0x0cc0, 0x3f8},
	{&MC68000DASM::d68020_cas_32       , 0xffc0, 0x0ec0, 0x3f8},
	{&MC68000DASM::d68020_cas2_16      , 0xffff, 0x0cfc, 0x000},
	{&MC68000DASM::d68020_cas2_32      , 0xffff, 0x0efc, 0x000},
	{&MC68000DASM::d68000_chk_16       , 0xf1c0, 0x4180, 0xbff},
	{&MC68000DASM::d68020_chk_32       , 0xf1c0, 0x4100, 0xbff},
	{&MC68000DASM::d68020_chk2_cmp2_8  , 0xffc0, 0x00c0, 0x27b},
	{&MC68000DASM::d68020_chk2_cmp2_16 , 0xffc0, 0x02c0, 0x27b},
	{&MC68000DASM::d68020_chk2_cmp2_32 , 0xffc0, 0x04c0, 0x27b},
	{&MC68000DASM::d68040_cinv         , 0xff20, 0xf400, 0x000},
	{&MC68000DASM::d68000_clr_8        , 0xffc0, 0x4200, 0xbf8},
	{&MC68000DASM::d68000_clr_16       , 0xffc0, 0x4240, 0xbf8},
	{&MC68000DASM::d68000_clr_32       , 0xffc0, 0x4280, 0xbf8},
	{&MC68000DASM::d68000_cmp_8        , 0xf1c0, 0xb000, 0xbff},
	{&MC68000DASM::d68000_cmp_16       , 0xf1c0, 0xb040, 0xfff},
	{&MC68000DASM::d68000_cmp_32       , 0xf1c0, 0xb080, 0xfff},
	{&MC68000DASM::d68000_cmpa_16      , 0xf1c0, 0xb0c0, 0xfff},
	{&MC68000DASM::d68000_cmpa_32      , 0xf1c0, 0xb1c0, 0xfff},
	{&MC68000DASM::d68000_cmpi_8       , 0xffc0, 0x0c00, 0xbf8},
	{&MC68000DASM::d68020_cmpi_pcdi_8  , 0xffff, 0x0c3a, 0x000},
	{&MC68000DASM::d68020_cmpi_pcix_8  , 0xffff, 0x0c3b, 0x000},
	{&MC68000DASM::d68000_cmpi_16      , 0xffc0, 0x0c40, 0xbf8},
	{&MC68000DASM::d68020_cmpi_pcdi_16 , 0xffff, 0x0c7a, 0x000},
	{&MC68000DASM::d68020_cmpi_pcix_16 , 0xffff, 0x0c7b, 0x000},
	{&MC68000DASM::d68000_cmpi_32      , 0xffc0, 0x0c80, 0xbf8},
	{&MC68000DASM::d68020_cmpi_pcdi_32 , 0xffff, 0x0cba, 0x000},
	{&MC68000DASM::d68020_cmpi_pcix_32 , 0xffff, 0x0cbb, 0x000},
	{&MC68000DASM::d68000_cmpm_8       , 0xf1f8, 0xb108, 0x000},
	{&MC68000DASM::d68000_cmpm_16      , 0xf1f8, 0xb148, 0x000},
	{&MC68000DASM::d68000_cmpm_32      , 0xf1f8, 0xb188, 0x000},
	{&MC68000DASM::d68020_cpbcc_16     , 0xf1c0, 0xf080, 0x000},
	{&MC68000DASM::d68020_cpbcc_32     , 0xf1c0, 0xf0c0, 0x000},
	{&MC68000DASM::d68020_cpdbcc       , 0xf1f8, 0xf048, 0x000},
	{&MC68000DASM::d68020_cpgen        , 0xf1c0, 0xf000, 0x000},
	{&MC68000DASM::d68020_cprestore    , 0xf1c0, 0xf140, 0x37f},
	{&MC68000DASM::d68020_cpsave       , 0xf1c0, 0xf100, 0x2f8},
	{&MC68000DASM::d68020_cpscc        , 0xf1c0, 0xf040, 0xbf8},
	{&MC68000DASM::d68020_cptrapcc_0   , 0xf1ff, 0xf07c, 0x000},
	{&MC68000DASM::d68020_cptrapcc_16  , 0xf1ff, 0xf07a, 0x000},
	{&MC68000DASM::d68020_cptrapcc_32  , 0xf1ff, 0xf07b, 0x000},
	{&MC68000DASM::d68040_cpush        , 0xff20, 0xf420, 0x000},
	{&MC68000DASM::d68000_dbcc         , 0xf0f8, 0x50c8, 0x000},
	{&MC68000DASM::d68000_dbra         , 0xfff8, 0x51c8, 0x000},
	{&MC68000DASM::d68000_divs         , 0xf1c0, 0x81c0, 0xbff},
	{&MC68000DASM::d68000_divu         , 0xf1c0, 0x80c0, 0xbff},
	{&MC68000DASM::d68020_divl         , 0xffc0, 0x4c40, 0xbff},
	{&MC68000DASM::d68000_eor_8        , 0xf1c0, 0xb100, 0xbf8},
	{&MC68000DASM::d68000_eor_16       , 0xf1c0, 0xb140, 0xbf8},
	{&MC68000DASM::d68000_eor_32       , 0xf1c0, 0xb180, 0xbf8},
	{&MC68000DASM::d68000_eori_to_ccr  , 0xffff, 0x0a3c, 0x000},
	{&MC68000DASM::d68000_eori_to_sr   , 0xffff, 0x0a7c, 0x000},
	{&MC68000DASM::d68000_eori_8       , 0xffc0, 0x0a00, 0xbf8},
	{&MC68000DASM::d68000_eori_16      , 0xffc0, 0x0a40, 0xbf8},
	{&MC68000DASM::d68000_eori_32      , 0xffc0, 0x0a80, 0xbf8},
	{&MC68000DASM::d68000_exg_dd       , 0xf1f8, 0xc140, 0x000},
	{&MC68000DASM::d68000_exg_aa       , 0xf1f8, 0xc148, 0x000},
	{&MC68000DASM::d68000_exg_da       , 0xf1f8, 0xc188, 0x000},
	{&MC68000DASM::d68020_extb_32      , 0xfff8, 0x49c0, 0x000},
	{&MC68000DASM::d68000_ext_16       , 0xfff8, 0x4880, 0x000},
	{&MC68000DASM::d68000_ext_32       , 0xfff8, 0x48c0, 0x000},
	{&MC68000DASM::d68040_fpu          , 0xffc0, 0xf200, 0x000},
	{&MC68000DASM::d68000_illegal      , 0xffff, 0x4afc, 0x000},
	{&MC68000DASM::d68000_jmp          , 0xffc0, 0x4ec0, 0x27b},
	{&MC68000DASM::d68000_jsr          , 0xffc0, 0x4e80, 0x27b},
	{&MC68000DASM::d68000_lea          , 0xf1c0, 0x41c0, 0x27b},
	{&MC68000DASM::d68000_link_16      , 0xfff8, 0x4e50, 0x000},
	{&MC68000DASM::d68020_link_32      , 0xfff8, 0x4808, 0x000},
	{&MC68000DASM::d68000_lsr_s_8      , 0xf1f8, 0xe008, 0x000},
	{&MC68000DASM::d68000_lsr_s_16     , 0xf1f8, 0xe048, 0x000},
	{&MC68000DASM::d68000_lsr_s_32     , 0xf1f8, 0xe088, 0x000},
	{&MC68000DASM::d68000_lsr_r_8      , 0xf1f8, 0xe028, 0x000},
	{&MC68000DASM::d68000_lsr_r_16     , 0xf1f8, 0xe068, 0x000},
	{&MC68000DASM::d68000_lsr_r_32     , 0xf1f8, 0xe0a8, 0x000},
	{&MC68000DASM::d68000_lsr_ea       , 0xffc0, 0xe2c0, 0x3f8},
	{&MC68000DASM::d68000_lsl_s_8      , 0xf1f8, 0xe108, 0x000},
	{&MC68000DASM::d68000_lsl_s_16     , 0xf1f8, 0xe148, 0x000},
	{&MC68000DASM::d68000_lsl_s_32     , 0xf1f8, 0xe188, 0x000},
	{&MC68000DASM::d68000_lsl_r_8      , 0xf1f8, 0xe128, 0x000},
	{&MC68000DASM::d68000_lsl_r_16     , 0xf1f8, 0xe168, 0x000},
	{&MC68000DASM::d68000_lsl_r_32     , 0xf1f8, 0xe1a8, 0x000},
	{&MC68000DASM::d68000_lsl_ea       , 0xffc0, 0xe3c0, 0x3f8},
	{&MC68000DASM::d68000_move_8       , 0xf000, 0x1000, 0xbff},
	{&MC68000DASM::d68000_move_16      , 0xf000, 0x3000, 0xfff},
	{&MC68000DASM::d68000_move_32      , 0xf000, 0x2000, 0xfff},
	{&MC68000DASM::d68000_movea_16     , 0xf1c0, 0x3040, 0xfff},
	{&MC68000DASM::d68000_movea_32     , 0xf1c0, 0x2040, 0xfff},
	{&MC68000DASM::d68000_move_to_ccr  , 0xffc0, 0x44c0, 0xbff},
	{&MC68000DASM::d68010_move_fr_ccr  , 0xffc0, 0x42c0, 0xbf8},
	{&MC68000DASM::d68000_move_to_sr   , 0xffc0, 0x46c0, 0xbff},
	{&MC68000DASM::d68000_move_fr_sr   , 0xffc0, 0x40c0, 0xbf8},
	{&MC68000DASM::d68000_move_to_usp  , 0xfff8, 0x4e60, 0x000},
	{&MC68000DASM::d68000_move_fr_usp  , 0xfff8, 0x4e68, 0x000},
	{&MC68000DASM::d68010_movec        , 0xfffe, 0x4e7a, 0x000},
	{&MC68000DASM::d68000_movem_pd_16  , 0xfff8, 0x48a0, 0x000},
	{&MC68000DASM::d68000_movem_pd_32  , 0xfff8, 0x48e0, 0x000},
	{&MC68000DASM::d68000_movem_re_16  , 0xffc0, 0x4880, 0x2f8},
	{&MC68000DASM::d68000_movem_re_32  , 0xffc0, 0x48c0, 0x2f8},
	{&MC68000DASM::d68000_movem_er_16  , 0xffc0, 0x4c80, 0x37b},
	{&MC68000DASM::d68000_movem_er_32  , 0xffc0, 0x4cc0, 0x37b},
	{&MC68000DASM::d68000_movep_er_16  , 0xf1f8, 0x0108, 0x000},
	{&MC68000DASM::d68000_movep_er_32  , 0xf1f8, 0x0148, 0x000},
	{&MC68000DASM::d68000_movep_re_16  , 0xf1f8, 0x0188, 0x000},
	{&MC68000DASM::d68000_movep_re_32  , 0xf1f8, 0x01c8, 0x000},
	{&MC68000DASM::d68010_moves_8      , 0xffc0, 0x0e00, 0x3f8},
	{&MC68000DASM::d68010_moves_16     , 0xffc0, 0x0e40, 0x3f8},
	{&MC68000DASM::d68010_moves_32     , 0xffc0, 0x0e80, 0x3f8},
	{&MC68000DASM::d68000_moveq        , 0xf100, 0x7000, 0x000},
	{&MC68000DASM::d68040_move16_pi_pi , 0xfff8, 0xf620, 0x000},
	{&MC68000DASM::d68040_move16_pi_al , 0xfff8, 0xf600, 0x000},
	{&MC68000DASM::d68040_move16_al_pi , 0xfff8, 0xf608, 0x000},
	{&MC68000DASM::d68040_move16_ai_al , 0xfff8, 0xf610, 0x000},
	{&MC68000DASM::d68040_move16_al_ai , 0xfff8, 0xf618, 0x000},
	{&MC68000DASM::d68000_muls         , 0xf1c0, 0xc1c0, 0xbff},
	{&MC68000DASM::d68000_mulu         , 0xf1c0, 0xc0c0, 0xbff},
	{&MC68000DASM::d68020_mull         , 0xffc0, 0x4c00, 0xbff},
	{&MC68000DASM::d68000_nbcd         , 0xffc0, 0x4800, 0xbf8},
	{&MC68000DASM::d68000_neg_8        , 0xffc0, 0x4400, 0xbf8},
	{&MC68000DASM::d68000_neg_16       , 0xffc0, 0x4440, 0xbf8},
	{&MC68000DASM::d68000_neg_32       , 0xffc0, 0x4480, 0xbf8},
	{&MC68000DASM::d68000_negx_8       , 0xffc0, 0x4000, 0xbf8},
	{&MC68000DASM::d68000_negx_16      , 0xffc0, 0x4040, 0xbf8},
	{&MC68000DASM::d68000_negx_32      , 0xffc0, 0x4080, 0xbf8},
	{&MC68000DASM::d68000_nop          , 0xffff, 0x4e71, 0x000},
	{&MC68000DASM::d68000_not_8        , 0xffc0, 0x4600, 0xbf8},
	{&MC68000DASM::d68000_not_16       , 0xffc0, 0x4640, 0xbf8},
	{&MC68000DASM::d68000_not_32       , 0xffc0, 0x4680, 0xbf8},
	{&MC68000DASM::d68000_or_er_8      , 0xf1c0, 0x8000, 0xbff},
	{&MC68000DASM::d68000_or_er_16     , 0xf1c0, 0x8040, 0xbff},
	{&MC68000DASM::d68000_or_er_32     , 0xf1c0, 0x8080, 0xbff},
	{&MC68000DASM::d68000_or_re_8      , 0xf1c0, 0x8100, 0x3f8},
	{&MC68000DASM::d68000_or_re_16     , 0xf1c0, 0x8140, 0x3f8},
	{&MC68000DASM::d68000_or_re_32     , 0xf1c0, 0x8180, 0x3f8},
	{&MC68000DASM::d68000_ori_to_ccr   , 0xffff, 0x003c, 0x000},
	{&MC68000DASM::d68000_ori_to_sr    , 0xffff, 0x007c, 0x000},
	{&MC68000DASM::d68000_ori_8        , 0xffc0, 0x0000, 0xbf8},
	{&MC68000DASM::d68000_ori_16       , 0xffc0, 0x0040, 0xbf8},
	{&MC68000DASM::d68000_ori_32       , 0xffc0, 0x0080, 0xbf8},
	{&MC68000DASM::d68020_pack_rr      , 0xf1f8, 0x8140, 0x000},
	{&MC68000DASM::d68020_pack_mm      , 0xf1f8, 0x8148, 0x000},
	{&MC68000DASM::d68000_pea          , 0xffc0, 0x4840, 0x27b},
	{&MC68000DASM::d68040_p000         , 0xff80, 0xf500, 0x000},
	{&MC68000DASM::d68000_reset        , 0xffff, 0x4e70, 0x000},
	{&MC68000DASM::d68000_ror_s_8      , 0xf1f8, 0xe018, 0x000},
	{&MC68000DASM::d68000_ror_s_16     , 0xf1f8, 0xe058, 0x000},
	{&MC68000DASM::d68000_ror_s_32     , 0xf1f8, 0xe098, 0x000},
	{&MC68000DASM::d68000_ror_r_8      , 0xf1f8, 0xe038, 0x000},
	{&MC68000DASM::d68000_ror_r_16     , 0xf1f8, 0xe078, 0x000},
	{&MC68000DASM::d68000_ror_r_32     , 0xf1f8, 0xe0b8, 0x000},
	{&MC68000DASM::d68000_ror_ea       , 0xffc0, 0xe6c0, 0x3f8},
	{&MC68000DASM::d68000_rol_s_8      , 0xf1f8, 0xe118, 0x000},
	{&MC68000DASM::d68000_rol_s_16     , 0xf1f8, 0xe158, 0x000},
	{&MC68000DASM::d68000_rol_s_32     , 0xf1f8, 0xe198, 0x000},
	{&MC68000DASM::d68000_rol_r_8      , 0xf1f8, 0xe138, 0x000},
	{&MC68000DASM::d68000_rol_r_16     , 0xf1f8, 0xe178, 0x000},
	{&MC68000DASM::d68000_rol_r_32     , 0xf1f8, 0xe1b8, 0x000},
	{&MC68000DASM::d68000_rol_ea       , 0xffc0, 0xe7c0, 0x3f8},
	{&MC68000DASM::d68000_roxr_s_8     , 0xf1f8, 0xe010, 0x000},
	{&MC68000DASM::d68000_roxr_s_16    , 0xf1f8, 0xe050, 0x000},
	{&MC68000DASM::d68000_roxr_s_32    , 0xf1f8, 0xe090, 0x000},
	{&MC68000DASM::d68000_roxr_r_8     , 0xf1f8, 0xe030, 0x000},
	{&MC68000DASM::d68000_roxr_r_16    , 0xf1f8, 0xe070, 0x000},
	{&MC68000DASM::d68000_roxr_r_32    , 0xf1f8, 0xe0b0, 0x000},
	{&MC68000DASM::d68000_roxr_ea      , 0xffc0, 0xe4c0, 0x3f8},
	{&MC68000DASM::d68000_roxl_s_8     , 0xf1f8, 0xe110, 0x000},
	{&MC68000DASM::d68000_roxl_s_16    , 0xf1f8, 0xe150, 0x000},
	{&MC68000DASM::d68000_roxl_s_32    , 0xf1f8, 0xe190, 0x000},
	{&MC68000DASM::d68000_roxl_r_8     , 0xf1f8, 0xe130, 0x000},
	{&MC68000DASM::d68000_roxl_r_16    , 0xf1f8, 0xe170, 0x000},
	{&MC68000DASM::d68000_roxl_r_32    , 0xf1f8, 0xe1b0, 0x000},
	{&MC68000DASM::d68000_roxl_ea      , 0xffc0, 0xe5c0, 0x3f8},
	{&MC68000DASM::d68010_rtd          , 0xffff, 0x4e74, 0x000},
	{&MC68000DASM::d68000_rte          , 0xffff, 0x4e73, 0x000},
	{&MC68000DASM::d68020_rtm          , 0xfff0, 0x06c0, 0x000},
	{&MC68000DASM::d68000_rtr          , 0xffff, 0x4e77, 0x000},
	{&MC68000DASM::d68000_rts          , 0xffff, 0x4e75, 0x000},
	{&MC68000DASM::d68000_sbcd_rr      , 0xf1f8, 0x8100, 0x000},
	{&MC68000DASM::d68000_sbcd_mm      , 0xf1f8, 0x8108, 0x000},
	{&MC68000DASM::d68000_scc          , 0xf0c0, 0x50c0, 0xbf8},
	{&MC68000DASM::d68000_stop         , 0xffff, 0x4e72, 0x000},
	{&MC68000DASM::d68000_sub_er_8     , 0xf1c0, 0x9000, 0xbff},
	{&MC68000DASM::d68000_sub_er_16    , 0xf1c0, 0x9040, 0xfff},
	{&MC68000DASM::d68000_sub_er_32    , 0xf1c0, 0x9080, 0xfff},
	{&MC68000DASM::d68000_sub_re_8     , 0xf1c0, 0x9100, 0x3f8},
	{&MC68000DASM::d68000_sub_re_16    , 0xf1c0, 0x9140, 0x3f8},
	{&MC68000DASM::d68000_sub_re_32    , 0xf1c0, 0x9180, 0x3f8},
	{&MC68000DASM::d68000_suba_16      , 0xf1c0, 0x90c0, 0xfff},
	{&MC68000DASM::d68000_suba_32      , 0xf1c0, 0x91c0, 0xfff},
	{&MC68000DASM::d68000_subi_8       , 0xffc0, 0x0400, 0xbf8},
	{&MC68000DASM::d68000_subi_16      , 0xffc0, 0x0440, 0xbf8},
	{&MC68000DASM::d68000_subi_32      , 0xffc0, 0x0480, 0xbf8},
	{&MC68000DASM::d68000_subq_8       , 0xf1c0, 0x5100, 0xbf8},
	{&MC68000DASM::d68000_subq_16      , 0xf1c0, 0x5140, 0xff8},
	{&MC68000DASM::d68000_subq_32      , 0xf1c0, 0x5180, 0xff8},
	{&MC68000DASM::d68000_subx_rr_8    , 0xf1f8, 0x9100, 0x000},
	{&MC68000DASM::d68000_subx_rr_16   , 0xf1f8, 0x9140, 0x000},
	{&MC68000DASM::d68000_subx_rr_32   , 0xf1f8, 0x9180, 0x000},
	{&MC68000DASM::d68000_subx_mm_8    , 0xf1f8, 0x9108, 0x000},
	{&MC68000DASM::d68000_subx_mm_16   , 0xf1f8, 0x9148, 0x000},
	{&MC68000DASM::d68000_subx_mm_32   , 0xf1f8, 0x9188, 0x000},
	{&MC68000DASM::d68000_swap         , 0xfff8, 0x4840, 0x000},
	{&MC68000DASM::d68000_tas          , 0xffc0, 0x4ac0, 0xbf8},
	{&MC68000DASM::d68000_trap         , 0xfff0, 0x4e40, 0x000},
	{&MC68000DASM::d68020_trapcc_0     , 0xf0ff, 0x50fc, 0x000},
	{&MC68000DASM::d68020_trapcc_16    , 0xf0ff, 0x50fa, 0x000},
	{&MC68000DASM::d68020_trapcc_32    , 0xf0ff, 0x50fb, 0x000},
	{&MC68000DASM::d68000_trapv        , 0xffff, 0x4e76, 0x000},
	{&MC68000DASM::d68000_tst_8        , 0xffc0, 0x4a00, 0xbf8},
	{&MC68000DASM::d68020_tst_pcdi_8   , 0xffff, 0x4a3a, 0x000},
	{&MC68000DASM::d68020_tst_pcix_8   , 0xffff, 0x4a3b, 0x000},
	{&MC68000DASM::d68020_tst_i_8      , 0xffff, 0x4a3c, 0x000},
	{&MC68000DASM::d68000_tst_16       , 0xffc0, 0x4a40, 0xbf8},
	{&MC68000DASM::d68020_tst_a_16     , 0xfff8, 0x4a48, 0x000},
	{&MC68000DASM::d68020_tst_pcdi_16  , 0xffff, 0x4a7a, 0x000},
	{&MC68000DASM::d68020_tst_pcix_16  , 0xffff, 0x4a7b, 0x000},
	{&MC68000DASM::d68020_tst_i_16     , 0xffff, 0x4a7c, 0x000},
	{&MC68000DASM::d68000_tst_32       , 0xffc0, 0x4a80, 0xbf8},
	{&MC68000DASM::d68020_tst_a_32     , 0xfff8, 0x4a88, 0x000},
	{&MC68000DASM::d68020_tst_pcdi_32  , 0xffff, 0x4aba, 0x000},
	{&MC68000DASM::d68020_tst_pcix_32  , 0xffff, 0x4abb, 0x000},
	{&MC68000DASM::d68020_tst_i_32     , 0xffff, 0x4abc, 0x000},
	{&MC68000DASM::d68000_unlk         , 0xfff8, 0x4e58, 0x000},
	{&MC68000DASM::d68020_unpk_rr      , 0xf1f8, 0x8180, 0x000},
	{&MC68000DASM::d68020_unpk_mm      , 0xf1f8, 0x8188, 0x000},
	{&MC68000DASM::d68851_p000         , 0xffc0, 0xf000, 0x000},
	{&MC68000DASM::d68851_pbcc16       , 0xffc0, 0xf080, 0x000},
	{&MC68000DASM::d68851_pbcc32       , 0xffc0, 0xf0c0, 0x000},
	{&MC68000DASM::d68851_pdbcc        , 0xfff8, 0xf048, 0x000},
	{&MC68000DASM::d68851_p001         , 0xffc0, 0xf040, 0x000},
	{&MC68000DASM::d68040_fbcc_16      , 0xffc0, 0xf280, 0x000},
	{&MC68000DASM::d68040_fbcc_32      , 0xffc0, 0xf2c0, 0x000},
	{0, 0, 0, 0}
};

#endif /* USE_DEBUGGER */
