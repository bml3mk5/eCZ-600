/** @file cocoa_rec_video.mm

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.03 -

 @brief [ record video using Cocoa ]
 */

#import "cocoa_rec_video.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_COCOA)

#import "../../emu.h"
#import "../rec_video.h"
#import "../../csurface.h"

COCOA_REC_VIDEO::COCOA_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;
}

COCOA_REC_VIDEO::~COCOA_REC_VIDEO()
{

}

bool COCOA_REC_VIDEO::Capture(int type, CSurface *surface, const char *file_name)
{
	// save PNG file

	// allocate bitmap buffer
	int bps = (int)surface->BitsPerPixel() / surface->BytesPerPixel();
	int spp = 3; // RGB // surface->format->BytesPerPixel;
	int bf = NSAlphaFirstBitmapFormat; // NS32BitLittleEndianBitmapFormat; // NSBitmapFormat;
	NSBitmapImageRep *img = [[NSBitmapImageRep alloc]
			initWithBitmapDataPlanes:NULL // (unsigned char * _Nullable *)surface->pixels
			pixelsWide:surface->Width()
			pixelsHigh:surface->Height()
			bitsPerSample:bps
			samplesPerPixel:spp
			hasAlpha:NO isPlanar:NO
			colorSpaceName:NSDeviceRGBColorSpace
			bitmapFormat:bf
			bytesPerRow:surface->BytesPerLine()
			bitsPerPixel:surface->BitsPerPixel()];
	if (img == nil) return false;

	// copy buffer
	unsigned char *buf = [img bitmapData];
	int size = surface->BytesPerLine() * surface->Height();
	memcpy(buf, surface->GetBuffer(), size);

	// convert format
	NSDictionary *properties = @{
		NSImageInterlaced : @0
	};
	NSData *data = [img representationUsingType:NSPNGFileType
									 properties:properties];

	// write to file
	BOOL rc = [data writeToFile:[NSString stringWithUTF8String:file_name] atomically:YES];

	return rc;
}

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_COCOA */

