/** @file cocoa_fontpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.10.30 -

 @brief [ font panel ]
 */

#ifndef COCOA_FONTPANEL_H
#define COCOA_FONTPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

/**
	@brief Font dialog box
*/
@interface CocoaFontPanel : NSFontPanel
{
	CocoaButton *btnOK;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
@end

#endif /* COCOA_FONTPANEL_H */
