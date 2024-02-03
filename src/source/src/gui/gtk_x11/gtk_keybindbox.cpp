/** @file gtk_keybindbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#include <gdk/gdkx.h>
#include "gtk_keybindbox.h"
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
	GtkWidget *nb;
	GtkWidget *vboxall;
	GtkWidget *hboxall;
	GtkWidget *vbox;
	GtkWidget *btn;

	char label[128];

	GtkWidget *boxall = create_vbox(cont);

	// create notebook tab
	nb = create_notebook(boxall);

	for(int tab_num=0; tab_num < KeybindData::TABS_MAX; tab_num++) {
		KeybindDataControl *kc = new KeybindDataControl();
		ctrls.Add(kc);

		// add note(tab) to notebook
		vboxall = create_vbox(NULL);
		add_note(nb, vboxall, LABELS::keybind_tab[tab_num]);
		hboxall = create_hbox(vboxall);

		kc->Create(this, hboxall, tab_num, 480, 300
				, G_CALLBACK(OnKeyDown), G_CALLBACK(OnDoubleClick), G_CALLBACK(OnFocusIn));

		vbox = create_vbox(hboxall);
		btn = create_button(vbox, CMsg::Load_Default, G_CALLBACK(OnClickLoadDefault));
		g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Load_Preset_VDIGIT), i+1);
			btn = create_button(vbox, label, G_CALLBACK(OnClickLoadPreset));
			g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Save_Preset_VDIGIT), i+1);
			btn = create_button(vbox, label, G_CALLBACK(OnClickSavePreset));
			g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
			kc->chkCombi = create_check_box(vboxall, LABELS::keybind_combi[tab_num], kc->GetCombi() != 0);
		}
	}

	ShowAfter(boxall);

	emu->set_pause(1, true);

	return true;
}

void KeybindBox::Hide()
{
	KeybindControlBox::Hide();

	emu->set_pause(1, false);
}

}; /* namespace GUI_GTK_X11 */


