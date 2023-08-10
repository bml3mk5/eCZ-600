/** @file adpcm.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@par Origin okim6528.cpp on MAME 0.293 and msm5205.cpp on CSP
	@author Sasaji
	@date   2022.02.22 -

	@brief [ MSM6258 modoki (ADPCM) ]
*/

#include "adpcm.h"
#include <math.h>
#include "../../utility.h"
#include "../../fileio.h"
#include "../../logging.h"

#ifdef _DEBUG
//#define OUT_DEBUG_CALC logging->out_debugf
#define OUT_DEBUG_CALC(...)
//#define OUT_DEBUG_SIGW logging->out_debugf
#define OUT_DEBUG_SIGW(...)
//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_CLKC logging->out_debugf
#define OUT_DEBUG_CLKC(...)
#else
#define OUT_DEBUG_CALC(...)
#define OUT_DEBUG_SIGW(...)
#define OUT_DEBUG_REGW(...)
#define OUT_DEBUG_CLKC(...)
#endif

#define ADPCM_DECODE_FILTER 0
#define ADPCM_SAMPLES_FILTER 3
#define MIX_SAMPLES_FILTER 3

/* clock divider */
const double ADPCM::c_dividers[4] = { 1024.0, 768.0, 512.0, 768.0 };
// step size index shift table
const int ADPCM::c_index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

ADPCM::ADPCM(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: SOUND_BASE(parent_vm, parent_emu, identifier)
{
	set_class_name("ADPCM");

	init_output_signals(&outputs_mck);
//	volume_m = 1024;
	volume_l = volume_r = 1024;

	m_tables_computed = 0;
	m_base_clock = 4096000;
	m_clock_mag = 1;
	m_period = 3900.0;

#define OUTPUT_BITS 10
	m_output_bits = OUTPUT_BITS;
}

void ADPCM::initialize()
{
	/* compute the difference tables */
	compute_tables();

	/* stream system initialize */
	m_timer_event_id = -1;

	m_prescaler = 0;

	m_signal_max = (1 << (m_output_bits - 1)) - 1;
	m_signal_min = -(1 << (m_output_bits - 1));

	m_pan_left = 0xffffffff;
	m_pan_right = 0xffffffff;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ADPCM::reset()
{
	warm_reset(true);
}

void ADPCM::warm_reset(bool por)
{
	/* initialize work */
	m_reg_cmd = 0;
	m_reg_sts = 0;
	m_reg_rdata = 0;
	m_reg_wdata = 0;

//	m_data    = 0;
	m_vclk    = 0;
	m_req_data = 0;
//	m_reset   = 0;
	m_signal  = 0;
	m_step    = 0;

	m_silent_count = 0;

	memset(m_signals_buff_l, 0, sizeof(m_signals_buff_l));
	memset(m_signals_buff_r, 0, sizeof(m_signals_buff_r));
	m_signals_w = 0;
	memset(m_prev_signals, 0, sizeof(m_prev_signals));
	memset(m_prev_samples, 0, sizeof(m_prev_samples));
	m_prev_sample_l = 0;
	m_prev_sample_r = 0;
//	m_data_in = 0;

	/* initialize clock */
	change_clock();

	if (!por) {
		cancel_event(this, m_timer_event_id);
	}
	m_timer_event_id = -1;
}

/*
 *   Compute the difference table
 */
void ADPCM::compute_tables()
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		double stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			m_diff_lookup[step*16 + nib] = (int)((double)nbl2bit[nib][0] *
				(stepval * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8));
		}
	}

	m_tables_computed = 1;

}

