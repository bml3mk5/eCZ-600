/** @file win_cbitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/
#include <windows.h>
#include <gdiplus.h>
#include "win_cbitmap.h"
#include "win_main.h"

#pragma comment(lib, "gdiplus.lib")

CBitmap::CBitmap()
{
	hDC = NULL;
	hBmp = NULL;
	token = NULL;

	enable = false;
}
CBitmap::CBitmap(DWORD id, int type)
{
	Create(id, type);
}
CBitmap::CBitmap(CBitmap &src, int x, int y, int w, int h)
{
	Create(src, x, y, w, h);
}

CBitmap::~CBitmap()
{
	if (enable) {
		Release();
	}
}

/// create bitmap
bool CBitmap::Create(HINSTANCE hInst, HDC hdc, DWORD id, int type)
{
	hDC = NULL;
	hBmp = NULL;
	token = NULL;

	enable = false;

	bool rc = false;
	switch(type) {
	case 1:
		rc = this->Load(hInst, hdc, id);
		break;
	default:
		rc = this->LoadBMP(hInst, hdc, id);
		break;
	}
	return rc;
}

/// create bitmap
bool CBitmap::Create(DWORD id, int type)
{
	HDC hdc = ::GetDC(hMainWindow);
	bool rc = Create(hInstance, hdc, id, type);
	::ReleaseDC(hMainWindow, hdc);
	return rc;
}

/// create bitmap
bool CBitmap::Create(HDC hdc, CBitmap &src, int x, int y, int w, int h)
{
	hDC = NULL;
	hBmp = NULL;
	token = NULL;

	enable = false;

	return this->Copy(hdc, src, x, y, w, h);
}

/// create bitmap
bool CBitmap::Create(CBitmap &src, int x, int y, int w, int h)
{
	HDC hdc = ::GetDC(hMainWindow);
	bool rc = this->Copy(hdc, src, x, y, w, h);
	::ReleaseDC(hMainWindow, hdc);
	return rc;
}

/// release bitmap
void CBitmap::Release()
{
	if (hDC) {
		DeleteDC(hDC);
		hDC = NULL;
	}
	if (hBmp) {
		DeleteObject(hBmp);
		hBmp = NULL;
	}

	enable = false;
}

bool CBitmap::LoadBMP(HINSTANCE hInst, HDC hdc, DWORD id)
{
	enable = false;

	BITMAPINFO bInfo;
	ZeroMemory(&bInfo, sizeof(bInfo));
	bInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(id));
	if (hBmp == NULL) return enable;
	GetDIBits(hdc, hBmp, 0, 1, NULL, &bInfo, DIB_RGB_COLORS);
	if (!SetDC(hdc)) return enable;
	SetSize(bInfo);

	enable = true;

	return enable;
}

bool CBitmap::Load(HINSTANCE hInst, HDC hdc, DWORD id)
{
	enable = false;

	HRSRC hRsrc = FindResource(hInst, MAKEINTRESOURCE(id), _T("IMAGE"));
	if(hRsrc == NULL) return enable;

	HGLOBAL hResource = LoadResource(hInst, hRsrc);
	if(hResource == NULL) return enable;

	const void* pResourceData = LockResource(hResource);
	if(pResourceData == NULL) return enable;

	if (!Startup()) return enable;

	DWORD dwResourceSize = SizeofResource(hInst, hRsrc);
	HGLOBAL hResourceBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
	if(hResourceBuffer == NULL) return enable;

	void* pResourceBuffer = GlobalLock(hResourceBuffer);
	if(pResourceBuffer != NULL) {

		CopyMemory(pResourceBuffer, pResourceData, dwResourceSize);
		IStream* pIStream = NULL;
		if(CreateStreamOnHGlobal(hResourceBuffer, FALSE, &pIStream) == S_OK) {
			Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromStream(pIStream);
			if(pBitmap != NULL) {
				Gdiplus::Color color(0, 0, 0);
				if (pBitmap->GetHBITMAP(color, &hBmp) == Gdiplus::Ok) {
					width = pBitmap->GetWidth();
					height = pBitmap->GetHeight();
					if (SetDC(hdc)) {
						enable = true;
					}
				}
				delete pBitmap;
			}
		}

	}
	GlobalUnlock(hResourceBuffer);
	GlobalFree(hResourceBuffer);

	Shutdown();

	return enable;
}

bool CBitmap::SetDC(HDC hdc)
{
	hDC = CreateCompatibleDC(hdc);
	if (hDC == NULL) return false;
	SelectObject(hDC, hBmp);
	return true;
}

void CBitmap::SetSize(BITMAPINFO &info)
{
	width = (int)info.bmiHeader.biWidth;
	height = (int)info.bmiHeader.biHeight;
}

bool CBitmap::Copy(HDC hdc, CBitmap &src, int x, int y, int w, int h)
{
	enable = false;

	BITMAPINFO bInfo;
	ZeroMemory(&bInfo, sizeof(bInfo));
	bInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hdc, src.hBmp, 0, 1, NULL, &bInfo, DIB_RGB_COLORS);
	bInfo.bmiHeader.biWidth = w;
	bInfo.bmiHeader.biHeight = h;

	hBmp = CreateDIBitmap(hdc, &bInfo.bmiHeader, 0, NULL, NULL, DIB_RGB_COLORS);
	if (hBmp == NULL) return enable;
	SetSize(bInfo);
	if (!SetDC(hdc)) return enable;

	BitBlt(hDC, 0, 0, width, height, src.hDC, x, y, SRCCOPY);

	enable = true;

	return enable;
}

bool CBitmap::Startup()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	return (Gdiplus::GdiplusStartup(&token, &gdiplusStartupInput, NULL) == Gdiplus::Ok);
}

void CBitmap::Shutdown()
{
	Gdiplus::GdiplusShutdown(token);
	token = NULL;
}
