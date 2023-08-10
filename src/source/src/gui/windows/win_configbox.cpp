/** @file win_configbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ config box ]
*/
#include "win_configbox.h"
#include "../../emu.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "win_folderbox.h"
#include "win_filebox.h"
#include "win_fontbox.h"
#include "../../res/resource.h"
#include "../../config.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "../../rec_video_defs.h"
#include "win_gui.h"
#include "../../labels.h"
#include <math.h>

namespace GUI_WIN
{

#ifdef USE_IOPORT_FDD
#define IOPORT_STARTNUM 0
#else
#define IOPORT_STARTNUM 2
#endif

ConfigBox::ConfigBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_CONFIGURE, new_font, new_emu, new_gui)
{
	hInstance = hInst;

//	fdd_type  = pConfig->fdd_type;
	io_port = pConfig->io_port;

	selected_tabctrl = 0;
}

ConfigBox::~ConfigBox()
{
}

INT_PTR ConfigBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hCtrl;

	//
	CDialogBox::onInitDialog(message, wParam, lParam);

//	fdd_type = emu->get_parami(VM::ParamFddType);
	io_port = emu->get_parami(VM::ParamIOPort);
#if defined(_MBS1)
	sys_mode = emu->get_parami(VM::ParamSysMode);
#endif

	CBox *vbox;
	CBox *hbox;

	_TCHAR str[128];

	VM *vm = emu->get_vm();
	int vali;
	uint32_t valu;

	tab_items.Clear();

	// tab control
	CBox *box_all = new CBox(CBox::VerticalBox, 0, margin, _T("all"));
	CBox *box_tab = AdjustTabControl(box_all, IDC_TAB1, IDC_STATIC_0);
	box_tab->SetTabItems(&tab_items);

	HWND hTabCtrl = GetDlgItem(hDlg, IDC_TAB1);

	TCITEM tcitm;
	tcitm.mask = TCIF_TEXT;

	for(int i=0; LABELS::tabs[i] != CMsg::End; i++) {
		tcitm.pszText = (LPSTR)CMSGVM(LABELS::tabs[i]);
		TabCtrl_InsertItem(hTabCtrl , i , &tcitm);
		tab_items.Add(new CTabItemIds());
	}
	TabCtrl_SetCurSel(hTabCtrl, selected_tabctrl);

	// ----------------------------------------
	// 0:CPU, Memory
	// ----------------------------------------
	CBox *box_0all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("0all"));
	tab_items.SetCurrentPosition(0);

	// rom path
	hbox = box_0all->AddBox(CBox::HorizontalBox, 0, 0, _T("rompath"));
	CreateEditBoxWithLabel(hbox, IDC_ROM_PATH, CMsg::ROM_Path_ASTERISK, pConfig->rom_path.GetM(), 30);
	CreateButton(hbox, IDC_BTN_ROM_PATH, CMsg::Folder_, 5);

#if defined(_X68000)
	// main ram size
	CBox *box_ram0 = CreateGroup(box_0all, IDC_STATIC, CMsg::RAM, CBox::VerticalBox);
	hbox = box_ram0->AddBox(CBox::HorizontalBox, 0, 0, _T("mainram"));
	CreateComboBoxWithLabel(hbox, IDC_COMBO_EXMEM, CMsg::RAM_Size_ASTERISK, LABELS::main_ram_size, emu->get_parami(VM::ParamMainRamSizeNum), 5);
	if (0 <= pConfig->main_ram_size_num && pConfig->main_ram_size_num <= 4) {
		UTILITY::tcscpy(str, sizeof(str), CMSGM(LB_Now_SP));
		UTILITY::tcscat(str, sizeof(str), CMSGVM(LABELS::main_ram_size[pConfig->main_ram_size_num]));
		UTILITY::tcscat(str, sizeof(str), _T(")"));
	} else {
		str[0] = _T('\0');
	}
	CreateStatic(hbox, IDC_STATIC, str);
	// overwrite size on SRAM
	CreateCheckBox(box_ram0, IDC_CHK_SRAM_RAM_SIZE, CMsg::No_overwrite_RAM_size_on_SRAM, FLG_ORIG_SRAM_RAM_SIZE != 0);

	// CPU
	CBox *box_cpu = CreateGroup(box_0all, IDC_STATIC, CMsg::CPU, CBox::VerticalBox);
	// undef opcode
	CreateCheckBox(box_cpu, IDC_CHK_ADDRERR, CMsg::Show_message_when_the_address_error_occured_in_the_CPU, FLG_SHOWMSG_ADDRERR != 0);
