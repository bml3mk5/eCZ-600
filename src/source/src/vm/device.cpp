/** @file device.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5/MBS1 by Sasaji at 2015.09.01

	@brief [ device base class ]
*/

#include "device.h"
#include "vm.h"
#include "../emu.h"
#include "../depend.h"
#include "../fileio.h"
#ifdef _DEBUG
#include <cassert>
#endif

/// This is the device that managed under vm
DEVICE::DEVICE(VM* parent_vm, EMU* parent_emu, const char *identifier) : vm(parent_vm), emu(parent_emu)
{
	prev_device = vm->last_device;
	next_device = NULL;
	if(vm->first_device == NULL) {
		// this is the first device
		vm->first_device = this;
		this_device_id = 0;
	} else {
		// this is not the first device
		vm->last_device->next_device = this;
		this_device_id = vm->last_device->this_device_id + 1;
	}
	vm->last_device = this;

	// primary event manager
	event_manager = NULL;

	available = true;

#ifdef USE_EMU_INHERENT_SPEC
	now_reset = false;
#endif
	//
	memset(this_class_name, 0, sizeof(this_class_name));
	memset(&vm_state_ident, 0, sizeof(vm_state_ident));
	set_identifier(identifier);
	vm_state_ident.device_id = this_device_id;
}

/// This is the device that independent with vm
DEVICE::DEVICE(EMU* parent_emu, const char *identifier) : vm(NULL), emu(parent_emu)
{
	// no managed device
	this_device_id = -1;
	prev_device = NULL;
	next_device = NULL;

	// primary event manager
	event_manager = NULL;

	available = true;

#ifdef USE_EMU_INHERENT_SPEC
	now_reset = false;
#endif
	//
	memset(this_class_name, 0, sizeof(this_class_name));
	memset(&vm_state_ident, 0, sizeof(vm_state_ident));
	set_identifier(identifier);
	vm_state_ident.device_id = this_device_id;
}

DEVICE::~DEVICE() {}

void DEVICE::initialize() {}
void DEVICE::release() {}

void DEVICE::update_config() {}
void DEVICE::save_state(FILEIO* fio) {}
bool DEVICE::load_state(FILEIO* fio)
{
	return true;
}

void DEVICE::set_class_name(const char *name)
{
	if (name != NULL) {
		for(int i=0; i<12 && name[i]; i++) {
			this_class_name[i] = name[i];
			vm_state_ident.class_name[i] = name[i];
		}
	}
}
const char *DEVICE::get_class_name() const
{
	return this_class_name;
}
const char *DEVICE::get_identifier() const
{
	return this_identifier;
}
void DEVICE::set_identifier(const char *identifier)
{
	memset(this_identifier, 0, sizeof(this_identifier));
	memset(&vm_state_ident.identifier, 0, sizeof(vm_state_ident.identifier));
	if (identifier != NULL) {
		for(int i=0; i<4 && identifier[i]; i++) {
			this_identifier[i] = identifier[i];
			vm_state_ident.identifier[i] = identifier[i];
		}
	}
}
void DEVICE::set_vm_state_class_name(const char *name)
{
	if (name != NULL) {
		memset(vm_state_ident.class_name, 0, 12);
		for(int i=0; i<12 && name[i]; i++) {
			vm_state_ident.class_name[i] = name[i];
		}
	}
}
DEVICE *DEVICE::get_prev_device()
{
	return prev_device;
}
DEVICE *DEVICE::get_next_device()
{
	return next_device;
}
int DEVICE::get_id() const
{
	return this_device_id;
}

// control

void DEVICE::reset() {}
void DEVICE::special_reset()
{
	reset();
}
void DEVICE::enable(bool value)
{
	available = value;
}

bool DEVICE::enable() const
{
	return available;
}

// memory bus

/// @param[in] addr  : address
/// @param[in] data  : data
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
void DEVICE::write_data_n(uint32_t addr, uint32_t data, int width)
{
}

/// @param[in] addr  : address
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
/// @return data
uint32_t DEVICE::read_data_n(uint32_t addr, int width)
{
	return 0xffffffff;
}

