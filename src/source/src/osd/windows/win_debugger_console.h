/** @file win_debugger_console.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ win debugger console ]
*/

#ifndef WIN_DEBUGGER_CONSOLE_H
#define WIN_DEBUGGER_CONSOLE_H

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../../common.h"
#include "../../debugger_defs.h"
#include <windows.h>

/**
	@brief Thread to run a debugger
*/
class DebuggerThread
{
public:
	DebuggerThread(debugger_thread_t *param);
	~DebuggerThread();
	bool isRunning() const;
	void start();
	int wait();
private:
	HANDLE th;
	debugger_thread_t *p;
};

#endif /* USE_DEBUGGER */
#endif /* WIN_DEBUGGER_CONSOLE_H */

