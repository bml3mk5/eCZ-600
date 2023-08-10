/** @file config.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for X68000 by Sasaji at 2022.02.22

	@brief [ config ]

	@note
	Config ini file uses UTF-8 as string code.
	Under windows: convert to MBCS(shift jis) code from UTF-8.
	Under mac/linux: always uses UTF-8, so LANG environment should be set UTF-8. 
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "vm/vm_defs.h"
#include "emu.h"
#include "cchar.h"
#include "cptrlist.h"
//#ifdef _UNICODE
//#define SI_CONVERT_GENERIC	// use generic simple.ini on all platform
//#endif
//#include "SimpleIni.h"
//#include "simple_ini.h"

class CSimpleIni;

#define FILE_VERSION	0x30

/// @ingroup Enums
/// @brief bit mask of Config::io_port
enum IOPORT_MASKS {
	IOPORT_MSK_ALL		= 0x00000000,
};

/// @ingroup Macros
///@{
///@}

/// @ingroup Enums
/// @brief bit position of Config::io_port
enum IOPORT_POS {
	IOPORT_POS_NONE = 0,
};

/// @ingroup Enums
/// @brief bit mask of Config::misc_flags
enum MISC_FLAG_MASKS {
	MSK_SHOWLEDBOX		= 0x001,	///< bit0: show led
	MSK_SHOWMSGBOARD	= 0x002,	///< bit1: show msg
	MSK_USEJOYSTICK		= 0x004,	///< bit2: use joystick
	MSK_INSIDELEDBOX	= 0x008,	///< bit3: inside led
	MSK_LEDBOX_ALL		= 0x009,
	MSK_USELIGHTPEN		= 0x010,	///< bit4: enable lightpen
	MSK_USEMOUSE		= 0x020,	///< bit5: enable mouse
	MSK_USEPIAJOYSTICK	= 0x040,	///< bit6: use joystick on port
	MSK_USEJOYSTICK_ALL	= 0x044,
	MSK_SHOWMSG_ADDRERR	= 0x100,	///< bit8: show msg when address error
	MSK_USEKEY2JOYSTICK	= 0x400,	///< bit10: use key to joystick
	MSK_PIAJOY_ALL		= 0x440,
	MSK_SHOWDLG_ALL		= 0x5ff,
};

/// @ingroup Macros
///@{
#define FLG_SHOWLEDBOX		(pConfig->misc_flags & MSK_SHOWLEDBOX)
#define FLG_SHOWMSGBOARD	(pConfig->misc_flags & MSK_SHOWMSGBOARD)
#define FLG_USEJOYSTICK		(pConfig->misc_flags & MSK_USEJOYSTICK)
#define FLG_INSIDELEDBOX	(pConfig->misc_flags & MSK_INSIDELEDBOX)
#define FLG_LEDBOX_ALL		(pConfig->misc_flags & MSK_LEDBOX_ALL)
#define FLG_USELIGHTPEN		(pConfig->misc_flags & MSK_USELIGHTPEN)
#define FLG_USEMOUSE		(pConfig->misc_flags & MSK_USEMOUSE)
#define FLG_USEPIAJOYSTICK	(pConfig->misc_flags & MSK_USEPIAJOYSTICK)
#define FLG_USEJOYSTICK_ALL	(pConfig->misc_flags & MSK_USEJOYSTICK_ALL)
#define FLG_SHOWMSG_ADDRERR	(pConfig->misc_flags & MSK_SHOWMSG_ADDRERR)
#define FLG_USEKEY2JOYSTICK (pConfig->misc_flags & MSK_USEKEY2JOYSTICK)
#define FLG_PIAJOY_ALL		(pConfig->misc_flags & MSK_PIAJOY_ALL)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::original
enum ORIGINAL_MASKS {
	MSK_ORIG_SRAM_CLR_PWRON		= 0x0001,	///< bit0: clear SRAM when power on
	MSK_ORIG_SRAM_SAVE_PWROFF	= 0x0002,	///< bit1: save SRAM when power off
	MSK_ORIG_SRAM_RAM_SIZE		= 0x0004,	///< bit2: no overwrite RAM size on SRAM
	MSK_ORIG_SRAM_CHG_BOOT_DEV	= 0x0008,	///< bit3: show msg when boot device was changed
	MSK_ORIG_BORDER_COLOR		= 0x0010,	///< bit4: set gray color on border area
	MSK_ORIG_FDINSERT			= 0x0040,	///< bit6: check fd insert when get track
};

/// @ingroup Macros
///@{
#define FLG_ORIG_SRAM_CLR_PWRON		(pConfig->original & MSK_ORIG_SRAM_CLR_PWRON)
#define FLG_ORIG_SRAM_SAVE_PWROFF	(pConfig->original & MSK_ORIG_SRAM_SAVE_PWROFF)
#define FLG_ORIG_SRAM_RAM_SIZE		(pConfig->original & MSK_ORIG_SRAM_RAM_SIZE)
#define FLG_ORIG_SRAM_CHG_BOOT_DEV  (pConfig->original & MSK_ORIG_SRAM_CHG_BOOT_DEV)
#define FLG_ORIG_BORDER_COLOR		(pConfig->original & MSK_ORIG_BORDER_COLOR)
#define FLG_ORIG_FDINSERT			(pConfig->original & MSK_ORIG_FDINSERT)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::option_fdd
enum OPTIONFDD_MASKS {
	MSK_DELAY_FDSEARCH	= 0x0001,	///< bit0: ignore delay by searching sector
	MSK_DELAY_FDSEEK	= 0x0002,	///< bit1: ignore delay by seeking track
	MSK_DELAY_FD_MASK	= 0x0003,	///< delay fdd flags mask
	MSK_DELAY_FD_SFT	= 0,		///< delay fdd flags shift
	MSK_CHECK_FDDENSITY	= 0x0010,	///< bit4: check a difference of density in a floppy disk
	MSK_CHECK_FDMEDIA	= 0x0020,	///< bit5: check a difference of media in a floppy disk
	MSK_CHECK_FD_MASK	= 0x0030,	///< check fd media flags mask
	MSK_CHECK_FD_SFT	= 4,		///< check fd media flags shift
	MSK_SAVE_FDPLAIN	= 0x0100,	///< bit8: save plain image as it is (default: save it as converted d88 image)
	MSK_SAVE_FD_MASK	= 0x0100,	///< save fd flags mask
	MSK_SAVE_FD_SFT		= 8,		///< save fd flags shift
};

/// @ingroup Macros
///@{
#define FLG_DELAY_FDSEARCH	(pConfig->option_fdd & MSK_DELAY_FDSEARCH)
#define FLG_DELAY_FDSEEK	(pConfig->option_fdd & MSK_DELAY_FDSEEK)
#define FLG_CHECK_FDDENSITY	(pConfig->option_fdd & MSK_CHECK_FDDENSITY)
#define FLG_CHECK_FDMEDIA	(pConfig->option_fdd & MSK_CHECK_FDMEDIA)
#define FLG_SAVE_FDPLAIN	(pConfig->option_fdd & MSK_SAVE_FDPLAIN)
///@}

/// @ingroup Enums
/// @brief bit mask of Config::option_hdd
enum OPTIONHDD_MASKS {
	MSK_DELAY_HDSEEK	= 0x0002,	///< bit1: ignore delay by seeking track
	MSK_DELAY_HD_MASK	= 0x0002,	///< delay hdd flags mask
	MSK_DELAY_HD_SFT	= 0,		///< delay hdd flags shift
};

/// @ingroup Macros
///@{
#define FLG_DELAY_HDSEEK	(pConfig->option_hdd & MSK_DELAY_HDSEEK)
///@}

/// @ingroup Enums
/// @brief sound volumes
enum VOLUME_POS {
	VOLUME_MASTER = 0,
	VOLUME_FM_OPM,
	VOLUME_ADPCM,
	VOLUME_FDD,
	VOLUME_HDD,
	VOLUME_NUMS
};

/// directory path
class CDirPath : public CTchar
{
public:
	void SetFromPath(const _TCHAR *file_path);
	void Set(const _TCHAR *dir_path);
private:
	void Set(const CDirPath &) {}
	void Set(const _TCHAR *, int) {}
};

/// file path
class CFilePath : public CTchar
{
public:
	void Set(const _TCHAR *file_path);
};

#define MAX_HISTORY	20

/// recent path
class CRecentPath
{
public:
	CTchar path;
	int    num;
public:
	CRecentPath();
	CRecentPath(const CRecentPath &src);
	CRecentPath(const _TCHAR *srcpath, int srcnum);
	~CRecentPath();
	void Set(const _TCHAR *srcpath, int srcnum);
	void Clear();
	bool Match(const _TCHAR *tagpath, int tagnum);
};

/// recent path list
class CRecentPathList : public CPtrList<CRecentPath>
{
public:
	bool updated;
public:
	CRecentPathList();
	~CRecentPathList();
	bool Match(const _TCHAR *tagpath, int tagnum);
	void Update(const _TCHAR *tagpath, int tagnum);
};

/// @brief Read/Write config file
///
///	@note
///	Config ini file uses UTF-8 as string code.
///	Under windows: convert to MBCS(shift jis) code from UTF-8.
///	Under mac/linux: always uses UTF-8, so LANG environment should be set UTF-8. 
class Config
{
public:
	int version1;	// config file version
	int version2;

	// recent files
#ifdef USE_FD1
	CDirPath initial_disk_path;
#if defined(USE_FD8) || defined(USE_FD7)
	CRecentPathList recent_disk_path[8];
	CRecentPath opened_disk_path[8];
#elif defined(USE_FD6) || defined(USE_FD5)
	CRecentPathList recent_disk_path[6];
	CRecentPath opened_disk_path[6];
#else
	CRecentPathList recent_disk_path[4];
	CRecentPath opened_disk_path[4];
#endif
#endif
#ifdef USE_HD1
	CDirPath initial_hard_disk_path;
	CRecentPathList recent_hard_disk_path[1];
	CRecentPath opened_hard_disk_path[1];
#endif
#ifdef USE_DATAREC
	CDirPath initial_datarec_path;
	CRecentPathList recent_datarec_path;
	CRecentPath opened_datarec_path;
	bool realmode_datarec;
#endif
#ifdef USE_CART1
	CDirPath initial_cart_path;
	CRecentPath recent_cart_path[1];
#endif
#ifdef USE_QD1
	CDirPath initial_quickdisk_path;
	CRecentPath recent_quickdisk_path[1];
#endif
#ifdef USE_MEDIA
	CDirPath initial_media_path;
	CRecentPath recent_media_path;
#endif
#ifdef USE_BINARY_FILE1
	CDirPath initial_binary_path;
	CRecentPath recent_binary_path[2];
#endif

	// screen
	int window_mode;
	int window_position_x;
	int window_position_y;
	uint8_t stretch_screen;
//	bool cutout_screen;
	int pixel_aspect;
	uint8_t capture_type;

	// sound
	int sound_frequency;
	int sound_latency;

	// virtual machine
	int cpu_power;
	bool now_power_off;
	bool use_power_off;
#ifdef USE_FD1
	bool ignore_crc;
	int  mount_fdd;
#endif
#ifdef USE_HD1
	int  mount_hdd;
#endif
#ifdef USE_DIPSWITCH
	uint8_t dipswitch;
#endif
#ifdef USE_BOOT_MODE
	int boot_mode;		// MZ-800, PASOPIA, PC-8801MA, PC-98DO
#endif
#ifdef USE_CPU_CLOCK_LOW
	bool cpu_clock_low;	// PC-8801MA, PC-9801E, PC-9801VM, PC-98DO
#endif
#if defined(_HC80) || defined(_PASOPIA) || defined(_PC8801MA)
	int device_type;
#endif
#if defined(USE_MONITOR_TYPE) || defined(USE_SCREEN_ROTATE)
	int monitor_type;
#endif
#ifdef USE_SCANLINE
	uint8_t scan_line;
#endif
#if defined(_MBS1)
	uint8_t sys_mode;
	bool mem_nowait;
	uint8_t tvsuper;
#endif
	bool sync_irq;
#ifdef USE_AFTERIMAGE
	int  afterimage;
#endif
#ifdef USE_KEEPIMAGE
	int  keepimage;
#endif
#ifdef USE_DIRECT3D
	uint8_t use_direct3d;
	uint8_t d3d_filter_type;
#endif
#ifdef USE_OPENGL
	uint8_t use_opengl;
	uint8_t gl_filter_type;
#endif

#if defined(_X68000)
	uint8_t main_ram_size_num;
	int raster_int_skew;
	int vdisp_skew;
#endif

	int io_port;
	// bit0: show led  bit1: show msg  bit2: use joystick  bit3: inside led
	// bit4: enable lightpen bit5: enable mouse bit6: use pia joystick
	// bit8: show msg when address error
	// bit10 : use key 2 joystick
	int misc_flags;
	// bit0: clear SRAM when power on
	// bit1: save SRAM when power off
	// bit2: no overwrite RAM size on SRAM
	// bit3: show msg when boot device was changed
	// bit4: set gray color on border area
	// bit6: check fd insert when get track
	int original;

#ifdef USE_FD1
	// bit0: ignore delay by searching sector
	// bit1: ignore delay by seeking track
	// bit4: check a difference of density in a floppy disk
	// bit5: check a difference of media in a floppy disk
	// bit8: save plain image as it is (default: save it as converted d88 image)
	int option_fdd;
#endif
#ifdef USE_HD1
	// bit1: ignore delay by seeking track
	int option_hdd;
#endif

#ifdef USE_STATE
	CDirPath initial_state_path;
	CRecentPath saved_state_path;
	CRecentPathList recent_state_path;
#endif

	int fps_no;
	uint8_t screen_video_size;

	int disp_device_no;
	int screen_width;
	int screen_height;

#ifdef USE_MESSAGE_BOARD
	CTchar msgboard_info_fontname;
	CTchar msgboard_msg_fontname;
	uint8_t msgboard_info_fontsize;
	uint8_t msgboard_msg_fontsize;
#endif

	CDirPath rom_path;

	int volume;
	bool mute;
#ifdef USE_FD1
	int fdd_volume;
	bool fdd_mute;
#endif
#ifdef USE_HD1
	int hdd_volume;
	bool hdd_mute;
#endif

#ifdef USE_AUTO_KEY
	CDirPath initial_autokey_path;
	CRecentPath opened_autokey_path;
#endif

	//
#if defined(GUI_TYPE_AGAR)
	CTchar menu_fontname;
	uint8_t menu_fontsize;
#endif

#if defined(_X68000)
	int opm_volume;
	int adpcm_volume;
	bool opm_mute;
	bool adpcm_mute;
#endif

#ifdef USE_PRINTER
	CDirPath initial_printer_path;
#endif
#ifdef MAX_PRINTER
	CTchar printer_server_host[MAX_PRINTER];
	int    printer_server_port[MAX_PRINTER];
	bool   printer_direct[MAX_PRINTER];
	bool   printer_online[MAX_PRINTER];
	double printer_delay[MAX_PRINTER];
#endif

#ifdef MAX_COMM
	int    comm_dipswitch[MAX_COMM];
	CTchar comm_server_host[MAX_COMM];
	int    comm_server_port[MAX_COMM];
	bool   comm_server[MAX_COMM];
//	bool   comm_connect[MAX_COMM];
	bool   comm_through[MAX_COMM];
	bool   comm_binary[MAX_COMM];
#endif

#ifdef USE_UART
	int    comm_uart_baudrate;
	int    comm_uart_databit;
	int    comm_uart_parity;
	int    comm_uart_stopbit;
	int    comm_uart_flowctrl;
#endif

	CDirPath snapshot_path;

#ifdef USE_LEDBOX
	uint8_t led_pos;
	VmPoint led_dist[2];
#endif

#if defined(USE_WIN)
	CFilePath font_path;
#endif
#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_WX) || defined(USE_WX2)
	CDirPath font_path;
#endif

	bool reckey_recording;
	bool reckey_playing;

#ifdef USE_DATAREC
	bool wav_reverse;
	bool wav_half;
	bool wav_correct;
	uint8_t wav_correct_type;
	int wav_correct_amp[2];

	uint8_t wav_sample_rate;
	uint8_t wav_sample_bits;
#endif

#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif

#ifdef USE_DIRECTINPUT
	enum en_direct_input_flags {
		DIRECTINPUT_ENABLE = 0x01,
		DIRECTINPUT_AVAIL = 0x04,
	};
	// bit0:enable/disable bit2:available
	uint8_t use_direct_input;
#endif

#ifdef USE_DEBUGGER
	int    debugger_imm_start;
	CTchar debugger_server_host;
	int    debugger_server_port;
#endif

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	int    joy_mashing[MAX_JOYSTICKS][KEYBIND_JOY_BUTTONS];
	int    joy_axis_threshold[MAX_JOYSTICKS][6];
	int    joy_type[MAX_JOYSTICKS];
#endif

#ifdef USE_PERFORMANCE_METER
	bool   show_pmeter;
#endif

	CTchar language;

public:
	Config();
	~Config();

	void initialize();
	void load(const _TCHAR *path);
	void save();
	void release();

	static bool get_number_in_path(_TCHAR *path, int *number);
	static bool set_number_in_path(_TCHAR *path, size_t size, int number);

private:
	CSimpleIni *ini;

	CTchar ini_file;

	bool load_ini_file(const _TCHAR *file);
	void save_ini_file(const _TCHAR *file);

	const _TCHAR *conv_to_npath(const _TCHAR *path);
	const _TCHAR *conv_from_npath(const _TCHAR *npath);
	void get_str_value(const _TCHAR *section, const _TCHAR *key, CTchar &str);
	void get_dirpath_value(const _TCHAR *section, const _TCHAR *key, CDirPath &path);
	void get_filepath_value(const _TCHAR *section, const _TCHAR *key, CFilePath &path);
	void get_recentpath_value(const _TCHAR *section, const _TCHAR *key, CRecentPathList &pathlist);

	static int conv_volume(int);
};

extern Config *pConfig;

#endif /* CONFIG_H */