void DEVICE::write_data8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_data8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::write_data16(uint32_t addr, uint32_t data)
{
	write_data8(addr, data & 0xff);
	write_data8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_data16(uint32_t addr)
{
	uint32_t val = read_data8(addr);
	val |= read_data8(addr + 1) << 8;
	return val;
}

/// convenience function to store 16 + 8 bits data
void DEVICE::write_data24(uint32_t addr, uint32_t data)
{
	write_data8(addr, data & 0xff);
	write_data8(addr + 1, (data >> 8) & 0xff);
	write_data8(addr + 2, (data >> 16) & 0xff);
}

/// convenience function to load 16 + 8 bits data
uint32_t DEVICE::read_data24(uint32_t addr)
{
	uint32_t val = read_data8(addr);
	val |= read_data8(addr + 1) << 8;
	val |= read_data8(addr + 2) << 16;
	return val;
}

void DEVICE::write_data32(uint32_t addr, uint32_t data)
{
	write_data8(addr, data & 0xff);
	write_data8(addr + 1, (data >> 8) & 0xff);
	write_data8(addr + 2, (data >> 16) & 0xff);
	write_data8(addr + 3, (data >> 24) & 0xff);
}

uint32_t DEVICE::read_data32(uint32_t addr)
{
	uint32_t val = read_data8(addr);
	val |= read_data8(addr + 1) << 8;
	val |= read_data8(addr + 2) << 16;
	val |= read_data8(addr + 3) << 24;
	return val;
}

void DEVICE::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_data8(addr, data);
}

uint32_t DEVICE::read_data8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_data8(addr);
}

void DEVICE::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1;
	write_data8w(addr, data & 0xff, &wait0);
	write_data8w(addr + 1, (data >> 8) & 0xff, &wait1);
	*wait = wait0 + wait1;
}

uint32_t DEVICE::read_data16w(uint32_t addr, int* wait)
{
	int wait0, wait1;
	uint32_t val = read_data8w(addr, &wait0);
	val |= read_data8w(addr + 1, &wait1) << 8;
	*wait = wait0 + wait1;
	return val;
}

/// convenience function to store 16 + 8 bits data
void DEVICE::write_data24w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1, wait2;
	write_data8w(addr, data & 0xff, &wait0);
	write_data8w(addr + 1, (data >> 8) & 0xff, &wait1);
	write_data8w(addr + 2, (data >> 16) & 0xff, &wait2);
	*wait = wait0 + wait1 + wait2;
}

/// convenience function to load 16 + 8 bits data
uint32_t DEVICE::read_data24w(uint32_t addr, int* wait)
{
	int wait0, wait1, wait2;
	uint32_t val = read_data8w(addr, &wait0);
	val |= read_data8w(addr + 1, &wait1) << 8;
	val |= read_data8w(addr + 2, &wait2) << 16;
	*wait = wait0 + wait1 + wait2;
	return val;
}

void DEVICE::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1, wait2, wait3;
	write_data8w(addr, data & 0xff, &wait0);
	write_data8w(addr + 1, (data >> 8) & 0xff, &wait1);
	write_data8w(addr + 2, (data >> 16) & 0xff, &wait2);
	write_data8w(addr + 3, (data >> 24) & 0xff, &wait3);
	*wait = wait0 + wait1 + wait2 + wait3;
}

uint32_t DEVICE::read_data32w(uint32_t addr, int* wait)
{
	int wait0, wait1, wait2, wait3;
	uint32_t val = read_data8w(addr, &wait0);
	val |= read_data8w(addr + 1, &wait1) << 8;
	val |= read_data8w(addr + 2, &wait2) << 16;
	val |= read_data8w(addr + 3, &wait3) << 24;
	*wait = wait0 + wait1 + wait2 + wait3;
	return val;
}

uint32_t DEVICE::fetch_op(uint32_t addr, int *wait)
{
	return read_data8w(addr, wait);
}

void DEVICE::latch_address(uint32_t addr, int *wait)
{
}

void DEVICE::write_dma_data_n(uint32_t addr, uint32_t data, int width)
{
	return write_data_n(addr, data, width);
}

uint32_t DEVICE::read_dma_data_n(uint32_t addr, int width)
{
	return read_data_n(addr, width);
}

void DEVICE::write_dma_data8(uint32_t addr, uint32_t data)
{
	write_data8(addr, data);
}

