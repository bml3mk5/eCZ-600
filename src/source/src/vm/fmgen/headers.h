/// @file headers.h

#ifndef WIN_HEADERS_H
#define WIN_HEADERS_H

#ifdef _WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "../../depend.h"

//#ifdef _MSC_VER
//	#undef max
//	#define max _MAX
//	#undef min
//	#define min _MIN
//#endif

// disable warning C4996 for microsoft visual c++ 2005
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4996 )
#endif

#endif	// WIN_HEADERS_H
