/** @file version.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ version info ]
*/

#ifndef VERSION_H
#define VERSION_H

#define APP_NAME		"SHARP X68000 Emulator 'eCZ-600'"
#define APP_FILENAME	"x68000.exe"
#define APP_INTERNAME	"eCZ-600"
#define APP_COPYRIGHT	"Copyright (C) 2011,2012-2024 Common Source Code Project, Sasaji"
#define APP_VERSION		"0.0.1.291"
#define APP_VER_MAJOR	0
#define APP_VER_MINOR	0
#define APP_VER_REV		1
#define APP_VER_BUILD	291

#if defined(__MINGW32__)
#if defined(x86_64) || defined(__x86_64)
#define PLATFORM "Windows(MinGW) 64bit"
#elif defined(i386) || defined(__i386)
#define PLATFORM "Windows(MinGW) 32bit"
#else
#define PLATFORM "Windows(MinGW)"
#endif
#elif defined(_WIN32)
#if defined(_WIN64) || defined(_M_X64)
#define PLATFORM "Windows 64bit"
#else
#define PLATFORM "Windows 32bit"
#endif
#elif defined(linux)
#ifdef __x86_64
#define PLATFORM "Linux 64bit"
#elif __i386
#define PLATFORM "Linux 32bit"
#else
#define PLATFORM "Linux"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#ifdef __arm64
#define PLATFORM "MacOS Arm 64bit"
#elif __x86_64
#define PLATFORM "MacOS Intel 64bit"
#elif __i386
#define PLATFORM "MacOS Intel 32bit"
#else
#define PLATFORM "MacOS"
#endif
#elif defined(__FreeBSD__)
#ifdef __x86_64
#define PLATFORM "FreeBSD 64bit"
#elif __i386
#define PLATFORM "FreeBSD 32bit"
#else
#define PLATFORM "FreeBSD"
#endif
#else
#define PLATFORM "Unknown"
#endif

#endif /* VERSION_H */
