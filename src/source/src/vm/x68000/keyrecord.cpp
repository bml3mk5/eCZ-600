/** @file keyrecord.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22

	@brief [ key recording ]
*/

#include "keyrecord.h"
#include "../../emu.h"
#include "../device.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../version.h"

#ifdef USE_KEY_RECORD
#define KEY_RECORD_SYSTEM_CODE	0x1000
#define KEY_RECORD_MAX			0x1200
#define KEY_RECORD_HEADER		"KEYRECORD_X68000"
#endif

// ============================================================================

KEYRECORD_CACHE::KEYRECORD_CACHE()
{
	m_type = 0;
	Clear();
}
KEYRECORD_CACHE::KEYRECORD_CACHE(int type)
{
	m_type = type;
	Clear();
}
KEYRECORD_CACHE::~KEYRECORD_CACHE()
{
}
void KEYRECORD_CACHE::Clear()
{
	m_wpos = 0;
	m_rpos = 0;
	m_cnt = 0;
}
void KEYRECORD_CACHE::Set(uint64_t clk, int cnt, const int code[5])
{
	struct st_cache *p = &m_caches[m_wpos];
	p->clk = clk;
	p->cnt = cnt;
	if (p->cnt > 5) p->cnt = 5;
	for(int i=0; i<p->cnt; i++) {
		p->code[i] = code[i];
	}
	m_wpos++;
	m_cnt++;
	if (m_wpos >= CACHE_SIZE) m_wpos = 0;
}
struct KEYRECORD_CACHE::st_cache *KEYRECORD_CACHE::FindFirst(uint64_t now_clk)
{
	struct st_cache *found = NULL;
	while (m_cnt > 0) {
		struct st_cache *p = &m_caches[m_rpos];
		if (now_clk < p->clk) {
			break;
		}

		found = p;
		m_rpos++;
		m_cnt--;
		if (m_rpos >= CACHE_SIZE) m_rpos = 0;
	}
	return found;
}

// ============================================================================

KEYRECORD::KEYRECORD(EMU* parent_emu)
{
	emu = parent_emu;
//	mouse_stat = emu->mouse_buffer();

	fkro = new FILEIO();
	fkri = new FILEIO();
	m_end_of_file = false;
	pConfig->reckey_recording = false;
	pConfig->reckey_playing = false;
	key_rec_sum_clock = 0;

	clear_play_buffer();
	clear_record_buffer();

#ifdef _DEBUG_KEYRECORD
	memset(vm_key_dbg_stat, 0, sizeof(vm_key_dbg_stat));
#endif

	m_syskey_cache.SetType(0);
	m_key_cache.SetType(1);
#ifdef USE_LIGHTPEN
	m_lpen_cache.SetType(2);
#endif
#ifdef USE_MOUSE
	m_mouse_cache.SetType(3);
#endif
#ifdef USE_PIAJOYSTICK
	m_joypia_cache[0].SetType(4);
	m_joypia_cache[1].SetType(4);
#endif
}

KEYRECORD::~KEYRECORD()
{
	stop_reckey();

	delete fkro;
	delete fkri;
}

// ----------------------------------------------------------------------------
void KEYRECORD::clear_play_buffer()
{
//	memset(vm_syskey_recp_stat, 0, sizeof(vm_syskey_recp_stat));
	memset(vm_key_recp_stat, 0, sizeof(vm_key_recp_stat));
#ifdef USE_MOUSE
	memset(&mouse_recp_stat, 0, sizeof(mouse_recp_stat));
#endif
	memset(joypia_recp_stat, 0, sizeof(joypia_recp_stat));
#ifdef USE_LIGHTPEN
	memset(lpen_recp_stat, 0, sizeof(lpen_recp_stat));
#endif
	emu->clear_vm_key_status(VM_KEY_STATUS_KEYREC);
}

void KEYRECORD::clear_record_buffer()
{
//	memset(vm_syskey_recr_stat, 0, sizeof(vm_syskey_recr_stat));
	memset(vm_key_recr_stat, 0, sizeof(vm_key_recr_stat));
#ifdef USE_MOUSE
	memset(&mouse_recr_stat, 0, sizeof(mouse_recr_stat));
#endif
	memset(joypia_recr_stat, 0, sizeof(joypia_recr_stat));
#ifdef USE_LIGHTPEN
	memset(lpen_recr_stat, 0, sizeof(lpen_recr_stat));
#endif
}

