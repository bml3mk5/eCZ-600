/** @file mc68000dasm.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.01.01 -

	@brief [ mc68000 disassembler ]
*/

#ifndef MC68000DASM_H
#define MC68000DASM_H

#ifndef USE_DEBUGGER
#define DASM_SET_CPU_TYPE(c)

//#define DASM_SET_MEM1(ad, fc, da)
//#define DASM_SET_MEM2(ad, fc, da, w)
#define DASM_ADD_MEM1(ad, fc, da)
#define DASM_ADD_MEM2(ad, fc, da, w)

#define DASM_PUSH_ADDR(ad)
#define DASM_ADD_CODE(code)
#define DASM_PUSH_VECTOR(ca, vn, ad)
#define DASM_PUSH_INTVEC(ad, sr)
//#define DASM_PUSH_RESVEC(pc)
#define DASM_PUSH_AERR(ad)
#define DASM_PUSH_AERR2(ad, fc, da, w)
#define DASM_SET_AERR()
#define DASM_PUSH_BERR(ad, cd)
#define DASM_SET_SIGNALS(sig)
#define DASM_ADD_SIGNALS(sig)
#define DASM_PUSH_RESET(sr)
#define DASM_PUSH_BUSREQ(ad, sr)
#define DASM_PUSH_HALT(sr)
#define DASM_PUSH_PREF(pc, code, cy, sr)
#define DASM_SET_REG(ac, cy, sr)
#define DASM_UPDATE_REG(sr)
#define DASM_SET_CYCLES(ac, cy)
#define DASM_PRINT_DASM(p, pc, ops, l, f)


#else /* USE_DEBUGGER*/
#define DASM_SET_CPU_TYPE(c)			dasm.set_cpu_type(c)

//#define DASM_SET_MEM1(ad, fc, da)		dasm.set_mem(ad, fc, da)
//#define DASM_SET_MEM2(ad, fc, da, w)	dasm.set_mem(ad, fc, da, w)
#define DASM_ADD_MEM1(ad, fc, da)		dasm.add_mem(ad, fc, da)
#define DASM_ADD_MEM2(ad, fc, da, w)	dasm.add_mem(ad, fc, da, w)

#define DASM_PUSH_ADDR(ad)				dasm.push_addr(ad)
#define DASM_ADD_CODE(code)				dasm.add_code(code)
#define DASM_PUSH_VECTOR(ca, vn, ad)	dasm.push_vector(ca, vn, ad)
#define DASM_PUSH_INTVEC(ad, sr)		dasm.push_intvec(ad, sr)
//#define DASM_PUSH_RESVEC(ad)			dasm.push_resvec(ad)
#define DASM_PUSH_AERR(ad)				dasm.push_aerr(ad)
#define DASM_PUSH_AERR2(ad, fc, da, w)	dasm.push_aerr(ad, fc, da, w)
#define DASM_SET_AERR()					dasm.set_aerr()
#define DASM_PUSH_BERR(ad, cd)			dasm.push_berr(ad, cd)
#define DASM_SET_SIGNALS(sig)			dasm.set_signals(sig)
#define DASM_ADD_SIGNALS(sig)			dasm.add_signals(sig)
#define DASM_PUSH_RESET(sr)				dasm.push_reset(sr)
#define DASM_PUSH_BUSREQ(ad, sr)		dasm.push_busreq(ad, sr)
#define DASM_PUSH_HALT(sr)				dasm.push_halt(sr)
#define DASM_PUSH_PREF(ad, code, cy, sr)	dasm.push_pref(ad, code, cy, sr)
#define DASM_SET_REG(ac, cy, sr)		dasm.set_reg(ac, cy, sr)
#define DASM_UPDATE_REG(sr)				dasm.update_reg(sr)
#define DASM_SET_CYCLES(ac, cy)			dasm.set_cycles(ac, cy)
#define DASM_PRINT_DASM(p, pc, ops, l, f)	dasm.print_dasm(p, pc, ops, l, f)

#ifndef BIT
#define BIT(x, n) ((x >> n) & 1)
#endif

#include "../emu.h"
#include "../common.h"
#include "vm_defs.h"
#include "mc68000_consts.h"
#include <vector>

