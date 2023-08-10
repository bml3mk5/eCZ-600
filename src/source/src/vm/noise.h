/** @file noise.h

	Skelton for retropc emulator

	@author : Sasaji
	@date   : 2022.02.22-

	[ noise player ]
*/

#ifndef NOISE_H
#define NOISE_H

#include "vm_defs.h"
#include "../cchar.h"

//#define USE_NOISE_STEREO
#define USE_NOISE_U8

// ----------------------------------------------------------------------

/**
	@brief NOISE BASE has function to play a PCM data
*/
template<class SAMPLES_T>
class NOISE_BASE
{
protected:
	int m_alloc_size;
	SAMPLES_T *p_buffer_l;
	int m_volume_l;
#ifdef USE_NOISE_STEREO
	SAMPLES_T *p_buffer_r;
	int m_volume_r;
#endif
	int m_rpos;
	int m_samples;
	int m_period;	// the end of position of playing samples
	bool m_loop;
//	bool m_mute;
	bool m_now_playing;

	virtual int32_t conv_sample(uint8_t sam, int vol) {
		return ((int)sam - 128) * vol / 128;
	}
	virtual int32_t conv_sample(int16_t sam, int vol) {
		return (int)sam * vol / 16384;
	}

public:
	struct vm_state_st {
		uint8_t m_revision;
		uint8_t m_loop;
		uint8_t m_now_playing;
		char reserved1;
		int m_rpos;
		int m_period;	// the end of position of playing samples
		int reserved2;
	};

public:
	NOISE_BASE();
	virtual ~NOISE_BASE();

	virtual SAMPLES_T base() const = 0;
	virtual void alloc(int size);
	virtual void clear();

	void play();
	void stop();

	void set_volume(int l, int r);
	void mix(int32_t* buffer, int cnt);
	void save_state(struct vm_state_st &v);
	bool load_state(const struct vm_state_st &v);
	size_t get_state_size() const;
	void set_period(int value) {
		if (0 < m_period && m_period <= m_samples) {
			m_period = value;
		} else {
			m_period = m_samples;
		}
	}
	void set_loop(bool value) {
		m_loop = value;
	}
//	void set_mute(bool value) {
//		m_mute = value;
//	}
	bool is_enable() const {
		return (m_samples > 0);
	}
	bool now_playing() const {
		return m_now_playing;
	}
};

template<class SAMPLES_T>
NOISE_BASE<SAMPLES_T>::NOISE_BASE()
{
	m_alloc_size = 0;
	p_buffer_l = NULL;
	m_volume_l = 0;
#ifdef USE_NOISE_STEREO
	p_buffer_r = NULL;
	m_volume_r = 0;
#endif
	m_rpos = 0;
	m_samples = 0;
	m_period = 0;
	m_loop = false;
//	m_mute = false;
	m_now_playing = false;
}

template<class SAMPLES_T>
NOISE_BASE<SAMPLES_T>::~NOISE_BASE()
{
	delete [] p_buffer_l;
#ifdef USE_NOISE_STEREO
	delete [] p_buffer_r;
#endif
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::alloc(int size)
{
	if (m_alloc_size == 0) {
		p_buffer_l = new SAMPLES_T[size];
#ifdef USE_NOISE_STEREO
		p_buffer_r = new SAMPLES_T[size];
#endif
		m_alloc_size = size;

		for(int i=0; i<m_alloc_size; i++) {
			p_buffer_l[i] = base();
#ifdef USE_NOISE_STEREO
			p_buffer_r[i] = base();
#endif
		}
	}
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::clear()
{
	delete [] p_buffer_l;
	p_buffer_l = NULL;
#ifdef USE_NOISE_STEREO
	delete [] p_buffer_r;
	p_buffer_r = NULL;
#endif
	m_alloc_size = 0;
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::play()
{
	if (m_samples > 0) {
		m_now_playing = true;
		m_rpos = 0;
	}
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::stop()
{
	m_now_playing = false;
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::set_volume(int l, int r)
{
	m_volume_l = l;
#ifdef USE_NOISE_STEREO
	m_volume_r = r;
#endif
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::mix(int32_t* buffer, int cnt)
{
	if (!m_now_playing) {
		return;
	}
	for(int i=0; i<cnt; i++) {
		int32_t sample = conv_sample((SAMPLES_T)p_buffer_l[m_rpos], m_volume_l);
		*buffer += sample;
		buffer++;
#ifdef USE_NOISE_STEREO
		sample = conv_sample((SAMPLES_T)p_buffer_r[m_rpos], m_volume_r);
#endif
		*buffer += sample;
		buffer++;
		m_rpos++;
		if (m_rpos >= m_period) {
			m_rpos = 0;
			if (!m_loop) {
				m_now_playing = false;
				break;
			}
		}
	}
}

template<class SAMPLES_T>
void NOISE_BASE<SAMPLES_T>::save_state(struct vm_state_st &v)
{
	v.m_revision = 1;

	v.m_loop = m_loop ? 1 : 0;
	v.m_now_playing = m_now_playing ? 1 : 0;

	v.m_rpos = Int32_LE(m_rpos);
	v.m_period = Int32_LE(m_period);
}

template<class SAMPLES_T>
bool NOISE_BASE<SAMPLES_T>::load_state(const struct vm_state_st &v)
{
	if (v.m_revision != 1) {
		return false;
	}

	m_loop = (v.m_loop != 0);
	m_now_playing = (v.m_now_playing != 0);

	m_rpos = Int32_LE(v.m_rpos);
	m_period = Int32_LE(v.m_period);

	if (m_period >= m_samples) {
		m_period = m_samples;
	}
	if (m_rpos >= m_period) {
		m_rpos = 0;
		if (!m_loop) {
			m_now_playing = false;
		}
	}
	if (m_samples <= 0) {
		m_now_playing = false;
		m_rpos = 0;
	}

	return true;
}

template<class SAMPLES_T>
size_t NOISE_BASE<SAMPLES_T>::get_state_size() const
{
	return sizeof(struct vm_state_st);
}

// ----------------------------------------------------------------------

/**
	@brief NOISE has function to play a 8bits PCM data
*/
class NOISE : public NOISE_BASE<uint8_t>
{
private:
	CTchar m_file_name;

public:
	NOISE() : NOISE_BASE<uint8_t>() {}
	~NOISE() {}

	uint8_t base() const { return 128; }

	void set_file_name(const _TCHAR *file_name);
	const _TCHAR *get_file_name() const;
	int load_wav_file(const _TCHAR *path, int sample_rate);

};

// ----------------------------------------------------------------------

/**
	@brief NOISE FIFO has function to play a 16bits PCM stream
*/
class NOISE_FIFO : public NOISE_BASE<int16_t>
{
private:
	int m_wpos;

public:
	NOISE_FIFO();
	~NOISE_FIFO() {}

	int16_t base() const { return 0; }
	void alloc(int size);

	void add(int16_t sam);
	void shift();
	void mix(int32_t* buffer, int cnt);
	bool is_full() const {
		return (m_wpos >= m_alloc_size);
	}
};

#endif /* NOISE_H */

