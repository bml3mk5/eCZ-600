/** @file floppy_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ floppy drive definition ]
*/

#ifndef FLOPPY_DEFS_H
#define FLOPPY_DEFS_H

#include "../vm_defs.h"

/// @ingroup Enums
/// @brief signals FDC, FLOPPY, FDD and DISK
enum SIG_FLOPPY_IDS {
#ifdef USE_SIG_FLOPPY_ACCESS
	SIG_FLOPPY_ACCESS				= 0,
#endif
	SIG_FLOPPY_READ_ID				= 1,	///< read id (FDC to DISK)
//	SIG_FLOPPY_READ_ID_TRACK_NUM	= 2,
//	SIG_FLOPPY_READ_ID_HEAD_NUM		= 3,
	SIG_FLOPPY_HEAD_SELECT			= 5,	///< head select (FDC to DISK)
	SIG_FLOPPY_READ					= 6,	///< read sector (FDC to DISK)
	SIG_FLOPPY_READ_TRACK			= 7,	///< read track (FDC to DISK)
	SIG_FLOPPY_WRITE				= 8,	///< write sector (FDC to DISK)
	SIG_FLOPPY_WRITE_TRACK			= 9,	///< write track (FDC to DISK)
	SIG_FLOPPY_WRITEDELETE			= 10,	///< write sector with deleted mark (FDC to DISK)
	SIG_FLOPPY_WRITEPROTECT			= 11,	///< write protect (DISK to FDC)
	SIG_FLOPPY_STEP					= 12,	///< step and dirc (FDC to DISK)
	SIG_FLOPPY_HEADLOAD				= 13,	///< head load (DISK to FDC)
	SIG_FLOPPY_READY				= 14,	///< ready (DISK to FDC)
	SIG_FLOPPY_TRACK0				= 15,	///< track 0 (DISK to FDC)
	SIG_FLOPPY_INDEX				= 16,	///< index hole (DISK to FDC)
	SIG_FLOPPY_DELETED				= 17,	///< deleted mark (DISK to FDC)

#ifdef SET_CURRENT_TRACK_IMMEDIATELY
	SIG_FLOPPY_CURRENTTRACK			= 18,
#endif
	SIG_FLOPPY_SECTOR_NUM			= 19,	///< sector number
	SIG_FLOPPY_SECTOR_SIZE			= 20,	///< sector size
	SIG_FLOPPY_TRACK_SIZE			= 21,	///< track size
	SIG_FLOPPY_TRACK_REMAIN_SIZE	= 22,	///< track remain size

	SIG_FLOPPY_IRQ					= 26,	///< IRQ from FDC
	SIG_FLOPPY_DRQ					= 27,	///< DRQ from FDC

	SIG_FLOPPY_DENSITY				= 29,	///< density to FDC
	SIG_FLOPPY_FORCE_READY			= 30	///< always ready
};

#endif /* FLOPPY_DEFS_H */

