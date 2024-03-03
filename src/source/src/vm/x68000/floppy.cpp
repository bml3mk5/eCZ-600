/** @file floppy.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.11.05 -

	@brief [ floppy drive ]
*/

#include <math.h>
#include "floppy.h"
#include "fdc.h"
#include "../disk.h"
#include "../../emu.h"
#include "../vm.h"
#include "../../logging.h"
#include "../../utility.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../parsewav.h"

#if defined(_DEBUG_FLOPPY) // || defined(_DEBUG_FDC)
#define OUT_DEBUG logging->out_debugf
#define OUT_DEBUG2(...)
#define _DEBUG_FLOPPY_ALL
static uint8_t debug_regr[8];
static int     debug_regr_cnt[8];
#define OUT_DEBUG_REGR(emu, addr, data, msg) out_debug_regr(emu, addr, data, msg)
#else
#define OUT_DEBUG(...)
#define OUT_DEBUG2(...)
#define OUT_DEBUG_REGR(emu, addr, data, msg)
#endif

#ifdef _DEBUG
//#define DEBUG_READY
//#define OUT_DEBUG_READY logging->out_debugf
#define OUT_DEBUG_READY(...)
//#define OUT_DEBUG_MOTOR logging->out_debugf
#define OUT_DEBUG_MOTOR(...)
//#define OUT_DEBUG_DISK logging->out_debugf
#define OUT_DEBUG_DISK(...)
//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)
#else
#define OUT_DEBUG_READY(...)
#define OUT_DEBUG_MOTOR(...)
#define OUT_DEBUG_DISK(...)
#define OUT_DEBUG_REGW(...)
#endif

#define DRIVE_MASK	(MAX_DRIVE - 1)

#define USE_FLOPPY_READY_ON_ALL_DRIVES

// ----------------------------------------------------------------------------

void FLOPPY::cancel_my_event(int event_no)
{
	if(m_register_id[event_no] != -1) {
		cancel_event(this, m_register_id[event_no]);
		m_register_id[event_no] = -1;
	}
}

void FLOPPY::cancel_my_events()
{
	for(int i = 0; i < EVENT_IDS_MAX; i++) {
		cancel_my_event(i);
	}
}

void FLOPPY::register_my_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event(this, event_no, wait, false, &m_register_id[event_no]);
}

void FLOPPY::register_index_hole_event(int event_no, int wait)
{
	cancel_my_event(event_no);
	register_event_by_clock(this, event_no, wait, false, &m_register_id[event_no], &m_index_hole_next_clock);
}

// ----------------------------------------------------------------------------

void FLOPPY::initialize()
{
	// setup/reset floppy drive
	for(int i = 0; i < MAX_DRIVE; i++) {
		p_disk[i] = new DISK(i);

		m_fdd[i].side = 0;
		m_fdd[i].track = 0;
		m_fdd[i].index = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
		m_fdd[i].access = false;
#endif
		m_fdd[i].ready = 0;
		m_fdd[i].motor_warmup = 0;
#ifdef USE_FLOPPY_HEAD_LOADING
		m_fdd[i].head_loading = 0;
#endif
		m_fdd[i].delay_write = 0;

		m_fdd[i].motor_on = false;
		m_fdd[i].inserted = false;
		m_fdd[i].shown_media_error = false;

		m_fdd[i].drv_ctrl = 0;
		m_fdd[i].opened_intr = 0;
		m_fdd[i].closed_intr = 0;
		m_fdd[i].force_eject = 0;
	}
	m_ignore_crc = false;

	m_sectorcnt = 0;
	m_sectorcnt_cont = false;

	m_index_hole = 0;
	m_index_hole_next_clock = 0;
#ifdef USE_FLOPPY_HEAD_LOADING
	m_head_load = 0;
#endif

	for(int i = 0; i < EVENT_IDS_MAX; i++) {
		m_register_id[i] = -1;
	}
	m_now_irq = 0;

	m_drv_num = 0;
	m_drv_ctrl = 0;
	m_accs_drv = 0;

	m_led_blink = 0;

//	m_opm_addr = 0;
	m_delay_ready_on = DELAY_READY_ON_H;
	m_force_ready = false;

//	m_wav_enable = 0;
	m_wav_loaded_at_first = false;

	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
		m_noises[ft][FLOPPY_WAV_SEEK].set_file_name(_T("fddseek.wav"));
		m_noises[ft][FLOPPY_WAV_MOTOR].set_file_name(_T("fddmotor.wav"));
		m_noises[ft][FLOPPY_WAV_MOTOR].set_loop(true);
		m_noises[ft][FLOPPY_WAV_EJECT].set_file_name(_T("fddeject.wav"));
	}

	register_frame_event(this);
}

/// power on reset
void FLOPPY::reset()
{
	load_wav();

	for(int i = 0; i < MAX_DRIVE; i++) {
		m_fdd[i].track = 0;
		m_fdd[i].index = 0;
#ifdef USE_SIG_FLOPPY_ACCESS
		m_fdd[i].access = false;
#endif
		// set drive type
		set_drive_type(i, DRIVE_TYPE_2HD);
	}

	warm_reset(true);
}

/// reset signal
void FLOPPY::warm_reset(bool por)
{
#ifdef _DEBUG_FLOPPY_ALL
	for(int i=0; i<5; i++) {
		debug_regr[i] = 0x55;
		debug_regr_cnt[i] = 0;
	}
#endif

	if (!por) {
		cancel_my_events();
	} else {
		// events were already canceled by EVENT::reset()
		for(int i = 0; i < EVENT_IDS_MAX; i++) {
			m_register_id[i] = -1;
		}
	}

	for(int i = 0; i < MAX_DRIVE; i++) {
		m_fdd[i].ready = 0;
		m_fdd[i].motor_warmup = 0;
#ifdef USE_FLOPPY_HEAD_LOADING
		m_fdd[i].head_loading = 0;
#endif
		m_fdd[i].motor_on = false;
		m_fdd[i].inserted = p_disk[i]->inserted;
		m_fdd[i].shown_media_error = false;
		m_fdd[i].drv_ctrl = 0;
		m_fdd[i].opened_intr = 0;
		m_fdd[i].closed_intr = 0;
		m_fdd[i].force_eject = 0;
	}

	m_drv_num = 0;
//	m_drv_sel = 0xff;

	m_motor_on_expand = 0;

	m_drv_ctrl = 0;
	m_accs_drv = 0;

	m_index_hole = 0;

	set_drive_speed();

	m_density = 0;

//	m_opm_addr = 0;
	m_force_ready = false;

	//	irqflg = false;
//	irqflgprev = irqflg;
//	drqflg = false;
//	drqflgprev = drqflg;

	m_ignore_write = false;

//	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
//		for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
//			m_noises[ft][ty].stop();
//		}
//	}
	m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_SEEK].stop();
	m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].stop();

	register_my_event(EVENT_INDEXHOLE_ON, m_delay_index_hole);	// index hole

