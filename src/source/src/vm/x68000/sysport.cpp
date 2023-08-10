/** @file sysport.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ system port ]
*/

#include "sysport.h"
#include "../../emumsg.h"
#include "../vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
//#include "../mc68000_consts.h"
#include "display.h"
#include "memory.h"
#include "board.h"
#include "keyboard.h"

void SYSPORT::initialize()
{
	m_power_off = 0x55555555;
}

void SYSPORT::reset()
{
	m_power_off = 0x55555555;
}

//void SYSPORT::cancel_my_event(int &id)
//{
//	if (id >= 0) cancel_event(this, id);
//	id = -1;
//}

void SYSPORT::write_io8(uint32_t addr, uint32_t data)
{
	// $E8E000
	switch(addr & 0x7) {
	case 0x00:
		// contrast
		d_display->write_signal(DISPLAY::SIG_CONTRAST, data, 0x0f);
		break;
	case 0x01:
		// tv control
		break;
	case 0x02:
		// screen developper control
		break;
	case 0x03:
		// HRL / NMI / KEY
		if (data & 4) {
			// negate NMI signal 
			d_board->write_signal(SIG_CPU_IRQ, 0, 0x80);	// IPL7
		}
		// keyboard send enable
		d_keyboard->write_signal(KEYBOARD::SIG_KEY_ENABLE_SEND, data, 0x08);
		break;
	case 0x06:
		// SRAM write enable
		d_memory->write_signal(MEMORY::SIG_SRAM_WRITABLE, data, 0xff);
		break;
	case 0x07:
		// power off control
		m_power_off <<= 8;
		m_power_off |= (data & 0x0f);
		if ((m_power_off & 0xffffff) == 0x000f0f) {
			// power off / restart
			if (d_board->read_signal(BOARD::SIG_BOARD_POWER)) pConfig->now_power_off = true;
			emumsg.Send(EMUMSG_ID_RESET);
		}
		break;
	default:
		break;
	}
}

uint32_t SYSPORT::read_io8(uint32_t addr)
{
	// $E8E000
	uint32_t data = 0xff;
	switch(addr & 0x7) {
	case 0x00:
		// contrast
		data = d_display->read_signal(DISPLAY::SIG_CONTRAST);
		break;
	case 0x01:
		// tv control
		data = 0xf7;
		break;
	case 0x03:
		// HRL / NMI / KEY
		data = 0xf8;
		break;
	case 0x05:
		// Clock 10MHz is 0xff / 16MHz(XVI) is 0xfe / x68030 is 0xdc?
		data = 0xff;
		break;
	default:
		break;
	}
	return data;
}

#ifdef USE_DEBUGGER
uint32_t SYSPORT::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	switch(addr & 0x7) {
	case 0x06:
		// SRAM write enable
		data = d_memory->read_signal(MEMORY::SIG_SRAM_WRITABLE);
		break;
	case 0x07:
		// power off control
		data = (m_power_off & 0xffffff);
		break;
	default:
		data = read_io8(addr);
	}
	return data;
}
#endif

void SYSPORT::write_signal(int id, uint32_t data, uint32_t mask)
{
//	uint16_t prev = 0;
	switch (id) {
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		reset();
		break;
	}
}

void SYSPORT::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------

void SYSPORT::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	vm_state.m_power_off = Uint32_LE(m_power_off);	///< set id to go power off phase

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool SYSPORT::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	m_power_off = Uint32_LE(vm_state.m_power_off);	///< set id to go power off phase

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
static const _TCHAR *c_reg_names[] = {
	_T("CONTRAST"),
	_T("TV_CONTROL"),
	_T("DEVELOPPER"),
	_T("KEY_NMI_HRL"),
	_T("RESERVED4"),
	_T("CPUCLK_SEL"),
	_T("SRAM_WRITE"),
	_T("POWEROFF"),
	NULL
};

bool SYSPORT::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	uint32_t num = find_debug_reg_name(c_reg_names, reg);
	return debug_write_reg(num, data);
}

bool SYSPORT::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < 8) {
		write_io8(reg_num, data);
		return true;
	}
	return false;
}

void SYSPORT::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("SYSTEM PORT:\n"));
	for(int reg_num=0; reg_num<8; reg_num++) {
		if (reg_num == 4) {
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
			continue;
		}
		uint32_t data = debug_read_io8(reg_num);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X(%s):%02X"), reg_num, c_reg_names[reg_num], data);
	}
}
#endif
