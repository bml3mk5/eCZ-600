/** @file keyboard.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ keyboard ]
*/

//#ifdef _UNICODE
//#define SI_CONVERT_GENERIC	// use generic simple.ini on all platform
//#endif
#include "keyboard.h"
//#include "../../emu.h"
#include "../../simple_ini.h"
//#include "display.h"
#include "mfp.h"
#include "mouse.h"
#ifdef USE_PIAJOYSTICK
#include "../i8255.h"
#endif
#include "../../emumsg.h"
#ifdef USE_KEY_RECORD
#include "keyrecord.h"
#endif
#include "keyboard_bind.h"
#include "joypad.h"
#include "../../config.h"
#include "../../labels.h"
#include "../../utility.h"

#define KEYBIND_HEADER	"KEYBIND3"

#ifdef _DEBUG_KEYBOARD
#define OUT_DEBUG logging->out_debugf
#else
#define OUT_DEBUG(...)
#endif
#ifdef _DEBUG
//#define OUT_DEBUG_PRESS logging->out_debugf
#define OUT_DEBUG_PRESS(...)
#else
#define OUT_DEBUG_PRESS(...)
#endif

KEYBOARD::KEYBOARD(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("KEYBOARD");

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		d_joy[i] = NULL;
	}
#endif
}

KEYBOARD::~KEYBOARD()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		delete d_joy[i];
	}
#endif
}

void KEYBOARD::initialize()
{
	p_key_stat = emu->key_buffer();
	memset(m_key_pressed, 0, sizeof(m_key_pressed));
	memset(m_key_repeated, 0, sizeof(m_key_repeated));
	m_key_enable= 0;
	m_sending_count = 0;
	m_first_scan = true;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	m_joy_flags = 0;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		p_joy_stat[i] = emu->joy_buffer(i);
		p_joy_real_stat[i] = emu->joy_real_buffer(i);
		m_joy_register_id[i] = -1;
	}
	select_joypad();
#endif

	p_key_mod = emu->get_key_mod_ptr();
	m_pause_pressed = false;
	m_altkey_pressed = false;
	m_mouse_pressed = false;
	m_nmisw_pressed = false;
	m_powersw_pressed = false;

	memset(vm_key_stat, 0, sizeof(vm_key_stat));
	emu->set_vm_key_status_buffer(vm_key_stat, KEYBIND_KEYS);

	m_counter = 0;
	m_key_register_id = -1;

	memcpy(scan2key_map, scan2key_defmap, sizeof(scan2key_defmap));
#ifdef USE_JOYSTICK
	memcpy(joy2key_map, joy2key_defmap, sizeof(joy2key_defmap));
#ifdef USE_PIAJOYSTICKBIT
	memcpy(sjoy2joy_map, sjoy2joyb_defmap, sizeof(sjoy2joy_defmap));
#else
	memcpy(sjoy2joy_map, sjoy2joy_defmap, sizeof(sjoy2joy_defmap));
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	memcpy(scan2joy_map, scan2joy_defmap, sizeof(scan2joy_defmap));
#endif
	for(int i=0; i<KEYBIND_PRESETS; i++) {
		memcpy(scan2key_preset_map[i], scan2key_defmap, sizeof(scan2key_defmap));
#ifdef USE_JOYSTICK
		memcpy(joy2key_preset_map[i], joy2key_defmap, sizeof(joy2key_defmap));
#ifdef USE_PIAJOYSTICKBIT
		memcpy(sjoy2joy_preset_map[i], sjoy2joyb_defmap, sizeof(sjoy2joy_defmap));
#else
		memcpy(sjoy2joy_preset_map[i], sjoy2joy_defmap, sizeof(sjoy2joy_defmap));
#endif
#endif
#ifdef USE_KEY2JOYSTICK
		memcpy(scan2joy_preset_map[i], scan2joy_defmap, sizeof(scan2joy_defmap));
#endif
	}

	clear_joy2joyk_map();

	// load keybind
	if (load_ini_file() != true) {
		load_cfg_file();
	}

	convert_map();

	// for dialog box
	int max_tabs = 0;
	for(; max_tabs < 4 && LABELS::keybind_tab[max_tabs] != CMsg::End; max_tabs++) {}

	int idx = 0;
	gKeybind.SetVmKeyMap(idx, kb_scan2key_map, (int)(sizeof(kb_scan2key_map) / sizeof(kb_scan2key_map[0])));
#ifdef USE_JOYSTICK
	idx++;
	gKeybind.SetVmKeyMap(idx, kb_scan2key_map, (int)(sizeof(kb_scan2key_map) / sizeof(kb_scan2key_map[0])));
#ifdef USE_PIAJOYSTICK
	idx++;
