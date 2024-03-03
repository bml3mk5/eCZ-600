/** @file labels.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2021.04.18 -

	@brief [ labels ]
*/

#include "labels.h"
#include "config.h"
#include "rec_video_defs.h"
#include "vm/vm_defs.h"

namespace LABELS {

/// Tab titles on configure dialog
const CMsg::Id tabs[] = {
	CMsg::CPU_Memory,
	CMsg::SRAM,
	CMsg::Screen,
	CMsg::FDD_HDD,
	CMsg::Network,
#if defined(_MBS1)
	CMsg::Sound,
#endif
	CMsg::End
};

/// Power state when start up
const CMsg::Id power_state[] = {
	CMsg::Inherit_the_state_when_shut_down,
	CMsg::Always_power_on,
	CMsg::Always_power_off,
	CMsg::End
};

/// I/O port list
const CMsg::Id io_port[] = { 
	CMsg::End
};

/// I/O port bit position
const uint8_t io_port_pos[] = {
	0
};

/// floppy type
const CMsg::Id fdd_type[] = {
	CMsg::End
};

/// correct wave on loading
const CMsg::Id correct[] = {
	CMsg::None_,
	CMsg::COS_Wave,
	CMsg::SIN_Wave,
	CMsg::End
};

/// correct amplify
const _TCHAR *correct_amp[] = {
	_T("1200Hz"), _T("2400Hz"), NULL
};

/// sampling rate
const _TCHAR *wav_sampling_rate[] = {
	_T("11025"), _T("22050"), _T("44100"), _T("48000"), NULL
};
/// sample bits
const _TCHAR *wav_sampling_bits[] = {
	_T("8"), _T("16"), NULL
};

#ifdef USE_DIRECT3D
/// drawing
const CMsg::Id d3d_use[] = {
	CMsg::Default, CMsg::Use_Direct3D_Sync, CMsg::Use_Direct3D_Async, CMsg::End
};
/// filter
const CMsg::Id d3d_filter[] = {
	CMsg::None_, CMsg::Point, CMsg::Linear, CMsg::End
};
#endif
#ifdef USE_OPENGL
/// drawing
const CMsg::Id opengl_use[] = {
	CMsg::Default, CMsg::Use_OpenGL_Sync, CMsg::Use_OpenGL_Async, CMsg::End
};
/// filter
const CMsg::Id opengl_filter[] = {
	CMsg::Nearest_Neighbour, CMsg::Linear, CMsg::End
};
#endif

/// show led box
const CMsg::Id led_show[] = {
	CMsg::Hide, CMsg::Show_Inside, CMsg::Show_Outside, CMsg::End
};
/// led box position
const CMsg::Id led_pos[] = {
	CMsg::LeftTop, CMsg::RightTop, CMsg::LeftBottom, CMsg::RightBottom, CMsg::End
};

/// format on capture
const _TCHAR *capture_fmt[] = {
	_T("BMP"),
#ifdef USE_CAPTURE_SCREEN_PNG
	_T("PNG"),
#endif
	NULL
};

/// sound samples
const CMsg::Id sound_samples[] = {
	CMsg::F8000Hz,
	CMsg::F11025Hz,
	CMsg::F22050Hz,
	CMsg::F44100Hz,
	CMsg::F48000Hz,
	CMsg::F96000Hz,
	CMsg::End
};

/// sound late
const CMsg::Id sound_late[] = {
	CMsg::S50msec,
	CMsg::S75msec,
	CMsg::S100msec,
	CMsg::S200msec,
	CMsg::S300msec,
	CMsg::S400msec,
	CMsg::End
};

/// baud rate
const CMsg::Id comm_baud[] = {
	CMsg::S_300baud_F_1200baud, CMsg::S_600baud_F_2400baud, CMsg::S_1200baud_F_4800baud, CMsg::S_2400baud_F_9600baud, CMsg::End
};

/// UART baud rate
const  _TCHAR *comm_uart_baudrate[] = {
	_T("110"), _T("300"), _T("600"), _T("1200"), _T("2400"), _T("4800"), _T("9600"),
	_T("19200"), _T("38400"), _T("57600"),	_T("115200"),
	_T("230400"), _T("460800"), _T("921600"),
	NULL
};

/// UART data bit
const _TCHAR *comm_uart_databit[] = {
	_T("7"), _T("8"), NULL
};

/// UART parity
const CMsg::Id comm_uart_parity[] = {
	CMsg::None_, CMsg::Odd, CMsg::Even, CMsg::End
};

/// UART stop bit
const _TCHAR *comm_uart_stopbit[] = {
	_T("1"), _T("2"), NULL
};

/// UART flow control
const CMsg::Id comm_uart_flowctrl[] = {
	CMsg::None_, CMsg::Xon_Xoff, CMsg::Hardware, CMsg::End
};

/// memory size
const CMsg::Id main_ram_size[] = {
	CMsg::S1MB, CMsg::S2MB, CMsg::S4MB, CMsg::S6MB, CMsg::S8MB, CMsg::S10MB, CMsg::End
};

/// extension of a data recorder image
const char *datarec_exts = "l3;l3b;l3c;wav;t9x";

/// extension of a floppy disk image
const char *floppy_disk_exts = "d68;d88;xdf;hdm;2hd";
const char *blank_floppy_disk_exts = "d68;d88";

/// extension of a hard disk image
const char *hard_disk_exts = "hdf";
const char *blank_hard_disk_exts = "hdf";

/// extension of a state file
const char *state_file_exts = "x6r";

/// extension of a key recording file
const char *key_rec_file_exts = "x6k";

/// extension of a auto keying file
const char *autokey_file_exts = "txt;bas;lpt";

/// extension of a printing file
const char *printing_file_exts = "lpt;txt;bas";

	
/// Volume labels
const CMsg::Id volume[] = {
	CMsg::Master,
    CMsg::FM_OPM,
    CMsg::ADPCM,
    CMsg::FDD,
    CMsg::HDD,
	CMsg::End
};

/// Keybind labels
const CMsg::Id keybind_col[][2] = {
	{ CMsg::X68000_Key,  CMsg::BindVDIGIT },
#ifdef USE_JOYSTICK
	{ CMsg::X68000_Key,  CMsg::JoypadVDIGIT },
#ifdef USE_PIAJOYSTICK
	{ CMsg::X68000_Port, CMsg::JoypadVDIGIT },
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	{ CMsg::X68000_Port, CMsg::BindVDIGIT },
#endif
	{ CMsg::End, CMsg::End }
};

/// Keybind tabs
const CMsg::Id keybind_tab[] = {
	CMsg::Keyboard,
#ifdef USE_JOYSTICK
	CMsg::Joypad_to_Key,
#ifdef USE_PIAJOYSTICK
	CMsg::Joypad_Port_Connected,
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	CMsg::Key_to_Joypad,
#endif
	CMsg::End
};

/// Keybind buttons
const CMsg::Id keybind_btn[] = {
	CMsg::Load_Default,
	CMsg::Null,		// separate space
	CMsg::Load_Preset_1,
	CMsg::Load_Preset_2,
	CMsg::Load_Preset_3,
	CMsg::Load_Preset_4,
	CMsg::Null,		// separate space
	CMsg::Save_Preset_1,
	CMsg::Save_Preset_2,
	CMsg::Save_Preset_3,
	CMsg::Save_Preset_4,
	CMsg::End
};

/// Keybind options
const CMsg::Id keybind_combi[] = {
	CMsg::Null,
#ifdef USE_JOYSTICK
	CMsg::Recognize_as_another_key_when_pressed_two_buttons,
#ifdef USE_PIAJOYSTICK
# ifndef USE_PIAJOYSTICKBIT
	CMsg::Null,
# else
	CMsg::Signals_are_negative_logic,
# endif
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	CMsg::Null,
#endif
	CMsg::End
};

/// Joypad axis
const CMsg::Id joypad_axis[] = {
	CMsg::X_axis,
	CMsg::Y_axis,
	CMsg::Z_axis,
	CMsg::R_axis,
	CMsg::U_axis,
	CMsg::V_axis,
	CMsg::End
};

/// Joypad type
const CMsg::Id joypad_type[] = {
	CMsg::Standard,
	CMsg::MegaDrive_3_Buttons,
	CMsg::MegaDrive_6_Buttons,
	CMsg::Power_Stick_Fighter,
	CMsg::Power_Stick_Fighter_MD,
	CMsg::Twin_Plus_XPD_1LR,
	CMsg::Cyber_Stick,
	CMsg::End
};

#if defined(_X68000)
/// boot devices on SRAM
const _TCHAR *boot_devices[] = {
	_T("STD"),
	_T("HD0"), _T("HD1"), _T("HD2"), _T("HD3"),
	_T("2HD0"), _T("2HD1"), _T("2HD2"), _T("2HD3"),
	_T("ROM"),
	_T("SRAM"),
	NULL
};
/// RS-232C baud rate on SRAM
const  _TCHAR *rs232c_baudrate[] = {
	_T("75"), _T("150"), _T("300"), _T("600"), _T("1200"), _T("2400"), _T("4800"), _T("9600"),
	_T("17361"), NULL
};
/// RS-232C data bit
const _TCHAR *rs232c_databit[] = {
	_T("5"), _T("6"), _T("7"), _T("8"), NULL
};
/// RS-232C stop bit
const _TCHAR *rs232c_stopbit[] = {
	_T("1"), _T("2"), _T("1.5"), NULL
};
/// RS-232C flow control
const CMsg::Id rs232c_flowctrl[] = {
	CMsg::None_, CMsg::SI_SO, CMsg::Xon_Xoff, CMsg::Xon_Xoff_and_SI_SO, CMsg::End
};
/// Purpose of SRAM free area
const CMsg::Id sram_purpose[] = {
	CMsg::Unused, CMsg::SRAMDISK, CMsg::Program, CMsg::End 
};
/// Key repeat delay
const _TCHAR *key_repeat_delay[] = {
	_T("200"), _T("300"), _T("400"), _T("500"), _T("600"), _T("700"), _T("800"), _T("900"),
	_T("1000"), _T("1100"), _T("1200"), _T("1300"), _T("1400"), _T("1500"), _T("1600"), _T("1700"),
	NULL
};
/// Key repeat rate
const _TCHAR *key_repeat_rate[] = {
	_T("30"), _T("35"), _T("40"), _T("45"), _T("50"), _T("55"), _T("60"), _T("65"),
	_T("70"), _T("75"), _T("80"), _T("85"), _T("90"), _T("100"), _T("105"), _T("110"),
	NULL
};

#endif

}; /* namespace LABELS */
