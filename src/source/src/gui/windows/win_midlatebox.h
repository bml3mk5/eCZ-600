/** @file win_midlatebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.12 -

	@brief [ midi latency box ]
*/

#ifndef WIN_MIDLATEBOX_H
#define WIN_MIDLATEBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

/**
	@brief midi latency box
*/
class MidLateBox : public CDialogBox
{
private:
	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR onOK(UINT message, WPARAM wParam, LPARAM lParam);

public:
	MidLateBox(HINSTANCE, CFont *, EMU *, GUI *);
	~MidLateBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_MIDLATEBOX_H */
