/** @file cocoa_recvidpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.05.21 -

 @brief [ record video panel ]
 */

#ifndef COCOA_RECVIDEO_PANEL_H
#define COCOA_RECVIDEO_PANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

#define COCOA_RECVIDEO_LIBS 5

/**
	@brief Record video dialog box
*/
@interface CocoaRecVideoPanel : CocoaBasePanel
{
	int typnum;
	int codnums[COCOA_RECVIDEO_LIBS];
	int quanums[COCOA_RECVIDEO_LIBS];
	bool enables[COCOA_RECVIDEO_LIBS];

	CocoaTabView *tabView;
	CocoaPopUpButton *codbtn[COCOA_RECVIDEO_LIBS];
	CocoaPopUpButton *quabtn[COCOA_RECVIDEO_LIBS];
	CocoaButton *btnOK;
}
- (id)initWithCont:(bool)cont;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
- (void)selectTab:(int)num;
@end

/**
	@brief Delegate for record video dialog
*/
@interface CocoaRecVideoTabViewDelegate : NSObject <NSTabViewDelegate>
{
	CocoaRecVideoPanel *panel;
}
- (id)initWithPanel:(CocoaRecVideoPanel *)new_panel;
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
@end

#endif /* COCOA_RECVIDEO_PANEL_H */
