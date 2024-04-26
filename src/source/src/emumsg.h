/** @file emumsg.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.01 -

	@brief [ emu message ]
*/

#ifndef EMUMSG_H
#define EMUMSG_H

#include "common.h"
#include "vm/vm_defs.h"

/// emulator thread messages
#define EMUMSG_MSG_BUFFER_SIZE	60

/// @ingroup Enums
/// Message type of EmuMsg
enum en_emumsg_msg {
	EMUMSG_MSG_NONE = 0,
	EMUMSG_MSG_COMMAND = 1,
	EMUMSG_MSG_DROPFILES,
	EMUMSG_MSG_UNKNOWN
};

/// @ingroup Enums
/// Message ids of EmuMsg
enum en_emumsg_id {
	EMUMSG_ID_NONE = 0,

	EMUMSG_ID_POWER_ON = 101,

	EMUMSG_ID_RESET,
	EMUMSG_ID_FORCE_RESET,
	EMUMSG_ID_SPECIAL_RESET,
	EMUMSG_ID_WARM_RESET,

	EMUMSG_ID_INTERRUPT,

	EMUMSG_ID_SYSTEM_PAUSE,
	EMUMSG_ID_PAUSE,

	EMUMSG_ID_CPU_POWER,

	EMUMSG_ID_SYNC_IRQ,

	EMUMSG_ID_AUTOKEY_OPEN,
	EMUMSG_ID_AUTOKEY_START,
	EMUMSG_ID_AUTOKEY_STOP,

	EMUMSG_ID_LOAD_STATE,
	EMUMSG_ID_SAVE_STATE,
	EMUMSG_ID_RECENT_STATE,
	EMUMSG_ID_RECKEY_PLAY,
	EMUMSG_ID_RECKEY_STOP_PLAY,
	EMUMSG_ID_RECKEY_REC,
	EMUMSG_ID_RECKEY_STOP_REC,

#ifdef USE_FD1
	EMUMSG_ID_OPEN_FD,
	EMUMSG_ID_CLOSE_FD,
	EMUMSG_ID_CHANGE_FD,
	EMUMSG_ID_WRITEPROTECT_FD,
	EMUMSG_ID_RECENT_FD,
	EMUMSG_ID_SELECT_D88_BANK,
#endif

#ifdef USE_HD1
	EMUMSG_ID_OPEN_HD,
	EMUMSG_ID_CLOSE_HD,
	EMUMSG_ID_WRITEPROTECT_HD,
	EMUMSG_ID_RECENT_HD,
#endif

#ifdef USE_DATAREC
	EMUMSG_ID_PLAY_DATAREC,
	EMUMSG_ID_REC_DATAREC,
	EMUMSG_ID_CLOSE_DATAREC,
	EMUMSG_ID_REWIND_DATAREC,
	EMUMSG_ID_FAST_FORWARD_DATAREC,
	EMUMSG_ID_STOP_DATAREC,
	EMUMSG_ID_REAL_DATAREC,
	EMUMSG_ID_RECENT_DATAREC,
#endif

	EMUMSG_ID_SCREEN_SCANLINE1,
	EMUMSG_ID_SCREEN_SCANLINE_A,
	EMUMSG_ID_SCREEN_AFTERIMAGE1,
	EMUMSG_ID_SCREEN_AFTERIMAGE_A,
	EMUMSG_ID_SCREEN_KEEPIMAGE1,
	EMUMSG_ID_SCREEN_KEEPIMAGE_A,

	EMUMSG_ID_SCREEN_START_RECORD,
	EMUMSG_ID_SCREEN_STOP_RECORD,
	EMUMSG_ID_SCREEN_RESIZE_RECORD,
	EMUMSG_ID_SCREEN_CAPTURE,

	EMUMSG_ID_SOUND_START_RECORD,
	EMUMSG_ID_SOUND_STOP_RECORD,

	EMUMSG_ID_PRINTER_SAVE,
	EMUMSG_ID_PRINTER_PRINT,
	EMUMSG_ID_PRINTER_CLEAR,
	EMUMSG_ID_PRINTER_DIRECT,
	EMUMSG_ID_PRINTER_ONLINE,

	EMUMSG_ID_COMM_SERVER,
	EMUMSG_ID_COMM_CONNECT,

#ifdef USE_CART1
	EMUMSG_ID_OPEN_CART,
	EMUMSG_ID_CLOSE_CART,
	EMUMSG_ID_RECENT_CART,
#endif

#ifdef USE_QD1
	EMUMSG_ID_OPEN_QD,
	EMUMSG_ID_CLOSE_QD,
	EMUMSG_ID_WRITEPROTECT_QD,
	EMUMSG_ID_RECENT_QD,
#endif

#ifdef USE_MEDIA
	EMUMSG_ID_OPEN_MEDIA,
	EMUMSG_ID_CLOSE_MEDIA,
	EMUMSG_ID_RECENT_MEDIA,
#endif

#ifdef USE_BINARY_FILE1
	EMUMSG_ID_LOAD_BINARY,
	EMUMSG_ID_SAVE_BINARY,
	EMUMSG_ID_CLOSE_BINARY,
	EMUMSG_ID_RECENT_BINARY,
#endif

#ifdef USE_PIAJOYSTICK
	EMUMSG_ID_MODIFY_JOYTYPE,
#endif

	EMUMSG_ID_UNKNOWN,
};

/// for message send/receive
class EmuMsgItem {
private:
	en_emumsg_msg msg;
	en_emumsg_id  id;

	int drv;
	int num;
	uint32_t flags;
	bool multi;
	bool sys_pause;
	_TCHAR *file1;

	void SetFile(const _TCHAR *new_file);

public:
	EmuMsgItem();
	~EmuMsgItem();
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_num);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file, bool new_sys_pause);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, int new_num);
	EmuMsgItem(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file);

	void Initialize(en_emumsg_msg new_msg, en_emumsg_id new_id);
	void Terminate();
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_num);
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file);
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, const _TCHAR *new_file, bool new_sys_pause);
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi);
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, int new_num);
	void Set(en_emumsg_msg new_msg, en_emumsg_id new_id, int new_drv, const _TCHAR *new_file);

	en_emumsg_msg GetMsg() { return msg; }
	en_emumsg_id GetId() { return id; }
	int GetDrv() { return drv; }
	int GetNum() { return num; }
	int GetBankNum() { return num; }
	uint32_t GetFlags() { return flags; }
	bool GetMulti() { return multi; }
	bool GetSysPause() { return sys_pause; }
	const _TCHAR *GetFile() { return file1; }
};

/// process message for emu thread
class EmuMsg {
private:
	int emumsg_push;
	int emumsg_pop;
	int emumsg_count;
	EmuMsgItem *stack[EMUMSG_MSG_BUFFER_SIZE];

	EmuMsgItem *Get();
	void Push(EmuMsgItem *new_item);
	void Pop();

public:
	EmuMsg();
	~EmuMsg();
	void Send(en_emumsg_id id);
	void Send(en_emumsg_id id, int new_num);
	void Send(en_emumsg_id id, const _TCHAR *new_file);
	void Send(en_emumsg_id id, const _TCHAR *new_file, bool new_sys_pause);
	void Send(en_emumsg_id id, int new_drv, const _TCHAR *new_file, int new_bank, uint32_t new_flags, bool new_multi);
	void Send(en_emumsg_id id, int new_drv, int new_num);
	void Send(en_emumsg_id id, int new_drv, const _TCHAR *new_file);
	int Process();
};

extern EmuMsg emumsg;

#endif /* EMUMSG_H */
