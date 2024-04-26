/** @file cocoa_hdtypepanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.03.20 -

 @brief [ select hd device type panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_hdtypepanel.h"
#import "../../emu.h"
#import "../../labels.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaHDTypePanel
@synthesize deviceType;
- (id)initWithType:(int)drv type:(int)type
{
	[super init];

	drive = drv;
	deviceType = type;

	[self setTitleById:CMsg::Select_Device_Type];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN];

	char label[_MAX_PATH];
	if (drive < MAX_SASI_HARD_DISKS) {
		UTILITY::sprintf(label, _MAX_PATH, CMSG(Select_a_device_type_on_SASI_disk_VDIGIT), drive);
	} else {
		UTILITY::sprintf(label, _MAX_PATH, CMSG(Select_a_device_type_on_SCSI_disk_VDIGIT), drive - MAX_SASI_HARD_DISKS);
	}
	[CocoaLabel createT:box_all title:label];

	CocoaLayout *hbox = [box_all addBox:HorizontalBox :MiddlePos :COCOA_DEFAULT_MARGIN];

	for(int i=0; LABELS::hd_device_type[i]; i++) {
		CocoaRadioButton *b = [CocoaRadioButton createT:hbox title:LABELS::hd_device_type[i] index:i action:@selector(selectRadio:) value:i == deviceType];
		list.push_back(b);
	}

	[CocoaLabel createI:box_all title:CMsg::Need_restart_program_or_PowerOn];

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	[CocoaButton createI:hbox title:CMsg::OK action:@selector(dialogOk:) width:120];


	[box_all realize:self];

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
	for(int i=0; i<(int)list.size(); i++) {
		if ([list.at(i) state] == NSControlStateValueOn) {
			deviceType = i;
			break;
		}
	}
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogCancel:(id)sender
{
	// Cancel button is pushed
	[self close];
}

- (void)selectRadio:(CocoaRadioButton *)sender
{
	int idx = [sender index];
	for(int i=0; i<(int)list.size(); i++) {
		[list.at(i) setState:(i == idx ? NSControlStateValueOn : NSControlStateValueOff)];
	}
}

@end