#ifdef USE_FLOPPY_TIMEOUT
	// motor off time
	register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
#endif

	m_now_irq = 0;
	set_irq(0, 0);
}

void FLOPPY::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		delete p_disk[i];
	}
//	for(int ft = 0; ft < FLOPPY_WAV_FDDTYPES; ft++) {
//		for(int ty = 0; ty < FLOPPY_WAV_SNDTYPES; ty++) {
//			delete[] p_wav_data[ft][ty];
//		}
//	}
}

// ----------------------------------------------------------------------------

void FLOPPY::load_wav()
{
	// allocation
	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		m_noises[ft][FLOPPY_WAV_SEEK].alloc(m_sample_rate / 5);		// 0.2sec max
		m_noises[ft][FLOPPY_WAV_MOTOR].alloc(m_sample_rate / 5);	// 0.2sec max
		m_noises[ft][FLOPPY_WAV_EJECT].alloc(m_sample_rate * 2);	// 2sec max
	}

	// load wav file
	for (int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		load_wav_files(m_noises[ft], FLOPPY_WAV_SNDTYPES, m_wav_loaded_at_first);
	}

	m_wav_loaded_at_first = true;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	// 5inch or 8inch
	switch(addr & 0x7) {
		case 0x0:
		case 0x1:
			// FDC access ok
			OUT_DEBUG2(_T("fddw a:%05x d:%02x"), (addr << 1) | 1, data);
			d_fdc->write_io8(addr, data);
			break;
		case 0x2:
			// drive control
			set_drive_control(data);
			OUT_DEBUG(_T("fddw a:%05x d:%02x"), (addr << 1) | 1, data);
			break;
		case 0x3:
			// access drive
			set_access_drive(data);
			OUT_DEBUG_REGW(_T("fddw a:%05x d:%02x"), (addr << 1) | 1, data);
			break;
#if 0
		case 0x10:
			// OPM address
			m_opm_addr = data;
			break;
		case 0x11:
			// OPM data ($1B only)
			if (m_opm_addr == 0x1b) {
				m_force_ready = ((data & 0x40) != 0);
			}
			break;
#endif
		default:
			break;
	}
}

#ifdef _DEBUG_FLOPPY_ALL
void out_debug_regr(EMU *emu, uint32_t addr, uint32_t data, const TCHAR *msg)
{
	int n = addr & 7;
	if (debug_regr[n] != data) {
		if (debug_regr_cnt[n] > 0) {
			OUT_DEBUG(_T("fddr a:%05x d:%02x repeat %d times"), addr, debug_regr[n], debug_regr_cnt[n]);
		}
		OUT_DEBUG(_T("fddr a:%05x d:%02x %s"), addr, data, msg != NULL ? msg : _T(""));
		debug_regr_cnt[n] = 0;
	} else {
		debug_regr_cnt[n]++;
	}
	debug_regr[n] = data;
}
#endif

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	switch(addr & 0x7) {
		case 0x0:
		case 0x1:
			// FDC access ok
			data = d_fdc->read_io8(addr & 0xf);
//			OUT_DEBUG_REGR(emu, (addr << 1) | 1, data, NULL);
			break;
		case 0x2:
			// FDD drive status
			data = get_drive_status(true);
			OUT_DEBUG_REGR(emu, (addr << 1) | 1, data, NULL);
			break;
		case 0x3:
			// FDC access drive
			data = m_accs_drv;
			OUT_DEBUG_REGR(emu, (addr << 1) | 1, data, NULL);
			break;
		default:
			break;
	}

	return data;
}

#if 0
void FLOPPY::write_dma_io8(uint32_t addr, uint32_t data)
{
}

uint32_t FLOPPY::read_dma_io8(uint32_t addr)
{
}
#endif

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	id &= 0xff;

	switch(id) {
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		warm_reset(false);
		return;
	case SIG_FLOPPY_FORCE_READY:
		m_force_ready = ((data & mask) != 0);
		return;
	}

	int ndrv_num = m_drv_num;
	if (ndrv_num >= MAX_DRIVE) return;

	switch (id) {
#ifdef USE_SIG_FLOPPY_ACCESS
		case SIG_FLOPPY_ACCESS:
			fdd[ndrv_num].access = (data & 1) ? true : false;
			break;
#endif
		case SIG_FLOPPY_WRITE:
			if(!m_ignore_write && m_fdd[ndrv_num].index < p_disk[ndrv_num]->sector_size) {
				p_disk[ndrv_num]->sector_data[m_fdd[ndrv_num].index] = (data & mask);
				m_fdd[ndrv_num].index++;
				m_fdd[ndrv_num].delay_write = DELAY_WRITE_FRAME;
			}
			break;
		case SIG_FLOPPY_WRITE_TRACK:
			if(!m_ignore_write && m_fdd[ndrv_num].index < p_disk[ndrv_num]->track_size) {
				p_disk[ndrv_num]->track[m_fdd[ndrv_num].index] = (data & mask);
				m_fdd[ndrv_num].index++;
				m_fdd[ndrv_num].delay_write = DELAY_WRITE_FRAME;
			}
			break;
		case SIG_FLOPPY_WRITEDELETE:
			if (!m_ignore_write) {
				p_disk[ndrv_num]->deleted = (data & 1) ? 0x10 : 0;
			}
			break;
		case SIG_FLOPPY_STEP:
			{
				int cur_track = m_fdd[ndrv_num].track;

				if (data < 0x80) m_fdd[ndrv_num].track++;
				else if (data > 0x80) m_fdd[ndrv_num].track--;

				if (m_fdd[ndrv_num].track < 0) m_fdd[ndrv_num].track = 0;
				if (m_fdd[ndrv_num].track > 255) m_fdd[ndrv_num].track = 255;

				if (cur_track != m_fdd[ndrv_num].track) {
					// fddseek sound turn on
					m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_SEEK].play();
//					m_wav_play[FLOPPY_WAV_SEEK] = 1;
//					m_wav_play_pos[FLOPPY_WAV_SEEK] = 0;
				}
			}
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			// TODO
			if (true) {
				// 2HD
				// 1,666,666.6... bits per track
				p_disk[ndrv_num]->track_size = DELAY_INDEX_HOLE_H / 8;
			} else {
				// 2DD
				// 1,000,000 bits per track
				p_disk[ndrv_num]->track_size = (DELAY_INDEX_HOLE / 2 / 8);
			}
			m_fdd[ndrv_num].index = 0;
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			fdd[ndrv_num].track = (data & mask);
			break;
