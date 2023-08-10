/** @file mc68000fpu.h

	Skelton for retropc emulator

	@par Origin MAME 0.239, 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@note This is a part of MC68000BASE class, so included in mc68000.h.

	@brief [ MC68000 FPU ]
*/

#ifndef MC68000FPU_H
#define MC68000FPU_H

//#include "../common.h"
#include "vm_defs.h"
#include "mc68000_consts.h"

#ifdef USE_MC68000FPU

enum en_fpcc_cond {
	FPCC_N          = 0x08000000,
	FPCC_Z          = 0x04000000,
	FPCC_I          = 0x02000000,
	FPCC_NAN        = 0x01000000,
	FPES_OE         = 0x00002000,
	FPAE_IOP        = 0x00000080,
};

static double fx80_to_double(floatx80 fx);

static floatx80 double_to_fx80(double in);

static const uint32_t pkmask2[18];
static const uint32_t pkmask3[18];
inline floatx80 load_extended_float80(uint32_t ea);
inline void store_extended_float80(uint32_t ea, floatx80 fpr);
inline floatx80 load_pack_float80(uint32_t ea);
inline void store_pack_float80(uint32_t ea, int k, floatx80 fpr);
inline floatx80 propagateFloatx80NaNOneArg(floatx80 a);
inline void normalizeFloatx80Subnormal(uint64_t aSig, int32_t *zExpPtr, uint64_t *zSigPtr);
inline floatx80 getman(floatx80 src);
inline void SET_CONDITION_CODES(floatx80 reg);
inline int TEST_CONDITION(int condition);
//uint8_t READ_EA_8(int ea);
//uint16_t READ_EA_16(int ea);
//uint32_t READ_EA_32(int ea);
//uint64_t READ_EA_64(int ea);
floatx80 READ_EA_FPE(int mode, int reg, uint32 di_mode_ea);
floatx80 READ_EA_PACK(int ea);
//void WRITE_EA_8(int ea, uint8_t data);
//void WRITE_EA_16(int ea, uint16_t data);
//void WRITE_EA_32(int ea, uint32_t data);
//void WRITE_EA_64(int ea, uint64_t data);
void WRITE_EA_FPE(int mode, int reg, floatx80 fpr, uint32 di_mode_ea);
void WRITE_EA_PACK(int ea, int k, floatx80 fpr);
void fpgen_rm_reg(uint16_t w2);
void fmove_reg_mem(uint16_t w2);
void fmove_fpcr(uint16_t w2);
void fmovem(uint16_t w2);
void fscc();
void fbcc16();
void fbcc32();
void m68040_fpu_op0();
int perform_fsave(uint32_t addr, int inc);
void do_frestore_null();
void m68040_do_fsave(uint32_t addr, int reg, int inc);
void m68040_do_frestore(uint32_t addr, int reg);
void m68040_fpu_op1();
void m68881_ftrap();

#else

#define m68040_fpu_op0()
#define m68040_fpu_op1()
#define m68881_mmu_ops()

#endif

#endif /* MC68000FPU_H */

