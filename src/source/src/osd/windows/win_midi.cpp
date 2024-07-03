/** @file win_midi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.10 -

	@brief [ win32 midi ]
*/

#include "win_midi.h"

#ifdef USE_MIDI

#include "../../emu_osd.h"
#include "../../main.h"
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"

#pragma comment(lib, "winmm.lib")

//#define DEBUG_QUEUE

extern EMU *emu;

////////////////////////////////////////
// wrapper functions on EMU class
////////////////////////////////////////

void EMU_OSD::EMU_MIDI()
{
	cmidi = NULL;
}
void EMU_OSD::initialize_midi()
{
	cmidi = new CMidi();

	set_midiout_delay_time(pConfig->midiout_delay);
	cmidi->OpenOutByName(pConfig->midiout_device.Get());
}
void EMU_OSD::release_midi()
{
	const _TCHAR *name = cmidi->GetOpenedOutName();
	if (name) {
		pConfig->midiout_device.Set(name);
	} else {
		pConfig->midiout_device.Clear();
	}
	delete cmidi;
	cmidi = NULL;
}
void EMU_OSD::reset_midi(bool power_on)
{
	if (cmidi) {
		cmidi->Reset(power_on);
	}
}
void EMU_OSD::update_midi()
{
	if (cmidi) {
		cmidi->Update();
	}
}
void EMU_OSD::pause_midi()
{
	if (cmidi) {
		cmidi->Pause(vm_pause != 0);
	}
}

void EMU_OSD::set_midi_cpu_power()
{
	if (cmidi) {
		cmidi->SetCPUPower();
	}
}

int EMU_OSD::enum_midiins()
{
	return cmidi ? cmidi->EnumIn() : 0;
}
int EMU_OSD::enum_midiouts()
{
	return cmidi ? cmidi->EnumOut() : 0;
}
void EMU_OSD::get_midiin_description(int idx, _TCHAR *buf, size_t size)
{
	if (cmidi) {
		cmidi->GetInDescription(idx, buf, size);
	}
}
void EMU_OSD::get_midiout_description(int idx, _TCHAR *buf, size_t size)
{
	if (cmidi) {
		cmidi->GetOutDescription(idx, buf, size);
	}
}

#if 0
void EMU_OSD::enable_midiin_connect(int idx)
{
	if (!cmidi) return;
	if (cmidi->IsConnectedIn(idx)) {
		// close if alreay connected
		cmidi->CloseIn();
	} else if (cmidi->CountIn() > 0) {
		cmidi->CloseIn();
		cmidi->OpenIn(idx);
	}
}
void EMU_OSD::enable_midiout_connect(int idx)
{
	if (!cmidi) return;
	if (cmidi->IsConnectedOut(idx)) {
		// close if alreay connected
		cmidi->CloseOut();
	} else if (cmidi->CountOut() > 0) {
		cmidi->CloseOut();
		cmidi->OpenOut(idx);
	}
}
#endif

bool EMU_OSD::now_midiin_connecting(int idx)
{
	return cmidi ? cmidi->IsConnectedIn(idx) : false;
}
bool EMU_OSD::now_midiout_connecting(int idx)
{
	return cmidi ? cmidi->IsConnectedOut(idx) : false;
}
void EMU_OSD::set_midiout_delay_time(int val)
{
	if (cmidi) {
		cmidi->SetOutDelayTime(val);
	}
}

bool EMU_OSD::open_midiin(int idx)
{
	return cmidi ? cmidi->OpenIn(idx) : false;
}
bool EMU_OSD::is_opened_midiin()
{
	return cmidi ? cmidi->IsOpenedIn() : false;
}
void EMU_OSD::close_midiin()
{
	if (cmidi) {
		cmidi->CloseIn();
	}
}
bool EMU_OSD::open_midiout(int idx)
{
	return cmidi ? cmidi->OpenOut(idx) : false;
}
bool EMU_OSD::is_opened_midiout()
{
	return cmidi ? cmidi->IsOpenedOut() : false;
}
void EMU_OSD::close_midiout()
{
	if (cmidi) {
		cmidi->CloseOut();
	}
}
uint8_t EMU_OSD::recv_data_from_midiin()
{
	return 0;
}

