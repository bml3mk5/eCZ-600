/** @file win_configbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ config box ]
*/

#ifndef WIN_CONFIGBOX_H
#define WIN_CONFIGBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

namespace GUI_WIN
{

/**
	@brief Config dialog box
*/
class ConfigBox : public CDialogBox
{
private:
	HINSTANCE  hInstance;

	int        fdd_type;

	int	       io_port;
#if defined(_MBS1)
	int        sys_mode;
#endif
//	int        main_ram_size_num;

	CPtrList<CTchar> lang_list;

	CTabItems tab_items;

	int selected_tabctrl;

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onNotify(UINT, WPARAM, LPARAM);
	INT_PTR onOK(UINT, WPARAM, LPARAM);

	void select_tabctrl(int tab_num);

public:
	ConfigBox(HINSTANCE, CFont *, EMU *, GUI *);
	~ConfigBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_CONFIGBOX_H */