uint32_t DEVICE::read_dma_data8(uint32_t addr)
{
	return read_data8(addr);
}

void DEVICE::write_dma_data16(uint32_t addr, uint32_t data)
{
	write_data16(addr, data);
}

uint32_t DEVICE::read_dma_data16(uint32_t addr)
{
	return read_data16(addr);
}

void DEVICE::write_dma_data32(uint32_t addr, uint32_t data)
{
	write_data32(addr, data);
}

uint32_t DEVICE::read_dma_data32(uint32_t addr)
{
	return read_data32(addr);
}

// i/o bus

/// @param[in] addr  : address
/// @param[in] data  : data
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
void DEVICE::write_io_n(uint32_t addr, uint32_t data, int width)
{
}

/// @param[in] addr  : address
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
/// @return data
uint32_t DEVICE::read_io_n(uint32_t addr, int width)
{
	return 0xffffffff;
}

/// @param[in] addr  : address
/// @param[in] data  : data
/// @param[in] mask  : write data partly
void DEVICE::write_io_m(uint32_t addr, uint32_t data, uint32_t mask)
{
}

/// @param[in] addr  : address
/// @param[in] mask  : read data partly
/// @return data
uint32_t DEVICE::read_io_m(uint32_t addr, uint32_t mask)
{
	return 0;
}

void DEVICE::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_io8(uint32_t addr)
{
#ifdef IOBUS_RETURN_ADDR
	return (addr & 1 ? addr >> 8 : addr) & 0xff;
#else
	return 0xff;
#endif
}

void DEVICE::write_io16(uint32_t addr, uint32_t data)
{
	write_io8(addr, data & 0xff);
	write_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_io16(uint32_t addr)
{
	uint32_t val = read_io8(addr);
	val |= read_io8(addr + 1) << 8;
	return val;
}

void DEVICE::write_io32(uint32_t addr, uint32_t data)
{
	write_io8(addr, data & 0xff);
	write_io8(addr + 1, (data >> 8) & 0xff);
	write_io8(addr + 2, (data >> 16) & 0xff);
	write_io8(addr + 3, (data >> 24) & 0xff);
}

uint32_t DEVICE::read_io32(uint32_t addr)
{
	uint32_t val = read_io8(addr);
	val |= read_io8(addr + 1) << 8;
	val |= read_io8(addr + 2) << 16;
	val |= read_io8(addr + 3) << 24;
	return val;
}

void DEVICE::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_io8(addr, data);
}

uint32_t DEVICE::read_io8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_io8(addr);
}

void DEVICE::write_io16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1;
	write_io8w(addr, data & 0xff, &wait0);
	write_io8w(addr + 1, (data >> 8) & 0xff, &wait1);
	*wait = wait0 + wait1;
}

uint32_t DEVICE::read_io16w(uint32_t addr, int* wait)
{
	int wait0, wait1;
	uint32_t val = read_io8w(addr, &wait0);
	val |= read_io8w(addr + 1, &wait1) << 8;
	*wait = wait0 + wait1;
	return val;
}

void DEVICE::write_io32w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1, wait2, wait3;
	write_io8w(addr, data & 0xff, &wait0);
	write_io8w(addr + 1, (data >> 8) & 0xff, &wait1);
	write_io8w(addr + 2, (data >> 16) & 0xff, &wait2);
	write_io8w(addr + 3, (data >> 24) & 0xff, &wait3);
	*wait = wait0 + wait1 + wait2 + wait3;
}

uint32_t DEVICE::read_io32w(uint32_t addr, int* wait)
{
	int wait0, wait1, wait2, wait3;
	uint32_t val = read_io8w(addr, &wait0);
	val |= read_io8w(addr + 1, &wait1) << 8;
	val |= read_io8w(addr + 2, &wait2) << 16;
	val |= read_io8w(addr + 3, &wait3) << 24;
	*wait = wait0 + wait1 + wait2 + wait3;
	return val;
}

/// @param[in] addr  : address
/// @param[in] data  : data
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
void DEVICE::write_dma_io_n(uint32_t addr, uint32_t data, int width)
{
}

/// @param[in] addr  : address
/// @param[in] width : bus width 1:byte 2:word 4:dword (depend on vm architecture)
/// @return data
uint32_t DEVICE::read_dma_io_n(uint32_t addr, int width)
{
	return 0xffffffff;
}