// ----------------------------------------------------------------------------
void KEYRECORD::read_to_cache()
{
	if (!pConfig->reckey_playing) return;

	uint64_t limit_clk = d_event->get_current_clock() + (CPU_CLOCKS / 60);
	unsigned long long int clk = 0;
	int type = -1;
	int code[5];
	int rows = 0;
	if (!m_end_of_file) {
		bool is_full = false;
		is_full |= m_syskey_cache.IsFull();
		is_full |= m_key_cache.IsFull();
#ifdef USE_LIGHTPEN
		is_full |= m_lpen_cache.IsFull();
#endif
#ifdef USE_MOUSE
		is_full |= m_mouse_cache.IsFull();
#endif
#ifdef USE_PIAJOYSTICK
		is_full |= m_joypia_cache[0].IsFull();
		is_full |= m_joypia_cache[1].IsFull();
#endif
		while(!is_full && rows < 128) {
			if (fkri->Fgets(rec_key_rec_buff, sizeof(rec_key_rec_buff)) == NULL) {
				m_end_of_file = true;
				break;
			}

			if (!(0x30 <= rec_key_rec_buff[0] && rec_key_rec_buff[0] <= 0x39)) {
				continue;
			}

			int cols = sscanf(rec_key_rec_buff, "%llu:%d:", &clk, &type);

			if (cols < 2) {
				continue;
			}

//			clk <<= clock_scale;

			char *p = strchr(rec_key_rec_buff, ':');
			p = strchr(p+1, ':');
			switch(type) {
			case 1:
				// key
				cols = sscanf(p, ":%x:%d", &code[0], &code[1]);
				if (cols == 2) {
					if (code[0] < KEYBIND_KEYS) {
						// normal key
						m_key_cache.Set((uint64_t)clk, cols, code);
						rows++;
					} else if (code[0] >= KEY_RECORD_SYSTEM_CODE && code[0] < KEY_RECORD_MAX) {
						// system key
						m_syskey_cache.Set((uint64_t)clk, cols, code);
						rows++;
					}
				}
				break;
			case 2:
#ifdef USE_LIGHTPEN
				// lightpen
				cols = sscanf(p, ":%d:%d:%x", &code[0], &code[1], &code[2]);
				if (cols == 3) {
					m_lpen_cache.Set((uint64_t)clk, cols, code);
					rows++;
				}
#endif
				break;
			case 3:
#ifdef USE_MOUSE
				// mouse
				cols = sscanf(p, ":%d:%d:%d", &code[0], &code[1], &code[2]);
				if (cols == 3) {
					m_mouse_cache.Set((uint64_t)clk, cols, code);
					rows++;
				}
#endif
				break;
			case 4:
#ifdef USE_PIAJOYSTICK
				// joystick
				cols = sscanf(p, ":%d:%x:%x:%x", &code[0], &code[1], &code[2], &code[3]);
				if (cols == 4 && code[0] >= 0 && code[0] < 2) {
					m_joypia_cache[code[0]].Set((uint64_t)clk, cols, code);
					rows++;
				}
#endif
				break;
			default:
				break;
			}

			if (limit_clk < (uint64_t)clk) {
				// too far
				break;
			}
		}
	} else {
		// end of file
		bool end_of_data = true;
		end_of_data &= m_syskey_cache.IsEmpty();
		end_of_data &= m_key_cache.IsEmpty();
#ifdef USE_LIGHTPEN
		end_of_data &= m_lpen_cache.IsEmpty();
#endif
#ifdef USE_MOUSE
		end_of_data &= m_mouse_cache.IsEmpty();
#endif
#ifdef USE_PIAJOYSTICK
		end_of_data &= m_joypia_cache[0].IsEmpty();
		end_of_data &= m_joypia_cache[1].IsEmpty();
#endif
		if (end_of_data) {
			// end of play
			stop_reckey(true, false);
		}
	}
}

