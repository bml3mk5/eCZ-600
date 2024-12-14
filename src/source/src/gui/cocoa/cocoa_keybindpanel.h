/** @file cocoa_keybindpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ keybind panel ]
 */

#ifndef COCOA_KEYBINDPANEL_H
#define COCOA_KEYBINDPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "cocoa_keybindctrl.h"
#import "../gui_keybinddata.h"
#import "../../vm/vm.h"

//@interface CocoaTableFieldView : NSTableView
//- (void)keyDown:(NSEvent *)event;
//- (BOOL)textShouldBeginEditing:(NSText *)textObject;
//@end

/**
	@brief Keybind dialog box
*/
@interface CocoaKeybindPanel : CocoaBasePanel
{
	CocoaTabView *tabView;
	NSMutableArray *tableViews;
	Uint32 enable_axes;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogCancel:(id)sender;
- (void)dialogOk:(id)sender;
- (void)loadDefaultPreset:(id)sender;
- (void)loadPreset:(id)sender;
- (void)savePreset:(id)sender;
- (void)clickJoyAxis:(id)sender;

@end

#endif /* COCOA_KEYBINDPANEL_H */
