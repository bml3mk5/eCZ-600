/** @file gtk_configbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.09.22 -

	@brief [ config box ]
*/

#include "gtk_configbox.h"
#include "gtk_folderbox.h"
#include "gtk_filebox.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../clocale.h"
#include "../../msgboard.h"
#include "../../utility.h"
#include "../../msgs.h"
#include "gtk_x11_gui.h"
#include "../../labels.h"
#include <math.h>

extern EMU *emu;

namespace GUI_GTK_X11
{

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

ConfigBox::ConfigBox(GUI *new_gui) : DialogBox(new_gui)
{
}

ConfigBox::~ConfigBox()
{
}

bool ConfigBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Configure);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *nb;
	GtkWidget *vboxall;
	GtkWidget *hboxall;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *lbox;
	GtkWidget *mbox;
	GtkWidget *rbox;
	GtkWidget *ctrl;
//	GtkWidget *lbl;
//	GtkWidget *btn;

	char buf[128];

	// create notebook tab
	nb = create_notebook(cont);

	// ----------------------------------------
	// 0 CPU, Memory
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[0]);

	// rom path
	hbox = create_hbox(vboxall);
	txtROMPath = create_text_with_label(hbox, CMsg::ROM_Path_ASTERISK, pConfig->rom_path.GetN(), 40);
	create_button(hbox,CMsg::Folder_, G_CALLBACK(OnSelectROMPath));

#if defined(_X68000)
	// main ram size
	create_frame(vboxall, CMsg::RAM, &vbox, &hbox);
	comRamSize = create_combo_box(hbox, CMsg::RAM_Size_ASTERISK, LABELS::main_ram_size, emu->get_parami(VM::ParamMainRamSizeNum));
	if (0 <= pConfig->main_ram_size_num && pConfig->main_ram_size_num <= 5) {
		UTILITY::tcscpy(buf, sizeof(buf), CMSG(LB_Now_SP));
		UTILITY::tcscat(buf, sizeof(buf), CMSGV(LABELS::main_ram_size[pConfig->main_ram_size_num]));
		UTILITY::tcscat(buf, sizeof(buf), _T(")"));
	} else {
		buf[0] = _T('\0');
	}
	create_label(hbox, buf);

	// overwrite size on SRAM
	chkOwSizeSRAM = create_check_box(vbox, CMsg::No_overwrite_RAM_size_on_SRAM, FLG_ORIG_SRAM_RAM_SIZE != 0);

	// CPU
	create_frame(vboxall, CMsg::CPU, &vbox, &hbox);
	chkAddrErr = create_check_box(hbox, CMsg::Show_message_when_the_address_error_occured_in_the_CPU, FLG_SHOWMSG_ADDRERR != 0);
#endif

	// power status
	create_frame(vboxall, CMsg::Behavior_of_Power_On_Off, &vbox, &hbox);
	// power off
	chkPowerOff = create_check_box(hbox, CMsg::Enable_the_state_of_power_off, pConfig->use_power_off);

	// power state when start up
	hbox = create_hbox(vbox);
	comPowerState = create_combo_box(hbox, CMsg::Power_State_When_Start_Up, LABELS::power_state, pConfig->power_state_when_start_up);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program_or_PowerOn);

	// ----------------------------------------
	// 1 SRAM
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[1]);

#if defined(_X68000)
	// behavior
	create_frame(vboxall, CMsg::Behavior, &vbox, &hbox);
	chkSRAMClear = create_check_box(hbox, CMsg::Clear_SRAM_when_power_on, FLG_ORIG_SRAM_CLR_PWRON != 0);
	create_label(hbox, _T(" "));
	chkSRAMSave  = create_check_box(hbox, CMsg::Save_SRAM_when_power_off, FLG_ORIG_SRAM_SAVE_PWROFF != 0);
	hbox = create_hbox(vbox);
	chkSRAMChanged = create_check_box(hbox, CMsg::Show_message_when_parameters_related_on_start_up_was_changed, FLG_ORIG_SRAM_CHG_BOOT_DEV != 0);

	// SRAM
	hboxall = create_hbox(vboxall);
	lbox = create_vbox(hboxall);
	mbox = create_vbox(hboxall);
	rbox = create_vbox(hboxall);

	create_label(mbox, _T(" "));

	VM *vm = emu->get_vm();
	int valuei = 0;
	uint32_t valueu = 0;

	// R
	create_label(lbox, CMsg::Parameters_in_SRAM_);

	// main ram size
	hbox = create_hbox(lbox);
	valueu = vm->get_sram_ram_size();
	UTILITY::sprintf(buf, sizeof(buf), "%08x", valueu);
	ctrl = create_text_with_label(hbox, CMsg::RAM_Size_Hex, buf, 10);
	set_enable(ctrl, false);

	// ROM start address
	hbox = create_hbox(lbox);
	valueu = vm->get_sram_rom_start_address();
	UTILITY::sprintf(buf, sizeof(buf), "%08x", valueu);
	txtSramRomAddr = create_text_with_label(hbox, CMsg::ROM_Start_Address_Hex, buf, 10);

	// SRAM start address
	hbox = create_hbox(lbox);
	valueu = vm->get_sram_sram_start_address();
	UTILITY::sprintf(buf, sizeof(buf), "%08x", valueu);
	txtSramSramAddr = create_text_with_label(hbox, CMsg::SRAM_Start_Address_Hex, buf, 10);

	// boot device
	hbox = create_hbox(lbox);
	valuei = vm->get_sram_boot_device();
	comSramBootDevice = create_combo_box(hbox, CMsg::Boot_Device, LABELS::boot_devices, valuei);

	// RS-232C
	valueu = vm->get_sram_rs232c();
	create_frame(lbox, CMsg::RS_232C, &vbox, &hbox);
	valuei = vm->get_sram_rs232c_baud_rate(valueu);
	create_label(hbox, CMsg::Baud_Rate);
	comSramRS232CBaud = create_combo_box(hbox, " ", LABELS::rs232c_baudrate, valuei);
	hbox = create_hbox(vbox);
	valuei = vm->get_sram_rs232c_databit(valueu);
	create_label(hbox, CMsg::Data_Bit);
	comSramRS232CDataBit = create_combo_box(hbox, " ", LABELS::rs232c_databit, valuei);
	hbox = create_hbox(vbox);
	valuei = vm->get_sram_rs232c_parity(valueu);
	create_label(hbox, CMsg::Parity);
	comSramRS232CParity = create_combo_box(hbox, " ", LABELS::comm_uart_parity, valuei);
	hbox = create_hbox(vbox);
	valuei = vm->get_sram_rs232c_stopbit(valueu);
	create_label(hbox, CMsg::Stop_Bit);
	comSramRS232CStopBit = create_combo_box(hbox, " ", LABELS::rs232c_stopbit, valuei);
	hbox = create_hbox(vbox);
	valuei = vm->get_sram_rs232c_flowctrl(valueu);
	create_label(hbox, CMsg::Flow_Control);
	comSramRS232CFlowCtrl = create_combo_box(hbox, " ", LABELS::rs232c_flowctrl, valuei);

	// L

	// alarm
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_alarm_onoff();
	chkSramAlarm = create_check_box(hbox, CMsg::Enable_alarm, valuei == 0);
