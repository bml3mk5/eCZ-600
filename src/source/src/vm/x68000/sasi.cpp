/** @file sasi.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.03 -

	@brief [ shugart associates system interface ]
*/

#include "sasi.h"
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

SASI::SASI(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: SOUND_BASE(parent_vm, parent_emu, identifier)
{
	set_class_name("SASI");
	init_output_signals(&outputs_irq);
	init_output_signals(&outputs_drq);

	d_ctrls = new SASI_CTRLS();
}

SASI::~SASI()
{
	delete d_ctrls;
}

void SASI::initialize()
{
	int max_id = MAX_HARD_DISKS / SASI_CTRL::UNITS_PER_CTRL;
	for(int id=0; id<max_id; id++) {
		d_ctrls->push_back(new SASI_CTRL(vm, emu, this, id));
	}
	d_selected_ctrl = NULL;

	m_signal_status = 0;

	m_noises[SASI_WAV_SEEK].set_file_name(_T("hddseek.wav"));
	m_wav_loaded_at_first = false;
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
	if (!por) {
		for(int id=0; id<(int)d_ctrls->size(); id++) {
			d_ctrls->at(id)->warm_reset(por);
		}
	}
	d_selected_ctrl = NULL;
	m_signal_status = 0;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	// $E96000 >> 1
	switch(addr & 0x3) {
	case 0x00:
		// SASI data write
		if (d_selected_ctrl) {
			d_selected_ctrl->write_data(data);
		}
		break;
	case 0x01:
		// SEL signal negate
		m_signal_status &= ~SIGSTAT_SEL;
		if (d_selected_ctrl) {
			d_selected_ctrl->selected(data);
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
			data = d_selected_ctrl->read_data();
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
	m_signal_status |= SIGSTAT_SEL;
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
		d_selected_ctrl->select();
	}
	OUT_DEBUG_SEL(_T("SASI: Select: Req:%02X Select:%d Status:%04X"), data, d_selected_ctrl ? id : -1, m_signal_status); 
}

void SASI::update_signal_status(uint32_t on, uint32_t off)
{
	m_signal_status |= on;
	m_signal_status &= ~off;
}

// ----------------------------------------------------------------------------

void SASI::set_irq(bool onoff)
{
	write_signals(&outputs_irq, onoff ? 0xffffffff : 0);
}

void SASI::clr_drq(int ctrl_num)
{
	m_signal_status &= ~(SIGSTAT_DRQ0 << ctrl_num);
	bool onoff = ((m_signal_status & SIGSTAT_DRQ_MASK) != 0);
	BIT_ONOFF(m_signal_status, SIGSTAT_REQ, onoff);
	OUT_DEBUG_SIG(_T("SASI: Update status:%04X"), m_signal_status); 
	write_signals(&outputs_drq, onoff ? 0xffffffff : 0);
}

void SASI::set_drq(int ctrl_num)
{
	m_signal_status |= (SIGSTAT_DRQ0 << ctrl_num);
	bool onoff = ((m_signal_status & SIGSTAT_DRQ_MASK) != 0);
	BIT_ONOFF(m_signal_status, SIGSTAT_REQ, onoff);
	OUT_DEBUG_SIG(_T("SASI: Update status:%04X"), m_signal_status); 
	write_signals(&outputs_drq, onoff ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------

void SASI::load_wav()
{
	// allocation
	m_noises[SASI_WAV_SEEK].alloc(m_sample_rate / 10);		// 0.1sec max

	// load wav file
	load_wav_files(m_noises, SASI_WAV_SNDTYPES, m_wav_loaded_at_first);

	m_wav_loaded_at_first = true;
}

void SASI::initialize_sound(int rate, int decibel)
{
	SOUND_BASE::initialize_sound(rate, decibel);

	// load wav file
	load_wav();
}

void SASI::mix(int32_t* buffer, int cnt)
{
	for(int ty=0; ty<SASI_WAV_SNDTYPES; ty++) {
		m_noises[ty].mix(buffer, cnt);
	}
}

void SASI::set_volume(int decibel, bool vol_mute)
{
	int wav_volume = decibel_to_volume(decibel);

	if (vol_mute) wav_volume = 0;
	for(int ty=0; ty<SASI_WAV_SNDTYPES; ty++) {
		m_noises[ty].set_volume(wav_volume, wav_volume);
	}
}

// ----------------------------------------------------------------------------

void SASI::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	if (d_selected_ctrl) {
		vm_state.m_selected_ctrl_id = Int32_LE(d_selected_ctrl->get_ctrl_id());
	} else {
		vm_state.m_selected_ctrl_id = Int32_LE(-1);
	}
	vm_state.m_signal_status =Uint32_LE(m_signal_status);
	for(int ty=0; ty<SASI_WAV_SNDTYPES; ty++) {
		m_noises[ty].save_state(vm_state.m_noises[ty]);
	}

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool SASI::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	int ctrl_id = Int32_LE(vm_state.m_selected_ctrl_id);
	if (ctrl_id >= 0 && ctrl_id < (int)d_ctrls->size()) {
		d_selected_ctrl = d_ctrls->at(ctrl_id);
	} else {
		d_selected_ctrl = NULL;
	}
	m_signal_status =Uint32_LE(vm_state.m_signal_status);
	for(int ty=0; ty<SASI_WAV_SNDTYPES; ty++) {
		m_noises[ty].load_state(vm_state.m_noises[ty]);
	}

	return true;
}

// ----------------------------------------------------------------------------

bool SASI::open_disk(int drv, const _TCHAR *path, uint32_t flags)
{
	int id = (drv / SASI_CTRL::UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_CTRL::UNITS_PER_CTRL);
	return d_ctrls->at(id)->open_disk(unit_num, path, flags);
}

bool SASI::close_disk(int drv, uint32_t flags)
{
	int id = (drv / SASI_CTRL::UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_CTRL::UNITS_PER_CTRL);
	return d_ctrls->at(id)->close_disk(unit_num, flags);
}

bool SASI::disk_mounted(int drv)
{
	int id = (drv / SASI_CTRL::UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SASI_CTRL::UNITS_PER_CTRL);
	return d_ctrls->at(id)->disk_mounted(unit_num);
}

bool SASI::is_same_disk(int drv, const _TCHAR *path)
{
	int curr_id = (drv / SASI_CTRL::UNITS_PER_CTRL);
	int curr_unit = (drv % SASI_CTRL::UNITS_PER_CTRL);
	bool match = false;
	for(int id=0; id<(int)d_ctrls->size() && !match; id++) {
		for(int unit_num=0; unit_num<SASI_CTRL::UNITS_PER_CTRL && !match; unit_num++) {
			if (curr_id == id && curr_unit == unit_num) continue;
			match = d_ctrls->at(id)->is_same_disk(unit_num, path);
		}
	}
	return match;
}

/// b12 : hdbusy
uint32_t SASI::get_led_status() const
{
	return (m_signal_status & SIGSTAT_BSY) << 11;
}

// ----------------------------------------------------------------------------

void SASI::play_seek_sound()
{
	m_noises[SASI_WAV_SEEK].play();
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
	UTILITY::stprintf(buffer, buffer_len, _T("Current Control:%d  Status:%02X")
		, d_selected_ctrl ? d_selected_ctrl->get_ctrl_id() : -1
		, m_signal_status & SIGSTAT_MASK
	);
	if (d_selected_ctrl) {
		d_selected_ctrl->debug_regs_info(buffer, buffer_len);
	}
}
#endif

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SASI_CTRL::SASI_CTRL(VM* parent_vm, EMU* parent_emu, SASI *host, int ctrl_id)
	: DEVICE(parent_vm, parent_emu, NULL)
{
	d_host = host;
	m_ctrl_id = ctrl_id;

	// can connect two units per control
	d_units = new SASI_UNITS(UNITS_PER_CTRL);
	for(int i=m_ctrl_id*UNITS_PER_CTRL; i<MAX_HARD_DISKS && i<(m_ctrl_id+1)*UNITS_PER_CTRL; i++) {
		d_units->Add(new HARDDISK(ctrl_id));
	}

	char s_identifier[32];
	UTILITY::sprintf(s_identifier, sizeof(s_identifier), "%d", ctrl_id);
	set_identifier(s_identifier);

	clear();
}

SASI_CTRL::~SASI_CTRL()
{
	delete d_units;
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

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		m_register_id[id] = -1;
	}
	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		m_event_arg[i] = 0;
	}

	d_current_disk = get_disk_unit(m_unit_number);
}

void SASI_CTRL::initialize()
{
	clear();
}

void SASI_CTRL::reset()
{
	clear();
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

/// @brief select this disk
/// @return true:can use this / false:cannot use
bool SASI_CTRL::select()
{
	clear();

	bool mounted = unit_mounted_at_least();
	if (mounted) {
		// set busy
		update_signal(
			SASI::SIGSTAT_BSY,
			SASI::SIGSTAT_REQ | SASI::SIGSTAT_CD | SASI::SIGSTAT_IO | SASI::SIGSTAT_MSG
		);
	}
	return mounted;
}

/// @brief selected this disk
void SASI_CTRL::selected(uint32_t data)
{
	bool mounted = unit_mounted_at_least();
	if (mounted) {
		// go to command phase
		update_signal(
			SASI::SIGSTAT_REQ | SASI::SIGSTAT_BSY | SASI::SIGSTAT_CD,
			SASI::SIGSTAT_IO | SASI::SIGSTAT_MSG
		);
		m_phase = PHASE_COMMAND;
		OUT_DEBUG_SEL(_T("SASI: Ctrl:%d Selected"), m_ctrl_id); 
	}
}

void SASI_CTRL::write_data(uint32_t data)
{
	switch(m_phase) {
	case PHASE_COMMAND:
		// command phase
		OUT_DEBUG_CMD(_T("SASI: WR Command Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_recv_data_pos);
		write_control(data);
		break;
	case PHASE_TRANSFER:
		// transfer phase
		switch(m_command_code) {
		case CMD_WRITE:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("SASI: WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// one block data wrote
				recv_next_disk_data();
			} else {
				// set drq and request to write
				clr_drq();
				set_drq_delay(DELAY_TRANSFER_DATA);
			}
			break;
		case CMD_EXTEND_ARGS:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("SASI: WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				finish_extended_args();
			} else {
				// set drq and request to write
				clr_drq();
				set_drq_delay(DELAY_TRANSFER_DATA);
			}
			break;
		default:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("SASI: WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_recv_data_pos);
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
		OUT_DEBUG_DATW(_T("SASI: WR Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), m_phase, m_command_code, data, m_recv_data_pos);
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
		m_curr_block = ((data & 0x1f) << 16);

		d_current_disk = get_disk_unit(m_unit_number);
		break;
	case 2:
		// logical unit address (m)
		m_curr_block |= ((data & 0xff) << 8);
		break;
	case 3:
		// logical unit address (l)
		m_curr_block |= (data & 0xff);
		break;
	case 4:
		// number of blocks
		m_num_blocks = (data & 0xff);
		break;
	case 5:
		// control bytes
		break;
	default:
		break;
	}
	m_command_pos++;

	if (m_command_pos == m_command_len) {
		// start operation
		update_signal(
			0,
			SASI::SIGSTAT_REQ	// off request
		);
		process_command();
	}
}

uint32_t SASI_CTRL::read_data()
{
	uint32_t data = 0;
	switch(m_phase) {
	case PHASE_TRANSFER:
		switch(m_command_code) {
		case CMD_READ:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATR(_T("SASI: RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_send_data_pos);
			m_send_data_pos++;
			if (m_send_data_pos >= m_send_data_len) {
				send_next_disk_data();
			} else {
				// set drq and request to read
				clr_drq();
				set_drq_delay(DELAY_TRANSFER_DATA);
			}
			break;
		default:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATR(_T("SASI: RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_send_data_pos);
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
		OUT_DEBUG_DATR(_T("SASI: RD Status Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		// go to message phase
		send_message();
		break;
	case PHASE_MESSAGE:
		// read message
		data = m_send_data[m_send_data_pos];
		OUT_DEBUG_DATR(_T("SASI: RD Message Phase Cmd:%d Data:%02X Pos:%d"), m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		// complete, so go to busfree phase
		go_idle();
		break;
	default:
		// unknown phase
		OUT_DEBUG_DATR(_T("SASI: RD Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), m_phase, m_command_code, data, m_send_data_pos);
		break;
	}

	return data;
}

// class 0 commands
const uint8_t SASI_CTRL::c_command_table[32] = {
	// 0                  1                2:Request Syndrome  3
	CMD_TEST_DRIVE_READY, CMD_RECALIBRATE, CMD_UNSUPPORTED,    CMD_REQUEST_SENSE,
	// 4              5:Check Track Format  6                 7:Format Bad Track
	CMD_FORMAT_DRIVE, CMD_UNSUPPORTED,      CMD_FORMAT_TRACK, CMD_UNSUPPORTED,
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
		m_command_code = CMD_EXTEND_ARGS;
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
	case CMD_EXTEND_ARGS:
		// 0xc2 ??
		recv_extended_args();
		break;
	case CMD_DIAGNOSTIC:
		// 0xe0 ??
		diagnostic();
		break;
	default:
		// unsupport command, so go to status phase
		OUT_DEBUG_CMD(_T("SASI: CMD:Unknown Unit:%d"), m_unit_number );
		send_status(STA_COMPLETE, ERR_NO_STATUS);
		break;
	}
}

void SASI_CTRL::test_drive_ready()
{
	OUT_DEBUG_CMD(_T("SASI: CMD:TEST_DRIVE_READY Unit:%d"), m_unit_number);

	bool rc = unit_mounted(m_unit_number);
	if (!rc) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}
	send_status(STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::recalibrate()
{
	OUT_DEBUG_CMD(_T("SASI: CMD:RECALIBRATE Unit:%d"), m_unit_number);

	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	bool rc = d_current_disk->seek(0);
	if (!rc) {
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}
	
	int sum = d_current_disk->get_cylinder_diff(0);
	play_seek_sound(sum);

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(sum);
	}
	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::send_sense()
{
	// go to transfer phase
	update_signal(
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_IO,
		SASI::SIGSTAT_MSG | SASI::SIGSTAT_CD
	);
	m_phase = PHASE_TRANSFER;

	m_send_data_pos = 0;
	m_send_data_len = 4;
	m_send_data[0] = m_send_error;
	m_send_data[1] = (m_curr_block >> 16) & 0x1f;
	m_send_data[2] = (m_curr_block >> 8) & 0xff;
	m_send_data[3] = m_curr_block & 0xff;

	OUT_DEBUG_CMD(_T("SASI: CMD:REQUEST_SENSE Unit:%d Code:%02X"), m_unit_number, m_send_error);
}

void SASI_CTRL::send_disk_data(HARDDISK *disk, int cylinder_sum)
{
	// go to transfer phase
	update_signal(
		SASI::SIGSTAT_IO,
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_MSG | SASI::SIGSTAT_CD
	);
	m_phase = PHASE_TRANSFER;

	m_send_data_pos = 0;
	m_send_data_len = 256;

	// read from hard disk
	bool rc = disk->read_buffer(m_curr_block, m_send_data_len, m_send_data);
	OUT_DEBUG_CMD(_T("SASI: CMD:READ Unit:%d Block:%d Read Result:%s"), m_unit_number, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// read error
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_sum);
	}
	play_seek_sound(cylinder_sum);

	// set drq and request to read
	set_drq_delay(delay);
}

void SASI_CTRL::send_first_disk_data()
{
	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int sum = d_current_disk->get_cylinder_diff(m_curr_block);
	OUT_DEBUG_CMD(_T("SASI: CMD:READ Unit:%d Block:%d Start (Sum:%d)"), m_unit_number, m_curr_block, sum);
	send_disk_data(d_current_disk, sum);
}

void SASI_CTRL::send_next_disk_data()
{
	// one block was read
	clr_drq();

	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		int sum = d_current_disk->get_cylinder_diff(m_curr_block);
		OUT_DEBUG_CMD(_T("SASI: CMD:READ Unit:%d Block:%d Next (Sum:%d)"), m_unit_number, m_curr_block, sum);
		send_disk_data(d_current_disk, sum);
	} else {
		// complete
		OUT_DEBUG_CMD(_T("SASI: CMD:READ Unit:%d Block:%d Finish"), m_unit_number, m_curr_block);
		send_status(STA_COMPLETE, ERR_NO_STATUS);
	}
}

void SASI_CTRL::recv_disk_data(HARDDISK *disk, int cylinder_sum)
{
	// go to transfer phase
	update_signal(
		0,
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_MSG | SASI::SIGSTAT_IO | SASI::SIGSTAT_CD
	);
	m_phase = PHASE_TRANSFER;

	m_recv_data_pos = 0;
	m_recv_data_len = 256;

	int delay = DELAY_TRANSFER_PHASE;
	play_seek_sound(cylinder_sum);

	// set drq and request to write
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_sum);
	}
	set_drq_delay(delay);
}

void SASI_CTRL::recv_first_disk_data()
{
	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int sum = d_current_disk->get_cylinder_diff(m_curr_block);
	OUT_DEBUG_CMD(_T("SASI: CMD:WRITE Unit:%d Block:%d Start (Sum:%d)"), m_unit_number, m_curr_block, sum);
	recv_disk_data(d_current_disk, sum);
}

void SASI_CTRL::recv_next_disk_data()
{
	// one block was written
	clr_drq();

	if (!d_current_disk) {
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

	rc = d_current_disk->write_buffer(m_curr_block, m_recv_data_len, m_recv_data);
	OUT_DEBUG_CMD(_T("SASI: CMD:WRITE Unit:%d Block:%d Wrote Result:%s"), m_unit_number, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// write error
		send_status(STA_ERROR, ERR_WRITE_FAULT);
		return;
	}

	// next block
	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		int sum = d_current_disk->get_cylinder_diff(m_curr_block);
		OUT_DEBUG_CMD(_T("SASI: CMD:WRITE Unit:%d Block:%d Next (Sum:%d)"), m_unit_number, m_curr_block, sum);
		recv_disk_data(d_current_disk, sum);
	} else {
		// complete
		OUT_DEBUG_CMD(_T("SASI: CMD:WRITE Unit:%d Block:%d Finish"), m_unit_number, m_curr_block);
		send_status(STA_COMPLETE, ERR_NO_STATUS);
	}
}

void SASI_CTRL::seek()
{
	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	bool rc = d_current_disk->seek(m_curr_block);
	OUT_DEBUG_CMD(_T("SASI: CMD:SEEK Unit:%d Block:%d Result:%s"), m_unit_number, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// error
		send_status(STA_ERROR, ERR_NO_SEEK_COMPLETE);
		return;
	}

	int sum = d_current_disk->get_cylinder_diff(m_curr_block);
	play_seek_sound(sum);

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(sum);
	}
	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::format_disk()
{
	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	OUT_DEBUG_CMD(_T("SASI: CMD:FORMAT_DRIVE Unit:%d"), m_unit_number);
	d_current_disk->format_disk();
	send_status(STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::format_track()
{
	// clear request
	update_signal(
		0,
		SASI::SIGSTAT_REQ
	);

	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	int sum = d_current_disk->get_cylinder_diff(m_curr_block);
	play_seek_sound(sum);

	OUT_DEBUG_CMD(_T("SASI: CMD:FORMAT_TRACK Unit:%d Block:%d Start (Sum:%d)"), m_unit_number, m_curr_block, sum);
	d_current_disk->format_track(m_curr_block);

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(sum);
	}
	send_status_delay(delay, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::recv_extended_args()
{
	// go to transfer phase (write)
	update_signal(
		0,
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_MSG | SASI::SIGSTAT_IO | SASI::SIGSTAT_CD
	);
	m_phase = PHASE_TRANSFER;

	m_recv_data_pos = 0;
	m_recv_data_len = 10;

	OUT_DEBUG_CMD(_T("SASI: CMD:C2(Unknown) Unit:%d"), m_unit_number);

	if (!d_current_disk) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}
	if (!unit_mounted(m_unit_number)) {
		send_status(STA_ERROR, ERR_DRIVE_NOT_READY);
		return;
	}

	set_drq_delay(DELAY_TRANSFER_PHASE);
}

void SASI_CTRL::finish_extended_args()
{
	clr_drq();

	OUT_DEBUG_CMD(_T("SASI: CMD:C2(Unknown) Unit:%d Finished"), m_unit_number);
#ifdef _DEBUG
	for(int i=0; i<10; i++) {
		OUT_DEBUG_CMD(_T("SASI: CMD:C2(Unknown) Data:%d:%02X"), i, m_recv_data[i]);
	}
#endif
	// go to status phase
	send_status(STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::diagnostic()
{
	// unknown
	OUT_DEBUG_CMD(_T("SASI: CMD:E0(Unknown) Unit:%d"), m_unit_number);

	send_status_delay(DELAY_TRANSFER_PHASE, STA_COMPLETE, ERR_NO_STATUS);
}

void SASI_CTRL::send_status(uint8_t error_occured, uint8_t error_code)
{
	// go to status phase
	update_signal(
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_IO | SASI::SIGSTAT_CD,
		SASI::SIGSTAT_MSG
	);
	m_phase = PHASE_STATUS;
	// send irq
	d_host->set_irq(true);

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = ((m_unit_number << 5) | error_occured) & 0x7f;
	m_send_message = MSG_COMPLETE;
	m_send_error = ((m_command_class << 4) | error_code);

	OUT_DEBUG_STAT(_T("SASI: Unit:%d Status:%02X"), m_unit_number, m_send_data[0]);
}

void SASI_CTRL::send_message()
{
	// go to message phase
	update_signal(
		SASI::SIGSTAT_REQ | SASI::SIGSTAT_IO | SASI::SIGSTAT_CD | SASI::SIGSTAT_MSG,
		0
	);
	m_phase = PHASE_MESSAGE;
	// send irq
	d_host->set_irq(false);

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = m_send_message;
}

void SASI_CTRL::go_idle()
{
	// go to busfree phase
	update_signal(
		0,
		SASI::SIGSTAT_BSY | SASI::SIGSTAT_REQ | SASI::SIGSTAT_IO | SASI::SIGSTAT_CD | SASI::SIGSTAT_MSG
	);
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
		d_host->play_seek_sound();
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

void SASI_CTRL::send_status_delay(int delay, uint8_t error_occured, uint8_t error_code)
{
	cancel_my_event(EVENT_STATUS);
	m_event_arg[EVENT_ARG_STATUS_TYPE] = error_occured;
	m_event_arg[EVENT_ARG_STATUS_CODE] = error_code;
	register_my_event(EVENT_STATUS, delay);
}

void SASI_CTRL::update_signal(uint32_t on, uint32_t off)
{
	d_host->update_signal_status(on, off);
}

void SASI_CTRL::update_signal_delay(int delay, uint32_t on, uint32_t off)
{
	cancel_my_event(EVENT_SIGNAL);
	m_event_arg[EVENT_ARG_SIGNAL_ON] = on;
	m_event_arg[EVENT_ARG_SIGNAL_OFF] = off;
	register_my_event(EVENT_SIGNAL, delay);
}

void SASI_CTRL::clr_drq()
{
	d_host->clr_drq(m_ctrl_id);
}

void SASI_CTRL::set_drq_delay(int delay)
{
	cancel_my_event(EVENT_DRQ);
	register_my_event(EVENT_DRQ, delay);
}

// ----------------------------------------------------------------------------

void SASI_CTRL::cancel_my_event(int event_id)
{
	if (m_register_id[event_id] >= 0) {
		cancel_event(this, m_register_id[event_id]);
		m_register_id[event_id] = -1;
	}
}

void SASI_CTRL::register_my_event(int event_id, int wait)
{
	register_event(this, event_id, wait, false, &m_register_id[event_id]);
}

// ----------------------------------------------------------------------------

void SASI_CTRL::event_callback(int event_id, int err)
{
	m_register_id[event_id] = -1;
	switch(event_id) {
	case EVENT_SIGNAL:
		OUT_DEBUG_CMD(_T("SASI: Delayed Status on:%02X off:%02X"), m_event_arg[EVENT_ARG_SIGNAL_ON], m_event_arg[EVENT_ARG_SIGNAL_OFF]); 
		d_host->update_signal_status(m_event_arg[EVENT_ARG_SIGNAL_ON], m_event_arg[EVENT_ARG_SIGNAL_OFF]);
		break;
	case EVENT_DRQ:
		d_host->set_drq(m_ctrl_id);
		break;
	case EVENT_STATUS:
		send_status(m_event_arg[EVENT_ARG_STATUS_TYPE], m_event_arg[EVENT_ARG_STATUS_CODE]);
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

	vm_state.m_phase = Int32_LE(m_phase);
	vm_state.m_command_pos = Int32_LE(m_command_pos);
	vm_state.m_curr_block = Int32_LE(m_curr_block);
	vm_state.m_num_blocks = Int32_LE(m_num_blocks);

	vm_state.m_command_class = m_command_class;
	vm_state.m_command_code = m_command_code;
	vm_state.m_command_len = m_command_len;
	vm_state.m_unit_number = m_unit_number;
	vm_state.m_send_message = m_send_message;
	vm_state.m_send_error = m_send_error;

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

	m_phase = Int32_LE(vm_state.m_phase);
	m_command_pos = Int32_LE(vm_state.m_command_pos);
	m_curr_block = Int32_LE(vm_state.m_curr_block);
	m_num_blocks = Int32_LE(vm_state.m_num_blocks);

	m_command_class = vm_state.m_command_class;
	m_command_code = vm_state.m_command_code;
	m_command_len = vm_state.m_command_len;
	m_unit_number = vm_state.m_unit_number;
	m_send_message = vm_state.m_send_message;
	m_send_error = vm_state.m_send_error;

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

	d_current_disk = get_disk_unit(m_unit_number);

	return true;
}

// ----------------------------------------------------------------------------

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

bool SASI_CTRL::is_same_disk(int unit_num, const _TCHAR *path)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_same_file(path);
}

// ----------------------------------------------------------------------------

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
	for(int unit=0; unit<d_units->Count(); unit++) {
		rc |= d_units->Item(unit)->mounted();
	}
	return rc;
}

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

const _TCHAR *c_phase_label[] = {
	_T("BUSFREE"),
	_T("SELECTION"),
	_T("COMMAND"),
	_T("TRANSFER"),
	_T("STATUS"),
	_T("MESSAGE"),
	NULL
};

const _TCHAR *c_command_label[] = {
	_T("NONE"),
	_T("TEST_DRIVE_READY ($00)"),
	_T("RECALIBRATE ($01)"),
	_T("REQUEST_SENSE ($03)"),
	_T("FORMAT_DRIVE ($04)"),
	_T("FORMAT_TRACK ($06)"),
	_T("READ ($08)"),
	_T("WRITE ($0A)"),
	_T("SEEK ($0B)"),
	_T("EXTEND_ARGS ($C2)"),
	_T("DIAGNOSTIC ($E0)"),
	NULL
};

void SASI_CTRL::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscat(buffer, buffer_len, _T("\nCurrent Phase:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_phase < PHASE_UNKNOWN ? c_phase_label[m_phase] : _T("Unknown")
	);
	UTILITY::tcscat(buffer, buffer_len, _T("  Command:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_command_code < CMD_UNKNOWN ? c_command_label[m_command_code] : _T("Unknown")
	);
}
#endif

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