void EMU_OSD::send_data_to_midiout(uint8_t data)
{
	cmidi->QueueInData(data);
}

void EMU_OSD::send_midi_reset_message()
{
	cmidi->SendAllSoundOffMessage();
}

////////////////////////////////////////
// MIDI on windows
////////////////////////////////////////

CMidi::CMidi()
	: CMidiTimeKeeper()
{
	hMidiIn = NULL;
	hMidiOut = NULL;
	m_midiin_dev_id = -1;
	m_midiout_dev_id = -1;

	m_curr_message_idx = 0;
	for(int i=0; i<MESSAGES_MAX; i++) {
//		m_message[i].msg.lpData = (LPSTR)m_message[i].buf;
		m_message[i].msg.dwFlags = MHDR_DONE;
		m_message[i].msg.dwBufferLength = 0;
	}

	m_queueEvent = NULL;
	m_queueThread = NULL;
	m_thread_working = false;

	if (!StartThread()) {
		StopThread();
	}
}

CMidi::~CMidi()
{
	CloseIn();
	CloseOut();
	StopThread();
}

/// @brief for time keeper
uint32_t CMidi::GetCurrentSystemTime() const
{
	return (uint32_t)timeGetTime();
}

void CMidi::RequestToSendQueue()
{
	if (m_queueEvent) {
		// request to the queue thread to process this queue
		SetEvent(m_queueEvent);
	}
}

void CMidi::Reset(bool power_on)
{
	ResetTimeKeeper();
}

int CMidi::EnumIn()
{
	MMRESULT res;
	MIDIINCAPS caps;
	UINT num, devid;

	num = midiInGetNumDevs();

	m_inlist.Clear();
	for(devid=0; devid<num && devid<MIDI_MAX_PORTS; devid++) {
		res = midiInGetDevCaps(devid, &caps, sizeof(caps));
		if (res != MMSYSERR_NOERROR) {
			continue;
		}
		m_inlist.Add(new CMidiPort((int)devid, caps.szPname));
	}
	return (int)num;
}

int CMidi::EnumOut()
{
	MMRESULT res;
	MIDIOUTCAPS caps;
	UINT num, devid;

	num = midiOutGetNumDevs();

	m_outlist.Clear();
	for(devid=0; devid<num && devid<MIDI_MAX_PORTS; devid++) {
		res = midiOutGetDevCaps(devid, &caps, sizeof(caps));
		if (res != MMSYSERR_NOERROR) {
			continue;
		}
		m_outlist.Add(new CMidiPort((int)devid, caps.szPname));
	}
	return (int)num;
}

int CMidi::CountIn() const
{
	return m_inlist.Count();
}

int CMidi::CountOut() const
{
	return m_outlist.Count();
}

int CMidi::FindIn(const TCHAR *name) const
{
	return m_inlist.Find(name);
}

int CMidi::FindOut(const TCHAR *name) const
{
	return m_outlist.Find(name);
}

int CMidi::FindInBySubString(const TCHAR *substr) const
{
	return m_inlist.FindBySubString(substr);
}

int CMidi::FindOutBySubString(const TCHAR *substr) const
{
	return m_outlist.FindBySubString(substr);
}