//	set_enable(ctrl, false);

	// alarm time
	hbox = create_hbox(rbox);
	valueu = vm->get_sram_alarm_time();
	UTILITY::sprintf(buf, sizeof(buf), "%08x", valueu);
	ctrl = create_text_with_label(hbox, CMsg::Alarm_Time_Hex, buf, 10);
	set_enable(ctrl, false);

	// alarm duration
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_alarm_duration();
	UTILITY::sprintf(buf, sizeof(buf), "%d", valuei);
	ctrl = create_text_with_label(hbox, CMsg::Alarm_Duration, buf, 10);
	set_enable(ctrl, false);

	// contrast
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_contrast();
	create_label(hbox, CMsg::Contrast_on_Monitor);
	spnSramContrast = create_spin(hbox, 0, 15, valuei);

	// purpose of SRAM free area
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_purpose();
	comSramPurpose = create_combo_box(hbox, CMsg::Purpose_of_SRAM_free_area, LABELS::sram_purpose, valuei);

	// key repeat delay
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_key_repeat_delay();
	comSramKRDelay = create_combo_box(hbox, CMsg::Key_Repeat_Delay, LABELS::key_repeat_delay, valuei);
	create_label(hbox, CMsg::msec);

	// key repeat rate
	hbox = create_hbox(rbox);
	valuei = vm->get_sram_key_repeat_rate();
	comSramKRRate = create_combo_box(hbox, CMsg::Key_Repeat_Rate, LABELS::key_repeat_rate, valuei);
	create_label(hbox, CMsg::msec);

	// keyLED
	create_frame(rbox, CMsg::Status_of_Key_LED_when_power_on, &vbox, &hbox);
	valuei = vm->get_sram_key_led();
	chkSramKLEDkana = create_check_box(hbox, CMsg::Kana, (valuei & 1) != 0);
	chkSramKLEDromaji = create_check_box(hbox, CMsg::Roma_ji, (valuei & 2) != 0);
	chkSramKLEDcinput = create_check_box(hbox, CMsg::Code_Input, (valuei & 4) != 0);
	hbox = create_hbox(vbox);
	chkSramKLEDcaps = create_check_box(hbox, CMsg::CAPS, (valuei & 8) != 0);
	chkSramKLEDins = create_check_box(hbox, CMsg::INS, (valuei & 16) != 0);
	chkSramKLEDhira = create_check_box(hbox, CMsg::Hiragana, (valuei & 32) != 0);
	chkSramKLEDzen = create_check_box(hbox, CMsg::Zenkaku, (valuei & 64) != 0);

	// Accumulated operating time
	hbox = create_hbox(rbox);
	valueu = vm->get_sram_accumulated_operating_time();
	UTILITY::sprintf(buf, sizeof(buf), "%u", valueu);
	ctrl = create_text_with_label(hbox, CMsg::Accumulated_operating_time, buf, 8);
	create_label(hbox, CMsg::min_);
	set_enable(ctrl, false);

	// Number of times turned off
	hbox = create_hbox(rbox);
	valueu = vm->get_sram_times_of_the_power_off();
	UTILITY::sprintf(buf, sizeof(buf), "%u", valueu);
	ctrl = create_text_with_label(hbox, CMsg::Times_of_turned_off, buf, 8);
	set_enable(ctrl, false);
#endif

	// ----------------------------------------
	// 2 Screen
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[2]);

	hboxall = create_hbox(vboxall);

	int use_opengl = 0;
	int gl_filter_type = 0;
