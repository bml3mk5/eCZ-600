/// @file paw_defs.h
///
/// @author Sasaji
/// @date   2019.08.01
///


#ifndef _PARSEWAV_DEFS_H_
#define _PARSEWAV_DEFS_H_

#include "../common.h"


namespace PARSEWAV 
{

//#define TERMINATOR_SIZE		4
//#define DATA_BUFFER_SIZE	(256 + TERMINATOR_SIZE)

enum enum_baud_rate_idx {
	IDX_PTN_600 =	0,
	IDX_PTN_1200 =	1,
	IDX_PTN_2400 =	2,
	IDX_PTN_300 =	3,
};

struct st_pattern {
	const uint8_t *ptn;
	int            len;
};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_DEFS_H_ */
