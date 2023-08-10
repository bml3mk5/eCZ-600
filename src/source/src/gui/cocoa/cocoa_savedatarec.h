/** @file cocoa_savedatarec.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2017.12.29 -

 @brief [ save datarec panel ]
 */

#ifndef COCOA_SAVEDATAREC_H
#define COCOA_SAVEDATAREC_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

/**
	@brief Save file dialog for datarec
*/
@interface CocoaSaveDatarec : NSSavePanel
{
	int typnum;
}
- (id)init;
- (void)changeFileType:(CocoaRadioGroup *)sender;
@end

#endif /* COCOA_SAVEDATAREC_H */
