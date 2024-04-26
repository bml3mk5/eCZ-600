/** @file win_seldrvbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.06 -

	@brief [ select drive box ]
*/

#include "win_seldrvbox.h"
#include "../../emu.h"
#include "../../utility.h"
#include <commctrl.h>

namespace GUI_WIN
{

#define IDC_BUTTON	42101

SelDrvBox::SelDrvBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_SELDRVBOX, new_font, new_emu, new_gui)
{
	def_drv = 0;
	UTILITY::tcscpy(prefix, sizeof(prefix) / sizeof(prefix[0]), CMSG(Drive));
}

SelDrvBox::~SelDrvBox()
{
}

INT_PTR SelDrvBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDialogBox::onInitDialog(message, wParam, lParam);

	// layout
	_TCHAR label[64];
	HWND bh[USE_FLOPPY_DISKS];

	CBox *box_all = new CBox(CBox::HorizontalBox, 0, margin);

	// button
	for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
		int pdrv = (drv == 0 ? def_drv : (drv == def_drv ? 0 : drv));
		UTILITY::sprintf_utf8(label, 64, _T("%s%d"), prefix, pdrv);
		bh[pdrv] = CreateButton(box_all, IDC_BUTTON + pdrv, label, 6);
	}
	box_all->Realize(*this);

	if (def_drv != 0) {
		RECT re0, re;
		GetControlPos(IDC_BUTTON, &re0);
		GetControlPos(IDC_BUTTON + def_drv, &re);
		::SetWindowPos(bh[def_drv], NULL, re0.left, re0.top, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
		::SetWindowPos(bh[0], NULL, re.left, re.top, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
	}

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR SelDrvBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	int id = (int)LOWORD(wParam);
	if (id >= IDC_BUTTON && id < (IDC_BUTTON + USE_FLOPPY_DISKS)) {
		::EndDialog(hDlg, id - IDC_BUTTON);
		hDlg = NULL;
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

void SelDrvBox::SetDrive(int val)
{
	def_drv = val;
}

void SelDrvBox::SetPrefix(const _TCHAR *val)
{
	UTILITY::tcscpy(prefix, sizeof(prefix) / sizeof(prefix[0]), val);
}

}; /* namespace GUI_WIN */
