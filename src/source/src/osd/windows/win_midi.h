/** @file win_midi.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.10 -

	@brief [ win32 midi ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_MIDI
#ifndef WIN_MIDI_H
#define WIN_MIDI_H

#include "../osd_midi.h"
#include "../../emu_osd.h"
#include <Windows.h>
#include <MMSystem.h>


/**
	@brief Manage midi
*/
class CMidi : public CMidiTimeKeeper
{
protected:
	CMidiPortList m_inlist;
	CMidiPortList m_outlist;

	HMIDIIN hMidiIn;
	HMIDIOUT hMidiOut;

	int m_midiin_dev_id;
	int m_midiout_dev_id;

	CMutex m_mutexin;
	CMutex m_mutexout;

	enum {
		MESSAGES_MAX = 16
	};
	int m_curr_message_idx;
	struct st_message {
		MIDIHDR msg;
//		uint8_t buf[256];
	} m_message[MESSAGES_MAX];

	HANDLE m_queueEvent;
	HANDLE m_queueThread;
	bool m_thread_working;

	uint32_t GetCurrentSystemTime() const;

	void ClosedIn();
	void ClosedOut();

	static void CALLBACK InProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	static void CALLBACK OutProc(HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

	bool StartThread();
	void StopThread();

	static DWORD WINAPI QueueThreadProc(LPVOID lpParameter);
	DWORD QueueThread();

public:
	CMidi();
	virtual ~CMidi();
	void Reset(bool power_on);

	int EnumIn();
	int EnumOut();
	int CountIn() const;
	int CountOut() const;
	int FindIn(const TCHAR *name) const;
	int FindOut(const TCHAR *name) const;
	int FindInBySubString(const TCHAR *substr) const;
	int FindOutBySubString(const TCHAR *substr) const;

	bool OpenIn(int dev_id);
	bool OpenOut(int dev_id);
	bool OpenInByName(const _TCHAR *device_name);
	bool OpenOutByName(const _TCHAR *device_name);
	bool IsOpenedIn() const;
	bool IsOpenedOut() const;
	bool IsConnectedIn(int dev_id) const;
	bool IsConnectedOut(int dev_id) const;
	int  GetConnectedIn() const;
	int  GetConnectedOut() const;
	const _TCHAR *GetOpenedInName() const;
	const _TCHAR *GetOpenedOutName() const;
	void CloseIn();
	void CloseOut();

	bool StartIn();
	bool StopIn();

	void QueueInData(uint8_t data);
//	void SendData(uint8_t data);
	void SendQueue(const uint8_t *buffer, int length);

	void RequestToSendQueue();

	void GetInDescription(int idx, _TCHAR *buf, size_t size) const;
	void GetOutDescription(int idx, _TCHAR *buf, size_t size) const;
	const _TCHAR *GetInName(int idx) const;
	const _TCHAR *GetOutName(int idx) const;
	void SetOutDelayTime(int delay) { m_delay_time = delay; }
	int GetOutDelayTime() const { return m_delay_time; }
};

#endif /* WIN_MIDI_H */
#endif /* USE_MIDI */