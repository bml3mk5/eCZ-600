/** @file cocoa_recaudpanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2016.11.05 -

 @brief [ record audio panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_recaudpanel.h"
#import "../../video/rec_audio.h"
#import "../../config.h"
#import "../../emu.h"
#import "../../clocale.h"
#import "../../utility.h"

extern EMU *emu;

static const char *type_label[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
	_T("avkit"),
#endif
#ifdef USE_REC_AUDIO_WAVE
	_T("wave"),
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	_T("ffmpeg"),
#endif
#endif
	NULL };
static const int type_ids[] = {
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_AVKIT
	RECORD_AUDIO_TYPE_AVKIT,
#endif
#ifdef USE_REC_AUDIO_WAVE
	RECORD_AUDIO_TYPE_WAVE,
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	RECORD_AUDIO_TYPE_FFMPEG,
#endif
#endif
	0 };

@implementation CocoaRecAudioPanel
- (id)init
{
	int i;
	for(i=0; i < COCOA_RECAUDIO_LIBS; i++) {
		codnums[i] = 0;
		quanums[i] = 0;
		enables[i] = false;
		codbtn[i] = nil;
		quabtn[i] = nil;
	}

	int codnum = emu->get_parami(VM::ParamRecAudioCodec);
//	int quanum = emu->get_parami(VM::ParamRecAudioQuality);
	typnum = 0;
	for(i=0; type_ids[i] != 0; i++) {
		if (type_ids[i] == emu->get_parami(VM::ParamRecAudioType)) {
			typnum = i;
			codnums[i] = codnum;
//			quanums[i] = quanum;
			break;
		}
	}

	[super init];

	[self setTitleById:CMsg::Record_Sound];
	[self setShowsResizeIndicator:NO];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	tabView = [CocoaTabView create:box_tab width:300 height:32];
	CocoaRecAudioTabViewDelegate *dele = [[CocoaRecAudioTabViewDelegate alloc] initWithPanel:self];
	[tabView setDelegate:dele];


	NSTabViewItem *tab;
	CocoaView *tab_view;
//	CocoaLabel *label;

	CocoaLayout *vbox;
	CocoaLayout *hbox;

	for(i=0; type_ids[i] != 0; i++) {
		tab = [tabView addTabItemT:_tgettext(type_label[i])];
		tab_view = (CocoaView *)[tab view];
		[box_tab setContentView:tab_view];

		enables[i] = emu->rec_sound_enabled(type_ids[i]);

		char name[10];
		UTILITY::sprintf(name, sizeof(name), "V%d", i);
		vbox = [box_tab addBox:VerticalBox :0 :0 :name];
		UTILITY::sprintf(name, sizeof(name), "H%d", i);
		hbox = [vbox addBox:HorizontalBox :CenterPos | MiddlePos :0 :name];

		switch(type_ids[i]) {
		case RECORD_AUDIO_TYPE_WAVE:
			[CocoaLabel createI:hbox title:CMsg::Select_a_sample_rate_on_sound_menu_in_advance];
			break;
		default:
			[CocoaLabel createI:hbox title:CMsg::Codec_Type width:120];

			const char **codlbl = emu->get_rec_sound_codec_list(type_ids[i]);
			codbtn[i] = [CocoaPopUpButton createT:hbox items:codlbl action:nil selidx:codnums[i] width:160];

//			[CocoaLabel create:re titleid:CMsg::Quality];

//			const char **qualbl = emu->get_rec_audio_quality_list(type_ids[i]);
//			quabtn[i] = [CocoaPopUpButton create:re items:qualbl action:nil selidx:quanums[i]];
			break;
		}

		if (!enables[i]) {
			hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Hlib")];
			[CocoaLabel createI:hbox title:CMsg::Need_install_library];
		}
	}

	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	btnOK = [CocoaButton createI:hbox title:CMsg::Start action:@selector(dialogOk:) width:120];

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
//	int quanum = (int)[quabtn[typnum] indexOfSelectedItem];
	emu->set_parami(VM::ParamRecAudioType, type_ids[typnum]);
	emu->set_parami(VM::ParamRecAudioCodec, codnum);
//	emu->set_parami(VM::ParamRecAudioQuality, quanum);

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

@implementation CocoaRecAudioTabViewDelegate
- (id)initWithPanel:(CocoaRecAudioPanel *)new_panel
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

