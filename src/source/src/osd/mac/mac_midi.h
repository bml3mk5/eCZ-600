/** @file mac_midi.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.15 -

	@brief [ mac midi ]
*/

#include "../../vm/vm_defs.h"

#ifdef USE_MIDI
#ifndef MAC_MIDI_H
#define MAC_MIDI_H

#include "../osd_midi.h"
#include "../../emu_osd.h"
#include <CoreMIDI/CoreMIDI.h>
#include <CoreAudio/CoreAudio.h>
#include <SDL.h>

/**
	@brief Manage midi
*/
class CMidi : public CMidiTimeKeeper
{
protected:
	CMidiPortList m_inlist;
	CMidiPortList m_outlist;

	MIDIClientRef	mInClientRef;
	MIDIPortRef		mInPortRef;
	MIDIEndpointRef mInEpRef;
	MIDIClientRef	mOutClientRef;
	MIDIPortRef		mOutPortRef;
	MIDIEndpointRef mOutEpRef;

	int m_midiin_dev_id;
	int m_midiout_dev_id;

	CMutex m_mutexin;
	CMutex m_mutexout;

	enum {
		MESSAGES_MAX = 16,
		PACKETLIST_BUFMAX = 128
	};
	int m_curr_message_idx;
	struct st_message {
		MIDIPacketList *packetList;
	} m_message[MESSAGES_MAX];

	SDL_mutex *m_queueMutex;
	SDL_cond *m_queueCond;
	SDL_Thread *m_queueThread;
	bool m_thread_working;

	uint32_t GetCurrentSystemTime() const;
	
	static void GetEndpointPropertyName(MIDIEndpointRef epRef, char *buf, int size);

	void ClosedIn();
	void ClosedOut();

	static void InPortProc(const MIDIPacketList *pktList, void *readProcRefCon, void *srcConnRefCon);
	static void InProc(const MIDINotification *message, void *refCon);
	static void OutProc(const MIDINotification *message, void *refCon);

	bool StartThread();
	void StopThread();

	static int QueueThreadProc(void *ptr);
	int QueueThread();

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

#endif /* MAC_MIDI_H */
#endif /* USE_MIDI */
