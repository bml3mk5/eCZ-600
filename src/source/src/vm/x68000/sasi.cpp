/** @file sasi.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.03 -

	@brief [ shugart associates system interface ]
*/

#include "sasi.h"
#include "scsi.h"
#include "../../emumsg.h"
#include "../vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"
#include "../harddisk.h"

#ifdef _DEBUG
//#define OUT_DEBUG_SIG(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_SIG(...)
#define OUT_DEBUG_SEL(...) logging->out_debugf(__VA_ARGS__)
//#define OUT_DEBUG_SEL(...)
#define OUT_DEBUG_CMD(...) logging->out_debugf(__VA_ARGS__)
//#define OUT_DEBUG_CMD(...)
//#define OUT_DEBUG_DATW(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATW(...)
//#define OUT_DEBUG_DATR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATR(...)
#define OUT_DEBUG_STAT(...) logging->out_debugf(__VA_ARGS__)
//#define OUT_DEBUG_STAT(...)
#else
#define OUT_DEBUG_SIG(...)
#define OUT_DEBUG_SEL(...)
#define OUT_DEBUG_CMD(...)
#define OUT_DEBUG_DATW(...)
#define OUT_DEBUG_DATR(...)
#define OUT_DEBUG_STAT(...)
#endif
#define DEBUG_PREFIX _T("SASI")

// ----------------------------------------------------------------------

SASI::SASI(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: SXSI_HOST(parent_vm, parent_emu, identifier)
{
	set_class_name("SASI");

	int max_id = USE_SASI_HARD_DISKS / SASI_UNITS_PER_CTRL;
	for(int id=0; id<max_id; id++) {
		SASI_CTRL_RELAY *relay = new SASI_CTRL_RELAY(vm, emu, "A", this, id);
		d_ctrls->push_back(relay);
	}
}

SASI::~SASI()
{
}

void SASI::initialize()
{
	SXSI_HOST::initialize();

	d_selected_ctrl = NULL;

	m_signal_status = 0;
	for(int i=0; i<BUSTYPE_MAX; i++) {
		m_bus_status[i] = 0;
	}
}

void SASI::release()
{
//  devices are released on destractor of VM
//	for(int id=0; id<(int)d_ctrls->size(); id++) {
//		delete d_ctrls->Item(id);
//	}
}

void SASI::reset()
{
	load_wav();

	warm_reset(true);
}

void SASI::warm_reset(bool por)
{
//	if (!por) {
//		for(int id=0; id<(int)d_ctrls->size(); id++) {
//			d_ctrls->at(id)->warm_reset(por);
//		}
//	}
	d_selected_ctrl = NULL;
	m_signal_status = 0;
	for(int i=0; i<BUSTYPE_MAX; i++) {
		m_bus_status[i] = 0;
	}
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	// $E96000 >> 1
	switch(addr & 0x3) {
	case 0x00:
		// SASI data write
		if (d_selected_ctrl) {
			update_bus_status(HOST_ID, BUSTYPE_ACK, true);
			d_selected_ctrl->accept_ack();
			set_drq(false);
			d_selected_ctrl->write_data(data);
			update_bus_status(HOST_ID, BUSTYPE_ACK, false);
			d_selected_ctrl->request_next();
		}
		break;
	case 0x01:
		// SEL signal negate
		update_bus_status(HOST_ID, BUSTYPE_SEL, false);
		if (d_selected_ctrl) {
			d_selected_ctrl->go_command_phase();
		}
		break;
	case 0x02:
		// RES signal assert for 300us
		warm_reset(false);
		break;
	case 0x03:
		// SASI data write and SEL signal assert (for selection)
		select_control(data);
		break;
	default:
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	// $E96000 >> 1
	uint32_t data = 0xff;
	switch(addr & 0x3) {
	case 0x00:
		// SASI data read
		if (d_selected_ctrl) {
			update_bus_status(HOST_ID, BUSTYPE_ACK, true);
			d_selected_ctrl->accept_ack();
			set_drq(false);
			data = d_selected_ctrl->read_data();
			update_bus_status(HOST_ID, BUSTYPE_ACK, false);
			d_selected_ctrl->request_next();
		}
		break;
	case 0x01:
		// SASI status read
		data = m_signal_status & SIGSTAT_MASK;
		break;
	default:
		break;
	}
	return data;
}

uint32_t SASI::get_bus_signal() const
{
	return m_signal_status;
}

void SASI::write_signal(int id, uint32_t data, uint32_t mask)
{
//	uint16_t prev = 0;
	switch (id) {
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		if (!(data & mask)) {
			warm_reset(false);
		}
		break;
	}
}

/// @brief select harddisk by id
void SASI::select_control(uint32_t data)
{
	update_bus_status(HOST_ID, BUSTYPE_SEL, true);
	d_selected_ctrl = NULL;
	int id = 0;
	for(; id<(int)d_ctrls->size(); id++) {
		if ((data & 0xff) == (uint32_t)(1 << id)) {
			// select ID
			d_selected_ctrl = d_ctrls->at(id);
			break;
		}
	}
	if (d_selected_ctrl) {
		if (!d_selected_ctrl->select()) {
			d_selected_ctrl = NULL;
		}
	}

	OUT_DEBUG_SEL(_T("%s H: Select: Req:%02X Select:%d Status:%04X"), DEBUG_PREFIX
		, data
		, d_selected_ctrl ? id : -1
		, m_signal_status); 
}

/// asserted REQ signal on the bus
void SASI::accept_req(uint32_t flags)
{
	set_drq((flags & AR_TRANSFERRING) != 0);

	set_irq((flags & AR_COMPLETED) != 0);
}

#if 0
void SASI::update_signal_status(uint32_t on, uint32_t off)
{
	m_signal_status |= on;
	m_signal_status &= ~off;
}
#endif

const uint8_t SASI::bus_status_2_sig_map[BUSTYPE_MAX] = {
	SIGSTAT_IO,		///< I/O (from control)
	SIGSTAT_CD,		///< Control/Data (from control)
	SIGSTAT_MSG,	///< Message (from control)
	SIGSTAT_BSY,	///< Busy (from control)
	SIGSTAT_SEL,	///< Select (to control) (EMU original)
	0,				///< ATN (to control)
	SIGSTAT_ACK,	///< ACK (to control)
	SIGSTAT_REQ,	///< Request (from control)
};

#if 0
void SASI::set_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3)
{
	uint32_t mask = (1 << ctrl_num);
#define SET_BUS_STATUS(sig_type) \
	if (sig_type < BUSTYPE_MAX) { \
		m_bus_status[sig_type] |= mask; \
		BIT_ONOFF(m_signal_status, bus_status_2_sig_map[sig_type], m_bus_status[sig_type] != 0); \
	}
	SET_BUS_STATUS(sig_type0)
	SET_BUS_STATUS(sig_type1)
	SET_BUS_STATUS(sig_type2)
	SET_BUS_STATUS(sig_type3)
	OUT_DEBUG_SIG(_T("%s H: ID%d Set Status:%04X"), DEBUG_PREFIX
		, ctrl_num
		, m_signal_status); 
}

void SASI::clr_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type0, SXSI_SIGNAL_STATUS_TYPE sig_type1, SXSI_SIGNAL_STATUS_TYPE sig_type2, SXSI_SIGNAL_STATUS_TYPE sig_type3)
{
	uint32_t mask = ~(1 << ctrl_num);
#define CLR_BUS_STATUS(sig_type) \
	if (sig_type < BUSTYPE_MAX) { \
		m_bus_status[sig_type] &= mask; \
		BIT_ONOFF(m_signal_status, bus_status_2_sig_map[sig_type], m_bus_status[sig_type] != 0); \
	}
	CLR_BUS_STATUS(sig_type0)
	CLR_BUS_STATUS(sig_type1)
	CLR_BUS_STATUS(sig_type2)
	CLR_BUS_STATUS(sig_type3)
	OUT_DEBUG_SIG(_T("%s H: ID%d Clr Status:%04X"), DEBUG_PREFIX
		, ctrl_num
		, m_signal_status); 
}
#endif