void ADPCM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
	case REG_DATA:
		// write data
		OUT_DEBUG_REGW(_T("ADPCM DATAW data:%02X prev:%02X"), data, m_reg_wdata);
		if (((m_reg_wdata ^ data) & 0xff) == 0) {
			m_silent_count++;
		} else {
			m_silent_count=0;
		}
		m_reg_wdata = data;
		m_req_data = 0;
		if (m_reg_sts & STATUS_PLAYING) {
			calc_samples(m_reg_wdata);
			calc_samples(m_reg_wdata >> 4);
		}
		break;
	case REG_CMD_STS:
		// write command
		m_reg_cmd = data;
		if (data & COMMAND_STOP) {
			play(false);

			m_reg_sts &= ~(STATUS_PLAYING | STATUS_RECORDING);
			m_reg_sts |= STATUS_IDLING;	// bit7 is 1

			// fall down mck, so busreq negate.
			write_signals(&outputs_mck, 0);

		} else if (data & COMMAND_PLAY) {
//			if (!(m_reg_sts & STATUS_PLAYING)) {
				play(true);

				m_reg_sts = STATUS_PLAYING;	// bit7 is 0

//			}
		}
		break;
	}
}
uint32_t ADPCM::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

	switch(addr & 1) {
	case REG_DATA:
		// read data
		data = m_reg_rdata;
		break;
	case REG_CMD_STS:
		// read status
		data = (m_reg_sts & STATUS_IDLING) | 0x40;
		break;
	}
	return data;
}

void ADPCM::write_signal(int id, uint32_t data, uint32_t mask)
{
	int nval;

	switch(id) {
	case SIG_DACK:
		// for X68k
		// fall down mck, so busreq negate.
		write_signals(&outputs_mck, 0);
		break;
	case SIG_SEL_FREQUENCY:
		// set various parameter
		// D2-D3: sampling frequency
		nval = (data & 0x0c) >> 2;
		if (nval != m_prescaler) {
			m_prescaler = nval;
			change_clock();
		}
		// D0-D1: PCM pan (left or right) for X68k
		m_pan_right = (data & 0x01) ? 0 : 0xffffffff;
		m_pan_left  = (data & 0x02) ? 0 : 0xffffffff;

		OUT_DEBUG_SIGW(_T("ADPCM: WRITE FREQ clk:%d prescale:%d period:%f"), m_base_clock * m_clock_mag, m_prescaler, m_period);
		break;
	case SIG_SEL_CLOCK:
		nval = 2 - (data & mask & 1);
		if (nval != m_clock_mag) {
			// recalc sampling rate
			m_clock_mag = nval;
			change_clock();
		}
		break;
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		warm_reset(false);
		break;
	}
}

