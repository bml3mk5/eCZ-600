/** @file cocoa_fontpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.10.30 -

 @brief [ font panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_fontpanel.h"

@implementation CocoaFontPanel
- (id)init
{
	[super init];

	NSView *view = [self contentView];

	NSRect re_box;
	re_box = [self frame];

	// button

//	re = [view makeRect:re.origin.x+re.size.width-120:re.origin.y+re.size.height+view.margin:120:32];
	NSRect re;
	re.origin.x = 0; re.origin.y = 0;
	re.size.width = 120; re.size.height= 32;
	CocoaButton *btnCancel = [CocoaButton create:re titleid:CMsg::Cancel action:@selector(dialogCancel:)];
	[view addSubview:btnCancel];

	re.origin.x = re.size.width + re.origin.x;
//	re = [view addPosition:re:-120-view.margin:0:120:32];
	btnOK = [CocoaButton create:re titleid:CMsg::OK action:@selector(dialogOk:)];
	[view addSubview:btnOK];

	re_box.size.height += 32;
	[self setFrame:re_box display:YES];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSModalResponseCancel];
	[super close];
}

- (void)dialogOk:(id)sender
{
	// OK button is pushed
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogCancel:(id)sender
{
	// Cancel button is pushed
	[self close];
}
@end
