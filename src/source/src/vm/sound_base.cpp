/** @file sound_base.cpp

	Skelton for retropc emulator

	@author : Sasaji
	@date   : 2022.02.22-

	[ sound base ]
*/

#include "sound_base.h"
#include <math.h>
#include "noise.h"
#include "vm.h"
#include "../config.h"
#include "../fileio.h"
#include "../logging.h"
//#include "../utility.h"
//#include "parsewav.h"

// ----------------------------------------------------------------------

SOUND_BASE::SOUND_BASE(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	m_sample_rate = 48000;
}

// ----------------------------------------------------------------------

void SOUND_BASE::initialize_sound(int sample_rate, int clock, int samples, int mode)
{
	m_sample_rate = sample_rate;
}

void SOUND_BASE::initialize_sound(int sample_rate, int decibel_l, int decibel_r)
{
	m_sample_rate = sample_rate;
	set_volume(decibel_l, decibel_r, false);
}

void SOUND_BASE::initialize_sound(int sample_rate, int decibel)
{
	m_sample_rate = sample_rate;
	set_volume(decibel, false);
}

// ----------------------------------------------------------------------

int SOUND_BASE::decibel_to_volume(int decibel)
{
	// +1 equals +0.5dB (same as fmgen)
//	return (int)(1024.0 * pow(10.0, decibel / 40.0) + 0.5);
	return (int)(16384.0 * pow(10.0, decibel / 40.0) + 0.5);
}

int32_t SOUND_BASE::apply_volume(int32_t sample, int volume)
{
	return sample * volume / 16384;
}

// ----------------------------------------------------------------------

void SOUND_BASE::load_wav_files(NOISE *noises, int noise_nums, bool is_first_time)
{
	// load wav file
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path;
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		for (int ty=0; ty<noise_nums; ty++) {
			if (noises[ty].load_wav_file(app_path, m_sample_rate) > 0) {
				logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, noises[ty].get_file_name());
			}
		}
	}

	for (int ty=0; ty<noise_nums; ty++) {
		if (!noises[ty].is_enable()) {
			noises[ty].clear();
			if (!is_first_time) {
				logging->out_debugf(_T("%s couldn't be loaded."), noises[ty].get_file_name());
			}
		}
	}
}
