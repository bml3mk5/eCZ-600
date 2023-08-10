/** @file cocoa_key_trans.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @note based on SDL_QuartzKeys.h, SDL_QuartzEvents.m

 @brief [ key translate ]
 */

#import "cocoa_key_trans.h"
#include <Carbon/Carbon.h>
#if 0
#include <IOKit/IOKitLib.h>	/* For powersave handling */
#include <IOKit/hidsystem/IOLLEvent.h>
#endif
#include "../../emu.h"

extern EMU *emu;


#ifndef USE_SDL2

/* The translation table from a macintosh key scancode to a SDL keysym */
static SDLKey             keymap[256];        /* Mac OS X to SDL key mapping */
//static Uint32 translate_sdlkey(int scancode, Uint32 sdl_keycode);
static int initialized = 0;

/* These are the Macintosh key scancode constants -- from Inside Macintosh */

#define QZ_ESCAPE		0x35
#define QZ_F1			0x7A
#define QZ_F2			0x78
#define QZ_F3			0x63
#define QZ_F4			0x76
#define QZ_F5			0x60
#define QZ_F6			0x61
#define QZ_F7			0x62
#define QZ_F8			0x64
#define QZ_F9			0x65
#define QZ_F10			0x6D
#define QZ_F11			0x67
#define QZ_F12			0x6F
#define QZ_F13			0x69
#define QZ_F14			0x6B
#define QZ_F15			0x71
/*
#define QZ_PRINT		0x69
#define QZ_SCROLLOCK    0x6B
#define QZ_PAUSE		0x71
*/
#define QZ_POWER		0x7F
#define QZ_BACKQUOTE	0x32
#define QZ_1			0x12
#define QZ_2			0x13
#define QZ_3			0x14
#define QZ_4			0x15
#define QZ_5			0x17
#define QZ_6			0x16
#define QZ_7			0x1A
#define QZ_8			0x1C
#define QZ_9			0x19
#define QZ_0			0x1D
#define QZ_MINUS		0x1B
#define QZ_EQUALS		0x18
#define QZ_BACKSPACE	0x33
#define QZ_INSERT		0x72
#define QZ_HOME			0x73
#define QZ_PAGEUP		0x74
#define QZ_NUMLOCK		0x47
#define QZ_KP_EQUALS	0x51
#define QZ_KP_DIVIDE	0x4B
#define QZ_KP_MULTIPLY	0x43
#define QZ_TAB			0x30
#define QZ_q			0x0C
#define QZ_w			0x0D
#define QZ_e			0x0E
#define QZ_r			0x0F
#define QZ_t			0x11
#define QZ_y			0x10
#define QZ_u			0x20
#define QZ_i			0x22
#define QZ_o			0x1F
#define QZ_p			0x23
#define QZ_LEFTBRACKET	0x21
#define QZ_RIGHTBRACKET	0x1E
#define QZ_BACKSLASH	0x2A
#define QZ_DELETE		0x75
#define QZ_END			0x77
#define QZ_PAGEDOWN		0x79
#define QZ_KP7			0x59
#define QZ_KP8			0x5B
#define QZ_KP9			0x5C
#define QZ_KP_MINUS		0x4E
#define QZ_CAPSLOCK		0x39
#define QZ_a			0x00
#define QZ_s			0x01
#define QZ_d			0x02
#define QZ_f			0x03
#define QZ_g			0x05
#define QZ_h			0x04
#define QZ_j			0x26
#define QZ_k			0x28
#define QZ_l			0x25
#define QZ_SEMICOLON	0x29
#define QZ_QUOTE		0x27
#define QZ_RETURN		0x24
#define QZ_KP4			0x56
#define QZ_KP5			0x57
#define QZ_KP6			0x58
#define QZ_KP_PLUS		0x45
#define QZ_LSHIFT		0x38
#define QZ_z			0x06
#define QZ_x			0x07
#define QZ_c			0x08
#define QZ_v			0x09
#define QZ_b			0x0B
#define QZ_n			0x2D
#define QZ_m			0x2E
#define QZ_COMMA		0x2B
#define QZ_PERIOD		0x2F
#define QZ_SLASH		0x2C
#if 1	/* Panther now defines right side keys */
#define QZ_RSHIFT		0x3C
#endif
#define QZ_UP			0x7E
#define QZ_KP1			0x53
#define QZ_KP2			0x54
#define QZ_KP3			0x55
#define QZ_KP_ENTER		0x4C
#define QZ_LCTRL		0x3B
#define QZ_LALT			0x3A
#define QZ_LMETA		0x37
#define QZ_SPACE		0x31
#if 1	/* Panther now defines right side keys */
#define QZ_RMETA		0x36
#define QZ_RALT			0x3D
#define QZ_RCTRL		0x3E
#endif
#define QZ_LEFT			0x7B
#define QZ_DOWN			0x7D
#define QZ_RIGHT		0x7C
#define QZ_KP0			0x52
#define QZ_KP_PERIOD	0x41

