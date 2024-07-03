/** @file win_midlatebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.12 -

	@brief [ midi latency box ]
*/

#include "win_midlatebox.h"
#include "../../emu.h"
#include "../../msgs.h"
#include "../../utility.h"
#include "win_gui.h"
#include <commctrl.h>

namespace GUI_WIN
{

MidLateBox::MidLateBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_MIDLATEBOX, new_font, new_emu, new_gui)
{
}

MidLateBox::~MidLateBox()
{
}

INT_PTR MidLateBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	// layout
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);

	CBox *hbox = box_all->AddBox(CBox::HorizontalBox, 0, 0, _T("midiod"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Output_Latency);
	HWND hCtrl = CreateEditBox(hbox, IDC_EDIT_MIDIOUT_DELAY, (int)pConfig->midiout_delay, 6);
	CreateUpDown(hbox, IDC_SPIN_MIDIOUT_DELAY, hCtrl, 0, 2000, pConfig->midiout_delay);
	CreateStatic(hbox, IDC_STATIC, CMsg::msec);

	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos, margin);
	box_all->AddBox(box_btn);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	box_all->Realize(*this);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR MidLateBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);

	if (wId == IDOK) {
		onOK(message, wParam, lParam);
	}
	if (wId == IDOK || wId == IDCANCEL) {
		::EndDialog(hDlg, wId);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR MidLateBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL rc = FALSE;
	int valuei = GetDlgItemInt(hDlg, IDC_EDIT_MIDIOUT_DELAY, &rc, true);
	if (rc == TRUE && 0 <= valuei && valuei <= 2000) {
		pConfig->midiout_delay = valuei;
		emu->set_midiout_delay_time(valuei);
	}
	return (INT_PTR)TRUE;
}

}; /* namespace GUI_WIN */
