/** @file sdl_cbitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef SDL_CBITMAP_BASE_H
#define SDL_CBITMAP_BASE_H

#include "../../common.h"
#include "sdl_csurface.h"

/**
	@brief manage bitmap
*/
class CBitmapBase : public CSurface
{
public:
	CBitmapBase();
	CBitmapBase(CBitmapBase &src, int x, int y, int w, int h);
	virtual ~CBitmapBase();

	virtual	bool Load(const _TCHAR *file_name, CPixelFormat *format);
	virtual bool Copy(CBitmapBase &src, int x, int y, int w, int h);
};

#if defined(USE_WX) || defined(USE_WX2)
#include "../../video/wxwidgets/wx_bitmap.h"
#elif defined(_WIN32)
#include "../../video/windows/win_bitmap.h"
#elif defined(__MACH__) && defined(__APPLE__)
#include "../../video/cocoa/cocoa_bitmap.h"
#else
#include "../../video/libpng/png_bitmap.h"
#endif

#endif /* SDL_CBITMAP_BASE_H */
