/** @file cocoa_keybindpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ keybind panel ]
 */

#import <Carbon/Carbon.h>
#import "cocoa_gui.h"
#import "cocoa_keybindpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "../../labels.h"
#import "../../keycode.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

@implementation CocoaTableData
- (id)initWithValue:(CocoaTableView *)new_parent :(int)new_tabnum
{
	[super init];

	parent = new_parent;

	kbdata.Init(emu, new_tabnum);

	// get parameters
	selected.row = -1;
	selected.col = -1;

	return self;
}
- (int)tab_num
{
	return kbdata.m_tab_num;
}
- (int)devType
{
	return kbdata.m_devtype;
}
- (int)selectedCol
{
	return selected.col;
}
- (void)setSelectCol:(int)col row:(int)row
{
	selected.col = col;
	selected.row = row;
}
- (void)reloadCell:(int)col row:(int)row;
{
	[parent reloadCell:col row:row];
}
- (Uint32)combi
{
	return kbdata.GetCombi();
}
- (void)setCombi:(Uint32)value
{
	kbdata.SetCombi(value);
}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return kbdata.GetNumberOfRows();
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	char label[128];
	int col = 0;
	if ([[tableColumn identifier] isEqualToString:@"1"]) {
		col++;
	}
	if([[tableColumn identifier] isEqualToString:@"vmkey"]) {
		col=-1;
	}
	kbdata.GetCellString((int)row,col,label);
	return [NSString stringWithUTF8String:label];
}
- (void)tableView:(NSTableView *)tableView setObjectValue:(id)obj forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
}

- (void)SetVmKeyMap:(Uint16 *)vmKeyMap :(int)size
{
	kbdata.SetVmKeyMap(vmKeyMap,size);
}

- (void)SetVmKey:(int)idx :(Uint16)code
{
	kbdata.SetVmKey(idx,code);
}

- (bool)SetVmKeyCode:(int)idx :(Uint16)code
{
	return kbdata.SetVmKeyCode(idx,code);
}

- (void)SetVkKeyMap:(Uint32 *)vkKeyMap
{
	kbdata.SetVkKeyMap(vkKeyMap);
}

- (void)SetVkKeyDefMap:(Uint32 *)vkKeyDefMap :(int)rows :(int)cols
{
	kbdata.SetVkKeyDefMap(vkKeyDefMap,rows,cols);
}

- (void)SetVkKeyPresetMap:(Uint32 *)vkKeyMap :(int)idx
{
	kbdata.SetVkKeyPresetMap(vkKeyMap,idx);
}

//- (bool)SetVkKeyCodeR:(int)row :(codecols_t *)obj :(Uint32)code :(char *)label
//{
//	return kbdata.SetVkKeyCode(row,obj,code,label);
//}

- (bool)SetVkKeyCode:(int)row :(int)col :(Uint32)code :(char *)label
{
	return kbdata.SetVkKeyCode(row,col,code,label);
}

- (bool)SetVkKeyCode:(Uint32)code :(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.SetVkKeyCode(selected.row,selected.col,code,label);
}

- (bool)ClearVkKeyCode:(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.ClearVkKeyCode(selected.row,selected.col,label);
}

- (bool)ClearCellByCode:(Uint32)code :(char *)label :(int *)row :(int *)col
{
	return kbdata.ClearCellByVkKeyCode(code, label, row, col);
}

- (bool)SetVkJoyCode:(Uint32)code0 :(Uint32)code1 :(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.SetVkJoyCode(selected.row,selected.col,code0,code1,label);
}

- (bool)ClearVkJoyCode:(char *)label
{
	if (selected.row < 0 || selected.col < 0) return false;
	return kbdata.ClearVkJoyCode(selected.row,selected.col,label);
}

- (void)loadDefaultPreset
{
	kbdata.LoadDefaultPreset();
}

- (void)loadPreset:(int)idx
{
	kbdata.LoadPreset(idx);
}

- (void)savePreset:(int)idx
{
	kbdata.SavePreset(idx);
}

- (void)SetData
{
	kbdata.SetData();
}

- (void)onClickCell:(NSTableView *)sender
{
#ifdef USE_NSCELL
	int row = (int)[sender clickedRow];
	int col = (int)[sender clickedColumn];
	col--;

	selected.row = row;
	selected.col = col;

	if (col >= 0) {
		[sender editColumn:col+1 row:row withEvent:nil select:YES];
	}
#endif
}

- (void)onDoubleClickCell:(NSTableView *)sender
{
}

- (void)setJoyMask:(Uint32 *)mask
{
	joy_mask = mask;
}

