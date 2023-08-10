/** @file win_recaudbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio box ]
*/
#include "win_recaudbox.h"
#include "../../emu.h"
#include "../../video/rec_audio.h"
#include "win_gui.h"

namespace GUI_WIN
{

// list
static const char *audio_type_label[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_MMF
	_T("media foundation"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	NULL };
static const int audio_type_ids[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_MMF
	RECORD_AUDIO_TYPE_MMF,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
#endif
	0 };

RecAudioBox::RecAudioBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_RECAUDIOBOX, new_font, new_emu, new_gui)
{
	typnum = 0;
	memset(codnums, 0, sizeof(codnums));
	memset(quanums, 0, sizeof(quanums));
	memset(enables, 0, sizeof(enables));
}

RecAudioBox::~RecAudioBox()
{
}

/// create modal dialog box
INT_PTR RecAudioBox::Show(HWND hWnd)
{
	return (audio_type_ids[0] ? ::DialogBoxParam(hInstance, MAKEINTRESOURCE(dialogId), hWnd, Proc, (LPARAM)this) : IDCANCEL);
}

INT_PTR RecAudioBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

//	int codnum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quanum = emu->get_parami(VM::ParamRecAudioQuality);
	int i;
	typnum = 0;
	for(i=0; audio_type_ids[i] != 0; i++) {
		if (audio_type_ids[i] == emu->get_parami(VM::ParamRecAudioType)) {
			typnum = i;
			break;
		}
	}

	// layout
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin, _T("all"));
	CBox *box_tab = AdjustTabControl(box_all, IDC_TAB1, IDC_STATIC_0);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	TCITEM tcitm;
	tcitm.mask = TCIF_TEXT;

	int item_id = IDC_TAB1 + 1;

//	DWORD dwStyle = WS_CHILD | WS_VISIBLE;

	//
	for(i=0; audio_type_ids[i] != 0; i++) {
		tcitm.pszText = (LPSTR)_tgettext(audio_type_label[i]);
		TabCtrl_InsertItem(hTabCtrl ,i , &tcitm);
		enables[i] = emu->rec_sound_enabled(audio_type_ids[i]);

		switch(audio_type_ids[i]) {
			case RECORD_AUDIO_TYPE_WAVE:
			{
				item_range[i].first = item_id;
				CreateStatic(box_tab, item_id, CMsg::Select_a_sample_rate_on_sound_menu_in_advance);
				item_id++;
				item_range[i].last = item_id - 1;
				break;
			}
			default:
			{
				item_range[i].first = item_id;
				CBox *box_v = box_tab->AddBox(CBox::VerticalBox);

				CBox *box_h = box_v->AddBox(CBox::HorizontalBox);
				CreateStatic(box_h, item_id, CMsg::Codec_Type, 80, 0);
				item_id++;

				const char **codlbl = emu->get_rec_sound_codec_list(audio_type_ids[i]);
				CreateComboBox(box_h, item_id, codlbl, codnums[i], 12, true);
				cod_ids[i] = item_id;
				item_id++;

				if (!enables[i]) {
					CreateStatic(box_v, item_id, CMsg::Need_install_library);
					item_id++;
				}

				item_range[i].last = item_id - 1;
				break;
			}
		}
	}
	// select tab
	TabCtrl_SetCurSel(hTabCtrl, typnum);

	// tab control size

	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos);
	box_all->AddBox(box_btn);
	AdjustButton(box_btn, IDOK, 8);
	AdjustButton(box_btn, IDCANCEL, 8);

	box_all->Realize(*this);

	select_tabctrl(typnum);

	delete box_all;

	return (INT_PTR)TRUE;
}

void RecAudioBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;
	int cmdShow;

	typnum = tab_num;
	for(int i=0; audio_type_ids[i] != 0 && i < WIN_RECAUDIO_LIBS; i++) {
		cmdShow = (tab_num == i ? SW_SHOW : SW_HIDE);

		for(int id=item_range[i].first; id<=item_range[i].last; id++) {
			hCtrl = GetDlgItem(hDlg, id);
			ShowWindow(hCtrl, cmdShow);
		}
	}
	hCtrl = GetDlgItem(hDlg, IDOK);
	EnableWindow(hCtrl, enables[typnum]);
}

INT_PTR RecAudioBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);

	if (wId == IDOK) {
		onOK(message, wParam, lParam);
	}
	return CDialogBox::onCommand(message, wParam, lParam);
}

INT_PTR RecAudioBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR lpNmHdr = (NMHDR *)lParam;
	int i;
	if (lpNmHdr->idFrom == IDC_TAB1) {
		switch (lpNmHdr->code) {
		case TCN_SELCHANGE:
			i = TabCtrl_GetCurSel(lpNmHdr->hwndFrom);
			select_tabctrl(i);
			break;
		}
	}
	return (INT_PTR)TRUE;
}
INT_PTR RecAudioBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (cod_ids[typnum]) codnums[typnum] = (int)SendDlgItemMessage(hDlg, cod_ids[typnum], CB_GETCURSEL, 0, 0);
//	if (qua_ids[typnum]) quanums[typnum] = (int)SendDlgItemMessage(hDlg, qua_ids[typnum], CB_GETCURSEL, 0, 0);
	emu->set_parami(VM::ParamRecAudioType, audio_type_ids[typnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnums[typnum]);
//	emu->set_parami(VM::ParamRecAudioQuality, quanums[typnum]);

	return (INT_PTR)TRUE;
}

}; /* namespace GUI_WIN */