#endif

	// power off
	CreateCheckBox(box_0all, IDC_CHK_POWEROFF, CMsg::Enable_the_state_of_power_off, pConfig->use_power_off);

	// text
	CreateStatic(box_0all, IDC_STATIC, CMsg::Need_restart_program_or_PowerOn);

	// ----------------------------------------
	// 1:SRAM
	// ----------------------------------------
	CBox *box_1all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("1all"));
	tab_items.SetCurrentPosition(1);

	// behavior
	vbox = CreateGroup(box_1all, IDC_STATIC, CMsg::Behavior, CBox::VerticalBox);
	hbox = vbox->AddBox(CBox::HorizontalBox, 0, 0, _T("sramh"));
	CreateCheckBox(hbox, IDC_CHK_SRAM_CLR_PWR_ON, CMsg::Clear_SRAM_when_power_on, FLG_ORIG_SRAM_CLR_PWRON != 0);
	CreateStatic(hbox, IDC_STATIC, _T("  "));
	CreateCheckBox(hbox, IDC_CHK_SRAM_SAVE_PWR_OFF, CMsg::Save_SRAM_when_power_off, FLG_ORIG_SRAM_SAVE_PWROFF != 0);
	hbox = vbox->AddBox(CBox::HorizontalBox, 0, 0, _T("sramh2"));
	CreateCheckBox(hbox, IDC_CHK_SRAM_CHG_BOOT_DEVICE, CMsg::Show_message_when_boot_device_was_changed, FLG_ORIG_SRAM_CHG_BOOT_DEV != 0);

	//
	//
	CreateStatic(box_1all, IDC_STATIC_1, _T(""), 256, 1, 0, WS_EX_STATICEDGE);

	CBox *box_1hall = box_1all->AddBox(CBox::HorizontalBox, 0, 0, _T("1hall"));
	CBox *box_1l = box_1hall->AddBox(CBox::VerticalBox, 0, 0, _T("1l"));
	CBox *box_1m = box_1hall->AddBox(CBox::VerticalBox, 0, 0, _T("1m"));
	CBox *box_1r = box_1hall->AddBox(CBox::VerticalBox, 0, 0, _T("1r"));

	CreateStatic(box_1l, IDC_STATIC, CMsg::Parameters_in_SRAM_);
	CreateStatic(box_1m, IDC_STATIC, _T(""), 8, 8);

	// L

	// ram size
	hbox = box_1l->AddBox(CBox::HorizontalBox, 0, 0, _T("ramsiz"));
	valu = vm->get_sram_ram_size();
	UTILITY::stprintf(str, sizeof(str), _T("%x"), valu);
	CreateEditBoxWithLabel(hbox, IDC_EDIT_SRAM_RAM_SIZE, CMsg::RAM_Size_Hex, str, 10);
	EnableDlgItem(IDC_EDIT_SRAM_RAM_SIZE, false);

	// rom start address
	hbox = box_1l->AddBox(CBox::HorizontalBox, 0, 0, _T("romstart"));
	valu = vm->get_sram_rom_start_address();
	UTILITY::stprintf(str, sizeof(str), _T("%x"), valu);
	CreateEditBoxWithLabel(hbox, IDC_EDIT_SRAM_ROM_ADDRESS, CMsg::ROM_Start_Address_Hex, str, 10);

	// sram start address
	hbox = box_1l->AddBox(CBox::HorizontalBox, 0, 0, _T("ramstart"));
	valu = vm->get_sram_sram_start_address();
	UTILITY::stprintf(str, sizeof(str), _T("%x"), valu);
	CreateEditBoxWithLabel(hbox, IDC_EDIT_SRAM_SRAM_ADDRESS, CMsg::SRAM_Start_Address_Hex, str, 10);

	// boot device
	hbox = box_1l->AddBox(CBox::HorizontalBox, 0, 0, _T("booth"));
	vali = vm->get_sram_boot_device();
	CreateComboBoxWithLabel(hbox, IDC_COMBO_SRAM_BOOT_DEVICE, CMsg::Boot_Device, LABELS::boot_devices, vali, 6);

	// RS-232C
	int w_232c = 100;
	CBox *box_232c = CreateGroup(box_1l, IDC_STATIC, CMsg::RS_232C, CBox::VerticalBox);
	CBox *box_232c0 = box_232c->AddBox(CBox::HorizontalBox, 0, 0, _T("232c0"));
	vbox = box_232c0->AddBox(CBox::VerticalBox, 0, 0, _T("232c1"));
	CreateStatic(vbox, IDC_STATIC, CMsg::Baud_Rate, w_232c);
	CreateStatic(vbox, IDC_STATIC, CMsg::Data_Bit, w_232c);
	CreateStatic(vbox, IDC_STATIC, CMsg::Parity, w_232c);
	CreateStatic(vbox, IDC_STATIC, CMsg::Stop_Bit, w_232c);
	CreateStatic(vbox, IDC_STATIC, CMsg::Flow_Control, w_232c);
	//
	vbox = box_232c0->AddBox(CBox::VerticalBox, 0, 0, _T("232c2"));
	valu = vm->get_sram_rs232c();
	vali = vm->get_sram_rs232c_baud_rate(valu);
	CreateComboBox(vbox, IDC_COMBO_SRAM_RS232C_BAUDRATE, LABELS::rs232c_baudrate, vali, 8);
	vali = vm->get_sram_rs232c_databit(valu);
	CreateComboBox(vbox, IDC_COMBO_SRAM_RS232C_DATABIT, LABELS::rs232c_databit, vali, 8);
	vali = vm->get_sram_rs232c_parity(valu);
	CreateComboBox(vbox, IDC_COMBO_SRAM_RS232C_PARITY, LABELS::comm_uart_parity, vali, 8);
	vali = vm->get_sram_rs232c_stopbit(valu);
	CreateComboBox(vbox, IDC_COMBO_SRAM_RS232C_STOPBIT, LABELS::rs232c_stopbit, vali, 8);
	vali = vm->get_sram_rs232c_flowctrl(valu);
	CreateComboBox(vbox, IDC_COMBO_SRAM_RS232C_FLOWCTRL, LABELS::rs232c_flowctrl, vali, 8);

	// R

	// alarm
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("alarm1h"));
	vali = vm->get_sram_alarm_onoff();
	CreateCheckBox(hbox, IDC_CHK_SRAM_ALARM_ONOFF, CMsg::Enable_alarm, vali == 0);
	EnableDlgItem(IDC_CHK_SRAM_ALARM_ONOFF, false);

	// alarm time
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("alarm2h"));
	valu = vm->get_sram_alarm_time();
	UTILITY::stprintf(str, sizeof(str), _T("%08x"), valu);
	CreateEditBoxWithLabel(hbox, IDC_COMBO_SRAM_ALARM_TIME, CMsg::Alarm_Time_Hex, str, 10);
	EnableDlgItem(IDC_COMBO_SRAM_ALARM_TIME, false);

	// alarm duration
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("alarm3h"));
	vali = vm->get_sram_alarm_duration();
	CreateEditBoxWithLabel(hbox, IDC_COMBO_SRAM_ALARM_DURATION, CMsg::Alarm_Duration, vali, 10);
	EnableDlgItem(IDC_COMBO_SRAM_ALARM_DURATION, false);

	// contrast
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("cont1h"));
	vali = vm->get_sram_contrast();
	CreateEditBoxWithLabel(hbox, IDC_EDIT_SRAM_CONTRAST, CMsg::Contrast_on_Monitor, vali, 6);

	// eject fd
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("eject"));
	vali = vm->get_sram_fd_eject();
	CreateCheckBox(hbox, IDC_CHK_SRAM_EJECT_FD, CMsg::Eject_FD_when_power_off, vali != 0);

	// purpose of SRAM free area
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("purp1h"));
	vali = vm->get_sram_purpose();
	CreateComboBoxWithLabel(hbox, IDC_COMBO_SRAM_PURPOSE, CMsg::Purpose_of_SRAM_free_area, LABELS::sram_purpose, vali, 6);

	// key repeat
	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("keyr1h"));
	vali = vm->get_sram_key_repeat_delay();
	CreateComboBoxWithLabel(hbox, IDC_COMBO_SRAM_KEY_REPEAT_DELAY, CMsg::Key_Repeat_Delay, LABELS::key_repeat_delay, vali, 4);
	CreateStatic(hbox, IDC_STATIC, CMsg::msec);

	hbox = box_1r->AddBox(CBox::HorizontalBox, 0, 0, _T("keyr2h"));
	vali = vm->get_sram_key_repeat_rate();
	CreateComboBoxWithLabel(hbox, IDC_COMBO_SRAM_KEY_REPEAT_RATE, CMsg::Key_Repeat_Rate, LABELS::key_repeat_rate, vali, 4);
	CreateStatic(hbox, IDC_STATIC, CMsg::msec);

	// ----------------------------------------
	// 2:Screen
	// ----------------------------------------
	CBox *box_2all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("2all"));
	tab_items.SetCurrentPosition(2);

	CBox *box_2h = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("2h"));

	CBox *box_2lv = box_2h->AddBox(CBox::VerticalBox, 0, 0, _T("2lv"));

	// DirectX
	CBox *box_d3d = CreateGroup(box_2lv, IDC_STATIC, CMsg::Drawing, CBox::VerticalBox);
	CBox *box_d3d_h = box_d3d->AddBox(CBox::HorizontalBox, 0, 0, _T("d3d_h"));

	vbox = box_d3d_h->AddBox(CBox::VerticalBox, 0, 0, _T("d3d_tit"));
