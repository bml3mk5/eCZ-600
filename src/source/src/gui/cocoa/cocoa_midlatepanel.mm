/** @file cocoa_midlatepanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.06.12 -

 @brief [ midi latency panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_midlatepanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "../../msgs.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaMidLatePanel
- (id)init
{
	[super init];

	[self setTitleById:CMsg::MIDI_Output_Latency];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN];

	CocoaLayout *hbox = [box_all addBox:HorizontalBox :MiddlePos :0 :_T("MIDIOD")];
	[CocoaLabel createI:hbox title:CMsg::Output_Latency];
	int valuei = pConfig->midiout_delay;
	stpMIDIOutDelay = [CocoaStepper createN:hbox min:0 max:2000 value:valuei width:60];
	[CocoaLabel createI:hbox title:CMsg::msec];

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
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
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogOk:(id)sender
{
	int valuei = [stpMIDIOutDelay intValue];
	if (0 <= valuei && valuei <= 2000) {
		pConfig->midiout_delay = valuei;
		emu->set_midiout_delay_time(valuei);
	}
	[self close];
}

- (void)dialogCancel:(id)sender
{
	// Cancel button is pushed
	[self close];
}

@end
