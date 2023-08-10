/** @file cocoa_joysetpanel.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2023.01.07 -

 @brief [ joypad setting panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_joysetpanel.h"
#import "../../config.h"
#import "../../emumsg.h"
#import "../../msgs.h"
#import "../../labels.h"
#import "../gui_keybinddata.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaJoySettingPanel
- (id)init
{
	_TCHAR label[64];
	int tx = 80;
	int sx = 256;
	int sy = 24;

	[super init];

	[self setTitleById:CMsg::Joypad_Setting];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)

	CocoaLayout *hbox_all = [box_all addBox:HorizontalBox];
	CocoaLayout *hbox;

	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CocoaLayout *vbox = [hbox_all addBox:VerticalBox];

		hbox = [vbox addBox:HorizontalBox];
		UTILITY::stprintf(label, 64, CMSG(JoypadVDIGIT), i + 1);
		[CocoaLabel createT:hbox title:label];
		int val;
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		pop[i] = [CocoaPopUpButton createI:hbox items:LABELS::joypad_type action:nil selidx:val];
#else
		pop[i] = nil;
#endif

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Button_Mashing_Speed];
		[hbox addSpace:32 :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 3")];

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				mash[i][k] = nil;
				continue;
			}
			
			hbox = [vbox addBox:HorizontalBox];
			CMsg::Id titleid = (CMsg::Id)cVmJoyLabels[kk].id;
			[CocoaLabel createI:hbox title:titleid align:NSCenterTextAlignment width:tx height:sy];

			val = pConfig->joy_mashing[i][k];
			int n = i * KEYBIND_JOY_BUTTONS + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:3 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:4];
			mash[i][k] = slider;
		}

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Analog_to_Digital_Sensitivity];
		[hbox addSpace:32 :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 10")];

		for(int k=0; k < 6; k++) {
			hbox = [vbox addBox:HorizontalBox];

			CMsg::Id titleid = LABELS::joypad_axis[k];
			[CocoaLabel createI:hbox title:titleid align:NSCenterTextAlignment width:tx height:sy];

			val = 10 - pConfig->joy_axis_threshold[i][k];
			int n = i * 6 + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:10 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:11];
			axis[i][k] = slider;
		}
	}

#endif

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
	[NSApp stopModalWithCode:NSCancelButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
	[self setData];
	// OK button is pushed
	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogCancel:(id)sender
{
	[self close];
}

- (void)setData
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
#ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)[pop[i] indexOfSelectedItem];
#endif
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			pConfig->joy_mashing[i][k] = [mash[i][k] intValue];
		}
		for(int k=0; k<6; k++) {
			pConfig->joy_axis_threshold[i][k] = 10 - [axis[i][k] intValue];
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
#ifdef USE_JOYSTICK_TYPE
	// will change joypad type in emu thread
	emumsg.Send(EMUMSG_ID_MODIFY_JOYTYPE);
#endif
#endif
}

#if 0
- (void)changeSlider:(CocoaSlider *)sender
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = [sender index];
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;
	pConfig->joy_mashing[i][k] = [sender intValue];
	emu->set_joy_mashing();
#endif
}
#endif

@end
