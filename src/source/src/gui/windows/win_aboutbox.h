/** @file win_aboutbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21 -

	@brief [ about box ]
*/

#ifndef WIN_ABOUTBOX_H
#define WIN_ABOUTBOX_H

#include <Windows.h>
#include "win_dialogbox.h"

namespace GUI_WIN
{

/**
	@brief About dialog box
*/
class AboutBox : public CDialogBox
{
private:
	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);

public:
	AboutBox(HINSTANCE hinst, CFont *new_font, EMU *new_emu, GUI *new_gui);
	~AboutBox();
};

}; /* namespace GUI_WIN */

#endif /* WIN_ABOUTBOX_H */
