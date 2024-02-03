/** @file gtk_joysetbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.07 -

	@brief [ joypad setting box ]
*/

#ifndef GUI_GTK_JOYSETBOX_H
#define GUI_GTK_JOYSETBOX_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../vm/vm_defs.h"
#include "../../emu.h"
#include "gtk_keybindctrl.h"

namespace GUI_GTK_X11
{

/**
	@brief Joypad setting dialog box
*/
class JoySettingBox : public KeybindControlBox
{
private:
	GtkWidget *com[MAX_JOYSTICKS];
	GtkWidget *scale[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	GtkWidget *axis[MAX_JOYSTICKS][6];

	bool SetData();

public:
	JoySettingBox(GUI *new_gui);
	~JoySettingBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_JOYSETBOX_H */
