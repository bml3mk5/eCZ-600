/** @file emu_input.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for X68000 by Sasaji at 2022.02.22

	@brief [ emu input ]
*/

#include "../emu.h"
#include "../vm/vm.h"
#include "../config.h"
#include <stdlib.h>
#include "../fifo.h"
#include "../fileio.h"
#include "emu_input.h"
#include "../utility.h"

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
#ifndef USE_PIAJOYSTICKBIT
///
const int EMU::joy_allow_map[16] = {
	-1, 0, 1, -1, 2, 3, 4, -1, 5, 6, 7, -1, -1, -1, -1, -1
};
#endif
#endif

void EMU::EMU_INPUT()
{
	vm_key_status = NULL;
	vm_key_status_size = 0;

	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	clear_vm_key_map();

	vm_key_history = new FIFOINT(64);

#ifdef USE_KEY2JOYSTICK
	clear_key2joy_map();
	memset(key2joy_status, 0, sizeof(key2joy_status));
#endif

#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
	memset(joy_mashing_mask, 0xff, sizeof(joy_mashing_mask));
	joy_mashing_count = 0;
#endif

#ifdef USE_JOYSTICK
	clear_joy2joy_map();
	memset(joy2joy_status, 0, sizeof(joy2joy_status));
	// joy_num = 0;
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy_enabled[i] = false;
	}
#endif

#ifdef USE_AUTO_KEY
	// initialize autokey
	autokey_buffer = NULL;
	autokey_phase = 0;
	autokey_shift = 0;
	autokey_enabled = false;
#endif
}

void EMU::initialize_input()
{
	logging->out_debug(_T("EMU::initialize_input"));

#ifdef USE_KEY2JOYSTICK
	key2joy_enabled = FLG_USEKEY2JOYSTICK ? true : false;
#endif
#ifdef USE_JOYSTICK
	use_joystick = FLG_USEJOYSTICK_ALL ? true : false;
#endif
	// initialize joysticks
	initialize_joystick();

	// initialize key to joystick
	initialize_key2joy();

	// mouse emulation is disabled
	mouse_disabled = 1;
	initialize_mouse(FLG_USEMOUSE ? true : false);

#ifdef USE_SHIFT_NUMPAD_KEY
	// initialize shift+numpad conversion
	memset(key_converted, 0, sizeof(key_converted));
	key_shift_pressed = key_shift_released = false;
#endif
#ifdef USE_AUTO_KEY
	// initialize autokey
	autokey_buffer = new FIFOINT(65536);
//	autokey_buffer->clear();
	autokey_phase = 31;	// wait a few sec.
	autokey_shift = 0;
	autokey_enabled = false;
#endif
	lost_focus = false;
	key_mod = 0;
}

void EMU::initialize_joystick()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	modify_joy_mashing();
#endif

#ifdef USE_JOYSTICK
	for(int i=0; i<MAX_JOYSTICKS; i++) {
		joy_enabled[i] = false;
	}
	reset_joystick();
#endif
}

void EMU::initialize_key2joy()
{
#ifdef USE_KEY2JOYSTICK
	reset_key2joy();
#endif
}

void EMU::reset_joystick()
{
}

void EMU::reset_key2joy()
{
}

void EMU::set_joy_range(bool enable, int mintd, int maxtd, int threshold, struct st_joy_range &out)
{
#ifdef USE_JOYSTICK
	int offset = (mintd + 1 + maxtd) / 2;
	int range = (maxtd + 1 - mintd);
	out.enable = enable;
	out.offset = offset;
	out.range = range;
	if (enable) {
		range = range * threshold / 20;
	} else {
		range /= 2;
	}
	out.mintd = - range + offset;
	out.maxtd = range + offset;
#endif
}

void EMU::set_joy_threshold(int threshold, struct st_joy_range &out)
{
#ifdef USE_JOYSTICK
	bool enable = out.enable;
	int offset = out.offset;
	int range = out.range;
	if (enable) {
		range = range * threshold / 20;
	} else {
		range /= 2;
	}
	out.mintd = - range + offset;
	out.maxtd = range + offset;
#endif
}