#endif
		case SIG_FLOPPY_HEADLOAD:
#ifdef USE_FLOPPY_HEAD_LOADING
			m_head_load = (data & mask);
			if (p_disk[ndrv_num]->inserted) {
				if (m_head_load && !m_fdd[ndrv_num].head_loading) {
					// fddheadon sound turn on
					m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_HEADON].play();
					m_wav_play[FLOPPY_WAV_HEADON] = 1;
					m_wav_play_pos[FLOPPY_WAV_HEADON] = 0;
				}
			}
			if (m_head_load) {
				m_fdd[ndrv_num].head_loading = 120;
			}
			OUT_DEBUG(_T("fdd %d sig HEADLOAD %x UL:%d"), ndrv_num, m_head_load, m_fdd[ndrv_num].head_loading);
#endif
#ifdef USE_FLOPPY_TIMEOUT
			// When drive is ready, update timeout
			if ((p_disk[ndrv_num]->inserted && m_fdd[ndrv_num].ready >= 2) || m_force_ready) {
				register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
			}
#endif
			break;
		case SIG_FLOPPY_HEAD_SELECT:
			m_fdd[ndrv_num].side = (data & mask) ? 1 : 0;
			break;
		case SIG_FLOPPY_DENSITY:
			m_density = (data & mask) ? 1 : 0;
			break;
	}
}

