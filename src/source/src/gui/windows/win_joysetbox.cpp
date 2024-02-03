/** @file win_joysetbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.07 -

	@brief [ joypad setting box ]
*/
#include "win_joysetbox.h"
#include "../../emu.h"
#include "../../emumsg.h"
#include "win_gui.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../gui_keybinddata.h"
#include "../../keycode.h"
#include "../../utility.h"

namespace GUI_WIN
{

JoySettingBox::JoySettingBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_JOYSETTING, new_font, new_emu, new_gui)
{
}

JoySettingBox::~JoySettingBox()
{
}

INT_PTR JoySettingBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hCtrl;
	TCITEM tcitm;
	_TCHAR label[KBLABEL_MAXLEN];
	int tab_offset = KeybindData::TAB_JOY2JOY;

	CDialogBox::onInitDialog(message, wParam, lParam);

	// create dialog
	selected_tabctrl = 0;

	SIZE siz;
	font->GetTextSize(hDlg, NULL, &siz);

	// calcrate number of tabs
	for(int tab_num=tab_offset; tab_num < KeybindData::TABS_MAX; tab_num++) {
		hCtrl =	CreateControl(NULL, _T("KeyBindCtrl"), IDC_CUSTOM0 + tab_num - tab_offset, 100, 380, WS_BORDER | WS_VSCROLL, 0, SM_CXVSCROLL);
		KeybindControl *kc = KeybindControl::GetPtr(hCtrl);

		kc->Init(emu, tab_num, font->GetFont());
		kc->SetCellSize(siz.cx * 18 + padding * 2, siz.cy + padding * 2);
		kc->MapDefaultVmKey();
		kc->SetJoyMask(&enable_axes);
		kc->Update();

		kc->SetTitleLabel(LABELS::keybind_col[tab_num][0], LABELS::keybind_col[tab_num][1]);

		kbctl.push_back(kc);
	}

	//

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);
	CBox *box_hall = box_all->AddBox(CBox::HorizontalBox);

	//
	// controller type and button mashing 
	//
	SIZE sz;
	sz.cx = 120; sz.cy = 24;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
//	CBox *hbox_joy = box_hall->AddBox(CBox::HorizontalBox, 0, margin);

	int val = 0;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CBox *vbox = box_hall->AddBox(CBox::VerticalBox);

		UTILITY::stprintf(label, 64, CMSGM(JoypadVDIGIT), i + 1);
		CreateStatic(vbox, IDC_STATIC, label);
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		CreateComboBox(vbox, IDC_COMBO_JOY1 + i, LABELS::joypad_type, val, 8);
#endif
		CBox *hbox = vbox->AddBox(CBox::HorizontalBox);
		CreateStatic(hbox, IDC_STATIC, CMsg::Button_Mashing_Speed);
		hbox->AddSpace(32, 0);
		CreateStatic(hbox, IDC_STATIC, _T("0 <-> 3"));

		for(int k=0; k < KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) break;

			CBox *box = vbox->AddBox(CBox::HorizontalBox);

			CMsg::Id id = (CMsg::Id)cVmJoyLabels[kk].id;
			CreateStatic(box, IDC_STATIC, CMSGVM(id), 80, 0, SS_CENTER);
			val = pConfig->joy_mashing[i][k];
			CreateSlider(box, IDC_SLIDER_JOY1 + i * KEYBIND_JOY_BUTTONS + k,
				sz.cx, sz.cy, 0, 3, 1, val, false);
		}

		hbox = vbox->AddBox(CBox::HorizontalBox);
		CreateStatic(hbox, IDC_STATIC, CMsg::Analog_to_Digital_Sensitivity);
		hbox->AddSpace(32, 0);
		CreateStatic(hbox, IDC_STATIC, _T("0 <-> 10"));

		for(int k=0; k < 6; k++) {
			CBox *box = vbox->AddBox(CBox::HorizontalBox);

			CMsg::Id id = LABELS::joypad_axis[k];
			CreateStatic(box, IDC_STATIC, CMSGVM(id), 80, 0, SS_CENTER);
			val = 10 - pConfig->joy_axis_threshold[i][k];
			CreateSlider(box, IDC_SLIDER_AXIS1 + i * 6 + k,
				sz.cx, sz.cy, 0, 10, 1, val, false);
		}
	}