void EMU::release_input()
{
	// release buffer
	if (vm_key_history) {
		delete vm_key_history;
		vm_key_history = NULL;
	}

	// release mouse
	if(!mouse_disabled) {
		disable_mouse(0);
	}

#ifdef USE_AUTO_KEY
	// release autokey buffer
	if(autokey_buffer) {
//		autokey_buffer->release();
		delete autokey_buffer;
		autokey_buffer = NULL;
	}
#endif
}

void EMU::release_joystick()
{
#ifdef USE_JOYSTICK
	// release joystick
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
		joy_enabled[i] = false;
	}
	memset(joy2joy_status, 0, sizeof(joy2joy_status));
#endif
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
#endif
}

void EMU::release_key2joy()
{
#ifdef USE_KEY2JOYSTICK
	memset(key2joy_status, 0, sizeof(key2joy_status));
#endif
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	memset(joy_status, 0, sizeof(joy_status));
#endif
}

void EMU::convert_joy_status(int num)
{
#ifdef USE_JOYSTICK
	// convert
	for(int n = 0; n <= joy2joy_map_size && n < 32; n++) {
		uint32_t code = joy2joy_map[num][n];
		if (joy2joy_status[num][0] & code) {
			joy_status[num][0] |= joy2joy_idx[n];
		}
	}
	joy_status[num][1] = joy2joy_status[num][1];

#ifdef USE_ANALOG_JOYSTICK
	for(int n=2; n<8; n++) {
		int kk = joy2joy_ana_map[num][n-2];
		joy_status[num][n] = joy2joy_status[num][kk];
		if (joy2joy_ana_rev[num][n-2]) joy_status[num][n] = 1023 - joy_status[num][n];
	}
#endif
#endif
}

void EMU::update_input()
{
}

void EMU::update_mouse()
{
}

