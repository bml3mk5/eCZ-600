/** @file osd_midi.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.10 -

	@brief [ midi related utils ]
*/

#include "../vm/vm_defs.h"

#ifdef USE_MIDI
#ifndef OSD_MIDI_H
#define OSD_MIDI_H

//#include "../emu_osd.h"
#include "../config.h"
#include "../cchar.h"
#include "../cptrlist.h"
#include "../cmutex.h"

enum en_midi_buffer {
	MIDI_MSG_BUFFER_MAX = 128
};

/**
	@brief midi port
*/
class CMidiPort
{
protected:
	int m_id;
	CTchar m_name;

public:
	CMidiPort();
	CMidiPort(int id, const _TCHAR *name);
	virtual ~CMidiPort();

	int GetID() const;
	const TCHAR *GetName() const;
	bool CompareID(int id) const;
	bool CompareName(const TCHAR *name) const;
	bool MatchNamePartly(const TCHAR *substr) const;
};

/**
	@brief midi port list
*/
class CMidiPortList : public CPtrList<CMidiPort>
{
public:
	CMidiPortList();
	virtual ~CMidiPortList();

	int Find(int id) const;
	int Find(const TCHAR *name) const;
	int FindBySubString(const TCHAR *substr) const;

	void GetDescription(int idx, _TCHAR *buf, size_t size) const;
	const _TCHAR *GetName(int idx) const;
};

/**
	@brief buffer
*/
class CMidiBuffer
{
protected:
	int     m_length;
	uint8_t m_buffer[MIDI_MSG_BUFFER_MAX];

public:
	CMidiBuffer();
	virtual ~CMidiBuffer();

	void PushOneDataToBuffer(uint8_t data);
	void ClearBuffer();
	const uint8_t *GetBuffer() const;
	int GetBufferLength() const;

//	int CopyFromBuffer(uint8_t *dst, int dst_len);
};

/**
	@brief midi message queue
*/
class CMidiMessageQueue
{
protected:
	int m_write_pos;
	int m_read_pos;
	int m_count;
	CMutex m_mutex;
	enum {
		QUEUE_MAX = 1024,
	};
	struct st_queue {
		uint32_t clock;
		int      length;
		uint8_t  buffer[MIDI_MSG_BUFFER_MAX];
	} m_queue[QUEUE_MAX];

public:
	CMidiMessageQueue();
	virtual ~CMidiMessageQueue();

	int Count() const;
	void ClearPartly(int count);
	void Push(uint32_t clock, const uint8_t *buffer, int length);
	void Push(uint32_t clock, uint8_t data);
	int  Pop(uint8_t **buffer);
	int  Peek(const uint8_t **buffer) const;
	bool IsArrived(uint32_t clock) const;
	uint32_t GetClock() const;
};

/**
	@brief midi sequencer
*/
class CMidiSequencer : public CMidiBuffer
{
private:
	uint8_t m_running_status;
	bool m_three_bytes;

	enum en_exclusive_flags {
		FROM_CONFIG = 0x01,
	};

	struct st_exclusive {
		int     len;
		int		flags;
		int		prev_type;	// send message specified type before this
		uint8_t msg[MIDI_MSG_BUFFER_MAX];
	} m_exclusive_current[MIDI_TYPE_END];

	static const struct st_exclusive_default {
		int     len;
		int		prev_type;	// send message specified type before this
		const uint8_t *msg;
	} c_exclusive_default[MIDI_TYPE_END];

	void SetExclusiveToConfig();
	void GetExclusiveFromConfig();

protected:

	CMidiMessageQueue m_queue;

	/// @note need implement on inherited class
	virtual uint32_t GetCurrentSystemTime() const { return 0; }
	/// @note need implement on inherited class
	virtual void RequestToSendQueue() {}

	int ParseData(uint8_t data);
	void InitSequencer();
	void TermSequencer();

public:
	CMidiSequencer();
	virtual ~CMidiSequencer();

	void SendAllSoundOffMessage();
};

/**
	@brief midi time keeper
*/
class CMidiTimeKeeper : public CMidiSequencer
{
protected:
#ifdef USE_MIDI_TIMEKEEP_OLD
//	uint32_t m_host_time_start_pause;
//	uint64_t m_vm_clock_start_pause;
//	int32_t m_host_time_offset;
//	int64_t m_vm_clock_offset;
//	bool m_now_pause_on;
//	uint32_t m_adjust_prev;
#else
	uint32_t m_host_time_base;
	uint64_t m_vm_clock_base;
#endif

	uint64_t m_vm_freq_base;	// freq / 1000

	int m_delay_time;

	uint32_t GetDelayedVmTime();

#ifdef USE_MIDI_TIMEKEEP_OLD
	int AdjustDifference(uint32_t vtime);
#endif

	void ResetTimeKeeper();

public:
	CMidiTimeKeeper();
	virtual ~CMidiTimeKeeper();

	void Update();
	void Pause(bool on);
	void SetCPUPower();
};

#endif /* OSD_MIDI_H */
#endif /* USE_MIDI */