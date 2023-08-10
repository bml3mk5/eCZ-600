/** @file win_ledbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#include "win_ledbox.h"
#include <windowsx.h>

//
// for windows
//

LedBox::LedBox() : LedBoxBase()
{
	hLedBox = NULL;
	hInstance = NULL;
	hParent = NULL;
#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
	hSufDC = NULL;
	hSufBmp = NULL;
	memset(&bmiSuf, 0, sizeof(bmiSuf));
	bmiSuf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
#endif
}

LedBox::~LedBox()
{
#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
	if (hSufDC) ::DeleteDC(hSufDC);
	if (hSufBmp) ::DeleteObject(hSufBmp);
#endif
}

void LedBox::SetHandle(HINSTANCE hInst, HWND hWnd)
{
	hInstance = hInst;
	hParent   = hWnd;
}

void LedBox::show_dialog()
{
	if (hLedBox) {
		ShowWindow(hLedBox, (visible && !inside) ? SW_SHOWNORMAL : SW_HIDE);
		PostMessage(hParent, WM_ACTIVATE, WA_ACTIVE, 0);
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

	hLedBox = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_KBLEDBOX), hParent, LedBoxProc, (LPARAM)this);
}

INT_PTR CALLBACK LedBox::LedBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hDCtl;
	static POINT pStart;
	LedBox *myObj;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		myObj = (LedBox *)lParam;
		myObj->adjust_dialog_size(hDlg);
		return (INT_PTR)TRUE;
	}
#ifdef NO_TITLEBAR
	case WM_LBUTTONDOWN:
		pStart.x = GET_X_LPARAM(lParam);
		pStart.y = GET_Y_LPARAM(lParam);
		return (INT_PTR)FALSE;
	case WM_MOUSEMOVE:
		myObj = (LedBox *)GetWindowLongPtr(hDlg, DWLP_USER);
		if (wParam & MK_LBUTTON) {
			myObj->mouse_move(hDlg, pStart, lParam);
		}
		return (INT_PTR)FALSE;
	case WM_LBUTTONUP:
		hDCtl = GetParent(hDlg);
		SetActiveWindow(hDCtl);
		return (INT_PTR)FALSE;
#endif
	case WM_COMMAND:
		return (INT_PTR)TRUE;
#ifndef NO_TITLEBAR
	case WM_MOVE:
		myObj = (LedBox *)GetWindowLongPtr(hDlg, DWLP_USER);
		myObj->set_dist(hDlg);
		return (INT_PTR)FALSE;
#endif
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_ACTIVE) {
			hDCtl = GetParent(hDlg);
			SetActiveWindow(hDCtl);
		}
		return (INT_PTR)FALSE;
	case WM_PAINT:
		{
			myObj = (LedBox *)GetWindowLongPtr(hDlg, DWLP_USER);
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hDlg, &ps);
			myObj->update_dialog(hdc);
			EndPaint(hDlg, &ps);
			return (INT_PTR)FALSE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void LedBox::Move()
{
	WINDOWINFO wi;
	WINDOWINFO wid;

	if (hLedBox) {
		GetWindowInfo(hParent, &wi);
		GetWindowInfo(hLedBox, &wid);
		int x = wi.rcWindow.left + dist.x;
		int y = wi.rcWindow.top + dist.y;
		SetWindowPos(hLedBox, HWND_TOPMOST, x, y, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
	}
}

/// param[in] place : bit0:1=right  bit1:1=bottom  bit4:1=fullscreen
void LedBox::move_in_place(int place)
{
//	WINDOWINFO wt;
	WINDOWINFO wi;
	WINDOWINFO wid;

	if (hLedBox) {
//		HWND hRoot = GetDesktopWindow();
//		GetWindowInfo(hRoot, &wt);
		GetWindowInfo(hParent, &wi);
		GetWindowInfo(hLedBox, &wid);
		int w = wid.rcWindow.right - wid.rcWindow.left;
		int h = wid.rcWindow.bottom - wid.rcWindow.top;
		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = wi.rcWindow.right - wi.rcWindow.left - w;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = wi.rcWindow.bottom - wi.rcWindow.top - (place & 0x10 ? h : 4);
			} else {
				dist.y = 0 - (place & 0x10 ? 0 : h - 4);
			}
		}
		int x = wi.rcWindow.left + dist.x;
		int y = wi.rcWindow.top + dist.y;
//		if (y < wt.rcWindow.top) y = wt.rcWindow.top;
//		if (y + h > wt.rcWindow.bottom) y = wt.rcWindow.bottom - h;
		MoveWindow(hLedBox, x, y, w, h, true);
	}
}

#ifdef NO_TITLEBAR
void LedBox::mouse_move(HWND hDlg, const POINT &pStart, const LPARAM &lParam)
{
	WINDOWINFO wi;
	WINDOWINFO wid;

	GetWindowInfo(hParent, &wi);
	GetWindowInfo(hDlg, &wid);
	int x = GET_X_LPARAM(lParam) - pStart.x + wid.rcWindow.left;
	int y = GET_Y_LPARAM(lParam) - pStart.y + wid.rcWindow.top;
	dist.x = wid.rcWindow.left - wi.rcWindow.left;
	dist.y = wid.rcWindow.top - wi.rcWindow.top;
	SetWindowPos(hDlg, HWND_TOPMOST, x, y, 1, 1, SWP_NOSIZE | SWP_NOZORDER);
}
#endif

#ifndef NO_TITLEBAR
void LedBox::set_dist(HWND hDlg)
{
	WINDOWINFO wi;
	WINDOWINFO wid;

	GetWindowInfo(hParent, &wi);
	GetWindowInfo(hDlg, &wid);
	dist.x = wid.rcWindow.left - wi.rcWindow.left;
	dist.y = wid.rcWindow.top - wi.rcWindow.top;
}
#endif

void LedBox::adjust_dialog_size(HWND hDlg)
{
	WINDOWINFO wi;
	GetWindowInfo(hDlg, &wi);
	int width = parent_pt.w + wi.rcWindow.right - wi.rcWindow.left - wi.rcClient.right + wi.rcClient.left;
	int height = parent_pt.h + wi.rcWindow.bottom - wi.rcWindow.top - wi.rcClient.bottom + wi.rcClient.top;
	SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
	HDC hdc = ::GetDC(hDlg);
	hSufDC = ::CreateCompatibleDC(hdc);
	hSufBmp = ::CreateCompatibleBitmap(hdc, Width(), Height());
	bmiSuf.bmiHeader.biWidth = Width();
	bmiSuf.bmiHeader.biHeight = - Height();	// flipped
	bmiSuf.bmiHeader.biPlanes = 1;
	bmiSuf.bmiHeader.biBitCount = BitsPerPixel();
	bmiSuf.bmiHeader.biCompression = BI_RGB;
	bmiSuf.bmiHeader.biSizeImage =  Width() * Height() * BytesPerPixel();
	::SelectObject(hSufDC, hSufBmp);
	::ReleaseDC(hDlg, hdc);
#endif
}

void LedBox::need_update_dialog()
{
	if (hLedBox) {
		::InvalidateRect(hLedBox, NULL, TRUE);
		::UpdateWindow(hLedBox);
	}
}

void LedBox::update_dialog(HDC hdc)
{
#if defined(USE_WIN)
	::BitBlt(hdc, 0, 0, parent_pt.w, parent_pt.h, hMainDC, 0, 0, SRCCOPY);
#elif defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX)
	::SetDIBits(hdc, hSufBmp, 0, Height(), GetBuffer(), &bmiSuf, DIB_RGB_COLORS);
	::BitBlt(hdc, 0, 0, parent_pt.w, parent_pt.h, hSufDC, 0, 0, SRCCOPY);
#endif
}

#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