void EMU::update_autokey()
{
#ifdef USE_AUTO_KEY
	// auto key
	switch(autokey_phase) {
	case 1:
		if(autokey_buffer && !autokey_buffer->empty()) {
#if 0
			// update graph key status
			int graph = autokey_buffer->read_not_remove(0) & AUTO_KEY_GRAPH_MASK;
			if(graph && !autokey_shift) {
				vm_key_down(AUTO_KEY_GRAPH, VM_KEY_STATUS_AUTOKEY);
				logging->out_debugf(_T("autokey phase:% 2d: graph down"), autokey_phase);
				autokey_shift = graph;
				autokey_phase++;
				break;
			}
#endif
			// update shift key status
			int shift = autokey_buffer->read_not_remove(0) & AUTO_KEY_SHIFT_MASK;
			if(shift && !autokey_shift) {
				vm_key_down(AUTO_KEY_SHIFT, VM_KEY_STATUS_AUTOKEY);
				logging->out_debugf(_T("autokey phase:% 2d: shift down"), autokey_phase);
				autokey_shift = shift;
				autokey_phase++;
				break;
			}
			autokey_shift = 0;
		}
		autokey_phase++;
		break;
	case 2:
		if(autokey_buffer && !autokey_buffer->empty()) {
			autokey_code = autokey_buffer->read_not_remove(0) & AUTO_KEY_MASK;
			vm_key_down(autokey_code, VM_KEY_STATUS_AUTOKEY);
			logging->out_debugf(_T("autokey phase:% 2d: key down %x"), autokey_phase, autokey_code);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY:
		if(!autokey_buffer || autokey_buffer->empty()) {
			autokey_phase = 30;
		} else {
			// wait response from vm
			logging->out_debugf(_T("autokey phase:% 2d: wait "), autokey_phase);
		}
		break;
	case USE_AUTO_KEY + 2:
		if(autokey_shift & AUTO_KEY_SHIFT_MASK) {
			vm_key_up(AUTO_KEY_SHIFT, VM_KEY_STATUS_AUTOKEY);
			logging->out_debugf(_T("autokey phase:% 2d: shift up:"), autokey_phase);
		}
#if 0
		if(autokey_shift & AUTO_KEY_GRAPH_MASK) {
			vm_key_up(AUTO_KEY_GRAPH, VM_KEY_STATUS_AUTOKEY);
			logging->out_debugf(_T("autokey phase:% 2d: graph up"), autokey_phase);
		}
#endif
		autokey_shift = 0;
		if(autokey_buffer && !autokey_buffer->empty()) {
			vm_key_up(autokey_buffer->read_not_remove(0) & AUTO_KEY_MASK, VM_KEY_STATUS_AUTOKEY);
		}
		logging->out_debugf(_T("autokey phase:% 2d: key up %x"), autokey_phase, autokey_code);
//		autokey_code = 0;
		autokey_phase++;
		break;
	case USE_AUTO_KEY + 3:
		if(!autokey_buffer || autokey_buffer->empty()) {
			autokey_phase = 30;
		} else {
			// wait response from vm
			logging->out_debugf(_T("autokey phase:% 2d: wait "), autokey_phase);
		}
		break;
	case USE_AUTO_KEY + 4:
		autokey_code = 0;
		if(autokey_buffer && !autokey_buffer->empty()) {
			// wait enough while vm analyzes one line
			int code = autokey_buffer->read();
			logging->out_debugf(_T("autokey phase:% 2d: code %x"), autokey_phase, code);
			if(code == AUTO_KEY_RETURN) {
				autokey_phase++;	// wait 30 frames
				break;
			}
		}
		// through
	case 30:
	case 150:
		if(autokey_buffer && !autokey_buffer->empty()) {
			autokey_phase = 1;
		}
		else {
			stop_auto_key();
			autokey_phase = 0;
		}
		break;
	default:
		if(autokey_phase) {
			autokey_phase++;
		}
		break;
	}
#endif
}

/// @brief vm must call this function when cpu scanned the key i/o port in vm
void EMU::recognized_key(uint16_t key_code)
{
#ifdef USE_AUTO_KEY
	switch(autokey_phase) {
	case USE_AUTO_KEY:
		if(autokey_code) {
			logging->out_debugf(_T("recognized_key: phase:%d auto_key:%x vm_key:%x phase change"),autokey_phase,autokey_code,key_code);
			if (autokey_code == key_code) {
				autokey_phase = USE_AUTO_KEY + 1;
			}
		}
		break;
	case USE_AUTO_KEY + 3:
		if (autokey_code) {
			logging->out_debugf(_T("recognized_key: phase:%d auto_key:%x vm_key:%x phase change"),autokey_phase,autokey_code,key_code);
			if (autokey_code == key_code) {
				autokey_phase =  USE_AUTO_KEY + 4;
			}
		}
		break;
	case 0:
		break;
	default:
//		if (autokey_code) {
//			logging->out_debugf(_T("recognized_key: phase:%d auto_key:%x vm_key:%x keep"),autokey_phase,autokey_code,key_code);
//		}
		break;
	}
#endif
}

void EMU::update_joystick()
{
}

int EMU::key_down_up(uint8_t type, int code, long status)
{
	return 1;
}

/// @brief key pressed
int EMU::key_down(int code, bool keep_frames)
{
#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
		if (key2joy_down(code)) {
			return code;
		}
	}
#endif

	key_status[code] = keep_frames ? (KEY_KEEP_FRAMES * FRAME_SPLIT_NUM) : 0x80;
	vm_key_down(vm_key_map[code], VM_KEY_STATUS_KEYBOARD);
#ifdef NOTIFY_KEY_DOWN_TO_GUI
	gui->KeyDown(code, mod);
#endif
#ifdef NOTIFY_KEY_DOWN
	vm->key_down(code);
#endif
	return code;
}

/// @brief key released
void EMU::key_up(int code, bool keep_frames)
{
	key_status[code] &= 0x7f;
	vm_key_up(vm_key_map[code], VM_KEY_STATUS_KEYBOARD);
#ifdef NOTIFY_KEY_DOWN_TO_GUI
	gui->KeyUp(code);
#endif
#ifdef NOTIFY_KEY_DOWN
	if(!key_status[code]) {
		vm->key_up(code);
	}
#endif
#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
		key2joy_up(code);
	}
#endif
}

/// @brief clear the vm key status
void EMU::clear_vm_key_status(uint8_t mask)
{
	if (!vm_key_status) return;
	for(int code = 0; code < vm_key_status_size; code++) {
		if (vm_key_status[code] & mask) {
			vm_key_status[code] &= ~mask;
			vm_key_history->write(code);
		}
	}
}

/// @brief set the vm key status as pressed
/// @param[in] code
/// @param[in] mask : flags
void EMU::vm_key_down(int code, uint8_t mask)
{
	if (0 <= code && vm_key_status) {
		vm_key_status[code] |= mask;
		vm_key_history->write(code);
	}
}