void ADPCM::calc_samples(uint8_t data)
{
//	if (m_reg_sts & STATUS_PLAYING) {
//		int nibble_shift = m_nibble_shift;
//		if (!m_nibble_shift) {
//			m_data_in = m_reg_wdata;
//		}

		/* Compute the new amplitude and update the current step */
		int nibble = data & 0xf;
//		m_data_in >>= 4;

		/* Output to the buffer */
		int32_t signal = clock_adpcm(nibble);

#if (ADPCM_SAMPLES_FILTER == 1)
		// low pass filter
		for(int i=1; i>=0; i--) {
			m_prev_signals[i + 1] = m_prev_signals[i];
			m_prev_samples[i + 1] = m_prev_samples[i];
		}
		m_prev_signals[0] = signal; // * 16;

		signal = (m_prev_signals[0] * 5 + m_prev_samples[1] * 11) / 16;
		m_prev_samples[0] = signal;

		m_signals_buff_l[m_signals_w] = signal & m_pan_left;
		m_signals_buff_r[m_signals_w] = signal & m_pan_right;

#elif (ADPCM_SAMPLES_FILTER == 2)
		// band pass filter
		for(int i=1; i>=0; i--) {
			m_prev_signals[i + 1] = m_prev_signals[i];
			m_prev_samples[i + 1] = m_prev_samples[i];
		}
		signal *= 64;
		m_prev_signals[0] = signal;

		int32_t subval = 0;
#if 1
		subval += 23*(m_prev_samples[1]); 
		subval -= 6*(m_prev_samples[2]); 
		subval += 11*(m_prev_signals[0]); 
		subval -= 10*(m_prev_signals[1]);
		subval /= 32;
		m_prev_samples[0] = subval;
#else
#if MAX_PREV_SIGNALS == 16
		subval += (m_prev_signals[0] - m_prev_signals[15]) * 12;
		subval += (m_prev_signals[8] - m_prev_signals[15]) * 6;
		subval += (m_prev_signals[12] - m_prev_signals[15]) * 2;
#elif MAX_PREV_SIGNALS == 32
		subval += (m_prev_signals[0] - m_prev_signals[31]) * 10;
		subval += (m_prev_signals[8] - m_prev_signals[31]) * 7;
		subval += (m_prev_signals[16] - m_prev_signals[31]) * 4;
		subval += (m_prev_signals[24] - m_prev_signals[31]) * 2;
		subval += (m_prev_signals[28] - m_prev_signals[31]);
#endif
#endif
		m_signals_buff_l[m_signals_w] = subval & m_pan_left;
		m_signals_buff_r[m_signals_w] = subval & m_pan_right;

#elif (ADPCM_SAMPLES_FILTER == 3)
		/* compression filter */
#if (OUTPUT_BITS == 12)
		if (signal < 256 && signal >= -256) {
			signal *= 16;
		} else if (signal < 512 && signal >= -512) {
			int32_t nsignal = (abs(signal) - 256) * 4 + 256 * 16;
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		} else if (signal < 1024 && signal >= -1024) {
			int32_t nsignal = (abs(signal) - 512) + ((512 - 256) * 4 + 256 * 16);
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		} else {
			int32_t nsignal = (abs(signal) - 1024) / 4 + (1024 - 512) + ((512 - 256) * 4 + 256 * 16);
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		}

#elif (OUTPUT_BITS == 11)
		if (signal < 448 && signal >= -448) {
			signal *= 16;
		} else {
			int32_t nsignal = (abs(signal) - 448) * 4 + 448 * 16;
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		}

#elif (OUTPUT_BITS == 10)
		if (signal < 160 && signal >= -160) {
			signal *= 32;
		} else if (signal < 368 && signal >= -368) {
			int32_t nsignal = (abs(signal) - 160) * 16 + 160 * 32;
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		} else {
			int32_t nsignal = (abs(signal) - 368) * 8 + (368 - 160) * 16 + 160 * 32;
			if (signal >= 0) signal = nsignal;
			else signal = - nsignal;
		}

#else
		signal *= 16;

#endif
		m_signals_buff_l[m_signals_w] = signal & m_pan_left;
		m_signals_buff_r[m_signals_w] = signal & m_pan_right;

#elif (ADPCM_SAMPLES_FILTER == 4)
		for(int i=1; i>=0; i--) {
			m_prev_signals[i + 1] = m_prev_signals[i];
			m_prev_samples[i + 1] = m_prev_samples[i];
		}
		m_prev_signals[0] = signal;
		int32_t normalize = m_prev_samples[1];
		int32_t subval = m_prev_signals[0] - m_prev_signals[1];
		if (subval >= 640 || subval < -640) {
			if (normalize < 128) normalize += 4;
		}
		signal *= 16;
		if (normalize > 0) {
			signal /= 4;
			normalize--;
		}

		m_prev_samples[0] = normalize;

		m_signals_buff_l[m_signals_w] = signal & m_pan_left;
		m_signals_buff_r[m_signals_w] = signal & m_pan_right;

#else
		// no filter
		signal *= 16;
		m_signals_buff_l[m_signals_w] = signal & m_pan_left;
		m_signals_buff_r[m_signals_w] = signal & m_pan_right;

#endif
		if (m_signals_w < (BUFF_SIZE - 2)) m_signals_w++;

//		m_nibble_shift ^= 4;

//		/* Update the parameters */
//		m_nibble_shift = nibble_shift;
//	}
}

