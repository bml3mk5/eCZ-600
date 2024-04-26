/** @file cocoa_seldrvpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.12.12 -

 @brief [ select drive panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_seldrvpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaSelDrvPanel
@synthesize selectedDrive;
- (id)initWithPrefix:(int)defdrv prefix:(const char *)prefix
{
	[super init];

	selectedDrive = defdrv;

	[self setTitleById:CMsg::Select_Drive];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :HorizontalBox :0 :COCOA_DEFAULT_MARGIN];

	for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
		char label[64];
		if (prefix) {
			UTILITY::sprintf(label, sizeof(label), "%s%d", prefix, drv);
		} else {
			UTILITY::sprintf(label, sizeof(label), "%s%d", CMSG(Drive), drv);
		}
		CocoaButton *btn = [CocoaButton createT:box_all title:label action:@selector(dialogOk:) width:80];
		[btn setRelatedObject:(id)(intptr_t)drv];
//		if (selectedDrive == drv) {
//			[btn setHighlighted:YES];
//		}
	}

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogOk:(id)sender
{
	CocoaButton *btn = (CocoaButton *)sender;
	selectedDrive = (int)(intptr_t)[btn relatedObject];
	[self close];
}

@end
