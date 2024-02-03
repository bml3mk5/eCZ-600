/** @file cocoa_joysetpanel.h

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2023.01.07 -

 @brief [ joypad setting panel ]
 */

#ifndef COCOA_JOYSETTINGPANEL_H
#define COCOA_JOYSETTINGPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../vm/vm_defs.h"
#import "../../emu.h"

/**
	@brief Joypad setting dialog box
*/
@interface CocoaJoySettingPanel : CocoaBasePanel
{
	NSMutableArray *tableViews;
	Uint32 enable_axes;

	CocoaPopUpButton *pop[MAX_JOYSTICKS];
	CocoaSlider *mash[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	CocoaSlider *axis[MAX_JOYSTICKS][6];
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
- (void)loadDefaultPreset:(id)sender;
- (void)loadPreset:(id)sender;
- (void)savePreset:(id)sender;
- (void)clickJoyAxis:(id)sender;
- (void)setData;

//- (void)changeSlider:(CocoaSlider *)sender;
@end

#endif /* COCOA_JOYSETTINGPANEL_H */