// ----------------------------------------------------------------------------
#if 0
void KEYRECORD::reading_keys(int num)
{
#ifdef USE_KEY_RECORD
	if (pConfig->reckey_playing) {
		uint64_t now_clk = d_event->get_current_clock();
		int cols = -1;
		uint64_t clk = 0;
		int type = -1;
		int code[3];
		char *p = NULL;

		while(pConfig->reckey_playing) {
			if (0x30 <= rec_key_rec_buff[0] && rec_key_rec_buff[0] <= 0x39) {
				cols = sscanf(rec_key_rec_buff,"%llu:%d:",&clk,&type);
				if ((int64_t)clk + key_rec_sum_clock < 0) {
					clk = 0;
				} else {
					clk += key_rec_sum_clock;
				}
//				logging->out_logf(LOG_DEBUG,_T("RecKey:n:%llu:%llu"),now_clk,clk);
				if ((now_clk + 1) < clk) {
					break;
				}
//				logging->out_logf(LOG_DEBUG,_T("RecKey:keyin"));

				if (cols == 2 && clk > 0) {
					p = strchr(rec_key_rec_buff, ':');
					p = strchr(p+1, ':');
					switch(type) {
					case 1:
						// key
						cols = sscanf(p,":%x:%d",&code[0],&code[1]);
						if (cols == 2 && code[0] >= 0) {
							int code0 = (code[0] << 1) + 1;
#ifdef _DEBUG_KEYRECORD
							logging->out_logf(LOG_DEBUG, _T("RecKey%d %02x nc:%llu %c %llu nk:%d %c %d %s")
								, num, code[0]
								, now_clk, (now_clk == clk ? _T('=') : _T('!')), clk
								, *counter_ptr, (*counter_ptr == code0 ? _T('=') : _T('!')), code0
								, (code[1] & 1) ? _T("ON") : _T("OFF"));
#endif
							if (code[0] < KEYBIND_KEYS) {
								// normal key
								vm_key_recp_stat[code[0]]=(code[1] & 1);
								if (num == 0 && (clk < now_clk || code0 < (*counter_ptr))) {
									// adjust timing
									*counter_ptr = code0;
//									*key_scan_code_ptr = ((code0 >> 1) & ((*kb_mode_ptr & 0x08) ? 0x07 : 0x7f));

									logging->out_debugf(_T("RecKey%d %02x adjust k:%d"), num, code[0], *counter_ptr);
								}
							} else if (code[0] >= KEY_RECORD_SYSTEM_CODE && code[0] < KEY_RECORD_MAX) {
								if (code[1] & 1) {
									// global key
									emu->system_key_down(code[0] & 0xfff);
									emu->execute_global_keys(code[0] & 0xfff, 2);
								}
							}
						}
						break;
					case 2:
#ifdef USE_LIGHTPEN
						// lightpen
						cols = sscanf(p,":%d:%d:%x",&code[0],&code[1],&code[2]);
						if (cols == 3) {
							memcpy(lpen_recp_stat, code, sizeof(lpen_recp_stat));
						}
#endif
						break;
#ifdef USE_MOUSE
					case 3:
						// mouse
						cols = sscanf(p, ":%d:%d:%d"
							,&code[0],&code[1],&code[2]
						);
						if (cols == 3) {
							mouse_recp_stat[0] = code[0];
							mouse_recp_stat[1] = code[1];
							mouse_recp_stat[2] = code[2];
						}
						break;
#endif
					case 4:
						// joystick on PIA
						cols = sscanf(p, ":%x:%x"
							,&code[0],&code[1]
						);
#ifdef _DEBUG_KEYRECORD
						logging->out_logf(LOG_DEBUG, _T("RecKey%d %llu %c %llu j0:%02x j1:%02x")
							, num
							, now_clk, (now_clk == clk ? _T('=') : _T('!')), clk
							, code[0], code[1]
						);
#endif
						if (cols == 2) {
							joypia_recp_stat[0] = (uint8_t)code[0];
							joypia_recp_stat[1] = (uint8_t)code[1];
						}
						break;
					}
				}
			}
			// read next data
			if (fkri->Fgets(rec_key_rec_buff, sizeof(rec_key_rec_buff)) == NULL) {
				stop_reckey(true, false);
				break;
			}
		}
	}
#endif /* USE_KEY_RECORD */
}
#endif

#if 0
bool KEYRECORD::processing_keys(int code, bool pressed)
{
	if (pConfig->reckey_playing) pressed = playing_keys(code, pressed);
	if (pConfig->reckey_recording) recording_keys(code, pressed);
	return pressed;
}
#endif