# ifdef USE_PIAJOYSTICKBIT
	gKeybind.SetVmKeyMap(idx, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
# else
	gKeybind.SetVmKeyMap(idx, kb_sjoy2joy_map, (int)(sizeof(kb_sjoy2joy_map) / sizeof(kb_sjoy2joy_map[0])));
# endif
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	idx++;
# ifdef USE_PIAJOYSTICKBIT
	gKeybind.SetVmKeyMap(idx, kb_sjoy2joybit_map, (int)(sizeof(kb_sjoy2joybit_map) / sizeof(kb_sjoy2joybit_map[0])));
# else
	gKeybind.SetVmKeyMap(idx, kb_scan2joy_map, (int)(sizeof(kb_scan2joy_map) / sizeof(kb_scan2joy_map[0])));
# endif
#endif

	idx = 0;
	gKeybind.SetVkKeySize(idx, KEYBIND_KEYS);
#ifdef USE_JOYSTICK
	idx++;
	gKeybind.SetVkKeySize(idx, KEYBIND_KEYS);
#ifdef USE_PIAJOYSTICK
	idx++;
	gKeybind.SetVkKeySize(idx, KEYBIND_JOYS);
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	idx++;
	gKeybind.SetVkKeySize(idx, KEYBIND_JOYS);
#endif

	idx = 0;
	gKeybind.SetVkKeyDefMap(idx, scan2key_defmap);
#ifdef USE_JOYSTICK
	idx++;
	gKeybind.SetVkKeyDefMap(idx, joy2key_defmap);
#ifdef USE_PIAJOYSTICK
	idx++;
# ifdef USE_PIAJOYSTICKBIT
	gKeybind.SetVkKeyDefMap(idx, sjoy2joybit_defmap);
# else
	gKeybind.SetVkKeyDefMap(idx, sjoy2joy_defmap);
# endif
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	idx++;
# ifdef USE_PIAJOYSTICKBIT
	gKeybind.SetVkKeyDefMap(idx, scan2joybit_defmap);
# else
	gKeybind.SetVkKeyDefMap(idx, scan2joy_defmap);
# endif
#endif

	idx = 0;
	gKeybind.SetVkKeyMap(idx, scan2key_map);
#ifdef USE_JOYSTICK
	idx++;
	gKeybind.SetVkKeyMap(idx, joy2key_map);
#ifdef USE_PIAJOYSTICK
	idx++;
	gKeybind.SetVkKeyMap(idx, sjoy2joy_map);
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	idx++;
	gKeybind.SetVkKeyMap(idx, scan2joy_map);
#endif

	for(int i=0; i<KEYBIND_PRESETS; i++) {
		idx = 0;
		gKeybind.SetVkKeyPresetMap(idx, i, scan2key_preset_map[i]);
#ifdef USE_JOYSTICK
		idx++;
		gKeybind.SetVkKeyPresetMap(idx, i, joy2key_preset_map[i]);
#ifdef USE_PIAJOYSTICK
		idx++;
		gKeybind.SetVkKeyPresetMap(idx, i, sjoy2joy_preset_map[i]);
#endif
#endif
#ifdef USE_KEY2JOYSTICK
		idx++;
		gKeybind.SetVkKeyPresetMap(idx, i, scan2joy_preset_map[i]);
#endif
	}

#ifdef USE_KEY_RECORD
	reckey->set_counter_ptr(&m_counter);
#endif

	// event
	register_frame_event(this);
//	register_vline_event(this);
}

void KEYBOARD::convert_map()
{
	emu->clear_vm_key_map();
	for(int k=0; k<KEYBIND_KEYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			emu->set_vm_key_map(scan2key_map[k].d[i], k);
		}
	}
#ifdef USE_PIAJOYSTICK
	int aidx = -1;
	emu->clear_joy2joy_idx();
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		if (!(sjoy2joy_typemap[k] & (CODE_TABLE_FLAG_JOYKEY | CODE_TABLE_FLAG_JOYANA))) {
			emu->set_joy2joy_idx(k, sjoy2joy_defidx[k]);
		}
	}
	emu->clear_joy2joy_map();
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			if (sjoy2joy_typemap[k] & CODE_TABLE_FLAG_JOYANA) {
				if (aidx < 0) aidx = k;
				emu->set_joy2joy_ana_map(i, k - aidx, sjoy2joy_map[k].d[i]);
			} else if (sjoy2joy_typemap[k] & CODE_TABLE_FLAG_JOYKEY) {
				set_joy2joyk_map(i, k, sjoy2joy_map[k].d[i]);
			} else {
				emu->set_joy2joy_map(i, k, sjoy2joy_map[k].d[i]);
			}
		}
	}