int32_t ADPCM::clock_adpcm(uint8_t nibble)
{
	m_signal += m_diff_lookup[m_step * 16 + (nibble & 15)];

	/* clamp to the maximum */
	if (m_signal > m_signal_max) 
		m_signal = m_signal_max;
	else if (m_signal < m_signal_min)
		m_signal = m_signal_min;

	/* adjust the step size and clamp */
	m_step += c_index_shift[nibble & 7];
	if (m_step > 48)
		m_step = 48;
	else if (m_step < 0)
		m_step = 0;

#if (ADPCM_DECODE_FILTER == 1)
	// low pass filter
	for(int i=1; i>=0; i--) {
		m_prev_signals[i + 1] = m_prev_signals[i];
		m_prev_samples[i + 1] = m_prev_samples[i];
	}
	m_prev_signals[0] = m_signal;

	m_signal = (m_prev_signals[0] * 5 + m_prev_samples[1] * 11) / 16;

	m_prev_samples[0] = m_signal;

	OUT_DEBUG_CALC(_T("ADPCM N:%X STEP:%02d X:% 4d SIG:% 4d"), nibble, m_step, signal_x, m_signal); 

#elif (ADPCM_DECODE_FILTER == 2)
	// compression
	int32_t signal_x = m_signal * m_signal;
#if (OUTPUT_BITS == 11)
	signal_x /= ((m_signal_max + 1) * 3);
#elif (OUTPUT_BITS == 10)
	signal_x /= ((m_signal_max + 1) * 32);
#else
	signal_x /= ((m_signal_max + 1) * 2);
#endif

	if (m_signal > 0) m_signal -= signal_x;
	else if (m_signal < 0) m_signal += signal_x;

	OUT_DEBUG_CALC(_T("ADPCM N:%X STEP:%02d X:% 4d SIG:% 4d"), nibble, m_step, signal_x, m_signal); 

#elif (ADPCM_DECODE_FILTER == 3)
	// compression
	int32_t signal_x = m_signal;
#if (OUTPUT_BITS == 12) || (OUTPUT_BITS == 11)
	if (m_signal >= 512) {
		signal_x = (m_signal - 512) / 2 + 512;
	} else if (m_signal < -512) {
		signal_x = (m_signal + 512) / 2 - 512;
	}
#elif (OUTPUT_BITS == 10)
	if (m_signal >= 440) {
		signal_x = (m_signal - 440) / 8 + 440;
	} else if (m_signal < -440) {
		signal_x = (m_signal + 440) / 8 - 440;
	}
#endif
	m_signal = signal_x;

	OUT_DEBUG_CALC(_T("ADPCM N:%X STEP:%02d X:% 4d SIG:% 4d"), nibble, m_step, signal_x, m_signal); 

#elif (ADPCM_DECODE_FILTER == 4)
	for(int i=1; i>=0; i--) {
		m_prev_signals[i + 1] = m_prev_signals[i];
		m_prev_samples[i + 1] = m_prev_samples[i];
	}
	m_prev_signals[0] = m_signal;

	int32_t signal_x = m_prev_signals[0] - m_prev_signals[1];
	if (signal_x < 512 && signal_x >= -512) {
		m_signal *= 16;
	} else if (signal_x < 1024 && signal_x >= -1024) {
		m_signal *= 4;
	} else if (signal_x < 2048 && signal_x >= -2048) {
		m_signal /= 4;
	} else {
		m_signal /= 16;
	}

	m_prev_samples[0] = m_signal;

	OUT_DEBUG_CALC(_T("ADPCM N:%X STEP:%02d X:% 4d SIG:% 4d"), nibble, m_step, signal_x, m_signal); 


#else
	OUT_DEBUG_CALC(_T("ADPCM N:%X STEP:%02d SIG:% 4d"), nibble, m_step, m_signal); 

#endif

	return m_signal;
}

/* timer callback at VCLK both edge on ADPCM */
void ADPCM::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_TIMER:
		m_timer_event_id = -1;
		m_vclk ^= 1;