#ifdef USE_OPENGL
	use_opengl = pConfig->use_opengl;
	gl_filter_type = pConfig->gl_filter_type;
#endif
	create_frame(hboxall, CMsg::Drawing, &vbox, &hbox);
	comUseOpenGL = create_combo_box(hbox,CMsg::Method_ASTERISK,LABELS::opengl_use,use_opengl);
	hbox = create_hbox(vbox);
	comGLFilter = create_combo_box(hbox,CMsg::Filter_Type,LABELS::opengl_filter,gl_filter_type);
#ifndef USE_OPENGL
	gtk_widget_set_sensitive(comUseOpenGL, FALSE);
	gtk_widget_set_sensitive(comGLFilter, FALSE);
#endif
#ifdef USE_SCREEN_MIX_SURFACE
	// double buffering
	hbox = create_hbox(vbox);
	chkDoubleBuffer = create_check_box(hbox, CMsg::Use_double_buffering_when_method_is_default, pConfig->double_buffering);
#endif
	// border color
	hbox = create_hbox(vbox);
	chkBorderColor = create_check_box(hbox, CMsg::Set_gray_color_on_the_border_area, FLG_ORIG_BORDER_COLOR != 0);

	create_frame(hboxall, CMsg::CRTC, &vbox, &hbox);
#if defined(_X68000)
	UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->raster_int_vskew);
	txtRasterSkewV = create_text_with_label(hbox, CMsg::Raster_Interrupt_Vertical_Skew, buf, 8);
	hbox = create_hbox(vbox);
	UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->raster_int_hskew);
	txtRasterSkewH = create_text_with_label(hbox, CMsg::Raster_Interrupt_Horizontal_Skew, buf, 8);
	hbox = create_hbox(vbox);
	UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->vdisp_skew);
	txtVertSkew = create_text_with_label(hbox, CMsg::Vertical_Disp_Skew, buf, 8);
#endif

	int led_show = gui->GetLedBoxPhase(-1);
	hbox = create_hbox(vboxall);
	comLEDShow = create_combo_box(hbox,CMsg::LED,LABELS::led_show,led_show);
	comLEDPos = create_combo_box(hbox,CMsg::Position,LABELS::led_pos,pConfig->led_pos);

	hbox = create_hbox(vboxall);
	comCapType = create_combo_box(hbox,CMsg::Capture_Type,LABELS::capture_fmt,pConfig->capture_type);

	hbox = create_hbox(vboxall);
	txtSnapPath = create_text_with_label(hbox, CMsg::Snapshot_Path, pConfig->snapshot_path.GetN(), 40);
	create_button(hbox,CMsg::Folder_, G_CALLBACK(OnSelectSnapPath));

	hbox = create_hbox(vboxall);
	txtFontPath = create_text_with_label(hbox, CMsg::Font_Path, pConfig->font_path.GetN(), 40);
	create_button(hbox, CMsg::Folder_, G_CALLBACK(OnSelectFontPath));

	hbox = create_hbox(vboxall);
	txtMsgFontName = create_text_with_label(hbox, CMsg::Message_Font, pConfig->msgboard_msg_fontname.GetN(), 24);
	UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->msgboard_msg_fontsize);
	txtMsgFontSize = create_text_with_label(hbox, CMsg::_Size, buf, 3);
	create_button(hbox, CMsg::File_, G_CALLBACK(OnSelectMessageFont));

	hbox = create_hbox(vboxall);
	txtInfoFontName = create_text_with_label(hbox, CMsg::Info_Font, pConfig->msgboard_info_fontname.GetN(), 24);
	UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->msgboard_info_fontsize);
	txtInfoFontSize = create_text_with_label(hbox, CMsg::_Size, buf, 3);
	create_button(hbox, CMsg::File_, G_CALLBACK(OnSelectInfoFont));

	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);
	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);

	hbox = create_hbox(vboxall);
	comLanguage = create_combo_box(hbox, CMsg::Language_ASTERISK, lang_list, lang_selidx);

	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Need_restart_program);

	// ----------------------------------------
	// 3 Tape, FDD
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[3]);

#ifdef USE_DATAREC
	hboxall = create_hbox(vboxall);

	create_frame(hboxall, CMsg::Load_Wav_File_from_Tape, &vbox, &hbox);
	chkReverseWave = create_check_box(hbox, CMsg::Reverse_Wave, pConfig->wav_reverse);
	hbox = create_hbox(vbox);
	chkHalfWave = create_check_box(hbox, CMsg::Half_Wave, pConfig->wav_half);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Correct);
	int correct_type = pConfig->wav_correct ? pConfig->wav_correct_type + 1 : 0;
	create_radio_box(hbox, LABELS::correct, 3, radCorrectType, correct_type);

	hbox = create_hbox(vbox);
	for(int i=0; i<2; i++) {
		UTILITY::sprintf(buf, sizeof(buf), "%d", pConfig->wav_correct_amp[i]);
		txtCorrectAmp[i] = create_text_with_label(hbox, LABELS::correct_amp[i], buf, 4);
	}

	create_frame(hboxall, CMsg::Save_Wav_File_to_Tape, &vbox, &hbox);
	comSampleRate = create_combo_box(hbox,CMsg::Sample_Rate,LABELS::sound_rate,pConfig->wav_sample_rate);
	hbox = create_hbox(vbox);
	comSampleBits = create_combo_box(hbox,CMsg::Sample_Bits,LABELS::sound_bits,pConfig->wav_sample_bits);
