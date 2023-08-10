/** @file debugger_console.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.03.01 -

	@brief [ debugger console ]

	@note Original author is Takeda.Toshiya on 2006.08.18 -
*/

#ifndef DEBUGGER_CONSOLE_H
#define DEBUGGER_CONSOLE_H

#include "../vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "../emu.h"
#include "../vm/device.h"
#include "../vm/debugger.h"
//#include <fcntl.h>
//#include "res/resource.h"
//#include "fileio.h"
#include "../cchar.h"
#include "../cptrlist.h"
#if defined(USE_WIN)
#include "windows/win_debugger_console.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "SDL/sdl_debugger_console.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "wxwidgets/wxw_debugger_console.h"
#elif defined(USE_QT)
#include "qt/qt_debugger_console.h"
#endif

#ifdef _WIN32
#define USE_CONSOLE_WINDOW 1
#endif

#define USE_TELNET_SERVER 1

#undef USE_STANDARD_INOUT

#ifdef USE_CONSOLE_WINDOW
#include <windows.h>
#include <conio.h>
#include <io.h>
#endif

#ifdef USE_STANDARD_INOUT
#ifndef _WIN32
#include <sys/select.h>
#endif
#endif

#if defined(USE_WIN)
#include <process.h>
#endif

#if defined(_BML3MK5) || defined(_MBS1)
# if defined(_MBS1) && defined(USE_Z80B_CARD)
#define USE_IOPORT_ACCESS
# elif defined(_MBS1) && defined(USE_MPC_68008)
#define USE_EXCEPTION_BREAKPOINT
# else
#undef  USE_IOPORT_ACCESS
# endif
#else
#define USE_IOPORT_ACCESS
#define USE_EXCEPTION_BREAKPOINT
#endif

#if defined(_MBS1)
#define USE_BREAKPOINT_PHYSICAL
#endif

#define DC_MAX_COMMAND_LEN	128
#define DC_MAX_COMMAND_HISTORY	64
#define DC_MAX_BUFFER_LEN 8192

#define DC_MAX_CPUS 2

class FILEIO;
class DebuggerSocket;
class BreakPoints;
class BreakPoint;

///
/// @brief Remain input command history on debugger
///
class DebuggerStorage
{
private:
	CPtrList<CTchar> command_history;
//	int last_history_ptr;
	int history_ptr;

	CTchar current_cpu_name;

	bool now_running;	// now execute on "g" command

	int  FindHistory(const _TCHAR *command) const;
	int  GetHistory(int index, _TCHAR *command, size_t size) const;

public:
	DebuggerStorage();
	~DebuggerStorage();

	void AddHistory(const _TCHAR *command);
	bool PrevHistory();
	bool NextHistory();
	int  GetCurrentHistory(_TCHAR *command, size_t size) const;
//	bool IsHistoryEnabled(int index) const;

	void SetCurrentCPUName(const char *name);
	const _TCHAR *GetCurrentCPUName() const { return current_cpu_name; }

	void Running(bool val) { now_running = val; }
	bool NowRunning() const { return now_running; }
};

///
/// @brief Hash of charactor and keycode
///
class ConsoleKeyCode
{
private:
	_TCHAR  chr;
	uint8_t key;
public:
	ConsoleKeyCode();
	~ConsoleKeyCode();
	void Clear();
	void Set(_TCHAR chr_, uint8_t key_);
	void Get(_TCHAR &chr_, uint8_t &key_) const;
	_TCHAR Chr() const { return chr; }
	uint8_t Key() const { return key; }
};

#define DBG_KEY_BUFFER_SIZE 256

///
/// @brief Ring buffer on keying data
///
class ConsoleRingBuffer
{
private:
	ConsoleKeyCode keys[DBG_KEY_BUFFER_SIZE];
	int w_pos;
	int r_pos;
	_TCHAR prev_chr;

public:
	ConsoleRingBuffer();
	~ConsoleRingBuffer();
	void Clear();
	void Push(_TCHAR chr_, uint8_t key_);
	bool Pop(_TCHAR &chr, uint8_t &key);
};

///
/// @brief Command buffer on debugger console
///
class CommandBuffer
{
private:
	_TCHAR command[DC_MAX_COMMAND_LEN + 1];
	_TCHAR prev_command[DC_MAX_COMMAND_LEN + 1];
	int commandptr;
	int commandright;

