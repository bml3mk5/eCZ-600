/** @file noise.cpp

	Skelton for retropc emulator

	@author : Sasaji
	@date   : 2022.02.22-

	[ noise player ]
*/

#include "noise.h"
#include "../fileio.h"
#include "../utility.h"
#include "parsewav.h"

// ----------------------------------------------------------------------

void NOISE::set_file_name(const _TCHAR *file_name)
{
	m_file_name.Set(file_name);
}

const _TCHAR *NOISE::get_file_name() const
{
	return m_file_name.Get();
}

/// @return <0:already loaded / >0:loaded successfully / 0:failed loading
int NOISE::load_wav_file(const _TCHAR *path, int sample_rate)
{
	_TCHAR			 file_path[_MAX_PATH];
	wav_header_t	 head;
	wav_fmt_chank_t  fmt;
	wav_data_chank_t data;
	size_t			 data_len;
	PARSEWAV::Util	 wavu;
	FILEIO			 fio;

	if(m_samples > 0) {
		// already loaded
		return -1;
	}

	UTILITY::stprintf(file_path, _MAX_PATH, _T("%s%s"), path, m_file_name.Get());
	if(fio.Fopen(file_path, FILEIO::READ_BINARY)) {
		if (wavu.CheckWavFormat(fio, &head, &fmt, &data, &data_len) >= 0) {
			m_samples = (int)wavu.ReadWavData(fio, &fmt, data_len, p_buffer_l, sample_rate, sizeof(*p_buffer_l) * 8, m_alloc_size);
#ifdef USE_NOISE_STEREO
			memcpy(p_buffer_r, p_buffer_l, sizeof(*p_buffer_l) * m_alloc_size);
#endif
			m_period = m_samples;
		}
		fio.Fclose();
	}
	return m_samples;
}

// ----------------------------------------------------------------------

NOISE_FIFO::NOISE_FIFO() : NOISE_BASE<int16_t>()
{
	m_wpos = 0;
	m_loop = true;
	m_now_playing = true;
}

void NOISE_FIFO::alloc(int size)
{
	NOISE_BASE<int16_t>::alloc(size);
	m_samples = m_alloc_size;
	m_period = m_samples;
}

void NOISE_FIFO::add(int16_t sam)
{
	if (m_wpos < m_alloc_size) {
		p_buffer_l[m_wpos] = sam;
#ifdef USE_NOISE_STEREO
		p_buffer_r[m_wpos] = sam;
#endif
		m_wpos++;
	}
}

void NOISE_FIFO::mix(int32_t* buffer, int cnt)
{
	if (m_wpos > m_rpos) {
		NOISE_BASE<int16_t>::mix(buffer, cnt);
		// shift
		shift();
	}
}

void NOISE_FIFO::shift()
{
	if (m_wpos > (int)(m_alloc_size >> 1)) {
		for(int pos=m_rpos; pos<m_wpos; pos++) {
			p_buffer_l[pos-m_rpos] = p_buffer_l[pos];
#ifdef USE_NOISE_STEREO
			p_buffer_r[pos-m_rpos] = p_buffer_r[pos];
#endif
		}
		m_wpos -= m_rpos;
		m_rpos = 0;
	}
}
