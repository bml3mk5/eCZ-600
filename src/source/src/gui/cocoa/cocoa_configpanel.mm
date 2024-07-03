/** @file cocoa_configpanel.mm

 SHARP X68000 Emulator 'eCZ-600'
 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2022.09.22 -

 @brief [ config panel ]
 */

#import "cocoa_gui.h"
#import "cocoa_configpanel.h"
#import "../../config.h"
#import "../../emu.h"
#import "../gui.h"
#import "../../clocale.h"
#import "../../msgboard.h"
#import "../../msgs.h"
#import "../../rec_video_defs.h"
#import "cocoa_fontpanel.h"
#import "../../labels.h"
#import "../../utility.h"

extern EMU *emu;
extern GUI *gui;

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

@implementation CocoaConfigPanel
- (id)init
{
	[super init];

	[self setTitleById:CMsg::Configure];
	[self setShowsResizeIndicator:FALSE];

	CocoaView *view = [self contentView];

	CocoaLayout *box_all = [CocoaLayout create:view :VerticalBox :0 :COCOA_DEFAULT_MARGIN :_T("box_all")];
	CocoaLayout *box_tab = [box_all addBox:TabViewBox :0 :COCOA_DEFAULT_MARGIN :_T("box_tab")];

	CocoaTabView *tabView = [CocoaTabView createI:box_tab tabs:LABELS::tabs width:300 height:200];

	NSTabViewItem *tab;
	CocoaView *tab_view;
//	CocoaBox *grp;
//	CocoaLabel *lbl;
	CocoaButton *btn;

	CocoaLayout *box_one;
	CocoaLayout *sbox;
	CocoaLayout *lbox;
	CocoaLayout *mbox;
	CocoaLayout *rbox;
	CocoaLayout *bbox;
	CocoaLayout *vbox;
	CocoaLayout *hbox;

	int i;
	int valuei = 0;
	uint32_t valueu = 0;

	_TCHAR bname[10];
	char str[128];

	// ----------------------------------------
	// CPU, Memory tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:0];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("CPU")];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("ROMPath")];
	[CocoaLabel createI:hbox title:CMsg::ROM_Path_ASTERISK];
	txtROMPath = [CocoaTextField createT:hbox text:pConfig->rom_path.Get() action:nil width:300];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtROMPath];

#if defined(_X68000)
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("RAM")];
	[CocoaBox createI:bbox :CMsg::RAM :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("RAM1")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("Size")];
	[CocoaLabel createI:hbox title:CMsg::RAM_Size_ASTERISK];
	popMainRam = [CocoaPopUpButton createI:hbox items:LABELS::main_ram_size action:nil selidx:emu->get_parami(VM::ParamMainRamSizeNum) width:120];
	if (0 <= pConfig->main_ram_size_num && pConfig->main_ram_size_num <= 5) {
		char str[128];
		strcpy(str, CMSG(LB_Now_SP));
		strcat(str, gMessages.Get(LABELS::main_ram_size[pConfig->main_ram_size_num]));
		strcat(str, ")");
		[CocoaLabel createT:hbox title:str];
	}
	// overwrite size on SRAM
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("owSRAM")];
	chkOwSizeSRAM = [CocoaCheckBox createI:hbox title:CMsg::No_overwrite_RAM_size_on_SRAM action:nil value:FLG_ORIG_SRAM_RAM_SIZE != 0];

	// CPU
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("CPU0")];
	[CocoaBox createI:bbox :CMsg::CPU :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("CPU1")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CPU11")];

	chkAddrErr = [CocoaCheckBox createI:hbox title:CMsg::Show_message_when_the_address_error_occured_in_the_CPU action:nil value:FLG_SHOWMSG_ADDRERR != 0];
#endif

	// power status
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Power")];
	[CocoaBox createI:bbox :CMsg::Behavior_of_Power_On_Off :260 :1];
	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Power1")];

	// power off
	chkPowerOff = [CocoaCheckBox createI:vbox title:CMsg::Enable_the_state_of_power_off action:nil value:pConfig->use_power_off];

	// power state when start up
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("PowerS")];
	[CocoaLabel createI:hbox title:CMsg::Power_State_When_Start_Up];
	popPowerState = [CocoaPopUpButton createI:hbox items:LABELS::power_state action:nil selidx:pConfig->power_state_when_start_up width:240];

	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];


	// ----------------------------------------
	// SRAM tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:1];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("SRAM")];

