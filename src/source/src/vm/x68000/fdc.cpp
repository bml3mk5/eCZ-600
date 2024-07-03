/** @file fdc.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@par Origin
	upd765a.cpp
	@author Sasaji
	@date   2022.02.22 -

	@brief [ UPD7265 modoki ]
*/

#include "fdc.h"
#include "floppy_defs.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"

//#define SET_CURRENT_TRACK_IMMEDIATELY 1

#ifdef _DEBUG
//#define OUT_DEBUG_CMD logging->out_debugf
#define OUT_DEBUG_CMD(...)
//#define OUT_DEBUG_READ logging->out_debugf
#define OUT_DEBUG_READ(...)
//#define OUT_DEBUG_WRITE logging->out_debugf
#define OUT_DEBUG_WRITE(...)
//#define OUT_DEBUG_STAT logging->out_debugf
#define OUT_DEBUG_STAT(...)
//#define OUT_DEBUG_STAT3 logging->out_debugf
#define OUT_DEBUG_STAT3(...)
//#define OUT_DEBUG_RESULT logging->out_debugf
#define OUT_DEBUG_RESULT(...)
//#define OUT_DEBUG_IRQ logging->out_debugf
#define OUT_DEBUG_IRQ(...)
//#define OUT_DEBUG_DRQ logging->out_debugf
#define OUT_DEBUG_DRQ(...)
//#define OUT_DEBUG_TIME logging->out_debugf
#define OUT_DEBUG_TIME(...)
#else
#define OUT_DEBUG_CMD(...)
#define OUT_DEBUG_READ(...)
#define OUT_DEBUG_WRITE(...)
#define OUT_DEBUG_STAT(...)
#define OUT_DEBUG_STAT3(...)
#define OUT_DEBUG_RESULT(...)
#define OUT_DEBUG_IRQ(...)
#define OUT_DEBUG_DRQ(...)
#define OUT_DEBUG_TIME(...)
#endif

#define DRIVE_MASK		(USE_FLOPPY_DISKS - 1)

#define GET_USEC()		(64 >> (m_clk_num + m_density))

// ----------------------------------------------------------------------------

void FDC::cancel_my_event(int event_id)
{
	if(m_register_id[event_id] != -1) {
		cancel_event(this, m_register_id[event_id]);
		m_register_id[event_id] = -1;
	}
}

void FDC::cancel_my_events()
{
	for(int i = 0; i < FDC_REGISTER_IDS; i++) {
		cancel_my_event(i);
	}
}

void FDC::register_my_event(int event_id, double usec)
{
	register_event(this, event_id, usec, false, &m_register_id[event_id]);
}

void FDC::register_my_event_by_clock(int event_id, int clock)
{
	register_event_by_clock(this, event_id, clock, false, &m_register_id[event_id]);
}

/// @param[in] next_phase : next phase after event occurred.
/// @param[in] usec : time to spend until occur event.
void FDC::register_phase_event_old(en_phases next_phase, double usec) {
	cancel_my_event(EVENT_PHASE);
	m_next_phase = next_phase;
	register_my_event(EVENT_PHASE, 100);
}

/// @param[in] next_phase : next phase after event occurred.
/// @param[in] usec : time to spend until occur event.
void FDC::register_phase_event_new(en_phases next_phase, double usec) {
	cancel_my_event(EVENT_PHASE);
	m_next_phase = next_phase;
	register_my_event(EVENT_PHASE, usec);
}

/// @param[in] next_phase : next phase after event occurred.
/// @param[in] clock : clock to spend until occur event.
void FDC::register_phase_event_by_clock(en_phases next_phase, int clock)
{
	cancel_my_event(EVENT_PHASE);
	m_next_phase = next_phase;
	register_my_event_by_clock(EVENT_PHASE, clock);
}

void FDC::register_seek_step_event(int drv)
{
	// get distance
	int steptime = (m_step_rate_time / (m_clk_num + 1)) * 1000; // msec -> usec
	if (FLG_DELAY_FDSEEK) steptime = 100;

	if(m_fdc[drv].cur_track != m_fdc[drv].tag_track) {
		register_my_event(EVENT_SEEK_STEP_0 + drv, steptime);
	}

	m_seekstat |= (1 << drv);
}

void FDC::register_seek_end_event(int drv)
{
	// get distance
	int seektime = 120;
#if 0
	if (m_fdc[drv].cur_track != m_fdc[drv].tag_track) {
		int steptime = (m_step_rate_time / (m_clk_num + 1)) * 1000; // msec -> usec
		int trk_sub = abs((int)m_fdc[drv].tag_track - m_fdc[drv].cur_track);
		if (trk_sub > 80) trk_sub = 80;
		seektime += steptime * trk_sub; // usec
	}
#endif
	register_my_event(EVENT_SEEK_END_0 + drv, seektime);
}

void FDC::register_search_event(int wait)
{
//	cancel_my_event(EVENT_SEARCH);
//	register_event_by_clock(this, (EVENT_SEARCH << 8) | cmdtype, wait, false, &register_id[EVENT_SEARCH]);
//	OUT_DEBUG2(_T("FDC\tRegist EVENT:%d id:%d w:%d"), EVENT_SEARCH, register_id[EVENT_SEARCH], wait);
//	now_search = true;
}

void FDC::register_drq_event()
{
	int usec = GET_USEC();	// 8us * 8bit (1MHz, FM) (delay 8)
	if(usec < 4) {
		usec = 4;
	}
	cancel_my_event(EVENT_LOST);
	cancel_my_event(EVENT_DRQ);
	register_my_event(EVENT_DRQ, (double)usec);
}

void FDC::register_lost_event(int bytes)
{
	int usec = GET_USEC();	// 8us * 8bit (1MHz, FM)
	usec *= (bytes + 1);
	usec += 2;

	cancel_my_event(EVENT_LOST);
	register_event(this, EVENT_LOST, (double)usec, false, &m_register_id[EVENT_LOST]);
}

void FDC::register_head_unload_event()
{
	cancel_my_event(EVENT_HEAD_UNLOAD);
	register_event(this, EVENT_HEAD_UNLOAD, (double)m_head_unload_time * 1000, false, &m_register_id[EVENT_HEAD_UNLOAD]);
}

// ----------------------------------------------------------------------------

void FDC::initialize()
{
	// config
	m_ignore_crc = pConfig->ignore_crc;

	// initialize fdc
	memset(m_fdc, 0, sizeof(m_fdc));
	memset(&m_id, 0, sizeof(m_id));

	m_phase = m_prevphase = PHASE_IDLE;
	m_main_status = S_RQM;
	m_seekstat = 0;
	m_seek_count = 0;
	m_data_count = 0;
	m_data_size = 0;
	p_cmd_args = m_command.b;
	p_result_codes = m_result_codes.b;

	for(int i = 0; i < FDC_REGISTER_IDS; i++) {
		m_register_id[i] = -1;
	}
	m_step_rate_time = m_head_unload_time = 0;
	m_no_dma_mode = false;
	m_step_rate_time = 1;
	m_head_load_time = 1;
	m_head_unload_time = 1;
	m_result = 0;
	m_write_id_phase = 0;
#ifdef UPD765A_DMA_MODE
	m_dma_data_lost = false;
#endif
}

void FDC::release()
{
}

/// power on reset
void FDC::reset()
{
	warm_reset(true);
}

/// master reset
void FDC::warm_reset(bool por)
{
	if (!por) {
		cancel_my_events();
	} else {
		// events were already canceled by EVENT::reset()
		for(int i = 0; i < FDC_REGISTER_IDS; i++) {
			m_register_id[i] = -1;
		}
	}

	memset(&m_command, 0, sizeof(m_command));
	memset(&m_result_codes, 0, sizeof(m_result_codes));
	memset(m_fdc, 0, sizeof(m_fdc));
	memset(&m_id, 0, sizeof(m_id));

	m_phase = m_prevphase = PHASE_IDLE;
	m_main_status = S_RQM;
	m_seekstat = 0;
	m_seek_count = 0;
	m_data_count = 0;
	m_data_size = 0;
	m_no_dma_mode = false;
	m_step_rate_time = 1;
	m_head_load_time = 1;
	m_head_unload_time = 1;
	m_result = 0;
	m_write_id_phase = 0;

	set_irq(false);
	set_drq(false);
}

// ----------------------------------------------------------------------------

void FDC::update_config()
{
	m_ignore_crc = pConfig->ignore_crc;
}