void DEVICE::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr, data);
}

uint32_t DEVICE::read_dma_io8(uint32_t addr)
{
	return read_io8(addr);
}

void DEVICE::write_dma_io16(uint32_t addr, uint32_t data)
{
	write_io16(addr, data);
}

uint32_t DEVICE::read_dma_io16(uint32_t addr)
{
	return read_io16(addr);
}

void DEVICE::write_dma_io32(uint32_t addr, uint32_t data)
{
	write_io32(addr, data);
}

uint32_t DEVICE::read_dma_io32(uint32_t addr)
{
	return read_io32(addr);
}

// memory mapped i/o

void DEVICE::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	write_io8(addr, data);
}

uint32_t DEVICE::read_memory_mapped_io8(uint32_t addr)
{
	return read_io8(addr);
}

void DEVICE::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr, data & 0xff);
	write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::read_memory_mapped_io16(uint32_t addr)
{
	uint32_t val = read_memory_mapped_io8(addr);
	val |= read_memory_mapped_io8(addr + 1) << 8;
	return val;
}

void DEVICE::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr, data & 0xff);
	write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
	write_memory_mapped_io8(addr + 2, (data >> 16) & 0xff);
	write_memory_mapped_io8(addr + 3, (data >> 24) & 0xff);
}

uint32_t DEVICE::read_memory_mapped_io32(uint32_t addr)
{
	uint32_t val = read_memory_mapped_io8(addr);
	val |= read_memory_mapped_io8(addr + 1) << 8;
	val |= read_memory_mapped_io8(addr + 2) << 16;
	val |= read_memory_mapped_io8(addr + 3) << 24;
	return val;
}

void DEVICE::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	write_memory_mapped_io8(addr, data);
}

uint32_t DEVICE::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return read_memory_mapped_io8(addr);
}

void DEVICE::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1;
	write_memory_mapped_io8w(addr, data & 0xff, &wait0);
	write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait1);
	*wait = wait0 + wait1;
}

uint32_t DEVICE::read_memory_mapped_io16w(uint32_t addr, int* wait)
{
	int wait0, wait1;
	uint32_t val = read_memory_mapped_io8w(addr, &wait0);
	val |= read_memory_mapped_io8w(addr + 1, &wait1) << 8;
	*wait = wait0 + wait1;
	return val;
}

void DEVICE::write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
{
	int wait0, wait1, wait2, wait3;
	write_memory_mapped_io8w(addr, data & 0xff, &wait0);
	write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait1);
	write_memory_mapped_io8w(addr + 2, (data >> 16) & 0xff, &wait2);
	write_memory_mapped_io8w(addr + 3, (data >> 24) & 0xff, &wait3);
	*wait = wait0 + wait1 + wait2 + wait3;
}

uint32_t DEVICE::read_memory_mapped_io32w(uint32_t addr, int* wait)
{
	int wait0, wait1, wait2, wait3;
	uint32_t val = read_memory_mapped_io8w(addr, &wait0);
	val |= read_memory_mapped_io8w(addr + 1, &wait1) << 8;
	val |= read_memory_mapped_io8w(addr + 2, &wait2) << 16;
	val |= read_memory_mapped_io8w(addr + 3, &wait3) << 24;
	*wait = wait0 + wait1 + wait2 + wait3;
	return val;
}

void DEVICE::write_external_data8(uint32_t addr, uint32_t data)
{
}

uint32_t DEVICE::read_external_data8(uint32_t addr)
{
	return 0;
}

// device to device

void DEVICE::init_output_signals(outputs_t *items)
{
	items->count = 0;
}

void DEVICE::register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift_left, uint32_t negative)
{
	int c = items->count++;
#ifdef _DEBUG
	assert(c < MAX_OUTPUT);
#endif
	items->item[c].device = device;
	items->item[c].id = id;
	items->item[c].mask = mask;
	items->item[c].shift = shift_left;
	items->item[c].negative = negative;
}

void DEVICE::register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, uint32_t negative)
{
	register_output_signal(items, device, id, mask, 0, negative);
}

//void DEVICE::register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask)
//{
//	register_output_signal(items, device, id, mask, 0, 0);
//}