class DEBUGGER;
class DEVICE;

#define MC68000DASM_PCSTACK_COUNT 20

/// signals on MC68000
enum mc68000dasm_signal_index {
	MC68000_IDX_RESET = 0,
	MC68000_IDX_IPL,
	MC68000_IDX_BERR,
	MC68000_IDX_HALT,
	MC68000_IDX_END
};
enum mc68000dasm_categories {
	CATEGORY_INVALID = 0,
	CATEGORY_NORMAL,
	CATEGORY_EXCEPTION,
	CATEGORY_INTR_VECNUM,
	CATEGORY_AERROR,
	CATEGORY_BERROR,
	CATEGORY_RESET,
	CATEGORY_BUSREQ,
	CATEGORY_HALT,
	CATEGORY_PREF,
};
enum mc68000dasm_flags {
	FLAG_VALID  = 0x0001,
	FLAG_STORE  = 0x0010,
	FLAG_WRITE  = 0x0020,
	FLAG_1BYTE  = 0x0000,
	FLAG_2BYTES = 0x0040,
	FLAG_4BYTES = 0x0080,
	FLAG_MEMALL = 0x00f0,
};

//////////////////////////////////////////////////////////////////////

/// record a data accessed the bus of MC68000
class MC68000DASM_DATA
{
public:
	uint32_t m_phyaddr;
	uint32_t m_addr;
	uint32_t m_data;
	uint16_t m_fc;
	uint16_t m_flags;		// bit1:data store=1  bit2:write data=1  bit3:2bytes data=1 bit4:4bytes data=1

	MC68000DASM_DATA();
	MC68000DASM_DATA(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_);
	~MC68000DASM_DATA() {};

	void clear();
	void set(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_);
};

//////////////////////////////////////////////////////////////////////

/// record operation accessing the bus of MC68000
class MC68000DASM_DATAS
{
private:
	MC68000DASM_DATA m_datas[2];
	int m_len;
public:
	MC68000DASM_DATAS();
	~MC68000DASM_DATAS() {}

	void clear();
	int size() const { return m_len; }
	const MC68000DASM_DATA &at(int pos) const { return m_datas[pos]; }
	void set_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_);
	void add_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_);
};

//////////////////////////////////////////////////////////////////////

/// pointer of registers on MC68000
class MC68000DASM_REGPTR
{
public:
	uint32_t *p_pc;           /* Program Counter */
	uint32_t *p_dar;          /* Data and Address Registers */
	uint32_t *p_sp;           /* User, Interrupt, and Master Stack Pointers */
#ifdef USE_MC68000VBR
	uint32_t *p_vbr;          /* Vector Base Register (m68010+) */
	uint32_t *p_sfc;          /* Source Function Code Register (m68010+) */
	uint32_t *p_dfc;          /* Destination Function Code Register (m68010+) */
#endif
#ifdef USE_MC68000CACHE
	uint32_t *p_cacr;         /* Cache Control Register (m68020, unemulated) */
	uint32_t *p_caar;         /* Cache Address Register (m68020, unemulated) */
#endif
#ifdef USE_MC68000FPU
	uint32_t *p_fpiar;        /* FPU Instruction Address Register (m68040) */
	uint32_t *p_fpsr;         /* FPU Status Register (m68040) */
	uint32_t *p_fpcr;         /* FPU Control Register (m68040) */
#endif
	uint32_t *p_signals;      /* Asserted signals */

public:
	MC68000DASM_REGPTR();
	void set_regptrs(
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
	);
	virtual ~MC68000DASM_REGPTR() {}
};

//////////////////////////////////////////////////////////////////////

/// record the processed code, register and status of MC68000
class MC68000DASM_REG
{
public:
	enum mc68000dasm_reg_defines {
		CODELEN_MAX = 7,
		CODELEN_SHOW = 5,
	};

public:
	uint32_t phyaddr;			// physical address of op code
	uint32_t addr;				// start address of op code
	uint16_t ops[CODELEN_MAX];  // op code and operands
	uint16_t opslen;			// length of op

