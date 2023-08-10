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
KeybindControl::KeybindControl() : KeybindData()
{
	ClearAllControl();
}

KeybindControl::~KeybindControl()
{
}

void KeybindControl::ClearAllControl()
{
	grid = NULL;
	memset(cells, 0, sizeof(cells));
	chkCombi = NULL;
}

//
//
//
KeybindBox::KeybindBox(GUI *new_gui) : DialogBox(new_gui)
{
	for(int tab=0; tab < KeybindData::TABS_MAX; tab++) {
		ctrls.Add(new KeybindControl());
	}
	m_timeout_id = 0;
	selected_cell = NULL;
	enable_axes = ~0;
}

KeybindBox::~KeybindBox()
{
	// remove joypad status updater
	if (m_timeout_id) {
		g_source_remove(m_timeout_id);
		m_timeout_id = 0;
	}
}

bool KeybindBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

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
	GtkWidget *scroll;
	GtkWidget *vbox;
	GtkWidget *btn;
	GtkWidget *cell;

	char buf[128];

	GtkWidget *boxall = create_vbox(cont);

	// create notebook tab
	nb = create_notebook(boxall);

	for(int tab=0; tab < ctrls.Count(); tab++) {
		KeybindControl *kc = ctrls[tab];
		kc->Init(emu, tab);

		vboxall = create_vbox(NULL);
		add_note(nb, vboxall, LABELS::keybind_tab[tab]);
		hboxall = create_hbox(vboxall);

		scroll = create_scroll_win(hboxall, 480, 300);
		kc->grid = create_grid(scroll);

		attach_to_grid(kc->grid, create_label(NULL, LABELS::keybind_col[tab][0]), 0, 0);
		for(int col=1; col < 3; col++) {
			sprintf(buf, CMSGV(LABELS::keybind_col[tab][1]), col);
			attach_to_grid(kc->grid, create_label(NULL, buf), col, 0);
		}

		for(int row=0; row < kc->GetNumberOfRows(); row++) {
			for(int col=0; col < 3; col++) {
				if (col == 0) {
					cell = create_label(NULL, kc->GetCellString(row, -1));
				} else {
					cell = create_text(NULL, kc->GetCellString(row, col-1), 3);
					g_object_set_data(G_OBJECT(cell), "tab", (gpointer)(intptr_t)tab);
					g_object_set_data(G_OBJECT(cell), "row", (gpointer)(intptr_t)row);
					g_object_set_data(G_OBJECT(cell), "col", (gpointer)(intptr_t)(col-1));
					if (kc->m_devtype == KeybindData::DEVTYPE_KEYBOARD) {
						g_signal_connect(G_OBJECT(cell), "key-press-event", G_CALLBACK(OnKeyDown), (gpointer)this);
					}
					g_signal_connect(G_OBJECT(cell), "button-press-event", G_CALLBACK(OnDoubleClick), (gpointer)this);
					g_signal_connect(G_OBJECT(cell), "focus-in-event", G_CALLBACK(OnFocusIn), (gpointer)this);
					kc->cells[row][col-1]=cell;
				}
				attach_to_grid(kc->grid, cell, col, row + 1);
			}
		}
		vbox = create_vbox(hboxall);
		btn = create_button(vbox, CMsg::Load_Default, G_CALLBACK(OnClickLoadDefault));
		g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(buf, CMSG(Load_Preset_VDIGIT), i+1);
			btn = create_button(vbox, buf, G_CALLBACK(OnClickLoadPreset));
			g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		create_label(vbox, "");
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(buf, CMSG(Save_Preset_VDIGIT), i+1);
			btn = create_button(vbox, buf, G_CALLBACK(OnClickSavePreset));
			g_object_set_data(G_OBJECT(btn), "tab", (gpointer)(intptr_t)tab);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		if (LABELS::keybind_combi[tab] != CMsg::Null) {
			kc->chkCombi = create_check_box(vboxall, LABELS::keybind_combi[tab], kc->GetCombi() != 0);
		}
	}
	GtkWidget *hbox = create_hbox(boxall);
	create_check_box(hbox, CMsg::Enable_Z_axis, (enable_axes & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0, 2, G_CALLBACK(OnClickAxis));
	create_check_box(hbox, CMsg::Enable_R_axis, (enable_axes & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0, 3, G_CALLBACK(OnClickAxis));
	create_check_box(hbox, CMsg::Enable_U_axis, (enable_axes & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0, 4, G_CALLBACK(OnClickAxis));
	create_check_box(hbox, CMsg::Enable_V_axis, (enable_axes & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0, 5, G_CALLBACK(OnClickAxis));

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));

	// add joypad status updater
	if (!m_timeout_id) {
		m_timeout_id = g_timeout_add(100, OnTimeout, (gpointer)this);
	}

	// unselect
	selected_cell = NULL;

	emu->set_pause(1, true);

	return true;
}

void KeybindBox::Hide()
{
	DialogBox::Hide();
	for(int tab=0; tab < ctrls.Count(); tab++) {
		KeybindControl *kc = ctrls[tab];
		kc->ClearAllControl();
	}
	// remove joypad status updater
	if (m_timeout_id) {
		g_source_remove(m_timeout_id);
		m_timeout_id = 0;
	}
	emu->set_pause(1, false);
}

void KeybindBox::Update()
{
	GtkWidget *entry;
	for(int tab=0; tab < ctrls.Count(); tab++) {
		KeybindControl *kc = ctrls[tab];
		for(int row=0; row < kc->GetNumberOfRows(); row++) {
			for(int col=0; col < 2; col++) {
				entry = kc->cells[row][col];
				set_text(entry, kc->GetCellString(row, col));
			}
		}
		if (kc->chkCombi) {
			set_check_state(kc->chkCombi, kc->GetCombi() != 0);
		}
	}
}

bool KeybindBox::SetData()
{
	for(int tab=0; tab<ctrls.Count(); tab++) {
		KeybindControl *kc = ctrls[tab];
		kc->SetData();
		if (kc->chkCombi) {
			kc->SetCombi(get_check_state(kc->chkCombi) ? 1 : 0);
		}
	}

	emu->save_keybind();

	return true;
}

int KeybindBox::TranslateCode(int code, int scancode)
{
#ifndef USE_GTK
	Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	code = X11_TranslateKeycode(display, scancode);
#endif
	emu->translate_keysym(0,code,(short)scancode,&code);
	return code;
}

bool KeybindBox::ClearKeyCellByCode(int tab, int code, int scancode)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindControl *kc = ctrls[tab];
	code = TranslateCode(code, scancode);
	char label[128];
	int row, col;
	bool rc = kc->ClearCellByVkKeyCode(code, label, &row, &col);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(kc->cells[row][col]), label);
	}
	return rc;
}

