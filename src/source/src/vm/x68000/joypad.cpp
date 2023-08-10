/** @file joypad.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.22 -

	@brief [ joypad ]
*/

#include "joypad.h"
#include "../../emu.h"
#include "../device.h"
#include "../i8255.h"
#include "keyrecord.h"
#include "../../config.h"

enum en_joy_flags {
	JOY_ALLOW =   0x000000f,
	JOY_YAALLOW = 0x00000f0,
	JOY_BTN_A =   0x0010000,
	JOY_BTN_B =   0x0020000,
	JOY_BTN_C =   0x0040000,
	JOY_BTN_D =   0x0080000,
	JOY_BTN_X =   0x0080000, // same as D
	JOY_BTN_E1 =  0x0100000,
	JOY_BTN_Y =   0x0100000, // same as E1
	JOY_BTN_E2 =  0x0200000,
	JOY_BTN_Z =   0x0200000, // same as E2
	JOY_BTN_L =   0x0400000,
	JOY_BTN_R =   0x0800000,
	JOY_BTN_SEL = 0x1000000,
	JOY_BTN_STA = 0x2000000,

	JOY_YAALLOW_SFT = 4,
	JOY_BTN_A_SFT =   16,
	JOY_BTN_B_SFT =   17,
	JOY_BTN_C_SFT =   18,
	JOY_BTN_D_SFT =   19,
	JOY_BTN_X_SFT =   19,
	JOY_BTN_E1_SFT =  20,
	JOY_BTN_Y_SFT =   20,
	JOY_BTN_E2_SFT =  21,
	JOY_BTN_Z_SFT =   21,
	JOY_BTN_L_SFT =   22,
	JOY_BTN_R_SFT =   23,
	JOY_BTN_SEL_SFT = 24,
	JOY_BTN_STA_SFT = 25,
};

//
//
//
JOYPAD_BASE::JOYPAD_BASE(DEVICE *pio, int idx, uint32_t *joy_stat)
{
	m_idx = idx;
	d_pio = pio;
#ifdef USE_KEY_RECORD
	p_reckey = NULL;
#endif

	d_event = NULL;
	m_event_id = 0;
	p_register_id = NULL;

	p_joy_stat = joy_stat;
	m_joy_flags = 0;
	m_joy_flags_mask = (0x10 << idx);
}

JOYPAD_BASE::~JOYPAD_BASE()
{
}

#ifdef USE_KEY_RECORD
void JOYPAD_BASE::set_keyrecord_ptr(KEYRECORD *reckey)
{
	p_reckey = reckey;
}
#endif

void JOYPAD_BASE::set_event_param(DEVICE *dev, int event_id, int *register_id)
{
	d_event = dev;
	m_event_id = event_id;
	p_register_id = register_id;

}

void JOYPAD_BASE::register_event(double usec)
{
	d_event->register_event(d_event, m_event_id, usec, false, p_register_id);
}

void JOYPAD_BASE::reset()
{
}

void JOYPAD_BASE::update_joy()
{
}

void JOYPAD_BASE::modify_flags(uint8_t flags)
{
}

void JOYPAD_BASE::event_callback(int id)
{
}

void JOYPAD_BASE::save_state(uint8_t *data, int size)
{
	data[0] = m_joy_flags;
}

void JOYPAD_BASE::load_state(const uint8_t *data, int size)
{
	m_joy_flags = data[0];
}

//
// standard
//
JOYPAD_STD::JOYPAD_STD(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_BASE(pio, idx, joy_stat)
{
	m_joy_stat = 0;
	m_joy_stat_mask = ~0;
}

JOYPAD_STD::~JOYPAD_STD()
{
}

void JOYPAD_STD::reset()
{
	m_joy_stat = 0;
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat, 0xff);
}

void JOYPAD_STD::update_joy()
{
	m_joy_stat = 0;

	if (FLG_PIAJOY_ALL) {
		m_joy_stat = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_A | JOY_BTN_B)) >> (JOY_BTN_A_SFT - 5));
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, &m_joy_stat, 1);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat & m_joy_stat_mask), 0xff);	// negative
}

void JOYPAD_STD::modify_flags(uint8_t flags)
{
	if (flags & m_joy_flags_mask) {
		m_joy_stat_mask = 0;
	} else {
		m_joy_stat_mask = ~0;
	}
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat & m_joy_stat_mask), 0xff);	// negative

	m_joy_flags = flags;
}

void JOYPAD_STD::save_state(uint8_t *data, int size)
{
	JOYPAD_BASE::save_state(data, size);

	data[1] = (uint8_t)m_joy_stat;
	data[2] = m_joy_stat_mask ? 0xff : 0;
}