uint32_t FLOPPY::read_signal(int id)
{
	uint32_t data = 0;

	id &= 0xffff;

	int ndrv_num = m_drv_num;
	if (ndrv_num >= MAX_DRIVE) {
		switch (id) {
		case SIG_FLOPPY_WRITEPROTECT:
			data = m_ignore_write ? 1 : 0;
			break;
		case SIG_FLOPPY_READY:
			data = m_force_ready ? 1 : 0;
			break;
		case SIG_FLOPPY_DENSITY:
			data = m_density;
			break;
		}
		return data;
	}

	switch (id) {
		case SIG_FLOPPY_READ_ID:
			if(m_fdd[ndrv_num].index < 6) {
				data = p_disk[ndrv_num]->id[m_fdd[ndrv_num].index];
				m_fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_READ:
			if(m_fdd[ndrv_num].index < p_disk[ndrv_num]->sector_size) {
				data = p_disk[ndrv_num]->sector_data[m_fdd[ndrv_num].index];
				m_fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_READ_TRACK:
			if(m_fdd[ndrv_num].index < p_disk[ndrv_num]->track_size) {
				data = p_disk[ndrv_num]->track[m_fdd[ndrv_num].index];
				m_fdd[ndrv_num].index++;
			}
			break;
		case SIG_FLOPPY_WRITEPROTECT:
			data = (p_disk[ndrv_num]->write_protected || m_ignore_write) ? 1 : 0;
			break;
		case SIG_FLOPPY_HEADLOAD:
#ifdef USE_FLOPPY_HEAD_LOADING
			data = (m_fdd[ndrv_num].head_loading & 0xff) ? 1 : 0;
#else
			data = 1;
#endif
			break;
		case SIG_FLOPPY_READY:
			data = ((p_disk[ndrv_num]->inserted && m_fdd[ndrv_num].ready >= 2) || m_force_ready) ? 1 : 0;
			break;
		case SIG_FLOPPY_TRACK0:
			data = (m_fdd[ndrv_num].track == 0) ? 1 : 0;
			break;
		case SIG_FLOPPY_INDEX:
			data = m_index_hole;
			break;
		case SIG_FLOPPY_DELETED:
			data = p_disk[ndrv_num]->deleted ? 1 : 0;
			break;
#ifdef SET_CURRENT_TRACK_IMMEDIATELY
		case SIG_FLOPPY_CURRENTTRACK:
			data = m_fdd[ndrv_num].track;
			break;
#endif
		case SIG_FLOPPY_SECTOR_NUM:
			data = p_disk[ndrv_num]->sector_nums;
			break;
		case SIG_FLOPPY_SECTOR_SIZE:
			data = p_disk[ndrv_num]->sector_size;
			break;
		case SIG_FLOPPY_TRACK_SIZE:
			data = p_disk[ndrv_num]->track_size;
			break;
		case SIG_FLOPPY_TRACK_REMAIN_SIZE:
			data = (p_disk[ndrv_num]->track_size - m_fdd[ndrv_num].index);
			break;
		case SIG_FLOPPY_DENSITY:
			data = m_density;
			break;
	}

	return data;
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void FLOPPY::event_frame()
{
	for(int i=0; i<MAX_DRIVE; i++) {
#ifdef USE_FLOPPY_HEAD_LOADING
		if (!m_head_load && (m_fdd[i].head_loading & 0xff)) {
			m_fdd[i].head_loading--;
		}
#endif
		// flash writing data to a real file
		if (m_fdd[i].delay_write > 0) {
			m_fdd[i].delay_write--;
			if (m_fdd[i].delay_write == 0) {
				p_disk[i]->flash();
			}
		}
		// assert interrupt after opened (inserted) disk
		if (m_fdd[i].opened_intr) {
			m_fdd[i].opened_intr--;
			if (m_fdd[i].opened_intr == 0) {
				// update current status
				m_fdd[i].inserted = p_disk[i]->inserted;
				set_irq(0xffffffff, 1 << i);
			}
		}
		// assert interrupt after closed (ejected) disk
		if (m_fdd[i].closed_intr) {
			m_fdd[i].closed_intr--;
			if (m_fdd[i].closed_intr == 0) {
				set_irq(0xffffffff, 1 << i);
			}
		}
		if (m_fdd[i].force_eject) {
			m_fdd[i].force_eject--;
		}
	}

	m_led_blink++;
	m_led_blink %= 60;
}

void FLOPPY::event_callback(int event_id, int err)
{
	int event_no = event_id;

	m_register_id[event_no] = -1;

	switch(event_no) {
		case EVENT_MOTOR_TIMEOUT:
			// motor off
#ifdef USE_FLOPPY_TIMEOUT
			OUT_DEBUG_MOTOR(_T("fdd -1 event MOTOR TIMEOUT!"));
			motor(-1, false);
#endif
			break;
		case EVENT_INDEXHOLE_ON:
			// index hole on
			m_index_hole = 1;
			register_index_hole_event(EVENT_INDEXHOLE_OFF, m_limit_index_hole);	// delay time of index hole off
			m_index_hole_next_clock += (m_delay_index_hole - m_limit_index_hole); 
			break;
		case EVENT_INDEXHOLE_OFF:
			// index hole off
			m_index_hole = 0;
			for(int i=0; i<MAX_DRIVE; i++) {
				if (m_fdd[i].motor_warmup) {
					m_fdd[i].motor_warmup <<= 1;
					if (m_fdd[i].motor_warmup > 4) m_fdd[i].motor_warmup = 0;
					OUT_DEBUG_MOTOR(_T("fdd %d event MOTOR WARMUP %d"), i, m_fdd[i].motor_warmup);
				}
			}
			register_index_hole_event(EVENT_INDEXHOLE_ON, m_delay_index_hole - m_limit_index_hole);	// index hole on
			break;
		case EVENT_READY_ON_0:
		case EVENT_READY_ON_1:
		case EVENT_READY_ON_2:
		case EVENT_READY_ON_3:
		case EVENT_READY_ON_4:
		case EVENT_READY_ON_5:
		case EVENT_READY_ON_6:
		case EVENT_READY_ON_7:
			{
				int i = event_no - EVENT_READY_ON_0;
				m_fdd[i].ready <<= 1;
				OUT_DEBUG_READY(_T("fdd %d event READY %d"), i, m_fdd[i].ready);
			}
			break;
		case EVENT_MOTOR_OFF:
			// motor off
			motor(-1, false);
			break;
//		case EVENT_INTERRUPT_OFF:
//			set_irq(false);
	}

}

// ----------------------------------------------------------------------------
/// $E94005 write
/// @param [in] data
void FLOPPY::set_drive_control(uint8_t data)
{
	if (data & 0xf) {
		m_drv_ctrl = data & 0xff;
	} else if (m_drv_ctrl & 0xf) {
		for (int drv=0; drv<MAX_DRIVE; drv++) {
			if (m_drv_ctrl & (1 << drv)) {
				// eject
				if (m_drv_ctrl & OPS_EJECT_ON) {
					close_disk(drv, 0);
				}
				m_fdd[drv].drv_ctrl = m_drv_ctrl & (OPS_EJECT_SW_MSK | OPS_LED_BLINK);	// led status
			}
		}
		m_drv_ctrl = 0;
	}
}

/// $E94005 read
/// @param [in] negate_intr : interrupt
uint8_t FLOPPY::get_drive_status(bool negate_intr)
{
	uint8_t data = 0;
	for(int drv=0; drv<MAX_DRIVE; drv++) {
		if (m_drv_ctrl & (1 << drv)) {
			if (m_fdd[drv].inserted) {
				data |= 0x80;
			}
			if (negate_intr) set_irq(0, 1 << drv);
		}
	}
	return data;
}

/// $E94007
/// @param [in] data
void FLOPPY::set_access_drive(uint8_t data)
{
	m_accs_drv = data;

	m_drv_num = (data & 3);

	if (m_drv_num >= MAX_DRIVE) return;

	// set drive speed
	set_drive_speed();

	if (data & ADS_MOTOR_ON) {
		// motor on
		OUT_DEBUG_MOTOR(_T("fdd %d MOTOR ON REQUEST data:%02x"), m_drv_num, data);
		motor(m_drv_num, true);
		m_motor_on_expand = 1;
	} else if ((data & ADS_MOTOR_ON) == 0 && m_motor_on_expand != 0) {
		// motor on -> off
		OUT_DEBUG_MOTOR(_T("fdd %d MOTOR OFF REQUEST data:%02x"), m_drv_num, data);
		register_my_event(EVENT_MOTOR_OFF, DELAY_MOTOR_OFF);	// delay motor off
		m_motor_on_expand = 1;
		// motor off signal and unselect any drives
		for(int i=0; i<MAX_DRIVE; i++) {
			m_fdd[i].motor_on = false;
		}
	} else {
		// motor off
		OUT_DEBUG_MOTOR(_T("fdd %d MOTOR OFF FORCE data:%02x"), m_drv_num, data);
		motor(-1, false);
	}
}

void FLOPPY::motor(int drv, bool val)
{
	if (drv >= MAX_DRIVE) return;

	if (val) {
		// motor on
#ifndef USE_FLOPPY_READY_ON_ALL_DRIVES
		m_fdd[drv].motor_on = true;

#ifdef USE_FLOPPY_TIMEOUT
		register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
#endif

		// motor sound on
		if (!m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].now_playing() && p_disk[drv]->inserted) {
			m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].play();
		}

		OUT_DEBUG_READY(_T("fdd %d MOTOR ON REQUEST READY:%d WARMUP:%d"), drv, m_fdd[drv].ready, m_fdd[drv].motor_warmup);

		// ready on
		if (m_fdd[drv].ready == 0) {
			if (p_disk[drv]->inserted) {
				m_fdd[drv].ready = 1;
				register_my_event(EVENT_READY_ON_0 + drv, m_delay_ready_on);	// delay ready on
				m_fdd[drv].motor_warmup = 1;
			}
		}

		cancel_my_event(EVENT_MOTOR_OFF);

		OUT_DEBUG_READY(_T("fdd %d MOTOR ON  READY:%d WARMUP:%d"), drv, m_fdd[drv].ready, m_fdd[drv].motor_warmup);
#else
		// motor on signal always turn on all drives
		bool inserted = false;
		for(int i=0; i<MAX_DRIVE; i++) {
			m_fdd[i].motor_on = true;
			inserted |= (p_disk[drv]->inserted != 0);
		}
#ifdef USE_FLOPPY_TIMEOUT
		register_my_event(EVENT_MOTOR_TIMEOUT, DELAY_MOTOR_TIMEOUT);	// delay motor off(timeout) (60s)
#endif
		// motor sound on
		if (!m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].now_playing() && inserted) {
			m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].play();
		}

		// ready on
#ifdef DEBUG_READY
		uint8_t prev_ready[MAX_DRIVE];
		for(int i=0; i<MAX_DRIVE; i++) {
			prev_ready[i] = m_fdd[i].ready;
		}
#endif
		for(int i=0; i<MAX_DRIVE; i++) {
			if (m_fdd[i].ready == 0) {
				if (p_disk[i]->inserted) {
					m_fdd[i].ready = 1;
					register_my_event(EVENT_READY_ON_0 + i, m_delay_ready_on);	// delay ready on
					m_fdd[i].motor_warmup = 1;
				}
				OUT_DEBUG_READY(_T("fdd %d MOTOR ON REQUEST READY:%d -> %d WARMUP:%d"), drv, prev_ready[i], m_fdd[i].ready, m_fdd[i].motor_warmup);
			}
#ifdef DEBUG_READY
			if (prev_ready[i] != m_fdd[i].ready) {
				OUT_DEBUG_READY(_T("fdd %d MOTOR ON  READY:%d -> %d WARMUP:%d"), drv, prev_ready[i], m_fdd[i].ready, m_fdd[i].motor_warmup);
			}
#endif
		}

		cancel_my_event(EVENT_MOTOR_OFF);
#endif
	} else {
		// motor off
		m_ignore_write = false;

		// motor sound off
		uint8_t all_ready = 0;
		for(int i=0; i<MAX_DRIVE; i++) {
			if (drv == -1 || drv == i) {
				m_fdd[i].motor_on = false;
				m_fdd[i].ready = 0;
				m_fdd[i].motor_warmup = 0;
				OUT_DEBUG_READY(_T("fdd %d MOTOR OFF READY:%d WARMUP:%d"), drv, m_fdd[i].ready, m_fdd[i].motor_warmup);
			}
			all_ready |= m_fdd[i].ready;
		}
		if (all_ready < 2) {
			// reset motor flag when all drives stopped
			m_motor_on_expand = 0;

//			m_drv_sel = 0xff;

			m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].stop();
		}

		OUT_DEBUG_READY(_T("fdd %d MOTOR OFF  SOUND:%d"), drv, all_ready);
	}
}

