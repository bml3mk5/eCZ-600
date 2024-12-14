/** @file gtk_keybindctrl.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.01.07 -

	@brief [ keybind control ]
*/

#include "gtk_keybindctrl.h"
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
KeybindDataControl::KeybindDataControl() : KeybindData(), Control()
{
	ClearAllControl();
}

KeybindDataControl::~KeybindDataControl()
{
}

void KeybindDataControl::ClearAllControl()
{
	grid = NULL;
	memset(cells, 0, sizeof(cells));
	chkCombi = NULL;
}

void KeybindDataControl::Create(Control *parent, GtkWidget* box, int tab_num, int width, int height
		,GCallback on_key_down, GCallback on_double_click, GCallback on_focus_in)
{
	char label[128];

	Init(emu, tab_num);

	GtkWidget *scroll = create_scroll_win(box, width, height);
	grid = create_grid(scroll);

	attach_to_grid(grid, create_label(NULL, LABELS::keybind_col[tab_num][0]), 0, 0);
	for(int col=1; col < 3; col++) {
		UTILITY::sprintf(label, sizeof(label), CMSGV(LABELS::keybind_col[tab_num][1]), col);
		attach_to_grid(grid, create_label(NULL, label), col, 0);
	}

	GtkWidget *cell;
	for(int row=0; row < GetNumberOfRows(); row++) {
		for(int col=0; col < 3; col++) {
			if (col == 0) {
				cell = create_label(NULL, GetCellString(row, -1));
			} else {
				cell = create_text(NULL, GetCellString(row, col-1), 3);
				g_object_set_data(G_OBJECT(cell), "ctrl", (gpointer)this);
				g_object_set_data(G_OBJECT(cell), "row", (gpointer)(intptr_t)row);
				g_object_set_data(G_OBJECT(cell), "col", (gpointer)(intptr_t)(col-1));
				if (m_devtype == KeybindData::DEVTYPE_KEYBOARD) {
					g_signal_connect(G_OBJECT(cell), "key-press-event", on_key_down, (gpointer)parent);
				}
				g_signal_connect(G_OBJECT(cell), "button-press-event", on_double_click, (gpointer)parent);
				g_signal_connect(G_OBJECT(cell), "focus-in-event", on_focus_in, (gpointer)parent);
				cells[row][col-1]=cell;
			}
			attach_to_grid(grid, cell, col, row + 1);
		}
	}
}

void KeybindDataControl::Update()
{
	GtkWidget *entry;
	for(int row=0; row < GetNumberOfRows(); row++) {
		for(int col=0; col < 2; col++) {
			entry = cells[row][col];
			set_text(entry, GetCellString(row, col));
		}
	}
	if (chkCombi) {
		set_check_state(chkCombi, GetCombi() != 0);
	}
}

GtkWidget *KeybindDataControl::AddCheckBox(GtkWidget *box, int tab_num)
{
	if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
		chkCombi = create_check_box(box, LABELS::keybind_combi[tab_num], GetCombi() != 0);
	}
	return chkCombi;
}

/// override
void KeybindDataControl::SetData()
{
	KeybindData::SetData();
	if (chkCombi) {
		SetCombi(get_check_state(chkCombi) ? 1 : 0);
	}
}

int KeybindDataControl::TranslateCode(int code, int scancode)
{
#ifndef USE_GTK
	Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	code = X11_TranslateKeycode(display, scancode);
#endif
	emu->translate_keysym(0,code,(short)scancode,&code);
	return code;
}

bool KeybindDataControl::ClearKeyCellByCode(int code, int scancode)
{
	if (m_flags == FLAG_DENY_DUPLICATE) {
		code = TranslateCode(code, scancode);
		char label[128];
		int row, col;
		bool rc = ClearCellByVkKeyCode(code, label, &row, &col);
		if (rc && row >= 0 && col >= 0) {
			gtk_entry_set_text(GTK_ENTRY(cells[row][col]), label);
		}
		return rc;
	} else {
		return true;
	}
}