- (Uint32)joyMask
{
	return *joy_mask;
}
@end

#ifndef USE_NSCELL
@implementation CocoaNText
- (id)initWithDataSource:(NSTableView *)new_tbl data:(CocoaTableData *)new_data col:(NSTableColumn *)tableColumn row:(int)new_row
{
	[super init];
//	[self setEditable:YES];
//	[self setSelectable:YES];
	dataSource = new_data;

	int new_col = 0;
	if ([[tableColumn identifier] isEqualToString:@"1"]) {
		new_col++;
	}
	if([[tableColumn identifier] isEqualToString:@"vmkey"]) {
		new_col=-1;
	}
	col = new_col;
	row = new_row;

	if ([dataSource devType] == KeybindData::DEVTYPE_JOYPAD) {
		// update joystick status per 0.1 sec
		joy_stat = emu->joy_real_buffer();
//		emu->reset_joystick();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		[loop addTimer:timer forMode:NSRunLoopCommonModes];
		[loop addTimer:timer forMode:NSModalPanelRunLoopMode];
	} else {
		joy_stat = NULL;
		timer = nil;
	}
//	[self setFocusRingType:NSFocusRingTypeDefault];
	[self setString:[dataSource tableView:new_tbl objectValueForTableColumn:tableColumn row:row]];

	selClick = FALSE;

	return self;
}
- (void)dealloc
{
	if (timer != nil) [timer invalidate];
	[super dealloc];
}
- (BOOL)acceptsFirstResponder
{
	return YES;
}
- (void)keyDown:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)flagsChanged:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)keyUp:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		selClick = TRUE;
		[self setNeedsDisplay:YES];
	}
}
- (void)mouseDown:(NSEvent *)event
{
	int count = (int)[event clickCount];
	[dataSource setSelectCol:col+1 row:row];
	selClick = TRUE;
	[self setNeedsDisplay:YES];
	if (count != 2) return;
	char label[128];
	int tab = [dataSource tab_num];
	if (tab != 0) {
		[dataSource SetVkJoyCode:0:label];
	} else {
		[dataSource SetVkKeyCode:0:label];
	}
	[self setString:[NSString stringWithUTF8String:label]];
}
- (void)updateJoy
{
	char label[128];

	if (col >= 0) {
		emu->update_joystick();
		uint32_t *joy_stat = emu->joy_real_buffer(col);
		if (joy_stat[0]) {
			[dataSource SetVkJoyCode:joy_stat[0]:label];
			[self setString:[NSString stringWithUTF8String:label]];
			[self setNeedsDisplay:YES];
		}
	}
}
- (void)setString:(NSString *)aString
{
	str = [[NSAttributedString alloc] initWithString:(aString ? aString : @"")];
}
- (void)drawRect:(NSRect)theRect
{
	NSRect re = [self bounds];

	[[NSColor whiteColor] set];
	NSRectFill(re);

	if (col >= 0) {
		if (selClick && self == [NSView focusView]) {
			NSSetFocusRingStyle(NSFocusRingBelow);
		}

		[[NSColor grayColor] set];
		NSFrameRect(re);
	}

	[str drawInRect: NSMakeRect(
		re.origin.x + 6.0, re.origin.y - 1.0,
		re.size.width, re.size.height)];

	[self setNeedsDisplay:NO];
}
//- (void)resetCursorRects
//{
//	NSRect rect = [self bounds];
//	NSCursor* cursor = [NSCursor IBeamCursor];
//	[self addCursorRect:rect cursor:cursor];
//}
@end
#else /* USE_NSCELL */
@implementation CocoaText
- (id)initWithDataSource:(CocoaTableData *)new_data
{
	[super init];
	[self setEditable:YES];
	[self setSelectable:YES];
//	[self setBordered:YES];
	dataSource = new_data;

	if ([dataSource devType] == KeybindData::DEVTYPE_JOYPAD) {
		// update joystick status per 0.1 sec
//		emu->reset_joystick();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		[loop addTimer:timer forMode:NSRunLoopCommonModes];
		[loop addTimer:timer forMode:NSModalPanelRunLoopMode];
	} else {
		timer = nil;
	}

	return self;
}
- (void)dealloc
{
	if (timer != nil) [timer invalidate];
	[super dealloc];
}
- (void)keyDown:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		int row;
		int col;
		[dataSource ClearCellByCode:code:label:&row:&col];
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		if (row >= 0 && col >= 0) {
			[dataSource reloadCell:col row:row];
		}
	}
}
- (void)flagsChanged:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		int row;
		int col;
		[dataSource ClearCellByCode:code:label:&row:&col];
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
		if (row >= 0 && col >= 0) {
			[dataSource reloadCell:col row:row];
		}
	}
}
- (void)keyUp:(NSEvent *)event
{
	char label[128];
	int tab = [dataSource tab_num];
	if (tab == 0) {
		int code = SDL_QZ_HandleKeyEvents(event);
		[dataSource SetVkKeyCode:code:label];
		[self setString:[NSString stringWithUTF8String:label]];
	}

}
- (BOOL)performKeyEquivalent:(NSEvent *)event
{
	return NO;
}
- (void)mouseDown:(NSEvent *)event
{
	int count = (int)[event clickCount];
	if (count != 2) return;
	char label[128];
	int tab = [dataSource tab_num];
	if (tab != 0) {
		[dataSource ClearVkJoyCode:label];
	} else {
		[dataSource ClearVkKeyCode:label];
	}
	[self setString:[NSString stringWithUTF8String:label]];
}
- (void)mouseMoved:(NSEvent *)event
{
	// keep allow cursor
	[[NSCursor arrowCursor] set];
}
- (void)cursorUpdate:(NSEvent *)event
{
	// keep allow cursor
	[[NSCursor arrowCursor] set];
}
- (void)updateJoy
{
	int col = [dataSource selectedCol];
	char label[128];

    if (col >= 0) {
        emu->update_joystick();
	uint32_t *joy_stat = emu->joy_real_buffer(col);
	Uint32 joy_mask = [dataSource joyMask];
        if (joy_stat[0] & joy_mask) {
		[dataSource SetVkJoyCode:joy_stat[0] & joy_mask:joy_stat[1]:label];
		[self setString:[NSString stringWithUTF8String:label]];
        }
    }
}