/// set drive type 2HD or 2DD
void FLOPPY::set_drive_speed()
{
	if (m_accs_drv & ADS_FD_TYPE) {
		// 2DD
		m_delay_index_hole = DELAY_INDEX_HOLE;
		m_delay_ready_on = DELAY_READY_ON;
		m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].set_period(m_sample_rate / 5);	// set 0.2sec
	} else {
		// 2HD
		m_delay_index_hole = DELAY_INDEX_HOLE_H;
		m_delay_ready_on = DELAY_READY_ON_H;
		m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_MOTOR].set_period(m_sample_rate / 6);	// set 0.16..sec
	}
	m_delay_index_hole = (int)((double)m_delay_index_hole * CPU_CLOCKS / 1000000.0);
	m_limit_index_hole = (int)(300.0 * CPU_CLOCKS / 1000000.0);

	m_delay_index_mark = 147 * 8 * 2;	// GAP4a + Sync + Index mark + GAP1 on MFM 
	if (m_accs_drv & ADS_FD_TYPE) {
		// 2DD
		m_delay_index_mark *= 2;
	}
	m_delay_index_mark = (int)((double)m_delay_index_mark * CPU_CLOCKS / 1000000.0);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------
bool FLOPPY::search_track(int channel)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return false;

	m_fdd[drvnum].index = 0;

	return p_disk[drvnum]->get_track(m_fdd[drvnum].track, m_fdd[drvnum].side);
}

bool FLOPPY::verify_track(int channel, int track)
{
	// verify track number
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return false;

	return p_disk[drvnum]->verify_track(track);
}

int FLOPPY::get_current_track_number(int channel)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return 0;

	return p_disk[drvnum]->id_c_in_track[0];
}

/// search sector
///
/// @param[in] fdcnum : FDC number
/// @param[in] drvnum : drive number
/// @param[in] index  : position of target sector in track
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector_main(int fdcnum, int drvnum, int index)
{
	int status = 0;

	do {
		int sta = p_disk[drvnum]->get_sector_by_index(m_fdd[drvnum].track, m_fdd[drvnum].side, index);
		if (sta) {
			if (sta == 3 && !m_fdd[drvnum].shown_media_error) {
				// different media type
				logging->out_logf_x(LOG_ERROR, CMsg::The_media_type_in_drive_VDIGIT_is_different_from_specified_one, drvnum); 
				m_fdd[drvnum].shown_media_error = true;
			}
			status = 1; // SECTOR NOT FOUND
			break;
		}
		// check density
		if (FLG_CHECK_FDDENSITY) {
			if (p_disk[drvnum]->density) {
				uint8_t sector_density = (((*p_disk[drvnum]->density) & 0x40) ? 0 : 1);
				if (m_density ^ sector_density) {
					if (!m_fdd[drvnum].shown_media_error) {
						logging->out_logf_x(LOG_ERROR, CMsg::The_density_in_track_VDIGIT_side_VDIGIT_is_different_from_specified_one, m_fdd[drvnum].track, m_fdd[drvnum].side); 
						m_fdd[drvnum].shown_media_error = true;
					}
					status = 1; // SECTOR NOT FOUND
					break;
				}
			}
		}

		m_fdd[drvnum].index = 0;

		if (p_disk[drvnum]->status && !m_ignore_crc) {
			status |= 2;	// CRC ERROR
		}
		if (p_disk[drvnum]->deleted) {
			status |= 4;	// DELETED MARK DETECTED
		}
	} while(0);

	OUT_DEBUG_DISK(_T("FDD %d SEARCH SECTOR MAIN idx:%d status:%d"), drvnum, index, status);

	return status;
}

/// search sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector(int channel)
{
	int drvnum = m_drv_num;

	int sector_num = p_disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	m_sectorcnt = (m_sectorcnt % sector_num);

	OUT_DEBUG_DISK(_T("FDD %d SEARCH SECTOR ID sectorcnt:%d"), drvnum, sectorcnt);

	return search_sector_main(0, drvnum, m_sectorcnt);
}

//int FLOPPY::search_sector(int channel, int sect)
//{
//	return search_sector(channel, sect, false, 0);
//}

/// search sector
///
/// @param[in] channel: FDC number(upper 16bit) 
/// @param[in] track  : track number
/// @param[in] sect   : sector number
/// @param[in] compare_side : whether compare side number or not
/// @param[in] side   : side number if compare number
/// @return bit0:Record Not Found / bit1:CRC Error / bit2:Deleted Mark Detected
int FLOPPY::search_sector(int channel, int track, int sect, bool compare_side, int side)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return 1;

	int sector_num = p_disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	if (sect < 256) {
		m_sectorcnt = p_disk[drvnum]->sector_pos[sect];
		m_sectorcnt = (m_sectorcnt % sector_num);
	}

	int status = search_sector_main(0, drvnum, m_sectorcnt);
	do {
		if (status) {
			break;
		}

		// check track in id field
		if(p_disk[drvnum]->id[0] != track) {
			status = 1;
			break;
		}
		// check sector in id field
		if(p_disk[drvnum]->id[2] != sect) {
			status = 1;
			break;
		}
		// check side in id field
		if(compare_side && p_disk[drvnum]->id[1] != side) {
			status = 1;
			break;
		}
	} while(0);

	OUT_DEBUG_DISK(_T("FDD %d SEARCH SECTOR trk:%d sec:%d status:%d"), drvnum, track, sect, status);

	return status;
}

