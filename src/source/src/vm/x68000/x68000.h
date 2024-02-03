/** @file x68000.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ virtual machine ]
*/

#ifndef X68000_H
#define X68000_H

#include "x68000_defs.h"
#include "../../common.h"
//#include "registers.h"

class EMU;
class DEVICE;
class EVENT;

class CRTC;
class YM2151;
class ADPCM;
#if defined(USE_MC68030)
class MC68030;
#elif defined(USE_MC68EC030)
class MC68EC030;
#elif defined(USE_MC68020MMU)
class MC68020MMU;
#elif defined(USE_MC68020)
class MC68020;
#elif defined(USE_MC68010)
class MC68010;
#elif defined(USE_MC68008)
class MC68008;
#else
class MC68000;
#endif

class BOARD;
class MFP;
class DMAC;
class DISPLAY;
class SPRITE_BG;
class KEYBOARD;
class MOUSE;
class MEMORY;
class SCC;
class SYSPORT;
class COMM;
class I8255;
#ifdef USE_FD1
class FDC;
class FLOPPY;
#endif
class SASI;
class RTC;
#ifdef USE_PRINTER
class PRINTER;
#endif

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif
class KEYRECORD;

/// for sleep/resume the machine
#pragma pack(1)
typedef struct vm_state_header_st {
	char header[16];
	uint16_t version;
	uint16_t revision;
	uint32_t param;
	uint16_t emu_major;
	uint16_t emu_minor;
	uint16_t emu_rev;
	uint16_t emu_build;
} vm_state_header_t;
#pragma pack()

/**
	@brief virtual machine for X68000
*/
class VM
{
protected:
	EMU* emu;

	/// @name devices
	//@{
	EVENT*		main_event;

	CRTC*		crtc;
	YM2151*		opm;
	ADPCM*		adpcm;
#if defined(USE_MC68030)
	 MC68030 *cpu;
#elif defined(USE_MC68EC030)
	MC68EC030 *cpu;
#elif defined(USE_MC68020MMU)
	MC68020MMU *cpu;
#elif defined(USE_MC68020)
	MC68020 *cpu;
#elif defined(USE_MC68010)
	MC68010 *cpu;
#elif defined(USE_MC68008)
	MC68008 *cpu;
#else
	MC68000 *cpu;
#endif

	BOARD*		board;
	MFP*		mfp;
	DMAC*		dmac;
	DISPLAY*	display;
	SPRITE_BG*	sp_bg;
	KEYBOARD*	key;
	MOUSE*		mouse;
	MEMORY*		memory;
	SCC*		scc;
	SYSPORT*	sysport;
	COMM*		comm[MAX_COMM];
	I8255*		pio;
#ifdef USE_FD1
	FDC*		fdc;
	FLOPPY*		fdd;
#endif
#ifdef USE_HD1
	SASI*		sasi;
#endif
	RTC*		rtc;
#ifdef USE_PRINTER
	PRINTER*    printer[MAX_PRINTER];
#endif

	DEVICE*		dummy;

//#ifdef USE_DEBUGGER
//	DEBUGGER* debugger;
//#endif
	//@}

	KEYRECORD* reckey;

