/** @file sdl_debugger_console.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ sdl debugger console ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "sdl_debugger_console.h"
#include "../debugger_console.h"

static int SDLCALL debugger_thread(void *lpx)
{
	debugger_thread_t *p = (debugger_thread_t *)lpx;

	p->running = true;

	// initialize
	DebuggerConsole *dc = new DebuggerConsole(p);

	dc->Process();

	// release
	delete dc;

	p->running = false;

	return 0;
}

DebuggerThread::DebuggerThread(debugger_thread_t *param)
{
	th = NULL;
	p = param;
}

void DebuggerThread::start()
{
#if defined(USE_SDL) || defined(USE_WX)
	th = SDL_CreateThread(debugger_thread, p);
#elif defined(USE_SDL2) || defined(USE_WX2)
	th = SDL_CreateThread(debugger_thread, "debugger_thread", p);
#endif
}

int DebuggerThread::wait()
{
	int status = 0;
	if (th) {
		SDL_WaitThread(th, &status);
	}
	return status;
}

bool DebuggerThread::isRunning() const
{
	return (th != NULL);
}

#endif /* USE_DEBUGGER */
