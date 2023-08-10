/** @file main.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ main ]
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#if defined(USE_WIN)
#include "osd/windows/win_main.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_main.h"
#elif defined(USE_QT)
#include "osd/qt/qt_main.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_main.h"
#endif

#endif