	void change_fdd_type(int num, bool reset);

public:
	DEVICE* first_device;
	DEVICE* last_device;

public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	/// @name initialize
	//@{
	VM(EMU* parent_emu);
	~VM();
	//@}
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	/// @name drive virtual machine
	//@{
	void run(int split_num);
	double get_frame_rate();
	bool now_skip();
	void update_params();
	void pause(int value);
	//@}
	/// @name draw screen
	//@{
	void set_display_size(int left, int top, int right, int bottom);
	void draw_screen();
	uint64_t update_led();
	//@}
	/// @name sound generation
	//@{
	void initialize_sound(int rate, int samples);
	void reset_sound(int rate, int samples);
	audio_sample_t* create_sound(int* extra_frames, int samples);
	//@}
	/// @name input event from emu
	//@{
	void key_down(int code);
	void key_up(int code);
	void system_key_down(int code);
	void system_key_up(int code);
	//@}
	/// @name socket event from emu
	//@{
//	void network_connected(int ch);
//	void network_disconnected(int ch);
//	void network_writeable(int ch);
//	void network_readable(int ch);
//	void network_accepted(int ch, int new_ch);
//	uint8_t* get_sendbuffer(int ch, int* size, int* flags);
//	void inc_sendbuffer_ptr(int ch, int size);
//	uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
//	uint8_t* get_recvbuffer1(int ch);
//	void inc_recvbuffer_ptr(int ch, int size);
	//@}
	/// @name control menu for user interface
	//@{
	void reset();
	void force_reset();
	void special_reset();
	bool now_special_reset();
	void warm_reset(int onoff);
	void assert_interrupt(int num);
	void update_config();
	void change_dipswitch(int num);
	bool save_state(const _TCHAR* filename);
	bool load_state(const _TCHAR* filename);
#ifdef USE_KEY_RECORD
	bool play_reckey(const _TCHAR* filename);
	bool record_reckey(const _TCHAR* filename);
	void stop_reckey(bool stop_play = true, bool stop_record = true);
#endif
	void change_archtecture(int id, int num, bool reset);
	//@}
#ifdef USE_DATAREC
	/// @name tape menu for user interface
	//@{
	bool play_datarec(const _TCHAR* file_path);
	bool rec_datarec(const _TCHAR* file_path);
	void close_datarec();
	void rewind_datarec();
	void fast_forward_datarec();
	void stop_datarec();
	void realmode_datarec();
	bool datarec_opened(bool play_mode);
	//@}
#endif
#ifdef USE_FD1
	/// @name floppy disk menu for user interface
	//@{
	bool open_disk(int drv, const _TCHAR* file_path, int offset, uint32_t flags);
	bool close_disk(int drv, uint32_t flags);
	int  change_disk(int drv);
	bool disk_inserted(int drv);
	int  get_disk_side(int drv);
	void toggle_disk_write_protect(int drv);
	bool disk_write_protected(int drv);
	bool is_same_disk(int drv, const _TCHAR *file_path, int offset);
	//@}
#endif
#ifdef USE_HD1
	/// @name hard disk menu for user interface
	//@{
	bool open_hard_disk(int drv, const _TCHAR* file_path, uint32_t flags);
	bool close_hard_disk(int drv, uint32_t flags);
	bool hard_disk_mounted(int drv);
	bool is_same_hard_disk(int drv, const _TCHAR *file_path);
	//@}
#endif
	/// @name sound menu for user interface
	//@{
	void set_volume();
	//@}
#ifdef USE_PRINTER
	/// @name printer menu for user interface
	//@{
	bool save_printer(int dev, const _TCHAR* filename);
	void clear_printer(int dev);
	int  get_printer_buffer_size(int dev);
	uint8_t* get_printer_buffer(int dev);
	void enable_printer_direct(int dev);
	bool print_printer(int dev);
	void toggle_printer_online(int dev);
	//@}
#endif
	/// @name comm menu for user interface
	//@{
	void enable_comm_server(int dev);
	void enable_comm_connect(int dev, int num);
	bool now_comm_connecting(int dev, int num);
	void send_comm_telnet_command(int dev, int num);
	//@}
	/// @name options menu for user interface
	//@{
	void modify_joytype();
	void save_keybind();
	//@}
	/// @name dialogs for user interface
	//@{
	void load_sram_file();
	void save_sram_file();
	enum en_sram_addresses {
		SRAM_MAIN_RAM_SIZE = 0x08,
		SRAM_ROM_START_ADDRESS = 0x0c,
		SRAM_SRAM_START_ADDRESS = 0x10,
		SRAM_ALARM_DURATION = 0x14,
		SRAM_BOOT_DEVICE = 0x18,
		SRAM_RS232C = 0x1a,
		SRAM_KEY_LED = 0x1c,
		SRAM_ALARM_ON_TIME = 0x22,
		SRAM_ALARM_ON_OFF = 0x26,
		SRAM_CONTRAST = 0x28,
		SRAM_FD_EJECT = 0x29,
		SRAM_PURPOSE = 0x2d,
		SRAM_KEY_REPEAT_DELAY = 0x3a,
		SRAM_KEY_REPEAT_RATE = 0x3b,
		SRAM_SASI_HDD_NUMS = 0x5a,
	};
	uint32_t get_sram_ram_size() const;
	uint32_t get_sram_rom_start_address() const;
	void set_sram_rom_start_address(uint32_t val);
	uint32_t get_sram_sram_start_address() const;
	void set_sram_sram_start_address(uint32_t val);
	enum en_sram_boot_devices {
		BOOT_DEVICE_STD = 0,
		BOOT_DEVICE_HD0,
		BOOT_DEVICE_HD1,
		BOOT_DEVICE_HD2,
		BOOT_DEVICE_HD3,
		BOOT_DEVICE_2HD0,
		BOOT_DEVICE_2HD1,
		BOOT_DEVICE_2HD2,
		BOOT_DEVICE_2HD3,
		BOOT_DEVICE_ROM,
		BOOT_DEVICE_SRAM,
		BOOT_DEVICE_UNKNOWN
	};
	int get_sram_boot_device() const;
	uint32_t conv_sram_boot_device(int pos) const;
	void set_sram_boot_device(int pos);
	uint16_t get_sram_rs232c() const;
	void set_sram_rs232c(uint32_t val);
	enum en_sram_rs232c_baud_rates {
		BAUD_RATE_75,
		BAUD_RATE_150,
		BAUD_RATE_300,
		BAUD_RATE_600,
		BAUD_RATE_1200,
		BAUD_RATE_2400,
		BAUD_RATE_4800,
		BAUD_RATE_9600,
		BAUD_RATE_17361,
		BAUD_RATE_UNKNOWN
	};
	int get_sram_rs232c_baud_rate(uint32_t value) const;
	uint32_t conv_sram_rs232c_baud_rate(int pos) const;
	enum en_sram_rs232c_databits {
		DATABIT_5BITS,
		DATABIT_6BITS,
		DATABIT_7BITS,
		DATABIT_8BITS,
		DATABIT_UNKNOWN
	};
	int get_sram_rs232c_databit(uint32_t value) const;
	uint32_t conv_sram_rs232c_databit(int pos) const;
	enum en_sram_rs232c_parities {
		SRAM_PARITY_NONE,
		SRAM_PARITY_ODD,
		SRAM_PARITY_EVEN,
		SRAM_PARITY_UNKNOWN
	};
	int get_sram_rs232c_parity(uint32_t value) const;
	uint32_t conv_sram_rs232c_parity(int pos) const;
	enum en_sram_rs232c_stopbits {
		STOPBIT_1BIT,
		STOPBIT_2BITS,
		STOPBIT_1_5BITS,
		STOPBIT_UNKNOWN
	};
	int get_sram_rs232c_stopbit(uint32_t value) const;
	uint32_t conv_sram_rs232c_stopbit(int pos) const;
	enum en_sram_rs232c_flowctrls {
		FLOWCTRL_NONE,
		FLOWCTRL_SISO,
		FLOWCTRL_XONXOFF,
		FLOWCTRL_XONXOFF_SISO,
		FLOWCTRL_UNKNOWN
	};
	int get_sram_rs232c_flowctrl(uint32_t value) const;
	uint32_t conv_sram_rs232c_flowctrl(int pos) const;
	bool get_sram_alarm_onoff() const;
	void set_sram_alarm_onoff(bool val);
	uint32_t get_sram_alarm_time() const;
	int get_sram_alarm_duration() const;
	int get_sram_contrast() const;
	void set_sram_contrast(int val);
	int get_sram_fd_eject() const;
	void set_sram_fd_eject(int val);
	int get_sram_purpose() const;
	void set_sram_purpose(int val);
	int get_sram_key_repeat_delay() const;
	void set_sram_key_repeat_delay(int pos);
	int get_sram_key_repeat_rate() const;
	void set_sram_key_repeat_rate(int pos);
	int get_sram_key_led() const;
	void set_sram_key_led(int val);
	int get_sram_sasi_hdd_nums() const;
	void set_sram_sasi_hdd_nums(int val);
	//@}