//		m_vclk &= 1;
		// MCK edge changed
		if (m_vclk) {
			write_signals(&outputs_mck, 0);
			OUT_DEBUG_CLKC(_T("ADPCM MCK 0"));
		} else {
			// requset data
			if (m_req_data) {
				// no data or same data was written in register on previous cycle
//				m_signal = 0;
//				m_step = 0;
//				memset(m_prev_signals, 0, sizeof(m_prev_signals));
//				memset(m_prev_samples, 0, sizeof(m_prev_samples));
				if (m_reg_sts & STATUS_PLAYING) {
					calc_samples(0);
					calc_samples(8);
				}
				m_silent_count = 0;
				OUT_DEBUG_REGW(_T("ADPCM DATA NO WROTE"));
			}
			m_req_data = 1;
			write_signals(&outputs_mck, 0xffffffff);
			OUT_DEBUG_CLKC(_T("ADPCM MCK 1"));
		}
		// next event
		register_event(this, EVENT_TIMER, m_period, false, &m_timer_event_id);
		break;
	default:
		break;
	}
}

/*
 *    Handle a change of the selector
 */
void ADPCM::play(bool onoff)
{
	if (m_timer_event_id != -1) {
		cancel_event(this, m_timer_event_id);
		m_timer_event_id = -1;
	}
	if (onoff) {
		/* Also reset the ADPCM parameters */
		m_signal = 0;
		m_step = 0;
		memset(m_prev_signals, 0, sizeof(m_prev_signals));
		memset(m_prev_samples, 0, sizeof(m_prev_samples));
//		m_nibble_shift = 0;
		m_vclk = 1;

		m_time_signal_one = m_period;
		m_time_signal = m_time_signal_one;
		m_time_host_one = 1000000.0 / (double)m_rate_on_host;
		m_time_host = m_time_host_one;
		m_signals_buff_l[0] = 0;
		m_signals_buff_r[0] = 0;
		m_signals_w = 0;

		register_event(this, EVENT_TIMER, 8, false, &m_timer_event_id);
	}
}

