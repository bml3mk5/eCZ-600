/** @file utils.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ utils ]
*/

#ifndef UTILS_H
#define UTILS_H

#if defined(USE_WIN)

#elif defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX) || defined(USE_WX2)

#elif defined(USE_QT)
#include "osd/qt/qt_utils.h"
#endif

#endif /* UTILS_H */
