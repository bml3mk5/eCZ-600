/** @file gtk_recvidbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.30 -

	@brief [ record video box ]
*/

#include "gtk_recvidbox.h"
#include "gtk_x11_gui.h"
#include "../../video/rec_video.h"
#include "../../emu.h"
#include "../../clocale.h"

extern EMU *emu;

namespace GUI_GTK_X11
{

// list
static const char *type_label[] = {
#ifdef USE_REC_VIDEO_VFW
	_T("video for windows"),
#endif
#ifdef USE_REC_VIDEO_QTKIT
	_T("qtkit"),
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	_T("ffmpeg"),
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_VIDEO_VFW
	RECORD_VIDEO_TYPE_VFW,
#endif
#ifdef USE_REC_VIDEO_QTKIT
	RECORD_VIDEO_TYPE_QTKIT,
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	RECORD_VIDEO_TYPE_FFMPEG,
#endif
	0 };

RecVideoBox::RecVideoBox(GUI *new_gui) : DialogBox(new_gui)
{
	typnum = 0;
	memset(codnums, 0, sizeof(codnums));
	memset(quanums, 0, sizeof(quanums));
	memset(enables, 0, sizeof(enables));
	memset(comCod, 0, sizeof(comCod));
	memset(comQua, 0, sizeof(comQua));
	btnOk = NULL;
	conti = false;
}

RecVideoBox::~RecVideoBox()
{
}

bool RecVideoBox::Show(GtkWidget *parent_window, int num, bool continuous)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	fpsnum = num;
	conti = continuous;
	int codnum = emu->get_parami(VM::ParamRecVideoCodec);
	int quanum = emu->get_parami(VM::ParamRecVideoQuality);
	int i;
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecVideoType)) {
			typnum = i;
			codnums[i] = codnum;
			quanums[i] = quanum;
			break;
		}
	}

	create_dialog(window, CMsg::Record_Screen);
	btnOk = add_accept_button(conti ? CMsg::Next : CMsg::Start);
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

		enables[i] = emu->rec_video_enabled(type_ids[i]);

		vbox = create_vbox(vboxall);

		hbox = create_hbox(vbox);
		const char **codlbl = emu->get_rec_video_codec_list(type_ids[i]);
		comCod[i] = create_combo_box(hbox,CMsg::Codec_Type,codlbl,codnums[i]);
		hbox = create_hbox(vbox);
		const CMsg::Id *qualbl = emu->get_rec_video_quality_list(type_ids[i]);
		comQua[i] = create_combo_box(hbox,CMsg::Quality,qualbl,quanums[i]);

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

void RecVideoBox::HideB(bool canceled)
{
	Hide();
	if (!conti || canceled) {
		emu->set_pause(1, false);
	}
}

void RecVideoBox::Hide()
{
	DialogBox::Hide();
}

bool RecVideoBox::SetData()
{
	int selnum = gtk_notebook_get_current_page((GtkNotebook *)notebook);
	codnums[selnum] = get_combo_sel_num(comCod[selnum]);
	quanums[selnum] = get_combo_sel_num(comQua[selnum]);
	emu->set_parami(VM::ParamRecVideoType, type_ids[selnum]);
	emu->set_parami(VM::ParamRecVideoCodec, codnums[selnum]);
	emu->set_parami(VM::ParamRecVideoQuality, quanums[selnum]);

	if (conti) {
		gui->ShowRecordVideoAndAudioDialog(fpsnum);
	} else {
		gui->PostEtStartRecordVideo(fpsnum);
	}
	return true;
}

void RecVideoBox::ChangeTab(int idx)
{
	typnum = idx;
	gtk_widget_set_sensitive(btnOk, enables[idx]);
}

gboolean RecVideoBox::OnChangeTab(GtkNotebook *nb, gint idx, gpointer user_data)
{
	RecVideoBox *obj = (RecVideoBox *)user_data;
	obj->ChangeTab(idx);
	return TRUE;
}

void RecVideoBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	RecVideoBox *obj = (RecVideoBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->HideB(response_id != GTK_RESPONSE_ACCEPT);
}

}; /* namespace GUI_GTK_X11 */