#if defined(_X68000)
	// behavior
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SRAM")];
	[CocoaBox createI:bbox :CMsg::Behavior :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SRAM1")];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SRAM11")];
	chkSRAMClear = [CocoaCheckBox createI:hbox title:CMsg::Clear_SRAM_when_power_on action:nil value:FLG_ORIG_SRAM_CLR_PWRON != 0];
	[CocoaLabel createT:hbox title:_T(" ")];
	chkSRAMSave = [CocoaCheckBox createI:hbox title:CMsg::Save_SRAM_when_power_off action:nil value:FLG_ORIG_SRAM_SAVE_PWROFF != 0];
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SRAM12")];
	chkSRAMChanged = [CocoaCheckBox createI:hbox title:CMsg::Show_message_when_parameters_related_on_start_up_was_changed action:nil value:FLG_ORIG_SRAM_CHG_BOOT_DEV != 0];

	// parameters in SRAM
	VM *vm = emu->get_vm();

	[box_one addControl:[[NSBox alloc] init] width:256 height:2];

	[CocoaLabel createI:box_one title:CMsg::Parameters_in_SRAM_];

	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("SRAMS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("SRAML")];
	mbox = [sbox addBox:VerticalBox :0 :0 :_T("SRAMM")];
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("SRAMR")];
	
	[mbox addSpace:8 :8];

	// L

	// ram size
	hbox = [lbox addBox:HorizontalBox :0 :0 :_T("ramsize")];
	[CocoaLabel createI:hbox title:CMsg::RAM_Size_Hex];
	valueu = vm->get_sram_ram_size();
	UTILITY::sprintf(str, sizeof(str), "%08x", valueu);
	CocoaTextField *txt = [CocoaTextField createT:hbox text:str action:nil width:120];
	[txt setEditable:FALSE];
	[txt setEnabled:FALSE];

	// rom start address
	hbox = [lbox addBox:HorizontalBox :0 :0 :_T("romstart")];
	[CocoaLabel createI:hbox title:CMsg::ROM_Start_Address_Hex];
	valueu = vm->get_sram_rom_start_address();
	UTILITY::sprintf(str, sizeof(str), "%08x", valueu);
	txtSramRomStartAddr = [CocoaTextField createT:hbox text:str action:nil width:120];

	// ram start address
	hbox = [lbox addBox:HorizontalBox :0 :0 :_T("ramstart")];
	[CocoaLabel createI:hbox title:CMsg::SRAM_Start_Address_Hex];
	valueu = vm->get_sram_sram_start_address();
	UTILITY::sprintf(str, sizeof(str), "%08x", valueu);
	txtSramRamStartAddr = [CocoaTextField createT:hbox text:str action:nil width:120];

	// boot device
	hbox = [lbox addBox:HorizontalBox :0 :0 :_T("bootdev")];
	[CocoaLabel createI:hbox title:CMsg::Boot_Device];
	valuei = vm->get_sram_boot_device();
	popSramBootDevice = [CocoaPopUpButton createT:hbox items:LABELS::boot_devices action:nil selidx:valuei];

	// RS-232C
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("RS232C")];
	[CocoaBox createI:bbox :CMsg::RS_232C :260 :1];
	valueu = vm->get_sram_rs232c();

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("RS232CV")];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("BaudRate")];
	valuei = vm->get_sram_rs232c_baud_rate(valueu);
	[CocoaLabel createI:hbox title:CMsg::Baud_Rate width:120];
	popSramRS232CBaud = [CocoaPopUpButton createT:hbox items:LABELS::rs232c_baudrate action:nil selidx:valuei];
	
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DataBit")];
	valuei = vm->get_sram_rs232c_databit(valueu);
	[CocoaLabel createI:hbox title:CMsg::Data_Bit width:120];
	popSramRS232CDataBit = [CocoaPopUpButton createT:hbox items:LABELS::rs232c_databit action:nil selidx:valuei];
	
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Parity")];
	valuei = vm->get_sram_rs232c_parity(valueu);
	[CocoaLabel createI:hbox title:CMsg::Parity width:120];
	popSramRS232CParity = [CocoaPopUpButton createI:hbox items:LABELS::comm_uart_parity action:nil selidx:valuei];
	
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("StopBit")];
	valuei = vm->get_sram_rs232c_stopbit(valueu);
	[CocoaLabel createI:hbox title:CMsg::Stop_Bit width:120];
	popSramRS232CStopBit = [CocoaPopUpButton createT:hbox items:LABELS::rs232c_stopbit action:nil selidx:valuei];
	
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FlowCtrl")];
	valuei = vm->get_sram_rs232c_flowctrl(valueu);
	[CocoaLabel createI:hbox title:CMsg::Flow_Control width:120];
	popSramRS232CFlowCtrl = [CocoaPopUpButton createI:hbox items:LABELS::rs232c_flowctrl action:nil selidx:valuei];
	
	// R

	// alarm
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("alarm")];
	chkSramAlarm = [CocoaCheckBox createI:hbox title:CMsg::Enable_alarm action:nil value:vm->get_sram_alarm_onoff()];
//	[chk setEnabled:FALSE];

	// alarm time
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("alarmst")];
	valueu = vm->get_sram_alarm_time();
	UTILITY::sprintf(str, sizeof(str), "%08x", valueu);
	[CocoaLabel createI:hbox title:CMsg::Alarm_Time_Hex];
	txt = [CocoaTextField createT:hbox text:str action:nil width:120];
	[txt setEditable:FALSE];
	[txt setEnabled:FALSE];

	// alarm duration
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("alarmdu")];
	valuei = vm->get_sram_alarm_duration();
	[CocoaLabel createI:hbox title:CMsg::Alarm_Duration];
	txt = [CocoaTextField createN:hbox num:valuei action:nil width:120];
	[txt setEditable:FALSE];
	[txt setEnabled:FALSE];

	// contrast
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("contrast")];
	valuei = vm->get_sram_contrast();
	[CocoaLabel createI:hbox title:CMsg::Contrast_on_Monitor];
	stpSramContrast = [CocoaStepper createN:hbox min:0 max:15 value:valuei width:80];

	// purpose of SRAM free area
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("purpose")];
	[CocoaLabel createI:hbox title:CMsg::Purpose_of_SRAM_free_area];
	valuei = vm->get_sram_purpose();
	popSramPurpose = [CocoaPopUpButton createI:hbox items:LABELS::sram_purpose action:nil selidx:valuei];

	// key repeat delay
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("krdelay")];
	[CocoaLabel createI:hbox title:CMsg::Key_Repeat_Delay];
	valuei = vm->get_sram_key_repeat_delay();
	popSramKRDelay = [CocoaPopUpButton createT:hbox items:LABELS::key_repeat_delay action:nil selidx:valuei];

	// key repeat rate
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("krrate")];
	[CocoaLabel createI:hbox title:CMsg::Key_Repeat_Rate];
	valuei = vm->get_sram_key_repeat_rate();
	popSramKRRate = [CocoaPopUpButton createT:hbox items:LABELS::key_repeat_rate action:nil selidx:valuei];

	// key LED
	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("keyLED")];
	[CocoaBox createI:bbox :CMsg::Status_of_Key_LED_when_power_on :260 :1];
	valuei = vm->get_sram_key_led();
	vbox = [bbox addBox:VerticalBox :0 :0 :_T("keyLEDV")];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("keyLEDh1")];
	chkSramKLEDkana = [CocoaCheckBox createI:hbox title:CMsg::Kana action:nil value:(valuei & 1) != 0];
	chkSramKLEDromaji = [CocoaCheckBox createI:hbox title:CMsg::Roma_ji action:nil value:(valuei & 2) != 0];
	chkSramKLEDcinput = [CocoaCheckBox createI:hbox title:CMsg::Code_Input action:nil value:(valuei & 4) != 0];
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("keyLEDh2")];
	chkSramKLEDcaps = [CocoaCheckBox createI:hbox title:CMsg::CAPS action:nil value:(valuei & 8) != 0];
	chkSramKLEDins = [CocoaCheckBox createI:hbox title:CMsg::INS action:nil value:(valuei & 16) != 0];
	chkSramKLEDhira = [CocoaCheckBox createI:hbox title:CMsg::Hiragana action:nil value:(valuei & 32) != 0];
	chkSramKLEDzen = [CocoaCheckBox createI:hbox title:CMsg::Zenkaku action:nil value:(valuei & 64) != 0];

	// Accumulated operating time
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("optime")];
	valueu = vm->get_sram_accumulated_operating_time();
	[CocoaLabel createI:hbox title:CMsg::Accumulated_operating_time];
	txt = [CocoaTextField createN:hbox num:valueu action:nil width:80];
	[CocoaLabel createI:hbox title:CMsg::min_];
	[txt setEditable:FALSE];
	[txt setEnabled:FALSE];

	// Number of times turned off
	hbox = [rbox addBox:HorizontalBox :0 :0 :_T("pwtimes")];
	valueu = vm->get_sram_times_of_the_power_off();
	[CocoaLabel createI:hbox title:CMsg::Times_of_turned_off];
	txt = [CocoaTextField createN:hbox num:valueu action:nil width:80];
	[txt setEditable:FALSE];
	[txt setEnabled:FALSE];
