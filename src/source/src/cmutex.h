/** @file cmutex.h

	@author Sasaji
	@date   2016.11.01

	@brief mutex
*/

#ifndef CMUTEX_H
#define CMUTEX_H

#include "common.h"

#if defined(USE_WIN)
#include <Windows.h>
#elif defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_mutex.h>
#elif defined(USE_QT)
class QMutex;
#elif defined(USE_WX) || defined(USE_WX2)
class wxMutex;
#endif

/**
	@brief Mutual Exclusion

	Wrapped CRITICAL_SECTION on Windows. Wrapped SDL_mutex on SDL.
*/
class CMutex
{
private:
#if defined(USE_WIN)
	CRITICAL_SECTION cs;
#elif defined(USE_SDL) || defined(USE_SDL2)
	SDL_mutex *mux;
#elif defined(USE_WX) || defined(USE_WX2)
	wxMutex *mux;
#elif defined(USE_QT)
	QMutex *mux;
#endif

public:
	CMutex();
	~CMutex();

	void lock();
	void unlock();
};

#endif /* CMUTEX_H */