void JOYPAD_STD::load_state(const uint8_t *data, int size)
{
	JOYPAD_BASE::load_state(data, size);

	m_joy_stat = data[1];
	m_joy_stat_mask = data[2] ? ~0 : 0;
}

//
// megadrive 3buttons
//
JOYPAD_MD3::JOYPAD_MD3(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_BASE(pio, idx, joy_stat)
{
	m_joy_idx = 0;
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;
}

JOYPAD_MD3::~JOYPAD_MD3()
{
}

void JOYPAD_MD3::reset()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[0], 0xff);
}

void JOYPAD_MD3::update_joy()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;

	if (FLG_PIAJOY_ALL) {
		// SEL L
		// - START A - (L) (L) DOWN UP
		m_joy_stat[0] = (p_joy_stat[0] & JOY_ALLOW) | 0x0c
			| ((p_joy_stat[0] & JOY_BTN_A) >> (JOY_BTN_A_SFT - 5))
			| ((p_joy_stat[0] & JOY_BTN_STA) >> (JOY_BTN_STA_SFT - 6));

		// SEL H
		// - C B - RIGHT LEFT DOWN UP
		m_joy_stat[1] = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_C | JOY_BTN_B)) >> (JOY_BTN_B_SFT - 5));
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 2);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative
}

void JOYPAD_MD3::modify_flags(uint8_t flags)
{
	if (flags & m_joy_flags_mask) {
		m_joy_idx = 1;
	} else {
		m_joy_idx = 0;
	}
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative

	m_joy_flags = flags;
}

void JOYPAD_MD3::save_state(uint8_t *data, int size)
{
	JOYPAD_BASE::save_state(data, size);

	data[1] = (uint8_t)m_joy_idx;
	for(int i=0; i<2; i++) {
		data[i+2] = (uint8_t)m_joy_stat[i];
	}
}

void JOYPAD_MD3::load_state(const uint8_t *data, int size)
{
	JOYPAD_BASE::load_state(data, size);

	m_joy_idx = data[1];
	for(int i=0; i<2; i++) {
		m_joy_stat[i] = data[i+2];
	}
}

//
// megadrive 6buttons
//
JOYPAD_MD6::JOYPAD_MD6(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_BASE(pio, idx, joy_stat)
{
	m_joy_idx = 0;
	m_joy_cnt = 0;
	m_prev_clk = 0;
	for(int i=0; i<(int)(sizeof(m_joy_stat)/sizeof(m_joy_stat[0])); i++) {
		m_joy_stat[i] = 0;
	}
	m_sub_clk = CPU_CLOCKS * 12 / 10000;	// 1.2msec
}

JOYPAD_MD6::~JOYPAD_MD6()
{
}

void JOYPAD_MD6::reset()
{
	m_joy_idx = 0;
	m_joy_cnt = 0;
	m_prev_clk = 0;
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;
	m_joy_stat[3] = 0;
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[0], 0xff);
}

void JOYPAD_MD6::update_joy()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;
	m_joy_stat[3] = 0;

	if (FLG_PIAJOY_ALL) {
		// - START A - (L) (L) DOWN UP
		m_joy_stat[0] = (p_joy_stat[0] & JOY_ALLOW) | 0x0c
			| ((p_joy_stat[0] & JOY_BTN_A) >> (JOY_BTN_A_SFT - 5))
			| ((p_joy_stat[0] & JOY_BTN_STA) >> (JOY_BTN_STA_SFT - 6));
		m_joy_stat[2] = m_joy_stat[0] | 0x0f;
		m_joy_stat[4] = m_joy_stat[0] & 0xf0;

		// - C B - RIGHT LEFT DOWN UP
		m_joy_stat[1] = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_C | JOY_BTN_B)) >> (JOY_BTN_B_SFT - 5));

		// - Z Y - X SELECT (H) (H)
		m_joy_stat[3] = ((p_joy_stat[0] & (JOY_BTN_Z | JOY_BTN_Y)) >> (JOY_BTN_Y_SFT - 5))
			| ((p_joy_stat[0] & JOY_BTN_X) >> (JOY_BTN_X_SFT - 3))
			| ((p_joy_stat[0] & JOY_BTN_SEL) >> (JOY_BTN_SEL_SFT - 2));
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 5);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative
}

