/** @file win_volumebox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ volume box ]
*/

#ifndef WIN_VOLUMEBOX_H
#define WIN_VOLUMEBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"
#include <vector>

namespace GUI_WIN
{

/**
	@brief Volume dialog box
*/
class VolumeBox : public CDialogBox
{
private:
	static INT_PTR CALLBACK VolumeBoxProc(HWND, UINT, WPARAM, LPARAM);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onHScroll(UINT, WPARAM, LPARAM);
	INT_PTR onVScroll(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);

	void SetVolume();
	void SetVolumeText(int num);

	std::vector<int *> volumes;
	std::vector<bool *> mutes;

public:
	VolumeBox(HINSTANCE, CFont *, EMU *, GUI *);
	~VolumeBox();

};

}; /* namespace GUI_WIN */

#endif /* WIN_VOLUMEBOX_H */
