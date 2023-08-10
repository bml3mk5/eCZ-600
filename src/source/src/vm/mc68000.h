/** @file mc68000.h

	Skelton for retropc emulator

	@par Origin MAME 0.293, 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@note
		When you use this device, must include "mc68000_consts.h" and define one of following directives:
		USE_MC68000
		USE_MC68008
		USE_MC68010
		USE_MC68020
		USE_MC68020MMU
		USE_MC68EC030
		USE_MC68030

	@brief [ MC68000 ]
*/

#ifndef MC68000_H
#define MC68000_H

#include "vm_defs.h"
#include "device.h"
#include "mc68000_consts.h"
#include "mc68000dasm.h"
#ifdef USE_MC68000FPU
#include "softfloat/softfloat.h"
#endif
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define ILLEGAL_PC_MAX 14

#ifdef INLINE
#undef INLINE
#endif
#define INLINE

class EMU;
class MC68000BASE;
class MC68000ABERR;

/* ======================================================================== */

#define NUM_CPU_TYPES 8

#ifndef U64
#define U64(x) x##ULL
#endif

#ifndef WORD_ALIGNED
#define WORD_ALIGNED(a)                 (((a) & 1) == 0)
#endif
#ifndef DWORD_ALIGNED
#define DWORD_ALIGNED(a)                (((a) & 3) == 0)
#endif
#ifndef QWORD_ALIGNED
#define QWORD_ALIGNED(a)                (((a) & 7) == 0)
#endif

/* Check for > 32bit sizes */
#define MAKE_INT_8(A) (int8_t)(A)
#define MAKE_INT_16(A) (int16_t)(A)
#define MAKE_INT_32(A) (int32_t)(A)

/* ---------------------------- General Macros ---------------------------- */

/* Bit Isolation Macros */
#define BIT_0(A)  ((A) & 0x00000001)
#define BIT_1(A)  ((A) & 0x00000002)
#define BIT_2(A)  ((A) & 0x00000004)
#define BIT_3(A)  ((A) & 0x00000008)
#define BIT_4(A)  ((A) & 0x00000010)
#define BIT_5(A)  ((A) & 0x00000020)
#define BIT_6(A)  ((A) & 0x00000040)
#define BIT_7(A)  ((A) & 0x00000080)
#define BIT_8(A)  ((A) & 0x00000100)
#define BIT_9(A)  ((A) & 0x00000200)
#define BIT_A(A)  ((A) & 0x00000400)
#define BIT_B(A)  ((A) & 0x00000800)
#define BIT_C(A)  ((A) & 0x00001000)
#define BIT_D(A)  ((A) & 0x00002000)
#define BIT_E(A)  ((A) & 0x00004000)
#define BIT_F(A)  ((A) & 0x00008000)
#define BIT_10(A) ((A) & 0x00010000)
#define BIT_11(A) ((A) & 0x00020000)
#define BIT_12(A) ((A) & 0x00040000)
#define BIT_13(A) ((A) & 0x00080000)
#define BIT_14(A) ((A) & 0x00100000)
#define BIT_15(A) ((A) & 0x00200000)
#define BIT_16(A) ((A) & 0x00400000)
#define BIT_17(A) ((A) & 0x00800000)
#define BIT_18(A) ((A) & 0x01000000)
#define BIT_19(A) ((A) & 0x02000000)
#define BIT_1A(A) ((A) & 0x04000000)
#define BIT_1B(A) ((A) & 0x08000000)
#define BIT_1C(A) ((A) & 0x10000000)
#define BIT_1D(A) ((A) & 0x20000000)
#define BIT_1E(A) ((A) & 0x40000000)
#define BIT_1F(A) ((A) & 0x80000000)

/* Get the most significant bit for specific sizes */
#define GET_MSB_8(A)  ((A) & 0x80)
#define GET_MSB_9(A)  ((A) & 0x100)
#define GET_MSB_16(A) ((A) & 0x8000)
#define GET_MSB_17(A) ((A) & 0x10000)
#define GET_MSB_32(A) ((A) & 0x80000000)
#define GET_MSB_33(A) ((A) & U64(0x100000000))

/* Isolate nibbles */
#define LOW_NIBBLE(A)  ((A) & 0x0f)
#define HIGH_NIBBLE(A) ((A) & 0xf0)

/* These are used to isolate 8, 16, and 32 bit sizes */
#define MASK_OUT_ABOVE_2(A)  ((A) & 3)
#define MASK_OUT_ABOVE_8(A)  ((A) & 0xff)
#define MASK_OUT_ABOVE_16(A) ((A) & 0xffff)
#define MASK_OUT_BELOW_2(A)  ((A) & ~3)
#define MASK_OUT_BELOW_8(A)  ((A) & ~0xff)
#define MASK_OUT_BELOW_16(A) ((A) & ~0xffff)

/* No need to mask if we are 32 bit */
#define MASK_OUT_ABOVE_32(A) ((A) & 0xffffffffU)
#define MASK_OUT_BELOW_32(A) ((A) & ~0xffffffffU)

/* Shift & Rotate Macros. */
#define LSL(A, C) ((A) << (C))
#define LSR(A, C) ((A) >> (C))

/* We have to do this because the morons at ANSI decided that shifts
* by >= data size are undefined.
*/
#define LSR_32(A, C) ((C) < 32 ? (A) >> (C) : 0)
#define LSL_32(A, C) ((C) < 32 ? (A) << (C) : 0)

#define LSL_32_64(A, C) ((A) << (C))
#define LSR_32_64(A, C) ((A) >> (C))
#define ROL_33_64(A, C) (LSL_32_64(A, C) | LSR_32_64(A, 33-(C)))
#define ROR_33_64(A, C) (LSR_32_64(A, C) | LSL_32_64(A, 33-(C)))

#define ROL_8(A, C)      MASK_OUT_ABOVE_8(LSL(A, C) | LSR(A, 8-(C)))
#define ROL_9(A, C)                      (LSL(A, C) | LSR(A, 9-(C)))
#define ROL_16(A, C)    MASK_OUT_ABOVE_16(LSL(A, C) | LSR(A, 16-(C)))
#define ROL_17(A, C)                     (LSL(A, C) | LSR(A, 17-(C)))
#define ROL_32(A, C)    MASK_OUT_ABOVE_32(LSL_32(A, C) | LSR_32(A, 32-(C)))
#define ROL_33(A, C)                     (LSL_32(A, C) | LSR_32(A, 33-(C)))

#define ROR_8(A, C)      MASK_OUT_ABOVE_8(LSR(A, C) | LSL(A, 8-(C)))
#define ROR_9(A, C)                      (LSR(A, C) | LSL(A, 9-(C)))
#define ROR_16(A, C)    MASK_OUT_ABOVE_16(LSR(A, C) | LSL(A, 16-(C)))
#define ROR_17(A, C)                     (LSR(A, C) | LSL(A, 17-(C)))
#define ROR_32(A, C)    MASK_OUT_ABOVE_32(LSR_32(A, C) | LSL_32(A, 32-(C)))
#define ROR_33(A, C)                     (LSR_32(A, C) | LSL_32(A, 33-(C)))


/* ------------------------------ CPU Access ------------------------------ */

/* Access the CPU registers */
#define REG_DA()           m_dar /* easy access to data and address regs */
#define REG_D()            m_dar
#define REG_A()            (m_dar+8)
#define REG_PPC()          m_ppc
#define REG_PC()           m_pc
#define REG_SP_BASE()      m_sp
#define REG_USP()          m_sp[0]
#define REG_ISP()          m_sp[2]
#define REG_MSP()          m_sp[3]
#define REG_SP()           m_dar[15]

#define REG_FP()           m_fpr
#define REG_FPCR()         m_fpcr
#define REG_FPSR()         m_fpsr
#define REG_FPIAR()        m_fpiar


/* -------------------------- EA / Operand Access ------------------------- */

#define READ_EA_8(EA) read_ea_8(EA)
#define READ_EA_16(EA) read_ea_16(EA)
#define READ_EA_32(EA) read_ea_32(EA)
#define READ_EA_64(EA) read_ea_64(EA)

