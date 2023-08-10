/** @file sdl_input_keysym.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.03.08

	@brief [ sdl input keysym ]
*/

#include "sdl_emu.h"
#include "../../vm/vm.h"
#include <SDL.h>
#include "../../gui/gui.h"
#include "sdl_input.h"

#if 0
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, long status, int *new_code, bool *new_keep_frames)
{
	short scan_code = (short)((status & 0x1ff0000) >> 16);
	return translate_keysym(type, code, scan_code, new_code, new_keep_frames);
}
#endif

/// @param[in] type bit0:0:down 1:up
/// @param[in] code SDL keycode
/// @param[in] scan_code native key scan code 
/// @param[out] new_code
/// @param[out] new_keep_frames
uint8_t EMU_OSD::translate_keysym(uint8_t type, int code, short scan_code, int *new_code, bool *new_keep_frames)
{
	int n_code = 0;
	bool n_keep_frames = false;
	uint8_t n_type = type;

#ifdef USE_SDL2

	if (scan_code < 256) {
		n_code = scancode2keycode[scan_code];
	}
	if (n_code == 0) {
		n_code = (scan_code & (KEY_STATUS_SIZE-1)) | KEYCODE_EXTENDED;
	}

#else /* !USE_SDL2 */

#if defined(_WIN32)
	/* Windows */
	if(code == 0xf0) {
		n_code = KEYCODE_CAPSLOCK;
		n_keep_frames = true;
	} else if(code == 0xf2) {
		n_code = KEYCODE_KATAHIRA;
		n_keep_frames = true;
	} else {
		if (code >= SDLK_UP && code <= SDLK_NUMLOCK) {
			n_code = sdlcode2keycode2[code - 256];
		} else if (scan_code < 128) {
			n_code = scancode2keycode[scan_code];
		} else {
			for(const sc2kc_t *p = scancode2keycode2; p->sc >= 0; p++) {
				if (p->sc == scan_code) {
					n_code = p->kc;
					break;
				}
			}
		}
	}
	if (n_code == 0) {
		n_code = code | KEYCODE_EXTENDED;
	}
#elif defined(__APPLE__) && defined(__MACH__)
	/* Apple MacOSX */
	if(scan_code == 0x39) {
		// caps lock
		// toggled on/off when every push this key.
		n_code = KEYCODE_CAPSLOCK;
		n_keep_frames = true;
		n_type = 0;	// always keydown
	} else if(scan_code == 0) {
		for(const ks2kc_t *p = keysym2keycode; p->ks >= 0; p++) {
			if (p->ks == code) {
				n_code = p->kc;
				break;
			}
		}
	} else if (scan_code < 128) {
		n_code = scancode2keycode[scan_code];
	} else if (scan_code == 255) {
		n_type = 1;
	} else {
		n_code = (code & (KEY_STATUS_SIZE-1)) | KEYCODE_EXTENDED;
	}
#else
	/* Unix and X Window system */
	if (code < 128) {
		n_code = sdlcode2keycode[code];
	} else if (code >= 256 && code < 336) {
		n_code = sdlcode2keycode2[code - 256];
	}
	if (n_code == 0) {
		for(const sc2kc_t *p = scancode2keycode; p->sc >= 0; p++) {
			if (p->sc == scan_code) {
				n_code = p->kc;
				break;
			}
		}
	}
#endif

#endif /* !USE_SDL2 */

	if (n_code == KEYCODE_LSHIFT || n_code == KEYCODE_RSHIFT) {
		BIT_ONOFF(key_mod, KEY_MOD_SHIFT_KEY, (type & 1) == 0);
	}

	if (new_code) *new_code = n_code;
	if (new_keep_frames) *new_keep_frames = n_keep_frames;

	return n_type;
}

#ifdef USE_OLD_KEYINPUT
/// Translate key code
void EMU_OSD::translate_sdl_keysym(SDL_Event *e, int *new_type, int *new_scancode, int *new_sym)
{
	translate_sdl_keysym(e->key.type, e->key.keysym.scancode, e->key.keysym.sym, e->key.keysym.mod, new_type, new_scancode, new_sym);
}

