/** @file vm_defs.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5 by Sasaji at 2011.06.17,
	Modified for MBS1 by Sasaji at 2015.09.01

	@brief [ common header ]
*/

#ifndef _VM_DEFS_H_
#define _VM_DEFS_H_

// GIJUTSU-HYORON-SHA Babbase-2nd
#ifdef _BABBAGE2ND
#include "babbage2nd/babbage2nd_defs.h"
#endif

// Nintendo Family BASIC
#ifdef _FAMILYBASIC
#include "familybasic/familybasic_defs.h"
#endif

// FUJITSU FM16pi
#ifdef _FM16PI
#include "fm16pi/fm16pi_defs.h"
#endif

// FUJITSU FMR-30
#ifdef _FMR30
#include "fmr30/fmr30_defs.h"
#endif

// FUJITSU FMR-50
#ifdef _FMR50
#include "fmr50/fmr50_defs.h"
#endif

// FUJITSU FMR-60
#ifdef _FMR60
#include "fmr50/fmr50_defs.h"
#endif

// FUJITSU FMR-CARD
#ifdef _FMRCARD
#include "fmr50/fmr50_defs.h"
#endif

// CASIO FP-1100
#ifdef _FP1100
#include "fp1100/fp1100_defs.h"
#endif

// EPSON HC-20
#ifdef _HC20
#include "hc20/hc20_defs.h"
#endif

// EPSON HC-40
#ifdef _HC40
#include "hc40/hc40_defs.h"
#endif

// EPSON HC-80
#ifdef _HC80
#include "hc80/hc80_defs.h"
#endif

// TOSHIBA J-3100GT
#ifdef _J3100SL
#include "j3100/j3100_defs.h"
#endif

// TOSHIBA J-3100SL
#ifdef _J3100SL
#include "j3100/j3100_defs.h"
#endif

// IBM Japan Ltd PC/JX
#ifdef _JX
#include "jx/jx_defs.h"
#endif

// SORD m5
#ifdef _M5
#include "m5/m5_defs.h"
#endif

// SEIKO MAP-1010
#ifdef _MAP1010
#include "phc25/phc25_defs.h"
#endif

// MITSUBISHI Elec. MULTI8
#ifdef _MULTI8
#include "multi8/multi8_defs.h"
#endif

// Japan Electronics College MYCOMZ-80A
#ifdef _MYCOMZ80A
#include "mycomz80a/mycomz80a_defs.h"
#endif

// SHARP MZ-80K
#ifdef _MZ80K
#include "mz80k/mz80k_defs.h"
#endif

// SHARP MZ-700
#ifdef _MZ700
#include "mz700/mz700_defs.h"
#endif

// SHARP MZ-800
#ifdef _MZ800
#include "mz700/mz700_defs.h"
#endif

// SHARP MZ-1200
#ifdef _MZ1200
#include "mz80k/mz80k_defs.h"
#endif

// SHARP MZ-1500
#ifdef _MZ1500
#include "mz700/mz700_defs.h"
#endif

// SHARP MZ-2500
#ifdef _MZ2500
#include "mz2500/mz2500_defs.h"
#endif

// SHARP MZ-2800
#ifdef _MZ2800
#include "mz2800/mz2800_defs.h"
#endif

// SHARP MZ-3500
#ifdef _MZ3500
#include "mz3500/mz3500_defs.h"
#endif

// SHARP MZ-5500
#ifdef _MZ5500
#include "mz5500/mz5500_defs.h"
#endif

// SHARP MZ-6500
#ifdef _MZ6500
#include "mz5500/mz5500_defs.h"
#endif

// SHARP MZ-6550
#ifdef _MZ6550
#include "mz5500/mz5500_defs.h"
#endif

// NEC N5200
#ifdef _N5200
#include "n5200/n5200_defs.h"
#endif

// TOSHIBA PASOPIA
#ifdef _PASOPIA
#include "pasopia/pasopia_defs.h"
#endif

// TOSHIBA PASOPIA 7
#ifdef _PASOPIA7
#include "pasopia7/pasopia7_defs.h"
#endif

// NEC PC-8201
#ifdef _PC8201
#include "pc8201/pc8201_defs.h"
#endif

// NEC PC-8201A
#ifdef _PC8201A
#include "pc8201/pc8201_defs.h"
#endif

// NEC PC-8801MA
#ifdef _PC8801MA
#include "pc9801/pc8801_defs.h"
#endif

// NEC PC-9801
#ifdef _PC9801
#include "pc9801/pc9801_defs.h"
#endif

// NEC PC-9801E/F/M
#ifdef _PC9801E
#include "pc9801/pc9801_defs.h"
#endif

// NEC PC-9801VM
#ifdef _PC9801VM
#include "pc9801/pc9801_defs.h"
#endif

// NEC PC-98DO
#ifdef _PC98DO
#include "pc9801/pc9801_defs.h"
#endif

// NEC PC-98HA
#ifdef _PC98HA
#include "pc98ha/pc98ha_defs.h"
#endif

// NEC PC-98LT
#ifdef _PC98LT
#include "pc98ha/pc98ha_defs.h"
#endif

// NEC PC-100
#ifdef _PC100
#include "pc100/pc100_defs.h"
#endif

// SANYO PHC-20
#ifdef _PHC20
#include "phc20/phc20_defs.h"
#endif

// SANYO PHC-25
#ifdef _PHC25
#include "phc25/phc25_defs.h"
#endif

// CASIO PV-1000
#ifdef _PV1000
#include "pv1000/pv1000_defs.h"
#endif

// CASIO PV-2000
#ifdef _PV2000
#include "pv2000/pv2000_defs.h"
#endif

// TOMY PYUTA
#ifdef _PYUTA
#include "pyuta/pyuta_defs.h"
#endif

// EPSON QC-10
#ifdef _QC10
#include "qc10/qc10_defs.h"
#endif

// BANDAI RX-78
#ifdef _RX78
#include "rx78/rx78_defs.h"
#endif

// SEGA SC-3000
#ifdef _SC3000
#include "sc3000/sc3000_defs.h"
#endif

// EPOCH Super Cassette Vision
#ifdef _SCV
#include "scv/scv_defs.h"
#endif

// NEC TK-80BS (COMPO BS/80)
#ifdef _TK80BS
#include "tk80bs/tk80bs_defs.h"
#endif

// CANON X-07
#ifdef _X07
#include "x07/x07_defs.h"
#endif

// SHARP X1turbo
#ifdef _X1TURBO
#include "x1/x1_defs.h"
#endif

// SHARP X1twin
#ifdef _X1TWIN
#include "x1/x1_defs.h"
#endif

// SHINKO SANGYO YS-6464A
#ifdef _YS6464A
#include "ys6464a/ys6464a_defs.h"
#endif

// HITACHI Basic Master Level3 Mark5
#ifdef _BML3MK5
#include "bml3mk5/bml3mk5_defs.h"
#endif

// HITACHI MB-S1
#ifdef _MBS1
#include "mbs1/mbs1_defs.h"
#endif

// MC68000_MINI
#ifdef _MC68000_MINI
#include "mc68000_mini/mc68000_mini_defs.h"
#endif

// SHARP X68000
#ifdef _X68000
#include "x68000/x68000_defs.h"
#endif

#endif /* _VM_DEFS_H_ */
