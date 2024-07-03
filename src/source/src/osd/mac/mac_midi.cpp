/** @file mac_midi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.15 -

	@brief [ mac midi ]
*/

#include "mac_midi.h"

#ifdef USE_MIDI

#include "../SDL/sdl_emu.h"
#include "../../main.h"
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"
#include "../../version.h"

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
	mInClientRef = kMIDIInvalidUniqueID;
	mInPortRef = kMIDIInvalidUniqueID;
	mInEpRef = kMIDIInvalidUniqueID;
	mOutClientRef = kMIDIInvalidUniqueID;
	mOutPortRef = kMIDIInvalidUniqueID;
	mOutEpRef = kMIDIInvalidUniqueID;
	m_midiin_dev_id = -1;
	m_midiout_dev_id = -1;

	m_curr_message_idx = 0;
	for(int i=0; i<MESSAGES_MAX; i++) {
		m_message[i].packetList = (MIDIPacketList *)new uint8_t[PACKETLIST_BUFMAX];
	}

	m_queueMutex = NULL;
	m_queueCond = NULL;
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
	for(int i=0; i<MESSAGES_MAX; i++) {
		uint8_t *ptr = (uint8_t *)m_message[i].packetList;
		delete [] ptr;
	}
}

/// @brief for time keeper
uint32_t CMidi::GetCurrentSystemTime() const
{
	return (uint32_t)SDL_GetTicks();
}

void CMidi::RequestToSendQueue()
{
	if (m_queueCond) {
		// request to the queue thread to process this queue
		SDL_CondSignal(m_queueCond);
	}
}

void CMidi::Reset(bool power_on)
{
	ResetTimeKeeper();
}

void CMidi::GetEndpointPropertyName(MIDIEndpointRef epRef, char *buf, int size)
{
	OSStatus sts;
	CFStringRef strEpRef = nil;
	CFStringRef strDevRef = nil;
	char strEp[64];
	char strDev[64];

	sts = MIDIObjectGetStringProperty(epRef, kMIDIPropertyName, &strEpRef);
	if (sts != noErr) {
		return;
	}
	MIDIEntityRef entRef;
	MIDIDeviceRef devRef;
	MIDIEndpointGetEntity(epRef, &entRef);
	if (entRef) {
		MIDIEntityGetDevice(entRef, &devRef);
		if (devRef) {
			sts = MIDIObjectGetStringProperty(devRef, kMIDIPropertyName, &strDevRef);
		}
	}
	strEp[0] = 0;
	strDev[0] = 0;
	if (strEpRef) {
		CFStringGetCString(strEpRef, strEp, 64, kCFStringEncodingUTF8);
	}
	if (strDevRef) {
		CFStringGetCString(strDevRef, strDev, 64, kCFStringEncodingUTF8);
	}
	buf[0] = 0;
	if (strDev[0]) {
		UTILITY::strcat(buf, size, strDev);
	}
	if (strEp[0]) {
		if (strlen(buf) > 0) UTILITY::strcat(buf, size, " - ");
		UTILITY::strcat(buf, size, strEp);
	}
	if (strlen(buf) == 0) {
		UTILITY::strcpy(buf, size, "Unknown");
	}
}

int CMidi::EnumIn()
{
	ItemCount cnt;
	ItemCount idx;
	char name[64];

	cnt = MIDIGetNumberOfSources();

	m_inlist.Clear();
	for(idx=0; idx<cnt && idx<MIDI_MAX_PORTS; idx++) {
		MIDIEndpointRef epRef = MIDIGetSource(idx);
		if (!epRef) {
			continue;
		}
		GetEndpointPropertyName(epRef, name, sizeof(name));
		m_inlist.Add(new CMidiPort((int)idx, name));
	}
	return (int)cnt;
}

int CMidi::EnumOut()
{
	ItemCount cnt;
	ItemCount idx;
	char name[64];

	cnt = MIDIGetNumberOfDestinations();

	m_outlist.Clear();
	for(idx=0; idx<cnt && idx<MIDI_MAX_PORTS; idx++) {
		MIDIEndpointRef epRef = MIDIGetDestination(idx);
		if (!epRef) {
			continue;
		}
		GetEndpointPropertyName(epRef, name, sizeof(name));
		m_outlist.Add(new CMidiPort((int)idx, name));
	}
	return (int)cnt;
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
	OSStatus sts;
	char str[32];

	if (mInEpRef) return true;

	bool rc = true;
	m_mutexin.lock();
	do {
		// create midi client
		UTILITY::strcpy(str, sizeof(str), APP_INTERNAME);
		UTILITY::strcat(str, sizeof(str), "_srcClient");
		CFStringRef client_name = CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8);
		sts = MIDIClientCreate(client_name, &InProc, (void *)this, &mInClientRef);
		if (sts != noErr) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenIn::MIDIClientCreate failed:%d"), sts);
			rc = false;
			break;
		}

		// create midi-in port
		UTILITY::strcpy(str, sizeof(str), APP_INTERNAME);
		UTILITY::strcat(str, sizeof(str), "_srcPort");
		CFStringRef port_name = CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8);
		sts = MIDIInputPortCreate(mInClientRef, port_name, &InPortProc, (void *)this, &mInPortRef);
		if (sts != noErr) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenIn::MIDIInputPortCreate failed:%d"), sts);
			rc = false;
			break;
		}

		// get the end-point
		mInEpRef = MIDIGetSource((ItemCount)dev_id);
		if (!mInEpRef) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenIn::MIDIGetSource failed."));
			rc = false;
			break;
		}
		m_midiin_dev_id = dev_id;
		logging->out_logf(LOG_DEBUG, _T("Connected MIDI In: %d"), dev_id);
	} while(0);
	m_mutexin.unlock();
	return rc;
}

