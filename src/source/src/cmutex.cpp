/** @file cmutex.cpp

	@author Sasaji
	@date   2016.11.01

	@brief mutex
*/

#include "cmutex.h"

#if defined(USE_QT)
#include <QMutex>
#elif defined(USE_WX) || defined(USE_WX2)
#include <wx/thread.h>
#endif

CMutex::CMutex()
{
#if defined(USE_WIN)
	InitializeCriticalSection(&cs);
#elif defined(USE_SDL) || defined(USE_SDL2)
	mux = SDL_CreateMutex();
#elif defined(USE_WX) || defined(USE_WX2)
	mux = new wxMutex();
#elif defined(USE_QT)
	mux = new QMutex();
#endif
}

CMutex::~CMutex()
{
#if defined(USE_WIN)
	DeleteCriticalSection(&cs);
#elif defined(USE_SDL) || defined(USE_SDL2)
	SDL_DestroyMutex(mux);
#elif defined(USE_WX) || defined(USE_WX2)
	delete mux;
#elif defined(USE_QT)
	delete mux;
#endif
}

void CMutex::lock()
{
#if defined(USE_WIN)
	EnterCriticalSection(&cs);
#elif defined(USE_SDL) || defined(USE_SDL2)
	SDL_mutexP(mux);
#elif defined(USE_WX) || defined(USE_WX2)
	mux->Lock();
#elif defined(USE_QT)
	mux->lock();
#endif
}

void CMutex::unlock()
{
#if defined(USE_WIN)
	LeaveCriticalSection(&cs);
#elif defined(USE_SDL) || defined(USE_SDL2)
	SDL_mutexV(mux);
#elif defined(USE_WX) || defined(USE_WX2)
	mux->Unlock();
#elif defined(USE_QT)
	mux->unlock();
#endif
}

