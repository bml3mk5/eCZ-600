/** @file keycode.h

	Skelton for retropc emulator


	@author Sasaji
	@date   2015.04.30

	@brief [ keycode ]
*/

#ifndef KEYCODE_DEFINES_H
#define KEYCODE_DEFINES_H

// ----------------------------------------------------------------------

#define KEYCODE_KEEP_FRAMES		0x8000

#define KEYCODE_EXTENDED		0x100

// ----------------------------------------------------------------------

#define KEYCODE_UNKNOWN			0x00

#define KEYCODE_LSHIFT			0x01	//
#define KEYCODE_RSHIFT			0x02	//
#define KEYCODE_LCTRL			0x03	//
#define KEYCODE_RCTRL			0x04	//
#define KEYCODE_LALT			0x05	// Left alt / option
#define KEYCODE_RALT			0x06	// Right alt / option

#define KEYCODE_BACKSPACE		0x08	// VK_BACK
#define KEYCODE_TAB				0x09	// VK_TAB

#define KEYCODE_LSUPER			0x0a	// LWIN
#define KEYCODE_RSUPER			0x0b	// RWIN

#define KEYCODE_CLEAR			0x0c	// VK_CLEAR
#define KEYCODE_RETURN			0x0d	// VK_RETURN

#define KEYCODE_SHIFT			0x10	// VK_SHIFT
#define KEYCODE_CONTROL			0x11	// VK_CONTROL
#define KEYCODE_ALT				0x12	// VK_MENU

#define KEYCODE_PAUSE			0x13	// VK_PAUSE
#define KEYCODE_CAPSLOCK		0x14	// VK_CAPITAL

#define KEYCODE_KATAHIRA		0x15	// katakana hiragana
#define KEYCODE_MUHENKAN		0x16	// muhenkan
#define KEYCODE_HENKAN			0x17	// henkan
#define KEYCODE_GRAVE			0x18	// ` Hankaku Zenkaku Kanji
#define KEYCODE_EISU			0x19	// eisu
#define KEYCODE_KANA			0x1a	// kana

#define KEYCODE_ESCAPE			0x1b	// VK_ESCAPE

#define KEYCODE_RIGHT			0x1c	// RIGHT
#define KEYCODE_LEFT			0x1d	// LEFT
#define KEYCODE_UP				0x1e	// UP
#define KEYCODE_DOWN			0x1f	// DOWN

#define KEYCODE_SPACE			0x20	// VK_SPACE

#define KEYCODE_PAGEUP			0x21	// VK_PRIOR
#define KEYCODE_PAGEDOWN		0x22	// VK_NEXT
#define KEYCODE_END				0x23	// 
#define KEYCODE_HOME			0x24	// 
#define KEYCODE_INSERT			0x25	// 
#define KEYCODE_DELETE			0x26	//

//#define KEYCODE_EXCLAIM		0x21	// !
//#define KEYCODE_QUOTEDBL		0x22	// "
//#define KEYCODE_HASH			0x23	// #
//#define KEYCODE_DOLLAR		0x24	// $
//#define KEYCODE_PERCENT		0x25	// %
//#define KEYCODE_AMPERSAND		0x26	// &
#define KEYCODE_QUOTE			0x27	// '
#define KEYCODE_LPAREN			0x28	// (
#define KEYCODE_RPAREN			0x29	// )
#define KEYCODE_ASTERISK		0x2a	// *
#define KEYCODE_PLUS			0x2b	// +
#define KEYCODE_COMMA			0x2c	// ,
#define KEYCODE_MINUS			0x2d	// -
#define KEYCODE_PERIOD			0x2e	// .
#define KEYCODE_SLASH			0x2f	// /

#define KEYCODE_0				0x30
#define KEYCODE_1				0x31
#define KEYCODE_2				0x32
#define KEYCODE_3				0x33
#define KEYCODE_4				0x34
#define KEYCODE_5				0x35
#define KEYCODE_6				0x36
#define KEYCODE_7				0x37
#define KEYCODE_8				0x38
#define KEYCODE_9				0x39

#define KEYCODE_COLON			0x3a	// :
#define KEYCODE_SEMICOLON		0x3b	// ;
#define KEYCODE_LESS			0x3c	// <
#define KEYCODE_EQUALS			0x3d	// =
#define KEYCODE_GREATER			0x3e	// >
#define KEYCODE_QUESTION		0x3f	// ?
#define KEYCODE_AT				0x40	// @

