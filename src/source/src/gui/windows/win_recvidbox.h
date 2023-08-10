/** @file win_recvidbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.30 -

	@brief [ record video box ]
*/

#ifndef WIN_RECVIDEOBOX_H
#define WIN_RECVIDEOBOX_H

#include <windows.h>
#include "win_dialogbox.h"
#include "../../config.h"

namespace GUI_WIN
{

#define WIN_RECVIDEO_LIBS 5

/**
	@brief Record video dialog box
*/
class RecVideoBox : public CDialogBox
{
private:
	int typnum;
	int codnums[WIN_RECVIDEO_LIBS];
	int quanums[WIN_RECVIDEO_LIBS];
	int cod_ids[WIN_RECVIDEO_LIBS];
	int qua_ids[WIN_RECVIDEO_LIBS];
	bool enables[WIN_RECVIDEO_LIBS];
	bool cont;

	struct st_item_range {
		int first;
		int last;
	} item_range[WIN_RECVIDEO_LIBS];

	void select_tabctrl(int tab_num);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onNotify(UINT, WPARAM, LPARAM);
	INT_PTR onOK(UINT, WPARAM, LPARAM);

public:
	RecVideoBox(HINSTANCE, CFont *, EMU *, GUI *);
	~RecVideoBox();
	INT_PTR Show(HWND hWnd, bool continuous = false);

};

}; /* namespace GUI_WIN */

#endif /* WIN_RECVIDEOBOX_H */