	uint32_t pc;				// program counter
	uint32_t dar[16];			// data and address register
	uint32_t sp[4];				// stack pointer on super visor mode
	uint16_t sr;				// status register

	uint16_t cycles;

#ifdef USE_MC68000VBR
	uint32_t vbr;				// Vector Base Register (m68010+)
	uint16_t sfc;				// Source Function Code Register (m68010+)
	uint16_t dfc;				// Destination Function Code Register (m68010+)
#endif

#ifdef USE_MC68000CACHE
	uint32_t cacr;		// Cache Control Register
	uint32_t caar;		// Cache Address Register
#endif

	uint32_t isignals;			// signals

	uint16_t category;			// category
	uint16_t vec_num;			// vector number

	MC68000DASM_DATAS rw;		// read/write data

public:
	MC68000DASM_REG();
	MC68000DASM_REG(uint32_t phyaddr_, uint32_t addr_, uint32_t pc_, const uint32_t *dar_, const uint32_t *sp_, uint16_t sr_, uint32_t signals_);
	~MC68000DASM_REG() {}
	void clear();
	void set(uint32_t phyaddr_, uint32_t addr_);
	void set(uint32_t phyaddr_, uint32_t addr_, uint32_t pc_, const uint32_t *dar_, const uint32_t *sp_, uint16_t sr_, uint32_t signals_);
//	void set_regs(int accum_, int cycles_, uint32_t pc_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_regs(int accum_, int cycles_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_regs(uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_cycles(int accum_, int cycles_);
	bool add_code(uint32_t code_);
	void set_reset(uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_busreq(uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_halt(uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_pref(uint32_t code_, int cycles_, uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
	void set_aerr();
	void set_berr(uint32_t code_);
	void set_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_ = 0);
	void add_data(uint32_t phyaddr_, uint32_t addr_, uint16_t fc_, uint32_t data_, bool write_, uint16_t addflags_ = 0);
	void set_flag_vector(uint16_t category_, uint16_t vec_num_);
	void set_flag_intvec(uint16_t sr_, const MC68000DASM_REGPTR &ptr_);
//	void set_flag_resvec();

	uint32_t get_phyaddr() const { return phyaddr; }
	uint32_t get_addr() const { return addr; }
//	uint32_t get_pc() const { return pc; }
	const uint16_t *get_ops() const { return ops; }
	uint16_t get_opslen() const { return opslen; }
	uint16_t get_cycles() const { return cycles; }

	const MC68000DASM_DATAS &get_rwdata() const { return rw; }

	void set_signals(uint32_t val) { isignals = val; }
	void add_signals(uint32_t val) { isignals |= val; /* add signals */ }
	uint32_t get_signals() const { return isignals; }

	uint16_t get_category() const { return category; }
	uint16_t get_vec_num() const { return vec_num; }

	bool is_valid() const { return (category != CATEGORY_INVALID); }
};

//////////////////////////////////////////////////////////////////////

/// list of the processed code, register and status of MC68000
class MC68000DASM_REGS : public MC68000DASM_REGPTR
{
public:
	enum mc68000dasm_regs_defines {
		MC68000DASM_PCSTACK_NUM = 1000,
	};

protected:
	MC68000DASM_REG regs[MC68000DASM_PCSTACK_NUM];
	MC68000DASM_REG *current_reg;
	int current_idx;

	void push_stack(uint32_t phyaddr, uint32_t addr);

public:
	MC68000DASM_REGS();
	~MC68000DASM_REGS() {}

	int  get_stack(int index, MC68000DASM_REG &stack);
};

//////////////////////////////////////////////////////////////////////

/**
	@brief MC68000 disassembler
*/
class MC68000DASM : public MC68000DASM_REGS
{
public:
	typedef struct opcode_st {
		uint8_t       c;
		const _TCHAR *s;
	} opcode_t;

	enum mc68000dasm_defines {
		LINE_LEN = 512,
		CMDLINE_LEN	= 128,
	};

private:
	DEVICE *d_cpu;
	DEBUGGER *debugger;
	int m_cpu_type;

	_TCHAR line[LINE_LEN];

	_TCHAR cmd[CMDLINE_LEN];

	static const _TCHAR *m_except[17];

	static const _TCHAR *m_bcc[16];
	static const _TCHAR *m_cpcc[64];
	static const _TCHAR *btst_str[4];
	static const _TCHAR *m_mmuregs[8];
	static const _TCHAR *m_mmucond[16];

	/* used by ops like asr, ror, addq, etc */
	static const uint32_t m_3bit_qdata_table[8];
	static const uint32_t m_5bit_data_table[32];

	static const _TCHAR m_tr_table[];
	static const _TCHAR m_sr_table[];
	static const _TCHAR m_su_table[];
	static const _TCHAR m_si_table[];

	_TCHAR m_addr_format[8];
	_TCHAR m_addr_format_s[8];
	uint32_t m_addr_mask;

	enum {
		WKWORD_LEN = 48
	};

	enum en_immsize {
		SIZE_8BITS = 0,
		SIZE_16BITS = 1,
		SIZE_32BITS = 2,
	};

	struct st_bag {
		uint32_t pc;
	} m_bag;

	bool m_lower_case;	///< replace to lower case instruction

	int  make_cmd_str(uint32_t addr, const uint16_t *ops, int opslen, _TCHAR *str, int len);

	void make_comment_str(const _TCHAR *comment, _TCHAR *str, int len);

	void make_reg_direct_str(_TCHAR reg, uint8_t data, _TCHAR *str, int len);
	void make_data_reg_direct_str(uint8_t data, _TCHAR *str, int len);
	void make_address_reg_direct_str(uint8_t data, _TCHAR *str, int len);
	void make_address_reg_indirect_str(uint8_t data, _TCHAR *str, int len);
	void make_address_reg_indirect_postinc_str(uint8_t data, _TCHAR *str, int len);
	void make_address_reg_indirect_predec_str(uint8_t data, _TCHAR *str, int len);

	void make_reg_reg_direct_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len);
	void make_reg_reg_indirect_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len);
	void make_reg_reg_indirect_postinc_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len);
	void make_reg_reg_indirect_predec_str(_TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, _TCHAR *str, int len);

