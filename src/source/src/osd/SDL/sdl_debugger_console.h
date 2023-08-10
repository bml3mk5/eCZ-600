/** @file sdl_debugger_console.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ sdl debugger console ]
*/

#ifndef SDL_DEBUGGER_CONSOLE_H
#define SDL_DEBUGGER_CONSOLE_H

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../../common.h"
#include "../../debugger_defs.h"
#include <SDL.h>

/**
	@brief Thread to run a debugger
*/
class DebuggerThread
{
public:
	DebuggerThread(debugger_thread_t *param);
	bool isRunning() const;
	void start();
	int wait();
private:
	SDL_Thread *th;
	debugger_thread_t *p;
};

#endif /* USE_DEBUGGER */
#endif /* SDL_DEBUGGER_CONSOLE_H */

