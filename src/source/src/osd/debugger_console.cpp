/** @file debugger_console.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.03.01 -

	@brief [ debugger console ]

	@note Original auther is Takeda.Toshiya on 2006.08.18 -
*/

#include "debugger_console.h"

#ifdef USE_DEBUGGER

#include "../common.h"
#include "../depend.h"
#include "../debugger_bpoint.h"
#include "../debugger_symbol.h"
#include "../debugger_socket.h"
#include "../csurface.h"
#include "../gui/gui.h"
#include "../keycode.h"
#include "../config.h"
#include "../fileio.h"
#include "../utility.h"
#if !(defined(_WIN32) && defined(_MSC_VER))
#include <dirent.h>
#endif

#ifdef Status
#undef Status
#endif

#define DEBUGGER_PROMPT _T(" - ")

#if defined(_BML3MK5)
#define ADDR_FORMAT "%04X"
#define ADDR_STRING "%-4s"
#define PHYS_ADDR_FORMAT "%04X"
#define PHYS_ADDR_STRING "%-4s"
#define DATA_FORMAT "%02X"
#define IOADDR_FORMAT "%02X"
#elif defined(_MBS1)
#define ADDR_FORMAT "%04X"
#define ADDR_STRING "%-4s"
#define PHYS_ADDR_FORMAT "%05X"
#define PHYS_ADDR_STRING "%-5s"
#define DATA_FORMAT "%02X"
#define IOADDR_FORMAT "%02X"
#else
#define ADDR_FORMAT "%06X"
#define ADDR_STRING "%-6s"
#define PHYS_ADDR_FORMAT "%06X"
#define PHYS_ADDR_STRING "%-6s"
#define DATA_FORMAT "%08X"
#define IOADDR_FORMAT "%08X"
#endif
#define DATA_BYTE_FORMAT "%02X"

enum enDCommandsIdx {
	ID_DUMP_MEMORY = 1,
	ID_EDIT_MEMORY_BYTE,
	ID_EDIT_MEMORY_WORD,
	ID_EDIT_MEMORY_DWORD,
	ID_EDIT_MEMORY_ASCII,
#ifdef USE_EMU_INHERENT_SPEC
	ID_DUMP_PHYSICAL_MEMORY,
	ID_EDIT_PHYSICAL_MEMORY_BYTE,
	ID_EDIT_PHYSICAL_MEMORY_WORD,
	ID_EDIT_PHYSICAL_MEMORY_DWORD,
	ID_EDIT_PHYSICAL_MEMORY_ASCII,
#endif
#ifdef USE_IOPORT_ACCESS
	ID_INPUT_IO_BYTE,
	ID_INPUT_IO_WORD,
	ID_INPUT_IO_DWORD,
	ID_OUTPUT_IO_BYTE,
	ID_OUTPUT_IO_WORD,
	ID_OUTPUT_IO_DWORD,
#endif
	ID_ACCESS_REGISTER,
	ID_ACCESS_DEVICE,
	ID_SHOW_REGISTER,
	ID_HIDE_REGISTER,
	ID_SHOW_MEMORY_MAP,
#ifdef _MBS1
	ID_SHOW_ADDRESS_MAP,
	ID_EDIT_ADDRESS_MAP,
	ID_SHOW_MEMORY_SPACE_MAP,
	ID_EDIT_MEMORY_SPACE_MAP,
#endif
	ID_SEARCH_BYTE,
	ID_SEARCH_WORD,
	ID_SEARCH_DWORD,
	ID_SEARCH_ASCII,
#ifdef USE_EMU_INHERENT_SPEC
	ID_SEARCH_PHYSICAL_BYTE,
	ID_SEARCH_PHYSICAL_WORD,
	ID_SEARCH_PHYSICAL_DWORD,
	ID_SEARCH_PHYSICAL_ASCII,
#endif
	ID_UNASSEMBLE,
#ifdef USE_EMU_INHERENT_SPEC
	ID_UNASSEMBLE_PHYSICAL,
#endif
	ID_CALC_HEXA,
	ID_SET_FILE_NAME,
	ID_LOAD_BINARY_FILE,
	ID_SAVE_BINARY_FILE,
#ifdef USE_EMU_INHERENT_SPEC
	ID_LOAD_BINARY_FILE_PHYSICAL,
	ID_SAVE_BINARY_FILE_PHYSICAL,
#endif
#ifdef USE_EMU_INHERENT_SPEC
	ID_SAVE_IMAGE_FILE,
#endif
	ID_SET_BREAK_POINT,
	ID_CLEAR_BREAK_POINT,
	ID_DISABLE_BREAK_POINT,
	ID_ENABLE_BREAK_POINT,
	ID_LIST_BREAK_POINT,
	ID_SET_BREAK_POINT_PHYSICAL,
	ID_CLEAR_BREAK_POINT_PHYSICAL,
	ID_DISABLE_BREAK_POINT_PHYSICAL,
	ID_ENABLE_BREAK_POINT_PHYSICAL,
	ID_SET_BREAK_POINT_FOR_MEMRD,
	ID_CLEAR_BREAK_POINT_FOR_MEMRD,
	ID_DISABLE_BREAK_POINT_FOR_MEMRD,
	ID_ENABLE_BREAK_POINT_FOR_MEMRD,
//	ID_LIST_BREAK_POINT_FOR_MEMRD,
	ID_SET_BREAK_POINT_FOR_MEMWR,
	ID_CLEAR_BREAK_POINT_FOR_MEMWR,
	ID_DISABLE_BREAK_POINT_FOR_MEMWR,
	ID_ENABLE_BREAK_POINT_FOR_MEMWR,
//	ID_LIST_BREAK_POINT_FOR_MEMWR,
	ID_SET_BREAK_POINT_FOR_MEMRD_PHYSICAL,
	ID_CLEAR_BREAK_POINT_FOR_MEMRD_PHYSICAL,
	ID_DISABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL,
	ID_ENABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL,
	ID_SET_BREAK_POINT_FOR_MEMWR_PHYSICAL,
	ID_CLEAR_BREAK_POINT_FOR_MEMWR_PHYSICAL,
	ID_DISABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL,
	ID_ENABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL,
#ifdef USE_IOPORT_ACCESS
	ID_SET_BREAK_POINT_FOR_IORD,
	ID_CLEAR_BREAK_POINT_FOR_IORD,
	ID_DISABLE_BREAK_POINT_FOR_IORD,
	ID_ENABLE_BREAK_POINT_FOR_IORD,
//	ID_LIST_BREAK_POINT_FOR_IORD,
	ID_SET_BREAK_POINT_FOR_IOWR,
	ID_CLEAR_BREAK_POINT_FOR_IOWR,
	ID_DISABLE_BREAK_POINT_FOR_IOWR,
	ID_ENABLE_BREAK_POINT_FOR_IOWR,
//	ID_LIST_BREAK_POINT_FOR_IOWR,
#endif
	ID_SET_BREAK_POINT_FOR_INTR,
	ID_CLEAR_BREAK_POINT_FOR_INTR,
	ID_DISABLE_BREAK_POINT_FOR_INTR,
	ID_ENABLE_BREAK_POINT_FOR_INTR,
//	ID_LIST_BREAK_POINT_FOR_INTR,
	ID_SET_BREAK_POINT_FOR_EXCEPT,
	ID_CLEAR_BREAK_POINT_FOR_EXCEPT,
	ID_DISABLE_BREAK_POINT_FOR_EXCEPT,
	ID_ENABLE_BREAK_POINT_FOR_EXCEPT,
	ID_SET_TRACE_POINT,
	ID_CLEAR_TRACE_POINT,
	ID_DISABLE_TRACE_POINT,
	ID_ENABLE_TRACE_POINT,
	ID_LIST_TRACE_POINT,
	ID_SET_TRACE_POINT_PHYSICAL,
	ID_CLEAR_TRACE_POINT_PHYSICAL,
	ID_DISABLE_TRACE_POINT_PHYSICAL,
	ID_ENABLE_TRACE_POINT_PHYSICAL,
	ID_SET_TRACE_POINT_FOR_MEMRD,
	ID_CLEAR_TRACE_POINT_FOR_MEMRD,
	ID_DISABLE_TRACE_POINT_FOR_MEMRD,
	ID_ENABLE_TRACE_POINT_FOR_MEMRD,
//	ID_LIST_TRACE_POINT_FOR_MEMRD,
	ID_SET_TRACE_POINT_FOR_MEMWR,
	ID_CLEAR_TRACE_POINT_FOR_MEMWR,
	ID_DISABLE_TRACE_POINT_FOR_MEMWR,
	ID_ENABLE_TRACE_POINT_FOR_MEMWR,
//	ID_LIST_TRACE_POINT_FOR_MEMWR,
	ID_SET_TRACE_POINT_FOR_MEMRD_PHYSICAL,
	ID_CLEAR_TRACE_POINT_FOR_MEMRD_PHYSICAL,
	ID_DISABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL,
	ID_ENABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL,
	ID_SET_TRACE_POINT_FOR_MEMWR_PHYSICAL,
	ID_CLEAR_TRACE_POINT_FOR_MEMWR_PHYSICAL,
	ID_DISABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL,
	ID_ENABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL,
#ifdef USE_IOPORT_ACCESS
	ID_SET_TRACE_POINT_FOR_IORD,
	ID_CLEAR_TRACE_POINT_FOR_IORD,
	ID_DISABLE_TRACE_POINT_FOR_IORD,
	ID_ENABLE_TRACE_POINT_FOR_IORD,
//	ID_LIST_TRACE_POINT_FOR_IORD,
	ID_SET_TRACE_POINT_FOR_IOWR,
	ID_CLEAR_TRACE_POINT_FOR_IOWR,
	ID_DISABLE_TRACE_POINT_FOR_IOWR,
	ID_ENABLE_TRACE_POINT_FOR_IOWR,
//	ID_LIST_TRACE_POINT_FOR_IOWR,
#endif
	ID_SET_TRACE_POINT_FOR_INTR,
	ID_CLEAR_TRACE_POINT_FOR_INTR,
	ID_DISABLE_TRACE_POINT_FOR_INTR,
	ID_ENABLE_TRACE_POINT_FOR_INTR,
//	ID_LIST_TRACE_POINT_FOR_INTR,
	ID_SET_TRACE_POINT_FOR_EXCEPT,
	ID_CLEAR_TRACE_POINT_FOR_EXCEPT,
	ID_DISABLE_TRACE_POINT_FOR_EXCEPT,
	ID_ENABLE_TRACE_POINT_FOR_EXCEPT,
	ID_SET_SYMBOL,
	ID_CLEAR_SYMBOL,
	ID_LOAD_SYMBOL,
	ID_EXECUTE,
	ID_TRACE,
	ID_TRACE_BACK,
	ID_SHOW_CLOCK,
	ID_OUTPUT_LOG_FILE,
	ID_SEND_TO_EMULATOR,
#ifdef USE_EMU_INHERENT_SPEC
	ID_BASIC,
#endif
	ID_SWITCH_CPU,
	ID_PWD,
	ID_CHD,
	ID_DIR,
	ID_USAGE,
	ID_QUIT,
	ID_END
};

/// command
struct st_commands_map {
	const _TCHAR   *cmd;
	enDCommandsIdx  idx;
};
static struct st_commands_map commands_map[] = {
	{ _T("D"),	ID_DUMP_MEMORY },
	{ _T("E"),	ID_EDIT_MEMORY_BYTE },
	{ _T("EB"),	ID_EDIT_MEMORY_BYTE },
	{ _T("EW"),	ID_EDIT_MEMORY_WORD },
	{ _T("ED"),	ID_EDIT_MEMORY_DWORD },
	{ _T("EA"),	ID_EDIT_MEMORY_ASCII },
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("DP"),	ID_DUMP_PHYSICAL_MEMORY },
	{ _T("EP"),	ID_EDIT_PHYSICAL_MEMORY_BYTE },
	{ _T("EPB"),ID_EDIT_PHYSICAL_MEMORY_BYTE },
	{ _T("EPW"),ID_EDIT_PHYSICAL_MEMORY_WORD },
	{ _T("EPD"),ID_EDIT_PHYSICAL_MEMORY_DWORD },
	{ _T("EPA"),ID_EDIT_PHYSICAL_MEMORY_ASCII },
#endif
#ifdef USE_IOPORT_ACCESS
	{ _T("I"),	ID_INPUT_IO_BYTE },
	{ _T("IB"),	ID_INPUT_IO_BYTE },
	{ _T("IW"),	ID_INPUT_IO_WORD },
	{ _T("ID"),	ID_INPUT_IO_DWORD },
	{ _T("O"),	ID_OUTPUT_IO_BYTE },
	{ _T("OB"),	ID_OUTPUT_IO_BYTE },
	{ _T("OW"),	ID_OUTPUT_IO_WORD },
	{ _T("OD"),	ID_OUTPUT_IO_DWORD },
#endif
	{ _T("R"),	ID_ACCESS_REGISTER },
	{ _T("RD"),	ID_ACCESS_DEVICE },
	{ _T("RS"),	ID_SHOW_REGISTER },
	{ _T("RH"),	ID_HIDE_REGISTER },
	{ _T("M"),	ID_SHOW_MEMORY_MAP },
#ifdef _MBS1
	{ _T("X"),	ID_SHOW_ADDRESS_MAP },
	{ _T("XE"),	ID_EDIT_ADDRESS_MAP },
	{ _T("MAP"),ID_SHOW_MEMORY_SPACE_MAP },
	{ _T("MAPE"),ID_EDIT_MEMORY_SPACE_MAP },
#endif
	{ _T("S"),	ID_SEARCH_BYTE },
	{ _T("SB"),	ID_SEARCH_BYTE },
	{ _T("SW"),	ID_SEARCH_WORD },
	{ _T("SD"),	ID_SEARCH_DWORD },
	{ _T("SA"),	ID_SEARCH_ASCII },
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("SP"),	ID_SEARCH_PHYSICAL_BYTE },
	{ _T("SPB"),ID_SEARCH_PHYSICAL_BYTE },
	{ _T("SPW"),ID_SEARCH_PHYSICAL_WORD },
	{ _T("SPD"),ID_SEARCH_PHYSICAL_DWORD },
	{ _T("SPA"),ID_SEARCH_PHYSICAL_ASCII },
#endif
	{ _T("U"),	ID_UNASSEMBLE },
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("UP"), ID_UNASSEMBLE_PHYSICAL },
#endif
	{ _T("H"),	ID_CALC_HEXA },
	{ _T("DN"),	ID_SET_FILE_NAME },
	{ _T("DL"),	ID_LOAD_BINARY_FILE },
	{ _T("DS"),	ID_SAVE_BINARY_FILE },
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("DPL"),ID_LOAD_BINARY_FILE_PHYSICAL },
	{ _T("DPS"),ID_SAVE_BINARY_FILE_PHYSICAL },
#endif
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("DIS"),ID_SAVE_IMAGE_FILE },
#endif
	{ _T("BP"),	ID_SET_BREAK_POINT },
	{ _T("BC"),	ID_CLEAR_BREAK_POINT },
	{ _T("BD"),	ID_DISABLE_BREAK_POINT },
	{ _T("BE"),	ID_ENABLE_BREAK_POINT },
	{ _T("BL"),	ID_LIST_BREAK_POINT },
#ifdef USE_BREAKPOINT_PHYSICAL
	{ _T("BPP"),ID_SET_BREAK_POINT_PHYSICAL },
	{ _T("BPC"),ID_CLEAR_BREAK_POINT_PHYSICAL },
	{ _T("BPD"),ID_DISABLE_BREAK_POINT_PHYSICAL },
	{ _T("BPE"),ID_ENABLE_BREAK_POINT_PHYSICAL },
#endif
	{ _T("RBP"),ID_SET_BREAK_POINT_FOR_MEMRD },
	{ _T("WBP"),ID_SET_BREAK_POINT_FOR_MEMWR },
	{ _T("RBC"),ID_CLEAR_BREAK_POINT_FOR_MEMRD },
	{ _T("WBC"),ID_CLEAR_BREAK_POINT_FOR_MEMWR },
	{ _T("RBD"),ID_DISABLE_BREAK_POINT_FOR_MEMRD },
	{ _T("WBD"),ID_DISABLE_BREAK_POINT_FOR_MEMWR },
	{ _T("RBE"),ID_ENABLE_BREAK_POINT_FOR_MEMRD },
	{ _T("WBE"),ID_ENABLE_BREAK_POINT_FOR_MEMWR },
//	{ _T("RBL"),ID_LIST_BREAK_POINT_FOR_MEMRD },
//	{ _T("WBL"),ID_LIST_BREAK_POINT_FOR_MEMWR },
#ifdef USE_BREAKPOINT_PHYSICAL
	{ _T("RBPP"),ID_SET_BREAK_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WBPP"),ID_SET_BREAK_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RBPC"),ID_CLEAR_BREAK_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WBPC"),ID_CLEAR_BREAK_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RBPD"),ID_DISABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WBPD"),ID_DISABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RBPE"),ID_ENABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WBPE"),ID_ENABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL },
#endif
#ifdef USE_IOPORT_ACCESS
	{ _T("IBP"),ID_SET_BREAK_POINT_FOR_IORD },
	{ _T("OBP"),ID_SET_BREAK_POINT_FOR_IOWR },
	{ _T("IBC"),ID_CLEAR_BREAK_POINT_FOR_IORD },
	{ _T("OBC"),ID_CLEAR_BREAK_POINT_FOR_IOWR },
	{ _T("IBD"),ID_DISABLE_BREAK_POINT_FOR_IORD },
	{ _T("OBD"),ID_DISABLE_BREAK_POINT_FOR_IOWR },
	{ _T("IBE"),ID_ENABLE_BREAK_POINT_FOR_IORD },
	{ _T("OBE"),ID_ENABLE_BREAK_POINT_FOR_IOWR },
//	{ _T("IBL"),ID_LIST_BREAK_POINT_FOR_IORD },
//	{ _T("OBL"),ID_LIST_BREAK_POINT_FOR_IOWR },
#endif
	{ _T("NBP"),ID_SET_BREAK_POINT_FOR_INTR },
	{ _T("NBC"),ID_CLEAR_BREAK_POINT_FOR_INTR },
	{ _T("NBD"),ID_DISABLE_BREAK_POINT_FOR_INTR },
	{ _T("NBE"),ID_ENABLE_BREAK_POINT_FOR_INTR },
//	{ _T("NBL"),ID_LIST_BREAK_POINT_FOR_INTR },
	{ _T("EBP"),ID_SET_BREAK_POINT_FOR_EXCEPT },
	{ _T("EBC"),ID_CLEAR_BREAK_POINT_FOR_EXCEPT },
	{ _T("EBD"),ID_DISABLE_BREAK_POINT_FOR_EXCEPT },
	{ _T("EBE"),ID_ENABLE_BREAK_POINT_FOR_EXCEPT },
	{ _T("TP"),	ID_SET_TRACE_POINT },
	{ _T("TC"),	ID_CLEAR_TRACE_POINT },
	{ _T("TD"),	ID_DISABLE_TRACE_POINT },
	{ _T("TE"),	ID_ENABLE_TRACE_POINT },
	{ _T("TL"),	ID_LIST_TRACE_POINT },
#ifdef USE_BREAKPOINT_PHYSICAL
	{ _T("TPP"),ID_SET_TRACE_POINT_PHYSICAL },
	{ _T("TPC"),ID_CLEAR_TRACE_POINT_PHYSICAL },
	{ _T("TPD"),ID_DISABLE_TRACE_POINT_PHYSICAL },
	{ _T("TPE"),ID_ENABLE_TRACE_POINT_PHYSICAL },
#endif
	{ _T("RTP"),ID_SET_TRACE_POINT_FOR_MEMRD },
	{ _T("WTP"),ID_SET_TRACE_POINT_FOR_MEMWR },
	{ _T("RTC"),ID_CLEAR_TRACE_POINT_FOR_MEMRD },
	{ _T("WTC"),ID_CLEAR_TRACE_POINT_FOR_MEMWR },
	{ _T("RTD"),ID_DISABLE_TRACE_POINT_FOR_MEMRD },
	{ _T("WTD"),ID_DISABLE_TRACE_POINT_FOR_MEMWR },
	{ _T("RTE"),ID_ENABLE_TRACE_POINT_FOR_MEMRD },
	{ _T("WTE"),ID_ENABLE_TRACE_POINT_FOR_MEMWR },
//	{ _T("RTL"),ID_LIST_TRACE_POINT_FOR_MEMRD },
//	{ _T("WTL"),ID_LIST_TRACE_POINT_FOR_MEMWR },
#ifdef USE_BREAKPOINT_PHYSICAL
	{ _T("RTPP"),ID_SET_TRACE_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WTPP"),ID_SET_TRACE_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RTPC"),ID_CLEAR_TRACE_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WTPC"),ID_CLEAR_TRACE_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RTPD"),ID_DISABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WTPD"),ID_DISABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL },
	{ _T("RTPE"),ID_ENABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL },
	{ _T("WTPE"),ID_ENABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL },
#endif
#ifdef USE_IOPORT_ACCESS
	{ _T("ITP"),ID_SET_TRACE_POINT_FOR_IORD },
	{ _T("OTP"),ID_SET_TRACE_POINT_FOR_IOWR },
	{ _T("ITC"),ID_CLEAR_TRACE_POINT_FOR_IORD },
	{ _T("OTC"),ID_CLEAR_TRACE_POINT_FOR_IOWR },
	{ _T("ITD"),ID_DISABLE_TRACE_POINT_FOR_IORD },
	{ _T("OTD"),ID_DISABLE_TRACE_POINT_FOR_IOWR },
	{ _T("ITE"),ID_ENABLE_TRACE_POINT_FOR_IORD },
	{ _T("OTE"),ID_ENABLE_TRACE_POINT_FOR_IOWR },
//	{ _T("ITL"),ID_LIST_TRACE_POINT_FOR_IORD },
//	{ _T("OTL"),ID_LIST_TRACE_POINT_FOR_IOWR },
#endif
	{ _T("NTP"),ID_SET_TRACE_POINT_FOR_INTR },
	{ _T("NTC"),ID_CLEAR_TRACE_POINT_FOR_INTR },
	{ _T("NTD"),ID_DISABLE_TRACE_POINT_FOR_INTR },
	{ _T("NTE"),ID_ENABLE_TRACE_POINT_FOR_INTR },
//	{ _T("NTL"),ID_LIST_TRACE_POINT_FOR_INTR },
	{ _T("ETP"),ID_SET_TRACE_POINT_FOR_EXCEPT },
	{ _T("ETC"),ID_CLEAR_TRACE_POINT_FOR_EXCEPT },
	{ _T("ETD"),ID_DISABLE_TRACE_POINT_FOR_EXCEPT },
	{ _T("ETE"),ID_ENABLE_TRACE_POINT_FOR_EXCEPT },
	{ _T("SL"),	ID_SET_SYMBOL },
	{ _T("SC"),	ID_CLEAR_SYMBOL },
	{ _T("SLL"),ID_LOAD_SYMBOL },
	{ _T("G"),	ID_EXECUTE },
	{ _T("T"),	ID_TRACE },
	{ _T("TB"),	ID_TRACE_BACK },
	{ _T("CL"),	ID_SHOW_CLOCK },
	{ _T(">"),	ID_OUTPUT_LOG_FILE },
	{ _T("!"),	ID_SEND_TO_EMULATOR },
#ifdef USE_EMU_INHERENT_SPEC
	{ _T("BAS"),ID_BASIC },
#endif
	{ _T("CPU"),ID_SWITCH_CPU },
	{ _T("PWD"),ID_PWD },
	{ _T("CD"), ID_CHD },
	{ _T("DIR"),ID_DIR },
	{ _T("LS"), ID_DIR },
	{ _T("?"),	ID_USAGE },
	{ _T("Q"),	ID_QUIT },
	{ NULL, ID_END }
};

#ifdef USE_CONSOLE_WINDOW
/// VK code to keycode on console window
struct st_vkkey2keycode {
	WORD	vk_code;
	uint8_t	code;
};
/// hash table of VK code to keycode on console window
static const st_vkkey2keycode vkkey2keycode[] = {
	{ VK_UP,	KEYCODE_UP },
	{ VK_DOWN,	KEYCODE_DOWN },
	{ VK_RIGHT,	KEYCODE_RIGHT },
	{ VK_LEFT,	KEYCODE_LEFT },
	{ 0, 0 }
};
#endif

#ifdef USE_TELNET_SERVER
/// escape sequence to keycode on telnet
struct st_esc2keycode {
	const _TCHAR *seq;
	uint8_t	code;
};
/// hash table of escape sequence to keycode on telnet
static const st_esc2keycode esc2keycode[] = {
	{ _T("[A"),	KEYCODE_UP },
	{ _T("[B"),	KEYCODE_DOWN },
	{ _T("[C"),	KEYCODE_RIGHT },
	{ _T("[D"),	KEYCODE_LEFT },
	{ NULL, 0 }
};
#endif

#ifdef USE_CONSOLE_WINDOW
static BOOL WINAPI ctrl_c_handler(DWORD type)
{
	return TRUE;
}
#endif

// ----------------------------------------------------------------------------

///
DebuggerStorage::DebuggerStorage()
{
//	last_history_ptr = 0;
	history_ptr = 0;

	now_running = false;
}

DebuggerStorage::~DebuggerStorage()
{
}
int DebuggerStorage::FindHistory(const _TCHAR *command) const
{
	int match = -1;
	for(int i=0; i<command_history.Count(); i++) {
		const CTchar *itm = command_history[i];
		if (itm->MatchString(command)) {
			match = i;
			break;
		}
	}
	return match;
}
void DebuggerStorage::AddHistory(const _TCHAR *command)
{
	int match = FindHistory(command);
	if (match >= 0) {
		command_history.Delete(match);
	}
	if (command_history.Count() >= DC_MAX_COMMAND_HISTORY) {
		command_history.Delete(0);
	}
	command_history.Add(new CTchar(command));
	history_ptr = command_history.Count();
//	last_history_ptr++;
//	if (last_history_ptr >= DC_MAX_COMMAND_HISTORY) {
//		last_history_ptr = 0;
//	}
}
bool DebuggerStorage::PrevHistory()
{
	if (history_ptr <= 0) return false;
	history_ptr--;
	return true;
}
bool DebuggerStorage::NextHistory()
{
	if (history_ptr >= (command_history.Count() - 1)) return false;
	history_ptr++;
	return true;
}
#if 0
bool DebuggerStorage::IsHistoryEnabled(int index) const
{
	index = last_history_ptr - index;
	index = (index + DC_MAX_COMMAND_HISTORY + DC_MAX_COMMAND_HISTORY) % DC_MAX_COMMAND_HISTORY;
	return (command_history[index].Length() > 0);
}
int DebuggerStorage::GetHistory(int index, _TCHAR *command, size_t size) const
{
	index = last_history_ptr - index;
	index = (index + DC_MAX_COMMAND_HISTORY + DC_MAX_COMMAND_HISTORY) % DC_MAX_COMMAND_HISTORY;
	UTILITY::tcscpy(command, size, command_history[index].Get());
	return command_history[index].Length();
}
#endif
int DebuggerStorage::GetHistory(int index, _TCHAR *command, size_t size) const
{
	UTILITY::tcscpy(command, size, command_history[index]->Get());
	return command_history[index]->Length();
}
int DebuggerStorage::GetCurrentHistory(_TCHAR *command, size_t size) const
{
	return GetHistory(history_ptr, command, size);
}
void DebuggerStorage::SetCurrentCPUName(const char *name)
{
	current_cpu_name.SetN(name);
}

