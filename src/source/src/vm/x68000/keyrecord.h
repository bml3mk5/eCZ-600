/** @file keyrecord.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22

	@brief [ key recording ]
*/

#ifndef KEYRECORD_H
#define KEYRECORD_H

#include "../vm_defs.h"
#include "../../common.h"
//#include "../../emu.h"
//#include "../device.h"
//#include "../../config.h"
//#include "../../fileio.h"

#ifdef _DEBUG
#define _DEBUG_KEYRECORD
#endif

class EMU;
class DEVICE;
class FILEIO;
class CRecentPath;

/// key recording cache
class KEYRECORD_CACHE
{
public:
	enum en_cache {
		CACHE_SIZE = 256
	};
	struct st_cache {
		uint64_t clk;
		int      cnt;
		int      code[5];
	};
private:
	struct st_cache m_caches[CACHE_SIZE];
	int m_type;
	int m_wpos;
	int m_rpos;
	int m_cnt;

public:
	KEYRECORD_CACHE();
	KEYRECORD_CACHE(int type);
	~KEYRECORD_CACHE();
	void Clear();
	void Set(uint64_t clk, int cnt, const int code[5]);
	struct st_cache *FindFirst(uint64_t now_clk);

	void SetType(int type) { m_type = type; }
	bool IsEmpty() const { return (m_cnt == 0); }
	bool IsFull() const { return (m_cnt >= (CACHE_SIZE - 1)); }
};

/// record a motion of key pressing/releasing
class KEYRECORD
{
private:
	EMU *emu;
	DEVICE *d_event;
//	DEVICE *d_keyboard;
//	DEVICE *d_mouse;

//	int *mouse_stat;

//	uint8_t *key_scan_code_ptr;
	int *counter_ptr;

	// key record
	FILEIO *fkro, *fkri;
	bool m_end_of_file;
//	uint8_t vm_syskey_recp_stat[256];
//	uint8_t vm_syskey_recr_stat[256];
	uint8_t vm_key_recp_stat[KEYBIND_KEYS];
	uint8_t vm_key_recr_stat[KEYBIND_KEYS];
#ifdef USE_LIGHTPEN
	int   lpen_recp_stat[3];
	int   lpen_recr_stat[3];
#endif
#ifdef USE_MOUSE
	int   mouse_recp_stat[3];
	int   mouse_recr_stat[3];
#endif
	uint32_t joypia_recp_stat[2][3];
	uint32_t joypia_recr_stat[2][3];
	char  rec_key_tmp_buff[512];
	char  rec_key_rec_buff[512];
	int64_t key_rec_sum_clock;

	KEYRECORD_CACHE m_syskey_cache;
	KEYRECORD_CACHE m_key_cache;
#ifdef USE_LIGHTPEN
	KEYRECORD_CACHE m_lpen_cache;
#endif
#ifdef USE_MOUSE
	KEYRECORD_CACHE m_mouse_cache;
#endif
#ifdef USE_PIAJOYSTICK
	KEYRECORD_CACHE m_joypia_cache[2];
#endif
#ifdef _DEBUG
	uint8_t vm_key_dbg_stat[KEYBIND_KEYS];
#endif

	void clear_play_buffer();
	void clear_record_buffer();

	bool set_relative_path(const char *key, const _TCHAR *base_path, CRecentPath &path);
	void get_file_path(const _TCHAR *base_path, _TCHAR **file_path, int *bank_num);

//	inline bool playing_keys(int code, bool pressed);
//	inline void recording_keys(int code, bool pressed);
	inline void playing_mouse_status(int *mstat);
	inline void recording_mouse_status(const int *mstat);
	inline void playing_joypia_status(int num, uint32_t *joystat, int size);
	inline void recording_joypia_status(int num, const uint32_t *joystat, int size);
#ifdef USE_LIGHTPEN
	inline void playing_lightpen_status(int *mstat);
	inline void recording_lightpen_status(const int *mstat);
#endif

public:
	KEYRECORD(EMU* parent_emu);
	~KEYRECORD();

	void read_to_cache();

//	void reading_keys(int num);

//	bool processing_keys(int code, bool pressed);
	void playing_key();
	void recording_key(int code, bool pressed);
	void playing_system_keys();
	void recording_system_keys(int code, bool pressed);
	void processing_mouse_status(int *mstat);
	void processing_joypia_status(int num, uint32_t *joystat, int size);
#ifdef USE_LIGHTPEN
	void processing_lightpen_status(int *mstat);
#endif

	bool play_reckey(const _TCHAR* filename);
	bool record_reckey(const _TCHAR* filename);
	void stop_reckey(bool stop_play = true, bool stop_record = true);

	void set_context_event(DEVICE* value) {
		d_event = value;
	}
//	void set_context_keyboard(DEVICE* value) {
//		d_keyboard = value;
//	}
//	void set_context_mouse(DEVICE* value) {
//		d_mouse = value;
//	}
//	void set_key_scan_code_ptr(uint8_t *value) {
//		key_scan_code_ptr = value;
//	}
	void set_counter_ptr(int *value) {
		counter_ptr = value;
	}
//#ifdef USE_LIGHTPEN
//	inline int get_lpen_recp_stat(int num) {
//		return lpen_recp_stat[num];
//	}
//#endif
//	inline uint8_t *get_mouse_recp_stat() {
//		return mouse_recp_stat;
//	}
	inline uint8_t get_vm_key_recp_stat(int code) {
		return vm_key_recp_stat[code];
	}
};

#endif /* KEYRECORD_H */
