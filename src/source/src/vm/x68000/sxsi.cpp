/** @file sxsi.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.03 -

	@brief [ SASI/SCSI base class ]
*/

#include "sxsi.h"
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
//#define OUT_DEBUG_SEL(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_SEL(...)
//#define OUT_DEBUG_CMD(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_CMD(...)
//#define OUT_DEBUG_DATW(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATW(...)
//#define OUT_DEBUG_DATR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATR(...)
//#define OUT_DEBUG_STAT(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_STAT(...)
#else
#define OUT_DEBUG_SIG(...)
#define OUT_DEBUG_SEL(...)
#define OUT_DEBUG_CMD(...)
#define OUT_DEBUG_DATW(...)
#define OUT_DEBUG_DATR(...)
#define OUT_DEBUG_STAT(...)
#endif

// ----------------------------------------------------------------------

SXSI_HOST::SXSI_HOST(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: SOUND_BASE(parent_vm, parent_emu, identifier)
{
	init_output_signals(&outputs_irq);
	init_output_signals(&outputs_drq);

	d_ctrls = new SXSI_CTRLS();
}

SXSI_HOST::~SXSI_HOST()
{
	delete d_ctrls;
}

void SXSI_HOST::initialize()
{
	m_noises[SXSI_WAV_HDD_SEEK].set_file_name(_T("hddseek.wav"));
	m_noises[SXSI_WAV_MO_EJECT].set_file_name(_T("moeject.wav"));
	m_wav_loaded_at_first = false;
}

const _TCHAR *SXSI_HOST::c_sigtype_names[] = {
	_T("IO "),
	_T("CD "),
	_T("MSG"),
	_T("BSY"),
	_T("SEL"),
	_T("ATN"),
	_T("ACK"),
	_T("REQ"),
	NULL
};

void SXSI_HOST::update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff)
{
}

bool SXSI_HOST::is_active_bus_status(SXSI_SIGNAL_STATUS_TYPE sig_type) const
{
	return (m_bus_status[sig_type] != 0);
}

// ----------------------------------------------------------------------------

void SXSI_HOST::save_state_parts(struct vm_state_st &vm_state)
{
	for(int ty=0; ty<SXSI_WAV_SNDTYPES; ty++) {
		m_noises[ty].save_state(vm_state.m_noises[ty]);
	}
	for(int i=0; i<BUSTYPE_MAX; i++) {
		vm_state.m_bus_status[i] = Uint32_LE(m_bus_status[i]);
	}
}

void SXSI_HOST::load_state_parts(const struct vm_state_st &vm_state)
{
	for(int ty=0; ty<SXSI_WAV_SNDTYPES; ty++) {
		m_noises[ty].load_state(vm_state.m_noises[ty]);
	}
	for(int i=0; i<BUSTYPE_MAX; i++) {
		m_bus_status[i] = Uint32_LE(vm_state.m_bus_status[i]);
	}
}

// ----------------------------------------------------------------------------

void SXSI_HOST::load_wav()
{
	// allocation
	m_noises[SXSI_WAV_HDD_SEEK].alloc(m_sample_rate / 10);		// 0.1sec max
	m_noises[SXSI_WAV_MO_EJECT].alloc(m_sample_rate * 2);		// 2.0sec max

	// load wav file
	load_wav_files(m_noises, SXSI_WAV_SNDTYPES, m_wav_loaded_at_first);

	m_wav_loaded_at_first = true;
}

void SXSI_HOST::initialize_sound(int rate, int decibel)
{
	SOUND_BASE::initialize_sound(rate, decibel);

	// load wav file
	load_wav();
}

void SXSI_HOST::mix(int32_t* buffer, int cnt)
{
	for(int ty=0; ty<SXSI_WAV_SNDTYPES; ty++) {
		m_noises[ty].mix(buffer, cnt);
	}
}

void SXSI_HOST::set_volume(int decibel, bool vol_mute)
{
	int wav_volume = decibel_to_volume(decibel);

	if (vol_mute) wav_volume = 0;
	for(int ty=0; ty<SXSI_WAV_SNDTYPES; ty++) {
		m_noises[ty].set_volume(wav_volume, wav_volume);
	}
}

// ----------------------------------------------------------------------------

