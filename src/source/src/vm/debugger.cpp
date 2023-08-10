/** @file debugger.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2014.09.02-

	@brief [ debugger ]

	@note Modified by Sasaji at 2016.02.01 -
*/

#include "debugger.h"

#ifdef USE_DEBUGGER

#include "../emu.h"
#include "../config.h"
#include "../cmutex.h"
#include "../utility.h"
#include "../osd/debugger_console.h"


// ----------------------------------------------------------------------------

DEBUGGER::DEBUGGER(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEBUGGER_BPOINTS(parent_vm, parent_emu, identifier), DEBUGGER_SYMBOLS()
{
	dc = NULL;
	d_mem = d_io = NULL;
}

DEBUGGER::~DEBUGGER()
{
}

// ----------------------------------------------------------------------------

bool DEBUGGER::reach_break_point_at(uint32_t addr)
{
	if (m_now_debugging) {
		check_break_points(addr);
		if(m_now_going > 0) {
			m_now_going--;
		}
		if(!m_now_going) {
			m_now_suspended = true;
		}
		return m_now_suspended;
	} else {
		return false;
	}
}

void DEBUGGER::check_break_points(uint32_t addr)
{
#ifdef _MBS1
	find_fetch_break_trace_points(this, BreakPoints::BP_FETCH_OP, addr, 1);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	find_fetch_break_trace_points(this, BreakPoints::BP_FETCH_OP_PH, phy_addr, 1);
#else
	find_fetch_break_trace_points(this, BreakPoints::BP_FETCH_OP, addr, 1);
#endif
}

void DEBUGGER::check_intr_break_points(uint32_t addr, uint32_t mask)
{
	find_intr_break_trace_points(this, BreakPoints::BP_INTERRUPT, addr, mask);
}

void DEBUGGER::check_exception_break_points(uint32_t addr, uint32_t vector)
{
	find_exception_break_trace_points(this, BreakPoints::BP_EXCEPTION, addr, vector);
}

// ----------------------------------------------------------------------------

void DEBUGGER::write_data8(uint32_t addr, uint32_t data)
{
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
	d_mem->write_data8(addr, data);
	find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, addr, data & 0xff);
}
uint32_t DEBUGGER::read_data8(uint32_t addr)
{
	uint32_t data = d_mem->read_data8(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER::write_data16(uint32_t addr, uint32_t data)
{
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 2, data & 0xffff);
	d_mem->write_data16(addr, data);
}
uint32_t DEBUGGER::read_data16(uint32_t addr)
{
	uint32_t data = d_mem->read_data16(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER::write_data32(uint32_t addr, uint32_t data)
{
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 4, data);
	d_mem->write_data32(addr, data);
}
uint32_t DEBUGGER::read_data32(uint32_t addr)
{
	uint32_t data = d_mem->read_data32(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 4, data);
	return data;
}
void DEBUGGER::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
#ifdef _MBS1
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY_PH, phy_addr, 1, data & 0xff);
#else
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
#endif
	d_mem->write_data8w(addr, data, wait);
#ifdef _MBS1
	find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, phy_addr, data & 0xff);
#else
	find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, addr, data & 0xff);
#endif
}
uint32_t DEBUGGER::read_data8w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data8w(addr, wait);
#ifdef _MBS1
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY_PH, phy_addr, 1, data & 0xff);
#else
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
#endif
	return data;
}
void DEBUGGER::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 2, data & 0xffff);
	d_mem->write_data16w(addr, data, wait);
}
uint32_t DEBUGGER::read_data16w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data16w(addr, wait);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 4, data);
	d_mem->write_data32w(addr, data, wait);
}
uint32_t DEBUGGER::read_data32w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data32w(addr, wait);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 4, data);
	return data;
}
uint32_t DEBUGGER::fetch_op(uint32_t addr, int *wait)
{
	uint32_t data = d_mem->fetch_op(addr, wait);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data);
	return data;
}
void DEBUGGER::latch_address(uint32_t addr, int *wait)
{
	d_mem->latch_address(addr, wait);
}