#endif


	// ----------------------------------------
	// Screen tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:2];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Scrn")];
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("ScrnS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("ScrnL")];

	// OpenGL
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("OpenGLB")];
	[CocoaBox createI:bbox :CMsg::Drawing :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("OpenGLV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("UseGLH")];
	[CocoaLabel createI:hbox title:CMsg::Method_ASTERISK];
	popUseOpenGL = [CocoaPopUpButton createI:hbox items:LABELS::opengl_use action:nil selidx:pConfig->use_opengl];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("GLFilH")];
	[CocoaLabel createI:hbox title:CMsg::Filter_Type];
	popGLFilter = [CocoaPopUpButton createI:hbox items:LABELS::opengl_filter action:nil selidx:pConfig->gl_filter_type];

#ifdef USE_SCREEN_MIX_SURFACE
	// double buffering
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("DBuffer")];
	chkDoubleBuffer = [CocoaCheckBox createI:hbox title:CMsg::Use_double_buffering_when_method_is_default action:nil value:pConfig->double_buffering];
#endif

	// border color
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("Border")];
	chkBorderColor = [CocoaCheckBox createI:hbox title:CMsg::Set_gray_color_on_the_border_area action:nil value:FLG_ORIG_BORDER_COLOR != 0];

	// CRTC skew
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("CRTCR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("CRTCB")];
	[CocoaBox createI:bbox :CMsg::CRTC :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("CRTCV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("RasSkewV")];
	[CocoaLabel createI:hbox title:CMsg::Raster_Interrupt_Vertical_Skew];
	txtRasterSkewV = [CocoaTextField createN:hbox num:pConfig->raster_int_vskew action:nil width:80];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("RasSkewH")];
	[CocoaLabel createI:hbox title:CMsg::Raster_Interrupt_Horizontal_Skew];
	txtRasterSkewH = [CocoaTextField createN:hbox num:pConfig->raster_int_hskew action:nil width:80];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("VertSkew")];
	[CocoaLabel createI:hbox title:CMsg::Vertical_Disp_Skew];
	txtVertSkew = [CocoaTextField createN:hbox num:pConfig->vdisp_skew action:nil width:80];

	// LED
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDH")];
	[CocoaLabel createI:hbox title:CMsg::LED];
	int led_show = gui->GetLedBoxPhase(-1);
	popLEDShow = [CocoaPopUpButton createI:hbox items:LABELS::led_show action:nil selidx:led_show];

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("LEDPOSH")];
	[CocoaLabel createI:hbox title:CMsg::Position];
	popLEDPosition = [CocoaPopUpButton createI:hbox items:LABELS::led_pos action:nil selidx:pConfig->led_pos];

	// Capture
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("CaptureH")];
	[CocoaLabel createI:hbox title:CMsg::Capture_Type];
	popCaptureType = [CocoaPopUpButton createT:hbox items:LABELS::capture_fmt action:nil selidx:pConfig->capture_type];

	// Snapshot path
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("SnapPath")];
	[CocoaLabel createI:hbox title:CMsg::Snapshot_Path];
	txtSnapPath = [CocoaTextField createT:hbox text:pConfig->snapshot_path.Get() action:nil width:360];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtSnapPath];

	// Font path
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("FontPath")];
	[CocoaLabel createI:hbox title:CMsg::Font_Path];
	txtFontPath = [CocoaTextField createT:hbox text:pConfig->font_path.Get() action:nil width:360];
	btn = [CocoaButton createI:hbox title:CMsg::Folder_ action:@selector(showFolderPanel:)];
	[btn setRelatedObject:txtFontPath];

	// Message font
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("MsgFont")];
	[CocoaLabel createI:hbox title:CMsg::Message_Font];
	txtMsgFontName = [CocoaTextField createT:hbox text:pConfig->msgboard_msg_fontname.Get() action:nil width:200];

	[CocoaLabel createI:hbox title:CMsg::_Size];
	txtMsgFontSize = [CocoaTextField createN:hbox num:pConfig->msgboard_msg_fontsize action:nil width:100];
	btn = [CocoaButton createI:hbox title:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtMsgFontName];

	// Information font
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("InfoFont")];
	[CocoaLabel createI:hbox title:CMsg::Info_Font];
	txtInfoFontName = [CocoaTextField createT:hbox text:pConfig->msgboard_info_fontname.Get() action:nil width:200];

	[CocoaLabel createI:hbox title:CMsg::_Size];
	txtInfoFontSize = [CocoaTextField createN:hbox num:pConfig->msgboard_info_fontsize action:nil width:100];
	btn = [CocoaButton createI:hbox title:CMsg::File_ action:@selector(showFilePanel:)];
	[btn setRelatedObject:txtInfoFontName];

	// Language
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);

	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);

	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("Lang")];
	[CocoaLabel createI:hbox title:CMsg::Language_ASTERISK];
	popLanguage = [CocoaPopUpButton createL:hbox items:&lang_list action:nil selidx:lang_selidx];

	//
	hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("NeedRes")];
	[CocoaLabel createI:hbox title:CMsg::Need_restart_program];

	// ----------------------------------------
	// Tape, FDD tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:3];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Tape")];

