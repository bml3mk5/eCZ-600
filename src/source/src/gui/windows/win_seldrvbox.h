/** @file win_seldrvbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.06 -

	@brief [ select drive box ]
*/

#ifndef WIN_SELDRVBOX_H
#define WIN_SELDRVBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

/**
	@brief Select drive dialog box
*/
class SelDrvBox : public CDialogBox
{
private:
	int def_drv;
	_TCHAR prefix[32];

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);

public:
	SelDrvBox(HINSTANCE, CFont *, EMU *, GUI *);
	~SelDrvBox();
	void SetDrive(int);
	void SetPrefix(const _TCHAR *);
};

}; /* namespace GUI_WIN */

#endif /* WIN_SELDRVBOX_H */