#endif
#ifdef USE_KEY2JOYSTICK
	emu->clear_key2joy_map();
	for(uint32_t k=0; k<KEYBIND_JOYS; k++) {
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			if (k >= 0x0c) {
				// buttons
				emu->set_key2joy_map(scan2joy_map[k].d[i], i, (k - 0x0c) | 0x80000000);
			} else {
				// allows
				emu->set_key2joy_map(scan2joy_map[k].d[i], i, k);
			}
		}
	}
#endif
}

void KEYBOARD::reset()
{
	m_key_enable = 0;
	m_sending_count = 0;

	m_counter = 0;
	memset(m_regs, 0, sizeof(m_regs));
	m_regs[REG_LED_STAT] = 0x7f;
	m_regs[REG_REP_DELAY] = 0x03;
	m_regs[REG_REP_TIME] = 0x04;
	m_first_scan = true;

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		if (d_joy[i]) d_joy[i]->reset();
	}
	m_joy_flags = 0;
#endif

#ifdef _DEBUG_KEYBOARD
	frame_counter = 0;
	event_counter = 0;
#endif
	cancel_event(this, m_key_register_id);
	m_key_register_id = -1;

	// key counter 5000(us/key) / 128(keys) = 39.0625
	register_event(this, EVENT_KEY_SCAN, 39.0625, true, &m_key_register_id);
}

void KEYBOARD::release()
{
	emu->set_vm_key_status_buffer(NULL, 0);
	// save keybind
	save_keybind();
}

// ----------------------------------------------------------------------------

void KEYBOARD::modify_joytype()
{
	select_joypad();
}

void KEYBOARD::save_keybind()
{
	// save keybind
	save_cfg_file();
	save_ini_file();
}

void KEYBOARD::clear_joy2joyk_map()
{
	memset(sjoy2joyk_map, 0, sizeof(sjoy2joyk_map));
}

void KEYBOARD::set_joy2joyk_map(int num, int idx, uint32_t joy_code)
{
	int key_code = (sjoy2joy_typemap[idx] & 0xff);
	sjoy2joyk_map[key_code].d[num] = joy_code;
}

/// cfg file is no longer supported.
bool KEYBOARD::load_cfg_file()
{
	return true;
}

/// cfg file is no longer supported.
void KEYBOARD::save_cfg_file()
{
}

// ----------------------------------------------------------------------------

void KEYBOARD::load_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, uint32_t *map, uint32_t *preset)
{
	_TCHAR section[100];
	int cols = KEYBIND_ASSIGN;

	const CSimpleIniSection *csection = ini->Find(section_name);
	int count = 0;
	if (csection) {
		memset(map, 0, sizeof(uint32_t) * rows * cols);
		count = csection->Count();
	}
	for (int idx = 0; idx < count; idx++) {
		const CSimpleIniItem *item = csection->Item(idx);
		int k = 0;
		int i = 0;
		int rc = _stscanf(item->GetKey().Get(), _T("%02x_%d"), &k, &i);
		if (rc != 2 || k < 0 || k >= rows || i < 0 || i >= cols) {
			continue;
		}
		long v = item->GetLong(0);
		map[k * cols + i] = (uint32_t)(v & 0xffffffff);
	}
	for (int no = 0; no < KEYBIND_PRESETS; no++) {
		uint32_t *preset1 = &preset[no * rows * cols];
		UTILITY::stprintf(section, 100, _T("%sPreset%d"), section_name, no + 1);
		csection = ini->Find(section);
		count = 0;
		if (csection) {
			memset(preset1, 0, sizeof(uint32_t) * rows * cols);
			count = csection->Count();
		}
		for (int idx = 0; idx < count; idx++) {
			const CSimpleIniItem *item = csection->Item(idx);
			int k = 0;
			int i = 0;
			int rc = _stscanf(item->GetKey().Get(), _T("%02x_%d"), &k, &i);
			if (rc != 2 || k < 0 || k >= rows || i < 0 || i >= cols) {
				continue;
			}
			long v = item->GetLong(0);
			preset1[k * cols + i] = (uint32_t)(v & 0xffffffff);
		}
	}
}

bool KEYBOARD::load_ini_file()
{
	const _TCHAR *app_path;

	CSimpleIni *ini = new CSimpleIni();
//#ifdef _UNICODE
//	ini->SetUnicode(true);
//#endif

	// load ini file
	app_path = emu->initialize_path();

	_TCHAR file_path[_MAX_PATH];
	UTILITY::concat(file_path, _MAX_PATH, app_path, _T("keybind.ini"), NULL);

	if (ini->LoadFile(file_path) == true) {
		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, _T("keybind.ini"));
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("keybind.ini"));
		delete ini;
		return false;
	}
	// check file version
	if (_tcscmp(ini->GetValue(_T(""), _T("Version"), _T("")), _T(VK_KEY_TYPE)) != 0) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_is_not_compatible_use_default_setting, _T("keybind.ini"));
		delete ini;
		return false;
	}

	load_ini_file_one(ini, _T("Keyboard2Key"), KEYBIND_KEYS, (uint32_t *)scan2key_map, (uint32_t *)scan2key_preset_map);