	int  make_address_reg_indirect_displace_str(uint8_t data, const uint16_t *ops, int opslen, _TCHAR *str, int len);

	int  make_absolute_address_str(uint32_t pc, uint8_t reg, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int  make_immediage_str_u(en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10 = false);
	inline int make_immediage_str_u8(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_u(SIZE_8BITS, ops, opslen, str, len); }
	inline int make_immediage_str_u16(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_u(SIZE_16BITS, ops, opslen, str, len); }
	inline int make_immediage_str_u32(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_u(SIZE_32BITS, ops, opslen, str, len); }
	int  make_immediage_str_s(en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10 = false);
	inline int make_immediage_str_s8(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_s(SIZE_8BITS, ops, opslen, str, len); }
	inline int make_immediage_str_s16(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_s(SIZE_16BITS, ops, opslen, str, len); }
	inline int make_immediage_str_s32(const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_immediage_str_s(SIZE_32BITS, ops, opslen, str, len); }


	void make_signed_str_8(uint8_t val, _TCHAR *str, int len, bool base10 = false);
	void make_signed_str_16(uint16_t val, _TCHAR *str, int len, bool base10 = false);
	void make_signed_str_32(uint32_t val, _TCHAR *str, int len, bool base10 = false);

	int make_ea_index_str(uint8_t reg, const uint16_t *ops, int opslen, _TCHAR *str, int len);
//	int make_pc_index_str(uint32_t pc, const uint16_t *ops, int opslen, _TCHAR *str, int len);

	void make_l_w_str(bool long_data, _TCHAR *str, int len);

	int make_ea_mode_str(uint8_t mode, uint8_t reg, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	inline int make_ea_mode_str_8 (uint8_t mode, uint8_t reg, const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_ea_mode_str(mode, reg, SIZE_8BITS, ops, opslen, str, len); }
	inline int make_ea_mode_str_16(uint8_t mode, uint8_t reg, const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_ea_mode_str(mode, reg, SIZE_16BITS, ops, opslen, str, len); }
	inline int make_ea_mode_str_32(uint8_t mode, uint8_t reg, const uint16_t *ops, int opslen, _TCHAR *str, int len) { return make_ea_mode_str(mode, reg, SIZE_32BITS, ops, opslen, str, len); }

	enum en_regname {
		REG_CCR = 0,
		REG_SR = 1,
		REG_USP = 2,
		REG_UNKNOWN = 3,
	};
	void join_words(_TCHAR *str, int len, const _TCHAR words[][WKWORD_LEN]);
	void strlower(_TCHAR *str);
	void make_name_str(const _TCHAR *name, bool sup, _TCHAR *str, int len, const _TCHAR *pre = NULL, const _TCHAR *post = NULL);
	void make_cpname_str(uint16_t cpnum, const _TCHAR *name, bool sup, _TCHAR *str, int len, const _TCHAR *pre = NULL, const _TCHAR *post = NULL);
	int make_r_base_str(const _TCHAR *name, bool sup, const _TCHAR reg, uint32_t d, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_r_r_base_str(const _TCHAR *name, bool sup, _TCHAR reg1, uint32_t d1, _TCHAR reg2, uint32_t d2, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_dr_d_s_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_dr_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ar_ar_d_s_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ar_ar_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_ar_s_d_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_arpl_arpl_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_miar_miar_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ar_ccrsr_str(const _TCHAR *name, bool sup, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ccrsr_ar_str(const _TCHAR *name, bool sup, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, const _TCHAR *pre = NULL);
	int make_ea_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, const _TCHAR *pre = NULL);
	int make_ea_dr_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dr_dr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ea_ar_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	void make_ccrsr_str(en_regname regn, _TCHAR *str, int len);
	int make_ea_ccrsr_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ccrsr_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_uimm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10 = false);
	int make_simm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len, bool base10 = false);
	int make_simm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_uimm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_qimm_ea_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_qimm_dr_str(const _TCHAR *name, bool sup, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_uimm_ccrsr_str(const _TCHAR *name, bool sup, en_immsize immsize, en_regname regn, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_ar_simm_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);

	int make_bcc_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int make_dbcc_str(const _TCHAR *name, bool sup, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);


	static inline uint32_t read_op32(const uint16_t *ops, int &opscnt);

	static void set_ext_addr(uint16_t addr);

	/* Extension word formats */
	static inline uint32_t ext_8bit_displacement(uint32_t A)          { return ((A)&0xff); }
	static inline uint32_t ext_full(uint32_t A)                       { return BIT(A, 8); }
	static inline uint32_t ext_effective_zero(uint32_t A)             { return (((A)&0xe4) == 0xc4 || ((A)&0xe2) == 0xc0); }
	static inline uint32_t ext_base_register_present(uint32_t A)      { return (!BIT(A, 7)); }
	static inline uint32_t ext_index_register_present(uint32_t A)     { return (!BIT(A, 6)); }
	static inline uint32_t ext_index_register(uint32_t A)             { return (((A)>>12)&7); }
	static inline uint32_t ext_index_scale(uint32_t A)                { return (((A)>>9)&3); }
	static inline uint32_t ext_index_long(uint32_t A)                 { return BIT(A, 11); }
	static inline uint32_t ext_index_ar(uint32_t A)                   { return BIT(A, 15); }
	static inline uint32_t ext_base_displacement_present(uint32_t A)  { return (((A)&0x30) > 0x10); }
	static inline uint32_t ext_base_displacement_word(uint32_t A)     { return (((A)&0x30) == 0x20); }
	static inline uint32_t ext_base_displacement_long(uint32_t A)     { return (((A)&0x30) == 0x30); }
	static inline uint32_t ext_outer_displacement_present(uint32_t A) { return (((A)&3) > 1 && ((A)&0x47) < 0x44); }
	static inline uint32_t ext_outer_displacement_word(uint32_t A)    { return (((A)&3) == 2 && ((A)&0x47) < 0x44); }
	static inline uint32_t ext_outer_displacement_long(uint32_t A)    { return (((A)&3) == 3 && ((A)&0x47) < 0x44); }

	static inline int32_t sext_7bit_int(uint32_t value) { return (value & 0x40) ? (value | 0xffffff80) : (value & 0x7f); }
	static inline uint32_t make_int_8(uint8_t value) { return (uint32_t)(int8_t)(value); }
	static inline uint32_t make_int_16(uint16_t value) { return (uint32_t)(int16_t)(value); }
	static inline uint32_t make_int_32(uint32_t value) { return (uint32_t)(int32_t)(value); }

	typedef int (MC68000DASM::*opcode_handler)(const uint16_t *ops, int opslen, _TCHAR *str, int len);

	struct opcode_struct {
		opcode_handler handler; /* handler function */
		uint32_t mask;          /* mask on opcode */
		uint32_t match;         /* what to match after masking */
		uint32_t ea_mask;       /* what ea modes are allowed */
	};

	static bool compare_mask_bits(uint16_t a, uint16_t b);
//	static bool compare_nof_true_bits(struct opcode_index * aidx, struct opcode_index * bidx);

//	void build_opcode_table_sub();
	void build_opcode_table();
//	static bool valid_ea(uint32_t opcode, uint32_t mask);

	uint16_t instruction_table[0x10000];
	static const opcode_struct m_opcode_info[];

	int d68000_illegal(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_1010(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_1111(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_abcd_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_abcd_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_add_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_adda_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_adda_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addq_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addq_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addq_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_rr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_rr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_rr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_mm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_mm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_addx_mm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_and_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_andi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_andi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_andi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_andi_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_andi_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_asl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bcc_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
//	int d68000_bchg_clr_set_tst_r_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
//	int d68000_bchg_clr_set_tst_s_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bchg_r(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bchg_s(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bclr_r(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bclr_s(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_bkpt(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	void d68020_bfxxx_offset_width(uint16_t extension, _TCHAR *str, int len);
	int d68020_bfxxx_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfchg(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfclr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfxxx_di_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfexts(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfextu(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfffo(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfins(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bfset(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bftst(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bra_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bra_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bra_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bset_r(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bset_s(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bsr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_bsr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_bsr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_btst_r(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_btst_s(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_callm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas2_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas2_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cas2_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_chk_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_chk_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_chk2_cmp2_temp(const _TCHAR *post, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_chk2_cmp2_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_chk2_cmp2_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_chk2_cmp2_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_cache_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_cinv(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_clr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_clr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_clr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmp_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmp_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmp_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpa_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpa_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcdi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcix_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcdi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcix_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcdi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cmpi_pcix_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_cmpm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpbcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpbcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpdbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpgen(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cprestore(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpsave(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cpscc(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cptrapcc_0(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cptrapcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_cptrapcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_cpush(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_dbra(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_dbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_divs(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_divu(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_divl(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eor_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eor_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eor_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eori_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eori_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eori_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eori_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_eori_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_exg_dd(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_exg_aa(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_exg_da(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ext_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ext_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_extb_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68881_ftrap(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_fpu(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_jmp(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_jsr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_link_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_link_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_lsl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movea_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movea_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_move_fr_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_fr_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_fr_usp(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_move_to_usp(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_movec(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	bool d68000_movem_regs_temp(_TCHAR reg, uint8_t data, bool msb, bool centre, _TCHAR *str, int len);
	int d68000_movem_pd_regs_temp(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_pd_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_pd_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_pd_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_erre_regs_temp(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_er_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_re_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movem_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_re_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_er_temp(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_movep_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_moves_temp(const _TCHAR *name, en_immsize immsize, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_moves_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_moves_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_moves_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_moveq(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_move16_pi_pi(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_move16_pi_al(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_move16_al_pi(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_move16_ai_al(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_move16_al_ai(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_muls(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_mulu(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_mull(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_nbcd(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_neg_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_neg_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_neg_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_negx_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_negx_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_negx_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_nop(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_not_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_not_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_not_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_or_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ori_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ori_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ori_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ori_to_ccr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ori_to_sr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_pack_unpk_rr(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_pack_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_pack_unpk_mm(const _TCHAR *name, const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_pack_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_pea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_reset(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_ror_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rol_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxr_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_s_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_s_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_s_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_r_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_r_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_r_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_roxl_ea(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68010_rtd(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rte(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_rtm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rtr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_rts(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sbcd_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sbcd_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_scc(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_stop(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_er_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_er_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_er_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_re_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_re_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_sub_re_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_suba_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_suba_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subq_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subq_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subq_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_rr_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_rr_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_rr_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_mm_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_mm_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_subx_mm_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_swap(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_tas(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_trap(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_trapcc_0(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_trapcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_trapcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_trapv(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_tst_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcdi_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcix_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_i_8(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_tst_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_a_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcdi_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcix_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_i_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_tst_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_a_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcdi_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_pcix_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_tst_i_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68000_unlk(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_unpk_rr(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68020_unpk_mm(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	void fc_to_string(uint16_t modes, _TCHAR *str, int len);
	int d68040_p000(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68851_p000(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68851_pbcc16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68851_pbcc32(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68851_pdbcc(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68851_p001(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_fbcc_16(const uint16_t *ops, int opslen, _TCHAR *str, int len);
	int d68040_fbcc_32(const uint16_t *ops, int opslen, _TCHAR *str, int len);

public:
	MC68000DASM();
	~MC68000DASM();

	void push_addr(uint32_t addr);
//	void set_pc(uint32_t pc);
	void add_code(uint16_t code);
	void push_vector(uint16_t category, uint16_t vec_num, uint32_t addr);
	void push_intvec(uint32_t addr, uint16_t sr);
//	void push_resvec(uint32_t addr);
	void push_reset(uint16_t sr);
	void push_busreq(uint32_t addr, uint16_t sr);
	void push_halt(uint16_t sr);
	void push_pref(uint32_t addr, uint32_t code, int cycles, uint16_t sr);
	void push_aerr(uint32_t addr);
	void push_aerr(uint32_t addr, uint16_t fc, uint8_t data, bool write);
	void set_aerr();
	void push_berr(uint32_t addr, uint32_t code);
	void set_signals(uint32_t sig);
	void add_signals(uint32_t sig);

	void set_mem(uint32_t addr, uint16_t fc, uint8_t data, bool write = false);
	void set_mem(uint32_t addr, uint16_t fc, uint16_t data, bool write = false);
	void set_mem(uint32_t addr, uint16_t fc, uint32_t data, bool write = false);
	void add_mem(uint32_t addr, uint16_t fc, uint8_t data, bool write = false);
	void add_mem(uint32_t addr, uint16_t fc, uint16_t data, bool write = false);
	void add_mem(uint32_t addr, uint16_t fc, uint32_t data, bool write = false);
//	void set_phymem(uint32_t phyaddr);

	void set_reg(int accum, int cycles, uint16_t sr);
	void update_reg(uint16_t sr);

	void set_cycles(int accum, int cycles);

	int print_dasm(const MC68000DASM_REG &reg);
	int print_dasm(uint32_t phyaddr, uint32_t addr, const uint16_t *ops, int opslen, uint16_t category, uint16_t vec_num);
	int print_dasm_label(int type, uint32_t addr);
	int print_dasm_preprocess(int type, uint32_t addr, int flags);
	int print_dasm_processed();
	int print_dasm_traceback(int index);

	void print_cycles(int cycles);
	void print_regs(const MC68000DASM_REG &reg);
	void print_regs_current();
//	void print_regs_current(uint32_t pc, const uint32_t *dar, const uint32_t *sp, uint16_t sr, uint32_t signals);
	int print_regs_traceback(int index);

	void print_memory_datas(const MC68000DASM_DATAS &rws);

	const _TCHAR *get_line() const {
		return line;
	}
	size_t get_line_length() const {
		return _tcslen(line);
	}

	void set_context_cpu(DEVICE* device);
#if 0
	void set_context_progmem(DEVICE* device) {
		d_progmem = device;
	}
	void set_context_cpu_space(DEVICE* device) {
		d_cpuspace = device;
	}
#endif
	void set_context_debugger(DEBUGGER *device) {
		debugger = device;
	}
	void set_cpu_type(int cpu_type) {
		m_cpu_type = cpu_type;
	}
};

#endif /* USE_DEBUGGER */

#endif /* MC68000DASM_H */
