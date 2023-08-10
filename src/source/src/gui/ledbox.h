/** @file ledbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ led box ]
*/

#ifndef LEDBOX_H
#define LEDBOX_H

#include "../common.h"

#if defined(USE_WIN)
#include "../osd/windows/win_ledboxbase.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "../osd/SDL/sdl_ledboxbase.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "../osd/wxwidgets/wxw_ledboxbase.h"
#elif defined(USE_QT)
#include "../osd/qt/qt_ledboxbase.h"
#endif

#endif /* LEDBOX_H */

