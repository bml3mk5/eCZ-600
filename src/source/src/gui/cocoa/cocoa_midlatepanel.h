/** @file cocoa_midlatepanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.06.12 -

 @brief [ midi latency panel ]
 */

#ifndef COCOA_MIDLATEPANEL_H
#define COCOA_MIDLATEPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

/**
	@brief Select midi latency dialog box
*/
@interface CocoaMidLatePanel : CocoaBasePanel
{
	CocoaStepper *stpMIDIOutDelay;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
@end

#endif /* COCOA_MIDLATEPANEL_H */