#define KEYCODE_A				0x41
#define KEYCODE_B				0x42
#define KEYCODE_C				0x43
#define KEYCODE_D				0x44
#define KEYCODE_E				0x45
#define KEYCODE_F				0x46
#define KEYCODE_G				0x47
#define KEYCODE_H				0x48
#define KEYCODE_I				0x49
#define KEYCODE_J				0x4a
#define KEYCODE_K				0x4b
#define KEYCODE_L				0x4c
#define KEYCODE_M				0x4d
#define KEYCODE_N				0x4e
#define KEYCODE_O				0x4f
#define KEYCODE_P				0x50
#define KEYCODE_Q				0x51
#define KEYCODE_R				0x52
#define KEYCODE_S				0x53
#define KEYCODE_T				0x54
#define KEYCODE_U				0x55
#define KEYCODE_V				0x56
#define KEYCODE_W				0x57
#define KEYCODE_X				0x58
#define KEYCODE_Y				0x59
#define KEYCODE_Z				0x5a

#define KEYCODE_LBRACKET		0x5b	// [
#define KEYCODE_BACKSLASH		0x5c	//
#define KEYCODE_RBRACKET		0x5d	// ]
#define KEYCODE_CARET			0x5e	// ^
#define KEYCODE_UNDERSCORE		0x5f	// _

#define KEYCODE_KP_0			0x60	// VK_NUMPAD0
#define KEYCODE_KP_1			0x61	// VK_NUMPAD1
#define KEYCODE_KP_2			0x62	// VK_NUMPAD2
#define KEYCODE_KP_3			0x63	// VK_NUMPAD3
#define KEYCODE_KP_4			0x64	// VK_NUMPAD4
#define KEYCODE_KP_5			0x65	// VK_NUMPAD5
#define KEYCODE_KP_6			0x66	// VK_NUMPAD6
#define KEYCODE_KP_7			0x67	// VK_NUMPAD7
#define KEYCODE_KP_8			0x68	// VK_NUMPAD8
#define KEYCODE_KP_9			0x69	// VK_NUMPAD9

#define KEYCODE_KP_MULTIPLY		0x6a	// VK_MULTIPLY
#define KEYCODE_KP_PLUS			0x6b	// VK_ADD
#define KEYCODE_KP_ENTER		0x6c
#define KEYCODE_KP_MINUS		0x6d	// VK_SUBTRACT
#define KEYCODE_KP_PERIOD		0x6e	// VK_DECIMAL
#define KEYCODE_KP_DIVIDE		0x6f	// VK_DIVIDE

#define KEYCODE_F1				0x70	// VK_F1
#define KEYCODE_F2				0x71	// VK_F2
#define KEYCODE_F3				0x72	// VK_F3
#define KEYCODE_F4				0x73	// VK_F4
#define KEYCODE_F5				0x74	// VK_F5
#define KEYCODE_F6				0x75	// VK_F6
#define KEYCODE_F7				0x76	// VK_F7
#define KEYCODE_F8				0x77	// VK_F8
#define KEYCODE_F9				0x78	// VK_F9
#define KEYCODE_F10				0x79	// VK_F10
#define KEYCODE_F11				0x7a	// VK_F11
#define KEYCODE_F12				0x7b	// VK_F12
#define KEYCODE_F13				0x7c	// VK_F13
#define KEYCODE_F14				0x7d	// VK_F14
#define KEYCODE_F15				0x7e	// VK_F15
// for mac keyboard
#define KEYCODE_F16				0x7f	// VK_F16
#define KEYCODE_F17				0x80	// VK_F17	// VK_F1
#define KEYCODE_F18				0x81	// VK_F18
#define KEYCODE_F19				0x82	// VK_F19
#define KEYCODE_F20				0x83	// VK_F20
#define KEYCODE_F21				0x84	// VK_F21
#define KEYCODE_F22				0x85	// VK_F22

#define KEYCODE_WORLD_0			0x86
#define KEYCODE_WORLD_1			0x87
#define KEYCODE_WORLD_2			0x88
#define KEYCODE_WORLD_3			0x89
#define KEYCODE_WORLD_4			0x8a
#define KEYCODE_WORLD_5			0x8b
#define KEYCODE_WORLD_6			0x8c
#define KEYCODE_WORLD_7			0x8d
#define KEYCODE_WORLD_8			0x8e
#define KEYCODE_WORLD_9			0x8f