	// ----------------------------------------
	// for each device
	// ----------------------------------------
	/// @name event callbacks
	//@{
	void register_event(DEVICE* device, int event_id, int usec, bool loop, int* register_id);
	void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id);
	void cancel_event(DEVICE* device, int register_id);
	void register_frame_event(DEVICE* dev);
	void register_vline_event(DEVICE* dev);
	void set_lines_per_frame(int lines);
	//@}
	/// @name clock
	//@{
	uint64_t get_current_clock();
	uint64_t get_passed_clock(uint64_t prev);
//	uint32_t get_pc();
	//@}
	/// @name get devices
	//@{
	DEVICE* get_device(int id);
	DEVICE* get_device(char *name, char *identifier);
	//@}

	/// for EMU::get_parami method
	enum enumParamiId {
		ParamIOPort = 0,			///< current I/O port settings
		ParamVmKeyMapSize0,
		ParamVmKeyMapSize1,
		ParamVmKeyMapSize2,
		ParamVmKeyMapSize3,
		ParamVkKeyMapKeys0,
		ParamVkKeyMapKeys1,
		ParamVkKeyMapKeys2,
		ParamVkKeyMapKeys3,
		ParamVkKeyMapAssign,
		ParamVkKeyPresets,
		ParamRecVideoType,
		ParamRecVideoCodec,
		ParamRecVideoQuality,
		ParamRecAudioType,
		ParamRecAudioCodec,
		ParamMainRamSizeNum,
		ParamHideScreen,
		ParamiUnknown
	};
	/// for EMU::get_paramv method
	enum enumParamvId {
		ParamVmKeyMap0 = 0,
		ParamVmKeyMap1,
		ParamVmKeyMap2,
		ParamVmKeyMap3,
		ParamVkKeyDefMap0,
		ParamVkKeyDefMap1,
		ParamVkKeyDefMap2,
		ParamVkKeyDefMap3,
		ParamVkKeyMap0,
		ParamVkKeyMap1,
		ParamVkKeyMap2,
		ParamVkKeyMap3,
		ParamVkKeyPresetMap00,
		ParamVkKeyPresetMap01,
		ParamVkKeyPresetMap02,
		ParamVkKeyPresetMap03,
		ParamVkKeyPresetMap10,
		ParamVkKeyPresetMap11,
		ParamVkKeyPresetMap12,
		ParamVkKeyPresetMap13,
		ParamVkKeyPresetMap20,
		ParamVkKeyPresetMap21,
		ParamVkKeyPresetMap22,
		ParamVkKeyPresetMap23,
		ParamVkKeyPresetMap30,
		ParamVkKeyPresetMap31,
		ParamVkKeyPresetMap32,
		ParamVkKeyPresetMap33,
		ParamvUnknown
	};
	enum enumHideScreenFlags {
		Graphic0Mask = 0x0001,
		Graphic1Mask = 0x0002,
		Graphic2Mask = 0x0004,
		Graphic3Mask = 0x0008,
		Graphic4Mask = 0x0010,
		TextMask = 0x0020,
		SpriteMask = 0x0040,
		BG0Mask = 0x0080,
		BG0Sft = 7,
		BG1Mask = 0x0100,
		BG1Sft = 8,
	};
	/// for VM::change_archtecture method
	enum enumArchId {
		ArchFddType = 0,
	};
	// ----------------------------------------
	// access to emu class 
	// ----------------------------------------
	/// @name load rom image
	//@{
	static bool load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
		, uint8_t *data, size_t size
		, const uint8_t *first_data = NULL, size_t first_data_size = 0
		, const uint8_t *last_data = NULL,  size_t last_data_size = 0);
	//@}
	/// @name get/set VM specific parameter
	//@{
	int get_parami(enumParamiId id) const;
	void set_parami(enumParamiId id, int val);
	void *get_paramv(enumParamvId id) const;
	void set_paramv(enumParamvId id, void *val);
	//@}
	/// @name misc
	//@{
	const _TCHAR *application_path() const;
	const _TCHAR *initialize_path() const;
	bool get_pause(int idx) const;
	void set_pause(int idx, bool val);
	void get_edition_string(char *buffer, size_t buffer_len) const;
	//@}

