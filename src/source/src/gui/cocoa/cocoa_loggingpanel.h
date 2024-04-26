/** @file cocoa_loggingpanel.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.04.14 -

 @brief [ log panel ]
 */

#ifndef COCOA_LOGGINGPANEL_H
#define COCOA_LOGGINGPANEL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../config.h"

/**
	@brief Log dialog box
*/
@interface CocoaLoggingPanel : CocoaBasePanel
{
	bool m_initialized;

	CocoaTextView *txtPath;
	CocoaTextView *txtLog;
	CocoaButton *btnUpdate;
	CocoaButton *btnClose;

	NSRect m_client_re;

	int m_buffer_size;
	char *p_buffer;
}
- (id)init;
//- (NSInteger)runModal;
- (void)run;
- (void)close;
- (void)dialogClose:(id)sender;
- (void)dialogUpdate:(id)sender;
- (void)windowDidResize:(NSNotification *)notification;
- (void)adjustButtonControl;
@end

@interface CocoaLoggingPanelDelegate : NSObject<NSWindowDelegate>
{
	CocoaLoggingPanel *parent;
}
@property (assign) CocoaLoggingPanel *parent;
- (id)initWithParent:(CocoaLoggingPanel *)n_parent;
- (void)windowDidResize:(NSNotification *)notification;
@end

#endif /* COCOA_LOGGINGPANEL_H */