#endif

	// FDD
#ifdef USE_FD1
	hboxall = create_hbox(vboxall);
	// fdd mount
	create_frame(hboxall, CMsg::Floppy_Disk_Drive, &vbox, &hbox);
	create_label(hbox, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		UTILITY::sprintf(buf, sizeof(buf), "%d ", i);
		chkFDMount[i] = create_check_box(hbox, buf, pConfig->mount_fdd & (1 << i));
	}
	hbox = create_hbox(vbox);
	chkDelayFd1 = create_check_box(hbox, CMsg::Ignore_delays_to_find_sector, (FLG_DELAY_FDSEARCH != 0));
	hbox = create_hbox(vbox);
	chkDelayFd2 = create_check_box(hbox, CMsg::Ignore_delays_to_seek_track, (FLG_DELAY_FDSEEK != 0));
	hbox = create_hbox(vbox);
	chkFdDensity = create_check_box(hbox, CMsg::Suppress_checking_for_density, (FLG_CHECK_FDDENSITY == 0));
	hbox = create_hbox(vbox);
	chkFdMedia = create_check_box(hbox, CMsg::Suppress_checking_for_media_type, (FLG_CHECK_FDMEDIA == 0));
	hbox = create_hbox(vbox);
	chkFdSavePlain = create_check_box(hbox, CMsg::Save_a_plain_disk_image_as_it_is, (FLG_SAVE_FDPLAIN != 0));

	// SRAM for fd
	create_frame(hboxall, CMsg::Parameters_for_floppy_disk_in_SRAM, &vbox, &hbox);

	// eject fd
	valuei = vm->get_sram_fd_eject();
	chkSramFdEject = create_check_box(hbox, CMsg::Eject_FD_when_power_off, valuei != 0);
#endif

	// HDD
#ifdef USE_HD1
	hboxall = create_hbox(vboxall);
	// hdd mount
	create_frame(hboxall, CMsg::Hard_Disk_Drive, &vbox, &hbox);
	create_label(hbox, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<MAX_HARD_DISKS; i++) {
		int idx = pConfig->GetHardDiskIndex(i);
		if (idx < 0) continue;
		if (i==0 || i==MAX_SASI_HARD_DISKS) {
			hbox = create_hbox(vbox);
			create_label(hbox, "  ");
		}
		if (i<MAX_SASI_HARD_DISKS) {
			int sdrv = i / SASI_UNITS_PER_CTRL;
			int sunit = i % SASI_UNITS_PER_CTRL;
			UTILITY::sprintf(buf, sizeof(buf), "SASI%d u%d ", sdrv, sunit);
		} else {
			UTILITY::sprintf(buf, sizeof(buf), "SCSI%d ", i - MAX_SASI_HARD_DISKS);
		}
		chkHDMount[idx] = create_check_box(hbox, buf, pConfig->mount_hdd & (1 << i));
	}

	hbox = create_hbox(vbox);
	chkDelayHd2 = create_check_box(hbox, CMsg::Ignore_delays_to_seek_track, (FLG_DELAY_HDSEEK != 0));

	// SRAM for hd
	create_frame(hboxall, CMsg::Parameters_for_hard_disk_in_SRAM, &vbox, &hbox);

	// number of SASI HDDs
	valuei = vm->get_sram_sasi_hdd_nums();
	create_label(hbox, CMsg::Number_of_SASI_HDDs);
	spnSramNumHdds = create_spin(hbox, 0, 15, valuei);

	// SCSI enable flag
	hbox = create_hbox(vbox);
	chkSramScsiEn = create_check_box(hbox, CMsg::SCSI_enable_flag, vm->get_sram_scsi_enable_flag());

	// SCSI host ID
	hbox = create_hbox(vbox);
	valuei = vm->get_sram_scsi_host_id();
	UTILITY::sprintf(buf, sizeof(buf), "%d", valuei);
	txtSramScsiId = create_text_with_label(hbox, CMsg::SCSI_host_ID, buf, 4);

	// SASI on SCSI
	hbox = create_hbox(vbox);
	valueu = vm->get_sram_sasi_hdd_on_scsi();
	UTILITY::sprintf(buf, sizeof(buf), "%x", valueu);
	txtSramSasiOnScsi = create_text_with_label(hbox, CMsg::SASI_HDDs_on_SCSI, buf, 4);


	// SCSI Type
	hboxall = create_hbox(vboxall);
	create_frame(hboxall, CMsg::SCSI_Type_ASTERISK, &vbox, &hbox);
	create_radio_box(hbox, LABELS::scsi_type, SCSI_TYPE_END, radSCSIType, emu->get_parami(VM::ParamSCSIType));
//	hbox = create_hbox(vbox);
	UTILITY::tcscpy(buf, sizeof(buf), CMSG(LB_Now_SP));
	UTILITY::tcscat(buf, sizeof(buf), CMSGV(LABELS::scsi_type[pConfig->scsi_type]));
	UTILITY::tcscat(buf, sizeof(buf), _T(")"));
	create_label(hbox, buf);

	hboxall = create_hbox(vboxall);
	create_label(hboxall, CMsg::Need_restart_program_or_PowerOn);
