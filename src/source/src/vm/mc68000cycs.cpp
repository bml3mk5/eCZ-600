/** @file mc68000cycs.cpp

	Skelton for retropc emulator

	@par Origin MAME 0.293, 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@brief [ MC68000 cycles ]
*/

#include "mc68000.h"

/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* Used by shift & rotate instructions */
const uint8_t MC68000BASE::m68ki_shift_8_table[65] =
{
	0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff
};
const uint16_t MC68000BASE::m68ki_shift_16_table[65] =
{
	0x0000, 0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
	0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff
};
const uint32_t MC68000BASE::m68ki_shift_32_table[65] =
{
	0x00000000, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
	0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
	0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
	0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
	0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
	0xfffffffc, 0xfffffffe, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

#ifndef USE_MEM_REAL_MACHINE_CYCLE
/* Number of clock cycles to use for exception processing.
 * I used 4 for any vectors that are undocumented for processing times.
 */
const int8_t MC68000BASE::m68ki_exception_cycle_table[NUM_CPU_TYPES][M68K_EXCEPTION_CATEGORY_OTHER + 1] =
{
	{ /* 000 */
			40, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			34, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			34, /*  9: Trace                                              */
			 4, /* 10: 1010                                               */
			 4, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			44, /* 15: Interrupt                                          */
			34, /* 32: TRAP #                                             */
			 4
	},
	{ /* 070 - not even pretending to be correct */
			40, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			126, /*  2: Bus Error                             (unemulated) */
			126, /*  3: Address Error                         (unemulated) */
			38, /*  4: Illegal Instruction                                */
			44, /*  5: Divide by Zero                                     */
			44, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			38, /*  8: Privilege Violation                                */
			38, /*  9: Trace                                              */
			 4, /* 10: 1010                                               */
			 4, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			44, /* 15: Interrupt                                          */
			38, /* 32: TRAP #                                             */
			 4
	},
	{ /* 010 */
			40, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			126, /*  2: Bus Error                             (unemulated) */
			126, /*  3: Address Error                         (unemulated) */
			38, /*  4: Illegal Instruction                                */
			44, /*  5: Divide by Zero                                     */
			44, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			38, /*  8: Privilege Violation                                */
			38, /*  9: Trace                                              */
			 4, /* 10: 1010                                               */
			 4, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			44, /* 15: Interrupt                                          */
			38, /* 32: TRAP #                                             */
			 4
	},
	{ /* 020 */
			 4, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			30, /* 15: Interrupt                                          */
			20, /* 32: TRAP #0                                            */
			 4
	},
	{ /* 030 - not correct */
			 4, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			30, /* 15: Interrupt                                          */
			20, /* 32: TRAP #                                             */
			 4
	},
	{ /* 040 */ // TODO: these values are not correct
			 4, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			30, /* 15: Interrupt                                          */
			20, /* 32: TRAP #                                             */
			 4
	},
	{ /* CPU32 */
			 4, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			30, /* 15: Interrupt                                          */
			20, /* 32: TRAP #                                             */
			 4
	},
	{ /* ColdFire - not correct */
			 4, /*  0: Reset - Initial Stack Pointer                      */
			 4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			 4, /* 14: Format Error                                       */
			30, /* 15: Interrupt                                          */
			20, /* 32: TRAP #                                             */
			 4
	},
};
#else /* USE_MEM_REAL_MACHINE_CYCLE */
/* Number of clock cycles to use for exception processing.
 * Except read/write bus cycles
 */
const int8_t MC68000BASE::m68ki_exception_cycle_table[NUM_CPU_TYPES][M68K_EXCEPTION_CATEGORY_OTHER + 1] =
{
	{ /* 000 */
			(40 -  5*4), /*  0: Reset - Initial Stack Pointer                      */
			 0,			 /*  1: Reset - Initial Program Counter                    */
			(50 - 11*4), /*  2: Bus Error                             (unemulated) */
			(50 - 11*4), /*  3: Address Error                         (unemulated) */
			(34 -  7*4), /*  4: Illegal Instruction                                */
			(38 -  7*4), /*  5: Divide by Zero                                     */
			(40 -  7*4), /*  6: CHK                                                */
			(34 -  7*4), /*  7: TRAPV                                              */
			(34 -  7*4), /*  8: Privilege Violation                                */
			(34 -  7*4), /*  9: Trace                                              */
			(34 -  7*4), /* 10: 1010                                               */
			(34 -  7*4), /* 11: 1111                                               */
			0,			 /* 14: RESERVED (Format Error)                            */
			(44 -  8*4), /* 15: Interrupt                                          */
			(34 -  7*4), /* 32: TRAP #                                             */
			0
	},
	{ /* 070 - not even pretending to be correct */
			(40 -  5*4), /*  0: Reset - Initial Stack Pointer                      */
			0,			 /*  1: Reset - Initial Program Counter                    */
			(126 -30*4), /*  2: Bus Error                             (unemulated) */
			(126 -30*4), /*  3: Address Error                         (unemulated) */
			(38 -  9*4), /*  4: Illegal Instruction                                */
			(44 -  9*4), /*  5: Divide by Zero                                     */
			(44 -  9*4), /*  6: CHK                                                */
			(34 -  9*4), /*  7: TRAPV                                              */
			(38 -  9*4), /*  8: Privilege Violation                                */
			(38 -  8*4), /*  9: Trace                                              */
			(38 -  9*4), /* 10: 1010                                               */
			(38 -  9*4), /* 11: 1111                                               */
			0,			 /* 14: Format Error                                       */
			(46 -  9*4), /* 15: Interrupt                                          */
			(38 -  8*4), /* 32: TRAP #                                             */
			0
	},
	{ /* 010 */
			(40 -  5*4), /*  0: Reset - Initial Stack Pointer                      */
			0,			 /*  1: Reset - Initial Program Counter                    */
			(126 -30*4), /*  2: Bus Error                             (unemulated) */
			(126 -30*4), /*  3: Address Error                         (unemulated) */
			(38 -  9*4), /*  4: Illegal Instruction                                */
			(44 -  9*4), /*  5: Divide by Zero                                     */
			(44 -  9*4), /*  6: CHK                                                */
			(34 -  9*4), /*  7: TRAPV                                              */
			(38 -  9*4), /*  8: Privilege Violation                                */
			(38 -  8*4), /*  9: Trace                                              */
			(38 -  9*4), /* 10: 1010                                               */
			(38 -  9*4), /* 11: 1111                                               */
			0,			 /* 14: Format Error                                       */
			(46 -  9*4), /* 15: Interrupt                                          */
			(38 -  8*4), /* 32: TRAP #                                             */
			0
	},
	{ /* 020 - not correct */
			0, /*  0: Reset - Initial Stack Pointer                      */
			0, /*  1: Reset - Initial Program Counter                    */
			6, /*  2: Bus Error                             (unemulated) */
			6, /*  3: Address Error                         (unemulated) */
			7, /*  4: Illegal Instruction                                */
			0, /*  5: Divide by Zero                                     */
			0, /*  6: CHK                                                */
			4, /*  7: TRAPV                                              */
			4, /*  8: Privilege Violation                                */
			5, /*  9: Trace                                              */
			7, /* 10: 1010                                               */
			7, /* 11: 1111                                               */
			0, /* 14: Format Error                                       */
			8, /* 15: Interrupt                                          */
			4, /* 32: TRAP #                                             */
			0
	},
	{ /* 030 */ // TODO: these values are not correct
			0, /*  0: Reset - Initial Stack Pointer                      */
			0, /*  1: Reset - Initial Program Counter                    */
			0, /*  2: Bus Error                             (unemulated) */
			0, /*  3: Address Error                         (unemulated) */
			0, /*  4: Illegal Instruction                                */
			0, /*  5: Divide by Zero                                     */
			0, /*  6: CHK                                                */
			0, /*  7: TRAPV                                              */
			0, /*  8: Privilege Violation                                */
			0, /*  9: Trace                                              */
			0, /* 10: 1010                                               */
			0, /* 11: 1111                                               */
			0, /* 14: Format Error                                       */
			0, /* 15: Interrupt                                          */
			0, /* 32: TRAP #                                             */
			0
	},
	{ /* 040 */ // TODO: these values are not correct
			0, /*  0: Reset - Initial Stack Pointer                      */
			0, /*  1: Reset - Initial Program Counter                    */
			0, /*  2: Bus Error                             (unemulated) */
			0, /*  3: Address Error                         (unemulated) */
			0, /*  4: Illegal Instruction                                */
			0, /*  5: Divide by Zero                                     */
			0, /*  6: CHK                                                */
			0, /*  7: TRAPV                                              */
			0, /*  8: Privilege Violation                                */
			0, /*  9: Trace                                              */
			0, /* 10: 1010                                               */
			0, /* 11: 1111                                               */
			0, /* 14: Format Error                                       */
			0, /* 15: Interrupt                                          */
			0, /* 32: TRAP #                                             */
			0
	},
	{ /* CPU32 */ // TODO: these values are not correct
			0, /*  0: Reset - Initial Stack Pointer                      */
			0, /*  1: Reset - Initial Program Counter                    */
			0, /*  2: Bus Error                             (unemulated) */
			0, /*  3: Address Error                         (unemulated) */
			0, /*  4: Illegal Instruction                                */
			0, /*  5: Divide by Zero                                     */
			0, /*  6: CHK                                                */
			0, /*  7: TRAPV                                              */
			0, /*  8: Privilege Violation                                */
			0, /*  9: Trace                                              */
			0, /* 10: 1010                                               */
			0, /* 11: 1111                                               */
			0, /* 14: Format Error                                       */
			0, /* 15: Interrupt                                          */
			0, /* 32: TRAP #                                             */
			0
	},
	{ /* ColdFire */ // TODO: these values are not correct
			0, /*  0: Reset - Initial Stack Pointer                      */
			0, /*  1: Reset - Initial Program Counter                    */
			0, /*  2: Bus Error                             (unemulated) */
			0, /*  3: Address Error                         (unemulated) */
			0, /*  4: Illegal Instruction                                */
			0, /*  5: Divide by Zero                                     */
			0, /*  6: CHK                                                */
			0, /*  7: TRAPV                                              */
			0, /*  8: Privilege Violation                                */
			0, /*  9: Trace                                              */
			0, /* 10: 1010                                               */
			0, /* 11: 1111                                               */
			0, /* 14: Format Error                                       */
			0, /* 15: Interrupt                                          */
			0, /* 32: TRAP #                                             */
			0
	},
};
#endif

#ifndef USE_MEM_REAL_MACHINE_CYCLE
const int8_t MC68000BASE::m68ki_ea_idx_cycle_table[64] =
{
	/* 0x00 */
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	/* 0x10 */
		0, /* ..01.000 no memory indirect, base nullptr             */
	5, /* ..01..01 memory indirect,    base nullptr, outer nullptr */
	7, /* ..01..10 memory indirect,    base nullptr, outer 16   */
	7, /* ..01..11 memory indirect,    base nullptr, outer 32   */
	0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,
	/* 0x20 */
	2, /* ..10.000 no memory indirect, base 16               */
	7, /* ..10..01 memory indirect,    base 16,   outer nullptr */
	9, /* ..10..10 memory indirect,    base 16,   outer 16   */
	9, /* ..10..11 memory indirect,    base 16,   outer 32   */
	0,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,
	/* 0x30 */
	6, /* ..11.000 no memory indirect, base 32               */
	11, /* ..11..01 memory indirect,    base 32,   outer nullptr */
	13, /* ..11..10 memory indirect,    base 32,   outer 16   */
	13, /* ..11..11 memory indirect,    base 32,   outer 32   */
	0, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13
};
#else /* USE_MEM_REAL_MACHINE_CYCLE */
const int8_t MC68000BASE::m68ki_ea_idx_cycle_table[64] =
{
	/* 0x00 */
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	/* 0x10 */
	0, /* ..01.000 no memory indirect, base nullptr             */
	1, /* ..01..01 memory indirect,    base nullptr, outer nullptr */
	-1, /* ..01..10 memory indirect,    base nullptr, outer 16   */
	-1, /* ..01..11 memory indirect,    base nullptr, outer 32   */
	0,  1,  -1,  -1,  0,  1,  -1,  -1,  0,  1,  -1,  -1,
	/* 0x20 */
	-2, /* ..10.000 no memory indirect, base 16               */
	-1, /* ..10..01 memory indirect,    base 16,   outer nullptr */
	-3, /* ..10..10 memory indirect,    base 16,   outer 16   */
	-3, /* ..10..11 memory indirect,    base 16,   outer 32   */
	-2,  -1,  -3,  -3,  -2,  -1,  -3,  -3, -2,  -1,  -3,  -3,
	/* 0x30 */
	2, /* ..11.000 no memory indirect, base 32               */
	-1, /* ..11..01 memory indirect,    base 32,   outer nullptr */
	1, /* ..11..10 memory indirect,    base 32,   outer 16   */
	1, /* ..11..11 memory indirect,    base 32,   outer 32   */
	2, -1, 1, 1,  2, -1, 1, 1,  2, -1, 1, 1
};
#endif /* USE_MEM_REAL_MACHINE_CYCLE */