#define WRITE_EA_8(EA, DA) write_ea_8(EA, DA)
#define WRITE_EA_16(EA, DA) write_ea_16(EA, DA)
#define WRITE_EA_32(EA, DA) write_ea_32(EA, DA)
#define WRITE_EA_64(EA, DA) write_ea_64(EA, DA)

/*
 * The general instruction format follows this pattern:
 * .... XXX. .... .YYY
 * where XXX is register X and YYY is register Y
 */
/* Data Register Isolation */
#define DX() (REG_D()[(m_ir >> 9) & 7])
#define DY() (REG_D()[m_ir & 7])
/* Address Register Isolation */
#define AX() (REG_A()[(m_ir >> 9) & 7])
#define AY() (REG_A()[m_ir & 7])

/* Effective Address Calculations */
#define EA_AY_AI_8()   AY()                              /* address register indirect */
#define EA_AY_AI_16()  EA_AY_AI_8()
#define EA_AY_AI_32()  EA_AY_AI_8()
#define EA_AY_PI_8()   (AY()++)                          /* postincrement (size = byte) */
#define EA_AY_PI_16()  ((AY()+=2)-2)                     /* postincrement (size = word) */
#define EA_AY_PI_32()  ((AY()+=4)-4)                     /* postincrement (size = long) */
#define EA_AY_PD_8()   (--AY())                          /* predecrement (size = byte) */
#define EA_AY_PD_16()  (AY()-=2)                         /* predecrement (size = word) */
#define EA_AY_PD_32()  (AY()-=4)                         /* predecrement (size = long) */
#define EA_AY_DI_8()   get_ea_di(AY())                   /* displacement */
#define EA_AY_DI_16()  EA_AY_DI_8()
#define EA_AY_DI_32()  EA_AY_DI_8()
#define EA_AY_IX_8()   get_ea_ix(AY())                   /* indirect + index */
#define EA_AY_IX_16()  EA_AY_IX_8()
#define EA_AY_IX_32()  EA_AY_IX_8()

#define EA_AX_AI_8()   AX()
#define EA_AX_AI_16()  EA_AX_AI_8()
#define EA_AX_AI_32()  EA_AX_AI_8()
#define EA_AX_PI_8()   (AX()++)
#define EA_AX_PI_16()  ((AX()+=2)-2)
#define EA_AX_PI_32()  ((AX()+=4)-4)
#define EA_AX_PD_8()   (--AX())
#define EA_AX_PD_16()  (AX()-=2)
#define EA_AX_PD_32()  (AX()-=4)
#define EA_AX_DI_8()   get_ea_di(AX())
#define EA_AX_DI_16()  EA_AX_DI_8()
#define EA_AX_DI_32()  EA_AX_DI_8()
#define EA_AX_IX_8()   get_ea_ix(AX())
#define EA_AX_IX_16()  EA_AX_IX_8()
#define EA_AX_IX_32()  EA_AX_IX_8()

#define EA_A7_PI_8()   ((REG_A()[7]+=2)-2)
#define EA_A7_PD_8()   (REG_A()[7]-=2)

#define EA_AW_8()      get_ea_aw()                     /* absolute word */
#define EA_AW_16()     EA_AW_8()
#define EA_AW_32()     EA_AW_8()
#define EA_AL_8()      get_ea_al()                     /* absolute long */
#define EA_AL_16()     EA_AL_8()
#define EA_AL_32()     EA_AL_8()
#define EA_PCDI_8()    get_ea_pcdi()                   /* pc indirect + displacement */
#define EA_PCDI_16()   EA_PCDI_8()
#define EA_PCDI_32()   EA_PCDI_8()
#define EA_PCIX_8()    get_ea_pcix()                   /* pc indirect + index */
#define EA_PCIX_16()   EA_PCIX_8()
#define EA_PCIX_32()   EA_PCIX_8()

#define OPER_I_8()     read_imm_8()
#define OPER_I_16()    read_imm_16()
#define OPER_I_32()    read_imm_32()

/* --------------------------- Status Register ---------------------------- */

/* Flag Calculation Macros */
#define CFLAG_8(A)  (A)
#define CFLAG_16(A) ((A)>>8)

#define CFLAG_ADD_32(S, D, R) (((S & D) | (~R & (S | D)))>>23)
#define CFLAG_SUB_32(S, D, R) (((S & R) | (~D & (S | R)))>>23)

#define VFLAG_ADD_8(S, D, R)  ((S^R) & (D^R))
#define VFLAG_ADD_16(S, D, R) (((S^R) & (D^R))>>8)
#define VFLAG_ADD_32(S, D, R) (((S^R) & (D^R))>>24)

#define VFLAG_SUB_8(S, D, R)  ((S^D) & (R^D))
#define VFLAG_SUB_16(S, D, R) (((S^D) & (R^D))>>8)
#define VFLAG_SUB_32(S, D, R) (((S^D) & (R^D))>>24)

#define NFLAG_8(A)  (uint32_t)(A)
#define NFLAG_16(A) (uint32_t)((A)>>8)
#define NFLAG_32(A) (uint32_t)((A)>>24)
#define NFLAG_64(A) (uint32_t)((A)>>56)

#define ZFLAG_8(A)  MASK_OUT_ABOVE_8(A)
#define ZFLAG_16(A) MASK_OUT_ABOVE_16(A)
#define ZFLAG_32(A) MASK_OUT_ABOVE_32(A)


/* Flag values */
#define NFLAG_SET   0x80
#define NFLAG_CLEAR 0
#define CFLAG_SET   0x100
#define CFLAG_CLEAR 0
#define XFLAG_SET   0x100
#define XFLAG_CLEAR 0
#define VFLAG_SET   0x80
#define VFLAG_CLEAR 0
#define ZFLAG_SET   0
#define ZFLAG_CLEAR 0xffffffff

#define SFLAG_SET   4
#define SFLAG_CLEAR 0
#define MFLAG_SET   2
#define MFLAG_CLEAR 0

/* Turn flag values into 1 or 0 */
#define XFLAG_1() ((m_x_flag >> 8) & 1)
#define NFLAG_1() ((m_n_flag >> 7) & 1)
#define VFLAG_1() ((m_v_flag >> 7) & 1)
#define ZFLAG_1() (!m_not_z_flag)
#define CFLAG_1() ((m_c_flag >> 8) & 1)


/* Conditions */
#define COND_CS() (m_c_flag & 0x100)
#define COND_CC() (!COND_CS())
#define COND_VS() (m_v_flag & 0x80)
#define COND_VC() (!COND_VS())
#define COND_NE() (m_not_z_flag)
#define COND_EQ() (!COND_NE())
#define COND_MI() (m_n_flag & 0x80)
#define COND_PL() (!COND_MI())
#define COND_LT() ((m_n_flag ^ m_v_flag) & 0x80)
#define COND_GE() (!COND_LT())
#define COND_HI() (COND_CC() && COND_NE())
#define COND_LS() (COND_CS() || COND_EQ())
#define COND_GT() (COND_GE() && COND_NE())
#define COND_LE() (COND_LT() || COND_EQ())

/* Reversed conditions */
#define COND_NOT_CS() COND_CC()
#define COND_NOT_CC() COND_CS()
#define COND_NOT_VS() COND_VC()
#define COND_NOT_VC() COND_VS()
#define COND_NOT_NE() COND_EQ()
#define COND_NOT_EQ() COND_NE()
#define COND_NOT_MI() COND_PL()
#define COND_NOT_PL() COND_MI()
#define COND_NOT_LT() COND_GE()
#define COND_NOT_GE() COND_LT()
#define COND_NOT_HI() COND_LS()
#define COND_NOT_LS() COND_HI()
#define COND_NOT_GT() COND_LE()
#define COND_NOT_LE() COND_GT()

/* Not real conditions, but here for convenience */
#define COND_XS() (m_x_flag & 0x100)
#define COND_XC() (!COND_XS)


