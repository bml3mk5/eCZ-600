/** @file sxsi.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.03 -

	@brief [ SASI/SCSI base class ]
*/

#ifndef SXSI_H
#define SXSI_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"
#include "../sound_base.h"
#include "../noise.h"
#include "../harddisk.h"
#include "../../cptrlist.h"
#include <vector>

class EMU;
class SXSI_CTRL_RELAY;
class SXSI_CTRL;
class SXSI_CTRLS;
class SXSI_UNITS;

/**
	@brief SASI/SCSI host class
*/
class SXSI_HOST : public SOUND_BASE
{
public:
	enum SXSI_SIGNAL_STATUS_TYPE {
		BUSTYPE_IO  = 0,
		BUSTYPE_CD,
		BUSTYPE_MSG,
		BUSTYPE_BSY,
		BUSTYPE_SEL,
		BUSTYPE_ATN,
		BUSTYPE_ACK,
		BUSTYPE_REQ,
		BUSTYPE_MAX,
		BUSTYPE_NONE = 0xffff
	};

protected:
//	DEVICE *d_memory;
	SXSI_CTRLS *d_ctrls;
	SXSI_CTRL_RELAY *d_selected_ctrl;

	// output signals
	outputs_t outputs_irq;
	outputs_t outputs_drq;

	static const _TCHAR *c_sigtype_names[];
	uint32_t m_bus_status[BUSTYPE_MAX];	///< communication signals and status

	enum WAV_SNDTYPES {
		SXSI_WAV_HDD_SEEK	= 0,
		SXSI_WAV_MO_EJECT,
		SXSI_WAV_SNDTYPES
	};
	NOISE m_noises[SXSI_WAV_SNDTYPES];
	bool m_wav_loaded_at_first;

	void load_wav();

	// for save config
#pragma pack(1)
	struct vm_state_st {
		NOISE::vm_state_st m_noises[SXSI_WAV_SNDTYPES];

		uint32_t m_bus_status[BUSTYPE_MAX];
	};
#pragma pack()

	void save_state_parts(struct vm_state_st &vm_state);
	void load_state_parts(const struct vm_state_st &vm_state);

public:
	SXSI_HOST(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~SXSI_HOST();

	// common functions
	virtual void initialize();

	void initialize_sound(int rate, int decibel);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int decibel, bool vol_mute);

	// unique functions
//	void set_context_memory(DEVICE *device) {
//		d_memory = device;
//	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void init_context_irq() {
		init_output_signals(&outputs_irq);
	}
	void init_context_drq() {
		init_output_signals(&outputs_drq);
	}

//	virtual void set_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3);
//	virtual void clr_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3);
	virtual void update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff);
	virtual bool is_active_bus_status(SXSI_SIGNAL_STATUS_TYPE sig_type) const;
	virtual uint32_t get_bus_signal() const { return 0; }
//	virtual void set_irq(bool onoff) {}
//	virtual void set_drq(bool onoff) {}

	enum en_accept_req_flags {
		AR_NONE = 0,
		AR_TRANSFERRING = 0x01,
		AR_COMPLETED = 0x02,
	};
	virtual void accept_req(uint32_t flags) {}
	virtual void go_idle() {}
	virtual void reselection(int ctrl_id) {}

	//
	void play_seek_sound(int device_type);
	void play_eject_sound(int device_type);
};

/**
	@brief SASI/SCSI base class
*/
class SXSI_CTRL : public DEVICE
{
protected:
	SXSI_HOST *d_host;
	SXSI_CTRL_RELAY *d_relay;
	int		 m_ctrl_id;
	HARDDISK *d_current_disk;

	int		m_phase;
	uint8_t m_unit_number;	///< b2-b0
	uint8_t m_command_class;
	uint8_t m_command_code;
	uint8_t m_command_len;
	int		m_command_pos;
	uint8_t m_command_data[16];

	uint32_t m_curr_block;
	int		 m_num_blocks;

	void update_bus_status(SXSI_HOST::SXSI_SIGNAL_STATUS_TYPE bus_type, bool onoff);

