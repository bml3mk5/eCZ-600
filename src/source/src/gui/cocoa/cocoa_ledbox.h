/** @file cocoa_ledbox.h

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.12.20 -

 @brief [ led box ]
 */

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#ifndef COCOA_LEDBOX_H
#define COCOA_LEDBOX_H

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif /* __OBJC__ */

#include "../ledbox.h"

#define NO_TITLEBAR

#ifdef __OBJC__

class LedBox;

/**
	@brief Led indicator view
*/
@interface CocoaLedBoxView : NSView
{
	SDL_Surface *suf;
	NSBitmapImageRep *img;
}
- (id)initWithSurface:(SDL_Surface *)surface;
- (void)drawRect:(NSRect)dirtyRect;
- (NSBitmapImageRep *)allocBuffer;
- (void)copyBuffer;
@end

/**
	@brief Led indicator window
*/
@interface CocoaLedBox : NSWindow
{
	LedBox *ledbox;
}
- (id)initWithSurface:(LedBox *)obj surface:(SDL_Surface *)suf;
- (void)dialogOk:(id)sender;
- (void)move;
- (void)setDist;
- (void)setOwnerWindow:(id)parent;
#ifdef NO_TITLEBAR
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
#endif
@end

/**
	@brief Delegate for led indicator window
*/
@interface CocoaLedBoxDelegate : NSObject <NSWindowDelegate>
- (void)windowDidMove:(NSNotification *)notification;
@end

void ledbox_set_owner_window(NSWindow *owner);

#endif /* __OBJC__ */

/**
	@brief LedBox is the window that display the access indicator outside the main window.
*/
class LedBox : public LedBoxBase
{
private:
#ifdef __OBJC__
	CocoaLedBox *ledBoxWin;
	NSWindow *parentWin;
#else
	void *ledBoxWin;
	void *parentWin;
#endif
#ifdef NO_TITLEBAR
	VmPoint pStart;
#endif

	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

public:
	LedBox();
	~LedBox();

	void CreateDialogBox();
	void MouseDown();
	void MouseMove();
	void Move();
	void SetDist();
	void SetOwnerWindow(void *parent);
};

#endif /* COCOA_LEDBOX_H */

#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
