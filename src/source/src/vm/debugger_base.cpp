/** @file debugger_base.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.02-

	@brief [ debugger base ]
*/

#include "debugger_base.h"

#ifdef USE_DEBUGGER

#include "../emu.h"
#include "../utility.h"

// ----------------------------------------------------------------------------

DEBUGGER_BUS_BASE::DEBUGGER_BUS_BASE(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	m_relate_on_cpu = false;
}

DEBUGGER_BUS_BASE::~DEBUGGER_BUS_BASE()
{
}

void DEBUGGER_BUS_BASE::set_namef(const _TCHAR *format, ...)
{
	_TCHAR ident[_MAX_PATH];
	va_list ap;
	va_start(ap, format);
	UTILITY::vstprintf(ident, sizeof(ident)/sizeof(ident[0]), format, ap);
	va_end(ap);
	m_name.Set(ident);
}

// ----------------------------------------------------------------------------

DEBUGGER_BASE::DEBUGGER_BASE(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEBUGGER_BUS_BASE(parent_vm, parent_emu, identifier)
{
	m_file_path.Set(_T("debug.bin"));
	clear_all();
}

DEBUGGER_BASE::~DEBUGGER_BASE()
{
}

void DEBUGGER_BASE::clear_all()
{
	m_now_going = 0;
	m_now_debugging = 0;
	m_now_suspended = false;
	d_detected = NULL;
}

void DEBUGGER_BASE::start_debugging()
{
	m_now_going = 0;
	m_now_debugging = 1;
}

void DEBUGGER_BASE::stop_debugging()
{
	clear_all();
}

void DEBUGGER_BASE::go_suspend()
{
	if (m_now_debugging != 0) {
		m_now_suspended = true;
		m_now_debugging |= 2;
	}
}

void DEBUGGER_BASE::go_suspend_at_first()
{
	if (m_now_debugging == 1) {
		m_now_suspended = true;
		m_now_debugging |= 2;
	}
}

bool DEBUGGER_BASE::now_suspend() const
{
	return (m_now_debugging != 0 && m_now_suspended);
}

void DEBUGGER_BASE::clear_suspend()
{
	m_now_suspended = false;
}

#endif /* USE_DEBUGGER */