// ----------------------------------------------------------------------------

ConsoleKeyCode::ConsoleKeyCode()
{
	Clear();
}

ConsoleKeyCode::~ConsoleKeyCode()
{
}

void ConsoleKeyCode::Clear()
{
	chr = 0;
	key = 0;
}

void ConsoleKeyCode::Set(_TCHAR chr_, uint8_t key_)
{
	chr = chr_;
	key = key_;
}

void ConsoleKeyCode::Get(_TCHAR &chr_, uint8_t &key_) const
{
	chr_ = chr;
	key_ = key;
}

//

ConsoleRingBuffer::ConsoleRingBuffer()
{
	Clear();
}

ConsoleRingBuffer::~ConsoleRingBuffer()
{
}

void ConsoleRingBuffer::Clear()
{
	w_pos = 0;
	r_pos = 0;
	prev_chr = _T('\0');
}

void ConsoleRingBuffer::Push(_TCHAR chr_, uint8_t key_)
{
	// ignore LF when pressed CR+LF.
	if (chr_ == 0x0a && prev_chr == 0x0d) return;
	keys[w_pos].Set(chr_, key_);
	prev_chr = chr_;
	w_pos = ((w_pos + 1) % DBG_KEY_BUFFER_SIZE);
}

bool ConsoleRingBuffer::Pop(_TCHAR &chr, uint8_t &key)
{
	if (r_pos == w_pos) return false;
	keys[r_pos].Get(chr, key);
	r_pos = ((r_pos + 1) % DBG_KEY_BUFFER_SIZE);
	return true;
}

// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer()
{
	Clear();
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Clear()
{
	memset(command, 0, sizeof(command));
	memset(prev_command, 0, sizeof(prev_command));
	commandptr = commandright = 0;
}

void CommandBuffer::ClearCurr()
{
	memset(command, 0, sizeof(command));
	commandptr = commandright = 0;
}

void CommandBuffer::Add(_TCHAR chr_)
{
	command[commandptr++] = chr_;
	commandright++;
}

void CommandBuffer::Insert(_TCHAR chr_)
{
	RShift();
	Add(chr_);
}

_TCHAR *CommandBuffer::Ptr()
{
	return command;
}

const _TCHAR *CommandBuffer::Ptr() const
{
	return command;
}

const _TCHAR *CommandBuffer::CurrPtr() const
{
	return &command[commandptr-1];
}

const _TCHAR *CommandBuffer::NextPtr() const
{
	return &command[commandptr];
}

int CommandBuffer::CurrPos() const
{
	return commandptr;
}

int CommandBuffer::LastLen() const
{
	return commandright - commandptr + 1;
}

int CommandBuffer::Len() const
{
	return commandright;
}

void CommandBuffer::SetLen(int val)
{
	commandptr = commandright = val;
}

int CommandBuffer::Size() const
{
	return (int)(sizeof(command) / sizeof(command[0]));
}

_TCHAR CommandBuffer::Left()
{
	if (commandptr == 0) return 0;
	commandptr--;
	return KEYCODE_BACKSPACE;
}

_TCHAR CommandBuffer::Right()
{
	if (commandptr >= commandright) return 0;
	return command[commandptr++];
}

void CommandBuffer::Tail()
{
	commandptr = commandright;
}

void CommandBuffer::BackSpace()
{
	if (commandptr == 0) return;

	commandptr--;
	commandright--;
	// lshift
	LShift();
	command[commandright] = _T(' ');
}

void CommandBuffer::RShift()
{
	for (int i = commandright; i > commandptr; i--) {
		command[i] = command[i-1];
	}
}

void CommandBuffer::LShift()
{
	for (int i = commandptr; i < commandright; i++) {
		command[i] = command[i+1];
	}
}

void CommandBuffer::CopyFromPrev()
{
	memcpy(command, prev_command, sizeof(command) / sizeof(command[0]));
}

void CommandBuffer::Decide()
{
	command[commandptr] = _T('\0');
	commandptr = Trim(command, commandptr);
	memcpy(prev_command, command, sizeof(command) / sizeof(command[0]));
}

void CommandBuffer::ShrinkPrev(int pos)
{
	prev_command[pos] = _T('\0');
}

int CommandBuffer::Trim(_TCHAR *str, int len)
{
	if (len > 0) {
		UTILITY::rtrim(str);
		len = (int)_tcslen(str);
	}
	return len;
}

bool CommandBuffer::IsFull() const
{
	return (commandptr >= DC_MAX_COMMAND_LEN);
}

bool CommandBuffer::IsEmpty() const
{
	return (commandptr == 0);
}

bool CommandBuffer::IsTail() const
{
	return (commandptr >= commandright);
}

/// run latest command again?
bool CommandBuffer::IsRepeat() const
{
	return (commandptr == 0 && prev_command[0] != _T('\0'));
}

// ----------------------------------------------------------------------------

///
void DebuggerConsole::devs_st::Clear()
{
	index = -1;
	cpu = NULL;
	mem = NULL;
	debugger = NULL;
	prog_addr_mask = 0;
	data_addr_mask = 0;
	phys_type = 0;
	dump_addr = 0;
	dump_phys_addr = 0;
	dasm_addr = 0;
	dasm_phys_addr = 0;
	show_regs = true;
	next = NULL;
}

// ----------------------------------------------------------------------------

///
DebuggerConsole::DebuggerConsole(debugger_thread_t *p)
{
	dp = p;

	storage = dp->emu->get_debugger_storage();
#ifdef USE_TELNET_SERVER
	telnet = dp->emu->get_debugger_socket();
#endif

	num_of_cpus = dp->num_of_cpus;

	for(int i=0; i < DC_MAX_CPUS; i++) {
		devs[i].Clear();
	}

	int cpu_index = dp->cpu_index;

	int st = cpu_index >= 0 ? cpu_index : 0;
	int ed = cpu_index >= 0 ? cpu_index : num_of_cpus - 1;

	struct devs_st *prev_dev = NULL;
	for(int i = st; i <= ed; i++) {
		struct devs_st *dev = &devs[i];
		dev->index = i;
		dev->cpu = dp->vm->get_cpu(i);
		dev->mem = dp->vm->get_memory(i);
		dev->debugger = (DEBUGGER *)dev->cpu->get_debugger();

		if (dev->cpu) dev->cpu->set_debugger_console(this);
		if (dev->mem) dev->mem->set_debugger_console(this);
		dev->debugger->set_debugger_console(this);

		dev->debugger->start_debugging();

		if (prev_dev) prev_dev->next = dev;
		prev_dev = dev;
	}

	// wait until vm process suspend
	while(1) {
		bool sus = true;
		for(int i = st; i <= ed; i++) {
			sus = (sus && (!devs[i].cpu->enable() || devs[i].debugger->now_suspend() || NowDisable()));
		}
		if (sus) break;

		CDelay(10);
	}
	current_color = White;

	current_dev = &devs[st];
	storage->SetCurrentCPUName(current_dev->cpu->get_class_name());

	logfile = NULL;
	logfilename[0] = _T('\0');

	for(int i = st; i <= ed; i++) {
		struct devs_st *dev = &devs[i];
		dev->prog_addr_mask = dev->cpu->debug_prog_addr_mask();
		dev->data_addr_mask = dev->cpu->debug_data_addr_mask();
		dev->phys_type = 0;
		dev->dump_addr = 0;
		dev->dump_phys_addr = 0;
		dev->dasm_addr = dev->cpu->get_next_pc();
		dev->dasm_phys_addr = 0;
	}

	UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Debugger - %s"), _T(DEVICE_NAME));
	cp932 = false;

#ifdef USE_CONSOLE_WINDOW
	AllocConsole();
	SetConsoleTitle(buffer);
	SetConsoleCtrlHandler(ctrl_c_handler, TRUE);

	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	cp932 = (GetConsoleCP() == 932);

	COORD coord;
	coord.X = 80;
	coord.Y = 4000;

	SetConsoleScreenBufferSize(hStdOut, coord);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
#endif

	echo_bak = true;

//	memset(command, 0, sizeof(command));
//	memset(prev_command, 0, sizeof(prev_command));
//	commandptr = commandright = 0;

//	history_ptr = 0;

	paramnum = 0;

	memset(key_stat_tmp, 0, KEY_STATUS_SIZE);
}

DebuggerConsole::~DebuggerConsole()
{
	// release logfile
	if(logfile != NULL) {
		if(logfile->IsOpened()) {
			logfile->Fclose();
		}
		delete logfile;
		logfile = NULL;
	}

#ifdef USE_CONSOLE_WINDOW
	// release console
	SetConsoleCtrlHandler(ctrl_c_handler, FALSE);
	FreeConsole();
#endif

	for(int i=0; i < DC_MAX_CPUS; i++) {
		struct devs_st *dev = &devs[i];
		if (dev->cpu) dev->cpu->set_debugger_console(NULL);
		if (dev->mem) dev->mem->set_debugger_console(NULL);
		if (dev->debugger) dev->debugger->set_debugger_console(NULL);
	}
}

#ifdef USE_CONSOLE_WINDOW
const WORD cw_color_map[DebuggerConsole::EndColor] = {
	0,
	FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_INTENSITY,
	FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};
#endif
#ifdef USE_TELNET_SERVER
const _TCHAR *ts_color_map[DebuggerConsole::EndColor] = {
#if 0
	// dark
	_T("\x1b[30m"),	// black
	_T("\x1b[34m"),	// blue
	_T("\x1b[31m"),	// red
	_T("\x1b[35m"),	// magenta
	_T("\x1b[32m"),	// green
	_T("\x1b[36m"),	// cyan
	_T("\x1b[33m"),	// yellow
	_T("\x1b[37m")	// white
#else
	// light
	_T("\x1b[90m"),	// black
	_T("\x1b[94m"),	// blue
	_T("\x1b[91m"),	// red
	_T("\x1b[95m"),	// magenta
	_T("\x1b[92m"),	// green
	_T("\x1b[96m"),	// cyan
	_T("\x1b[93m"),	// yellow
	_T("\x1b[97m")	// white
#endif
};
#endif

void DebuggerConsole::SetTextColor(enTextColor color)
{
	if (color < EndColor) {
#ifdef USE_CONSOLE_WINDOW
		SetConsoleTextAttribute(hStdOut, cw_color_map[color]);
#endif
#ifdef USE_TELNET_SERVER
		telnet->write_data(ts_color_map[color], (int)_tcslen(ts_color_map[color]));
#endif
		current_color = color;
	}
}

void DebuggerConsole::Printf(enTextColor color, const _TCHAR *format, ...)
{
	va_list ap;
	SetTextColor(color);

	va_start(ap, format);
	Vprintf(format, ap);
	va_end(ap);
}

void DebuggerConsole::Printf(const _TCHAR *format, ...)
{
	va_list ap;

	va_start(ap, format);
	Vprintf(format, ap);
	va_end(ap);
}

void DebuggerConsole::Vprintf(const _TCHAR *format, va_list ap)
{
	_TCHAR nbuffer[DC_MAX_BUFFER_LEN];

	UTILITY::vstprintf(nbuffer, DC_MAX_BUFFER_LEN, format, ap);

	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(nbuffer, sizeof(_TCHAR), _tcslen(nbuffer));
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, nbuffer, (DWORD)_tcslen(nbuffer), &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) telnet->write_data(nbuffer, (int)_tcslen(nbuffer));
#endif
#ifdef USE_STANDARD_INOUT
	_fputts(nbuffer, stdout);
#endif
}

void DebuggerConsole::Print(enTextColor color, const _TCHAR *str, bool cr)
{
	SetTextColor(color);
	Print(str, cr);
}

void DebuggerConsole::Print(const _TCHAR *str, bool cr)
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(str, sizeof(_TCHAR), _tcslen(str));
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, str, (DWORD)_tcslen(str), &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) {
		telnet->write_data(str, (int)_tcslen(str));
	}
#endif
#ifdef USE_STANDARD_INOUT
	_fputts(str, stdout);
#endif
	if (cr) Cr();
}

void DebuggerConsole::PrintfError(const _TCHAR *format, ...)
{
	va_list ap;
	enTextColor prev_color = current_color;
	SetTextColor(Red);

	va_start(ap, format);
	Vprintf(format, ap);
	va_end(ap);

	SetTextColor(prev_color);
}

void DebuggerConsole::PrintError(const _TCHAR *str, bool cr)
{
	enTextColor prev_color = current_color;
	SetTextColor(Red);
	Print(str, cr);
	SetTextColor(prev_color);
}

void DebuggerConsole::Out(enTextColor color, bool cr)
{
	SetTextColor(color);
	Out(cr);
}

void DebuggerConsole::Out(bool cr)
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(buffer, sizeof(_TCHAR), _tcslen(buffer));
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, buffer, (DWORD)_tcslen(buffer), &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) {
		telnet->write_data(buffer, (int)_tcslen(buffer));
	}
#endif
#ifdef USE_STANDARD_INOUT
	_fputts(buffer, stdout);
#endif
	if (cr) Cr();
}

void DebuggerConsole::PutCh(_TCHAR c)
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(&c, sizeof(_TCHAR), 1);
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, &c, 1, &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) telnet->write_data(&c, 1);
#endif
#ifdef USE_STANDARD_INOUT
	_fputtc(c, stdout);
#endif
}

void DebuggerConsole::PutChs(_TCHAR c, size_t len)
{
	if (len <= 0 || DC_MAX_COMMAND_LEN <= len) return;

	_TCHAR buf[DC_MAX_COMMAND_LEN + 1];

	for(size_t i=0; i<len; i++) {
		buf[i] = c;
	}

	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(buf, sizeof(_TCHAR), len);
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, buf, (DWORD)len, &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) {
		telnet->write_data(buf, (int)len);
	}
#endif
#ifdef USE_STANDARD_INOUT
	fwrite(buf, sizeof(_TCHAR), len, stdout);
#endif
}

void DebuggerConsole::Puts(const _TCHAR *str, size_t len)
{
	if (len <= 0) return;

	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fwrite(str, sizeof(_TCHAR), len);
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, str, (DWORD)len, &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) {
		telnet->write_data(str, (int)len);
	}
#endif
#ifdef USE_STANDARD_INOUT
	fwrite(str, sizeof(_TCHAR), len, stdout);
#endif
}

void DebuggerConsole::Cr()
{
	if(logfile != NULL && logfile->IsOpened()) {
		logfile->Fputc('\n');
	}
#ifdef USE_CONSOLE_WINDOW
	DWORD dwWritten;
	WriteConsole(hStdOut, _T("\n"), (DWORD)_tcslen(_T("\n")), &dwWritten, NULL);
#endif
#ifdef USE_TELNET_SERVER
	if (echo_bak) {
		telnet->write_data(_T("\n"), (int)_tcslen(_T("\n")));
	}
#endif
#ifdef USE_STANDARD_INOUT
	_fputts(_T("\n"), stdout);
#endif
}

void DebuggerConsole::Flush()
{
#ifdef USE_CONSOLE_WINDOW
#endif
#ifdef USE_TELNET_SERVER
#endif
#ifdef USE_STANDARD_INOUT
	fflush(stdout);
#endif
}

_TCHAR *DebuggerConsole::GetBuffer(bool clear)
{
	if (clear) {
		memset(buffer, 0, sizeof(buffer));
	}
	return buffer;
}

const _TCHAR *DebuggerConsole::GetParam(int idx)
{
	if (0 <= idx && idx < paramnum) return params[idx];
	else return NULL;
}

// ---------------------------------------------------------------------------

bool DebuggerConsole::SetCommandLine(_TCHAR chr, uint8_t key, bool echo)
{
	bool enter_done = false;
	bool prev_echo_bak = echo_bak;
	echo_bak = echo;
	if(chr == 0x0d || chr == 0x0a) {
		command_buffer.Tail();
		if(command_buffer.IsRepeat()) {
			// run latest command again
			command_buffer.CopyFromPrev();
			Printf(_T("%s\n"), command_buffer.Ptr());
			enter_done = true;
		} else if(!command_buffer.IsEmpty()) {
			command_buffer.Decide();
			storage->AddHistory(command_buffer.Ptr());
//			history_ptr = 0;
			Cr();
			enter_done = true;
		}
	} else if(chr == 0x08 || chr == 0x7f) {
		// BS or DEL
		if(!command_buffer.IsEmpty()) {
			command_buffer.BackSpace();
			// move cursor position
			PutCh(KEYCODE_BACKSPACE);
			// display
			Puts(command_buffer.NextPtr(), command_buffer.LastLen());
			// move cursor position
			PutChs(KEYCODE_BACKSPACE, command_buffer.LastLen());
		}
	} else if(chr >= 0x20 && chr <= 0x7e && !command_buffer.IsFull() && !(chr == 0x20 && command_buffer.IsEmpty())) {
		// insert a key in current position
		command_buffer.Insert(chr);
		// display
		Puts(command_buffer.CurrPtr(), command_buffer.LastLen());
		// move cursor position
		PutChs(KEYCODE_BACKSPACE, command_buffer.LastLen() - 1);
	} else if(key) {
		if (key == KEYCODE_UP || key == KEYCODE_DOWN) {
//			int history_ptr_stored = history_ptr;
			bool history_enable = false;
			if (key == KEYCODE_DOWN) {
				history_enable = storage->NextHistory();
			} else if (key == KEYCODE_UP) {
				history_enable = storage->PrevHistory();
			}
			if(history_enable) {
				PutChs(KEYCODE_BACKSPACE, command_buffer.CurrPos());
				PutChs(_T(' '), command_buffer.CurrPos());
				PutChs(KEYCODE_BACKSPACE, command_buffer.CurrPos());
				command_buffer.SetLen(storage->GetCurrentHistory(command_buffer.Ptr(), command_buffer.Size()));
				Puts(command_buffer.Ptr(), command_buffer.Len());
			} else {
//				history_ptr = history_ptr_stored;
			}
		} else if (key == KEYCODE_LEFT) {
			if(!command_buffer.IsEmpty()) {
				PutCh(command_buffer.Left());
			}
		} else if (key == KEYCODE_RIGHT) {
			if(!command_buffer.IsTail()) {
				PutCh(command_buffer.Right());
			}
		}
	}
//	if (enter_done) {
//		command_buffer.Clear();
//	}
	echo_bak = prev_echo_bak;
	return enter_done;
}

bool DebuggerConsole::ReadInput()
{
	bool enter_done = false;
	_TCHAR  chr = 0;
	uint8_t key = 0;

#ifdef USE_CONSOLE_WINDOW
	DWORD dwRead;
	INPUT_RECORD ir[16];
	if(GetNumberOfConsoleInputEvents(hStdIn, &dwRead)) {
		if(dwRead > 0 && ReadConsoleInput(hStdIn, ir, 16, &dwRead)) {
			for(unsigned int i = 0; i < dwRead; i++) {
				if(!(ir[i].EventType & KEY_EVENT)) {
					continue;
				}
				if (ir[i].Event.KeyEvent.bKeyDown) {
#ifdef _UNICODE
					chr = ir[i].Event.KeyEvent.uChar.UnicodeChar;
#else
					chr = ir[i].Event.KeyEvent.uChar.AsciiChar;
#endif
					WORD vk_code = ir[i].Event.KeyEvent.wVirtualKeyCode;
					key = 0;
					if(chr == 0 && vk_code != 0) {
						for(int n = 0; vkkey2keycode[n].vk_code != 0; n++) {
							if (vk_code == vkkey2keycode[n].vk_code) {
								key = vkkey2keycode[n].code;
								break;
							}
						}
					}
					key_buffer.Push(chr, key);
				}
			}
		}
	}
#endif

	int readed;
#ifdef USE_TELNET_SERVER
	// input from terminal
	_TCHAR data[64];
	int ignore = 0;
	if ((readed = telnet->read_data(data, 64)) > 0) {
		for(int i = 0; i < readed; i++) {
			chr = data[i];
			key = 0;
			// ignore telnet command
			if (i >= 2 && (uint8_t)data[i-1] == 0xff && (uint8_t)chr >= 0xfb && (uint8_t)chr <= 0xfe) {
				ignore = 2;
			} else if ((uint8_t)chr >= 0xf0) {
				ignore = 1;
			} else if (chr == 0x1b) {
				// escape sequence
				for(int n = 0; esc2keycode[n].seq != NULL; n++) {
					int len = (int)_tcslen(esc2keycode[n].seq);
					if (len + 1 == readed && _tcsncmp(&data[i+1], esc2keycode[n].seq, len) == 0) {
						key = esc2keycode[n].code;
						chr = 0;
						i = readed - 1;
						break;
					}
				}
			}
			if (ignore > 0) {
				ignore--;
				continue;
			}
			key_buffer.Push(chr, key);
		}
	}
#endif
#ifdef USE_STANDARD_INOUT
	// input from stdin
	fd_set sel;
	FD_ZERO(&sel);
	FD_SET(0, &sel);
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = 10000;
	int rc = select(1, &sel, NULL, NULL, &tim);
	if (rc > 0) {
		for(int i=0; i < 64 && (readed = _fgettc(stdin)) != EOF; i++) {
			enter_done = SetCommandLine((_TCHAR)readed, true);
			if (enter_done) {
				break;
			}
		}
	}
#endif

	while(key_buffer.Pop(chr, key)) {
		if (chr || key) {
			enter_done = SetCommandLine(chr, key, true);
			if (enter_done) {
				break;
			}
		}
	}
	return enter_done;
}

bool DebuggerConsole::EscapePressed()
{
	bool pressed = false;
#ifdef USE_CONSOLE_WINDOW
	if (!pressed) {
//		pressed = ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0 && IsConsoleActive());
		DWORD dwRead;
		INPUT_RECORD ir[16];
		if(GetNumberOfConsoleInputEvents(hStdIn, &dwRead)) {
			if(dwRead > 0 && ReadConsoleInput(hStdIn, ir, 16, &dwRead)) {
				for(unsigned int i = 0; i < dwRead; i++) {
#ifdef _UNICODE
					if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.UnicodeChar) {
						_TCHAR chr = ir[i].Event.KeyEvent.uChar.UnicodeChar;
#else
					if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown && ir[i].Event.KeyEvent.uChar.AsciiChar) {
						_TCHAR chr = ir[i].Event.KeyEvent.uChar.AsciiChar;
#endif
						if (chr == 0x1b) {
							pressed = true;
							break;
						}
					}
				}
			}
		}
	}
#endif
	int readed;
#ifdef USE_TELNET_SERVER
	if (!pressed) {
		_TCHAR data[16];
		if ((readed = telnet->read_data(data, 16)) > 0) {
			for(int i = 0; i < readed; i++) {
				if (data[i] == 0x1b) {
					pressed = true;
					break;
				}
			}
		}
	}
#endif
#ifdef USE_STANDARD_INOUT
	if (!pressed) {
		fd_set sel;
		FD_ZERO(&sel);
		FD_SET(0, &sel);
		struct timeval tim;
		tim.tv_sec = 0;
		tim.tv_usec = 1000;
		int rc = select(1, &sel, NULL, NULL, &tim);
		if (rc > 0) {
			for(int i=0; i < 16 && (readed = _fgettc(stdin)) != EOF; i++) {
				if (readed == 0x1b) {
					pressed = true;
					break;
				}
			}
		}
	}
#endif
	return pressed;
}

int DebuggerConsole::ParseInput()
{
	paramnum = UTILITY::get_parameters(command_buffer.Ptr(), DC_MAX_COMMAND_LEN, params, 32);
	return paramnum;
}

void DebuggerConsole::ClearInput()
{
	command_buffer.ClearCurr();
}

bool DebuggerConsole::NowDisable() const
{
	return pConfig->now_power_off;
}

bool DebuggerConsole::NowPausing() const
{
	return emu->get_pause(3);
}

// ---------------------------------------------------------------------------

uint32_t DebuggerConsole::DeciToInt(const _TCHAR *str)
{
	// decimal
	return (uint32_t)_tcstoul(str, NULL, 10);
}

/// convert integer from hexa string
///
/// @param[in] str : 'a' : one ascii character
///                  %nn : decimal string
///                  0000:0000 : hexa string with colon
///                  otherwise hexa string
/// @return unsigned integer
uint32_t DebuggerConsole::HexaToInt(const _TCHAR *str)
{
	_TCHAR tmp[DC_MAX_BUFFER_LEN], *s;

	if(str == NULL || _tcslen(str) == 0) {
		return 0;
	}
	memset(tmp, 0, sizeof(tmp));
	UTILITY::tcscpy(tmp, DC_MAX_BUFFER_LEN, str);

	if(_tcslen(tmp) == 3 && tmp[0] == _T('\'') && tmp[2] == _T('\'')) {
		// ank
		return tmp[1] & 0xff;
	} else if((s = _tcsstr(tmp, _T(":"))) != NULL) {
		// 0000:0000
		s[0] = _T('\0');
		return (HexaToInt(tmp) << 4) + HexaToInt(s + 1);
	} else if(tmp[0] == _T('%')) {
		// decimal
		return (uint32_t)_tcstoul(tmp + 1, NULL, 10);
	}
	return (uint32_t)_tcstoul(tmp, NULL, 16);
}

uint8_t DebuggerConsole::HexaToByte(const char *value) const
{
	char tmp[3];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = '\0';
	return (uint8_t)strtoul(tmp, NULL, 16);
}

uint16_t DebuggerConsole::HexaToWord(const char *value) const
{
	char tmp[5];
	tmp[0] = value[0];
	tmp[1] = value[1];
	tmp[2] = value[2];
	tmp[3] = value[3];
	tmp[4] = '\0';
	return (uint16_t)strtoul(tmp, NULL, 16);
}

uint32_t DebuggerConsole::HexaToUInt(const char *value, int len) const
{
	if (!value || len <= 0) return 0;
	int i = 0;
	char tmp[9];
	if (len > 8) len = 8;
	for(; i<len; i++) {
		tmp[i] = value[i];
	}
	tmp[i] = '\0';
	return (uint32_t)strtoul(tmp, NULL, 16);
}

bool DebuggerConsole::IsHexa(const _TCHAR *str) const
{
	_TCHAR *err = NULL;
	// hexa
	_tcstol(str, &err, 16);
	return (err == NULL);
}

/// convert integer from hexa string
///
/// @param[in] str : 'a' : one ascii character
///                  %nn : decimal string
///                  otherwise hexa string
/// @param[in] data : converted integer
/// @return true / false
bool DebuggerConsole::HexaToInt(const _TCHAR *str, uint32_t *data) const
{
	_TCHAR *err = NULL;
	int ad = 0;

	if(str == NULL || _tcslen(str) == 0) {
		// zero
	} else if(_tcslen(str) == 3 && str[0] == _T('\'') && str[2] == _T('\'')) {
		// ank
		ad = (str[1] & 0xff);
	} else if(str[0] == _T('%')) {
		// decimal
		ad = (uint32_t)_tcstol(str + 1, &err, 10);
	} else {
		// hexa
		ad = (uint32_t)_tcstol(str, &err, 16);
	}
	if (data) *data = ad;
	return (err != NULL && *err == _T('\0'));
}