#ifdef USE_DATAREC
	sbox = [box_one addBox:HorizontalBox :0 :0 :_T("TapeS")];
	lbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeL")];

	// Load wave
	bbox = [lbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("LoadWavB")];
	[CocoaBox createI:bbox :CMsg::Load_Wav_File_from_Tape :300 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("LoadWavV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("LoadWavH")];
	chkReverseWave = [CocoaCheckBox createI:hbox title:CMsg::Reverse_Wave action:nil value:pConfig->wav_reverse];
	chkHalfWave = [CocoaCheckBox createI:hbox title:CMsg::Half_Wave action:nil value:pConfig->wav_half];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrH")];
	[CocoaLabel createI:hbox title:CMsg::Correct];
	int corr_idx = pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0;
	radCorrect = [CocoaRadioGroup create:hbox width:260 cols:3 titleids:LABELS::correct action:nil selidx:corr_idx];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("CorrAmpH")];
	for(i=0; i<2; i++) {
		[CocoaLabel createT:hbox title:i == 0 ? "1200Hz" : "2400Hz"];
		txtCorrectAmp[i] = [CocoaTextField createN:hbox num:pConfig->wav_correct_amp[i] action:nil align:NSRightTextAlignment width:60];
	}

	// Save wave
	rbox = [sbox addBox:VerticalBox :0 :0 :_T("TapeR")];

	bbox = [rbox addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("SaveWavB")];
	[CocoaBox createI:bbox :CMsg::Save_Wav_File_to_Tape :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("SaveWavV")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveRatH")];
	[CocoaLabel createI:hbox title:CMsg::Sample_Rate];
	popSampleRate = [CocoaPopUpButton createT:hbox items:LABELS::wav_sampling_rate action:nil selidx:pConfig->wav_sample_rate];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SaveBitH")];
	[CocoaLabel createI:hbox title:CMsg::Sample_Bits];
	popSampleBits = [CocoaPopUpButton createT:hbox items:LABELS::wav_sampling_bits action:nil selidx:pConfig->wav_sample_bits];
#endif

	// FDD
#ifdef USE_FD1
	CocoaLayout *hbox_fdd = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("FDD")];
	bbox = [hbox_fdd addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Fdd2LB")];
	[CocoaBox createI:bbox :CMsg::Floppy_Disk_Drive :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Fdd2V")];

	// mount fdd
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MountH")];
	[CocoaLabel createI:hbox title:CMsg::When_start_up_mount_disk_at_];
	for(i=0; i<USE_FLOPPY_DISKS; i++) {
		char name[4];
		UTILITY::sprintf(name, sizeof(name), "%d ", i);
		chkFddMount[i] = [CocoaCheckBox createT:hbox title:name action:nil value:(pConfig->mount_fdd & (1 << i))];
	}

	chkDelayFd1 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_find_sector action:nil value:(FLG_DELAY_FDSEARCH != 0)];
	chkDelayFd2 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_seek_track action:nil value:(FLG_DELAY_FDSEEK != 0)];
	chkFdDensity = [CocoaCheckBox createI:vbox title:CMsg::Suppress_checking_for_density action:nil value:(FLG_CHECK_FDDENSITY == 0)];
	chkFdMedia = [CocoaCheckBox createI:vbox title:CMsg::Suppress_checking_for_media_type action:nil value:(FLG_CHECK_FDMEDIA == 0)];
	chkFdSavePlain = [CocoaCheckBox createI:vbox title:CMsg::Save_a_plain_disk_image_as_it_is action:nil value:(FLG_SAVE_FDPLAIN != 0)];

	bbox = [hbox_fdd addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Fdd2RB")];
	[CocoaBox createI:bbox :CMsg::Parameters_for_floppy_disk_in_SRAM :200 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Fdd2V")];

	// eject fd
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("ejectfd")];
	valuei = vm->get_sram_fd_eject();
	chkSramFdEject = [CocoaCheckBox createI:hbox title:CMsg::Eject_FD_when_power_off action:nil value:valuei != 0];
#endif

	// HDD
#ifdef USE_HD1
	CocoaLayout *hbox_hdd = [box_one addBox:HorizontalBox :MiddlePos :0 :_T("HDD")];
	bbox = [hbox_hdd addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Hdd2LB")];
	[CocoaBox createI:bbox :CMsg::Hard_Disk_Drive :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Hdd2LV")];

	// mount hdd
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MountH")];
	[CocoaLabel createI:hbox title:CMsg::When_start_up_mount_disk_at_];
	for(i=0; i<MAX_HARD_DISKS; i++) {
		int idx = pConfig->GetHardDiskIndex(i);
		if (idx < 0) continue;
		char name[12];
		if (i==0 || i==MAX_SASI_HARD_DISKS) {
			hbox = [vbox addBox:HorizontalBox :MiddlePos :0];
			[CocoaLabel createT:hbox title:"  "];
		}
		if (i<MAX_SASI_HARD_DISKS) {
			int sdrv = i / SASI_UNITS_PER_CTRL;
			int sunit = i % SASI_UNITS_PER_CTRL;
			UTILITY::sprintf(name, sizeof(name), "SASI%d u%d ", sdrv, sunit);
		} else {
			UTILITY::sprintf(name, sizeof(name), "SCSI%d ", i - MAX_SASI_HARD_DISKS);
		}
		chkHddMount[idx] = [CocoaCheckBox createT:hbox title:name action:nil value:(pConfig->mount_hdd & (1 << i))];
	}

	chkDelayHd2 = [CocoaCheckBox createI:vbox title:CMsg::Ignore_delays_to_seek_track action:nil value:(FLG_DELAY_HDSEEK != 0)];

	bbox = [hbox_hdd addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Hdd2RB")];
	[CocoaBox createI:bbox :CMsg::Parameters_for_hard_disk_in_SRAM :200 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Hdd2RV")];

	// number of SASI HDDs
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("numhdd")];
	[CocoaLabel createI:hbox title:CMsg::Number_of_SASI_HDDs];
	valuei = vm->get_sram_sasi_hdd_nums();
	stpSramNumHdds = [CocoaStepper createN:hbox min:0 max:15 value:valuei width:60];

	// SCSI enable flag
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("scsien")];
	chkSramScsiEn = [CocoaCheckBox createI:hbox title:CMsg::SCSI_enable_flag action:nil value:vm->get_sram_scsi_enable_flag()];

	// SCSI host ID
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("scsiid")];
	[CocoaLabel createI:hbox title:CMsg::SCSI_host_ID];
	valuei = vm->get_sram_scsi_host_id();
	txtSramScsiId = [CocoaTextField createN:hbox num:valuei action:nil width:60];

	// SASI HDDs on SCSI
	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("sasiscsi")];
	[CocoaLabel createI:hbox title:CMsg::SASI_HDDs_on_SCSI];
	valueu = vm->get_sram_sasi_hdd_on_scsi();
	UTILITY::sprintf(str, sizeof(str), "%x", valueu);
	txtSramSasiOnScsi = [CocoaTextField createT:hbox text:str action:nil width:60];


	// scsi type
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("Hdd3B")];
	[CocoaBox createI:bbox :CMsg::SCSI_Type_ASTERISK :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("Hdd3V")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SCSITY")];
	valuei = emu->get_parami(VM::ParamSCSIType);
#ifndef USE_SCSI_TYPE_IN
	if (valuei > SCSI_TYPE_EX) valuei = SCSI_TYPE_EX;
#endif
	for(i=0; i<SCSI_TYPE_END; i++) {
		radSCSIType[i] = [CocoaRadioButton createI:hbox title:LABELS::scsi_type[i] index:i action:@selector(selectSCSIType:) value:i == valuei];
	}
//	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("SCSITYN")];
	UTILITY::strcpy(str, sizeof(str), CMSG(LB_Now_SP));
	UTILITY::strcat(str, sizeof(str), CMSGV(LABELS::scsi_type[pConfig->scsi_type]));
	UTILITY::strcat(str, sizeof(str), _T(")"));
	[CocoaLabel createT:hbox title:str];

	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];
#endif

	// ----------------------------------------
	// Sound tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:4];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Sound")];

	// MIDI
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("MIDI")];
	[CocoaBox createI:bbox :CMsg::MIDI :320 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("MIDI1")];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIE")];
	bool valueb = ((vm->get_parami(VM::ParamIOPort) & IOPORT_MIDI) != 0);
	chkEnableMIDI = [CocoaCheckBox createI:hbox title:CMsg::Enable_MIDI_Board_CZ_6BM1_ASTERISK action:nil value:valueb];
	UTILITY::strcpy(str, sizeof(str), CMSG(LB_Now_SP));
	UTILITY::strcat(str, sizeof(str), valueb ? CMSG(Enable) : CMSG(Disable));
	UTILITY::strcat(str, sizeof(str), _T(")"));
	[CocoaLabel createT:hbox title:str];

