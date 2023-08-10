/** @file rec_video_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ record video definition ]
*/

#ifndef RECORD_VIDEO_DEFS_H
#define RECORD_VIDEO_DEFS_H

#if (defined(__APPLE__) && defined(__MACH__))
// macosx version
#include <AvailabilityMacros.h>
#endif

// =====================================================================
// ---------------------------------------------------------------------
#if defined(USE_WIN) \
 || ((defined(USE_SDL) || defined(USE_SDL2)) && defined(_WIN32)) \
 || ((defined(USE_WX) || defined(USE_WX2)) && defined(_WIN32))
/// @ingroup Macros
/// @brief record video using video for windows
//#define USE_REC_VIDEO_VFW
#endif

// ---------------------------------------------------------------------
#if defined(USE_WIN)
/// @ingroup Macros
/// @brief record video using microsoft media foundation
//#define USE_REC_VIDEO_MMF		1
#endif

// ---------------------------------------------------------------------
#if ((defined(USE_SDL) || defined(USE_SDL2)) && defined(__APPLE__) && defined(__MACH__)) \
 || ((defined(USE_WX) || defined(USE_WX2)) && defined(__APPLE__) && defined(__MACH__))
/// @ingroup Macros
/// @brief record video using AVFoundation
//#define USE_REC_VIDEO_AVKIT		0x01
#endif

// ---------------------------------------------------------------------
#if !defined(MAC_OS_X_VERSION_10_13)
# if ((defined(USE_SDL) || defined(USE_SDL2)) && defined(__APPLE__) && defined(__MACH__)) \
 || ((defined(USE_WX) || defined(USE_WX2)) && defined(__APPLE__) && defined(__MACH__))
/// @ingroup Macros
/// @brief record video using QuickTime
//#define USE_REC_VIDEO_QTKIT		0x02
# endif
#endif

// ---------------------------------------------------------------------
/// @ingroup Macros
/// @brief record video using ffmpeg
//#define USE_REC_VIDEO_FFMPEG	1

// ---------------------------------------------------------------------
#if defined(USE_REC_VIDEO_VFW) \
 || defined(USE_REC_VIDEO_MMF) \
 || defined(USE_REC_VIDEO_AVKIT) \
 || defined(USE_REC_VIDEO_QTKIT) \
 || defined(USE_REC_VIDEO_FFMPEG)
/// @ingroup Macros
/// @brief can record video using any tool
#define USE_REC_VIDEO
#endif

// =====================================================================
// ---------------------------------------------------------------------
/// @ingroup Macros
/// @brief record audio with wav format
#define USE_REC_AUDIO_WAVE

// ---------------------------------------------------------------------
#if defined(USE_WIN)
/// @ingroup Macros
/// @brief record audio using microsoft media foundation
//#define USE_REC_AUDIO_MMF		2
#endif

// ---------------------------------------------------------------------
#if ((defined(USE_SDL) || defined(USE_SDL2)) && defined(__APPLE__) && defined(__MACH__)) \
 || ((defined(USE_WX) || defined(USE_WX2)) && defined(__APPLE__) && defined(__MACH__))
/// @ingroup Macros
/// @brief record audio using AVFoundation
//#define USE_REC_AUDIO_AVKIT		0x10
#endif

// ---------------------------------------------------------------------
/// @ingroup Macros
/// @brief record audio using ffmpeg
//#define USE_REC_AUDIO_FFMPEG	2

// ---------------------------------------------------------------------
#if defined(USE_REC_AUDIO_WAVE) \
 || defined(USE_REC_AUDIO_MMF) \
 || defined(USE_REC_AUDIO_AVKIT) \
 || defined(USE_REC_AUDIO_FFMPEG)
/// @ingroup Macros
/// @brief can record audio using any tool
#define USE_REC_AUDIO
#endif

// =====================================================================
// ---------------------------------------------------------------------
#if defined(USE_WIN) \
 || ((defined(USE_SDL) || defined(USE_SDL2)) && defined(_WIN32))
/// @ingroup Macros
/// @brief capture screen using GDI+
#define USE_CAP_SCREEN_WIN		1
#endif

// ---------------------------------------------------------------------
#if ((defined(USE_SDL) || defined(USE_SDL2)) && defined(__APPLE__) && defined(__MACH__))
/// @ingroup Macros
/// @brief capture screen using Cocoa
#define USE_CAP_SCREEN_COCOA		1
#endif

// ---------------------------------------------------------------------
#if ((defined(USE_SDL) || defined(USE_SDL2)) && !defined(_WIN32) && !defined(__APPLE__) && !defined(__MACH__))
/// @ingroup Macros
/// @brief capture screen using libpng
#define USE_CAP_SCREEN_LIBPNG	1
#endif

// ---------------------------------------------------------------------
#if (defined(USE_WX) || defined(USE_WX2))
/// @ingroup Macros
/// @brief capture screen using wxWidgets
//#define USE_CAP_SCREEN_WX		1
#endif

// ---------------------------------------------------------------------
#if defined(USE_QT)
/// @ingroup Macros
/// @brief capture screen using Qt
#define USE_CAP_SCREEN_QT		1
#endif

// ---------------------------------------------------------------------
#if defined(USE_CAP_SCREEN_WIN) \
 || defined(USE_CAP_SCREEN_COCOA) \
 || defined(USE_CAP_SCREEN_LIBPNG) \
 || defined(USE_CAP_SCREEN_WX) \
 || defined(USE_CAP_SCREEN_QT)
/// @ingroup Macros
/// @brief can caputure screen with PNG format
#define USE_CAPTURE_SCREEN_PNG
#endif

// =====================================================================

#endif /* RECORD_VIDEO_DEFS_H */
