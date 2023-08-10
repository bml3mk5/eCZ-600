/** @file win_recaudbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio box ]
*/

#ifndef WIN_RECAUDIOBOX_H
#define WIN_RECAUDIOBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

#define WIN_RECAUDIO_LIBS 5

/**
	@brief Record audio dialog box
*/
class RecAudioBox : public CDialogBox
{
private:
	int typnum;
	int codnums[WIN_RECAUDIO_LIBS];
	int quanums[WIN_RECAUDIO_LIBS];
	int cod_ids[WIN_RECAUDIO_LIBS];
	int qua_ids[WIN_RECAUDIO_LIBS];
	bool enables[WIN_RECAUDIO_LIBS];

	struct st_item_range {
		int first;
		int last;
	} item_range[WIN_RECAUDIO_LIBS];

	void select_tabctrl(int tab_num);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onNotify(UINT, WPARAM, LPARAM);
	INT_PTR onOK(UINT, WPARAM, LPARAM);

public:
	RecAudioBox(HINSTANCE, CFont *, EMU *, GUI *);
	~RecAudioBox();
	INT_PTR Show(HWND hWnd);

};

}; /* namespace GUI_WIN */

#endif /* WIN_RECAUDIOBOX_H */