void JOYPAD_MD6::modify_flags(uint8_t flags)
{
	uint64_t cur_clk = emu->get_current_clock();

	if (flags & m_joy_flags_mask) {
		m_joy_idx = 1;
		if (cur_clk < (m_prev_clk + m_sub_clk)) {
			m_joy_cnt++;
			if (m_joy_cnt == 7) {
				m_joy_idx = 1;
				m_joy_cnt = 0;
			} else if (m_joy_cnt == 5) {
				m_joy_idx = 3;
			}
		} else {
			m_joy_cnt = 0;
		}
	} else {
		m_joy_idx = 0;
		if (cur_clk < (m_prev_clk + m_sub_clk)) {
			m_joy_cnt++;
			if (m_joy_cnt == 6) {
				m_joy_idx = 4;
			} else if (m_joy_cnt == 4) {
				m_joy_idx = 2;
				m_prev_clk = cur_clk;
			}
		} else {
			m_joy_cnt = 0;
			m_prev_clk = cur_clk;
		}
	}

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative

	m_joy_flags = flags;
}

void JOYPAD_MD6::save_state(uint8_t *data, int size)
{
	JOYPAD_BASE::save_state(data, size);

	data[1] = (uint8_t)m_joy_idx;
	data[2] = (uint8_t)m_joy_cnt;
	for(int i=0; i<5; i++) {
		data[i+3] = (uint8_t)m_joy_stat[i];
	}

	uint64_t *data_u64 = (uint64_t *)data;
	data_u64[1] = m_prev_clk;
	data_u64[2] = m_sub_clk;
}

void JOYPAD_MD6::load_state(const uint8_t *data, int size)
{
	JOYPAD_BASE::load_state(data, size);

	m_joy_idx = data[1];
	m_joy_cnt = data[2];
	for(int i=0; i<5; i++) {
		m_joy_stat[i] = data[i+3];
	}

	uint64_t *data_u64 = (uint64_t *)data;
	m_prev_clk = data_u64[1];
	m_sub_clk = data_u64[2];
}

//
// capcom power stick fighter
//
JOYPAD_CPSF::JOYPAD_CPSF(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_MD3(pio, idx, joy_stat)
{
}

void JOYPAD_CPSF::update_joy()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;

	if (FLG_PIAJOY_ALL) {
		// SEL L
		// - B A - RIGHT LEFT DOWN UP
		m_joy_stat[0] = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_B | JOY_BTN_A)) >> (JOY_BTN_A_SFT - 5));

		// SEL H
		// - START L - SELECT X Y R
		m_joy_stat[1] = (p_joy_stat[0] & (JOY_BTN_R));		// R
		m_joy_stat[1] >>= (JOY_BTN_R_SFT - (JOY_BTN_SEL_SFT - 3)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_SEL));	// select
		m_joy_stat[1] >>= (JOY_BTN_SEL_SFT - 3 - (JOY_BTN_Y_SFT - 1)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_STA | JOY_BTN_Y));	// start Y
		m_joy_stat[1] >>= (JOY_BTN_Y_SFT - 1 - (JOY_BTN_X_SFT - 2)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_L | JOY_BTN_X));		// L X
		m_joy_stat[1] >>= (JOY_BTN_X_SFT - 2);
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 2);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative

//	logging->out_debugf(_T("JOYPAD_CPSF: i:%d 0:%04X 1:%04X"), m_joy_idx, m_joy_stat[0], m_joy_stat[1]);
}

//
// megadrive 6 buttons on capcom power stick fighter
//
JOYPAD_CPSF_MD::JOYPAD_CPSF_MD(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_MD3(pio, idx, joy_stat)
{
}

void JOYPAD_CPSF_MD::update_joy()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;

	if (FLG_PIAJOY_ALL) {
		// SEL L
		// - B A - RIGHT LEFT DOWN UP
		m_joy_stat[0] = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_B | JOY_BTN_A)) >> (JOY_BTN_A_SFT - 5));

		// SEL H
		// - START C - SELECT X Y Z
		m_joy_stat[1] = (p_joy_stat[0] & (JOY_BTN_SEL | JOY_BTN_Z));	// select Z
		m_joy_stat[1] >>= (JOY_BTN_Z_SFT - (JOY_BTN_Y_SFT - 1)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_STA | JOY_BTN_Y));	// start Y
		m_joy_stat[1] >>= (JOY_BTN_Y_SFT - 1 - (JOY_BTN_X_SFT - 2)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_X));	// X
		m_joy_stat[1] >>= (JOY_BTN_X_SFT - 2 - (JOY_BTN_C_SFT - 5)); // 4
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_C));	// C
		m_joy_stat[1] >>= (JOY_BTN_C_SFT - 5);	// 5
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 2);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative

