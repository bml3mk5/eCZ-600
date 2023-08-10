/** @file csurface.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ csurface ]
*/

#ifndef CSURFACE_H
#define CSURFACE_H

#if defined(USE_WIN)
#include "osd/windows/win_csurface.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_csurface.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_csurface.h"
#elif defined(USE_QT)
#include "osd/qt/qt_csurface.h"
#endif

#endif /* CSURFACE_H */
