/** @file sdl_cbitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/
#include "sdl_cbitmap.h"
#include "sdl_csurface.h"

CBitmapBase::CBitmapBase()
	: CSurface()
{
}

CBitmapBase::CBitmapBase(CBitmapBase &src, int x, int y, int w, int h)
	: CSurface()
{
	this->Copy(src, x, y, w, h);
}

CBitmapBase::~CBitmapBase()
{
}

bool CBitmapBase::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	return false;
}

bool CBitmapBase::Copy(CBitmapBase &src, int x, int y, int w, int h)
{
	VmRectWH re;

	re.x = x;
	re.y = y;
	re.w = w;
	re.h = h;

	CPixelFormat fmt = src.GetPixelFormat();
	bool enable = Create(w, h, fmt);
	if (enable) {
		src.Blit(re, *this, re);
	}
	return enable;
}