/* Wierd, these keys are on my iBook under Mac OS X */
#define QZ_IBOOK_ENTER		0x34
#define QZ_IBOOK_LEFT		0x3B
#define QZ_IBOOK_RIGHT		0x3C
#define QZ_IBOOK_DOWN		0x3D
#define QZ_IBOOK_UP			0x3E

#define QZ_FN			0x3F

void SDL_QZ_InitOSKeymap()
{
    BOOL saw_layout = NO;
    UInt32 state;
    UInt32 value;
    Uint16 i;
    int world = SDLK_WORLD_0;

	/* alreay initialized? */
	if (initialized) return;

    for ( i=0; i<SDL_TABLESIZE(keymap); ++i )
        keymap[i] = SDLK_UNKNOWN;

    /* This keymap is almost exactly the same as the OS 9 one */
    keymap[QZ_ESCAPE] = SDLK_ESCAPE;
    keymap[QZ_F1] = SDLK_F1;
    keymap[QZ_F2] = SDLK_F2;
    keymap[QZ_F3] = SDLK_F3;
    keymap[QZ_F4] = SDLK_F4;
    keymap[QZ_F5] = SDLK_F5;
    keymap[QZ_F6] = SDLK_F6;
    keymap[QZ_F7] = SDLK_F7;
    keymap[QZ_F8] = SDLK_F8;
    keymap[QZ_F9] = SDLK_F9;
    keymap[QZ_F10] = SDLK_F10;
    keymap[QZ_F11] = SDLK_F11;
    keymap[QZ_F12] = SDLK_F12;
    keymap[QZ_F13] = SDLK_F13;
    keymap[QZ_F14] = SDLK_F14;
    keymap[QZ_F15] = SDLK_F15;
/*
    keymap[QZ_PRINT] = SDLK_PRINT;
    keymap[QZ_SCROLLOCK] = SDLK_SCROLLOCK;
    keymap[QZ_PAUSE] = SDLK_PAUSE;
*/
    keymap[QZ_POWER] = SDLK_POWER;
    keymap[QZ_BACKQUOTE] = SDLK_BACKQUOTE;
    keymap[QZ_1] = SDLK_1;
    keymap[QZ_2] = SDLK_2;
    keymap[QZ_3] = SDLK_3;
    keymap[QZ_4] = SDLK_4;
    keymap[QZ_5] = SDLK_5;
    keymap[QZ_6] = SDLK_6;
    keymap[QZ_7] = SDLK_7;
    keymap[QZ_8] = SDLK_8;
    keymap[QZ_9] = SDLK_9;
    keymap[QZ_0] = SDLK_0;
    keymap[QZ_MINUS] = SDLK_MINUS;
    keymap[QZ_EQUALS] = SDLK_EQUALS;
    keymap[QZ_BACKSPACE] = SDLK_BACKSPACE;
    keymap[QZ_INSERT] = SDLK_INSERT;
    keymap[QZ_HOME] = SDLK_HOME;
    keymap[QZ_PAGEUP] = SDLK_PAGEUP;
    keymap[QZ_NUMLOCK] = SDLK_NUMLOCK;
    keymap[QZ_KP_EQUALS] = SDLK_KP_EQUALS;
    keymap[QZ_KP_DIVIDE] = SDLK_KP_DIVIDE;
    keymap[QZ_KP_MULTIPLY] = SDLK_KP_MULTIPLY;
    keymap[QZ_TAB] = SDLK_TAB;
    keymap[QZ_q] = SDLK_q;
    keymap[QZ_w] = SDLK_w;
    keymap[QZ_e] = SDLK_e;
    keymap[QZ_r] = SDLK_r;
    keymap[QZ_t] = SDLK_t;
    keymap[QZ_y] = SDLK_y;
    keymap[QZ_u] = SDLK_u;
    keymap[QZ_i] = SDLK_i;
    keymap[QZ_o] = SDLK_o;
    keymap[QZ_p] = SDLK_p;
    keymap[QZ_LEFTBRACKET] = SDLK_LEFTBRACKET;
    keymap[QZ_RIGHTBRACKET] = SDLK_RIGHTBRACKET;
    keymap[QZ_BACKSLASH] = SDLK_BACKSLASH;
    keymap[QZ_DELETE] = SDLK_DELETE;
    keymap[QZ_END] = SDLK_END;
    keymap[QZ_PAGEDOWN] = SDLK_PAGEDOWN;
    keymap[QZ_KP7] = SDLK_KP7;
    keymap[QZ_KP8] = SDLK_KP8;
    keymap[QZ_KP9] = SDLK_KP9;
    keymap[QZ_KP_MINUS] = SDLK_KP_MINUS;
    keymap[QZ_CAPSLOCK] = SDLK_CAPSLOCK;
    keymap[QZ_a] = SDLK_a;
    keymap[QZ_s] = SDLK_s;
    keymap[QZ_d] = SDLK_d;
    keymap[QZ_f] = SDLK_f;
    keymap[QZ_g] = SDLK_g;
    keymap[QZ_h] = SDLK_h;
    keymap[QZ_j] = SDLK_j;
    keymap[QZ_k] = SDLK_k;
    keymap[QZ_l] = SDLK_l;
    keymap[QZ_SEMICOLON] = SDLK_SEMICOLON;
    keymap[QZ_QUOTE] = SDLK_QUOTE;
    keymap[QZ_RETURN] = SDLK_RETURN;
    keymap[QZ_KP4] = SDLK_KP4;
    keymap[QZ_KP5] = SDLK_KP5;
    keymap[QZ_KP6] = SDLK_KP6;
    keymap[QZ_KP_PLUS] = SDLK_KP_PLUS;
    keymap[QZ_LSHIFT] = SDLK_LSHIFT;
    keymap[QZ_RSHIFT] = SDLK_RSHIFT;
    keymap[QZ_z] = SDLK_z;
    keymap[QZ_x] = SDLK_x;
    keymap[QZ_c] = SDLK_c;
    keymap[QZ_v] = SDLK_v;
    keymap[QZ_b] = SDLK_b;
    keymap[QZ_n] = SDLK_n;
    keymap[QZ_m] = SDLK_m;
    keymap[QZ_COMMA] = SDLK_COMMA;
    keymap[QZ_PERIOD] = SDLK_PERIOD;
    keymap[QZ_SLASH] = SDLK_SLASH;
    keymap[QZ_UP] = SDLK_UP;
    keymap[QZ_KP1] = SDLK_KP1;
    keymap[QZ_KP2] = SDLK_KP2;
    keymap[QZ_KP3] = SDLK_KP3;
    keymap[QZ_KP_ENTER] = SDLK_KP_ENTER;
    keymap[QZ_LCTRL] = SDLK_LCTRL;
    keymap[QZ_LALT] = SDLK_LALT;
    keymap[QZ_LMETA] = SDLK_LMETA;
    keymap[QZ_RCTRL] = SDLK_RCTRL;
    keymap[QZ_RALT] = SDLK_RALT;
    keymap[QZ_RMETA] = SDLK_RMETA;
    keymap[QZ_SPACE] = SDLK_SPACE;
    keymap[QZ_LEFT] = SDLK_LEFT;
    keymap[QZ_DOWN] = SDLK_DOWN;
    keymap[QZ_RIGHT] = SDLK_RIGHT;
    keymap[QZ_KP0] = SDLK_KP0;
    keymap[QZ_KP_PERIOD] = SDLK_KP_PERIOD;
//    keymap[QZ_IBOOK_ENTER] = SDLK_KP_ENTER;
//    keymap[QZ_IBOOK_RIGHT] = SDLK_RIGHT;
//    keymap[QZ_IBOOK_DOWN] = SDLK_DOWN;
//    keymap[QZ_IBOOK_UP]      = SDLK_UP;
//    keymap[QZ_IBOOK_LEFT] = SDLK_LEFT;

    /* 
        Up there we setup a static scancode->keysym map. However, it will not
        work very well on international keyboard. Hence we now query MacOS
        for its own keymap to adjust our own mapping table. However, this is
        basically only useful for ascii char keys. This is also the reason
        why we keep the static table, too.
     */

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= 1050)
    if (TISCopyCurrentKeyboardLayoutInputSource() != NULL) {
        TISInputSourceRef src = TISCopyCurrentKeyboardLayoutInputSource();
        if (src != NULL) {
            CFDataRef data = (CFDataRef)
                TISGetInputSourceProperty(src,
                    kTISPropertyUnicodeKeyLayoutData);
            if (data != NULL) {
                const UCKeyboardLayout *layout = (const UCKeyboardLayout *)
                    CFDataGetBytePtr(data);
                if (layout != NULL) {
                    const UInt32 kbdtype = LMGetKbdType();
                    saw_layout = YES;

                    /* Loop over all 127 possible scan codes */
                    for (i = 0; i < 0x7F; i++) {
                        UniChar buf[16];
                        UniCharCount count = 0;

                        /* We pretend a clean start to begin with (i.e. no dead keys active */
                        state = 0;

                        if (UCKeyTranslate(layout, i, kUCKeyActionDown, 0, kbdtype,
                                           0, &state, 16, &count, buf) != noErr) {
                            continue;
                        }

                        /* If the state become 0, it was a dead key. We need to
                           translate again, passing in the new state, to get
                           the actual key value */
                        if (state != 0) {
                            if (UCKeyTranslate(layout, i, kUCKeyActionDown, 0, kbdtype,
                                               0, &state, 16, &count, buf) != noErr) {
                                continue;
                            }
                        }

                        if (count != 1) {
                            continue;  /* no multi-char. Use SDL 1.3 instead. :) */
                        }

                        value = (UInt32) buf[0];
                        if (value >= 128) {
                            /* Some non-ASCII char, map it to SDLK_WORLD_* */
                            if (world < 0xFF) {
                                keymap[i] = (SDLKey)world++;
                            }
                        } else if (value >= 32) {     /* non-control ASCII char */
                            keymap[i] = (SDLKey)value;
                        }
                    }
                }
            }
            CFRelease(src);
        }
    }
