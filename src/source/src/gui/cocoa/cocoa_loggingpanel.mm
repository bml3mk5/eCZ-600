/** @file cocoa_loggingpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.04.24 -

 @brief [ log panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_loggingpanel.h"
#import "../../emu.h"
#import "../../labels.h"
#import "../../logging.h"
#import "../../utility.h"

extern EMU *emu;

@implementation CocoaLoggingPanel
- (id)init
{
	[super init];
	
	m_initialized = false;
	m_buffer_size = 0;
	p_buffer = NULL;
	
	int style = (int)[self styleMask];
	style |= NSWindowStyleMaskResizable;
	[self setStyleMask:style];
	
	[self setTitleById:CMsg::Log];
	[self setShowsResizeIndicator:NO];
	
	CocoaView *view = [self contentView];
	
	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	
	txtPath = [CocoaTextView create:box_all edit:false hasvs:false hashs:false width:480 height:32];
	
	txtLog = [CocoaTextView create:box_all edit:false hasvs:true hashs:true width:480 height:320];
	
	CocoaLayout *hbox = [box_all addBox:HorizontalBox :0 :0 :_T("BTN")];
	btnUpdate = [CocoaButton createI:hbox title:CMsg::Update action:@selector(dialogUpdate:) width:120];
	btnClose = [CocoaButton createI:hbox title:CMsg::Close action:@selector(dialogClose:) width:120];
	
	[box_all realize:self];
	
	m_initialized = true;
	
	m_client_re = [self frame];
	[self setMinSize:m_client_re.size];
	[self adjustButtonControl];

	[txtPath setString:[NSString stringWithUTF8String:logging->get_log_path()]];
	
	[self setDelegate:[[CocoaLoggingPanelDelegate alloc] initWithParent:self]];
	
	return self;
}

//- (NSInteger)runModal
//{
//	return [NSApp runModalForWindow:self];
//}

- (void)run
{
	[self orderFront:nil];
	[self makeKeyWindow];
}

- (void)close
{
	//	[NSApp stopModalWithCode:NSModalResponseOK];
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
	[super close];
}

- (void)dialogClose:(id)sender
{
	[self close];
}

- (void)dialogUpdate:(id)sender
{
	if (!p_buffer) {
		m_buffer_size = 1024 * 1024;
		p_buffer = new char[m_buffer_size];
	}
	p_buffer[0] = 0;
	logging->get_log(p_buffer, m_buffer_size);
	[txtLog setString:[NSString stringWithUTF8String:p_buffer]];
	NSUInteger len = [[txtLog string] length];
	if (len < 4) len = 4;
	NSRange re = NSMakeRange(len - 4, 1);
	[txtLog scrollRangeToVisible:re];
}

- (void)windowDidResize:(NSNotification *)notification
{
	NSRect re = [self frame];
	
	int s_w = re.size.width - m_client_re.size.width;
	int s_h = re.size.height - m_client_re.size.height;
	
	m_client_re = re;
	
	re = [txtPath.parent frame];
	re.size.width += s_w;
	[txtPath.parent setFrame:re];
	re = [txtPath frame];
	re.size.width += s_w;
	[txtPath setFrame:re];
	
	re = [txtLog.parent frame];
	re.size.width += s_w;
	re.size.height += s_h;
	[txtLog.parent setFrame:re];
	re = [txtLog frame];
	re.size.width += s_w;
	re.size.height += s_h;
	[txtLog setFrame:re];

	[self adjustButtonControl];
}

- (void)adjustButtonControl
{
	NSRect content_re = [[self contentView] frame];
	NSRect re = [btnUpdate frame];
	re.origin.x = 4;
	re.origin.y = content_re.size.height - re.size.height - 4;
	[btnUpdate setFrame:re];

	re = [btnClose frame];
	re.origin.x = content_re.size.width - re.size.width - 4;
	re.origin.y = content_re.size.height - re.size.height - 4;
	[btnClose setFrame:re];
}

@end

@implementation CocoaLoggingPanelDelegate
@synthesize parent;
- (id)initWithParent:(CocoaLoggingPanel *)n_parent;
{
	[self setParent:n_parent];
	return self;
}
- (void)windowDidResize:(NSNotification *)notification
{
	[parent windowDidResize:notification];
}
@end