/// convert integer from hexa string
///
/// @param[in] str : 'a' : one ascii character
///                  0000-1111 : range 0000 and 1111
///                  %nn : decimal string
///                  otherwise hexa string
/// @param[out] addr : converted integer
/// @param[out] len : range (add+len)
/// @return true
bool DebuggerConsole::HexaToInt(const _TCHAR *str, uint32_t *addr, int *len)
{
	_TCHAR tmp[DC_MAX_BUFFER_LEN], *s;
	uint32_t ad = 0;
	int le = 0;
	bool rc = true;

	memset(tmp, 0, sizeof(tmp));
	UTILITY::tcscpy(tmp, DC_MAX_BUFFER_LEN, str);

	if(str == NULL || _tcslen(str) == 0) {
		// zero
	} else if(_tcslen(tmp) == 3 && tmp[0] == _T('\'') && tmp[2] == _T('\'')) {
		// ank
		ad = (tmp[1] & 0xff);
	} else if((s = _tcsstr(tmp, _T("-"))) != NULL) {
		// 0000-0000
		s[0] = _T('\0');
		ad = HexaToInt(tmp);
		le = HexaToInt(s + 1) - ad;
	} else if(tmp[0] == _T('%')) {
		// decimal
		ad = (uint32_t)_tcstol(tmp + 1, NULL, 10);
	} else {
		// hexa
		ad = HexaToInt(tmp);
	}
	if (addr) *addr = ad;
	if (len) *len = le;
	return rc;
}

bool DebuggerConsole::DeciToNum(const _TCHAR *str, int &st_line, int &ed_line)
{
	_TCHAR tmp[DC_MAX_BUFFER_LEN], *s;
	bool rc = true;

	memset(tmp, 0, sizeof(tmp));
	UTILITY::tcscpy(tmp, DC_MAX_BUFFER_LEN, str);

	if(str == NULL || _tcslen(str) == 0) {
		// zero
	} else if((s = _tcsstr(tmp, _T("-"))) != NULL) {
		// 0000-0000
		s[0] = _T('\0');
		if (str[0] != _T('-')) {
			st_line = (int)_tcstol(tmp, NULL, 10);
		}
		if (s[1] != _T('\0')) {
			ed_line = (int)_tcstol(s + 1, NULL, 10);
		}
	} else {
		// decimal
		st_line = (int)_tcstol(tmp, NULL, 10);
		ed_line = st_line;
	}
	return rc;
}

// ---------------------------------------------------------------------------

BreakPoints *DebuggerConsole::GetBreakPoints(int num)
{
	const struct st_bp_map {
		enBpTpType num;
		BreakPoints::en_break_point_type type;
		bool is_trace;
	} c_bp_map[] = {
		{ BP_FETCH, BreakPoints::BP_FETCH_OP, false },
		{ TP_FETCH, BreakPoints::BP_FETCH_OP, true  },
		{ BP_FETCH_PH, BreakPoints::BP_FETCH_OP_PH, false },
		{ TP_FETCH_PH, BreakPoints::BP_FETCH_OP_PH, true  },
		{ BP_MEMRD, BreakPoints::BP_READ_MEMORY, false },
		{ TP_MEMRD, BreakPoints::BP_READ_MEMORY, true  },
		{ BP_MEMRD_PH, BreakPoints::BP_READ_MEMORY_PH, false },
		{ TP_MEMRD_PH, BreakPoints::BP_READ_MEMORY_PH, true  },
		{ BP_MEMWR, BreakPoints::BP_WRITE_MEMORY, false },
		{ TP_MEMWR, BreakPoints::BP_WRITE_MEMORY, true  },
		{ BP_MEMWR_PH, BreakPoints::BP_WRITE_MEMORY_PH, false },
		{ TP_MEMWR_PH, BreakPoints::BP_WRITE_MEMORY_PH, true  },
		{ BP_IORD, BreakPoints::BP_INPUT_IO, false },
		{ TP_IORD, BreakPoints::BP_INPUT_IO, true  },
		{ BP_IOWR, BreakPoints::BP_OUTPUT_IO, false },
		{ TP_IOWR, BreakPoints::BP_OUTPUT_IO, true  },
		{ BP_INTR, BreakPoints::BP_INTERRUPT, false },
		{ TP_INTR, BreakPoints::BP_INTERRUPT, true  },
		{ BP_EXCEPT, BreakPoints::BP_EXCEPTION, false },
		{ TP_EXCEPT, BreakPoints::BP_EXCEPTION, true  },
		{ BP_UNKNOWN, BreakPoints::BP_FETCH_OP, false },
	};

	for(int i=0; c_bp_map[i].num != BP_UNKNOWN; i++) {
		if (num == c_bp_map[i].num) {
			if (c_bp_map[i].is_trace) {
				return current_dev->debugger->get_tps(c_bp_map[i].type);
			} else {
				return current_dev->debugger->get_bps(c_bp_map[i].type);
			}
		}
	}
	return NULL;
}

#if 0
bool DebuggerConsole::IsConsoleActive()
{
#ifdef USE_CONSOLE_WINDOW
	HWND hWnd = GetForegroundWindow();
//	return (hWnd != NULL && hWnd == GetConsoleWindow());
	return (hWnd != NULL && hWnd == FindWindow("ConsoleWindowClass", NULL));
#else
	return false;
#endif
}
#endif

// ---------------------------------------------------------------------------

int DebuggerConsole::GetAddressType(int num)
{
	int type = -1;
	if (num == 1) {
		type = current_dev->phys_type;
		if (paramnum >= 2) {
			type = HexaToInt(params[1]);
		}
		if (current_dev->mem->debug_physical_addr_mask(type) == 0) {
			PrintError(_T("No available memory on specified type now."));
			Cr();
			return -2;
		}
	}
	return type;
}

void DebuggerConsole::UsageAddressType(int num)
{
	switch(num) {
	case 1:
		Print(_T("  <type> - specify a kind of memory."));
		for(int i=0; i<10; i++) {
			if (!current_dev->mem->debug_physical_addr_type_name(i, buffer, DC_MAX_BUFFER_LEN)) {
				break;
			}
			if (buffer[0] != _T('\0')) {
				Printf(_T("    %d : "), i);
				Print(buffer);
			}
		}
		break;
	}
}

// ---------------------------------------------------------------------------

/// dump memory
///
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandDumpMemory(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageDumpMemory(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum > (pos+2)) {
		PrintError(_T("Invalid number of parameters."));
		Cr();
		UsageDumpMemory(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageDumpMemory(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);
	uint32_t data_mask = current_dev->cpu->debug_data_mask();
	uint32_t start_addr = (num == 1 ? current_dev->dump_phys_addr : current_dev->dump_addr);
	if(paramnum >= (1 + pos)) {
		start_addr = HexaToInt(params[pos]);
	}
	start_addr &= addr_mask;

	uint32_t end_addr = start_addr + 8 * 16 - 1;
	if(paramnum >= (2 + pos)) {
		end_addr = HexaToInt(params[pos+1]);
	}
	end_addr &= addr_mask;

	if(start_addr > end_addr) {
		end_addr = addr_mask;
	}
	// header
#ifdef _MBS1
	if (num == 1) {
		Printf(Cyan, _T(PHYS_ADDR_STRING), _T("ADDR"));
	} else {
		Printf(Cyan, _T(PHYS_ADDR_STRING), _T("PADDR "));
		Printf(Cyan, _T(ADDR_STRING), _T("ADDR"));
	}
#else
	Printf(Cyan, _T(ADDR_STRING), _T("ADDR"));
#endif
	Print(_T("  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F  0123456789ABCDEF"));
	// data
	for(uint64_t addr = (start_addr & ~0x0f); addr <= end_addr; addr++) {
		if(addr > addr_mask) {
			end_addr = addr_mask;
			break;
		}
		if((addr & 0x0f) == 0) {
#ifdef _MBS1
			if (num == 1) {
				Printf(White, _T(PHYS_ADDR_FORMAT), addr & addr_mask);
			} else {
				Printf(White, _T(PHYS_ADDR_FORMAT)_T(" "), current_dev->mem->debug_latch_address(addr & addr_mask));
				Printf(_T(ADDR_FORMAT), addr & addr_mask);
			}
#else
			Printf(White, _T(ADDR_FORMAT), addr & addr_mask);
#endif
			Print(_T(" "), false);
			memset(buffer, 0, sizeof(buffer));
		}
		if(addr < start_addr) {
			Print(_T("   "), false);
			buffer[addr & 0x0f] = _T(' ');
		} else {
			uint32_t data = current_dev->mem->debug_read_data8(type, addr & addr_mask) & data_mask;
//			Print((addr & 0xf) == 8 ? _T("-") : _T(" "), false);
			Print(_T(" "), false);
			Printf(_T(DATA_BYTE_FORMAT), data);
			// store an ascii code to buffer and also a hankaku katakana code if cp932 terminal
			buffer[addr & 0x0f] = ((data >= 0x20 && data <= 0x7e) || (cp932 && data >= 0xa1 && data <= 0xdf)) ? data : _T('.');
		}
		if((addr & 0x0f) == 0x0f) {
			Print(_T("  "), false);
			Out();
		}
	}
	if((end_addr & 0x0f) != 0x0f) {
		for(uint32_t addr = (end_addr & 0x0f) + 1; addr <= 0x0f; addr++) {
			Print(_T("   "), false);
		}
		Print(_T("  "), false);
		Out();
	}
	if (num == 1) {
		current_dev->dump_phys_addr = (end_addr + 1) & addr_mask;
		current_dev->phys_type = type;
		command_buffer.ShrinkPrev(2); // remove parameters to dump continuously
	} else {
		current_dev->dump_addr = (end_addr + 1) & addr_mask;
		command_buffer.ShrinkPrev(1); // remove parameters to dump continuously
	}
}

/// usage - dump memory
///
void DebuggerConsole::UsageDumpMemory(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr1(s, _T("D"), _T("[<start address> [<end address>]]"), _T("Dump memory."));
		break;
	case 1:
		UsageCmdStr1(s, _T("DP"), _T("[<type> [<start address> [<end address>]]]"), _T("Dump physical memory."));
		break;
	}
	if (s) return;

	UsageAddressType(num);
	Print(_T("  <start address> - specify start address."));
	Print(_T("  <end address> - specify end address."));
	Print(_T("  If end address isn't specified, end address is set start address + 128bytes."));
}

// ---------------------------------------------------------------------------

/// write to memory
///
/// @param[in] n : write data size 1:byte 2:word 4:dword
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandEditMemoryBinary(int n, int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageEditMemory(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum < (pos+2)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageEditMemory(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageEditMemory(false, num);
		return;
	}

	uint32_t addr = HexaToInt(params[pos]);
	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);

	addr &= addr_mask;

	for(int i = pos + 1; i < paramnum; i++) {
		switch(n) {
		case 1:
			current_dev->mem->debug_write_data8(type, addr, HexaToInt(params[i]) & 0xff);
			addr = (addr + 1) & addr_mask;
			break;
		case 2:
			current_dev->mem->debug_write_data16(type, addr, HexaToInt(params[i]) & 0xffff);
			addr = (addr + 2) & addr_mask;
			break;
		case 4:
			current_dev->mem->debug_write_data32(type, addr, HexaToInt(params[i]));
			addr = (addr + 4) & addr_mask;
			break;
		}
	}
}

// ---------------------------------------------------------------------------

/// write to memory using ASCII string
///
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandEditMemoryAscii(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageEditMemory(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum != (pos+2)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageEditMemory(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageEditMemory(false, num);
		return;
	}

	uint32_t addr = HexaToInt(params[pos]);
	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);

	addr &= addr_mask;

	const _TCHAR *token = params[pos+1];
	int len = (int)_tcslen(token);
	for(int i = 0; i < len; i++) {
		current_dev->mem->debug_write_data8(type, addr, token[i]);
		addr = (addr + 1) & addr_mask;
	}
}

/// usage - write to memory using ASCII string
void DebuggerConsole::UsageEditMemory(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr2(s, _T("E[{B,W,D}]"), _T("<address> <value> [<value> ...]"), _T("Edit memory (byte,word,dword).")
			,_T("EA"), _T("<address> \"<string>\""), _T("Edit memory (ascii)."));
		break;
	case 1:
		UsageCmdStr2(s, _T("EP[{B,W,D}]"), _T("<type> <address> <value> [<value> ...]"), _T("Edit physical memory (byte,word,dword).")
			,_T("EPA"), _T("<type> <address> \"<string>\""), _T("Edit physical memory (ascii)."));
		break;
	}
	if (s) return;

	UsageAddressType(num);
	Print(_T("  <address> - specify address."));
	Print(_T("  <value> - specify value."));
	Print(_T("  <string> - specify ascii string."));
}

// ---------------------------------------------------------------------------

/// read from I/O port
///
/// @param[in] n : write data size 1:byte 2:word 4:dword
void DebuggerConsole::CommandInputIOBinary(int n)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageEditIO(false);
		return;
	}
	if(paramnum != 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageEditIO(false);
		return;
	}
	if (!current_dev->cpu->debug_ioport_is_supported()) {
		PrintError(_T("Use D command instead because CPU takes memory mapped I/O."));
		return;
	}

	switch(n) {
	case 1:
		Printf(_T("%02X"), current_dev->cpu->debug_read_io8(HexaToInt(params[1])) & 0xff);
		Cr();
		break;
	case 2:
		Printf(_T("%02X"), current_dev->cpu->debug_read_io16(HexaToInt(params[1])) & 0xffff);
		Cr();
		break;
	case 4:
		Printf(_T("%02X"), current_dev->cpu->debug_read_io32(HexaToInt(params[1])));
		Cr();
		break;
	}
}

// ---------------------------------------------------------------------------

/// write to I/O port
///
/// @param[in] n : write data size 1:byte 2:word 4:dword
void DebuggerConsole::CommandOutputIOBinary(int n)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageEditIO(false);
		return;
	}
	if(paramnum != 3) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageEditIO(false);
		return;
	}
	if (!current_dev->cpu->debug_ioport_is_supported()) {
		PrintError(_T("Use E command instead because CPU takes memory mapped I/O."));
		return;
	}

	switch(n) {
	case 1:
		current_dev->cpu->debug_write_io8(HexaToInt(params[1]), HexaToInt(params[2]) & 0xff);
		break;
	case 2:
		current_dev->cpu->debug_write_io16(HexaToInt(params[1]), HexaToInt(params[2]) & 0xffff);
		break;
	case 4:
		current_dev->cpu->debug_write_io32(HexaToInt(params[1]), HexaToInt(params[2]));
		break;
	}
}

/// usage - read / write I/O port
void DebuggerConsole::UsageEditIO(bool s)
{
	UsageCmdStr2(s, _T("I[{B,W,D}]"), _T("<port>"), _T("Input port (byte,word,dword).")
		, _T("O[{B,W,D}]"), _T("<port> <value>"), _T("Output port (byte,word,dword)."));
	if (s) return;

	Print(_T("  <port> - specify i/o port address."));
	Print(_T("  <value> - specify value."));
}

// ---------------------------------------------------------------------------

/// read / write register in CPU
void DebuggerConsole::CommandAccessRegister()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageAccessRegister(false);
		return;
	}

	if(paramnum == 1) {
		// show
		current_dev->cpu->debug_regs_info(buffer, DC_MAX_BUFFER_LEN);
		Out();
	} else if(paramnum == 3) {
		// edit
		if(!current_dev->cpu->debug_write_reg(params[1], HexaToInt(params[2]))) {
			Print(_T("Unknown register: "), false);
			Print(params[1]);
		}
	} else {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageAccessRegister(false);
	}
}

/// usage - read / write register in CPU
void DebuggerConsole::UsageAccessRegister(bool s)
{
	UsageCmdStr1(s, _T("R"), _T("[<reg> <value>]"), _T("Show/Edit register(s) in cpu."));
	if (s) return;

	Print(_T("  <reg> - specify a register."));
	Print(_T("  <value> - specify a value."));
	Print(_T("  If reg and value is specified, modify specified register."));
}

// ---------------------------------------------------------------------------

/// read / write register in peripheral devices
void DebuggerConsole::CommandAccessDevice()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageAccessDevice(false);
		return;
	}

	if(paramnum < 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageAccessDevice(false);
		return;
	}

	uint32_t num = 0;
	const _TCHAR *dname = NULL;
	if (!dp->vm->get_debug_device_name(params[1], &num, NULL, &dname)) {
		PrintError(_T("Invalid device name: "), false);
		Print(params[1]);
		UsageAccessDevice(false);
		return;
	}

	if(paramnum == 2) {
		// show
		dp->vm->debug_regs_info(num, buffer, DC_MAX_BUFFER_LEN);
		Out();
	} else if(paramnum == 4) {
		// edit
		uint32_t reg_num;
		bool rc = HexaToInt(params[2], &reg_num);
		if (rc) {
			if(!dp->vm->debug_write_reg(num, reg_num, HexaToInt(params[3]))) {
				PrintError(_T("Invalid register number."));
			}
		} else {
			if(!dp->vm->debug_write_reg(num, params[2], HexaToInt(params[3]))) {
				PrintError(_T("Invalid register name."));
			}
		}
	} else {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageAccessDevice(false);
	}
}

/// usage - read / write register in peripheral devices
void DebuggerConsole::UsageAccessDevice(bool s)
{
	UsageCmdStr1(s, _T("RD"), _T("<device name> [{<regno>|<regname>} <value>]"), _T("Show/Edit register(s) in specified device."));
	if (s) return;

	Print(_T("  <device name> - specify following device name."));
	dp->vm->get_debug_device_names_str(buffer, DC_MAX_BUFFER_LEN);
	Print(buffer);
	Cr();
	Print(_T("  <regno>   - specify a register number."));
	Print(_T("  <regname> - specify a register name."));
	Print(_T("  <value>   - specify a value."));
	Print(_T("  If regno and value is specified, modify specified register."));
}

/// show / hide register on trace list
/// @param[in] n : show = 1 / hide = 0
void DebuggerConsole::CommandToggleRegister(int n)
{
	if(paramnum >= 2) {
		Cr();
		UsageToggleRegister(false);
		return;
	}

	current_dev->show_regs = (n != 0);
}

/// usage - show / hide register on trace list
void DebuggerConsole::UsageToggleRegister(bool s)
{
	UsageCmdStr1(s, _T("R{S,H}"), _T(""), _T("Show/Hide register(s) on the trace."));
	if (s) return;
}

// ---------------------------------------------------------------------------

/// search data in memory
///
/// @param[in] n : write data size 1:byte 2:word 4:dword
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandSearch(int n, int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSearch(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum < (pos+3)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageSearch(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageSearch(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);
	uint32_t start_addr = HexaToInt(params[pos]) & addr_mask;
	uint32_t end_addr = HexaToInt(params[pos+1]) & addr_mask;
	uint32_t list[32];
	_TCHAR data_format[6];
	for(int i = pos+2, j = 0; i < paramnum; i++, j++) {
		list[j] = HexaToInt(params[i]);
		if (n < 4) list[j] &= ((1 << (n * 8)) - 1);
	}
	UTILITY::stprintf(data_format, 6, _T("%%0%dX"), n * 2);
	start_addr &= ~(n - 1);
	bool no_data_found = true;
	for(uint64_t addr = start_addr; addr <= end_addr; addr+=n) {
		bool found = true;
		switch(n) {
		case 1:
			for(int i = pos+2, j = 0; i < paramnum; i++, j++) {
				if((current_dev->mem->debug_read_data8(type, (addr + j) & addr_mask) & 0xff) != list[j]) {
					found = false;
					break;
				}
			}
			break;
		case 2:
			for(int i = pos+2, j = 0; i < paramnum; i+=2, j+=2) {
				if((current_dev->mem->debug_read_data16(type, (addr + j) & addr_mask) & 0xffff) != list[j]) {
					found = false;
					break;
				}
			}
			break;
		case 4:
			for(int i = pos+2, j = 0; i < paramnum; i+=4, j+=4) {
				if(current_dev->mem->debug_read_data32(type, (addr + j) & addr_mask) != list[j]) {
					found = false;
					break;
				}
			}
			break;
		}
		if(found) {
			no_data_found = false;
			Printf(_T(ADDR_FORMAT), addr);
			for(int i = pos+2, j = 0; i < paramnum; i++, j++) {
				Print(_T(" "), false);
				Printf(data_format, list[j]);
			}
			Cr();
		}
		if(EscapePressed()) {
			no_data_found = false;
			Print(Yellow, _T("Aborted by user."));
			SetTextColor(White);
			break;
		}
	}
	if (no_data_found) {
		Print(Yellow, _T("No data found."));
		SetTextColor(White);
	}
}

/// write to memory using ASCII string
///
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandSearchAscii(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSearch(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum != (pos+3)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageSearch(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageSearch(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);
	uint32_t start_addr = HexaToInt(params[pos]) & addr_mask;
	uint32_t end_addr = HexaToInt(params[pos+1]) & addr_mask;

	const _TCHAR *token = params[pos + 2];
	int len = (int)_tcslen(token);
	bool no_data_found = true;
	for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
		bool found = true;
		for(int i = 0; i < len; i++) {
			if((const _TCHAR)current_dev->mem->debug_read_data8(type, (addr + i) & addr_mask) != token[i]) {
				found = false;
				break;
			}
		}
		if(found) {
			no_data_found = false;
			Printf(_T(ADDR_FORMAT), addr);
			Print(_T(" "), false);
			Print(token);
		}
		if(EscapePressed()) {
			no_data_found = false;
			Print(Yellow, _T("Aborted by user."));
			SetTextColor(White);
			break;
		}
	}
	if (no_data_found) {
		Print(Yellow, _T("No data found."));
		SetTextColor(White);
	}
}

/// usage - search data in memory
void DebuggerConsole::UsageSearch(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr2(s, _T("S[{B,W,D}]"), _T("<start address> <end address> <list>"), _T("Search data(s).")
			, _T("SA"), _T("<start address> <end address> \"<string>\""), _T("Search data (ascii)."));
		break;
	case 1:
		UsageCmdStr2(s, _T("SP[{B,W,D}]"), _T("<type> <start address> <end address> <list>"), _T("Search data(s) using physical address.")
			, _T("SPA"), _T("<type> <start address> <end address> \"<string>\""), _T("Search data (ascii) using physical address."));
		break;
	}
	if (s) return;

	UsageAddressType(num);
	Print(_T("  <start address> - specify start address."));
	Print(_T("  <end address> - specify end address."));
	Print(_T("  <list> - specify values separate by space to match data in memory."));
	Print(_T("  <string> - specify ascii string."));
}

// ---------------------------------------------------------------------------

/// disassemble data in memory using current CPUs mnemonic
///
/// @param[in] num : 1: using physical address
void DebuggerConsole::CommandUnassemble(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageUnassemble(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum > (pos+2)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageUnassemble(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageUnassemble(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->prog_addr_mask);

	uint32_t addr = (num == 1 ? current_dev->dasm_phys_addr : current_dev->dasm_addr);
	if(paramnum >= (pos+1)) {
		addr = HexaToInt(params[pos]) & addr_mask;
	}
	uint32_t end_addr = addr;
	if(paramnum >= (pos+2)) {
		end_addr = HexaToInt(params[pos+1]) & addr_mask;
	}
	for(uint32_t n = 0; (paramnum == (pos+2) ? addr < end_addr : n < 16); n++) {
		if (current_dev->cpu->debug_dasm_label(type, addr, buffer, DC_MAX_BUFFER_LEN)) {
			Out();
		}
		int len = current_dev->cpu->debug_dasm(type, addr, buffer, DC_MAX_BUFFER_LEN, 0x10);
#ifndef USE_EMU_INHERENT_SPEC
		Printf(_T("%08X  "), addr);
		for(int i = 0; i < len; i++) {
			Printf(_T("%02X"), current_dev->mem->debug_read_data8(type, (addr + i) & addr_mask));
		}
		for(int i = len; i < 8; i++) {
			Print(_T("  "), false);
		}
		Print(_T("  "), false);
#endif
		Out();
		addr = (addr + len) & addr_mask;
	}
	if (num == 1) {
		current_dev->dasm_phys_addr = addr;
		current_dev->phys_type = type;
		command_buffer.ShrinkPrev(2); // remove parameters to dump continuously
	} else {
		current_dev->dasm_addr = addr;
		command_buffer.ShrinkPrev(1); // remove parameters to dump continuously
	}
}

/// usage - disassemble data in memory using current CPUs mnemonic
void DebuggerConsole::UsageUnassemble(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr1(s, _T("U"), _T("[<start address> [<end address>]]"), _T("Unassemble data in memory."));
		break;
	case 1:
		UsageCmdStr1(s, _T("UP"), _T("<type> [<start address> [<end address>]]"), _T("Unassemble data using physical address."));
		break;
	}
	if (s) return;

	UsageAddressType(num);
	Print(_T("  <start address> - specify start address."));
	Print(_T("  <end address> - specify end address."));
	Print(_T("  If end address isn't specified, unassemble 16 steps."));
}

// ---------------------------------------------------------------------------

/// calculate data
void DebuggerConsole::CommandCalculateHexa()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageCalculateHexa(false);
		return;
	}

	if(paramnum != 3) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageCalculateHexa(false);
		return;
	}

	uint32_t l = HexaToInt(params[1]);
	uint32_t r = HexaToInt(params[2]);
	Printf(_T("Add:%08X  Sub:%08X"), l + r, l - r);
	Cr();
}

/// usage - calculate data
void DebuggerConsole::UsageCalculateHexa(bool s)
{
	UsageCmdStr1(s, _T("H"), _T("<value1> <value2>"), _T("Add and subtract hexa value1 and value2."));
	if (s) return;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSetFileName()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSetFileName(false);
		return;
	}

	if(paramnum >= 2) {
		current_dev->debugger->set_file_path(params[1]);
	}
	Printf(_T("Current dump file name is \"%s\"."), current_dev->debugger->get_file_path());
	Cr();
}

void DebuggerConsole::UsageSetFileName(bool s)
{
	UsageCmdStr1(s, _T("DN"), _T("[<filename>]"), _T("Show/Set filename for dump file."));
	if (s) return;

	Print(_T("  <filename> - specify file name."));
	Print(_T("  Filename is \"debug.bin\" in default."));
	UsageBinaryFileType();
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandLoadBinaryFile(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageBinaryFile(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum < (pos+1)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageBinaryFile(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageBinaryFile(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);

	bool rc = false;
	if(UTILITY::check_file_extension(current_dev->debugger->get_file_path(), _T(".hex"))) {
		// load from Intel hex format file
		uint32_t offset = 0;
		if(paramnum >= (pos + 1)) {
			offset = HexaToInt(params[pos]);
		}
		rc = LoadIntelHexFile(current_dev->debugger->get_file_path(), offset, addr_mask, type);
	} else if(UTILITY::check_file_extensions(current_dev->debugger->get_file_path(), _T(".s"), _T(".mot"), NULL)) {
		// load from Motorola S-Record format file
		uint32_t offset = 0;
		if(paramnum >= (pos + 1)) {
			offset = HexaToInt(params[pos]);
		}
		rc = LoadMotorolaSFile(current_dev->debugger->get_file_path(), offset, addr_mask, type);
	} else {
		// load from binary dump file
		uint32_t start_addr = 0, end_addr = addr_mask;
		if(paramnum >= (pos + 1)) {
			start_addr = HexaToInt(params[pos]) & addr_mask;
		}
		if(paramnum >= (pos + 2)) {
			end_addr = HexaToInt(params[pos+1]) & addr_mask;
		}
		rc = LoadBinaryRawFile(current_dev->debugger->get_file_path(), start_addr, end_addr, addr_mask, type);
	}
	if (rc) {
		Print(_T("Loaded from "), false);
	} else {
		PrintError(_T("Can't open "), false);
	}
	Print(current_dev->debugger->get_file_path());
}

/// load from binary dump file
bool DebuggerConsole::LoadBinaryRawFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::READ_BINARY);
	if(!rc) return rc;

	for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
		int data = fio.Fgetc();
		if(data == EOF) {
			break;
		}
		current_dev->mem->debug_write_data8(type, addr & addr_mask, data);
	}
	fio.Fclose();
	return rc;
}

