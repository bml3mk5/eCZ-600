/** @file win_bitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef WIN_BITMAP_H
#define WIN_BITMAP_H

#include "../../cbitmap.h"
#include "../../depend.h"
#include <windows.h>

/**
	@brief manage bitmap
*/
class CBitmap : public CBitmapBase
{
protected:
	ULONG_PTR token;
	bool Startup();
	void Shutdown();

public:
	CBitmap();
	CBitmap(const _TCHAR *file_name, CPixelFormat *format);
	CBitmap(CBitmap &src, int x, int y, int w, int h);
	~CBitmap();

	bool Load(const _TCHAR *file_name, CPixelFormat *format);
};

#endif /* WIN_BITMAP_H */