/* Operand fetching */
#define OPER_AY_AI_8() oper_ay_ai_8()
#define OPER_AY_AI_16() oper_ay_ai_16()
#define OPER_AY_AI_32() oper_ay_ai_32()
#define OPER_AY_PI_8() oper_ay_pi_8()
#define OPER_AY_PI_16() oper_ay_pi_16()
#define OPER_AY_PI_32() oper_ay_pi_32()
#define OPER_AY_PD_8() oper_ay_pd_8()
#define OPER_AY_PD_16() oper_ay_pd_16()
#define OPER_AY_PD_32() oper_ay_pd_32()
#define OPER_AY_DI_8() oper_ay_di_8()
#define OPER_AY_DI_16() oper_ay_di_16()
#define OPER_AY_DI_32() oper_ay_di_32()
#define OPER_AY_IX_8() oper_ay_ix_8()
#define OPER_AY_IX_16() oper_ay_ix_16()
#define OPER_AY_IX_32() oper_ay_ix_32()

#define OPER_AX_AI_8() oper_ax_ai_8()
#define OPER_AX_AI_16() oper_ax_ai_16()
#define OPER_AX_AI_32() oper_ax_ai_32()
#define OPER_AX_PI_8() oper_ax_pi_8()
#define OPER_AX_PI_16() oper_ax_pi_16()
#define OPER_AX_PI_32() oper_ax_pi_32()
#define OPER_AX_PD_8() oper_ax_pd_8()
#define OPER_AX_PD_16() oper_ax_pd_16()
#define OPER_AX_PD_32() oper_ax_pd_32()
#define OPER_AX_DI_8() oper_ax_di_8()
#define OPER_AX_DI_16() oper_ax_di_16()
#define OPER_AX_DI_32() oper_ax_di_32()
#define OPER_AX_IX_8() oper_ax_ix_8()
#define OPER_AX_IX_16() oper_ax_ix_16()
#define OPER_AX_IX_32() oper_ax_ix_32()

#define OPER_A7_PI_8() oper_a7_pi_8()
#define OPER_A7_PD_8() oper_a7_pd_8()

#define OPER_AW_8() oper_aw_8()
#define OPER_AW_16() oper_aw_16()
#define OPER_AW_32() oper_aw_32()
#define OPER_AL_8() oper_al_8()
#define OPER_AL_16() oper_al_16()
#define OPER_AL_32() oper_al_32()
#define OPER_PCDI_8() oper_pcdi_8()
#define OPER_PCDI_16() oper_pcdi_16()
#define OPER_PCDI_32() oper_pcdi_32()
#define OPER_PCIX_8() oper_pcix_8()
#define OPER_PCIX_16() oper_pcix_16()
#define OPER_PCIX_32() oper_pcix_32()

/* --------------------------------------------------------------- */

#ifdef USE_CPU_REAL_MACHINE_CYCLE

#define SET_ICOUNT(x) m_icount += (x)
#define RESET_ICOUNT(x) m_icount -= (x)
#define ACCUM_ICOUNT() (m_icount - m_initial_count)
#define SUBST_ICOUNT(x)

#else /* USE_CPU_REAL_MACHINE_CYCLE */

#define SET_ICOUNT(x) m_icount -= (x)
#define RESET_ICOUNT(x) m_icount += (x)
#define ACCUM_ICOUNT() (m_initial_count - m_icount)
#define SUBST_ICOUNT(x) m_icount -= (x)

#endif /* !USE_CPU_REAL_MACHINE_CYCLE */


#ifdef USE_MEM_REAL_MACHINE_CYCLE

#define SET_MEM_ICOUNT_000() SET_ICOUNT(4)
#define SET_MEM_ICOUNT_020() SET_ICOUNT(3)

#else /* USE_MEM_REAL_MACHINE_CYCLE */

#define SET_MEM_ICOUNT_000()
#define SET_MEM_ICOUNT_020()

#endif /* !USE_MEM_REAL_MACHINE_CYCLE */

/* ----------------------------- MMU ----------------------------- */
#ifdef USE_MC68000MMU

//#define SET_MMU_TMP_BUSERROR_RW(A) m_mmu_tmp_buserror_rw = (A)
//#define GET_MMU_TMP_BUSERROR_RW() m_mmu_tmp_buserror_rw
//#define SET_MMU_TMP_BUSERROR_FC(A) m_mmu_tmp_buserror_fc = (A)
//#define GET_MMU_TMP_BUSERROR_FC() m_mmu_tmp_buserror_fc
//#define SET_MMU_TMP_BUSERROR_SZ(A) m_mmu_tmp_buserror_sz = (A)
//#define GET_MMU_TMP_BUSERROR_SZ() m_mmu_tmp_buserror_sz
#define SET_MMU_TMP_FC(A) m_mmu_tmp_fc = (A)
#define GET_MMU_TMP_FC() m_mmu_tmp_fc
#define SET_MMU_TMP_RW(A) m_mmu_tmp_rw = (A)
#define GET_MMU_TMP_RW() m_mmu_tmp_rw
#define SET_MMU_TMP_SZ(A) m_mmu_tmp_sz = (A)
#define GET_MMU_TMP_SZ() m_mmu_tmp_sz

/* HMMU enable types for use with m68k_set_hmmu_enable() */
enum en_m68ki_hmmu_types {
	M68K_HMMU_DISABLE   = 0,   /* no translation */
	M68K_HMMU_ENABLE_II = 1,   /* Mac II style fixed translation */
	M68K_HMMU_ENABLE_LC = 2    /* Mac LC style fixed translation */
};
enum en_m68ki_mmu_sz {
	M68K_SZ_LONG = 0,
	M68K_SZ_BYTE = 1,
	M68K_SZ_WORD = 2,
};

#else /* !USE_MC68000MMU */

//#define SET_MMU_TMP_BUSERROR_RW(A)
//#define GET_MMU_TMP_BUSERROR_RW() 0
//#define SET_MMU_TMP_BUSERROR_FC(A)
//#define GET_MMU_TMP_BUSERROR_FC() 0
//#define SET_MMU_TMP_BUSERROR_SZ(A)
//#define GET_MMU_TMP_BUSERROR_SZ() 0
#define SET_MMU_TMP_FC(A)
#define GET_MMU_TMP_FC() 0
#define SET_MMU_TMP_RW(A)
#define GET_MMU_TMP_RW() 0
#define SET_MMU_TMP_SZ(A)
#define GET_MMU_TMP_SZ() 0

#endif

//////////////////////////////////////////////////////////////////////////////

/**
	@brief MC68000ABERR - store the status occurring the address and bus error
*/
class MC68000ABERR
{
public:
	bool m_occured;
	uint32_t m_num;
	uint32_t m_address;
	uint32_t m_mode;
	uint32_t m_fc;
	uint32_t m_size;

	struct vm_state {	// 32bytes
		uint32_t m_num;
		uint32_t m_address;
		uint32_t m_mode;
		uint32_t m_fc;

		uint32_t m_size;
		uint8_t  m_occured;
		char reserved[11];
	};

	MC68000ABERR();
	MC68000ABERR(uint32_t num, uint32_t address, uint32_t mode, uint32_t fc);
	MC68000ABERR(uint32_t num, uint32_t address, uint32_t mode, uint32_t fc, uint32_t size);
	~MC68000ABERR() {}
	void clear();
	void save_state(struct vm_state &v);
	bool load_state(const struct vm_state &v);
	size_t get_state_size() const;
};

//////////////////////////////////////////////////////////////////////////////

/**
	@brief MC68000BASE - base class of MC680x0
*/
class MC68000BASE : public DEVICE
{
// ----------------------------------------------------------
// enumerate
protected:
	/* Different ways to stop the CPU */
	enum en_m68ki_stop_causes {
		STOP_LEVEL_NONE       = 0x00,
		STOP_LEVEL_STOP       = 0x01,
		STOP_LEVEL_INTER_HALT = 0x02,
		STOP_LEVEL_OUTER_HALT = 0x04,
		STOP_LEVEL_BUSREQ     = 0x08,
		STOP_LEVEL_BGACK      = 0x10,
	};

	/* Used for 68000 address error processing */
	enum en_m68ki_aerr_berr_flags {
		INSTRUCTION_YES = 0x00,
		INSTRUCTION_NO  = 0x08,
		MODE_READ       = 0x01,
		MODE_WRITE      = 0x00
	};

