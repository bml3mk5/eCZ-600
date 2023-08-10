/** @file wav_rec_audio.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio using wave format ]
*/

#ifndef WAV_RECORD_AUDIO_H
#define WAV_RECORD_AUDIO_H

#include "../../rec_video_defs.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_WAVE)

#include "../../common.h"

class EMU;
class REC_AUDIO;
class FILEIO;

/**
	@brief Record audio using wave format
*/
class WAV_REC_AUDIO
{
private:
	EMU *emu;
	REC_AUDIO *audio;
	const _TCHAR *rec_path;
	int rec_rate;
	int rec_bytes;
	FILEIO *rec_fp;

	void Release();
	void RemoveFile();

	// record sound
	typedef struct {
		uint32_t dwRIFF;
		uint32_t dwFileSize;
		uint32_t dwWAVE;
		uint32_t dwfmt_;
		uint32_t dwFormatSize;
		uint16_t wFormatTag;
		uint16_t wChannels;
		uint32_t dwSamplesPerSec;
		uint32_t dwAvgBytesPerSec;
		uint16_t wBlockAlign;
		uint16_t wBitsPerSample;
		uint32_t dwdata;
		uint32_t dwDataLength;
	} wavheader_t;

public:
	WAV_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio);
	~WAV_REC_AUDIO();

	bool IsEnabled();

	/// true:OK false:ERROR
	bool Start(_TCHAR *path, size_t path_size, int sample_rate);
	void Stop();
	bool Restart();
//	bool Record(uint8_t *buffer, int samples);
//	bool Record(int16_t *buffer, int samples);
	bool Record(int32_t *buffer, int samples);

	const _TCHAR **GetCodecList();
};

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_WAVE */

#endif /* WAV_RECORD_AUDIO_H */