#ifdef USE_MIDI
	CPtrList<CTchar> midiout_list;
	int midiout_cnt = gui->EnumMidiOuts();
	int midiout_conn = 0;
	midiout_list.Add(new CTchar(CMSG(None_)));
	for(int idx = 0; idx < midiout_cnt && idx < MIDI_MAX_PORTS; idx++) {
		gui->GetMidiOutDescription(idx, str, sizeof(str)/sizeof(str[0]));
		if (gui->NowConnectingMidiOut(idx)) {
			midiout_conn = idx + 1;
		}
		midiout_list.Add(new CTchar(str));
	}
	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIO")];
	[CocoaLabel createI:hbox title:CMsg::Now_Connecting_MIDI_Output];
	popMIDIOut = [CocoaPopUpButton createL:hbox items:&midiout_list action:nil selidx:midiout_conn];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIOD")];
	[CocoaLabel createI:hbox title:CMsg::Output_Latency];
	valuei = pConfig->midiout_delay;
	stpMIDIOutDelay = [CocoaStepper createN:hbox min:0 max:2000 value:valuei width:60];
	[CocoaLabel createI:hbox title:CMsg::msec];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIR")];
	[CocoaLabel createI:hbox title:CMsg::MIDI_Reset_Type];
	popMIDIResetType = [CocoaPopUpButton createI:hbox items:LABELS::midi_type action:nil selidx:pConfig->midi_send_reset_type];
	btn = [CocoaButton createI:hbox title:CMsg::Send_message_now action:@selector(sendMIDIResetMessage:)];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIRC")];
	[CocoaLabel createI:hbox title:CMsg::When_send_reset_message_];
	chkMIDIResPowerOn = [CocoaCheckBox createI:hbox title:CMsg::Power_on action:nil value:((pConfig->midi_flags & MIDI_FLAG_RES_POWERON) != 0)];
	chkMIDIResPowerOff = [CocoaCheckBox createI:hbox title:CMsg::Power_off action:nil value:((pConfig->midi_flags & MIDI_FLAG_RES_POWEROFF) != 0)];
	chkMIDIResHardRes = [CocoaCheckBox createI:hbox title:CMsg::Reset_by_hand action:nil value:((pConfig->midi_flags & MIDI_FLAG_RES_HARDRES) != 0)];
	chkMIDIResEndApp = [CocoaCheckBox createI:hbox title:CMsg::End_of_app_ action:nil value:((pConfig->midi_flags & MIDI_FLAG_RES_END_APP) != 0)];

	hbox = [vbox addBox:HorizontalBox :MiddlePos :0 :_T("MIDIRT")];
	chkMIDINoRTMsg = [CocoaCheckBox createI:hbox title:CMsg::Does_not_send_system_real_time_messages_ action:nil value:((pConfig->midi_flags & MIDI_FLAG_NO_REALTIME_MSG) != 0)];
#endif

	[CocoaLabel createI:box_one title:CMsg::Need_restart_program_or_PowerOn];

	// ----------------------------------------
	// Network tab
	// ----------------------------------------
	tab = [tabView tabViewItemAtIndex:5];
	tab_view = (CocoaView *)[tab view];
	[box_tab setContentView:tab_view];

	box_one = [box_tab addBox:VerticalBox :0 :0 :_T("Net")];

