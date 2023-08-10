/** @file joypad.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.22 -

	@brief [ joypad ]
*/

#ifndef JOYPAD_H
#define JOYPAD_H

#include "../vm_defs.h"
#include "../../common.h"

class DEVICE;
class KEYRECORD;

/**
	@brief joypad base class
*/
class JOYPAD_BASE
{
public:
	enum en_types {
		TYPE_STD = 0,
		TYPE_MD3,
		TYPE_MD6,
		TYPE_CPSF,
		TYPE_CPSF_MD,
		TYPE_XPDLR,
		TYPE_CYBER,
	};
protected:
	int       m_idx;
	DEVICE   *d_pio;

#ifdef USE_KEY_RECORD
	KEYRECORD *p_reckey;
#endif

	uint32_t *p_joy_stat;
	uint8_t   m_joy_flags;
	uint8_t   m_joy_flags_mask;

	// for event
	DEVICE *d_event;
	int	    m_event_id;
	int	   *p_register_id;

	virtual void register_event(double usec);

public:
	JOYPAD_BASE(DEVICE *pio, int idx, uint32_t *joy_stat);
	virtual ~JOYPAD_BASE();

#ifdef USE_KEY_RECORD
	void set_keyrecord_ptr(KEYRECORD *reckey);
#endif
	void set_event_param(DEVICE *dev, int event_id, int *register_id);

	virtual int get_type() const = 0;
	virtual void reset();
	virtual void update_joy();
	virtual void modify_flags(uint8_t flags);
	virtual void event_callback(int id);

	virtual void save_state(uint8_t *data, int size);
	virtual void load_state(const uint8_t *data, int size);
};

/**
	@brief standard joypad (ATARI specification)
*/
class JOYPAD_STD : public JOYPAD_BASE
{
private:
	uint32_t m_joy_stat;
	uint32_t m_joy_stat_mask;

public:
	JOYPAD_STD(DEVICE *pio, int idx, uint32_t *joy_stat);
	virtual ~JOYPAD_STD();

	virtual int get_type() const { return TYPE_STD; }
	virtual void reset();
	virtual void update_joy();
	virtual void modify_flags(uint8_t flags);

	virtual void save_state(uint8_t *data, int size);
	virtual void load_state(const uint8_t *data, int size);
};

/**
	@brief megadrive 3 buttons joypad
*/
class JOYPAD_MD3 : public JOYPAD_BASE
{
protected:
	int m_joy_idx;
	uint32_t m_joy_stat[2];

public:
	JOYPAD_MD3(DEVICE *pio, int idx, uint32_t *joy_stat);
	virtual ~JOYPAD_MD3();

	virtual int get_type() const { return TYPE_MD3; }
	virtual void reset();
	virtual void update_joy();
	virtual void modify_flags(uint8_t flags);

	virtual void save_state(uint8_t *data, int size);
	virtual void load_state(const uint8_t *data, int size);
};

/**
	@brief megadrive 6 buttons joypad
*/
class JOYPAD_MD6 : public JOYPAD_BASE
{
private:
	int m_joy_idx;
	int m_joy_cnt;
	uint32_t m_joy_stat[5];
	uint64_t m_prev_clk;
	uint64_t m_sub_clk;
public:
	JOYPAD_MD6(DEVICE *pio, int idx, uint32_t *joy_stat);
	virtual ~JOYPAD_MD6();

	virtual int get_type() const { return TYPE_MD6; }
	virtual void reset();
	virtual void update_joy();
	virtual void modify_flags(uint8_t flags);

	virtual void save_state(uint8_t *data, int size);
	virtual void load_state(const uint8_t *data, int size);
};

/**
	@brief capcom power stick fighter
*/
class JOYPAD_CPSF : public JOYPAD_MD3
{
public:
	JOYPAD_CPSF(DEVICE *pio, int idx, uint32_t *joy_stat);

	virtual int get_type() const { return TYPE_CPSF; }
	virtual void update_joy();
};

/**
	@brief capcom power stick fighter for megadrive
*/
class JOYPAD_CPSF_MD : public JOYPAD_MD3
{
public:
	JOYPAD_CPSF_MD(DEVICE *pio, int idx, uint32_t *joy_stat);

	virtual int get_type() const { return TYPE_CPSF_MD; }
	virtual void update_joy();
};

/**
	@brief micomsoft XPD-1LR
*/
class JOYPAD_XPDLR : public JOYPAD_MD3
{
public:
	JOYPAD_XPDLR(DEVICE *pio, int idx, uint32_t *joy_stat);

	virtual int get_type() const { return TYPE_XPDLR; }
	virtual void update_joy();
};

/**
	@brief cyber stick (analog mode)
*/
class JOYPAD_CYBER : public JOYPAD_BASE
{
private:
	int m_phase;
	int m_joy_idx;
	uint32_t m_joy_stat[12];
	uint32_t m_ack_sel;
	int m_speed;
	int m_next_speed;

public:
	JOYPAD_CYBER(DEVICE *pio, int idx, uint32_t *joy_stat);
	virtual ~JOYPAD_CYBER();

	virtual int get_type() const { return TYPE_CYBER; }
	virtual void reset();
	virtual void update_joy();
	virtual void modify_flags(uint8_t flags);
	virtual void event_callback(int id);

	virtual void save_state(uint8_t *data, int size);
	virtual void load_state(const uint8_t *data, int size);
};

#endif /* JOYPAD_H */

