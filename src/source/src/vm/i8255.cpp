/** @file i8255.cpp

	Skelton for retropc emulator

	@author : Takeda.Toshiya
	@date   : 2006.06.01-

	@brief [ i8255 ]
*/

#include "i8255.h"
#include "../fileio.h"

I8255::I8255(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name(_T("I8255"));

	for(int i = 0; i < 3; i++) {
		init_output_signals(&port[i].outputs);
		port[i].wreg = port[i].rreg = 0;//0xff;
	}
	clear_ports_by_cmdreg = false;
}

void I8255::reset()
{
	for(int i = 0; i < 3; i++) {
		port[i].rmask = 0xff;
		port[i].first = true;
		port[i].mode = 0;
	}
}

void I8255::write_io8(uint32_t addr, uint32_t data)
{
	int ch = addr & 3;

	switch(ch) {
	case 0:
	case 1:
	case 2:
		if(port[ch].wreg != data || port[ch].first) {
			write_signals(&port[ch].outputs, port[ch].wreg = data);
			port[ch].first = false;
		}
#ifndef I8255_AUTO_HAND_SHAKE
		if(ch == 0) {
			if(port[0].mode == 1 || port[0].mode == 2) {
				uint32_t val = port[2].wreg & ~BIT_OBF_A;
				if(port[2].wreg & BIT_ACK_A) {
					val &= ~BIT_INTR_A;
				}
				write_io8(2, val);
			}
		} else if(ch == 1) {
			if(port[1].mode == 1) {
				uint32_t val = port[2].wreg & ~BIT_OBF_B;
				if(port[2].wreg & BIT_ACK_B) {
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		}
#endif
		break;
	case 3:
		if(data & 0x80) {
			port[0].mode = (data & 0x40) ? 2 : ((data >> 5) & 1);
			port[0].rmask = (port[0].mode == 2) ? 0xff : (data & 0x10) ? 0xff : 0;
			port[1].mode = (data >> 2) & 1;
			port[1].rmask = (data & 2) ? 0xff : 0;
			port[2].rmask = ((data & 8) ? 0xf0 : 0) | ((data & 1) ? 0xf : 0);
			// clear ports
			if(clear_ports_by_cmdreg) {
				write_io8(0, 0);
				write_io8(1, 0);
				write_io8(2, 0);
			}
			// setup control signals
			if(port[0].mode != 0 || port[1].mode != 0) {
				uint32_t val = port[2].wreg;
				if(port[0].mode == 1 || port[0].mode == 2) {
					val &= ~BIT_IBF_A;
					val |= BIT_OBF_A;
					val &= ~BIT_INTR_A;
				}
				if(port[1].mode == 1) {
					if(port[1].mode == 0xff) {
						val &= ~BIT_IBF_B;
					} else {
						val |= BIT_OBF_B;
					}
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		} else {
			uint32_t val = port[2].wreg;
			int bit = (data >> 1) & 7;
			if(data & 1) {
				val |= 1 << bit;
			} else {
				val &= ~(1 << bit);
			}
			write_io8(2, val);
		}
		break;
	}
}

uint32_t I8255::read_io8(uint32_t addr)
{
	int ch = addr & 3;

	switch(ch) {
	case 0:
	case 1:
	case 2:
		if(ch == 0) {
			if(port[0].mode == 1 || port[0].mode == 2) {
				uint32_t val = port[2].wreg & ~BIT_IBF_A;
				if(port[2].wreg & BIT_STB_A) {
					val &= ~BIT_INTR_A;
				}
				write_io8(2, val);
			}
		} else if(ch == 1) {
			if(port[1].mode == 1) {
				uint32_t val = port[2].wreg & ~BIT_IBF_B;
				if(port[2].wreg & BIT_STB_B) {
					val &= ~BIT_INTR_B;
				}
				write_io8(2, val);
			}
		}
		return (port[ch].rreg & port[ch].rmask) | (port[ch].wreg & ~port[ch].rmask);
	}
	return 0xff;
}

void I8255::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_PORT_A:
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
#ifdef I8255_AUTO_HAND_SHAKE
		if(port[0].mode == 1 || port[0].mode == 2) {
			uint32_t val = port[2].wreg | BIT_IBF_A;
			if(port[2].wreg & BIT_STB_A) {
				val |= BIT_INTR_A;
			}
			write_io8(2, val);
		}
#endif
		break;
	case SIG_PORT_B:
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
#ifdef I8255_AUTO_HAND_SHAKE
		if(port[1].mode == 1) {
			uint32_t val = port[2].wreg | BIT_IBF_B;
			if(port[2].wreg & BIT_STB_B) {
				val |= BIT_INTR_B;
			}
			write_io8(2, val);
		}
#endif
		break;
	case SIG_PORT_C:
#ifndef I8255_AUTO_HAND_SHAKE
		if(port[0].mode == 1 || port[0].mode == 2) {
			if(mask & BIT_STB_A) {
				if((port[2].rreg & BIT_STB_A) && !(data & BIT_STB_A)) {
					write_io8(2, port[2].wreg | BIT_IBF_A);
				} else if(!(port[2].rreg & BIT_STB_A) && (data & BIT_STB_A)) {
					if(port[2].wreg & BIT_STB_A) {
						write_io8(2, port[2].wreg | BIT_INTR_A);
					}
				}
			}
			if(mask & BIT_ACK_A) {
				if((port[2].rreg & BIT_ACK_A) && !(data & BIT_ACK_A)) {
					write_io8(2, port[2].wreg | BIT_OBF_A);
				} else if(!(port[2].rreg & BIT_ACK_A) && (data & BIT_ACK_A)) {
					if(port[2].wreg & BIT_ACK_A) {
						write_io8(2, port[2].wreg | BIT_INTR_A);
					}
				}
			}
		}
		if(port[1].mode == 1) {
			if(port[0].rmask == 0xff) {
				if(mask & BIT_STB_B) {
					if((port[2].rreg & BIT_STB_B) && !(data & BIT_STB_B)) {
						write_io8(2, port[2].wreg | BIT_IBF_B);
					} else if(!(port[2].rreg & BIT_STB_B) && (data & BIT_STB_B)) {
						if(port[2].wreg & BIT_STB_B) {
							write_io8(2, port[2].wreg | BIT_INTR_B);
						}
					}
				}
			} else {
				if(mask & BIT_ACK_B) {
					if((port[2].rreg & BIT_ACK_B) && !(data & BIT_ACK_B)) {
						write_io8(2, port[2].wreg | BIT_OBF_B);
					} else if(!(port[2].rreg & BIT_ACK_B) && (data & BIT_ACK_B)) {
						if(port[2].wreg & BIT_ACK_B) {
							write_io8(2, port[2].wreg | BIT_INTR_B);
						}
					}
				}
			}
		}
#endif
		port[2].rreg = (port[2].rreg & ~mask) | (data & mask);
		break;
	case SIG_CPU_RESET:
		now_reset = (data & mask) != 0;
		if (!now_reset) {
			reset();
		}
		break;
	}
}

uint32_t I8255::read_signal(int id)
{
	switch(id) {
	case SIG_PORT_A:
		return port[0].wreg;
	case SIG_PORT_B:
		return port[1].wreg;
	case SIG_PORT_C:
		return port[2].wreg;
	}
	return 0;
}

// ----------------------------------------------------------------------------

void I8255::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	vm_state_ident.version = Uint16_LE(1);

	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	for(int i=0; i<3; i++) {
		vm_state.port[i].wreg = Uint32_LE(port[i].wreg);
		vm_state.port[i].rreg = Uint32_LE(port[i].rreg);
		vm_state.port[i].rmask = Uint32_LE(port[i].rmask);
		vm_state.port[i].mode = Uint32_LE(port[i].mode);
		vm_state.port[i].first = port[i].first ? 1 : 0;
	}

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool I8255::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	for(int i=0; i<3; i++) {
		port[i].wreg = Uint32_LE(vm_state.port[i].wreg);
		port[i].rreg = Uint32_LE(vm_state.port[i].rreg);
		port[i].rmask = Uint32_LE(vm_state.port[i].rmask);
		port[i].mode = Uint32_LE(vm_state.port[i].mode);
		port[i].first = vm_state.port[i].first != 0;
	}

	return true;
}