/// @brief set the vm key status as released
/// @param[in] code
/// @param[in] mask : flags
void EMU::vm_key_up(int code, uint8_t mask)
{
	if (0 <= code && vm_key_status) {
		vm_key_status[code] &= ~mask;
		vm_key_history->write(code);
	}
}

/// @brief set the virtual key status as pressed
/// @param[in] code
/// @param[in] mask : flags
void EMU::vkey_key_down(int code, uint8_t mask)
{
#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
		const uint32_t *scan2key = (const uint32_t *)emu->get_paramv(VM::ParamVkKeyDefMap0);
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			int vk_code = scan2key[code * KEYBIND_ASSIGN + i];
			if (key2joy_down(vk_code)) {
				return;
			}
		}
	}
#endif

	vm_key_down(code, mask);
}

/// @brief set the virtual key status as released
/// @param[in] code
/// @param[in] mask : flags
void EMU::vkey_key_up(int code, uint8_t mask)
{
	vm_key_up(code, mask);

#ifdef USE_KEY2JOYSTICK
	if (key2joy_enabled) {
		const uint32_t *scan2key = (const uint32_t *)emu->get_paramv(VM::ParamVkKeyDefMap0);
		for(int i=0; i<KEYBIND_ASSIGN; i++) {
			int vk_code = scan2key[code * KEYBIND_ASSIGN + i];
			key2joy_up(vk_code);
		}
	}
#endif
}

/// @brief clear the vm key mapping table
void EMU::clear_vm_key_map()
{
	for(uint32_t i=0; i<KEY_STATUS_SIZE; i++) {
		vm_key_map[i] = -1;
	}
}

/// @brief set the vm key mapping table
void EMU::set_vm_key_map(uint32_t key_code, int vm_scan_code)
{
	if (0 < key_code && key_code < KEY_STATUS_SIZE) {
		vm_key_map[key_code] = vm_scan_code;
	}
}

#if 0
/// @brief get the vm key mapping table
int EMU::get_vm_key_map(uint32_t key_code) const
{
	if (0 < key_code && key_code < KEY_STATUS_SIZE) {
		return vm_key_map[key_code];
	}
	return 0;
}
#endif

void EMU::modify_joy_threshold()
{
#if defined(USE_JOYSTICK)
	for(int i = 0; i < MAX_JOYSTICKS; i++) {
		set_joy_threshold(pConfig->joy_axis_threshold[i][0], joy_prm[i].x);
		set_joy_threshold(pConfig->joy_axis_threshold[i][1], joy_prm[i].y);
		set_joy_threshold(pConfig->joy_axis_threshold[i][2], joy_prm[i].z);
		set_joy_threshold(pConfig->joy_axis_threshold[i][3], joy_prm[i].r);
		set_joy_threshold(pConfig->joy_axis_threshold[i][4], joy_prm[i].u);
		set_joy_threshold(pConfig->joy_axis_threshold[i][5], joy_prm[i].v);
	}
#endif
}

void EMU::modify_joy_mashing()
{
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	for(int n=0; n<16; n++) {
		for(int i=0; i<MAX_JOYSTICKS; i++) {
			joy_mashing_mask[i][n] = 0xffffffff;
			for(int k=0; k<KEYBIND_JOY_BUTTONS && k<16; k++) {
				if (!pConfig->joy_mashing[i][k]) continue;

				int nn = 1 << (4 - pConfig->joy_mashing[i][k]);
				if (n & nn) {
					joy_mashing_mask[i][n] &= ~(0x10000 << k);
				}
			}
		}
	}
#endif
}

void EMU::clear_joy2joy_idx()
{
#ifdef USE_JOYSTICK
	memset(joy2joy_idx, 0, sizeof(joy2joy_idx));
#endif
}

void EMU::set_joy2joy_idx(int pos, uint32_t joy_code)
{
#ifdef USE_JOYSTICK
	if (pos < 0 || pos >= 32) return;
	joy2joy_idx[pos] = joy_code;
#endif
}

/// @brief clear the joypad button mapping table
void EMU::clear_joy2joy_map()
{
#ifdef USE_JOYSTICK
	joy2joy_map_size = 0;
	memset(joy2joy_map, 0, sizeof(joy2joy_map));
#ifdef USE_ANALOG_JOYSTICK
	for(int i=0; i<2; i++) {
		for(int n=0; n<6; n++) {
			joy2joy_ana_map[i][n] = n + 2;
			joy2joy_ana_rev[i][n] = false;
		}
	}
#endif
#endif
}

