/** @file depend.h

	Skelton for retropc emulator


	@author Sasaji
	@date   2012.03.01

	@brief [ dependency definition ]
*/

#ifndef DEPEND_H
#define DEPEND_H

//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <sys/types.h>
//#include <sys/stat.h>

//======================================================================

/* Support Outside Ledbox */
#if defined(_WIN32) \
 ||(defined(__APPLE__) && defined(__MACH__)) \
 || defined(USE_SDL2_LEDBOX) \
 || defined(USE_X11_LEDBOX) \
 || defined(USE_GTK_LEDBOX)
#define USE_OUTSIDE_LEDBOX 1
#endif

/* Resource Directory */
#ifdef _WIN32
#define RESDIR "res\\"
#elif defined(__APPLE__) && defined(__MACH__)
#define RESDIR "../Resources/"
#else
#define RESDIR "res/"
#endif

#ifdef NO_USE_WINAPI
#define _stricmp strcasecmp
#define _wcsicmp wcscasecmp
#endif

//======================================================================
#ifndef _WIN32
	/* MacOSX or Unix */

#define _stricmp strcasecmp
#define _wcsicmp wcscasecmp

#endif /* !_WIN32 */

//======================================================================
#ifdef _UNICODE
	/* _UNICODE */

#ifndef _WIN32
wchar_t *_wgetenv(const wchar_t *);
int _wsystem(const wchar_t *);
# endif /* !_WIN32 */

//======================================================================
#else
	/* !_UNICODE */

//======================================================================
#endif	/* _UNICODE */

#endif	/* DEPEND_H */
