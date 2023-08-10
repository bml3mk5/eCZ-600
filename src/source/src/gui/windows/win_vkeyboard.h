/** @file win_vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef WIN_VKEYBOARD_H
#define WIN_VKEYBOARD_H

#include <windows.h>
#include <windowsx.h>
#include "../../res/resource.h"
#include "../vkeyboard.h"

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public OSDBase
{
private:
	HINSTANCE	hInstance;

	HWND		hVkbd;
	HWND		hParent;
#if defined(USE_SDL) || defined(USE_SDL2)
	HDC			hSufDC;
	HBITMAP		hSufBmp;
	BITMAPINFO	bmiSuf;
#endif
	void adjust_window_size();
	void set_dist();
	void need_update_window(PressedInfo_t *, bool);
	void update_window();
	void init_dialog(HWND);
	void paint_window(HDC, RECT *);

	static INT_PTR CALLBACK Proc(HWND, UINT, WPARAM, LPARAM);

public:
	VKeyboard(HINSTANCE, HWND);
	~VKeyboard();

#if defined(USE_WIN)
	bool Create();
#elif defined(USE_SDL) || defined(USE_SDL2)
	bool Create(const _TCHAR *res_path);
#endif
	void Show(bool = true);
	void Close();
};

} /* namespace Vkbd */

#endif /* WIN_VKEYBOARD_H */
