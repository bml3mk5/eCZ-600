/** @file win_key_trans.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.30 -

	@brief [ win to SDL key translate ]
*/

#include "win_key_trans.h"
#include "../../emu.h"

extern EMU *emu;

namespace GUI_WIN
{

#ifndef USE_SDL2

#ifndef USE_OLD_KEYINPUT
UINT32 translate_to_sdlkey(WPARAM wParam, LPARAM lParam)
{
	return (UINT32)wParam;
}
#else
// translate VK key to SDL key code
// from SDL_dibevents.c

static SDLKey VK_keymap[SDLK_LAST];
static SDL_keysym *TranslateKey(WPARAM vkey, UINT scancode, SDL_keysym *keysym, int pressed);

static HKL hLayoutUS = NULL;

#define REPEATED_KEYMASK	(1<<30)
#define EXTENDED_KEYMASK	(1<<24)

UINT32 translate_to_sdlkey(WPARAM wParam, LPARAM lParam)
{
	SDL_keysym keysym;

	/* Ignore repeated keys */
	if ( lParam & REPEATED_KEYMASK ) {
		return 0;
	}
	keysym.sym = SDLK_UNKNOWN;
	switch (wParam) {
		case VK_CONTROL:
			if ( lParam & EXTENDED_KEYMASK )
				wParam = VK_RCONTROL;
			else
				wParam = VK_LCONTROL;
			break;
		case VK_SHIFT:
			/* EXTENDED trick doesn't work here */
			if (GetKeyState(VK_LSHIFT) & 0x8000) {
				wParam = VK_LSHIFT;
			} else if (GetKeyState(VK_RSHIFT) & 0x8000) {
				wParam = VK_RSHIFT;
			}
			break;
		case VK_MENU:
			if ( lParam & EXTENDED_KEYMASK )
				wParam = VK_RMENU;
			else
				wParam = VK_LMENU;
			break;
	}
	TranslateKey(wParam,HIWORD(lParam),&keysym,1);

	int new_type = 0;
	int new_sym = 0;

//	emu->translate_sdl_keysym(SDL_KEYDOWN, keysym.scancode, keysym.sym, keysym.mod, &new_type, NULL, &new_sym);

	return (UINT32)new_sym;
}

#ifndef VK_0
#define VK_0	'0'
#define VK_1	'1'
#define VK_2	'2'
#define VK_3	'3'
#define VK_4	'4'
#define VK_5	'5'
#define VK_6	'6'
#define VK_7	'7'
#define VK_8	'8'
#define VK_9	'9'
#define VK_A	'A'
#define VK_B	'B'
#define VK_C	'C'
#define VK_D	'D'
#define VK_E	'E'
#define VK_F	'F'
#define VK_G	'G'
#define VK_H	'H'
#define VK_I	'I'
#define VK_J	'J'
#define VK_K	'K'
#define VK_L	'L'
#define VK_M	'M'
#define VK_N	'N'
#define VK_O	'O'
#define VK_P	'P'
#define VK_Q	'Q'
#define VK_R	'R'
#define VK_S	'S'
#define VK_T	'T'
#define VK_U	'U'
#define VK_V	'V'
#define VK_W	'W'
#define VK_X	'X'
#define VK_Y	'Y'
#define VK_Z	'Z'
#endif /* VK_0 */

/* These keys haven't been defined, but were experimentally determined */
#define VK_SEMICOLON	0xBA
#define VK_EQUALS	0xBB
#define VK_COMMA	0xBC
#define VK_MINUS	0xBD
#define VK_PERIOD	0xBE
#define VK_SLASH	0xBF
#define VK_GRAVE	0xC0
#define VK_LBRACKET	0xDB
#define VK_BACKSLASH	0xDC
#define VK_RBRACKET	0xDD
#define VK_APOSTROPHE	0xDE
#define VK_BACKTICK	0xDF
#define VK_OEM_102	0xE2

void SDL_DIB_InitOSKeymap()
{
	int	i;
	char	current_layout[KL_NAMELENGTH];

	if (VK_keymap[VK_BACK] == SDLK_BACKSPACE) return;

	GetKeyboardLayoutName(current_layout);

	hLayoutUS = LoadKeyboardLayout("00000409", KLF_NOTELLSHELL);

	if (!hLayoutUS) {
		//printf("Failed to load US keyboard layout. Using current.\n");
		hLayoutUS = GetKeyboardLayout(0);
	}
	LoadKeyboardLayout(current_layout, KLF_ACTIVATE);


	/* Map the VK keysyms */
	for ( i=0; i<SDL_arraysize(VK_keymap); ++i )
		VK_keymap[i] = SDLK_UNKNOWN;

	VK_keymap[VK_BACK] = SDLK_BACKSPACE;
	VK_keymap[VK_TAB] = SDLK_TAB;
	VK_keymap[VK_CLEAR] = SDLK_CLEAR;
	VK_keymap[VK_RETURN] = SDLK_RETURN;
	VK_keymap[VK_PAUSE] = SDLK_PAUSE;
	VK_keymap[VK_ESCAPE] = SDLK_ESCAPE;
	VK_keymap[VK_SPACE] = SDLK_SPACE;
	VK_keymap[VK_APOSTROPHE] = SDLK_QUOTE;
	VK_keymap[VK_COMMA] = SDLK_COMMA;
	VK_keymap[VK_MINUS] = SDLK_MINUS;
	VK_keymap[VK_PERIOD] = SDLK_PERIOD;
	VK_keymap[VK_SLASH] = SDLK_SLASH;
	VK_keymap[VK_0] = SDLK_0;
	VK_keymap[VK_1] = SDLK_1;
	VK_keymap[VK_2] = SDLK_2;
	VK_keymap[VK_3] = SDLK_3;
	VK_keymap[VK_4] = SDLK_4;
	VK_keymap[VK_5] = SDLK_5;
	VK_keymap[VK_6] = SDLK_6;
	VK_keymap[VK_7] = SDLK_7;
	VK_keymap[VK_8] = SDLK_8;
	VK_keymap[VK_9] = SDLK_9;
	VK_keymap[VK_SEMICOLON] = SDLK_SEMICOLON;
	VK_keymap[VK_EQUALS] = SDLK_EQUALS;
	VK_keymap[VK_LBRACKET] = SDLK_LEFTBRACKET;
	VK_keymap[VK_BACKSLASH] = SDLK_BACKSLASH;
	VK_keymap[VK_OEM_102] = SDLK_LESS;
	VK_keymap[VK_RBRACKET] = SDLK_RIGHTBRACKET;
	VK_keymap[VK_GRAVE] = SDLK_BACKQUOTE;
	VK_keymap[VK_BACKTICK] = SDLK_BACKQUOTE;
	VK_keymap[VK_A] = SDLK_a;
	VK_keymap[VK_B] = SDLK_b;
	VK_keymap[VK_C] = SDLK_c;
	VK_keymap[VK_D] = SDLK_d;
	VK_keymap[VK_E] = SDLK_e;
	VK_keymap[VK_F] = SDLK_f;
	VK_keymap[VK_G] = SDLK_g;
	VK_keymap[VK_H] = SDLK_h;
	VK_keymap[VK_I] = SDLK_i;
	VK_keymap[VK_J] = SDLK_j;
	VK_keymap[VK_K] = SDLK_k;
	VK_keymap[VK_L] = SDLK_l;
	VK_keymap[VK_M] = SDLK_m;
	VK_keymap[VK_N] = SDLK_n;
	VK_keymap[VK_O] = SDLK_o;
	VK_keymap[VK_P] = SDLK_p;
	VK_keymap[VK_Q] = SDLK_q;
	VK_keymap[VK_R] = SDLK_r;
	VK_keymap[VK_S] = SDLK_s;
	VK_keymap[VK_T] = SDLK_t;
	VK_keymap[VK_U] = SDLK_u;
	VK_keymap[VK_V] = SDLK_v;
	VK_keymap[VK_W] = SDLK_w;
	VK_keymap[VK_X] = SDLK_x;
	VK_keymap[VK_Y] = SDLK_y;
	VK_keymap[VK_Z] = SDLK_z;
	VK_keymap[VK_DELETE] = SDLK_DELETE;

	VK_keymap[VK_NUMPAD0] = SDLK_KP0;
	VK_keymap[VK_NUMPAD1] = SDLK_KP1;
	VK_keymap[VK_NUMPAD2] = SDLK_KP2;
	VK_keymap[VK_NUMPAD3] = SDLK_KP3;
	VK_keymap[VK_NUMPAD4] = SDLK_KP4;
	VK_keymap[VK_NUMPAD5] = SDLK_KP5;
	VK_keymap[VK_NUMPAD6] = SDLK_KP6;
	VK_keymap[VK_NUMPAD7] = SDLK_KP7;
	VK_keymap[VK_NUMPAD8] = SDLK_KP8;
	VK_keymap[VK_NUMPAD9] = SDLK_KP9;
	VK_keymap[VK_DECIMAL] = SDLK_KP_PERIOD;
	VK_keymap[VK_DIVIDE] = SDLK_KP_DIVIDE;
	VK_keymap[VK_MULTIPLY] = SDLK_KP_MULTIPLY;
	VK_keymap[VK_SUBTRACT] = SDLK_KP_MINUS;
	VK_keymap[VK_ADD] = SDLK_KP_PLUS;

	VK_keymap[VK_UP] = SDLK_UP;
	VK_keymap[VK_DOWN] = SDLK_DOWN;
	VK_keymap[VK_RIGHT] = SDLK_RIGHT;
	VK_keymap[VK_LEFT] = SDLK_LEFT;
	VK_keymap[VK_INSERT] = SDLK_INSERT;
	VK_keymap[VK_HOME] = SDLK_HOME;
	VK_keymap[VK_END] = SDLK_END;
	VK_keymap[VK_PRIOR] = SDLK_PAGEUP;
	VK_keymap[VK_NEXT] = SDLK_PAGEDOWN;

	VK_keymap[VK_F1] = SDLK_F1;
	VK_keymap[VK_F2] = SDLK_F2;
	VK_keymap[VK_F3] = SDLK_F3;
	VK_keymap[VK_F4] = SDLK_F4;
	VK_keymap[VK_F5] = SDLK_F5;
	VK_keymap[VK_F6] = SDLK_F6;
	VK_keymap[VK_F7] = SDLK_F7;
	VK_keymap[VK_F8] = SDLK_F8;
	VK_keymap[VK_F9] = SDLK_F9;
	VK_keymap[VK_F10] = SDLK_F10;
	VK_keymap[VK_F11] = SDLK_F11;
	VK_keymap[VK_F12] = SDLK_F12;
	VK_keymap[VK_F13] = SDLK_F13;
	VK_keymap[VK_F14] = SDLK_F14;
	VK_keymap[VK_F15] = SDLK_F15;

	VK_keymap[VK_NUMLOCK] = SDLK_NUMLOCK;
	VK_keymap[VK_CAPITAL] = SDLK_CAPSLOCK;
	VK_keymap[VK_SCROLL] = SDLK_SCROLLOCK;
	VK_keymap[VK_RSHIFT] = SDLK_RSHIFT;
	VK_keymap[VK_LSHIFT] = SDLK_LSHIFT;
	VK_keymap[VK_RCONTROL] = SDLK_RCTRL;
	VK_keymap[VK_LCONTROL] = SDLK_LCTRL;
	VK_keymap[VK_RMENU] = SDLK_RALT;
	VK_keymap[VK_LMENU] = SDLK_LALT;
	VK_keymap[VK_RWIN] = SDLK_RSUPER;
	VK_keymap[VK_LWIN] = SDLK_LSUPER;

	VK_keymap[VK_HELP] = SDLK_HELP;
#ifdef VK_PRINT
	VK_keymap[VK_PRINT] = SDLK_PRINT;
#endif
	VK_keymap[VK_SNAPSHOT] = SDLK_PRINT;
	VK_keymap[VK_CANCEL] = SDLK_BREAK;
	VK_keymap[VK_APPS] = SDLK_MENU;
}

#define EXTKEYPAD(keypad) ((scancode & 0x100)?(mvke):(keypad))


static int SDL_MapVirtualKey(int scancode, int vkey)
{
	int	mvke  = MapVirtualKeyEx(scancode & 0xFF, 1, hLayoutUS);

	switch(vkey) {
		/* These are always correct */
		case VK_DIVIDE:
		case VK_MULTIPLY:
		case VK_SUBTRACT:
		case VK_ADD:
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
		/* These are already handled */
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_LMENU:
		case VK_RMENU:
		case VK_SNAPSHOT:
		case VK_PAUSE:
			return vkey;
	}
	switch(mvke) {
		/* Distinguish between keypad and extended keys */
		case VK_INSERT: return EXTKEYPAD(VK_NUMPAD0);
		case VK_DELETE: return EXTKEYPAD(VK_DECIMAL);
		case VK_END:    return EXTKEYPAD(VK_NUMPAD1);
		case VK_DOWN:   return EXTKEYPAD(VK_NUMPAD2);
		case VK_NEXT:   return EXTKEYPAD(VK_NUMPAD3);
		case VK_LEFT:   return EXTKEYPAD(VK_NUMPAD4);
		case VK_CLEAR:  return EXTKEYPAD(VK_NUMPAD5);
		case VK_RIGHT:  return EXTKEYPAD(VK_NUMPAD6);
		case VK_HOME:   return EXTKEYPAD(VK_NUMPAD7);
		case VK_UP:     return EXTKEYPAD(VK_NUMPAD8);
		case VK_PRIOR:  return EXTKEYPAD(VK_NUMPAD9);
	}
	return mvke?mvke:vkey;
}

static SDL_keysym *TranslateKey(WPARAM vkey, UINT scancode, SDL_keysym *keysym, int pressed)
{
	/* Set the keysym information */
	keysym->scancode = (unsigned char) scancode;
	keysym->mod = KMOD_NONE;
	keysym->unicode = 0;

	if ((vkey == VK_RETURN) && (scancode & 0x100)) {
		/* No VK_ code for the keypad enter key */
		keysym->sym = SDLK_KP_ENTER;
	}
	else {
		keysym->sym = VK_keymap[SDL_MapVirtualKey(scancode, (int)vkey)];
	}

#if 0
	if ( pressed && SDL_TranslateUNICODE ) {
		BYTE	keystate[256];
		WCHAR	wchars[2];

		GetKeyboardState(keystate);
		/* Numlock isn't taken into account in ToUnicode,
		 * so we handle it as a special case here */
		if ((keystate[VK_NUMLOCK] & 1) && vkey >= VK_NUMPAD0 && vkey <= VK_NUMPAD9)
		{
			keysym->unicode = (Uint16)(vkey - VK_NUMPAD0 + '0');
		}
//		else if (SDL_ToUnicode((UINT)vkey, scancode, keystate, wchars, sizeof(wchars)/sizeof(wchars[0]), 0) > 0)
		else if (ToUnicode((UINT)vkey, scancode, keystate, wchars, sizeof(wchars)/sizeof(wchars[0]), 0) > 0)
		{
			keysym->unicode = wchars[0];
		}
	}
#endif
	return(keysym);
}
#endif

#else /* USE_SDL2 */

#include "scancodes_windows.h"

UINT32 translate_to_sdlkey(WPARAM wParam, LPARAM lParam)
{
    SDL_Scancode code;
    char bIsExtended;
    int nScanCode = ( lParam >> 16 ) & 0xFF;

    /* 0x45 here to work around both pause and numlock sharing the same scancode, so use the VK key to tell them apart */
    if ( nScanCode == 0 || nScanCode == 0x45 )
    {
        switch( wParam )
        {
        case VK_CLEAR: return SDL_SCANCODE_CLEAR;
        case VK_MODECHANGE: return SDL_SCANCODE_MODE;
        case VK_SELECT: return SDL_SCANCODE_SELECT;
        case VK_EXECUTE: return SDL_SCANCODE_EXECUTE;
        case VK_HELP: return SDL_SCANCODE_HELP;
        case VK_PAUSE: return SDL_SCANCODE_PAUSE;
        case VK_NUMLOCK: return SDL_SCANCODE_NUMLOCKCLEAR;

        case VK_F13: return SDL_SCANCODE_F13;
        case VK_F14: return SDL_SCANCODE_F14;
        case VK_F15: return SDL_SCANCODE_F15;
        case VK_F16: return SDL_SCANCODE_F16;
        case VK_F17: return SDL_SCANCODE_F17;
        case VK_F18: return SDL_SCANCODE_F18;
        case VK_F19: return SDL_SCANCODE_F19;
        case VK_F20: return SDL_SCANCODE_F20;
        case VK_F21: return SDL_SCANCODE_F21;
        case VK_F22: return SDL_SCANCODE_F22;
        case VK_F23: return SDL_SCANCODE_F23;
        case VK_F24: return SDL_SCANCODE_F24;

        case VK_OEM_NEC_EQUAL: return SDL_SCANCODE_KP_EQUALS;
        case VK_BROWSER_BACK: return SDL_SCANCODE_AC_BACK;
        case VK_BROWSER_FORWARD: return SDL_SCANCODE_AC_FORWARD;
        case VK_BROWSER_REFRESH: return SDL_SCANCODE_AC_REFRESH;
        case VK_BROWSER_STOP: return SDL_SCANCODE_AC_STOP;
        case VK_BROWSER_SEARCH: return SDL_SCANCODE_AC_SEARCH;
        case VK_BROWSER_FAVORITES: return SDL_SCANCODE_AC_BOOKMARKS;
        case VK_BROWSER_HOME: return SDL_SCANCODE_AC_HOME;
        case VK_VOLUME_MUTE: return SDL_SCANCODE_AUDIOMUTE;
        case VK_VOLUME_DOWN: return SDL_SCANCODE_VOLUMEDOWN;
        case VK_VOLUME_UP: return SDL_SCANCODE_VOLUMEUP;

        case VK_MEDIA_NEXT_TRACK: return SDL_SCANCODE_AUDIONEXT;
        case VK_MEDIA_PREV_TRACK: return SDL_SCANCODE_AUDIOPREV;
        case VK_MEDIA_STOP: return SDL_SCANCODE_AUDIOSTOP;
        case VK_MEDIA_PLAY_PAUSE: return SDL_SCANCODE_AUDIOPLAY;
        case VK_LAUNCH_MAIL: return SDL_SCANCODE_MAIL;
        case VK_LAUNCH_MEDIA_SELECT: return SDL_SCANCODE_MEDIASELECT;

        case VK_OEM_102: return SDL_SCANCODE_NONUSBACKSLASH;

        case VK_ATTN: return SDL_SCANCODE_SYSREQ;
        case VK_CRSEL: return SDL_SCANCODE_CRSEL;
        case VK_EXSEL: return SDL_SCANCODE_EXSEL;
        case VK_OEM_CLEAR: return SDL_SCANCODE_CLEAR;

        case VK_LAUNCH_APP1: return SDL_SCANCODE_APP1;
        case VK_LAUNCH_APP2: return SDL_SCANCODE_APP2;

        default: return SDL_SCANCODE_UNKNOWN;
        }
    }

    if ( nScanCode > 127 )
        return SDL_SCANCODE_UNKNOWN;

    code = windows_scancode_table[nScanCode];

    bIsExtended = ( lParam & ( 1 << 24 ) ) != 0;
    if ( !bIsExtended )
    {
        switch ( code )
        {
        case SDL_SCANCODE_HOME:
            return SDL_SCANCODE_KP_7;
        case SDL_SCANCODE_UP:
            return SDL_SCANCODE_KP_8;
        case SDL_SCANCODE_PAGEUP:
            return SDL_SCANCODE_KP_9;
        case SDL_SCANCODE_LEFT:
            return SDL_SCANCODE_KP_4;
        case SDL_SCANCODE_RIGHT:
            return SDL_SCANCODE_KP_6;
        case SDL_SCANCODE_END:
            return SDL_SCANCODE_KP_1;
        case SDL_SCANCODE_DOWN:
            return SDL_SCANCODE_KP_2;
        case SDL_SCANCODE_PAGEDOWN:
            return SDL_SCANCODE_KP_3;
        case SDL_SCANCODE_INSERT:
            return SDL_SCANCODE_KP_0;
        case SDL_SCANCODE_DELETE:
            return SDL_SCANCODE_KP_PERIOD;
        case SDL_SCANCODE_PRINTSCREEN:
            return SDL_SCANCODE_KP_MULTIPLY;
        default:
            break;
        }
    }
    else
    {
        switch ( code )
        {
        case SDL_SCANCODE_RETURN:
            return SDL_SCANCODE_KP_ENTER;
        case SDL_SCANCODE_LALT:
            return SDL_SCANCODE_RALT;
        case SDL_SCANCODE_LCTRL:
            return SDL_SCANCODE_RCTRL;
        case SDL_SCANCODE_SLASH:
            return SDL_SCANCODE_KP_DIVIDE;
        case SDL_SCANCODE_CAPSLOCK:
            return SDL_SCANCODE_KP_PLUS;
        default:
            break;
        }
    }

    return code;

}

//void SDL_DIB_InitOSKeymap(void)
//{
//}

#endif /* USE_SDL2 */

}; /* namespace GUI_WIN */