bool FLOPPY::make_track(int channel)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return false;

	return p_disk[drvnum]->make_track(m_fdd[drvnum].track, m_fdd[drvnum].side, m_density);
}

bool FLOPPY::parse_track(int channel)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return false;

//	return disk[drvnum]->parse_track(fdd[drvnum].track, fdd[drvnum].side, density);
	return p_disk[drvnum]->parse_track2(m_fdd[drvnum].track, m_fdd[drvnum].side, m_density);
}

// ----------------------------------------------------------------------------

int FLOPPY::get_a_round_clock(int channel)
{
	return m_delay_index_hole;
}

int FLOPPY::get_head_loading_clock(int channel)
{
#ifdef USE_FLOPPY_HEAD_LOADING
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return HEAD_LOADED_CLOCK;

	return (m_fdd[m_drv_num].head_loading & 0xff) ? 200 : HEAD_LOADED_CLOCK;
#else
	return 1;
#endif
}

int FLOPPY::get_index_hole_remain_clock()
{
	int64_t sum = 0;
	sum = m_index_hole_next_clock - get_current_clock();
	if (sum < 0) sum = 0;
	return (int)sum;
}

int FLOPPY::calc_index_hole_search_clock(int channel)
{
	int sum = get_head_loading_clock(channel);
	int idx_sum = 0;
	m_sectorcnt_cont = false;
	if (!FLG_DELAY_FDSEARCH) {
		idx_sum = get_index_hole_remain_clock();
		if (sum >= idx_sum) sum = idx_sum;
		else sum = idx_sum + m_delay_index_hole;
	}

//	logging->out_debugf(_T("calc_index_hole_search_clock: idx:%06d sum:%06d"), idx_sum, sum);
	return sum;
}

int FLOPPY::get_clock_arrival_sector(int channel, int sect, int delay)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return DELAY_INDEX_HOLE;

	int sector_num = p_disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;
	int sector_pos = 0;
	if (sect < 256) {
		sector_pos = p_disk[drvnum]->sector_pos[sect];
		sector_pos = (sector_pos % sector_num);
	}

	int sect_sum = sector_pos * (m_delay_index_hole - m_delay_index_mark) / sector_num;
	int idx_sum = get_index_hole_remain_clock() - delay;
	int sum = sect_sum + m_delay_index_mark + idx_sum - m_delay_index_hole;
	if (sum < 0) sum += m_delay_index_hole;
	sum += delay;

//	logging->out_debugf(_T("get_clock_arrival_sector: sect:%02d %06d idx:%06d sum:%06d"), sect, sect_sum, delay_index_hole - idx_sum, sum);
	return (sum);
}

int FLOPPY::get_clock_next_sector(int channel, int delay)
{
	int drvnum = m_drv_num;
	if (drvnum >= MAX_DRIVE) return DELAY_INDEX_HOLE;

//	int fdcnum = (channel >> 16);

	int sector_num = p_disk[drvnum]->sector_nums;
	if (sector_num <= 0) sector_num = 16;

	int sect_sum = (m_delay_index_hole - m_delay_index_mark) / sector_num;
	int idx_sum = get_index_hole_remain_clock() - delay;
	if (idx_sum < 0) idx_sum += m_delay_index_hole;
	int sector_pos = sector_num - (idx_sum / sect_sum);
	if (sector_pos < 0) sector_pos = 0;

	if (FLG_DELAY_FDSEARCH && m_sectorcnt_cont) {
		sector_pos = m_sectorcnt + 1;
	}

	sector_pos %= sector_num;

	m_sectorcnt = sector_pos;
	m_sectorcnt_cont = true;

	if (FLG_DELAY_FDSEARCH) return 200;

	sect_sum = sector_pos * sect_sum;
	int sum = sect_sum + m_delay_index_mark + idx_sum - m_delay_index_hole;
	if (sum < 0) sum += m_delay_index_hole;
	sum += delay;

//	logging->out_debugf(_T("get_clock_next_sector: sect_pos:%02d cur_clk:%lld next_clk:%lld idx:%06d sect_sum:%06d sum:%06d"), sector_pos, get_current_clock(), index_hole_next_clock, idx_sum, sect_sum, sum);
	return (sum);
}

int FLOPPY::calc_sector_search_clock(int channel, int sect)
{
	int sum_clk = 0;
	m_sectorcnt_cont = false;
	if (!FLG_DELAY_FDSEARCH) sum_clk = get_clock_arrival_sector(channel, sect, get_head_loading_clock(channel));
//	logging->out_debugf(_T("calc_sector_search_clock: sect:%02d hld:%06d clk:%06d"), sect, sum_hld, sum_clk);
	return sum_clk;
}

