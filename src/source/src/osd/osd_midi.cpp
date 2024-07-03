/** @file osd_midi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.10 -

	@brief [ midi related utils ]
*/

#include "osd_midi.h"

#ifdef USE_MIDI

//#include "../emu_osd.h"
//#include "../main.h"
#include "../config.h"
//#include "../vm/device.h"
#include "../utility.h"

//#define DEBUG_LOG_TIME_KEEPER


////////////////////////////////////////

CMidiPort::CMidiPort()
{
	m_id = -1;
}

CMidiPort::CMidiPort(int id, const _TCHAR *name)
{
	m_id = id;
	m_name.Set(name);
}

CMidiPort::~CMidiPort()
{
}

int CMidiPort::GetID() const
{
	return m_id;
}

const TCHAR *CMidiPort::GetName() const
{
	return m_name.Get();
}

bool CMidiPort::CompareID(int id) const
{
	return (m_id == id);
}

bool CMidiPort::CompareName(const TCHAR *name) const
{
	return (_tcscmp(m_name.Get(), name) == 0);
}

bool CMidiPort::MatchNamePartly(const TCHAR *substr) const
{
	return (_tcsstr(m_name.Get(), substr) != NULL);
}

////////////////////////////////////////

CMidiPortList::CMidiPortList()
	: CPtrList<CMidiPort>()
{
}

CMidiPortList::~CMidiPortList()
{
}

int CMidiPortList::Find(int id) const
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->CompareID(id)) {
			match = i;
			break;
		}
	}
	return match;
}

int CMidiPortList::Find(const TCHAR *name) const
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->CompareName(name)) {
			match = i;
			break;
		}
	}
	return match;
}

int CMidiPortList::FindBySubString(const TCHAR *substr) const
{
	int match = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->MatchNamePartly(substr)) {
			match = i;
			break;
		}
	}
	return match;
}

void CMidiPortList::GetDescription(int idx, _TCHAR *buf, size_t size) const
{
	if (idx >= Count()) {
		return;
	}
	const CMidiPort *item = Item(idx);
	UTILITY::tcscpy(buf, size, item->GetName()); 
}

const _TCHAR *CMidiPortList::GetName(int idx) const
{
	if (idx >= Count()) {
		return NULL;
	}
	const CMidiPort *item = Item(idx);
	return item->GetName();
}

////////////////////////////////////////

CMidiBuffer::CMidiBuffer()
{
	m_length = 0;
}

CMidiBuffer::~CMidiBuffer()
{
}

void CMidiBuffer::PushOneDataToBuffer(uint8_t data)
{
	if (MIDI_MSG_BUFFER_MAX <= m_length) return;

	m_buffer[m_length++] = data;
}

void CMidiBuffer::ClearBuffer()
{
	m_length = 0;
}

const uint8_t *CMidiBuffer::GetBuffer() const
{
	return m_buffer;
}

int CMidiBuffer::GetBufferLength() const
{
	return m_length;
}

#if 0
int CMidiBuffer::CopyFromBuffer(uint8_t *dst, int dst_len)
{
	int len = dst_len < m_length ? dst_len : m_length;
	memcpy(dst, m_buffer, len);
	m_length = 0;
	return len;
}
#endif

////////////////////////////////////////

CMidiMessageQueue::CMidiMessageQueue()
{
	m_write_pos = 0;
	m_read_pos = 0;
	m_count = 0;
	for(int i=0; i<QUEUE_MAX; i++) {
		memset(&m_queue[i], 0, sizeof(m_queue[0]));
	}
}

CMidiMessageQueue::~CMidiMessageQueue()
{
}

int CMidiMessageQueue::Count() const
{
	return m_count;
}

void CMidiMessageQueue::ClearPartly(int count)
{
	m_mutex.lock();

	m_write_pos = m_read_pos + count;
	if(m_write_pos >= QUEUE_MAX) {
		m_write_pos -= QUEUE_MAX;
	}

	m_count = count;

	m_mutex.unlock();
}