#ifdef USE_JOYSTICK
	load_ini_file_one(ini, _T("Joypad2Key"), KEYBIND_KEYS, (uint32_t *)joy2key_map, (uint32_t *)joy2key_preset_map);
#ifdef USE_PIAJOYSTICK
	load_ini_file_one(ini, _T("Joypad2Joy"), KEYBIND_JOYS, (uint32_t *)sjoy2joy_map, (uint32_t *)sjoy2joy_preset_map);
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	load_ini_file_one(ini, _T("Keyboard2Joy"), KEYBIND_JOYS, (uint32_t *)scan2joy_map, (uint32_t *)scan2joy_preset_map);
#endif

	delete ini;

	return true;
}

void KEYBOARD::save_ini_file_one(CSimpleIni *ini, const _TCHAR *section_name, int rows, const uint32_t *map, const uint32_t *preset)
{
	_TCHAR section[100];
	_TCHAR key[100];
	int cols = KEYBIND_ASSIGN;

    for (int k = 0; k < rows; k++) {
		for (int i = 0; i < cols; i++) {
			UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
			const uint32_t v = map[k * cols + i];
			if (v != 0) {
				ini->SetLongValue(section_name, key, v, NULL, true);
			}
		}
	}
    for (int no = 0; no < KEYBIND_PRESETS; no++) {
		UTILITY::stprintf(section, 100, _T("%sPreset%d"), section_name, no + 1);
		for (int k = 0; k < rows; k++) {
			for (int i = 0; i < cols; i++) {
				const uint32_t *preset1 = &preset[no * rows * cols];
				UTILITY::stprintf(key, 100, _T("%02x_%d"), k, i);
				const uint32_t v = preset1[k * cols + i];
				if (v != 0) {
					ini->SetLongValue(section, key, v, NULL, true);
				}
			}
		}
	}
}

void KEYBOARD::save_ini_file()
{
	const _TCHAR *app_path;
	_TCHAR comment[100];

	CSimpleIni *ini = new CSimpleIni();
//#ifdef _UNICODE
//	ini->SetUnicode(true);
//#endif

	// section
	UTILITY::stprintf(comment, 100, _T("; %s keybind file"), _T(DEVICE_NAME));
	ini->SetValue(_T(""), _T("Version"), _T(VK_KEY_TYPE), comment);

	save_ini_file_one(ini, _T("Keyboard2Key"), KEYBIND_KEYS, (const uint32_t *)scan2key_map, (const uint32_t *)scan2key_preset_map);
#ifdef USE_JOYSTICK
	save_ini_file_one(ini, _T("Joypad2Key"), KEYBIND_KEYS, (const uint32_t *)joy2key_map, (const uint32_t *)joy2key_preset_map);
#ifdef USE_PIAJOYSTICK
	save_ini_file_one(ini, _T("Joypad2Joy"), KEYBIND_JOYS, (const uint32_t *)sjoy2joy_map, (const uint32_t *)sjoy2joy_preset_map);
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	save_ini_file_one(ini, _T("Keyboard2Joy"), KEYBIND_JOYS, (const uint32_t *)scan2joy_map, (const uint32_t *)scan2joy_preset_map);
#endif

	// save ini file
	app_path = emu->initialize_path();

	_TCHAR file_path[_MAX_PATH];
	UTILITY::concat(file_path, _MAX_PATH, app_path, _T("keybind.ini"), NULL);

	if (ini->SaveFile(file_path) == true) {
		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_saved, _T("keybind.ini"));
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_saved, _T("keybind.ini"));
	}

	delete ini;
}

