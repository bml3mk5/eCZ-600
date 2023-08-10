/** @file win_keybindbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.03.31 -

	@brief [ keybind box ]
*/

#ifndef WIN_KEYBINDBOX_H
#define WIN_KEYBINDBOX_H

#include <windows.h>
#include "win_keybindctrl.h"
#include "win_dialogbox.h"
#include <vector>

class EMU;

namespace GUI_WIN
{

/**
	@brief Keybind dialog box
*/
class KeybindBox : public CDialogBox
{
private:
	std::vector<KeybindControl *> kbctl;

	int selected_tabctrl;

	uint32_t enable_axes;

	INT_PTR onInitDialog(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onCommand(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onNotify(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onMouseWheel(UINT message, WPARAM wParam, LPARAM lParam);
//	INT_PTR onControlColorStatic(UINT, WPARAM, LPARAM);
	INT_PTR onClickOk();
	INT_PTR onClickLoadDefault();
	INT_PTR onClickLoadPreset(int idx);
	INT_PTR onClickSavePreset(int idx);
	INT_PTR onClickAxis(int id);

	void select_tabctrl(int tab_num);
	int get_combi_id(KeybindControl *kbctl);

public:
	KeybindBox(HINSTANCE, CFont *, EMU *, GUI *);
	~KeybindBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_KEYBINDBOX_H */