#endif

	//
	// tab control for button assining
	//
	CBox *box_vall = box_hall->AddBox(CBox::VerticalBox);
	CBox *box_tab = AdjustTabControl(box_vall, IDC_TAB1, IDC_STATIC_0);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	tcitm.mask = TCIF_TEXT;

	for(int tab_num=0; tab_num<(int)kbctl.size(); tab_num++) {
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, CMSGVM(LABELS::keybind_tab[tab_num+tab_offset]));
		tcitm.pszText = label;
		TabCtrl_InsertItem(hTabCtrl , tab_num , &tcitm);
	}
	TabCtrl_SetCurSel(hTabCtrl, selected_tabctrl);

	//
	// adjust control size
	//

	// kb control
	CBox *box_vall0 = NULL;
	for(int tab_num=0; tab_num<(int)kbctl.size(); tab_num++) {
		CBox *box_v = box_tab->AddBox(CBox::VerticalBox);
		CBox *box_kb = box_v->AddBox(CBox::VerticalBox);
		AdjustControl(box_kb, IDC_CUSTOM0 + tab_num, kbctl[tab_num]->GetWidth(), 380, SM_CXVSCROLL);

#if 0
		if (LABELS::joyset_combi[tab_num] != CMsg::Null) {
			int id = get_combi_id(kbctl[tab_num]);
			if (id) {
				CreateCheckBox(box_kb, id, LABELS::keybind_combi[tab_num], false);
				CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
			}
		}
#endif
		if (tab_num == 0) {
			box_vall0 = box_v;
		}
	}

	// buttons
	CBox *box_hbtn = box_vall0->AddBox(CBox::HorizontalBox);
	CBox *box_vbtn = box_hbtn->AddBox(CBox::VerticalBox);
	int n = 0;
	for(int i=0; LABELS::keybind_btn[i] != CMsg::End; i++) {
		if (LABELS::keybind_btn[i] == CMsg::Null) {
			box_vbtn = box_hbtn->AddBox(CBox::VerticalBox);
			continue;
		}
		CreateButton(box_vbtn, IDC_BTN_LOAD_DEFAULT + n, LABELS::keybind_btn[i], 8);
		n++;
	}

	// joypad
	enable_axes = ~0;
	CBox *hbox = box_vall->AddBox(CBox::HorizontalBox);
	CreateCheckBox(hbox, IDC_CHK_AXIS3, CMsg::Enable_Z_axis, (enable_axes & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0);
	CreateCheckBox(hbox, IDC_CHK_AXIS4, CMsg::Enable_R_axis, (enable_axes & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0);
	CreateCheckBox(hbox, IDC_CHK_AXIS5, CMsg::Enable_U_axis, (enable_axes & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0);
	CreateCheckBox(hbox, IDC_CHK_AXIS6, CMsg::Enable_V_axis, (enable_axes & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0);

	// tab control size

	// ok cancel
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8, false);

	RECT prc;
	GetClientRect(hTabCtrl, &prc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
	box_tab->SetTopMargin(prc.top + 8);


	box_all->Realize(*this);

	select_tabctrl(selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onHScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
//	SetValue((HWND)lParam);

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onVScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
//	SetValue((HWND)lParam);

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDOK:
	case IDCANCEL:
		if (LOWORD(wParam) == IDOK) {
			onClickOk();
		}
		::EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;

	case IDC_BTN_LOAD_DEFAULT:
		return onClickLoadDefault();

	case IDC_BTN_LOAD_PRESET1:
	case IDC_BTN_LOAD_PRESET2:
	case IDC_BTN_LOAD_PRESET3:
	case IDC_BTN_LOAD_PRESET4:
		return onClickLoadPreset(LOWORD(wParam) - IDC_BTN_LOAD_PRESET1);

	case IDC_BTN_SAVE_PRESET1:
	case IDC_BTN_SAVE_PRESET2:
	case IDC_BTN_SAVE_PRESET3:
	case IDC_BTN_SAVE_PRESET4:
		return onClickSavePreset(LOWORD(wParam) - IDC_BTN_SAVE_PRESET1);

	case IDC_CHK_AXIS1:
	case IDC_CHK_AXIS2:
	case IDC_CHK_AXIS3:
	case IDC_CHK_AXIS4:
	case IDC_CHK_AXIS5:
	case IDC_CHK_AXIS6:
		return onClickAxis(LOWORD(wParam));
	}
	return (INT_PTR)FALSE;
}

INT_PTR JoySettingBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	// change tab
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

INT_PTR JoySettingBox::onMouseWheel(UINT message, WPARAM wParam, LPARAM lParam)
{
	SendDlgItemMessage(hDlg, IDC_CUSTOM0 + selected_tabctrl, message, wParam, lParam);
	return (INT_PTR)TRUE;
}

#if 0
INT_PTR JoySettingBox::onControlColorStatic(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE h = (HANDLE)GetStockObject(NULL_BRUSH);
	SetBkMode((HDC)wParam, TRANSPARENT);
	return (INT_PTR)h;
}
#endif

INT_PTR JoySettingBox::onClickOk()
{
	for(int tab_num=0; tab_num<(int)kbctl.size(); tab_num++) {
		kbctl[tab_num]->SetData();
#if 0
		int id = get_combi_id(kbctl[tab_num]);
		if (id) {
			kbctl[tab_num]->SetCombi(IsDlgButtonChecked(hDlg, id) ? 1 : 0);
		}
#endif
	}

	SetValue();

	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onClickLoadDefault()
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->LoadDefaultPreset();

#if 0
	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
	}
#endif
	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onClickLoadPreset(int idx)
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->LoadPreset(idx);

#if 0
	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		CheckDlgButton(hDlg, id, kbctl[tab_num]->GetCombi() != 0);
	}
#endif
	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onClickSavePreset(int idx)
{
	int tab_num = selected_tabctrl;

	kbctl[tab_num]->SavePreset(idx);

#if 0
	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		kbctl[tab_num]->SetCombi(IsDlgButtonChecked(hDlg, id) ? 1 : 0);
	}
#endif
	return (INT_PTR)TRUE;
}

INT_PTR JoySettingBox::onClickAxis(int id)
{
	uint32_t bits = 0;

	switch(id) {
	case IDC_CHK_AXIS3:
		bits = (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT);
		break;
	case IDC_CHK_AXIS4:
		bits = (JOYCODE_R_UP | JOYCODE_R_DOWN);
		break;
	case IDC_CHK_AXIS5:
		bits = (JOYCODE_U_LEFT | JOYCODE_U_RIGHT);
		break;
	case IDC_CHK_AXIS6:
		bits = (JOYCODE_V_UP | JOYCODE_V_DOWN);
		break;
	default:
		break;
	}

	BIT_ONOFF(enable_axes, bits, (IsDlgButtonChecked(hDlg, id) != 0));

	return (INT_PTR)TRUE;
}

void JoySettingBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;

	selected_tabctrl = tab_num;
	for(int i=0; i<(int)kbctl.size(); i++) {
		hCtrl = GetDlgItem(hDlg, IDC_CUSTOM0 + i);
		ShowWindow(hCtrl, i == selected_tabctrl ? SW_SHOW : SW_HIDE);
	}

	hCtrl = GetDlgItem(hDlg, IDC_CHK_COMBI1);
	ShowWindow(hCtrl, SW_HIDE);
	hCtrl = GetDlgItem(hDlg, IDC_CHK_COMBI2);
	ShowWindow(hCtrl, SW_HIDE);

#if 0
	int id = get_combi_id(kbctl[tab_num]);
	if (id) {
		hCtrl = GetDlgItem(hDlg, id);
		ShowWindow(hCtrl, SW_SHOW);
	}
#endif
}

void JoySettingBox::SetValue()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		for(int k = 0; k < KEYBIND_JOY_BUTTONS; k++) {
			int id = IDC_SLIDER_JOY1 + i * KEYBIND_JOY_BUTTONS + k;
			pConfig->joy_mashing[i][k] = (int)SendDlgItemMessage(hDlg, id, TBM_GETPOS, 0, 0);
		}
		for(int k = 0; k < 6; k++) {
			int id = IDC_SLIDER_AXIS1 + i * 6 + k;
			pConfig->joy_axis_threshold[i][k] = 10 - (int)SendDlgItemMessage(hDlg, id, TBM_GETPOS, 0, 0);
		}
#ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)::SendDlgItemMessage(hDlg, IDC_COMBO_JOY1 + i, CB_GETCURSEL, 0, 0);
#endif
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
void JoySettingBox::SetValue(HWND ctrl)
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = (int)::GetDlgCtrlID(ctrl);
	i -= IDC_SLIDER_JOY1;
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;

	pConfig->joy_mashing[i][k] = (int)SendMessage(ctrl, TBM_GETPOS, 0, 0);
	emu->set_joy_mashing();
#endif
}
#endif

}; /* namespace GUI_WIN */