#define KEYCODE_NUMLOCK			0x90	// VK_NUMLOCK
#define KEYCODE_SCROLLLOCK		0x91	// VK_SCROLL
#define KEYCODE_SYSREQ			0x92	// Print Screen / SysRq

#define KEYCODE_KP_EQUALS		0x93	

#define KEYCODE_HELP			0x94
#define KEYCODE_PRINT			0x95
#define KEYCODE_MENU			0x96	// application
#define KEYCODE_POWER			0x97
#define KEYCODE_EURO			0x98
#define KEYCODE_UNDO			0x99

#define KEYCODE_LGUI			0x9a	// Left command (MAC)
#define KEYCODE_RGUI			0x9b	// Right command (MAC)
#define KEYCODE_MODE			0x9c
#define KEYCODE_COMPOSE			0x9d

#define KEYCODE_SELECT			0x9e

#define KEYCODE_FUNCTION		0x9f	// fn

#define KEYCODE_JISHO			0xa0
#define KEYCODE_MASSHOU			0xa1
#define KEYCODE_TOUROKU			0xa2
#define KEYCODE_LOYAYUBI		0xa3
#define KEYCODE_ROYAYUBI		0xa4

#define KEYCODE_BROWSER_BACK	0xa6
#define KEYCODE_BROWSER_FORWARD	0xa7
#define KEYCODE_BROWSER_REFRESH	0xa8
#define KEYCODE_BROWSER_STOP	0xa9
#define KEYCODE_BROWSER_SEARCH	0xaa
#define KEYCODE_BROWSER_FAVOR	0xab
#define KEYCODE_BROWSER_HOME	0xac

#define KEYCODE_VOLUME_MUTE		0xad
#define KEYCODE_VOLUME_DOWN		0xae
#define KEYCODE_VOLUME_UP		0xaf

#define KEYCODE_KP_COMMA		0xb0

// ----------------------------------------------------------------------

#define GLOBALKEY_RETURN		0x0d
#define GLOBALKEY_CONTROL		0x11
#define GLOBALKEY_0				0x30
#define GLOBALKEY_1				0x31
#define GLOBALKEY_2				0x32
#define GLOBALKEY_3				0x33
#define GLOBALKEY_4				0x34
#define GLOBALKEY_5				0x35
#define GLOBALKEY_6				0x36
#define GLOBALKEY_7				0x37
#define GLOBALKEY_8				0x38
#define GLOBALKEY_9				0x39
#define GLOBALKEY_A				0x41
#define GLOBALKEY_B				0x42
#define GLOBALKEY_C				0x43
#define GLOBALKEY_D				0x44
#define GLOBALKEY_E				0x45
#define GLOBALKEY_F				0x46
#define GLOBALKEY_G				0x47
#define GLOBALKEY_H				0x48
#define GLOBALKEY_I				0x49
#define GLOBALKEY_J				0x4a
#define GLOBALKEY_K				0x4b
#define GLOBALKEY_L				0x4c
#define GLOBALKEY_M				0x4d
#define GLOBALKEY_N				0x4e
#define GLOBALKEY_O				0x4f
#define GLOBALKEY_P				0x50
#define GLOBALKEY_Q				0x51
#define GLOBALKEY_R				0x52
#define GLOBALKEY_S				0x53
#define GLOBALKEY_T				0x54
#define GLOBALKEY_U				0x55
#define GLOBALKEY_V				0x56
#define GLOBALKEY_W				0x57
#define GLOBALKEY_X				0x58
#define GLOBALKEY_Y				0x59
#define GLOBALKEY_Z				0x5a
#define GLOBALKEY_KP_0			0x60
#define GLOBALKEY_KP_1			0x61
#define GLOBALKEY_KP_2			0x62
#define GLOBALKEY_KP_3			0x63
#define GLOBALKEY_KP_4			0x64
#define GLOBALKEY_KP_5			0x65
#define GLOBALKEY_KP_6			0x66
#define GLOBALKEY_KP_7			0x67
#define GLOBALKEY_KP_8			0x68
#define GLOBALKEY_KP_9			0x69
#define GLOBALKEY_m				0x6d
#define GLOBALKEY_SFT_0			0xc0
#define GLOBALKEY_SFT_1			0xc1
#define GLOBALKEY_SFT_2			0xc2
#define GLOBALKEY_SFT_3			0xc3
#define GLOBALKEY_SFT_4			0xc4
#define GLOBALKEY_SFT_5			0xc5
#define GLOBALKEY_SFT_6			0xc6
#define GLOBALKEY_SFT_7			0xc7
#define GLOBALKEY_SFT_8			0xc8
#define GLOBALKEY_SFT_9			0xc9
#define GLOBALKEY_F1			0x11a
#define GLOBALKEY_F2			0x11b
#define GLOBALKEY_F3			0x11c
#define GLOBALKEY_F4			0x11d
#define GLOBALKEY_F5			0x11e
#define GLOBALKEY_F6			0x11f
#define GLOBALKEY_F7			0x120
#define GLOBALKEY_F8			0x121
#define GLOBALKEY_F9			0x122
#define GLOBALKEY_F10			0x123
#define GLOBALKEY_F11			0x124
#define GLOBALKEY_F12			0x125
#define GLOBALKEY_F13			0x126
#define GLOBALKEY_F14			0x127
#define GLOBALKEY_F15			0x128