/// load from Intel hex format file
bool DebuggerConsole::LoadIntelHexFile(const _TCHAR *file_path, uint32_t offset, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::READ_ASCII);
	if(!rc) return rc;

	uint32_t linear = 0, segment = 0;
	char line[DC_MAX_BUFFER_LEN];
	while(fio.Fgets(line, sizeof(line)) != NULL) {
		if(line[0] != ':') continue;
		int rtype = HexaToByte(line + 7);
		if(rtype == 0x00) {
			uint32_t bytes = HexaToByte(line + 1);
			uint32_t addr = HexaToWord(line + 3) + offset + linear + segment;
			for(uint32_t i = 0; i < bytes; i++) {
				current_dev->mem->debug_write_data8(type, (addr + i) & addr_mask, HexaToByte(line + 9 + 2 * i));
			}
		} else if(rtype == 0x01) {
			break;
		} else if(rtype == 0x02) {
			segment = HexaToWord(line + 9) << 4;
//			start_addr = 0;
		} else if(rtype == 0x04) {
			linear = HexaToWord(line + 9) << 16;
//			start_addr = 0;
		}
	}
	fio.Fclose();
	return rc;
}

/// load from Motorola S-Record format file
bool DebuggerConsole::LoadMotorolaSFile(const _TCHAR *file_path, uint32_t offset, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::READ_ASCII);
	if(!rc) return rc;

	char line[DC_MAX_BUFFER_LEN];
	uint32_t addr_field = 0;	// 1:16bits 2:24bits 3:32bits
	while(fio.Fgets(line, sizeof(line)) != NULL) {
		if(line[0] != 'S') continue;	// first char is 'S'
		if(line[1] < '0' || '9' < line[1]) continue;	// second char is [0-9]
		// field type
		int rtype = line[1] - 0x30;
		// address field
		if (rtype >= 1 && rtype <= 3) addr_field = rtype;
		else if (rtype >= 7 && rtype <= 9) addr_field = 10 - rtype;

		// field length
		uint32_t len = HexaToByte(&line[2]);
		uint32_t csum = len;
		if (len == 0) continue;

		if (rtype >= 1 && rtype <= 3) {
			// data field
			uint32_t addr = HexaToUInt(&line[4], addr_field * 2 + 2) + offset;
			uint8_t data = 0;
			for(uint32_t i = 0; i < addr_field + 1; i++) {
				data = HexaToByte(&line[4 + i * 2]);
				csum += data;
			}
			len -= addr_field + 2;
			for(uint32_t i = 0; i < len; i++) {
				data = HexaToByte(&line[addr_field * 2 + 6 + i * 2]);
				current_dev->mem->debug_write_data8(type, (addr + i) & addr_mask, data);
				csum += data;
			}
			// check sum
			csum = ((~csum) & 0xff);
			data = HexaToByte(&line[addr_field * 2 + 6 + len * 2]);
			if (csum != data) {
				rc = false;
			}
		} else if (rtype >= 7 && rtype <= 9) {
			// execute address and terminate
			break;
		}
	}

	fio.Fclose();
	return rc;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSaveBinaryFile(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageBinaryFile(false, num);
		return;
	}

	int pos = (num == 1 ? 2 : 1);

	if(paramnum != (pos+2)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageBinaryFile(false, num);
		return;
	}

	int type = GetAddressType(num);
	if (type < -1) {
		UsageBinaryFile(false, num);
		return;
	}

	uint32_t addr_mask = (num == 1 ? current_dev->mem->debug_physical_addr_mask(type) : current_dev->data_addr_mask);

	uint32_t start_addr = HexaToInt(params[pos]) & addr_mask, end_addr = HexaToInt(params[pos+1]) & addr_mask;

	bool rc = false;
	if(UTILITY::check_file_extension(current_dev->debugger->get_file_path(), _T(".hex"))) {
		// save to Intel hex format file
		rc = SaveIntelHexFile(current_dev->debugger->get_file_path(), start_addr, end_addr, addr_mask, type);
	} else if(UTILITY::check_file_extensions(current_dev->debugger->get_file_path(), _T(".s"), _T(".mot"), NULL)) {
		// save to Motorola S-Record format file
		rc = SaveMotorolaSFile(current_dev->debugger->get_file_path(), start_addr, end_addr, addr_mask, type);
	} else {
		// save to binary dump file
		rc = SaveBinaryRawFile(current_dev->debugger->get_file_path(), start_addr, end_addr, addr_mask, type);
	}
	if (rc) {
		Print(_T("Saved to "), false);
	} else {
		PrintError(_T("Can't open "), false);
	}
	Print(current_dev->debugger->get_file_path());
}

/// save to binary dump file
bool DebuggerConsole::SaveBinaryRawFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::WRITE_BINARY);
	if(!rc) return rc;

	for(uint32_t addr = start_addr; addr <= end_addr; addr++) {
		fio.Fputc(current_dev->mem->debug_read_data8(type, addr & addr_mask));
	}

	fio.Fclose();
	return rc;
}

// save to Intel hex format file
bool DebuggerConsole::SaveIntelHexFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::WRITE_ASCII);
	if(!rc) return rc;

	uint32_t addr = start_addr;
	uint32_t segment = 0;
	while(addr <= end_addr) {
		uint32_t len = 0;
		uint32_t sum = 0;
		if (segment != ((start_addr & 0xf0000) >> 4)) {
			// type 02 segment
			len = 2;
			segment = ((start_addr & 0xf0000) >> 4);
			sum = len + 0x02 + ((segment >> 8) & 0xff) + (segment & 0xff);
			fio.Fprintf(":%02X000002%04X%02X\n", len, segment & 0xffff, (0x100 - (sum & 0xff)) & 0xff);
		}
		// type 00 data
		len = MIN(end_addr - addr + 1, 16);
		sum = len + ((addr >> 8) & 0xff) + (addr & 0xff) + 0x00;
		fio.Fprintf(":%02X%04X%02X", len, addr & 0xffff, 0x00);
		for(uint32_t i = 0; i < len; i++) {
			uint8_t data = current_dev->mem->debug_read_data8(type, (addr++) & addr_mask);
			sum += data;
			fio.Fprintf("%02X", data);
		}
		fio.Fprintf("%02X\n", (0x100 - (sum & 0xff)) & 0xff);
	}
	// type 01 eof
	fio.Fprintf(":00000001FF\n");
	fio.Fclose();
	return rc;
}

// save to Motorola S-Record format file
bool DebuggerConsole::SaveMotorolaSFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type)
{
	FILEIO fio;
	bool rc = fio.Fopen(file_path, FILEIO::WRITE_ASCII);
	if(!rc) return rc;

	uint32_t addr = start_addr;
	uint32_t len = 0;
	uint32_t sum = 0;
	uint32_t addr_field = (addr > 0xffff ? 2 : 1);

	while(addr <= end_addr) {
		// type S1 or S2 data
		len = MIN(end_addr - addr + 1, 16) + addr_field + 2;
		sum = len;
		if (addr_field == 2) {
			fio.Fprintf("S2%02X%06X", len, addr & 0xffffff);
		} else {
			fio.Fprintf("S1%02X%04X", len, addr & 0xffff);
		}
		for(uint32_t i = 0; i < addr_field + 1; i++) {
			sum += ((addr >> (i * 8)) & 0xff);
		}
		len -= addr_field + 2;
		for(uint32_t i = 0; i < len; i++) {
			uint8_t data = current_dev->mem->debug_read_data8(type, (addr++) & addr_mask);
			sum += data;
			fio.Fprintf("%02X", data);
		}
		fio.Fprintf("%02X\n", (~sum) & 0xff);
	}
	// type S9 or S8
	len = addr_field + 2;
	sum = len;
	if (addr_field == 2) {
		fio.Fprintf("S8%02X%06X", len, start_addr & 0xffffff);
	} else {
		fio.Fprintf("S9%02X%04X", len, start_addr & 0xffff);
	}
	for(uint32_t i = 0; i < addr_field + 1; i++) {
		sum += ((start_addr >> (i * 8)) & 0xff);
	}
	fio.Fprintf("%02X\n", (~sum) & 0xff);
	fio.Fclose();
	return rc;
}

void DebuggerConsole::UsageBinaryFile(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr3(s
			, _T("DL"), _T("<start address> [<end address>]"), _T("Load from a dump file.")
			, _T("DL"), _T("<offset address>"), _T("Load from a Intel hex or Motorola S format file.")
			, _T("DS"), _T("<start address> <end address>"), _T("Save memory data to a dump file."));
		break;
	case 1:
		UsageCmdStr3(s
			, _T("DPL"), _T("<type> <start address> [<end address>]"), _T("Load from a dump file to memory of specified type.")
			, _T("DPL"), _T("<type> <offset address>"), _T("Load from a Intel hex or Motorola S format file.")
			, _T("DPS"), _T("<type> <start address> <end address>"), _T("Save memory of specified type to a dump file."));
		break;
	}
	if (s) return;

	UsageAddressType(num);
	Print(_T("  <start address> - specify start address."));
	Print(_T("  <end address> - specify end address."));
	Print(_T("  <offset address> - specify offset from start address."));
	Cr();
	Print(_T("  You can set the file name using DN command."));
	UsageBinaryFileType();
	Print(_T("  * When load a Intel hex or Motorola S format file, start address is set from a data in file."));
}

void DebuggerConsole::UsageBinaryFileType()
{
	Print(_T("  File format is decided by file extension:"));
	Print(_T("    \".hex\" - Intel hex format."));
	Print(_T("    \".s\" or \".mot\" - Motorola S format."));
	Print(_T("  Otherwise binary format."));
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSaveImageFile(int num)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageImageFile(false, num);
		return;
	}

	if(paramnum != 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageImageFile(false, num);
		return;
	}

	int type =  DeciToInt(params[1]);
	int w = 0, h = 0;
	int rc = current_dev->mem->get_debug_graphic_memory_size(type, &w, &h);
	if (rc == -2) {
		PrintError(_T("No support on current CPU and architecture."));
		Cr();
		return;
	} else if (rc < 0) {
		PrintError(_T("Invalid type."));
		Cr();
		UsageImageFile(false, num);
		return;
	}

	if (w <= 0 || h <= 0) {
		PrintError(_T("Fatal error. (Invalid size)"));
		Cr();
		return;
	}

#if defined(USE_WIN)
	CSurface imgSuf(w, h * -1);
#elif defined(USE_SDL) || defined(USE_SDL2)
	const CPixelFormat *fmt = dp->emu->get_pixel_format();
	CSurface imgSuf(w, h, *fmt);
#elif defined(USE_WX) || defined(USE_WX2)
	const CPixelFormat *fmt = dp->emu->get_pixel_format();
	CSurface imgSuf(w, h, *fmt);
#elif defined(USE_QT)
	CSurface imgSuf(w, h, QImage::Format_RGB32);
#endif
	if (!imgSuf.IsEnable()) {
		PrintError(_T("Fatal error. (can't create surface)"));
		Cr();
		return;
	}

	if (!current_dev->mem->debug_draw_graphic(type, w, h, (scrntype *)imgSuf.GetBuffer())) {
		PrintError(_T("fatal error. (can't create image)"));
		Cr();
		return;
	}

	if (dp->emu->debugger_save_image(w, h, &imgSuf)) {
		Print(_T("Image file was successfully saved"));
	} else {
		PrintError(_T("Fatal error. (can't save image)"));
	}

	Cr();
}

void DebuggerConsole::UsageImageFile(bool s, int num)
{
	switch(num) {
	case 0:
		UsageCmdStr1(s
			, _T("DIS"), _T("<type>"), _T("Save image file in memory."));
		break;
	}
	if (s) return;

	Print(_T("  <type> - specify type of memory."));
	for(int i=0; ; i++) {
		bool found = current_dev->mem->debug_graphic_type_name(i, buffer, DC_MAX_BUFFER_LEN);
		if (!found) break;
		if (buffer[0] != _T('\0')) {
			Printf(_T("    %d : "), i);
			Out();
		}
	}
	Print(_T("  File name will be named as YYYY-MM-DD-HH-MI-SS.bmp or .png."));
	Print(_T("  If snapshot path is set, image file is put on it."));
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/// set break point
///
/// @param[in] num : BP_FETCH / BP_IORW / BP_INTR / TP_FETCH / TP_IORW / TP_INTR
void DebuggerConsole::CommandSetBreakPoint(int num)
{
	int cat = (num & TRACEPOINT);
	num &= (TRACEPOINT - 1);

	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSetBreakPoint(false, cat | num);
		return;
	}

	BreakPoints *bps = GetBreakPoints(cat | num);

	if(paramnum < 2) {
		ListBreakPoint(bps, cat, num);
		return;
	}

	int rc = SetBreakPoint(bps, cat, num, params, paramnum);
	if (rc) {
		UsageSetBreakPoint(false, cat | num);
	}
}

int DebuggerConsole::SetBreakPoint(BreakPoints *bps, int cat, int num, const _TCHAR * const *ps, int pn)
{
	uint32_t addr = 0;
	uint32_t mask = 0;
	int len = 0;
	int idx = 0;
	const _TCHAR *iname = NULL;
	const _TCHAR *bptrstr = (cat != 0 ? _T("trace") : _T("break"));

	_TCHAR regname[32];
	void *regptr = NULL;
	int reglen = 0;
	uint32_t regval = 0;
	uint32_t regmask = 0;
	bool exp_enable = false;

	switch(num) {
	case BP_BASIC:
		// basic line number
		addr = current_dev->mem->debug_basic_get_line_number_ptr();
		if (ps[1][0] == _T('.')) {
			// set current number
			mask = current_dev->mem->debug_basic_get_line_number();
		} else {
			DeciToNum(ps[1], idx, len);
			mask = (uint32_t)idx;
			if (idx < len) {
				len = len - idx;
			} else {
				len = 0;
			}
		}
		break;
	case BP_EXCEPT:
		// exception
		if (!current_dev->cpu->debug_exception_is_supported()) {
			PrintError(_T("Exception is not supported."));
			Cr();
			return 0;
		}
		if (!current_dev->cpu->get_debug_exception_name_index(ps[1], &addr, NULL, &idx, &iname)) {
			PrintError(_T("Invalid exception name."));
			Cr();
			return 1;
		}
		mask = (uint32_t)-1;
		if (pn == 3) {
			mask = HexaToInt(ps[2]);
		}
		break;
	case BP_INTR:
		// interrupt
		if (!current_dev->cpu->get_debug_signal_name_index(ps[1], &addr, NULL, &idx, &iname)) {
			PrintError(_T("Invalid signal name."));
			Cr();
			return 1;
		}
		mask = BreakPoints::INTR_ON;
		if (pn == 3) {
			if (_tcsicmp(ps[2], _T("on")) == 0) mask = BreakPoints::INTR_ON;
			else if (_tcsicmp(ps[2], _T("off")) == 0) mask = BreakPoints::INTR_OFF;
			else if (_tcsicmp(ps[2], _T("chg")) == 0) mask = BreakPoints::INTR_CHG;
		}
		break;
	case BP_IORD:
	case BP_IOWR:
		// ioport
		if (!current_dev->cpu->debug_ioport_is_supported()) {
			PrintfError(_T("Use {R,W}%cP command instead because CPU takes memory mapped I/O."), (cat != 0 ? _T('T') : _T('B')));
			Cr();
			return 0;
		}
		if (pn >= 3) {
			exp_enable = ParseRWValue(&ps[2], pn - 2, reglen, regval, regmask);
			if (!exp_enable) {
				PrintError(_T("Invalid value."));
				return 0;
			}
		}
		HexaToInt(ps[1], &addr, &len);

		mask = current_dev->cpu->debug_io_addr_mask();
		break;
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
		// memory read / write
		if (pn >= 3) {
			exp_enable = ParseRWValue(&ps[2], pn - 2, reglen, regval, regmask);
			if (!exp_enable) {
				PrintError(_T("Invalid value."));
				return 0;
			}
		}

		HexaToInt(ps[1], &addr, &len);

		if (!(num & BP_FETCH_PH)) {
			addr &= current_dev->prog_addr_mask;
			mask = current_dev->prog_addr_mask;
		} else {
			mask = 0xffffffff;
		}
		break;
	case BP_FETCH:
	case BP_FETCH_PH:
		// fetch address
		if (pn >= 3) {
			// parse expression
			exp_enable = ParseExpression(&ps[2], pn - 2, regname, regptr, reglen, regval);
			if (!exp_enable) {
				PrintError(_T("Invalid expression."));
				return 0;
			}
		}

		if (ps[1][0] == _T('.')) {
			// set current address
//			addr = current_dev->cpu->get_debug_pc(mask);
			addr = current_dev->cpu->get_debug_next_pc(mask);
		} else if (_tcsicmp(ps[1], _T("n")) == 0) {
			// set next address
//			addr = current_dev->cpu->get_debug_branch_pc(mask);
			addr = current_dev->cpu->get_debug_next_pc(mask);
			addr += current_dev->cpu->debug_dasm(-1, addr, NULL, 0, 0);
		} else {
			HexaToInt(ps[1], &addr, &len);
		}

		if (!(num & BP_FETCH_PH)) {
			addr &= current_dev->prog_addr_mask;
			mask = current_dev->prog_addr_mask;
		} else {
			mask = 0xffffffff;
		}
		break;
	default:
		PrintError(_T("Invalid breakpoint type."));
		return 0;
	}

	int add_idx = 0;
	BreakPoint *new_bp = NULL;
	int rc = bps->Add(addr, mask, len, regname, regptr, reglen, regval, regmask, new_bp, &add_idx);
	if(rc >= 0) {
		Printf(_T("%s %spoint%d "), rc ? _T("Update") : _T("Set"), bptrstr, add_idx + 1);
		PrintBreakPointInfo(num, new_bp, add_idx, iname);
		Cr();
		if (new_bp->Status() < 0) {
			Print(_T("But disable now bacause too many points are enable."));
		}
	} else {
		PrintfError(_T("Can't add the %spoint. Clear registering %spoint at first."), bptrstr, bptrstr);
		Cr();
	}
	return 0;
}

void DebuggerConsole::UsageSetBreakPoint(bool s, int num)
{
	switch(num) {
	case BP_FETCH:
	case BP_FETCH_PH:
		UsageCmdStrN(s, _T("BP"), _T("[{<address(es)>,<sign>} [<expression>]]"), _T("Set/Show breakpoint(s).")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("BPP"), _T("[{<physical address(es)>,<sign>} [<expression>]]"), _T("BP using physical address.")
#endif
			, NULL);
		break;
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
		UsageCmdStrN(s, _T("{R,W}BP"), _T("[<address(es)> [<value>]]"), _T("Set/Show breakpoint(s) (memory access).")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("{R,W}BPP"), _T("[<physical address(es)> [<value>]]"), _T("{R,W}BP using physical address.")
#endif
			, NULL);
		break;
	case BP_IORD:
	case BP_IOWR:
		UsageCmdStr1(s, _T("{I,O}BP"), _T("[<port address(es)> [<value>]]"), _T("Set/Show breakpoint(s) (i/o access)."));
		break;
	case BP_INTR:
		UsageCmdStr1(s, _T("NBP"), _T("[<signal name> [<switch>]]"), _T("Set/Show breakpoint(s) (interrupt changed)."));
		break;
	case BP_EXCEPT:
		UsageCmdStr1(s, _T("EBP"), _T("[<exception name> [<vector>]]"), _T("Set/Show breakpoint(s) (exception occured)."));
		break;
	case TP_FETCH:
	case TP_FETCH_PH:
		UsageCmdStrN(s, _T("TP"), _T("[{<address(es)>,<sign>} [<expression>]]"), _T("Set/Show tracepoint(s).")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("TPP"), _T("[{<physical address(es)>,<sign>} [<expression>]]"), _T("TP using physical address.")
#endif
			, NULL);
		break;
	case TP_MEMRD:
	case TP_MEMRD_PH:
	case TP_MEMWR:
	case TP_MEMWR_PH:
		UsageCmdStrN(s, _T("{R,W}TP"), _T("[<address(es)> [<value>]]"), _T("Set/Show tracepoint(s) (memory access).")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("{R,W}TPP"), _T("[<physical address(es)> [<value>]]"), _T("{R,W}TP using physical address.")
#endif
			, NULL);
		break;
	case TP_IORD:
	case TP_IOWR:
		UsageCmdStr1(s, _T("{I,O}TP"), _T("[<port address(es)> [<value>]]"), _T("Set/Show tracepoint(s) (i/o access)."));
		break;
	case TP_INTR:
		UsageCmdStr1(s, _T("NTP"), _T("[<signal name> [<switch>]]"), _T("Set/Show tracepoint(s) (interrupt changed)."));
		break;
	case TP_EXCEPT:
		UsageCmdStr1(s, _T("ETP"), _T("[<exception name> [<vector>]]"), _T("Set/Show tracepoint(s) (exception occured)."));
		break;
	}
	if (s) return;

	switch(num) {
	case BP_FETCH:
	case BP_FETCH_PH:
		Print(_T("  BP : break it when cpu fetches a opcode at specified address."));
#ifdef USE_BREAKPOINT_PHYSICAL
		Print(_T("  BPP: BP using physical address."));
#endif
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <sign> - specify one of following character:"));
		Print(_T("  * \".\"(period)  - set current address."));
		Print(_T("  * \"n\" or \"N\" - set next address."));
		Print(_T("  <expression> - break it when matching with a value in register. ex. A=12"));
		break;
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
		Print(_T("  RBP: break it after cpu read a data at specified address."));
		Print(_T("  WBP: break it after cpu wrote a data at specified address."));
#ifdef USE_BREAKPOINT_PHYSICAL
		Print(_T("  RBPP: RBP using physical address."));
		Print(_T("  WBPP: WBP using physical address."));
#endif
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <value> - break it when matching with read/write value."));
		Print(_T("            you can also set a value and mask using format \"value&mask\"."));
		break;
	case BP_IORD:
	case BP_IOWR:
		Print(_T("  IBP: break it after cpu read a data at specified port."));
		Print(_T("  OBP: break it after cpu wrote a data at specified port."));
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <value> - break it when matching with read/write value."));
		Print(_T("            you can also set a value and mask using format \"value&mask\"."));
		if (!current_dev->cpu->debug_ioport_is_supported()) {
			PrintError(_T("  * This is not supported on current cpu."));
		}
		break;
	case BP_INTR:
		Print(_T("  NBP: break it when cpu acknowledged that an interrupt signal is changed."));
		current_dev->cpu->get_debug_signal_names_str(buffer, DC_MAX_BUFFER_LEN);
		Print(_T("  <signal name> - "), false);
		Out();
		Print(_T("  <switch> - set following word:"));
		Print(_T("  * catch a rising edge if specify \"ON\" or omit this argument."));
		Print(_T("  * catch a fallen edge if specify \"OFF\"."));
		Print(_T("  * catch changing a condition of each devices if specify \"CHG\"."));
		Cr();
		break;
	case BP_EXCEPT:
		Print(_T("  EBP: break it when an exception occured in cpu."));
		Print(_T("  <exception name> - "), false);
		if (current_dev->cpu->debug_exception_is_supported()) {
			current_dev->cpu->get_debug_exception_names_str(buffer, DC_MAX_BUFFER_LEN);
			Out();
		} else {
			PrintError(_T("(no support on current cpu)"));
		}
		Print(_T("  <vector> - set the vector number."));
		break;
	case TP_FETCH:
	case TP_FETCH_PH:
		Print(_T("  TP : start trace when cpu fetches a opcode at specified address."));
#ifdef USE_BREAKPOINT_PHYSICAL
		Print(_T("  TPP: TP using physical address."));
#endif
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <sign> - set following character:"));
		Print(_T("  * set current address if specify \".\"(period)."));
		Print(_T("  * set next address if specify \"n\" or \"N\"."));
		Print(_T("  <expression> - trace when matching with a value in register. ex. A=12"));
		break;
	case TP_MEMRD:
	case TP_MEMRD_PH:
	case TP_MEMWR:
	case TP_MEMWR_PH:
		Print(_T("  RTP: start trace after cpu read a data at specified address."));
		Print(_T("  WTP: start trace after cpu wrote a data at specified address."));
#ifdef USE_BREAKPOINT_PHYSICAL
		Print(_T("  RTPP: RTP using physical address."));
		Print(_T("  WTPP: WTP using physical address."));
#endif
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <value> - trace when matching with read/write value."));
		Print(_T("            you can also set a value and mask using format \"value&mask\"."));
		break;
	case TP_IORD:
	case TP_IOWR:
		Print(_T("  ITP: start trace after cpu read a data at specified port."));
		Print(_T("  OTP: start trace after cpu wrote a data at specified port."));
		Print(_T("  <address(es)> - set one <address> or set range <start address>-<end address>."));
		Print(_T("  <value> - trace when matching with read/write value."));
		Print(_T("            you can also set a value and mask using format \"value&mask\"."));
		if (!current_dev->cpu->debug_ioport_is_supported()) {
			PrintError(_T("  * This is not supported on current cpu."));
		}
		break;
	case TP_INTR:
		Print(_T("  NTP: start trace when cpu acknowledged that an interrupt signal is changed."));
		Print(_T("  <signal name> - "), false);
		current_dev->cpu->get_debug_signal_names_str(buffer, DC_MAX_BUFFER_LEN);
		Out();
		Print(_T("  <switch> - set following word:"));
		Print(_T("  * catch a rising edge if specify \"ON\" or omit this argument."));
		Print(_T("  * catch a fallen edge if specify \"OFF\"."));
		Print(_T("  * catch changing a condition of each devices if specify \"CHG\"."));
		Cr();
		break;
	case TP_EXCEPT:
		Print(_T("  ETP: start trace when an exception occured in cpu."));
		Print(_T("  <exception name> - "), false);
		if (current_dev->cpu->debug_exception_is_supported()) {
			current_dev->cpu->get_debug_exception_names_str(buffer, DC_MAX_BUFFER_LEN);
			Out();
		} else {
			PrintError(_T("(no support on current cpu)"));
		}
		Print(_T("  <vector> - set the vector number."));
		break;
	}
}

bool DebuggerConsole::ParseExpression(const _TCHAR * const *ps, int pn, _TCHAR *regname, void * &regptr, int &reglen, uint32_t &regval)
{
	const _TCHAR *new_ps[4];
	int new_pn;
	_TCHAR addr_str[16];

	if (pn < 1) return false;

	const _TCHAR *msign = _tcsstr(ps[0], _T("="));
	if (msign > ps[0]) {
		UTILITY::tcsncpy(addr_str, 16, ps[0], msign - ps[0]);
		new_ps[0] = UTILITY::lskip(addr_str);
		new_ps[1] = msign + 1;
		new_pn = 2;
	} else if (pn >= 3 && _tcsicmp(ps[1], _T("=")) == 0) {
		new_ps[0] = UTILITY::lskip(ps[0]);
		new_ps[1] = ps[2];
		new_pn = 2;
	} else {
		return false;
	}
	return ParseExpressionMain(new_ps, new_pn, regname, regptr, reglen, regval);
}

bool DebuggerConsole::ParseExpressionMain(const _TCHAR * const *ps, int pn, _TCHAR *regname, void * &regptr, int &reglen, uint32_t &regval)
{
	if (pn < 2) return false;

	// ex: REG 0123
	UTILITY::tcscpy(regname, 8, ps[0]);
	UTILITY::rtrim(regname);

	regval = HexaToInt(ps[1]);

	// get register pointer
	if (!current_dev->cpu->get_debug_reg_ptr(regname, 8, regptr, reglen)) return false;

	// baundary check
	switch(reglen) {
	case 1:
		return (regval <= 0xff);
	case 2:
		return (regval <= 0xffff);
	default:
		return true;
	}

	return true;
}

