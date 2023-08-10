/** @file win_bitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/
#include <windows.h>
#include <gdiplus.h>
#include "win_bitmap.h"
#include "../../cchar.h"

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#endif

CBitmap::CBitmap()
	: CBitmapBase()
{
	token = NULL;
}

CBitmap::CBitmap(const _TCHAR *file_name, CPixelFormat *format)
	: CBitmapBase()
{
	token = NULL;

	this->Load(file_name, format);
}

CBitmap::CBitmap(CBitmap &src, int x, int y, int w, int h)
	: CBitmapBase(src, x, y, w, h)
{
	token = NULL;
}

CBitmap::~CBitmap()
{
}

bool CBitmap::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	bool enable = false;

	if (!Startup()) return enable;

	CTchar wfile_name(file_name);

	Gdiplus::Bitmap *bmp = Gdiplus::Bitmap::FromFile(wfile_name.GetW());
	if (bmp) {
		int width = bmp->GetWidth();
		int height = bmp->GetHeight();

		Gdiplus::Rect re(0, 0, width, height);
		Gdiplus::BitmapData data;
		if (bmp->LockBits(&re, Gdiplus::ImageLockModeRead, PixelFormat32bppRGB, &data) == Gdiplus::Ok) {
			if (format) {
				enable = Create(width, height, *format);
			} else {
				enable = Create(width, height, CPixelFormat::BGRA32);
			}
			if (enable) {
				SDL_LockSurface(suf);
				memcpy(suf->pixels, data.Scan0, data.Stride * data.Height);
				SDL_UnlockSurface(suf);
			}
			bmp->UnlockBits(&data);
		}

		delete bmp;
	}

	Shutdown();

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
