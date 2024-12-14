/** @file cocoa_keybindctrl.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.01.07 -

 @brief [ keybind control ]
 */

#import "cocoa_gui.h"
#import "cocoa_keybindctrl.h"
#import "../../config.h"
#import "../../emumsg.h"
#import "../../msgs.h"
#import "../../labels.h"
#import "../../keycode.h"
#import "../../utility.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

/**
	key table data for keybind dialog
*/
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
- (int)flags
{
	return kbdata.m_flags;
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

//- (void)SetVmKeyMap:(Uint16 *)vmKeyMap :(int)size
//{
//	kbdata.SetVmKeyMap(vmKeyMap,size);
//}

- (void)SetVmKey:(int)idx :(Uint16)code
{
	kbdata.SetVmKey(idx,code);
}

- (bool)SetVmKeyCode:(int)idx :(Uint16)code
{
	return kbdata.SetVmKeyCode(idx,code);
}

//- (void)SetVkKeyMap:(uint32_key_assign_t *)vkKeyMap
//{
//	kbdata.SetVkKeyMap(vkKeyMap);
//}

//- (void)SetVkKeyDefMap:(const uint32_key_assign_t *)vkKeyDefMap :(int)rows
//{
//	kbdata.SetVkKeyDefMap(vkKeyDefMap, rows);
//}

//- (void)SetVkKeyPresetMap:(uint32_key_assign_t *)vkKeyMap :(int)idx
//{
//	kbdata.SetVkKeyPresetMap(vkKeyMap, idx);
//}

//- (bool)SetVkKeyCodeR:(int)row :(codecols_t *)obj :(Uint32)code :(char *)label
//{
//	  return kbdata.SetVkKeyCode(row,obj,code,label);
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
	if (kbdata.m_flags == KeybindData::FLAG_DENY_DUPLICATE) {
		return kbdata.ClearCellByVkKeyCode(code, label, row, col);
	} else {
		*row = -1;
		*col = -1;
		return true;
	}
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
/**
	Text control in key table
*/
@implementation CocoaNText
- (id)initWithDataSource:(NSTableView *)new_tbl data:(CocoaTableData *)new_data col:(NSTableColumn *)tableColumn row:(int)new_row
{
	[super init];
//	  [self setEditable:YES];
//	  [self setSelectable:YES];
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
//		  emu->reset_joystick();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		  [loop addTimer:timer forMode:NSRunLoopCommonModes];
		[loop addTimer:timer forMode:NSModalPanelRunLoopMode];
	} else {
		joy_stat = NULL;
		timer = nil;
	}
//	  [self setFocusRingType:NSFocusRingTypeDefault];
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
//	  NSRect rect = [self bounds];
//	  NSCursor* cursor = [NSCursor IBeamCursor];
//	  [self addCursorRect:rect cursor:cursor];
//}
@end
#endif /* !USE_NSCELL */

#ifdef USE_NSCELL
/**
	Text control in key table
*/
@implementation CocoaText
- (id)initWithDataSource:(CocoaTableData *)new_data
{
	[super init];
	[self setEditable:YES];
	[self setSelectable:YES];
//	  [self setBordered:YES];
	dataSource = new_data;

	if ([dataSource devType] == KeybindData::DEVTYPE_JOYPAD) {
		// update joystick status per 0.1 sec
//		  emu->reset_joystick();
		NSRunLoop *loop = [NSRunLoop currentRunLoop];
		timer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(updateJoy) userInfo:nil repeats:YES];
//		  [loop addTimer:timer forMode:NSRunLoopCommonModes];
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
	int devtype = [dataSource devType];
	if (devtype == KeybindData::DEVTYPE_KEYBOARD) {
		[dataSource ClearVkKeyCode:label];
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
	int devtype = [dataSource devType];
	if (devtype == KeybindData::DEVTYPE_KEYBOARD) {
		[dataSource ClearVkKeyCode:label];
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
	int devtype = [dataSource devType];
	if (devtype == KeybindData::DEVTYPE_KEYBOARD) {
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
	int devtype = [dataSource devType];
	if (devtype == KeybindData::DEVTYPE_KEYBOARD) {
		[dataSource ClearVkKeyCode:label];
	} else {
		[dataSource ClearVkJoyCode:label];
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

/**
	text cell in key table
*/
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
//	  [super init];
	[super initTextCell:aString];
	[self setEditable:YES];
	[self setSelectable:YES];
	[self setBordered:YES];
	return self;
}
#if !defined(MAC_OS_X_VERSION_10_13)
- (id)initImageCell:(NSImage *)anImage
{
//	  [super init];
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

#define COLUMN_WIDTH 100.0

/**
	Tables in keybind control
*/
@implementation CocoaTableView
@synthesize chkCombi;
+ (CocoaTableView *)create:(NSRect)re tabnum:(int)tab_num cellWidth:(int)cellWidth
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me initWithFrame:re];
	[me createMain:re.size tabnum:tab_num cellWidth:cellWidth];
	return me;
}
+ (CocoaTableView *)createW:(int)width height:(int)height tabnum:(int)tab_num cellWidth:(int)cellWidth
{
	CocoaTableView *me = [CocoaTableView alloc];
	[me init];
	NSSize sz = NSMakeSize(width, height);
	[me createMain:sz tabnum:tab_num cellWidth:cellWidth];
	return me;
}
- (void)createMain:(NSSize)sz tabnum:(int)tab_num cellWidth:(int)cellWidth
{
	NSRect re_ctl;

	chkCombi = nil;
	cell_width = cellWidth;
	[self setBorderType:NSBezelBorder];

	re_ctl = NSMakeRect(0,0,sz.width,24);
	NSScroller *hs = [[NSScroller alloc] initWithFrame:re_ctl];
//	  [self addSubview:hs];
	[self setHorizontalScroller:hs];
	[self setHasHorizontalScroller:YES];
	re_ctl = NSMakeRect(0,0,24,sz.height);
	NSScroller *vs = [[NSScroller alloc] initWithFrame:re_ctl];
//	  [self addSubview:vs];
	[self setVerticalScroller:vs];
	[self setHasVerticalScroller:YES];

	[self setAutohidesScrollers:NO];
//	  [self setAutoresizesSubviews:YES];

//	  re_ctl = NSMakeRect(10,10,sz.width,24);
	NSTableHeaderView *hv = [[NSTableHeaderView alloc] init];

//	  re_ctl = NSMakeRect(10,128,sz.width, sz.height);
	NSTableView *table = [[NSTableView alloc] init];
//	  [me addSubview:table];
	[table setHeaderView:hv];

	[table setAllowsColumnReordering:NO];
	[table setAllowsColumnResizing:NO];
	[table setAllowsMultipleSelection:NO];
	[table setAllowsEmptySelection:YES];
//	  [table setGridStyleMask:(NSTableViewSolidVerticalGridLineMask | NSTableViewSolidHorizontalGridLineMask)];
//	  [table setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	char label[128];
	CMsg::Id label_id;

	NSTableColumn *col;
	col = [[NSTableColumn alloc] initWithIdentifier:@"vmkey"];

	label_id = LABELS::keybind_col[tab_num][0];

	[col.headerCell setStringValue:[NSString stringWithUTF8String:gMessages.Get(label_id)]];
	[col setWidth:cell_width];
	[col setEditable:NO];
	[table addTableColumn:col];
	for(int i=0; i<2; i++) {
		UTILITY::sprintf(label, sizeof(label), "%d", i);
		col = [[NSTableColumn alloc] initWithIdentifier:[NSString stringWithUTF8String:label]];
		UTILITY::sprintf(label, sizeof(label), CMSGV(LABELS::keybind_col[tab_num][1]), i+1);
		[col.headerCell setStringValue:[NSString stringWithUTF8String:label]];
#ifdef USE_NSCELL
		[col setDataCell:[[CocoaTextCell alloc] init]];
#endif
		[col setWidth:cell_width];
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
//	  [table setDoubleAction:@selector(onDoubleClickCell:)];
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
- (void)setCellWidth:(int)width
{
	cell_width = width;
}
- (void)setJoyMask:(Uint32 *)mask
{
	[[self data] setJoyMask:mask];
}
- (void)SetData
{
	[self setCombiCheckData];
	[[self data] SetData];
}
- (void)LoadDefaultPresetData
{
	[[self data] loadDefaultPreset];
	[self updateCombiCheckButton];
}
- (void)LoadPresetData:(int)idx
{
	[[self data] loadPreset:idx];
	[self updateCombiCheckButton];
}
- (void)SavePresetData:(int)idx
{
	[self setCombiCheckData];
	[[self data] savePreset:idx];
}
- (CocoaCheckBox *)addCombiCheckButton:(CocoaLayout *)layout tabnum:(int)tab_num
{
	if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
		bool val = [[self data] combi];
		chkCombi = [CocoaCheckBox createI:layout title:LABELS::keybind_combi[tab_num] action:nil value:val];
	}
	return chkCombi;
}
- (void)setCombiCheckData
{
	if (chkCombi) {
		[[self data] setCombi:[chkCombi state] == NSControlStateValueOn ? 1 : 0];
	}
}
- (void)updateCombiCheckButton
{
	if (chkCombi) {
		[chkCombi setState:[[self data] combi] != 0 ? NSControlStateValueOn : NSControlStateValueOff];
	}
}
@end

#ifndef USE_NSCELL
/**
	Tables in keybind control
*/
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
//	  NSLog(@"%@:%d",[tableColumn identifier], row);
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
#endif /* !USE_NSCELL */

/**
	Button control in keybind control
*/
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