/// update SASI bus signals
void SASI::update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff)
{
#ifdef _DEBUG
	uint32_t prev = m_bus_status[sig_type];
#endif

	BIT_ONOFF(m_bus_status[sig_type], 1 << ctrl_num, onoff);
	BIT_ONOFF(m_signal_status, bus_status_2_sig_map[sig_type], m_bus_status[sig_type] != 0);

#ifdef _DEBUG
	if (prev != m_bus_status[sig_type]) {
		OUT_DEBUG_SIG(_T("%s H: ID%d %s %s SIGS:%04X"), DEBUG_PREFIX
			, ctrl_num
			, c_sigtype_names[sig_type]
			, onoff ? _T("on ") : _T("off")
			, m_signal_status); 
	}
#endif
}

// ----------------------------------------------------------------------------

void SASI::set_irq(bool onoff)
{
	write_signals(&outputs_irq, onoff ? 0xffffffff : 0);
}

void SASI::set_drq(bool onoff)
{
	BIT_ONOFF(m_signal_status, SIGSTAT_DRQ, onoff);
	OUT_DEBUG_SIG(_T("%s H: Update DRQ Status:%04X"), DEBUG_PREFIX, m_signal_status); 
	write_signals(&outputs_drq, onoff ? 0xffffffff : 0);
}

void SASI::go_idle()
{
	// busfree phase
	update_bus_status(HOST_ID, BUSTYPE_SEL, false);
	update_bus_status(HOST_ID, BUSTYPE_ACK, false);
	OUT_DEBUG_STAT(_T("%s H: Go BUSFREE phase"), DEBUG_PREFIX);
	
	set_drq(false);
	set_irq(false);
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void SASI::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);	// Version
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	SXSI_HOST::save_state_parts(vm_state.m_sxsi_host);

	if (d_selected_ctrl) {
		vm_state.m_selected_ctrl_id = Int32_LE(d_selected_ctrl->get_ctrl_id());
	} else {
		vm_state.m_selected_ctrl_id = Int32_LE(-1);
	}
	SET_Uint32_LE(m_signal_status);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool SASI::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	SXSI_HOST::load_state_parts(vm_state.m_sxsi_host);

	int ctrl_id = Int32_LE(vm_state.m_selected_ctrl_id);
	if (ctrl_id >= 0 && ctrl_id < (int)d_ctrls->size()) {
		d_selected_ctrl = d_ctrls->at(ctrl_id);
	} else {
		d_selected_ctrl = NULL;
	}
	GET_Uint32_LE(m_signal_status);

	return true;
}

