/** @file mouse.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22

	@brief [ mouse ]
*/

#include "mouse.h"
#include "mfp.h"
#include "scc.h"
#include "../../fileio.h"

#ifdef _DEBUG_MOUSE
#define OUT_DEBUG logging->out_debugf
#else
#define OUT_DEBUG(...)
#endif

void MOUSE::initialize()
{
	p_mouse_stat = emu->mouse_buffer();

	m_register_id = -1;

	register_frame_event(this);
}

void MOUSE::reset()
{
	m_phase = 0;

	m_register_id = -1;
	m_scc_rtsb = 1;

	memset(m_stat, 0, sizeof(m_stat));
	memset(m_stat_lock, 0, sizeof(m_stat_lock));

}

void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch (id) {
		case SCC::SIG_RTSB:
			// request to send mouse status
			if (m_scc_rtsb != 0 && (data & mask) == 0) {
				req_to_send_mouse_status(EVENT_MOUSE_STAT_FROM_SCC);
			}
			m_scc_rtsb = (data & mask);
			break;

		case SIG_RTS_TO_MFP:
			// request to send mouse status
			req_to_send_mouse_status(EVENT_MOUSE_STAT_FROM_MFP);
			break;

		case SIG_CPU_RESET:
			now_reset = ((data & mask) != 0);
			reset();
			break;
	}
}

void MOUSE::update_mouse()
{
	int i;
//	bool now_irq = false;

	m_stat[0] = 0;
	m_stat[1] = 0;
	m_stat[2] = 0;

#ifdef USE_KEY_RECORD
	reckey->processing_mouse_status(p_mouse_stat);
#endif

	if (FLG_USEMOUSE != 0 || pConfig->reckey_playing) {
		for(i = 0; i < 2; i++) {
#ifndef USE_MOUSE_ABSOLUTE
//			int sum = (int8_t)m_stat[i + 1];
			int sum = p_mouse_stat[i];
			if (sum > 127) {
				m_stat[0] |= (0x10 << (i << 1));	// overflow flag
				m_stat[i + 1] = (uint8_t)(127);
			} else if (sum < -128) {
				m_stat[0] |= (0x20 << (i << 1));	// underflow flag
				m_stat[i + 1] = (uint8_t)(-128);
			} else {
				m_stat[i + 1] = (uint8_t)sum;
			}
#endif
		}

		// buttons
		for(i = 0; i < 2; i++) {
			m_stat[0] |= ((p_mouse_stat[2] & (1 << i)) ? (0x01 << i) : 0);
		}
	}

	OUT_DEBUG(_T("mouse_stat %4d %4d %08X  mst %02X %d %d")
		,p_mouse_stat[0],p_mouse_stat[1],p_mouse_stat[2],m_stat[0],(int8_t)m_stat[1],(int8_t)m_stat[2]);
}

void MOUSE::req_to_send_mouse_status(int event_id)
{
	// send status
	m_phase = 0;
	// decide status
	memcpy(m_stat_lock, m_stat, sizeof(m_stat_lock));
	// clear current status
	memset(m_stat, 0, sizeof(m_stat));
	// 4800baud * (1start + 8bits + 2stop)
	register_event(this, event_id, 2291.67, false, &m_register_id);
}

void MOUSE::send_mouse_status(int event_id)
{
	DEVICE *dev;
	int sig_id;

	if (event_id == EVENT_MOUSE_STAT_FROM_SCC) {
		dev = d_scc;
		sig_id = SCC::SIG_RXDB;
	} else {
		dev = d_mfp;
		sig_id = MFP::SIG_SI;
	}

	// send status
	dev->write_signal(sig_id, m_stat_lock[m_phase], 0xff);

	OUT_DEBUG(_T("mouse sent: %d %4d") , m_phase, m_stat_lock[m_phase]);

	m_phase++;
	if (m_phase <= 2) {
		// 4800baud * (1start + 8bits + 2stop)
		register_event(this, event_id, 2291.67, false, &m_register_id);
	}
}


#if 0
void MOUSE::set_mouse_position(int px, int py)
{
}
#endif

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MOUSE::event_frame()
{
	update_mouse();
}

void MOUSE::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MOUSE_STAT_FROM_MFP:
	case EVENT_MOUSE_STAT_FROM_SCC:
		m_register_id = -1;
		send_mouse_status(event_id);
		break;
	}
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)
#define SET_Double_LE(v) { pair64_t tmp; tmp.db = v; vm_state.v = Uint64_LE(tmp.u64); }

void MOUSE::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<3; i++) {
		SET_Byte(m_stat[i]);		// update current status
		SET_Byte(m_stat_lock[i]);	// locked status to send
	}
	SET_Byte(m_scc_rtsb);		// RTS from scc

	SET_Int32_LE(m_register_id);	// send mouse status
	SET_Int32_LE(m_phase);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)
#define GET_Double_LE(v) { pair64_t tmp; tmp.u64 = Uint64_LE(vm_state.v); v = tmp.db; }

bool MOUSE::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	for(int i=0; i<3; i++) {
		GET_Byte(m_stat[i]);		// update current status
		GET_Byte(m_stat_lock[i]);	// locked status to send
	}
	GET_Byte(m_scc_rtsb);		// RTS from scc

	GET_Int32_LE(m_register_id);	// send mouse status
	GET_Int32_LE(m_phase);

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
#endif