#endif

	// ----------------------------------------
	// 4 sound
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[4]);

	// MIDI
	hboxall = create_hbox(vboxall);
	create_frame(hboxall, CMsg::MIDI, &vbox, &hbox);

	bool valueb = ((vm->get_parami(VM::ParamIOPort) & IOPORT_MIDI) != 0);
	chkEnableMIDI = create_check_box(hbox, CMsg::Enable_MIDI_Board_CZ_6BM1_ASTERISK, valueb);
	UTILITY::tcscpy(buf, sizeof(buf), CMSG(LB_Now_SP));
	UTILITY::tcscat(buf, sizeof(buf), valueb ? CMSG(Enable) : CMSG(Disable));
	UTILITY::tcscat(buf, sizeof(buf), _T(")"));
	create_label(hbox, buf);
#ifdef USE_MIDI
	CPtrList<CTchar> midiout_list;
	int midiout_cnt = gui->EnumMidiOuts();
	int midiout_conn = 0;
	midiout_list.Add(new CTchar(CMSG(None_)));
	for(int idx = 0; idx < midiout_cnt && idx < MIDI_MAX_PORTS; idx++) {
		gui->GetMidiOutDescription(idx, buf, sizeof(buf)/sizeof(buf[0]));
		if (gui->NowConnectingMidiOut(idx)) {
			midiout_conn = idx + 1;
		}
		midiout_list.Add(new CTchar(buf));
	}
	hbox = create_hbox(vbox);
	comMIDIOut = create_combo_box(hbox, CMsg::Now_Connecting_MIDI_Output, midiout_list, midiout_conn);

	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Output_Latency);
	valuei = pConfig->midiout_delay;
	spnMIDIOutDelay = create_spin(hbox, 0, 2000, valuei);
	create_label(hbox, CMsg::msec);

	hbox = create_hbox(vbox);
	comMIDIResetType = create_combo_box(hbox, CMsg::MIDI_Reset_Type, LABELS::midi_type, pConfig->midi_send_reset_type);
	create_button(hbox, CMsg::Send_message_now, G_CALLBACK(OnSelectSendMIDIResetMessage));

	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::When_send_reset_message_);
	chkMIDIResPowerOn = create_check_box(hbox, CMsg::Power_on, (pConfig->midi_flags & MIDI_FLAG_RES_POWERON) != 0);
	chkMIDIResPowerOff = create_check_box(hbox, CMsg::Power_off, (pConfig->midi_flags & MIDI_FLAG_RES_POWEROFF) != 0);
	chkMIDIResHardRes = create_check_box(hbox, CMsg::Reset_by_hand, (pConfig->midi_flags & MIDI_FLAG_RES_HARDRES) != 0);
	chkMIDIResEndApp = create_check_box(hbox, CMsg::End_of_app_, (pConfig->midi_flags & MIDI_FLAG_RES_END_APP) != 0);

	hbox = create_hbox(vbox);
	chkMIDINoRTMsg = create_check_box(hbox, CMsg::Does_not_send_system_real_time_messages_, (pConfig->midi_flags & MIDI_FLAG_NO_REALTIME_MSG) != 0);
#endif
	hboxall = create_hbox(vboxall);
	create_label(hboxall, CMsg::Need_restart_program_or_PowerOn);

	// ----------------------------------------
	// 5 network
	// ----------------------------------------

	vboxall = create_vbox(NULL);
	add_note(nb, vboxall, LABELS::tabs[5]);

	// LPT
#ifdef MAX_PRINTER
	for(int drv=0; drv<MAX_PRINTER; drv++) {
		hbox = create_hbox(vboxall);
		sprintf(buf, CMSG(Printer_Hostname), drv);
		txtLPTHost[drv] = create_text_with_label(hbox, buf, pConfig->printer_server_host[drv].GetN(), 20);
		sprintf(buf, "%d", pConfig->printer_server_port[drv]);
		txtLPTPort[drv] = create_text_with_label(hbox, CMsg::_Port, buf, 5);
		sprintf(buf, "%.1f", pConfig->printer_delay[drv]);
		txtLPTDelay[drv] = create_text_with_label(hbox, CMsg::_Print_delay, buf, 5);
		create_label(hbox, CMsg::msec);
	}
#endif
#ifdef MAX_COMM
	// COM
	for(int drv=0; drv<MAX_COMM; drv++) {
		hbox = create_hbox(vboxall);
		sprintf(buf, CMSG(Communication_Hostname), drv);
		txtCOMHost[drv] = create_text_with_label(hbox, buf, pConfig->comm_server_host[drv].GetN(), 20);
		sprintf(buf, "%d", pConfig->comm_server_port[drv]);
		txtCOMPort[drv] = create_text_with_label(hbox, CMsg::_Port, buf, 5);
//		comCOMBaud[drv] = create_combo_box(hbox, " ", LABELS::comm_baud, pConfig->comm_dipswitch[drv]);
	}
#endif
#ifdef USE_DEBUGGER
	hbox = create_hbox(vboxall);
	create_label(hbox, CMsg::Connectable_host_to_Debugger);
	hbox = create_hbox(vboxall);
	txtDbgrHost = create_text_with_label(hbox, " ", pConfig->debugger_server_host.GetN(), 20);
	sprintf(buf, "%d", pConfig->debugger_server_port);
	txtDbgrPort = create_text_with_label(hbox, CMsg::_Port, buf, 5);