#endif

#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1050)
    if (!saw_layout) {
        /* Get a pointer to the systems cached KCHR */
        const void *KCHRPtr = (const void *)GetScriptManagerVariable(smKCHRCache);
        if (KCHRPtr)
        {
            /* Loop over all 127 possible scan codes */
            for (i = 0; i < 0x7F; i++)
            {
                /* We pretend a clean start to begin with (i.e. no dead keys active */
                state = 0;

                /* Now translate the key code to a key value */
                value = KeyTranslate(KCHRPtr, i, &state) & 0xff;

                /* If the state become 0, it was a dead key. We need to translate again,
                    passing in the new state, to get the actual key value */
                if (state != 0)
                    value = KeyTranslate(KCHRPtr, i, &state) & 0xff;

                /* Now we should have an ascii value, or 0. Try to figure out to which SDL symbol it maps */
                if (value >= 128) {     /* Some non-ASCII char, map it to SDLK_WORLD_* */
                    if (world < 0xFF) {
                        keymap[i] = world++;
                    }
                } else if (value >= 32) {     /* non-control ASCII char */
                    keymap[i] = value;
                }
            }
        }
    }
#endif

    /* 
        The keypad codes are re-setup here, because the loop above cannot
        distinguish between a key on the keypad and a regular key. We maybe
        could get around this problem in another fashion: NSEvent's flags
        include a "NSNumericPadKeyMask" bit; we could check that and modify
        the symbol we return on the fly. However, this flag seems to exhibit
        some weird behaviour related to the num lock key
    */
    keymap[QZ_KP0] = SDLK_KP0;
    keymap[QZ_KP1] = SDLK_KP1;
    keymap[QZ_KP2] = SDLK_KP2;
    keymap[QZ_KP3] = SDLK_KP3;
    keymap[QZ_KP4] = SDLK_KP4;
    keymap[QZ_KP5] = SDLK_KP5;
    keymap[QZ_KP6] = SDLK_KP6;
    keymap[QZ_KP7] = SDLK_KP7;
    keymap[QZ_KP8] = SDLK_KP8;
    keymap[QZ_KP9] = SDLK_KP9;
    keymap[QZ_KP_MINUS] = SDLK_KP_MINUS;
    keymap[QZ_KP_PLUS] = SDLK_KP_PLUS;
    keymap[QZ_KP_PERIOD] = SDLK_KP_PERIOD;
    keymap[QZ_KP_EQUALS] = SDLK_KP_EQUALS;
    keymap[QZ_KP_DIVIDE] = SDLK_KP_DIVIDE;
    keymap[QZ_KP_MULTIPLY] = SDLK_KP_MULTIPLY;
    keymap[QZ_KP_ENTER] = SDLK_KP_ENTER;

    keymap[QZ_FN] = SDLK_WORLD_5;

	initialized = 1;
}