void KEYRECORD::playing_key()
{
	if (!pConfig->reckey_playing) return;

#ifdef USE_KEY_RECORD
	// read from cahce
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_key_cache.FindFirst(now_clk);
		if (!p) {
			break;
		}
		// normal key
		int code = p->code[0];
		vm_key_recp_stat[code] = ((vm_key_recp_stat[code] & 0xfe) | (p->code[1] & 1));

		switch(vm_key_recp_stat[code] & 0x03) {
		case 0x01:
			emu->vm_key_down(code, VM_KEY_STATUS_KEYREC);
			vm_key_recp_stat[code] |= 0x02;
#ifdef _DEBUG_KEYRECORD
			logging->out_logf(LOG_DEBUG, _T("   Key0 %02x  c:%llu kc:%d ON")
				, code, (unsigned long long int)d_event->get_current_clock(), *counter_ptr);
#endif
			break;
		case 0x02:
			emu->vm_key_up(code, VM_KEY_STATUS_KEYREC);
			vm_key_recp_stat[code] &= ~0x02;
#ifdef _DEBUG_KEYRECORD
			logging->out_logf(LOG_DEBUG, _T("   Key0 %02x  c:%llu kc:%d OFF")
				, code, (unsigned long long int)d_event->get_current_clock(), *counter_ptr);
#endif
			break;
		default:
			break;
		}
	} while(0);
#endif /* USE_KEY_RECORD */
}

void KEYRECORD::recording_key(int code, bool pressed)
{
	if (!pConfig->reckey_recording) return;

#ifdef USE_KEY_RECORD
	if (pressed && vm_key_recr_stat[code] == 0) {
		UTILITY::sprintf(rec_key_tmp_buff,sizeof(rec_key_tmp_buff),"%llu:1:%04x:1\n"
			, (unsigned long long int)d_event->get_current_clock(), code);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		vm_key_recr_stat[code] = 1;

	} else if (!pressed && vm_key_recr_stat[code] != 0) {
		UTILITY::sprintf(rec_key_tmp_buff,sizeof(rec_key_tmp_buff),"%llu:1:%04x:0\n"
			, (unsigned long long int)d_event->get_current_clock(), code);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		vm_key_recr_stat[code] = 0;

	}
#endif /* USE_KEY_RECORD */
}

#if 0
bool KEYRECORD::playing_keys(int code, bool pressed)
{
#ifdef USE_KEY_RECORD
	// read from cahce
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_key_cache.FindFirst(now_clk);
		if (!p) {
			break;
		}
		// normal key
		vm_key_recp_stat[p->code[0]]=(p->code[1] & 1);

	} while(0);

	if (pressed != true) {
		// record key pressed ?
		if (vm_key_recp_stat[code]) {
			pressed = true;
		}
	}

#ifdef _DEBUG_KEYRECORD
	if ((vm_key_dbg_stat[code] != 0) != pressed) {
		logging->out_logf(LOG_DEBUG, _T("   Key0 %02x  c:%llu kc:%d %s")
			, code, d_event->get_current_clock()
			, *counter_ptr, pressed ? _T("ON") : _T("OFF"));

		vm_key_dbg_stat[code] = (pressed ? 1 : 0);
	}
#endif

	return pressed;
#else /* USE_KEY_RECORD */
	return false;
#endif /* !USE_KEY_RECORD */
}
#endif

#if 0
void KEYRECORD::recording_keys(int code, bool pressed)
{
#ifdef USE_KEY_RECORD
	if (pressed && vm_key_recr_stat[code] == 0) {
		sprintf(rec_key_tmp_buff,"%llu:1:%04x:1\n"
			, d_event->get_current_clock(), code);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		vm_key_recr_stat[code] = 1;

	} else if (!pressed && vm_key_recr_stat[code] != 0) {
		sprintf(rec_key_tmp_buff,"%llu:1:%04x:0\n"
			, d_event->get_current_clock(), code);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		vm_key_recr_stat[code] = 0;

	}
#endif /* USE_KEY_RECORD */
}
#endif

