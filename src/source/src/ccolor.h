/** @file ccolor.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#ifndef CCOLOR_H
#define CCOLOR_H

#if defined(USE_WIN)
#include "osd/windows/win_ccolor.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_ccolor.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_ccolor.h"
#elif defined(USE_QT)
#include "osd/qt/qt_ccolor.h"
#endif

#endif /* CCOLOR_H */
