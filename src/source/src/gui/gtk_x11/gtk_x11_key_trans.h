/** @file gtk_x11_key_trans.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ gtk(x11) to SDL key translate ]
*/

#ifndef GUI_GTK_X11_KEY_TRANS_H
#define GUI_GTK_X11_KEY_TRANS_H

#include "../../common.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
//#include <gtk/gtk.h>

namespace GUI_GTK_X11
{

void X11_InitKeymap(void);

uint32_t X11_TranslateKeycode(Display *display, KeyCode kc);

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_X11_KEY_TRANS_H */