int SDL_QZ_HandleKeyEvents(NSEvent *event) {
	short scancode = (short)[event keyCode];
	int code = keymap[scancode];

	emu->translate_keysym(0, code, scancode, &code);
	return code;
}

#else /* !USE_SDL2 */

#import "scancodes_darwin.h"

static void UpdateKeymap();

void SDL_QZ_InitOSKeymap() {
    UpdateKeymap();

    /* Set our own names for the platform-dependent but layout-independent keys */
    /* This key is NumLock on the MacBook keyboard. :) */
    /*SDL_SetScancodeName(SDL_SCANCODE_NUMLOCKCLEAR, "Clear");*/
//    SDL_SetScancodeName(SDL_SCANCODE_LALT, "Left Option");
//    SDL_SetScancodeName(SDL_SCANCODE_LGUI, "Left Command");
//    SDL_SetScancodeName(SDL_SCANCODE_RALT, "Right Option");
//    SDL_SetScancodeName(SDL_SCANCODE_RGUI, "Right Command");
}

int SDL_QZ_HandleKeyEvents(NSEvent *event) {
	unsigned short scancode = [event keyCode];
    SDL_Scancode code;
#if 0
    const char *text;
#endif

	if ((scancode == 10 || scancode == 50) && KBGetLayoutType(LMGetKbdType()) == kKeyboardISO) {
        /* see comments in SDL_cocoakeys.h */
        scancode = 60 - scancode;
    }
    if (scancode < SDL_arraysize(darwin_scancode_table)) {
        code = darwin_scancode_table[scancode];
    }
    else {
        /* Hmm, does this ever happen?  If so, need to extend the keymap... */
        code = SDL_SCANCODE_UNKNOWN;
    }

	int new_code = 0;
	emu->translate_keysym(0, 0, (short)code, &new_code);
	return new_code;
}