@end

@implementation CocoaTextCell
- (id)init
{
	[super init];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
- (id)initWithCoder:(NSCoder *)decoder
{
	[super initWithCoder:decoder];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
- (id)initTextCell:(NSString *)aString
{
//	[super init];
	[super initTextCell:aString];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
#if !defined(MAC_OS_X_VERSION_10_13)
- (id)initImageCell:(NSImage *)anImage
{
//	[super init];
	[super initImageCell:anImage];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
#endif
- (void)selectWithFrame:(NSRect)re inView:(NSView *)ctlView editor:(NSText *)text delegate:(id)obj start:(NSInteger)start length:(NSInteger)length
{
	NSTableView *view = (NSTableView *)ctlView;
	CocoaText *ntext = [[CocoaText alloc] initWithDataSource:[view dataSource]];
	[ntext setString:[text string]];
	[super selectWithFrame:re inView:ctlView editor:ntext delegate:obj start:start length:0];
}
@end
#endif /* USE_NSCELL */

//@implementation CocoaTableFieldView
//- (void)keyDown:(NSEvent *)event
//{
//	int i=0;
//}
//- (BOOL)textShouldBeginEditing:(NSText *)textObject
//{
//	return YES;
//}
//@end

@implementation CocoaTableView
@synthesize chkCombi;
+ (CocoaTableView *)create:(NSRect)re tabnum:(int)tab_num
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me initWithFrame:re];
	[me createMain:re.size tabnum:tab_num];
	return me;
}
+ (CocoaTableView *)createW:(int)width height:(int)height tabnum:(int)tab_num
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me init];
	NSSize sz = NSMakeSize(width, height);
	[me createMain:sz tabnum:tab_num];
	return me;
}
- (void)createMain:(NSSize)sz tabnum:(int)tab_num
{
	NSRect re_ctl;

	chkCombi = nil;
	[self setBorderType:NSBezelBorder];

	re_ctl = NSMakeRect(0,0,sz.width,24);
	NSScroller *hs = [[NSScroller alloc] initWithFrame:re_ctl];
//	[self addSubview:hs];
	[self setHorizontalScroller:hs];
	[self setHasHorizontalScroller:YES];
	re_ctl = NSMakeRect(0,0,24,sz.height);
	NSScroller *vs = [[NSScroller alloc] initWithFrame:re_ctl];
//	[self addSubview:vs];
	[self setVerticalScroller:vs];
	[self setHasVerticalScroller:YES];

	[self setAutohidesScrollers:NO];
//	[self setAutoresizesSubviews:YES];

	re_ctl = NSMakeRect(10,10,sz.width,24);
	NSTableHeaderView *hv = [[NSTableHeaderView alloc] init];

	re_ctl = NSMakeRect(10,128,sz.width, sz.height);
	NSTableView *table = [[NSTableView alloc] init];
//	[me addSubview:table];
	[table setHeaderView:hv];

	[table setAllowsColumnReordering:NO];
	[table setAllowsColumnResizing:NO];
	[table setAllowsMultipleSelection:NO];
	[table setAllowsEmptySelection:YES];
//	[table setGridStyleMask:(NSTableViewSolidVerticalGridLineMask | NSTableViewSolidHorizontalGridLineMask)];
//	[table setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	char label[128];
	CMsg::Id label_id;

	NSTableColumn *col;
	col = [[NSTableColumn alloc] initWithIdentifier:@"vmkey"];

	label_id = LABELS::keybind_col[tab_num][0];

	[col.headerCell setStringValue:[NSString stringWithUTF8String:gMessages.Get(label_id)]];
	[col setWidth:120.0];
	[col setEditable:NO];
	[table addTableColumn:col];
	for(int i=0; i<2; i++) {
		sprintf(label, "%d", i);
		col = [[NSTableColumn alloc] initWithIdentifier:[NSString stringWithUTF8String:label]];
		sprintf(label, CMSGV(LABELS::keybind_col[tab_num][1]), i+1);
		[col.headerCell setStringValue:[NSString stringWithUTF8String:label]];
#ifdef USE_NSCELL
		[col setDataCell:[[CocoaTextCell alloc] init]];
#endif
		[col setWidth:120.0];
		[col setEditable:YES];
		[table addTableColumn:col];
	}
	[self setDocumentView:table];
#ifndef USE_NSCELL
	[table setDelegate:[[CocoaTableViewDelegate alloc] init]];
#endif
	CocoaTableData *data = [[CocoaTableData alloc] initWithValue:self :tab_num];
	[table setDataSource:data];
	[table setTarget:data];
	[table setAction:@selector(onClickCell:)];
//	[table setDoubleAction:@selector(onDoubleClickCell:)];
}
- (CocoaTableData *)data
{
	return (CocoaTableData *)[[self documentView] dataSource];
}
- (void)reloadCell:(int)col row:(int)row;
{
	NSTableView *table = [self documentView];
	[table reloadDataForRowIndexes:[NSIndexSet indexSetWithIndex:row] columnIndexes:[NSIndexSet indexSetWithIndex:col + 1]];
}
- (void)deleteColumns
{
	NSTableView *table = [self documentView];
	NSArray *cols = [table tableColumns];
	NSUInteger count = [cols count];
	NSUInteger i;
	for  (i=0; i < count; i++) {
		[table removeTableColumn:[[table tableColumns] objectAtIndex:0]];
	}
}
- (void)setJoyMask:(Uint32 *)mask
{
	[[self data] setJoyMask:mask];
}
@end

