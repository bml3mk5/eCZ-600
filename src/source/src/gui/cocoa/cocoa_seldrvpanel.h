/** @file cocoa_seldrvpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.12.12 -

 @brief [ select drive panel ]
 */

#ifndef COCOA_SELDRVPANEL_H
#define COCOA_SELDRVPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

/**
	@brief Select drive dialog box
*/
@interface CocoaSelDrvPanel : CocoaBasePanel
{
	int selectedDrive;
}
@property (nonatomic) int selectedDrive;
- (id)initWithPrefix:(int)defdrv prefix:(const char *)prefix;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
@end

#endif /* COCOA_SELDRVPANEL_H */
