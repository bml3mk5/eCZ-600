/** @file cocoa_joysetpanel.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2023.01.07 -

 @brief [ joypad setting panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_joysetpanel.h"
#import "../../config.h"
#import "../../emumsg.h"
#import "../../msgs.h"
#import "../../labels.h"
#import "../../keycode.h"
#import "../../utility.h"
#import "../gui_keybinddata.h"
#import "cocoa_keybindctrl.h"
#import "cocoa_key_trans.h"

extern EMU *emu;

@implementation CocoaJoySettingPanel
- (id)init
{
	char label[64];

	[super init];

	SDL_QZ_InitOSKeymap();

	[self setTitleById:CMsg::Joypad_Setting];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_hall = [box_all addBox:HorizontalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_hall")];
	CocoaLayout *hbox;

	int tx = 80;
	int sx = 140;
	int sy = 24;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		CocoaLayout *vbox = [box_hall addBox:VerticalBox];

		UTILITY::stprintf(label, 64, CMSG(JoypadVDIGIT), i + 1);
		[CocoaLabel createT:vbox title:label];
		int val;
#ifdef USE_JOYSTICK_TYPE
		val = pConfig->joy_type[i];
		pop[i] = [CocoaPopUpButton createI:vbox items:LABELS::joypad_type action:nil selidx:val];
#else
		pop[i] = nil;
#endif

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Button_Mashing_Speed];
		[hbox addSpace:32 :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 3")];

		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			int kk = k + VM_JOY_LABEL_BUTTON_A;
			if (kk >= VM_JOY_LABELS_MAX) {
				mash[i][k] = nil;
				continue;
			}
			
			hbox = [vbox addBox:HorizontalBox];
			CMsg::Id titleid = (CMsg::Id)cVmJoyLabels[kk].id;
			[CocoaLabel createI:hbox title:titleid align:NSTextAlignmentCenter width:tx height:sy];

			val = pConfig->joy_mashing[i][k];
			int n = i * KEYBIND_JOY_BUTTONS + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:3 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:4];
			mash[i][k] = slider;
		}

		hbox = [vbox addBox:HorizontalBox];
		[CocoaLabel createI:hbox title:CMsg::Analog_to_Digital_Sensitivity];
		[hbox addSpace:32 :sy];
		[CocoaLabel createT:hbox title:_T("0 <-> 10")];

		for(int k=0; k < 6; k++) {
			hbox = [vbox addBox:HorizontalBox];

			CMsg::Id titleid = LABELS::joypad_axis[k];
			[CocoaLabel createI:hbox title:titleid align:NSTextAlignmentCenter width:tx height:sy];

			val = 10 - pConfig->joy_axis_threshold[i][k];
			int n = i * 6 + k;
			CocoaSlider *slider = [CocoaSlider createN:hbox index:n action:nil min:0 max:10 value:val width:sx height:sy];
			[slider.cell setVertical:NO];
			[slider setAllowsTickMarkValuesOnly:YES];
			[slider setNumberOfTickMarks:11];
			axis[i][k] = slider;
		}
	}
#endif

	// tab control

	CocoaLayout *box_vall = [box_hall addBox:VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_vall")];
	CocoaLayout *box_tab = [box_vall addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];
	CocoaLayout *box_sep;
	CocoaLayout *box_one;

	CocoaTabView *tabView = [CocoaTabView create:box_tab width:370 height:360];
	CocoaView *tab_view;

	CocoaButton *btn;
	NSTabViewItem *tab;
	int tab_offset = KeybindData::TAB_JOY2JOY;

	tableViews = [NSMutableArray array];
	for(int tab_num=tab_offset; tab_num<KeybindData::TABS_MAX; tab_num++) {
		CocoaTableView *tableView = [CocoaTableView createW:370 height:360 tabnum:tab_num cellWidth:100];
		[tableView setJoyMask:&enable_axes];
		[tableViews addObject:tableView];
	}

	// create an item in the tab
	for(int tab_num=0; tab_num<[tableViews count]; tab_num++) {
		tab = [tabView addTabItemI:LABELS::keybind_tab[tab_num+tab_offset]];
		tab_view = (CocoaView *)[tab view];
		[box_tab setContentView:tab_view];

		// table
		box_sep = [box_tab addBox:VerticalBox :0 :0];

		CocoaTableView *tv = [tableViews objectAtIndex:tab_num];
		[box_sep addControl:tv width:370 height:360];

		// checkbox
#if 0
		if (LABELS::keybind_combi[tab_num] != CMsg::Null) {
			CocoaCheckBox *chk = [CocoaCheckBox createI:box_one title:LABELS::keybind_combi[tab_num] action:nil value:[[tv data] combi]];
			[tv setChkCombi:chk];
		}
#endif
		// button (lower)

		btn = [CocoaButton createI:box_sep title:CMsg::Load_Default action:@selector(loadDefaultPreset:) width:160];
		[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:-1]];

		CocoaLayout *box_hbtn = [box_sep addBox:HorizontalBox :0 :0];
		box_one = [box_hbtn addBox:VerticalBox :0 :0];

		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Load_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:box_one title:label action:@selector(loadPreset:) width:160];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];

		}

		box_one = [box_hbtn addBox:VerticalBox :0 :0];

		for(int i=0; i<KEYBIND_PRESETS; i++) {
			UTILITY::sprintf(label, sizeof(label), CMSG(Save_Preset_VDIGIT), i+1);
			btn = [CocoaButton createT:box_one title:label action:@selector(savePreset:) width:160];
			[btn setRelatedObject:[[CocoaButtonAttr alloc]initWithValue:tab_num:i]];
		}
	}

	// axes of joypad
	
	enable_axes = ~0;
	CocoaLayout *hbox_joy = [box_vall addBox:HorizontalBox];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_Z_axis index:2 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_Z_LEFT | JOYCODE_Z_RIGHT)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_R_axis index:3 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_R_UP | JOYCODE_R_DOWN)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_U_axis index:4 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_U_LEFT | JOYCODE_U_RIGHT)) != 0];
	[CocoaCheckBox createI:hbox_joy title:CMsg::Enable_V_axis index:5 action:@selector(clickJoyAxis:) value:(enable_axes & (JOYCODE_V_UP | JOYCODE_V_DOWN)) != 0];

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
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
#if 0
		CocoaCheckBox *chk = [tv chkCombi];
		if (chk) {
			[data setCombi:[chk state] == NSControlStateValueOn ? 1 : 0];
		}
#endif
	}

	emu->save_keybind();

	[self setData];

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
		[data setCombi:[chk state] == NSControlStateValueOn ? 1 : 0];
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
	BIT_ONOFF(enable_axes, bits, [chk state] == NSControlStateValueOn);
}

- (void)setData
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
#ifdef USE_JOYSTICK_TYPE
		pConfig->joy_type[i] = (int)[pop[i] indexOfSelectedItem];
#endif
		for(int k=0; k<KEYBIND_JOY_BUTTONS; k++) {
			pConfig->joy_mashing[i][k] = [mash[i][k] intValue];
		}
		for(int k=0; k<6; k++) {
			pConfig->joy_axis_threshold[i][k] = 10 - [axis[i][k] intValue];
		}
	}
	emu->modify_joy_mashing();
	emu->modify_joy_threshold();
#ifdef USE_JOYSTICK_TYPE
	// will change joypad type in emu thread
	emumsg.Send(EMUMSG_ID_MODIFY_JOYTYPE);
#endif
#endif
}

#if 0
- (void)changeSlider:(CocoaSlider *)sender
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int i = [sender index];
	int k = i % KEYBIND_JOY_BUTTONS;
	i /= KEYBIND_JOY_BUTTONS;
	pConfig->joy_mashing[i][k] = [sender intValue];
	emu->set_joy_mashing();
#endif
}
#endif

@end
