/** @file cocoa_recvidpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.05.21 -

 @brief [ record video panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_recvidpanel.h"
#import "../../video/rec_video.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "../../utility.h"

extern EMU *emu;

static const char *type_label[] = {
#ifdef USE_REC_VIDEO
#ifdef USE_REC_VIDEO_AVKIT
	_T("avkit"),
#endif
#ifdef USE_REC_VIDEO_QTKIT
	_T("qtkit"),
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_VIDEO
#ifdef USE_REC_VIDEO_AVKIT
	RECORD_VIDEO_TYPE_AVKIT,
#endif
#ifdef USE_REC_VIDEO_QTKIT
	RECORD_VIDEO_TYPE_QTKIT,
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	RECORD_VIDEO_TYPE_FFMPEG,
#endif
#endif
	0 };

@implementation CocoaRecVideoPanel
- (id)initWithCont:(bool)cont
{
	int i;
	for(i=0; i < COCOA_RECVIDEO_LIBS; i++) {
		codnums[i] = 0;
		quanums[i] = 0;
		enables[i] = false;
		codbtn[i] = nil;
		quabtn[i] = nil;
	}

	int codnum = emu->get_parami(VM::ParamRecVideoCodec);
	int quanum = emu->get_parami(VM::ParamRecVideoQuality);
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecVideoType)) {
			typnum = i;
			codnums[i] = codnum;
			quanums[i] = quanum;
			break;
		}
	}

	[super init];

	[self setTitleById:CMsg::Record_Screen];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	tabView = [CocoaTabView create:box_tab width:300 height:200];
	CocoaRecVideoTabViewDelegate *dele = [[CocoaRecVideoTabViewDelegate alloc] initWithPanel:self];
	[tabView setDelegate:dele];

	NSTabViewItem *tab;
	CocoaView *tab_view;
//	CocoaLabel *label;

	CocoaLayout *vbox;
	CocoaLayout *hbox;

	char name[10];

	for(i=0; type_ids[i] != 0; i++) {
		tab = [tabView addTabItemT:_tgettext(type_label[i])];
		tab_view = (CocoaView *)[tab view];
		[box_tab setContentView:tab_view];

		enables[i] = emu->rec_video_enabled(type_ids[i]);

		UTILITY::sprintf(name, sizeof(name), "V%d", i);
		vbox = [box_tab addBox:VerticalBox :0 :0 :name];
		UTILITY::sprintf(name, sizeof(name), "H%d_0", i);
		hbox = [vbox addBox:HorizontalBox :CenterPos | MiddlePos :0 :name];

		[CocoaLabel createI:hbox title:CMsg::Codec_Type width:120];

		const char **codlbl = emu->get_rec_video_codec_list(type_ids[i]);
		codbtn[i] = [CocoaPopUpButton createT:hbox items:codlbl action:nil selidx:codnums[i] width:160];

		UTILITY::sprintf(name, sizeof(name), "H%d_1", i);
		hbox = [vbox addBox:HorizontalBox :CenterPos | MiddlePos :0 :name];

		[CocoaLabel createI:hbox title:CMsg::Quality width:120];

		const CMsg::Id *qualbl = emu->get_rec_video_quality_list(type_ids[i]);
		quabtn[i] = [CocoaPopUpButton createI:hbox items:qualbl action:nil selidx:quanums[i] width:160];

		if (!enables[i]) {
			UTILITY::sprintf(name, sizeof(name), "E%d", i);
			hbox = [vbox addBox:HorizontalBox :0 :0 :name];
			[CocoaLabel createI:hbox title:CMsg::Need_install_library];
		}
	}

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	btnOK = [CocoaButton createI:hbox title:cont ? CMsg::Next : CMsg::Start action:@selector(dialogOk:) width:120];

	[box_all realize:self];

	return self;
}

- (NSInteger)runModal
{
	return type_ids[0] ? [NSApp runModalForWindow:self] : NSModalResponseCancel;
}

- (void)close
{
	[NSApp stopModalWithCode:NSModalResponseCancel];
	[super close];
}

- (void)dialogOk:(id)sender
{
	typnum = (int)[tabView indexOfTabViewItem:[tabView selectedTabViewItem]];
	int codnum = (int)[codbtn[typnum] indexOfSelectedItem];
	int quanum = (int)[quabtn[typnum] indexOfSelectedItem];
	emu->set_parami(VM::ParamRecVideoType, type_ids[typnum]);
	emu->set_parami(VM::ParamRecVideoCodec, codnum);
	emu->set_parami(VM::ParamRecVideoQuality, quanum);

    // OK button is pushed
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}
- (void)selectTab:(int)num
{
	[btnOK setEnabled:enables[num]];
}

@end

@implementation CocoaRecVideoTabViewDelegate
- (id)initWithPanel:(CocoaRecVideoPanel *)new_panel
{
	[super init];
	panel = new_panel;
	return self;
}
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	[panel selectTab:(int)[tabView indexOfTabViewItem:tabViewItem]];
}

@end