#endif
	// uart
	int uart_baud_index = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == (int)strtol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			uart_baud_index = i;
			break;
		}
	}
	create_frame(vboxall, CMsg::Settings_of_serial_ports_on_host, &vbox, &hbox);
	create_label(hbox, CMsg::Baud_Rate);
	comCOMUartBaud = create_combo_box(hbox, " ", LABELS::comm_uart_baudrate, uart_baud_index);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Data_Bit);
	comCOMUartDataBit = create_combo_box(hbox, " ", LABELS::comm_uart_databit, pConfig->comm_uart_databit - 7);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Parity);
	comCOMUartParity = create_combo_box(hbox, " ", LABELS::comm_uart_parity, pConfig->comm_uart_parity);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Stop_Bit);
	comCOMUartStopBit = create_combo_box(hbox, " ", LABELS::comm_uart_stopbit, pConfig->comm_uart_stopbit - 1);
	hbox = create_hbox(vbox);
	create_label(hbox, CMsg::Flow_Control);
	comCOMUartFlowCtrl = create_combo_box(hbox, " ", LABELS::comm_uart_flowctrl, pConfig->comm_uart_flowctrl);
	create_label(vbox, CMsg::Need_re_connect_to_serial_port_when_modified_this);

	//

	gtk_widget_show_all(dialog);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));

	emu->set_pause(1, true);

	return true;
}

void ConfigBox::Hide()
{
	DialogBox::Hide();
	emu->set_pause(1, false);
}