void DEVICE::write_signals(outputs_t *items, uint32_t data)
{
	for(int i = 0; i < items->count; i++) {
		output_t *item = &items->item[i];
		data ^= item->negative;
		int shift = item->shift;
		uint32_t val = (shift < 0) ? (data >> (-shift)) : (data << shift);
		uint32_t mask = (shift < 0) ? (item->mask >> (-shift)) : (item->mask << shift);
		item->device->write_signal(item->id, val, mask);
	}
};

void DEVICE::write_signal(int id, uint32_t data, uint32_t mask) {}

uint32_t DEVICE::read_signal(int ch)
{
	return 0;
}

// z80 daisy chain

void DEVICE::set_context_intr(DEVICE* device, uint32_t bit) {}
void DEVICE::set_context_child(DEVICE* device) {}

// interrupt device to device
void DEVICE::set_intr_iei(bool val) {}

// interrupt device to cpu
void DEVICE::set_intr_line(bool line, bool pending, uint32_t bit) {}

// interrupt cpu to device
uint32_t DEVICE::get_intr_ack()
{
	return 0xff;
}

void DEVICE::notify_intr_reti() {}
void DEVICE::notify_intr_ei() {}
void DEVICE::update_intr_condition() {}

// dma
void DEVICE::do_dma() {}

// cpu
int DEVICE::run(int clock)
{
	// when clock == -1, run one opecode
	return 0;
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
int DEVICE::run(int clock, int accum, int cycles)
{
	return 0;
}
#endif

uint32_t DEVICE::get_pc()
{
	return 0;
}

uint32_t DEVICE::get_next_pc()
{
	return 0;
}

// bios
bool DEVICE::bios_call(uint32_t pc, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	return false;
}

bool DEVICE::bios_int(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag)
{
	return false;
}

// event manager
DEVICE* event_manager;


void DEVICE::set_context_event_manager(DEVICE* device)
{
	event_manager = device;
}

int DEVICE::event_manager_id()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->this_device_id;
}

void DEVICE::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id, uint64_t *expire_clock)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event(device, event_id, usec, loop, register_id, expire_clock);
}

void DEVICE::register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id, uint64_t *expire_clock)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_event_by_clock(device, event_id, clock, loop, register_id, expire_clock);
}

void DEVICE::modify_event_by_clock(int register_id, int clock)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->modify_event_by_clock(register_id, clock);
}

void DEVICE::cancel_event(DEVICE* device, int register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->cancel_event(device, register_id);
}

int DEVICE::is_registerd_event(const char *class_name, const char *identifier, int register_id)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->is_registerd_event(class_name, identifier, register_id);
}

void DEVICE::register_frame_event(DEVICE* device)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_frame_event(device);
}

bool DEVICE::is_registerd_frame_event(const char *class_name, const char *identifier)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->is_registerd_frame_event(class_name, identifier);
}

void DEVICE::register_vline_event(DEVICE* device)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->register_vline_event(device);
}

bool DEVICE::is_registerd_vline_event(const char *class_name, const char *identifier)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->is_registerd_vline_event(class_name, identifier);
}

uint64_t DEVICE::get_current_clock()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_current_clock();
}

uint64_t DEVICE::get_passed_clock(uint64_t prev)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_clock(prev);
}

double DEVICE::get_passed_usec(uint64_t prev)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_passed_usec(prev);
}

void DEVICE::set_cpu_clock(uint32_t clk)
{
}

uint32_t DEVICE::get_cpu_clock() const
{
	return 1;
}

int DEVICE::get_current_power()
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_current_power();
}

void DEVICE::set_number_of_cpu(int nums)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->set_number_of_cpu(nums);
}

uint32_t DEVICE::get_cpu_pc(int index)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	return event_manager->get_cpu_pc(index);
}

void DEVICE::set_frames_per_sec(double frames)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->set_frames_per_sec(frames);
}

void DEVICE::set_lines_per_frame(int lines)
{
	if(event_manager == NULL) {
		event_manager = vm->first_device->next_device;
	}
	event_manager->set_lines_per_frame(lines);
}

void DEVICE::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame) {}

bool DEVICE::search_track(int channel)
{
	return false;
}

