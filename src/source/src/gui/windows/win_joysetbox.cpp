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
	CDialogBox::onInitDialog(message, wParam, lParam);

	// layout
	SIZE sz;
	sz.cx = 160; sz.cy = 24;
	_TCHAR label[64];

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	CBox *hbox_all = box_all->AddBox(CBox::HorizontalBox, 0, margin);

	int val = 0;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CBox *vbox = hbox_all->AddBox(CBox::VerticalBox);

		CBox *hbox = vbox->AddBox(CBox::HorizontalBox);
		UTILITY::stprintf(label, 64, CMSGM(JoypadVDIGIT), i + 1);
		CreateStatic(hbox, IDC_STATIC, label);
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		CreateComboBox(hbox, IDC_COMBO_JOY1 + i, LABELS::joypad_type, val, 8);
#endif
		hbox = vbox->AddBox(CBox::HorizontalBox);
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

	// button
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8, false);

	box_all->Realize(*this);

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
	if (LOWORD(wParam) == IDOK) {
		SetValue();
	}
	return CDialogBox::onCommand(message, wParam, lParam);
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