void KEYRECORD::playing_system_keys()
{
#ifdef USE_KEY_RECORD
	if (!pConfig->reckey_playing) return;

	// read from cahce
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_syskey_cache.FindFirst(now_clk);
		if (!p) {
			break;
		}
		if (p->code[1] & 1) {
			// global key
			emu->execute_global_keys(p->code[0] & 0x1ff, 2);
		} else {
			emu->release_global_keys(p->code[0] & 0x1ff, 2);
		}
	} while(0);
#endif /* USE_KEY_RECORD */
}

void KEYRECORD::recording_system_keys(int code, bool pressed)
{
#ifdef USE_KEY_RECORD
	if (!pConfig->reckey_recording) return;

	code |= KEY_RECORD_SYSTEM_CODE;
	if (code < KEY_RECORD_MAX) {
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "%llu:1:%04x:%d\n"
			, (unsigned long long int)d_event->get_current_clock()
			, code, pressed ? 1 : 0);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);

	}
#endif /* USE_KEY_RECORD */
}

#ifdef USE_MOUSE
void KEYRECORD::processing_mouse_status(int *mstat)
{
#ifdef USE_KEY_RECORD
	if (pConfig->reckey_playing) playing_mouse_status(mstat);
	if (pConfig->reckey_recording) recording_mouse_status(mstat);
#endif
}

void KEYRECORD::playing_mouse_status(int *mstat)
{
#ifdef USE_KEY_RECORD
	// read from cahce
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_mouse_cache.FindFirst(now_clk);
		if (!p) {
			break;
		}
		mouse_recp_stat[0] = p->code[0];
		mouse_recp_stat[1] = p->code[1];
		mouse_recp_stat[2] = p->code[2];

	} while(0);

	mstat[0] = mouse_recp_stat[0];
	mstat[1] = mouse_recp_stat[1];
	mstat[2] = mouse_recp_stat[2];
#endif
}

void KEYRECORD::recording_mouse_status(const int *mstat)
{
#ifdef USE_KEY_RECORD
	if (memcmp(mstat, mouse_recr_stat, sizeof(mouse_recr_stat)) != 0) {
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "%llu:3:%d:%d:%d\n"
			, (unsigned long long int)d_event->get_current_clock()
			,mstat[0], mstat[1], mstat[2]
		);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		memcpy(mouse_recr_stat, mstat, sizeof(mouse_recr_stat));

	}
#endif /* USE_KEY_RECORD */
}
#endif

void KEYRECORD::processing_joypia_status(int num, uint32_t *joystat, int size)
{
#ifdef USE_KEY_RECORD
	if (pConfig->reckey_playing) playing_joypia_status(num, joystat, size);
	if (pConfig->reckey_recording) recording_joypia_status(num, joystat, size);
#endif
}

void KEYRECORD::playing_joypia_status(int num, uint32_t *joystat, int size)
{
#ifdef USE_KEY_RECORD
	// read from cahce
#ifdef USE_PIAJOYSTICK
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_joypia_cache[num].FindFirst(now_clk);
		if (!p) {
			break;
		}
		joypia_recp_stat[num][0] = p->code[1];
		joypia_recp_stat[num][1] = p->code[2];
		joypia_recp_stat[num][2] = p->code[3];

	} while(0);

	for(int i=0; i<size; i++) {
		int n = (i >> 2);
		joystat[i] |= (joypia_recp_stat[num][n] & 0xff);
		joypia_recp_stat[num][n] >>= 8;
	}
#endif
#endif
}

void KEYRECORD::recording_joypia_status(int num, const uint32_t *joystat, int size)
{
#ifdef USE_KEY_RECORD
	uint32_t code[3];
	memset(code, 0, sizeof(code));
	for(int i=(size-1); i>=0; i--) {
		int n = (i >> 2);
		code[n] |= (joystat[i] & 0xff);
		code[n] <<= 8;
	}
	bool modified = (memcmp(joypia_recr_stat[num], code, sizeof(code)) != 0);
	if (modified) {
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "%llu:4:%d:%x:%x:%x\n"
			, (unsigned long long int)d_event->get_current_clock()
			, num, code[0], code[1], code[2]
		);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);

		memcpy(joypia_recr_stat[num], code, sizeof(code));
	}
#endif /* USE_KEY_RECORD */
}

