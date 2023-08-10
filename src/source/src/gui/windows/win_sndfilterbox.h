/** @file win_sndfilterbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.11.26 -

	@brief [ sound filter box ]
*/

#ifndef WIN_SNDFILTERBOX_H
#define WIN_SNDFILTERBOX_H

#ifdef USE_DEBUG_SOUND_FILTER

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

/**
	@brief Sound filter dialog box
*/
class SndFilterBox : public CDialogBox
{
private:
	static INT_PTR CALLBACK SndFilterBoxProc(HWND, UINT, WPARAM, LPARAM);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onHScroll(UINT, WPARAM, LPARAM);
	INT_PTR onVScroll(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);

	void SetVolume();

public:
	SndFilterBox(HINSTANCE, CFont *, EMU *, GUI *);
	~SndFilterBox();

};

}; /* namespace GUI_WIN */

#endif

#endif /* WIN_SNDFILTERBOX_H */
