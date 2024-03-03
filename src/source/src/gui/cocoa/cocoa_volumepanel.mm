/** @file cocoa_volumepanel.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ volume panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_volumepanel.h"
#import "../../emu.h"
#import "../../labels.h"
#import "../../clocale.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaVolumePanel
- (id)init
{
	[super init];

	[self setPtr];

	[self setTitleById:CMsg::Volume];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *hbox;
	CocoaLayout *vbox;

	hbox = [box_all addBox:HorizontalBox];

	int n = 0;
	for(int i=0; LABELS::volume[i] != CMsg::End; i++) {
		bool wrap = (LABELS::volume[i] == CMsg::Null);
		if (wrap) {
			NSBox *sep = [[NSBox alloc] init];
			[box_all addControl:sep width:(i * 80) height:2];

			hbox = [box_all addBox:HorizontalBox];
			continue;
		}

		vbox = [hbox addBox:VerticalBox];
		[CocoaLabel createI:vbox title:LABELS::volume[i] align:NSTextAlignmentCenter width:80 height:32];

		CocoaSlider *slider = [CocoaSlider createN:vbox index:n action:@selector(changeSlider:) value:[self volume:n] width:80 height:120];
		if (NSAppKitVersionNumber > 1349.0 /* NSAppKitVersionNumber10_10_Max */) {
			[slider.cell setVertical:YES];
		}

		p_lbl[n] = [CocoaLabel createT:vbox title:"00" align:NSTextAlignmentCenter width:80 height:16];
		[self setVolumeText:n];

		[CocoaCheckBox createI:vbox title:CMsg::Mute index:n action:@selector(changeMute:) value:[self mute:n] width:80 height:32];

		if (i == 0) {
			// separator
			NSBox *sep = [[NSBox alloc] init];
			[hbox addControl:sep width:2 height:200];
		}
		n++;
	}

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Close action:@selector(dialogClose:) width:120];

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

- (void)dialogClose:(id)sender
{
	[self close];
}

- (void)setPtr
{
	int i = 0;
	p_volume[i++] = &pConfig->volume;
	p_volume[i++] = &pConfig->opm_volume;
	p_volume[i++] = &pConfig->adpcm_volume;
	p_volume[i++] = &pConfig->fdd_volume;
	p_volume[i++] = &pConfig->hdd_volume;

	i = 0;
	p_mute[i++] = &pConfig->mute;
	p_mute[i++] = &pConfig->opm_mute;
	p_mute[i++] = &pConfig->adpcm_mute;
	p_mute[i++] = &pConfig->fdd_mute;
	p_mute[i++] = &pConfig->hdd_mute;
}

- (void)setVolumeText:(int)idx
{
	char str[8];
	UTILITY::sprintf(str, sizeof(str), "%02d", [self volume:idx]);
	[p_lbl[idx] setStringValue:[NSString stringWithUTF8String:str]];
}

- (bool)mute:(int)idx
{
	return *p_mute[idx];
}

- (int)volume:(int)idx
{
	return *p_volume[idx];
}

- (void)changeSlider:(CocoaSlider *)sender
{
	*p_volume[sender.index] = [sender intValue];
	[self setVolumeText:sender.index];
	emu->set_volume(0);
}

- (void)changeMute:(CocoaCheckBox *)sender
{
	*p_mute[sender.index] = ([sender state] == NSControlStateValueOn);
	emu->set_volume(0);
}

@end
