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

extern EMU *emu;

namespace GUI_GTK_X11
{

JoySettingBox::JoySettingBox(GUI *new_gui) : DialogBox(new_gui)
{
}

JoySettingBox::~JoySettingBox()
{
}

bool JoySettingBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Joypad_Setting);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *boxall;
	GtkWidget *hboxall;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *lbl;
	_TCHAR label[64];
	int tx = 80;
	int sx = 256;
	int sy = 24;

	//

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)

	boxall = create_vbox(cont);
	hboxall = create_hbox(boxall);

	for(int i=0; i<MAX_JOYSTICKS; i++) {
		vbox = create_vbox(hboxall);

		hbox = create_hbox(vbox);
		UTILITY::stprintf(label, 64, CMSG(JoypadVDIGIT), i + 1);
		int val;
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		com[i] = create_combo_box(hbox, label, LABELS::joypad_type, val);
#endif

		hbox = create_hbox(vbox);
		create_label(hbox, CMsg::Button_Mashing_Speed);
		lbl = create_label(hbox, _T(" "));
		gtk_widget_set_size_request(lbl, 32, sy);
		create_label(hbox, _T("0 <-> 3"));

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				scale[i][k] = NULL;
				continue;
			}
			
			hbox = create_hbox(vbox);
			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			lbl = create_label(hbox, id);
			gtk_widget_set_size_request(lbl, tx, sy);
			val = pConfig->joy_mashing[i][k];
			int n = i * KEYBIND_JOY_BUTTONS + k;
			scale[i][k] = create_scale_box(hbox, 0, 3, val, false, n);
			gtk_scale_set_value_pos(GTK_SCALE(scale[i][k]), GTK_POS_RIGHT);
			gtk_widget_set_size_request(scale[i][k], sx, sy);
		}

		hbox = create_hbox(vbox);
		create_label(hbox, CMsg::Analog_to_Digital_Sensitivity);
		lbl = create_label(hbox, _T(" "));
		gtk_widget_set_size_request(lbl, 32, sy);
		create_label(hbox, _T("0 <-> 10"));

		for(int k=0; k<6; k++) {
			hbox = create_hbox(vbox);
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

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);

	emu->set_pause(1, true);

	return true;
}

void JoySettingBox::Hide()
{
	DialogBox::Hide();
	emu->set_pause(1, false);
}

void JoySettingBox::SetData()
{
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

void JoySettingBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	JoySettingBox *obj = (JoySettingBox *)user_data;
	obj->SetData();
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


