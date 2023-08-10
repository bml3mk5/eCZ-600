/** @file gtk_recaudbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.30 -

	@brief [ record audio box ]
*/

#include "gtk_recaudbox.h"
#include "gtk_x11_gui.h"
#include "../../video/rec_audio.h"
#include "../../emu.h"
#include "../../clocale.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

// list
static const char *type_label[] = {
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_QTKIT
	_T("qtkit"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_QTKIT
	RECORD_AUDIO_TYPE_QTKIT,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
	0 };

RecAudioBox::RecAudioBox(GUI *new_gui) : DialogBox(new_gui)
{
	typnum = 0;
	memset(codnums, 0, sizeof(codnums));
//	memset(quanums, 0, sizeof(quanums));
	memset(enables, 0, sizeof(enables));
	memset(comCod, 0, sizeof(comCod));
//	memset(comQua, 0, sizeof(comQua));
	btnOk = NULL;
	fpsnum = -1;
}

RecAudioBox::~RecAudioBox()
{
}

bool RecAudioBox::Show(GtkWidget *parent_window, int vid_fps_num)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	fpsnum = vid_fps_num;
	int codnum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quanum = emu->get_parami(VM::ParamRecAudioQuality);
	int i;
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecAudioType)) {
			typnum = i;
			codnums[i] = codnum;
//			quanums[i] = quanum;
			break;
		}
	}

	create_dialog(window, CMsg::Record_Sound);
	btnOk = add_accept_button(CMsg::Start);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vboxall;
	GtkWidget *vbox;
	GtkWidget *hbox;

	// create notebook tab
	notebook = create_notebook(cont);
	g_signal_connect(G_OBJECT(notebook), "change-current-page", G_CALLBACK(OnChangeTab), (gpointer)this);

	//
	for(i=0; type_ids[i] != 0; i++) {
		vboxall = create_vbox(NULL);
		add_note(notebook, vboxall, gettext(type_label[i]));

		enables[i] = emu->rec_sound_enabled(type_ids[i]);

		vbox = create_vbox(vboxall);

		switch(type_ids[i]) {
		case RECORD_AUDIO_TYPE_WAVE:
			hbox = create_hbox(vbox);
			create_label(hbox, CMsg::Select_a_sample_rate_on_sound_menu_in_advance);
			break;
		default:
			hbox = create_hbox(vbox);
			const char **codlbl = emu->get_rec_sound_codec_list(type_ids[i]);
			comCod[i] = create_combo_box(hbox,CMsg::Codec_Type,codlbl,codnums[i]);
//			hbox = create_hbox(vbox);
//			const char **qualbl = emu->get_rec_sound_quality_list(type_ids[i]);
//			comQua[i] = create_combo_box(hbox,CMSG(Quality),qualbl,quanums[i]);
			break;
		}

		if (!enables[i]) {
			hbox = create_hbox(vbox);
			create_label(hbox, CMsg::Need_install_library);
		}
	}

	ChangeTab(0);

	//
	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));

	emu->set_pause(1, true);

	return true;
}

void RecAudioBox::Hide()
{
	DialogBox::Hide();
	emu->set_pause(1, false);
}

bool RecAudioBox::SetData()
{
	int selnum = gtk_notebook_get_current_page((GtkNotebook *)notebook);
	codnums[selnum] = get_combo_sel_num(comCod[selnum]);
//	quanums[selnum] = get_combo_sel_num(comQua[selnum]);
	emu->set_parami(VM::ParamRecAudioType, type_ids[selnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnums[selnum]);
//	emu->set_parami(VM::ParamRecAudioQuality, quanums[selnum]);

	if (fpsnum >= 0) {
		gui->PostEtStartRecordVideo(fpsnum);
	}
	gui->PostEtStartRecordSound();
	return true;
}

void RecAudioBox::ChangeTab(int idx)
{
	typnum = idx;
	gtk_widget_set_sensitive(btnOk, enables[idx]);
}

gboolean RecAudioBox::OnChangeTab(GtkNotebook *nb, gint idx, gpointer user_data)
{
	RecAudioBox *obj = (RecAudioBox *)user_data;
	obj->ChangeTab(idx);
	return TRUE;
}

void RecAudioBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	RecAudioBox *obj = (RecAudioBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


