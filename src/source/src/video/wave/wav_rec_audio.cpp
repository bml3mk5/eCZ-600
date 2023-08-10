/** @file wav_rec_audio.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.11.03 -

	@brief [ record audio using wave format ]
*/

#include "../../rec_video_defs.h"

#if defined(USE_REC_AUDIO) && defined(USE_REC_AUDIO_WAVE)

#include "wav_rec_audio.h"
#include "../rec_audio.h"
#include "../../emu.h"
#include "../../fileio.h"
#include "../../config.h"
#include "../../utility.h"

WAV_REC_AUDIO::WAV_REC_AUDIO(EMU *new_emu, REC_AUDIO *new_audio)
{
	emu = new_emu;
	audio = new_audio;
	rec_rate = 0;
	rec_fp = NULL;
	rec_path = NULL;

}

WAV_REC_AUDIO::~WAV_REC_AUDIO()
{
	Release();
}

void WAV_REC_AUDIO::Release()
{
	delete rec_fp;
	rec_fp = NULL;
}

void WAV_REC_AUDIO::RemoveFile()
{
	FILEIO::RemoveFile(rec_path);
}

bool WAV_REC_AUDIO::IsEnabled()
{
	return true;
}

bool WAV_REC_AUDIO::Start(_TCHAR *path, size_t path_size, int sample_rate)
{
	if (path != NULL) {
		UTILITY::tcscat(path, path_size, _T(".wav"));
		rec_path = path;
	}
	rec_rate = sample_rate;

	rec_fp = new FILEIO();
	if(!rec_fp->Fopen(rec_path, FILEIO::WRITE_BINARY)) {
		// failed to open the wave file
		logging->out_logf(LOG_ERROR,_T("Failed to open %s."), rec_path);
		goto FIN;
	}

	// write dummy wave header
	wavheader_t header;

	memset(&header, 0, sizeof(wavheader_t));
	rec_fp->Fwrite(&header, sizeof(wavheader_t), 1);
	rec_bytes = 0;

	return true;

FIN:
	Release();
	RemoveFile();
	logging->out_log_x(LOG_ERROR, CMsg::Couldn_t_start_recording_audio);
	return false;
}

void WAV_REC_AUDIO::Stop()
{
	// update wave header
	wavheader_t header;

	header.dwRIFF = 0x46464952;
	header.dwFileSize = rec_bytes + sizeof(wavheader_t) - 8;
	header.dwWAVE = 0x45564157;
	header.dwfmt_ = 0x20746d66;
	header.dwFormatSize = 16;
	header.wFormatTag = 1;
	header.wChannels = 2;
#ifdef USE_AUDIO_U8
	header.wBitsPerSample = 8;
#else
	header.wBitsPerSample = 16;
#endif
	header.dwSamplesPerSec = rec_rate;
	header.wBlockAlign = header.wChannels * header.wBitsPerSample / 8;
	header.dwAvgBytesPerSec = header.dwSamplesPerSec * header.wBlockAlign;
	header.dwdata = 0x61746164;
	header.dwDataLength = rec_bytes;

	if (rec_fp) {
		rec_fp->Fseek(0, FILEIO::SEEKSET);
		rec_fp->Fwrite(&header, sizeof(wavheader_t), 1);
		rec_fp->Fclose();
	}

	Release();
}

bool WAV_REC_AUDIO::Restart()
{
	audio->Stop();
	// create new file
	return audio->Start(RECORD_AUDIO_TYPE_WAVE, rec_rate, false);
}

#if 0
bool WAV_REC_AUDIO::Record(uint8_t *buffer, int samples)
{
	// record sound
	int length = sizeof(uint8_t) * samples * 2;
	rec_fp->Fwrite(buffer, length, 1);
	rec_bytes += length;
	return true;
}

bool WAV_REC_AUDIO::Record(int16_t *buffer, int samples)
{
	// record sound
	int length = sizeof(int16_t) * samples * 2;
	rec_fp->Fwrite(buffer, length, 1);
	rec_bytes += length;
	return true;
}
#endif

bool WAV_REC_AUDIO::Record(int32_t *buffer, int samples)
{
	// record sound
	int length = sizeof(audio_sample_t) * samples * 2;
	audio_sample_t dat;
	for(int i=0; i<(samples << 1); i++) {
		dat = (audio_sample_t)buffer[i];
		rec_fp->Fwrite(&dat, sizeof(audio_sample_t), 1);
	}
	rec_bytes += length;
	return true;
}

const _TCHAR **WAV_REC_AUDIO::GetCodecList()
{
	return NULL;
}

#endif /* USE_REC_AUDIO && USE_REC_AUDIO_WAVE */
