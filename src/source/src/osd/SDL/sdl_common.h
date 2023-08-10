/** @file sdl_common.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.11.01 -

	@brief [ common header ]
*/

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
#include "inttypes.h"
#else
#include <stdint.h>
#endif

#include <string.h>

#if defined(_WIN32)
#include <tchar.h>
#else
#include "../../extra/tchar.h"
#endif

#if defined(_MSC_VER)
#include <stdlib.h>

// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if (_MSC_VER >= 1400)
#pragma warning( disable : 4100 )
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

// check memory leaks
#if defined(_DEBUG)
//#define _CRTDBG_MAP_ALLOC
#undef _malloca
#include <crtdbg.h>
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#if _MSC_VER > 1500
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

#endif /* _MSC_VER */

//#if defined(USE_GTK)
//#include <gtk/gtk.h>
//#if defined(USE_OPENGL)
//#if !GTK_CHECK_VERSION(3,16,0)
//#undef USE_OPENGL
//#endif
//#endif /* USE_OPENGL */
//#endif /* USE_GTK */

/********** platform dependency **********/
#if defined(_WIN32)
/********** windows **********/

#if !defined(_UNICODE) && !defined(USE_WIN)
// Treat char string as UTF-8 on SDL on Windows.
// So, needs converting to MBCS string when call Windows API directly.
#define USE_UTF8_ON_MBCS	1
#endif

#elif (defined(__APPLE__) && defined(__MACH__))
/********** macosx **********/
#include <unistd.h>
#define __stdcall

#else
/********** linux (Unix) **********/
#include <unistd.h>
#define __stdcall

#define USE_LIB_GTK
#ifdef GUI_TYPE_AGAR
#define USE_X11_LEDBOX
#define USE_X11_VKEYBOARD
#else
#define USE_GTK_LEDBOX
#define USE_GTK_VKEYBOARD
#endif

#endif

#if defined(USE_SDL) || defined(USE_WX)
/* SDL1 */

#define CDelay(ms) SDL_Delay(ms)

# define SDL_Keycode		SDLKey

# define SDL_WINDOW_FULLSCREEN	SDL_FULLSCREEN
# define SDL_WINDOW_OPENGL		SDL_OPENGL

#elif defined(USE_SDL2)
/* SDL2 */

#define CDelay(ms) SDL_Delay(ms)

# if defined(GUI_TYPE_GTK_X11)
//#  define GUI_USE_FOREIGN_WINDOW
# endif

#endif

#endif /* SDL_COMMON_H */
