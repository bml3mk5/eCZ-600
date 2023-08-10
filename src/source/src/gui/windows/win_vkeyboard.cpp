/** @file win_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include "win_vkeyboard.h"

#ifdef _MSC_VER
#pragma comment(lib, "imm32.lib")
#endif

#include "../../emu.h"
#include "../../res/resource.h"

#if defined(USE_SDL2)
#include "win_key_trans.h"
#endif

extern EMU *emu;

namespace Vkbd {

//
// for windows
//
VKeyboard::VKeyboard(HINSTANCE hInst, HWND hWnd) : OSDBase()
{
	hInstance = hInst;
	hParent = hWnd;
	hVkbd = NULL;
#if defined(USE_SDL) || defined(USE_SDL2)
	hSufDC = NULL;
	hSufBmp = NULL;
	memset(&bmiSuf, 0, sizeof(bmiSuf));
	bmiSuf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
#endif
}

VKeyboard::~VKeyboard()
{
}

void VKeyboard::Show(bool show)
{
	if (!hVkbd) return;

	Base::Show(show);
	::ShowWindow(hVkbd, show ? SW_SHOWNORMAL : SW_HIDE);
}

#if defined(USE_WIN)
bool VKeyboard::Create()
{
	if (hVkbd) return true;

	hVkbd = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_VKEYBOARD), hParent, Proc, (LPARAM)this);
	if (hVkbd) {
		load_bitmap();
		create_surface();
		adjust_window_size();
		set_dist();
		closed = false;
	}
	return true;
}
#elif defined(USE_SDL) || defined(USE_SDL2)
bool VKeyboard::Create(const _TCHAR *res_path)
{
	if (hVkbd) return true;

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	hVkbd = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_VKEYBOARD), hParent, Proc, (LPARAM)this);
	if (hVkbd) {
		adjust_window_size();
		set_dist();
		closed = false;
	}
	return true;
}
#endif

void VKeyboard::Close()
{
	if (!hVkbd) return;

#if defined(USE_SDL) || defined(USE_SDL2)
	if (hSufBmp) {
		::DeleteObject(hSufBmp);
		hSufBmp = NULL;
	}
	if (hSufDC) {
		::ReleaseDC(hVkbd, hSufDC);
	}
#endif

	::EndDialog(hVkbd, 0);
	hVkbd = NULL;

	unload_bitmap();

	CloseBase();
}

INT_PTR CALLBACK VKeyboard::Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	PAINTSTRUCT ps;
	HDC hdc;
	VKeyboard *myObj = (VKeyboard *)GetWindowLongPtr(hDlg, DWLP_USER);

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		myObj->paint_window(hdc, &ps.rcPaint);
		EndPaint(hDlg, &ps);
		return (INT_PTR)FALSE;
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		myObj = (VKeyboard *)lParam;
		myObj->init_dialog(hDlg);
		return (INT_PTR)TRUE;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		myObj->MouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return (INT_PTR)FALSE;
	case WM_LBUTTONUP:
		myObj->MouseUp();
		return (INT_PTR)FALSE;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
#ifdef USE_ALT_F10_KEY
		return (INT_PTR)FALSE;	// not activate menu when hit ALT/F10
#endif
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (emu) {
#if defined(USE_WIN)
			emu->key_down_up(message & 1, LOBYTE(wParam), (long)lParam);
#elif defined(USE_SDL)
			long scan_code = (long)((lParam & 0x1ff0000) >> 16);
			emu->key_down_up(message & 1, LOBYTE(wParam), scan_code);
#elif defined(USE_SDL2)
			UINT32 scan_code = GUI_WIN::translate_to_sdlkey(wParam, lParam);
			emu->key_down_up(message & 1, (int)wParam, (short)scan_code);
#endif
		}
		return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			myObj->Close();
			return (INT_PTR)TRUE;
		}
		break;
	case WM_CLOSE:
		myObj->Close();
		return (INT_PTR)TRUE;
	case WM_MENUCHAR:
		// ignore accel key and suppress beep
		::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, MNC_CLOSE << 16);
		return (INT_PTR)TRUE;
//	case WM_DESTROY:
//		break;
	}
	return (INT_PTR)FALSE;
}

void VKeyboard::adjust_window_size()
{
	if (!hVkbd || !pSurface) return;

	WINDOWINFO wi;

	// calc client size in the window
	GetWindowInfo(hVkbd, &wi);
	int width = wi.rcClient.left - wi.rcWindow.left + wi.rcWindow.right - wi.rcClient.right + pSurface->Width();
	int height = wi.rcClient.top - wi.rcWindow.top + wi.rcWindow.bottom - wi.rcClient.bottom + pSurface->Height();
	::SetWindowPos(hVkbd, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void VKeyboard::set_dist()
{
	if (!hVkbd || !hParent) return;

	WINDOWINFO wip;
	WINDOWINFO wi;

	HDC hdcScr = ::GetDC(NULL);
//	int desktop_width = ::GetDeviceCaps(hdcScr, HORZRES);
	int desktop_height = ::GetDeviceCaps(hdcScr, VERTRES);
	::ReleaseDC(NULL, hdcScr);

	::GetWindowInfo(hParent, &wip);
	::GetWindowInfo(hVkbd, &wi);

	int wp = wip.rcWindow.right - wip.rcWindow.left;
//	int hp = wip.rcWindow.bottom - wip.rcWindow.top;
	int w = wi.rcWindow.right - wi.rcWindow.left;
	int h = wi.rcWindow.bottom - wi.rcWindow.top;

	int x = (wp - w) / 2 + wip.rcWindow.left;
	int y = wip.rcWindow.bottom;

	if (y + h > desktop_height) {
		y = (desktop_height - h);
	}

	::SetWindowPos(hVkbd, HWND_TOPMOST, x, y, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!hVkbd) return;

	need_update_window_base(info, onoff);

	RECT re = { info->re.left, info->re.top, info->re.right, info->re.bottom };
	::InvalidateRect(hVkbd, &re, TRUE);
}

void VKeyboard::update_window()
{
	if (!hVkbd) return;

	::UpdateWindow(hVkbd);
}

void VKeyboard::paint_window(HDC hdc, RECT *re)
{
	if (!pSurface) return;

#if defined(USE_WIN)
	::BitBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top, pSurface->GetDC(), re->left, re->top, SRCCOPY);
#elif defined(USE_SDL) || defined(USE_SDL2)
	scrntype *p = (scrntype *)pSurface->GetBuffer();
	p += (re->top * pSurface->Width());
	::SetDIBits(hdc, hSufBmp, pSurface->Height() - re->bottom, re->bottom - re->top, p, &bmiSuf, DIB_RGB_COLORS);
	::BitBlt(hdc, re->left, re->top, re->right - re->left, re->bottom - re->top, hSufDC, re->left, re->top, SRCCOPY);
#endif
}

void VKeyboard::init_dialog(HWND hDlg)
{
	// disable ime
	ImmAssociateContext(hDlg, NULL);

	// set icon on sysmenu
	HICON hIcon = ::LoadIconA(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	::SendMessageA(hDlg, WM_SETICON, (WPARAM)NULL, (LPARAM)hIcon);

#if defined(USE_SDL) || defined(USE_SDL2)
	// create bitmap buffer
	if (pSurface) {
		HDC hdc = ::GetDC(hDlg);
		hSufDC = ::CreateCompatibleDC(hdc);
		hSufBmp = ::CreateCompatibleBitmap(hdc, pSurface->Width(), pSurface->Height());
		bmiSuf.bmiHeader.biWidth = pSurface->Width();
		bmiSuf.bmiHeader.biHeight = - pSurface->Height();	// flipped
		bmiSuf.bmiHeader.biPlanes = 1;
		bmiSuf.bmiHeader.biBitCount = pSurface->BitsPerPixel();
		bmiSuf.bmiHeader.biCompression = BI_RGB;
		bmiSuf.bmiHeader.biSizeImage = pSurface->Width() * pSurface->Height() * pSurface->BytesPerPixel();
		::SelectObject(hSufDC, hSufBmp);
		::ReleaseDC(hDlg, hdc);
	}
#endif
}

} /* namespace Vkbd */