//	logging->out_debugf(_T("JOYPAD_CPSF_MD: i:%d 0:%04X 1:%04X"), m_joy_idx, m_joy_stat[0], m_joy_stat[1]);
}

//
// micomsoft XPD-1LR
//
JOYPAD_XPDLR::JOYPAD_XPDLR(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_MD3(pio, idx, joy_stat)
{
}

void JOYPAD_XPDLR::update_joy()
{
	m_joy_stat[0] = 0;
	m_joy_stat[1] = 0;

	if (FLG_PIAJOY_ALL) {
		// SEL L
		// - B A - L_RIGHT L_LEFT L_DOWN L_UP
		m_joy_stat[0] = (p_joy_stat[0] & JOY_ALLOW)
			| ((p_joy_stat[0] & (JOY_BTN_B | JOY_BTN_A)) >> (JOY_BTN_A_SFT - 5));

		// SEL H
		// - B A - R_RIGHT R_LEFT R_DOWN R_UP
		m_joy_stat[1] |= (p_joy_stat[0] & (JOY_BTN_B | JOY_BTN_A));
		m_joy_stat[1] >>= (JOY_BTN_A_SFT - 5 - JOY_YAALLOW_SFT);
		m_joy_stat[1] = (p_joy_stat[0] & JOY_YAALLOW);
		m_joy_stat[1] >>= JOY_YAALLOW_SFT;
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 2);
#endif

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~m_joy_stat[m_joy_idx], 0xff);	// negative

//	logging->out_debugf(_T("JOYPAD_CPSF: i:%d 0:%04X 1:%04X"), m_joy_idx, m_joy_stat[0], m_joy_stat[1]);
}

//
// cyber stick (analog mode)
//
JOYPAD_CYBER::JOYPAD_CYBER(DEVICE *pio, int idx, uint32_t *joy_stat)
	: JOYPAD_BASE(pio, idx, joy_stat)
{
	m_phase = 0;
	m_joy_idx = 0;
	for(int i=0; i<12; i++) {
		m_joy_stat[i] = 0;
	}
	m_ack_sel = 0;
	m_speed = 1;
	m_next_speed = 1;
}

JOYPAD_CYBER::~JOYPAD_CYBER()
{
}

void JOYPAD_CYBER::reset()
{
	m_phase = 0;
	for(int i=0; i<12; i++) {
		m_joy_stat[i] = 0;
	}
	m_ack_sel = 0;
	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat[0] | m_ack_sel), 0xff);
}

void JOYPAD_CYBER::update_joy()
{
	for(int i=0; i<12; i++) {
		m_joy_stat[i] = 0;
	}

	if (FLG_PIAJOY_ALL) {
		// - - - - A B C D
		m_joy_stat[0] = ((p_joy_stat[0] & JOY_BTN_A) >> (JOY_BTN_A_SFT - 3))
			| ((p_joy_stat[0] & JOY_BTN_B) >> (JOY_BTN_B_SFT - 2))
			| ((p_joy_stat[0] & JOY_BTN_C) >> (JOY_BTN_C_SFT - 1))
			| ((p_joy_stat[0] & JOY_BTN_D) >> JOY_BTN_D_SFT);

		// - - - - E1 E2 Start Select
		m_joy_stat[1] = (p_joy_stat[0] & (JOY_BTN_STA | JOY_BTN_SEL));	// start / select
		m_joy_stat[1] >>= (JOY_BTN_SEL_SFT - (JOY_BTN_E2_SFT - 2)); // 5
		m_joy_stat[1] |= (p_joy_stat[0] & JOY_BTN_E2);		// E2
		m_joy_stat[1] >>= (JOY_BTN_E2_SFT - 2 - (JOY_BTN_E1_SFT - 3)); // 2
		m_joy_stat[1] |= (p_joy_stat[0] & JOY_BTN_E1);		// E1
		m_joy_stat[1] >>= (JOY_BTN_E1_SFT - 3); // 9

		// Yaxis on right stick
		m_joy_stat[6] = (p_joy_stat[5] >> 2);
		m_joy_stat[2] = (m_joy_stat[6] >> 4);
		m_joy_stat[6] &= 0xf;
		m_joy_stat[2] &= 0xf;
		m_joy_stat[6] = 15 - m_joy_stat[6];
		m_joy_stat[2] = 15 - m_joy_stat[2];

		// Xaxis on right stick
		m_joy_stat[7] = (p_joy_stat[4] >> 2);
		m_joy_stat[3] = (m_joy_stat[7] >> 4);
		m_joy_stat[7] &= 0xf;
		m_joy_stat[3] &= 0xf;
		m_joy_stat[7] = 15 - m_joy_stat[7];
		m_joy_stat[3] = 15 - m_joy_stat[3];

		// Yaxis on left stick
		m_joy_stat[8] = (p_joy_stat[3] >> 2);
		m_joy_stat[4] = (m_joy_stat[8] >> 4);
		m_joy_stat[8] &= 0xf;
		m_joy_stat[4] &= 0xf;
		m_joy_stat[8] = 15 - m_joy_stat[8];
		m_joy_stat[4] = 15 - m_joy_stat[4];

		// Xaxis on left stick
		m_joy_stat[9] = (p_joy_stat[2] >> 2);
		m_joy_stat[5] = (m_joy_stat[9] >> 4);
		m_joy_stat[9] &= 0xf;
		m_joy_stat[5] &= 0xf;
		m_joy_stat[9] = 15 - m_joy_stat[9];
		m_joy_stat[5] = 15 - m_joy_stat[5];

		// - - - - A B A' B'
		m_joy_stat[10] = m_joy_stat[0] & 0x0c;
	}

#ifdef USE_KEY_RECORD
	p_reckey->processing_joypia_status(m_idx, m_joy_stat, 12);
#endif

//	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat[m_joy_idx] | m_ack_sel), 0xff);	// negative

//	logging->out_debugf(_T("JOYPAD_CPSF: i:%d 0:%04X 1:%04X"), m_joy_idx, m_joy_stat[0], m_joy_stat[1]);
}