int FLOPPY::calc_next_sector_clock(int channel)
{
	return get_clock_next_sector(channel, get_head_loading_clock(channel));
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------
void FLOPPY::set_irq(uint32_t data, uint32_t mask)
{
	m_now_irq = (data & mask ? m_now_irq | mask : m_now_irq & ~mask);
	write_signals(&outputs_irq, m_now_irq ? 0xffffffff : 0);
}

//void FLOPPY::set_drq(bool val)
//{
//	// Always relase HALT signal
//	write_signals(&outputs_drq, 0);
//}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------
bool FLOPPY::open_disk(int drv, const _TCHAR *path, int offset, uint32_t flags)
{
	if(drv < MAX_DRIVE) {
		bool rc = p_disk[drv]->open(path, offset, flags);
		if (rc) {
			m_fdd[drv].delay_write = 0;
			m_fdd[drv].shown_media_error = false;

			if (!(flags & OPEN_DISK_FLAGS_FORCELY)) {
				// insert event
				m_fdd[drv].opened_intr = m_fdd[drv].closed_intr + 30;
			} else {
				// update current status
				m_fdd[drv].inserted = p_disk[drv]->inserted;
			}
		}

		OUT_DEBUG_DISK(_T("FDD %d OPEN rc:%d"), drv, rc ? 1 : 0);

		return rc;
	} else {
		return false;
	}
}

bool FLOPPY::close_disk(int drv, uint32_t flags)
{
	if(drv < MAX_DRIVE) {
		if (flags & OPEN_DISK_FLAGS_FORCELY) {

			p_disk[drv]->close();

		} else {

			if ((m_fdd[drv].drv_ctrl & OPS_EJECT_SW_MSK) != 0) {
				if (m_fdd[drv].force_eject == 0) {
					// eject switch is locked now
					logging->out_logf_x(LOG_WARN, CMsg::Eject_switch_is_locked_on_drive_VDIGIT_Reselect_the_menu, drv);
					m_fdd[drv].force_eject = 180;

					OUT_DEBUG_DISK(_T("FDD %d EJECT LOCKED"), drv);

					return false;
				} else {
					logging->out_logf_x(LOG_INFO, CMsg::The_disk_on_drive_VDIGIT_was_ejected_forcely, drv);
				}
			}
			if (p_disk[drv]->inserted) {
				// eject sound
				m_noises[FLOPPY_WAV_TYPE2HD][FLOPPY_WAV_EJECT].play();
			}

			p_disk[drv]->close();

			set_disk_side(drv, 0);
			motor(drv, false);
			m_fdd[drv].delay_write = 0;
			m_fdd[drv].inserted = p_disk[drv]->inserted;
			m_fdd[drv].ready = m_fdd[drv].motor_warmup = 0;

			m_fdd[drv].closed_intr = m_fdd[drv].opened_intr + 30;

			cancel_my_event(EVENT_READY_ON_0 + drv);

		}

		OUT_DEBUG_DISK(_T("FDD %d CLOSE"), drv);

	}
	return true;
}

int FLOPPY::change_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		set_disk_side(drv, 1 - m_fdd[drv].side);
		motor(drv, false);
		return m_fdd[drv].side;
	} else {
		return 0;
	}
}

void FLOPPY::set_disk_side(int drv, int side)
{
	m_fdd[drv].side = (side & 1) % p_disk[drv]->num_of_side;
//	if (m_fdd[drv].side) {
//		sidereg |= (1 << drv);
//	} else {
//		sidereg &= ~(1 << drv);
//	}
}

int FLOPPY::get_disk_side(int drv)
{
	return (p_disk[drv]->num_of_side > 1 ? m_fdd[drv].side % p_disk[drv]->num_of_side : -1);
}

bool FLOPPY::disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return p_disk[drv]->inserted;
	}
	return false;
}

void FLOPPY::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		p_disk[drv]->set_drive_type(type);
	}
}

uint8_t FLOPPY::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return p_disk[drv]->get_drive_type();
	}
	return DRIVE_TYPE_UNK;
}

uint8_t FLOPPY::fdc_status()
{
	return 0;
}

void FLOPPY::toggle_disk_write_protect(int drv)
{
	if(drv < MAX_DRIVE) {
		if (m_ignore_write) {
			m_ignore_write = false;
			p_disk[drv]->set_write_protect(true);
		}
		p_disk[drv]->set_write_protect(!(p_disk[drv]->write_protected));
	}
}

bool FLOPPY::disk_write_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return (p_disk[drv]->write_protected || m_ignore_write);
	}
	return true;
}

bool FLOPPY::is_same_disk(int drv, const _TCHAR *file_path, int offset)
{
	if(drv < MAX_DRIVE) {
		return p_disk[drv]->is_same_file(file_path, offset);
	}
	return false;
}

/// Is the disk already inserted in another drive?
int FLOPPY::inserted_disk_another_drive(int drv, const _TCHAR *file_path, int offset)
{
	int match = -1;
	for(int i=0; i<MAX_DRIVE; i++) {
		if (i == drv) continue;
		if (p_disk[i]->is_same_file(file_path, offset)) {
			match = i;
			break;
		}
	}
	return match;
}

// ----------------------------------------------------------------------------
#if 0
uint16_t FLOPPY::get_drive_select()
{
	// for led indicator
	// b3-b0: drive select, b4-b7: 0:green led, 1:red led b8-11:inserted?
	uint16_t data = 0;

	if (!pConfig->now_power_off) {
		switch(pConfig->fdd_type) {
		case FDD_TYPE_5FDD:
		case FDD_TYPE_58FDD:
			// 5inch or 8inch
			int drvnum = m_drv_num;
			if (drvnum >= MAX_DRIVE) break;

			if (m_fdd[drvnum].head_loading > 0 && m_fdd[drvnum].ready >= 2) {
				data |= ((1 << drvnum) | (0x10 << drvnum));
			}
			break;
		}
	}

	// inserted diskette ?
	for(int i=0; i<MAX_DRIVE; i++) {
		if (p_disk[i]->inserted) {
			data |= (1 << (8 + i));
		}
	}

	return data;
}
#endif

/// b0-b2 FDD0, b3-b5 FDD1, b6-b8 FDD2, b9-b11 FDD3
uint32_t FLOPPY::get_led_status() const
{
	// b0: led green
	// b1: led red
	// b2: led eject
	uint16_t data = 0;

	for(int drv=(MAX_DRIVE-1); drv>=0; --drv) {
		data <<= 3;
		if (m_fdd[drv].inserted) {
			// drive led
			if (m_fdd[drv].motor_on && m_fdd[drv].ready && drv == m_drv_num) {
				data |= 0x02;
			} else {
				data |= 0x01;
			}
			// eject led
			if (!(m_fdd[drv].drv_ctrl & OPS_EJECT_SW_MSK)) {
				data |= 0x04;
			}
		} else {
			// drive led blink
			if ((m_fdd[drv].drv_ctrl & OPS_LED_BLINK) != 0 && (m_led_blink < 30)) {
				data |= 0x01;
			}
		}
	}
	return data;
}

// ----------------------------------------------------------------------------
void FLOPPY::initialize_sound(int rate, int decibel)
{
	SOUND_BASE::initialize_sound(rate, decibel);

	// load wav file
	load_wav();
}

void FLOPPY::set_volume(int decibel, bool vol_mute)
{
	int wav_volume = decibel_to_volume(decibel);

	if (vol_mute) wav_volume = 0;
	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].set_volume(wav_volume, wav_volume);
		}
	}
}

void FLOPPY::mix(int32_t* buffer, int cnt)
{
//	int32_t *buffer_start = buffer;

	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].mix(buffer, cnt);
		}
	}
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void FLOPPY::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	// 0
	SET_Bool(m_ignore_crc);
	SET_Byte(m_drv_num);	///< drive number
	SET_Byte(m_index_hole);	///< index hole