/// @brief set the joypad button mapping table
void EMU::set_joy2joy_map(int num, int pos, uint32_t joy_code)
{
#ifdef USE_JOYSTICK
	if (num < 0 || num >= MAX_JOYSTICKS) return;
	if (pos < 0 || pos >= 32) return;
	joy2joy_map[num][pos] = joy_code;
	if (joy_code != 0 && pos > joy2joy_map_size) joy2joy_map_size = pos;
#endif
}

#if 0
/// @brief get the joypad button mapping table
uint32_t EMU::get_joy2joy_map(int num, int pos) const
{
#ifdef USE_JOYSTICK
	if (num < 0 || num >= MAX_JOYSTICKS) return 0;
	if (pos < 0 || pos >= 32) return 0;
	return joy2joy_map[num][pos];
#endif
	return 0;
}
#endif

void EMU::set_joy2joy_ana_map(int num, int pos, uint32_t joy_code)
{
#if defined(USE_JOYSTICK) && defined(USE_ANALOG_JOYSTICK)
	if (num < 0 || num >= MAX_JOYSTICKS) return;
	if (pos < 0 || pos >= 6) return;
	int n = -1;
	bool rev = false;
	switch(joy_code) {
	case JOYCODE_ANA_X_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_X:
		n = 0;
		break;
	case JOYCODE_ANA_Y_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_Y:
		n = 1;
		break;
	case JOYCODE_ANA_Z_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_Z:
		n = 2;
		break;
	case JOYCODE_ANA_R_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_R:
		n = 3;
		break;
	case JOYCODE_ANA_U_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_U:
		n = 4;
		break;
	case JOYCODE_ANA_V_REV:
		rev = true;
		// [through]
	case JOYCODE_ANA_V:
		n = 5;
		break;
	}
	if (n >= 0) {
		joy2joy_ana_map[num][pos] = n + 2;
		joy2joy_ana_rev[num][pos] = rev;
	}
#endif
}

/// @brief clear the key2joypad mapping table
void EMU::clear_key2joy_map()
{
#ifdef USE_KEY2JOYSTICK
	memset(key2joy_map, 0, sizeof(key2joy_map));
#endif
}

/// @brief set the key2joypad mapping table
void EMU::set_key2joy_map(uint32_t key_code, int num, uint32_t joy_code)
{
#ifdef USE_KEY2JOYSTICK
	if (0 < key_code && key_code < KEY_STATUS_SIZE) {
#ifndef USE_PIAJOYSTICKBIT
		if (joy_code & 0x80000000) {
			joy_code &= 0x7fffffff;
			joy_code = (0x10000 << joy_code);
		}
		key2joy_map[num][key_code] |= joy_code;
#else
		if (num != 0) return;
		if (joy_code >= 8) return;
		key2joy_map[key_code] = (1 << joy_code);
#endif
	}
#endif
}

/// @brief get the key2joypad mapping table
uint8_t EMU::get_key2joy_map(uint32_t key_code) const
{
#ifdef USE_KEY2JOYSTICK
	if (0 < key_code && key_code < KEY_STATUS_SIZE) {
#ifndef USE_PIAJOYSTICKBIT
		return key2joy_map[0][key_code];
#else
		return key2joy_map[key_code];
#endif
	}
#endif
	return 0;
}

/// @brief convert pressed key to joypad status
/// @return true: key pressed
bool EMU::key2joy_down(int code)
{
#ifdef USE_KEY2JOYSTICK
	if (key2joy_map[code]) {
#ifdef USE_PIAJOYSTICKBIT
		key2joy_status[0] |= key2joy_map[code];
		vm_key_down(vm_key_map[code], VM_KEY_STATUS_KEY2JOY);
		return true;
#else
		for(int i=0; i<MAX_JOYSTICKS; i++) {
			uint32_t joy_code = key2joy_map[i][code];
			uint32_t k = (joy_code & 0xf);
			int kk = joy_allow_map[k];
			if (kk >= 0) key2joy_status[i][kk] |= k;
			key2joy_status[i][8] |= (joy_code & 0xfffffff0);
		}
		vm_key_down(vm_key_map[code], VM_KEY_STATUS_KEY2JOY);
		return true;
#endif
	}
#endif
	return false;
}