// ----------------------------------------------------------------------------

bool SASI::open_disk(int drv, const _TCHAR *path, uint32_t flags)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->open_disk(unit_num, path, flags);
}

bool SASI::close_disk(int drv, uint32_t flags)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	bool rc = d_ctrls->at(id)->close_disk(unit_num, flags);
	return rc;
}

bool SASI::disk_mounted(int drv)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->disk_mounted(unit_num);
}

void SASI::toggle_disk_write_protect(int drv)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	bool val = d_ctrls->at(id)->write_protected(unit_num);
	d_ctrls->at(id)->set_write_protect(unit_num, !val);
}

bool SASI::disk_write_protected(int drv) const
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->write_protected(unit_num);
}

bool SASI::is_same_disk(int drv, const _TCHAR *path)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->is_same_disk(unit_num, path);
}

/// Is the disk already mounted on another drive?
int SASI::mounted_disk_another_drive(int drv, const _TCHAR *path)
{
	int curr_id = (drv / SASI_UNITS_PER_CTRL);
	int curr_unit = (drv % SASI_UNITS_PER_CTRL);
	int match = -1;
	for(int id=0; id<(int)d_ctrls->size() && match < 0; id++) {
		for(int unit_num=0; unit_num<SASI_UNITS_PER_CTRL && match < 0; unit_num++) {
			if (curr_id == id && curr_unit == unit_num) continue;
			if (d_ctrls->at(id)->is_same_disk(unit_num, path)) {
				match = id * SASI_UNITS_PER_CTRL + unit_num;
			}
		}
	}
	return match;
}

void SASI::change_device_type(int drv, int num)
{
	int id = (drv / SASI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return;
	return d_ctrls->at(id)->change_device_type(num);
}

/// b12 : hdbusy
uint32_t SASI::get_led_status() const
{
	return (m_signal_status & SIGSTAT_BSY) << 11;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void SASI::debug_write_io8(uint32_t addr, uint32_t data)
{
	// $E96000 >> 1
	switch(addr & 0x3) {
	case 0x00:
		// SASI data write
		if (d_selected_ctrl) {
			d_selected_ctrl->debug_write_data(data);
		}
		break;
	default:
		break;
	}
}

uint32_t SASI::debug_read_io8(uint32_t addr)
{
	// $E96000 >> 1
	uint32_t data = 0xff;
	switch(addr & 0x3) {
	case 0x00:
		// SASI data read
		if (d_selected_ctrl) {
			data = d_selected_ctrl->debug_read_data();
		}
		break;
	case 0x01:
		// SASI status read
		data = m_signal_status & SIGSTAT_MASK;
		break;
	default:
		break;
	}
	return data;
}

bool SASI::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool SASI::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void SASI::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::strcat(buffer, buffer_len, _T("SASI\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T("  Current Control:%d  Status:%02X\n")
		, d_selected_ctrl ? d_selected_ctrl->get_ctrl_id() : -1
		, m_signal_status & SIGSTAT_MASK
	);
	for(int id=0; id<(int)d_ctrls->size(); id++) {
		d_ctrls->at(id)->debug_regs_info(buffer, buffer_len);
	}
}
#endif


// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SASI_CTRL::SASI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id)
	: SXSI_CTRL(parent_vm, parent_emu, identifier, host, relay, ctrl_id)
{
	set_class_name("SASI_CTRL");
}

SASI_CTRL::~SASI_CTRL()
{
}

void SASI_CTRL::clear()
{
	m_phase = PHASE_BUSFREE;
	m_command_pos = 0;
	m_command_class = 0;
	m_command_code = CMD_NONE;
	m_command_len = 6;
	m_unit_number = 0;
	m_curr_block = 0;
	m_num_blocks = 0;
	m_send_message = MSG_COMPLETE;

	m_send_data_pos = 0;
	m_send_data_len = 0;
	m_send_data[0] = 0;
	m_recv_data_pos = 0;
	m_recv_data_len = 0;
	m_recv_data[0] = 0;

	memset(m_command_data, 0, sizeof(m_command_data));

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		m_register_id[id] = -1;
	}
	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		m_event_arg[i] = 0;
	}

	d_current_disk = d_relay->get_disk_unit(m_unit_number);
}

void SASI_CTRL::initialize()
{
	clear();
}

void SASI_CTRL::reset()
{
	warm_reset(true);
}

void SASI_CTRL::warm_reset(bool por)
{
	if (!por) {
		for(int id=0; id<EVENT_IDS_MAX; id++) {
			cancel_my_event(id);
		}
	}
	clear();
}

// ----------------------------------------------------------------------------

const _TCHAR *SASI_CTRL::c_phase_labels[] = {
	_T("BUSFREE"),
	_T("SELECTION"),
	_T("COMMAND"),
	_T("TRANSFER"),
	_T("STATUS"),
	_T("MESSAGE"),
	NULL
};