	// for save config
#pragma pack(1)
	struct vm_state_st {
		int		revision;
		int		m_phase;
		uint8_t m_unit_number;	///< b2-b0
		uint8_t m_command_class;
		uint8_t m_command_code;
		uint8_t m_command_len;
		int		m_command_pos;

		uint8_t m_command_data[16];

		uint32_t m_curr_block;
		int		 m_num_blocks;
		int		reserved[2];
	};
#pragma pack()

	void save_state_parts(struct vm_state_st &vm_state);
	void load_state_parts(const struct vm_state_st &vm_state);

public:
	SXSI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id);
	virtual ~SXSI_CTRL();

	// unique functions
	virtual void clear() {}
	virtual bool select();
	virtual void go_command_phase() {}
	virtual void go_message_out_phase() {}
	virtual void accept_ack() {}
	virtual void request_next() {}
	virtual void write_data(uint32_t data) {}
	virtual uint32_t read_data() { return 0; }

//	const SXSI_UNITS *get_disk_units() const;
//	HARDDISK *get_disk_unit(int unit_num) const;
//	virtual bool unit_mounted(int unit_num) const;
//	virtual bool unit_mounted_at_least() const;

	// user interfaces
	virtual void change_device_type(int num) {}

	SXSI_CTRL_RELAY *get_relay() { return d_relay; }
	int get_phase() const { return m_phase; }
	int get_command_pos() const { return m_command_pos; }

#ifdef USE_DEBUGGER
	virtual void debug_write_data(uint32_t data) {}
	virtual uint32_t debug_read_data() { return 0; }
	virtual void debug_regs_info(_TCHAR *buffer, size_t buffer_len) {}
#endif
};

/**
	@brief SASI/SCSI relay class
*/
class SXSI_CTRL_RELAY : public DEVICE
{
protected:
	SXSI_HOST *d_host;
	std::vector<SXSI_CTRL *> d_ctrls;
	SXSI_UNITS *d_units;
	SXSI_CTRL *d_selected_ctrl;
	int m_ctrl_id;
	int m_device_type;

	// for save config
#pragma pack(1)
	struct vm_state_st {
		int m_selected_ctrl_idx;
		int m_device_type;
		int reserved[2];
	};
#pragma pack()

public:
	SXSI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id, int num_of_units);
	virtual ~SXSI_CTRL_RELAY();

	void add_ctrl(SXSI_CTRL *ctrl);

	virtual void save_state(FILEIO *fio);
	virtual bool load_state(FILEIO *fio);

	// unique functions
//	void clear();
	bool select();
	void go_command_phase();
	void go_message_out_phase();
	void accept_ack();
	void request_next();

	void write_data(uint32_t data);
	uint32_t read_data();

	int get_phase() const;
	int get_command_pos() const;

	// user interfaces
	bool open_disk(int unit_num, const _TCHAR *path, uint32_t flags);
	bool close_disk(int unit_num, uint32_t flags);
	bool disk_mounted(int unit_num) const;
	void set_write_protect(int unit_num, bool val);
	bool write_protected(int unit_num) const;
	bool is_same_disk(int unit_num, const _TCHAR *path);

	virtual void change_device_type(int num);
	virtual int get_device_type() const;

	int get_ctrl_id() const { return m_ctrl_id; }
	HARDDISK *get_disk_unit(int unit_num) const;
//	bool unit_mounted(int unit_num) const;
	virtual bool unit_mounted_at_least() const;

	const SXSI_UNITS *get_units() const { return d_units; }

#ifdef USE_DEBUGGER
	void debug_write_data(uint32_t data);
	uint32_t debug_read_data();
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

/**
	@brief SASI/SCSI controll list
*/
class SXSI_CTRLS : public std::vector<SXSI_CTRL_RELAY *>
{
public:
	SXSI_CTRLS();
};

/**
	@brief SASI/SCSI unit list
*/
class SXSI_UNITS : public CPtrList<HARDDISK>
{
public:
	SXSI_UNITS(int alloc_size);
};

#endif /* SXSI_H */