bool DebuggerConsole::ParseRWValue(const _TCHAR * const *ps, int pn, int &reglen, uint32_t &regval, uint32_t &regmask)
{
	const _TCHAR *new_ps[4];
	int new_pn;
	_TCHAR addr_str[16];

	if (pn < 1) return false;

	const _TCHAR *msign = _tcsstr(ps[0], _T("&"));
	if (msign > ps[0]) {
		UTILITY::tcsncpy(addr_str, 16, ps[0], msign - ps[0]);
		new_ps[0] = addr_str;
		new_ps[1] = msign + 1;
		new_pn = 2;
	} else if (pn >= 3 && _tcsicmp(ps[1], _T("&")) == 0) {
		new_ps[0] = ps[0];
		new_ps[1] = ps[2];
		new_pn = 2;
	} else if (pn == 1) {
		new_ps[0] = ps[0];
		new_ps[1] = ps[0];
		new_pn = 1;
	} else {
		return false;
	}
	return ParseRWValueMain(new_ps, new_pn, reglen, regval, regmask);
}

bool DebuggerConsole::ParseRWValueMain(const _TCHAR * const *ps, int pn, int &reglen, uint32_t &regval, uint32_t &regmask)
{
	if (pn < 1) return false;

	reglen = 2;
	regval = HexaToInt(ps[0]);
	if (pn >= 2) {
		regmask = HexaToInt(ps[1]);
	} else {
		regmask = ~0;
	}
	return true;
}

void DebuggerConsole::PrintBreakPointInfo(int num, const BreakPoint *bp, int i, const _TCHAR *iname)
{
	switch(num) {
	case BP_BASIC:
		// basic
		Printf(_T("%s %u")
			, bp->Len() > 0 ? _T("between") : _T("at")
			, bp->Mask()
		);
		if (bp->Len() > 0) {
			Printf(_T(" to %u")
				, bp->Mask() + bp->Len()
			);
		}
		break;
	case BP_EXCEPT:
	case BP_INTR:
		// interrupt or exception
		Printf(_T(" %s %s"), (bp->Mask() & 2) != 0 ? _T("change condition") : (bp->Mask() & 1) != 0 ? _T("occur") : _T("release"), iname);
		break;
	case BP_IORD:
	case BP_IOWR:
		// io port
		PrintBreakPointIoportInfo(num, bp);
		bp->RegValue(buffer, DC_MAX_BUFFER_LEN);
		Out(false);
		break;
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
		// memory
		PrintBreakPointAddressInfo(num, bp);
		bp->RegValue(buffer, DC_MAX_BUFFER_LEN);
		Out(false);
		break;
	case BP_FETCH:
	case BP_FETCH_PH:
		// fetch address
		PrintBreakPointAddressInfo(num, bp);
		bp->Expression(buffer, DC_MAX_BUFFER_LEN);
		Out(false);
		break;
	}
}

void DebuggerConsole::PrintBreakPointAddressInfo(int num, const BreakPoint *bp)
{
	_TCHAR fmt[256];
#ifdef USE_BREAKPOINT_PHYSICAL
	bool is_physical = ((num & BP_FETCH_PH) != 0);
#endif
	UTILITY::stprintf(fmt, sizeof(fmt) / sizeof(fmt[0]), _T("%s %saddress %s%s%s")
		, bp->Len() > 0 ? _T("between") : _T("at")
#ifdef USE_BREAKPOINT_PHYSICAL
		, is_physical ? _T("physical ") : _T("logical ")
		, is_physical ? _T(PHYS_ADDR_FORMAT) : _T(ADDR_FORMAT)
#else
		, _T("")
		, _T(ADDR_FORMAT)
#endif
		, bp->Len() > 0 ? _T(" and ") : _T("")
#ifdef USE_BREAKPOINT_PHYSICAL
		, bp->Len() > 0 ? (is_physical ? _T(PHYS_ADDR_FORMAT) : _T(ADDR_FORMAT)) : _T("")
#else
		, bp->Len() > 0 ? _T(ADDR_FORMAT) : _T("")
#endif
	);
	if (bp->Len() > 0) {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, fmt, bp->Addr(), bp->Addr() + bp->Len());
	} else {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, fmt, bp->Addr());
	}
}

void DebuggerConsole::PrintBreakPointIoportInfo(int num, const BreakPoint *bp)
{
	_TCHAR fmt[256];

	UTILITY::stprintf(fmt, sizeof(fmt) / sizeof(fmt[0]), _T("%s port %s%s%s")
		, bp->Len() > 0 ? _T("between") : _T("at")
		, _T(IOADDR_FORMAT)
		, bp->Len() > 0 ? _T(" and ") : _T("")
		, bp->Len() > 0 ? _T(IOADDR_FORMAT) : _T("")
	);
	if (bp->Len() > 0) {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, fmt, bp->Addr(), bp->Addr() + bp->Len());
	} else {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, fmt, bp->Addr());
	}
}

// ---------------------------------------------------------------------------

/// @param[in] num : 0:addr 1:ioport 2:interrupt / 8:trace point
void DebuggerConsole::CommandClearBreakPoint(int num)
{
	int cat = (num & TRACEPOINT);
	num &= (TRACEPOINT - 1);

	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageChangeBreakPoint(false, cat | num);
		return;
	}

	BreakPoints *bps = GetBreakPoints(cat | num);

	if(paramnum < 2) {
		ListBreakPoint(bps, cat, num);
		return;
	}

	if (ClearBreakPoint(bps, cat, num, paramnum, params)) {
		PrintError(_T("Invalid number exists in parameter(s)."));
		Cr();
		UsageChangeBreakPoint(false, cat | num);
	}
}

int DebuggerConsole::ClearBreakPoint(BreakPoints *bps, int cat, int num, int pn, _TCHAR **ps)
{
	const _TCHAR *bptrstr = (cat != 0 ? _T("trace") : _T("break"));

	if(pn == 2 && (_tcsicmp(ps[1], _T("*")) == 0 || _tcsicmp(ps[1], _T("ALL")) == 0)) {
		bps->DeleteAll();
		Printf(_T("Cleared all %sepoint(s)."), bptrstr);
		Cr();
	} else if(pn >= 2) {
		for(int i = 1; i < pn; i++) {
			int index = DeciToInt(ps[i]);
			if (index <= 0) {
				PrintError(_T("Invalid number."));
				return 0;
			}
			bool rc = bps->Delete(index - 1);
			if(!rc) {
				PrintfError(_T("Can't clear at %d."), index);
				Cr();
			} else {
				Printf(_T("Cleared %spoint%d."), bptrstr, index);
				Cr();
			}
		}
	} else {
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------

/// @param[in] num : 0:addr 1:ioport 2:interrupt / 8:trace point
void DebuggerConsole::CommandChangeBreakPoint(int num)
{
	int cat = (num & TRACEPOINT);
	num &= (TRACEPOINT - 1);

	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageChangeBreakPoint(false, cat | num);
		return;
	}

	BreakPoints *bps = GetBreakPoints(cat | num);

	if(paramnum < 2) {
		ListBreakPoint(bps, cat, num);
		return;
	}

	if (ChangeBreakPoint(bps, cat, num, paramnum, params)) {
		PrintError(_T("Invalid number exists in parameter(s)."));
		Cr();
		UsageChangeBreakPoint(false, cat | num);
	}
}

int DebuggerConsole::ChangeBreakPoint(BreakPoints *bps, int cat, int num, int pn, _TCHAR **ps)
{
	const _TCHAR *bptrstr = (cat != 0 ? _T("trace") : _T("break"));

	bool enabled = (ps[0][1] == _T('E') || ps[0][1] == _T('e') || ps[0][2] == _T('E') || ps[0][2] == _T('e') || ps[0][3] == _T('E') || ps[0][3] == _T('e'));
	const _TCHAR *enastr = (enabled ? _T("enable") : _T("disable"));

	if(pn == 2 && (_tcsicmp(ps[1], _T("*")) == 0 || _tcsicmp(ps[1], _T("ALL")) == 0)) {
		if (enabled) {
			if (bps->EnableAll()) {
				Printf(_T("Enabled some %spoint(s)."), bptrstr);
			} else {
				PrintfError(_T("Can't enable %spoint(s)."), bptrstr);
			}
		} else {
			bps->DisableAll();
			Printf(_T("Disabled all %spoint(s)."), bptrstr);
		}
		Cr();
	} else if(pn >= 2) {
		for(int i = 1; i < pn; i++) {
			int index = DeciToInt(ps[i]);
			if (index <= 0) {
				PrintError(_T("Invalid number."));
				return 0;
			}
			int rc = (enabled ? bps->Enable(index - 1) : bps->Disable(index - 1));
			if(rc == BreakPoints::ERR_OUT_OF_RANGE) {
				PrintfError(_T("%d is out of range."), index);
				Cr();
			} else if (rc == BreakPoints::ERR_TOO_MANY_ITEMS) {
				PrintfError(_T("Can't %s at %spoint%d because too many items are %s."), enastr, bptrstr, index, enastr);
				Cr();
			} else if (rc == BreakPoints::ERR_ALREADY_EXISTS) {
				Printf(_T("Already %s at %spoint%d."), enastr, bptrstr, index);
				Cr();
			} else if (rc < 0) {
				PrintfError(_T("Can't %s at %spoint%d."), enastr, bptrstr, index);
				Cr();
			} else {
				Printf(_T("%sd %spoint%d."), enastr, bptrstr, index);
				Cr();
			}
		}
	} else {
		return 1;
	}
	return 0;
}

void DebuggerConsole::UsageChangeBreakPoint(bool s, int num)
{
	switch(num) {
	case BP_FETCH:
	case BP_FETCH_PH:
		UsageCmdStrN(s, _T("B{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by BP.")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("BP{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by BPP.")
#endif
			, NULL);
		break;
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
		UsageCmdStrN(s, _T("{R,W}B{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by {R,W}BP.")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("{R,W}BP{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by {R,W}BPP.")
#endif
			, NULL);
		break;
	case BP_IORD:
	case BP_IOWR:
		UsageCmdStr1(s, _T("{I,O}B{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by {I,O}BP."));
		break;
	case BP_INTR:
		UsageCmdStr1(s, _T("NB{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by NBP."));
		break;
	case BP_EXCEPT:
		UsageCmdStr1(s, _T("EB{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) which is set by EBP."));
		break;
	case TP_FETCH:
	case TP_FETCH_PH:
		UsageCmdStrN(s, _T("T{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by TP.")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("TP{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by TPP.")
#endif
			, NULL);
		break;
	case TP_MEMRD:
	case TP_MEMRD_PH:
	case TP_MEMWR:
	case TP_MEMWR_PH:
		UsageCmdStrN(s, _T("{R,W}T{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by {R,W}TP.")
#ifdef USE_BREAKPOINT_PHYSICAL
			, _T("{R,W}TP{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by {R,W}TPP.")
#endif
			, NULL);
		break;
	case TP_IORD:
	case TP_IOWR:
		UsageCmdStr1(s, _T("{I,O}T{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by {I,O}TP."));
		break;
	case TP_INTR:
		UsageCmdStr1(s, _T("NT{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by NTP."));
		break;
	case TP_EXCEPT:
		UsageCmdStr1(s, _T("ET{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) which is set by ETP."));
		break;
	}
	if (s) return;

	switch(num) {
	case BP_FETCH:
	case BP_FETCH_PH:
	case BP_MEMRD:
	case BP_MEMRD_PH:
	case BP_MEMWR:
	case BP_MEMWR_PH:
	case BP_IORD:
	case BP_IOWR:
	case BP_INTR:
	case BP_EXCEPT:
		Print(_T("  <list> - specify number(s) by decimal."));
		Print(_T("  * or ALL - perform it about all breakpoints."));
		break;
	case TP_FETCH:
	case TP_FETCH_PH:
	case TP_MEMRD:
	case TP_MEMRD_PH:
	case TP_MEMWR:
	case TP_MEMWR_PH:
	case TP_IORD:
	case TP_IOWR:
	case TP_INTR:
	case TP_EXCEPT:
		Print(_T("  <list> - specify number(s) by decimal."));
		Print(_T("  * or ALL - perform it about all tracepoints."));
		break;
	}
}

// ---------------------------------------------------------------------------

/// @param[in] num: type number of break point
/// @param[in] all_list: show all usage about break point
void DebuggerConsole::CommandListBreakPoint(int num, bool all_list)
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageListBreakPoint(false, num);
		return;
	}

	if(paramnum != 1) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageListBreakPoint(false, num);
		return;
	}

	if (all_list) {
		const struct st_map {
			enBpTpType num;
			_TCHAR pre;
			_TCHAR post;
		} map[] = {
			{ BP_FETCH,	_T('\0'),	_T('\0') },
#ifdef USE_BREAKPOINT_PHYSICAL
			{ BP_FETCH_PH,_T('\0'),	_T('P') },
#endif
			{ BP_MEMRD,	_T('R'),	_T('\0') },
#ifdef USE_BREAKPOINT_PHYSICAL
			{ BP_MEMRD_PH,_T('R'),	_T('P') },
#endif
			{ BP_MEMWR,	_T('W'),	_T('\0') },
#ifdef USE_BREAKPOINT_PHYSICAL
			{ BP_MEMWR_PH,_T('W'),	_T('P') },
#endif
#ifdef USE_IOPORT_ACCESS
			{ BP_IORD,	_T('I'),	_T('\0') },
			{ BP_IOWR,	_T('O'),	_T('\0') },
#endif
			{ BP_INTR,	_T('N'),	_T('\0') },
#ifdef USE_EXCEPTION_BREAKPOINT
			{ BP_EXCEPT,_T('E'),	_T('\0') },
#endif
			{ BP_UNKNOWN, _T('\0'),	_T('\0') }
		};
		for(int i=0; map[i].num != BP_UNKNOWN; i++) {
			_TCHAR name[8];
			int namelen = 0;
			if (map[i].pre)  name[namelen++] = map[i].pre;
			name[namelen++] = (num & TRACEPOINT ? _T('T') : _T('B'));
			name[namelen++] = _T('P');
			if (map[i].post) name[namelen++] = map[i].post;
			name[namelen] = _T('\0');
			Print(name, false);
			Print(_T(":"));
			BreakPoints *bps = GetBreakPoints((num & TRACEPOINT) | map[i].num);
			ListBreakPoint(bps, num & TRACEPOINT, map[i].num);
		}
	} else {
		BreakPoints *bps = GetBreakPoints(num);
		ListBreakPoint(bps, num & TRACEPOINT, num & (TRACEPOINT - 1));
	}
}

int DebuggerConsole::ListBreakPoint(BreakPoints *bps, int cat, int num)
{
	const _TCHAR *bptrstr = (cat != 0 ? _T("trace") : _T("break"));

	bool support = true;
	switch(num) {
	case BP_IORD:
	case BP_IOWR:
		support = current_dev->cpu->debug_ioport_is_supported();
		break;	
	case BP_EXCEPT:
		support = current_dev->cpu->debug_exception_is_supported();
		break;	
	}
	if (!support) {
		PrintError(_T("This is not supported on current cpu."));
		return 0;
	}

	bool found = false;
	for(int i = 0; ; i++) {
		const BreakPoint *bp = bps->TableItem(i);
		if (!bp) break;

		if(bp->Status()) {
			found = true;
			const _TCHAR *iname = NULL;
			if (num == BP_INTR) {
				current_dev->cpu->get_debug_signal_name_index(bp->Addr(), NULL, NULL, &iname);
			} else if (num == BP_EXCEPT) {
				current_dev->cpu->get_debug_exception_name_index(bp->Addr(), NULL, NULL, &iname);
			}
			Printf(_T("#%d: %s"), i + 1, bp->Status() == 1 ? _T("enable ") : _T("disable "));
			PrintBreakPointInfo(num, bp, i, iname);
			Cr();

		}
	}
	if (!found) {
		Printf(_T("No %spoint exists."), bptrstr);
		Cr();
	}
	return 0;
}

void DebuggerConsole::UsageListBreakPoint(bool s, int num)
{
	if (num & TRACEPOINT) {
		UsageCmdStr1(s, _T("TL"), NULL, _T("List all tracepoints of any type."));
	} else {
		UsageCmdStr1(s, _T("BL"), NULL, _T("List all breakpoints of any type."));
	}
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSetSymbol()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSetSymbol(false);
		return;
	}

	if(paramnum != 1 && paramnum != 3) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageSetSymbol(false);
		return;
	}

//	int rc;
	if (paramnum == 3) {
		SetSymbol(HexaToInt(params[1]), params[2]);
	} else {
		ListSymbol();
	}
}

void DebuggerConsole::UsageSetSymbol(bool s)
{
	UsageCmdStr1(s, _T("SL"), _T("[<address> <label>]"), _T("Show/Set symbol label."));
	if (s) return;

	Print(_T("  <address> - specify an address."));
	Print(_T("  <label> - specify a label string."));
}

int DebuggerConsole::SetSymbol(uint32_t addr, const _TCHAR *label)
{
	current_dev->debugger->add_symbol(addr, label);
	Printf(_T("Set symbol: %04X \""), addr);
	Print(label, false);
	Print(_T("\""));
	return 0;
}

int DebuggerConsole::ListSymbol()
{
	int i=0;
	for(; ; i++) {
		const Symbol *symbol = current_dev->debugger->get_symbol(i);
		if (!symbol) break;
		Printf(_T("%04X: "), symbol->Addr());
		Print(symbol->Name());
	}
	if (i == 0) {
		Print(_T("No symbol exists."));
	}
	return i;
}

void DebuggerConsole::CommandClearSymbol()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageClearSymbol(false);
		return;
	}

	if(paramnum < 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageClearSymbol(false);
		return;
	}

	ClearSymbol(paramnum, params);
}

void DebuggerConsole::UsageClearSymbol(bool s)
{
	UsageCmdStr1(s, _T("SC"), _T("{*,ALL,<address>}"), _T("Clear symbol label."));

	if (s) return;

	Print(_T("  <address> - specify address."));
	Print(_T("  * or ALL - perform it about all symbols."));
}

int DebuggerConsole::ClearSymbol(int pn, _TCHAR **ps)
{
	if(pn == 2 && (_tcsicmp(ps[1], _T("*")) == 0 || _tcsicmp(ps[1], _T("ALL")) == 0)) {
		current_dev->debugger->release_symbols();
		Print(_T("Clear all symbols."));
	} else if(pn >= 2) {
		for(int i = 1; i < pn; i++) {
			uint32_t addr = HexaToInt(ps[i]);
			const Symbol *symbol = current_dev->debugger->release_symbol(addr);
			if(symbol) {
				Printf(_T("Clear symbol: %04X \""), symbol->Addr());
				Print(symbol->Name(), false);
				Print(_T("\""));
			} else {
				Printf(_T("Not exist symbol: %04X"), addr);
				Cr();
			}
		}
	} else {
		return 1;
	}
	return 0;
}

void DebuggerConsole::CommandLoadSymbol()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageLoadSymbol(false);
		return;
	}

	if(paramnum != 2 && paramnum != 3) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageLoadSymbol(false);
		return;
	}
	int format = 0;
	if (paramnum == 3) {
		if (_tcsicmp(params[2], _T("R")) == 0) format = 1;
		else if (_tcsicmp(params[2], _T("TSC")) == 0) format = 2;
	}

	LoadSymbol(params[1], format);
}

void DebuggerConsole::UsageLoadSymbol(bool s)
{
	UsageCmdStr1(s, _T("SLL"), _T("<file path> [R|TSC]"), _T("Load symbol label from a file."));

	if (s) return;

	Print(_T("  <file path> - specify a file."));
	Print(_T("  R           - read a file that data is reversed addr and label."));
	Print(_T("  TSC         - read a file that data is in order of label, type and addr."));
	Print(_T("    File format is like following per line by default."));
	Print(_T("      <addr> <label> <addr> <label> ..."));
	Print(_T("    When set R option:"));
	Print(_T("      <label> <addr> <label> <addr> ..."));
	Print(_T("    When set TSC option:"));
	Print(_T("      <label> <type> <addr> <label> <type> <addr> ..."));
	Print(_T("      (<type> is ignored.)"));
	Print(_T("    * You can use space, tab, comma and semicolon as delimiter."));
}

int DebuggerConsole::LoadSymbol(const _TCHAR *file_path, int format)
{
	const _TCHAR *delimiter = _T("\t #$*,;");

	FILEIO fio;
	if(fio.Fopen(file_path, FILEIO::READ_ASCII)) {
		current_dev->debugger->release_symbols();
		_TCHAR line[1024];
		while(fio.Fgets(line, sizeof(line) / sizeof(line[0])) != NULL) {
			_TCHAR *next = NULL;
			_TCHAR *token1 = UTILITY::tcstok(line, delimiter, &next);
			while(token1 != NULL) {
				if(_tcslen(token1) > 0) {
					_TCHAR *token2 = UTILITY::tcstok(NULL, delimiter, &next);
					while(token2 != NULL) {
						if (format == 2) {
							_TCHAR *token3 = UTILITY::tcstok(NULL, delimiter, &next);
							while(token3 != NULL) {
								UTILITY::rtrim(token3, _T("\r\n"));
								if(_tcslen(token3) > 0) {
									const _TCHAR *addr = token3;
									const _TCHAR *name = token1;
									current_dev->debugger->add_symbol(HexaToInt(addr), name);
									token1 = token2 = NULL;
									break;
								}
								token3 = UTILITY::tcstok(NULL, delimiter, &next);
							}
							token2 = NULL;
						} else {
							UTILITY::rtrim(token2, _T("\r\n"));
							if(_tcslen(token2) > 0) {
								const _TCHAR *addr = (format == 1 ? token2 : token1);
								const _TCHAR *name = (format == 1 ? token1 : token2);
								current_dev->debugger->add_symbol(HexaToInt(addr), name);
								token1 = NULL;
								break;
							}
							token2 = UTILITY::tcstok(NULL, delimiter, &next);
						}
					}
				}
				token1 = UTILITY::tcstok(NULL, delimiter, &next);
			}
		}
		fio.Fclose();
	} else {
		PrintError(_T("Can't open "), false);
		Print(file_path);
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandExecute()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageExecute(false);
		return;
	}

	if(paramnum > 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageExecute(false);
		return;
	}

	if (NowDisable()) {
		Print(Yellow, _T("Can't execute now because power is off."));
		SetTextColor(White);
		return;
	}

	bool stored = false;
	if (paramnum >= 2) {
		if (_tcsicmp(params[1], _T("N")) == 0) {
			// step over
			uint32_t naddr = current_dev->cpu->get_debug_next_pc(0);
			naddr += current_dev->cpu->debug_dasm(-1, naddr, NULL, 0, 0);
			current_dev->debugger->store_break_points(
				naddr & current_dev->prog_addr_mask,
				0,
				0);
			stored = true;
		} else {
			current_dev->debugger->store_break_points(
				HexaToInt(params[1]) & current_dev->prog_addr_mask,
				0,
				0);
			stored = true;
		}
	}

	SetKeyStatus();
	storage->Running(true);

	uint32_t breaked = 0;
	while (!dp->request_terminate && !breaked && !NowBreakPointInDebugger()) {
		ClearSuspendInDebugger(-1);
		while(!dp->request_terminate && !current_dev->debugger->now_suspend()) {
			if (EscapePressed()) {
				breaked |= 1;
				break;
			} else if (NowDisable()) {
				breaked |= 2;
				break;
//			} else if (NowPausing()) {
//				breaked |= 4;
//				break;
			}
			CDelay(10);
		}
		// break cpu

		if (!breaked && NowPausing()) continue;

		// switch to cpu on which hitting break point
		SwitchCPU(GetCPUIndexHitBreakOrTracePoint());

		current_dev->debugger->now_going(0);
		while(!dp->request_terminate && !current_dev->debugger->now_suspend() && !NowDisable() && !NowPausing()) {
			CDelay(10);
		}
		current_dev->dasm_addr = current_dev->cpu->get_next_pc();

		TraceCurrent();

		if (current_dev->show_regs && !current_dev->debugger->now_basicreason()) {
			current_dev->cpu->debug_regs_info(buffer, DC_MAX_BUFFER_LEN);
			Out(White);
		}

		TraceNext();

		current_dev->debugger->reset_trace_point();

		BasicTraceCurrent();
	}

	storage->Running(false);
	PrintBreakedReason(breaked);

	SetTextColor(White);

	if(stored) {
		current_dev->debugger->restore_break_points();
	}
}

void DebuggerConsole::UsageExecute(bool s)
{
	UsageCmdStr1(s, _T("G"), _T("[{<address>,<sign>}]"), _T("Continue processing."));
	if (s) return;

	Print(_T("  <address> - specify address to break it."));
	Print(_T("  <sign> - specify following character:"));
	Print(_T("  * \"n\" or \"N\" - set next address (step over)."));
	Print(_T("  If address or sign is specified, add its address to the breakpoint list and disable any other breakpoints."));
	Print(_T("  Break it if esc key was pressed."));
}

void DebuggerConsole::PrintBreakedReason(uint32_t reason)
{
	BreakPoints::en_break_point_type hit = current_dev->debugger->hit_break_point();
	const _TCHAR *iname = NULL;
	uint32_t mask = 0;
	if(hit != BreakPoints::BP_END) {
		const BreakPoints *bps = current_dev->debugger->get_bps(hit);
//		int idx = bps->HitIndex();
		const BreakPoint *bp = bps->Hit();
		SetTextColor(Red);
		switch(hit) {
		case BreakPoints::BP_FETCH_OP:
		case BreakPoints::BP_FETCH_OP_PH:
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName());
			Printf(buffer, current_dev->cpu->get_next_pc());
			break;
		case BreakPoints::BP_READ_MEMORY:
		case BreakPoints::BP_READ_MEMORY_PH:
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : memory %s was read"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), _T(ADDR_FORMAT));
			Printf(buffer, current_dev->cpu->get_next_pc(), bp->Addr());
			PrintBreakedDevice();
			break;
		case BreakPoints::BP_WRITE_MEMORY:
		case BreakPoints::BP_WRITE_MEMORY_PH:
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : memory %s was written"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), _T(ADDR_FORMAT));
			Printf(buffer, current_dev->cpu->get_next_pc(), bp->Addr());
			PrintBreakedDevice();
			break;
		case BreakPoints::BP_INPUT_IO:
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : port %s was read"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), _T(ADDR_FORMAT));
			Printf(buffer, current_dev->cpu->get_next_pc(),bp->Addr());
			PrintBreakedDevice();
			break;
		case BreakPoints::BP_OUTPUT_IO:
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : port %s was written"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), _T(IOADDR_FORMAT));
			Printf(buffer, current_dev->cpu->get_next_pc(), bp->Addr());
			PrintBreakedDevice();
			break;
		case BreakPoints::BP_INTERRUPT:
			current_dev->cpu->get_debug_signal_name_index(bp->Addr(), &mask, NULL, &iname);
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : %s signal %s"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), iname, (mask & 1) ? _T("occured") : _T("released"));
			Printf(buffer, current_dev->cpu->get_next_pc());
			break;
		case BreakPoints::BP_EXCEPTION:
			current_dev->cpu->get_debug_exception_name_index(bp->Addr(), &mask, NULL, &iname);
			UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s : %s exception occured"),
				_T(ADDR_FORMAT), storage->GetCurrentCPUName(), iname);
			Printf(buffer, current_dev->cpu->get_next_pc());
			if (mask) {
				// vector number
				Printf(_T(" and read vector %02X"), mask);
			}
			break;
		case BreakPoints::BP_BASIC_NUMBER:
			Printf(_T("Breaked at line number %u on BASIC"), bp->Mask());
			break;
		default:
			break;
		}
		Cr();
		current_dev->debugger->reset_break_point();
	} else if (reason & 1) {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s: esc key was pressed"),
			_T(ADDR_FORMAT), storage->GetCurrentCPUName());
		Printf(Red, buffer, current_dev->cpu->get_next_pc());
		Cr();
	} else if (reason & 2) {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s: can't execute"),
			_T(ADDR_FORMAT), storage->GetCurrentCPUName());
		Printf(Red, buffer, current_dev->cpu->get_next_pc());
		Cr();
	} else if (reason & 4) {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s on %s: now pausing"),
			_T(ADDR_FORMAT), storage->GetCurrentCPUName());
		Printf(Red, buffer, current_dev->cpu->get_next_pc());
		Cr();
	}
}