#ifdef MAX_PRINTER
	for(i=0; i<MAX_PRINTER; i++) {
		char name[128];
		UTILITY::sprintf(name, sizeof(name), CMSG(Printer_Hostname), i);

		UTILITY::stprintf(bname, sizeof(bname)/sizeof(bname[0]), _T("LPT%d"), i);
		hbox = [box_one addBox:HorizontalBox :MiddlePos :0 :bname];
		[CocoaLabel createT:hbox title:name width:120];
		txtLPTHost[i] = [CocoaTextField createT:hbox text:pConfig->printer_server_host[i].Get() action:nil width:140];

		[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
		txtLPTPort[i] = [CocoaTextField createN:hbox num:pConfig->printer_server_port[i] action:nil align:NSTextAlignmentRight width:80];

		[CocoaLabel createI:hbox title:CMsg::_Print_delay align:NSTextAlignmentRight];
		char delay[128];
		UTILITY::sprintf(delay, sizeof(delay), "%.1f", pConfig->printer_delay[i]);
		txtLPTDelay[i] = [CocoaTextField createT:hbox text:delay action:nil align:NSTextAlignmentRight width:80];

		[CocoaLabel createI:hbox title:CMsg::msec align:NSTextAlignmentRight];
	}
#endif
#ifdef MAX_COMM
	for(i=0; i<MAX_COMM; i++) {
		char name[128];
		UTILITY::sprintf(name, sizeof(name), CMSG(Communication_Hostname), i);

		UTILITY::stprintf(bname, sizeof(bname)/sizeof(bname[0]), _T("COM%d"), i);
		hbox = [box_one addBox:HorizontalBox :0 :0 :bname];
		[CocoaLabel createT:hbox title:name width:120];
		txtCOMHost[i] = [CocoaTextField createT:hbox text:pConfig->comm_server_host[i].Get() action:nil width:140];

		[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
		txtCOMPort[i] = [CocoaTextField createN:hbox num:pConfig->comm_server_port[i] action:nil align:NSTextAlignmentRight width:80];

//		popCOMDipswitch[i] = [CocoaPopUpButton createI:hbox items:LABELS::comm_baud action:nil selidx:(pConfig->comm_dipswitch[i]-1)];
	}
#endif
#ifdef USE_DEBUGGER
	hbox = [box_one addBox:HorizontalBox :0 :0 :_T("Debugger")];

	[CocoaLabel createI:hbox title:CMsg::Connectable_host_to_Debugger];
	txtDbgrHost = [CocoaTextField createT:hbox text:pConfig->debugger_server_host.Get() action:nil width:140];

	[CocoaLabel createI:hbox title:CMsg::_Port align:NSTextAlignmentRight];
	txtDbgrPort = [CocoaTextField createN:hbox num:pConfig->debugger_server_port action:nil align:NSTextAlignmentRight width:80];
#endif
	// uart
	int uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			uart_baud_index = i;
			break;
		}
	}
	bbox = [box_one addBox:BoxViewBox :0 :COCOA_DEFAULT_MARGIN :_T("UART")];
	[CocoaBox createI:bbox :CMsg::Settings_of_serial_ports_on_host :260 :1];

	vbox = [bbox addBox:VerticalBox :0 :0 :_T("UARTV")];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("BaudRate")];
	[CocoaLabel createI:hbox title:CMsg::Baud_Rate width:120];
	popCOMUartBaud = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_baudrate action:nil selidx:uart_baud_index];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("DataBit")];
	[CocoaLabel createI:hbox title:CMsg::Data_Bit width:120];
	popCOMUartDataBit = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_databit action:nil selidx:(pConfig->comm_uart_databit - 7)];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("Parity")];
	[CocoaLabel createI:hbox title:CMsg::Parity width:120];
	popCOMUartParity = [CocoaPopUpButton createI:hbox items:LABELS::comm_uart_parity action:nil selidx:pConfig->comm_uart_parity];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("StopBit")];
	[CocoaLabel createI:hbox title:CMsg::Stop_Bit width:120];
	popCOMUartStopBit = [CocoaPopUpButton createT:hbox items:LABELS::comm_uart_stopbit action:nil selidx:(pConfig->comm_uart_stopbit - 1)];

	hbox = [vbox addBox:HorizontalBox :0 :0 :_T("FlowCtrl")];
	[CocoaLabel createI:hbox title:CMsg::Flow_Control width:120];
	popCOMUartFlowCtrl = [CocoaPopUpButton createI:hbox items:LABELS::comm_uart_flowctrl action:nil selidx:pConfig->comm_uart_flowctrl];

	[CocoaLabel createI:vbox title:CMsg::Need_re_connect_to_serial_port_when_modified_this];


	// button

	hbox = [box_all addBox:HorizontalBox :RightPos | TopPos :0 :_T("BTN")];
	[CocoaButton createI:hbox title:CMsg::Cancel action:@selector(dialogCancel:) width:120];
	[CocoaButton createI:hbox title:CMsg::OK action:@selector(dialogOk:) width:120];


	[box_all realize:self];

//	[self selectFddType:radFddType];

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
	int i;
	int valuel;
//	int val;

	// set data
	pConfig->use_power_off = ([chkPowerOff state] == NSControlStateValueOn);

	// power state when start up
	pConfig->power_state_when_start_up = (int)[popPowerState indexOfSelectedItem];

	pConfig->rom_path.Set([[txtROMPath stringValue] UTF8String]);

	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, [chkAddrErr state] == NSControlStateValueOn);

	emu->set_parami(VM::ParamMainRamSizeNum, (int)[popMainRam indexOfSelectedItem]);

#ifdef USE_FD1
	for(i=0;i<USE_FLOPPY_DISKS;i++) {
		pConfig->mount_fdd = (([chkFddMount[i] state] == NSControlStateValueOn) ? pConfig->mount_fdd | (1 << i) : pConfig->mount_fdd & ~(1 << i));
	}
	pConfig->option_fdd = ([chkDelayFd1 state] == NSControlStateValueOn ? MSK_DELAY_FDSEARCH : 0)
		| ([chkDelayFd2 state] == NSControlStateValueOn ? MSK_DELAY_FDSEEK : 0)
		| ([chkFdDensity state] == NSControlStateValueOn ? 0 : MSK_CHECK_FDDENSITY)
		| ([chkFdMedia state] == NSControlStateValueOn ? 0 : MSK_CHECK_FDMEDIA)
		| ([chkFdSavePlain state] == NSControlStateValueOn ? MSK_SAVE_FDPLAIN : 0);
#endif

#ifdef USE_HD1
	for(i=0;i<MAX_HARD_DISKS;i++) {
		int idx = pConfig->GetHardDiskIndex(i);
		if (idx < 0) continue;
		BIT_ONOFF(pConfig->mount_hdd, (1 << i), [chkHddMount[idx] state] == NSControlStateValueOn);
	}
	pConfig->option_hdd = ([chkDelayHd2 state] == NSControlStateValueOn ? MSK_DELAY_HDSEEK : 0);
	
	valuel = 0;
	for(i=0;i<SCSI_TYPE_END;i++) {
		if ([radSCSIType[i] state] == NSControlStateValueOn) {
			valuel = i;
			break;
		}
	}
	emu->set_parami(VM::ParamSCSIType, valuel);
#endif

#if 0
	for(i=IOPORT_STARTNUM;i<IOPORT_NUMS;i++) {
		if ((1 << i) & IOPORT_MSK_ALL) {
			val = emu->get_parami(VM::ParamIOPort);
			if ([chkIOPort[i] state] == NSControlStateValueOn) {
				val |= (1 << i);
			} else {
				val &= ~(1 << i);
			}
			emu->set_parami(VM::ParamIOPort, val);
		}
	}
#endif

	pConfig->use_opengl = (int)[popUseOpenGL indexOfSelectedItem];
	pConfig->gl_filter_type = [popGLFilter indexOfSelectedItem];

	pConfig->led_pos = [popLEDPosition indexOfSelectedItem];
	pConfig->capture_type = [popCaptureType indexOfSelectedItem];

	// crtc
#if defined(_X68000)
	valuel = [txtRasterSkewV intValue];
	if (-128 <= valuel && valuel <= 128) {
		pConfig->raster_int_vskew = valuel;
	}
	valuel = [txtRasterSkewH intValue];
	if (-512 <= valuel && valuel <= 512) {
		pConfig->raster_int_hskew = valuel;
	}
	valuel = [txtVertSkew intValue];
	if (-128 <= valuel && valuel <= 128) {
		pConfig->vdisp_skew = valuel;
	}
#endif

	pConfig->snapshot_path.Set([[txtSnapPath stringValue] UTF8String]);
	pConfig->font_path.Set([[txtFontPath stringValue] UTF8String]);

	pConfig->msgboard_msg_fontname.Set([[txtMsgFontName stringValue] UTF8String]);
	pConfig->msgboard_info_fontname.Set([[txtInfoFontName stringValue] UTF8String]);
	valuel = [txtMsgFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		pConfig->msgboard_msg_fontsize = (int)valuel;
	}
	valuel = [txtInfoFontSize intValue];
	if (1 <= valuel && valuel <= 60) {
		pConfig->msgboard_info_fontsize = (int)valuel;
	}