	void Add(_TCHAR chr_);
	void RShift();
	void LShift();
	int Trim(_TCHAR *str, int len);
public:
	CommandBuffer();
	~CommandBuffer();
	void Clear();
	void ClearCurr();
	void Insert(_TCHAR chr_);
	_TCHAR *Ptr();
	const _TCHAR *Ptr() const;
	const _TCHAR *CurrPtr() const;
	const _TCHAR *NextPtr() const;
	int CurrPos() const;
	int LastLen() const;
	int Len() const;
	void SetLen(int val);
	int Size() const;
	_TCHAR Left();
	_TCHAR Right();
	void Tail();
	void BackSpace();
	void CopyFromPrev();
	void Decide();
	void ShrinkPrev(int pos);
	bool IsFull() const;
	bool IsEmpty() const;
	bool IsTail() const;
	bool IsRepeat() const;
};

///
/// @brief Console on debugger
///
class DebuggerConsole
{
public:
	enum enTextColor {
		Black = 0,
		Blue,
		Red,
		Magenta,
		Green,
		Cyan,
		Yellow,
		White,
		EndColor
	};

private:
	debugger_thread_t *dp;
	int num_of_cpus;

	struct devs_st {
		int index;

		DEVICE *cpu;
		DEVICE *mem;
		DEBUGGER *debugger;

		uint32_t prog_addr_mask;
		uint32_t data_addr_mask;

		int phys_type;
		uint32_t dump_addr;
		uint32_t dump_phys_addr;

		uint32_t dasm_addr;
		uint32_t dasm_phys_addr;

		bool show_regs;

		struct devs_st *next;

		void Clear();
	} devs[DC_MAX_CPUS];

	struct devs_st *current_dev;

#ifdef USE_TELNET_SERVER
	DebuggerSocket *telnet;
#endif
#ifdef USE_CONSOLE_WINDOW
	HANDLE hStdIn;
	HANDLE hStdOut;
#endif
	bool cp932;
	bool echo_bak;

	FILEIO* logfile;
	_TCHAR logfilename[_MAX_PATH];

	ConsoleRingBuffer key_buffer;

	_TCHAR buffer[DC_MAX_BUFFER_LEN];

	CommandBuffer command_buffer;
//	_TCHAR command[DC_MAX_COMMAND_LEN + 1];
//	_TCHAR prev_command[DC_MAX_COMMAND_LEN + 1];
//	int commandptr;
//	int commandright;

	DebuggerStorage *storage;

//	int history_ptr;

	_TCHAR *params[32];
	int paramnum;

	uint8_t key_stat_tmp[KEY_STATUS_SIZE];

	enTextColor current_color;

private:
	enum enBpTpType {
		BP_UNKNOWN = -1,

		BREAKPOINT = 0x00,
		BP_FETCH = 0x00,
		BP_MEMRD = 0x01,
		BP_MEMWR = 0x02,
		BP_IORD = 0x03,
		BP_IOWR = 0x04,
		BP_INTR = 0x05,
		BP_EXCEPT = 0x06,
		BP_FETCH_PH = 0x08,
		BP_MEMRD_PH = 0x09,
		BP_MEMWR_PH = 0x0a,
		BP_BASIC = 0x0f,

		TRACEPOINT = 0x10,
		TP_FETCH = 0x10,
		TP_MEMRD = 0x11,
		TP_MEMWR = 0x12,
		TP_IORD = 0x13,
		TP_IOWR = 0x14,
		TP_INTR = 0x15,
		TP_EXCEPT = 0x16,
		TP_FETCH_PH = 0x18,
		TP_MEMRD_PH = 0x19,
		TP_MEMWR_PH = 0x1a,
		TP_BASIC = 0x1f,
	};

private:
	uint32_t DeciToInt(const _TCHAR *str);
	uint32_t HexaToInt(const _TCHAR *str);
	uint8_t  HexaToByte(const char *value) const;
	uint16_t HexaToWord(const char *value) const;
	uint32_t HexaToUInt(const char *value, int len) const;
	bool IsHexa(const _TCHAR *str) const;
	bool HexaToInt(const _TCHAR *str, uint32_t *data) const;
	bool HexaToInt(const _TCHAR *str, uint32_t *addr, int *len);
	bool DeciToNum(const _TCHAR *str, int &st_line, int &ed_line);
	BreakPoints *GetBreakPoints(int num);
//	bool IsConsoleActive();

	bool SetCommandLine(_TCHAR chr, uint8_t key, bool echo);

