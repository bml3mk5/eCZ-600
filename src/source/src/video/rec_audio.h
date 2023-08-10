/** @file rec_audio.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio ]
*/

#ifndef RECORD_AUDIO_H
#define RECORD_AUDIO_H

#include "../rec_video_defs.h"
#include "../common.h"

#ifdef USE_REC_AUDIO_WAVE
class WAV_REC_AUDIO;
#endif
#ifdef USE_REC_AUDIO_MMF
class MMF_REC_AUDIO;
#endif
#ifdef USE_REC_AUDIO_AVKIT
class AVK_REC_AUDIO;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
class FFM_REC_AUDIO;
#endif

enum en_record_audio_type {
	RECORD_AUDIO_TYPE_WAVE = 1,
	RECORD_AUDIO_TYPE_MMF,
	RECORD_AUDIO_TYPE_FFMPEG,
	RECORD_AUDIO_TYPE_AVKIT,
	RECORD_AUDIO_TYPE_UNKNOWN = -1
};

#define SOUND_RECORD_IN_MIXER

class EMU;

/**
	@brief Record audio
*/
class REC_AUDIO
{
private:
	EMU *emu;
	bool now_recording;
	int rec_type;
	_TCHAR rec_path[_MAX_PATH];

#ifdef USE_REC_AUDIO_WAVE
	WAV_REC_AUDIO *wavaudio;
#endif
#ifdef USE_REC_AUDIO_MMF
	MMF_REC_AUDIO *mmfaudio;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	AVK_REC_AUDIO *avkaudio;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	FFM_REC_AUDIO *ffmaudio;
#endif

public:
	REC_AUDIO(EMU *new_emu);
	~REC_AUDIO();

	/// true:OK false:ERROR
	bool Start(int type, int sample_rate, bool show_dialog);
	void Stop();
	bool Restart();
//	bool Record(uint8_t *buffer, int samples);
//	bool Record(int16_t *buffer, int samples);
	bool Record(int32_t *buffer, int samples);
	bool NowRecording() {
		return now_recording;
	}
	bool *GetNowRecordingPtr() {
		return &now_recording;
	}
	void CreateFileName(_TCHAR *file_path, const char *extension);

	bool IsEnabled(int type);
	const _TCHAR **GetCodecList(int type);

};

#endif /* RECORD_AUDIO_H */