#ifndef USE_NSCELL
@implementation CocoaTableViewDelegate
- (id)init
{
	[super init];
	cells = [[NSMutableDictionary alloc] init];
	return self;
}
- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	CocoaTableData *data = [tableView dataSource];
	NSMutableString *idt = [NSMutableString stringWithString:[tableColumn identifier]];
	[idt appendFormat:@"-%d", (int)row];
	CocoaNText *ntext = [cells objectForKey:idt];
	if (ntext == nil) {
		ntext = [[CocoaNText alloc] initWithDataSource:tableView data:data col:tableColumn row:(int)row];
		[cells setObject:ntext forKey:idt];
	}
	return ntext;
}
//- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *) tableColumn row:(NSInteger)row
//{
//	NSLog(@"%@:%d",[tableColumn identifier], row);
//}
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	return NO;
}
- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return 20.0;
}
@end
#endif

@implementation CocoaButtonAttr
@synthesize tabNum;
@synthesize idx;
- (id)initWithValue:(int)new_tabnum :(int)new_idx
{
	[super init];
	tabNum = new_tabnum;
	idx = new_idx;
	return self;
}
@end

@implementation CocoaKeybindPanel
- (id)init
{
	[super init];

	SDL_QZ_InitOSKeymap();

	[self setTitleById:CMsg::Keybind];
	[self setShowsResizeIndicator:FALSE];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];
	CocoaLayout *box_sep;
	CocoaLayout *box_one;

	CocoaTabView *tabView = [CocoaTabView createI:box_tab tabs:LABELS::keybind_tab width:500 height:400];
	CocoaView *tab_view;

	CocoaButton *btn;
	char name[64];
	NSTabViewItem *tab;

	tableViews = [NSMutableArray array];
	for(int tab_num=0; tab_num<KeybindData::TABS_MAX; tab_num++) {
		CocoaTableView *tableView = [CocoaTableView createW:390 height:400 tabnum:tab_num];
		[tableViews addObject:tableView];
		[tableView setJoyMask:&enable_axes];
	}

	//
	for(int tab_num=0; tab_num<[tableViews count]; tab_num++) {
		tab = [tabView tabViewItemAtIndex:tab_num];
		tab_view = (CocoaView *)[tab view];
		[box_tab setContentView:tab_view];

		// table
		box_sep = [box_tab addBox:HorizontalBox :0 :0];
		box_one = [box_sep addBox:VerticalBox :0 :0];

		CocoaTableView *tv = [tableViews objectAtIndex:tab_num];
		[box_one addControl:tv width:390 height:400];

		// checkbox

		if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
			CocoaCheckBox *chk = [CocoaCheckBox createI:box_one title:LABELS::keybind_combi[tab_num] action:nil value:[[tv data] combi]];
			[tv setChkCombi:chk];
		}

		// button (right side)

		box_one = [box_sep addBox:VerticalBox :0 :0];

		btn = [CocoaButton createI:box_one title:CMsg::Load_Default action:@selector(loadDefaultPreset:) width:180];
		[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:-1]];

		[box_one addSpace:180 :32];
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(name, CMSG(Load_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:box_one title:name action:@selector(loadPreset:) width:180];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];

		}

		[box_one addSpace:180 :32];
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			sprintf(name, CMSG(Save_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:box_one title:name action:@selector(savePreset:) width:180];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];
		}
	}

	// axes of joypad
	
	enable_axes = ~0;
	CocoaLayout *hbox_joy = [box_all addBox:HorizontalBox];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_Z_axis index:2 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_R_axis index:3 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_U_axis index:4 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_V_axis index:5 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0];

	// button

	CocoaLayout *hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	[CocoaButton createI:hbox title:CMsg::OK action:@selector(dialogOk:) width:120];

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return [NSApp runModalForWindow:self];
}

