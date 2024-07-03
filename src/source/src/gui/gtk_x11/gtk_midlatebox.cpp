/** @file gtk_midlatebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.12 -

	@brief [ midi latency box ]
*/

#include "gtk_midlatebox.h"
#include "../gui.h"
#include "../../msgs.h"
#include "../../config.h"
#include "../../utility.h"
#ifdef USE_GTK
#include <cairo/cairo.h>
#endif

//extern EMU *emu;
//extern GUI *gui;

namespace GUI_GTK_X11
{

MidLateBox::MidLateBox(GUI *new_gui) : DialogBox(new_gui)
{
	spnMIDIOutDelay = NULL;
}

MidLateBox::~MidLateBox()
{
}

bool MidLateBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::MIDI_Output_Latency);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vboxall;
	GtkWidget *hbox;

	vboxall = create_vbox(cont);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Output_Latency);
	int valuei = pConfig->midiout_delay;
	spnMIDIOutDelay = create_spin(hbox, 0, 2000, valuei);
	create_label(hbox, CMsg::msec);

	//

	gtk_widget_show_all(dialog);

	return true;
}

void MidLateBox::SetData()
{
	int val = (int)get_spin_value(spnMIDIOutDelay);
	if (0 <= val && val <= 2000) {
		pConfig->midiout_delay = val;
		emu->set_midiout_delay_time(val);
	}
}

}; /* namespace GUI_GTK_X11 */