void CMidiMessageQueue::Push(uint32_t clock, const uint8_t *buffer, int length)
{
	if(m_count >= QUEUE_MAX) {
		// queue is full
		return;
	}
	int min_len = MIN(length, MIDI_MSG_BUFFER_MAX);

	m_mutex.lock();

	struct st_queue *q = &m_queue[m_write_pos];

	q->clock = clock;
	memcpy(q->buffer, buffer, min_len);
	q->length = min_len;

	m_write_pos++;
	if(m_write_pos >= QUEUE_MAX) {
		m_write_pos = 0;
	}
	m_count++;

	m_mutex.unlock();
}

void CMidiMessageQueue::Push(uint32_t clock, uint8_t data)
{
	if(m_count >= QUEUE_MAX) {
		// queue is full
		return;
	}

	m_mutex.lock();

	struct st_queue *q = &m_queue[m_write_pos];

	q->clock = clock;
	q->buffer[0] = data;
	q->length = 1;

	m_write_pos++;
	if(m_write_pos >= QUEUE_MAX) {
		m_write_pos = 0;
	}
	m_count++;

	m_mutex.unlock();
}

int CMidiMessageQueue::Pop(uint8_t **buffer)
{
	if(m_count <= 0) {
		if (buffer) *buffer = NULL;
		return 0;
	}

	m_mutex.lock();

	struct st_queue *q = &m_queue[m_read_pos];

	if (buffer) *buffer = q->buffer;

	m_read_pos++;
	if(m_read_pos >= QUEUE_MAX) {
		m_read_pos = 0;
	}
	m_count--;

	m_mutex.unlock();

	return q->length;
}

int CMidiMessageQueue::Peek(const uint8_t **buffer) const
{
	if(m_count <= 0) {
		if (buffer) *buffer = NULL;
		return 0;
	}
	const struct st_queue *q = &m_queue[m_read_pos];

	if (buffer) *buffer = q->buffer;

	return q->length;
}

uint32_t CMidiMessageQueue::GetClock() const
{
	if(m_count <= 0) {
		return 0;
	}
	const struct st_queue *q = &m_queue[m_read_pos];
	return q->clock;
}

bool CMidiMessageQueue::IsArrived(uint32_t clock) const
{
	if(m_count <= 0) {
		return false;
	}
	const struct st_queue *q = &m_queue[m_read_pos];
	return q->clock <= clock;
}

////////////////////////////////////////

const struct CMidiSequencer::st_exclusive_default CMidiSequencer::c_exclusive_default[MIDI_TYPE_END] = {
	{ 6, -1,           (const uint8_t *)"\xF0\x7E\x7F\x09\x01\xF7"},	// GM System On
	{11, MIDI_TYPE_GM, (const uint8_t *)"\xF0\x41\x10\x42\x12\x40\x00\x7F\x00\x41\xF7"},	// GS Reset
	{11, -1,           (const uint8_t *)"\xF0\x41\x10\x16\x12\x7F\x00\x00\x00\x01\xF7"},	// LA Reset
	{ 9, MIDI_TYPE_GM, (const uint8_t *)"\xF0\x43\x10\x4C\x00\x00\x7E\x00\xF7"},	// XG System On Reset
	{ 0, -1, NULL}
};

CMidiSequencer::CMidiSequencer()
{
	m_running_status = 0;
	m_three_bytes = false;

	InitSequencer();
}

CMidiSequencer::~CMidiSequencer()
{
	TermSequencer();
}

void CMidiSequencer::InitSequencer()
{
	GetExclusiveFromConfig();
}

void CMidiSequencer::TermSequencer()
{
	SetExclusiveToConfig();
}

void CMidiSequencer::SetExclusiveToConfig()
{
	_TCHAR buf[MIDI_MSG_BUFFER_MAX * 2];
	for(int i=0; i<MIDI_TYPE_END; i++) {
		if (m_exclusive_current[i].flags & FROM_CONFIG) {
			// convert binary to string
			int len = m_exclusive_current[i].len;
			buf[0] = '\0';
			for(int l=0; l<len; l++) {
				UTILITY::sntprintf(buf, sizeof(buf), _T("%02X"), m_exclusive_current[i].msg[l]);
			}
			pConfig->midi_exclusive[i].Set(buf);
		} else {
			pConfig->midi_exclusive[i].Clear();
		}
	}
}