void JOYPAD_CYBER::modify_flags(uint8_t flags)
{
	if ((m_joy_flags ^ flags) & m_joy_flags_mask) {
		if (m_phase == 0 && (flags & m_joy_flags_mask) != 0) {
			// 0 -> 1
			// set SEL (b5)
			m_ack_sel = 0x20;
			m_joy_idx = 0;
			// send ack signal to host after 70us
			register_event(70.0);
			// comminucation speed
			m_next_speed = 1;
			m_phase = 1;

			d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat[m_joy_idx] | m_ack_sel), 0xff);	// negative
		}
	}

	m_joy_flags = flags;
}

void JOYPAD_CYBER::event_callback(int id)
{
	switch(m_phase) {
	case 1:
		// set ACK (b6) and set SEL (b5)
		m_ack_sel = 0x60;
		// send ack signal to host after 16us
		register_event(16.0 * m_speed);
		// comminucation speed
		if (!(m_joy_flags & m_joy_flags_mask)) m_next_speed++;
		m_phase = 2;
		break;
	case 2:
		// reset ACK (b6) and reset SEL (b5)
		m_ack_sel = 0x00;
		m_joy_idx++;
		// send ack signal to host after 4us
		register_event(4.0 * m_speed);
		m_phase = 3;
		break;
	case 3:
		// set ACK (b6) and reset SEL (b5)
		m_ack_sel = 0x40;
		// send ack signal to host after 16us
		register_event(16.0 * m_speed);
		m_phase = 4;
		break;
	case 4:
		// reset ACK (b6) and set SEL (b5)
		m_ack_sel = 0x20;
		m_joy_idx++;
		if (m_joy_idx >= 12) {
			// end
			m_phase = 0;
			m_joy_idx = 0;
			m_speed = m_next_speed;
		} else {
			// send ack signal to host after 20us
			register_event(20.0 * m_speed);
			m_phase = 1;
		}
		break;
	}

	d_pio->write_signal(I8255::SIG_PORT_A + m_idx, ~(m_joy_stat[m_joy_idx] | m_ack_sel), 0xff);	// negative
}

void JOYPAD_CYBER::save_state(uint8_t *data, int size)
{
	JOYPAD_BASE::save_state(data, size);

	data[1] = (uint8_t)m_joy_idx;
	data[2] = (uint8_t)m_phase;
	data[3] = (uint8_t)m_ack_sel;
	data[4] = (uint8_t)m_speed;
	data[5] = (uint8_t)m_next_speed;
	for(int i=0; i<12; i++) {
		data[i+6] = (uint8_t)m_joy_stat[i];
	}
}

void JOYPAD_CYBER::load_state(const uint8_t *data, int size)
{
	JOYPAD_BASE::load_state(data, size);

	m_joy_idx = data[1];
	m_phase = data[2];
	m_ack_sel = data[3];
	m_speed = data[4];
	m_next_speed = data[5];
	for(int i=0; i<12; i++) {
		m_joy_stat[i] = data[i+6];
	}
}
