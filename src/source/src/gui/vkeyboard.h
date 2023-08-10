/** @file vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ virtual keyboard ]
*/

#ifndef VKEYBOARD_H
#define VKEYBOARD_H

#include "../common.h"

/// @namespace Vkbd
/// @brief Virtual Keyboard Modules

#if defined(USE_WIN)
#include "../osd/windows/win_vkeyboardbase.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "../osd/SDL/sdl_vkeyboardbase.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "../osd/wxwidgets/wxw_vkeyboardbase.h"
#elif defined(USE_QT)
#include "../osd/qt/qt_vkeyboardbase.h"
#endif

#endif /* VKEYBOARD_H */

