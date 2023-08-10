/** @file mouse.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22

	@brief [ mouse ]
*/

#ifndef MOUSE_H
#define MOUSE_H

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"
#include "../../config.h"
#ifdef USE_KEY_RECORD
#include "keyrecord.h"
#endif

/**
	@brief Mouse emulate
*/
class MOUSE : public DEVICE
{
public:
	enum SIG_IDS {
		SIG_RTS_TO_MFP = 1,	// mouse on keyboard
	};
private:
	enum EVENT_IDS {
		EVENT_MOUSE_STAT_FROM_MFP = 1,	// mouse on keyboard
		EVENT_MOUSE_STAT_FROM_SCC,	// mouse on main machine
	};

private:
	DEVICE *d_board;
#ifdef USE_KEY_RECORD
	KEYRECORD *reckey;
#endif
	DEVICE *d_mfp;
	DEVICE *d_scc;

	int *p_mouse_stat;

	uint8_t m_stat[3];		// update current status
	uint8_t m_stat_lock[3];	// locked status to send

	int m_register_id;	// send mouse status
	int m_phase;
	uint8_t m_scc_rtsb;		// RTS from scc

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_stat[3];		// update current status
		uint8_t m_stat_lock[3];	// locked status to send
		uint8_t m_scc_rtsb;		// RTS from scc
		char reserved;

		int m_register_id;	// send mouse status
		int m_phase;
	};
#pragma pack()

	void update_mouse();

	void req_to_send_mouse_status(int event_id);
	void send_mouse_status(int event_id);

public:
	MOUSE(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier)
	{
		set_class_name("MOUSE");
	}
	~MOUSE() {}

	void initialize();
	void reset();

	void write_signal(int id, uint32_t data, uint32_t mask);

	void event_frame();
	void event_callback(int event_id, int err);

	void set_context_board(DEVICE *device) {
		d_board = device;
	}
#ifdef USE_KEY_RECORD
	void set_keyrecord(KEYRECORD *value) {
		reckey = value;
	}
#endif
	void set_context_mfp(DEVICE* device) {
		d_mfp = device;
	}
	void set_context_scc(DEVICE* device) {
		d_scc = device;
	}

//	void set_mouse_position(int px, int py);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
#endif
};

#endif /* MOUSE_H */
