/** @file emu_osd.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ emulation i/f ]

*/

#ifndef EMU_OSD_H
#define EMU_OSD_H

#if defined(USE_WIN)
#include "osd/windows/win_emu.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "osd/SDL/sdl_emu.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_emu.h"
#elif defined(USE_QT)
#include "osd/qt/qt_emu.h"
#endif

#endif /* EMU_OSD_H */
