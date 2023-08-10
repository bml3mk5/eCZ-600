/** @file mc68000_consts.h

	Skelton for retropc emulator

	@par Origin  MAME 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@brief [ MC68000 constant values ]
*/

#ifndef MC68000_CONSTS_H
#define MC68000_CONSTS_H

/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */
/* ======================================================================== */

#define M68KI_REG_DEFAULT_VAL 0x87654321

/* Signals */
enum en_m68ki_signals {
	SIG_M68K_IPL0 = 80,
	SIG_M68K_IPL1,
	SIG_M68K_IPL2,
	SIG_M68K_BUSERR,
	SIG_M68K_ADDRERR,
	SIG_M68K_VPA_AVEC,
//	SIG_M68K_BUSREQ,
	SIG_M68K_FC,
};

/* Signal Masks */
enum en_m68ki_signal_masks {
	SIG_MASK_M68K_HALT		 = 0x00001,
	SIG_MASK_M68K_INTER_HALT = 0x00002,
	SIG_MASK_M68K_BUSERR	 = 0x00004,
	SIG_MASK_M68K_VPA_AVEC	 = 0x00008,
	SIG_MASK_M68K_BUSREQ	 = 0x00010,
	SIG_MASK_M68K_ADDRERR	 = 0x00020,
	SIG_MASK_M68K_STOPBYSW	 = 0x00040,
	SIG_MASK_M68K_EXCEPTION	 = 0x00080,
	SIG_MASK_M68K_SOFTALL	 = 0x000e2,
	SIG_MASK_M68K_IPL0		 = 0x00100,
	SIG_MASK_M68K_IPL1		 = 0x00200,
	SIG_MASK_M68K_IPL2		 = 0x00400,
	SIG_MASK_M68K_IPLALL	 = 0x00700,
	SIG_MASK_M68K_MMUBUSERR	 = 0x00800,
	SIG_MASK_M68K_BUSERRALL	 = 0x00804,
	SIG_MASK_M68K_RESET		 = 0x08000,
	SIG_MASK_M68K_DEBUG_INTR = 0xf0000,
};

/* Function codes set by CPU during data/address bus activity */
enum en_m68ki_function_codes {
	M68K_FC_USER_DATA          = 1,
	M68K_FC_USER_PROGRAM       = 2,
	M68K_FC_SUPERVISOR_DATA    = 5,
	M68K_FC_SUPERVISOR_PROGRAM = 6,
	M68K_FC_CPU_SPACE          = 7
};

/* CPU types for deciding what to emulate */
enum en_m68ki_cpu_types {
	CPU_TYPE_000      = 0x00000001,
	CPU_TYPE_008      = 0x00000002,
	CPU_TYPE_010      = 0x00000004,
	CPU_TYPE_EC020    = 0x00000008,
	CPU_TYPE_020      = 0x00000010,
	CPU_TYPE_EC030    = 0x00000020,
	CPU_TYPE_030      = 0x00000040,
	CPU_TYPE_EC040    = 0x00000080,
	CPU_TYPE_LC040    = 0x00000100,
	CPU_TYPE_040      = 0x00000200,
	CPU_TYPE_SCC070   = 0x00000400,
	CPU_TYPE_FSCPU32  = 0x00000800,
	CPU_TYPE_COLDFIRE = 0x00001000,
};

/* Disable certain comparisons if we're not using all CPU types */
#define CPU_TYPE                  (m_cpu_type)

#define CPU_TYPE_IS_COLDFIRE()    (CPU_TYPE & (CPU_TYPE_COLDFIRE))

#define CPU_TYPE_IS_040_PLUS()    (CPU_TYPE & (CPU_TYPE_040 | CPU_TYPE_EC040))

#define CPU_TYPE_IS_030_PLUS()    (CPU_TYPE & (CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040))

#define CPU_TYPE_IS_020_PLUS()    (CPU_TYPE & (CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE))

#define CPU_TYPE_IS_020()         (CPU_TYPE & (CPU_TYPE_EC020 | CPU_TYPE_020))
#define CPU_TYPE_IS_020_VARIANT() (CPU_TYPE & (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_FSCPU32))

#define CPU_TYPE_IS_EC020_PLUS()  (CPU_TYPE & (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE))
#define CPU_TYPE_IS_EC020_LESS()  (CPU_TYPE & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020))

#define CPU_TYPE_IS_010()         (CPU_TYPE == CPU_TYPE_010)
#define CPU_TYPE_IS_010_PLUS()    (CPU_TYPE & (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 | CPU_TYPE_COLDFIRE))
#define CPU_TYPE_IS_010_LESS()    (CPU_TYPE & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_SCC070))

#define CPU_TYPE_IS_000()         (CPU_TYPE == CPU_TYPE_000 || CPU_TYPE == CPU_TYPE_008)

#define CPU_TYPE_IS_070()         (CPU_TUPE == CPU_TYPE_SCC070)

/* ======================================================================== */

#ifndef USE_MC68000VBR
#if !(defined(USE_MC68000) || defined(USE_MC68008))
#define USE_MC68000VBR 1
#endif
#endif

#ifndef USE_MC68000CACHE
#if defined(USE_MC68020) || defined(USE_MC68EC020) || defined(USE_MC68020MMU)
#define USE_MC68000CACHE 1
#endif
#endif

#ifndef USE_MC68000MMU
#if defined(USE_MC68020MMU) || defined(USE_MC68030)
#define USE_MC68000MMU 1
#endif
#endif

#ifndef USE_MC68000FPU
#if defined(USE_MC68030) || defined(USE_MC68EC030)
#define USE_MC68000FPU 1
#endif
#endif

#ifndef USE_MC68020
#if defined(USE_MC68030) || defined(USE_MC68EC030)
#define USE_MC68020 1
#endif
#endif

#ifdef USE_MC68000MMU
/* MMU constants */
#if defined(USE_MC68030)
#define MMU_ATC_ENTRIES (22)    // 68030 has 22
#else
#define MMU_ATC_ENTRIES (64)    // 68851 has 64
#endif
#endif

#endif /* MC68000_CONSTS_H */