#ifdef USE_DIRECT3D
	CreateStatic(vbox, IDC_STATIC, CMsg::Method);
	CreateStatic(vbox, IDC_STATIC, CMsg::Filter_Type);
#endif
#ifdef USE_OPENGL
	CreateStatic(vbox, IDC_STATIC, CMsg::Method_ASTERISK);
	CreateStatic(vbox, IDC_STATIC, CMsg::Filter_Type);
#endif

	vbox = box_d3d_h->AddBox(CBox::VerticalBox, 0, 0, _T("d3d_com"));
#ifdef USE_DIRECT3D
	// d3d use
	CreateComboBox(vbox, IDC_COMBO_D3D_USE, LABELS::d3d_use, pConfig->use_direct3d, 6);
	// d3d filter type
	CreateComboBox(vbox, IDC_COMBO_D3D_FILTER, LABELS::d3d_filter, pConfig->d3d_filter_type, 6);
#endif
#ifdef USE_OPENGL
	// opengl use
	CreateComboBox(vbox, IDC_COMBO_OPENGL_USE, LABELS::opengl_use, pConfig->use_opengl, 6);
	// opengl filter type
	CreateComboBox(vbox, IDC_COMBO_OPENGL_FILTER, LABELS::opengl_filter, pConfig->gl_filter_type, 6);
#endif

	// border color
	CreateCheckBox(box_d3d, IDC_CHK_BORDER_COLOR, CMsg::Set_gray_color_on_the_border_area, FLG_ORIG_BORDER_COLOR != 0);

	// crtc
	CBox *box_2rv = box_2h->AddBox(CBox::VerticalBox, 0, 0, _T("1rv"));
//	(box_2rv);

	CBox *box_crtc = CreateGroup(box_2rv, IDC_STATIC, CMsg::CRTC, CBox::HorizontalBox);

	CBox *box_crtc_tit = box_crtc->AddBox(CBox::VerticalBox, 0, 0, _T("crtc_tit"));

	hbox = box_crtc_tit->AddBox(CBox::HorizontalBox, 0, 0, _T("crtc_ras"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Raster_Interrupt_Skew);
	hCtrl = CreateEditBox(hbox, IDC_EDIT_RASTER_INT, 0, 6);
	CreateUpDown(hbox, IDC_SPIN_RASTER_INT, hCtrl, -128, 128, pConfig->raster_int_skew);

	hbox = box_crtc_tit->AddBox(CBox::HorizontalBox, 0, 0, _T("crtc_vdis"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Vertical_Disp_Skew);
	hCtrl = CreateEditBox(hbox, IDC_EDIT_VDISP, 0, 6);
	CreateUpDown(hbox, IDC_SPIN_VDISP, hCtrl, -128, 128, pConfig->vdisp_skew);

	// LED
#ifdef USE_LEDBOX
	int led_show = gui->GetLedBoxPhase(-1);
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("led"));
	// led show
	CreateComboBoxWithLabel(hbox, IDC_COMBO_LED_SHOW, CMsg::LED, LABELS::led_show, led_show, 8);
	// led pos
	CreateComboBoxWithLabel(hbox, IDC_COMBO_LED_POS, CMsg::Position, LABELS::led_pos, pConfig->led_pos, 8);
#endif

	// Capture Type
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("capture"));
	CreateComboBoxWithLabel(hbox, IDC_COMBO_CAPTURE_TYPE, CMsg::Capture_Type, LABELS::capture_fmt, pConfig->capture_type, 8);

	//
	// snapshot path
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("snapshot"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Snapshot_Path, 100);
	CreateEditBox(hbox, IDC_SNAP_PATH, pConfig->snapshot_path.GetM(), 30);
	CreateButton(hbox, IDC_BTN_SNAP_PATH, CMsg::Folder_, 5);

	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontfile"));
#if defined(USE_WIN)
	// font file
	CreateStatic(hbox, IDC_STATIC, CMsg::Font_File_ASTERISK, 100);
	CreateEditBox(hbox, IDC_FONT_FILE, pConfig->font_path.GetM(), 30);
	CreateButton(hbox, IDC_BTN_FONT_FILE, CMsg::File_, 5);
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
	// font path
	CreateStatic(hbox, IDC_STATIC, CMsg::Font_Path, 100);
	CreateEditBox(hbox, IDC_FONT_FILE, pConfig->font_path.GetM(), 30);
	CreateButton(hbox, IDC_BTN_FONT_FILE, CMsg::Folder_, 5);
#endif

	// message font
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontns"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Message_Font, 100);
	CreateEditBox(hbox, IDC_MSG_FONT_NAME_S, pConfig->msgboard_msg_fontname.GetM(), 10);
	CreateEditBoxWithLabel(hbox, IDC_MSG_FONT_SIZE_S, CMsg::_Size, pConfig->msgboard_msg_fontsize, 3);
	CreateButton(hbox, IDC_BTN_FONT_NAME_S, CMsg::Font_, 5);

	// info font
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("fontnl"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Info_Font, 100);
	CreateEditBox(hbox, IDC_MSG_FONT_NAME_L, pConfig->msgboard_info_fontname.GetM(), 10);
	CreateEditBoxWithLabel(hbox, IDC_MSG_FONT_SIZE_L, CMsg::_Size, pConfig->msgboard_info_fontsize, 3);
	CreateButton(hbox, IDC_BTN_FONT_NAME_L, CMsg::Font_, 5);

	// language
	hbox = box_2all->AddBox(CBox::HorizontalBox, 0, 0, _T("lang"));
	lang_list.Clear();
	clocale->GetLocaleNamesWithDefault(lang_list);
	int lang_selidx = clocale->SelectLocaleNameIndex(lang_list, pConfig->language);
	CreateComboBoxWithLabel(hbox, IDC_COMBO_LANGUAGE, CMsg::Language_ASTERISK, lang_list, lang_selidx, 8);

	// text
	CreateStatic(box_2all, IDC_STATIC, CMsg::Need_restart_program);

	// ----------------------------------------
	// 3:Tape, FDD
	// ----------------------------------------
	CBox *box_3all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("3all"));
	tab_items.SetCurrentPosition(3);