// ----------------------------------------------------------------------------

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
		case MFP::SIG_SO:
			switch(data & 0xf0) {
			case 0x00:
			case 0x10:
			case 0x20:
			case 0x30:
				// display tv control
				m_regs[REG_TV_CTRL] = (data & mask & REG_MASK_TV_CTRL);
				break;
			case 0x40:
				if (!(data & 0x08)) {
					// request mouse status
//					if ((m_regs[REG_MS_CTRL] & 1) != 0 && (data & 1) == 0) {
//						d_mouse->write_signal(MOUSE::SIG_RTS_TO_MFP, 1, 1);
//					}
					m_regs[REG_MS_CTRL] = (data & mask & REG_MASK_MS_CTRL);
				} else {
					// request key enable
					m_regs[REG_KEY_ENA] = (data & mask & REG_MASK_KEY_ENA);
				}
				break;
			case 0x50:
				switch(data & 0x0c) {
				case 0x00:
					// display control type
					m_regs[REG_DISP_CTRL] = (data & mask & REG_MASK_DISP_CTRL);
					break;
				case 0x04:
					// bright led
					m_regs[REG_LED_BRIGHT] = (data & mask & REG_MASK_LED_BRIGHT);
					break;
				case 0x08:
					// display control
					m_regs[REG_DISP_ENA] = (data & mask & REG_MASK_DISP_ENA);
					break;
				case 0x0c:
					// opt2 key control
					m_regs[REG_OPT2_ENA] = (data & mask & REG_MASK_OPT2_ENA);
					break;
				}
				break;
			case 0x60:
				// key repeat delay time
				m_regs[REG_REP_DELAY] = (data & mask & REG_MASK_REP_DELAY);
				break;
			case 0x70:
				// key repeat time
				m_regs[REG_REP_TIME] = (data & mask & REG_MASK_REP_TIME);
				break;
			default:
				// change led status
				m_regs[REG_LED_STAT] = (data & mask & REG_MASK_LED_STAT);
				break;
			}
			break;

		case MFP::SIG_RR:
			// mfp can receive data on usart
			BIT_ONOFF(m_key_enable, KEY_ENABLE_MFP, (data & mask) != 0);
			break;

		case SIG_KEY_ENABLE_SEND:
			// key enable flag on system port
			BIT_ONOFF(m_key_enable, KEY_ENABLE_SYSIO, (data & mask) != 0);
			break;

		case SIG_JOY_ENABLE_SEND:
			// flags on i8255 port c for joystick
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
			for(int i=0; i<MAX_JOYSTICKS; i++) {
				if (d_joy[i]) d_joy[i]->modify_flags(data);
			}
			m_joy_flags = (data & 0xf0);
#endif
			break;

		case SIG_CPU_RESET:
			now_reset = ((data & mask) != 0);
			reset();
			break;
	}
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	return 0xff;
}

void KEYBOARD::update_special_key()
{
	uint32_t pressed = 0;

	for(int code = 0x84; code <= 0x86; code++) {
		pressed = pressing_key(code);
		switch(code) {
			case 0x84:
				// nmi switch
				pressed &= PRESS_ONOFF_MASK;
				if ((pressed ^ m_nmisw_pressed) & pressed) {
					d_board->write_signal(SIG_CPU_IRQ, 7, 7);
				}
				m_nmisw_pressed = pressed;
				break;
			case 0x85:
				// power switch
				pressed &= PRESS_ONOFF_MASK;
				if ((pressed ^ m_powersw_pressed) & pressed) {
					emumsg.Send(EMUMSG_ID_SPECIAL_RESET);
				}
				m_powersw_pressed = pressed;
				break;
			case 0x86:
				// reset switch
				pressed &= PRESS_ONOFF_MASK;
				if ((pressed ^ (now_reset ? 1 : 0))) {
					d_board->write_signal(SIG_CPU_RESET, pressed ? 2 : 0, 2);
				}
				break;
		}
	}
}

void KEYBOARD::update_system_key()
{
	uint32_t pressed = 0;

	for(int code = 0x80; code <= 0x82; code++) {
		pressed = pressing_key(code);
		switch(code) {
			case 0x80:
				// pause
				pressed &= PRESS_ONOFF_MASK;
				if ((pressed ^ m_pause_pressed) & pressed) {
					emumsg.Send(EMUMSG_ID_PAUSE);
				}
				m_pause_pressed = pressed;
				break;
			case 0x81:
				// alt
				pressed &= PRESS_ONOFF_MASK;
				m_altkey_pressed = pressed;
				BIT_ONOFF(*p_key_mod, KEY_MOD_ALT_KEY, m_altkey_pressed);
				break;
			case 0x82:
				// mouse on/off
				if ((pressed ^ m_mouse_pressed) & pressed & PRESS_ONOFF_MASK) {
					if (!(pressed & VM_KEY_STATUS_KEYREC)) emu->post_command_message(ID_OPTIONS_MOUSE);
				}
				m_mouse_pressed = pressed;
				break;
		}
	}
}

void KEYBOARD::update_joy_pia()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		if (d_joy[i]) d_joy[i]->update_joy();
	}
#endif
}