bool KeybindDataControl::SetKeyCell(int row, int col, int code, int scancode, GtkWidget *widget)
{
	char label[128];
	code = TranslateCode(code, scancode);
	bool rc = SetVkKeyCode(row, col, code, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindDataControl::ClearKeyCell(int row, int col, GtkWidget *widget)
{
	char label[128];
	bool rc = ClearVkKeyCode(row, col, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindDataControl::SetJoyCell(int row, int col, uint32_t code, GtkWidget *widget)
{
	char label[128];
	bool rc = SetVkJoyCode(row, col, code, code, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindDataControl::ClearJoyCell(int row, int col, GtkWidget *widget)
{
	char label[128];
	bool rc = ClearVkJoyCode(row, col, label);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(widget), label);
	}
	return rc;
}

bool KeybindDataControl::ClearCell(int row, int col, GtkWidget *widget)
{
	bool rc = false;
	if (m_devtype == KeybindData::DEVTYPE_KEYBOARD) {
		rc = ClearKeyCell(row, col, widget);
	} else {
		rc = ClearJoyCell(row, col, widget);
	}
	return rc;
}

//

KeybindControlBox::KeybindControlBox(GUI *new_gui)
	: DialogBox(new_gui)
{
	m_timeout_id = 0;
	notebook = NULL;
	selected_cell = NULL;
	enable_axes = ~0;
}

KeybindControlBox::~KeybindControlBox()
{
	// remove joypad status updater
	if (m_timeout_id) {
		g_source_remove(m_timeout_id);
		m_timeout_id = 0;
	}
}

bool KeybindControlBox::Show(GtkWidget *parent_window)
{
	return DialogBox::Show(parent_window);
}

void KeybindControlBox::ShowAfter(GtkWidget *boxall)
{
	if (!dialog) return;

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
}

void KeybindControlBox::Hide()
{
	DialogBox::Hide();
	ctrls.Clear();

	// remove joypad status updater
	if (m_timeout_id) {
		g_source_remove(m_timeout_id);
		m_timeout_id = 0;
	}
}

void KeybindControlBox::Update()
{
	for(int tab=0; tab < ctrls.Count(); tab++) {
		KeybindDataControl *kc = ctrls[tab];
		kc->Update();
	}
}

bool KeybindControlBox::SetData()
{
	for(int tab=0; tab<ctrls.Count(); tab++) {
		KeybindDataControl *kc = ctrls[tab];
		kc->SetData();
	}

	emu->save_keybind();

	return true;
}

#if 0
bool KeybindControlBox::ClearKeyCellByCode(int tab, int code, int scancode)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindDataControl *kc = ctrls[tab];
	code = KeybindDataControl::TranslateCode(code, scancode);
	char label[128];
	int row, col;
	bool rc = kc->ClearCellByVkKeyCode(code, label, &row, &col);
	if (rc) {
		gtk_entry_set_text(GTK_ENTRY(kc->cells[row][col]), label);
	}
	return rc;
}

bool KeybindControlBox::SetKeyCell(int tab, int row, int col, int code, int scancode, GtkWidget *widget)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindDataControl *kc = ctrls[tab];
	if (!kc) return false;
	return kc->SetKeyCell(row, col, code, scancode, widget);
}

bool KeybindControlBox::ClearKeyCell(int tab, int row, int col, GtkWidget *widget)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindDataControl *kc = ctrls[tab];
	if (!kc) return false;
	return kc->ClearKeyCell(row, col, widget);
}

bool KeybindControlBox::SetJoyCell(int tab, int row, int col, uint32_t code, GtkWidget *widget)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindDataControl *kc = ctrls[tab];
	if (!kc) return false;
	return kc->SetJoyCell(row, col, code, widget);
}

bool KeybindControlBox::ClearJoyCell(int tab, int row, int col, GtkWidget *widget)
{
	if (tab < 0 || tab >= ctrls.Count()) return false;
	KeybindDataControl *kc = ctrls[tab];
	if (!kc) return false;
	return kc->ClearJoyCell(row, col, widget);
}
#endif

