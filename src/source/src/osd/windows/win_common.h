/** @file win_common.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ common header ]
*/

#ifndef WIN_COMMON_H
#define WIN_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
#include "inttypes.h"
#else
#include <stdint.h>
#endif

#include <string.h>

#include <tchar.h>


#if defined(_MSC_VER)
#include <stdlib.h>

// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

// check memory leaks
#if defined(_DEBUG)
//#define _CRTDBG_MAP_ALLOC
#undef _malloca
#include <crtdbg.h>
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#if _MSC_VER > 1500
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

#endif /* _MSC_VER */

/* WIN API */
#define CDelay(ms) Sleep(ms)

#endif /* WIN_COMMON_H */