bool ConfigBox::SetData()
{
	int val = 0;

	// power off
	pConfig->use_power_off = (get_check_state(chkPowerOff));

	// power state when start up
	pConfig->power_state_when_start_up = get_combo_sel_num(comPowerState);

#ifdef USE_FD1
	pConfig->mount_fdd = get_check_state_num(chkFDMount, USE_FLOPPY_DISKS);
	pConfig->option_fdd = (get_check_state(chkDelayFd1) ? MSK_DELAY_FDSEARCH : 0)
		| (get_check_state(chkDelayFd2) ? MSK_DELAY_FDSEEK : 0)
		| (get_check_state(chkFdDensity) ? 0 : MSK_CHECK_FDDENSITY)
		| (get_check_state(chkFdMedia) ? 0 : MSK_CHECK_FDMEDIA)
		| (get_check_state(chkFdSavePlain) ? MSK_SAVE_FDPLAIN : 0);
#endif

#ifdef USE_HD1
	for(int i=0; i<MAX_HARD_DISKS; i++) {
		int idx = pConfig->GetHardDiskIndex(i);
		if (idx < 0) continue;
		bool chkd = get_check_state(chkHDMount[idx]);
		BIT_ONOFF(pConfig->mount_hdd, (1 << i), chkd);
	}
	pConfig->option_hdd = (get_check_state(chkDelayHd2) ? MSK_DELAY_HDSEEK : 0);
	// SCSI Type
	emu->set_parami(VM::ParamSCSIType, get_radio_state_idx(radSCSIType, SCSI_TYPE_END));
#endif

#ifdef USE_OPENGL
	pConfig->use_opengl = (uint8_t)get_combo_sel_num(comUseOpenGL);
	pConfig->gl_filter_type = (uint8_t)get_combo_sel_num(comGLFilter);
#endif

	int led_show = get_combo_sel_num(comLEDShow);
	pConfig->led_pos = get_combo_sel_num(comLEDPos);

	pConfig->capture_type = get_combo_sel_num(comCapType);

	pConfig->snapshot_path.Set(get_text(txtSnapPath));
	pConfig->font_path.Set(get_text(txtFontPath));
	pConfig->msgboard_msg_fontname.Set(get_text(txtMsgFontName));
	pConfig->msgboard_info_fontname.Set(get_text(txtInfoFontName));
	val = 0;
	if (sscanf(get_text(txtMsgFontSize), "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_msg_fontsize = val;
	}
	val = 0;
	if (sscanf(get_text(txtInfoFontSize), "%d", &val) == 1 && val >= 6 && val <= 60) {
		pConfig->msgboard_info_fontsize = val;
	}

	// crtc
#if defined(_X68000)
	val = 0;
	if (sscanf(get_text(txtRasterSkewV), "%d", &val) == 1 && -128 <= val && val <= 128) {
		pConfig->raster_int_vskew = val;
	}
	val = 0;
	if (sscanf(get_text(txtRasterSkewH), "%d", &val) == 1 && -512 <= val && val <= 512) {
		pConfig->raster_int_hskew = val;
	}
	val = 0;
	if (sscanf(get_text(txtVertSkew), "%d", &val) == 1  && -128 <= val && val <= 128) {
		pConfig->vdisp_skew = val;
	}
#endif

#ifdef USE_SCREEN_MIX_SURFACE
	// double buffering
	pConfig->double_buffering = get_check_state(chkDoubleBuffer);
#endif

	// border color
	BIT_ONOFF(pConfig->original, MSK_ORIG_BORDER_COLOR, get_check_state(chkBorderColor));

	val = get_combo_sel_num(comLanguage);
	clocale->ChooseLocaleName(lang_list, val, pConfig->language);

#ifdef USE_DATAREC
	pConfig->wav_reverse = get_check_state(chkReverseWave);
	pConfig->wav_half = get_check_state(chkHalfWave);
	pConfig->wav_correct_type = (uint8_t)get_radio_state_idx(radCorrectType, 3);
	pConfig->wav_correct = (pConfig->wav_correct_type > 0);
	if (pConfig->wav_correct_type > 0) pConfig->wav_correct_type--;

	for(int i=0; i<2; i++) {
		val = 0;
		if (sscanf(get_text(txtCorrectAmp[i]), "%d", &val) == 1 && val >= 100 && val <= 5000) {
			pConfig->wav_correct_amp[i] = val;
		}
	}

	pConfig->wav_sample_rate = (uint8_t)get_combo_sel_num(comSampleRate);
	pConfig->wav_sample_bits = (uint8_t)get_combo_sel_num(comSampleBits);
#endif
#ifdef MAX_PRINTER
	for(int i=0; i<MAX_PRINTER; i++) {
		pConfig->printer_server_host[i].Set(get_text(txtLPTHost[i]));
		val = 0;
		if (sscanf(get_text(txtLPTPort[i]), "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->printer_server_port[i] = val;
		}
		double valued = 0.0;
		valued = strtod(get_text(txtLPTDelay[i]), NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
#endif
#ifdef MAX_COMM
	for(int i=0; i<MAX_COMM; i++) {
		pConfig->comm_server_host[i].Set(get_text(txtCOMHost[i]));
		val = 0;
		if (sscanf(get_text(txtCOMPort[i]), "%d", &val) == 1 && val > 0 && val < 65535) {
			pConfig->comm_server_port[i] = val;
		}
//		pConfig->comm_dipswitch[i] = get_combo_sel_num(comCOMBaud[i]);
	}
#endif
#ifdef USE_DEBUGGER
	pConfig->debugger_server_host.Set(get_text(txtDbgrHost));
	val = 0;
	if (sscanf(get_text(txtDbgrPort), "%d", &val) == 1 && val > 0 && val < 65535) {
		pConfig->debugger_server_port = val;
	}
#endif
	int uart_baud_index = get_combo_sel_num(comCOMUartBaud);
	pConfig->comm_uart_baudrate = (int)strtol(LABELS::comm_uart_baudrate[uart_baud_index], NULL, 10);
	pConfig->comm_uart_databit = get_combo_sel_num(comCOMUartDataBit) + 7;
	pConfig->comm_uart_parity = get_combo_sel_num(comCOMUartParity);
	pConfig->comm_uart_stopbit = get_combo_sel_num(comCOMUartStopBit) + 1;
	pConfig->comm_uart_flowctrl = get_combo_sel_num(comCOMUartFlowCtrl);

	pConfig->rom_path.Set(get_text(txtROMPath));

#if defined(_X68000)
	emu->set_parami(VM::ParamMainRamSizeNum, get_combo_sel_num(comRamSize));
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_RAM_SIZE, get_check_state(chkOwSizeSRAM));

	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, get_check_state(chkAddrErr));

	// SRAM
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CLR_PWRON, get_check_state(chkSRAMClear));
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_SAVE_PWROFF, get_check_state(chkSRAMSave));
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CHG_BOOT_DEV, get_check_state(chkSRAMChanged));

	VM *vm = emu->get_vm();
	uint32_t valueu = 0;
	char *endptr = NULL;

	// ROM start address
	valueu = (uint32_t)strtol(get_text(txtSramRomAddr), &endptr, 16);
	if (endptr && *endptr == '\0') {
		vm->set_sram_rom_start_address(valueu);
	}
	// SRAM start address
	valueu = (uint32_t)strtol(get_text(txtSramSramAddr), &endptr, 16);
	if (endptr && *endptr == '\0') {
		vm->set_sram_sram_start_address(valueu);
	}
	// boot device
	vm->set_sram_boot_device(get_combo_sel_num(comSramBootDevice));
	// RS-232C
	valueu = 0;
	valueu |= vm->conv_sram_rs232c_baud_rate(get_combo_sel_num(comSramRS232CBaud));
	valueu |= vm->conv_sram_rs232c_databit(get_combo_sel_num(comSramRS232CDataBit));
	valueu |= vm->conv_sram_rs232c_parity(get_combo_sel_num(comSramRS232CParity));
	valueu |= vm->conv_sram_rs232c_stopbit(get_combo_sel_num(comSramRS232CStopBit));
	valueu |= vm->conv_sram_rs232c_flowctrl(get_combo_sel_num(comSramRS232CFlowCtrl));
	vm->set_sram_rs232c(valueu);
	// alarm
	vm->set_sram_alarm_onoff(get_check_state(chkSramAlarm));
	// contrast
	val = (int)get_spin_value(spnSramContrast);
	if (val >= 0 && val <= 15) {
		vm->set_sram_contrast(val);
	}
	// eject fd
	val = get_check_state(chkSramFdEject) ? 1 : 0;
	vm->set_sram_fd_eject(val);
	// purpose of SRAM free area
	vm->set_sram_purpose(get_combo_sel_num(comSramPurpose));
	// key repeat delay
	vm->set_sram_key_repeat_delay(get_combo_sel_num(comSramKRDelay));
	// key repeat rate
	vm->set_sram_key_repeat_rate(get_combo_sel_num(comSramKRRate));
	// key LED
	val = 0;
	val |= get_check_state(chkSramKLEDkana) ? 1 : 0;
	val |= get_check_state(chkSramKLEDromaji) ? 2 : 0;
	val |= get_check_state(chkSramKLEDcinput) ? 4 : 0;
	val |= get_check_state(chkSramKLEDcaps) ? 8 : 0;
	val |= get_check_state(chkSramKLEDins) ? 16 : 0;
	val |= get_check_state(chkSramKLEDhira) ? 32 : 0;
	val |= get_check_state(chkSramKLEDzen) ? 64 : 0;
	vm->set_sram_key_led(val);