bool KeybindBox::SetKeyCell(int tab, int row, int col, int code, int scancode, GtkWidget *widget)
{
	char label[128];
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindControl *kc = ctrls[tab];
	code = TranslateCode(code, scancode);
	bool rc = kc->SetVkKeyCode(row, col, code, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindBox::ClearKeyCell(int tab, int row, int col, GtkWidget *widget)
{
	char label[128];
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindControl *kc = ctrls[tab];
	bool rc = kc->ClearVkKeyCode(row, col, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindBox::SetJoyCell(int tab, int row, int col, uint32_t code, GtkWidget *widget)
{
	char label[128];
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindControl *kc = ctrls[tab];
	bool rc = kc->SetVkJoyCode(row, col, code, code, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindBox::ClearJoyCell(int tab, int row, int col, GtkWidget *widget)
{
	char label[128];
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindControl *kc = ctrls[tab];
	bool rc = kc->ClearVkJoyCode(row, col, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindBox::ClearCell(GtkWidget *widget)
{
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"tab");
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	KeybindData *kc = ctrls[tab];
	bool rc = false;
	if (kc->m_devtype == KeybindData::DEVTYPE_KEYBOARD) {
		rc = ClearKeyCell(tab, row, col, widget);
	} else {
		rc = ClearJoyCell(tab, row, col, widget);
	}
	return rc;
}

void KeybindBox::UpdateJoy()
{
	emu->update_joystick();

	if (!selected_cell) return;

	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(selected_cell),"tab");
	KeybindData *kc = ctrls[tab];
	if (kc->m_devtype != KeybindData::DEVTYPE_JOYPAD) return;

	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(selected_cell),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(selected_cell),"col");

	uint32_t *joy_stat = emu->joy_real_buffer(col);

	if (joy_stat[0] & enable_axes) {
		SetJoyCell(tab, row, col, joy_stat[0] & enable_axes, selected_cell);
	}
}

void KeybindBox::LoadPreset(int tab, int idx)
{
	if (tab < 0 || tab >= ctrls.Count()) return;
	KeybindControl *kc = ctrls[tab];
	kc->LoadPreset(idx);
	Update();
}

void KeybindBox::SavePreset(int tab, int idx)
{
	if (tab < 0 || tab >= ctrls.Count()) return;
	KeybindControl *kc = ctrls[tab];
	kc->SavePreset(idx);
}

void KeybindBox::ToggleAxis(GtkWidget *widget)
{
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	bool chkd = get_check_state(widget);
	uint32_t bits = 0;
	switch(num) {
	case 2:
		bits = (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT);
		break;
	case 3:
		bits = (JOYCODE_R_UP | JOYCODE_R_DOWN);
		break;
	case 4:
		bits = (JOYCODE_U_LEFT | JOYCODE_U_RIGHT);
		break;
	case 5:
		bits = (JOYCODE_V_UP | JOYCODE_V_DOWN);
		break;
	}

	BIT_ONOFF(enable_axes, bits, chkd);
}

gboolean KeybindBox::OnKeyDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (event->type != GDK_KEY_PRESS) return FALSE;
	GdkEventKey *key_event = (GdkEventKey *)event;
	int code = (int)key_event->keyval;
	int scancode =  key_event->hardware_keycode;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"tab");
	obj->ClearKeyCellByCode(tab, code, scancode);
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	obj->SetKeyCell(tab, row, col, code, scancode, widget);
	return TRUE;
}

gboolean KeybindBox::OnDoubleClick(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (event->type != GDK_2BUTTON_PRESS) return FALSE;
	obj->ClearCell(widget);
	return TRUE;
}

gboolean KeybindBox::OnFocusIn(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (event->type != GDK_FOCUS_CHANGE) return FALSE;
	obj->SetSelectedCell(widget);
	return TRUE;
}

void KeybindBox::OnClickLoadDefault(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	obj->LoadPreset(tab, -1);
}

void KeybindBox::OnClickLoadPreset(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->LoadPreset(tab, num);
}

void KeybindBox::OnClickSavePreset(GtkButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	int tab = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"tab");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->SavePreset(tab, num);
}

void KeybindBox::OnClickAxis(GtkToggleButton *button, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	obj->ToggleAxis((GtkWidget *)button);
}

void KeybindBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

gboolean KeybindBox::OnTimeout(gpointer user_data)
{
	KeybindBox *obj = (KeybindBox *)user_data;
	obj->UpdateJoy();
	return TRUE;
}

}; /* namespace GUI_GTK_X11 */


