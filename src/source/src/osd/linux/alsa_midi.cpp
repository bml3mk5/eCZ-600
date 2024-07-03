/** @file alsa_midi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.15 -

	@brief [ alsa midi ]
*/

#include "alsa_midi.h"

#ifdef USE_MIDI

#include "../SDL/sdl_emu.h"
#include "../../main.h"
#include "../../config.h"
#include "../../vm/device.h"
#include "../../utility.h"
#include "../../version.h"

//#define DEBUG_QUEUE

extern EMU *emu;

#define CreateDeviceID(cardnum, devnum, subnum) (cardnum) + (devnum) * 256 + (subnum) * 65536
#define GetCardNumberById(id) ((id) & 0xff)
#define GetDeviceNumberById(id) (((id) >> 8) & 0xff)
#define GetSubNumberById(id) (((id) >> 16) & 0xff)

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
	mInPort = NULL;
	mOutPort = NULL;
	m_midiin_dev_id = -1;
	m_midiout_dev_id = -1;

	m_curr_message_idx = 0;
	for(int i=0; i<MESSAGES_MAX; i++) {
		m_message[i].buffer = NULL;
		m_message[i].length = 0;
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

int CMidi::EnumSubDevices(snd_ctl_t *ctl, int card, int device, bool is_input, CMidiPortList &list)
{
	snd_rawmidi_info_t *info;
	const char *name;
	const char *sub_name;
	int err;

	snd_rawmidi_stream_t stdir = (is_input ? SND_RAWMIDI_STREAM_INPUT : SND_RAWMIDI_STREAM_OUTPUT);

	snd_rawmidi_info_alloca(&info);
	snd_rawmidi_info_set_device(info, device);

	snd_rawmidi_info_set_stream(info, stdir);
	err = snd_ctl_rawmidi_info(ctl, info);
	if (err < 0) {
		// no information
		return 0;
	}
	int subs = snd_rawmidi_info_get_subdevices_count(info);

	char buf[64];

	for (int sub = 0; sub < subs; sub++) {
		snd_rawmidi_info_set_subdevice(info, sub);
		err = snd_ctl_rawmidi_info(ctl, info);
		if (err < 0) {
			logging->out_logf(LOG_ERROR, _T("CMidi::EnumDevice failed:%d:%d:%d: %s")
				, card, device, sub, snd_strerror(err));
			continue;
		}
		name = snd_rawmidi_info_get_name(info);
		sub_name = snd_rawmidi_info_get_subdevice_name(info);

		int id = 0;
		buf[0] = 0;
		if (sub == 0 && sub_name[0] == '\0') {
			id = CreateDeviceID(card, device, 255);
			UTILITY::sprintf(buf, sizeof(buf), "%s - %d", name, device);
			list.Add(new CMidiPort((int)id, buf));
			break; // 1subdevice

		} else {
			id = CreateDeviceID(card, device, sub);
			UTILITY::strcpy(buf, sizeof(buf), name);
			UTILITY::strcat(buf, sizeof(buf), " - ");
			UTILITY::strcat(buf, sizeof(buf), sub_name);
			list.Add(new CMidiPort((int)id, buf));
		}
	}

	return subs;
}

int CMidi::EnumDevices(int card, bool is_input, CMidiPortList &list)
{
	char name[32];
	int err;

	int devs = 0;
	snd_ctl_t *ctl = NULL;
	sprintf(name, "hw:%d", card);
	if ((err = snd_ctl_open(&ctl, name, 0)) < 0) {
		logging->out_logf(LOG_ERROR, _T("CMidi::EnumCardDevices::snd_ctl_open failed %d: %s"), card, snd_strerror(err));
		return devs;
	}

	int device = -1;
	for (;;) {
		if (snd_ctl_rawmidi_next_device(ctl, &device) < 0) {
			break;
		}
		if (device < 0) {
			break;
		}
		EnumSubDevices(ctl, card, device, is_input, list);
		devs++;
	}
	snd_ctl_close(ctl);
	return devs;
}

int CMidi::Enum(bool is_input, CMidiPortList &list)
{
	int card = -1;
	for (;;) {
		if (snd_card_next(&card) < 0) {
			break;
		}
		if (card < 0) {
			break;
		}
		EnumDevices(card, is_input, list);
	}
	return list.Count();
}

int CMidi::EnumIn()
{
	m_inlist.Clear();
	return Enum(true, m_inlist);
}

int CMidi::EnumOut()
{
	m_outlist.Clear();
	return Enum(false, m_outlist);
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

void CMidi::MakeDeviceName(int id, char *name, int size)
{
	int card = GetCardNumberById(id);
	int device = GetDeviceNumberById(id);
	int subnum = GetSubNumberById(id);
	if (subnum < 255)  {
		UTILITY::sprintf(name, size, "hw:%d,%d,%d"
			, card, device, subnum);
	} else {
		UTILITY::sprintf(name, size, "hw:%d,%d"
			, card, device);
	}
}

bool CMidi::OpenIn(int dev_id)
{
	int sts;

	if (mInPort) return true;

	if (m_inlist.Count() <= dev_id) {
		return false;
	}
	int id = m_inlist.Item(dev_id)->GetID();
	char device_name[32];
	MakeDeviceName(id, device_name, (int)sizeof(device_name));
	int mode = SND_RAWMIDI_NONBLOCK; // SND_RAWMIDI_SYNC

	m_mutexin.lock();
	sts = snd_rawmidi_open(&mInPort, NULL, device_name, mode);
	m_mutexin.unlock();
	if (sts < 0) {
		logging->out_logf(LOG_ERROR, _T("Cannot connect MIDI Input %s: %s"),device_name,snd_strerror(sts));
		return false;
	}
	logging->out_logf(LOG_DEBUG, _T("Connected MIDI In: %s"), device_name);
	m_midiin_dev_id = dev_id;
	return true;
}

bool CMidi::OpenOut(int dev_id)
{
	int sts;

	if (mOutPort) return true;

	if (m_outlist.Count() <= dev_id) {
		return false;
	}

	int id = m_outlist.Item(dev_id)->GetID();
	char device_name[32];
	MakeDeviceName(id, device_name, (int)sizeof(device_name));
	int mode = SND_RAWMIDI_NONBLOCK; // SND_RAWMIDI_SYNC

	m_mutexout.lock();
	sts = snd_rawmidi_open(NULL, &mOutPort, device_name, mode);
	m_mutexout.unlock();
	if (sts < 0) {
		logging->out_logf(LOG_ERROR, _T("Cannot connect MIDI Output %s: %s"),device_name,snd_strerror(sts));
		return false;
	}
	logging->out_logf(LOG_DEBUG, _T("Connected MIDI Out: %s"), device_name);
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
	return (mInPort != NULL);
}

bool CMidi::IsOpenedOut() const
{
	return (mOutPort != NULL);
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

void CMidi::CloseIn()
{
	if (!mInPort) return;

	m_mutexin.lock();
	snd_rawmidi_drop(mInPort);
	snd_rawmidi_close(mInPort);
	mInPort = NULL;
	m_midiin_dev_id = -1;
	m_mutexin.unlock();

	logging->out_log(LOG_DEBUG, _T("Disconnected MIDI In"));
}

void CMidi::CloseOut()
{
	if (!mOutPort) return;

	m_mutexout.lock();
	snd_rawmidi_drop(mOutPort);
	snd_rawmidi_close(mOutPort);
	mOutPort = NULL;
	m_midiout_dev_id = -1;
	m_mutexout.unlock();

	logging->out_log(LOG_DEBUG, _T("Disconnected MIDI Out"));
}

void CMidi::ClosedIn()
{
}

void CMidi::ClosedOut()
{
}

bool CMidi::StartIn()
{
	return false;
}

bool CMidi::StopIn()
{
	return false;
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
	int err = 0;

	struct st_message *m = &m_message[m_curr_message_idx];
	m->buffer = buffer;
	m->length = length;

	m_mutexout.lock();
	if (mOutPort) {
		// send
		err = snd_rawmidi_write(mOutPort, m->buffer, m->length);
//		snd_rawmidi_drain(mOutPort);
	}
	m_mutexout.unlock();

	if (err < 0) {
		logging->out_logf(LOG_ERROR, _T("snd_rawmidi_write failed: %d"), err);
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
