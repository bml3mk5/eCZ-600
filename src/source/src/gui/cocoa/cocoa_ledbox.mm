/** @file cocoa_ledbox.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.12.20 -

 @brief [ led box ]
 */

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#import "cocoa_ledbox.h"
#import "cocoa_basepanel.h"
#import "../gui.h"

extern GUI *gui;

@implementation CocoaLedBoxView
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
//		[self setFrameSize:[img size]];
		NSSize siz = [img size];
		siz.width += 2;
		siz.height += 2;
		[self setFrameSize:siz];
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
		int w = suf->w + 2;
		int h = suf->h + 2;
		NSPoint po = NSMakePoint(1,1);
		[img drawAtPoint:po];
		NSFrameRect(NSMakeRect(0,0,w,h));
	}
	[self setNeedsDisplay:NO];
}
- (NSBitmapImageRep *)allocBuffer
{
	int bps = (int)suf->format->BitsPerPixel / suf->format->BytesPerPixel;
	int spp = 3; // RGB // surface->format->BytesPerPixel;
	int bf = NS32BitLittleEndianBitmapFormat; // NSBitmapFormat;
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
@end

@implementation CocoaLedBox
- (id)initWithSurface:(LedBox *)obj surface:(SDL_Surface *)suf
{
	[super init];

	ledbox = obj;

	self.title = @"";
	// set style
#ifdef NO_TITLEBAR
	NSUInteger style = [self styleMask];
	style &= ~(NSWindowStyleMaskTitled);
	[self setStyleMask:style];
#endif

	CocoaLedBoxView *nview = [[CocoaLedBoxView alloc] initWithSurface:suf];
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

#ifndef NO_TITLEBAR
	// set delegate
	[self setDelegate:[[CocoaLedBoxDelegate alloc] init]];
#endif

	// hide
	[self orderOut:nil];

	return self;
}

- (void)dialogOk:(id)sender
{
	[super close];
}

- (void)move
{
	ledbox->Move();
}

- (void)setDist
{
	ledbox->SetDist();
}

- (void)setOwnerWindow:(id)parent
{
	ledbox->SetOwnerWindow(parent);
}

#ifdef NO_TITLEBAR
- (void)mouseDown:(NSEvent *)theEvent
{
	ledbox->MouseDown();
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	ledbox->MouseMove();
}

- (void)mouseUp:(NSEvent *)theEvent
{
	ledbox->MouseMove();
}
#endif
@end

@implementation CocoaLedBoxDelegate
- (void)windowDidMove:(NSNotification *)notification
{
	CocoaLedBox *ledBoxWin = [notification object];
	[ledBoxWin setDist];
}
@end

void ledbox_set_owner_window(NSWindow *owner)
{
	// send window object to ledbox
	if (owner) {
		NSArray *windows = [NSApp windows];
		for(int i=0; i<[windows count]; i++) {
			NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
			if ([[window className] isEqualToString:@"CocoaLedBox"]) {
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

LedBox::LedBox() : LedBoxBase()
{
	ledBoxWin = NULL;
	parentWin = NULL;
}

LedBox::~LedBox()
{
	if (ledBoxWin) [ledBoxWin release];
}

void LedBox::CreateDialogBox()
{
	NSArray *windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if ([[window className] hasPrefix:@"SDL"]) {
			SetOwnerWindow(window);
			break;
		}
	}

	ledBoxWin = [[CocoaLedBox alloc] initWithSurface:this surface:suf];
}

void LedBox::SetOwnerWindow(void *parent)
{
	parentWin = (NSWindow *)parent;
}

void LedBox::show_dialog()
{
	if (ledBoxWin) {
		if (visible && !inside) {
			[ledBoxWin orderFront:nil];
			need_update_dialog();
		} else {
			[ledBoxWin orderOut:nil];
		}
	}
}

void LedBox::MouseDown()
{
	NSPoint pt = [NSEvent mouseLocation];
	pStart.x = pt.x;
	pStart.y = pt.y;
}

void LedBox::MouseMove()
{
	NSPoint pt = [NSEvent mouseLocation];
	NSPoint pp;
	pp.x = pt.x - pStart.x;
	pp.y = pt.y - pStart.y;
	pStart.x = pt.x;
	pStart.y = pt.y;

	NSRect fre = [ledBoxWin frame];
	fre.origin.x += pp.x;
	fre.origin.y += pp.y;
	[ledBoxWin setFrame:fre display:NO];
	SetDist();
}

void LedBox::Move()
{
	if (parentWin) {
		NSRect re = [parentWin frame];
		NSRect fre = [ledBoxWin frame];

		fre.origin.x = re.origin.x + dist.x;
		fre.origin.y = re.origin.y + re.size.height - dist.y - fre.size.height;

		[ledBoxWin setFrame:fre display:NO];
	}
}

void LedBox::SetDist()
{
	if (parentWin) {
		NSRect re = [parentWin frame];
		NSRect fre = [ledBoxWin frame];

		dist.x = fre.origin.x - re.origin.x;
		dist.y = re.origin.y + re.size.height - fre.origin.y - fre.size.height;
	}
}

void LedBox::move_in_place(int place)
{
	if (parentWin) {
//		NSRect rt = [[NSScreen mainScreen] frame];
		NSRect re = [parentWin frame];
		NSRect fre = [ledBoxWin frame];
		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = re.size.width - fre.size.width;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = re.size.height - (place & 0x10 ? fre.size.height : 4);
			} else {
				dist.y = 0 - (place & 0x10 ? 0 : fre.size.height - 4);
			}
		}

		fre.origin.x = re.origin.x + dist.x;
		fre.origin.y = re.origin.y + re.size.height - dist.y - fre.size.height;
//		if (fre.origin.y < 0) fre.origin.y = 0;
//		if (fre.origin.y + fre.size.height > rt.size.height) fre.origin.y = rt.size.height - fre.size.height;

		[ledBoxWin setFrame:fre display:NO];
	}
}

void LedBox::need_update_dialog()
{
	if (ledBoxWin) {
		CocoaLedBoxView *vw = (CocoaLedBoxView *)[ledBoxWin contentView];
		[vw copyBuffer];
		if (gui->IsFullScreen()) {
			[ledBoxWin displayIfNeeded];
			[ledBoxWin display];
		}
	}
}

#else /* USE_SDL2 && USE_SDL2_LEDBOX */

#import <Cocoa/Cocoa.h>
#include "../sdl2_ledbox.h"
#include <SDL_syswm.h>

void LedBox::create_dialog_box_sub()
{
	if (wLedBox) {
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		SDL_GetWindowWMInfo(wLedBox, &info);
		NSWindow *window = info.info.cocoa.window;
		NSUInteger style = [window styleMask];
		style &= ~(NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask);
		[window setStyleMask:style];
	}
}

#endif /* USE_SDL2 && USE_SDL2_LEDBOX */

