/** @file debugger_defs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [ defines for debugger ]
*/

#ifndef DEBUGGER_DEFS_H
#define DEBUGGER_DEFS_H

#include "common.h"
#include "vm/vm_defs.h"

#ifdef USE_DEBUGGER

class EMU;
class VM;

/// store status on debugger thread
typedef struct {
	EMU *emu;
	VM *vm;
	int cpu_index;
	int num_of_cpus;
	bool running;
	bool request_terminate;
} debugger_thread_t;

#endif

#endif /* DEBUGGER_DEFS_H */