// ----------------------------------------------------------------------

#define JOYCODE_UP				0x00000001
#define JOYCODE_DOWN			0x00000002
#define JOYCODE_LEFT			0x00000004
#define JOYCODE_RIGHT			0x00000008

#define JOYCODE_UPLEFT			0x00000005
#define JOYCODE_DOWNLEFT		0x00000006

#define JOYCODE_UPRIGHT			0x00000009
#define JOYCODE_DOWNRIGHT		0x0000000a

#define JOYCODE_R_UP			0x00000010
#define JOYCODE_R_DOWN			0x00000020
#define JOYCODE_Z_LEFT			0x00000040
#define JOYCODE_Z_RIGHT			0x00000080

#define JOYCODE_RZ_UPLEFT		0x00000050
#define JOYCODE_RZ_DOWNLEFT		0x00000060

#define JOYCODE_RZ_UPRIGHT		0x00000090
#define JOYCODE_RZ_DOWNRIGHT	0x000000a0

#define JOYCODE_V_UP			0x00000100
#define JOYCODE_V_DOWN			0x00000200
#define JOYCODE_U_LEFT			0x00000400
#define JOYCODE_U_RIGHT			0x00000800

#define JOYCODE_VU_UPLEFT		0x00000500
#define JOYCODE_VU_DOWNLEFT		0x00000600

#define JOYCODE_VU_UPRIGHT		0x00000900
#define JOYCODE_VU_DOWNRIGHT	0x00000a00

#define JOYCODE_POV_UP			0x00001000
#define JOYCODE_POV_DOWN		0x00002000
#define JOYCODE_POV_LEFT		0x00004000
#define JOYCODE_POV_RIGHT		0x00008000

#define JOYCODE_POV_UPLEFT		0x00005000
#define JOYCODE_POV_DOWNLEFT	0x00006000

#define JOYCODE_POV_UPRIGHT		0x00009000
#define JOYCODE_POV_DOWNRIGHT	0x0000a000

#define JOYCODE_BTN_A			0x00010000
#define JOYCODE_BTN_B			0x00020000
#define JOYCODE_BTN_C			0x00040000
#define JOYCODE_BTN_D			0x00080000
#define JOYCODE_BTN_E			0x00100000
#define JOYCODE_BTN_F			0x00200000
#define JOYCODE_BTN_G			0x00400000
#define JOYCODE_BTN_H			0x00800000
#define JOYCODE_BTN_I			0x01000000
#define JOYCODE_BTN_J			0x02000000
#define JOYCODE_BTN_K			0x04000000
#define JOYCODE_BTN_L			0x08000000
#define JOYCODE_BTN_M			0x10000000
#define JOYCODE_BTN_N			0x20000000
#define JOYCODE_BTN_O			0x40000000
#define JOYCODE_BTN_P			0x80000000

// ----------------------------------------------------------------------

#define JOYCODE_ANA_Y			0x00000001
#define JOYCODE_ANA_Y_REV		0x00000002
#define JOYCODE_ANA_X			0x00000004
#define JOYCODE_ANA_X_REV		0x00000008
#define JOYCODE_ANA_R			0x00000010
#define JOYCODE_ANA_R_REV		0x00000020
#define JOYCODE_ANA_Z			0x00000040
#define JOYCODE_ANA_Z_REV		0x00000080
#define JOYCODE_ANA_V			0x00000100
#define JOYCODE_ANA_V_REV		0x00000200
#define JOYCODE_ANA_U			0x00000400
#define JOYCODE_ANA_U_REV		0x00000800

// ----------------------------------------------------------------------

#endif	/* KEYCODE_DEFINES_H */