void KEYBOARD::select_joypad()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		if (d_joy[i]) {
			if (d_joy[i]->get_type() == pConfig->joy_type[i]) continue;
		}

		delete d_joy[i];

		switch(pConfig->joy_type[i]) {
		case JOYPAD_BASE::TYPE_MD3:
			d_joy[i] = new JOYPAD_MD3(d_pio, i, p_joy_stat[i]);
			break;
		case JOYPAD_BASE::TYPE_MD6:
			d_joy[i] = new JOYPAD_MD6(d_pio, i, p_joy_stat[i]);
			break;
		case JOYPAD_BASE::TYPE_CPSF:
			d_joy[i] = new JOYPAD_CPSF(d_pio, i, p_joy_stat[i]);
			break;
		case JOYPAD_BASE::TYPE_CPSF_MD:
			d_joy[i] = new JOYPAD_CPSF_MD(d_pio, i, p_joy_stat[i]);
			break;
		case JOYPAD_BASE::TYPE_XPDLR:
			d_joy[i] = new JOYPAD_XPDLR(d_pio, i, p_joy_stat[i]);
			break;
		case JOYPAD_BASE::TYPE_CYBER:
			d_joy[i] = new JOYPAD_CYBER(d_pio, i, p_joy_stat[i]);
			break;
		default:
			d_joy[i] = new JOYPAD_STD(d_pio, i, p_joy_stat[i]);
			break;
		}
		pConfig->joy_type[i] = d_joy[i]->get_type();

#ifdef USE_KEY_RECORD
		d_joy[i]->set_keyrecord_ptr(reckey);
#endif

		d_joy[i]->set_event_param(this, EVENT_JOY1_DELAY + i, &m_joy_register_id[i]);
	}
#endif
}

// ----------------------------------------------------------------------------