	bool ReadInput();
	int ParseInput();
	void ClearInput();
	bool EscapePressed();
	bool NowDisable() const;
	bool NowPausing() const;

	int GetAddressType(int num);
	void UsageAddressType(int num);

	void CommandDumpMemory(int num);
	void UsageDumpMemory(bool s, int num);
	void CommandEditMemoryBinary(int n, int num);
	void CommandEditMemoryAscii(int num);
	void UsageEditMemory(bool s, int num);
	void CommandInputIOBinary(int n);
	void CommandOutputIOBinary(int n);
	void UsageEditIO(bool s);
	void CommandAccessRegister();
	void UsageAccessRegister(bool s);
	void CommandAccessDevice();
	void UsageAccessDevice(bool s);
	void CommandToggleRegister(int n);
	void UsageToggleRegister(bool s);
#ifdef USE_EMU_INHERENT_SPEC
	void CommandShowMemoryMap();
	void UsageShowMemoryMap(bool s);
#endif
#ifdef _MBS1
	void CommandShowAddressMap();
	void UsageShowAddressMap(bool s);
	void CommandEditAddressMap();
	void UsageEditAddressMap(bool s);

	void CommandShowMemorySpaceMap();
	void UsageShowMemorySpaceMap(bool s);
	void CommandEditMemorySpaceMap();
	void UsageEditMemorySpaceMap(bool s);
#endif
	void CommandSearch(int n, int num);
	void CommandSearchAscii(int num);
	void UsageSearch(bool s, int num);
	void CommandUnassemble(int num);
	void UsageUnassemble(bool s, int num);

	void CommandCalculateHexa();
	void UsageCalculateHexa(bool s);
	void CommandSetFileName();
	void UsageSetFileName(bool s);
	void CommandLoadBinaryFile(int num);
	void CommandSaveBinaryFile(int num);
	bool LoadBinaryRawFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type);
	bool LoadIntelHexFile(const _TCHAR *file_path, uint32_t offset, uint32_t addr_mask, int type);
	bool LoadMotorolaSFile(const _TCHAR *file_path, uint32_t offset, uint32_t addr_mask, int type);
	bool SaveBinaryRawFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type);
	bool SaveIntelHexFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type);
	bool SaveMotorolaSFile(const _TCHAR *file_path, uint32_t start_addr, uint32_t end_addr, uint32_t addr_mask, int type);
	void UsageBinaryFile(bool s, int num);
	void UsageBinaryFileType();
	void CommandSaveImageFile(int num);
	void UsageImageFile(bool s, int num);

	void CommandSetBreakPoint(int num);
	int  SetBreakPoint(BreakPoints *bps, int cat, int num, const _TCHAR * const *ps, int pn);
	void UsageSetBreakPoint(bool s, int num);
	bool ParseExpression(const _TCHAR * const *ps, int pn, _TCHAR *regname, void * &regptr, int &reglen, uint32_t &regval);
	bool ParseExpressionMain(const _TCHAR * const *ps, int pn, _TCHAR *regname, void * &regptr, int &reglen, uint32_t &regval);
	bool ParseRWValue(const _TCHAR * const *ps, int pn, int &reglen, uint32_t &regval, uint32_t &regmask);
	bool ParseRWValueMain(const _TCHAR * const *ps, int pn, int &reglen, uint32_t &regval, uint32_t &regmask);
	void PrintBreakPointInfo(int num, const BreakPoint *bp, int i, const _TCHAR *iname);
	void PrintBreakPointAddressInfo(int num, const BreakPoint *bp);
	void PrintBreakPointIoportInfo(int num, const BreakPoint *bp);
	void CommandClearBreakPoint(int num);
	int  ClearBreakPoint(BreakPoints *bps, int cat, int num, int pn, _TCHAR **ps);
	void CommandChangeBreakPoint(int num);
	int  ChangeBreakPoint(BreakPoints *bps, int cat, int num, int pn, _TCHAR **ps);
	void UsageChangeBreakPoint(bool s, int num);
	void CommandListBreakPoint(int num, bool all_list = false);
	int  ListBreakPoint(BreakPoints *bps, int cat, int num);
	void UsageListBreakPoint(bool s, int num);

	void CommandSetSymbol();
	void UsageSetSymbol(bool s);
	int  SetSymbol(uint32_t addr, const _TCHAR *label);
	int  ListSymbol();
	void CommandClearSymbol();
	void UsageClearSymbol(bool s);
	int  ClearSymbol(int pn, _TCHAR **ps);

	void CommandLoadSymbol();
	void UsageLoadSymbol(bool s);
	int  LoadSymbol(const _TCHAR *file_path, int format);

	void CommandExecute();
	void UsageExecute(bool s);
	void PrintBreakedReason(uint32_t reason);
	void PrintBreakedDevice();
	void CommandTrace();
	void UsageTrace(bool s);
	void CommandTraceBack();
	void UsageTraceBack(bool s);

	void CommandShowClock();
	void UsageShowClock(bool s);

	void CommandSwitchCPU();
	void UsageSwitchCPU(bool s);

	bool SwitchCPU(int idx);

	void CommandPWD();
	void UsagePWD(bool s);
	void CommandCHD();
	void UsageCHD(bool s);
	void CommandDIR();
	void UsageDIR(bool s);
	void PWD();
	bool CHD(const _TCHAR *path);
	void DIR(const _TCHAR *path);

	void CommandQuit();
	void UsageQuit(bool s);

	void CommandOutputLogFile();
	void UsageOutputLogFile(bool s);

	void CommandSendToEmulator();
	void CommandSendResetToEmulator();
	void CommandSendKeyToEmulator();
	void UsageSendToEmulator(bool s);