static void UpdateKeymap()
{
#if 0
    TISInputSourceRef key_layout;
    const void *chr_data;
    int i;
    SDL_Scancode scancode;
    SDL_Keycode keymap[SDL_NUM_SCANCODES];

    /* See if the keymap needs to be updated */
    key_layout = TISCopyCurrentKeyboardLayoutInputSource();

    SDL_GetDefaultKeymap(keymap);

    /* Try Unicode data first */
    CFDataRef uchrDataRef = TISGetInputSourceProperty(key_layout, kTISPropertyUnicodeKeyLayoutData);
    if (uchrDataRef)
        chr_data = CFDataGetBytePtr(uchrDataRef);
    else
        goto cleanup;

    if (chr_data) {
        UInt32 keyboard_type = LMGetKbdType();
        OSStatus err;

        for (i = 0; i < SDL_arraysize(darwin_scancode_table); i++) {
            UniChar s[8];
            UniCharCount len;
            UInt32 dead_key_state;

            /* Make sure this scancode is a valid character scancode */
            scancode = darwin_scancode_table[i];
            if (scancode == SDL_SCANCODE_UNKNOWN ||
                (keymap[scancode] & SDLK_SCANCODE_MASK)) {
                continue;
            }

            dead_key_state = 0;
            err = UCKeyTranslate ((UCKeyboardLayout *) chr_data,
                                  i, kUCKeyActionDown,
                                  0, keyboard_type,
                                  kUCKeyTranslateNoDeadKeysMask,
                                  &dead_key_state, 8, &len, s);
            if (err != noErr)
                continue;

            if (len > 0 && s[0] != 0x10) {
                keymap[scancode] = s[0];
            }
        }
        SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
        return;
    }

cleanup:
    CFRelease(key_layout);
#endif
}

#endif /* USE_SDL2 */

