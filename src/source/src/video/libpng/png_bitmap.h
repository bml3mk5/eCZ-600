/** @file png_bitmap.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2017.02.26 -

 @brief [ cbitmap ]
 */

#ifndef _PNG_BITMAP_H_
#define _PNG_BITMAP_H_

#include "../../cbitmap.h"

/**
	@brief manage bitmap
*/
class CBitmap : public CBitmapBase
{
public:
	CBitmap();
	CBitmap(const _TCHAR *file_name, CPixelFormat *format);
	~CBitmap();

	bool Load(const _TCHAR *file_name, CPixelFormat *format);
};

#endif /* _PNG_BITMAP_H_ */