/// @brief selected this disk
void SASI_CTRL::go_command_phase()
{
	// go to command phase
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
	update_bus_status(SXSI_HOST::BUSTYPE_CD,  true);

	m_phase = PHASE_COMMAND;
	OUT_DEBUG_SEL(_T("%s C: Ctrl:%d-%d Selected"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number); 
}

/// accept ACK signal from host
void SASI_CTRL::accept_ack()
{
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
}

void SASI_CTRL::request_next()
{
	if (m_register_id[EVENT_REQ] >= 0) {
		// Don't assert REQ signal yet
		return;
	}

	switch(m_phase) {
	case PHASE_BUSFREE:
		update_bus_status(SXSI_HOST::BUSTYPE_BSY, false);
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, false);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
		// BUSFREE phase on host
		d_host->go_idle();

		break;

	case PHASE_COMMAND:
		if (m_command_pos < m_command_len) {
			// ready to next data
			update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		}
		break;

	case PHASE_TRANSFER:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_TRANSFERRING);
		break;

	case PHASE_STATUS:
		if (m_register_id[EVENT_STATUS] < 0) {
			update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
			update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
			update_bus_status(SXSI_HOST::BUSTYPE_IO, true);
			update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
			d_host->accept_req(SXSI_HOST::AR_COMPLETED);
		}
		break;

	case PHASE_MESSAGE:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, true);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, true);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_NONE);
		break;

	default:
		break;
	}
}

const _TCHAR *SASI_CTRL::c_command_labels[CMD_UNKNOWN+1] = {
	_T("NONE"),
	_T("TEST DRIVE READY"),
	_T("RECALIBRATE"),
	_T("REQUEST SENSE"),
	_T("FORMAT DRIVE"),
	_T("FORMAT TRACK"),
	_T("FORMAT BAD TRACK"),
	_T("READ"),
	_T("WRITE"),
	_T("SEEK"),
	_T("CONFIG DRIVE"),
	_T("DIAGNOSTIC"),
	_T("Unknown")
};

void SASI_CTRL::write_data(uint32_t data)
{
	switch(m_phase) {
	case PHASE_COMMAND:
		// command phase
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d WR Command Phase Cmd:[%s] Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, c_command_labels[m_command_code < CMD_UNKNOWN ? m_command_code : CMD_UNKNOWN]
			, data, m_command_pos);
		write_control(data);
		break;
	case PHASE_TRANSFER:
		// transfer phase
		switch(m_command_code) {
		case CMD_WRITE:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// one block data wrote
				recv_next_disk_data();
			} else {
				// set drq and request to write
				set_request_delay(DELAY_TRANSFER_DATA, m_phase);
			}
			break;
		case CMD_CONFIG_DRIVE:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				finish_config_drive();
			} else {
				// set drq and request to write
				set_request_delay(DELAY_TRANSFER_DATA, m_phase);
			}
			break;
		default:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// go to status phase
				send_status(STA_COMPLETE, ERR_NO_STATUS);
			}
			break;
		}
		break;
	default:
		// unknown phase
		OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_phase, m_command_code, data, m_recv_data_pos);
		break;
	}
}

void SASI_CTRL::write_control(uint32_t data)
{
	switch(m_command_pos) {
	case 0:
		// command code
		parse_command_code(data & 0xff);
		break;
	case 1:
		// logical unit number and address (h)
		m_unit_number = ((data & 0xe0) >> 5);
		d_current_disk = d_relay->get_disk_unit(m_unit_number);
		break;
	default:
		break;
	}

	m_command_data[m_command_pos] = (data & 0xff);
	m_command_pos++;

	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);	// off request

	if (m_command_pos == m_command_len) {
		// parse parameter
		switch(m_command_class) {
		case 2:
			parse_command_param_group_2();
			break;
		case 1:
			parse_command_param_group_1();
			break;
		default:
			parse_command_param_group_0();
			break;
		}
		// start operation
		process_command();
	}
}

void SASI_CTRL::parse_command_param_group_0()
{
	// logical block address (h)
	m_curr_block = ((uint32_t)(m_command_data[1] & 0x1f) << 16);
	// logical block address (m)
	m_curr_block |= ((uint32_t)m_command_data[2] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[3];
	// number of blocks
	m_num_blocks = m_command_data[4];
	// control bytes
}

void SASI_CTRL::parse_command_param_group_1()
{
	// logical block address (h)
	m_curr_block = ((uint32_t)(m_command_data[1] & 0x1f) << 16);
	// logical block address (m)
	m_curr_block |= ((uint32_t)m_command_data[2] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[3];
	// number of blocks
	m_num_blocks = m_command_data[4];
	// dest. logical block address (h)
	// dest. logical block address (m)
	// dest. logical block address (l)
	// dest. device address
	// control bytes
}

void SASI_CTRL::parse_command_param_group_2()
{
	// logical block address (h)
	m_curr_block = ((uint32_t)m_command_data[2] << 24);
	// logical block address (m0)
	m_curr_block |= ((uint32_t)m_command_data[3] << 16);
	// logical block address (m1)
	m_curr_block |= ((uint32_t)m_command_data[4] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[5];
	// number of blocks
	m_num_blocks = m_command_data[6];
	// control bytes
}

uint32_t SASI_CTRL::read_data()
{
	uint32_t data = 0;

	switch(m_phase) {
	case PHASE_TRANSFER:
		switch(m_command_code) {
		case CMD_READ:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_send_data_pos);
			m_send_data_pos++;
			if (m_send_data_pos >= m_send_data_len) {
				send_next_disk_data();
			} else {
				// set drq and request to read
				set_request_delay(DELAY_TRANSFER_DATA, m_phase);
			}
			break;
		default:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_send_data_pos);
			m_send_data_pos++;
			if (m_send_data_pos >= m_send_data_len) {
				// go to status phase
				send_status(STA_COMPLETE, ERR_NO_STATUS);
			}
			break;
		}
		break;
	case PHASE_STATUS:
		// read status
		data = m_send_data[m_send_data_pos];
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Status Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		// go to message phase
		send_message();
		break;
	case PHASE_MESSAGE:
		// read message
		data = m_send_data[m_send_data_pos];
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Message Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		// complete, so go to busfree phase
		go_idle();
		break;
	default:
		// unknown phase
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_phase, m_command_code, data, m_send_data_pos);
		break;
	}

	return data;
}