void CMidiSequencer::GetExclusiveFromConfig()
{
	uint8_t buf[MIDI_MSG_BUFFER_MAX];
	for(int i=0; i<MIDI_TYPE_END; i++) {
		memset(&m_exclusive_current[i], 0, sizeof(m_exclusive_current[i]));
		int len = pConfig->midi_exclusive[i].Length();
		const _TCHAR *p = pConfig->midi_exclusive[i].Get();
		_TCHAR c;
		buf[0] = '\0';
		bool odd = false;
		int buflen = 0;
		// convert string to binary
		for(int l=0; l<len; l++) {
			if (p[l] >= _T('0') && p[l] <= _T('9')) {
				c = p[l] - _T('0');
			} else if (p[l] >= _T('A') && p[l] <= _T('F')) {
				c = p[l] - _T('A') + 10;
			} else if (p[l] >= _T('a') && p[l] <= _T('f')) {
				c = p[l] - _T('a') + 10;
			} else {
				continue;
			}
			if (odd) {
				buf[buflen] |= c;
				buflen++;
				odd = false;
			} else {
				buf[buflen]=(c << 4);
				odd = true;
			}
		}

		m_exclusive_current[i].prev_type = c_exclusive_default[i].prev_type;

		if (buflen > 0) {
			if (memcmp(buf, c_exclusive_default[i].msg, c_exclusive_default[i].len) == 0) {
				// same as default
				buflen = 0;
			} else {
				// set message in config file
				m_exclusive_current[i].len = buflen;
				m_exclusive_current[i].flags = FROM_CONFIG;
				memcpy(m_exclusive_current[i].msg, buf, buflen);
			}
		}
		if (buflen == 0) {
			// set default message
			m_exclusive_current[i].flags = 0;
			if (c_exclusive_default[i].len > 0) {
				m_exclusive_current[i].len = c_exclusive_default[i].len;
				memcpy(m_exclusive_current[i].msg, c_exclusive_default[i].msg, c_exclusive_default[i].len);
			} else {
				m_exclusive_current[i].len = 0;
			}
		}
	}
}

void CMidiSequencer::SendAllSoundOffMessage()
{
	uint8_t buf[MIDI_MSG_BUFFER_MAX];
	int len;

	// get current time (ms)
	uint32_t htime = GetCurrentSystemTime();

	m_queue.ClearPartly(2);

	// Bn 78 00 All sound off
	len = 3;
	buf[1] = 0x78; buf[2] = 0;
	for(int i=0; i<16; i++) {
		buf[0] = 0xb0 + i;
		// send
		m_queue.Push(htime, buf, len);
		htime++;
	}
	// All note off, omni on, poly
	len = 3;
	buf[2] = 0;
	for(int i=0; i<16; i++) {
		buf[0] = 0xb0 + i;
		for(int j=0; j<3; j++) {
			buf[1] = 0x7b + j*2; // 7b, 7d, 7f
			// send
			m_queue.Push(htime, buf, len);
			htime++;
		}
	}
	// Reset all controller
	len = 3;
	buf[1] = 0x79; buf[2] = 0;
	for(int i=0; i<16; i++) {
		buf[0] = 0xb0 + i;
		// send
		m_queue.Push(htime, buf, len);
		htime++;
	}

	// Exclusive message
	int type = pConfig->midi_send_reset_type;
	if (type < 0 || type >= MIDI_TYPE_END) {
		return;
	}
	if (m_exclusive_current[type].len <= 0) {
		return;
	}
	if (m_exclusive_current[type].prev_type >= 0) {
		// send
		int prev = m_exclusive_current[type].prev_type;
		m_queue.Push(htime, m_exclusive_current[prev].msg, m_exclusive_current[prev].len); 
		htime++;
	}
	// send
	m_queue.Push(htime, m_exclusive_current[type].msg, m_exclusive_current[type].len);

	// request to the queue thread to process this queue
	RequestToSendQueue();

	// wait
	emu->sleep(100);
}