#ifdef USE_DATAREC
	CBox *box_3h = box_3all->AddBox(CBox::HorizontalBox, 0, 0, _T("3h"));

	CBox *box_3lv = box_3h->AddBox(CBox::VerticalBox, 0, 0, _T("3lv"));

	// Load wav file
	CBox *box_ldwav = CreateGroup(box_3lv, IDC_STATIC, CMsg::Load_Wav_File_from_Tape, CBox::VerticalBox);

	hbox = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav0"));
	CreateCheckBox(hbox, IDC_CHK_REVERSE, CMsg::Reverse_Wave, pConfig->wav_reverse);
	CreateCheckBox(hbox, IDC_CHK_HALFWAVE, CMsg::Half_Wave, pConfig->wav_half);
	hbox = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav1"));
	CreateStatic(hbox, IDC_STATIC, CMsg::Correct);
	for(int i=0; LABELS::correct[i] != CMsg::End; i++) {
		CreateRadioButton(hbox, IDC_RADIO_NOCORR + i, LABELS::correct[i], (i == 0));
	}
	hbox = box_ldwav->AddBox(CBox::HorizontalBox, 0, 0, _T("ldwav2"));
	for(int i=0; i<2; i++) {
		CreateStatic(hbox, IDC_STATIC_CORRAMP0 + i, LABELS::correct_amp[i]);
		CreateEditBox(hbox, IDC_TXT_CORRAMP0 + i, pConfig->wav_correct_amp[i], 4, WS_EX_RIGHT);
	}
	CheckDlgButton(hDlg, IDC_RADIO_NOCORR, !pConfig->wav_correct);
	CheckDlgButton(hDlg, IDC_RADIO_COSW, pConfig->wav_correct && pConfig->wav_correct_type == 0);
	CheckDlgButton(hDlg, IDC_RADIO_SINW, pConfig->wav_correct && pConfig->wav_correct_type == 1);

	// Save wav file
	CBox *box_3rv = box_3h->AddBox(CBox::VerticalBox, 0, 0, _T("2rv"));

	CBox *box_svwav = CreateGroup(box_3rv, IDC_STATIC, CMsg::Save_Wav_File_to_Tape, CBox::HorizontalBox);

	vbox = box_svwav->AddBox(CBox::VerticalBox, 0, 0, _T("svwav_t"));
	CreateStatic(vbox, IDC_STATIC, CMsg::Sample_Rate);
	CreateStatic(vbox, IDC_STATIC, CMsg::Sample_Bits);

	vbox = box_svwav->AddBox(CBox::VerticalBox, 0, 0, _T("svwav_c"));
	CreateComboBox(vbox, IDC_COMBO_SRATE, LABELS::sound_rate, pConfig->wav_sample_rate, 4);
	CreateComboBox(vbox, IDC_COMBO_SBITS, LABELS::sound_bits, pConfig->wav_sample_bits, 4);
#endif

	// FDD
#ifdef USE_FD1
	CBox *box_fdd = CreateGroup(box_3all, IDC_STATIC, CMsg::Floppy_Disk_Drive, CBox::VerticalBox);

	// mount fdd
	hbox = box_fdd->AddBox(CBox::HorizontalBox, 0, 0, _T("fdd_mount"));
	CreateStatic(hbox, IDC_STATIC, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<MAX_DRIVE; i++) {
		UTILITY::stprintf(str, sizeof(str), _T("%d"), i);
		CreateCheckBox(hbox, IDC_CHK_FDD_MOUNT0 + i, str, (pConfig->mount_fdd & (1 << i)) != 0);
	}

	CreateCheckBox(box_fdd, IDC_CHK_DELAYFD1, CMsg::Ignore_delays_to_find_sector, FLG_DELAY_FDSEARCH != 0);
	CreateCheckBox(box_fdd, IDC_CHK_DELAYFD2, CMsg::Ignore_delays_to_seek_track, FLG_DELAY_FDSEEK != 0);
	CreateCheckBox(box_fdd, IDC_CHK_FDDENSITY, CMsg::Suppress_checking_for_density, FLG_CHECK_FDDENSITY == 0);
	CreateCheckBox(box_fdd, IDC_CHK_FDMEDIA, CMsg::Suppress_checking_for_media_type, FLG_CHECK_FDMEDIA == 0);
	CreateCheckBox(box_fdd, IDC_CHK_SAVE_FDPLAIN, CMsg::Save_a_plain_disk_image_as_it_is, FLG_SAVE_FDPLAIN != 0);
#endif

	// HDD
#ifdef USE_HD1
	CBox *box_hdd = CreateGroup(box_3all, IDC_STATIC, CMsg::Hard_Disk_Drive, CBox::VerticalBox);

	// mount hdd
	hbox = box_hdd->AddBox(CBox::HorizontalBox, 0, 0, _T("hdd_mount"));
	CreateStatic(hbox, IDC_STATIC, CMsg::When_start_up_mount_disk_at_);
	for(int i=0; i<MAX_HARD_DISKS; i++) {
		UTILITY::stprintf(str, sizeof(str), _T("%d"), i);
		CreateCheckBox(hbox, IDC_CHK_HDD_MOUNT0 + i, str, (pConfig->mount_hdd & (1 << i)) != 0);
	}

	CreateCheckBox(box_hdd, IDC_CHK_DELAYHD2, CMsg::Ignore_delays_to_seek_track, FLG_DELAY_HDSEEK != 0);
#endif

	// ----------------------------------------
	// 4:Network
	// ----------------------------------------
	CBox *box_4all = box_tab->AddBox(CBox::VerticalBox, 0, 0, _T("4all"));
	tab_items.SetCurrentPosition(4);

#ifdef MAX_PRINTER
	// LPT
	for(int i=0; i<MAX_PRINTER; i++) {
		hbox = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("hh"));
		UTILITY::stprintf(str, sizeof(str), CMSGM(Printer_Hostname), i); 
		CreateStatic(hbox, IDC_STATIC, str, 100);
		CreateEditBox(hbox, IDC_HOSTNAME_LPT0 + i, pConfig->printer_server_host[i], 12, WS_EX_LEFT);
		CreateEditBoxWithLabel(hbox, IDC_PORT_LPT0 + i, CMsg::_Port, pConfig->printer_server_port[i], 6, WS_EX_RIGHT);
		UTILITY::stprintf(str, sizeof(str), _T("%.1f"), pConfig->printer_delay[i]);
		CreateEditBoxWithLabel(hbox, IDC_DELAY_LPT0 + i, CMsg::_Print_delay, str, 6, WS_EX_RIGHT);
		CreateStatic(hbox, IDC_STATIC, CMsg::msec);
	}
#endif
#ifdef MAX_COMM
	// COM
	for(int i=0; i<MAX_COMM; i++) {
		hbox = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("hh"));
		UTILITY::stprintf(str, sizeof(str), CMSGM(Communication_Hostname), i); 
		CreateStatic(hbox, IDC_STATIC, str, 100);
		CreateEditBox(hbox, IDC_HOSTNAME_COM0 + i, pConfig->comm_server_host[i], 12, WS_EX_LEFT);
		CreateEditBoxWithLabel(hbox, IDC_PORT_COM0 + i, CMsg::_Port, pConfig->comm_server_port[i], 6, WS_EX_RIGHT);
