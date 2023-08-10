/** @file clocale.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01

	@brief [i18n]
*/

#ifndef CLOCALE_H
#define CLOCALE_H

#if defined(USE_WIN)
#include "osd/simple_clocale.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
//#if defined(linux)
//#include "osd/gnu_clocale.h"
//#else
#include "osd/simple_clocale.h"
//#endif
#elif defined(USE_QT)
#include "osd/qt/qt_clocale.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "osd/wxwidgets/wxw_clocale.h"
#endif

#endif /* CLOCALE_H */