// class 0 commands
const uint8_t SASI_CTRL::c_command_table[32] = {
	// 0                  1                2:Request Syndrome  3
	CMD_TEST_DRIVE_READY, CMD_RECALIBRATE, CMD_UNSUPPORTED,    CMD_REQUEST_SENSE,
	// 4              5:Check Track Format  6                 7
	CMD_FORMAT_DRIVE, CMD_UNSUPPORTED,      CMD_FORMAT_TRACK, CMD_FORMAT_BAD_TRACK,
	// 8      9:(none)         10         11
	CMD_READ, CMD_UNSUPPORTED, CMD_WRITE, CMD_SEEK,
	// 12:(none)     13:(none)        14:(none)        15:(none)
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 16:(none)     17:(none)        18:Reserve Unit  19:Release Unit
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 20:(none)     21:(none)        22:Read Capacity 23:(none)
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 24:(none)     25:(none)        26:Read Diag     27:Write Diag
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 28:(none)     29:(none)        30:(none)        31:Inquiry
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED
};

void SASI_CTRL::parse_command_code(uint8_t data)
{
	m_command_class = ((data >> 5) & 7);
	switch(m_command_class) {
	case 7:
		// 0xe0 ??
		m_command_code = CMD_DIAGNOSTIC;
		break;
	case 6:
		// 0xc2 ??
		m_command_code = CMD_CONFIG_DRIVE;
		break;
	default:
		m_command_code = c_command_table[data & 0x1f];
		break;
	}
	if (m_command_class == 1) {
		m_command_len = 10;
	} else {
		m_command_len = 6;
	}
}

void SASI_CTRL::process_command()
{
	switch(m_command_code) {
	case CMD_TEST_DRIVE_READY:
		// test drive ready
		test_drive_ready();
		break;
	case CMD_RECALIBRATE:
		// recalibrate
		recalibrate();
		break;
	case CMD_REQUEST_SENSE:
		// request sense
		send_sense();
		break;
	case CMD_FORMAT_DRIVE:
		// format drive
		format_disk();
		break;
	case CMD_FORMAT_TRACK:
		// format track
		format_track();
		break;
	case CMD_FORMAT_BAD_TRACK:
		// format bad track
		format_bad_track();
		break;
	case CMD_READ:
		// read datas, go to transfer phase
		send_first_disk_data();
		break;
	case CMD_WRITE:
		// write datas, go to transfer phase
		recv_first_disk_data();
		break;
	case CMD_SEEK:
		// seek
		seek();
		break;
	case CMD_CONFIG_DRIVE:
		// 0xc2 ??
		recv_config_drive();
		break;
	case CMD_DIAGNOSTIC:
		// 0xe0 ??
		diagnostic();
		break;
	default:
		// unsupport command, so go to status phase
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:Unknown"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number);
		send_status(STA_COMPLETE, ERR_NO_STATUS);
		break;
	}
}

/// TEST UNIT READY (00)
///
void SASI_CTRL::test_drive_ready()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[TEST DRIVE READY]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	send_status(STA_COMPLETE, ERR_NO_STATUS);
}

/// REZERO UNIT (01)
///
void SASI_CTRL::recalibrate()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int cylinder_diff = 0;
	bool rc = d_current_disk->seek(0, &cylinder_diff);
	if (!rc) {
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[RECALIBRATE] (Diff:%d Delay:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, cylinder_diff, delay);

	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

/// REQUEST SENSE (03)
///
void SASI_CTRL::send_sense()
{
	// go to transfer phase
	update_bus_status(SXSI_HOST::BUSTYPE_IO,  true);

	m_send_data_pos = 0;
	m_send_data_len = 4;
	m_send_data[0] = m_send_error;
	m_send_data[1] = (m_curr_block >> 16) & 0x1f;
	m_send_data[2] = (m_curr_block >> 8) & 0xff;
	m_send_data[3] = m_curr_block & 0xff;

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[REQUEST SENSE] Code:%02X"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_send_error);

	set_request_delay(8, PHASE_TRANSFER);
}

void SASI_CTRL::init_send_buffer(HARDDISK *disk)
{
	m_send_data_pos = 0;
	m_send_data_len = disk->get_sector_size();
	if (m_send_data_len > BUFFER_MAX) m_send_data_len = BUFFER_MAX;
}

void SASI_CTRL::send_disk_data(HARDDISK *disk)
{
	// go to transfer phase
	update_bus_status(SXSI_HOST::BUSTYPE_IO,  true);

	// read from hard disk
	int cylinder_diff = 0;
	bool rc = disk->read_buffer(m_curr_block, m_send_data_len, m_send_data, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Read Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// read error
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_diff);
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Start (Diff:%d Delay:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff, delay);

	play_seek_sound(cylinder_diff);

	// set request signal to read
	set_request_delay(delay, PHASE_TRANSFER);
}

/// READ (08)
/// send data to host
void SASI_CTRL::send_first_disk_data()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	init_send_buffer(d_current_disk);
	send_disk_data(d_current_disk);
}