//		CreateComboBox(hbox, IDC_COMBO_COM0 + i, LABELS::comm_baud, pConfig->comm_dipswitch[i] - 1, 8);
	}
#endif
#ifdef USE_DEBUGGER
	// Debugger
	hbox = box_4all->AddBox(CBox::HorizontalBox, 0, 0, _T("dbg"));
	CreateEditBoxWithLabel(hbox, IDC_HOSTNAME_DBGR, CMsg::Connectable_host_to_Debugger, pConfig->debugger_server_host, 12, WS_EX_LEFT);
	CreateEditBoxWithLabel(hbox, IDC_PORT_DBGR, CMsg::_Port, pConfig->debugger_server_port, 6, WS_EX_RIGHT);
#endif
	// uart
//	int uart_w = font->GetTextWidth(hDlg, _T("mmmmmmmmmmmm"));
	int uart_w = 100;
	CBox *box_uart = CreateGroup(box_4all, IDC_STATIC, CMsg::Settings_of_serial_ports_on_host, CBox::VerticalBox);
	CBox *box_uah0 = box_uart->AddBox(CBox::HorizontalBox, 0, 0, _T("uart0"));
	vbox = box_uah0->AddBox(CBox::VerticalBox, 0, 0, _T("uart1"));
	CreateStatic(vbox, IDC_STATIC, CMsg::Baud_Rate, uart_w);
	CreateStatic(vbox, IDC_STATIC, CMsg::Data_Bit, uart_w);
	CreateStatic(vbox, IDC_STATIC, CMsg::Parity, uart_w);
	CreateStatic(vbox, IDC_STATIC, CMsg::Stop_Bit, uart_w);
	CreateStatic(vbox, IDC_STATIC, CMsg::Flow_Control, uart_w);
	vbox = box_uah0->AddBox(CBox::VerticalBox, 0, 0, _T("uart2"));
	// uart
	vali = 0;
	for(int i=0; LABELS::comm_uart_baudrate[i] != NULL; i++) {
		if (pConfig->comm_uart_baudrate == _tcstol(LABELS::comm_uart_baudrate[i], NULL, 10)) {
			vali = i;
			break;
		}
	}
	CreateComboBox(vbox, IDC_COMBO_UART_BAUDRATE, LABELS::comm_uart_baudrate, vali, 6);
	vali = pConfig->comm_uart_databit - 7; // 7bit: 0 8bit: 1
	if (vali < 0 || 1 < vali) vali = 1;
	CreateComboBox(vbox, IDC_COMBO_UART_DATABIT, LABELS::comm_uart_databit, vali, 6);
	CreateComboBox(vbox, IDC_COMBO_UART_PARITY, LABELS::comm_uart_parity, pConfig->comm_uart_parity, 6);
	CreateComboBox(vbox, IDC_COMBO_UART_STOPBIT, LABELS::comm_uart_stopbit, pConfig->comm_uart_stopbit - 1, 6);
	CreateComboBox(vbox, IDC_COMBO_UART_FLOWCTRL, LABELS::comm_uart_flowctrl, pConfig->comm_uart_flowctrl, 6);
	CreateStatic(box_uart, IDC_STATIC, CMsg::Need_re_connect_to_serial_port_when_modified_this);

	// ----------------------------------------
	// button
	CBox *box_btn = box_all->AddBox(CBox::HorizontalBox, CBox::RightPos);
	CreateButton(box_btn, IDOK, CMsg::OK, 8, true);
	CreateButton(box_btn, IDCANCEL, CMsg::Cancel, 8);

	RECT prc;
	GetClientRect(hTabCtrl, &prc);
	TabCtrl_AdjustRect(hTabCtrl, FALSE, &prc);
	box_tab->SetTopMargin(prc.top + 8);

	box_all->Realize(*this);

	select_tabctrl(selected_tabctrl);

	delete box_all;

	return (INT_PTR)TRUE;
}

INT_PTR ConfigBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_IOPORT_FDD
	HWND hCtrl;
#endif
	WORD wId = LOWORD(wParam);

	if (wId == IDOK) {
		onOK(message, wParam, lParam);
	}
#if 0
	else if (wId == IDC_RADIO_NOFDD) {
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, true);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, true);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_3FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_5FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5_8FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_5_8FDD) {
		CheckDlgButton(hDlg, IDC_RADIO_NOFDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_3FDD, 0);
		CheckDlgButton(hDlg, IDC_RADIO_5FDD, 0);
#ifdef USE_IOPORT_FDD
		// I/O port
		CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
		EnableWindow(hCtrl, false);
		CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
		EnableWindow(hCtrl, false);
#endif
		return (INT_PTR)FALSE;
	}
#endif
	else if (wId == IDC_RADIO_NOCORR)
	{
		CheckDlgButton(hDlg, IDC_RADIO_COSW, 0);
		CheckDlgButton(hDlg, IDC_RADIO_SINW, 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_COSW)
	{
		CheckDlgButton(hDlg, IDC_RADIO_NOCORR, 0);
		CheckDlgButton(hDlg, IDC_RADIO_SINW, 0);
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_RADIO_SINW)
	{
		CheckDlgButton(hDlg, IDC_RADIO_NOCORR, 0);
		CheckDlgButton(hDlg, IDC_RADIO_COSW, 0);
		return (INT_PTR)FALSE;
	}
//	else if (wId == IDC_CHK_CORRECT)
//	{
//		if (IsDlgButtonChecked(hDlg, IDC_CHK_CORRECT)) {
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_COSW);
//			EnableWindow(hCtrl, true);
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_SINW);
//			EnableWindow(hCtrl, true);
//		} else {
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_COSW);
//			EnableWindow(hCtrl, false);
//			hCtrl = GetDlgItem(hDlg, IDC_RADIO_SINW);
//			EnableWindow(hCtrl, false);
//		}
//		return (INT_PTR)FALSE;
//	}
#if 0
	else if (wId == IDC_CHK_IOPORT1)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT1)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT2)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT2)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT6)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT6)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT7, 0);
#if defined(_MBS1)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT10, 0);
#endif
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT7)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT7)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT6, 0);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_CHK_IOPORT10)
	{
		if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT10)) {
			CheckDlgButton(hDlg, IDC_CHK_IOPORT6, 0);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT7, 1);
		}
		return (INT_PTR)FALSE;
	}
