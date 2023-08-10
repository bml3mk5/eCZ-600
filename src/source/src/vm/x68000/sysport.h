/** @file sysport.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ system port ]
*/

#ifndef SYSPORT_H
#define SYSPORT_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"

class EMU;

/**
	@brief system port
*/
class SYSPORT : public DEVICE
{
public:
	/// @brief signals on SYSPORT
	enum SIG_SYSPORT_IDS {
		SIG_SYSPORT_POWER = 11,
	};

private:
	/// @brief events on SYSPORT
	enum EVENT_SYSPORT_IDS {
		EVENT_SYSPORT_WRESET_RELEASE = 1,
	};

private:
	DEVICE *d_display;
	DEVICE *d_memory;
	DEVICE *d_board;
	DEVICE *d_keyboard;

	uint32_t m_power_off;	///< set id to go power off phase

// for save config
#pragma pack(1)
	struct vm_state_st {
		uint32_t m_power_off;	///< set id to go power off phase
		uint32_t reserved[3];
	};
#pragma pack()

public:
	SYSPORT(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("SYSPORT");
		d_display = NULL;
		d_memory = NULL;
		d_board = NULL;
		d_keyboard = NULL;
	}
	~SYSPORT() {}

	// common functions
	void initialize();
	void reset();
//	void cancel_my_event(int &id);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);

	// unique functions
	void set_context_display(DEVICE *device) {
		d_display = device;
	}
	void set_context_memory(DEVICE *device) {
		d_memory = device;
	}
	void set_context_board(DEVICE *device) {
		d_board = device;
	}
	void set_context_keyboard(DEVICE *device) {
		d_keyboard = device;
	}

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* SYSPORT_H */

