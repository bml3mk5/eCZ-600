/** @file cocoa_bitmap.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2017.02.26 -

 @brief [ cbitmap ]
 */

#import "cocoa_bitmap.h"

CBitmap::CBitmap()
	: CBitmapBase()
{
}

CBitmap::CBitmap(const _TCHAR *file_name, CPixelFormat *format)
	: CBitmapBase()
{
	this->Load(file_name, format);
}

CBitmap::CBitmap(CBitmap &src, int x, int y, int w, int h)
	: CBitmapBase(src, x, y, w, h)
{
}

CBitmap::~CBitmap()
{
}

bool CBitmap::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	NSString *nfile_name = [NSString stringWithUTF8String:file_name];

	NSBitmapImageRep *img = (NSBitmapImageRep *)[NSBitmapImageRep imageRepWithContentsOfFile:nfile_name];
	if (img == nil) return false;

	// copy buffer
	NSSize sz = [img size];
	CGFloat srcr,srcg,srcb;
	bool enable = false;
	if (format) {
		enable = Create(sz.width, sz.height, *format);
	} else {
		enable = Create(sz.width, sz.height, CPixelFormat::RGBA32);
	}
	if (enable) {
		Lock();
		for(int y=0; y<sz.height; y++) {
			scrntype *dst = (scrntype *)suf->pixels;
			dst += y * suf->w;
			for(int x=0; x<sz.width; x++) {
				NSColor *c = [img colorAtX:x y:y];
				[c getRed:&srcr green:&srcg blue:&srcb alpha:nil];
				srcr *= (suf->format->Rmask);
				srcg *= (suf->format->Gmask);
				srcb *= (suf->format->Bmask);

				*dst = ((scrntype)srcr)
					 | ((scrntype)srcg)
					 | ((scrntype)srcb)
					 | ((scrntype)suf->format->Amask);

				dst++;
			}
		}
		Unlock();
	}

	return enable;
}