#endif
	else if (wId == IDC_BTN_SNAP_PATH)
	{
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_SNAP_PATH, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_folder_to_save_snapshot_images), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_SNAP_PATH, buf);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_FILE)
	{
#ifdef USE_WIN
		const CMsg::Id filter[] =
			{ CMsg::Supported_Files_ttf_otf, CMsg::All_Files_, CMsg::End };
		_TCHAR buf[_MAX_PATH];
		FileBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
		if (fbox.Show(
			filter,
			CMSGM(Select_a_font_file_for_showing_messages),
			NULL,
			_T("ttf"),
			false,
			buf)) {
			SetDlgItemText(hDlg, IDC_FONT_FILE, buf);
		}
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_font_folder_for_showing_messages), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_FONT_FILE, buf);
		}
#endif
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_NAME_L)
	{
		_TCHAR buf[_MAX_PATH];
		double font_size;
		BOOL rc;
		FontBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf, _MAX_PATH);
		font_size = (double)GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, &rc, TRUE);
		if (fbox.Show(
			CMSGM(Select_a_font),
			NULL, buf, _MAX_PATH, &font_size)) {
			SetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf);
			SetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, (UINT)font_size, TRUE);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_FONT_NAME_S)
	{
		_TCHAR buf[_MAX_PATH];
		double font_size;
		BOOL rc;
		FontBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf, _MAX_PATH);
		font_size = (double)GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, &rc, TRUE);
		if (fbox.Show(
			CMSGM(Select_a_font),
			NULL, buf, _MAX_PATH, &font_size)) {
			SetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf);
			SetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, (UINT)font_size, TRUE);
		}
		return (INT_PTR)FALSE;
	}
	else if (wId == IDC_BTN_ROM_PATH)
	{
		_TCHAR buf[_MAX_PATH];
		FolderBox fbox(hDlg);
		GetDlgItemText(hDlg, IDC_ROM_PATH, buf, _MAX_PATH);
		if (buf[0] == _T('\0')) {
			const _TCHAR *tbuf = emu->application_path();
#if defined(USE_UTF8_ON_MBCS)
			UTILITY::conv_to_native_path(tbuf, buf, _MAX_PATH);
#else
			UTILITY::tcscpy(buf, _MAX_PATH, tbuf);
#endif
		}
		if (fbox.Show(CMSGM(Select_a_folder_containing_the_rom_images), buf, _MAX_PATH)) {
			SetDlgItemText(hDlg, IDC_ROM_PATH, buf);
		}
		return (INT_PTR)FALSE;
	}

	if (wId == IDOK || wId == IDCANCEL) {
		::EndDialog(hDlg, wId);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR ConfigBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	// change tab
	LPNMHDR lpNmHdr = (NMHDR *)lParam;
	int i;
	switch(lpNmHdr->idFrom) {
	case IDC_TAB1:
		switch (lpNmHdr->code) {
		case TCN_SELCHANGE:
			i = TabCtrl_GetCurSel(lpNmHdr->hwndFrom);
			select_tabctrl(i);
			break;
		}
		break;
	default:
		break;
	}
	return (INT_PTR)TRUE;
}

INT_PTR ConfigBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	_TCHAR buf[_MAX_PATH];
	int valuel;
	BOOL rc;

	// power off
	pConfig->use_power_off = (IsDlgButtonChecked(hDlg, IDC_CHK_POWEROFF) == BST_CHECKED);


#ifdef USE_FD1
	pConfig->mount_fdd = 0;
	for (int i=0; i<MAX_DRIVE; i++) {
		pConfig->mount_fdd |= (IsDlgButtonChecked(hDlg, IDC_CHK_FDD_MOUNT0 + i) == BST_CHECKED ? 1 << i : 0);
	}
	pConfig->option_fdd = (IsDlgButtonChecked(hDlg, IDC_CHK_DELAYFD1) == BST_CHECKED ? MSK_DELAY_FDSEARCH : 0)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_DELAYFD2) == BST_CHECKED ? MSK_DELAY_FDSEEK : 0)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_FDDENSITY) == BST_CHECKED ? 0 : MSK_CHECK_FDDENSITY)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_FDMEDIA) == BST_CHECKED ? 0 : MSK_CHECK_FDMEDIA)
					| (IsDlgButtonChecked(hDlg, IDC_CHK_SAVE_FDPLAIN) == BST_CHECKED ? MSK_SAVE_FDPLAIN : 0);
#endif

#ifdef USE_HD1
	pConfig->mount_hdd = 0;
	for (int i=0; i<MAX_HARD_DISKS; i++) {
		pConfig->mount_hdd |= (IsDlgButtonChecked(hDlg, IDC_CHK_HDD_MOUNT0 + i) == BST_CHECKED ? 1 << i : 0);
	}
	pConfig->option_hdd = (IsDlgButtonChecked(hDlg, IDC_CHK_DELAYHD2) == BST_CHECKED ? MSK_DELAY_HDSEEK : 0);
#endif

#ifdef USE_DATAREC
	pConfig->wav_reverse = (IsDlgButtonChecked(hDlg, IDC_CHK_REVERSE) == BST_CHECKED);
	pConfig->wav_half = (IsDlgButtonChecked(hDlg, IDC_CHK_HALFWAVE) == BST_CHECKED);
	pConfig->wav_correct = !(IsDlgButtonChecked(hDlg, IDC_RADIO_NOCORR) == BST_CHECKED);
	pConfig->wav_correct_type = (IsDlgButtonChecked(hDlg, IDC_RADIO_SINW) == BST_CHECKED ? 1 : 0);
	valuel = GetDlgItemInt(hDlg, IDC_TXT_CORRAMP0, &rc, false);
	if (rc == TRUE && 100 <= valuel && valuel <= 5000) {
		pConfig->wav_correct_amp[0] = valuel;
	}
	valuel = GetDlgItemInt(hDlg, IDC_TXT_CORRAMP1, &rc, false);
	if (rc == TRUE && 100 <= valuel && valuel <= 5000) {
		pConfig->wav_correct_amp[1] = valuel;
	}

	pConfig->wav_sample_rate = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_SRATE, CB_GETCURSEL, 0, 0);
	pConfig->wav_sample_bits = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_SBITS, CB_GETCURSEL, 0, 0);
#endif

	// I/O port address
	for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
		int pos = LABELS::io_port_pos[i];
		if ((1 << pos) & IOPORT_MSK_ALL) {
			if (IsDlgButtonChecked(hDlg, IDC_CHK_IOPORT1 + pos) != BST_UNCHECKED)  {
				io_port |= (1 << pos);
			} else {
				io_port &= ~(1 << pos);
			}
		}
	}
	// crtc
	valuel = GetDlgItemInt(hDlg, IDC_EDIT_RASTER_INT, &rc, true);
	if (rc == TRUE && -128 <= valuel && valuel <= 128) {
		pConfig->raster_int_skew = valuel;
	}
	valuel = GetDlgItemInt(hDlg, IDC_EDIT_VDISP, &rc, true);
	if (rc == TRUE && -128 <= valuel && valuel <= 128) {
		pConfig->vdisp_skew = valuel;
	}

	// border color
	BIT_ONOFF(pConfig->original, MSK_ORIG_BORDER_COLOR, IsDlgButtonChecked(hDlg, IDC_CHK_BORDER_COLOR) == BST_CHECKED);

