/** @file debugger_base.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.02-

	@brief [ debugger base ]
*/

#ifndef DEBUGGER_BASE_H
#define DEBUGGER_BASE_H

#include "vm_defs.h"

#ifdef USE_DEBUGGER

#include "device.h"
#include "../debugger_defs.h"
#include "../cchar.h"

class EMU;
class VM;

/**
	@brief debugger bus on VM
*/
class DEBUGGER_BUS_BASE : public DEVICE
{
protected:
	bool m_relate_on_cpu;
	CTchar m_name;

public:
	DEBUGGER_BUS_BASE(VM* parent_vm, EMU* parent_emu, const char *identifier);
	virtual ~DEBUGGER_BUS_BASE();

	void set_name(const _TCHAR *name) { m_name.Set(name); }
	void set_namef(const _TCHAR *format, ...);
	const _TCHAR *get_name() const { return m_name.Get(); }
	void rerate_to_cpu(bool val) { m_relate_on_cpu = val; }
	bool rerated_on_cpu() const { return m_relate_on_cpu; }
};

/**
	@brief debugger on VM

	This is a class to communicate to debugger thread, and manage break points.
*/
class DEBUGGER_BASE : public DEBUGGER_BUS_BASE
{
protected:
	CTchar m_file_path;
	int m_now_going;
	int m_now_debugging;
	bool m_now_suspended;
	DEBUGGER_BUS_BASE *d_detected;	// the device detected a break point 

public:
	DEBUGGER_BASE(VM* parent_vm, EMU* parent_emu, const char *identifier);
	virtual ~DEBUGGER_BASE();

	void clear_all();

	void set_file_path(const _TCHAR *file_path) { m_file_path.Set(file_path); }
	const _TCHAR *get_file_path() const { return m_file_path.Get(); }

	void start_debugging();
	void stop_debugging();

	void go_suspend();
	void go_suspend_at_first();
	bool now_suspend() const;
	virtual void clear_suspend();

	void now_going(int val) { m_now_going = val; }
	int  now_going() const { return m_now_going; }
//	void now_debugging(bool val) { m_now_debugging = val; }
	bool now_debugging() const { return (m_now_debugging != 0); }
	void now_suspended(bool val) { m_now_suspended = val; }
	bool now_suspended() const { return m_now_suspended; }

	void set_detected(DEBUGGER_BUS_BASE *dbg) { d_detected = dbg; }
	DEBUGGER_BUS_BASE *get_detected() { return d_detected; }
};

#endif /* USE_DEBUGGER */
#endif /* DEBUGGER_BASE_H */