void SXSI_HOST::play_seek_sound(int device_type)
{
	switch(device_type) {
	case HARDDISK::DEVICE_TYPE_SASI_HDD:
	case HARDDISK::DEVICE_TYPE_SCSI_HDD:
		m_noises[SXSI_WAV_HDD_SEEK].play();
		break;
	default:
		break;
	}
}

void SXSI_HOST::play_eject_sound(int device_type)
{
	switch(device_type) {
	case HARDDISK::DEVICE_TYPE_SCSI_MO:
		m_noises[SXSI_WAV_MO_EJECT].play();
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SXSI_CTRL::SXSI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id)
	: DEVICE(parent_vm, parent_emu, NULL)
{
	d_host = host;
	d_relay = relay;
	m_ctrl_id = ctrl_id;
	m_unit_number = 0;
	d_current_disk = NULL;

	char s_identifier[32];
	UTILITY::sprintf(s_identifier, sizeof(s_identifier), "%s%d", identifier, ctrl_id);
	set_identifier(s_identifier);
}

SXSI_CTRL::~SXSI_CTRL()
{
}

// ----------------------------------------------------------------------------

bool SXSI_CTRL::select()
{
	clear();

	bool mounted = d_relay->unit_mounted_at_least();
	if (mounted) {
		// set busy
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD,  false);
		update_bus_status(SXSI_HOST::BUSTYPE_IO,  false);
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_BSY, true);
	}
	return mounted;
}

void SXSI_CTRL::update_bus_status(SXSI_HOST::SXSI_SIGNAL_STATUS_TYPE bus_type, bool onoff)
{
	d_host->update_bus_status(m_ctrl_id, bus_type, onoff);
}

// ----------------------------------------------------------------------------
#if 0
const SXSI_UNITS *SXSI_CTRL::get_disk_units() const
{
	return d_relay->get_units();
}

HARDDISK *SXSI_CTRL::get_disk_unit(int unit_num) const
{
	return d_relay->get_disk_unit(unit_num);
}

bool SXSI_CTRL::unit_mounted(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->mounted();
}

bool SXSI_CTRL::unit_mounted_at_least() const
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

#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)

void SXSI_CTRL::save_state_parts(struct vm_state_st &vm_state)
{
	vm_state.revision = Int32_LE(0);
	SET_Int32_LE(m_phase);
	SET_Byte(m_unit_number);
	SET_Byte(m_command_class);
	SET_Byte(m_command_code);
	SET_Byte(m_command_len);
	SET_Int32_LE(m_command_pos);

	memcpy(vm_state.m_command_data, m_command_data, sizeof(vm_state.m_command_data));

	SET_Uint32_LE(m_curr_block);
	SET_Int32_LE(m_num_blocks);
}

#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)

void SXSI_CTRL::load_state_parts(const struct vm_state_st &vm_state)
{
	GET_Int32_LE(m_phase);
	GET_Byte(m_unit_number);
	GET_Byte(m_command_class);
	GET_Byte(m_command_code);
	GET_Byte(m_command_len);
	GET_Int32_LE(m_command_pos);

	memcpy(m_command_data, vm_state.m_command_data, sizeof(m_command_data));

	GET_Uint32_LE(m_curr_block);
	GET_Int32_LE(m_num_blocks);
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SXSI_CTRL_RELAY::SXSI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id, int num_of_units)
	: DEVICE(parent_vm, parent_emu, NULL)
{
	m_ctrl_id = ctrl_id;
	d_host = host;
	d_selected_ctrl = NULL;

	d_units = new SXSI_UNITS(num_of_units);
	for(int i=0; i<num_of_units; i++) {
		d_units->Add(new HARDDISK(ctrl_id));
	}

	char s_identifier[32];
	UTILITY::sprintf(s_identifier, sizeof(s_identifier), "%s%d", identifier, ctrl_id);
	set_identifier(s_identifier);
}

SXSI_CTRL_RELAY::~SXSI_CTRL_RELAY()
{
	delete d_units;
}

void SXSI_CTRL_RELAY::add_ctrl(SXSI_CTRL *ctrl)
{
	d_ctrls.push_back(ctrl);
	if (!d_selected_ctrl) {
		d_selected_ctrl = ctrl;
	}
}

// ----------------------------------------------------------------------------