#ifdef USE_SCREEN_MIX_SURFACE
	// double buffering
	pConfig->double_buffering = ([chkDoubleBuffer state] == NSControlStateValueOn);
#endif

	// border color
	BIT_ONOFF(pConfig->original, MSK_ORIG_BORDER_COLOR, [chkBorderColor state] == NSControlStateValueOn);

	// language
	valuel = (int)[popLanguage indexOfSelectedItem];
	clocale->ChooseLocaleName(lang_list, valuel, pConfig->language);

#ifdef USE_DATAREC
	pConfig->wav_reverse = ([chkReverseWave state] == NSControlStateValueOn);
	pConfig->wav_half = ([chkHalfWave state] == NSControlStateValueOn);
//	pConfig->wav_correct = ([chkCorrectWave state] == NSControlStateValueOn);
	pConfig->wav_correct = ([radCorrect selectedColumn] == 0);
	pConfig->wav_correct_type = [radCorrect selectedColumn];
	pConfig->wav_correct_type = (pConfig->wav_correct_type > 0 ? pConfig->wav_correct_type - 1 : 0);
	valuel = [txtCorrectAmp[0] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		pConfig->wav_correct_amp[0] = (int)valuel;
	}
	valuel = [txtCorrectAmp[1] intValue];
	if (valuel >= 100 && valuel <= 5000) {
		pConfig->wav_correct_amp[1] = (int)valuel;
	}

	pConfig->wav_sample_rate = [popSampleRate indexOfSelectedItem];
	pConfig->wav_sample_bits = [popSampleBits indexOfSelectedItem];
#endif

#ifdef MAX_PRINTER
	for(i=0;i<MAX_PRINTER;i++) {
		pConfig->printer_server_host[i].Set([[txtLPTHost[i] stringValue] UTF8String]);
		valuel = [txtLPTPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			pConfig->printer_server_port[i] = (int)valuel;
		}
		double valued = 0.0;
		valued = strtod([[txtLPTDelay[i] stringValue] UTF8String], NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
#endif
#ifdef MAX_COMM
	for(i=0;i<MAX_COMM;i++) {
		pConfig->comm_server_host[i].Set([[txtCOMHost[i] stringValue] UTF8String]);
		valuel = [txtCOMPort[i] intValue];
		if (0 <= valuel && valuel <= 65535) {
			pConfig->comm_server_port[i] = (int)valuel;
		}
//		pConfig->comm_dipswitch[i] = (int)[popCOMDipswitch[i] indexOfSelectedItem] + 1;
	}
#endif
#ifdef USE_DEBUGGER
	pConfig->debugger_server_host.Set([[txtDbgrHost stringValue] UTF8String]);
	valuel = [txtDbgrPort intValue];
	if (0 <= valuel && valuel <= 65535) {
		pConfig->debugger_server_port = (int)valuel;
	}
#endif
	int uart_baud_index = (int)[popCOMUartBaud indexOfSelectedItem];
	pConfig->comm_uart_baudrate = (int)strtol(LABELS::comm_uart_baudrate[uart_baud_index], NULL, 10);
	pConfig->comm_uart_databit = (int)[popCOMUartDataBit indexOfSelectedItem] + 7;
	pConfig->comm_uart_parity = (int)[popCOMUartParity indexOfSelectedItem];
	pConfig->comm_uart_stopbit = (int)[popCOMUartStopBit indexOfSelectedItem] + 1;
	pConfig->comm_uart_flowctrl = (int)[popCOMUartFlowCtrl indexOfSelectedItem];

#if defined(_X68000)
	// SRAM
	VM *vm = emu->get_vm();
	uint32_t valueu = 0;
	char *endptr = NULL;

	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CLR_PWRON, [chkSRAMClear state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_SAVE_PWROFF, [chkSRAMSave state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CHG_BOOT_DEV, [chkSRAMChanged state] == NSControlStateValueOn);
	
	// ROM start address
	valueu = (uint32_t)strtol([[txtSramRomStartAddr stringValue] UTF8String], &endptr, 16);
	if (endptr && *endptr == 0) {
		vm->set_sram_rom_start_address(valueu);
	}

	// SRAM start address
	valueu = (uint32_t)strtol([[txtSramRamStartAddr stringValue] UTF8String], &endptr, 16);
	if (endptr && *endptr == 0) {
		vm->set_sram_sram_start_address(valueu);
	}

	// boot device
	vm->set_sram_boot_device((int)[popSramBootDevice indexOfSelectedItem]);

	// RS-232C
	valueu = 0;
	valueu |= vm->conv_sram_rs232c_baud_rate((int)[popSramRS232CBaud indexOfSelectedItem]);
	valueu |= vm->conv_sram_rs232c_databit((int)[popSramRS232CDataBit indexOfSelectedItem]);
	valueu |= vm->conv_sram_rs232c_parity((int)[popSramRS232CParity indexOfSelectedItem]);
	valueu |= vm->conv_sram_rs232c_stopbit((int)[popSramRS232CStopBit indexOfSelectedItem]);
	valueu |= vm->conv_sram_rs232c_flowctrl((int)[popSramRS232CFlowCtrl indexOfSelectedItem]);
	vm->set_sram_rs232c(valueu);

	// alarm
	vm->set_sram_alarm_onoff([chkSramAlarm state] == NSControlStateValueOn);

	// contrast
	valuel = [stpSramContrast intValue];
	if (valuel >= 0 && valuel <= 15) {
		vm->set_sram_contrast(valuel);
	}

	// eject fd
	vm->set_sram_fd_eject([chkSramFdEject state] == NSControlStateValueOn ? 1 : 0);

	// purpose of SRAM free area
	vm->set_sram_purpose((int)[popSramPurpose indexOfSelectedItem]);

	// key repeat delay
	vm->set_sram_key_repeat_delay((int)[popSramKRDelay indexOfSelectedItem]);

	// key repeat rate
	vm->set_sram_key_repeat_rate((int)[popSramKRRate indexOfSelectedItem]);

	// key LED
	valuel = 0;
	valuel |= [chkSramKLEDkana state] == NSControlStateValueOn ? 1 : 0;
	valuel |= [chkSramKLEDromaji state] == NSControlStateValueOn ? 2 : 0;
	valuel |= [chkSramKLEDcinput state] == NSControlStateValueOn ? 4 : 0;
	valuel |= [chkSramKLEDcaps state] == NSControlStateValueOn ? 8 : 0;
	valuel |= [chkSramKLEDins state] == NSControlStateValueOn ? 16 : 0;
	valuel |= [chkSramKLEDhira state] == NSControlStateValueOn ? 32 : 0;
	valuel |= [chkSramKLEDzen state] == NSControlStateValueOn ? 64 : 0;
	vm->set_sram_key_led(valuel);

#ifdef USE_HD1
	// number of SASI HDDs
	valuel = [stpSramNumHdds intValue];
	if (valuel >= 0 && valuel <= 15) {
		vm->set_sram_sasi_hdd_nums(valuel);
	}
	// SCSI enable flag
	vm->set_sram_scsi_enable_flag([chkSramScsiEn state] == NSControlStateValueOn);

	// SCSI host ID
	valuel = [txtSramScsiId intValue];
	if (valuel >= 0 && valuel <= 7) {
		vm->set_sram_scsi_host_id(valuel);
	}

	// SASI HDDs on SCSI
	valueu = (uint32_t)strtol([[txtSramSasiOnScsi stringValue] UTF8String], &endptr, 16);
	if (endptr && *endptr == 0) {
		vm->set_sram_sasi_hdd_on_scsi(valueu & 0xff);
	}
#endif
#endif

	int io_port = vm->get_parami(VM::ParamIOPort);
	BIT_ONOFF(io_port, IOPORT_MIDI, [chkEnableMIDI state] == NSControlStateValueOn);
	vm->set_parami(VM::ParamIOPort, io_port);
#ifdef USE_MIDI
	int midiout_conn = (int)[popMIDIOut indexOfSelectedItem];
	gui->ConnectMidiOut(midiout_conn - 1);
	valuel = [stpMIDIOutDelay intValue];
	if (0 <= valuel && valuel <= 2000) {
		pConfig->midiout_delay = valuel;
		emu->set_midiout_delay_time(valuel);
	}
	pConfig->midi_send_reset_type = (int)[popMIDIResetType indexOfSelectedItem];
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_POWERON, [chkMIDIResPowerOn state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_POWEROFF, [chkMIDIResPowerOff state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_HARDRES, [chkMIDIResHardRes state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_END_APP, [chkMIDIResEndApp state] == NSControlStateValueOn);
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_NO_REALTIME_MSG, [chkMIDINoRTMsg state] == NSControlStateValueOn);
#endif

	// set message font
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}

	gui->ChangeLedBox((int)[popLEDShow indexOfSelectedItem]);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
	pConfig->save();
	emu->change_opengl_attr();

	emu->update_config();

#if defined(_X68000)
	vm->save_sram_file();
#endif

	// OK button is pushed
	[NSApp stopModalWithCode:NSModalResponseOK];
	[super close];
}

- (void)dialogCancel:(id)sender
{
    // Cancel button is pushed
	[self close];
}

- (void)showFolderPanel:(id)sender
{
	NSInteger result;

	CocoaTextField *text = [sender relatedObject];
	if (text == nil) return;

	NSOpenPanel *panel = [NSOpenPanel openPanel];

	// cannot select file
	[panel setCanChooseFiles:NO];
	[panel setCanChooseDirectories:YES];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[text stringValue]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		[text setStringValue:[filePath path]];
	}
}

- (void)showFilePanel:(id)sender
{
	NSInteger result;

	CocoaTextField *text = [sender relatedObject];
	if (text == nil) return;

	NSOpenPanel *panel = [NSOpenPanel openPanel];

	// can select file
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:NO];
	// set font folder
	NSURL *url = nil;
	BOOL exist = false;
	url = [NSURL fileURLWithPath:@"/System/Library/Fonts/"];
	exist = [url checkResourceIsReachableAndReturnError:nil];
	if (!exist) {
		url = [NSURL fileURLWithPath:@"/Library/Fonts/"];
	}
	[panel setDirectoryURL:url];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];
		NSArray *arr = [filePath pathComponents];
		[text setStringValue:[arr lastObject]];
	}
}

