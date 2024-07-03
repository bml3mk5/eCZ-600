/** @file sound_base.h

	Skelton for retropc emulator

	@author : Sasaji
	@date   : 2022.02.22-

	@brief [ sound base ]
*/

#ifndef SOUND_BASE_H
#define SOUND_BASE_H

#include "vm_defs.h"
#include "device.h"

// ----------------------------------------------------------------------

class NOISE;

/**
	@brief Sound Base provides various functions for sound device
*/
class SOUND_BASE : public DEVICE
{
protected:
	int m_sample_rate;

	void load_wav_files(NOISE *noises, int noise_nums, bool is_first_time);

	int decibel_to_volume(int decibel);
	int32_t apply_volume(int32_t sample, int volume);

public:
	SOUND_BASE(VM* parent_vm, EMU* parent_emu, const char* identifier);
	virtual ~SOUND_BASE() {}

	virtual void initialize_sound(int sample_rate, int clock, int samples, int mode);
	virtual void initialize_sound(int sample_rate, int decibel_l, int decibel_r);
	virtual void initialize_sound(int sample_rate, int decibel);
	virtual void set_volume(int decibel, bool vol_mute) {}
	virtual void set_volume(int decibel_l, int decibel_r, bool vol_mute) {}
};

#endif /* SOUND_BASE_H */

