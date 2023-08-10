/** @file win_ledbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#ifndef WIN_LEDBOX_H
#define WIN_LEDBOX_H

#include "../ledbox.h"
#include "../../depend.h"
#include <windows.h>
#include "../../res/resource.h"

#define NO_TITLEBAR

/**
	@brief LedBox is the window that display the access indicator outside the main window.
*/
class LedBox : public LedBoxBase
{
private:
	HWND		hLedBox;
	HINSTANCE	hInstance;
	HWND		hParent;

#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
	HDC         hSufDC;
	HBITMAP     hSufBmp;
	BITMAPINFO  bmiSuf;
#endif

	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

#ifdef NO_TITLEBAR
	void mouse_move(HWND, const POINT &, const LPARAM &);
#else
	void set_dist(HWND);
#endif
	void adjust_dialog_size(HWND);
	void update_dialog(HDC);

	static INT_PTR CALLBACK LedBoxProc(HWND, UINT, WPARAM, LPARAM);

public:
	LedBox();
	~LedBox();

	void SetHandle(HINSTANCE hInst, HWND hWnd);
	void CreateDialogBox();
	void Move();
};

#endif /* WIN_LEDBOX_H */

#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
