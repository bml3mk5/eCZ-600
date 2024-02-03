/** @file gtk_joysetbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.07 -

	@brief [ joypad setting box ]
*/

#include "gtk_joysetbox.h"
#include "../../emumsg.h"
#include "../../config.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../gui_keybinddata.h"
#include "../../utility.h"
#include "../../keycode.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

JoySettingBox::JoySettingBox(GUI *new_gui)
		: KeybindControlBox(new_gui)
{
}

JoySettingBox::~JoySettingBox()
{
}

bool JoySettingBox::Show(GtkWidget *parent_window)
{
	KeybindControlBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Joypad_Setting);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *nb;
	GtkWidget *vboxall;
//	GtkWidget *vboxbox;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *btn;
	GtkWidget *lbl;

	char label[128];

	GtkWidget *boxall = create_vbox(cont);
	GtkWidget *hboxall = create_hbox(boxall);

	int tx = 80;
	int sx = 160;
	int sy = 16;
	int cs = 0;

	//

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		vbox = create_vbox(hboxall, cs);

		UTILITY::sprintf(label, sizeof(label), CMSG(JoypadVDIGIT), i + 1);
		create_label(vbox, label);
		int val;
		hbox = create_hbox(vbox, cs);
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		com[i] = create_combo_box(hbox, "", LABELS::joypad_type, val);
#endif
		hbox = create_hbox(vbox, cs);
		create_label(hbox, CMsg::Button_Mashing_Speed);
		lbl = create_label(hbox, " ");
		gtk_widget_set_size_request(lbl, 32, sy);
		create_label(hbox, "0 <-> 3");

		GtkWidget *scroll = create_scroll_win(vbox, 240, 280);
		GtkWidget *svbox = create_vbox(scroll, cs);
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				scale[i][k] = NULL;
				continue;
			}
			
			hbox = create_hbox(svbox, cs);
			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			lbl = create_label(hbox, id);
			gtk_widget_set_size_request(lbl, tx, sy);
			val = pConfig->joy_mashing[i][k];
			int n = i * KEYBIND_JOY_BUTTONS + k;
			scale[i][k] = create_scale_box(hbox, 0, 3, val, false, n);
			gtk_scale_set_value_pos(GTK_SCALE(scale[i][k]), GTK_POS_RIGHT);
			gtk_widget_set_size_request(scale[i][k], sx, sy);
		}

		hbox = create_hbox(vbox, cs);
		create_label(hbox, CMsg::Analog_to_Digital_Sensitivity);
		lbl = create_label(hbox, " ");
		gtk_widget_set_size_request(lbl, 32, sy);
		create_label(hbox, "0 <-> 10");

		scroll = create_scroll_win(vbox, 240, 180);
		svbox = create_vbox(scroll, cs);
		for(int k=0; k<6; k++) {
			hbox = create_hbox(svbox, cs);
			CMsg::Id id = LABELS::joypad_axis[k];
			lbl = create_label(hbox, id);
			gtk_widget_set_size_request(lbl, tx, sy);
			val = 10 - pConfig->joy_axis_threshold[i][k];
			int n = i * 6 + k;
			axis[i][k] = create_scale_box(hbox, 0, 10, val, false, n);
			gtk_scale_set_value_pos(GTK_SCALE(axis[i][k]), GTK_POS_RIGHT);
			gtk_widget_set_size_request(axis[i][k], sx, sy);
		}
	}

#endif

	// create notebook tab
	vboxall = create_vbox(hboxall);
	nb = create_notebook(vboxall);

	int tab_offset = KeybindData::TAB_JOY2JOY;
	for(int tab_num=tab_offset; tab_num < KeybindData::TABS_MAX; tab_num++) {
		KeybindDataControl *kc = new KeybindDataControl();
		ctrls.Add(kc);

		// add note(tab) to notebook
		GtkWidget *vboxtab = create_vbox(NULL);
		add_note(nb, vboxtab, LABELS::keybind_tab[tab_num]);
//		vboxbox = create_vbox(vboxall);

		kc->Create(this, vboxtab, tab_num, 280, 320
			, G_CALLBACK(OnKeyDown), G_CALLBACK(OnDoubleClick), G_CALLBACK(OnFocusIn));

		hbox = create_hbox(vboxtab, cs);
		vbox = create_vbox(hbox, cs);
		btn = create_button(vbox, CMsg::Load_Default, G_CALLBACK(OnClickLoadDefault));
		g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
		hbox = create_hbox(vboxtab, cs);
		vbox = create_vbox(hbox, cs);
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Load_Preset_VDIGIT), i+1);
			btn = create_button(vbox, label, G_CALLBACK(OnClickLoadPreset));
			g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
		vbox = create_vbox(hbox, cs);
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Save_Preset_VDIGIT), i+1);
			btn = create_button(vbox, label, G_CALLBACK(OnClickSavePreset));
			g_object_set_data(G_OBJECT(btn), "ctrl", (gpointer)kc);
			g_object_set_data(G_OBJECT(btn), "num", (gpointer)(intptr_t)i);
		}
//		if (LABELS::keybind_combi[tab] != CMsg::Null) {
//			kc->chkCombi = create_check_box(vboxall, LABELS::keybind_combi[tab_num], kc->GetCombi() != 0);
//		}
	}
	ShowAfter(vboxall);

	emu->set_pause(1, true);

	return true;
}

void JoySettingBox::Hide()
{
	KeybindControlBox::Hide();

	emu->set_pause(1, false);
}

bool JoySettingBox::SetData()
{
	KeybindControlBox::SetData();

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
#ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)get_combo_sel_num(com[i]);
#endif
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			GtkWidget *widget = scale[i][k];
			if (!widget) continue;
			pConfig->joy_mashing[i][k] = (int)get_scale_value(widget);
		}
		for(int k = 0; k < 6; k++) {
			GtkWidget *widget = axis[i][k];
			if (!widget) continue;
			pConfig->joy_axis_threshold[i][k] = 10 - (int)get_scale_value(widget);
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
#ifdef USE_JOYSTICK_TYPE
	// will change joypad type in emu thread
	emumsg.Send(EMUMSG_ID_MODIFY_JOYTYPE);
#endif
#endif
	return true;
}

#if 0
void JoySettingBox::OnChangeValue(GtkWidget *widget, gpointer user_data)
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	JoySettingBox *obj = (JoySettingBox *)user_data;
	int i = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;
	pConfig->joy_mashing[i][k] = (int)obj->get_scale_value(widget);
	emu->set_joy_mashing();
#endif
}
#endif

}; /* namespace GUI_GTK_X11 */


