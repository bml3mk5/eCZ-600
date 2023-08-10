/** @file win_debugger_console.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ win debugger console ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "win_debugger_console.h"
#include "../debugger_console.h"

static unsigned __stdcall debugger_thread(void *lpx)
{
	debugger_thread_t *p = (debugger_thread_t *)lpx;

	p->running = true;

	// initialize
	DebuggerConsole *dc = new DebuggerConsole(p);

	dc->Process();

	// release
	delete dc;

	p->running = false;

	_endthreadex(0);
	return 0;
}

DebuggerThread::DebuggerThread(debugger_thread_t *param)
{
	th = (HANDLE)0;
	p = param;
}

DebuggerThread::~DebuggerThread()
{
	if (th) {
		CloseHandle(th);
	}
}

void DebuggerThread::start()
{
	th = (HANDLE)_beginthreadex(NULL, 0, debugger_thread, p, 0, NULL);
}

int DebuggerThread::wait()
{
	int status = 0;
	if (th != (HANDLE)0) {
		status = (DWORD)WaitForSingleObject(th, INFINITE);
	}
	return status;
}

bool DebuggerThread::isRunning() const
{
	return (th != (HANDLE)0);
}

#endif /* USE_DEBUGGER */