void ADPCM::change_clock()
{
	// sampling rate / 2
	m_period = 1000000.0 / m_base_clock / m_clock_mag * c_dividers[m_prescaler];
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

/// @param[in] sample_rate : sampling rate on host machine
/// @param[in] clock : my master clock
/// @param[in] samples : not used
/// @param[in] mode : not used
void ADPCM::initialize_sound(int sample_rate, int clock, int samples, int mode)
{
	m_rate_on_host = sample_rate;
//	m_select = mode;

	m_base_clock = clock;
	change_clock();
}

void ADPCM::mix(int32_t* buffer, int cnt)
{
	int32_t val_l;
	int32_t val_r;

#if (MIX_SAMPLES_FILTER == 3)
	/* low pass filter */
	if(m_signals_w) {
		/* if this voice is active */
		for(int i = 0; i < cnt; i++) {
			double nval_l = 0.0;
			double nval_r = 0.0;

			// convert sample rate for host machine
			int n = 0;
			while(m_time_signal <= m_time_host) {
				nval_l += m_time_signal * m_signals_buff_l[n];
				nval_r += m_time_signal * m_signals_buff_r[n];
				n++;
				m_time_host -= m_time_signal;
				m_time_signal = m_time_signal_one;
			}
			if (n > 0 && m_signals_w > 1) {
				// shift
				for(int i=0; i<(BUFF_SIZE-n); i++) {
					m_signals_buff_l[i] = m_signals_buff_l[i + n];
					m_signals_buff_r[i] = m_signals_buff_r[i + n];
				}
				m_signals_w -= n;
				if (m_signals_w < 0) m_signals_w = 0;
			}

			if (m_time_host < m_time_signal) {
				nval_l += m_time_host * m_signals_buff_l[0];
				nval_r += m_time_host * m_signals_buff_r[0];
				m_time_signal -= m_time_host;
				m_time_host = m_time_host_one;
			}
			// average
			nval_l /= m_time_host_one;
			nval_r /= m_time_host_one;

			m_prev_sample_l = ((int32_t)nval_l * 6 + m_prev_sample_l * 10) / 16;
			m_prev_sample_r = ((int32_t)nval_r * 6 + m_prev_sample_r * 10) / 16;

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	} else {
		for(int i = 0; i < cnt; i++) {
			m_prev_sample_l = m_prev_sample_l * 10 / 16;
			m_prev_sample_r = m_prev_sample_r * 10 / 16;

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	}

#elif (MIX_SAMPLES_FILTER == 2)
	/* no filter */
	if(m_signals_w) {
		/* if this voice is active */
		for(int i = 0; i < cnt; i++) {
			double nval_l = 0.0;
			double nval_r = 0.0;

			// convert sample rate for host machine
			int n = 0;
			while(m_time_signal <= m_time_host) {
				nval_l += m_time_signal * m_signals_buff_l[n];
				nval_r += m_time_signal * m_signals_buff_r[n];
				n++;
				m_time_host -= m_time_signal;
				m_time_signal = m_time_signal_one;
			}
			if (n > 0 && m_signals_w > 1) {
				// shift
				for(int i=0; i<(BUFF_SIZE-n); i++) {
					m_signals_buff_l[i] = m_signals_buff_l[i + n];
					m_signals_buff_r[i] = m_signals_buff_r[i + n];
				}
				m_signals_w -= n;
				if (m_signals_w < 0) m_signals_w = 0;
			}

			if (m_time_host < m_time_signal) {
				nval_l += m_time_host * m_signals_buff_l[0];
				nval_r += m_time_host * m_signals_buff_r[0];
				m_time_signal -= m_time_host;
				m_time_host = m_time_host_one;
			}
			// average
			nval_l /= m_time_host_one;
			nval_r /= m_time_host_one;

			m_prev_sample_l = (int32_t)nval_l;
			m_prev_sample_r = (int32_t)nval_r;

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	} else {
		for(int i = 0; i < cnt; i++) {
			m_prev_sample_l = 0;
			m_prev_sample_r = 0;

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	}

#else
	/* no filter */
	if(m_signals_w) {
		/* if this voice is active */
		for(int i = 0; i < cnt; i++) {
			if (m_signals_w > 1) {
				// shift
				for(int i=0; i<BUFF_SIZE-1; i++) {
					m_signals_buff_l[i] = m_signals_buff_l[i + 1];
					m_signals_buff_r[i] = m_signals_buff_r[i + 1];
				}
				m_signals_w--;
			}

			m_prev_sample_l = m_signals_buff_l[0];
			m_prev_sample_r = m_signals_buff_r[0];

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	} else {
		for(int i = 0; i < cnt; i++) {
			m_prev_sample_l = 0;
			m_prev_sample_r = 0;

			val_l = apply_volume(m_prev_sample_l, volume_l);
			val_r = apply_volume(m_prev_sample_r, volume_r);

			*buffer++ += val_l;	// L
			*buffer++ += val_r; // R
		}
	}

#endif
}

void ADPCM::set_volume(int decibel_l, int decibel_r, bool mute)
{
	if (mute) {
		decibel_l = decibel_r = -192;
	}
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void ADPCM::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	vm_state_ident.version = Uint16_LE(1);

	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	SET_Byte(m_reg_cmd);			///< command register
	SET_Byte(m_reg_sts);			///< status register
	SET_Byte(m_reg_rdata);			///< read data register
	SET_Byte(m_reg_wdata);			///< written data register
	SET_Int32_LE(m_base_clock);		///< clock rate (4MHz)
	SET_Int32_LE(m_clock_mag);		///< current clock magnification 
	SET_Int32_LE(m_timer_event_id);
	// 1
	SET_Double_LE(m_period);		///< event time
	SET_Int32_LE(m_vclk);			///< vclk signal
	SET_Int32_LE(m_req_data);		///< request data 
	// 2
	SET_Int32_LE(m_prescaler);		///< prescaler selector SAM1 and SAM2
	SET_Int32_LE(m_output_bits);	///< D/A precision is 10-bits but 12-bit data can be output serially to an external DAC
	SET_Int32_LE(m_signal);			///< current ADPCM signal
	SET_Int32_LE(m_step);			///< current ADPCM step
	// 3
	SET_Int32_LE(m_signal_max);
	SET_Int32_LE(m_signal_min);
	SET_Int32_LE(m_silent_count);	///< same data continued
	SET_Int32_LE(m_rate_on_host);	///< sampling rate on host machine
	// 4
	SET_Uint32_LE(m_pan_left);
	SET_Uint32_LE(m_pan_right);
	SET_Int32_LE(volume_l);
	SET_Int32_LE(volume_r);
	// 5
	for(int i=0; i<16 && i<BUFF_SIZE; i++) {
		SET_Int32_LE(m_signals_buff_l[i]);	///< current ADPCM signal buffered
		SET_Int32_LE(m_signals_buff_r[i]);	///< current ADPCM signal buffered
	}
	// 13
//	for(int i=0; i<16 && i<MAX_PREV_SIGNALS; i++) {
//		SET_Int32_LE(m_prev_signals[i]);
//	}
	// 21
	SET_Int32_LE(m_signals_w);		///< current ADPCM signal buffered
	SET_Int32_LE(m_prev_sample_l);
	SET_Int32_LE(m_prev_sample_r);
	// 22
	SET_Double_LE(m_time_host);			///< time per one sample
	SET_Double_LE(m_time_host_one);		///< time per one sample
	// 23
	SET_Double_LE(m_time_signal);
	SET_Double_LE(m_time_signal_one);

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

bool ADPCM::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	GET_Byte(m_reg_cmd);			///< command register
	GET_Byte(m_reg_sts);			///< status register
	GET_Byte(m_reg_rdata);			///< read data register
	GET_Byte(m_reg_wdata);			///< written data register
	GET_Int32_LE(m_base_clock);		///< clock rate (4MHz)
	GET_Int32_LE(m_clock_mag);		///< current clock magnification 
	GET_Int32_LE(m_timer_event_id);
	// 1
	GET_Double_LE(m_period);		///< event time
	GET_Int32_LE(m_vclk);			///< vclk signal
	GET_Int32_LE(m_req_data);		///< request data 
	// 2
	GET_Int32_LE(m_prescaler);		///< prescaler selector SAM1 and SAM2
//	GET_Int32_LE(m_output_bits);	///< D/A precision is 10-bits but 12-bit data can be output serially to an external DAC
	GET_Int32_LE(m_signal);			///< current ADPCM signal
	GET_Int32_LE(m_step);			///< current ADPCM step
	// 3
//	GET_Int32_LE(m_signal_max);
//	GET_Int32_LE(m_signal_min);
	GET_Int32_LE(m_silent_count);	///< same data continued
	GET_Int32_LE(m_rate_on_host);	///< sampling rate on host machine
	// 4
	GET_Uint32_LE(m_pan_left);
	GET_Uint32_LE(m_pan_right);
//	GET_Int32_LE(volume_l);
//	GET_Int32_LE(volume_r);
	// 5
	for(int i=0; i<16 && i<BUFF_SIZE; i++) {
		GET_Int32_LE(m_signals_buff_l[i]);	///< current ADPCM signal buffered
		GET_Int32_LE(m_signals_buff_r[i]);	///< current ADPCM signal buffered
	}
	// 13
//	for(int i=0; i<16 && i<MAX_PREV_SIGNALS; i++) {
//		GET_Int32_LE(m_prev_signals[i]);
//	}
	memset(m_prev_signals, 0, sizeof(m_prev_signals));
	memset(m_prev_samples, 0, sizeof(m_prev_samples));
	// 21
	GET_Int32_LE(m_signals_w);		///< current ADPCM signal buffered
	GET_Int32_LE(m_prev_sample_l);
	GET_Int32_LE(m_prev_sample_r);
	// 22
	GET_Double_LE(m_time_host);			///< time per one sample
	GET_Double_LE(m_time_host_one);		///< time per one sample
	// 23
	GET_Double_LE(m_time_signal);
	GET_Double_LE(m_time_signal_one);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t ADPCM::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

bool ADPCM::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool ADPCM::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void ADPCM::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::sntprintf(buffer, buffer_len, _T("CMD:%02X STATUS:%02X  SCLK:%d SAMP:%X(DIV:%.1f)  PAN(L:%d R:%d)\n")
		, m_reg_cmd, m_reg_sts
		, m_base_clock * m_clock_mag
		, m_prescaler, c_dividers[m_prescaler], m_pan_left & 1, m_pan_right & 1);
}
#endif

