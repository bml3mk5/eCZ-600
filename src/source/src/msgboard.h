/** @file msgboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ message board ]
*/

#ifndef MSGBOARD_H
#define MSGBOARD_H

#if defined(USE_WIN)
#include "osd/windows/win_msgboard.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_msgboard.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_msgboard.h"
#elif defined(USE_QT)
#include "osd/qt/qt_msgboard.h"
#endif

#endif /* MSGBOARD_H */