#ifdef USE_DIRECT3D
	// d3d use
	pConfig->use_direct3d = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_D3D_USE, CB_GETCURSEL, 0, 0);

	// d3d filter type
	pConfig->d3d_filter_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_D3D_FILTER, CB_GETCURSEL, 0, 0);
#endif
#ifdef USE_OPENGL
	// opengl use
	pConfig->use_opengl = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_OPENGL_USE, CB_GETCURSEL, 0, 0);

	// opengl filter type
	pConfig->gl_filter_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_OPENGL_FILTER, CB_GETCURSEL, 0, 0);
#endif

#ifdef USE_LEDBOX
	// led show
	int led_show = (int)SendDlgItemMessage(hDlg, IDC_COMBO_LED_SHOW, CB_GETCURSEL, 0, 0);
	// led pos
	pConfig->led_pos = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_LED_POS, CB_GETCURSEL, 0, 0);
#endif

	// capture type
	pConfig->capture_type = (uint8_t)SendDlgItemMessage(hDlg, IDC_COMBO_CAPTURE_TYPE, CB_GETCURSEL, 0, 0);

	// snapshot path
	GetDlgItemText(hDlg, IDC_SNAP_PATH, buf, _MAX_PATH);
	pConfig->snapshot_path.SetM(buf);
	// font file / path
	GetDlgItemText(hDlg, IDC_FONT_FILE, buf, _MAX_PATH);
	pConfig->font_path.SetM(buf);
	// message font name
	GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_S, buf, _MAX_PATH);
	pConfig->msgboard_msg_fontname.SetM(buf);
	GetDlgItemText(hDlg, IDC_MSG_FONT_NAME_L, buf, _MAX_PATH);
	pConfig->msgboard_info_fontname.SetM(buf);
	// message font size
	valuel = GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_L, &rc, TRUE);
	if (rc == TRUE && 1 <= valuel && valuel <= 60) {
		pConfig->msgboard_info_fontsize = (uint8_t)valuel;
	}
	valuel = GetDlgItemInt(hDlg, IDC_MSG_FONT_SIZE_S, &rc, TRUE);
	if (rc == TRUE && 1 <= valuel && valuel <= 60) {
		pConfig->msgboard_msg_fontsize = (uint8_t)valuel;
	}

	// language
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_LANGUAGE, CB_GETCURSEL, 0, 0);
	clocale->ChooseLocaleName(lang_list, valuel, pConfig->language);

#ifdef MAX_PRINTER
	// hostname, port
	for(int i=0; i<MAX_PRINTER; i++) {
		GetDlgItemText(hDlg, IDC_HOSTNAME_LPT0 + i, buf, _MAX_PATH);
		pConfig->printer_server_host[i].Set(buf);
		valuel = GetDlgItemInt(hDlg, IDC_PORT_LPT0 + i, &rc, TRUE);
		if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
			pConfig->printer_server_port[i] = valuel;
		}
		GetDlgItemText(hDlg, IDC_DELAY_LPT0 + i, buf, _MAX_PATH);
		double valued = _tcstod(buf, NULL);
		if (valued < 0.1) valued = 0.1;
		if (valued > 1000.0) valued = 1000.0;
		valued = floor(valued * 10.0 + 0.5) / 10.0;
		pConfig->printer_delay[i] = valued;
	}
#endif
#ifdef MAX_COMM
	for(int i=0; i<MAX_COMM; i++) {
		GetDlgItemText(hDlg, IDC_HOSTNAME_COM0 + i, buf, _MAX_PATH);
		pConfig->comm_server_host[i].Set(buf);
		valuel = GetDlgItemInt(hDlg, IDC_PORT_COM0 + i, &rc, TRUE);
		if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
			pConfig->comm_server_port[i] = valuel;
		}
//		valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_COM0 + i, CB_GETCURSEL, 0, 0);
//		if (valuel >= 0 && valuel <= 3) {
//			pConfig->comm_dipswitch[i] = valuel + 1;
//		}
	}
#endif
#ifdef USE_DEBUGGER
	GetDlgItemText(hDlg, IDC_HOSTNAME_DBGR, buf, _MAX_PATH);
	pConfig->debugger_server_host.Set(buf);
	valuel = GetDlgItemInt(hDlg, IDC_PORT_DBGR, &rc, TRUE);
	if (rc == TRUE && 0 <= valuel && valuel <= 65535) {
		pConfig->debugger_server_port = valuel;
	}
#endif
	// uart
	valuel = GetDlgItemInt(hDlg, IDC_COMBO_UART_BAUDRATE, &rc, FALSE);
	if (rc == TRUE && valuel > 0) pConfig->comm_uart_baudrate = valuel;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_DATABIT, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 1) pConfig->comm_uart_databit = valuel + 7; // 7bit: 0 8bit: 1
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_PARITY, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 4) pConfig->comm_uart_parity = valuel;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_STOPBIT, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 1) pConfig->comm_uart_stopbit = valuel + 1;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_UART_FLOWCTRL, CB_GETCURSEL, 0, 0);
	if (valuel >= 0 && valuel <= 2) pConfig->comm_uart_flowctrl = valuel;

	// rom path
	GetDlgItemText(hDlg, IDC_ROM_PATH, buf, _MAX_PATH);
	pConfig->rom_path.SetM(buf);

	// undef opcode
	BIT_ONOFF(pConfig->misc_flags, MSK_SHOWMSG_ADDRERR, IsDlgButtonChecked(hDlg, IDC_CHK_ADDRERR) == BST_CHECKED);

#if defined(_MBS1)
	// fmopn clock
	pConfig->opn_clock = (int)SendDlgItemMessage(hDlg, IDC_COMBO_FMOPN, CB_GETCURSEL, 0, 0);
	// fmopn irq
	pConfig->opn_irq = (int)SendDlgItemMessage(hDlg, IDC_COMBO_FMOPN_IRQ, CB_GETCURSEL, 0, 0);

	// use opn instead of psg
	pConfig->use_opn_expsg = (IsDlgButtonChecked(hDlg, IDC_CHK_USE_OPN_EXPSG) == BST_CHECKED);

	emu->set_parami(VM::ParamSysMode, sys_mode);
#endif

//	emu->set_parami(VM::ParamFddType, fdd_type);
	emu->set_parami(VM::ParamIOPort, io_port);

