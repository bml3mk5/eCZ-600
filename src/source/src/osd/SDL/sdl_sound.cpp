/** @file sdl_sound.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl sound ]

	@note
	This code is based on win32_sound.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "../../emu_osd.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../video/rec_audio.h"

//#define DSOUND_BUFFER_SIZE (uint32_t)(sound_samples * 8)
//#define DSOUND_BUFFER_HALF (uint32_t)(sound_samples * 4)


void EMU_OSD::EMU_SOUND()
{
	sound_prev_time = 0;
	memset(&audio_desired,0,sizeof(audio_desired));
#if defined(USE_SDL2) || defined(USE_WX2)
	audio_devid = 0;
#endif
}

void EMU_OSD::initialize_sound(int rate, int samples, int latency)
{
	EMU::initialize_sound(rate, samples, latency);

#if defined(USE_SDL2) || defined(USE_WX2)
#ifdef _DEBUG
	const int drivers = SDL_GetNumAudioDrivers();
	for (int i=0; i < drivers; ++i) {
		logging->out_debugf("audio_driver %d: %s",i, SDL_GetAudioDriver(i));
	}
	logging->out_debugf("Using audio driver: %s", SDL_GetCurrentAudioDriver());
	const int devices = SDL_GetNumAudioDevices(0);
	for(int i=0; i <devices; i++) {
		logging->out_debugf("audio_device %d %s",i, SDL_GetAudioDeviceName(i, 0));
	}
#endif
#endif

	memset(&audio_desired,0,sizeof(audio_desired));
	audio_desired.freq = rate;
#ifdef USE_AUDIO_U8
	audio_desired.format = AUDIO_U8;
#else
	audio_desired.format = AUDIO_S16;
#endif
	audio_desired.channels = 2;
	audio_desired.samples = (samples >> 1);
	audio_desired.callback = &update_sound;
	audio_desired.userdata = (void *)this;

#if defined(USE_SDL2) || defined(USE_WX2)
	audio_devid = SDL_OpenAudioDevice(NULL, 0, &audio_desired, &audio_obtained, 0);
	if (!audio_devid) {
		logging->out_logf(LOG_ERROR,"SDL_OpenAudioDevice: %s",SDL_GetError());
	}
#else
	if (SDL_OpenAudio(&audio_desired,&audio_obtained) != 0) {
		logging->out_logf(LOG_ERROR,"SDL_OpenAudio: %s",SDL_GetError());
	}
#endif
#ifdef _DEBUG_LOG
	logging->out_debugf(_T("audio_obtained.freq:%d"),audio_obtained.freq);
	logging->out_debugf(_T("audio_obtained.format:%04x"),audio_obtained.format);
	logging->out_debugf(_T("audio_obtained.channels:%d"),audio_obtained.channels);
	logging->out_debugf(_T("audio_obtained.samples:%d"),audio_obtained.samples);
	logging->out_debugf(_T("audio_obtained.silence:%d"),audio_obtained.silence);
	logging->out_debugf(_T("audio_obtained.size:%d"),audio_obtained.size);
#endif

	logging->out_logf(LOG_DEBUG, _T("sound ok: rate:%d samples:%d latency:%d"), rate, samples, latency);
	sound_ok = true;

	sound_prev_time = SDL_GetTicks();
}

void EMU_OSD::initialize_sound()
{
	EMU::initialize_sound();
}

void EMU_OSD::release_sound()
{
#if defined(USE_SDL2) || defined(USE_WX2)
	if (audio_devid) {
		SDL_CloseAudioDevice(audio_devid);
	}
#else
	SDL_CloseAudio();
#endif
	delete rec_audio;
	sound_ok = false;
}

void EMU_OSD::release_sound_on_emu_thread()
{
	end_sound();
}

void EMU_OSD::start_sound()
{
#if defined(USE_SDL2) || defined(USE_WX2)
	if (audio_devid) {
		SDL_PauseAudioDevice(audio_devid, 0);
	}
#else
	SDL_PauseAudio(0);
#endif
	sound_ok = true;
}

void EMU_OSD::end_sound()
{
#if defined(USE_SDL2) || defined(USE_WX2)
	if (audio_devid) {
		SDL_PauseAudioDevice(audio_devid, 1);
	}
#else
	SDL_PauseAudio(1);
#endif
	sound_ok = false;

	// stop recording
	stop_rec_sound();
}

/// @attention this function is called by another thread.
/// event callback so static function
void EMU_OSD::update_sound(void *userdata, Uint8 *stream, int len)
{
	EMU_OSD *emu = (EMU_OSD *)userdata;

//	logging->out_debugf("EMU_OSD::update_sound start: len:%d stream:%llx",len,stream);

//	if (emu->sound_prev_time + emu->sound_latency > sound_now_time) {
//		return;
//	}

	int extra_frames = 0;

	if(emu->sound_ok) {
		// sound buffer must be updated
		audio_sample_t* sound_buffer = emu->vm->create_sound(&extra_frames, emu->audio_obtained.samples);
#ifndef SOUND_RECORD_IN_MIXER
		record_sound(sound_buffer, sound_samples);
#endif
		if(sound_buffer) {
#if defined(USE_SDL2) || defined(USE_WX2)
			// TODO: should be SDL_MixAudioFormat
			SDL_memcpy(stream, (Uint8 *)sound_buffer, len);
#else
			SDL_MixAudio(stream, (Uint8 *)sound_buffer, len, SDL_MIX_MAXVOLUME);
#endif
		}
	}

//	logging->out_debug("EMU_OSD::update_sound end");
}

void EMU_OSD::mute_sound(bool mute)
{
	if (mute) {
// sound is continuous driving during mute
//		SDL_PauseAudio(1);
		now_mute = true;
	} else {
		if (sound_ok && !vm_pause){
//			SDL_PauseAudio(0);
			now_mute = false;
		}
	}
}

#if 0
void EMU_OSD::set_volume(int volume)
{
	// set volume for sound devices.
	if (vm) {
		vm->set_volume();
	}
}

void EMU_OSD::start_rec_sound(int type)
{
#ifdef USE_REC_AUDIO
	rec_audio->Start(type, sound_rate, false);
#endif
}

void EMU_OSD::stop_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Stop();
#endif
}

void EMU_OSD::restart_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Restart();
#endif
}
#endif

#if 0
void EMU_OSD::record_rec_sound(uint8_t *buffer, int samples)
{
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
}

void EMU_OSD::record_rec_sound(int16_t *buffer, int samples)
{
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
}
#endif

#if 0
void EMU_OSD::record_rec_sound(int32_t *buffer, int samples)
{
#ifdef SOUND_RECORD_IN_MIXER
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
#endif
}

bool EMU_OSD::now_rec_sound()
{
	return rec_audio->NowRecording();
}

const _TCHAR **EMU_OSD::get_rec_sound_codec_list(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->GetCodecList(type);
#else
	return NULL;
#endif
}

bool *EMU_OSD::get_now_rec_sound_ptr()
{
	return rec_audio->GetNowRecordingPtr();
}

bool EMU_OSD::rec_sound_enabled(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->IsEnabled(type);
#else
	return false;
#endif
}
#endif

void EMU_OSD::lock_sound_buffer()
{
#if defined(USE_SDL2) || defined(USE_WX2)
	if (audio_devid) SDL_LockAudioDevice(audio_devid);
#else
	SDL_LockAudio();
#endif
}
void EMU_OSD::unlock_sound_buffer()
{
#if defined(USE_SDL2) || defined(USE_WX2)
	if (audio_devid) SDL_UnlockAudioDevice(audio_devid);
#else
	SDL_UnlockAudio();
#endif
}
