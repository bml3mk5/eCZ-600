/** @file vkeyboard_x68000.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.09.22 -

	@brief [ virtual keyboard layout for x68000 ]
*/

#ifndef VKEYBOARD_X68000_H
#define VKEYBOARD_X68000_H

#include "vkeyboard.h"

namespace Vkbd {

const struct stBitmap_t cBmpParts[] = {
//	{ BITMAPIDS_VKEY_LED0,  73, 223, 32, 32 },	// Hiragana LED On
//	{ BITMAPIDS_VKEY_LED0, 436, 223, 32, 32 },	// Zenkaku LED On
//	{ BITMAPIDS_VKEY_LED0, 538,  48, 32, 32 },	// Kana LED On
//	{ BITMAPIDS_VKEY_LED0, 571,  48, 32, 32 },	// Romaji LED On
//	{ BITMAPIDS_VKEY_LED0, 604,  48, 32, 32 },	// CodeIn LED On
//	{ BITMAPIDS_VKEY_LED0, 571,  91, 32, 32 },	// Insert LED On
//	{ BITMAPIDS_VKEY_LED0, 648,  48, 32, 32 },	// Caps LED On
	{ BITMAPIDS_VKEY_LED0,   0, 0, 32, 32 },	// Hiragana LED On
	{ BITMAPIDS_VKEY_LED0,  32, 0, 32, 32 },	// Zenkaku LED On
	{ BITMAPIDS_VKEY_LED0,  64, 0, 32, 32 },	// Kana LED On
	{ BITMAPIDS_VKEY_LED0,  96, 0, 32, 32 },	// Romaji LED On
	{ BITMAPIDS_VKEY_LED0, 128, 0, 32, 32 },	// CodeIn LED On
	{ BITMAPIDS_VKEY_LED0, 160, 0, 32, 32 },	// Insert LED On
	{ BITMAPIDS_VKEY_LED0, 192, 0, 32, 32 },	// Caps LED On
	{ -1, 0, 0, 0, 0 }
};

const struct stLedPos_t cLedPos[] = {
	{ BITMAPPARTS_HIRAGANA,  73, 223 },	// Hiragana
	{ BITMAPPARTS_ZENKAKU,  436, 223 },	// Zenkaku
	{ BITMAPPARTS_KANA,     538,  48 },	// Kana
	{ BITMAPPARTS_ROMAJI,   571,  48 },	// Romaji
	{ BITMAPPARTS_CODEIN,   604,  48 },	// CodeIn
	{ BITMAPPARTS_INSERT,   571,  91 },	// Insert
	{ BITMAPPARTS_CAPSLOCK, 648,  48 },	// Caps
//	{ BITMAPPARTS_HIRAGANA,   0, 0 },	// Hiragana
//	{ BITMAPPARTS_ZENKAKU,   32, 0 },	// Zenkaku
//	{ BITMAPPARTS_KANA,      64, 0 },	// Kana
//	{ BITMAPPARTS_ROMAJI,    96, 0 },	// Romaji
//	{ BITMAPPARTS_CODEIN,   128, 0 },	// CodeIn
//	{ BITMAPPARTS_INSERT,   160, 0 },	// Insert
//	{ BITMAPPARTS_CAPSLOCK, 192, 0 },	// Caps
	{ -1, 0, 0 }
};

const Hori_t cvKeyHori0[] = {
//     X    W  Code
	{108,  40, 0x63, 0, -1, -1, -1},	// F1
	{149,  40, 0x64, 0, -1, -1, -1},	// F2
	{190,  40, 0x65, 0, -1, -1, -1},	// F3
	{231,  40, 0x66, 0, -1, -1, -1},	// F4
	{272,  40, 0x67, 0, -1, -1, -1},	// F5
	{322,  40, 0x68, 0, -1, -1, -1},	// F6
	{363,  40, 0x69, 0, -1, -1, -1},	// F7
	{404,  40, 0x6a, 0, -1, -1, -1},	// F8
	{445,  40, 0x6b, 0, -1, -1, -1},	// F9
	{486,  40, 0x6c, 0, -1, -1, -1},	// F10
	{  0,   0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori1[] = {
//     X    W  Code
	{ 14,  32, 0x61, 0, -1, -1, -1},	// BREAK
	{ 55,  32, 0x62, 0, -1, -1, -1},	// COPY 

	{538,  32, 0x5a, 0, -1, -1, BITMAPPARTS_KANA},	// Kana
	{571,  32, 0x5b, 0, -1, -1, LEDPARTS_ROMAJI},	// Romaji
	{604,  32, 0x5c, 0, -1, -1, BITMAPPARTS_CODEIN},	// CodeIn

	{648,  32, 0x5d, 0, -1, -1, BITMAPPARTS_CAPSLOCK},	// CAPS
	{681,  32, 0x52, 0, -1, -1, -1},	// KigouIns
	{713,  32, 0x53, 0, -1, -1, -1},	// Touroku
	{747,  32, 0x54, 0, -1, -1, -1},	// Help

	{  0,   0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori2[] = {
//     X    W  Code
	{ 16,  32, 0x01, 0, -1, -1, -1},	// ESC
	{ 49,  32, 0x02, 0, -1, -1, -1},	// 1
	{ 82,  32, 0x03, 0, -1, -1, -1},	// 2
	{115,  32, 0x04, 0, -1, -1, -1},	// 3
	{148,  32, 0x05, 0, -1, -1, -1},	// 4
	{181,  32, 0x06, 0, -1, -1, -1},	// 5
	{214,  32, 0x07, 0, -1, -1, -1},	// 6
	{247,  32, 0x08, 0, -1, -1, -1},	// 7
	{280,  32, 0x09, 0, -1, -1, -1},	// 8
	{313,  32, 0x0a, 0, -1, -1, -1},	// 9
	{346,  32, 0x0b, 0, -1, -1, -1},	// 0
	{378,  32, 0x0c, 0, -1, -1, -1},	// -
	{412,  32, 0x0d, 0, -1, -1, -1},	// ^
	{445,  32, 0x0e, 0, -1, -1, -1},	// YEN
	{478,  48, 0x0f, 0, -1, -1, -1},	// BS

	{538,  32, 0x36, 0, -1, -1, -1},	// HOME
	{571,  32, 0x5e, 0, -1, -1, LEDPARTS_INSERT},	// INS
	{604,  32, 0x37, 0, -1, -1, -1},	// DEL

	{648,  32, 0x3f, 0, -1, -1, -1},	// CLR
	{681,  32, 0x40, 0, -1, -1, -1},	// num /
	{714,  32, 0x41, 0, -1, -1, -1},	// num *
	{747,  32, 0x42, 0, -1, -1, -1},	// num -

	{  0,   0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori3[] = {
//     X    W  Code
	{ 16, 48, 0x10, 0, -1, -1, -1},	// TAB
	{ 65, 32, 0x11, 0, -1, -1, -1},	// Q 
	{ 98, 32, 0x12, 0, -1, -1, -1},	// W
	{131, 32, 0x13, 0, -1, -1, -1},	// E 
	{164, 32, 0x14, 0, -1, -1, -1},	// R
	{197, 32, 0x15, 0, -1, -1, -1},	// T
	{230, 32, 0x16, 0, -1, -1, -1},	// Y 
	{263, 32, 0x17, 0, -1, -1, -1},	// U
	{294, 32, 0x18, 0, -1, -1, -1},	// I
	{329, 32, 0x19, 0, -1, -1, -1},	// O
	{362, 32, 0x1a, 0, -1, -1, -1},	// P
	{395, 32, 0x1b, 0, -1, -1, -1},	// @
	{428, 32, 0x1c, 0, -1, -1, -1},	// [
	{461, 8, 0x1d, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1, -1},	// RETURN(left)

	{538, 32, 0x38, 0, -1, -1, -1},	// ROLL UP
	{571, 32, 0x39, 0, -1, -1, -1},	// ROLL DOWN
	{604, 32, 0x3a, 0, -1, -1, -1},	// UNDO

	{648, 32, 0x43, 0, -1, -1, -1},	// num 7
	{681, 32, 0x44, 0, -1, -1, -1},	// num 8
	{714, 32, 0x45, 0, -1, -1, -1},	// num 9
	{747, 32, 0x46, 0, -1, -1, -1},	// num +

	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori3a[] = {
//     X    W  Code
	{469, 57, 0x1d, KEYKIND_ARRAY, ARRAYKEYS_RETURN, -1, -1},	// RETURN(right)
	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori4[] = {
//     X    W  Code
	{ 16, 56, 0x71, KEYKIND_TOGGLE, TOGGLEKEYS_CTRL, -1, -1},	// CTRL
	{ 73, 32, 0x1e, 0, -1, -1, -1},	// A
	{105, 32, 0x1f, 0, -1, -1, -1},	// S
	{138, 32, 0x20, 0, -1, -1, -1},	// D
	{171, 32, 0x21, 0, -1, -1, -1},	// F
	{204, 32, 0x22, 0, -1, -1, -1},	// G
	{237, 32, 0x23, 0, -1, -1, -1},	// H
	{271, 32, 0x24, 0, -1, -1, -1},	// J
	{304, 32, 0x25, 0, -1, -1, -1},	// K
	{337, 32, 0x26, 0, -1, -1, -1},	// L
	{370, 32, 0x27, 0, -1, -1, -1},	// ;
	{403, 32, 0x28, 0, -1, -1, -1},	// :
	{434, 32, 0x29, 0, -1, -1, -1},	// ]

	{571, 32, 0x3c, 0, -1, -1, -1},	// Up

	{648, 32, 0x47, 0, -1, -1, -1},	// num 4
	{681, 32, 0x48, 0, -1, -1, -1},	// num 5
	{714, 32, 0x49, 0, -1, -1, -1},	// num 6
	{747, 32, 0x4a, 0, -1, -1, -1},	// num =

	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori4a[] = {
//     X    W  Code
	{538, 32, 0x3b, 0, -1, -1, -1},	// Left
	{604, 32, 0x3d, 0, -1, -1, -1},	// Right
	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori5[] = {
//     X    W  Code
	{ 16, 72, 0x70, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1, -1},	// LSHIFT 
	{ 89, 32, 0x2a, 0, -1, -1, -1},	// Z
	{122, 32, 0x2b, 0, -1, -1, -1},	// X
	{155, 32, 0x2c, 0, -1, -1, -1},	// C
	{188, 32, 0x2d, 0, -1, -1, -1},	// V
	{221, 32, 0x2e, 0, -1, -1, -1},	// B
	{254, 32, 0x2f, 0, -1, -1, -1},	// N
	{287, 32, 0x30, 0, -1, -1, -1},	// M
	{320, 32, 0x31, 0, -1, -1, -1},	// ,
	{353, 32, 0x32, 0, -1, -1, -1},	// .
	{386, 32, 0x33, 0, -1, -1, -1},	// /
	{419, 32, 0x34, 0, -1, -1, -1},	// _
	{452, 74, 0x70, KEYKIND_TOGGLE, TOGGLEKEYS_SHIFT, -1, -1},	// RSHIFT

	{571, 32, 0x3e, 0, -1, -1, -1},	// Down

	{648, 32, 0x4b, 0, -1, -1, -1},	// num 1
	{681, 32, 0x4c, 0, -1, -1, -1},	// num 2
	{714, 32, 0x4d, 0, -1, -1, -1},	// num 3

	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori5a[] = {
//     X    W  Code
	{747, 32, 0x4e, 0, -1, -1, -1},	// num Enter

	{  0,  0,    0, 0, -1, -1, -1}
};

const Hori_t cvKeyHori6[] = {
//     X    W  Code
	{ 73,  32, 0x5f, 0, -1, -1, BITMAPPARTS_HIRAGANA},	// Hiragana
	{106,  40, 0x55, 0, -1, -1, -1},	// XF1
	{147,  40, 0x56, 0, -1, -1, -1},	// XF2
	{188, 116, 0x35, 0, -1, -1, -1},	// Space
	{305,  48, 0x57, 0, -1, -1, -1},	// XF3
	{354,  40, 0x58, 0, -1, -1, -1},	// XF4
	{395,  40, 0x59, 0, -1, -1, -1},	// XF5
	{436,  32, 0x60, 0, -1, -1, BITMAPPARTS_ZENKAKU},	// Zenkaku

	{538,  48, 0x72, 0, -1, -1, -1},	// OPT.1
	{588,  48, 0x73, 0, -1, -1, -1},	// OPT.2

	{648,  32, 0x4f, 0, -1, -1, -1},	// num 0
	{681,  32, 0x50, 0, -1, -1, -1},	// num ,
	{714,  32, 0x51, 0, -1, -1, -1},	// num .

	{  0,   0,    0, 0, -1, -1, -1}
};

const Pos_t cVkbdKeyPos[] = {
//     Y    H  Array
	{  59, 21, cvKeyHori0 },	// PF Key
	{  48, 32, cvKeyHori1 },	// line 1
	{  91, 32, cvKeyHori2 },	// line 2
	{ 124, 32, cvKeyHori3 },	// line 3
	{ 124, 65, cvKeyHori3a },	// line 3 (return)
	{ 157, 32, cvKeyHori4 },	// line 4
	{ 157, 65, cvKeyHori4a },	// line 4 (left / right)
	{ 190, 32, cvKeyHori5 },	// line 5
	{ 190, 65, cvKeyHori5a },	// line 5 (num Enter)
	{ 223, 32, cvKeyHori6 },	// line 6
	{ 0, 0, NULL }
};

} /* namespace Vkbd */

#endif /* VKEYBOARD_X68000_H */