/// @brief convert released key to joypad status
void EMU::key2joy_up(int code)
{
#ifdef USE_KEY2JOYSTICK
	if (key2joy_map[code]) {
#ifdef USE_PIAJOYSTICKBIT
		key2joy_status[0] &= ~key2joy_map[code];
		vm_key_up(vm_key_map[code], VM_KEY_STATUS_KEY2JOY);
#else
		for(int i=0; i<MAX_JOYSTICKS; i++) {
			uint32_t joy_code = key2joy_map[i][code];
			uint32_t k = (joy_code & 0xf);
			int kk = joy_allow_map[k];
			if (kk >= 0) key2joy_status[i][kk] &= ~k;
			key2joy_status[i][8] &= ~(joy_code & 0xfffffff0);
		}
		vm_key_up(vm_key_map[code], VM_KEY_STATUS_KEY2JOY);
#endif
	}
#endif
}

#ifdef USE_BUTTON
void EMU::press_button(int num)
{
	int code = buttons[num].code;

	if(code) {
		key_down(code, false);
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
	}
	else {
		// code=0: reset virtual machine
		vm->reset();
	}
}
#endif

void EMU::initialize_mouse(bool enable)
{
	if (enable) enable_mouse(0);
}

void EMU::enable_mouse(int mode)
{
}

void EMU::disable_mouse(int mode)
{
}

void EMU::toggle_mouse()
{
	// toggle mouse enable / disable
	if(mouse_disabled & 1) {
		enable_mouse(0);
	} else {
		disable_mouse(0);
	}
}

bool EMU::get_mouse_enabled()
{
	return ((mouse_disabled & 1) == 0);
}

#if 0 // def USE_MOUSE_ABSOLUTE
void EMU::set_mouse_position()
{
}
#endif

void EMU::mouse_enter()
{

}

void EMU::mouse_move(int x, int y)
{

}

void EMU::mouse_leave()
{

}

void EMU::enable_joystick()
{
}

#ifdef USE_AUTO_KEY
bool EMU::start_auto_key(const char *str)
{
	bool rc = true;

	stop_auto_key();

	autokey_code = 0;
	autokey_buffer->clear();

	int size = (int)strlen(str);
	int prev_code = 0;
	int prev_mod = 0;
	parse_auto_key(str, size, prev_code, prev_mod);
	parsed_auto_key(prev_code, prev_mod);

	update_config();

	return rc;
}

bool EMU::open_auto_key(const _TCHAR *file_path)
{
	FILEIO *fp;
	char buf[1025];
	bool rc = true;

	stop_auto_key();

	autokey_code = 0;
	fp = new FILEIO();
	int prev_mod = 0;
	if (fp->Fopen(file_path, FILEIO::READ_BINARY)) {
		autokey_buffer->clear();
		int prev_code = 0;
		int size = 0;
		while((size = (int)fp->Fread(buf, sizeof(char), 1024)) > 0) {
			parse_auto_key(buf, size, prev_code, prev_mod);
		}
		fp->Fclose();

		parsed_auto_key(prev_code, prev_mod);

		pConfig->opened_autokey_path.Set(file_path, 0);
	} else {
		logging->out_log(LOG_ERROR, _T("Auto key file couldn't be opened."));
		rc = false;
	}
	delete fp;

	pConfig->initial_autokey_path.SetFromPath(file_path);

	update_config();

	return rc;
}