bool KeybindControlBox::ClearCell(GtkWidget *widget)
{
	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(widget),"ctrl");
	if (!kc) return false;
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	return kc->ClearCell(row, col, widget);
}

void KeybindControlBox::UpdateJoy()
{
	emu->update_joystick();

	if (!selected_cell) return;

	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(selected_cell),"ctrl");
	if (!kc) return;
	if (kc->m_devtype != KeybindData::DEVTYPE_JOYPAD) return;

	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(selected_cell),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(selected_cell),"col");

	uint32_t *joy_stat = emu->joy_real_buffer(col);

	if (joy_stat[0] & enable_axes) {
		kc->SetJoyCell(row, col, joy_stat[0] & enable_axes, selected_cell);
	}
}

void KeybindControlBox::LoadDefault()
{
	int tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if (tab < 0 || tab >= ctrls.Count()) return;
	KeybindDataControl *kc = ctrls[tab];
	kc->LoadPreset(-1);
	kc->Update();
}

void KeybindControlBox::LoadPreset(int idx)
{
	int tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if (tab < 0 || tab >= ctrls.Count()) return;
	KeybindDataControl *kc = ctrls[tab];
	kc->LoadPreset(idx);
	kc->Update();
}

void KeybindControlBox::SavePreset(int idx)
{
	int tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	if (tab < 0 || tab >= ctrls.Count()) return;
	KeybindDataControl *kc = ctrls[tab];
	kc->SavePreset(idx);
}

void KeybindControlBox::ToggleAxis(GtkWidget *widget)
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

gboolean KeybindControlBox::OnKeyDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	if (event->type != GDK_KEY_PRESS) return FALSE;

//	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(widget),"ctrl");
	if (!kc) return FALSE;
	int row = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"row");
	int col = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"col");
	kc->ClearCell(row, col, widget);
	GdkEventKey *key_event = (GdkEventKey *)event;
	int code = (int)key_event->keyval;
	int scancode =  key_event->hardware_keycode;
	kc->ClearKeyCellByCode(code, scancode);
	kc->SetKeyCell(row, col, code, scancode, widget);
	return TRUE;
}

gboolean KeybindControlBox::OnDoubleClick(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (event->type != GDK_2BUTTON_PRESS) return FALSE;
	obj->ClearCell(widget);
	return TRUE;
}

gboolean KeybindControlBox::OnFocusIn(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (event->type != GDK_FOCUS_CHANGE) return FALSE;
	obj->SetSelectedCell(widget);
	return TRUE;
}

void KeybindControlBox::OnClickLoadDefault(GtkButton *button, gpointer user_data)
{
//	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(button),"ctrl");
	if (kc) kc->LoadPreset(-1);
}

void KeybindControlBox::OnClickLoadPreset(GtkButton *button, gpointer user_data)
{
//	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(button),"ctrl");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	if (kc) kc->LoadPreset(num);
}

void KeybindControlBox::OnClickSavePreset(GtkButton *button, gpointer user_data)
{
//	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	KeybindDataControl *kc = (KeybindDataControl *)g_object_get_data(G_OBJECT(button),"ctrl");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	if (kc) kc->SavePreset(num);
}

void KeybindControlBox::OnClickLoadDefaultJ(GtkButton *button, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (!obj) return;
	obj->LoadDefault();
}

void KeybindControlBox::OnClickLoadPresetJ(GtkButton *button, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (!obj) return;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->LoadPreset(num);
}

void KeybindControlBox::OnClickSavePresetJ(GtkButton *button, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (!obj) return;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(button),"num");
	obj->SavePreset(num);
}

void KeybindControlBox::OnClickAxis(GtkToggleButton *button, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	obj->ToggleAxis((GtkWidget *)button);
}

void KeybindControlBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

gboolean KeybindControlBox::OnTimeout(gpointer user_data)
{
	KeybindControlBox *obj = (KeybindControlBox *)user_data;
	obj->UpdateJoy();
	return TRUE;
}

}; /* namespace GUI_GTK_X11 */


