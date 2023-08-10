/** @file gtk_input_keysym.cpp

	Skelton for retropc emulator
	GTK+ + SDL edition

	@author Sasaji
	@date   2017.01.27

	@brief [ gtk input keysym ]
*/

#include "../../emu_osd.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "gtk_input.h"

#if 0
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
{
	short scan_code = (short)((status & 0x1ff0000) >> 16);
	return translate_keysym(type, code, scan_code, new_code, new_keep_frames);
}
#endif

/// @param[in] type bit0:0:down 1:up
/// @param[in] code native code
/// @param[in] scan_code native key scan code
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames)
{
	int n_code = 0;
	bool n_keep_frames = false;
	uint8_t n_type = type;

#ifdef USE_GTK
	if (code < 128) {
		n_code = gtkcode2keycode1[code & 0x7f];
	}
    if ((code & 0xffff00) == 0xff00) {
		n_code = gtkcode2keycode2[code & 0xff];
	}
	if (n_code == 0) {
		for(const sc2kc_t *p = scancode2keycode; p->sc >= 0; p++) {
			if (p->sc == scan_code) {
				n_code = p->kc;
				break;
			}
		}
	}
#endif /* USE_GTK */

//	out_debugf(_T("trans_key: code:%02x -> %02x scan:%02x"),code,n_code,scan_code);

	if (n_code == KEYCODE_LSHIFT || n_code == KEYCODE_RSHIFT) {
		BIT_ONOFF(key_mod, KEY_MOD_SHIFT_KEY, (type & 1) == 0);
	}

	if (new_code) *new_code = n_code;
	if (new_keep_frames) *new_keep_frames = n_keep_frames;

	return n_type;
}
