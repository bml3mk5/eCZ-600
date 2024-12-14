/** @file rec_audio.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio ]
*/

#include "rec_audio.h"
#include "rec_common.h"
#include "../emu.h"
#include "../config.h"
#include "../utility.h"

#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
#include "wave/wav_rec_audio.h"
#endif
#ifdef USE_REC_AUDIO_MMF
#include "mmf/mmf_rec_audio.h"
#endif
#ifdef USE_REC_AUDIO_AVKIT
#include "avkit/avk_rec_audio.h"
#endif
#ifdef USE_REC_AUDIO_FFMPEG
#include "ffmpeg/ffm_rec_audio.h"
#endif
#endif /* USE_REC_AUDIO */

REC_AUDIO::REC_AUDIO(EMU *new_emu)
{
	emu = new_emu;
	now_recording = false;
	rec_type = 0;
	memset(rec_path, 0, sizeof(rec_path));

#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_WAVE
	wavaudio = new WAV_REC_AUDIO(new_emu, this);
#endif
#ifdef USE_REC_AUDIO_MMF
	mmfaudio = new MMF_REC_AUDIO(new_emu, this);
#endif
#ifdef USE_REC_AUDIO_AVKIT
	avkaudio = new AVK_REC_AUDIO(new_emu, this);
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	ffmaudio = new FFM_REC_AUDIO(new_emu, this);
#endif
#endif /* USE_REC_AUDIO */
}

REC_AUDIO::~REC_AUDIO()
{
#ifdef USE_REC_AUDIO
#ifdef USE_REC_AUDIO_FFMPEG
	delete ffmaudio;
#endif
#ifdef USE_REC_AUDIO_MMF
	delete mmfaudio;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	delete avkaudio;
#endif
#ifdef USE_REC_AUDIO_WAVE
	delete wavaudio;
#endif
#endif /* USE_REC_AUDIO */
}

bool REC_AUDIO::IsEnabled(int type)
{
	bool rc = false;
#ifdef USE_REC_AUDIO
	switch(type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		rc = wavaudio->IsEnabled();
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		rc = mmfaudio->IsEnabled();
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		rc = avkaudio->IsEnabled();
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		rc = ffmaudio->IsEnabled();
		break;
#endif
	default:
		break;
	}
#endif
	return rc;
}

bool REC_AUDIO::Start(int type, int sample_rate, bool with_video)
{
#ifdef USE_REC_AUDIO
	if (type <= 0 || sample_rate <= 0) {
		return false;
	}

	rec_type = type;

	gRecCommon.CreateFileName(!with_video, rec_path, sizeof(rec_path) / sizeof(rec_path[0]), NULL);

	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		now_recording = wavaudio->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), sample_rate);
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		now_recording = mmfaudio->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), sample_rate);
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		now_recording = avkaudio->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), sample_rate);
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		now_recording = ffmaudio->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), sample_rate);
		break;
#endif
	}
#endif
	return now_recording;
}

void REC_AUDIO::Stop()
{
	if (!now_recording) return;
#ifdef USE_REC_AUDIO
	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		wavaudio->Stop();
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		mmfaudio->Stop();
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		avkaudio->Stop();
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		ffmaudio->Stop();
		break;
#endif
	}
#endif
	now_recording = false;
}

bool REC_AUDIO::Restart()
{
	if (!now_recording) return false;
#ifdef USE_REC_AUDIO
	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		now_recording = wavaudio->Restart();
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		now_recording = mmfaudio->Restart();
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		now_recording = avkaudio->Restart();
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		now_recording = ffmaudio->Restart();
		break;
#endif
	}
#endif
	return now_recording;
}

#if 0
bool REC_AUDIO::Record(uint8_t *buffer, int samples)
{
	if (!now_recording) return false;

#ifdef USE_REC_AUDIO
	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		now_recording = wavaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		now_recording = mmfaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		now_recording = avkaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		now_recording = ffmaudio->Record(buffer, samples);
		break;
#endif
	}
#endif
	return now_recording;
}

bool REC_AUDIO::Record(int16_t *buffer, int samples)
{
	if (!now_recording) return false;

#ifdef USE_REC_AUDIO
	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		now_recording = wavaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		now_recording = mmfaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		now_recording = avkaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		now_recording = ffmaudio->Record(buffer, samples);
		break;
#endif
	}
#endif
	return now_recording;
}
#endif

bool REC_AUDIO::Record(int32_t *buffer, int samples)
{
	if (!now_recording) return false;

#ifdef USE_REC_AUDIO
	switch(rec_type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		now_recording = wavaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		now_recording = mmfaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		now_recording = avkaudio->Record(buffer, samples);
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		now_recording = ffmaudio->Record(buffer, samples);
		break;
#endif
	}
#endif
	return now_recording;
}

const _TCHAR **REC_AUDIO::GetCodecList(int type)
{
	const _TCHAR **list = NULL;
#ifdef USE_REC_AUDIO
	switch(type) {
#ifdef USE_REC_AUDIO_WAVE
	case RECORD_AUDIO_TYPE_WAVE:
		list = wavaudio->GetCodecList();
		break;
#endif
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		list = mmfaudio->GetCodecList();
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		list = avkaudio->GetCodecList();
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		list = ffmaudio->GetCodecList();
		break;
#endif
	default:
		break;
	}
#endif
	return list;
}

#if 0
const _TCHAR **REC_AUDIO::GetQualityList(int type)
{
	const _TCHAR **list = NULL;
#ifdef USE_REC_AUDIO
	switch(type) {
#ifdef USE_REC_AUDIO_MMF
	case RECORD_AUDIO_TYPE_MMF:
		list = mmfaudio->GetQualityList();
		break;
#endif
#ifdef USE_REC_AUDIO_AVKIT
	case RECORD_AUDIO_TYPE_AVKIT:
		list = avkaudio->GetQualityList();
		break;
#endif
#ifdef USE_REC_AUDIO_FFMPEG
	case RECORD_AUDIO_TYPE_FFMPEG:
		list = ffmaudio->GetQualityList();
		break;
#endif
	default:
		break;
	}
#endif
	return list;
}
#endif
