/** @file cocoa_keybindctrl.h

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2024.01.07 -

 @brief [ keybind control ]
 */

#ifndef COCOA_KEYBINDCTRL_H
#define COCOA_KEYBINDCTRL_H

#import <Cocoa/Cocoa.h>
#import "cocoa_basepanel.h"
#import "../../vm/vm_defs.h"
#import "../../emu.h"
#import "../gui_keybinddata.h"

#define USE_NSCELL 1

typedef struct selected_st {
	int row;
	int col;
} selected_cell_t;

//@class CocoaTableFieldView;
@class CocoaTableView;

/**
	@brief key table data for keybind dialog
*/
@interface CocoaTableData : NSObject <NSTableViewDataSource>
{
	KeybindData kbdata;
	CocoaTableView *parent;
	selected_cell_t selected;
	Uint32 *joy_mask;
}
- (id)initWithValue:(CocoaTableView *)new_parent :(int)new_tabnum;
- (Uint32)combi;
- (void)setCombi:(Uint32)value;
- (int)tab_num;
- (int)devType;
- (int)selectedCol;
- (void)setSelectCol:(int)col row:(int)row;
- (void)reloadCell:(int)col row:(int)row;
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)obj forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;

//- (void)SetVmKeyMap:(Uint16 *)vmKeyMap :(int)size;
- (void)SetVmKey:(int)idx :(Uint16)code;
- (bool)SetVmKeyCode:(int)idx :(Uint16)code;
//- (void)SetVkKeyMap:(uint32_key_assign_t *)vkKeyMap;
//- (void)SetVkKeyDefMap:(const uint32_key_assign_t *)vkKeyDefMap :(int)rows;
//- (void)SetVkKeyPresetMap:(uint32_key_assign_t *)vkKeyMap :(int)idx;
//- (bool)SetVkKeyCodeR:(int)row :(codecols_t *)obj :(Uint32)code :(char *)label;
- (bool)SetVkKeyCode:(int)row :(int)col :(Uint32)code :(char *)label;
- (bool)SetVkKeyCode:(Uint32)code :(char *)label;
- (bool)ClearVkKeyCode:(char *)label;
- (bool)ClearCellByCode:(Uint32)code :(char *)label :(int *)row :(int *)col;
- (bool)SetVkJoyCode:(Uint32)code0 :(Uint32)code1 :(char *)label;
- (bool)ClearVkJoyCode:(char *)label;
- (void)loadDefaultPreset;
- (void)loadPreset:(int)idx;
- (void)savePreset:(int)idx;
- (void)SetData;

- (void)onClickCell:(NSTableView *)sender;
- (void)onDoubleClickCell:(NSTableView *)sender;

- (void)setJoyMask:(Uint32 *)mask;
- (Uint32)joyMask;
@end

#ifndef USE_NSCELL
/**
	@brief Text control in key table
*/
@interface CocoaNText : NSView
{
	CocoaTableData *dataSource;
	int col;
	int row;

	Uint32 *joy_stat;	 // joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4-b31 = trigger #1-#28
	NSTimer *timer;

	NSAttributedString *str;

	BOOL selClick;
}
- (id)initWithDataSource:(NSTableView *)new_tbl data:(CocoaTableData *)new_data col:(NSTableColumn *)tableColumn row:(int)new_row;
- (void)dealloc;
- (BOOL)acceptsFirstResponder;
- (void)keyDown:(NSEvent *)event;
- (void)flagsChanged:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)updateJoy;
- (void)setString:(NSString *)aString;
- (void)drawRect:(NSRect)theRect;
//- (void)resetCursorRects;
@end
#endif /* !USE_NSCELL */

#ifdef USE_NSCELL
/**
	@brief Text control in key table
*/
@interface CocoaText : NSTextView
{
	CocoaTableData *dataSource;

	NSTimer *timer;
}
- (id)initWithDataSource:(CocoaTableData *)new_data;
- (void)dealloc;
- (void)keyDown:(NSEvent *)event;
- (void)flagsChanged:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
- (BOOL)performKeyEquivalent:(NSEvent *)event;
- (void)mouseDown:(NSEvent *)event;
- (void)updateJoy;
@end

/**
	@brief text cell in key table
*/
@interface CocoaTextCell : NSTextFieldCell
- (id)init;
- (id)initWithCoder:(NSCoder *)decoder;
- (id)initTextCell:(NSString *)aString;
#if !defined(MAC_OS_X_VERSION_10_13)
- (id)initImageCell:(NSImage *)anImage;
#endif
- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength;
@end
#endif /* USE_NSCELL */

/**
	@brief Tables in keybind control
*/
@interface CocoaTableView : NSScrollView
{
	CocoaCheckBox *chkCombi;
	int cell_width;
}
@property (strong,nonatomic) CocoaCheckBox *chkCombi;
+ (CocoaTableView *)create:(NSRect)re tabnum:(int)tab_num cellWidth:(int)cellWidth;
+ (CocoaTableView *)createW:(int)width height:(int)height tabnum:(int)tab_num cellWidth:(int)cellWidth;
- (CocoaTableData *)data;
- (void)reloadCell:(int)col row:(int)row;
- (void)deleteColumns;
- (void)setJoyMask:(Uint32 *)mask;
@end

#ifndef USE_NSCELL
/**
	@brief Tables in keybind control
*/
@interface CocoaTableViewDelegate : NSObject<NSTableViewDelegate>
{
	NSMutableDictionary *cells;
}
- (id)init;
- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;
//- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *) tableColumn row:(NSInteger)row;
//- (void)tableViewSelectionIsChanging:(NSNotification *)notification;
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row;
- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row;
@end
#endif /* !USE_NSCELL */

/**
	@brief Button control in keybind control
*/
@interface CocoaButtonAttr : NSObject
{
	int tabNum;
	int idx;
}
@property (nonatomic) int tabNum;
@property (nonatomic) int idx;
- (id)initWithValue:(int)new_tabnum :(int)new_idx;
@end

#endif /* COCOA_KEYBINDCTRL_H */