/// Translate key codes from US keylayout to JP keylayout.
void EMU::translate_sdl_keysym(Uint32 e_type, Uint16 e_scancode, SDL_Keycode e_sym, Uint16 e_mod, int *new_type, int *new_scancode, int *new_sym)
{
	int n_type = e_type;
	int n_scancode = e_scancode;
	int n_sym = e_sym;

#ifndef USE_SDL2
/* SDL1 */
#if defined(_WIN32)
	//
	// for windows
	//
	// translate to JIS layout from US layout
	switch(e_sym) {
	case KEYCODE_EQUALS:
		n_sym = KEYCODE_CARET;
		break;
	case KEYCODE_QUOTE:
		n_sym = KEYCODE_COLON;
		break;
	case KEYCODE_LEFTBRACKET:
		n_sym = KEYCODE_AT;
		break;
	case KEYCODE_BACKSLASH:
		switch(e_scancode) {
		case 0x2b:
			n_sym = KEYCODE_RIGHTBRACKET;
			break;
		}
		break;
	case KEYCODE_RIGHTBRACKET:
		n_sym = KEYCODE_LEFTBRACKET;
		break;
	case KEYCODE_CAPSLOCK:
		if (e_type == SDL_KEYUP && (e_mod & KMOD_CAPS) == 0) {
			n_type = SDL_KEYDOWN;
		}
		break;
	default:
		switch(e_scancode) {
		case 0x70:	// katakana
			n_sym = KEYCODE_KATAHIRA;
			break;
		case 0x79:	// henkan
			n_sym = KEYCODE_HENKAN;
			break;
		case 0x73:	// underscore
			n_sym = KEYCODE_UNDERSCORE;
			break;
		case 0x7b:	// muhenkan
			n_sym = KEYCODE_MUHENKAN;
			break;
		}
		break;
	}
#ifdef DEBUG_KEY_TRANS
	logging->out_debugf(_T("[win] translate_sdl_keysym: type:%02x -> %02x scancode:%02x -> %02x sym:%02x -> %02x")
		,e_type, n_type
		,e_scancode, n_scancode
		,e_sym, n_sym);
#endif

#elif defined(linux)
	//
	// for linux
	//
	switch(e_sym) {
	default:
		switch(e_scancode) {
		case 0x42:	// caps lock
			n_sym = KEYCODE_CAPSLOCK;
			break;
		case 0x64:	// henkan
			n_sym = KEYCODE_HENKAN;
			break;
		case 0x65:	// katakana / hiragana
			n_sym = KEYCODE_KATAHIRA;
			break;
		case 0x61:	// underscore
			n_sym = KEYCODE_UNDERSCORE;
			break;
		case 0x66:	// muhenkan
			n_sym = KEYCODE_MUHENKAN;
			break;
		}
		break;
	}
#ifdef DEBUG_KEY_TRANS
	logging->out_debugf("[linux] translate_sdl_keysym: type:%02x -> %02x sym:%02x -> %02x"
		,e_type, n_type
		,e_sym, n_sym);
#endif

#elif defined(__APPLE__) && defined(__MACH__)
	//
	// for MacOS X
	//
	// translate to JIS layout from US layout
	switch(e_sym) {
	case KEYCODE_EQUALS:
		n_sym = KEYCODE_CARET;
		break;
	case KEYCODE_QUOTE:
		n_sym = KEYCODE_COLON;
		break;
	case KEYCODE_LEFTBRACKET:
		n_sym = KEYCODE_AT;
		break;
	case KEYCODE_BACKSLASH:
		n_sym = KEYCODE_RIGHTBRACKET;
		break;
	case KEYCODE_RIGHTBRACKET:
		n_sym = KEYCODE_LEFTBRACKET;
		break;
	case KEYCODE_CAPSLOCK:
		if (e_type == SDL_KEYUP && (e_mod & KMOD_CAPS) == 0) {
			n_type = SDL_KEYDOWN;
		}
		n_sym = e_sym;
		break;
	default:
		switch(e_scancode) {
		case 0x5d:	// yen
			n_sym = KEYCODE_BACKSLASH;
			break;
		case 0x5e:	// underscore
			n_sym = KEYCODE_UNDERSCORE;
			break;
		case 0x6e:	// right win
			n_sym = KEYCODE_MENU;
			break;
		case 0x3f:  // fn
			n_sym = KEYCODE_FUNCTION;
			break;
		case 0x6a:  // F16
			n_sym = KEYCODE_F16;
			break;
		case 0x40:  // F17
			n_sym = KEYCODE_F17;
			break;
		case 0x4f:  // F18
			n_sym = KEYCODE_F18;
			break;
		case 0x50:  // F19
			n_sym = KEYCODE_F19;
			break;
		}
		break;
	}
#ifdef DEBUG_KEY_TRANS
	logging->out_debugf("[mac] translate_sdl_keysym: type:%02x -> %02x sym:%02x -> %02x"
		,e_type, n_type
		,e_sym, n_sym);
#endif

#elif defined(__FreeBSD__)
	//
	// for FreeBSD
	//
	switch(e_sym) {
	default:
		switch(e_scancode) {
		case 0x31:	// kanji
			n_sym = KEYCODE_GRAVE;
			break;
		case 0x42:	// caps lock
			n_sym = KEYCODE_CAPSLOCK;
			break;
		case 0x81:	// henkan
			n_sym = KEYCODE_HENKAN;
			break;
		case 0x83:	// muhenkan
			n_sym = KEYCODE_MUHENKAN;
			break;
		case 0xd0:	// katakana / hiragana
			n_sym = KEYCODE_KATAHIRA;
			break;
		case 0xd3:	// underscore
			n_sym = KEYCODE_UNDERSCORE;
			break;
		}
		break;
	}
#ifdef DEBUG_KEY_TRANS
	logging->out_debugf("[unix] translate_sdl_keysym: type:%02x -> %02x sym:%02x -> %02x"
		,e_type, n_type
		,e_sym, n_sym);
#endif

#endif
#endif /* USE_SDL2 */

	if (n_code == KEYCODE_LSHIFT || n_code == KEYCODE_RSHIFT) {
		BIT_ONOFF(key_mod, KEY_MOD_SHIFT_KEY, (type & 1) == 0);
	}

	if (new_type != NULL) *new_type = n_type;
	if (new_scancode != NULL) *new_scancode = n_scancode;
	if (new_sym  != NULL) *new_sym  = n_sym;
}

bool EMU_OSD::translate_key_down(int code, uint16_t mod)
{
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
	if(code == KEYCODE_CAPSLOCK) {
		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
		return true;
	}
//	else if(code == SDLK_WORLD_0) {
//		key_status[code] = KEY_KEEP_FRAMES * FRAME_SPLIT_NUM;
//	}
#endif

	return false;
}
#endif /* USE_OLD_KEYINPUT */