#ifdef USE_LIGHTPEN
void KEYRECORD::processing_lightpen_status(int *mstat)
{
#ifdef USE_KEY_RECORD
	if (pConfig->reckey_playing) playing_lightpen_status(mstat);
	if (pConfig->reckey_recording) recording_lightpen_status(mstat);
#endif
}

void KEYRECORD::playing_lightpen_status(int *mstat)
{
	// read from cahce
	uint64_t now_clk = d_event->get_current_clock();
	do {
		struct KEYRECORD_CACHE::st_cache *p = m_lpen_cache.FindFirst(now_clk);
		if (!p) {
			break;
		}
		lpen_recp_stat[0] = p->code[0];
		lpen_recp_stat[1] = p->code[1];
		lpen_recp_stat[2] = p->code[2];

	} while(0);

	mstat[0] = lpen_recp_stat[0];
	mstat[1] = lpen_recp_stat[1];
	mstat[2] = lpen_recp_stat[2];
}

void KEYRECORD::recording_lightpen_status(const int *mstat)
{
#ifdef USE_KEY_RECORD
	if (memcmp(mstat, lpen_recr_stat, sizeof(lpen_recr_stat)) != 0) {
		sprintf(rec_key_tmp_buff,"%llu:2:%d:%d:%x\n"
			, (unsigned long long int)d_event->get_current_clock()
			, mstat[0], mstat[1], mstat[2] & 3);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		memcpy(lpen_recr_stat, mstat, sizeof(lpen_recr_stat));
	}
#endif /* USE_KEY_RECORD */
}
#endif /* USE_LIGHTPEN */

