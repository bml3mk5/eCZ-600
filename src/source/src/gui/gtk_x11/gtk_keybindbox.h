/** @file gtk_keybindbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#ifndef GUI_GTK_KEYBINDBOX_H
#define GUI_GTK_KEYBINDBOX_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../gui_keybinddata.h"
#include "../../vm/vm.h"
#include "../../cptrlist.h"
#include "gtk_keybindctrl.h"

namespace GUI_GTK_X11
{

/**
	@brief Keybind dialog box
*/
class KeybindBox : public KeybindControlBox
{
public:
	KeybindBox(GUI *new_gui);
	~KeybindBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_KEYBINDBOX_H */