bool CMidi::OpenOut(int dev_id)
{
	OSStatus sts;
	char str[32];

	if (mOutEpRef) return true;

	bool rc = true;
	m_mutexout.lock();
	do {
		// create midi client
		UTILITY::strcpy(str, sizeof(str), APP_INTERNAME);
		UTILITY::strcat(str, sizeof(str), "_dstClient");
		CFStringRef client_name = CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8);
		sts = MIDIClientCreate(client_name, &OutProc, (void *)this, &mOutClientRef);
		if (sts != noErr) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenOut::MIDIClientCreate failed:%d"), sts);
			rc = false;
			break;
		}

		// create midi-out port
		UTILITY::strcpy(str, sizeof(str), APP_INTERNAME);
		UTILITY::strcat(str, sizeof(str), "_dstPort");
		CFStringRef port_name = CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8);
		sts = MIDIOutputPortCreate(mOutClientRef, port_name, &mOutPortRef);
		if (sts != noErr) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenOut::MIDIOutputPortCreate failed:%d"), sts);
			rc = false;
			break;
		}

		// get the end-point
		mOutEpRef = MIDIGetDestination((ItemCount)dev_id);
		if (!mOutEpRef) {
			logging->out_logf(LOG_ERROR, _T("CMidi::OpenOut::MIDIGetDestination failed."));
			rc = false;
			break;
		}
		m_midiout_dev_id = dev_id;
		logging->out_logf(LOG_DEBUG, _T("Connected MIDI Out: %d"), dev_id);
	} while(0);
	m_mutexout.unlock();
	return rc;
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
	return (mInPortRef != kMIDIInvalidUniqueID);
}