/// @brief select this disk
/// @return true:can use this / false:cannot use
bool SXSI_CTRL_RELAY::select()
{
	return d_selected_ctrl->select();
}

/// @brief selected this disk
void SXSI_CTRL_RELAY::go_command_phase()
{
	d_selected_ctrl->go_command_phase();
}

/// @brief go message out phase after selected this disk
void SXSI_CTRL_RELAY::go_message_out_phase()
{
	d_selected_ctrl->go_message_out_phase();
}

/// @brief host assert ack, and the control receive this  
void SXSI_CTRL_RELAY::accept_ack()
{
	d_selected_ctrl->accept_ack();
}

/// @brief host negate ack, the control assert req to do next process  
void SXSI_CTRL_RELAY::request_next()
{
	d_selected_ctrl->request_next();
}

void SXSI_CTRL_RELAY::write_data(uint32_t data)
{
	d_selected_ctrl->write_data(data);
}

uint32_t SXSI_CTRL_RELAY::read_data()
{
	return d_selected_ctrl->read_data();
}

int SXSI_CTRL_RELAY::get_phase() const
{
	return d_selected_ctrl->get_phase();
}

int SXSI_CTRL_RELAY::get_command_pos() const
{
	return d_selected_ctrl->get_command_pos();
}

// ----------------------------------------------------------------------------

bool SXSI_CTRL_RELAY::open_disk(int unit_num, const _TCHAR *path, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->open(path, flags);
}

bool SXSI_CTRL_RELAY::close_disk(int unit_num, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	disk->close();
	d_host->play_eject_sound(get_device_type());
	return true;
}

bool SXSI_CTRL_RELAY::disk_mounted(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->mounted();
}

void SXSI_CTRL_RELAY::set_write_protect(int unit_num, bool val)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return;
	}
	disk->set_write_protect(val);
}

bool SXSI_CTRL_RELAY::write_protected(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_write_protected();
}

bool SXSI_CTRL_RELAY::is_same_disk(int unit_num, const _TCHAR *path)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_same_file(path);
}

// ----------------------------------------------------------------------------

void SXSI_CTRL_RELAY::change_device_type(int num)
{
	m_device_type = num;
}

int SXSI_CTRL_RELAY::get_device_type() const
{
	return m_device_type;
}

// ----------------------------------------------------------------------------

HARDDISK *SXSI_CTRL_RELAY::get_disk_unit(int unit_num) const
{
	if (d_units->Count() <= unit_num) {
		return NULL;
	}
	return d_units->Item(unit_num);
}

#if 0
bool SXSI_CTRL_RELAY::unit_mounted(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->mounted();
}
#endif

bool SXSI_CTRL_RELAY::unit_mounted_at_least() const
{
	bool rc = false;
	for(int unit=0; unit<d_units->Count(); unit++) {
		rc |= d_units->Item(unit)->mounted();
	}
	return rc;
}

// ----------------------------------------------------------------------------

void SXSI_CTRL_RELAY::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	int idx = 0;
	for(; idx < (int)d_ctrls.size(); idx++) {
		if (d_selected_ctrl == d_ctrls.at(idx)) {
			break;
		}
	}
	vm_state.m_selected_ctrl_idx = Int32_LE(idx);
	vm_state.m_device_type = Int32_LE(m_device_type);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool SXSI_CTRL_RELAY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	int idx = Int32_LE(vm_state.m_selected_ctrl_idx);
	if (idx <= 0 && idx < (int)d_ctrls.size()) {
		d_selected_ctrl = d_ctrls.at(idx);
	}
	m_device_type = Int32_LE(vm_state.m_device_type);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void SXSI_CTRL_RELAY::debug_write_data(uint32_t data)
{
	d_selected_ctrl->debug_write_data(data);
}

uint32_t SXSI_CTRL_RELAY::debug_read_data()
{
	return d_selected_ctrl->debug_read_data();
}

void SXSI_CTRL_RELAY::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	d_selected_ctrl->debug_regs_info(buffer, buffer_len);
}
#endif

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SXSI_CTRLS::SXSI_CTRLS()
	: std::vector<SXSI_CTRL_RELAY *>()
{
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SXSI_UNITS::SXSI_UNITS(int alloc_size)
	: CPtrList<HARDDISK>(alloc_size)
{
}
