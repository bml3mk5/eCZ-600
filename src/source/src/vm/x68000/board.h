/** @file board.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ main board ]
*/

#ifndef BOARD_H
#define BOARD_H

#include "../vm_defs.h"
//#include "../../emu.h"
#include "../device.h"
//#include "../../config.h"

class EMU;

/**
	@brief main board

	* processing interrupt
	* processing ipl1 priority
	* processing interrupt vector number and iack signal 
*/
class BOARD : public DEVICE
{
public:
	/// @brief signals on BOARD
	enum SIG_BOARD_IDS {
		SIG_BOARD_POWER = 11,
	};

private:

	/// @brief events on BOARD
	enum EVENT_BOARD_IDS {
		EVENT_BOARD_WRESET_RELEASE = 1,
		EVENT_BOARD_PRESET_RELEASE = 2,
	};
#ifdef USE_DEBUGGER
	enum WRITE_REG_IDS {
		WREG_RESET = 0,
		WREG_IRQ,
		WREG_HALT,

		WREG_INT1MASK,
		WREG_INT1STAT,
		WREG_INT1VECNUM,
	};

#endif

private:
	outputs_t outputs_power;
	outputs_t outputs_reset;
	outputs_t outputs_irq;
	outputs_t outputs_halt;
//	outputs_t outputs_iack[8];

	uint16_t now_halt;
	uint16_t now_irq;
	uint16_t now_wreset;

	uint8_t m_now_fc;	///< function code on CPU
//	bool m_now_iack;	///< receiving IACK (int1)
	uint16_t m_now_irq_ack;

	uint8_t m_int1_mask;		///< int1 mask
	uint8_t m_int1_mask_s;		///< int1 mask swap flags 0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	uint8_t m_int1_status;		///< int1 flags  0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
	uint8_t m_int1_vec_num;		///< int1 vector bit0&1 are 0:FDC 1:FDD 2:HDD 3:Printer

	static const uint8_t c_int1_priority[4];	///< int1 priority 0:FDC 1:FDD 2:HDD 3:Printer 

	uint16_t m_int2_irq;		///< int2 flags
	uint16_t m_int4_irq;		///< int4 flags

	uint8_t m_vector;			///< int1 vector number occuring interrupt

	uint8_t m_front_power;		///< front power button and power led (b0: power sw on = 1)

	int wreset_register_id;	// normal reset
	int preset_register_id;	// power on reset

	DEVICE *d_cpu;
	DEVICE *d_mfp;

// for save config
#pragma pack(1)
	struct vm_state_st {
		uint32_t flags;
		uint16_t now_halt;
		uint16_t now_irq;
		uint16_t now_wreset;
		uint16_t m_now_irq_ack;
		uint8_t m_now_fc;	///< function code on CPU
		uint8_t reserved0;	// m_now_iack;	///< receiving IACK
		uint8_t m_int1_mask;		///< int1 mask
		uint8_t m_int1_mask_s;		///< int1 mask swap flags 0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD

		uint8_t m_int1_status;		///< int1 flags  0x80:FDC 0x40:FDD 0x20:Printer 0x10:HDD
		uint8_t m_int1_vec_num;		///< int1 vector bit0&1 are 0:FDC 1:FDD 2:HDD 3:Printer
		uint8_t m_vector;			///< int1 vector number occuring interrupt
		uint8_t m_front_power;		///< front power button and power led (b0: power sw on = 1)
		int wreset_register_id;	// normal reset
		int preset_register_id;	// power on reset
		uint16_t m_int2_irq;		///< int2 flags
		uint16_t m_int4_irq;		///< int4 flags
	};
#pragma pack()

public:
	BOARD(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("BOARD");
		init_output_signals(&outputs_power);
		init_output_signals(&outputs_reset);
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_halt);
//		for(int i=0; i<8; i++) {
//			init_output_signals(&outputs_iack[i]);
//		}
	}
	~BOARD() {}

	// common functions
	void initialize();
	void reset();
	void warm_reset(bool por);
	void cancel_my_event(int &id);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_external_data8(uint32_t addr);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void event_callback(int event_id, int err);
	void event_frame();
	uint32_t get_intr_ack();

	// unique functions
	void set_context_power(DEVICE* device, int id, uint32_t mask, uint32_t negative) {
		register_output_signal(&outputs_power, device, id, mask, negative);
	}
	void set_context_reset(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_reset, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_halt(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_halt, device, id, mask);
	}
//	void set_context_iack(int irq_num, DEVICE* device, int id, uint32_t mask) {
//		register_output_signal(&outputs_iack[irq_num], device, id, mask);
//	}
	void set_context_cpu(DEVICE *device) {
		d_cpu = device;
	}
	void set_context_mfp(DEVICE *device) {
		d_mfp = device;
	}
	uint32_t get_front_power_on() const;
	uint32_t get_led_status();

	uint32_t asserted_int2_devices() const;
	uint32_t asserted_int4_devices() const;

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* BOARD_H */

