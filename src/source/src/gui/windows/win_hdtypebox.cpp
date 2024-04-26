/** @file win_hdtypebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.20 -

	@brief [ select hd device type box ]
*/

#include "win_hdtypebox.h"
#include "../../emu.h"
#include "../../labels.h"
#include "../../utility.h"
#include "win_gui.h"
#include <commctrl.h>

namespace GUI_WIN
{

#define IDC_BUTTON	42101

HDTypeBox::HDTypeBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui, int drv, int device_type)
	: CDialogBox(hInst, IDD_HDTYPEBOX, new_font, new_emu, new_gui)
{
	m_drive = drv;
	m_device_type = device_type;
	m_count = 0;
}

HDTypeBox::~HDTypeBox()
{
}

INT_PTR HDTypeBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	// layout
	_TCHAR label[128];
	HWND rb[3];

	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin);

	if (IS_SASI_DRIVE(m_drive)) {
		int sdrv = m_drive / SASI_UNITS_PER_CTRL;
		UTILITY::stprintf(label, sizeof(label),
			CMSGM(Select_a_device_type_on_SASI_disk_VDIGIT),
			sdrv);
	} else {
		UTILITY::stprintf(label, sizeof(label),
			CMSGM(Select_a_device_type_on_SCSI_disk_VDIGIT),
			TO_SCSI_DRIVE(m_drive));
	}
	CreateStatic(box_all, IDC_STATIC, label);

	CBox *hbox = new CBox(CBox::HorizontalBox, 0, margin);
	box_all->AddBox(hbox);

	// radio button
	int i;
	for(i = 0; LABELS::hd_device_type[i]; i++) {
		rb[i] = CreateRadioButton(hbox, IDC_BUTTON + i, LABELS::hd_device_type[i], (i == 0));
	}
	m_count = i;

	CheckDlgButton(hDlg, IDC_BUTTON + m_device_type, true);

	CreateStatic(box_all, IDC_STATIC, CMsg::Need_restart_program_or_PowerOn);

	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::RightPos, margin);
	box_all->AddBox(box_btn);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	box_all->Realize(*this);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR HDTypeBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
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

INT_PTR HDTypeBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	for(int i=0; i<m_count; i++) {
		if (IsDlgButtonChecked(hDlg, IDC_BUTTON + i) == BST_CHECKED) {
			m_device_type = i;
			break;
		}
	}

	return (INT_PTR)TRUE;
}

int HDTypeBox::GetDeviceType() const
{
	return m_device_type;
}

}; /* namespace GUI_WIN */