- (void)showFontPanel:(id)sender
{
//	NSInteger result;

	CocoaObjectStructure *obj = [sender relatedObject];
	if (obj == nil) return;
	CocoaTextField *fname = [obj obj1];
	if (fname == nil) return;
	CocoaTextField *fsize = [obj obj2];
	if (fsize == nil) return;

	// set font
	NSFont *font = [NSFont fontWithName:[fname stringValue] size:[fsize integerValue]];

	CocoaFontPanel *panel = [[CocoaFontPanel alloc] init];

	[panel setPanelFont:font isMultiple:NO];

	// Display modal dialog
	[panel runModal];

//	if(result == NSModalResponseOK) {
//	}
//	[[NSFontManager sharedFontManager]orderFrontFontPanel:self];
}

- (void)changeFont:(id)sender
{

}

#if defined(_MBS1)
- (void)selectSysMode:(CocoaRadioButton *)sender
{
	int idx = [sender index];
	for(int i=0; i<2; i++) {
		[radSysMode[i] setState:(i == idx ? NSControlStateValueOn : NSControlStateValueOff)];
	}
}
- (void)selectFmOpn:(CocoaCheckBox *)sender
{
	CocoaCheckBox *receiver;
	if (sender == chkFmOpnEn) {
		receiver = chkIOPort[IOPORT_POS_FMOPN];
	} else {
		receiver = chkFmOpnEn;
	}
	[receiver setState:[sender state]];
}
- (void)selectExPsg:(CocoaCheckBox *)sender
{
	CocoaCheckBox *receiver;
	if (sender == chkExPsgEn) {
		receiver = chkIOPort[IOPORT_POS_EXPSG];
	} else {
		receiver = chkExPsgEn;
	}
	[receiver setState:[sender state]];
}
#endif

- (void)selectFddType:(CocoaRadioButton *)sender
{
}

- (void)selectIO:(CocoaCheckBox *)sender
{
}

- (void)selectCorrect:(CocoaCheckBox *)sender
{
//	if ([sender state] == NSControlStateValueOn) {
//		[radCorrect setEnabled:YES];
//	} else {
//		[radCorrect setEnabled:NO];
//	}
}

#ifdef USE_HD1
- (void)selectSCSIType:(CocoaRadioButton *)sender
{
	int idx = [sender index];
	for(int i=0; i<SCSI_TYPE_END; i++) {
		[radSCSIType[i] setState:(i == idx ? NSControlStateValueOn : NSControlStateValueOff)];
	}
}
#endif

#ifdef USE_MIDI
- (void)sendMIDIResetMessage:(id)sender
{
	int midiout_conn = (int)[popMIDIOut indexOfSelectedItem];
	gui->ConnectMidiOut(midiout_conn - 1);
	gui->SendMidiResetMessage((int)[popMIDIResetType indexOfSelectedItem]);
}
#endif
@end
