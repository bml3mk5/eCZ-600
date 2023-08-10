/** @file cocoa_vkeyboard.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2017.01.22 -

 @brief [ virtual keyboard ]
 */

#ifndef COCOA_VKEYBOARD_H
#define COCOA_VKEYBOARD_H

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif /* __OBJC__ */

#include "../vkeyboard.h"

#ifdef __OBJC__

namespace Vkbd {
	class VKeyboard;
}

/**
	@brief Virtual keyboard view
*/
@interface CocoaVKeyboardView : NSView
{
	SDL_Surface *suf;
	NSBitmapImageRep *img;
}
- (id)initWithSurface:(SDL_Surface *)surface;
- (void)drawRect:(NSRect)dirtyRect;
- (NSBitmapImageRep *)allocBuffer;
- (void)copyBuffer;
- (void)copyBufferPart:(NSRect)re;
@end

/**
	@brief Virtual keyboard window
*/
@interface CocoaVKeyboard : NSWindow
{
	Vkbd::VKeyboard *vkeyboard;
}
- (id)initWithSurface:(Vkbd::VKeyboard *)obj surface:(SDL_Surface *)suf;
- (void)close;
- (void)setDist;
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
@end

//@interface CocoaVKeyboardDelegate : NSObject <NSWindowDelegate>
//- (void)windowDidMove:(NSNotification *)notification;
//@end

void vkeyboard_set_owner_window(NSWindow *owner);

#endif /* __OBJC__ */

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public OSDBase
{
private:
#ifdef __OBJC__
	CocoaVKeyboard *vKbdWin;
	NSWindow *parentWin;
#else
	void *vKbdWin;
	void *parentWin;
#endif

	void adjust_window_size();
	void need_update_window(PressedInfo_t *, bool);
	void update_window();

	bool Create() { return false; }

public:
	VKeyboard();
	~VKeyboard();

	bool Create(const char *res_path);
	void Show(bool = true);
	void Close();
	void PostClose();

	void SetDist();
};

} /* namespace Vkbd */

#endif /* COCOA_VKEYBOARD_H */