bool CMidi::IsOpenedOut() const
{
	return (mOutPortRef != kMIDIInvalidUniqueID);
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

void CMidi::InPortProc(const MIDIPacketList *pktList, void *readProcRefCon, void *srcConnRefCon)
{
//	CMidi *midi = (CMidi *)readProcRefCon;
}

void CMidi::InProc(const MIDINotification *message, void *refCon)
{
//	CMidi *midi = (CMidi *)refCon;
}

void CMidi::OutProc(const MIDINotification *message, void *refCon)
{
#if 0
	CMidi *midi = (CMidi *)refCon;
	switch(message->messageID) {
//	case kMIDIMsgObjectRemoved:
//		{
//			const MIDIObjectAddRemoveNotification *m = (const MIDIObjectAddRemoveNotification *)message;
//		}
//		break;
	default:
		break;
	}
#endif
}

void CMidi::CloseIn()
{
	if (!mInEpRef) return;

	m_mutexin.lock();
	mInEpRef = kMIDIInvalidUniqueID;

	if (mInPortRef) {
		MIDIPortDispose(mInPortRef);
		mInPortRef = kMIDIInvalidUniqueID;
		logging->out_log(LOG_DEBUG, _T("Disconnected MIDI In"));
	}
	if (mInClientRef) {
		MIDIClientDispose(mInClientRef);
		mInClientRef = kMIDIInvalidUniqueID;
	}
	m_midiin_dev_id = -1;
	m_mutexin.unlock();
}

void CMidi::CloseOut()
{
	if (!mOutEpRef) return;

	m_mutexout.lock();
	mOutEpRef = kMIDIInvalidUniqueID;

	if (mOutPortRef) {
		MIDIPortDispose(mOutPortRef);
		mOutPortRef = kMIDIInvalidUniqueID;
		logging->out_log(LOG_DEBUG, _T("Disconnected MIDI Out"));
	}
	if (mOutClientRef) {
		MIDIClientDispose(mOutClientRef);
		mOutClientRef = kMIDIInvalidUniqueID;
	}
	m_midiout_dev_id = -1;
	m_mutexout.unlock();
}

void CMidi::ClosedIn()
{
}

void CMidi::ClosedOut()
{
}

bool CMidi::StartIn()
{
	OSStatus sts = MIDIPortConnectSource(mInPortRef, mInEpRef, nil);
	if (sts != noErr) {
		logging->out_logf(LOG_ERROR, ("CMidi::StartIn:MIDIPortConnectSource failed: %d"), sts);
		return false;
	}
	return true;
}

bool CMidi::StopIn()
{
	OSStatus sts = MIDIPortDisconnectSource(mInPortRef, mInEpRef);
	if (sts != noErr) {
		logging->out_logf(LOG_ERROR, ("CMidi::StopIn:MIDIPortDisconnectSource failed: %d"), sts);
		return false;
	}
	return true;
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
	OSStatus res = noErr;

	struct st_message *m = &m_message[m_curr_message_idx];

//	if (!(m->msg.dwFlags & MHDR_DONE)) {
//		// buffer is full, so lost the data
//		return;
//	}

//	logging->out_logf(LOG_DEBUG, _T("midiOut:idx%d flag:%x"), m_curr_message_idx, m->msg.dwFlags);
//	uint64_t	data_time = AudioGetCurrentHostTime();
//		data_time = AudioConvertHostTimeToNanos(data_time) + kSecondScale * 1;
//		data_time = AudioConvertNanosToHostTime(data_time);
	
	MIDIPacket *packet = MIDIPacketListInit(m->packetList);

	packet = MIDIPacketListAdd(m->packetList, PACKETLIST_BUFMAX, packet
							   , 0, length, buffer);

	m_mutexout.lock();
	if (mOutEpRef) {
		// send
		res = MIDISend(mOutPortRef, mOutEpRef, m->packetList);
	}
	m_mutexout.unlock();

	if (res != noErr) {
		logging->out_logf(LOG_ERROR, _T("MIDISend failed: %d"), (int)res);
	}

	// clean next message
	m_curr_message_idx++;
	if (m_curr_message_idx >= MESSAGES_MAX) {
		m_curr_message_idx = 0;
	}
//	m = &m_message[m_curr_message_idx];
}

#if 0
void CMidi::SendData(uint8_t data)
{
	OSStatus res;

	int rc = ParseData(data);

	if (!rc) {
		// wait for next data
		return;
	}

	uint32_t vtime = GetDelayedVmTime();

	struct st_message *m = &m_message[m_curr_message_idx];

	if (rc >= 0x100) {
		// normal or system message
		m_queue.Push(vtime, GetBuffer(), GetBufferLength());
		ClearBuffer();
	} else {
		// realtime message
		m_queue.Push(vtime, rc & 0xff);
	}

//	uint64_t	data_time = AudioGetCurrentHostTime();
//		data_time = AudioConvertHostTimeToNanos(data_time) + kSecondScale * 1;
//		data_time = AudioConvertNanosToHostTime(data_time);
	uint8_t *buffer;
	int length = m_queue.Pop(&buffer);
		
	MIDIPacket *packet = MIDIPacketListInit(m->packetList);

	packet = MIDIPacketListAdd(m->packetList, PACKETLIST_BUFMAX, packet
							   , 0, length, buffer);

	if (mOutEpRef) {
		// send
		res = MIDISend(mOutPortRef, mOutEpRef, m->packetList);
		if (res != noErr) {
			return;
		}
	}

	// clean next message
	m_curr_message_idx++;
	if (m_curr_message_idx >= MESSAGES_MAX) {
		m_curr_message_idx = 0;
	}
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

int CMidi::QueueThreadProc(void *ptr)
{
	CMidi *midi = (CMidi *)ptr;

	return midi->QueueThread();
}

int CMidi::QueueThread()
{
	int wait_result;
	int count;
	uint32_t cur_time;
	uint8_t *buffer;

	while(m_thread_working) {
		// wait to arrive new queue
		SDL_LockMutex(m_queueMutex);
		wait_result = SDL_CondWait(m_queueCond, m_queueMutex);
		SDL_UnlockMutex(m_queueMutex);
		if (wait_result != 0) {
			// error occured?
			m_thread_working = false;
			break;
		}
		// queue exists?
		while ((count = m_queue.Count()) > 0) {
			// arrived?
			cur_time = (uint32_t)SDL_GetTicks();
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
					SDL_Delay(t);
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
	// create mutex
	m_queueMutex = SDL_CreateMutex();
	if (!m_queueMutex) {
		logging->out_logf(LOG_ERROR, _T("CMidi::StartThread::SDL_CreateMutex failed: %s"), SDL_GetError());
		return false;
	}
	
	// create condition for message queue
	m_queueCond = SDL_CreateCond();
	if (!m_queueCond) {
		logging->out_logf(LOG_ERROR, _T("CMidi::StartThread::SDL_CreateCond failed: %s"), SDL_GetError());
		return false;
	}

	m_thread_working = true;

	// create thread
	m_queueThread = SDL_CreateThread(QueueThreadProc, "QueueThread", (void *)this);
	if (!m_queueThread) {
		logging->out_logf(LOG_ERROR, _T("CMidi::StartThread::SDL_CreateThread failed: %s"), SDL_GetError());
		return false;
	}

	return true;
}

void CMidi::StopThread()
{
	if (m_queueThread) {
		int sts;
		m_thread_working = false;
		SDL_CondSignal(m_queueCond);
		SDL_WaitThread(m_queueThread, &sts);
		m_queueThread = NULL;
	}
	if (m_queueCond) {
		SDL_DestroyCond(m_queueCond);
		m_queueCond = NULL;
	}
	if (m_queueMutex) {
		SDL_DestroyMutex(m_queueMutex);
		m_queueMutex = NULL;
	}
}

#endif /* USE_MIDI */