	/* Runmode */
	typedef enum en_m68ki_run_mode {
		RUN_MODE_NORMAL              = 0,
		RUN_MODE_BERR_AERR_RESET_WSF = 1, // writing the stack frame
		RUN_MODE_BERR_AERR_RESET     = 2  // stack frame done
	} m68ki_run_mode_t;

	/* Processing mode */
	typedef enum en_m68ki_proc_mode {
		PROC_MODE_NORMAL = 0,
		PROC_MODE_ASSERT_RESET,		// asserted reset signal
		PROC_MODE_NEGATE_RESET,		// reset process
//		PROC_MODE_INTERNAL_HALT,	// duplicated error
		PROC_MODE_EXCEPTION_BUS_ERROR,
		PROC_MODE_EXCEPTION_ADDRESS_ERROR,
		PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION,
		PROC_MODE_EXCEPTION_INTERRUPT,
		PROC_MODE_EXCEPTION_ZERO_DIVIDE,
		PROC_MODE_EXCEPTION_CHK,
		PROC_MODE_EXCEPTION_TRAPV,
		PROC_MODE_EXCEPTION_TRAPN,
		PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION,
		PROC_MODE_EXCEPTION_TRACE,
		PROC_MODE_EXCEPTION_1010,
		PROC_MODE_EXCEPTION_1111,
		PROC_MODE_EXCEPTION_BREAKPOINT,
		PROC_MODE_EXCEPTION_FORMAT_ERROR,

		PROC_MODE_EXCEPTION_MMU_CONFIGURATION,
	} m68ki_proc_mode_t;

/* instruction cache constants */
#define M68K_IC_SIZE 128

	/* Exception Vectors handled by emulation */
	enum en_m68ki_exception_vectors {
		M68K_EXCEPTION_RESET                    = 0,
		M68K_EXCEPTION_BUS_ERROR                = 2,
		M68K_EXCEPTION_ADDRESS_ERROR            = 3,
		M68K_EXCEPTION_ILLEGAL_INSTRUCTION      = 4,
		M68K_EXCEPTION_ZERO_DIVIDE              = 5,
		M68K_EXCEPTION_CHK                      = 6,
		M68K_EXCEPTION_TRAPV                    = 7,
		M68K_EXCEPTION_PRIVILEGE_VIOLATION      = 8,
		M68K_EXCEPTION_TRACE                    = 9,
		M68K_EXCEPTION_1010                     = 10,
		M68K_EXCEPTION_1111                     = 11,
		M68K_EXCEPTION_FORMAT_ERROR             = 14,
		M68K_EXCEPTION_UNINITIALIZED_INTERRUPT  = 15,
		M68K_EXCEPTION_SPURIOUS_INTERRUPT       = 24,
		M68K_EXCEPTION_INTERRUPT_AUTOVECTOR     = 24,
		M68K_EXCEPTION_TRAP_BASE                = 32,
		M68K_EXCEPTION_MMU_CONFIGURATION        = 56 // only on 020/030
	};

	/* Exception categories for cycles and debugger */
	enum en_m68ki_exception_categories {
		M68K_EXCEPTION_CATEGORY_SSP             = 0,
		M68K_EXCEPTION_CATEGORY_PC,
		M68K_EXCEPTION_CATEGORY_BUS_ERROR,
		M68K_EXCEPTION_CATEGORY_ADDRESS_ERROR,
		M68K_EXCEPTION_CATEGORY_ILLEGAL_INSTRUCTION,
		M68K_EXCEPTION_CATEGORY_ZERO_DIVIDE,
		M68K_EXCEPTION_CATEGORY_CHK,
		M68K_EXCEPTION_CATEGORY_TRAPV,
		M68K_EXCEPTION_CATEGORY_PRIVILEGE_VIOLATION,
		M68K_EXCEPTION_CATEGORY_TRACE,
		M68K_EXCEPTION_CATEGORY_1010,
		M68K_EXCEPTION_CATEGORY_1111,
		M68K_EXCEPTION_CATEGORY_FORMAT_ERROR,
		M68K_EXCEPTION_CATEGORY_INTERRUPT,
		M68K_EXCEPTION_CATEGORY_TRAP,
		M68K_EXCEPTION_CATEGORY_OTHER
	};

	/* Special interrupt acknowledge values.
	 * Use these as special returns from the interrupt acknowledge callback
	 * (specified later in this header).
	 */

	enum en_m68ki_interrupt_vectors {
	/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
	 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
	 * acknowledge cycle instead of DTACK.
	 */
		M68K_INT_ACK_AUTOVECTOR    = 0xffffffff,

	/* Causes the spurious interrupt vector (0x18) to be taken
	 * This happens in a real 68K if BERR is asserted during the interrupt
	 * acknowledge cycle (i.e. no devices responded to the acknowledge).
	 */
		M68K_INT_ACK_SPURIOUS      = 0xfffffffe,
	};

	/* Prefetch flags */
	enum en_m68ki_prefetch_flags {
		PREF_ENABLE = 0x0001,
		PREF_IGNORE = 0x0002,
	};

#ifdef USE_MC68000CACHE
	/* Cache status */
	enum en_m68ki_cache_control {
		M68K_CACR_IBE = 0x10, // Instruction Burst Enable
		M68K_CACR_CI  = 0x08, // Clear Instruction Cache
		M68K_CACR_CEI = 0x04, // Clear Entry in Instruction Cache
		M68K_CACR_FI  = 0x02, // Freeze Instruction Cache
		M68K_CACR_EI  = 0x01  // Enable Instruction Cache
	};
#endif

// ----------------------------------------------------------
protected:
#ifdef USE_DEBUGGER
	//for dis-assemble
	MC68000DASM dasm;
	DEBUGGER *d_debugger;
	bool m_now_debugging;
	DEBUGGER_BUS *d_mem_stored[8]; /* Memory access on debugger mode */
//	uint32_t m_debug_ea_old;
	uint16_t m_debug_npc;
	uint32_t m_debug_sr;
#endif
	outputs_t outputs_res;	/* software reset */
	outputs_t outputs_halt;	/* stopped after bus/address error exception */

	outputs_t outputs_fc;	/* function code */
	outputs_t outputs_bg;	/* bus ground */

	static bool m_emulation_initialized;

protected:
	// context
	DEVICE *d_mem[8];		 /* Memory device on each function code */
	uint32_t m_cpu_clock;
	uint32_t m_cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
	uint32_t m_dar[16];      /* Data and Address Registers */
	uint32_t m_ppc;          /* Previous program counter */
	uint32_t m_pc;           /* Program Counter */
	uint32_t m_sp[4];        /* User, Interrupt, and Master Stack Pointers */
	uint32_t m_fc;			 /* Function Code */
#ifdef USE_MC68000VBR
	uint32_t m_vbr;          /* Vector Base Register (m68010+) */
	uint32_t m_sfc;          /* Source Function Code Register (m68010+) */
	uint32_t m_dfc;          /* Destination Function Code Register (m68010+) */
#endif
#ifdef USE_MC68000CACHE
	uint32_t m_cacr;         /* Cache Control Register (m68020, unemulated) */
	uint32_t m_caar;         /* Cache Address Register (m68020, unemulated) */
#endif
	uint32_t m_ir;           /* Instruction Register */
#ifdef USE_MC68000FPU
	floatx80 m_fpr[8];       /* FPU Data Register (m68030/040) */
	uint32_t m_fpiar;        /* FPU Instruction Address Register (m68040) */
	uint32_t m_fpsr;         /* FPU Status Register (m68040) */
	uint32_t m_fpcr;         /* FPU Control Register (m68040) */
#endif
	uint32_t m_t1_flag;      /* Trace 1 */
	uint32_t m_t0_flag;      /* Trace 0 */
	uint32_t m_s_flag;       /* Supervisor:0x8 / User:0x0 */
	uint32_t m_m_flag;       /* Master: 0x04 / Interrupt:0x0 */
	uint32_t m_x_flag;       /* Extend */
	uint32_t m_n_flag;       /* Negative */
	uint32_t m_not_z_flag;   /* Zero, inverted for speedups */
	uint32_t m_v_flag;       /* Overflow */
	uint32_t m_c_flag;       /* Carry */
	uint32_t m_int_mask;     /* I0-I2 */
	uint32_t m_int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	uint32_t m_int_level_pref; /* Prefetch state of interrupt pins IPL0-IPL2 */
	uint32_t m_int_vec_num;  /* Interrupt vector number */
	uint32_t m_stopped;      /* Stopped state */
	uint32_t m_pref_flags;	 /* Valid data in prefetch queue */
	uint32_t m_pref_addr;    /* Last prefetch address */
	uint32_t m_pref_data;    /* Data in the prefetch queue */
	uint32_t m_sr_mask;      /* Implemented status register bits */
	uint32_t m_instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	m68ki_run_mode_t m_run_mode;   /* Stores whether we are processing a reset, bus error, address error, or something else */
	uint32_t m_proc_mode;    /* State of processing mode b7-b0:proc mode(m68ki_proc_mode_t) b11-b8:intr level */
	uint32_t m_signals;		 /* Asserted signals */
#ifdef USE_MC68000MMU
	bool   m_has_pmmu;	     /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	bool   m_has_hmmu;       /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	int    m_pmmu_enabled;   /* Indicates if the PMMU is enabled */
	int    m_hmmu_enabled;   /* Indicates if the HMMU is enabled */
#endif
#ifdef USE_MC68000FPU
	bool   m_has_fpu;        /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */
	int    m_fpu_just_reset; /* Indicates the FPU was just reset */
#endif