/// @return b7-b0: set ($F8 - $FF) if realtime message exists 
/// @return b8   : set 1 if one message is stored in buffer
int CMidiSequencer::ParseData(uint8_t data)
{
	int rc = 0;

	if (data & 0x80) {
		// status or system message
		if (data >= 0xf8) {
			// realtime message
			return data;
		}

		if (m_running_status == 1) {
			// end of system exclusibe data
			PushOneDataToBuffer(0xf7);

			// complete
			rc = 0x100;
		}

		if (data <= 0xf3) {
			m_running_status = data;
		} else {
			m_running_status = 0;
		}

		m_three_bytes = false;

		if (data == 0xf6) {
			// store to fifo
			PushOneDataToBuffer(data);

			return 0x100;
		}
	} else {
		if (m_three_bytes) {
			m_three_bytes = false;
			// store to fifo in position 3
			PushOneDataToBuffer(data);

			// complete
			return 0x100;

		} else {
			if (m_running_status == 0) {
				// ignore

			} else if (m_running_status == 1) {
				// system exclusibe data
				// store current data
				PushOneDataToBuffer(data);

			} else if (m_running_status < 0xc0) {
				m_three_bytes = true;
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

			} else if (m_running_status < 0xe0) {
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

				// complete
				rc = 0x100;

			} else if (m_running_status < 0xf0) {
				m_three_bytes = true;
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

			} else if (m_running_status == 0xf2) {
				m_three_bytes = true;
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

				m_running_status = 0;

			} else if (m_running_status == 0xf1 || m_running_status == 0xf3) {
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

				m_running_status = 0;
				// complete
				rc = 0x100;

			} else if (m_running_status == 0xf0) {
				// store status to fifo
				PushOneDataToBuffer(m_running_status);
				// store current data
				PushOneDataToBuffer(data);

				m_running_status = 1;

			}
		}
	}
	return rc;
}

////////////////////////////////////////

CMidiTimeKeeper::CMidiTimeKeeper()
	: CMidiSequencer()
{
#ifdef USE_MIDI_TIMEKEEP_OLD
	m_host_time_start_pause = 0;
	m_vm_clock_start_pause = 0;
	m_host_time_offset = GetCurrentSystemTime();
	m_vm_clock_offset = 0;
	m_now_pause_on = false;
	m_adjust_prev = 0;
#else
	m_host_time_base = 0;
	m_vm_clock_base = 0;
#endif

	m_vm_freq_base = (CPU_CLOCKS / 1000);

	m_delay_time = 150;

}

CMidiTimeKeeper::~CMidiTimeKeeper()
{
}

void CMidiTimeKeeper::ResetTimeKeeper()
{
	SetCPUPower();

#ifdef USE_MIDI_TIMEKEEP_OLD
	uint32_t htime = GetCurrentSystemTime();	// msec
	uint64_t vclock = emu->get_current_clock();

	htime -= m_host_time_offset;
	uint32_t vtime = (uint32_t)((vclock - m_vm_clock_offset) / m_vm_freq_base); // msec

	vtime += m_delay_time;

	int sub_time = (int)(vtime - htime);

	if (sub_time >= m_delay_time) {
		m_vm_clock_offset += ((sub_time - m_delay_time) * m_vm_freq_base);
	} else {
		m_host_time_offset += (m_delay_time - sub_time);
	}
#else
#endif
}

/// @brief call every frame
void CMidiTimeKeeper::Update()
{
#ifndef USE_MIDI_TIMEKEEP_OLD
	m_host_time_base = GetCurrentSystemTime();	// msec
	m_vm_clock_base = emu->get_current_clock();

#ifdef DEBUG_LOG_TIME_KEEPER
	logging->out_logf(LOG_DEBUG, _T("MIDI Update: F:%llu V:%llu H:%u")
		,m_vm_freq_base, m_vm_clock_base, m_host_time_base);
#endif
#endif
}