void DEBUGGER::write_io8(uint32_t addr, uint32_t data)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 1, data & 0xff);
	d_io->write_io8(addr, data);
}
uint32_t DEBUGGER::read_io8(uint32_t addr)
{
	uint32_t data = d_io->read_io8(addr);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER::write_io16(uint32_t addr, uint32_t data)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 2, data & 0xffff);
	d_io->write_io16(addr, data);
}
uint32_t DEBUGGER::read_io16(uint32_t addr)
{
	uint32_t data = d_io->read_io16(addr);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER::write_io32(uint32_t addr, uint32_t data)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 4, data);
	d_io->write_io32(addr, data);
}
uint32_t DEBUGGER::read_io32(uint32_t addr)
{
	uint32_t data = d_io->read_io32(addr);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 4, data);
	return data;
}
void DEBUGGER::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 1, data & 0xff);
	d_io->write_io8w(addr, data, wait);
}
uint32_t DEBUGGER::read_io8w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io8w(addr, wait);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER::write_io16w(uint32_t addr, uint32_t data, int* wait)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 2, data & 0xffff);
	d_io->write_io16w(addr, data, wait);
}
uint32_t DEBUGGER::read_io16w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io16w(addr, wait);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER::write_io32w(uint32_t addr, uint32_t data, int* wait)
{
	find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 4, data);
	d_io->write_io32w(addr, data, wait);
}
uint32_t DEBUGGER::read_io32w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io32w(addr, wait);
	find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 4, data);
	return data;
}

void DEBUGGER::write_dma_data_n(uint32_t addr, uint32_t data, int width)
{
#ifdef _MBS1
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, width, data);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY_PH, phy_addr, width, data);
#else
	find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, width, data);
#endif
	d_mem->write_dma_data_n(addr, data, width);
#ifdef _MBS1
	find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, phy_addr, data);
#else
	find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, addr, data);
#endif
}
uint32_t DEBUGGER::read_dma_data_n(uint32_t addr, int width)
{
	uint32_t data = d_mem->read_dma_data_n(addr, width);
#ifdef _MBS1
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, width, data);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY_PH, phy_addr, width, data);
#else
	find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, width, data);
#endif
	return data;
}

void DEBUGGER::write_signal(int id, uint32_t data, uint32_t mask)
{
	d_mem->write_signal(id, data, mask);
}
uint32_t DEBUGGER::read_signal(int id)
{
	return d_mem->read_signal(id);
}

//void DEBUGGER::add_cpu_trace(uint32_t pc)
//{
//	if(prev_cpu_trace != pc) {
//		cpu_trace[cpu_trace_ptr++] = prev_cpu_trace = pc;
//		cpu_trace_ptr &= (MAX_CPU_TRACE - 1);
//	}
//}

void DEBUGGER::set_debugger_console(DebuggerConsole *dc_)
{
	dc = dc_;
}

void DEBUGGER::clear_suspend()
{
	DEBUGGER_BASE::clear_suspend();
	m_now_basicreason = false;
}

/****************************************************************************/

DEBUGGER_BUS::DEBUGGER_BUS(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEBUGGER_BUS_BASE(parent_vm, parent_emu, identifier)
{
	d_parent = NULL;
	d_mem = d_io = NULL;
}

DEBUGGER_BUS::~DEBUGGER_BUS()
{
}

// ----------------------------------------------------------------------------

void DEBUGGER_BUS::write_data8(uint32_t addr, uint32_t data)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
	d_mem->write_data8(addr, data);
	d_parent->find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, addr, data & 0xff);
}
uint32_t DEBUGGER_BUS::read_data8(uint32_t addr)
{
	uint32_t data = d_mem->read_data8(addr);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER_BUS::write_data16(uint32_t addr, uint32_t data)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 2, data & 0xffff);
	d_mem->write_data16(addr, data);
}
uint32_t DEBUGGER_BUS::read_data16(uint32_t addr)
{
	uint32_t data = d_mem->read_data16(addr);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER_BUS::write_data32(uint32_t addr, uint32_t data)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 4, data);
	d_mem->write_data32(addr, data);
}
uint32_t DEBUGGER_BUS::read_data32(uint32_t addr)
{
	uint32_t data = d_mem->read_data32(addr);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 4, data);
	return data;
}
void DEBUGGER_BUS::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
#ifdef _MBS1
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY_PH, phy_addr, 1, data & 0xff);
#else
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
#endif
	d_mem->write_data8w(addr, data, wait);
#ifdef _MBS1
	d_parent->find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, phy_addr, data & 0xff);
#else
	d_parent->find_basic_break_trace_points(this, BreakPoints::BP_BASIC_NUMBER, d_mem, addr, data & 0xff);
#endif
}
uint32_t DEBUGGER_BUS::read_data8w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data8w(addr, wait);
#ifdef _MBS1
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
	uint32_t phy_addr = d_mem->debug_latch_address(addr);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY_PH, phy_addr, 1, data & 0xff);
#else
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
#endif
	return data;
}
void DEBUGGER_BUS::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 2, data & 0xffff);
	d_mem->write_data16w(addr, data, wait);
}
uint32_t DEBUGGER_BUS::read_data16w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data16w(addr, wait);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER_BUS::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 4, data);
	d_mem->write_data32w(addr, data, wait);
}
uint32_t DEBUGGER_BUS::read_data32w(uint32_t addr, int* wait)
{
	uint32_t data = d_mem->read_data32w(addr, wait);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 4, data);
	return data;
}
uint32_t DEBUGGER_BUS::fetch_op(uint32_t addr, int *wait)
{
	uint32_t data = d_mem->fetch_op(addr, wait);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data);
	return data;
}
void DEBUGGER_BUS::latch_address(uint32_t addr, int *wait)
{
	d_mem->latch_address(addr, wait);
}

