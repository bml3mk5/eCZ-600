/** @file emu_input.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ emu input ]
*/

#ifndef EMU_INPUT_H
#define EMU_INPUT_H

#define KEY_KEEP_FRAMES 3

#if defined(USE_WIN)
#include "windows/win_input.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "SDL/sdl_input.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "wxwidgets/wxw_input.h"
#elif defined(USE_QT)
#include "qt/qt_input.h"
#endif

#ifdef USE_AUTO_KEY

#define AUTO_KEY_SHIFT	0x70
#define AUTO_KEY_RETURN	0x1d
#define AUTO_KEY_KANA	0x5a
#define AUTO_KEY_CODEINPUT	0x5c
#define AUTO_KEY_NONE	0xff
#define AUTO_KEY_MASK		0x00ff
#define AUTO_KEY_SHIFT_MASK	0x0100
#define AUTO_KEY_UPPER_MASK	0x0400
#define AUTO_KEY_LOWER_MASK	0x0800
#define AUTO_KEY_KANA_MASK	0x1000
#define AUTO_KEY_KANJI_MASK	0x4000

static const int autokey_table[256] = {
	// 0x100: shift
	// 0x400: alphabet
	// 0x800: ALPHABET
	// 0x1000: hankaku katakana
	// use vm key scan code 0x80 - 0xfe
//  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f               
//0                                           BEL   BS    HT    LF    VT    FF    CR
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x00f,0x010,0x03e,0x03c,0x03d,0x01d,0x000,0x000,
//1                                                             SUB   EC                RS
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x03f,0x001,0x000,0x000,0x000,0x000,
//2
	0x035,0x102,0x103,0x104,0x105,0x106,0x107,0x108,0x109,0x10a,0x128,0x127,0x031,0x00c,0x032,0x033,
//3
	0x00b,0x002,0x003,0x004,0x005,0x006,0x007,0x008,0x009,0x00a,0x028,0x027,0x131,0x10c,0x132,0x133,
//4
	0x01b,0x41e,0x42e,0x42c,0x420,0x413,0x421,0x422,0x423,0x418,0x424,0x425,0x426,0x430,0x42f,0x419,
//5
	0x41a,0x411,0x414,0x41f,0x415,0x417,0x42d,0x412,0x42b,0x416,0x42a,0x01c,0x00e,0x029,0x00d,0x134,
//6
	0x11b,0x81e,0x82e,0x82c,0x820,0x813,0x821,0x822,0x823,0x818,0x824,0x825,0x826,0x830,0x82f,0x819,
//7
	0x81a,0x811,0x814,0x81f,0x815,0x817,0x82d,0x812,0x82b,0x816,0x82a,0x11c,0x10e,0x129,0x10d,0x034,
//8
	// Shift-JIS
	0x0000,0x1132,0x111c,0x1129,0x1131,0x1133,0x110b,0x1104,0x1113,0x1105,0x1106,0x1107,0x1108,0x1109,0x110a,0x112a,
//9
	0x100e,0x1004,0x1013,0x1005,0x1006,0x1007,0x1015,0x1022,0x1023,0x1028,0x102e,0x102b,0x1020,0x1014,0x101a,0x102c,
//a
	// katakana
	0x0000,0x1132,0x111c,0x1129,0x1131,0x1133,0x110b,0x1104,0x1113,0x1105,0x1106,0x1107,0x1108,0x1109,0x110a,0x112a,
	0x100e,0x1004,0x1013,0x1005,0x1006,0x1007,0x1015,0x1022,0x1023,0x1028,0x102e,0x102b,0x1020,0x1014,0x101a,0x102c,
	0x1011,0x101e,0x102a,0x1012,0x101f,0x1017,0x1018,0x1002,0x1031,0x1025,0x1021,0x102d,0x1003,0x100d,0x100c,0x1024,
	0x102f,0x1029,0x1033,0x1030,0x1008,0x1009,0x100a,0x1019,0x1026,0x1032,0x1027,0x1034,0x100b,0x1016,0x101b,0x101c,
//e
	// Shift-JIS
	0x1011,0x101e,0x102a,0x1012,0x101f,0x1017,0x1018,0x1002,0x1031,0x1025,0x1021,0x102d,0x1003,0x100d,0x100c,0x1024,
//f
	0x102f,0x1029,0x1033,0x1030,0x1008,0x1009,0x100a,0x1019,0x1026,0x1032,0x1027,0x1034,0x100b,0x1016,0x101b,0x0000
};
#endif

#endif	/* EMU_INPUT_H */
