/** @file debugger.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2014.09.02-

	@brief [ debugger ]

	@note Modified by Sasaji at 2016.02.01 -
*/

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "vm_defs.h"

#ifdef USE_DEBUGGER

#include "../debugger_defs.h"
#include "../debugger_bpoint.h"
#include "../debugger_symbol.h"
#include "../cchar.h"

#define DEBUGGER_COMMAND_LEN	128
#define DEBUGGER_COMMAND_HISTORY	32

//#define DEBUGGER_MAX_BUFF 1024

class EMU;
class DebuggerConsole;
class DEBUGGER_BPOINTS;
class DEBUGGER_SYMBOLS;
class DEBUGGER;
class DEBUGGER_BUS;
class CMutex;

/**
	@brief debugger on VM

	This is a class to communicate to debugger thread, and manage break points.
*/
class DEBUGGER : public DEBUGGER_BPOINTS, public DEBUGGER_SYMBOLS
{
private:
	DEVICE *d_mem, *d_io;
	DebuggerConsole *dc;

// ----------------------------------------------------------------------------

public:
	DEBUGGER(VM* parent_vm, EMU* parent_emu, const char *identifier);
	~DEBUGGER();

//	friend DEBUGGER_BUS;

	// common functions
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_data16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data16w(uint32_t addr, int* wait);
	void write_data32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data32w(uint32_t addr, int* wait);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void latch_address(uint32_t addr, int *wait);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	void write_io32(uint32_t addr, uint32_t data);
	uint32_t read_io32(uint32_t addr);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void write_io16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io16w(uint32_t addr, int* wait);
	void write_io32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io32w(uint32_t addr, int* wait);

	void write_dma_data16(uint32_t addr, uint32_t data);
	uint32_t read_dma_data16(uint32_t addr);
	void write_dma_data_n(uint32_t addr, uint32_t data, int width);
	uint32_t read_dma_data_n(uint32_t addr, int width);

	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	void set_debugger_console(DebuggerConsole *dc);

	bool reach_break_point_at(uint32_t addr);

	void check_break_points(uint32_t addr);
	void check_intr_break_points(uint32_t addr, uint32_t mask);
	void check_exception_break_points(uint32_t addr, uint32_t vector);

	void clear_suspend();

	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	DEVICE *get_context_mem() const
	{
		return d_mem;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	DEVICE *get_context_io() const
	{
		return d_io;
	}
};

/**
	@brief debugger for MEMORY or IO
*/
class DEBUGGER_BUS : public DEBUGGER_BUS_BASE
{
private:
	DEVICE *d_mem, *d_io;
	DEBUGGER *d_parent;

// ----------------------------------------------------------------------------

public:
	DEBUGGER_BUS(VM* parent_vm, EMU* parent_emu, const char *identifier);
	~DEBUGGER_BUS();

//	friend DEBUGGER;

	// common functions
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_data16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data16w(uint32_t addr, int* wait);
	void write_data32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data32w(uint32_t addr, int* wait);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void latch_address(uint32_t addr, int *wait);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	uint32_t read_io16(uint32_t addr);
	void write_io32(uint32_t addr, uint32_t data);
	uint32_t read_io32(uint32_t addr);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void write_io16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io16w(uint32_t addr, int* wait);
	void write_io32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io32w(uint32_t addr, int* wait);

	void write_dma_data_n(uint32_t addr, uint32_t data, int width);
	uint32_t read_dma_data_n(uint32_t addr, int width);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io16(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	void update_intr_condition();


	uint32_t debug_read_data8(int type, uint32_t addr);
	uint32_t debug_read_data16(int type, uint32_t addr);
	uint32_t debug_read_data32(int type, uint32_t addr);

	// unique functions
	DEVICE *get_debugger() {
		return (DEVICE *)this;
	}
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	DEVICE *get_context_mem() const
	{
		return d_mem;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	DEVICE *get_context_io() const
	{
		return d_io;
	}
	void set_parent(DEBUGGER* debugger)
	{
		d_parent = debugger;
	}
};

#endif /* USE_DEBUGGER */
#endif /* DEBUGGER_H */

