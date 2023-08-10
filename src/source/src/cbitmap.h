/** @file cbitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef CBITMAP_H
#define CBITMAP_H

#include "common.h"

#if defined(USE_WIN)
#include "osd/windows/win_cbitmap.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_cbitmap.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_cbitmap.h"
#elif defined(USE_QT)
#include "osd/qt/qt_cbitmap.h"
#endif

#endif /* CBITMAP_H */