uint32_t KEYBOARD::pressing_key(int key_code)
{
	uint32_t code = 0;
	uint32_t pressed = 0;
	int i = 0;

#ifdef USE_KEY_RECORD
	reckey->playing_key();
#endif

#if 0
	for(i=0; i<2; i++) {
		code = scan2key_map[key_code][i];
		if (code == 0) break;

		// key pressed ?
		if (p_key_stat[code]) {
			pressed = (PRESS_ON | PRESS_KEYBOARD);
			break;
		}
	}
#else
	// key pressed ?
	pressed = (vm_key_stat[key_code] & VM_KEY_STATUS_MASK);
	if (pressed) {
		 pressed |= PRESS_ON;
	}

#endif
#if defined(USE_JOYSTICK)
	if (!pressed && FLG_USEJOYSTICK) {
		for(i=0; i<MAX_JOYSTICKS; i++) {
			code = joy2key_map[key_code].d[i];
			if (code == 0) continue;

			// joypad pressed ?
			// allow key or button
			if (!(joy2key_map[KEYBIND_KEYS - 1].d[0] & 1)) {
				if ((p_joy_real_stat[i][0] & 0xf & code) == code || (p_joy_real_stat[i][0] & 0xfffffff0 & code) == code) {
					pressed = PRESS_ON;
					break;
				}
			} else {
				if ((p_joy_real_stat[i][0] & 0xf) == code || (p_joy_real_stat[i][0] & 0xfffffff0) == code) {
					pressed = PRESS_ON;
					break;
				}
			}
		}
	}
	if (!pressed && FLG_USEPIAJOYSTICK) {
		for(i=0; i<MAX_JOYSTICKS; i++) {
			code = sjoy2joyk_map[key_code].d[i];
			if (code == 0) continue;
			if ((p_joy_real_stat[i][0] & 0xf & code) == code || (p_joy_real_stat[i][0] & 0xfffffff0 & code) == code) {
				pressed = PRESS_ON;
				break;
			}
		}
	}
#endif
#if 0
	if (!pressed) {
		// auto key pressed ?
		if (vm_key_stat[key_code]) {
			pressed = PRESS_ON;
		}
	}
#endif
#ifdef USE_KEY_RECORD
//	if (!pressed) {
//		pressed = reckey->processing_keys(key_code, false) ? (PRESS_ON | PRESS_RECKEY) : 0;
//	}
	reckey->recording_key(key_code, pressed != 0);
#endif

	return pressed;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void KEYBOARD::event_frame()
{
#ifdef USE_KEY_RECORD
	// read record key if need
//	reckey->reading_keys(1);
	reckey->read_to_cache();
	reckey->playing_system_keys();
#endif

	// reset switch & break key
	update_special_key();
	// other system keys
	update_system_key();
#ifdef USE_LIGHTPEN
	// light pen
	update_light_pen();
#endif
	// joystick on PIA
	update_joy_pia();
}

#if 0
void KEYBOARD::event_vline(int v, int clock)
{
	update_keyboard();
}
#endif

void KEYBOARD::event_callback(int event_id, int err)
{
	switch (event_id) {
	case EVENT_KEY_SCAN:
		update_keyboard();
		break;
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	case EVENT_JOY1_DELAY:
		d_joy[0]->event_callback(event_id);
		break;
	case EVENT_JOY2_DELAY:
		d_joy[1]->event_callback(event_id);
		break;
#endif
	}
}

// ----------------------------------------------------------------------------
/// keyboard counter counts up every about 40usec. (vm inner time).
/// The status of a key is updated every 5msec.
void KEYBOARD::update_keyboard()
{
#ifdef _DEBUG_KEYBOARD
	if (event_counter == 0) {
		frame_counter = (frame_counter + 1) % 10000;
	}
	event_counter++;
	if (event_counter >= KEYBOARD_COUNTER_MAX) event_counter = 0;
#endif

	OUT_DEBUG("kbd ukd 1 fr:%04d ct:%03x rc:%4d s:-- k:%02x"
	, frame_counter, counter, remain_count, key_scan_code);

#ifdef USE_KEY_RECORD
	// read record key if need
//	reckey->reading_keys(0);
#endif

	if (m_sending_count) {
		// now sending data to mfp device
		m_sending_count--;
		if (!m_sending_count) {
			OUT_DEBUG_PRESS(_T("KEYBOARD: SENDOK:%02X"), m_counter); 
			emu->recognized_key(m_counter);
		}

	} else {
		m_counter++;
		if (m_counter >= 128) m_counter = 0;

		if (m_first_scan) {
			uint32_t pressed = pressing_key(m_counter);
			switch(m_counter) {
			case 0x57:
			case 0x58:
			case 0x59:
				// XF3, XF4, XF5
				if (pressed) {
					m_regs[REG_LED_BRIGHT] = (m_counter - 0x56) & REG_MASK_LED_BRIGHT;
				}
				break;
			case 0x7f:
				// send 0xff to MFP forcely after power on reset
				if (m_key_enable == KEY_ENABLE_ALL) {
					d_mfp->write_signal(MFP::SIG_SI, m_counter | 0x80, 0xff);
					m_sending_count = 128;
					m_first_scan = false;
				}
				break;
			default:
				break;
			}
		} else {
			uint32_t pressed = pressing_key(m_counter);
			if (pressed) {
				key_pressed();
			} else {
				key_released();
			}
		}
	}
}

void KEYBOARD::key_pressed()
{
	uint32_t scan_code = (m_counter & 0x7f);

	if (m_key_pressed[scan_code]) {
		// key pressing continuous
		m_key_pressed[scan_code]--;
	} else {
		// key pressed
		if (m_key_enable == KEY_ENABLE_ALL) {
			d_mfp->write_signal(MFP::SIG_SI, scan_code, 0xff);
			m_sending_count = 128;
		}
		if (!m_key_repeated[scan_code]) {
			// at first
			m_key_pressed[scan_code] = ((uint32_t)m_regs[REG_REP_DELAY] * 100 + 200) / 5;
			m_key_repeated[scan_code] = 1;
		} else {
			// repeat
			m_key_pressed[scan_code] = ((uint32_t)m_regs[REG_REP_TIME] * m_regs[REG_REP_TIME] * 5 + 30) / 5;
			m_key_repeated[scan_code] = 2;
		}
		OUT_DEBUG_PRESS(_T("KEYBOARD: PRESS:%02X"), scan_code); 
		m_key_pressed[scan_code]--;
	}
}

void KEYBOARD::key_released()
{
	uint32_t scan_code = (m_counter & 0x7f);

	if (m_key_pressed[scan_code] | m_key_repeated[scan_code]) {
		// key released
		if (m_key_enable == KEY_ENABLE_ALL) {
			d_mfp->write_signal(MFP::SIG_SI, scan_code | 0x80, 0xff);
			m_sending_count = 128;
		}
		m_key_pressed[scan_code] = 0;
		m_key_repeated[scan_code] = 0;
		OUT_DEBUG_PRESS(_T("KEYBOARD: RELEASE:%02X"), scan_code); 
	}
}

// ----------------------------------------------------------------------------
void KEYBOARD::system_key_down(int code)
{
#ifdef USE_KEY_RECORD
	reckey->recording_system_keys(code, true);
#endif
}

void KEYBOARD::system_key_up(int code)
{
#ifdef USE_KEY_RECORD
	reckey->recording_system_keys(code, false);
#endif
}

// ----------------------------------------------------------------------------
#ifdef USE_KEY_RECORD
bool KEYBOARD::play_reckey(const _TCHAR* filename)
{
	return reckey->play_reckey(filename);
}

bool KEYBOARD::record_reckey(const _TCHAR* filename)
{
	return reckey->record_reckey(filename);
}

void KEYBOARD::stop_reckey(bool stop_play, bool stop_record)
{
	reckey->stop_reckey(stop_play, stop_record);
}
#endif

// ----------------------------------------------------------------------------
uint32_t KEYBOARD::get_led_status() const
{
	return m_regs[REG_LED_STAT] | ((uint32_t)m_regs[REG_LED_BRIGHT] <<7);
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void KEYBOARD::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(2);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<REGS_END; i++) {
		SET_Byte(m_regs[i]);
	}
	SET_Byte(m_key_enable);
	SET_Bool(m_first_scan);

	SET_Int32_LE(m_sending_count);
	SET_Int32_LE(m_counter);
	SET_Int32_LE(m_key_register_id);

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	SET_Byte(m_joy_flags);
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		SET_Int32_LE(m_joy_register_id[i]);

		vm_state.joy_type[i] = (int8_t)d_joy[i]->get_type();
		d_joy[i]->save_state(vm_state.m_joy_data[i], (int)sizeof(vm_state.m_joy_data[i]));
	}