#ifdef USE_HD1
	// number of SASI HDDs
	val = (int)get_spin_value(spnSramNumHdds);
	if (val >= 0 && val <= 15) {
		vm->set_sram_sasi_hdd_nums(val);
	}
	// SCSI enable flag
	vm->set_sram_scsi_enable_flag(get_check_state(chkSramScsiEn));
	// SCSI host ID
	val = (int)strtol(get_text(txtSramScsiId), &endptr, 10);
	if (endptr && *endptr == '\0' && 0 <= val && val <= 7) {
		vm->set_sram_scsi_host_id(val);
	}
	// SASI on SCSI
	valueu = (uint32_t)strtol(get_text(txtSramSasiOnScsi), &endptr, 16);
	if (endptr && *endptr == '\0') {
		vm->set_sram_sasi_hdd_on_scsi(valueu & 0xff);
	}
#endif
#endif

	int io_port = vm->get_parami(VM::ParamIOPort);
	BIT_ONOFF(io_port, IOPORT_MIDI, get_check_state(chkEnableMIDI));
	vm->set_parami(VM::ParamIOPort, io_port);
#ifdef USE_MIDI
	int midiout_conn = get_combo_sel_num(comMIDIOut);
	gui->ConnectMidiOut(midiout_conn - 1);
	val = (int)get_spin_value(spnMIDIOutDelay);
	if (0 <= val && val <= 2000) {
		pConfig->midiout_delay = val;
		emu->set_midiout_delay_time(val);
	}
	pConfig->midi_send_reset_type = get_combo_sel_num(comMIDIResetType);
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_POWERON, get_check_state(chkMIDIResPowerOn));
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_POWEROFF, get_check_state(chkMIDIResPowerOff));
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_HARDRES, get_check_state(chkMIDIResHardRes));
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_RES_END_APP, get_check_state(chkMIDIResEndApp));
	BIT_ONOFF(pConfig->midi_flags, MIDI_FLAG_NO_REALTIME_MSG, get_check_state(chkMIDINoRTMsg));
#endif

	// set message font
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}

	gui->ChangeLedBox(led_show);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
	pConfig->save();
#ifdef USE_OPENGL
	emu->change_opengl_attr();
#endif
	emu->update_config();
#if defined(_X68000)
	vm->save_sram_file();
#endif

	return true;
}

void ConfigBox::ShowFolderBox(const char *title, GtkWidget *entry)
{
	FolderBox fbox(dialog);
	const char *path = gtk_entry_get_text(GTK_ENTRY(entry));
	if (fbox.Show(title, path)) {
		gtk_entry_set_text(GTK_ENTRY(entry),fbox.GetPath());
	}
}

void ConfigBox::ShowFontFileBox(const char *title, GtkWidget *entry)
{
	FileBox fbox;
	const CMsg::Id filter[] = {
			CMsg::Supported_Files_ttf_otf,
			CMsg::All_Files_,
			CMsg::End
	};
	const char *path = gtk_entry_get_text(GTK_ENTRY(entry));
	const char *dir = "";
//	const char *ext = ".ttf";
	if (fbox.Show(dialog, filter, title, dir, false, path)) {
		gtk_entry_set_text(GTK_ENTRY(entry),fbox.GetPath());
	}
}

void ConfigBox::ChangeFDDType(int index)
{
}

void ConfigBox::ChangeIOPort(int index)
{
}

void ConfigBox::ChangeFmOpn()
{
}

void ConfigBox::ChangeExPsg()
{
}

#ifdef USE_MIDI
void ConfigBox::SendMIDIResetMessage()
{
	int midiout_conn = get_combo_sel_num(comMIDIOut);
	gui->ConnectMidiOut(midiout_conn - 1);
	gui->SendMidiResetMessage(get_combo_sel_num(comMIDIResetType));
}
#endif

void ConfigBox::OnChangedFDD(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->ChangeFDDType(index);
}

void ConfigBox::OnChangedIO(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	int index = (int)(intptr_t)g_object_get_data(G_OBJECT(widget), "num");
	obj->ChangeIOPort(index);
}

void ConfigBox::OnChangedFmOpn(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ChangeFmOpn();
}

void ConfigBox::OnChangedExPsg(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ChangeExPsg();
}

void ConfigBox::OnSelectSnapPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(Snapshot_Path), obj->txtSnapPath);
}

void ConfigBox::OnSelectFontPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(Font_Path), obj->txtFontPath);
}

void ConfigBox::OnSelectMessageFont(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFontFileBox(CMSG(Message_Font), obj->txtMsgFontName);
}

void ConfigBox::OnSelectInfoFont(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFontFileBox(CMSG(Info_Font), obj->txtInfoFontName);
}

void ConfigBox::OnSelectROMPath(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->ShowFolderBox(CMSG(ROM_Path), obj->txtROMPath);
}

#ifdef USE_MIDI
void ConfigBox::OnSelectSendMIDIResetMessage(GtkWidget *widget, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	obj->SendMIDIResetMessage();
}
#endif

void ConfigBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	ConfigBox *obj = (ConfigBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->SetData();
	}
//	g_print("OnResponse: %d\n",response_id);
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


