/** @file cocoa_hdtypepanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.03.20 -

 @brief [ select hd device type panel ]
 */

#ifndef COCOA_HDTYPEPANEL_H
#define COCOA_HDTYPEPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import <vector>

/**
	@brief Select hd device type dialog box
*/
@interface CocoaHDTypePanel : CocoaBasePanel
{
	int drive;
	int deviceType;
	std::vector<CocoaRadioButton *> list;
}
@property (nonatomic) int deviceType;
- (id)initWithType:(int)drv type:(int)type;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
- (void)selectRadio:(CocoaRadioButton *)sender;
@end

#endif /* COCOA_HDTYPEPANEL_H */