void SASI_CTRL::send_next_disk_data()
{
	// one block was read
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		init_send_buffer(d_current_disk);
		send_disk_data(d_current_disk);

	} else {
		// complete
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Finish"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_curr_block);
		send_status(STA_COMPLETE, ERR_NO_STATUS);
	}
}

void SASI_CTRL::init_recv_buffer(HARDDISK *disk)
{
	m_recv_data_pos = 0;
	m_recv_data_len = disk->get_sector_size();
	if (m_recv_data_len > BUFFER_MAX) m_recv_data_len = BUFFER_MAX;
}

/// WRITE (0A)
/// receive data from host
void SASI_CTRL::recv_first_disk_data()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Start (BUS:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, d_host->get_bus_signal());

	init_recv_buffer(d_current_disk);

	// set REQ signal
	set_request_delay(DELAY_TRANSFER_PHASE, PHASE_TRANSFER);
}

void SASI_CTRL::recv_next_disk_data()
{
	// one block was written
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	// write to hard disk
	bool rc = d_current_disk->is_write_protected();
	if (rc) {
		// write protected
		send_status(STA_ERROR, ERR_WRITE_FAULT);
		return;
	}

	recv_disk_data(d_current_disk);
}

void SASI_CTRL::recv_disk_data(HARDDISK *disk)
{
	// go to transfer phase
	update_bus_status(SXSI_HOST::BUSTYPE_IO,  false);

	int cylinder_diff = 0;
	bool rc = disk->write_buffer(m_curr_block, m_recv_data_len, m_recv_data, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d Wrote Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// write error
		send_status(STA_ERROR, ERR_WRITE_FAULT);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;

	// set drq and request to write
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_diff);
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d (Diff:%d Delay:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff, delay);

	play_seek_sound(cylinder_diff);

	// next block
	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		init_recv_buffer(d_current_disk);
		// set REQ signal
		set_request_delay(delay, PHASE_TRANSFER);

	} else {
		// complete
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d Finish"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_curr_block);
		send_status(STA_COMPLETE, ERR_NO_STATUS);
	}
}

/// SEEK (0B)
///
void SASI_CTRL::seek()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int cylinder_diff = 0;
	bool rc = d_current_disk->seek(m_curr_block, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[SEEK] Block:%d Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// seek error
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}
	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

/// FORMAT UNIT (04)
///
void SASI_CTRL::format_disk()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[FORMAT DRIVE]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);
	d_current_disk->format_disk();
//	send_status(STA_COMPLETE, ERR_NO_STATUS);
	send_status_delay(5 * 1000 * 1000, STA_COMPLETE, ERR_NO_STATUS);
}

/// FORMAT TRACK (06)
///
void SASI_CTRL::format_track()
{
	// clear request
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);

	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int cylinder_diff = 0;
	d_current_disk->format_track(m_curr_block, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[FORMAT TRACK] Block:%d Start (Diff:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff);

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}

	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

/// FORMAT BAD TRACK (07)
///
void SASI_CTRL::format_bad_track()
{
	// clear request
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);

	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int cylinder_diff = 0;
	d_current_disk->format_track(m_curr_block, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[FORMAT BAD TRACK] Block:%d Start (Diff:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff);

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}

	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::recv_config_drive()
{
	// go to transfer phase (write)
	update_bus_status(SXSI_HOST::BUSTYPE_IO,  false);

	m_recv_data_pos = 0;
	m_recv_data_len = 10;

//	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:C2(Unknown)"), DEBUG_PREFIX
//		, m_ctrl_id, m_unit_number);

	if (!d_current_disk || !d_current_disk->mounted()) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	set_request_delay(DELAY_TRANSFER_PHASE, PHASE_TRANSFER);
}