void DEBUGGER_BUS::write_io8(uint32_t addr, uint32_t data)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 1, data & 0xff);
	d_io->write_io8(addr, data);
}
uint32_t DEBUGGER_BUS::read_io8(uint32_t addr)
{
	uint32_t data = d_io->read_io8(addr);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER_BUS::write_io16(uint32_t addr, uint32_t data)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 2, data & 0xffff);
	d_io->write_io16(addr, data);
}
uint32_t DEBUGGER_BUS::read_io16(uint32_t addr)
{
	uint32_t data = d_io->read_io16(addr);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER_BUS::write_io32(uint32_t addr, uint32_t data)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 4, data);
	d_io->write_io32(addr, data);
}
uint32_t DEBUGGER_BUS::read_io32(uint32_t addr)
{
	uint32_t data = d_io->read_io32(addr);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 4, data);
	return data;
}
void DEBUGGER_BUS::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 1, data & 0xff);
	d_io->write_io8w(addr, data, wait);
}
uint32_t DEBUGGER_BUS::read_io8w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io8w(addr, wait);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 1, data & 0xff);
	return data;
}
void DEBUGGER_BUS::write_io16w(uint32_t addr, uint32_t data, int* wait)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 2, data & 0xffff);
	d_io->write_io16w(addr, data, wait);
}
uint32_t DEBUGGER_BUS::read_io16w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io16w(addr, wait);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 2, data & 0xffff);
	return data;
}
void DEBUGGER_BUS::write_io32w(uint32_t addr, uint32_t data, int* wait)
{
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 4, data);
	d_io->write_io32w(addr, data, wait);
}
uint32_t DEBUGGER_BUS::read_io32w(uint32_t addr, int* wait)
{
	uint32_t data = d_io->read_io32w(addr, wait);
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 4, data);
	return data;
}

void DEBUGGER_BUS::write_dma_data_n(uint32_t addr, uint32_t data, int width)
{
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, width, data);
	d_mem->write_dma_data_n(addr, data, width);
}
uint32_t DEBUGGER_BUS::read_dma_data_n(uint32_t addr, int width)
{
	uint32_t data = d_mem->read_dma_data_n(addr, width);
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, width, data);
	return data;
}
void DEBUGGER_BUS::write_dma_io8(uint32_t addr, uint32_t data)
{
#ifdef USE_DMA_MEMORY_MAPPED_IO
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 1, data & 0xff);
#else
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 1, data & 0xff);
#endif
	d_io->write_dma_io8(addr, data);
}
uint32_t DEBUGGER_BUS::read_dma_io8(uint32_t addr)
{
	uint32_t data = d_io->read_dma_io8(addr);
#ifdef USE_DMA_MEMORY_MAPPED_IO
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 1, data & 0xff);
#else
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 1, data & 0xff);
#endif
	return data;
}
void DEBUGGER_BUS::write_dma_io16(uint32_t addr, uint32_t data)
{
#ifdef USE_DMA_MEMORY_MAPPED_IO
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_WRITE_MEMORY, addr, 2, data & 0xffff);
#else
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_OUTPUT_IO, addr, 2, data & 0xffff);
#endif
	d_io->write_dma_io16(addr, data);
}
uint32_t DEBUGGER_BUS::read_dma_io16(uint32_t addr)
{
	uint32_t data = d_io->read_dma_io16(addr);
#ifdef USE_DMA_MEMORY_MAPPED_IO
	d_parent->find_mem_break_trace_points(this, BreakPoints::BP_READ_MEMORY, addr, 2, data & 0xffff);
#else
	d_parent->find_io_break_trace_points(this, BreakPoints::BP_INPUT_IO, addr, 2, data & 0xffff);
#endif
	return data;
}

void DEBUGGER_BUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	d_mem->write_signal(id, data, mask);
}
uint32_t DEBUGGER_BUS::read_signal(int id)
{
	return d_mem->read_signal(id);
}

void DEBUGGER_BUS::update_intr_condition()
{
	d_mem->update_intr_condition();
}


uint32_t DEBUGGER_BUS::debug_read_data8(int type, uint32_t addr)
{
	return d_mem->debug_read_data8(type, addr);
}
uint32_t DEBUGGER_BUS::debug_read_data16(int type, uint32_t addr)
{
	return d_mem->debug_read_data16(type, addr);
}
uint32_t DEBUGGER_BUS::debug_read_data32(int type, uint32_t addr)
{
	return d_mem->debug_read_data32(type, addr);
}

#endif
