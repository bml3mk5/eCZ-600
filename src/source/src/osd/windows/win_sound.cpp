/** @file win_sound.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@brief [ win32 sound ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../cmutex.h"
#include "../../video/rec_audio.h"

#define USE_SOUND_NOTIFY

#define DSOUND_SAMPLE (sound_samples)
#define DSOUND_BUFFER_SIZE (DWORD)(DSOUND_SAMPLE << 3)
#define DSOUND_BUFFER_HALF (DWORD)(DSOUND_SAMPLE << 2)


#ifdef USE_AUDIO_U8
#define BITS_PER_SAMPLE	8
#else
#define BITS_PER_SAMPLE	16
#endif

void EMU_OSD::EMU_SOUND()
{
	sound_prev_play_c = ~0;
	sound_nt_event = NULL;
	sound_threadid = 0;
	sound_thread = NULL;
	lpdsnt = NULL;
	lpds = NULL;
	lpdsp = NULL;
	lpdsb = NULL;

	mux_sound_nt = new CMutex();
}

void EMU_OSD::initialize_sound(int rate, int samples, int latency)
{
	EMU::initialize_sound(rate, samples, latency);

	// initialize direct sound
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfex;

	if(FAILED(DirectSoundCreate(NULL, &lpds, NULL))) {
		return;
	}
	if(FAILED(lpds->SetCooperativeLevel(hMainWindow, DSSCL_PRIORITY))) {
		return;
	}

	// primary buffer
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsp, NULL))) {
		return;
	}
	ZeroMemory(&wfex, sizeof(wfex));
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 2;
	wfex.wBitsPerSample = BITS_PER_SAMPLE;
	wfex.nSamplesPerSec = sound_rate;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	if(FAILED(lpdsp->SetFormat(&wfex))) {
		return;
	}

	// secondary buffer
	ZeroMemory(&pcmwf, sizeof(pcmwf));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wBitsPerSample = BITS_PER_SAMPLE;
	pcmwf.wf.nSamplesPerSec = sound_rate;
	pcmwf.wf.nBlockAlign = pcmwf.wf.nChannels * pcmwf.wBitsPerSample / 8;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_STICKYFOCUS | DSBCAPS_GETCURRENTPOSITION2;
#ifdef USE_SOUND_NOTIFY
	dsbd.dwFlags |= DSBCAPS_CTRLPOSITIONNOTIFY;
#endif
	dsbd.dwBufferBytes = DSOUND_BUFFER_SIZE;
	dsbd.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	if(FAILED(lpds->CreateSoundBuffer(&dsbd, &lpdsb, NULL))) {
		return;
	}

#ifdef USE_SOUND_NOTIFY
	// create sound notify evnet
	if (FAILED(lpdsb->QueryInterface(IID_IDirectSoundNotify, (VOID**)&lpdsnt))) {
		logging->out_logf(LOG_ERROR,_T("EMU::initialize_sound: QueryInterface %ld"), GetLastError());
		return;
	}
	if ((sound_nt_event = CreateEvent(NULL, FALSE, FALSE, _T("emu_sound_notify"))) == NULL) {
		logging->out_logf(LOG_ERROR,_T("EMU::initialize_sound: CreateEvent %ld"), GetLastError());
		return;
	}

	// set fire point on sound buffer
	DSBPOSITIONNOTIFY dsnt[2];

	dsnt[0].dwOffset = 0;
	dsnt[0].hEventNotify = sound_nt_event;
	dsnt[1].dwOffset = DSOUND_BUFFER_HALF;
	dsnt[1].hEventNotify = sound_nt_event;

	lpdsnt->SetNotificationPositions(2, dsnt);
#endif

	// start new thread
	if ((sound_thread = CreateThread(NULL, 0, sound_proc, (LPVOID)this, 0, &sound_threadid)) == NULL) {
		logging->out_logf(LOG_ERROR,_T("EMU::initialize_sound: CreateThread %ld"), GetLastError());
		return;
	}

	logging->out_logf(LOG_DEBUG, _T("sound ok: rate:%d samples:%d latency:%d"), rate, samples, latency);
	sound_ok = true;
}

//void EMU_OSD::initialize_sound(int rate, int samples)
//{
//	initialize_sound(rate, samples, 100);
//}

void EMU_OSD::initialize_sound()
{
	EMU::initialize_sound();
}

void EMU_OSD::release_sound()
{
	// release direct sound
	if(lpdsnt) {
		lpdsnt->Release();
	}
	if(lpdsp) {
		lpdsp->Release();
	}
	if(lpdsb) {
		lpdsb->Release();
	}
	if(lpds) {
		lpds->Release();
	}
	lpdsnt = NULL;
	lpdsp = NULL;
	lpdsb = NULL;
	lpds = NULL;

	EMU::release_sound();

	delete mux_sound_nt;
}

void EMU_OSD::start_sound()
{
	if(sound_ok) {
		// start play
		if(!sound_started) {
			lpdsb->Play(0, 0, DSBPLAY_LOOPING);
			sound_started = true;
		}
	}
}

void EMU_OSD::end_sound()
{
	if (sound_started) {
		sound_finished = true;
		// stop sound
		lpdsb->Stop();

#ifdef USE_SOUND_NOTIFY
		// send event
		SetEvent(sound_nt_event);
#endif
	}
	if (sound_ok) {
		sound_ok = false;

		// wait thread
		if (WaitForSingleObject(sound_thread, 10000) == WAIT_TIMEOUT) {
			// kill thread by force
			if (sound_thread) {
				TerminateThread(sound_thread, 0);
			}
		}

#ifdef USE_SOUND_NOTIFY
		// close event
		if (sound_nt_event) {
			CloseHandle(sound_nt_event);
		}
#endif
	}
}

/// @attention called by another thread
DWORD EMU_OSD::update_sound_th()
{
//	out_debug(_T("EMU::update_sound_nt %ld"),timeGetTime());
	DWORD play_c, write_c, offset, size1, size2, next_wait, next_wait_ms;
	WORD *ptr1, *ptr2;

	// need finish?
	if (sound_finished) {
		return (DWORD)-1;
	}

	// check current playing position
	if(FAILED(lpdsb->GetCurrentPosition(&play_c, &write_c))) {
		logging->out_log(LOG_ERROR,_T("EMU::update_sound_th GetCurrentPosition failed."));
		return (DWORD)-1;
	}

	if(play_c < DSOUND_BUFFER_HALF) {
		offset = DSOUND_BUFFER_HALF;
		next_wait = DSOUND_BUFFER_HALF + (DSOUND_BUFFER_HALF / 4) - play_c;
	} else {
		offset = 0;
		next_wait = DSOUND_BUFFER_SIZE + (DSOUND_BUFFER_HALF / 4) - play_c;
	}

#ifndef USE_SOUND_NOTIFY
	if (sound_prev_play_c == play_c) {
		// The play cursor is not updated, so query it again after 8 ms.
		next_wait = sound_rate / 31;
		next_wait_ms = 8;

	} else
#endif
	{
		// ms
		next_wait_ms = (next_wait * 1000 / sound_rate / 4);

		// sound buffer must be updated
		int extra_frames = 0;

		lock_sound_buffer();
		int16_t* sound_buffer = vm->create_sound(&extra_frames, DSOUND_SAMPLE);
		unlock_sound_buffer();

		if (lpdsb->Lock(offset, DSOUND_BUFFER_HALF, (void **)&ptr1, &size1, (void**)&ptr2, &size2, 0) == DSERR_BUFFERLOST) {
			//		logging->out_log(LOG_ERROR,_T("EMU::update_sound DSERR_BUFFERLOST"));
			lpdsb->Restore();
		}
		if (sound_buffer) {
			if (ptr1) {
				CopyMemory(ptr1, sound_buffer, size1);
			}
			if (ptr2) {
				CopyMemory(ptr2, sound_buffer + size1, size2);
			}
		}
		lpdsb->Unlock(ptr1, size1, ptr2, size2);
	}

//	logging->out_logf(LOG_DEBUG,_T("EMU::update_sound off:%5d prev:%5d playc:%5d writec:%5d size1:%5d size2:%5d next:%ld %dms")
//		,offset,sound_prev_play_c,play_c,write_c,size1,size2,next_wait,next_wait_ms);

	sound_prev_play_c = play_c;

	return next_wait_ms;
}

DWORD WINAPI EMU_OSD::sound_proc(LPVOID lpParameter)
{
	EMU_OSD *emu = (EMU_OSD *)lpParameter;
	bool working = true;
	DWORD dwResult = 0;

#ifdef USE_SOUND_NOTIFY
	while(working){
		// wait event
		dwResult = MsgWaitForMultipleObjects(1, &emu->sound_nt_event, FALSE, INFINITE, QS_ALLEVENTS);

		switch(dwResult) {
		case WAIT_OBJECT_0 + 0:
		case WAIT_OBJECT_0 + 1:
			if (emu->update_sound_th() == (DWORD)-1) {
				working = false;
			}
			break;
		default:
			break;
		}
	}
#else
	while(working) {
		CDelay(dwResult);
		if ((dwResult = emu->update_sound_th()) == (DWORD)-1) {
			working = false;
			break;
		}
	}
#endif
	return 0;
}

void EMU_OSD::mute_sound(bool mute)
{
	if (mute) {
		now_mute = true;
	} else {
		if(!vm_pause) {
			now_mute = false;
#if 0
			if (sound_started) {
				lpdsb->Play(0, 0, DSBPLAY_LOOPING);
			}
#endif
		}
	}
}

#if 0
void EMU_OSD::set_volume(int volume)
{
//	if (lpdsb) {
//		lpdsb->SetVolume((LONG)volume);
//	}
	// set volume for sound devices.
	if (vm) {
		vm->set_volume();
	}
}
#endif

uint32_t EMU_OSD::adjust_sound_pos(uint32_t msec)
{
	DWORD play_c, write_c;
	DWORD cur_sec = 0;

	while(cur_sec < (msec + 200)) {
		// check current playing position
		lpdsb->GetCurrentPosition(&play_c, &write_c);
		if ((DSOUND_BUFFER_HALF / 10) < play_c && play_c < (DSOUND_BUFFER_HALF * 4 / 10) && msec <= (cur_sec + 200)) {
			break;
		}
		CDelay(10);
		cur_sec += 10;
	}
	logging->out_debugf(_T("EMU::adjust_sound: p:%ld %ldms"), play_c, cur_sec);
	return cur_sec;
}

void EMU_OSD::lock_sound_buffer()
{
	mux_sound_nt->lock();
}
void EMU_OSD::unlock_sound_buffer()
{
	mux_sound_nt->unlock();
}