void DebuggerConsole::PrintBreakedDevice()
{
	DEBUGGER_BUS_BASE *dbg = current_dev->debugger->get_detected();
	if (dbg && !dbg->rerated_on_cpu()) {
		UTILITY::tcscpy(buffer, DC_MAX_BUFFER_LEN, _T(" in "));
		UTILITY::tcscat(buffer, DC_MAX_BUFFER_LEN, current_dev->debugger->get_detected()->get_name());
		Print(buffer, false);
	} else {
		UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T(" at %s"), _T(ADDR_FORMAT));
		Printf(buffer, current_dev->cpu->get_pc());
	}
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandTrace()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageTrace(false);
		return;
	}

	if(paramnum > 3) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageTrace(false);
		return;
	}

	if (NowDisable()) {
		Print(Yellow, _T("Can't trace now because power is off."));
		SetTextColor(White);
		return;
	}

	int skips = 0;
	int steps = 1;
	if(paramnum >= 3) {
		skips = HexaToInt(params[2]);
	}
	if(paramnum >= 2) {
		steps = HexaToInt(params[1]);
	}

	current_dev->debugger->now_going(skips);

	SetKeyStatus();

	uint32_t breaked = 0;
	int step = 0;
	while (!dp->request_terminate && !breaked && !NowBreakPointInDebugger() && step < steps) {
		while(step < steps) {
			ClearSuspendInDebugger(-1);
			current_dev->debugger->now_going(skips);

			while(!dp->request_terminate && !current_dev->debugger->now_suspend() && !NowDisable() && !NowPausing()) {
				CDelay(10);
			}

			// switch to cpu on which hitting breakpoint / tracepoint
			SwitchCPU(GetCPUIndexHitBreakOrTracePoint());

			current_dev->dasm_addr = current_dev->cpu->get_next_pc();

			TraceCurrent();

			if (current_dev->show_regs && !current_dev->debugger->now_basicreason()) {
				current_dev->cpu->debug_regs_info(buffer, DC_MAX_BUFFER_LEN);
				Out(White);
			}

			if(current_dev->debugger->now_breaktracepoint()
			|| (dp->request_terminate)) {
				break;
			} else if (EscapePressed()) {
				breaked |= 1;
				break;
			} else if (NowDisable()) {
				breaked |= 2;
				break;
//			} else if (NowPausing()) {
//				breaked |= 4;
//				break;
			}

			step++;
		}

		TraceNext();

		current_dev->debugger->reset_trace_point();
	}

	PrintBreakedReason(breaked);

	SetTextColor(White);
}

void DebuggerConsole::UsageTrace(bool s)
{
	UsageCmdStr1(s, _T("T"), _T("[<count> [<skip count>]]"), _T("Trace memory while step processing."));
	if (s) return;

	Print(_T("  <count> - specify number of step."));
	Print(_T("  <skip count> - specify number of step to skip trace."));
	Print(_T("  Total steps become <count> * <skip count>."));
	Print(_T("  Stop tracing if esc key was pressed."));
}

// ---------------------------------------------------------------------------

void DebuggerConsole::TraceFirst()
{
	if (current_dev->show_regs) {
		current_dev->cpu->debug_regs_info(buffer, DC_MAX_BUFFER_LEN);
		Out(White);
	}
	TraceNext();

	UTILITY::stprintf(buffer, DC_MAX_BUFFER_LEN, _T("Breaked at %s"), _T(ADDR_FORMAT));
	Printf(Red, buffer, current_dev->cpu->get_next_pc());
	Cr();

	SetTextColor(White);
}

void DebuggerConsole::TraceCurrent()
{
	if (!current_dev->debugger->now_basicreason()) {
		uint32_t addr = current_dev->cpu->get_pc();
		if (current_dev->cpu->debug_dasm_label(-1, addr, buffer, DC_MAX_BUFFER_LEN)) {
			Out(Cyan);
		}
		current_dev->cpu->debug_dasm(-1, addr, buffer, DC_MAX_BUFFER_LEN, 1);
#ifndef USE_EMU_INHERENT_SPEC
		Printf(Cyan, _T("Done\t%08X  "), addr);
#else
		Print(Cyan, _T("  "), false);
#endif
		Out();
	}
}

void DebuggerConsole::TraceNext()
{
	if (!current_dev->debugger->now_basicreason()) {
		current_dev->cpu->debug_dasm(-1, current_dev->cpu->get_next_pc(), buffer, DC_MAX_BUFFER_LEN, 0);
#ifndef USE_EMU_INHERENT_SPEC
		Printf(Cyan, _T("Next\t%08X  "), cpu->get_next_pc());
#else
		Print(Cyan, _T("> "), false);
#endif
		Out();
	}
}

void DebuggerConsole::SetKeyStatus()
{
	uint8_t *k = dp->emu->key_buffer();
	memcpy(k, key_stat_tmp, KEY_STATUS_SIZE);
	memset(key_stat_tmp, 0, KEY_STATUS_SIZE);
}

int DebuggerConsole::GetCPUIndexEnabled() const
{
	int idx = -1;
	const struct devs_st *dev = &devs[0];
	while(dev) {
		if (!dev->cpu->enable()) {
			idx = (dev->index - 1);
			break;
		}
		dev = dev->next;
	}
	return idx;
}

int DebuggerConsole::GetCPUIndexHitBreakPoint() const
{
	int idx = -1;
	const struct devs_st *dev = &devs[0];
	while(dev) {
		if (dev->debugger->now_breakpoint()) {
			idx = dev->index;
			break;
		} else if (!dev->cpu->enable()) {
			idx = (dev->index - 1);
			break;
		}
		dev = dev->next;
	}
	return idx;
}

int DebuggerConsole::GetCPUIndexHitBreakOrTracePoint() const
{
	int idx = -1;
	const struct devs_st *dev = &devs[0];
	while(dev) {
		if (dev->debugger->now_breaktracepoint()) {
			idx = dev->index;
			break;
		} else if (!dev->cpu->enable()) {
			idx = (dev->index - 1);
			break;
		}
		dev = dev->next;
	}
	return idx;
}

bool DebuggerConsole::NowBreakPointInDebugger() const
{
	bool hit = false;
	const struct devs_st *dev = &devs[0];
	while(dev) {
		if (dev->debugger->now_breakpoint()) {
			hit = true;
			break;
		}
		dev = dev->next;
	}
	return hit;
}

void DebuggerConsole::ClearSuspendInDebugger(int step)
{
	struct devs_st *dev = &devs[0];
	while(dev) {
		dev->debugger->now_going(step);
		dev->debugger->clear_suspend();
		dev = dev->next;
	}
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandTraceBack()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageTraceBack(false);
		return;
	}

	if(paramnum > 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageTraceBack(false);
		return;
	}

	int steps = 25;
	if(paramnum >= 2) {
		steps = HexaToInt(params[1]);
		if (steps <= 0) steps = 1;
	}
	steps--;

	for(int i=steps; i>=0; ) {
		int ii;
		if ((ii = current_dev->cpu->debug_trace_back(i, buffer, DC_MAX_BUFFER_LEN)) >= -1) {
			Print(Yellow, _T("  "), false);
			Out();
		}
		if (current_dev->show_regs && current_dev->cpu->debug_trace_back_regs(i, buffer, DC_MAX_BUFFER_LEN) >= -1) {
			Out(White);
		}
		i = ii;
	}
	SetTextColor(White);
}

void DebuggerConsole::UsageTraceBack(bool s)
{
	UsageCmdStr1(s, _T("TB"), _T("[<count>]"), _T("Trace back the recently processed instructions."));
	if (s) return;

	Print(_T("  <count> - specify number of traceing."));
}

// ---------------------------------------------------------------------------

#ifdef USE_EMU_INHERENT_SPEC
/// show memory map
///
void DebuggerConsole::CommandShowMemoryMap()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageShowMemoryMap(false);
		return;
	}

	if(paramnum != 1) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageShowMemoryMap(false);
		return;
	}

	current_dev->mem->debug_memory_map_info(this);
}

void DebuggerConsole::UsageShowMemoryMap(bool s)
{
	UsageCmdStr1(s, _T("M"), NULL, _T("Show memory map."));
	if (s) return;
}
#endif

// ---------------------------------------------------------------------------

#ifdef _MBS1
/// show address map
///
void DebuggerConsole::CommandShowAddressMap()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageShowAddressMap(false);
		return;
	}

	if(paramnum > 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageShowAddressMap(false);
		return;
	}

	int idx = 0x100;
	if (paramnum == 2) {
		if (_tcsicmp(params[1], _T("ALL")) == 0) {
			idx = 0xff;
		} else {
			idx = (int)HexaToInt(params[1]);
		}
	}
	int rc = current_dev->mem->debug_address_map_info(this, idx);
	if (rc < 0) {
		PrintError(_T("No support on current CPU and architecture."));
	}
}

void DebuggerConsole::UsageShowAddressMap(bool s)
{
	UsageCmdStr1(s, _T("X"), _T("[<segment number> | ALL]"), _T("Show address map."));
	if (s) return;

	Print(_T(" X command shows values in $FE00 - $FE0F on System I/O."));
	Print(_T("  <segment number> - specify number of address segment."));
	Print(_T("                     If no set number, show current segment."));
	Print(_T("  ALL              - show all addresses."));
}

/// edit address map
///
void DebuggerConsole::CommandEditAddressMap()
{
	if(paramnum < 3 || paramnum > 18 || *params[1] == _T('?')) {
		Cr();
		UsageEditAddressMap(false);
		return;
	}

	int idx = (int)HexaToInt(params[1]);
	int vals[16];
	int rc = 0;
	int type = 0;

	if (paramnum == 3) {
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
		if (_tcsnicmp(params[2], _T("MAP"), 3) == 0)
#else
#ifdef _UNICODE
		if (wcsncasecmp(params[2], _T("MAP"), 3) == 0)
#else
		if (strncasecmp(params[2], _T("MAP"), 3) == 0)
#endif
#endif
		{
			vals[0] = (int)HexaToInt(&params[2][3]);
			type = 1;
		} else if (_tcsicmp(params[2], _T("RESTORE")) == 0) {
			type = 2;
		}
	} else if (paramnum == 4 && _tcsicmp(params[2], _T("MAP")) == 0) {
		vals[0] = (int)HexaToInt(params[3]);
		type = 1;
	}

	switch(type) {
	case 1:
		{
			// store values from map
			int count = 16;
			rc = current_dev->mem->debug_memory_space_map_get(this, vals[0], vals, count);
			if (rc == 0) rc = current_dev->mem->debug_address_map_edit(this, idx, vals, count);
		}
		break;
	case 2:
		{
			// restore previous values
			int count = 16;
			rc = current_dev->mem->debug_address_map_get_prev(this, idx, vals, count);
			if (rc == 0) rc = current_dev->mem->debug_address_map_edit(this, idx, vals, count);
		}
		break;
	default:
		{
			// store specified values
			int count = paramnum - 2;
			for(int i=0; i<count; i++) {
				vals[i] = (int)HexaToInt(params[i+2]);
			}
			rc = current_dev->mem->debug_address_map_edit(this, idx, vals, count);
		}
		break;
	}

	if (rc == 1) {
		PrintError(_T("Invalid segment number."));
	} else if (rc == 2) {
		PrintError(_T("No values found."));
	} else if (rc < 0) {
		PrintError(_T("No support on current CPU and architecture."));
	}
}

void DebuggerConsole::UsageEditAddressMap(bool s)
{
	UsageCmdStr3(s, _T("XE"), _T("<segment number> <a0> [<a1> [... [<aF>]]]"), _T("Modify address map.")
	, _T("XE"), _T("<segment number> MAP<n>"), _T("Store memory space map on S1 BASIC to address map.")
	, _T("XE"), _T("<segment number> RESTORE"), _T("Restore previous map to address map."));
	if (s) return;

	Print(_T(" XE command modifies values in $FE00 - $FE0F on System I/O."));
	Print(_T("  <segment number> - specify number of address segment."));
	Print(_T("  <a0> ... <aF>    - specify 16 addresses (0 - F)."));
	Print(_T("  <n>              - specify space number (0 - F)."));
	Print(_T("  RESTORE          - Restore the map before the last modification."));
}

/// show memory space map (on S1 BASIC)
///
void DebuggerConsole::CommandShowMemorySpaceMap()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageShowMemorySpaceMap(false);
		return;
	}

	bool show_usage = false;
	int idx = 0x100;
	if (paramnum == 2) {
		if (_tcsicmp(params[1], _T("ALL")) == 0) {
			idx = 16;
		} else if (_tcsicmp(params[1], _T("ASSIGN")) == 0) {
			idx = 0x200;
		} else if (_tcsicmp(params[1], _T("ID")) == 0) {
			idx = 0x500;
		} else if (_tcsicmp(params[1], _T("CALL")) == 0) {
			idx = 0x700;
		} else if (_tcsicmp(params[1], _T("DP")) == 0) {
			idx = 0x900;
		} else {
			idx = (int)(HexaToInt(params[1]) & 0xf);
		}
	} else if (paramnum == 3) {
		if (_tcsicmp(params[1], _T("ID")) == 0) {
			if (_tcsicmp(params[2], _T("ALL")) == 0) {
				idx = 0x416;
			} else {
				idx = (int)(HexaToInt(params[2]) & 0xf) | 0x400;
			}
		} else if (_tcsicmp(params[1], _T("CALL")) == 0) {
			if (_tcsicmp(params[2], _T("ALL")) == 0) {
				idx = 0x616;
			} else {
				idx = (int)(HexaToInt(params[2]) & 0xf) | 0x600;
			}
		} else if (_tcsicmp(params[1], _T("DP")) == 0) {
			if (_tcsicmp(params[2], _T("ALL")) == 0) {
				idx = 0x816;
			} else {
				idx = (int)(HexaToInt(params[2]) & 0xf) | 0x800;
			}
		} else {
			show_usage = true;
		}
	} else if(paramnum > 3) {
		show_usage = true;
	}

	if (show_usage) {
		PrintError(_T("Invalid parameter(s)."));
		Cr();
		UsageShowMemorySpaceMap(false);
		return;
	}
	int rc = current_dev->mem->debug_memory_space_map_info(this, idx);
	if (rc < 0) {
		PrintError(_T("No support on current CPU and architecture."));
	}
}

void DebuggerConsole::UsageShowMemorySpaceMap(bool s)
{
	if (s) {
		UsageCmdStr1(s, _T("MAP ..."), NULL, _T("Show various map related to S1 BASIC."));
		return;
	}
	UsageCmdStrN(s, _T("MAP"), _T("[<space number> | ALL]"), _T("Show memory space map on S1 BASIC.")
	, _T("MAP ID"), _T("[<space number> | ALL]"), _T("Show memory ident map on S1 BASIC.")
	, _T("MAP CALL"), _T("[<space number> | ALL]"), _T("Show systemcall addresses on S1 BASIC.")
	, _T("MAP DP"), _T("[<space number> | ALL]"), _T("Show direct pointer in specified space on S1 BASIC.")
	, _T("MAP ASSIGN"), _T(""), _T("Show assigned memory map by S1 BASIC.")
	, NULL);

	Print(_T("  <space number> - specify number of memory space."));
	Print(_T("                   If no set number, show current map."));
	Print(_T("  ALL            - show all memory map."));
}

/// edit memory space map (on S1 BASIC)
///
void DebuggerConsole::CommandEditMemorySpaceMap()
{
	if(paramnum < 3 || *params[1] == _T('?')) {
		Cr();
		UsageEditMemorySpaceMap(false);
		return;
	}

	bool show_usage = false;
	int idx = 0;
	int starg = 1;

	if (_tcsicmp(params[1], _T("DP")) == 0) {
		idx |= 0x800;
		starg = 2;
		if (paramnum < 4 || paramnum > 16) {
			show_usage = true;
		}
	} else if (_tcsicmp(params[1], _T("ID")) == 0) {
		idx |= 0x400;
		starg = 2;
		if (paramnum != 4) {
			show_usage = true;
		}
	} else {
		if (paramnum > 15) {
			show_usage = true;
		}
	}

	if (show_usage) {
		PrintError(_T("Invalid parameter(s)."));
		Cr();
		UsageEditMemorySpaceMap(false);
		return;
	}

	idx |= (int)(HexaToInt(params[starg]) & 0xf);
	starg++;

	int vals[16];
	int count = paramnum - starg;
	for(int i=0; i<count; i++) {
		vals[i] = (int)HexaToInt(params[i+starg]);
	}

	int rc = current_dev->mem->debug_memory_space_map_edit(this, idx, vals, count);
	if (rc == 1) {
		PrintError(_T("Invalid space number."));
	} else if (rc < 0) {
		PrintError(_T("No support on current CPU and architecture."));
	}
}

void DebuggerConsole::UsageEditMemorySpaceMap(bool s)
{
	if (s) {
		UsageCmdStr1(s, _T("MAPE ..."), NULL, _T("Edit various map related to S1 BASIC."));
		return;
	}

	UsageCmdStrN(s, _T("MAPE"), _T("<space number> <a0> [<a1> [... [<aC>]]]"), _T("Edit memory space map on S1 BASIC.")
	, _T("MAPE ID"), _T("<space number> <a0> [<a1> [... [<aC>]]]"), _T("Edit memory ident map on S1 BASIC.")
	, _T("MAPE DP"), _T("<space number> <dp>"), _T("Edit direct pointer in specified space on S1 BASIC.")
	, NULL);

	Print(_T("  <space number> - specify number of memory space."));
	Print(_T("  <a0> ... <aC>  - specify 13 addresses (0 - C)."));
	Print(_T("  <dp>           - specify direct pointer."));
}
#endif

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandShowClock()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageShowClock(false);
		return;
	}

	if(paramnum >= 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageShowClock(false);
		return;
	}

	for(int i=0; i<num_of_cpus; i++) {
		struct devs_st *dev = &devs[i];

		uint64_t clk = dev->cpu->get_current_clock();
		uint32_t freq = dev->cpu->get_cpu_clock();

		Printf(_T("%d: "), i);
#ifdef _UNICODE
		CWchar cname(dev->cpu->get_class_name());
		Print(cname);
#else
		Print(dev->cpu->get_class_name());
#endif
		Printf(_T("  Clock: %12lld    Frequency: %10d Hz"), clk, freq); Cr();

		if (freq > 0) {
			double ms = (double)clk / ((double)freq / 1000.0);
			double us = (double)clk / ((double)freq / 1000000.0);

			Printf(_T("  Time : %.4f us    %.4f ms"), us, ms); Cr();
		}
		Cr();
	}
}

void DebuggerConsole::UsageShowClock(bool s)
{
	UsageCmdStr1(s, _T("CL"), NULL, _T("Show current clock."));
	if (s) return;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSwitchCPU()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSwitchCPU(false);
		return;
	}

	if(paramnum == 1) {
		Print(_T("Current CPU is "), false);

	} else {
		int index = (int)HexaToInt(params[1]);
		if (!SwitchCPU(index)) {
			PrintError(_T("Can't switch from current CPU."));
			return;
		}
		Print(_T("Switched to "), false);
	}
	Print(storage->GetCurrentCPUName());
}

void DebuggerConsole::UsageSwitchCPU(bool s)
{
	UsageCmdStr1(s, _T("CPU"), _T("[<index number>]"), _T("Show current CPU / Switch to specified CPU for debugging."));
	if (s) return;

	Print(_T("  <index number> - specify index of CPU to switch."));
}

bool DebuggerConsole::SwitchCPU(int idx)
{
	if (idx < 0 || idx >= num_of_cpus) return false;

	if (!devs[idx].cpu->enable()) return false;

	current_dev = &devs[idx];
	storage->SetCurrentCPUName(current_dev->cpu->get_class_name());

	return true;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandPWD()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsagePWD(false);
		return;
	}

	PWD();
}

void DebuggerConsole::UsagePWD(bool s)
{
	UsageCmdStr1(s, _T("PWD"), NULL, _T("Show current working directory."));
	if (s) return;
}

void DebuggerConsole::CommandCHD()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageCHD(false);
		return;
	}
	if(paramnum != 2) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageCHD(false);
		return;
	}

	if (CHD(params[1])) {
		PWD();
	} else {
		PrintError(_T("Can't change directory."));
	}
}

void DebuggerConsole::UsageCHD(bool s)
{
	UsageCmdStr1(s, _T("CD"), _T("<path>"), _T("Change working directory."));
	if (s) return;

	Print(_T("  <path> - specify a directory."));
}

void DebuggerConsole::CommandDIR()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageDIR(false);
		return;
	}
	if(paramnum != 1) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageDIR(false);
		return;
	}

	DIR(NULL);
}

void DebuggerConsole::UsageDIR(bool s)
{
	UsageCmdStr2(s, _T("DIR"), NULL, _T("List files on current directory.")
				,   _T("LS"), NULL, _T("List files on current directory."));
	if (s) return;
}

void DebuggerConsole::PWD()
{
#if defined(_WIN32)
	if (!GetCurrentDirectory(DC_MAX_BUFFER_LEN, buffer))
#else
#if defined(_UNICODE)
	char nbuffer[DC_MAX_BUFFER_LEN];
	if (!getcwd(nbuffer, DC_MAX_BUFFER_LEN))
#else
	if (!getcwd(buffer, DC_MAX_BUFFER_LEN))
#endif
#endif
	{
		buffer[0] = _T('\0');
	}
#if defined(_UNICODE) && !defined(_WIN32)
	UTILITY::conv_utf8_to_wcs(nbuffer, DC_MAX_BUFFER_LEN, buffer, DC_MAX_BUFFER_LEN);
#endif
	Print(buffer);
}

bool DebuggerConsole::CHD(const _TCHAR *path)
{
	bool rc;
#if defined(_WIN32)
	rc = (SetCurrentDirectory(params[1]) != 0);
#else
#if defined(_UNICODE)
	char nbuffer[DC_MAX_BUFFER_LEN];
	UTILITY::conv_wcs_to_utf8(params[1], DC_MAX_BUFFER_LEN, nbuffer, DC_MAX_BUFFER_LEN);
	rc = (chdir(nbuffer) >= 0);
#else
	rc = (chdir(params[1]) >= 0);
#endif
#endif
	return rc;
}

void DebuggerConsole::DIR(const _TCHAR *path)
{
#if defined(_WIN32) && defined(_MSC_VER)
	WIN32_FIND_DATA file;
	HANDLE hFile = FindFirstFile(path ? path : _T(".\\*"), &file);
	while(hFile) {
		Print(file.cFileName);
		if (!FindNextFile(hFile, &file)) break;
	}
#else
	struct dirent *dp;
#if defined(_UNICODE)
	char nbuffer[DC_MAX_BUFFER_LEN];
	if (path) {
		UTILITY::conv_wcs_to_utf8(path, DC_MAX_BUFFER_LEN, nbuffer, DC_MAX_BUFFER_LEN);
	} else {
		strcpy(nbuffer, ".");
	}
	::DIR *dir = opendir(nbuffer);
#else
	::DIR *dir = opendir(path ? path : ".");
#endif
	if (!dir) return;

	while((dp = readdir(dir)) != NULL) {
#if defined(_UNICODE)
		wchar_t w_name[DC_MAX_BUFFER_LEN];
		UTILITY::conv_utf8_to_wcs(dp->d_name, DC_MAX_BUFFER_LEN, w_name, DC_MAX_BUFFER_LEN);
		Print(w_name);
#else
		Print(dp->d_name);
#endif
	}
	closedir(dir);
#endif
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandQuit()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageQuit(false);
		return;
	}

	Print(_T("Quit"));
	dp->emu->get_gui()->PostMtCloseDebugger();
}

void DebuggerConsole::UsageQuit(bool s)
{
	UsageCmdStr1(s, _T("Q"), NULL, _T("Quit debugger."));
	if (s) return;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandOutputLogFile()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageOutputLogFile(false);
		return;
	}

	if(paramnum >= 2) {
		if(logfile != NULL && logfile->IsOpened()) {
			logfile->Fclose();
			delete logfile;
			logfile = NULL;
			logfilename[0] = _T('\0');
		}
		if (_tcsicmp(params[1], _T("CLOSE")) != 0) {
			UTILITY::tcscpy(logfilename, _MAX_PATH, params[1]);
			logfile = new FILEIO();
			if (!logfile->Fopen(logfilename, FILEIO::WRITE_ASCII)) {
				delete logfile;
				logfile = NULL;
				logfilename[0] = _T('\0');
			}
		}
	}
	if (_tcslen(logfilename) > 0) {
		Printf(_T("Current log file is \"%s\"."), logfilename);
		Cr();
	} else {
		Print(_T("No log file is opened."));
	}
}