	/* Clocks required for instructions / exceptions */
	uint32_t m_cyc_bcc_notake_b;
	uint32_t m_cyc_bcc_notake_w;
	uint32_t m_cyc_dbcc_f_noexp;
	uint32_t m_cyc_dbcc_f_exp;
	uint32_t m_cyc_scc_r_true;
	uint32_t m_cyc_movem_w;
	uint32_t m_cyc_movem_l;
	uint32_t m_cyc_shift;
	uint32_t m_cyc_reset;

	int  m_initial_count;
	int  m_icount;                     /* Current clock / Number of clocks remaining */
	int  m_multiple_cycle;
	int  m_reset_cycles;
	uint32_t m_tracing;
	uint64_t m_total_icount;

	/* Address error */
	MC68000ABERR m_aberr;

	bool m_nmi_pending;

	const uint16_t *m_state_table;
	const int16_t  *m_cyc_instruction;
	const int8_t   *m_cyc_exception;

	/* Callbacks to host */
//	m68k_irq_acknowledge_callback int_ack_callback;             /* Interrupt Acknowledge */
//	m68k_bkpt_ack_func bkpt_ack_callback;         /* Breakpoint Acknowledge */
//	m68k_reset_func reset_instr_callback;         /* Called when a RESET instruction is encountered */
//	m68k_cmpild_func cmpild_instr_callback;       /* Called when a CMPI.L #v, Dn instruction is encountered */
//	m68k_rte_func rte_instr_callback;             /* Called when a RTE instruction is encountered */
//	m68k_tas_func tas_instr_callback;             /* Called when a TAS instruction is encountered, allows / disallows writeback */

#ifdef USE_MC68000MMU
	/* PMMU registers */
	uint32_t m_mmu_crp_aptr, m_mmu_crp_limit;
	uint32_t m_mmu_srp_aptr, m_mmu_srp_limit;
	uint32_t m_mmu_urp_aptr;    /* 040 only */
	uint32_t m_mmu_tc;
	uint16_t m_mmu_sr;
	uint32_t m_mmu_sr_040;
	uint32_t m_mmu_atc_tag[MMU_ATC_ENTRIES], m_mmu_atc_data[MMU_ATC_ENTRIES];
	uint32_t m_mmu_atc_rr;
	uint32_t m_mmu_tt0, m_mmu_tt1;
	uint32_t m_mmu_itt0, m_mmu_itt1, m_mmu_dtt0, m_mmu_dtt1;
	uint32_t m_mmu_acr0, m_mmu_acr1, m_mmu_acr2, m_mmu_acr3;
	uint32_t m_mmu_last_page_entry, m_mmu_last_page_entry_addr;

	uint16_t m_mmu_tmp_sr;      /* temporary hack: status code for ptest and to handle write protection */
	uint16_t m_mmu_tmp_fc;      /* temporary hack: function code for the mmu (moves) */
	uint16_t m_mmu_tmp_rw;      /* temporary hack: read/write (1/0) for the mmu */
	uint8_t  m_mmu_tmp_sz;      /* temporary hack: size for mmu */
//	uint32_t m_mmu_tmp_buserror_address;   /* temporary hack: (first) bus error address */
//	uint16_t m_mmu_tmp_buserror_occurred;  /* temporary hack: flag that bus error has occurred from mmu */
//	uint16_t m_mmu_tmp_buserror_fc;   /* temporary hack: (first) bus error fc */
//	uint16_t m_mmu_tmp_buserror_rw;   /* temporary hack: (first) bus error rw */
//	uint16_t m_mmu_tmp_buserror_sz;   /* temporary hack: (first) bus error size` */

	bool     m_mmu_tablewalk;             /* set when MMU walks page tables */
	uint32_t m_mmu_last_logical_addr;
#endif

#ifdef USE_MC68000CACHE
	uint32_t m_ic_address[M68K_IC_SIZE];   /* instruction cache address data */
	uint32_t m_ic_data[M68K_IC_SIZE];      /* instruction cache content data */
	bool     m_ic_valid[M68K_IC_SIZE];     /* instruction cache valid flags */
#endif

	// for resume
#pragma pack(1)
	struct vm_state_st {
		uint32_t m_cpu_clock;
		uint32_t m_cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
		uint32_t m_ppc;          /* Previous program counter */
		uint32_t m_pc;           /* Program Counter */
		// 1
		uint32_t m_dar[16];      /* Data and Address Registers */
		// 5
		uint32_t m_sp[4];        /* User, Interrupt, and Master Stack Pointers */
		// 6
		uint32_t m_fc;			 /* Function Code */
		uint32_t m_vbr;          /* Vector Base Register (m68010+) */
		uint32_t m_sfc;          /* Source Function Code Register (m68010+) */
		uint32_t m_dfc;          /* Destination Function Code Register (m68010+) */
		// 7
		uint32_t m_cacr;         /* Cache Control Register (m68020, unemulated) */
		uint32_t m_caar;         /* Cache Address Register (m68020, unemulated) */
		uint32_t m_ir;           /* Instruction Register */
		uint32_t m_fpiar;        /* FPU Instruction Address Register (m68040) */
		// 8
		uint32_t m_fpsr;         /* FPU Status Register (m68040) */
		uint32_t m_fpcr;         /* FPU Control Register (m68040) */
		uint32_t reserved1[2];
		// 9
		struct {
			uint64_t high;
			uint64_t low;
		} m_fpr[8];              /* FPU Data Register (m68030/040) */
		// 17
		uint32_t m_t1_flag;      /* Trace 1 */
		uint32_t m_t0_flag;      /* Trace 0 */
		uint32_t m_s_flag;       /* Supervisor:0x8 / User:0x0 */
		uint32_t m_m_flag;       /* Master: 0x04 / Interrupt:0x0 */
		// 18
		uint32_t m_x_flag;       /* Extend */
		uint32_t m_n_flag;       /* Negative */
		uint32_t m_not_z_flag;   /* Zero, inverted for speedups */
		uint32_t m_v_flag;       /* Overflow */
		// 19
		uint32_t m_c_flag;       /* Carry */
		uint32_t m_int_mask;     /* I0-I2 */
		uint32_t m_int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
		uint32_t m_int_level_pref; /* Prefetch state of interrupt pins IPL0-IPL2 */
		// 20
		uint32_t m_int_vec_num;  /* Interrupt vector number */
		uint32_t m_stopped;      /* Stopped state */
		uint32_t m_pref_flags;	 /* Valid data in prefetch queue */
		uint32_t m_pref_addr;    /* Last prefetch address */
		// 21
		uint32_t m_pref_data;    /* Data in the prefetch queue */
		uint32_t m_sr_mask;      /* Implemented status register bits */
		uint32_t m_instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
		uint32_t m_run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */
		// 22
		uint32_t m_proc_mode;    /* State of processing mode b7-b0:proc mode(m68ki_proc_mode_t) b11-b8:intr level */
		uint32_t m_signals;		 /* Asserted signals */