void FDC::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		// fdc data
		if((m_main_status & (S_RQM | S_DIO)) == S_RQM) {
			m_main_status &= ~S_RQM;

			switch(m_phase) {
			case PHASE_IDLE:
				m_command.cmd = data;
//				p_cmd_args = m_cmd_args.b;
				set_irq(false);
				accept_cmd();
				break;

			case PHASE_CMD:
				// get arguments on current command
				OUT_DEBUG_CMD(_T("clk:%d FDC: WRITE CMD ARG=%02X")
					, (int)get_current_clock(), data);

				*p_cmd_args++ = data;
				if(--m_command.count) {
					m_main_status |= S_RQM;
				} else {
					process_cmd();
				}
				break;

			case PHASE_WRITE:
				OUT_DEBUG_WRITE(_T("FDC: WRITE=%02"), data);

//				*p_bufptr++ = data;
				if ((m_command.cmd & 0x1f) == CMD_WRITE_ID) {
					d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK | m_channel, data, 0xff);
				} else {
					d_fdd->write_signal(SIG_FLOPPY_WRITE | m_channel, data, 0xff);
				}

//				set_drq(false);

				m_data_count++;
				if(m_data_count >= m_data_size) {
					execute_cmd();
				} else {
					register_drq_event();
				}
//				fdc[hdu & DRIVE_MASK].access = true;
				break;

			case PHASE_SCAN:
				if(data != 0xff) {
					uint32_t sdata = d_fdd->read_signal(SIG_FLOPPY_READ | m_channel) & 0xff;
					data &= 0xff;
					if(((m_command.cmd & 0x1f) == CMD_SCAN_EQUAL         && sdata != data) ||
					   ((m_command.cmd & 0x1f) == CMD_SCAN_LOW_OR_EQUAL  && sdata >  data) ||
					   ((m_command.cmd & 0x1f) == CMD_SCAN_HIGH_OR_EQUAL && sdata <  data)) {
						m_result &= ~ST2_SH;
					}
				}
//				p_bufptr++;
//				set_drq(false);
				m_data_count++;
				if(m_data_count >= m_data_size || (m_result & ST2_SH) == 0) {
					execute_cmd_scan();
				} else {
					register_drq_event();
				}

//				fdc[hdu & DRIVE_MASK].access = true;
				break;

			default:
				break;
			}
		}
	}
}

uint32_t FDC::read_io8(uint32_t addr)
{
	if(addr & 1) {
		// fdc data
		if((m_main_status & (S_RQM | S_DIO)) == (S_RQM | S_DIO)) {
			uint8_t data;
			m_main_status &= ~S_RQM;

			switch(m_phase) {
			case PHASE_RESULT:
				data = *p_result_codes++;

				OUT_DEBUG_RESULT(_T("clk:%d FDC: READ RESULT=%02X")
					, (int)get_current_clock(), data);

				if(--m_result_codes.count) {
					m_main_status |= S_RQM;
				} else {
					// EPSON QC-10 CP/M Plus
					bool clear_irq = true;
					if((m_command.cmd & 0x1f) == CMD_SENSE_INTER_STATUS) {
						for(int i = 0; i < 4; i++) {
							if(m_fdc[i].result) {
								clear_irq = false;
								break;
							}
						}
					}
					if(clear_irq) {
						set_irq(false);
					}
					shift_to_idle();
				}
				return data;

			case PHASE_READ:
				data = d_fdd->read_signal(SIG_FLOPPY_READ | m_channel);
//				data = *p_bufptr++;
				m_data_count++;

				OUT_DEBUG_READ(_T("FDC: READ=%02X"), data);

//				set_drq(false);
				if(m_data_count >= m_data_size) {
					// last data
					execute_cmd();
				} else {
					// next data
					register_drq_event();
				}

//				m_fdc[m_command.hdu & DRIVE_MASK].access = true;
				return data;

			default:
				break;
			}
		}
		return 0xff;
	} else {
		// FIXME: dirty patch for PC-8801 Kimochi Disk 2
		if(m_register_id[EVENT_PHASE] != -1 && m_next_phase == PHASE_EXEC) {
			cancel_my_event(EVENT_PHASE);
			m_phase = m_next_phase;
			execute_cmd();
		}
		// fdc status
		OUT_DEBUG_STAT(_T("FDC: STAT: %02X"), m_seekstat | m_main_status);

//		set_irq(false);

		return m_seekstat | m_main_status;
	}

	return 0xff;
}

void FDC::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr >> 1, data);
}

uint32_t FDC::read_dma_io8(uint32_t addr)
{
	return read_io8(addr >> 1);
}

void FDC::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		if (!now_reset) {
			warm_reset(false);
		}
		break;

	case SIG_TC:
		if(m_phase == PHASE_EXEC || m_phase == PHASE_READ || m_phase == PHASE_WRITE || m_phase == PHASE_SCAN || (m_phase == PHASE_RESULT && m_command.count == 7)) {
			if(data & mask) {
				if((m_phase == PHASE_READ  && ((m_command.cmd & 0x1f) == 0x06 || (m_command.cmd & 0x1f) == 0x0c) && m_command.count > 0) ||
				   (m_phase == PHASE_WRITE && ((m_command.cmd & 0x1f) == 0x05 || (m_command.cmd & 0x1f) == 0x09) && m_command.count > 0)) {
					if(m_main_status & S_RQM) {
						if(m_no_dma_mode) {
							write_signals(&outputs_irq, 0);
						} else {
							write_signals(&outputs_drq, 0);
						}
						m_main_status &= ~S_RQM;
					}
					cancel_my_events();
					int usec = GET_USEC();	// 8us * 8bit (1MHz, FM)
					register_phase_event_new(PHASE_TC, (double)(usec * m_command.count));
				} else {
					m_prevphase = m_phase;
					m_phase = PHASE_TC;
					execute_cmd();
				}
			}
		}
		break;

	case SIG_DACK:
		if (!(data & mask)) {
			set_drq(false);
		}
		break;

//	case SIG_MOTOR:
//		m_motor_on = ((data & mask) != 0);
//		break;

//	case SIG_MOTOR_NEG:
//		m_motor_on = ((data & mask) == 0);
//		break;

#ifdef UPD765A_EXT_DRVSEL
	case SIG_DRVSEL:
		m_command.hdu = (m_command.hdu & 4) | (data & DRIVE_MASK);
		write_signals(&outputs_hdu, m_command.hdu);
		break;
#endif

//	case SIG_IRQ_MASK:
//		if(!(m_irq_masked = ((data & mask) != 0))) {
//			write_signals(&outputs_irq, 0);
//		}
//		break;

//	case SIG_DRQ_MASK:
//		if(!(m_drq_masked = ((data & mask) != 0))) {
//			write_signals(&outputs_drq, 0);
//		}
//		break;

	}
}

uint32_t FDC::read_signal(int ch)
{
	// get access status
	uint32_t stat = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
	d_fdd->write_signal(SIG_FLOPPY_ACCESS | channel, 0, 1);
#endif

	return stat;
}