- (void)close
{
	[NSApp stopModalWithCode:NSCancelButton];
	[super close];
}

- (void)dialogOk:(id)sender
{
    // OK button is pushed
	for(int tab=0; tab<[tableViews count]; tab++) {
		CocoaTableView *tv = [tableViews objectAtIndex:tab];
		CocoaTableData *data = [tv data];
		[data SetData];
		CocoaCheckBox *chk = [tv chkCombi];
		if (chk) {
			[data setCombi:[chk state] == NSOnState ? 1 : 0];
		}
	}

	emu->save_keybind();

	[NSApp stopModalWithCode:NSOKButton];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}

- (void)loadDefaultPreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	CocoaTableView *tv = [tableViews objectAtIndex:[attr tabNum]];
	NSTableView *view = [tv documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	[data loadDefaultPreset];
	CocoaCheckBox *chk = [tv chkCombi];
	if (chk) {
		[chk setState:[data combi]];
	}
	[view reloadData];
}

- (void)loadPreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	CocoaTableView *tv = [tableViews objectAtIndex:[attr tabNum]];
	NSTableView *view = [tv documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	[data loadPreset:attr.idx];
	CocoaCheckBox *chk = [tv chkCombi];
	if (chk) {
		[chk setState:[data combi]];
	}
	[view reloadData];
}

- (void)savePreset:(id)sender
{
	CocoaButtonAttr *attr = (CocoaButtonAttr *)[sender relatedObject];
	CocoaTableView *tv = [tableViews objectAtIndex:[attr tabNum]];
	NSTableView *view = [tv documentView];
	CocoaTableData *data = [view dataSource];
	[view editColumn:0 row:[view selectedRow] withEvent:nil select:YES];
	CocoaCheckBox *chk = [tv chkCombi];
	if (chk) {
		[data setCombi:[chk state] == NSOnState ? 1 : 0];
	}
	[data savePreset:attr.idx];
	[view reloadData];
}

- (void)clickJoyAxis:(id)sender
{
	CocoaCheckBox *chk = (CocoaCheckBox *)sender;
	Uint32 bits = 0;
	switch([chk index]) {
	case 2:
		bits = (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT);
		break;
	case 3:
		bits = (JOYCODE_R_UP | JOYCODE_R_DOWN);
		break;
	case 4:
		bits = (JOYCODE_U_LEFT | JOYCODE_U_RIGHT);
		break;
	case 5:
		bits = (JOYCODE_V_UP | JOYCODE_V_DOWN);
		break;
	}
	BIT_ONOFF(enable_axes, bits, [chk state] == NSOnState);
}

@end
