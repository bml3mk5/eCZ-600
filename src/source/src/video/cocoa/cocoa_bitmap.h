/** @file cocoa_bitmap.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2017.02.26 -

 @brief [ cbitmap ]
 */

#ifndef _COCOA_BITMAP_H_
#define _COCOA_BITMAP_H_

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif
#include "../../common.h"
#include "../../cbitmap.h"

/**
	@brief manage bitmap
*/
class CBitmap : public CBitmapBase
{
public:
	CBitmap();
	CBitmap(const _TCHAR *file_name, CPixelFormat *format);
	CBitmap(CBitmap &src, int x, int y, int w, int h);
	~CBitmap();

	bool Load(const _TCHAR *file_name, CPixelFormat *format);
};

#endif /* _COCOA_BITMAP_H_ */
