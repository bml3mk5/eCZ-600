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
#import "../../utility.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

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
		CocoaTableView *tableView = [CocoaTableView createW:420 height:400 tabnum:tab_num cellWidth:120];
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
		[box_one addControl:tv width:420 height:400];

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
			UTILITY::sprintf(name, sizeof(name), CMSG(Load_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:box_one title:name action:@selector(loadPreset:) width:180];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];

		}

		[box_one addSpace:180 :32];
		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(name, sizeof(name), CMSG(Save_Preset_VDIGIT), i+1);
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
	[NSApp stopModalWithCode:NSModalResponseCancel];
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

	[NSApp stopModalWithCode:NSModalResponseOK];
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
