/** @file gtk_x11_key_trans.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ gtk(x11) to SDL key translate ]

	@note This is based on SDL_x11keyevents.c
*/

#include "gtk_x11_key_trans.h"
#include <SDL.h>

#define USE_XKBKEYCODETOKEYSYM 1

#ifdef USE_XKBKEYCODETOKEYSYM
#include <X11/XKBlib.h>
#endif

namespace GUI_GTK_X11
{

#ifndef USE_SDL2

static SDLKey ODD_keymap[256];
static SDLKey MISC_keymap[256];

void X11_InitKeymap(void)
{
	int i;

	/* Odd keys used in international keyboards */
	for ( i=0; i<(int)SDL_arraysize(ODD_keymap); ++i )
		ODD_keymap[i] = SDLK_UNKNOWN;

 	/* Some of these might be mappable to an existing SDLK_ code */
 	ODD_keymap[XK_dead_grave&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_acute&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_tilde&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_macron&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_breve&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_abovedot&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_diaeresis&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_abovering&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_doubleacute&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_caron&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_cedilla&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_ogonek&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_iota&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_voiced_sound&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_semivoiced_sound&0xFF] = SDLK_COMPOSE;
 	ODD_keymap[XK_dead_belowdot&0xFF] = SDLK_COMPOSE;
#ifdef XK_dead_hook
 	ODD_keymap[XK_dead_hook&0xFF] = SDLK_COMPOSE;
#endif
#ifdef XK_dead_horn
 	ODD_keymap[XK_dead_horn&0xFF] = SDLK_COMPOSE;
#endif

#ifdef XK_dead_circumflex
	/* These X keysyms have 0xFE as the high byte */
	ODD_keymap[XK_dead_circumflex&0xFF] = SDLK_CARET;
#endif
#ifdef XK_ISO_Level3_Shift
	ODD_keymap[XK_ISO_Level3_Shift&0xFF] = SDLK_MODE; /* "Alt Gr" key */
#endif

	/* Map the miscellaneous keys */
	for ( i=0; i<(int)SDL_arraysize(MISC_keymap); ++i )
		MISC_keymap[i] = SDLK_UNKNOWN;

	/* These X keysyms have 0xFF as the high byte */
	MISC_keymap[XK_BackSpace&0xFF] = SDLK_BACKSPACE;
	MISC_keymap[XK_Tab&0xFF] = SDLK_TAB;
	MISC_keymap[XK_Clear&0xFF] = SDLK_CLEAR;
	MISC_keymap[XK_Return&0xFF] = SDLK_RETURN;
	MISC_keymap[XK_Pause&0xFF] = SDLK_PAUSE;
	MISC_keymap[XK_Escape&0xFF] = SDLK_ESCAPE;
	MISC_keymap[XK_Delete&0xFF] = SDLK_DELETE;

	MISC_keymap[XK_KP_0&0xFF] = SDLK_KP0;		/* Keypad 0-9 */
	MISC_keymap[XK_KP_1&0xFF] = SDLK_KP1;
	MISC_keymap[XK_KP_2&0xFF] = SDLK_KP2;
	MISC_keymap[XK_KP_3&0xFF] = SDLK_KP3;
	MISC_keymap[XK_KP_4&0xFF] = SDLK_KP4;
	MISC_keymap[XK_KP_5&0xFF] = SDLK_KP5;
	MISC_keymap[XK_KP_6&0xFF] = SDLK_KP6;
	MISC_keymap[XK_KP_7&0xFF] = SDLK_KP7;
	MISC_keymap[XK_KP_8&0xFF] = SDLK_KP8;
	MISC_keymap[XK_KP_9&0xFF] = SDLK_KP9;
	MISC_keymap[XK_KP_Insert&0xFF] = SDLK_KP0;
	MISC_keymap[XK_KP_End&0xFF] = SDLK_KP1;
	MISC_keymap[XK_KP_Down&0xFF] = SDLK_KP2;
	MISC_keymap[XK_KP_Page_Down&0xFF] = SDLK_KP3;
	MISC_keymap[XK_KP_Left&0xFF] = SDLK_KP4;
	MISC_keymap[XK_KP_Begin&0xFF] = SDLK_KP5;
	MISC_keymap[XK_KP_Right&0xFF] = SDLK_KP6;
	MISC_keymap[XK_KP_Home&0xFF] = SDLK_KP7;
	MISC_keymap[XK_KP_Up&0xFF] = SDLK_KP8;
	MISC_keymap[XK_KP_Page_Up&0xFF] = SDLK_KP9;
	MISC_keymap[XK_KP_Delete&0xFF] = SDLK_KP_PERIOD;
	MISC_keymap[XK_KP_Decimal&0xFF] = SDLK_KP_PERIOD;
	MISC_keymap[XK_KP_Divide&0xFF] = SDLK_KP_DIVIDE;
	MISC_keymap[XK_KP_Multiply&0xFF] = SDLK_KP_MULTIPLY;
	MISC_keymap[XK_KP_Subtract&0xFF] = SDLK_KP_MINUS;
	MISC_keymap[XK_KP_Add&0xFF] = SDLK_KP_PLUS;
	MISC_keymap[XK_KP_Enter&0xFF] = SDLK_KP_ENTER;
	MISC_keymap[XK_KP_Equal&0xFF] = SDLK_KP_EQUALS;

	MISC_keymap[XK_Up&0xFF] = SDLK_UP;
	MISC_keymap[XK_Down&0xFF] = SDLK_DOWN;
	MISC_keymap[XK_Right&0xFF] = SDLK_RIGHT;
	MISC_keymap[XK_Left&0xFF] = SDLK_LEFT;
	MISC_keymap[XK_Insert&0xFF] = SDLK_INSERT;
	MISC_keymap[XK_Home&0xFF] = SDLK_HOME;
	MISC_keymap[XK_End&0xFF] = SDLK_END;
	MISC_keymap[XK_Page_Up&0xFF] = SDLK_PAGEUP;
	MISC_keymap[XK_Page_Down&0xFF] = SDLK_PAGEDOWN;

	MISC_keymap[XK_F1&0xFF] = SDLK_F1;
	MISC_keymap[XK_F2&0xFF] = SDLK_F2;
	MISC_keymap[XK_F3&0xFF] = SDLK_F3;
	MISC_keymap[XK_F4&0xFF] = SDLK_F4;
	MISC_keymap[XK_F5&0xFF] = SDLK_F5;
	MISC_keymap[XK_F6&0xFF] = SDLK_F6;
	MISC_keymap[XK_F7&0xFF] = SDLK_F7;
	MISC_keymap[XK_F8&0xFF] = SDLK_F8;
	MISC_keymap[XK_F9&0xFF] = SDLK_F9;
	MISC_keymap[XK_F10&0xFF] = SDLK_F10;
	MISC_keymap[XK_F11&0xFF] = SDLK_F11;
	MISC_keymap[XK_F12&0xFF] = SDLK_F12;
	MISC_keymap[XK_F13&0xFF] = SDLK_F13;
	MISC_keymap[XK_F14&0xFF] = SDLK_F14;
	MISC_keymap[XK_F15&0xFF] = SDLK_F15;

	MISC_keymap[XK_Num_Lock&0xFF] = SDLK_NUMLOCK;
	MISC_keymap[XK_Caps_Lock&0xFF] = SDLK_CAPSLOCK;
	MISC_keymap[XK_Scroll_Lock&0xFF] = SDLK_SCROLLOCK;
	MISC_keymap[XK_Shift_R&0xFF] = SDLK_RSHIFT;
	MISC_keymap[XK_Shift_L&0xFF] = SDLK_LSHIFT;
	MISC_keymap[XK_Control_R&0xFF] = SDLK_RCTRL;
	MISC_keymap[XK_Control_L&0xFF] = SDLK_LCTRL;
	MISC_keymap[XK_Alt_R&0xFF] = SDLK_RALT;
	MISC_keymap[XK_Alt_L&0xFF] = SDLK_LALT;
	MISC_keymap[XK_Meta_R&0xFF] = SDLK_RMETA;
	MISC_keymap[XK_Meta_L&0xFF] = SDLK_LMETA;
	MISC_keymap[XK_Super_L&0xFF] = SDLK_LSUPER; /* Left "Windows" */
	MISC_keymap[XK_Super_R&0xFF] = SDLK_RSUPER; /* Right "Windows */
	MISC_keymap[XK_Mode_switch&0xFF] = SDLK_MODE; /* "Alt Gr" key */
	MISC_keymap[XK_Multi_key&0xFF] = SDLK_COMPOSE; /* Multi-key compose */

	MISC_keymap[XK_Help&0xFF] = SDLK_HELP;
	MISC_keymap[XK_Print&0xFF] = SDLK_PRINT;
	MISC_keymap[XK_Sys_Req&0xFF] = SDLK_SYSREQ;
	MISC_keymap[XK_Break&0xFF] = SDLK_BREAK;
	MISC_keymap[XK_Menu&0xFF] = SDLK_MENU;
	MISC_keymap[XK_Hyper_R&0xFF] = SDLK_MENU;   /* Windows "Menu" key */
}

/* Get the translated SDL virtual keysym */
uint32_t X11_TranslateKeycode(Display *display, KeyCode kc)
{
	KeySym xsym;
	SDLKey key;

#ifdef USE_XKBKEYCODETOKEYSYM
	xsym = XkbKeycodeToKeysym(display, kc, 0, 0);
#else
	xsym = XKeycodeToKeysym(display, kc, 0);
#endif
#ifdef DEBUG_KEYS
	fprintf(stderr, "Translating key code %d -> 0x%.4x\n", kc, xsym);
#endif
	key = SDLK_UNKNOWN;
	if ( xsym ) {
		switch (xsym>>8) {
		    case 0x1005FF:
#ifdef SunXK_F36
			if ( xsym == SunXK_F36 )
				key = SDLK_F11;
#endif
#ifdef SunXK_F37
			if ( xsym == SunXK_F37 )
				key = SDLK_F12;
#endif
			break;
		    case 0x00:	/* Latin 1 */
			key = (SDLKey)(xsym & 0xFF);
			break;
		    case 0x01:	/* Latin 2 */
		    case 0x02:	/* Latin 3 */
		    case 0x03:	/* Latin 4 */
		    case 0x04:	/* Katakana */
		    case 0x05:	/* Arabic */
		    case 0x06:	/* Cyrillic */
		    case 0x07:	/* Greek */
		    case 0x08:	/* Technical */
		    case 0x0A:	/* Publishing */
		    case 0x0C:	/* Hebrew */
		    case 0x0D:	/* Thai */
			/* These are wrong, but it's better than nothing */
			key = (SDLKey)(xsym & 0xFF);
			break;
		    case 0xFE:
			key = ODD_keymap[xsym&0xFF];
			break;
		    case 0xFF:
			key = MISC_keymap[xsym&0xFF];
			break;
		    default:
			/*
			fprintf(stderr, "X11: Unhandled xsym, sym = 0x%04x\n",
					(unsigned int)xsym);
			*/
			break;
		}
	} else {
		/* X11 doesn't know how to translate the key! */
		switch (kc) {
		    /* Caution:
		       These keycodes are from the Microsoft Keyboard
		     */
		    case 115:
			key = SDLK_LSUPER;
			break;
		    case 116:
			key = SDLK_RSUPER;
			break;
		    case 117:
			key = SDLK_MENU;
			break;
		    default:
			/*
			 * no point in an error message; happens for
			 * several keys when we get a keymap notify
			 */
			break;
		}
	}
	return (Uint32)key;
}

#else /* !USE_SDL2 */

#include "scancodes_xfree86.h"

/* *INDENT-OFF* */
static const struct {
    KeySym keysym;
    SDL_Scancode scancode;
} KeySymToSDLScancode[] = {
    { XK_Return, SDL_SCANCODE_RETURN },
    { XK_Escape, SDL_SCANCODE_ESCAPE },
    { XK_BackSpace, SDL_SCANCODE_BACKSPACE },
    { XK_Tab, SDL_SCANCODE_TAB },
    { XK_Caps_Lock, SDL_SCANCODE_CAPSLOCK },
    { XK_F1, SDL_SCANCODE_F1 },
    { XK_F2, SDL_SCANCODE_F2 },
    { XK_F3, SDL_SCANCODE_F3 },
    { XK_F4, SDL_SCANCODE_F4 },
    { XK_F5, SDL_SCANCODE_F5 },
    { XK_F6, SDL_SCANCODE_F6 },
    { XK_F7, SDL_SCANCODE_F7 },
    { XK_F8, SDL_SCANCODE_F8 },
    { XK_F9, SDL_SCANCODE_F9 },
    { XK_F10, SDL_SCANCODE_F10 },
    { XK_F11, SDL_SCANCODE_F11 },
    { XK_F12, SDL_SCANCODE_F12 },
    { XK_Print, SDL_SCANCODE_PRINTSCREEN },
    { XK_Scroll_Lock, SDL_SCANCODE_SCROLLLOCK },
    { XK_Pause, SDL_SCANCODE_PAUSE },
    { XK_Insert, SDL_SCANCODE_INSERT },
    { XK_Home, SDL_SCANCODE_HOME },
    { XK_Prior, SDL_SCANCODE_PAGEUP },
    { XK_Delete, SDL_SCANCODE_DELETE },
    { XK_End, SDL_SCANCODE_END },
    { XK_Next, SDL_SCANCODE_PAGEDOWN },
    { XK_Right, SDL_SCANCODE_RIGHT },
    { XK_Left, SDL_SCANCODE_LEFT },
    { XK_Down, SDL_SCANCODE_DOWN },
    { XK_Up, SDL_SCANCODE_UP },
    { XK_Num_Lock, SDL_SCANCODE_NUMLOCKCLEAR },
    { XK_KP_Divide, SDL_SCANCODE_KP_DIVIDE },
    { XK_KP_Multiply, SDL_SCANCODE_KP_MULTIPLY },
    { XK_KP_Subtract, SDL_SCANCODE_KP_MINUS },
    { XK_KP_Add, SDL_SCANCODE_KP_PLUS },
    { XK_KP_Enter, SDL_SCANCODE_KP_ENTER },
    { XK_KP_Delete, SDL_SCANCODE_KP_PERIOD },
    { XK_KP_End, SDL_SCANCODE_KP_1 },
    { XK_KP_Down, SDL_SCANCODE_KP_2 },
    { XK_KP_Next, SDL_SCANCODE_KP_3 },
    { XK_KP_Left, SDL_SCANCODE_KP_4 },
    { XK_KP_Begin, SDL_SCANCODE_KP_5 },
    { XK_KP_Right, SDL_SCANCODE_KP_6 },
    { XK_KP_Home, SDL_SCANCODE_KP_7 },
    { XK_KP_Up, SDL_SCANCODE_KP_8 },
    { XK_KP_Prior, SDL_SCANCODE_KP_9 },
    { XK_KP_Insert, SDL_SCANCODE_KP_0 },
    { XK_KP_Decimal, SDL_SCANCODE_KP_PERIOD },
    { XK_KP_1, SDL_SCANCODE_KP_1 },
    { XK_KP_2, SDL_SCANCODE_KP_2 },
    { XK_KP_3, SDL_SCANCODE_KP_3 },
    { XK_KP_4, SDL_SCANCODE_KP_4 },
    { XK_KP_5, SDL_SCANCODE_KP_5 },
    { XK_KP_6, SDL_SCANCODE_KP_6 },
    { XK_KP_7, SDL_SCANCODE_KP_7 },
    { XK_KP_8, SDL_SCANCODE_KP_8 },
    { XK_KP_9, SDL_SCANCODE_KP_9 },
    { XK_KP_0, SDL_SCANCODE_KP_0 },
    { XK_KP_Decimal, SDL_SCANCODE_KP_PERIOD },
    { XK_Hyper_R, SDL_SCANCODE_APPLICATION },
    { XK_KP_Equal, SDL_SCANCODE_KP_EQUALS },
    { XK_F13, SDL_SCANCODE_F13 },
    { XK_F14, SDL_SCANCODE_F14 },
    { XK_F15, SDL_SCANCODE_F15 },
    { XK_F16, SDL_SCANCODE_F16 },
    { XK_F17, SDL_SCANCODE_F17 },
    { XK_F18, SDL_SCANCODE_F18 },
    { XK_F19, SDL_SCANCODE_F19 },
    { XK_F20, SDL_SCANCODE_F20 },
    { XK_F21, SDL_SCANCODE_F21 },
    { XK_F22, SDL_SCANCODE_F22 },
    { XK_F23, SDL_SCANCODE_F23 },
    { XK_F24, SDL_SCANCODE_F24 },
    { XK_Execute, SDL_SCANCODE_EXECUTE },
    { XK_Help, SDL_SCANCODE_HELP },
    { XK_Menu, SDL_SCANCODE_MENU },
    { XK_Select, SDL_SCANCODE_SELECT },
    { XK_Cancel, SDL_SCANCODE_STOP },
    { XK_Redo, SDL_SCANCODE_AGAIN },
    { XK_Undo, SDL_SCANCODE_UNDO },
    { XK_Find, SDL_SCANCODE_FIND },
    { XK_KP_Separator, SDL_SCANCODE_KP_COMMA },
    { XK_Sys_Req, SDL_SCANCODE_SYSREQ },
    { XK_Control_L, SDL_SCANCODE_LCTRL },
    { XK_Shift_L, SDL_SCANCODE_LSHIFT },
    { XK_Alt_L, SDL_SCANCODE_LALT },
    { XK_Meta_L, SDL_SCANCODE_LGUI },
    { XK_Super_L, SDL_SCANCODE_LGUI },
    { XK_Control_R, SDL_SCANCODE_RCTRL },
    { XK_Shift_R, SDL_SCANCODE_RSHIFT },
    { XK_Alt_R, SDL_SCANCODE_RALT },
    { XK_Meta_R, SDL_SCANCODE_RGUI },
    { XK_Super_R, SDL_SCANCODE_RGUI },
    { XK_Mode_switch, SDL_SCANCODE_MODE },
};

void X11_InitKeymap(void)
{
}

Uint32 X11_TranslateKeycode(Display *display, KeyCode keycode)
{
    KeySym keysym;
    int i;

#ifdef USE_XKBKEYCODETOKEYSYM
    keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
#else
    keysym = XKeycodeToKeysym(display, keycode, 0);
#endif
    if (keysym == NoSymbol) {
        return SDL_SCANCODE_UNKNOWN;
    }

    if (keysym >= XK_A && keysym <= XK_Z) {
        return SDL_SCANCODE_A + (keysym - XK_A);
    }

    if (keysym >= XK_0 && keysym <= XK_9) {
        return SDL_SCANCODE_0 + (keysym - XK_0);
    }

    for (i = 0; i < (int)SDL_arraysize(KeySymToSDLScancode); ++i) {
        if (keysym == KeySymToSDLScancode[i].keysym) {
            return (Uint32)KeySymToSDLScancode[i].scancode;
        }
    }
    return (Uint32)SDL_SCANCODE_UNKNOWN;
}

#endif /* USE_SDL2 */

}; /* namespace GUI_GTK_X11 */