		uint8_t  m_nmi_pending;

		uint8_t  m_has_pmmu;	 /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
		uint8_t  m_has_hmmu;     /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
		uint8_t  m_has_fpu;      /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */

		int    m_pmmu_enabled;   /* Indicates if the PMMU is enabled */
		// 23
		int    m_hmmu_enabled;   /* Indicates if the HMMU is enabled */
		int    m_fpu_just_reset; /* Indicates the FPU was just reset */

		uint32_t m_cyc_bcc_notake_b;
		uint32_t m_cyc_bcc_notake_w;
		// 24
		uint32_t m_cyc_dbcc_f_noexp;
		uint32_t m_cyc_dbcc_f_exp;
		uint32_t m_cyc_scc_r_true;
		uint32_t m_cyc_movem_w;
		// 25
		uint32_t m_cyc_movem_l;
		uint32_t m_cyc_shift;
		uint32_t m_cyc_reset;

		int  m_initial_count;
		// 26
		int  m_icount;                     /* Current clock / Number of clocks remaining */
		int  m_multiple_cycle;
		int  m_reset_cycles;
		uint32_t m_tracing;

		// 27
		uint64_t m_total_icount;
		uint32_t reserved2[2];

		/* PMMU registers */
		uint32_t m_mmu_crp_aptr, m_mmu_crp_limit;
		uint32_t m_mmu_srp_aptr, m_mmu_srp_limit;
		// 29
		uint32_t m_mmu_urp_aptr;    /* 040 only */
		uint32_t m_mmu_tc;

		uint32_t m_mmu_sr_040;
		uint32_t m_mmu_atc_rr;
		// 30
		uint32_t m_mmu_atc_tag[64], m_mmu_atc_data[64];
		// 62
		uint32_t m_mmu_tt0, m_mmu_tt1;
		uint32_t m_mmu_itt0, m_mmu_itt1;
		// 63
		uint32_t m_mmu_dtt0, m_mmu_dtt1;
		uint32_t m_mmu_acr0, m_mmu_acr1;
		// 64
		uint32_t m_mmu_acr2, m_mmu_acr3;
		uint32_t m_mmu_last_page_entry, m_mmu_last_page_entry_addr;
		// 65
		uint16_t m_mmu_sr;
		uint16_t m_mmu_tmp_sr;      /* temporary hack: status code for ptest and to handle write protection */
		uint16_t m_mmu_tmp_fc;      /* temporary hack: function code for the mmu (moves) */
		uint16_t m_mmu_tmp_rw;      /* temporary hack: read/write (1/0) for the mmu */
		uint8_t  m_mmu_tmp_sz;      /* temporary hack: size for mmu */
		uint8_t  m_mmu_tablewalk;             /* set when MMU walks page tables */
		uint16_t reserved3;
		uint32_t m_mmu_last_logical_addr;
		// 66
		uint32_t m_ic_address[128];   /* instruction cache address data */
		// 98
		uint32_t m_ic_data[128];      /* instruction cache content data */
		// 130
		uint8_t  m_ic_valid[128];     /* instruction cache valid flags */
		// 138
		struct MC68000ABERR::vm_state m_aberr;
		// 140
//		bool m_now_debugging;

//		uint32_t m_debug_ea_old;
//		uint16_t m_debug_npc;
//		uint32_t m_debug_sr;
	};
#pragma pack()

protected:
	// opcodes
	void run_one_opecode();
//	void op(uint8_t ireg);
//	inline void fetch_effective_address();

	void prefetch_irq_line(uint32_t data, uint32_t mask);
	void prefetch_irq_line(uint32_t id, bool onoff);
//	void set_irq_line(uint32_t data, uint32_t mask);
//	void set_irq_line(uint32_t id, bool onoff);

	void reset_peripherals();

	static const uint8_t    m68ki_shift_8_table[65];
	static const uint16_t   m68ki_shift_16_table[65];
	static const uint32_t   m68ki_shift_32_table[65];
	static const int8_t     m68ki_exception_cycle_table[NUM_CPU_TYPES][M68K_EXCEPTION_CATEGORY_OTHER + 1];
	static const int8_t     m68ki_ea_idx_cycle_table[64];

	inline void set_sm_flag_nosp(uint32_t value);
	inline void set_sr_noint_nosp(uint32_t value);

	typedef void (MC68000BASE::*opcode_handler_ptr)();

	static uint16_t m68ki_instruction_state_table[NUM_CPU_TYPES][0x10000]; /* opcode handler state numbers */
	static int16_t  m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

	struct opcode_handler_struct
	{
		uint16_t  match;			/* what to match after masking */
		uint16_t  mask;				/* mask on opcode */
		struct {
			int16_t a;				/* cycles included bus cycles */
			int16_t n;				/* cycles except bus cycles */
		} cycles[NUM_CPU_TYPES];	/* cycles each cpu type */
	};
	static const opcode_handler_ptr m68k_handler_table[];
	static const opcode_handler_struct m68k_opcode_table[];
	static const uint16_t m68k_state_illegal;

	void set_one(uint16_t opcode, uint16_t state, const opcode_handler_struct &s);
	void build_opcode_table();

protected:
	// ----------------------------------------------------------------------
	// low level access (abstract)

	virtual uint32_t RM8(uint32_t address, uint32_t fc) = 0;
	virtual void WM8(uint32_t address, uint32_t fc, uint32_t value) = 0;
	virtual uint32_t RM16(uint32_t address, uint32_t fc) = 0;
	virtual void WM16(uint32_t address, uint32_t fc, uint32_t value) = 0;
	virtual uint32_t RM32(uint32_t address, uint32_t fc) = 0;
	virtual void WM32(uint32_t address, uint32_t fc, uint32_t value) = 0;

	/// load from upper address at first
	virtual uint32_t RM32REV(uint32_t address, uint32_t fc) = 0;
	/// store to upper address at first
	virtual void WM32REV(uint32_t address, uint32_t fc, uint32_t value) = 0;

	virtual uint32_t RM_PROG_16(uint32_t address, uint32_t fc) = 0;
	virtual uint32_t RM_PROG_32(uint32_t address, uint32_t fc) = 0;

	uint32_t DEBUG_RM8(uint32_t addr, uint32_t fc);
	uint32_t DEBUG_RM16(uint32_t addr, uint32_t fc);

	// ----------------------------------------------------------------------

	INLINE void ic_clear();

	inline uint32_t ic_readimm16sub(uint32_t address);
	INLINE uint32_t ic_readimm16(uint32_t address);

	/* Read data immediately after the program counter */
	INLINE uint32_t read_imm_16_base();
	INLINE uint32_t read_imm_8();
	INLINE uint32_t read_imm_16();
	INLINE uint32_t read_imm_32();

	inline uint32_t read_first_16();

	INLINE uint32_t read_dummy_imm_16();

	// ----------------------------------------------------------------------

	// Read data with bus error check
	inline uint32_t read_8_bc (uint32_t address);
	inline uint32_t read_16_bc(uint32_t address);
	inline uint32_t read_32_bc(uint32_t address);

	// Write data with bus error check
	inline void write_8_bc (uint32_t address, uint32_t value);
	inline void write_16_bc(uint32_t address, uint32_t value);
	inline void write_32_bc(uint32_t address, uint32_t value);

	// Read data with address error check
	inline uint32_t read_8_ac (uint32_t address);
	inline uint32_t read_16_ac(uint32_t address);
	inline uint32_t read_32_ac(uint32_t address);

	// Write data with address error check
	inline void write_8_ac (uint32_t address, uint32_t value);
	inline void write_16_ac(uint32_t address, uint32_t value);
	inline void write_32_ac(uint32_t address, uint32_t value);

	// ----------------------------------------------------------------------

	// Read from the current address space
	INLINE uint32_t read_8(uint32_t address);
	INLINE uint32_t read_16(uint32_t address);
	INLINE uint32_t read_32(uint32_t address);

	// Write to the current data space
	INLINE void write_8(uint32_t address, uint32_t value);
	INLINE void write_16(uint32_t address, uint32_t value);
	INLINE void write_32(uint32_t address, uint32_t value);

