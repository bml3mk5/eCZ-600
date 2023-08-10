/** @file win_volumebox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ volume box ]
*/
#include "win_volumebox.h"
#include "../../emu.h"
#include "win_gui.h"
#include "../../labels.h"

namespace GUI_WIN
{

VolumeBox::VolumeBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_VOLUME, new_font, new_emu, new_gui)
{
	volumes.push_back(&pConfig->volume);
	volumes.push_back(&pConfig->opm_volume);
	volumes.push_back(&pConfig->adpcm_volume);
	volumes.push_back(&pConfig->fdd_volume);
	volumes.push_back(&pConfig->hdd_volume);

	mutes.push_back(&pConfig->mute);
	mutes.push_back(&pConfig->opm_mute);
	mutes.push_back(&pConfig->adpcm_mute);
	mutes.push_back(&pConfig->fdd_mute);
	mutes.push_back(&pConfig->hdd_mute);
}

VolumeBox::~VolumeBox()
{
}

INT_PTR VolumeBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	// layout
	SIZE siz;
	font->GetTextSize(hDlg, _T("0000000"), &siz);
	SIZE sli;
	sli.cx = 50; sli.cy = 50;

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);
	CBox *box_hall;
	int n = 0;
	int row = 0;
	for(int i=0; LABELS::volume[i] != CMsg::End; i++) {
		bool wrap = (LABELS::volume[i] == CMsg::Null);
		if (wrap) {
			CBox *box_sp = new CBox(CBox::HorizontalBox, CBox::CenterPos);
			box_all->AddBox(box_sp);
			CreateStatic(box_sp, IDC_STATIC_39 + row, _T(""), 160, 1, SS_CENTER, WS_EX_STATICEDGE);
			row++;
		}
		if (i == 0 || wrap) {
			box_hall = new CBox(CBox::HorizontalBox, CBox::CenterPos);
			box_all->AddBox(box_hall);
		}
		if (wrap) {
			continue;
		}
		CBox *box = new CBox(CBox::VerticalBox, 0, padding);
		box_hall->AddBox(box);
		if (volumes[n]) {
			CreateStatic(box, IDC_STATIC_1 + n, LABELS::volume[i], siz.cx, siz.cy * 2, SS_CENTER);
			CreateSlider(box, IDC_SLIDER_VOL1 + n, sli.cx, 100, 0, 100, 25, 100 - *volumes[n]);
			CreateStatic(box, IDC_STATIC_21 + n, _T(""), siz.cx, siz.cy, SS_CENTER);
			SetVolumeText(n);
		}
		if (mutes[n]) {
			CreateCheckBox(box, IDC_CHK_MUTE1 + n, CMsg::Mute, *mutes[n]);
		}
		if (i == 0) {
			CBox *box_sp = new CBox(CBox::HorizontalBox, CBox::MiddlePos, padding);
			box_hall->AddBox(box_sp);
			CreateStatic(box_sp, IDC_STATIC_0, _T(""), 1, 160, 0, WS_EX_STATICEDGE);
		}
		n++;
	}

	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos, padding);
	box_all->AddBox(box_btn);
	CreateButton(box_btn, IDOK, CMsg::Close, 8, true);

	box_all->Realize(*this);

	ResizeControl(IDC_STATIC_39, box_all->GetWidth() - 10, 1);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR VolumeBox::onHScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();
	return (INT_PTR)TRUE;
}

INT_PTR VolumeBox::onVScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();
	return (INT_PTR)TRUE;
}

INT_PTR VolumeBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	SetVolume();

	return CDialogBox::onCommand(message, wParam, lParam);
}

void VolumeBox::SetVolume()
{
	int count = (int)volumes.size();
	for(int i=0; i<count; i++) {
		if (volumes[i]) {
			*volumes[i] = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_VOL1 + i, TBM_GETPOS, 0, 0);
			*volumes[i] = 100 - (*volumes[i]);
			SetVolumeText(i);
		}
		if (mutes[i]) {
			*mutes[i] = (IsDlgButtonChecked(hDlg, IDC_CHK_MUTE1 + i) != 0);
		}
	}

	// callback
	if (emu) {
		emu->set_volume(0);
	}

	return;
}

void VolumeBox::SetVolumeText(int num)
{
	_TCHAR str[8];
	_stprintf(str, _T("%02d"), *volumes[num]);
	SetDlgItemText(hDlg, IDC_STATIC_21+num, str);
}

}; /* namespace GUI_WIN */