void SASI_CTRL::finish_config_drive()
{
//	clr_drq();

#ifdef _DEBUG
	{
		_TCHAR buff[128];
		buff[0] = _T('\0');
		for(int i=0; i<10; i++) {
			UTILITY::sntprintf(buff, 128, _T(" %02X"), m_recv_data[i]);
		}
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:C2(Unknown) Finished %s"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number, buff);
	}
#endif
	// go to status phase
	send_status(STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::diagnostic()
{
	// unknown
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:E0(Unknown)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	send_status_delay(DELAY_TRANSFER_PHASE, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::send_status(uint8_t error_occured, uint8_t error_code, bool from_event)
{
	// go to status phase
	m_phase = PHASE_STATUS;

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = ((m_unit_number << 5) | error_occured) & 0x7f;
	m_send_message = MSG_COMPLETE;
	m_send_error = ((m_command_class << 4) | error_code);

	OUT_DEBUG_STAT(_T("%s C: Ctrl:%d-%d CMD:[%s] Status:%02X Code:%02X"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_command_code < CMD_UNKNOWN ? c_command_labels[m_command_code] : _T("Unknown")
		, m_send_data[0], error_code);

	if (from_event) {
		// Update bus signal
		request_next();
	}
}

void SASI_CTRL::send_status_delay(int delay, uint8_t error_occured, uint8_t error_code)
{
	// go to status phase
	m_phase = PHASE_STATUS;

	cancel_my_event(EVENT_STATUS);
	m_event_arg[EVENT_ARG_STATUS_TYPE] = error_occured;
	m_event_arg[EVENT_ARG_STATUS_CODE] = error_code;
	register_my_event(EVENT_STATUS, delay);
}


void SASI_CTRL::send_message()
{
	// go to message phase
	m_phase = PHASE_MESSAGE;

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = m_send_message;
}

/// go busfree phase
void SASI_CTRL::go_idle()
{
	// go to busfree phase
	m_phase = PHASE_BUSFREE;

	m_send_data_pos = 0;
	m_send_data_len = 0;
	m_send_data[0] = 0;
	m_recv_data_pos = 0;
	m_recv_data_len = 0;
	m_recv_data[0] = 0;
}

// ----------------------------------------------------------------------------

void SASI_CTRL::play_seek_sound(int sum)
{
	if (sum > 0) {
		d_host->play_seek_sound(d_relay->get_device_type());
		if (sum > 1 && !FLG_DELAY_HDSEEK) {
			register_seek_sound(sum, HARDDISK::get_cylinder_to_cylinder_delay());
		}
	}
}

void SASI_CTRL::register_seek_sound(int sum, int delay)
{
	m_event_arg[EVENT_ARG_SEEK_TIME] = (uint32_t)sum;
	cancel_my_event(EVENT_SEEK);
	register_my_event(EVENT_SEEK, delay);
}

// ----------------------------------------------------------------------------

#if 0
void SASI_CTRL::update_signal(uint32_t on, uint32_t off)
{
	d_host->update_signal_status(on, off);
}

void SASI_CTRL::update_bus_status(SXSI_HOST::SXSI_SIGNAL_STATUS_TYPE bus_type, bool onoff)
{
	d_host->update_bus_status(m_ctrl_id, bus_type, onoff);
}

void SASI_CTRL::update_signal_delay(int delay, uint32_t on, uint32_t off)
{
	cancel_my_event(EVENT_SIGNAL);
	m_event_arg[EVENT_ARG_SIGNAL_ON] = on;
	m_event_arg[EVENT_ARG_SIGNAL_OFF] = off;
	register_my_event(EVENT_SIGNAL, delay);
}

void SASI_CTRL::set_drq()
{
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
	d_host->set_drq(true);
}

void SASI_CTRL::clr_drq()
{
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
	d_host->set_drq(false);
}
#endif

void SASI_CTRL::set_request_delay(int delay, int next_phase)
{
	cancel_my_event(EVENT_REQ);
	m_event_arg[EVENT_ARG_NEXT_PHASE] = next_phase;
	register_my_event(EVENT_REQ, delay);
}

void SASI_CTRL::send_request()
{
	if (d_host->is_active_bus_status(SXSI_HOST::BUSTYPE_ACK)) {
		// ACK signal is not off yet, so keep current phase until ACK signal become off.
		OUT_DEBUG_SIG(_T("%s C: Ctrl:%d-%d REQ Event Phase:[%s] Keep current phase because ACK is on (SIG:%02X)"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, c_phase_labels[m_phase]
			, d_host->get_bus_signal());
		register_my_event(EVENT_REQ, 8);

	} else {
		// assert REQ signal on the SASI BUS
		m_phase = m_event_arg[EVENT_ARG_NEXT_PHASE];
		OUT_DEBUG_SIG(_T("%s C: Ctrl:%d-%d REQ event Phase:[%s] (SIG:%02X)"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, c_phase_labels[m_phase]
			, d_host->get_bus_signal());
		request_next();

	}
}

// ----------------------------------------------------------------------------

void SASI_CTRL::cancel_my_event(int event_id)
{
	if (m_register_id[event_id] >= 0) {
		cancel_event(this, m_register_id[event_id]);
		m_register_id[event_id] = -1;
	}
}

void SASI_CTRL::register_my_event(int event_id, int usec)
{
	register_event(this, event_id, usec, false, &m_register_id[event_id]);
}

// ----------------------------------------------------------------------------

void SASI_CTRL::event_callback(int event_id, int err)
{
	m_register_id[event_id] = -1;
	switch(event_id) {
	case EVENT_REQ:
		send_request();
		break;
	case EVENT_STATUS:
		send_status(m_event_arg[EVENT_ARG_STATUS_TYPE], m_event_arg[EVENT_ARG_STATUS_CODE], true);
		break;
	case EVENT_SEEK:
		m_event_arg[EVENT_ARG_SEEK_TIME]--;
		play_seek_sound((int)m_event_arg[EVENT_ARG_SEEK_TIME]);
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void SASI_CTRL::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	SXSI_CTRL::save_state_parts(vm_state.m_sxsi_ctrl);

	SET_Byte(m_send_message);
	SET_Byte(m_send_error);

	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		vm_state.m_event_arg[i] = Uint32_LE(m_event_arg[i]);
	}

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		vm_state.m_register_id[id] = Int32_LE(m_register_id[id]);
	}

	vm_state.m_send_data_pos = Int32_LE(m_send_data_pos);
	vm_state.m_send_data_len = Int32_LE(m_send_data_len);
	memcpy(vm_state.m_send_data, m_send_data, sizeof(vm_state.m_send_data));

	vm_state.m_recv_data_pos = Int32_LE(m_recv_data_pos);
	vm_state.m_recv_data_len = Int32_LE(m_recv_data_len);
	memcpy(vm_state.m_recv_data, m_recv_data, sizeof(vm_state.m_recv_data));

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool SASI_CTRL::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	SXSI_CTRL::load_state_parts(vm_state.m_sxsi_ctrl);

	GET_Byte(m_send_message);
	GET_Byte(m_send_error);

	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		m_event_arg[i] = Uint32_LE(vm_state.m_event_arg[i]);
	}
	for(int id=0; id<EVENT_IDS_MAX; id++) {
		m_register_id[id] = Int32_LE(vm_state.m_register_id[id]);
	}
	m_send_data_pos = Int32_LE(vm_state.m_send_data_pos);
	m_send_data_len = Int32_LE(vm_state.m_send_data_len);
	memcpy(m_send_data, vm_state.m_send_data, sizeof(m_send_data));

	m_recv_data_pos = Int32_LE(vm_state.m_recv_data_pos);
	m_recv_data_len = Int32_LE(vm_state.m_recv_data_len);
	memcpy(m_recv_data, vm_state.m_recv_data, sizeof(m_recv_data));

	d_current_disk = d_relay->get_disk_unit(m_unit_number);

	return true;
}

// ----------------------------------------------------------------------------
#if 0
bool SASI_CTRL::open_disk(int unit_num, const _TCHAR *path, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->open(path, 256, flags);
}

bool SASI_CTRL::close_disk(int unit_num, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	disk->close();
	return true;
}

bool SASI_CTRL::disk_mounted(int unit_num) const
{
	return unit_mounted(unit_num);
}

void SASI_CTRL::set_write_protect(int unit_num, bool val)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return;
	}
	disk->set_write_protect(val);
}

bool SASI_CTRL::write_protected(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_write_protected();
}

bool SASI_CTRL::is_same_disk(int unit_num, const _TCHAR *path)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_same_file(path);
}
#endif

// ----------------------------------------------------------------------------
#if 0
HARDDISK *SASI_CTRL::get_disk_unit(int unit_num) const
{
	if (d_units->Count() <= unit_num) {
		return NULL;
	}
	return d_units->Item(unit_num);
}

bool SASI_CTRL::unit_mounted(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->mounted();
}

bool SASI_CTRL::unit_mounted_at_least() const
{
	bool rc = false;
	const SXSI_UNITS *units = get_disk_units();
	for(int unit=0; unit<units->Count(); unit++) {
		rc |= units->Item(unit)->mounted();
	}
	return rc;
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void SASI_CTRL::debug_write_data(uint32_t data)
{
	m_recv_data[m_recv_data_pos] = data & 0xff;
}

uint32_t SASI_CTRL::debug_read_data()
{
	return m_send_data[m_send_data_pos];
}

void SASI_CTRL::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T("SASI CTRL: ID:%d Unit:%d\n"), m_ctrl_id, m_unit_number);
	UTILITY::tcscat(buffer, buffer_len, _T("  Current Phase:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_phase < PHASE_UNKNOWN ? c_phase_labels[m_phase] : _T("Unknown")
	);
	UTILITY::tcscat(buffer, buffer_len, _T("  Command:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_command_code < CMD_UNKNOWN ? c_command_labels[m_command_code] : _T("Unknown")
	);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	for(int i=0; i<m_command_pos; i++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), m_command_data[i]);
	}
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
}
#endif

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SASI_CTRL_RELAY::SASI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id)
	: SXSI_CTRL_RELAY(parent_vm, parent_emu, identifier, host, ctrl_id, SASI_UNITS_PER_CTRL)
{
	set_class_name("SASI_CTRL_RE");
	m_device_type = HARDDISK::DEVICE_TYPE_SASI_HDD;

	SASI_CTRL *ctrl0 = new SASI_CTRL(vm, emu, "A", host, this, ctrl_id);
	SCSI_CTRL *ctrl1 = new SCSI_CTRL(vm, emu, "A", host, this, ctrl_id);
	add_ctrl(ctrl0);
	add_ctrl(ctrl1);
}