// ----------------------------------------------------------------------------
bool KEYRECORD::play_reckey(const _TCHAR* filename)
{
#ifdef USE_KEY_RECORD
	stop_reckey(true, false);

	char *p = NULL;
	int cols;
	int version;
	unsigned long long int start_clock;

	fkri->Fopen(filename ,FILEIO::READ_ASCII);
	if (fkri->IsOpened()) {
		m_end_of_file = false;

		// check header
		p = fkri->Fgets(rec_key_tmp_buff, sizeof(rec_key_tmp_buff));
		if (p == NULL || strncmp(rec_key_tmp_buff, KEY_RECORD_HEADER, strlen(KEY_RECORD_HEADER)) != 0) {
			// error
			logging->out_log_x(LOG_ERROR, CMsg::This_record_key_file_is_not_supported);
			goto FIN;
		}
		p = fkri->Fgets(rec_key_tmp_buff, sizeof(rec_key_tmp_buff));
		cols = sscanf(rec_key_tmp_buff, "Version:%d", &version);
		if (p == NULL || cols != 1 || version != 1) {
			// error
			logging->out_log_x(LOG_ERROR, CMsg::Record_key_file_is_invalid_version);
			goto FIN;
		}
		p = fkri->Fgets(rec_key_tmp_buff, sizeof(rec_key_tmp_buff));
		cols = sscanf(rec_key_tmp_buff, "StartClock:%llu", &start_clock);
		if (p == NULL || cols != 1) {
			// error
			logging->out_log_x(LOG_ERROR, CMsg::Record_key_file_has_invalid_parameter);
			goto FIN;
		}
//		logging->out_logf(LOG_DEBUG, _T("RecordKey StartClock:%llu"), start_clock);

		_TCHAR base_path[_MAX_PATH];

		UTILITY::get_dir_and_basename(filename, base_path, NULL);
		logging->out_debugf(_T("BasePath:%s"),base_path);

		// parse optional parameters
		_TCHAR *state_file = NULL;
#ifdef USE_DATAREC
		_TCHAR *tape_file = NULL;
		bool tape_playing = true;
#endif
#ifdef USE_FD1
		_TCHAR *disk_file[MAX_DRIVE];
		int disk_bank_num[MAX_DRIVE];
#endif
#ifdef USE_HD1
		_TCHAR *hard_disk_file[MAX_HARD_DISKS];
#endif
		int drv = 0;
		int major = 0;
		int minor = 0;
		int revision = 0;

#ifdef USE_FD1
		for(drv = 0; drv < MAX_DRIVE; drv++) {
			disk_file[drv] = NULL;
		}
#endif
#ifdef USE_HD1
		for(drv = 0; drv < MAX_HARD_DISKS; drv++) {
			hard_disk_file[drv] = NULL;
		}
#endif

		for(;;) {
			p = fkri->Fgets(rec_key_tmp_buff, sizeof(rec_key_tmp_buff));
			if (p == NULL || p[0] == '\r' || p[0] == '\n') {
				break;
			}
			if (strncmp(rec_key_tmp_buff, "StateFile:", 10) == 0) {
				get_file_path(base_path, &state_file, NULL);
				logging->out_debugf(_T("StateFile:%s"),state_file);
#ifdef USE_DATAREC
			} else if (strncmp(rec_key_tmp_buff, "TapeFile:", 9) == 0) {
				get_file_path(base_path, &tape_file, NULL);
				logging->out_debugf(_T("TapeFile:%s"),tape_file);
			} else if (strncmp(rec_key_tmp_buff, "TapeType:", 9) == 0) {
				if (strncmp(&rec_key_tmp_buff[9], "Rec", 3) == 0) {
					tape_playing = false;
				}
#endif
#ifdef USE_FD1
			} else if (sscanf(rec_key_tmp_buff, "Disk%dFile:", &drv) == 1) {
				if (0 <= drv && drv < MAX_DRIVE) {
					get_file_path(base_path, &disk_file[drv], &disk_bank_num[drv]);
					logging->out_debugf(_T("Disk%dFile:%s:%d"),drv,disk_file[drv],disk_bank_num[drv]);
				}
#endif
#ifdef USE_HD1
			} else if (sscanf(rec_key_tmp_buff, "HardDisk%dFile:", &drv) == 1) {
				if (0 <= drv && drv < MAX_HARD_DISKS) {
					get_file_path(base_path, &hard_disk_file[drv], NULL);
					logging->out_debugf(_T("HardDisk%dFile:%s"),drv,hard_disk_file[drv]);
				}
#endif
			} else if (sscanf(rec_key_tmp_buff, "EmulatorVersion:%d.%d.%d", &major, &minor, &revision) == 3) {
				logging->out_logf_x(LOG_INFO, CMsg::The_version_of_the_emulator_used_for_recording_is_VDIGIT_VDIGIT_VDIGIT, major, minor, revision);
			}
		}

		// open files
		if (state_file) {
			emu->load_state(state_file);
		}
#ifdef USE_DATAREC
		if (tape_file) {
			if (tape_playing) {
				emu->play_datarec(tape_file);
			} else {
				emu->rec_datarec(tape_file);
			}
		}
#endif
#ifdef USE_FD1
		for(drv = 0; drv < MAX_DRIVE; drv++) {
			if (disk_file[drv]) {
				if (!emu->is_same_floppy_disk(drv, disk_file[drv], disk_bank_num[drv])) {
					emu->open_floppy_disk_by_bank_num(drv, disk_file[drv], disk_bank_num[drv], OPEN_DISK_FLAGS_FORCELY, false);
				}
			} else {
				emu->close_floppy_disk(drv);
			}
		}
#endif
#ifdef USE_HD1
		for(drv = 0; drv < MAX_HARD_DISKS; drv++) {
			if (hard_disk_file[drv]) {
				if (!emu->is_same_hard_disk(drv, hard_disk_file[drv])) {
					emu->open_hard_disk(drv, hard_disk_file[drv], OPEN_DISK_FLAGS_FORCELY);
				}
			} else {
				emu->close_hard_disk(drv);
			}
		}
#endif

		delete [] state_file;
#ifdef USE_DATAREC
		delete [] tape_file;
#endif
#ifdef USE_FD1
		for(drv = 0; drv < MAX_DRIVE; drv++) {
			delete [] disk_file[drv];
		}
#endif
#ifdef USE_HD1
		for(drv = 0; drv < MAX_HARD_DISKS; drv++) {
			delete [] hard_disk_file[drv];
		}
#endif

		// adjust start clock
		key_rec_sum_clock = d_event->get_current_clock() - (uint64_t)start_clock;
		// check ok
		pConfig->reckey_playing = true;

		// clear buffer
		clear_play_buffer();

		// read first data
		read_to_cache();

//		memset(rec_key_rec_buff, 0, sizeof(rec_key_rec_buff));
//		fkri->Fgets(rec_key_rec_buff, sizeof(rec_key_rec_buff));

		logging->out_debugf(_T("RecKeyStart: c:%llu s:%llu")
			, (unsigned long long int)d_event->get_current_clock()
			, start_clock);
	}
FIN:
#endif /* USE_KEY_RECORD */
	return pConfig->reckey_playing;
}