void EMU::parse_auto_key(const char *buf, int size, int &prev_code, int &prev_mod)
{
	for(int i = 0; i < size; i++) {
		int code = buf[i] & 0xff;
#if 0
		if(prev_code == 0xd && code == 0xa) {
			prev_code = code;
			continue;	// cr-lf
		}
#else
		if (prev_code == 0xd && code == 0xa) {
			// cr-lf
			prev_code = code;
			continue;
		}
		if ((0x81 <= prev_code && prev_code <= 0x9f) || 0xe0 <= prev_code) {
			if (0x20 <= code) {
				// Shift-JIS
				if(!(prev_mod & AUTO_KEY_KANJI_MASK)) {
					autokey_buffer->write(AUTO_KEY_CODEINPUT);
					prev_mod |= AUTO_KEY_KANJI_MASK;
				}
				code = UTILITY::sjis_to_jis(prev_code << 8 | code);
				for(int i=0; i<4; i++) {
					int ch = ((code & 0xf000) >> 12) + 0x30;
					if (ch >= 0x3a) ch += 7;
					ch = autokey_table[ch];
					autokey_buffer->write(ch);
					code <<= 4;
				}
				prev_code = 0;
				continue;
			}
		} else if ((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
			// Shift-JIS?
			prev_code = code;
			continue;
		}
#endif
		if(prev_mod & AUTO_KEY_KANJI_MASK) {
			autokey_buffer->write(AUTO_KEY_CODEINPUT);
			prev_mod &= ~AUTO_KEY_KANJI_MASK;
		}

		prev_code = code;

		if((code = autokey_table[code]) != 0) {
#if 0
			int kana = code & (AUTO_KEY_KATA_MASK | AUTO_KEY_HIRA_MASK);
			int prev_kana = prev_mod & (AUTO_KEY_KATA_MASK | AUTO_KEY_HIRA_MASK);
			if(kana - prev_kana == AUTO_KEY_KATA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
			} else if(kana - prev_kana == AUTO_KEY_HIRA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
				autokey_buffer->write(AUTO_KEY_KANA);
			}
			if(prev_kana - kana == AUTO_KEY_HIRA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
			} else if(prev_kana - kana == AUTO_KEY_KATA_MASK) {
				autokey_buffer->write(AUTO_KEY_KANA);
				autokey_buffer->write(AUTO_KEY_KANA);
			}
#else
			if((prev_mod ^ code) & AUTO_KEY_KANA_MASK) {
				// hankaku katakana
				autokey_buffer->write(AUTO_KEY_KANA);
			}
#endif
			prev_mod = code & ~AUTO_KEY_MASK;

#if defined(USE_AUTO_KEY_NO_CAPS)
			if((code & AUTO_KEY_SHIFT_MASK) && !(code & (AUTO_KEY_UPPER_MASK | AUTO_KEY_LOWER_MASK))) {
#elif defined(USE_AUTO_KEY_CAPS)
			if(code & (AUTO_KEY_SHIFT_MASK | AUTO_KEY_LOWER_MASK)) {
#else
			if(code & (AUTO_KEY_SHIFT_MASK | AUTO_KEY_UPPER_MASK)) {
#endif
				autokey_buffer->write((code & AUTO_KEY_MASK) | AUTO_KEY_SHIFT_MASK);
			}
#if 0
			else if (code & AUTO_KEY_GRAPH_MASK) {
				autokey_buffer->write((code & AUTO_KEY_MASK) | AUTO_KEY_GRAPH_MASK);
			}
#endif
			else {
				autokey_buffer->write(code & AUTO_KEY_MASK);
			}
		}
	}
}

void EMU::parsed_auto_key(int prev_code, int prev_mod)
{
	if(prev_mod & AUTO_KEY_KANA_MASK) {
		autokey_buffer->write(AUTO_KEY_KANA);
	}
	if(prev_mod & AUTO_KEY_KANJI_MASK) {
		autokey_buffer->write(AUTO_KEY_CODEINPUT);
	}

	autokey_phase = (autokey_phase > 30 ? autokey_phase : 1);
	autokey_shift = 0;
	autokey_enabled = true;
}

void EMU::stop_auto_key()
{
#if 1
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_SHIFT, VM_KEY_STATUS_AUTOKEY);
	}
	if (vm_key_status) {
		for (int i=0; i<vm_key_status_size; i++) {
			vm_key_status[i] &= ~VM_KEY_STATUS_AUTOKEY;
		}
	}
#else
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_SHIFT, VM_KEY_STATUS_AUTOKEY);
	}
	if(autokey_shift) {
		vm_key_up(AUTO_KEY_GRAPH, VM_KEY_STATUS_AUTOKEY);
	}
	if (vm_key_status) {
		for (int i=0; i<vm_key_status_size; i++) {
			vm_key_status[i] &= ~VM_KEY_STATUS_AUTOKEY;
		}
	}
#endif
	autokey_phase = (autokey_phase > 30 ? autokey_phase : 0);
	autokey_shift = 0;
	autokey_enabled = false;

	pConfig->opened_autokey_path.Clear();

	update_config();
}
#endif /* USE_AUTO_KEY */