#ifdef USE_EMU_INHERENT_SPEC
	void CommandDebugBasic();
	void CommandDebugBasicVariables(int pn, _TCHAR **ps);
	void CommandDebugBasicList(int pn, _TCHAR **ps);
	void CommandDebugBasicTraceOnOff(int pn, _TCHAR **ps, bool en);
	void CommandDebugBasicCommand(int pn, _TCHAR **ps);
	void CommandDebugBasicError(int pn, _TCHAR **ps);
	void CommandDebugBasicSetBreakPoint(int pn, _TCHAR **ps, int num);
	void CommandDebugBasicClearBreakPoint(int pn, _TCHAR **ps, int num);
	void CommandDebugBasicChangeBreakPoint(int pn, _TCHAR **ps, int num);
	void CommandDebugBasicTraceBack(int pn, _TCHAR **ps);
	void UsageDebugBasic(bool s, int num);
#endif

	void CommandUsage();
	void Usage();
	void UsageCmdStr1(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1);
	void UsageCmdStr2(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1
		, const _TCHAR *cmd2, const _TCHAR *args2, const _TCHAR *desc2);
	void UsageCmdStr3(bool s, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1
		, const _TCHAR *cmd2, const _TCHAR *args2, const _TCHAR *desc2
		, const _TCHAR *cmd3, const _TCHAR *args3, const _TCHAR *desc3);
	void UsageCmdStrN(bool s, ...);
	void UsageCmdStr(bool s, int cnts, const _TCHAR **cmds);
	int  SetStr1(int cnt, const _TCHAR **cmds, const _TCHAR *cmd1, const _TCHAR *args1, const _TCHAR *desc1);

	void TraceFirst();
	void TraceCurrent();
	void TraceNext();

	void SetKeyStatus();
	int  GetCPUIndexEnabled() const;

	void BasicTraceCurrent();

	int  GetCPUIndexHitBreakPoint() const;
	int  GetCPUIndexHitBreakOrTracePoint() const;
	inline bool NowBreakPointInDebugger() const;
	inline void ClearSuspendInDebugger(int step);

public:
	DebuggerConsole(debugger_thread_t *p);
	~DebuggerConsole();

	void SetTextColor(enTextColor color);
	void Printf(enTextColor color, const _TCHAR *format, ...);
	void Printf(const _TCHAR *format, ...);
	void Vprintf(const _TCHAR *format, va_list ap);
	void Print(enTextColor color, const _TCHAR *str, bool cr = true);
	void Print(const _TCHAR *str, bool cr = true);
	void PrintfError(const _TCHAR *format, ...);
	void PrintError(const _TCHAR *str, bool cr = true);
	void Out(enTextColor color, bool cr = true);
	void Out(bool cr = true);
	void PutCh(_TCHAR c);
	void PutChs(_TCHAR c, size_t len);
	void Puts(const _TCHAR *str, size_t len);
	void Cr();
	void Flush();

	void Process();
	void PrintPrompt();

	_TCHAR *GetBuffer(bool clear = false);
	int GetBufferSize() const { return DC_MAX_BUFFER_LEN; }
	const _TCHAR *GetParam(int idx);
	int GetParamNums() const { return paramnum; }
};

#endif
#endif

