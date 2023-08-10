/** @file gui.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.01

	@brief [ gui ]
*/

#ifndef GUI_H
#define GUI_H

#if defined(GUI_TYPE_AGAR)

#include "agar/ag_gui.h"

#elif defined(GUI_TYPE_WINDOWS)

#ifndef USE_WIN
#define NOT_USE_GETTEXT_ACP 1
#endif
#include "windows/win_gui.h"

#elif defined(GUI_TYPE_COCOA)

#include "cocoa/cocoa_gui.h"

#elif defined(GUI_TYPE_GTK_X11)

#include "gtk_x11/gtk_x11_gui.h"

#elif defined(GUI_TYPE_QT)

#include "qt/qt_gui.h"

#elif defined(GUI_TYPE_WXWIDGETS)

#include "wxwidgets/wx_gui.h"

#else

#include "gui_base.h"

class GUI : public GUI_BASE
{
public:
	GUI(int argc, char **argv, EMU *new_emu) : GUI_BASE(argc, argv, new_emu) {}
};

#endif

#endif /* GUI_H */
