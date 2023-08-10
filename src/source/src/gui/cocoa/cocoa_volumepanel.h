/** @file cocoa_volumepanel.h

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ volume panel ]
 */

#ifndef COCOA_VOLUMEPANEL_H
#define COCOA_VOLUMEPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../config.h"

/**
	@brief Volume dialog box
*/
@interface CocoaVolumePanel : CocoaBasePanel
{
	int  *p_volume[VOLUME_NUMS];
	bool *p_mute[VOLUME_NUMS];
	CocoaLabel *p_lbl[VOLUME_NUMS];
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogClose:(id)sender;

- (void)setPtr;
- (void)setVolumeText:(int)idx;

- (bool)mute:(int)idx;
- (int)volume:(int)idx;

- (void)changeSlider:(CocoaSlider *)sender;
- (void)changeMute:(CocoaCheckBox *)sender;
@end

#endif /* COCOA_VOLUMEPANEL_H */