uint32_t DEVICE::verify_track()
{
	return 0;
}

bool DEVICE::verify_track(int channel, int track)
{
	return false;
}

int  DEVICE::get_current_track_number(int channel)
{
	return 0;
}

uint32_t DEVICE::search_sector(int side, bool compare)
{
	return 0;
}

int  DEVICE::search_sector(int channel)
{
	return 0;
}

int  DEVICE::search_sector(int channel, int track, int sect, bool compare_side, int side)
{
	return 0;
}

int  DEVICE::search_sector_by_index(int channel, uint8_t track, int index, uint8_t *compare_side, uint8_t *compare_sect, uint8_t *compare_size, uint8_t *disk_id)
{
	return 0;
}

bool DEVICE::make_track()
{
	return false;
}

bool DEVICE::make_track(int channel)
{
	return false;
}

bool DEVICE::parse_track()
{
	return false;
}

bool DEVICE::parse_track(int channel)
{
	return false;
}

int DEVICE::get_a_round_clock(int channel)
{
	return 1;
}

int DEVICE::get_head_loading_clock(int channel)
{
	return 1;
}

int DEVICE::get_index_hole_remain_clock()
{
	return 1;
}

int DEVICE::calc_index_hole_search_clock(int channel)
{
	return 1;
}

int DEVICE::get_clock_arrival_sector(int channel, int sect, int delay)
{
	return 1;
}

int DEVICE::calc_sector_search_clock(int channel, int sect)
{
	return 1;
}

int DEVICE::calc_next_sector_clock(int channel)
{
	return 1;
}

// event callback
void DEVICE::event_callback(int event_id, int err) {}
void DEVICE::event_pre_frame() {}	// this event is to update timing settings
void DEVICE::event_frame() {}
void DEVICE::event_vline(int v, int clock) {}
void DEVICE::event_hsync(int v, int h, int clock) {}

// sound
void DEVICE::mix(int32_t* buffer, int cnt) {}
//void DEVICE::set_volume(int volume) {}
//void DEVICE::set_volume(int ch, int decibel_l, int decibel_r) {}

// network
void DEVICE::network_connected(int ch) {}
void DEVICE::network_disconnected(int ch) {}
void DEVICE::network_writeable(int ch) {}
void DEVICE::network_readable(int ch) {}
void DEVICE::network_accepted(int ch, int new_ch) {}
uint8_t* DEVICE::get_sendbuffer(int ch, int* size, int* flags)
{
	return NULL;
}
void DEVICE::inc_sendbuffer_ptr(int ch, int size) {}
uint8_t* DEVICE::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
	return NULL;
}
uint8_t* DEVICE::get_recvbuffer1(int ch)
{
	return NULL;
}
void DEVICE::inc_recvbuffer_ptr(int ch, int size) {}
double DEVICE::get_send_speed_usec(int ch, bool is_byte)
{
	return 100.0;
}
double DEVICE::get_recv_speed_usec(int ch, bool is_byte)
{
	return 100.0;
}

// debugger

#ifdef USE_DEBUGGER
DEVICE *DEVICE::get_debugger()
{
	return NULL;
}
void DEVICE::set_debugger(DEVICE *dbg) {}

DEVICE *DEVICE::get_context_mem() const
{
	return NULL;
}

void DEVICE::set_debugger_console(DebuggerConsole *dc) {}

uint32_t DEVICE::debug_prog_addr_mask()
{
	return 0;
}

uint32_t DEVICE::debug_data_addr_mask()
{
	return 0;
}

uint32_t DEVICE::debug_physical_addr_mask(int type)
{
	return 0;
}

uint32_t DEVICE::debug_io_addr_mask()
{
	return 0;
}

uint32_t DEVICE::debug_data_mask()
{
	return 0;
}

bool DEVICE::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return false;
}

void DEVICE::debug_write_data8(int type, uint32_t addr, uint32_t data) {}

uint32_t DEVICE::debug_read_data8(int type, uint32_t addr)
{
	return 0xff;
}

