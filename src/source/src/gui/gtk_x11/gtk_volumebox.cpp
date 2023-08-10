/** @file gtk_volumebox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ volume box ]
*/

#include "gtk_volumebox.h"
#include "../../emu.h"
#include "../../clocale.h"
#include "../../labels.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

VolumeBox::VolumeBox(GUI *new_gui) : DialogBox(new_gui)
{
}

VolumeBox::~VolumeBox()
{
}

bool VolumeBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Volume);
	add_reject_button(CMsg::Close);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *hboxall;
	GtkWidget *vbox;
	GtkWidget *vol;
	GtkWidget *lbl;

	//
	SetPtr();

	hboxall = create_hbox(cont);

	int n = 0;
	for(int i=0; LABELS::volume[i] != CMsg::End; i++) {
		bool wrap = (LABELS::volume[i] == CMsg::Null);
		if (wrap) {
#if GTK_CHECK_VERSION(3,0,0)
			GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
			GtkWidget *sep = gtk_hseparator_new();
#endif
			add_widget(cont, sep);
			hboxall = create_hbox(cont);
			continue;
		}
		vbox = create_vbox(hboxall);
		lbl = create_label(vbox, LABELS::volume[i]);
		gtk_widget_set_size_request(lbl, 64, 40);
		vol = create_volume_box(vbox, GetVolume(n), n, G_CALLBACK(OnChangeVolume));
		gtk_widget_set_size_request(vol, 64, 112);
		create_check_box(vbox, CMsg::Mute, GetMute(n), n, G_CALLBACK(OnChangeMute));
		n++;
	}

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);

	return true;
}

void VolumeBox::SetPtr()
{
	int i = 0;
	p_volume[i++] = &pConfig->volume;
	p_volume[i++] = &pConfig->opm_volume;
	p_volume[i++] = &pConfig->adpcm_volume;
	p_volume[i++] = &pConfig->fdd_volume;
	p_volume[i++] = &pConfig->hdd_volume;

	i = 0;
	p_mute[i++] = &pConfig->mute;
	p_mute[i++] = &pConfig->opm_mute;
	p_mute[i++] = &pConfig->adpcm_mute;
	p_mute[i++] = &pConfig->fdd_mute;
	p_mute[i++] = &pConfig->hdd_mute;
}

void VolumeBox::SetVolume(int index, int value)
{
	*p_volume[index] = value;
	emu->set_volume(0);
}

int VolumeBox::GetVolume(int index)
{
	return *p_volume[index];
}

void VolumeBox::SetMute(int index, bool value)
{
	*p_mute[index] = value;
	emu->set_volume(0);
}

bool VolumeBox::GetMute(int index)
{
	return *p_mute[index];
}

void VolumeBox::OnChangeVolume(GtkWidget *widget, gpointer user_data)
{
//	g_print("OnChangeVolume\n");
	VolumeBox *obj = (VolumeBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->SetVolume(index, (int)obj->get_volume_value(widget));
}

void VolumeBox::OnChangeMute(GtkWidget *widget, gpointer user_data)
{
//	g_print("OnChangeMute\n");
	VolumeBox *obj = (VolumeBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->SetMute(index, obj->get_check_state(widget));
}

void VolumeBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
//	g_print("OnResponse\n");
	VolumeBox *obj = (VolumeBox *)user_data;
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