bool KEYRECORD::record_reckey(const _TCHAR* filename)
{
#ifdef USE_KEY_RECORD
	stop_reckey(false, true);

	fkro->Fopen(filename ,FILEIO::WRITE_ASCII);
	if (fkro->IsOpened()) {
		pConfig->reckey_recording = true;

		_TCHAR base_path[_MAX_PATH];
		char keyname[128];

		UTILITY::get_dir_and_basename(filename, base_path, NULL);

		// write header
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "%s\n", KEY_RECORD_HEADER);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "Version:%d\n", 1);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "StartClock:%llu\n"
			, (unsigned long long int)d_event->get_current_clock());
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "EmulatorVersion:%d.%d.%d\n", APP_VER_MAJOR, APP_VER_MINOR, APP_VER_REV);
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);

		set_relative_path("StateFile", base_path, pConfig->saved_state_path);

#ifdef USE_DATAREC
		if (set_relative_path("TapeFile", base_path, pConfig->opened_datarec_path)) {
			if (emu->datarec_opened(true)) {
				sprintf(rec_key_tmp_buff, "TapeType:Play\n");
				fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
			} else if (emu->datarec_opened(false)) {
				sprintf(rec_key_tmp_buff, "TapeType:Rec\n");
				fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);
			}
		}
#endif
#ifdef USE_FD1
		for(int drv = 0; drv < MAX_DRIVE; drv++) {
			UTILITY::sprintf(keyname, sizeof(keyname), "Disk%dFile", drv);
			set_relative_path(keyname, base_path, pConfig->opened_disk_path[drv]);
		}
#endif
#ifdef USE_HD1
		for(int drv = 0; drv < MAX_HARD_DISKS; drv++) {
			UTILITY::sprintf(keyname, sizeof(keyname), "HardDisk%dFile", drv);
			set_relative_path(keyname, base_path, pConfig->opened_hard_disk_path[drv]);
		}
#endif
		UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "\n");
		fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);

		// clear buffer
		clear_record_buffer();
	}
#endif /* USE_KEY_RECORD */
	return pConfig->reckey_recording;
}

void KEYRECORD::stop_reckey(bool stop_play, bool stop_record)
{
#ifdef USE_KEY_RECORD
	if (stop_play && fkri) {
		fkri->Fclose();
		pConfig->reckey_playing = false;
		clear_play_buffer();
	}
	if (stop_record && fkro) {
		fkro->Fclose();
		pConfig->reckey_recording = false;
		clear_record_buffer();
	}
#endif /* USE_KEY_RECORD */
}

bool KEYRECORD::set_relative_path(const char *key, const _TCHAR *base_path, CRecentPath &path)
{
	if (path.path.Length() <= 0) return false;

	_TCHAR npath[_MAX_PATH];
	char mpath[_MAX_PATH];

	UTILITY::tcscpy(npath, _MAX_PATH, path.path);
	UTILITY::make_relative_path(base_path, npath);
	if (path.num > 0) pConfig->set_number_in_path(npath, _MAX_PATH, path.num);
	UTILITY::cconv_from_native_path(npath, mpath, _MAX_PATH);
	UTILITY::sprintf(rec_key_tmp_buff, sizeof(rec_key_tmp_buff), "%s:%s\n", key, mpath);

	fkro->Fwrite(rec_key_tmp_buff, strlen(rec_key_tmp_buff), 1);

	return true;
}

void KEYRECORD::get_file_path(const _TCHAR *base_path, _TCHAR **file_path, int *bank_num)
{
	if (!(*file_path)) (*file_path) = new _TCHAR[_MAX_PATH];
	memset((*file_path), 0, sizeof(_TCHAR) * _MAX_PATH);
	const char *ps = strchr(rec_key_tmp_buff, ':') + 1;
	UTILITY::conv_to_native_path(ps, (*file_path), _MAX_PATH);
	if (bank_num) {
		(*bank_num) = 0;
		pConfig->get_number_in_path((*file_path), bank_num);
	}
	UTILITY::convert_path_separator(*file_path);
	UTILITY::make_absolute_path(base_path, *file_path);
}
