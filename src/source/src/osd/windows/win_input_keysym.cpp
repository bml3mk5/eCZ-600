/** @file win_input_keysym.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.04.01

	@brief [ win32 input keysym ]
*/

#include "win_emu.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "win_input.h"

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input  bit2:1:avail dinput
/// @param[in] code VK code
/// @param[in] status scan_code and modifier
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
{
	short scan_code = (short)((status & 0x1ff0000) >> 16);
	return translate_keysym(type, code, scan_code, new_code, new_keep_frames);
}

/// @param[in] type bit0:0:down 1:up  bit1:0:WM_KEYDOWN/UP 1:direct_input  bit2:1:avail dinput
/// @param[in] code VK code
/// @param[in] scan_code
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames)
{
	int n_code = 0;
	bool n_keep_frames = false;

#ifdef USE_DIRECTINPUT
	if (!(type & 2)) {
		if (type & 4) {
			if (code < 256) {
				n_code = vkkey2keycode[code];
				n_keep_frames = ((n_code & KEYCODE_KEEP_FRAMES) != 0);
				n_code &= 0xff;
				if (n_code == 0) return type;
			}
		} else
#endif
		{
			if(code == VK_SHIFT) {
				if(scan_code == 0x36 || GetKeyState(VK_RSHIFT) & 0x8000) n_code = KEYCODE_RSHIFT;
				else n_code = KEYCODE_LSHIFT;
			}
			else if(code == VK_CONTROL) {
				if((scan_code & 0x100) || GetKeyState(VK_RCONTROL) & 0x8000) n_code = KEYCODE_RCTRL;
				else n_code = KEYCODE_LCTRL;
			}
			else if(code == VK_MENU) {
				if((scan_code & 0x100) || GetKeyState(VK_RMENU) & 0x8000) n_code = KEYCODE_RALT;
				else n_code = KEYCODE_LALT;
			}
			else if(code == VK_RETURN) {
				if (scan_code & 0x100) n_code = KEYCODE_KP_ENTER;
				else n_code = KEYCODE_RETURN;
			}
			else if (code < 256) {
				n_code = vkkey2keycode[code];
				n_keep_frames = ((n_code & KEYCODE_KEEP_FRAMES) != 0);
				n_code &= 0xff;
			}
		}

		// convert numpad keys
		if (scan_code >= 0x47 && scan_code <= 0x53) {
			if (scancode2keycode47[scan_code - 0x47] != 0) n_code = scancode2keycode47[scan_code - 0x47];
		}
		else if (scan_code >= 0x70 && scan_code <= 0x7f) {
			if (scancode2keycode70[scan_code - 0x70] != 0) n_code = scancode2keycode70[scan_code - 0x70];
		}
//		else if (scan_code == 0x29) {
//			// code is set either Hankaku or Kanji in JP
//			n_code = KEYCODE_GRAVE;
//			n_keep_frames = true;
//			type &= ~1; // always key down
//		}
#ifdef USE_DIRECTINPUT
	} else {
		n_code = dikey2keycode[code];
		n_keep_frames = ((n_code & KEYCODE_KEEP_FRAMES) != 0);
		n_code &= 0xff;
	}
#endif

	if (n_code == 0) {
		// unknown key code
		n_code = code | KEYCODE_EXTENDED;
	}

	if (n_code == KEYCODE_LSHIFT || n_code == KEYCODE_RSHIFT) {
		BIT_ONOFF(key_mod, KEY_MOD_SHIFT_KEY, (type & 1) == 0);
	}

	if (new_code) *new_code = n_code;
	if (new_keep_frames) *new_keep_frames = n_keep_frames;

//	out_debugf("keytrans: %d %02x->%02x %04x keep:%s", type, code, n_code, scan_code, n_keep_frames ? "true" : "false");

	return (type & 1);
}