#ifdef USE_DEBUGGER
protected:
	// debugger
	enum en_device_name_map {
		DNM_MFP = 0,
		DNM_SCC,
		DNM_DMAC,
		DNM_CRTC,
		DNM_PALETTE,
		DNM_SPRITE_BG,
		DNM_VID_CTL,
		DNM_FDC,
		DNM_FDD,
		DNM_SASI,
		DNM_OPM,
		DNM_ADPCM,
		DNM_RTC,
		DNM_BOARD,	// interrupt control
		DNM_SYSPORT,	// system port
		DNM_KEYBOARD,
		DNM_SRAM,
		DNM_EVENT
	};
	static const struct st_device_name_map {
		const _TCHAR *name;
		uint32_t        num;
	} c_device_names_map[];

public:
	// debugger
	int get_cpus() const;
	DEVICE *get_cpu(int index);
	DEVICE *get_memory(int index);

	bool get_debug_device_name(const _TCHAR *param, uint32_t *num, int *idx, const _TCHAR **name);
	void get_debug_device_names_str(_TCHAR *buffer, size_t buffer_len);
	bool debug_write_reg(uint32_t num, uint32_t reg_num, uint32_t data);
	bool debug_write_reg(uint32_t num, const _TCHAR *reg, uint32_t data);
	void debug_regs_info(uint32_t num, _TCHAR *buffer, size_t buffer_len);
#endif

};

#endif /* X68000_H */