void CMidiTimeKeeper::Pause(bool on)
{
#ifdef USE_MIDI_TIMEKEEP_OLD
	if (on) {
		if (!m_now_pause_on) {
			// get current time
			m_host_time_start_pause = GetCurrentSystemTime();
			m_vm_clock_start_pause = emu->get_current_clock();
#ifdef DEBUG_LOG_TIME_KEEPER
			logging->out_logf(LOG_DEBUG, _T("MidiPauseOn CLK:%ld RT:%d"),
				m_vm_clock_start_pause, m_host_time_start_pause);
#endif
		}
	} else {
		if (m_now_pause_on) {
			// add elapsed time which paused on to adjust time
			uint32_t host_stop_time = GetCurrentSystemTime();
			uint64_t vm_stop_clock = emu->get_current_clock();
			m_host_time_offset += (host_stop_time - m_host_time_start_pause);
			m_vm_clock_offset += (vm_stop_clock - m_vm_clock_start_pause);
#ifdef DEBUG_LOG_TIME_KEEPER
			logging->out_logf(LOG_DEBUG, _T("MidiPauseOff CLK:%ld RT:%d => CLK:%ld RT:%d")
				,vm_stop_clock,host_stop_time
				,m_vm_clock_offset,m_host_time_offset);
#endif
		}
	}
	m_now_pause_on = on;
#endif
}

void CMidiTimeKeeper::SetCPUPower()
{
	int power = emu->get_current_power();

	uint64_t new_vm_freq = (CPU_CLOCKS / 1000);

	if (power < 1) {
		new_vm_freq /= (uint64_t)(1 << (1 - power));
	} else {
		new_vm_freq *= (uint64_t)(1 << (power - 1));
	}

#ifdef USE_MIDI_TIMEKEEP_OLD
	if (m_vm_freq_base != new_vm_freq) {
		// adjust offset
		uint64_t vclock = emu->get_current_clock();
		uint64_t vsub = vclock - m_vm_clock_offset;
		vsub = vsub * new_vm_freq / m_vm_freq_base;
		m_vm_clock_offset = vclock - vsub;
		m_vm_freq_base = new_vm_freq;
#ifdef DEBUG_LOG_TIME_KEEPER
		logging->out_logf(LOG_DEBUG, _T("MIDI ChangePower:%d Offset:V%ld H%d")
			, power, m_vm_clock_offset, m_host_time_offset);
#endif
	}

#else
	m_vm_freq_base = new_vm_freq;

#endif
}

uint32_t CMidiTimeKeeper::GetDelayedVmTime()
{
#ifdef USE_MIDI_TIMEKEEP_OLD
	uint32_t vtime = (uint32_t)((emu->get_current_clock() - m_vm_clock_offset) / m_vm_freq_base);	// msec
	vtime += m_delay_time;
	return vtime;
#else
	uint64_t vclock = emu->get_current_clock();
	uint32_t vtime = (uint32_t)((vclock - m_vm_clock_base) / m_vm_freq_base);	// msec
	vtime += m_host_time_base;
	vtime += m_delay_time;

#ifdef DEBUG_LOG_TIME_KEEPER
	{
		uint32_t htime = GetCurrentSystemTime();
		int subst = vtime - htime;
		logging->out_logf(LOG_DEBUG, _T("Time: VC:%llu V:%u H:%u => %d"), vclock, vtime, htime, subst);
	}
#endif
	return vtime;
#endif
}

#ifdef USE_MIDI_TIMEKEEP_OLD
int CMidiTimeKeeper::AdjustDifference(uint32_t vtime)
{
	uint32_t htime = GetCurrentSystemTime();
	int sub_time = (int)(htime - m_adjust_prev);
	if (sub_time >= 100) { // 100msec
		uint32_t h_stime = htime - m_host_time_offset;
		int subst = (int)(vtime - h_stime);
		if (subst < m_delay_time) {
			m_host_time_offset += ((m_delay_time - subst) / 10);
		} else if (subst > m_delay_time + 10) {
			m_vm_clock_offset += m_vm_freq_base;	// 1msec
		}
#ifdef DEBUG_LOG_TIME_KEEPER
		uint32_t h_sstime = htime - m_host_time_offset;
		int substs = (int)(vtime - h_sstime);
		logging->out_logf(LOG_DEBUG, _T("MIDI Adjust:T%d V:%u-H:%u=%d => %d")
			, sub_time, vtime, h_stime, subst, substs);
#endif
		m_adjust_prev = htime;
	}
	return sub_time;
}
#endif

#endif /* USE_MIDI */
