/** @file x68000_defs.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ virtual machine ]
*/

#ifndef X68000_DEFS_H
#define X68000_DEFS_H

#define FRAME_SPLIT_NUM	1

#define DEVICE_NAME		"SHARP X68000"
#define CONFIG_NAME		"x68000"
#define CLASS_NAME      "X68000"
#define CONFIG_VERSION		1

// device informations for virtual machine
#define USE_EMU_INHERENT_SPEC

#define FRAMES_PER_10SECS	554.6
#define FRAMES_PER_SEC		55.46
#define LINES_PER_FRAME 	568
#define CHARS_PER_LINE		137
#define SUPPORT_VARIABLE_TIMING

#define USE_MC68000	1
//#define USE_MC68008	1
//#define USE_MC68020MMU	1
#define USE_MC68000_IRQ_LEVEL

#define CPU_CLOCKS		 10000000
#define NUMBER_OF_CPUS		1
#define USE_CPU_REAL_MACHINE_CYCLE	1
#define USE_MEM_REAL_MACHINE_CYCLE	1
#define CLOCKS_CYCLE    300000000	// need divisible by 30
#define MAIN_SUB_CLOCK_RATIO 0

#define USE_DMA_MEMORY_MAPPED_IO

#define MAX_SOUND	4

//#define SCREEN_WIDTH		768
#define SCREEN_WIDTH		800
//#define SCREEN_HEIGHT		512
#define SCREEN_HEIGHT		600
#define LIMIT_MIN_WINDOW_WIDTH		768
#define LIMIT_MIN_WINDOW_HEIGHT		512
#define MIN_WINDOW_WIDTH		768
#define MIN_WINDOW_HEIGHT		512
#define MAX_WINDOW_WIDTH		800
#define MAX_WINDOW_HEIGHT		600
#define SCREEN_DEST_X		((SCREEN_WIDTH  - LIMIT_MIN_WINDOW_WIDTH ) / 2)
#define SCREEN_DEST_Y		((SCREEN_HEIGHT - LIMIT_MIN_WINDOW_HEIGHT) / 2)

// max devices connected to the output port
#define MAX_OUTPUT	18

// device informations for win32
#define USE_SPECIAL_RESET
//#define USE_DATAREC
//#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		3
//#define USE_AUTO_KEY_CAPS
#define USE_SCANLINE
//#define USE_DIPSWITCH
//#define DIPSWITCH_DEFAULT 0x03
#define HAS_YM2151
#define USE_FMGEN_STEREO
//#define USE_AUDIO_U8

#define USE_PRINTER
#define MAX_PRINTER		1
#define USE_MOUSE
//#define USE_MOUSE_ABSOLUTE
#define USE_JOYSTICK
#define USE_PIAJOYSTICK
//#define USE_PIAJOYSTICKBIT
#define USE_KEY2JOYSTICK
#define USE_ANALOG_JOYSTICK
#define USE_JOYSTICK_TYPE


#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
//#define HAS_MB8876
#define MAX_DRIVE		2
#define USE_DRIVE		2

#define USE_HD1
#define MAX_HARD_DISKS	1

#define USE_SOCKET
#define USE_UART
#define MAX_COMM		1

#define USE_STATE
#define USE_KEY_RECORD

#define USE_LEDBOX
#define USE_MESSAGE_BOARD
#define USE_VKEYBOARD

#if defined(USE_WIN)
#define USE_SCREEN_MIX_SURFACE

#define USE_SCREEN_D3D_TEXTURE
//#define USE_SCREEN_D3D_MIX_SURFACE

#define USE_DIRECTINPUT

#elif defined(USE_SDL)
#define USE_SCREEN_MIX_SURFACE

#elif defined(USE_SDL2)
#define USE_SCREEN_MIX_SURFACE

#define USE_SCREEN_SDL2_MIX_ON_RENDERER
#define USE_SCREEN_OPENGL_MIX_ON_RENDERER

#endif

//#define USE_PERFORMANCE_METER

#define RESUME_FILE_HEADER "RESUME_X68000"
#define RESUME_FILE_VERSION 1
#define RESUME_FILE_REVISION 1

#define KEYBIND_KEYS	136
#define KEYBIND_JOYS	28
#define KEYBIND_ASSIGN	2
#define KEYBIND_PRESETS	4

#define KEYBIND_JOY_BUTTONS	10

//#define _FDC_DEBUG_LOG

#endif /* X68000_DEFS_H */