	// Read from the current address space ignore alinment
	INLINE uint32_t read_naerr_8(uint32_t address);
	INLINE uint32_t read_naerr_16(uint32_t address);
	INLINE uint32_t read_naerr_32(uint32_t address);

	/* Read from the current address space */
//	INLINE uint32_t read_stack_8(uint32_t address);
	INLINE uint32_t read_stack_16(uint32_t address);
	INLINE uint32_t read_stack_32(uint32_t address);
	INLINE uint32_t read_stack_32_rev(uint32_t address);

	/* Write to the current data space */
//	INLINE void write_stack_8(uint32_t address, uint32_t value);
	INLINE void write_stack_16(uint32_t address, uint32_t value);
	INLINE void write_stack_32(uint32_t address, uint32_t value);
	INLINE void write_stack_32_rev(uint32_t address, uint32_t value);

	/* Read from vector number on the current memory */
	INLINE uint32_t read_intr_8(uint32_t address);
	INLINE uint32_t read_intr_16(uint32_t address);

	/* Indexed and PC-relative ea fetching */
	INLINE uint32_t get_ea_aw();
	INLINE uint32_t get_ea_al();
	INLINE uint32_t get_ea_pcdi();
	INLINE uint32_t get_ea_pcix();
	virtual uint32_t get_ea_ix(uint32_t An);
	INLINE uint32_t get_ea_di(uint32_t An);

	/* Operand fetching */
	INLINE uint32_t oper_ay_ai_8();
	INLINE uint32_t oper_ay_ai_16();
	INLINE uint32_t oper_ay_ai_32();
	INLINE uint32_t oper_ay_pi_8();
	INLINE uint32_t oper_ay_pi_16();
	INLINE uint32_t oper_ay_pi_32();
	INLINE uint32_t oper_ay_pd_8();
	INLINE uint32_t oper_ay_pd_16();
	INLINE uint32_t oper_ay_pd_32();
	INLINE uint32_t oper_ay_di_8();
	INLINE uint32_t oper_ay_di_16();
	INLINE uint32_t oper_ay_di_32();
	INLINE uint32_t oper_ay_ix_8();
	INLINE uint32_t oper_ay_ix_16();
	INLINE uint32_t oper_ay_ix_32();

	INLINE uint32_t oper_ax_ai_8();
	INLINE uint32_t oper_ax_ai_16();
	INLINE uint32_t oper_ax_ai_32();
	INLINE uint32_t oper_ax_pi_8();
	INLINE uint32_t oper_ax_pi_16();
	INLINE uint32_t oper_ax_pi_32();
	INLINE uint32_t oper_ax_pd_8();
	INLINE uint32_t oper_ax_pd_16();
	INLINE uint32_t oper_ax_pd_32();
	INLINE uint32_t oper_ax_di_8();
	INLINE uint32_t oper_ax_di_16();
	INLINE uint32_t oper_ax_di_32();
	INLINE uint32_t oper_ax_ix_8();
	INLINE uint32_t oper_ax_ix_16();
	INLINE uint32_t oper_ax_ix_32();

	INLINE uint32_t oper_a7_pi_8();
	INLINE uint32_t oper_a7_pd_8();

	INLINE uint32_t oper_aw_8();
	INLINE uint32_t oper_aw_16();
	INLINE uint32_t oper_aw_32();
	INLINE uint32_t oper_al_8();
	INLINE uint32_t oper_al_16();
	INLINE uint32_t oper_al_32();
	INLINE uint32_t oper_pcdi_8();
	INLINE uint32_t oper_pcdi_16();
	INLINE uint32_t oper_pcdi_32();
	INLINE uint32_t oper_pcix_8();
	INLINE uint32_t oper_pcix_16();
	INLINE uint32_t oper_pcix_32();

	/* Stack operations */
	INLINE void push_16(uint32_t value);
	INLINE void push_32(uint32_t value);
	INLINE uint32_t pull_16();
	INLINE uint32_t pull_32();

	INLINE void fake_push_16();
	INLINE void fake_push_32();
	INLINE void fake_pull_16();
	INLINE void fake_pull_32();

	/* Program flow operations */
	INLINE void jump(uint32_t new_pc);
	INLINE void jump_vector(uint32_t category, uint32_t vector);
	INLINE void branch_8(uint32_t offset);
	INLINE void branch_16(uint32_t offset);
	INLINE void branch_32(uint32_t offset);
	INLINE void non_branch_8(uint32_t offset);		// for debugger
	INLINE void non_branch_16(uint32_t offset);		// for debugger
	INLINE void non_branch_32(uint32_t offset);		// for debugger

	/* Status register operations. */
	inline void update_current_sp();                   // update current stack pointer
	INLINE void set_current_sp(uint32_t value);        // set a value of current stack pointer and backup it
	INLINE void set_s_flag(uint32_t value);            // Only bit 2 of value should be set
	INLINE void set_sm_flag(uint32_t value);           // only bits 1 and 2 of value should be set
	INLINE void set_ccr(uint32_t value);               // set the condition code register
	INLINE void set_sr(uint32_t value);                // set the status register
	INLINE void set_sr_noint(uint32_t value);          // set the status register

	/* Get the condition code register */
	INLINE uint32_t get_ccr();

	/* Get the status register */
	INLINE uint32_t get_sr();

	/* Set the fuction code */
	INLINE void set_fc(uint32_t new_fc);

	/* Exception processing */
	INLINE uint32_t init_exception(uint32_t category, uint32_t vector); /* Initial exception processing */
	inline void term_exception();

	INLINE void stack_frame_3word(uint32_t pc, uint32_t sr); /* Stack various frame types */
	INLINE void stack_frame_buserr(uint32_t sr);

	INLINE void stack_frame_0000(uint32_t pc, uint32_t sr, uint32_t vector);
	INLINE void stack_frame_0001(uint32_t pc, uint32_t sr, uint32_t vector);
	INLINE void stack_frame_0010(uint32_t sr, uint32_t vector);
	INLINE void stack_frame_1000(uint32_t pc, uint32_t sr, uint32_t vector);
	INLINE void stack_frame_1010(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address);
	INLINE void stack_frame_1011(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address);
	INLINE void stack_frame_0111(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address, bool in_mmu);

	INLINE void exception_interrupt(uint32_t int_level);
	INLINE void exception_trap(uint32_t category, uint32_t vector);
	INLINE void exception_trapN(uint32_t vector_base, uint32_t number);
	INLINE void exception_trace();
	INLINE void exception_privilege_violation();
	INLINE void exception_1010();
	INLINE void exception_1111();
	INLINE void exception_breakpoint();
	INLINE void exception_illegal();
	INLINE void exception_format_error();
	INLINE void exception_address_error();
	INLINE void exception_bus_error();

	inline void has_address_error_for_prog_read(uint32_t pc, uint32_t address, uint32_t fc);
	inline void has_address_error_for_data_write(uint32_t pc, uint32_t address, uint32_t fc, uint32_t value);
	inline void has_address_error_for_data_read(uint32_t pc, uint32_t address, uint32_t fc);
	inline void has_address_error_for_stack_write(uint32_t pc, uint32_t address, uint32_t fc, uint32_t value);
	inline void has_address_error_for_stack_read(uint32_t pc, uint32_t address, uint32_t fc);

	inline void has_bus_error_for_write(uint32_t address, uint32_t fc, uint32_t value);
	inline void has_bus_error_for_read(uint32_t address, uint32_t fc, uint32_t value);

	inline bool check_interrupts();            /* ASG: check for interrupts */

	void show_error_message(uint32_t pc, uint32_t address);

	/* Initiates trace checking before each instruction (t1) */
	inline void trace_t1() { m_tracing = m_t1_flag; }
	/* adds t0 to trace checking if we encounter change of flow */
	inline void trace_t0() { m_tracing |= m_t0_flag; }
	/* Clear all tracing */
	inline void clear_trace() { m_tracing = 0; }
	/* Cause a trace exception if we are tracing */
	inline void exception_if_trace();

	uint8_t  read_ea_8(int ea);
	uint16_t read_ea_16(int ea);
	uint32_t read_ea_32(int ea);
	uint64_t read_ea_64(int ea);

