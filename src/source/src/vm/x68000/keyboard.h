/** @file keyboard.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ keyboard ]
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../vm_defs.h"
#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"
//#include "keyrecord.h"
#include "../../osd/keybind.h"

#ifdef _DEBUG
/* #define _DEBUG_KEYBOARD */
#endif

class EMU;
class KEYRECORD;
class CSimpleIni;
class JOYPAD_BASE;

/**
	@brief keyboard
*/
class KEYBOARD : public DEVICE
{
public:
	enum SIG_IDS {
		SIG_KEY_ENABLE_SEND = 1,
		SIG_JOY_ENABLE_SEND = 2
	};
private:
	enum EVENT_IDS {
		EVENT_KEY_SCAN		= 1,
		EVENT_JOY1_DELAY,
		EVENT_JOY2_DELAY,
	};
	enum en_keyboard_regs {
		REG_TV_CTRL = 0,
		REG_MS_CTRL,
		REG_KEY_ENA,
		REG_DISP_CTRL,
		REG_LED_BRIGHT,
		REG_DISP_ENA,
		REG_OPT2_ENA,
		REG_REP_DELAY,
		REG_REP_TIME,
		REG_LED_STAT,
		REGS_END
	};
	enum en_keyboard_reg_masks {
		REG_MASK_TV_CTRL = 0x1f,
		REG_MASK_MS_CTRL = 0x01,
		REG_MASK_KEY_ENA = 0x01,
		REG_MASK_DISP_CTRL = 0x01,
		REG_MASK_LED_BRIGHT = 0x03,
		REG_MASK_DISP_ENA = 0x01,
		REG_MASK_OPT2_ENA = 0x01,
		REG_MASK_REP_DELAY = 0x0f,
		REG_MASK_REP_TIME = 0x0f,
		REG_MASK_LED_STAT = 0x7f,
	};

private:
	DEVICE *d_cpu;
	DEVICE *d_board;
#ifdef USE_KEY_RECORD
	KEYRECORD *reckey;
#endif
	DEVICE *d_mfp;
	DEVICE *d_mouse;
	DEVICE *d_pio;

	uint8_t m_regs[REGS_END];

	uint8_t* p_key_stat;
//	uint8_t  m_scan_code;
//	uint8_t  m_key_scan_code;
//	uint8_t  key_native_code;
	uint16_t m_key_pressed[128];
	uint8_t  m_key_repeated[128];

	enum en_key_enable_flags {
		KEY_ENABLE_SYSIO = 0x01,
		KEY_ENABLE_MFP = 0x02,
		KEY_ENABLE_ALL = 0x03,
	};
	uint8_t	 m_key_enable;	///< $e8e007 : bit3 and MFP : RR
	int		 m_sending_count;

	bool	 m_first_scan;		///< set after power on

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	JOYPAD_BASE *d_joy[MAX_JOYSTICKS];

	uint32_t *p_joy_stat[MAX_JOYSTICKS];
	uint32_t *p_joy_real_stat[MAX_JOYSTICKS];
	uint8_t	 m_joy_flags;	///< i8255 port c
	int m_joy_register_id[MAX_JOYSTICKS];	// joypad delay
#endif

	int	m_counter;	// keyboard counter

	int  *p_key_mod;
	uint32_t m_pause_pressed;
	uint32_t m_altkey_pressed;
	uint32_t m_mouse_pressed;
	uint32_t m_nmisw_pressed;
	uint32_t m_powersw_pressed;

	uint8_t vm_key_stat[KEYBIND_KEYS];

#ifdef _DEBUG_KEYBOARD
	int frame_counter;
	int event_counter;
#endif

	uint32_key_assign_t scan2key_map[KEYBIND_KEYS];
	uint32_key_assign_t scan2key_preset_map[KEYBIND_PRESETS][KEYBIND_KEYS];
	uint32_key_assign_t joy2key_map[KEYBIND_KEYS];
	uint32_key_assign_t joy2key_preset_map[KEYBIND_PRESETS][KEYBIND_KEYS];
	uint32_key_assign_t sjoy2joy_map[KEYBIND_JOYS];
	uint32_key_assign_t sjoy2joy_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
	uint32_key_assign_t sjoy2joyk_map[KEYBIND_KEYS];
#ifdef USE_KEY2JOYSTICK
	uint32_key_assign_t scan2joy_map[KEYBIND_JOYS];
	uint32_key_assign_t scan2joy_preset_map[KEYBIND_PRESETS][KEYBIND_JOYS];
#endif

	int m_key_register_id;	// keyboad clock

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_regs[REGS_END];
		uint8_t m_key_enable;
		uint8_t m_first_scan;
		char reserved[4];

		int m_sending_count;
		int m_counter;
		int m_key_register_id;
		uint32_t reserved1;

		// v2
		int m_joy_register_id[2];
		uint8_t m_joy_flags;
		char reserved2[3];
		int8_t joy_type[2];
		char reserved3[2];

		uint8_t m_joy_data[2][32];
	};
#pragma pack()

	void update_special_key();
#ifdef USE_LIGHTPEN
	void update_light_pen();
#endif
	void update_keyboard();
	void update_joy_pia();
	void select_joypad();

	enum en_pressing_device {
		PRESS_ON         = 0x01,
		PRESS_ONOFF_MASK = 0x01,
	};
	inline uint32_t pressing_key(int key_code);
	inline void key_pressed();
	inline void key_released();

	bool load_cfg_file();
	void save_cfg_file();
	bool load_ini_file();
	void save_ini_file();
	void convert_map();
	static void load_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, uint32_t *map, uint32_t *preset);
	static void save_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, const uint32_t *map, const uint32_t *preset);

public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~KEYBOARD();

	// common functions
	void initialize();
	void reset();
	void release();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
//	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	// unique functions
	void set_context_cpu(DEVICE* device) {
		d_cpu = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
#ifdef USE_KEY_RECORD
	void set_keyrecord(KEYRECORD *value) {
		reckey = value;
	}
#endif
	void set_context_mfp(DEVICE* device) {
		d_mfp = device;
	}
	void set_context_mouse(DEVICE* device) {
		d_mouse = device;
	}
	void set_context_pio(DEVICE* device) {
		d_pio = device;
	}
	uint32_t get_led_status() const;
	void update_system_key();
	void system_key_down(int code);
	void system_key_up(int code);

	void modify_joytype();
	void save_keybind();
	void clear_joy2joyk_map();
	void set_joy2joyk_map(int num, int idx, uint32_t joy_code);

#ifdef USE_KEY_RECORD
	bool play_reckey(const _TCHAR* filename);
	bool record_reckey(const _TCHAR* filename);
	void stop_reckey(bool stop_play = true, bool stop_record = true);
#endif

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	void debug_event_frame();
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* KEYBOARD_H */

