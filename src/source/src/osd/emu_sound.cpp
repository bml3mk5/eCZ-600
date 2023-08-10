/** @file emu_sound.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ emu sound ]
*/

#include "../emu.h"
#include "../vm/vm.h"
#include "../config.h"
#include "../video/rec_audio.h"


void EMU::EMU_SOUND()
{
	sound_rate = 0;
	sound_samples = 0;
	sound_latency_half = 0;
	sound_ok = sound_started = sound_finished = now_mute = false;

	rec_audio = new REC_AUDIO(this);
}

void EMU::initialize_sound(int rate, int samples, int latency)
{
	sound_rate = rate;
	sound_samples = samples;
	sound_latency_half = latency / 2;
	vm->initialize_sound(sound_rate, sound_samples);
}

//void EMU::initialize_sound(int rate, int samples)
//{
//	initialize_sound(rate, samples, 100);
//}

void EMU::initialize_sound()
{
	// load sound config
#ifdef SUPPORT_SOUND_FREQ_55467HZ
	// PC-8801/9801 series
	static int freq_table[8] = {2000, 4000, 8000, 11025, 22050, 44100, 55467, 96000};
#else
	static int freq_table[8] = {2000, 4000, 8000, 11025, 22050, 44100, 48000, 96000};	// Hz
#endif
	static int late_table[6] = {50, 75, 100, 200, 300, 400};	// ms

	if(!(0 <= pConfig->sound_frequency && pConfig->sound_frequency < 8)) {
		pConfig->sound_frequency = 6;	// default: 48KHz
	}
	if(!(0 <= pConfig->sound_latency && pConfig->sound_latency < 6)) {
		pConfig->sound_latency = 1;	// default: 75msec
	}
	int frequency = freq_table[pConfig->sound_frequency];
	int samples = (int)((double)(frequency * late_table[pConfig->sound_latency]) / 1000.0 + 0.5);
	int latency = late_table[pConfig->sound_latency];

	initialize_sound(frequency, samples, latency);
}

void EMU::release_sound()
{
	// stop recording
	stop_rec_sound();

	delete rec_audio;
	rec_audio = NULL;
}

void EMU::release_sound_on_emu_thread()
{
	// stop recording
	stop_rec_sound();
}

void EMU::start_sound()
{
}

void EMU::end_sound()
{
}

void EMU::mute_sound(bool mute)
{
}

void EMU::set_volume(int volume)
{
	// set volume for sound devices.
	if (vm) {
		vm->set_volume();
	}
}

uint32_t EMU::adjust_sound_pos(uint32_t msec)
{
	return 0;
}

void EMU::start_rec_sound(int type)
{
#ifdef USE_REC_AUDIO
	rec_audio->Start(type, sound_rate, false);
#endif
}

void EMU::stop_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Stop();
#endif
}

void EMU::restart_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Restart();
#endif
}

void EMU::record_rec_sound(int32_t *buffer, int samples)
{
#ifdef SOUND_RECORD_IN_MIXER
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
#endif
}

bool EMU::now_rec_sound()
{
	return rec_audio->NowRecording();
}

const _TCHAR **EMU::get_rec_sound_codec_list(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->GetCodecList(type);
#else
	return NULL;
#endif
}

bool *EMU::get_now_rec_sound_ptr()
{
	return rec_audio->GetNowRecordingPtr();
}

bool EMU::rec_sound_enabled(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->IsEnabled(type);
#else
	return false;
#endif
}