void SASI_CTRL_RELAY::change_device_type(int num)
{
	if (num < 0 || num >= HARDDISK::DEVICE_TYPE_MAX) {
		num = HARDDISK::DEVICE_TYPE_SASI_HDD;
	}

	// change control
	switch(num) {
	case HARDDISK::DEVICE_TYPE_SCSI_HDD:
	case HARDDISK::DEVICE_TYPE_SCSI_MO:
		d_selected_ctrl = d_ctrls.at(1);
		break;
	default:
		d_selected_ctrl = d_ctrls.at(0);
		break;
	}

	m_device_type = num;
}

bool SASI_CTRL_RELAY::unit_mounted_at_least() const
{
	bool rc = false;
	for(int unit=0; unit<d_units->Count(); unit++) {
		switch(m_device_type) {
		case HARDDISK::DEVICE_TYPE_SCSI_MO:
			// MO is removable
			rc = true;
			break;
		default:
			rc |= d_units->Item(unit)->is_valid_disk(m_device_type);
			break;
		}
	}
	return rc;
}

#if 0
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SASI_CTRLS::SASI_CTRLS()
	: std::vector<SASI_CTRL *>()
{
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SASI_UNITS::SASI_UNITS(int alloc_size)
	: CPtrList<HARDDISK>(alloc_size)
{
}
#endif
