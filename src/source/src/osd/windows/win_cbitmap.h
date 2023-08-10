/** @file win_cbitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef _WIN_CBITMAP_H_
#define _WIN_CBITMAP_H_

#include <windows.h>
#include "../../common.h"

/**
	@brief manage bitmap
*/
class CBitmap
{
protected:
	HDC       hDC;
	HBITMAP   hBmp;

	int       width;
	int       height;
	bool      enable;

	bool SetDC(HDC hdc);
	void SetSize(BITMAPINFO &info);

	ULONG_PTR token;
	bool Startup();
	void Shutdown();

	bool Create(HINSTANCE hInst, HDC hdc, DWORD id, int type);
	bool Create(HDC hdc, CBitmap &src, int x, int y, int w, int h);

public:
	CBitmap();
	CBitmap(DWORD id, int type);
	CBitmap(CBitmap &src, int x, int y, int w, int h);
	~CBitmap();

	bool Create(DWORD id, int type);
	bool Create(CBitmap &src, int x, int y, int w, int h);
	void Release();

	bool LoadBMP(HINSTANCE hInst, HDC hdc, DWORD id);
	bool Load(HINSTANCE hInst, HDC hdc, DWORD id);
	bool Copy(HDC hdc, CBitmap &src, int x, int y, int w, int h);

	HDC GetDC() { return hDC; }
	HBITMAP GetBitmap() { return hBmp; }

	bool IsEnable() { return enable; }

	int Width() { return width; }
	int Height() { return height; }
};

#endif