void DEVICE::debug_write_data16(int type, uint32_t addr, uint32_t data)
{
	// little endien
	debug_write_data8(type, addr, data & 0xff);
	debug_write_data8(type, addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::debug_read_data16(int type, uint32_t addr)
{
	// little endien
	uint32_t val = debug_read_data8(type, addr);
	val |= debug_read_data8(type, addr + 1) << 8;
	return val;
}

void DEVICE::debug_write_data32(int type, uint32_t addr, uint32_t data)
{
	// little endien
	debug_write_data16(type, addr, data & 0xffff);
	debug_write_data16(type, addr + 2, (data >> 16) & 0xffff);
}

uint32_t DEVICE::debug_read_data32(int type, uint32_t addr)
{
	// little endien
	uint32_t val = debug_read_data16(type, addr);
	val |= debug_read_data16(type, addr + 2) << 16;
	return val;
}

void DEVICE::debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait) {}

uint32_t DEVICE::debug_read_data8w(int type, uint32_t addr, int *wait)
{
	return 0xff;
}

bool DEVICE::debug_ioport_is_supported() const
{
	return true;
}

bool DEVICE::debug_exception_is_supported() const
{
	return false;
}

void DEVICE::debug_write_io8(uint32_t addr, uint32_t data) {}

uint32_t DEVICE::debug_read_io8(uint32_t addr)
{
	return 0xff;
}

void DEVICE::debug_write_io16(uint32_t addr, uint32_t data)
{
	debug_write_io8(addr, data & 0xff);
	debug_write_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t DEVICE::debug_read_io16(uint32_t addr)
{
	uint32_t val = debug_read_io8(addr);
	val |= debug_read_io8(addr + 1) << 8;
	return val;
}

void DEVICE::debug_write_io32(uint32_t addr, uint32_t data)
{
	debug_write_io16(addr, data & 0xffff);
	debug_write_io16(addr + 2, (data >> 16) & 0xffff);
}

uint32_t DEVICE::debug_read_io32(uint32_t addr)
{
	uint32_t val = debug_read_io16(addr);
	val |= debug_read_io16(addr + 2) << 16;
	return val;
}

uint32_t DEVICE::debug_latch_address(uint32_t addr)
{
	return 0;
}

void DEVICE::debug_write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	write_memory_mapped_io8(addr, data);
}

uint32_t DEVICE::debug_read_memory_mapped_io8(uint32_t addr)
{
	return read_memory_mapped_io8(addr);
}

void DEVICE::debug_event_frame() {}

uint32_t DEVICE::debug_read_bank(uint32_t addr)
{
	return 0;
}

bool DEVICE::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}
bool DEVICE::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}
bool DEVICE::debug_write_reg(int type, const _TCHAR *reg, uint32_t data)
{
	return false;
}
bool DEVICE::debug_write_reg(int type, uint32_t reg_num, uint32_t data)
{
	return false;
}
uint32_t DEVICE::find_debug_reg_name(const _TCHAR *list[], const _TCHAR *name)
{
	int match = -1;
	for(int num = 0; list[num] != NULL; num++) {
		if (_tcsicmp(list[num], name) == 0) {
			match = num;
			break;
		}
	}
	return (uint32_t)match;
}

void DEVICE::debug_regs_info(_TCHAR *buffer, size_t buffer_len) {}
void DEVICE::debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len) {}
bool DEVICE::get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen)
{
	return false;
}
void DEVICE::debug_memory_map_info(DebuggerConsole *dc) {}
int DEVICE::debug_address_map_info(DebuggerConsole *dc, int index)
{
	return -1;
}
int DEVICE::debug_address_map_edit(DebuggerConsole *dc, int index, int *values, int count)
{
	return -1;
}
int DEVICE::debug_address_map_get_prev(DebuggerConsole *dc, int index, int *values, int &count)
{
	return -1;
}
int DEVICE::debug_memory_space_map_info(DebuggerConsole *dc, int index)
{
	return -1;
}
int DEVICE::debug_memory_space_map_edit(DebuggerConsole *dc, int index, int *values, int count)
{
	return -1;
}
int DEVICE::debug_memory_space_map_get(DebuggerConsole *dc, int index, int *values, int &count)
{
	return -1;
}

int DEVICE::debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags)
{
	return 0;
}

int DEVICE::debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	return 0;
}

int DEVICE::debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len)
{
	return -2;
}

int DEVICE::debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len)
{
	return -2;
}

