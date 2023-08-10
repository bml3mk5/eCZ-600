/** @file emu_input_keysym.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ emu input keysym ]
*/

#include "../emu.h"
#include "../depend.h"
#include "../vm/vm.h"
#include "../gui/gui.h"
#include "emu_input.h"

/// notify accel key
int EMU::system_key_down(int code)
{
	if (vm) {
//      translate to global key in EMU::key_down_up
//		code = translate_global_key(code);
		vm->system_key_down(code);
	}
	return code;
}
void EMU::system_key_up(int code)
{
	if (vm) {
//      translate to global key in EMU::key_down_up
//		code = translate_global_key(code);
		vm->system_key_up(code);
	}
}

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input  bit2:1:avail dinput
/// @param[in] code VK code
/// @param[in] status scan_code and modifier
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
{
	return (type & 1);
}

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input  bit2:1:avail dinput
/// @param[in] code VK code
/// @param[in] scan_code
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU::translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames)
{
	return (type & 1);
}

/// execute accel key
bool EMU::execute_global_keys(int code, uint32_t status)
{
	if (gui) {
//      translate to global key in EMU::key_down_up
//		code = translate_global_key(code);
		return gui->ExecuteGlobalKeys(code, status);
	}
	return false;
}

/// release accel key
bool EMU::release_global_keys(int code, uint32_t status)
{
	if (gui) {
//      translate to global key in EMU::key_down_up
//		code = translate_global_key(code);
		return gui->ReleaseGlobalKeys(code, status);
	}
	return false;
}

/// translate
int EMU::translate_global_key(int code)
{
	if (code == KEYCODE_LCTRL || code == KEYCODE_RCTRL) {
		code = GLOBALKEY_CONTROL;
	} else if (code == KEYCODE_RETURN) {
		code = GLOBALKEY_RETURN;
	} else if (code == KEYCODE_0) {
		code = GLOBALKEY_0;
	} else if (KEYCODE_1 <= code && code <= KEYCODE_9) {
		code = code + GLOBALKEY_1 - KEYCODE_1;
	} else if (KEYCODE_A <= code && code <= KEYCODE_Z) {
		code = code + GLOBALKEY_A - KEYCODE_A;
	} else if (KEYCODE_F1 <= code && code <= KEYCODE_F12) {
		code = code + GLOBALKEY_F1 - KEYCODE_F1;
	} else if (KEYCODE_F13 <= code && code <= KEYCODE_F15) {
		code = code + GLOBALKEY_F13 - KEYCODE_F13;
	}
	if (key_mod & KEY_MOD_SHIFT_KEY) {
		if (GLOBALKEY_0 <= code && code <= GLOBALKEY_9) {
			code = code + GLOBALKEY_SFT_0 - GLOBALKEY_0;
		}
	}
	return code;
}

/// post message to main thread
void EMU::post_command_message(int id)
{
	if (gui) gui->PostCommandMessage(id);
}
