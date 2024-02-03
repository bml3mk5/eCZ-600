/** @file cocoa_vkeyboard.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2017.01.22 -

 @brief [ virtual keyboard ]
 */

#import "cocoa_vkeyboard.h"
#import "cocoa_basepanel.h"
#import "../../emu.h"
#import "../gui.h"
#import "cocoa_key_trans.h"

extern EMU *emu;
extern GUI *gui;

@implementation CocoaVKeyboardView
- (id)initWithSurface:(SDL_Surface *)surface
{
	[super init];

	suf = surface;
	img = nil;

	if (suf != NULL) {
		img = [self allocBuffer];
	}
	if (img != nil) {
		// set size of this view
		[self setFrameSize:[img size]];
		[self copyBuffer];
	} else {
		// cannot convert
		suf = NULL;
	}

	return self;
}
- (void)drawRect:(NSRect)dirtyRect
{
	if (img != nil) {
//		[img drawInRect:dirtyRect];
		[img drawInRect:dirtyRect fromRect:dirtyRect operation:NSCompositingOperationCopy fraction:1.0 respectFlipped:NO hints:nil];
	}
	[self setNeedsDisplay:NO];
}
- (NSBitmapImageRep *)allocBuffer
{
	int bps = (int)suf->format->BitsPerPixel / suf->format->BytesPerPixel;
	int spp = 3; // RGB // surface->format->BytesPerPixel;
	int bf = NSAlphaFirstBitmapFormat; // NS32BitLittleEndianBitmapFormat; // NSBitmapFormat;
	return [[NSBitmapImageRep alloc]
		   initWithBitmapDataPlanes:NULL
		   pixelsWide:suf->w
		   pixelsHigh:suf->h
		   bitsPerSample:bps
		   samplesPerPixel:spp
		   hasAlpha:NO isPlanar:NO
		   colorSpaceName:NSDeviceRGBColorSpace
		   bitmapFormat:bf
		   bytesPerRow:suf->pitch
		   bitsPerPixel:suf->format->BitsPerPixel];
}
- (void)copyBuffer
{
	if (suf == NULL || img == nil) return;

	unsigned char *buf = [img bitmapData];
	int size = suf->pitch * suf->h;
	memcpy(buf, suf->pixels, size);

	[self setNeedsDisplay:YES];
}
- (void)copyBufferPart:(NSRect)re
{
	if (suf == NULL || img == nil) return;

	unsigned char *dstbuf = [img bitmapData];
	unsigned char *srcbuf = (unsigned char *)suf->pixels;
	int start = suf->pitch * (int)re.origin.y;
	srcbuf += start;
	dstbuf += start;
	int size = suf->pitch * re.size.height;
	memcpy(dstbuf, srcbuf, size);

	re.origin.y = suf->h - re.size.height - re.origin.y;

	[self setNeedsDisplayInRect:re];
}
@end

@implementation CocoaVKeyboard
- (id)initWithSurface:(Vkbd::VKeyboard *)obj surface:(SDL_Surface *)suf
{
	[super init];

	vkeyboard = obj;

	[self setTitle:@"Virtual Keyboard"];

	CocoaVKeyboardView *nview = [[CocoaVKeyboardView alloc] initWithSurface:suf];
	// adjust view size in the window
	NSRect nre = [nview frame];
	NSRect ore = [[self contentView] frame];
	NSRect wre = [self frame];
	wre.size.width += (nre.size.width - ore.size.width);
	wre.size.height += (nre.size.height - ore.size.height);
	[self setFrame:wre display:YES];
	[self setContentView:nview];
	[self setOpaque:YES];

	// always top  
	[self setLevel:NSFloatingWindowLevel];

	// window style
	NSUInteger style = [self styleMask];
	style |= (NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable);
	style &= ~(NSWindowStyleMaskFullScreen | NSWindowStyleMaskResizable); // | NSWindowStyleMaskFullSizeContentView);
	[self setStyleMask:style];

	// set delegate
//	[self setDelegate:[[CocoaVKeyboardDelegate alloc] init]];

//	// hide
//	[self orderOut:nil];

	return self;
}

- (void)close
{
	[super close];
	vkeyboard->PostClose();
}

- (void)setDist
{
	vkeyboard->SetDist();
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint pt = [theEvent locationInWindow];
	NSRect re = [[self contentView] frame];
	vkeyboard->MouseDown(pt.x,  re.size.height - pt.y);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	vkeyboard->MouseUp();
}

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
}
@end

#if 0
@implementation CocoaVKeyboardDelegate
- (void)windowDidMove:(NSNotification *)notification
{
	CocoaVKeyboard *vKbdWin = [notification object];
	[vKbdWin setDist];
}
@end
#endif

void vkeyboard_set_owner_window(NSWindow *owner)
{
	// send window object to vkeyboard
	if (owner) {
		NSArray *windows = [NSApp windows];
		for(int i=0; i<[windows count]; i++) {
			NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
			if ([[window className] isEqualToString:@"CocoaVKeyboard"]) {
				[window performSelector:@selector(setOwnerWindow:) withObject:owner];
				[window performSelector:@selector(move)];
				long level = [owner level];
				if (level > 0) {
					[window setLevel:level+1];
				} else {
					[window setLevel:NSFloatingWindowLevel];
				}
			}
		}
	}
}

//
// for Cocoa
//

namespace Vkbd {

VKeyboard::VKeyboard() : OSDBase()
{
	vKbdWin = NULL;
	parentWin = NULL;

//	SDL_QZ_InitOSKeymap();
}

VKeyboard::~VKeyboard()
{
	if (vKbdWin) {
		[vKbdWin release];
		vKbdWin = NULL;
	}
}

bool VKeyboard::Create(const char *res_path)
{
	NSArray *windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if ([[window className] hasPrefix:@"SDL"]) {
			parentWin = window;
			break;
		}
	}

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	vKbdWin = [[CocoaVKeyboard alloc] initWithSurface:this surface:pSurface->Get()];
	SetDist();
	closed = false;

	return true;
}

void VKeyboard::Show(bool show)
{
	if (!vKbdWin) return;
	Base::Show(show);

	[vKbdWin orderFront:nil];
	[vKbdWin makeKeyWindow];
}

void VKeyboard::Close()
{
	if (!vKbdWin) return;

	[vKbdWin close];
}

void VKeyboard::PostClose()
{
	vKbdWin = NULL;
	unload_bitmap();
	CloseBase();
}

void VKeyboard::SetDist()
{
	if (!vKbdWin || !parentWin) return;

	NSRect pre = [parentWin frame];
	NSRect re = [vKbdWin frame];
	NSPoint ori;

	ori.x = ((pre.size.width - re.size.width) / 2) + pre.origin.x;
	ori.y = pre.origin.y - re.size.height;

	[vKbdWin setFrameOrigin:ori];
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	need_update_window_base(info, onoff);

	if (vKbdWin) {
		CocoaVKeyboardView *vw = (CocoaVKeyboardView *)[vKbdWin contentView];
		NSRect re = NSMakeRect(info->re.left, info->re.top, info->re.right - info->re.left, info->re.bottom - info->re.top);
		[vw copyBufferPart:re];
	}
}

void VKeyboard::update_window()
{
	if (!pSurface) return;

	if (vKbdWin) {
//		CocoaVKeyboardView *vw = (CocoaVKeyboardView *)[vKbdWin contentView];
//		[vw copyBuffer];
		if (gui->IsFullScreen()) {
			[vKbdWin displayIfNeeded];
			[vKbdWin display];
		}
	}
}

} /* namespace Vkbd */