uint32_t DEVICE::debug_address_mapping_rev(uint32_t addr)
{
	return 0;
}

bool DEVICE::reach_break_point()
{
	return false;
}

//void DEVICE::now_debugging(bool val)
//{
//}

//bool DEVICE::now_debugging() const
//{
//	return false;
//}

void DEVICE::go_suspend()
{
}

void DEVICE::go_suspend_at_first()
{
}

bool DEVICE::now_suspend() const
{
	return false;
}

uint32_t DEVICE::get_debug_pc(int type)
{
	return 0;
}

uint32_t DEVICE::get_debug_next_pc(int type)
{
	return 0;
}

uint32_t DEVICE::get_debug_branch_pc(int type)
{
	return 0;
}

bool DEVICE::get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	return false;
}

bool DEVICE::get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	return false;
}

void DEVICE::get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len) {}

bool DEVICE::get_debug_exception_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	return false;
}

bool DEVICE::get_debug_exception_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	return false;
}

void DEVICE::get_debug_exception_names_str(_TCHAR *buffer, size_t buffer_len) {}

int DEVICE::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	return -2;
}

bool DEVICE::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return false;
}

bool DEVICE::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{
	return false;
}

bool DEVICE::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{
	return false;
}

uint32_t DEVICE::debug_basic_get_line_number_ptr()
{
	return 0;
}

uint32_t DEVICE::debug_basic_get_line_number()
{
	return 0;
}

bool DEVICE::debug_basic_is_supported()
{
	return false;
}
void DEVICE::debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names) {}
void DEVICE::debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line) {}
void DEVICE::debug_basic_trace_onoff(DebuggerConsole *dc, bool enable) {}
void DEVICE::debug_basic_trace_current() {}
void DEVICE::debug_basic_trace_back(DebuggerConsole *dc, int num) {}
void DEVICE::debug_basic_command(DebuggerConsole *dc) {}
void DEVICE::debug_basic_error(DebuggerConsole *dc, int num) {}

//void DEVICE::debug_basic_set_break_point(uint32_t &addr) {}
bool DEVICE::debug_basic_check_break_point(uint32_t line, int len)
{
	return false;
}

#endif /* USE_DEBUGGER */

#ifdef USE_EMU_INHERENT_SPEC
// display
void DEVICE::update_display(int vline, int clock) {}
#endif
//
void DEVICE::get_edition_string(char *buffer, size_t buffer_len) const {}

//for resume

bool DEVICE::find_state_chunk(FILEIO *fio, const char *class_name, const char *identifier, vm_state_ident_t *vm_state_i)
{
	uint32_t size = sizeof(vm_state_ident_t);

	for(int retry = 0; retry < 2; retry++) {
		while (fio->Fread(vm_state_i, size, 1) == 1) {
			if (strncmp(class_name, vm_state_i->class_name, 12) == 0
			&&  strncmp(identifier, vm_state_i->identifier, 4) == 0) {
				// match chunk
				return true;
			}
			uint32_t chunk_size = Uint32_LE(vm_state_i->size);

			adjust_chunk_size(vm_state_i, chunk_size);

			if (chunk_size > size) {
				fio->Fseek(chunk_size - size, FILEIO::SEEKCUR);
			}
		}
		// retry first
		fio->Fseek(sizeof(vm_state_header_t), FILEIO::SEEKSET);
	}
	return false;
}

bool DEVICE::find_state_chunk(FILEIO *fio, vm_state_ident_t *vm_state_i)
{
	return find_state_chunk(fio, this_class_name, this_identifier, vm_state_i);
}

/// Function to absorb bugs that occurred in the past.
void DEVICE::adjust_chunk_size(vm_state_ident_t *vm_state_i, uint32_t &chunk_size)
{
#ifdef _MBS1
	// fix incorrect size on MEMORY class between V0.2.0 to V0.2.6 of mbs1
	if (strncmp("MEMORY", vm_state_i->class_name, 12) == 0
	&&  strncmp("", vm_state_i->identifier, 4) == 0
	&&  Uint16_LE(vm_state_i->version) == 0x44) {
		chunk_size += 0x1ac30;
		chunk_size &= 0xfffffff0;
	}
#endif
}

// for debug

void DEVICE::dummyf(const void *format, ...) {}