bool CMidi::OpenIn(int dev_id)
{
	MMRESULT res;

	if (hMidiIn) return true;

	m_mutexin.lock();
	res = midiInOpen(&hMidiIn, dev_id, (DWORD_PTR)InProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	m_mutexin.unlock();
	if (res != MMSYSERR_NOERROR) {
		return false;
	}
	logging->out_logf(LOG_DEBUG, _T("Connected MIDI In: %d"), dev_id);
	m_midiin_dev_id = dev_id;
	return true;
}

bool CMidi::OpenOut(int dev_id)
{
	MMRESULT res;

	if (hMidiOut) return true;

	m_mutexout.lock();
	res = midiOutOpen(&hMidiOut, dev_id, (DWORD_PTR)OutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	m_mutexout.unlock();
	if (res != MMSYSERR_NOERROR) {
		return false;
	}
	logging->out_logf(LOG_DEBUG, _T("Connected MIDI Out: %d"), dev_id);
	m_midiout_dev_id = dev_id;
	return true;
}

bool CMidi::OpenInByName(const _TCHAR *device_name)
{
	if (CountIn() <= 0) {
		EnumIn();
	}
	int idx = FindIn(device_name);
	if (idx < 0) {
		return false;
	}
	return OpenIn(idx);
}

bool CMidi::OpenOutByName(const _TCHAR *device_name)
{
	if (CountOut() <= 0) {
		EnumOut();
	}
	int idx = FindOut(device_name);
	if (idx < 0) {
		return false;
	}
	return OpenOut(idx);
}

bool CMidi::IsOpenedIn() const
{
	return (hMidiIn != NULL);
}

bool CMidi::IsOpenedOut() const
{
	return (hMidiOut != NULL);
}

bool CMidi::IsConnectedIn(int dev_id) const
{
	return (m_midiin_dev_id == dev_id && IsOpenedIn());
}

bool CMidi::IsConnectedOut(int dev_id) const
{
	return (m_midiout_dev_id == dev_id && IsOpenedOut());
}

int CMidi::GetConnectedIn() const
{
	return m_midiin_dev_id;
}

int CMidi::GetConnectedOut() const
{
	return m_midiout_dev_id;
}

const _TCHAR *CMidi::GetOpenedInName() const
{
	if (m_midiin_dev_id < 0) {
		return NULL;
	}
	return m_inlist.GetName(m_midiin_dev_id);
}

const _TCHAR *CMidi::GetOpenedOutName() const
{
	if (m_midiout_dev_id < 0) {
		return NULL;
	}
	return m_outlist.GetName(m_midiout_dev_id);
}

void CALLBACK CMidi::InProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	CMidi *midi = (CMidi *)dwInstance;

	switch(wMsg) {
	case MIM_DATA:
	case MIM_LONGDATA:
		break;
	case MIM_ERROR:
	case MIM_LONGERROR:
		break;
	case MIM_MOREDATA:
		break;
	case MIM_CLOSE:
		midi->ClosedIn();
		break;
	case MIM_OPEN:
		break;
	default:
		break;
	}
}

void CALLBACK CMidi::OutProc(HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	CMidi *midi = (CMidi *)dwInstance;

	switch(wMsg) {
	case MOM_DONE:
		{
			MIDIHDR *msg = (MIDIHDR *)dwParam1;
			midiOutUnprepareHeader(hMidiOut, msg, (UINT)sizeof(MIDIHDR));
		}
		break;
	case MOM_CLOSE:
		midi->ClosedOut();
		break;
	case MOM_OPEN:
		break;
	default:
		break;
	}
}

void CMidi::CloseIn()
{
	if (hMidiIn) {
		m_mutexin.lock();
		midiInClose(hMidiIn);
		hMidiIn = NULL;
		m_mutexin.unlock();
		m_midiin_dev_id = -1;
		logging->out_log(LOG_DEBUG, _T("Disconnected MIDI In"));
	}
}

void CMidi::CloseOut()
{
	if (hMidiOut) {
		m_mutexout.lock();
		midiOutClose(hMidiOut);
		hMidiOut = NULL;
		m_mutexout.unlock();
		m_midiout_dev_id = -1;
		logging->out_log(LOG_DEBUG, _T("Disconnected MIDI Out"));
	}
}

void CMidi::ClosedIn()
{
}

void CMidi::ClosedOut()
{
}

bool CMidi::StartIn()
{
	return (midiInStart(hMidiIn) == MMSYSERR_NOERROR);
}

bool CMidi::StopIn()
{
	return (midiInStop(hMidiIn) == MMSYSERR_NOERROR);
}

void CMidi::QueueInData(uint8_t data)
{
	int rc = ParseData(data);

	if (!rc) {
		// wait for next data
		return;
	}

	uint32_t vtime = GetDelayedVmTime();

#ifdef USE_MIDI_TIMEKEEP_OLD
	// adjust
#ifdef DEBUG_QUEUE
	int sub_time = AdjustDifference(dclock);
#else
	AdjustDifference(dclock);
#endif
#endif

	if (rc >= 0x100) {
		// normal or system message 
		m_queue.Push(vtime, GetBuffer(), GetBufferLength());
		ClearBuffer();
	} else {
		// realtime message
		m_queue.Push(vtime, rc & 0xff);
	}

#ifdef DEBUG_QUEUE
	{
		_TCHAR str[1024];
		const uint8_t *buf;
		int len = m_queue.Peek(&buf);
		UTILITY::stprintf(str, sizeof(str), _T("QueueIn:(%d)"), len);
		for(int i=0; i<len; i++) {
			UTILITY::sntprintf(str, sizeof(str), _T(" %02X"), buf[i]);
		}
		logging->out_log(LOG_DEBUG, str);
	}
#endif

	// request to the queue thread to process this queue
	RequestToSendQueue();
}

void CMidi::SendQueue(const uint8_t *buffer, int length)
{
	HRESULT res;

	struct st_message *m = &m_message[m_curr_message_idx];

	if (!(m->msg.dwFlags & MHDR_DONE)) {
		// buffer is full, so lost the data
		return;
	}

//	logging->out_logf(LOG_DEBUG, _T("midiOut:idx%d flag:%x"), m_curr_message_idx, m->msg.dwFlags);

	m->msg.lpData = (LPSTR)buffer;
	m->msg.dwBufferLength = (DWORD)length;

	m_mutexout.lock();
	if (hMidiOut) {
		// send
		m->msg.dwFlags = 0;
		res = midiOutPrepareHeader(hMidiOut, &m->msg, (UINT)sizeof(m->msg));
		if (res == MMSYSERR_NOERROR) {
			res = midiOutLongMsg(hMidiOut, &m->msg, sizeof(m->msg));
		}
	} else {
		// dummy
		m->msg.dwFlags = MHDR_DONE;
	}
	m_mutexout.unlock();

	// clean next message
	m_curr_message_idx++;
	if (m_curr_message_idx >= MESSAGES_MAX) {
		m_curr_message_idx = 0;
	}
	m = &m_message[m_curr_message_idx];
	m->msg.dwBufferLength = 0;
}

#if 0
void CMidi::SendData(uint8_t data)
{

	int rc = ParseData(data);

	if (!rc) {
		// wait for next data
		return;
	}

	struct st_message *m = &m_message[m_curr_message_idx];

	if (rc >= 0x100) {
		// normal or system message 
		m->msg.dwBufferLength = CopyFromBuffer(m->buf, 256);
	} else {
		// realtime message
		m->buf[0] = (rc & 0xff);
		m->msg.dwBufferLength = 1;
	}
#if 0 // def _DEBUG
	{
		_TCHAR str[1024];
		UTILITY::stprintf(str, sizeof(str), _T("MidiOut (% 4d):"), m->msg.dwBufferLength);
		for(int i=0; i<m->msg.dwBufferLength; i++) {
			UTILITY::sntprintf(str, sizeof(str), _T(" %02X"), m->buf[i]);
		}
		logging->out_debug(str);
	}
#endif
	if (hMidiOut) {
		// send
		m->msg.dwFlags = 0;
		res = midiOutPrepareHeader(hMidiOut, &m->msg, (UINT)sizeof(m->msg));
		if (res != MMSYSERR_NOERROR) {
			return;
		}
		res = midiOutLongMsg(hMidiOut, &m->msg, sizeof(m->msg));
	}

	// clean next message
	m_curr_message_idx++;
	if (m_curr_message_idx >= MESSAGES_MAX) {
		m_curr_message_idx = 0;
	}
	m = &m_message[m_curr_message_idx];
	m->msg.dwBufferLength = 0;
}
#endif

void CMidi::GetInDescription(int idx, _TCHAR *buf, size_t size) const
{
	m_inlist.GetDescription(idx, buf, size);
}

void CMidi::GetOutDescription(int idx, _TCHAR *buf, size_t size) const
{
	m_outlist.GetDescription(idx, buf, size);
}

const _TCHAR *CMidi::GetInName(int idx) const
{
	return m_inlist.GetName(idx);
}

const _TCHAR *CMidi::GetOutName(int idx) const
{
	return m_outlist.GetName(idx);
}

DWORD WINAPI CMidi::QueueThreadProc(LPVOID lpParameter)
{
	CMidi *midi = (CMidi *)lpParameter;

	return midi->QueueThread();
}

DWORD CMidi::QueueThread()
{
	DWORD wait_result;
	int count;
	uint32_t cur_time;
	uint8_t *buffer;

	while(m_thread_working) {
		// wait to arrive new queue
		wait_result = WaitForSingleObject(m_queueEvent, INFINITE);
		if (wait_result != WAIT_OBJECT_0 && wait_result != WAIT_TIMEOUT) {
			// error occured?
			m_thread_working = false;
			break;
		}
		// queue exists?
		while ((count = m_queue.Count()) > 0) {
			// arrived?
			cur_time = (uint32_t)timeGetTime();
#ifdef USE_MIDI_TIMEKEEP_OLD
			cur_time -= m_host_time_offset;
#endif
			uint32_t wait_time = m_queue.GetClock();
//			logging->out_logf(LOG_DEBUG, _T("Curr:%d Queue:%d Cnt:%d"), cur_time, wait_time, count);
			if (!m_queue.IsArrived(cur_time)) {
				// sleep until arrived time
//				uint32_t wait_time = m_queue.GetClock();
				if (wait_time >= 2 + cur_time) {
					uint32_t t = wait_time - cur_time - 1;
					if (t > 100) t = 100;
//					logging->out_logf(LOG_DEBUG, _T("Sleep:%d"), t);
					Sleep(t);
				}
				continue;
			}
			// proccess one queue
			int length = m_queue.Pop(&buffer);
			// send to MIDI device
			if (length > 0) {
				SendQueue(buffer, length);
			}
		}
	}
	return 0;
}

bool CMidi::StartThread()
{
	// create event for message queue
	m_queueEvent = CreateEvent(NULL, TRUE, FALSE, _T("MidiQueueEvent"));
	if (!m_queueEvent) {
		// cannot create event
		logging->out_logf(LOG_ERROR, _T("CMidi::StartThread::CreateEvent failed: %d"), GetLastError());
		return false;
	}

	DWORD thread_id;
	m_thread_working = true;

	// create thread
	m_queueThread = CreateThread(NULL, 0, QueueThreadProc, (LPVOID)this, 0, &thread_id);
	if (!m_queueThread) {
		// cannot create thread
		logging->out_logf(LOG_ERROR, _T("CMidi::StartThread::CreateThread failed: %d"), GetLastError());
		return false;
	}

	return true;
}

void CMidi::StopThread()
{
	if (m_queueThread) {
		m_thread_working = false;
		SetEvent(m_queueEvent);
		WaitForSingleObject(m_queueThread, 1000);
		CloseHandle(m_queueThread);
		m_queueThread = NULL;
	}
	if (m_queueEvent) {
		CloseHandle(m_queueEvent);
		m_queueEvent = NULL;
	}
}

#endif /* USE_MIDI */