#ifdef USE_FLOPPY_HEAD_LOADING
	SET_Byte(m_head_load);	///< head loaded
#endif
	SET_Byte(m_density);	///< density (FM = 0, MFM = 1)
	SET_Byte(m_motor_on_expand);	///< motor on (expand)
	SET_Byte(m_drv_ctrl);
	SET_Byte(m_accs_drv);
	SET_Uint64_LE(m_index_hole_next_clock);

	// 1
	for(int i=0; i<4 && i<MAX_DRIVE; i++) {
		SET_Int32_LE(m_fdd[i].side);
		SET_Int32_LE(m_fdd[i].track);
		SET_Int32_LE(m_fdd[i].index);
#ifdef USE_SIG_FLOPPY_ACCESS
		SET_Byte(m_fdd[i].access);
#endif
		SET_Byte(m_fdd[i].ready);			///< ready warmup:01 on:11
		SET_Byte(m_fdd[i].motor_warmup);	///< motor warming up:1
#ifdef USE_FLOPPY_HEAD_LOADING
		SET_Byte(m_fdd[i].head_loading);
#endif
		SET_Int32_LE(m_fdd[i].delay_write);

		SET_Bool(m_fdd[i].motor_on);
		SET_Bool(m_fdd[i].inserted);			///< later than disk->inserted
		SET_Bool(m_fdd[i].shown_media_error); 

		SET_Byte(m_fdd[i].drv_ctrl);
		SET_Byte(m_fdd[i].opened_intr);	///< delay time of asserting interrupt after inserted disk
		SET_Byte(m_fdd[i].closed_intr);	///< delay time of asserting interrupt after ejected disk
		SET_Byte(m_fdd[i].force_eject);	///< enable eject forcely while over zero
	}

	SET_Int32_LE(m_led_blink);	///< blink led counter

	SET_Int32_LE(m_delay_ready_on);

	SET_Bool(m_force_ready);
	SET_Bool(m_sectorcnt_cont);

	SET_Int32_LE(m_sectorcnt);

	SET_Uint32_LE(m_now_irq);	///< current interrupt status

	for(int i=0; i<15 && i<EVENT_IDS_MAX ; i++) {
		SET_Int32_LE(m_register_id[i]);
	}

	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].save_state(vm_state.m_noises[ft][ty]);
		}
	}

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool FLOPPY::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	// 0
	GET_Bool(m_ignore_crc);
	GET_Byte(m_drv_num);	///< drive number
	GET_Byte(m_index_hole);	///< index hole
#ifdef USE_FLOPPY_HEAD_LOADING
	GET_Byte(m_head_load);	///< head loaded
#endif
	GET_Byte(m_density);	///< density (FM = 0, MFM = 1)
	GET_Byte(m_motor_on_expand);	///< motor on (expand)
	GET_Byte(m_drv_ctrl);
	GET_Byte(m_accs_drv);
	GET_Uint64_LE(m_index_hole_next_clock);

	// 1
	for(int i=0; i<4 && i<MAX_DRIVE; i++) {
		GET_Int32_LE(m_fdd[i].side);
		GET_Int32_LE(m_fdd[i].track);
		GET_Int32_LE(m_fdd[i].index);
#ifdef USE_SIG_FLOPPY_ACCESS
		GET_Byte(m_fdd[i].access);
#endif
		GET_Byte(m_fdd[i].ready);			///< ready warmup:01 on:11
		GET_Byte(m_fdd[i].motor_warmup);	///< motor warming up:1
#ifdef USE_FLOPPY_HEAD_LOADING
		GET_Byte(m_fdd[i].head_loading);
#endif
		GET_Int32_LE(m_fdd[i].delay_write);

		GET_Bool(m_fdd[i].motor_on);
		GET_Bool(m_fdd[i].inserted);			///< later than disk->inserted
		GET_Bool(m_fdd[i].shown_media_error); 

		GET_Byte(m_fdd[i].drv_ctrl);
		GET_Byte(m_fdd[i].opened_intr);	///< delay time of asserting interrupt after inserted disk
		GET_Byte(m_fdd[i].closed_intr);	///< delay time of asserting interrupt after ejected disk
		GET_Byte(m_fdd[i].force_eject);	///< enable eject forcely while over zero
	}

	GET_Int32_LE(m_led_blink);	///< blink led counter

	GET_Int32_LE(m_delay_ready_on);

	GET_Bool(m_force_ready);
	GET_Bool(m_sectorcnt_cont);

	GET_Int32_LE(m_sectorcnt);

	GET_Uint32_LE(m_now_irq);	///< current interrupt status

	for(int i=0; i<15 && i<EVENT_IDS_MAX ; i++) {
		GET_Int32_LE(m_register_id[i]);
	}

	for(int ft=0; ft<FLOPPY_WAV_FDDTYPES; ft++) {
		for(int ty=0; ty<FLOPPY_WAV_SNDTYPES; ty++) {
			m_noises[ft][ty].load_state(vm_state.m_noises[ft][ty]);
		}
	}

	set_drive_speed();

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t FLOPPY::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	switch(addr & 0x1f) {
		case 0x0:
		case 0x1:
			// FDC access
			data = d_fdc->debug_read_io8(addr & 0xf);
			break;
		case 0x2:
			data = get_drive_status(true);
			break;
		case 0x3:
			data = m_accs_drv;
			break;
	}

	return data;
}

static const _TCHAR *c_reg_names[]  = {
	_T("DRV_CTRL"),
	_T("DRV_STAT"),
	_T("ACCS_DRV"),
	NULL
};

bool FLOPPY::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool FLOPPY::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	switch (reg_num) {
	case 0:
		m_drv_ctrl = data;
		return true;
	case 2:
		m_accs_drv = data;
		return true;
	}
	return false;
}

void FLOPPY::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("FDD:\n"));
	int drv_stat = get_drive_status(false);
	UTILITY::sntprintf(buffer, buffer_len, _T(" 00($E94005 W:%-8s):%02X\n"), c_reg_names[0], m_drv_ctrl);
	UTILITY::sntprintf(buffer, buffer_len, _T(" 01($E94005 R:%-8s):%02X\n"), c_reg_names[1], drv_stat);
	UTILITY::sntprintf(buffer, buffer_len, _T(" 02($E94007 W:%-8s):%02X\n"), c_reg_names[2], m_accs_drv);
}

#endif