void FDC::event_callback(int event_id, int err)
{
//	int drv;
//	int val;
//	bool now_index;

	switch(event_id) {
	case EVENT_PHASE:
		m_register_id[event_id] = -1;
		m_prevphase = m_phase;
		m_phase = m_next_phase;
		execute_cmd();
		break;

	case EVENT_DRQ:
		m_register_id[event_id] = -1;
		m_main_status |= S_RQM;

//		drv = m_command.hdu & DRIVE_MASK;
//		d_fdd->write_signal(ID, aa,bb);
//		m_fdc[drv].cur_position = (m_fdc[drv].cur_position + 1); // % disk[drv]->get_track_size();
//		m_fdc[drv].prev_clock = m_prev_drq_clock = get_current_clock();
		set_drq(true);
		break;

	case EVENT_LOST:
		OUT_DEBUG_RESULT(_T("FDC: DATA LOST  COUNT:%d"), m_data_count);
		m_register_id[event_id] = -1;
		m_result = ST0_AT | ST1_OR;
		set_drq(false);
		set_to_result7();
		break;

	case EVENT_RESULT7:
		m_register_id[event_id] = -1;
		set_to_result7_event();
		break;

#if 0
	case EVENT_INDEX:
		m_register_id[event_id] = -1;
		// index hole signal width is 5msec (thanks Mr.Sato)
		drv = m_command.hdu & DRIVE_MASK;
		now_index = (d_fdd->read_signal(SIG_FLOPPY_READY) != 0); /*&& get_cur_position(drv) < disk[drv]->get_bytes_per_usec(5000)*/
		if(m_prev_index != now_index) {
//			write_signals(&outputs_index, now_index ? 0xffffffff : 0);
			m_prev_index = now_index;
		}
		break;
#endif

	case EVENT_SEEK_STEP_0:
	case EVENT_SEEK_STEP_1:
	case EVENT_SEEK_STEP_2:
	case EVENT_SEEK_STEP_3:
		// seek step pulse
		m_register_id[event_id] = -1;
		seek_event(event_id - EVENT_SEEK_STEP_0);
		break;

	case EVENT_SEEK_END_0:
	case EVENT_SEEK_END_1:
	case EVENT_SEEK_END_2:
	case EVENT_SEEK_END_3:
		// seek end without step pulse (target track and current are same)
		m_register_id[event_id] = -1;
		seek_end(event_id - EVENT_SEEK_END_0);
		break;

	case EVENT_HEAD_UNLOAD:
		m_register_id[event_id] = -1;
		d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 0, 1);
		break;

	default:
//		m_register_id[event_id] = -1;
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

#ifdef _DEBUG
const _TCHAR *cmdstr[0x20] = {
	0, 0, _T("READ A TRACK"), _T("SPECIFY"),
	_T("SENCE DRIVE STATUS"), _T("WRITE DATA"), _T("READ DATA"), _T("RECALIBRATE"),
	_T("SENCE INTR STATUS"), _T("WRITE DELETED DATA"), _T("READ ID"), 0,
	_T("READ DELETED DATA"), _T("FORMAT A TRACK"), 0, _T("SEEK"),
	0, _T("SCAN EQUAL"), 0, 0,
	0, 0, 0, 0,
	0, _T("SCAN LOW OR EQUAL"), 0, 0,
	0, _T("SCAN HIGH OR EQUAL"), 0, 0
};
#endif

void FDC::accept_cmd()
{
	int cmd = m_command.cmd;

	OUT_DEBUG_CMD(_T("clk:%d FDC PHASE_IDLE CMD=%02X(%s)")
		, (int)get_current_clock()
		, cmd, cmdstr[cmd & 0x1f] ? cmdstr[cmd & 0x1f] : _T("INVALID"));

	switch(cmd & 0x1f) {
	case 0x02:
		accept_cmd_read_diagnostic();
		break;
	case 0x03:
		accept_cmd_specify();
		break;
	case 0x04:
		accept_cmd_sense_devstat();
		break;
	case 0x05:
	case 0x09:
		accept_cmd_write_data();
		break;
	case 0x06:
	case 0x0c:
		accept_cmd_read_data();
		break;
	case 0x07:
		accept_cmd_recalib();
		break;
	case 0x08:
		accept_cmd_sense_intstat();
		break;
	case 0x0a:
		accept_cmd_read_id();
		break;
	case 0x0d:
		accept_cmd_write_id();
		break;
	case 0x0f:
		accept_cmd_seek();
		break;
	case 0x11:
	case 0x19:
	case 0x1d:
		accept_cmd_scan();
		break;
	default:
		accept_cmd_invalid();
		break;
	}
}

// ----------------------------------------------------------------------------

void FDC::process_cmd()
{
	int cmd = m_command.cmd;

	OUT_DEBUG_CMD(_T("clk:%d FDC PHASE_CMD CMD=%02X(%s)")
		, (int)get_current_clock()
		, cmd, cmdstr[cmd & 0x1f] ? cmdstr[cmd & 0x1f] : _T("INVALID"));

	switch(cmd & 0x1f) {
	case 0x02:
		cmd_read_diagnostic();
		break;
	case 0x03:
		cmd_specify();
		break;
	case 0x04:
		cmd_sense_devstat();
		break;
	case 0x05:
	case 0x09:
		cmd_write_data();
		break;
	case 0x06:
	case 0x0c:
		cmd_read_data();
		break;
	case 0x07:
		cmd_recalib();
		break;
//	case 0x08:
//		cmd_sense_intstat();
//		break;
	case 0x0a:
		cmd_read_id();
		break;
	case 0x0d:
		cmd_write_id();
		break;
	case 0x0f:
		cmd_seek();
		break;
	case 0x11:
	case 0x19:
	case 0x1d:
		cmd_scan();
		break;
	default:
//		cmd_invalid();
		break;
	}
}

// ----------------------------------------------------------------------------

void FDC::execute_cmd()
{
	int cmd = m_command.cmd;

	OUT_DEBUG_CMD(_T("clk:%d FDC PHASE_EXEC CMD=%02X(%s)")
		, (int)get_current_clock()
		, cmd, cmdstr[cmd & 0x1f] ? cmdstr[cmd & 0x1f] : _T("INVALID"));

	switch(cmd & 0x1f) {
	case 0x02:
		execute_cmd_read_diagnostic();
		break;
//	case 0x03:
//		cmd_specify();
//		break;
//	case 0x04:
//		cmd_sense_devstat();
//		break;
	case 0x05:
	case 0x09:
		execute_cmd_write_data();
		break;
	case 0x06:
	case 0x0c:
		execute_cmd_read_data();
		break;
//	case 0x07:
//		cmd_recalib();
//		break;
//	case 0x08:
//		cmd_sense_intstat();
//		break;
	case 0x0a:
		execute_cmd_read_id();
		break;
	case 0x0d:
		execute_cmd_write_id();
		break;
//	case 0x0f:
//		cmd_seek();
//		break;
	case 0x11:
	case 0x19:
	case 0x1d:
		execute_cmd_scan();
		break;
	default:
//		cmd_invalid();
		break;
	}
}

/// sense drive status
void FDC::accept_cmd_sense_devstat()
{
	shift_to_cmd(1);
}

/// sense drive status
void FDC::cmd_sense_devstat()
{
	set_hdu(m_command.hdu);
	m_result_codes.devstat.st3 = get_devstat(m_command.hdu & 3);
	shift_to_result(1);
}

/// sense interrupt status
void FDC::accept_cmd_sense_intstat()
{
	for(int i = 0; i < 4; i++) {
		if(m_fdc[i].result) {
			m_result_codes.intstat.st0 = (uint8_t)m_fdc[i].result;
			m_result_codes.intstat.pcn = (uint8_t)m_fdc[i].cur_track;
			m_fdc[i].result = 0;
			shift_to_result(2);
			return;
		}
	}
#ifdef UPD765A_SENCE_INTSTAT_RESULT
	// IBM PC/JX
	m_result_codes.intstat.st0 = (uint8_t)ST0_AI;
#else
	m_result_codes.intstat.st0 = (uint8_t)ST0_IC;
#endif
	shift_to_result(1);
//	m_main_status &= ~S_CB;
}

/// drive status
uint8_t FDC::get_devstat(int drv)
{
	uint8_t status;
	uint8_t ready = (d_fdd->read_signal(SIG_FLOPPY_READY) != 0 ? ST3_RY : 0);

	if(drv >= USE_FLOPPY_DISKS) {
		status = drv
			| (m_command.hdu & ST3_HD)	// side select
			| ST3_TS	// two sides
			| ready	// ready
			;
//			| ST3_FT;	// bit7 is fault signal
	} else {
		status = drv	// bit0-1 drive unit number
			| (m_command.hdu & ST3_HD)	// side select
			| ST3_TS	// two sides
			| (d_fdd->read_signal(SIG_FLOPPY_TRACK0) ? ST3_T0 : 0)	// track 0	
			| ready	// ready
			| (d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT) ? ST3_WP : 0);	// write protected
		// bit7 is fault signal (not implemented)
	}

	OUT_DEBUG_STAT3(_T("FDC %d DEV STAT: %02X"), drv, status);

	return status;
}

/// seek track
void FDC::accept_cmd_seek()
{
	shift_to_cmd(2);
}

/// seek track
void FDC::cmd_seek()
{
	m_seek_count = RECALIB_COUNTS;
	seek(m_command.hdu & 3, m_command.seek.ncn);
//	shift_to_idle();
}

/// recalibrate (track to 0)
void FDC::accept_cmd_recalib()
{
	shift_to_cmd(1);
}

/// recalibrate (track to 0)
void FDC::cmd_recalib()
{
	m_seek_count = RECALIB_COUNTS;
	m_fdc[m_command.hdu & 3].cur_track = RECALIB_COUNTS;
	seek(m_command.hdu & 3, 0);
//	shift_to_idle();
}

/// seek track
void FDC::seek(int drv, int trk)
{
	// set target track number
	m_fdc[drv].tag_track = trk;

//	cancel_my_event(m_register_id[EVENT_SEEK_STEP_0 + drv]);
//	cancel_my_event(m_register_id[EVENT_SEEK_END_0 + drv]);

	if(m_fdc[drv].cur_track == m_fdc[drv].tag_track
		|| d_fdd->read_signal(SIG_FLOPPY_READY) == 0
		|| (m_command.cmd == 0x07 && d_fdd->read_signal(SIG_FLOPPY_TRACK0))
	) {
		// seek end
		register_seek_end_event(drv);
	} else {
		// next step
		register_seek_step_event(drv);
	}
}

void FDC::seek_event(int drv)
{
	uint32_t val = 0x80;

	if(m_fdc[drv].cur_track == m_fdc[drv].tag_track) {
		// same track
		val = 0x80;
	} else if(m_fdc[drv].cur_track < m_fdc[drv].tag_track) {
		m_fdc[drv].cur_track++;
		val = 1;	// plus
	} else if(m_fdc[drv].cur_track > m_fdc[drv].tag_track) {
		m_fdc[drv].cur_track--;
		val = (uint32_t)(-1);	// minus
	}

	// send step pulse
	d_fdd->write_signal(SIG_FLOPPY_STEP | m_channel, val, 0xff);

	m_seek_count--;

	if(m_fdc[drv].cur_track == m_fdc[drv].tag_track
		|| d_fdd->read_signal(SIG_FLOPPY_READY) == 0
		|| (m_command.cmd == 0x07 && d_fdd->read_signal(SIG_FLOPPY_TRACK0))
		|| m_seek_count <= 0) {
		// seek end
		register_seek_end_event(drv);
	} else {
		// next step
		register_seek_step_event(drv);
	}
}

/// seek end
void FDC::seek_end(int drv)
{
//	int trk = m_fdc[drv].cur_track;
	uint8_t status = 0;
	if (d_fdd->read_signal(SIG_FLOPPY_READY) == 0) status |= (ST0_AT | ST0_NR);
	if (m_seek_count <= 0) status |= (ST0_AT | ST0_EC);
	if (d_fdd->read_signal(SIG_FLOPPY_TRACK0)) m_fdc[drv].cur_track = 0;

	m_fdc[drv].result = drv | ST0_SE | status;

	m_result = m_fdc[drv].result;

	set_irq(true);
	m_seekstat &= ~(1 << drv);

	cancel_my_event(EVENT_SEEK_STEP_0 + drv);
	cancel_my_event(EVENT_SEEK_END_0 + drv);

	shift_to_idle();
//	// reset dsch flag
//	m_disk[drv]->changed = false;
}