#if defined(_X68000)
	// exram size
	emu->set_parami(VM::ParamMainRamSizeNum, (int)SendDlgItemMessage(hDlg, IDC_COMBO_EXMEM, CB_GETCURSEL, 0, 0));
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_RAM_SIZE, IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_RAM_SIZE) == BST_CHECKED);

	// SRAM
	VM *vm = emu->get_vm();
	_TCHAR *endptr = NULL;
	long vall = 0;

	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CLR_PWRON, IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_CLR_PWR_ON) == BST_CHECKED);
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_SAVE_PWROFF, IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_SAVE_PWR_OFF) == BST_CHECKED);
	BIT_ONOFF(pConfig->original, MSK_ORIG_SRAM_CHG_BOOT_DEV, IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_CHG_BOOT_DEVICE) == BST_CHECKED);

	// ram size
//	GetDlgItemText(hDlg, IDC_EDIT_SRAM_RAM_SIZE, buf, _MAX_PATH); 
//	vm->set_sram_ram_size(buf);

	// rom start address
	GetDlgItemText(hDlg, IDC_EDIT_SRAM_ROM_ADDRESS, buf, _MAX_PATH);
	endptr = NULL;
	vall = _tcstol(buf, &endptr, 16);
	if (endptr && *endptr == _T('\0')) {
		vm->set_sram_rom_start_address(vall);
	}

	// sram start address
	GetDlgItemText(hDlg, IDC_EDIT_SRAM_SRAM_ADDRESS, buf, _MAX_PATH);
	endptr = NULL;
	vall = _tcstol(buf, &endptr, 16);
	if (endptr && *endptr == _T('\0')) {
		vm->set_sram_sram_start_address(vall);
	}

	// boot device
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_BOOT_DEVICE, CB_GETCURSEL, 0, 0);
	vm->set_sram_boot_device(valuel);

	// RS-232C
	uint32_t valueu = 0;
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_RS232C_BAUDRATE, CB_GETCURSEL, 0, 0);
	valueu |= vm->conv_sram_rs232c_baud_rate(valuel);
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_RS232C_DATABIT, CB_GETCURSEL, 0, 0);
	valueu |= vm->conv_sram_rs232c_databit(valuel);
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_RS232C_PARITY, CB_GETCURSEL, 0, 0);
	valueu |= vm->conv_sram_rs232c_parity(valuel);
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_RS232C_STOPBIT, CB_GETCURSEL, 0, 0);
	valueu |= vm->conv_sram_rs232c_stopbit(valuel);
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_RS232C_FLOWCTRL, CB_GETCURSEL, 0, 0);
	valueu |= vm->conv_sram_rs232c_flowctrl(valuel);
	vm->set_sram_rs232c(valueu);

	// alarm
//	(IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_ALARM_ONOFF) == BST_CHECKED);
//	vm->set_sram_alarm_onoff();

	// alarm time
//	GetDlgItemText(hDlg, IDC_COMBO_SRAM_ALARM_TIME, buf, _MAX_PATH);

	// contrast
	valuel = (int)GetDlgItemInt(hDlg, IDC_EDIT_SRAM_CONTRAST, &rc, FALSE);
	if (rc == TRUE && valuel >= 0 && valuel <= 15) {
		vm->set_sram_contrast(valuel);
	}

	// eject fd
	valuel = IsDlgButtonChecked(hDlg, IDC_CHK_SRAM_EJECT_FD) == BST_CHECKED ? 1 : 0;
	vm->set_sram_fd_eject(valuel);

	// purpose of SRAM free area
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_PURPOSE, CB_GETCURSEL, 0, 0);
	vm->set_sram_purpose(valuel);

	// key repeat
	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_KEY_REPEAT_DELAY, CB_GETCURSEL, 0, 0);
	vm->set_sram_key_repeat_delay(valuel);

	valuel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_SRAM_KEY_REPEAT_RATE, CB_GETCURSEL, 0, 0);
	vm->set_sram_key_repeat_rate(valuel);

#endif // _X68000

	// message font
#ifdef USE_MESSAGE_BOARD
	MsgBoard *msgboard = emu->get_msgboard();
	if (msgboard) {
		msgboard->SetFont();
	}
#endif

#ifdef USE_LEDBOX
	gui->ChangeLedBox(led_show);
	gui->ChangeLedBoxPosition(pConfig->led_pos);
#endif

	pConfig->save();
#ifdef USE_OPENGL
	emu->change_opengl_attr();
#endif
	emu->update_config();
#if defined(_X68000)
	vm->save_sram_file();
#endif

	return (INT_PTR)TRUE;
}

#if 0
INT_PTR ConfigBox::onControlColorStatic(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE h = (HANDLE)GetStockObject(NULL_BRUSH);
	SetBkMode((HDC)wParam, TRANSPARENT);
	return (INT_PTR)h;
}
#endif

void ConfigBox::select_tabctrl(int tab_num)
{
	HWND hCtrl;
	int cmdShow;

	selected_tabctrl = tab_num;
	for(int i=0; i<tab_items.Count(); i++) {
		cmdShow = (selected_tabctrl == i ? SW_SHOW : SW_HIDE);

		CTabItemIds *ids = tab_items.Item(i);
		if (ids) {
			for(int j=0; j<(int)ids->size(); j++) {
				hCtrl = GetDlgItem(hDlg, ids->at(j));
				ShowWindow(hCtrl, cmdShow);
			}
		}
	}

#if 0
	switch(selected_tabctrl) {
	case 0:
		// Mode
		// Mode switch
		hCtrl = GetDlgItem(hDlg, IDC_STATIC_MODE_SW);
		ShowWindow(hCtrl, (pConfig->dipswitch & 4) != 0 ? SW_SHOWNORMAL : SW_HIDE);

		// I/O port address
		for(int i=0; LABELS::io_port[i] != CMsg::End; i++) {
			int pos = LABELS::io_port_pos[i];
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1 + pos, (io_port & (1 << pos)) != 0);
			hCtrl = GetDlgItem(hDlg, IDC_STATIC_IOPORT1 + pos);
			ShowWindow(hCtrl, (pConfig->io_port & (1 << pos)) != 0 ? SW_SHOWNORMAL : SW_HIDE);
		}
#ifdef USE_IOPORT_FDD
		if (fdd_type == FDD_TYPE_3FDD) {
			// I/O port (3inch)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 0);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
			EnableWindow(hCtrl, false);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 1);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
			EnableWindow(hCtrl, false);
		}
		else if (fdd_type == FDD_TYPE_5FDD
#if defined(_MBS1)
				|| fdd_type == FDD_TYPE_5HFDD)
#else
				|| fdd_type == FDD_TYPE_8FDD)
#endif
		{
			// I/O port (5inch)
			CheckDlgButton(hDlg, IDC_CHK_IOPORT1, 1);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT1);
			EnableWindow(hCtrl, false);
			CheckDlgButton(hDlg, IDC_CHK_IOPORT2, 0);
			hCtrl = GetDlgItem(hDlg, IDC_CHK_IOPORT2);
			EnableWindow(hCtrl, false);
		}
#endif
		break;
	default:
		break;
	}
#endif
}

}; /* namespace GUI_WIN */
