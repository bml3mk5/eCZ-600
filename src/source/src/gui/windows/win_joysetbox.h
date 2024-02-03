/** @file win_joysetbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2023.01.07 -

	@brief [ joypad setting box ]
*/

#ifndef WIN_JOYSETBOX_H
#define WIN_JOYSETBOX_H

#include <windows.h>
#include "win_keybindctrl.h"
#include "win_dialogbox.h"
#include "../../config.h"
#include <vector>

namespace GUI_WIN
{

/**
	@brief Volume dialog box
*/
class JoySettingBox : public CDialogBox
{
private:
	std::vector<KeybindControl *> kbctl;

	int selected_tabctrl;

	uint32_t enable_axes;

	static INT_PTR CALLBACK JoySettingBoxProc(HWND, UINT, WPARAM, LPARAM);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onHScroll(UINT, WPARAM, LPARAM);
	INT_PTR onVScroll(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onNotify(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onMouseWheel(UINT message, WPARAM wParam, LPARAM lParam);
//	INT_PTR onControlColorStatic(UINT, WPARAM, LPARAM);
	INT_PTR onClickOk();
	INT_PTR onClickLoadDefault();
	INT_PTR onClickLoadPreset(int idx);
	INT_PTR onClickSavePreset(int idx);
	INT_PTR onClickAxis(int id);

	void select_tabctrl(int tab_num);

	void SetValue();
//	void SetValue(HWND ctrl);

public:
	JoySettingBox(HINSTANCE, CFont *, EMU *, GUI *);
	~JoySettingBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_JOYSETBOX_H */
