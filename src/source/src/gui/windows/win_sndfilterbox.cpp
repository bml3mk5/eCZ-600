/** @file win_sndfilterbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.11.26 -

	@brief [ sound filter box ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUG_SOUND_FILTER

#include "win_sndfilterbox.h"
#include "../../emu.h"
#include <commctrl.h>

extern int RRX11;
extern int RRX12;
extern int RRY11;

namespace GUI_WIN
{

SndFilterBox::SndFilterBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_SNDFILTERBOX, new_font, new_emu, new_gui)
{
}

SndFilterBox::~SndFilterBox()
{
}

INT_PTR SndFilterBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL1, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL1, TBM_SETTICFREQ, 25, 0);
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL1, TBM_SETPOS  , TRUE, RRX11);

	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL2, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL2, TBM_SETTICFREQ, 25, 0);
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL2, TBM_SETPOS  , TRUE, RRX12);

	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL3, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL3, TBM_SETTICFREQ, 25, 0);
	SendDlgItemMessage(hDlg, IDC_SLIDER_VOL3, TBM_SETPOS  , TRUE, RRY11);

	SetDlgItemInt(hDlg, IDC_STATIC_11, RRX11, TRUE);
	SetDlgItemInt(hDlg, IDC_STATIC_12, RRX12, TRUE);
	SetDlgItemInt(hDlg, IDC_STATIC_13, RRY11, TRUE);

	// layout
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);

	for(int i=0; i<=2; i++) {
		CBox *box_hall = new CBox(CBox::HorizontalBox);
		box_all->AddBox(box_hall);
		AdjustControl(box_hall, IDC_STATIC_1 + i, 60, 32);
		AdjustControl(box_hall, IDC_STATIC_11 + i, 60, 32);
		AdjustControl(box_hall, IDC_SLIDER_VOL1 + i, 500, 32);
	}

	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos, padding);
	box_all->AddBox(box_btn);
	AdjustButton(box_btn, IDOK, 8);

	box_all->Realize(*this);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR SndFilterBox::onHScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();
	return (INT_PTR)TRUE;
}

INT_PTR SndFilterBox::onVScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();
	return (INT_PTR)TRUE;
}

INT_PTR SndFilterBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();

	return CDialogBox::onCommand(message, wParam, lParam);
}

void SndFilterBox::SetVolume()
{
	RRX11 = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_VOL1, TBM_GETPOS, 0, 0);
	RRX12 = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_VOL2, TBM_GETPOS, 0, 0);
	RRY11 = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_VOL3, TBM_GETPOS, 0, 0);
	::SetDlgItemInt(hDlg, IDC_STATIC_11, RRX11, TRUE);
	::SetDlgItemInt(hDlg, IDC_STATIC_12, RRX12, TRUE);
	::SetDlgItemInt(hDlg, IDC_STATIC_13, RRY11, TRUE);
	return;
}

}; /* namespace GUI_WIN */

#endif