/// read data
void FDC::accept_cmd_read_data()
{
	shift_to_cmd(8);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// read data
void FDC::cmd_read_data()
{
	get_sector_params();
	start_transfer_data();
}

/// read data
void FDC::execute_cmd_read_data()
{
	switch(m_phase) {
	case PHASE_EXEC:
		read_data((m_command.cmd & 0x1f) == CMD_DELETED_READ_DATA, false);
		break;
	case PHASE_READ:
		// after read last data
		if(m_result || !id_increment()) {
			// go result phase
			register_phase_event_new(PHASE_TIMER, 8);
			register_head_unload_event();
		} else {
			// next sector
			start_transfer_data();
		}
		break;
	case PHASE_TC:
		// terminal count
		cancel_my_events();
		set_to_result7();
		break;
	case PHASE_TIMER:
//		m_result |= ST0_AT | ST1_EN;
//		m_result |= ST1_EN;
		set_to_result7();
		break;
	default:
		break;
	}
}

/// write data
void FDC::accept_cmd_write_data()
{
	shift_to_cmd(8);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// write data
void FDC::cmd_write_data()
{
	get_sector_params();
	start_transfer_data();
}

/// write data
void FDC::execute_cmd_write_data()
{
	switch(m_phase) {
	case PHASE_EXEC:
		write_data((m_command.cmd & 0x1f) == CMD_DELETED_WRITE_DATA);
		break;
	case PHASE_WRITE:
		// after wrote last data
		if(m_result || !id_increment()) {
			// go result phase
			register_phase_event_new(PHASE_TIMER, 8);
			register_head_unload_event();
		} else {
			// next sector
			start_transfer_data();
		}
		break;
	case PHASE_TC:
		// terminal count
		cancel_my_events();
		set_to_result7();
		break;
	case PHASE_TIMER:
//		m_result |= ST0_AT | ST1_EN;
//		m_result |= ST1_EN;
		set_to_result7();
		break;
	default:
		break;
	}
}

/// sector scan
void FDC::accept_cmd_scan()
{
	shift_to_cmd(8);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// sector scan
void FDC::cmd_scan()
{
	get_sector_params();
	m_command.rw.dtl |= 0x100;
	start_transfer_data();
}

/// sector scan
void FDC::execute_cmd_scan()
{
	switch(m_phase) {
	case PHASE_EXEC:
		read_data(false, true);
		break;
	case PHASE_SCAN:
		// after read last data
		if(m_result || !id_increment()) {
			// go result phase
			register_phase_event_new(PHASE_TIMER, 8);
			register_head_unload_event();
		} else {
			// next sector
			start_transfer_data();
		}
		break;
	case PHASE_TC:
		cancel_my_events();
		set_to_result7();
		break;
	case PHASE_TIMER:
//		m_result |= ST0_AT | ST1_EN;
//		m_result |= ST1_EN;
		set_to_result7();
		break;
	default:
		break;
	}
}

/// read a track
void FDC::accept_cmd_read_diagnostic()
{
	shift_to_cmd(8);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// read a track
void FDC::cmd_read_diagnostic()
{
	get_sector_params();
	start_transfer_diagnostic();
}

/// read a track
void FDC::execute_cmd_read_diagnostic()
{
	switch(m_phase) {
	case PHASE_EXEC:
		read_diagnostic();
		break;
	case PHASE_READ:
		// go result phase
		register_phase_event_new(PHASE_TIMER, 8);
		register_head_unload_event();
		break;
	case PHASE_TC:
		// terminal count
		cancel_my_events();
		set_to_result7();
		break;
	case PHASE_TIMER:
//		m_result |= ST0_AT | ST1_EN;
//		m_result |= ST1_EN;
		set_to_result7();
		break;
	default:
		break;
	}
}

void FDC::read_data(bool deleted, bool scan)
{
//	int drv = m_command.hdu & 3;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;
	bool is_error = false;

	set_chrn();

	// m_result set on start_transfer_data() at first
	do {
		m_result |= check_cond();
		if (m_result & (ST1_ND | ST1_DE | ST1_MA)) {
			// no data found / CRC error / no address mark found
			is_error = true;
			break;
		}

		if (deleted) {
			// invert CM flag
			m_result ^= ST2_CM;
		}

		// deleted mark detected
		if (m_result & ST2_CM) {
			is_error = true;
			break;
		}
	} while(0);

	if(is_error) {
		// error
		m_result |= ST0_AT;
		register_phase_event_new(PHASE_TIMER, 8);
		register_head_unload_event();
		return;
	}

	// sector size in the current disk
	m_data_size = (int)d_fdd->read_signal(SIG_FLOPPY_SECTOR_SIZE | m_channel);
	// 
	int length = (m_command.rw.n != 0) ? (0x80 << MIN(m_command.rw.n, 7)) : (MIN(m_command.rw.dtl, 0x80));

	m_data_size = MIN(m_data_size, length);

	if(!scan) {
		shift_to_read();
	} else {
		shift_to_scan();
	}
	return;
}

void FDC::write_data(bool deleted)
{
//	int drv = m_command.hdu & 3;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;
	bool is_error = false;

	set_chrn();

	// m_result set on start_transfer_data() at first
	do {
		if (d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
			// write protected
			is_error = true;
			m_result |= ST1_NW;
		}

		m_result |= check_cond();
		if (m_result & (ST1_ND | ST1_DE | ST1_MA)) {
			// no data found / CRC error / no address mark found
			is_error = true;
			break;
		}

		if (deleted) {
			// invert CM flag
			m_result ^= ST2_CM;
		}

		// deleted mark detected
		if (m_result & ST2_CM) {
			is_error = true;
			break;
		}
	} while(0);

	if(is_error) {
		// error
		m_result |= ST0_AT;
		register_phase_event_new(PHASE_TIMER, 8);
		register_head_unload_event();
		return;
	}

	// deleted data mark
	d_fdd->write_signal(SIG_FLOPPY_WRITEDELETE, deleted ? 1 : 0, 1);

	// sector size in the current disk
	m_data_size = (int)d_fdd->read_signal(SIG_FLOPPY_SECTOR_SIZE | m_channel);
	// 
	int length = (m_command.rw.n != 0) ? (0x80 << MIN(m_command.rw.n, 7)) : (MIN(m_command.rw.dtl, 0x80));

	m_data_size = MIN(m_data_size, length);

	shift_to_write();
}

/// read a track
void FDC::read_diagnostic()
{
//	int drv = m_command.hdu & 3;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;
	bool is_error = false;

	do {
		m_result |= check_cond();
		if (m_result & ST0_NR) {
			// not ready
			is_error = true;
			break;
		}
	} while(0);

	if(is_error) {
		// error
		m_result |= ST0_AT;
		register_phase_event_new(PHASE_TIMER, 8);
		register_head_unload_event();
		return;
	}

	// sector size in the current disk
	m_data_size = (int)d_fdd->read_signal(SIG_FLOPPY_SECTOR_SIZE | m_channel);
	// 
	int length = (m_command.rw.n != 0) ? (0x80 << MIN(m_command.rw.n, 7)) : (MIN(m_command.rw.dtl, 0x80));

	m_data_size = MIN(m_data_size, length);

	shift_to_read();

	return;
}

#if 0
uint32_t FDC::read_sector()
{
//	if((m_command.cmd & CMD_FLAG_MFM) != (m_density ? CMD_FLAG_MFM : 0)) {
//		return ST0_AT | ST1_MA;
//	}

	return 0;
	int drv = m_command.hdu & DRIVE_MASK;
	int trk = m_fdc[drv].cur_track;
	int side = (m_command.hdu >> 2) & 1;

	// get sector counts in the current track
//	if(!disk[drv]->make_track(trk, side)) {
	if(!make_track()) {
		OUT_DEBUG(_T("FDC: TRACK NOT FOUND (TRK=%d SIDE=%d)"), trk, side);
		return ST0_AT | ST1_MA;
	}
	if((m_command.cmd & 0x40) != (m_density ? 0x40 : 0)) {
		return ST0_AT | ST1_MA;
	}
//	int secnum = disk[drv]->sector_num.sd;
//	if(!secnum) {
//		OUT_DEBUG(_T("FDC: NO SECTORS IN TRACK (TRK=%d SIDE=%d)"), trk, side);
//		return ST0_AT | ST1_MA;
//	}
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
#if 0
		if(disk[drv]->id[0] != id[0] || disk[drv]->id[1] != id[1] || disk[drv]->id[2] != id[2] /*|| disk[drv]->id[3] != id[3]*/) {
#else
		if(disk[drv]->id[0] != m_command.rw.c || disk[drv]->id[1] != m_command.rw.h || disk[drv]->id[2] != m_command.rw.r || disk[drv]->id[3] != m_command.rw.n) {
#endif
			continue;
		}
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC: SECTOR FOUND (TRK=%d SIDE=%d ID=%2x,%2x,%2x,%2x)\n"), trk, side, id[0], id[1], id[2], id[3]);
#endif
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
#if 0
		if(disk[drv]->invalid_format) {
			memset(m_buffer, m_density ? 0x4e : 0xff, sizeof(m_buffer));
			memcpy(m_buffer, disk[drv]->sector, disk[drv]->sector_size.sd);
		} else {
			memcpy(m_buffer, disk[drv]->track + disk[drv]->data_position[i], disk[drv]->get_track_size() - disk[drv]->data_position[i]);
			memcpy(m_buffer + disk[drv]->get_track_size() - disk[drv]->data_position[i], disk[drv]->track, disk[drv]->data_position[i]);
		}
		m_fdc[drv].next_trans_position = disk[drv]->data_position[i];
#endif		
		if((disk[drv]->addr_crc_error || disk[drv]->data_crc_error) && !disk[drv]->ignore_crc()) {
			return ST0_AT | ST1_DE | (disk[drv]->data_crc_error ? ST2_DD : 0);
		}
		if(disk[drv]->deleted) {
			return ST2_CM;
		}
		return 0;
	}
#ifdef _FDC_DEBUG_LOG
	this->out_debug_log(_T("FDC: SECTOR NOT FOUND (TRK=%d SIDE=%d ID=%2x,%2x,%2x,%2x)\n"), trk, side, id[0], id[1], id[2], id[3]);
#endif
	if(cy != m_command.rw.c && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
//	return ST0_AT | ST1_ND;
}
#endif

#if 0
uint32_t FDC::write_sector(bool deleted)
{
	int drv = m_command.hdu & DRIVE_MASK;
	int trk = m_fdc[drv].cur_track;
	int side = (m_command.hdu >> 2) & 1;

	if(d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
		return ST0_AT | ST1_NW;
	}
	return 0;
	// get sector counts in the current track
	if(!disk[drv]->get_track(trk, side)) {
		return ST0_AT | ST1_MA;
	}
	int secnum = disk[drv]->sector_num.sd;
	if(!secnum) {
		return ST0_AT | ST1_MA;
	}
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
		if(disk[drv]->id[0] != m_command.rw.c || disk[drv]->id[1] != m_command.rw.h || disk[drv]->id[2] != m_command.rw.r /*|| disk[drv]->id[3] != m_command.rw.n*/) {
			continue;
		}
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
		int size = 0x80 << MIN(m_command.rw.n, 7);
		memcpy(disk[drv]->sector, m_buffer, min(size, disk[drv]->sector_size.sd));
		disk[drv]->set_deleted(deleted);
		return 0;
	}
	if(cy != m_command.rw.c && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
	return ST0_AT | ST1_ND;
}
#endif

uint32_t FDC::find_id()
{
//	int drv = m_command.hdu & DRIVE_MASK;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;

	// get sector counts in the current track
	if(!d_fdd->search_track(m_channel)) {
		return ST0_AT | ST1_MA;
	}
//	if((m_command.cmd & CMD_FLAG_MFM) != (m_density ? CMD_FLAG_MFM : 0)) {
//		return ST0_AT | ST1_MA;
//	}
//	int secnum = disk[drv]->sector_num.sd;
//	if(!secnum) {
//		return ST0_AT | ST1_MA;
//	}
#if 0
	int cy = -1;
	for(int i = 0; i < secnum; i++) {
		if(!disk[drv]->get_sector(trk, side, i)) {
			continue;
		}
		cy = disk[drv]->id[0];
		if(disk[drv]->id[0] != m_command.rw.c || disk[drv]->id[1] != m_command.rw.h || disk[drv]->id[2] != m_command.rw.r /*|| disk[drv]->id[3] != m_command.rw.n*/) {
			continue;
		}
		if(disk[drv]->sector_size.sd == 0) {
			continue;
		}
		// sector number is matched
		m_fdc[drv].next_trans_position = disk[drv]->data_position[i];
		return 0;
	}
	if(cy != m_command.rw.c && cy != -1) {
		if(cy == 0xff) {
			return ST0_AT | ST1_ND | ST2_BC;
		} else {
			return ST0_AT | ST1_ND | ST2_NC;
		}
	}
#endif
	return ST0_AT | ST1_ND;
}

/// @return Not Ready: ST0_AT | ST0_NR 
uint32_t FDC::check_cond()
{
	int drv = m_command.hdu & 3;
//	m_command.hdue = m_command.hdu;

	if(drv >= USE_FLOPPY_DISKS) {
		// abnormal termination / not ready
		return ST0_AT | ST0_NR;
	}
	if(!d_fdd->read_signal(SIG_FLOPPY_READY)) {
		// abnormal termination / not ready / not detect address mark
		return ST0_AT | ST0_NR | ST1_MA;
	}
	return 0;
}

void FDC::get_sector_params()
{
	set_hdu(m_command.hdu);
//	m_command.hdue = m_command.rw.hdu;
//	m_command.rw.c = m_command.rw.c;
//	m_command.rw.h = m_command.rw.h;
//	m_command.rw.r = m_command.rw.r;
//	m_command.rw.n = m_command.rw.n;
//	m_command.rw.eot  = m_command.rw.eot;
//	m_command.rw.gpl  = m_command.rw.gpl;
//	m_command.rw.dtl  = m_command.rw.dtl;
}

// multisector access
bool FDC::id_increment()
{
	bool rc = false;
	if((m_command.cmd & 19) == 17) {
		// scan equal
		if((m_command.rw.dtl & 0xff) == 0x02) {
			m_command.rw.r++;
		}
	}
	if (m_command.rw.r > m_command.rw.eot) {
		m_result |= (ST0_AT | ST1_EN);
	}
	do {
		if(m_command.rw.r++ != m_command.rw.eot) {
			// next sector
			rc = true;
			break;
		}
		m_command.rw.r = 1;
		if(m_command.cmd & 0x80) {
			set_hdu(m_command.hdu ^ 4);
			m_command.rw.h ^= 1;
			if(m_command.rw.h & 1) {
				rc = true;
				break;
			}
		}
		m_command.rw.c++;

	} while(0);

	set_chrn();
	return rc;
}

void FDC::set_chrn()
{
	m_id.c = m_command.rw.c;
	m_id.h = m_command.rw.h;
	m_id.r = m_command.rw.r;
	m_id.n = m_command.rw.n;
}

/// read id
void FDC::accept_cmd_read_id()
{
	shift_to_cmd(1);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// read id
void FDC::cmd_read_id()
{
	set_hdu(m_command.hdu);
	start_transfer_id();
}

/// read id
void FDC::execute_cmd_read_id()
{
	switch(m_phase) {
	case PHASE_EXEC:
		read_id();
		break;
	case PHASE_TIMER:
		set_to_result7();
		break;
	default:
		break;
	}
}

/// format a track
void FDC::accept_cmd_write_id()
{
	shift_to_cmd(5);
	m_density = (m_command.cmd & CMD_FLAG_MFM ? 1 : 0);
	d_fdd->write_signal(SIG_FLOPPY_DENSITY, m_density, 0xff);
}

/// format a track
void FDC::cmd_write_id()
{
	set_hdu(m_command.hdu);
	m_id.n = m_command.format.n;
//	m_command.rw.eot = m_command.format.sc;
//	m_command.rw.gpl = m_command.format.gpl;
//	m_command.rw.dtl = m_command.format.data; // temporary
//	if (!m_command.format.sc) {
//		// sector per track is zero
//		register_phase_event_new(PHASE_TIMER, 8);
//		return;
//	}
	start_transfer_index();
//	m_data_size = 4 * m_command.rw.eot;
//		m_fdc[m_command.hdu & DRIVE_MASK].next_trans_position = get_cur_position(m_command.hdu & DRIVE_MASK);
//	shift_to_write();
}

/// format a track
void FDC::execute_cmd_write_id()
{
	switch(m_phase) {
	case PHASE_EXEC:
		write_id();
		break;
	case PHASE_WRITE:
		// next sector
		shift_to_write_id();
		break;
	case PHASE_TC:
		cancel_my_events();
		set_to_result7();
		break;
	case PHASE_TIMER:
//		m_result |= ST1_EN;
		set_to_result7();
		break;
	default:
		break;
	}
}

/// read address mark
void FDC::read_id()
{
//	int drv = m_command.hdu & 3;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;

	m_id.c = 0;
	m_id.h = 0;
	m_id.r = 0;
	m_id.n = 0;

	// m_result set on start_transfer_id() at first
	do {
		if (m_result & ST1_ND) {
			// no data found
			break;
		}

		m_result |= check_cond();
		if (m_result & ST1_MA) {
			// no address mark found
			break;
		}

		// read id
		m_id.c = d_fdd->read_signal(SIG_FLOPPY_READ_ID | m_channel);
		m_id.h = d_fdd->read_signal(SIG_FLOPPY_READ_ID | m_channel);
		m_id.r = d_fdd->read_signal(SIG_FLOPPY_READ_ID | m_channel);
		m_id.n = d_fdd->read_signal(SIG_FLOPPY_READ_ID | m_channel);
	} while(0);

	register_phase_event_new(PHASE_TIMER, 8);
	register_head_unload_event();
}

/// format a track
void FDC::write_id()
{
//	int drv = m_command.hdu & 3;
//	int trk = m_fdc[drv].cur_track;
//	int side = (m_command.hdu >> 2) & 1;
	bool is_error = false;

	m_id.n = m_command.format.n;

	// m_result set on start_transfer_data() at first
	do {
		if (d_fdd->read_signal(SIG_FLOPPY_WRITEPROTECT)) {
			// write protected
			is_error = true;
			m_result |= ST1_NW;
		}

		m_result |= check_cond();
		if (m_result & ST0_NR) {
			// not ready
			is_error = true;
			break;
		}
	} while(0);

	if(is_error) {
		// error
		m_result |= ST0_AT;
		register_phase_event_new(PHASE_TIMER, 8);
		register_head_unload_event();
		return;
	}

//	m_data_size = 0x80 << MIN(m_id.n, 7);

//	parse_track();
	m_write_id_phase = 0;
	m_sector_count = -1;
	d_fdd->write_signal(SIG_FLOPPY_TRACK_SIZE | m_channel, 1, 1);

	shift_to_write_id();

#if 0
	for(int i = 0; i < m_command.rw.eot && i < 256; i++) {
		for(int j = 0; j < 4; j++) {
			m_id[j] = m_buffer.b[4 * i + j];
		}
		disk[drv]->insert_sector(m_command.rw.c, m_command.rw.h, m_command.rw.r, m_command.rw.n, false, false, m_command.rw.dtl, length);
	}
#endif
	return;
}

/// specify step rate, head load rate and dma mode
void FDC::accept_cmd_specify()
{
	shift_to_cmd(2);
}

/// specify step rate, head load rate and dma mode
void FDC::cmd_specify()
{
	m_step_rate_time = 16 - (m_command.specify.srt_hut >> 4);
	m_head_unload_time = ((m_command.specify.srt_hut & 0xf) << 4);
	m_head_load_time = (m_command.specify.hlt & ~1);
	m_no_dma_mode = ((m_command.specify.hlt & 1) != 0);

	OUT_DEBUG_WRITE(_T("FDC SPECIFY STEP_RATE:%d NO_DMA:%d"), m_step_rate_time, m_no_dma_mode ? 1 : 0);

	shift_to_idle();
	m_main_status = S_RQM; //0xff;
}

/// unknown
void FDC::accept_cmd_invalid()
{
	m_result_codes.rw.st0 = (uint8_t)ST0_IC;
	shift_to_result(1);
}

void FDC::shift_to_idle()
{
	m_phase = PHASE_IDLE;
	m_main_status = S_RQM;
}

void FDC::shift_to_cmd(int length)
{
	m_phase = PHASE_CMD;
	m_main_status = S_RQM | S_CB;
	m_command.count = length;
	p_cmd_args = m_command.b;
}

void FDC::shift_to_exec()
{
	m_phase = PHASE_EXEC;
	execute_cmd();
}

void FDC::shift_to_read()
{
	m_phase = PHASE_READ;
	m_main_status = S_RQM | S_DIO | S_NDM | S_CB;

//	int drv = m_command.hdu & DRIVE_MASK;
//	m_fdc[drv].cur_position = m_fdc[drv].next_trans_position;
//	m_fdc[drv].prev_clock = m_prev_drq_clock = get_current_clock();
	set_drq(true);
}

void FDC::shift_to_write()
{
	m_phase = PHASE_WRITE;
	m_main_status = S_RQM | S_NDM | S_CB;
//	m_command.count = length;

//	int drv = m_command.hdu & DRIVE_MASK;
//	m_fdc[drv].cur_position = m_fdc[drv].next_trans_position;
//	m_fdc[drv].prev_clock = m_prev_drq_clock = get_current_clock();
	set_drq(true);
}

void FDC::shift_to_scan()
{
	m_phase = PHASE_SCAN;
	m_main_status = S_RQM | S_NDM | S_CB;
	m_result = ST2_SH;
//	m_command.count = length;

//	int drv = m_command.hdu & DRIVE_MASK;
//	m_fdc[drv].cur_position = m_fdc[drv].next_trans_position;
//	m_fdc[drv].prev_clock = m_prev_drq_clock = get_current_clock();
	set_drq(true);
}

struct st_ibm_format {
	int8_t  type;
	uint8_t data;
	int16_t count;
};

static const struct st_ibm_format c_fm_format1[] = {
	// FM
	{ 0, 0xff, 40 },	// GAP 4a
	{ 0, 0x00,  6 },	// SYNC
	{ 0, 0xfc,  1 },	// INDEX MARK
	{ 5, 0xff, 26 },	// GAP 1
	{ 0, 0x00,  6 },	// SYNC
	{ 0, 0xfe,  1 },	// ID MARK
	{ -1, 0, 0 }	// end
};

static const struct st_ibm_format c_mfm_format1[] = {
	// MFM
	{ 0, 0x4e, 80 },	// GAP 4a
	{ 0, 0x00, 12 },	// SYNC
	{ 0, 0xc2,  3 },	// INDEX MARK pre
	{ 0, 0xfc,  1 },	// INDEX MARK
	{ 5, 0x4e, 50 },	// GAP 1
	{ 0, 0x00, 12 },	// SYNC
	{ 0, 0xa1,  3 },	// ID MARK pre
	{ 0, 0xfe,  1 },	// ID MARK
	{ -1, 0, 0 }	// end
};

static const struct st_ibm_format *c_ibm_format1[2] = {
	c_fm_format1, c_mfm_format1
};

static const struct st_ibm_format c_fm_format2[] = {
	// FM
	{ 1, 0,     2 },	// CRC
	{ 0, 0xff, 11 },	// GAP 2
	{ 0, 0x00,  6 },	// SYNC
	{ 2, 0xfb,  1 },	// DATA MARK
	{ 3, 0x40,  0 },	// fill data
	{ 1, 0,     2 },	// CRC
	{ 4, 0xff,  0 },	// GAP 3
	{ 0, 0x00,  6 },	// SYNC
	{ 0, 0xfe,  1 },	// ID MARK
	{ -1, 0, 0 }	// end
};

static const struct st_ibm_format c_mfm_format2[] = {
	// MFM
	{ 1, 0,     2 },	// CRC
	{ 0, 0x4e, 22 },	// GAP 2
	{ 0, 0x00, 12 },	// SYNC
	{ 0, 0xa1,  3 },	// DATA MARK pre
	{ 2, 0xfb,  1 },	// DATA MARK
	{ 3, 0xe5,  0 },	// fill data
	{ 1, 0,     2 },	// CRC
	{ 4, 0x4e,  0 },	// GAP 3
	{ 0, 0x00, 12 },	// SYNC
	{ 0, 0xa1,  3 },	// ID MARK pre
	{ 0, 0xfe,  1 },	// ID MARK
	{ -1, 0, 0 }	// end
};

static const struct st_ibm_format *c_ibm_format2[2] = {
	c_fm_format2, c_mfm_format2
};

void FDC::shift_to_write_id()
{
	const struct st_ibm_format *format = c_ibm_format2[m_density];

	uint32_t data;
	int count;
	int bytes = 0;
	bool end_of_track = false;
	switch(m_write_id_phase) {
	case 0:
		// start
		format = c_ibm_format1[m_density];

		// through //
	case 2:
		// write data mark and fills data
		m_sector_count++;

		for(int pos = 0; format[pos].type >= 0 && !end_of_track; pos++) {
			count = format[pos].count;
			data = format[pos].data;
			switch(format[pos].type) {
			case 1:
				// CRC TODO:
				data = 0;
				break;
			case 2:
				// data mark or deleted data mark
				data = format[pos].data;
				break;
			case 3:
				// fill data
				data = m_command.format.data;
				count = (m_command.format.n != 0) ? (0x80 << MIN(m_command.format.n, 7)) : 0x80;
				break;
			case 4:
				// GAP 3
				count = (m_command.format.gpl != 0) ? m_command.format.gpl : 54;
				end_of_track = (m_sector_count >= m_command.format.sc);
				break;
			case 5:
				// GAP 1
				end_of_track = (m_sector_count >= m_command.format.sc);
				break;
			default:
				break;
			}
			for(int i=0; i<count; i++) {
				d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK | m_channel, data, 0xff);
				bytes++;
			}
		}
		if (!end_of_track) {
			// calc delay time and drq (CHRN)
			// 2HD:1us 2DD:2us
			// * MFM:1us FM:2us
			int delay = bytes * 8 * (1 << (m_density & 1)); // us

			register_phase_event_new(PHASE_WRITE, delay);

			m_data_count = 0;
			m_data_size = 4;
			m_write_id_phase = 1;

		} else {
			// fill GAP 4b
			count = d_fdd->read_signal(SIG_FLOPPY_TRACK_REMAIN_SIZE | m_channel);
			data = c_ibm_format1[m_density][0].data;
			for(int i=0; i<count; i++) {
				d_fdd->write_signal(SIG_FLOPPY_WRITE_TRACK | m_channel, data, 0xff);
				bytes++;
			}

			// calc delay time
			// 2HD:1us 2DD:2us
			// * MFM:1us FM:2us
			int delay = bytes * 8 * (1 << m_density); // us

			// parse raw data in the track
			// to convert d88 format track
			parse_track();

			register_phase_event_new(PHASE_TIMER, delay);

		}
		break;

	case 1:
	default:
		// request c h r n
		m_phase = PHASE_WRITE;
		m_main_status = S_RQM | S_NDM | S_CB;
		set_drq(true);
		m_write_id_phase++;
		break;
	}
}

void FDC::shift_to_result(int length)
{
	m_phase = PHASE_RESULT;
	m_main_status = S_RQM | S_DIO | S_CB;
	m_result_codes.count = length;
	p_result_codes = m_result_codes.b;
}

void FDC::set_to_result7()
{
#ifdef UPD765A_WAIT_RESULT7
	if(result7_id != -1) {
		cancel_event(this, result7_id);
		result7_id = -1;
	}
	if(phase != PHASE_TIMER) {
		register_event(this, EVENT_RESULT7, 100, false, &result7_id);
	} else
#endif
	set_to_result7_event();
	finish_transfer();
}

void FDC::set_to_result7_event()
{
#ifdef UPD765A_NO_ST1_EN_OR_FOR_RESULT7
	// for NEC PC-9801 (XANADU)
	m_result &= ~(ST1_EN | ST1_OR);
#endif
	m_result_codes.rw.st0 = (m_result & 0xf8) | (m_command.hdu & 7);
	m_result_codes.rw.st1 = ((m_result >>  8) & 0xff);
	m_result_codes.rw.st2 = ((m_result >> 16) & 0xff);
	m_result_codes.rw.c = m_id.c;
	m_result_codes.rw.h = m_id.h;
	m_result_codes.rw.r = m_id.r;
	m_result_codes.rw.n = m_id.n;

	OUT_DEBUG_RESULT(_T("clk:%d CMD END RESULT ST0:%02X ST1:%02X ST2:%02X")
		, (int)get_current_clock()
		, m_result_codes.rw.st0, m_result_codes.rw.st1, m_result_codes.rw.st2);

	set_irq(true);
	shift_to_result(7);
}

void FDC::set_to_result7_event(uint8_t c, uint8_t h, uint8_t r, uint8_t n)
{
	m_result_codes.rw.st0 = (m_result & 0xf8) | (m_command.hdu & 7);
	m_result_codes.rw.st1 = ((m_result >>  8) & 0xff);
	m_result_codes.rw.st2 = ((m_result >> 16) & 0xff);
	m_result_codes.rw.c = c;
	m_result_codes.rw.h = h;
	m_result_codes.rw.r = r;
	m_result_codes.rw.n = n;

	OUT_DEBUG_RESULT(_T("clk:%d CMD END RESULT ST0:%02X ST1:%02X ST2:%02X")
		, (int)get_current_clock()
		, m_result_codes.rw.st0, m_result_codes.rw.st1, m_result_codes.rw.st2);

	set_irq(true);
	shift_to_result(7);
	finish_transfer();
}

void FDC::start_transfer_old()
{
//	int drv = m_command.hdu & 3;

//	cancel_my_event(EVENT_UNLOAD_0 + drv);

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);
}

void FDC::start_transfer_data()
{
//	int drv = m_command.hdu & 3;
	int delay_clock = 100;

	// specified sector exists or not
	m_result = search_sector((m_command.hdu & 4) >> 2, m_command.rw.r, false);
	if (!FLG_DELAY_FDSEARCH) {
		if (m_result & ST1_ND) {
			// no sector found
			// twice index hole
			delay_clock = d_fdd->calc_index_hole_search_clock(m_channel);
			delay_clock += get_a_round_clock(m_channel);
		} else {
			// calcrate time to reach to the target sector
			delay_clock = d_fdd->calc_sector_search_clock(m_channel, m_command.rw.r);
		}
		if (!d_fdd->read_signal(SIG_FLOPPY_HEADLOAD | m_channel)) {
			// delay time for loading head
			delay_clock += (m_head_load_time * CPU_CLOCKS / 1000);
		}
	}

	OUT_DEBUG_TIME(_T("FDC: Cmd:%02X Delay clock:%d"), m_command.cmd, delay_clock);

	register_phase_event_by_clock(PHASE_EXEC, delay_clock);

//	cancel_my_event(EVENT_UNLOAD_0 + drv);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);
}

void FDC::start_transfer_diagnostic()
{
//	int drv = m_command.hdu & 3;
	int delay_clock = 100;

	// specified sector exists or not
	m_result = search_sector_by_index(0, &m_command.rw.h, &m_command.rw.r, &m_command.rw.n);
	if (!FLG_DELAY_FDSEARCH) {
		// calcrate time to reach to the index hole
		delay_clock = d_fdd->calc_index_hole_search_clock(m_channel);

		if (!d_fdd->read_signal(SIG_FLOPPY_HEADLOAD | m_channel)) {
			// delay time for loading head
			delay_clock += (m_head_load_time * CPU_CLOCKS / 1000);
		}
	}

	OUT_DEBUG_TIME(_T("FDC: Cmd:%02X Delay clock:%d"), m_command.cmd, delay_clock);

	register_phase_event_by_clock(PHASE_EXEC, delay_clock);

//	cancel_my_event(EVENT_UNLOAD_0 + drv);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);
}

/// search address mark
void FDC::start_transfer_id()
{
//	int drv = m_command.hdu & 3;
	int delay_clock;

	// readable address mark?
	m_result = search_addr();
	if (m_result & ST1_ND) {
		// no sector found
		// twice index hole
		delay_clock = d_fdd->calc_index_hole_search_clock(m_channel);
		delay_clock += get_a_round_clock(m_channel);
	} else {
		// calcrate time to reach to the target sector
		delay_clock = d_fdd->calc_next_sector_clock(m_channel);
	}
	if (!d_fdd->read_signal(SIG_FLOPPY_HEADLOAD | m_channel)) {
		// delay time for loading head
		delay_clock += (m_head_load_time * CPU_CLOCKS / 1000);
	}

	register_phase_event_by_clock(PHASE_EXEC, delay_clock);

//	cancel_my_event(EVENT_UNLOAD_0 + drv);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);
}

/// wait index mark
void FDC::start_transfer_index()
{
	int delay_clock;

	delay_clock = d_fdd->calc_index_hole_search_clock(m_channel);
	if (!d_fdd->read_signal(SIG_FLOPPY_HEADLOAD | m_channel)) {
		// delay time for loading head
		delay_clock += (m_head_load_time * CPU_CLOCKS / 1000);
	}

	register_phase_event_by_clock(PHASE_EXEC, delay_clock);

//	cancel_my_event(EVENT_UNLOAD_0 + drv);
	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);
}

void FDC::finish_transfer()
{
//	int drv = m_command.hdu & 3;

//	if (d_fdd->read_signal(SIG_FLOPPY_HEADLOAD)) {
//		cancel_my_event(EVENT_UNLOAD_0 + drv);
//		int time = (16 * (m_head_unload_time + 1) / (m_clk_num + 1)) * 1000; // msec -> usec
//		register_my_event(EVENT_UNLOAD_0 + drv, time);
//	}
}
// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int FDC::get_cur_position(int drv)
{
//	return (m_fdc[drv].cur_position + m_disk[drv]->get_bytes_per_usec(get_passed_usec(fdc[drv].prev_clock))) % disk[drv]->get_track_size();

	return 1;
}

double FDC::get_usec_to_exec_phase()
{
	uint32_t rc = search_sector(m_command.rw.h, m_command.rw.r, false);
	if (rc != 0) {
		// no data
		m_result = rc;
		// 1 round at least
		return 200000;
	}
	return d_fdd->calc_sector_search_clock(m_channel, m_command.rw.r);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

/// @return ST1_ND : No Data
uint32_t FDC::verify_track()
{
	if(!d_fdd->search_track(m_channel)) {
		return ST1_ND;
	}

	// verify track number
//	if(!(cmdreg & 4)) {
//		return 0;
//	}

	d_fdd->write_signal(SIG_FLOPPY_HEADLOAD | m_channel, 1, 1);

//	if(!d_fdd->verify_track(channel, trkreg)) {
//		return FDC_ST_SEEKERR;
//	}
	return 0;
}

/// @param[in] side    : side number
/// @param[in] sector  : sector number
/// @param[in] compare_side : for compare side number
/// @return ST1_ND : No Data
uint32_t FDC::search_sector(int side, int sector, bool compare_side)
{
	int drv = m_command.hdu & 3;

	// get track
	if(!d_fdd->search_track(m_channel)) {
		// no track
		return (ST2_WC | ST1_ND);
	}

	// scan sectors
	int stat = d_fdd->search_sector(m_channel, m_fdc[drv].cur_track, sector, compare_side, side);
	if (stat != 1) {
		// sector exists
		m_data_count = 0;
		return ((stat & 4) ? ST2_CM : 0) | ((stat & 2) ? (ST2_DD | ST1_DE) : 0);
	}

	// sector not found
	return ST1_ND;
}

/// @param[in] index        : index number of sector in the track
/// @param[in] compare_side : set side number (ID H) if need compareing
/// @param[in] compare_sect : set sector number (ID R) if need compareing
/// @param[in] compare_size : set sector size (ID N) if need compareing
/// @return ST1_ND : No Data
uint32_t FDC::search_sector_by_index(int index, uint8_t *compare_side, uint8_t *compare_sect, uint8_t *compare_size)
{
	int drv = m_command.hdu & 3;

	// get track
	if(!d_fdd->search_track(m_channel)) {
		// no track
		return (ST2_WC | ST1_ND);
	}

	// scan sectors
	int stat = d_fdd->search_sector_by_index(m_channel, m_fdc[drv].cur_track, index, compare_side, compare_sect, compare_size, m_id.b);
	m_data_count = 0;

	uint32_t rc = 0;
	if (stat & 4) rc |= ST2_CM;
	if (stat & 2) rc |= ST2_DD | ST1_DE;
	if (stat & 1) rc |= ST1_ND;
	return rc;
}

/// search id in address mark
uint32_t FDC::search_addr()
{
	// get track
	if(!d_fdd->search_track(m_channel)) {
		// no track
		return (ST2_WC | ST1_ND);
	}

	// get sector
	int stat = d_fdd->search_sector(m_channel);
	if (stat != 1) {
		// sector exists
		m_data_count = 0;
		return ((stat & 2) ? (ST2_DD | ST1_DE) : 0);
	}

	// sector not found
	return ST1_ND;
}

bool FDC::make_track()
{
	return d_fdd->make_track(m_channel);
}

bool FDC::parse_track()
{
	return d_fdd->parse_track(m_channel);
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------

void FDC::set_irq(bool val)
{
	OUT_DEBUG_IRQ(_T("FDC: IRQ=%d"), val ? 1 : 0);
//	write_signals(&outputs_irq, (val && !m_irq_masked) ? 0xffffffff : 0);
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void FDC::set_drq(bool val)
{
	OUT_DEBUG_DRQ(_T("FDC: DRQ=%d"), val ? 1 : 0);

//	// cancel next drq and data lost events
//	cancel_my_event(EVENT_DRQ);
	cancel_my_event(EVENT_LOST);

	// register data lost event if data exists
	if(val) {
#ifdef UPD765A_DMA_MODE
		// EPSON QC-10 CP/M Plus
		m_dma_data_lost = true;
#else
		if((m_command.cmd & 0x1f) != CMD_WRITE_ID) {
			register_lost_event(1);
		} else {
			// FIXME: write id (format a track)
			register_my_event(EVENT_LOST, 30000);
		}
#endif
	}
	if(m_no_dma_mode) {
//		write_signals(&outputs_irq, (val && !m_irq_masked) ? 0xffffffff : 0);
		write_signals(&outputs_irq, val ? 0xffffffff : 0);
	} else {
//		write_signals(&outputs_drq, (val && !m_drq_masked) ? 0xffffffff : 0);
		write_signals(&outputs_drq, val ? 0xffffffff : 0);
#ifdef UPD765A_DMA_MODE
		// EPSON QC-10 CP/M Plus
		if(val && dma_data_lost) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC: DATA LOST (DMA)\n"));
#endif
			result = ST1_OR;
			write_signals(&outputs_drq, 0);
			set_to_result7();
		}
#endif
	}
}

void FDC::set_hdu(uint8_t val)
{
//#ifdef UPD765A_EXT_DRVSEL
//	m_command.hdu = (m_command.hdu & 3) | (val & 4);
//#else
//	m_command.hdu = val;
//#endif
	write_signals(&outputs_hdu, val);
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void FDC::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<4; i++) {
		SET_Byte(m_fdc[i].tag_track);
		SET_Byte(m_fdc[i].cur_track);
		SET_Byte(m_fdc[i].result);
	}

	// 1
	SET_Byte(m_id.c);
	SET_Byte(m_id.h);
	SET_Byte(m_id.r);
	SET_Byte(m_id.n);

	SET_Byte(m_command.cmd);
	SET_Byte(m_command.count);
	SET_Byte(m_command.hdu);
	SET_Byte(m_command.rw.c);
	SET_Byte(m_command.rw.h);
	SET_Byte(m_command.rw.r);
	SET_Byte(m_command.rw.n);
	SET_Byte(m_command.rw.eot);
	SET_Byte(m_command.rw.gpl);
	SET_Byte(m_command.rw.dtl);

	// 2
	SET_Byte(m_phase);
	SET_Byte(m_prevphase);
	SET_Byte(m_main_status);
	SET_Byte(m_seekstat);

	SET_Int32_LE(m_result_codes.count);
	SET_Byte(m_result_codes.rw.st0);
	SET_Byte(m_result_codes.rw.st1);
	SET_Byte(m_result_codes.rw.st2);
	SET_Byte(m_result_codes.rw.c);
	SET_Byte(m_result_codes.rw.h);
	SET_Byte(m_result_codes.rw.r);
	SET_Byte(m_result_codes.rw.n);

	// 3
	SET_Int32_LE(m_seek_count);	///< number of seek pulse
	SET_Int32_LE(m_data_count);
	SET_Int32_LE(m_data_size);	///< sector size
	SET_Uint32_LE(m_result);

	// 4
	SET_Int32_LE(m_step_rate_time);		///< step rate time (ms)
	SET_Int32_LE(m_head_load_time);		///< head load time (ms)
	SET_Int32_LE(m_head_unload_time);	///< head unload time (ms)

	// config
	SET_Bool(m_ignore_crc);
	SET_Bool(m_no_dma_mode);
#ifdef UPD765A_DMA_MODE
	SET_Byte(m_dma_data_lost);
#endif
	SET_Byte(m_next_phase);

	// 5
	SET_Int32_LE(m_write_id_phase);
	SET_Int32_LE(m_sector_count);

	// clock
	SET_Int32_LE(m_clk_num);	// 0:4MHz 1:8MHz
	SET_Int32_LE(m_density);	// 0:single density(FM) 1:double density(MFM)

	// 6
	SET_Int32_LE(m_channel);

	// event
	for(int i=0; i<15 && i<FDC_REGISTER_IDS; i++) {
		SET_Int32_LE(m_register_id[i]);
	}

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_ENPHASES(v) v = (en_phases)vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool FDC::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	for(int i=0; i<4; i++) {
		GET_Byte(m_fdc[i].tag_track);
		GET_Byte(m_fdc[i].cur_track);
		GET_Byte(m_fdc[i].result);
	}

	// 1
	GET_Byte(m_id.c);
	GET_Byte(m_id.h);
	GET_Byte(m_id.r);
	GET_Byte(m_id.n);

	GET_Byte(m_command.cmd);
	GET_Byte(m_command.count);
	GET_Byte(m_command.hdu);
	GET_Byte(m_command.rw.c);
	GET_Byte(m_command.rw.h);
	GET_Byte(m_command.rw.r);
	GET_Byte(m_command.rw.n);
	GET_Byte(m_command.rw.eot);
	GET_Byte(m_command.rw.gpl);
	GET_Byte(m_command.rw.dtl);

	// 2
	GET_ENPHASES(m_phase);
	GET_ENPHASES(m_prevphase);
	GET_Byte(m_main_status);
	GET_Byte(m_seekstat);

	GET_Int32_LE(m_result_codes.count);
	GET_Byte(m_result_codes.rw.st0);
	GET_Byte(m_result_codes.rw.st1);
	GET_Byte(m_result_codes.rw.st2);
	GET_Byte(m_result_codes.rw.c);
	GET_Byte(m_result_codes.rw.h);
	GET_Byte(m_result_codes.rw.r);
	GET_Byte(m_result_codes.rw.n);

	// 3
	GET_Int32_LE(m_seek_count);	///< number of seek pulse
	GET_Int32_LE(m_data_count);
	GET_Int32_LE(m_data_size);	///< sector size
	GET_Uint32_LE(m_result);

	// 4
	GET_Int32_LE(m_step_rate_time);		///< step rate time (ms)
	GET_Int32_LE(m_head_load_time);		///< head load time (ms)
	GET_Int32_LE(m_head_unload_time);	///< head unload time (ms)

	// config
	GET_Bool(m_ignore_crc);
	GET_Bool(m_no_dma_mode);
#ifdef UPD765A_DMA_MODE
	GET_Byte(m_dma_data_lost);
#endif
	GET_ENPHASES(m_next_phase);

	// 5
	GET_Int32_LE(m_write_id_phase);
	GET_Int32_LE(m_sector_count);

	// clock
	GET_Int32_LE(m_clk_num);	// 0:4MHz 1:8MHz
	GET_Int32_LE(m_density);	// 0:single density(FM) 1:double density(MFM)

	// 6
	GET_Int32_LE(m_channel);

	// event
	for(int i=0; i<15 && i<FDC_REGISTER_IDS; i++) {
		GET_Int32_LE(m_register_id[i]);
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t FDC::debug_read_io8(uint32_t addr)
{
	return 0xff;
}

bool FDC::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool FDC::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void FDC::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len,
		_T(" CMD:%02X ARGS:%02X %02X %02X %02X %02X %02X %02X %02X\n"),
		m_command.cmd, m_command.b[0], m_command.b[1], m_command.b[2], m_command.b[3], m_command.b[4], m_command.b[5], m_command.b[6], m_command.b[7]);
	UTILITY::sntprintf(buffer, buffer_len,
		_T(" ST:%02X ST0:%02X ST1:%02X ST2:%02X RESULT:%02X %02X %02X %02X %02X %02X %02X %02X\n"),
		m_main_status,
		m_result & 0xff, (m_result >> 8) & 0xff, (m_result >> 16) & 0xff,
		m_result_codes.b[0], m_result_codes.b[1], m_result_codes.b[2], m_result_codes.b[3],
		m_result_codes.b[4], m_result_codes.b[5], m_result_codes.b[6], m_result_codes.b[7]);
	UTILITY::strcat(buffer, buffer_len, _T(" ST3:"));
	for(int i=0; i<4; i++) {
		int v = get_devstat(i);
		UTILITY::sntprintf(buffer, buffer_len,
			_T(" %d:%02X"), i, v);
	}
}


#endif