void DebuggerConsole::UsageOutputLogFile(bool s)
{
	UsageCmdStr1(s, _T(">"), _T("[<filename>]"), _T("Open/Close a log file."));
	if (s) return;

	Print(_T("  If filename is specified \"CLOSE\", close the log file."));
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandSendToEmulator()
{
	if(paramnum >= 2 && *params[1] == _T('?')) {
		Cr();
		UsageSendToEmulator(false);
		return;
	}

	if(paramnum == 1) {
		PrintError(_T("Invalid parameter."));
		Cr();
		UsageSendToEmulator(false);
		return;
	}

	if(_tcsicmp(params[1], _T("RESET")) == 0) {
		CommandSendResetToEmulator();
	} else if(_tcsicmp(params[1], _T("KEY")) == 0) {
		CommandSendKeyToEmulator();
	} else {
		PrintError(_T("Unknown keyword: "), false);
		Print(params[1]);
		Cr();
		UsageSendToEmulator(false);
	}
}

void DebuggerConsole::CommandSendResetToEmulator()
{
	if(paramnum == 2) {
//			dp->vm->reset();
		dp->emu->get_gui()->PostEtReset();

	} else if(paramnum == 3) {
		if(_tcsicmp(params[2], _T("ALL")) == 0) {
//				dp->vm->reset();
			dp->emu->get_gui()->PostEtReset();

		} if(_tcsicmp(params[2], _T("CPU")) == 0) {
			current_dev->cpu->reset();
		} else {
			PrintError(_T("Unknown device: "), false);
			Print(params[2]);
		}
	} else {
		PrintError(_T("Invalid parameter."));
		Cr();
		UsageSendToEmulator(false);
	}
}

void DebuggerConsole::CommandSendKeyToEmulator()
{
	if(paramnum == 3 || paramnum == 4) {
		int code =  HexaToInt(params[2]);
		if (code < 0 || KEY_STATUS_SIZE <= code) {
			PrintError(_T("Invalid key code."));
			Cr();
			UsageSendToEmulator(false);
			return;
		}
		int msec = 100;
		if(paramnum == 4) {
			msec = HexaToInt(params[3]);
		}
#ifdef SUPPORT_VARIABLE_TIMING
		key_stat_tmp[code] = MAX((int)(dp->vm->get_frame_rate() * (double)msec / 1000.0 + 0.5), 1);
#else
		key_stat_tmp[code] = MAX((int)(FRAMES_PER_SEC * (double)msec / 1000.0 + 0.5), 1);
#endif
#ifdef NOTIFY_KEY_DOWN
		dp->vm->key_down(code, false);
#endif
	} else {
		PrintError(_T("Invalid parameter."));
		Cr();
		UsageSendToEmulator(false);
	}
}

void DebuggerConsole::UsageSendToEmulator(bool s)
{
	UsageCmdStr1(s, _T("!"), NULL, _T("Send command to emulator."));
	if (s) return;

	Print(_T("! reset [all,cpu]"), false);
	Print(_T(" - "), false);
	Print(_T("Send restart."));
	Print(_T("! key <code> [<msec>]"), false);
	Print(_T(" - "), false);
	Print(_T("Send a key code."));
}

// ---------------------------------------------------------------------------

void DebuggerConsole::CommandDebugBasic()
{
	if (!current_dev->mem->debug_basic_is_supported()) {
		PrintError(_T("No support on current CPU and architecture."));
		return;
	}
	if(paramnum < 2 || *params[1] == _T('?')) {
		Cr();
		UsageDebugBasic(false, -1);
		return;
	}

	if (_tcsicmp(params[1], _T("VAR")) == 0) {
		// variable list
		CommandDebugBasicVariables(paramnum - 1, &params[1]);
	} else if (_tcsicmp(params[1], _T("LIST")) == 0) {
		// program list
		CommandDebugBasicList(paramnum - 1, &params[1]);
	} else if (_tcsicmp(params[1], _T("TRON")) == 0) {
		// trace on line number
		CommandDebugBasicTraceOnOff(paramnum - 1, &params[1], true);
	} else if (_tcsicmp(params[1], _T("TROFF")) == 0) {
		// trace off line number
		CommandDebugBasicTraceOnOff(paramnum - 1, &params[1], false);
	} else if (_tcsicmp(params[1], _T("BP")) == 0) {
		// break point at line number
		CommandDebugBasicSetBreakPoint(paramnum - 1, &params[1], BP_BASIC);
	} else if (_tcsicmp(params[1], _T("BC")) == 0) {
		// clear break point
		CommandDebugBasicClearBreakPoint(paramnum - 1, &params[1], BP_BASIC);
	} else if (_tcsicmp(params[1], _T("BD")) == 0) {
		// disable break point
		CommandDebugBasicChangeBreakPoint(paramnum - 1, &params[1], BP_BASIC);
	} else if (_tcsicmp(params[1], _T("BE")) == 0) {
		// enable break point
		CommandDebugBasicChangeBreakPoint(paramnum - 1, &params[1], BP_BASIC);
	} else if (_tcsicmp(params[1], _T("TP")) == 0) {
		// trace point at line number
		CommandDebugBasicSetBreakPoint(paramnum - 1, &params[1], TP_BASIC);
	} else if (_tcsicmp(params[1], _T("TC")) == 0) {
		// clear trace point
		CommandDebugBasicClearBreakPoint(paramnum - 1, &params[1], TP_BASIC);
	} else if (_tcsicmp(params[1], _T("TD")) == 0) {
		// disable trace point
		CommandDebugBasicChangeBreakPoint(paramnum - 1, &params[1], TP_BASIC);
	} else if (_tcsicmp(params[1], _T("TE")) == 0) {
		// enable trace point
		CommandDebugBasicChangeBreakPoint(paramnum - 1, &params[1], TP_BASIC);
	} else if (_tcsicmp(params[1], _T("TB")) == 0) {
		// trace back program list
		CommandDebugBasicTraceBack(paramnum - 1, &params[1]);
	} else if (_tcsicmp(params[1], _T("COMMAND")) == 0) {
		// command list
		CommandDebugBasicCommand(paramnum - 1, &params[1]);
	} else if (_tcsicmp(params[1], _T("ERROR")) == 0) {
		// error
		CommandDebugBasicError(paramnum - 1, &params[1]);
	} else {
		PrintError(_T("Invalid parameter."));
		Cr();
		UsageDebugBasic(false, -1);
	}
}

void DebuggerConsole::CommandDebugBasicVariables(int pn, _TCHAR **ps)
{
	if(pn >= 2 && *ps[1] == _T('?')) {
		Cr();
		UsageDebugBasic(false, 1);
		return;
	}
	current_dev->mem->debug_basic_variables(this, pn - 1, (const _TCHAR **)&ps[1]);
}

void DebuggerConsole::CommandDebugBasicList(int pn, _TCHAR **ps)
{
	if((pn >= 2 && *ps[1] == _T('?')) || pn >= 3) {
		Cr();
		UsageDebugBasic(false, 2);
		return;
	}
	int st_line = 0;
	int ed_line = -1;
	if (pn == 2) {
		if(_tcscmp(ps[1], _T(".")) == 0) {
			// current
			st_line = -1;
		} else {
			// line number
			DeciToNum(ps[1], st_line, ed_line);
		}
	}
	current_dev->mem->debug_basic_list(this, st_line, ed_line);
}

void DebuggerConsole::CommandDebugBasicTraceOnOff(int pn, _TCHAR **ps, bool en)
{
	if(pn != 1) {
		Cr();
		UsageDebugBasic(false, 3);
		return;
	}
	current_dev->mem->debug_basic_trace_onoff(this, en);
}

void DebuggerConsole::CommandDebugBasicCommand(int pn, _TCHAR **ps)
{
	if(pn != 1) {
		Cr();
		UsageDebugBasic(false, 6);
		return;
	}
	current_dev->mem->debug_basic_command(this);
}

void DebuggerConsole::CommandDebugBasicError(int pn, _TCHAR **ps)
{
	if((pn >= 2 && *ps[1] == _T('?')) || pn >= 3) {
		Cr();
		UsageDebugBasic(false, 7);
		return;
	}
	int num = -1;
	if (pn == 2) {
		num = (int)_tcstol(ps[1], NULL, 10);
	}
	current_dev->mem->debug_basic_error(this, num);
}

void DebuggerConsole::CommandDebugBasicSetBreakPoint(int pn, _TCHAR **ps, int num)
{
	if((pn >= 2 && *ps[1] == _T('?')) || pn >= 3) {
		Cr();
		UsageDebugBasic(false, num);
		return;
	}

	BreakPoints *bps = (num & 8 ? current_dev->debugger->get_tps(BreakPoints::BP_BASIC_NUMBER) : current_dev->debugger->get_bps(BreakPoints::BP_BASIC_NUMBER));

	if (pn >= 2) {
		if (!SetBreakPoint(bps, num >> 3, num & 7, ps, pn)) {
			current_dev->mem->debug_basic_trace_onoff(this, true);
		}
	} else {
		ListBreakPoint(bps, num & 8, num & 7);
	}
}

void DebuggerConsole::CommandDebugBasicClearBreakPoint(int pn, _TCHAR **ps, int num)
{
	if((pn >= 2 && *ps[1] == _T('?'))) {
		Cr();
		UsageDebugBasic(false, num + 1);
		return;
	}

	BreakPoints *bps = (num & 8 ? current_dev->debugger->get_tps(BreakPoints::BP_BASIC_NUMBER) : current_dev->debugger->get_bps(BreakPoints::BP_BASIC_NUMBER));

	if (ClearBreakPoint(bps, num & 8, num & 7, pn, ps)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageDebugBasic(false, num + 1);
	}
}

void DebuggerConsole::CommandDebugBasicChangeBreakPoint(int pn, _TCHAR **ps, int num)
{
	if((pn >= 2 && *ps[1] == _T('?'))) {
		Cr();
		UsageDebugBasic(false, num + 1);
		return;
	}

	BreakPoints *bps = (num & 8 ? current_dev->debugger->get_tps(BreakPoints::BP_BASIC_NUMBER) : current_dev->debugger->get_bps(BreakPoints::BP_BASIC_NUMBER));

	if (ChangeBreakPoint(bps, num & 8, num & 7, pn, ps)) {
		PrintError(_T("Invalid number of parameter(s)."));
		Cr();
		UsageDebugBasic(false, num + 1);
	}
}

void DebuggerConsole::CommandDebugBasicTraceBack(int pn, _TCHAR **ps)
{
	if(pn != 1) {
		Cr();
		UsageDebugBasic(false, 8);
		return;
	}
	current_dev->mem->debug_basic_trace_back(this, 0);
}

void DebuggerConsole::UsageDebugBasic(bool s, int num)
{
	const _TCHAR *cmds[32];
	int cnt = 0;

	if (s) {
		UsageCmdStr1(s, _T("BAS ..."), NULL, _T("Show various values ralated to BASIC."));
		return;
	}

	if(num < 0 || num == 1) {
		cnt = SetStr1(cnt, cmds, _T("BAS VAR"), _T("[<name(s)> ...]"), _T("Show variable list or value on BASIC."));
	}
	if(num < 0 || num == 2) {
		cnt = SetStr1(cnt, cmds, _T("BAS LIST"), _T("[<line number(s)>]"), _T("Show program list on BASIC."));
	}
	if(num < 0 || num == 4) {
		cnt = SetStr1(cnt, cmds, _T("BAS BP"), _T("[<line number(s)>]"), _T("Set/Show breakpoint for BASIC."));
	}
	if(num < 0 || num == 5) {
		cnt = SetStr1(cnt, cmds,  _T("BAS B{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable breakpoint(s) for BASIC."));
	}
	if(num < 0 || num == 12) {
		cnt = SetStr1(cnt, cmds, _T("BAS TP"), _T("[<line number(s)>]"), _T("Set/Show tracepoint for BASIC."));
	}
	if(num < 0 || num == 13) {
		cnt = SetStr1(cnt, cmds,  _T("BAS T{C,D,E}"), _T("{*,ALL,<list>}"), _T("Clear/Disable/Enable tracepoint(s) for BASIC."));
	}
	if(num < 0 || num == 8) {
		cnt = SetStr1(cnt, cmds, _T("BAS TB"), _T(""), _T("Trace back program list on BASIC."));
	}
	if(num < 0 || num == 3) {
		cnt = SetStr1(cnt, cmds, _T("BAS TRON/TROFF"), _T(""), _T("Trace on/off program list when hit breakpoint/tracepoint."));
	}
	if(num < 0 || num == 6) {
		cnt = SetStr1(cnt, cmds, _T("BAS COMMAND"), _T(""), _T("Show command and function list on BASIC."));
	}
	if(num < 0 || num == 7) {
		cnt = SetStr1(cnt, cmds, _T("BAS ERROR"), _T("[<error number>]"), _T("Show latest or specified error on BASIC."));
	}
	cmds[cnt] = NULL;
	UsageCmdStr(s, cnt, cmds);

	if(num < 0 || num == 1) {
		Print(_T("  <name(s)> - show value(s) matching with variable name."));
	}
	if(num < 0 || num == 2 || num == 4 || num == 12) {
		Print(_T("  <line number(s)> - set one decimal <number> or set range <start>-<end>."));
		Print(_T("  set current line number if specify \".\"(period)."));
	}
	if(num < 0 || num == 5 || num == 13) {
		Print(_T("  <list> - specify number."));
		Print(_T("  * or ALL - perform it about all breakpoints."));
	}
	if(num < 0 || num == 7) {
		Print(_T("  <error number> - set one decimal <number>."));
	}
}

void DebuggerConsole::BasicTraceCurrent()
{
	current_dev->mem->debug_basic_trace_current();
}

// ---------------------------------------------------------------------------

void DebuggerConsole::UsageCmdStr1(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1)
{
	Printf(_T("%-8s"), cmd1);
	Print(_T(" - "), false);
	Print(desc1);
	if (s) return;
	Cr();
	if (!args1) return;
	Print(_T("Usage: "), false);
	Print(cmd1, false);
	Print(_T(" "), false);
	Print(args1);
}

void DebuggerConsole::UsageCmdStr2(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1
	, const _TCHAR *cmd2, const _TCHAR *args2, const _TCHAR *desc2)
{
	Printf(_T("%-8s"), cmd1);
	Print(_T(" - "), false);
	Print(desc1);
	Printf(_T("%-8s"), cmd2);
	Print(_T(" - "), false);
	Print(desc2);
	if (s) return;
	Cr();
	if (!args1) return;
	Print(_T("Usage: "), false);
	Print(cmd1, false);
	Print(_T(" "), false);
	Print(args1);
	if (!args2) return;
	Print(_T("       "), false);
	Print(cmd2, false);
	Print(_T(" "), false);
	Print(args2);
}

void DebuggerConsole::UsageCmdStr3(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1
	, const _TCHAR *cmd2, const _TCHAR *args2, const _TCHAR *desc2
	, const _TCHAR *cmd3, const _TCHAR *args3, const _TCHAR *desc3)
{
	Printf(_T("%-8s"), cmd1);
	Print(_T(" - "), false);
	Print(desc1);
	Printf(_T("%-8s"), cmd2);
	Print(_T(" - "), false);
	Print(desc2);
	Printf(_T("%-8s"), cmd3);
	Print(_T(" - "), false);
	Print(desc3);
	if (s) return;
	Cr();
	if (!args1) return;
	Print(_T("Usage: "), false);
	Print(cmd1, false);
	Print(_T(" "), false);
	Print(args1);
	if (!args2) return;
	Print(_T("       "), false);
	Print(cmd2, false);
	Print(_T(" "), false);
	Print(args2);
	if (!args3) return;
	Print(_T("       "), false);
	Print(cmd3, false);
	Print(_T(" "), false);
	Print(args3);
}

void DebuggerConsole::UsageCmdStrN(bool s, ...)
{
	va_list ap;
	int ac = 0;
	const _TCHAR *av[30];
	const _TCHAR *p;

	va_start(ap, s);

	while((p = va_arg(ap, const _TCHAR *)) != NULL && ac < 30) {
		av[ac] = p;
		ac++;
	};

	for(int cnt = 0; cnt < ac; cnt += 3) {
		Printf(_T("%-8s"), av[cnt]);
		Print(_T(" - "), false);
		Print(av[cnt+2]);
	}

	if (s) return;
	Cr();

	for(int cnt = 0; cnt < ac; cnt += 3) {
		if (cnt == 0) Print(_T("Usage: "), false);
		else Print(_T("       "), false);
		Print(av[cnt], false);
		Print(_T(" "), false);
		Print(av[cnt+1]);
	}
	va_end(ap);
}

void DebuggerConsole::UsageCmdStr(bool s, int cnts, const _TCHAR **cmds)
{
	for(int cnt = 0; cnt < cnts; cnt += 3) {
		Printf(_T("%-8s"), cmds[cnt]);
		Print(_T(" - "), false);
		Print(cmds[cnt+2]);
	}

	if (s) return;
	Cr();

	for(int cnt = 0; cnt < cnts; cnt += 3) {
		if (cnt == 0) Print(_T("Usage: "), false);
		else Print(_T("       "), false);
		Print(cmds[cnt], false);
		Print(_T(" "), false);
		Print(cmds[cnt+1]);
	}
}

int DebuggerConsole::SetStr1(int cnt, const _TCHAR **cmds, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1)
{
	cmds[cnt++] = cmd1;
	cmds[cnt++] = args1;
	cmds[cnt++] = desc1;
	return cnt;
}

// ---------------------------------------------------------------------------

void DebuggerConsole::Usage()
{
	UsageDumpMemory(true, 0);
#ifdef USE_EMU_INHERENT_SPEC
	UsageDumpMemory(true, 1);
#endif
	UsageEditMemory(true, 0);
#ifdef USE_EMU_INHERENT_SPEC
	UsageEditMemory(true, 1);
#endif
#ifdef USE_IOPORT_ACCESS
	UsageEditIO(true);
#endif
	UsageAccessRegister(true);
	UsageAccessDevice(true);
	UsageToggleRegister(true);
#ifdef USE_EMU_INHERENT_SPEC
	UsageShowMemoryMap(true);
#endif
#ifdef _MBS1
	UsageShowAddressMap(true);
	UsageEditAddressMap(true);
	UsageShowMemorySpaceMap(true);
	UsageEditMemorySpaceMap(true);
#endif
	UsageSearch(true, 0);
#ifdef USE_EMU_INHERENT_SPEC
	UsageSearch(true, 1);
#endif
	UsageUnassemble(true, 0);
#ifdef USE_EMU_INHERENT_SPEC
	UsageUnassemble(true, 1);
#endif

	UsageCalculateHexa(true);
	UsageSetFileName(true);
	UsageBinaryFile(true, 0);
#ifdef USE_EMU_INHERENT_SPEC
	UsageBinaryFile(true, 1);
#endif
#ifdef USE_EMU_INHERENT_SPEC
	UsageImageFile(true, 0);
#endif

	UsageSetBreakPoint(true, BP_FETCH);
	UsageSetBreakPoint(true, BP_MEMRD);
#ifdef USE_IOPORT_ACCESS
	UsageSetBreakPoint(true, BP_IORD);
#endif
	UsageSetBreakPoint(true, BP_INTR);
	UsageSetBreakPoint(true, BP_EXCEPT);
	UsageChangeBreakPoint(true, BP_FETCH);
	UsageChangeBreakPoint(true, BP_MEMRD);
#ifdef USE_IOPORT_ACCESS
	UsageChangeBreakPoint(true, BP_IORD);
#endif
	UsageChangeBreakPoint(true, BP_INTR);
	UsageChangeBreakPoint(true, BP_EXCEPT);
//	UsageListBreakPoint(true, BP_FETCH);
//	UsageListBreakPoint(true, BP_MEMRW);
#ifdef USE_IOPORT_ACCESS
//	UsageListBreakPoint(true, BP_IORW);
#endif
//	UsageListBreakPoint(true, BP_INTR);

	UsageExecute(true);
	UsageTrace(true);
	UsageTraceBack(true);

	UsageSetBreakPoint(true, TP_FETCH);
	UsageSetBreakPoint(true, TP_MEMRD);
#ifdef USE_IOPORT_ACCESS
	UsageSetBreakPoint(true, TP_IORD);
#endif
	UsageSetBreakPoint(true, TP_INTR);
	UsageSetBreakPoint(true, TP_EXCEPT);
	UsageChangeBreakPoint(true, TP_FETCH);
	UsageChangeBreakPoint(true, TP_MEMRD);
#ifdef USE_IOPORT_ACCESS
	UsageChangeBreakPoint(true, TP_IORD);
#endif
	UsageChangeBreakPoint(true, TP_INTR);
	UsageChangeBreakPoint(true, TP_EXCEPT);
//	UsageListBreakPoint(true, TP_FETCH);
//	UsageListBreakPoint(true, TP_MEMRW);
#ifdef USE_IOPORT_ACCESS
//	UsageListBreakPoint(true, TP_IORD);
#endif
//	UsageListBreakPoint(true, TP_INTR);
	UsageSetSymbol(true);
	UsageClearSymbol(true);
	UsageLoadSymbol(true);

	UsageShowClock(true);

	UsageOutputLogFile(true);

	UsageSendToEmulator(true);

#ifdef USE_EMU_INHERENT_SPEC
	UsageDebugBasic(true, -1);
#endif
	UsagePWD(true);
	UsageCHD(true);
	UsageDIR(true);

	UsageSwitchCPU(true);

	UsageQuit(true);
	Cr();
	Print(_T("<value> - hexa, decimal(%d), ascii('a')"));
}

void DebuggerConsole::CommandUsage()
{
	if (paramnum >= 2) {
		int idx = -1;
		for(int i=0; commands_map[i].cmd != NULL; i++) {
			if (_tcsicmp(params[1], commands_map[i].cmd) == 0) {
				idx = i;
				break;
			}
		}
		if (idx >= 0) {
			switch(commands_map[idx].idx) {
			case ID_DUMP_MEMORY:
				UsageDumpMemory(false, 0);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_DUMP_PHYSICAL_MEMORY:
				UsageDumpMemory(false, 1);
				break;
#endif
			case ID_EDIT_MEMORY_BYTE:
			case ID_EDIT_MEMORY_WORD:
			case ID_EDIT_MEMORY_DWORD:
			case ID_EDIT_MEMORY_ASCII:
				UsageEditMemory(false, 0);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_EDIT_PHYSICAL_MEMORY_BYTE:
			case ID_EDIT_PHYSICAL_MEMORY_WORD:
			case ID_EDIT_PHYSICAL_MEMORY_DWORD:
			case ID_EDIT_PHYSICAL_MEMORY_ASCII:
				UsageEditMemory(false, 1);
				break;
#endif
#ifdef USE_IOPORT_ACCESS
			case ID_INPUT_IO_BYTE:
			case ID_INPUT_IO_WORD:
			case ID_INPUT_IO_DWORD:
			case ID_OUTPUT_IO_BYTE:
			case ID_OUTPUT_IO_WORD:
			case ID_OUTPUT_IO_DWORD:
				UsageEditIO(false);
				break;
#endif
			case ID_ACCESS_REGISTER:
				UsageAccessRegister(false);
				break;
			case ID_ACCESS_DEVICE:
				UsageAccessDevice(false);
				break;
			case ID_SHOW_REGISTER:
			case ID_HIDE_REGISTER:
				UsageToggleRegister(false);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_SHOW_MEMORY_MAP:
				UsageShowMemoryMap(false);
				break;
#endif
#ifdef _MBS1
			case ID_SHOW_ADDRESS_MAP:
				UsageShowAddressMap(false);
				break;
			case ID_EDIT_ADDRESS_MAP:
				UsageEditAddressMap(false);
				break;
			case ID_SHOW_MEMORY_SPACE_MAP:
				UsageShowMemorySpaceMap(false);
				break;
			case ID_EDIT_MEMORY_SPACE_MAP:
				UsageEditMemorySpaceMap(false);
				break;
#endif
			case ID_SEARCH_BYTE:
			case ID_SEARCH_WORD:
			case ID_SEARCH_DWORD:
			case ID_SEARCH_ASCII:
				UsageSearch(false, 0);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_SEARCH_PHYSICAL_BYTE:
			case ID_SEARCH_PHYSICAL_WORD:
			case ID_SEARCH_PHYSICAL_DWORD:
			case ID_SEARCH_PHYSICAL_ASCII:
				UsageSearch(false, 1);
				break;
#endif
			case ID_UNASSEMBLE:
				UsageUnassemble(false, 0);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_UNASSEMBLE_PHYSICAL:
				UsageUnassemble(false, 1);
				break;
#endif
			case ID_CALC_HEXA:
				UsageCalculateHexa(false);
				break;
			case ID_SET_FILE_NAME:
				UsageSetFileName(false);
				break;
			case ID_LOAD_BINARY_FILE:
			case ID_SAVE_BINARY_FILE:
				UsageBinaryFile(false, 0);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_LOAD_BINARY_FILE_PHYSICAL:
			case ID_SAVE_BINARY_FILE_PHYSICAL:
				UsageBinaryFile(false, 1);
				break;
#endif
#ifdef USE_EMU_INHERENT_SPEC
			case ID_SAVE_IMAGE_FILE:
				UsageImageFile(false, 0);
				break;
#endif
			case ID_SET_BREAK_POINT:
				UsageSetBreakPoint(false, BP_FETCH);
				break;
			case ID_CLEAR_BREAK_POINT:
			case ID_ENABLE_BREAK_POINT:
			case ID_DISABLE_BREAK_POINT:
				UsageChangeBreakPoint(false, BP_FETCH);
				break;
			case ID_LIST_BREAK_POINT:
				UsageListBreakPoint(false, BP_FETCH);
				break;
#ifdef USE_BREAKPOINT_PHYSICAL
			case ID_SET_BREAK_POINT_PHYSICAL:
				UsageSetBreakPoint(false, BP_FETCH_PH);
				break;
			case ID_CLEAR_BREAK_POINT_PHYSICAL:
			case ID_ENABLE_BREAK_POINT_PHYSICAL:
			case ID_DISABLE_BREAK_POINT_PHYSICAL:
				UsageChangeBreakPoint(false, BP_FETCH_PH);
				break;
#endif
			case ID_SET_BREAK_POINT_FOR_MEMRD:
			case ID_SET_BREAK_POINT_FOR_MEMWR:
				UsageSetBreakPoint(false, BP_MEMRD);
				break;
			case ID_CLEAR_BREAK_POINT_FOR_MEMRD:
			case ID_ENABLE_BREAK_POINT_FOR_MEMRD:
			case ID_DISABLE_BREAK_POINT_FOR_MEMRD:
			case ID_CLEAR_BREAK_POINT_FOR_MEMWR:
			case ID_ENABLE_BREAK_POINT_FOR_MEMWR:
			case ID_DISABLE_BREAK_POINT_FOR_MEMWR:
				UsageChangeBreakPoint(false, BP_MEMRD);
				break;
//			case ID_LIST_BREAK_POINT_FOR_MEM:
//				UsageListBreakPoint(false, BP_MEMRD);
//				break;
#ifdef USE_BREAKPOINT_PHYSICAL
			case ID_SET_BREAK_POINT_FOR_MEMRD_PHYSICAL:
			case ID_SET_BREAK_POINT_FOR_MEMWR_PHYSICAL:
				UsageSetBreakPoint(false, BP_MEMRD_PH);
				break;
			case ID_CLEAR_BREAK_POINT_FOR_MEMRD_PHYSICAL:
			case ID_ENABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL:
			case ID_DISABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL:
			case ID_CLEAR_BREAK_POINT_FOR_MEMWR_PHYSICAL:
			case ID_ENABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL:
			case ID_DISABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL:
				UsageChangeBreakPoint(false, BP_MEMRD_PH);
				break;
#endif
#ifdef USE_IOPORT_ACCESS
			case ID_SET_BREAK_POINT_FOR_IORD:
			case ID_SET_BREAK_POINT_FOR_IOWR:
				UsageSetBreakPoint(false, BP_IORD);
				break;
			case ID_CLEAR_BREAK_POINT_FOR_IORD:
			case ID_ENABLE_BREAK_POINT_FOR_IORD:
			case ID_DISABLE_BREAK_POINT_FOR_IORD:
			case ID_CLEAR_BREAK_POINT_FOR_IOWR:
			case ID_ENABLE_BREAK_POINT_FOR_IOWR:
			case ID_DISABLE_BREAK_POINT_FOR_IOWR:
				UsageChangeBreakPoint(false, BP_IORD);
				break;
//			case ID_LIST_BREAK_POINT_FOR_IO:
//				UsageListBreakPoint(false, BP_IORD);
//				break;
#endif
			case ID_SET_BREAK_POINT_FOR_INTR:
				UsageSetBreakPoint(false, BP_INTR);
				break;
			case ID_CLEAR_BREAK_POINT_FOR_INTR:
			case ID_ENABLE_BREAK_POINT_FOR_INTR:
			case ID_DISABLE_BREAK_POINT_FOR_INTR:
				UsageChangeBreakPoint(false, BP_INTR);
				break;
//			case ID_LIST_BREAK_POINT_FOR_INTR:
//				UsageListBreakPoint(false, BP_INTR);
//				break;
#ifdef USE_EXCEPTION_BREAKPOINT
			case ID_SET_BREAK_POINT_FOR_EXCEPT:
				UsageSetBreakPoint(false, BP_EXCEPT);
				break;
			case ID_CLEAR_BREAK_POINT_FOR_EXCEPT:
			case ID_ENABLE_BREAK_POINT_FOR_EXCEPT:
			case ID_DISABLE_BREAK_POINT_FOR_EXCEPT:
				UsageChangeBreakPoint(false, BP_EXCEPT);
				break;
#endif
			case ID_SET_SYMBOL:
				UsageSetSymbol(false);
				break;
			case ID_CLEAR_SYMBOL:
				UsageClearSymbol(false);
				break;
			case ID_LOAD_SYMBOL:
				UsageLoadSymbol(false);
				break;
			case ID_EXECUTE:
				UsageExecute(false);
				break;
			case ID_TRACE:
				UsageTrace(false);
				break;
			case ID_TRACE_BACK:
				UsageTraceBack(false);
				break;
			case ID_SHOW_CLOCK:
				UsageShowClock(false);
				break;
			case ID_SET_TRACE_POINT:
				UsageSetBreakPoint(false, TP_FETCH);
				break;
			case ID_CLEAR_TRACE_POINT:
			case ID_ENABLE_TRACE_POINT:
			case ID_DISABLE_TRACE_POINT:
				UsageChangeBreakPoint(false, TP_FETCH);
				break;
			case ID_LIST_TRACE_POINT:
				UsageListBreakPoint(false, TP_FETCH);
				break;
#ifdef USE_BREAKPOINT_PHYSICAL
			case ID_SET_TRACE_POINT_PHYSICAL:
				UsageSetBreakPoint(false, TP_FETCH_PH);
				break;
			case ID_CLEAR_TRACE_POINT_PHYSICAL:
			case ID_ENABLE_TRACE_POINT_PHYSICAL:
			case ID_DISABLE_TRACE_POINT_PHYSICAL:
				UsageChangeBreakPoint(false, TP_FETCH_PH);
				break;
#endif
			case ID_SET_TRACE_POINT_FOR_MEMRD:
			case ID_SET_TRACE_POINT_FOR_MEMWR:
				UsageSetBreakPoint(false, TP_MEMRD);
				break;
			case ID_CLEAR_TRACE_POINT_FOR_MEMRD:
			case ID_ENABLE_TRACE_POINT_FOR_MEMRD:
			case ID_DISABLE_TRACE_POINT_FOR_MEMRD:
			case ID_CLEAR_TRACE_POINT_FOR_MEMWR:
			case ID_ENABLE_TRACE_POINT_FOR_MEMWR:
			case ID_DISABLE_TRACE_POINT_FOR_MEMWR:
				UsageChangeBreakPoint(false, TP_MEMRD);
				break;
//			case ID_LIST_TRACE_POINT_FOR_MEM:
//				UsageListBreakPoint(false, TP_MEMRD);
//				break;
#ifdef USE_BREAKPOINT_PHYSICAL
			case ID_SET_TRACE_POINT_FOR_MEMRD_PHYSICAL:
			case ID_SET_TRACE_POINT_FOR_MEMWR_PHYSICAL:
				UsageSetBreakPoint(false, TP_MEMRD_PH);
				break;
			case ID_CLEAR_TRACE_POINT_FOR_MEMRD_PHYSICAL:
			case ID_ENABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL:
			case ID_DISABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL:
			case ID_CLEAR_TRACE_POINT_FOR_MEMWR_PHYSICAL:
			case ID_ENABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL:
			case ID_DISABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL:
				UsageChangeBreakPoint(false, TP_MEMRD_PH);
				break;
#endif
#ifdef USE_IOPORT_ACCESS
			case ID_SET_TRACE_POINT_FOR_IORD:
			case ID_SET_TRACE_POINT_FOR_IOWR:
				UsageSetBreakPoint(false, TP_IORD);
				break;
			case ID_CLEAR_TRACE_POINT_FOR_IORD:
			case ID_ENABLE_TRACE_POINT_FOR_IORD:
			case ID_DISABLE_TRACE_POINT_FOR_IORD:
			case ID_CLEAR_TRACE_POINT_FOR_IOWR:
			case ID_ENABLE_TRACE_POINT_FOR_IOWR:
			case ID_DISABLE_TRACE_POINT_FOR_IOWR:
				UsageChangeBreakPoint(false, TP_IORD);
				break;
//			case ID_LIST_TRACE_POINT_FOR_IO:
//				UsageListBreakPoint(false, TP_IORD);
//				break;
#endif
			case ID_SET_TRACE_POINT_FOR_INTR:
				UsageSetBreakPoint(false, TP_INTR);
				break;
			case ID_CLEAR_TRACE_POINT_FOR_INTR:
			case ID_ENABLE_TRACE_POINT_FOR_INTR:
			case ID_DISABLE_TRACE_POINT_FOR_INTR:
				UsageChangeBreakPoint(false, TP_INTR);
				break;
//			case ID_LIST_TRACE_POINT_FOR_INTR:
//				UsageListBreakPoint(false, TP_INTR);
//				break;
#ifdef USE_EXCEPTION_BREAKPOINT
			case ID_SET_TRACE_POINT_FOR_EXCEPT:
				UsageSetBreakPoint(false, TP_EXCEPT);
				break;
			case ID_CLEAR_TRACE_POINT_FOR_EXCEPT:
			case ID_ENABLE_TRACE_POINT_FOR_EXCEPT:
			case ID_DISABLE_TRACE_POINT_FOR_EXCEPT:
				UsageChangeBreakPoint(false, TP_EXCEPT);
				break;
#endif
			case ID_OUTPUT_LOG_FILE:
				UsageOutputLogFile(false);
				break;
			case ID_SEND_TO_EMULATOR:
				UsageSendToEmulator(false);
				break;
#ifdef USE_EMU_INHERENT_SPEC
			case ID_BASIC:
				UsageDebugBasic(false, -1);
				break;
#endif
			case ID_PWD:
				UsagePWD(false);
				break;
			case ID_CHD:
				UsageCHD(false);
				break;
			case ID_DIR:
				UsageDIR(false);
				break;
			case ID_SWITCH_CPU:
				UsageSwitchCPU(false);
				break;
			case ID_QUIT:
				UsageQuit(false);
				break;
			default:
				Usage();
				break;
			}
		} else {
			Usage();
		}
	} else {
		Usage();
	}
}

void DebuggerConsole::PrintPrompt()
{
	Print(storage->GetCurrentCPUName(), false);
	Print(DEBUGGER_PROMPT, false);
	Flush();
}

void DebuggerConsole::Process()
{
	TraceFirst();

	while(!dp->request_terminate) {
		// prompt
		PrintPrompt();

		// get command
		bool enter_done = false;
		while(!dp->request_terminate && !enter_done) {
			enter_done = ReadInput();
			CDelay(10);
		}
		// current cpu is enable?
		SwitchCPU(GetCPUIndexEnabled());

		// process command
		if(!dp->request_terminate && enter_done) {
			ParseInput();
			int idx = -1;
			for(int i=0; commands_map[i].cmd != NULL; i++) {
				if (_tcsicmp(params[0], commands_map[i].cmd) == 0) {
					idx = i;
					break;
				}
			}
			if (idx >= 0) {
				switch(commands_map[idx].idx) {
				case ID_DUMP_MEMORY:
					CommandDumpMemory(0);
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_DUMP_PHYSICAL_MEMORY:
					CommandDumpMemory(1);
					break;
#endif
				case ID_EDIT_MEMORY_BYTE:
					CommandEditMemoryBinary(1, 0);
					break;
				case ID_EDIT_MEMORY_WORD:
					CommandEditMemoryBinary(2, 0);
					break;
				case ID_EDIT_MEMORY_DWORD:
					CommandEditMemoryBinary(4, 0);
					break;
				case ID_EDIT_MEMORY_ASCII:
					CommandEditMemoryAscii(0);
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_EDIT_PHYSICAL_MEMORY_BYTE:
					CommandEditMemoryBinary(1, 1);
					break;
				case ID_EDIT_PHYSICAL_MEMORY_WORD:
					CommandEditMemoryBinary(2, 1);
					break;
				case ID_EDIT_PHYSICAL_MEMORY_DWORD:
					CommandEditMemoryBinary(4, 1);
					break;
				case ID_EDIT_PHYSICAL_MEMORY_ASCII:
					CommandEditMemoryAscii(1);
					break;
#endif
#ifdef USE_IOPORT_ACCESS
				case ID_INPUT_IO_BYTE:
					CommandInputIOBinary(1);
					break;
				case ID_INPUT_IO_WORD:
					CommandInputIOBinary(2);
					break;
				case ID_INPUT_IO_DWORD:
					CommandInputIOBinary(4);
					break;
				case ID_OUTPUT_IO_BYTE:
					CommandOutputIOBinary(1);
					break;
				case ID_OUTPUT_IO_WORD:
					CommandOutputIOBinary(2);
					break;
				case ID_OUTPUT_IO_DWORD:
					CommandOutputIOBinary(4);
					break;
#endif
				case ID_ACCESS_REGISTER:
					CommandAccessRegister();
					break;
				case ID_ACCESS_DEVICE:
					CommandAccessDevice();
					break;
				case ID_SHOW_REGISTER:
					CommandToggleRegister(1);
					break;
				case ID_HIDE_REGISTER:
					CommandToggleRegister(0);
					break;
				case ID_SHOW_MEMORY_MAP:
					CommandShowMemoryMap();
					break;
#ifdef _MBS1
				case ID_SHOW_ADDRESS_MAP:
					CommandShowAddressMap();
					break;
				case ID_EDIT_ADDRESS_MAP:
					CommandEditAddressMap();
					break;
				case ID_SHOW_MEMORY_SPACE_MAP:
					CommandShowMemorySpaceMap();
					break;
				case ID_EDIT_MEMORY_SPACE_MAP:
					CommandEditMemorySpaceMap();
					break;
#endif
				case ID_SEARCH_BYTE:
					CommandSearch(1, 0);
					break;
				case ID_SEARCH_WORD:
					CommandSearch(2, 0);
					break;
				case ID_SEARCH_DWORD:
					CommandSearch(4, 0);
					break;
				case ID_SEARCH_ASCII:
					CommandSearchAscii(0);
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_SEARCH_PHYSICAL_BYTE:
					CommandSearch(1, 1);
					break;
				case ID_SEARCH_PHYSICAL_WORD:
					CommandSearch(2, 1);
					break;
				case ID_SEARCH_PHYSICAL_DWORD:
					CommandSearch(4, 1);
					break;
				case ID_SEARCH_PHYSICAL_ASCII:
					CommandSearchAscii(1);
					break;
#endif
				case ID_UNASSEMBLE:
					CommandUnassemble(0);
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_UNASSEMBLE_PHYSICAL:
					CommandUnassemble(1);
					break;
#endif
				case ID_CALC_HEXA:
					CommandCalculateHexa();
					break;
				case ID_SET_FILE_NAME:
					CommandSetFileName();
					break;
				case ID_LOAD_BINARY_FILE:
					CommandLoadBinaryFile(0);
					break;
				case ID_SAVE_BINARY_FILE:
					CommandSaveBinaryFile(0);
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_LOAD_BINARY_FILE_PHYSICAL:
					CommandLoadBinaryFile(1);
					break;
				case ID_SAVE_BINARY_FILE_PHYSICAL:
					CommandSaveBinaryFile(1);
					break;
#endif
#ifdef USE_EMU_INHERENT_SPEC
				case ID_SAVE_IMAGE_FILE:
					CommandSaveImageFile(0);
					break;
#endif
				case ID_SET_BREAK_POINT:
					CommandSetBreakPoint(BP_FETCH);
					break;
				case ID_CLEAR_BREAK_POINT:
					CommandClearBreakPoint(BP_FETCH);
					break;
				case ID_DISABLE_BREAK_POINT:
				case ID_ENABLE_BREAK_POINT:
					CommandChangeBreakPoint(BP_FETCH);
					break;
				case ID_LIST_BREAK_POINT:
					CommandListBreakPoint(BP_FETCH, true);
					break;
#ifdef USE_BREAKPOINT_PHYSICAL
				case ID_SET_BREAK_POINT_PHYSICAL:
					CommandSetBreakPoint(BP_FETCH_PH);
					break;
				case ID_CLEAR_BREAK_POINT_PHYSICAL:
					CommandClearBreakPoint(BP_FETCH_PH);
					break;
				case ID_DISABLE_BREAK_POINT_PHYSICAL:
				case ID_ENABLE_BREAK_POINT_PHYSICAL:
					CommandChangeBreakPoint(BP_FETCH_PH);
					break;
#endif
				case ID_SET_BREAK_POINT_FOR_MEMRD:
					CommandSetBreakPoint(BP_MEMRD);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_MEMRD:
					CommandClearBreakPoint(BP_MEMRD);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_MEMRD:
				case ID_ENABLE_BREAK_POINT_FOR_MEMRD:
					CommandChangeBreakPoint(BP_MEMRD);
					break;
				case ID_SET_BREAK_POINT_FOR_MEMWR:
					CommandSetBreakPoint(BP_MEMWR);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_MEMWR:
					CommandClearBreakPoint(BP_MEMWR);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_MEMWR:
				case ID_ENABLE_BREAK_POINT_FOR_MEMWR:
					CommandChangeBreakPoint(BP_MEMWR);
					break;
//				case ID_LIST_BREAK_POINT_FOR_MEM:
//					CommandListBreakPoint(BP_MEMRD);
//					break;
#ifdef USE_BREAKPOINT_PHYSICAL
				case ID_SET_BREAK_POINT_FOR_MEMRD_PHYSICAL:
					CommandSetBreakPoint(BP_MEMRD_PH);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_MEMRD_PHYSICAL:
					CommandClearBreakPoint(BP_MEMRD_PH);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL:
				case ID_ENABLE_BREAK_POINT_FOR_MEMRD_PHYSICAL:
					CommandChangeBreakPoint(BP_MEMRD_PH);
					break;
				case ID_SET_BREAK_POINT_FOR_MEMWR_PHYSICAL:
					CommandSetBreakPoint(BP_MEMWR_PH);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_MEMWR_PHYSICAL:
					CommandClearBreakPoint(BP_MEMWR_PH);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL:
				case ID_ENABLE_BREAK_POINT_FOR_MEMWR_PHYSICAL:
					CommandChangeBreakPoint(BP_MEMWR_PH);
					break;
#endif
#ifdef USE_IOPORT_ACCESS
				case ID_SET_BREAK_POINT_FOR_IORD:
					CommandSetBreakPoint(BP_IORD);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_IORD:
					CommandClearBreakPoint(BP_IORD);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_IORD:
				case ID_ENABLE_BREAK_POINT_FOR_IORD:
					CommandChangeBreakPoint(BP_IORD);
					break;
				case ID_SET_BREAK_POINT_FOR_IOWR:
					CommandSetBreakPoint(BP_IOWR);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_IOWR:
					CommandClearBreakPoint(BP_IOWR);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_IOWR:
				case ID_ENABLE_BREAK_POINT_FOR_IOWR:
					CommandChangeBreakPoint(BP_IOWR);
					break;
//				case ID_LIST_BREAK_POINT_FOR_IO:
//					CommandListBreakPoint(BP_IORD);
//					break;
#endif
				case ID_SET_BREAK_POINT_FOR_INTR:
					CommandSetBreakPoint(BP_INTR);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_INTR:
					CommandClearBreakPoint(BP_INTR);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_INTR:
				case ID_ENABLE_BREAK_POINT_FOR_INTR:
					CommandChangeBreakPoint(BP_INTR);
					break;
//				case ID_LIST_BREAK_POINT_FOR_INTR:
//					CommandListBreakPoint(BP_INTR);
//					break;
#ifdef USE_EXCEPTION_BREAKPOINT
				case ID_SET_BREAK_POINT_FOR_EXCEPT:
					CommandSetBreakPoint(BP_EXCEPT);
					break;
				case ID_CLEAR_BREAK_POINT_FOR_EXCEPT:
					CommandClearBreakPoint(BP_EXCEPT);
					break;
				case ID_DISABLE_BREAK_POINT_FOR_EXCEPT:
				case ID_ENABLE_BREAK_POINT_FOR_EXCEPT:
					CommandChangeBreakPoint(BP_EXCEPT);
					break;
#endif
				case ID_SET_TRACE_POINT:
					CommandSetBreakPoint(TP_FETCH);
					break;
				case ID_CLEAR_TRACE_POINT:
					CommandClearBreakPoint(TP_FETCH);
					break;
				case ID_DISABLE_TRACE_POINT:
				case ID_ENABLE_TRACE_POINT:
					CommandChangeBreakPoint(TP_FETCH);
					break;
				case ID_LIST_TRACE_POINT:
					CommandListBreakPoint(TP_FETCH, true);
					break;
#ifdef USE_BREAKPOINT_PHYSICAL
				case ID_SET_TRACE_POINT_PHYSICAL:
					CommandSetBreakPoint(TP_FETCH_PH);
					break;
				case ID_CLEAR_TRACE_POINT_PHYSICAL:
					CommandClearBreakPoint(TP_FETCH_PH);
					break;
				case ID_DISABLE_TRACE_POINT_PHYSICAL:
				case ID_ENABLE_TRACE_POINT_PHYSICAL:
					CommandChangeBreakPoint(TP_FETCH_PH);
					break;
#endif
				case ID_SET_TRACE_POINT_FOR_MEMRD:
					CommandSetBreakPoint(TP_MEMRD);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_MEMRD:
					CommandClearBreakPoint(TP_MEMRD);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_MEMRD:
				case ID_ENABLE_TRACE_POINT_FOR_MEMRD:
					CommandChangeBreakPoint(TP_MEMRD);
					break;
				case ID_SET_TRACE_POINT_FOR_MEMWR:
					CommandSetBreakPoint(TP_MEMWR);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_MEMWR:
					CommandClearBreakPoint(TP_MEMWR);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_MEMWR:
				case ID_ENABLE_TRACE_POINT_FOR_MEMWR:
					CommandChangeBreakPoint(TP_MEMWR);
					break;
//				case ID_LIST_TRACE_POINT_FOR_MEM:
//					CommandListBreakPoint(TP_MEMRD);
//					break;
#ifdef USE_BREAKPOINT_PHYSICAL
				case ID_SET_TRACE_POINT_FOR_MEMRD_PHYSICAL:
					CommandSetBreakPoint(TP_MEMRD_PH);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_MEMRD_PHYSICAL:
					CommandClearBreakPoint(TP_MEMRD_PH);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL:
				case ID_ENABLE_TRACE_POINT_FOR_MEMRD_PHYSICAL:
					CommandChangeBreakPoint(TP_MEMRD_PH);
					break;
				case ID_SET_TRACE_POINT_FOR_MEMWR_PHYSICAL:
					CommandSetBreakPoint(TP_MEMWR_PH);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_MEMWR_PHYSICAL:
					CommandClearBreakPoint(TP_MEMWR_PH);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL:
				case ID_ENABLE_TRACE_POINT_FOR_MEMWR_PHYSICAL:
					CommandChangeBreakPoint(TP_MEMWR_PH);
					break;
#endif
#ifdef USE_IOPORT_ACCESS
				case ID_SET_TRACE_POINT_FOR_IORD:
					CommandSetBreakPoint(TP_IORD);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_IORD:
					CommandClearBreakPoint(TP_IORD);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_IORD:
				case ID_ENABLE_TRACE_POINT_FOR_IORD:
					CommandChangeBreakPoint(TP_IORD);
					break;
				case ID_SET_TRACE_POINT_FOR_IOWR:
					CommandSetBreakPoint(TP_IOWR);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_IOWR:
					CommandClearBreakPoint(TP_IOWR);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_IOWR:
				case ID_ENABLE_TRACE_POINT_FOR_IOWR:
					CommandChangeBreakPoint(TP_IOWR);
					break;
//				case ID_LIST_TRACE_POINT_FOR_IO:
//					CommandListBreakPoint(TP_IORD);
//					break;
#endif
				case ID_SET_TRACE_POINT_FOR_INTR:
					CommandSetBreakPoint(TP_INTR);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_INTR:
					CommandClearBreakPoint(TP_INTR);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_INTR:
				case ID_ENABLE_TRACE_POINT_FOR_INTR:
					CommandChangeBreakPoint(TP_INTR);
					break;
//				case ID_LIST_TRACE_POINT_FOR_INTR:
//					CommandListBreakPoint(TP_INTR);
//					break;
#ifdef USE_EXCEPTION_BREAKPOINT
				case ID_SET_TRACE_POINT_FOR_EXCEPT:
					CommandSetBreakPoint(TP_EXCEPT);
					break;
				case ID_CLEAR_TRACE_POINT_FOR_EXCEPT:
					CommandClearBreakPoint(TP_EXCEPT);
					break;
				case ID_DISABLE_TRACE_POINT_FOR_EXCEPT:
				case ID_ENABLE_TRACE_POINT_FOR_EXCEPT:
					CommandChangeBreakPoint(TP_EXCEPT);
					break;
#endif
				case ID_SET_SYMBOL:
					CommandSetSymbol();
					break;
				case ID_CLEAR_SYMBOL:
					CommandClearSymbol();
					break;
				case ID_LOAD_SYMBOL:
					CommandLoadSymbol();
					break;
				case ID_EXECUTE:
					CommandExecute();
					break;
				case ID_TRACE:
					CommandTrace();
					break;
				case ID_TRACE_BACK:
					CommandTraceBack();
					break;
				case ID_SHOW_CLOCK:
					CommandShowClock();
					break;
				case ID_QUIT:
					CommandQuit();
					break;
				case ID_OUTPUT_LOG_FILE:
					CommandOutputLogFile();
					break;
				case ID_SEND_TO_EMULATOR:
					CommandSendToEmulator();
					break;
#ifdef USE_EMU_INHERENT_SPEC
				case ID_BASIC:
					CommandDebugBasic();
					break;
#endif
				case ID_PWD:
					CommandPWD();
					break;
				case ID_CHD:
					CommandCHD();
					break;
				case ID_DIR:
					CommandDIR();
					break;
				case ID_SWITCH_CPU:
					CommandSwitchCPU();
					break;
				case ID_USAGE:
					CommandUsage();
					break;
				default:
					Print(_T("Unsupported command: "), false);
					Print(params[0]);
					break;
				}
			} else {
				Print(_T("Unknown command: "), false);
				Print(params[0]);
			}
			ClearInput();
		}
	}

	// stop debugger
	try {
		for(int i=0; i<DC_MAX_CPUS; i++) {
			struct devs_st *dev = &devs[i];
			if (dev->debugger) {
				dev->debugger->stop_debugging();
			}
		}
	} catch(...) {
	}
}

// ----------------------------------------------------------------------------

void EMU::EMU_DEBUGGER()
{
	hDebuggerThread = NULL;
	now_debugging = false;

	debugger_storage = NULL;
	debugger_socket = NULL;
}

void EMU::initialize_debugger()
{
	debugger_storage = new DebuggerStorage();
#ifdef USE_TELNET_SERVER
	debugger_socket = new DebuggerSocket(this, NULL);
#endif
	now_debugging = false;
}

void EMU::release_debugger()
{
	close_debugger();
#ifdef USE_TELNET_SERVER
	delete debugger_socket;
	debugger_socket = NULL;
#endif
	delete debugger_storage;
	debugger_storage = NULL;
}

void EMU::open_debugger()
{
	if(!now_debugging) {
		close_debugger();
		int num_of_cpus = vm->get_cpus();

		// check cpus
		bool ok = true;
		for(int i=0; i<num_of_cpus; i++) {
			ok = (ok && vm->get_cpu(i) != NULL && vm->get_cpu(i)->get_debugger() != NULL);
		}

		if (ok) {

			debugger_thread_param.emu = this;
			debugger_thread_param.vm = vm;
			debugger_thread_param.cpu_index = -1;
			debugger_thread_param.num_of_cpus = num_of_cpus;
			debugger_thread_param.request_terminate = false;

			hDebuggerThread = new DebuggerThread(&debugger_thread_param);
			hDebuggerThread->start();
			this->sleep(100);
			if (hDebuggerThread->isRunning())
			{
				stop_rec_sound();
				stop_rec_video();
#ifdef USE_TELNET_SERVER
				// acceptable from telnet terminal
				debugger_socket->enable_server(true);
#endif
				now_debugging = true;

				logging->out_log_x(LOG_INFO, CMsg::Debugger_was_started);
			} else {
				logging->out_log_x(LOG_ERROR, CMsg::Cannot_start_debugger);
			}
		}
	}
}

#if 0
void EMU::open_debugger(int cpu_index)
{
	if(!(now_debugging && debugger_thread_param.cpu_index == cpu_index)) {
		close_debugger();
		if(vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL) {
			debugger_thread_param.emu = this;
			debugger_thread_param.vm = vm;
			debugger_thread_param.cpu_index = cpu_index;
			debugger_thread_param.num_of_cpus = 1;
			debugger_thread_param.request_terminate = false;

			hDebuggerThread = new DebuggerThread(&debugger_thread_param);
			hDebuggerThread->start();
			this->sleep(100);
			if (hDebuggerThread->isRunning())
			{
				stop_rec_sound();
				stop_rec_video();
#ifdef USE_TELNET_SERVER
				// acceptable from telnet terminal
				debugger_socket->enable_server(true);
#endif
				now_debugging = true;

				logging->out_log_x(LOG_INFO, CMsg::Debugger_was_started);
			} else {
				logging->out_log_x(LOG_ERROR, CMsg::Cannot_start_debugger);
			}
		}
	}
}
#endif

void EMU::close_debugger()
{
	if(now_debugging) {
		if(debugger_thread_param.running) {
			debugger_thread_param.request_terminate = true;

			hDebuggerThread->wait();
		}
#ifdef USE_TELNET_SERVER
		_TCHAR data[32];
		// back to default color
		UTILITY::tcscpy(data, 32, _T("\x1b[39m\x1b[49m"));
		debugger_socket->write_data(data, (int)_tcslen(data));
		// disconnect
		debugger_socket->enable_server(false);
#endif

		delete hDebuggerThread;
		hDebuggerThread = NULL;

		now_debugging = false;

		logging->out_log_x(LOG_INFO, CMsg::Debugger_was_stopped);
	}
}

bool EMU::debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

void EMU::debugger_terminal_accepted()
{
#ifdef USE_TELNET_SERVER
	// send telnet command
	_TCHAR data[128];
	// telnet options
	UTILITY::tcscpy(data, sizeof(data) / sizeof(_TCHAR), _T("\xff\xfb\x03"));	// WILL Suppress Go Ahead
	UTILITY::tcscat(data, sizeof(data) / sizeof(_TCHAR), _T("\xff\xfb\x01"));	// WILL Echo
	debugger_socket->write_data(data, (int)_tcslen(data));
	// send color code
	UTILITY::tcscpy(data, sizeof(data) / sizeof(_TCHAR), _T("\x1b[97m\x1b[40m"));
	debugger_socket->write_data(data, (int)_tcslen(data));
	// message
	UTILITY::stprintf(data, sizeof(data) / sizeof(_TCHAR), _T("Connected to %s debugger. Type ? to see help.\n"), _T(CLASS_NAME));
	debugger_socket->write_data(data, (int)_tcslen(data));
	// prompt
	if (debugger_storage->NowRunning()) {
		UTILITY::tcscpy(data, sizeof(data) / sizeof(_TCHAR), _T("Now running. Press esc key to stop this.\n"));
	} else {
		UTILITY::concat(data, sizeof(data) / sizeof(_TCHAR), debugger_storage->GetCurrentCPUName(), DEBUGGER_PROMPT, NULL);
	}
	debugger_socket->write_data(data, (int)_tcslen(data));
#endif
}

#include "../video/rec_video.h"

bool EMU::debugger_save_image(int width, int height, CSurface *surface)
{
	VmRectWH size = {0, 0, width, height};

	return rec_video->Capture(SAVE_IMAGE_TYPE, size, surface, size);
}

#endif

