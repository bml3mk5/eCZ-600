/** @file gtk_keybindbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#include "gtk_keybindbox.h"
#include <gdk/gdkx.h>
#include "gtk_x11_key_trans.h"
#include "../../emu.h"
#include "../../config.h"
#include "../gui_base.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../keycode.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

//
//
//
KeybindBox::KeybindBox(GUI *new_gui)
		: KeybindControlBox(new_gui)
{
}

KeybindBox::~KeybindBox()
{
}

bool KeybindBox::Show(GtkWidget *parent_window)
{
	KeybindControlBox::Show(parent_window);

	if (dialog) return true;

#ifndef USE_GTK
	X11_InitKeymap();
#endif
	create_dialog(window, CMsg::Keybind);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vboxall;
	GtkWidget *hboxall;
	GtkWidget *vbox;
	GtkWidget *btn;

	char label[128];

	GtkWidget *boxall = create_vbox(cont);
	hboxall = create_hbox(boxall);

	// create notebook tab
	notebook = create_notebook(hboxall);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	int tab_offset = KeybindData::KB_TABS_MIN;
	for(int tab_num=tab_offset; tab_num < KeybindData::KB_TABS_MAX; tab_num++) {
		KeybindDataControl *kc = new KeybindDataControl();
		ctrls.Add(kc);

		// add note(tab) to notebook
		vboxall = create_vbox(NULL);
		add_note(notebook, vboxall, LABELS::keybind_tab[tab_num-tab_offset]);

		kc->Create(this, vboxall, tab_num, 240, 320
				, G_CALLBACK(OnKeyDown), G_CALLBACK(OnDoubleClick), G_CALLBACK(OnFocusIn));

		kc->AddCheckBox(vboxall, tab_num);
	}

	vbox = create_vbox(hboxall);
	btn = create_button(vbox, CMsg::Load_Default, G_CALLBACK(OnClickLoadDefaultJ));
	create_label(vbox, "");
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		UTILITY::sprintf(label, sizeof(label), CMSG(Load_Preset_VDIGIT), i+1);
		btn = create_button(vbox, label, G_CALLBACK(OnClickLoadPresetJ));
		g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
	}
	create_label(vbox, "");
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		UTILITY::sprintf(label, sizeof(label), CMSG(Save_Preset_VDIGIT), i+1);
		btn = create_button(vbox, label, G_CALLBACK(OnClickSavePresetJ));
		g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
	}

	ShowAfter(boxall);

	emu->set_pause(1, true);

	return true;
}

void KeybindBox::Hide()
{
	KeybindControlBox::Hide();
	notebook = NULL;
	emu->set_pause(1, false);
}

}; /* namespace GUI_GTK_X11 */