#endif

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool KEYBOARD::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	for(int i=0; i<REGS_END; i++) {
		GET_Byte(m_regs[i]);
	}
	GET_Byte(m_key_enable);
	GET_Bool(m_first_scan);

	GET_Int32_LE(m_sending_count);
	GET_Int32_LE(m_counter);
	GET_Int32_LE(m_key_register_id);

	if (vm_state_i.version == Uint16_LE(2)) {
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
		GET_Byte(m_joy_flags);
		for(int i=0; i<MAX_JOYSTICKS; i++) {
			GET_Int32_LE(m_joy_register_id[i]);

			pConfig->joy_type[i] = (int)vm_state.joy_type[i];

			select_joypad();

			d_joy[i]->load_state(vm_state.m_joy_data[i], (int)sizeof(vm_state.m_joy_data[i]));
		}
#endif
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t KEYBOARD::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0;
	return data;
}

void KEYBOARD::debug_event_frame()
{
	// reset switch & break key
	update_special_key();
	// other system keys
	update_system_key();
#ifdef USE_LIGHTPEN
	// light pen
	update_light_pen();
#endif
	// joystick on PIA
	update_joy_pia();
}

static const _TCHAR *c_reg_names[] = {
	_T("TV_CTRL"),
	_T("MS_CTRL"),
	_T("KEY_ENA"),
	_T("DISP_CTRL"),
	_T("LED_BRIGHT"),
	_T("DISP_ENA"),
	_T("OPT2_ENA"),
	_T("REP_DELAY"),
	_T("REP_TIME"),
	_T("LED_STAT"),
	NULL
};

bool KEYBOARD::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool KEYBOARD::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch (reg_num) {
	case REG_TV_CTRL:
		// display tv control
		m_regs[REG_TV_CTRL] = (data & REG_MASK_TV_CTRL);
		break;
	case REG_MS_CTRL:
		// mouse control
		m_regs[REG_MS_CTRL] = (data & REG_MASK_MS_CTRL);
		break;
	case REG_KEY_ENA:
		// request key enable
		m_regs[REG_KEY_ENA] = (data & REG_MASK_KEY_ENA);
		break;
	case REG_DISP_CTRL:
		// display control type
		m_regs[REG_DISP_CTRL] = (data & REG_MASK_DISP_CTRL);
		break;
	case REG_LED_BRIGHT:
		// bright led
		m_regs[REG_LED_BRIGHT] = (data & REG_MASK_LED_BRIGHT);
		break;
	case REG_DISP_ENA:
		// display control
		m_regs[REG_DISP_ENA] = (data & REG_MASK_DISP_ENA);
		break;
	case REG_OPT2_ENA:
		// opt2 key control
		m_regs[REG_OPT2_ENA] = (data & REG_MASK_OPT2_ENA);
		break;
	case REG_REP_DELAY:
		// key repeat delay time
		m_regs[REG_REP_DELAY] = (data & REG_MASK_REP_DELAY);
		break;
	case REG_REP_TIME:
		// key repeat time
		m_regs[REG_REP_TIME] = (data & REG_MASK_REP_TIME);
		break;
	case REG_LED_STAT:
		// change led status
		m_regs[REG_LED_STAT] = (data & REG_MASK_LED_STAT);
		break;
	default:
		return false;
		break;
	}
	return true;
}

void KEYBOARD::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, _T("KEYBOARD:\n"));

	for(int i=0; i<REGS_END; i++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" %X(%-10s):%02X"), i, c_reg_names[i], m_regs[i]);
		if ((i % 4) == 3) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
}

#endif

