/** @file i8255.h

	Skelton for retropc emulator

	@author : Takeda.Toshiya
	@date   : 2006.09.14 -

	[ i8255 ]
*/

#ifndef I8255_H
#define I8255_H

#include "vm.h"
#include "../emu.h"
#include "device.h"

/**
	@brief I8255 PPI - Programmable Peripheral Interface
*/
class I8255 : public DEVICE
{
public:
	enum en_sig_ids {
		SIG_PORT_A = 0,
		SIG_PORT_B = 1,
		SIG_PORT_C = 2,
	};

private:
	enum en_status_masks {
		// mode1 input
		BIT_IBF_A	= 0x20,	// PC5
		BIT_STB_A	= 0x10,	// PC4
		BIT_STB_B	= 0x04,	// PC2
		BIT_IBF_B	= 0x02,	// PC1

		BIT_OBF_A	= 0x80,	// PC7
		BIT_ACK_A	= 0x40,	// PC6
		BIT_ACK_B	= 0x04,	// PC2
		BIT_OBF_B	= 0x02,	// PC1

		BIT_INTR_A	= 0x08,	// PC3
		BIT_INTR_B	= 0x01,	// PC0
	};

private:
	struct st_port {
		uint8_t wreg;
		uint8_t rreg;
		uint8_t rmask;
		uint8_t mode;
		bool first;
		// output signals
		outputs_t outputs;
	} port[3];

	// for resume
#pragma pack(1)
	struct vm_state_st {
		struct {
			uint8_t wreg;
			uint8_t rreg;
			uint8_t rmask;
			uint8_t mode;
			uint8_t first;
			char reserved[3];
		} port[3];

		char reserved[8];
	};
#pragma pack()


public:
	I8255(VM* parent_vm, EMU* parent_emu, const char *identifier);
	~I8255() {}

	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift_left = 0)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift_left);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift_left = 0)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift_left);
	}
	void set_context_port_c(DEVICE* device, int id, uint32_t mask, int shift_left = 0)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift_left);
	}
	bool clear_ports_by_cmdreg;
};

#endif /* I8255_H */

