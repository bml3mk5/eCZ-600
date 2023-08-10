/** @file cocoa_recaudpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2016.11.05 -

 @brief [ record audio panel ]
 */

#ifndef COCOA_RECAUDIO_PANEL_H
#define COCOA_RECAUDIO_PANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"

#define COCOA_RECAUDIO_LIBS 5

/**
	@brief Record audio dialog box
*/
@interface CocoaRecAudioPanel : CocoaBasePanel
{
	int typnum;
	int codnums[COCOA_RECAUDIO_LIBS];
	int quanums[COCOA_RECAUDIO_LIBS];
	bool enables[COCOA_RECAUDIO_LIBS];

	CocoaTabView *tabView;
	CocoaPopUpButton *codbtn[COCOA_RECAUDIO_LIBS];
	CocoaPopUpButton *quabtn[COCOA_RECAUDIO_LIBS];
	CocoaButton *btnOK;
}
- (id)init;
- (NSInteger)runModal;
- (void)close;
- (void)dialogOk:(id)sender;
- (void)dialogCancel:(id)sender;
- (void)selectTab:(int)num;
@end

/**
	@brief Delegate for record audio dialog
*/
@interface CocoaRecAudioTabViewDelegate : NSObject <NSTabViewDelegate>
{
	CocoaRecAudioPanel *panel;
}
- (id)initWithPanel:(CocoaRecAudioPanel *)new_panel;
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
@end

#endif /* COCOA_RECAUDIO_PANEL_H */