	void write_ea_8(int ea, uint8_t data);
	void write_ea_16(int ea, uint16_t data);
	void write_ea_32(int ea, uint32_t data);
	void write_ea_64(int ea, uint64_t data);

#include "mc68000ops.h"
#include "mc68000fpu.h"
#include "mc68000mmu.h"

public:
	MC68000BASE(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68000BASE() {}

	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual void warm_reset(bool por);
	int run(int clock);
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	int run(int clock, int accum, int cycle);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc() {
		return m_ppc;
	}
	uint32_t get_next_pc() {
		return m_pc;
	}
	void set_cpu_clock(uint32_t clk) {
		m_cpu_clock = clk;
	}
	uint32_t get_cpu_clock() const {
		return m_cpu_clock;
	}
	uint64_t get_current_clock();

	// unique function
	void set_context_mem(int fc, DEVICE* device) {
		d_mem[fc & 7] = device;
//#ifdef USE_DEBUGGER
//		dasm.set_context_mem(fc, device);
//#endif /* USE_DEBUGGER */
	}
#if 0
	void set_context_program_mem(DEVICE* device) {
		d_program = device;
#ifdef USE_DEBUGGER
		dasm.set_context_progmem(device);
#endif /* USE_DEBUGGER */
	}
	void set_context_cpu_space(DEVICE* device) {
		d_cpuspace = device;
#ifdef USE_DEBUGGER
		dasm.set_context_cpu_space(device);
#endif /* USE_DEBUGGER */
	}
#endif
	void set_context_reset(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_res, device, id, mask);
	}
	void set_context_halt(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_halt, device, id, mask);
	}
	void set_context_fc(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_fc, device, id, mask);
	}
	void set_context_bg(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_bg, device, id, mask);
	}
	uint32_t *get_fc_ptr() {
		return &m_fc;
	}

	virtual void save_state(FILEIO *fio);
	virtual bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
		dasm.set_context_debugger(device);
	}
	DEVICE *get_debugger()
	{
		return d_debugger;
	}
	uint32_t debug_prog_addr_mask()
	{
		return 0xffffffff;
	}
	uint32_t debug_data_addr_mask()
	{
		return 0xffffffff;
	}
	uint32_t debug_data_mask()
	{
		return 0xffff;
	}
	uint32_t debug_io_addr_mask()
	{
		return 0xffffffff;
	}
	bool debug_ioport_is_supported() const
	{
		// mc680x0 have only memory mapped i/o
		return false;
	}
	bool debug_exception_is_supported() const
	{
		return true;
	}
	uint32_t debug_read_data16(int type, uint32_t addr);

	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	bool get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen);
	int debug_dasm(int type, uint32_t addr, _TCHAR *buffer, size_t buffer_len, int flags);
	int debug_dasm_label(int type, uint32_t addr, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len);
	int debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len);
	bool reach_break_point();
//	void go_suspend();
//	bool now_suspend() const;
	uint32_t get_debug_pc(int type);
	uint32_t get_debug_next_pc(int type);
	uint32_t get_debug_branch_pc(int type);

	typedef struct st_names_map {
		const _TCHAR *name;
		uint32_t        num;
	} names_map_t;

	static const names_map_t signal_names_map[];
	bool get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	bool get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	void get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len);

	static const names_map_t exception_names_map[];
	bool get_debug_exception_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	bool get_debug_exception_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	void get_debug_exception_names_str(_TCHAR *buffer, size_t buffer_len);
#endif /* USE_DEBUGGER */
};

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68000
/**
	@brief MC68000 - CPU

	@note The data bus width is 16bits.
	So a read/write data b15-b8 always exists at even address, and b7-b0 does at odd. 
*/
class MC68000 : public MC68000BASE
{
public:
	MC68000(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68000() {}

	// common functions
	virtual void initialize();
//	void reset();

#ifdef USE_DEBUGGER
	uint32_t debug_prog_addr_mask()
	{
		return 0xffffff;
	}
	uint32_t debug_data_addr_mask()
	{
		return 0xffffff;
	}
#endif

protected:
	// ----------------------------------------------------------------------
	// low level access

	virtual uint32_t RM8(uint32_t address, uint32_t fc);
	virtual void WM8(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM16(uint32_t address, uint32_t fc);
	virtual void WM16(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM32(uint32_t address, uint32_t fc);
	virtual void WM32(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM32REV(uint32_t address, uint32_t fc);
	virtual void WM32REV(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM_PROG_16(uint32_t address, uint32_t fc);
	virtual uint32_t RM_PROG_32(uint32_t address, uint32_t fc);
};
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68008
/**
	@brief MC68008 - CPU
*/
class MC68008 : public MC68000BASE
{
public:
	MC68008(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68008() {}

	// common functions
	virtual void initialize();
//	void reset();

#ifdef USE_DEBUGGER
	uint32_t debug_prog_addr_mask()
	{
		return 0xfffff;
	}
	uint32_t debug_data_addr_mask()
	{
		return 0xfffff;
	}
#endif

protected:
	// ----------------------------------------------------------------------
	// low level access

	virtual uint32_t RM8(uint32_t address, uint32_t fc);
	virtual void WM8(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM16(uint32_t address, uint32_t fc);
	virtual void WM16(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM32(uint32_t address, uint32_t fc);
	virtual void WM32(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM32REV(uint32_t address, uint32_t fc);
	virtual void WM32REV(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM_PROG_16(uint32_t address, uint32_t fc);
	virtual uint32_t RM_PROG_32(uint32_t address, uint32_t fc);
};
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68010
/**
	@brief MC68010 - CPU
*/
class MC68010 : public MC68000
{
public:
	MC68010(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68010() {}

	// common functions
	virtual void initialize();
//	void reset();
};
#endif

//////////////////////////////////////////////////////////////////////////////

#if defined(USE_MC68020) || defined(USE_MC68000MMU)
/**
	@brief MC68020 - CPU
*/
class MC68020 : public MC68000BASE
{
public:
	MC68020(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68020() {}

	// common functions
	virtual void initialize();
//	void reset();

protected:
	// ----------------------------------------------------------------------
	// low level access

	virtual uint32_t RM8(uint32_t address, uint32_t fc);
	virtual void WM8(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM16(uint32_t address, uint32_t fc);
	virtual void WM16(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM32(uint32_t address, uint32_t fc);
	virtual void WM32(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM32REV(uint32_t address, uint32_t fc);
	virtual void WM32REV(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM_PROG_16(uint32_t address, uint32_t fc);
	virtual uint32_t RM_PROG_32(uint32_t address, uint32_t fc);

	// ----------------------------------------------------------------------

	virtual uint32_t get_ea_ix(uint32_t An);

};
#endif

#ifdef USE_MC68000MMU
/**
	@brief MC68020 with MMU - CPU
*/
class MC68020MMU : public MC68020
{
public:
	MC68020MMU(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68020MMU() {}

	// common functions
	virtual void initialize();
//	void reset();

protected:
	// ----------------------------------------------------------------------
	// low level access

	virtual uint32_t RM8(uint32_t address, uint32_t fc);
	virtual void WM8(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM16(uint32_t address, uint32_t fc);
	virtual void WM16(uint32_t address, uint32_t fc, uint32_t value);
	virtual uint32_t RM32(uint32_t address, uint32_t fc);
	virtual void WM32(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM32REV(uint32_t address, uint32_t fc);
	virtual void WM32REV(uint32_t address, uint32_t fc, uint32_t value);

	virtual uint32_t RM_PROG_16(uint32_t address, uint32_t fc);
	virtual uint32_t RM_PROG_32(uint32_t address, uint32_t fc);
};
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68EC030
/**
	@brief MC68EC030 - CPU
*/
class MC68EC030 : public MC68020
{
public:
	MC68EC030(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68EC030() {}

	// common functions
	virtual void initialize();
//	void reset();
};
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68000MMU
/**
	@brief MC68030 - CPU
*/
class MC68030 : public MC68020MMU
{
public:
	MC68030(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~MC68030() {}

	// common functions
	virtual void initialize();
//	void reset();

};
#endif

#endif /* MC68000_H */

