/** @file scsi.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.23 -

	@brief [ Small Computer System Interface ]
*/

#include "scsi.h"
#include "sasi.h"
#include "../../emumsg.h"
#include "../vm.h"
#include "../../config.h"
#include "../../fileio.h"
#include "../../utility.h"
#include "../../logging.h"
#include "../harddisk.h"

#define SCSI_CLOCK 5000000	// X68000 5MHz

#ifdef _DEBUG
//#define OUT_DEBUG_WREG(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_WREG(...)
//#define OUT_DEBUG_RREG(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_RREG(...)
//#define OUT_DEBUG_WTC(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_WTC(...)
//#define OUT_DEBUG_RTC(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_RTC(...)
//#define OUT_DEBUG_SIG(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_SIG(...)
//#define OUT_DEBUG_SEL(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_SEL(...)
//#define OUT_DEBUG_CMD(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_CMD(...)
//#define OUT_DEBUG_DATW(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATW(...)
//#define OUT_DEBUG_DATR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATR(...)
//#define OUT_DEBUG_DATRE(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DATRE(...)
//#define OUT_DEBUG_STAT(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_STAT(...)
//#define OUT_DEBUG_REQ(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REQ(...)
//#define OUT_DEBUG_REQN(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_REQN(...)
//#define OUT_DEBUG_ACK(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_ACK(...)
//#define OUT_DEBUG_BUFF(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_BUFF(...)
//#define OUT_DEBUG_INTR(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_INTR(...)
//#define OUT_DEBUG_DRQ(...) logging->out_debugf(__VA_ARGS__)
#define OUT_DEBUG_DRQ(...)
#else
#define OUT_DEBUG_WREG(...)
#define OUT_DEBUG_RREG(...)
#define OUT_DEBUG_WTC(...)
#define OUT_DEBUG_RTC(...)
#define OUT_DEBUG_SIG(...)
#define OUT_DEBUG_SEL(...)
#define OUT_DEBUG_CMD(...)
#define OUT_DEBUG_DATW(...)
#define OUT_DEBUG_DATR(...)
#define OUT_DEBUG_DATRE(...)
#define OUT_DEBUG_STAT(...)
#define OUT_DEBUG_REQ(...)
#define OUT_DEBUG_REQN(...)
#define OUT_DEBUG_ACK(...)
#define OUT_DEBUG_BUFF(...)
#define OUT_DEBUG_INTR(...)
#define OUT_DEBUG_DRQ(...)
#endif
#define DEBUG_PREFIX _T("SCSI")

// ----------------------------------------------------------------------

SCSI::SCSI(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: SXSI_HOST(parent_vm, parent_emu, identifier)
{
	set_class_name("SCSI");

//	init_output_signals(&outputs_irq);
//	init_output_signals(&outputs_drq);
		
#ifdef USE_SCSI_SIG_OUTPUTS
	init_output_signals(&outputs_bsy);
	init_output_signals(&outputs_cd);
	init_output_signals(&outputs_io);
	init_output_signals(&outputs_msg);
	init_output_signals(&outputs_req);
		
	init_output_signals(&outputs_dat);
	init_output_signals(&outputs_sel);
	init_output_signals(&outputs_atn);
	init_output_signals(&outputs_ack);
	init_output_signals(&outputs_rst);
#endif

	m_buffer.allocate(8);

	int max_id = USE_SCSI_HARD_DISKS / SCSI_UNITS_PER_CTRL;
	for(int id=0; id<max_id; id++) {
		SCSI_CTRL_RELAY *relay = new SCSI_CTRL_RELAY(vm, emu, "C", this, id);
		d_ctrls->push_back(relay);
	}
}

SCSI::~SCSI()
{
//	delete d_ctrls;
}

// ----------------------------------------------------------------------

void SCSI::cancel_my_event(int event_id)
{
	if(m_register_id[event_id] != -1) {
		cancel_event(this, m_register_id[event_id]);
		m_register_id[event_id] = -1;
	}
}

void SCSI::cancel_my_events()
{
	for(int i = 0; i < EVENT_IDS_MAX; i++) {
		cancel_my_event(i);
	}
}

void SCSI::register_my_event(int event_id, double usec)
{
	register_event(this, event_id, usec, false, &m_register_id[event_id]);
}

void SCSI::register_my_event_by_clock(int event_id, int clock)
{
	register_event_by_clock(this, event_id, clock, false, &m_register_id[event_id]);
}

// ----------------------------------------------------------------------

void SCSI::initialize()
{
	SXSI_HOST::initialize();

	d_selected_ctrl = NULL;

	for(int i = 0; i < EVENT_IDS_MAX; i++) {
		m_register_id[i] = -1;
	}
	for(int i = 0; i < EVENT_ARGS_MAX; i++) {
		m_event_arg[i] = 0;
	}

	m_bdid = 0;
	for(int i=0; i<SCSI_REGS_END; i++) {
		m_regs[i] = 0;
	}
	m_regs[SCSI_REG_SCTL] = SCTL_RESET_AND_DISABLE;
	m_trans = 0;
	m_regs[SCSI_REG_SSTS] |= SSTS_TC_IS_ZERO;
	for(int i=0; i<BUSTYPE_MAX; i++) {
		m_bus_status[i] = 0;
	}

//	m_phase = PHASE_BUSFREE;

	m_pre_ints = 0;
	m_pre_atn = false;
	m_pre_drq = false;
	m_retry_selection = 0;
//	m_now_iack = false;

	m_buffer.clear();
	m_regs[SCSI_REG_SSTS] = SSTS_DREG_EMPTY;
	
	set_irq(false);
	set_drq(false);
}

void SCSI::release()
{
	//  devices are released on destractor of VM
}

void SCSI::reset()
{
	load_wav();

	// clear the BDID register only power on reset
	m_regs[SCSI_REG_BDID] = 0;
	m_bdid = 0;

	warm_reset(true);
}

void SCSI::warm_reset(bool por)
{
	if (!por) {
//		for(int id=0; id<(int)d_ctrls->size(); id++) {
//			d_ctrls->at(id)->warm_reset(por);
//		}
		cancel_my_events();
	} else {
		// events were already canceled by EVENT::reset()
		for(int i = 0; i < EVENT_IDS_MAX; i++) {
			m_register_id[i] = -1;
		}
	}

	d_selected_ctrl = NULL;

	// don't clear the BDID register
	for(int i=SCSI_REG_SCTL; i<SCSI_REGS_END; i++) {
		m_regs[i] = 0;
	}
	m_regs[SCSI_REG_SCTL] = SCTL_RESET_AND_DISABLE;
	m_trans = 0;
	m_regs[SCSI_REG_SSTS] |= SSTS_TC_IS_ZERO;
	for(int i=0; i<BUSTYPE_MAX; i++) {
		m_bus_status[i] = 0;
	}

//	m_phase = PHASE_BUSFREE;

	m_pre_ints = 0;
	m_pre_atn = false;
	m_pre_drq = false;
	m_retry_selection = 0;
//	m_now_iack = false;

	m_buffer.clear();
	m_regs[SCSI_REG_SSTS] = SSTS_DREG_EMPTY;

	set_irq(false);
	set_drq(false);
}

void SCSI::write_io8(uint32_t addr, uint32_t data)
{
	// addr is $EA0000 >> 1 (CZ-6BS1)
	// addr is $E96020 >> 1 (inner)
	switch(addr & 0xf) {
	case 0x00:
		// Bus Device ID (BDID)
		data &= 0x7;
		m_bdid = data;
		m_regs[SCSI_REG_BDID] = (1 << data);
		OUT_DEBUG_WREG(_T("%s H: WREG BDID:%d(0x%02x)"), DEBUG_PREFIX, m_bdid, m_regs[SCSI_REG_BDID]);
		break;
	case 0x01:
		// SPC control (SCTL)
		process_spc_control(data & 0xff);
		break;
	case 0x02:
		// Command (SCMD)
		process_spc_command(data & 0xff);
		break;
	case 0x04:
		// Reset Interrupt (INTS)
		reset_interrupt_status(data & 0xff);
		break;
	case 0x05:
		// SPC Diagnostic Control (SDGC)
		m_regs[SCSI_REG_SDGC] = (data & 0xef);
		OUT_DEBUG_WREG(_T("%s H: WREG SDGC:0x%02x"), DEBUG_PREFIX, m_regs[SCSI_REG_SDGC]);
		break;
	case 0x06:
		// Unknown
		break;
	case 0x08:
		// Phase Control (PCTL)
		write_pctl_register(data);
		break;
	case 0x0a:
		// Data Register (DREG)
		write_data_register(data);
		break;
	case 0x0b:
		// Temporary Register (TEMP)
		write_temp_register(data);
		break;
	case 0x0c:
		// Transfer Counter High (TCH)
		m_trans = ((m_trans & 0x00ffff) | ((data & 0xff) << 16));
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);
		OUT_DEBUG_WREG(_T("%s H: WREG TCH:0x%02x Trans:0x%06x SSTS:0x%02x"), DEBUG_PREFIX, data, m_trans, m_regs[SCSI_REG_SSTS]);
		break;
	case 0x0d:
		// Transfer Counter Middle (TCM)
		m_trans = ((m_trans & 0xff00ff) | ((data & 0xff) << 8));
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);
		OUT_DEBUG_WREG(_T("%s H: WREG TCM:0x%02x Trans:0x%06x SSTS:0x%02x"), DEBUG_PREFIX, data, m_trans, m_regs[SCSI_REG_SSTS]);
		break;
	case 0x0e:
		// Transfer Counter Low (TCL)
		m_trans = ((m_trans & 0xffff00) | (data & 0xff));
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);
		m_regs[SCSI_REG_MBC] = (data & 0x0f);
		OUT_DEBUG_WREG(_T("%s H: WREG TCL:0x%02x Trans:0x%06x SSTS:0x%02x"), DEBUG_PREFIX, data, m_trans, m_regs[SCSI_REG_SSTS]);
		break;
	default:
		OUT_DEBUG_WREG(_T("%s H: WREG unknown(%d):0x%02x"), DEBUG_PREFIX, (int)(addr & 0xf), data);
		break;
	}
}

#ifdef _DEBUG
static uint32_t prev_ints = 0xff;
#endif

/// now interrupt, send vector number
uint32_t SCSI::read_external_data8(uint32_t addr)
{
	uint32_t data = 0xf6;
	OUT_DEBUG_INTR(_T("%s H: Read Interrupt Vector:0x%02x Addr:0x%08x"), DEBUG_PREFIX, data, ((addr << 1) | 1));
	return data;
}

uint32_t SCSI::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;

//	if (m_now_iack) {
//		// now interrupt, send vector number
//		data = 0xf6;
//		OUT_DEBUG_INTR(_T("%s H: Read Interrupt Vector:0x%02x Addr:0x%08x"), DEBUG_PREFIX, data, ((addr << 1) | 1));
//		return data;
//	}

	// addr is $EA0000 >> 1 (CZ-6BS1)
	// addr is $E96020 >> 1 (inner)
	switch(addr & 0xf) {
	case 0x00:
		// Bus Device ID (BDID)
		data = m_regs[SCSI_REG_BDID];
		OUT_DEBUG_RREG(_T("%s H: RREG BDID:%d(0x%02x)"), DEBUG_PREFIX, m_bdid, data);
		break;
	case 0x01:
		// SPC control (SCTL)
		data = m_regs[SCSI_REG_SCTL];
		OUT_DEBUG_RREG(_T("%s H: RREG SCTL:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x02:
		// Command (SCMD)
		data = m_regs[SCSI_REG_SCMD];
		OUT_DEBUG_RREG(_T("%s H: RREG SCMD:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x04:
		// Interrupt Sense (INTS)
		data = m_regs[SCSI_REG_INTS];
#ifdef _DEBUG
		if (prev_ints != data) {
			OUT_DEBUG_RREG(_T("%s H: RREG INTS:0x%02x -> 0x%02x"), DEBUG_PREFIX, prev_ints, data);
			prev_ints = data;
		}
#endif
		break;
	case 0x05:
		// Phase Sense (PSNS)
		data = m_regs[SCSI_REG_PSNS];
		OUT_DEBUG_RREG(_T("%s H: RREG PSNS:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x06:
		// SPC Status (SSTS)
		data = m_regs[SCSI_REG_SSTS];
		OUT_DEBUG_RREG(_T("%s H: RREG SSTS:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x07:
		// SPC Error Status (SERR)
		data = (m_regs[SCSI_REG_SERR] & SERR_MASK);
		OUT_DEBUG_RREG(_T("%s H: RREG SERR:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x08:
		// Phase Control (PCTL)
		data = (m_regs[SCSI_REG_PCTL] & PCTL_MASK);
		OUT_DEBUG_RREG(_T("%s H: RREG PCTL:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x09:
		// Modified Byte Counter (MBC)
		data = m_regs[SCSI_REG_MBC];
		OUT_DEBUG_RREG(_T("%s H: RREG MBC:0x%02x"), DEBUG_PREFIX, data);
		break;
	case 0x0a:
		// Data Register (DREG)
		data = read_data_register();
		break;
	case 0x0b:
		// Temporary Register (TEMP)
		data = read_temp_register();
		break;
	case 0x0c:
		// Transfer Counter High (TCH)
		data = ((m_trans >> 16) & 0xff);
		OUT_DEBUG_RREG(_T("%s H: RREG TCH:0x%02x Trans:%06x"), DEBUG_PREFIX, data, m_trans);
		break;
	case 0x0d:
		// Transfer Counter Middle (TCM)
		data = ((m_trans >> 8) & 0xff);
		OUT_DEBUG_RREG(_T("%s H: RREG TCM:0x%02x Trans:%06x"), DEBUG_PREFIX, data, m_trans);
		break;
	case 0x0e:
		// Transfer Counter Low (TCL)
		data = (m_trans & 0xff);
		OUT_DEBUG_RREG(_T("%s H: RREG TCL:0x%02x Trans:%06x"), DEBUG_PREFIX, data, m_trans);
		break;
	default:
		OUT_DEBUG_RREG(_T("%s H: RREG unknown(%d):0x%02x"), DEBUG_PREFIX, (int)(addr & 0xf), data);
		break;
	}
	return data;
}

uint32_t SCSI::get_bus_signal() const
{
	return m_regs[SCSI_REG_PSNS];
}

#ifdef USE_SCSI_SIG_OUTPUTS
#ifdef SCSI_HOST_WIDE
void SCSI::write_dma_io16(uint32_t addr, uint32_t data)
#else
void SCSI::write_dma_io8(uint32_t addr, uint32_t data)
#endif
{
	OUT_DEBUG_DATW(_T("%s H: Write DMA %02X\n"), DEBUG_PREFIX, data);

	write_signals(&outputs_dat, data);
	
	#ifdef SCSI_HOST_AUTO_ACK
		// set ack to clear req signal immediately
		if(bsy_status && !io_status) {
			this->write_signal(SIG_SCSI_ACK, 1, 1);
		}
	#endif
}

#ifdef SCSI_HOST_WIDE
uint32_t SCSI::read_dma_io16(uint32_t addr)
#else
uint32_t SCSI::read_dma_io8(uint32_t addr)
#endif
{
	uint32_t value = m_buffer.read();
	m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_FULL;
	BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_EMPTY, m_buffer.empty());

	OUT_DEBUG_DATW(_T("%s H: Read DMA %02X\n"), DEBUG_PREFIX, value);

	#ifdef SCSI_HOST_AUTO_ACK
		// set ack to clear req signal immediately
		if(bsy_status && io_status) {
			this->write_signal(SIG_SCSI_ACK, 1, 1);
		}
	#endif
	return value;
}

uint32_t SCSI::read_signal(int id)
{
	// SCSI signals
	switch (id) {
	case SIG_SCSI_BSY:
		return m_bus_status[BUSSTS_BSY] ? 0xffffffff : 0;
		
	case SIG_SCSI_CD:
		return m_bus_status[BUSSTS_CD]  ? 0xffffffff : 0;
		
	case SIG_SCSI_IO:
		return m_bus_status[BUSSTS_IO]  ? 0xffffffff : 0;
		
	case SIG_SCSI_MSG:
		return m_bus_status[BUSSTS_MSG] ? 0xffffffff : 0;
		
	case SIG_SCSI_REQ:
		return m_bus_status[BUSSTS_REQ] ? 0xffffffff : 0;
		
	case SIG_SCSI_ACK:
		return m_bus_status[BUSSTS_ACK] ? 0xffffffff : 0;
	}
	
	// access lamp
//	uint32_t value = access ? 0xffffffff : 0;
//	access = false;
	return 0;
}
#endif

void SCSI::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifdef USE_SCSI_SIG_OUTPUTS
	switch(id) {
//	case SIG_IACK:
//		// receive interrupt ack signal
//		m_now_iack = ((data & mask) != 0);
//		break;
	// from initiator
	case SIG_SCSI_SEL:
		OUT_DEBUG_SIG(_T("%s H: SEL = %d\n"), DEBUG_PREFIX, (data & mask) ? 1 : 0);

		write_signals(&outputs_sel, (data & mask) ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_ATN:
//		OUT_DEBUG_SIG(_T("%s H: ATN = %d\n"), DEBUG_PREFIX, (data & mask) ? 1 : 0);

		write_signals(&outputs_atn, (data & mask) ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_ACK:
//		OUT_DEBUG_SIG(_T("%s H: ACK = %d\n"), DEBUG_PREFIX, (data & mask) ? 1 : 0);

		write_signals(&outputs_ack, (data & mask) ? 0xffffffff : 0);
		m_bus_status[BUSSTS_ACK] = data & mask;
		break;
		
	case SIG_SCSI_RST:
		OUT_DEBUG_SIG(_T("%s H: RST = %d\n"), DEBUG_PREFIX, (data & mask) ? 1 : 0);

		write_signals(&outputs_rst, (data & mask) ? 0xffffffff : 0);
		break;
		
	// from target
	case SIG_SCSI_DAT:
		m_buffer.write(data);
		m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_EMPTY;
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_FULL, m_buffer.full());
		break;
		
	case SIG_SCSI_BSY:
		m_bus_status[BUSSTS_BSY] &= ~mask;
		m_bus_status[BUSSTS_BSY] |= (data & mask);
		write_signals(&outputs_bsy, m_bus_status[BUSSTS_BSY] ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_CD:
		m_bus_status[BUSSTS_CD] &= ~mask;
		m_bus_status[BUSSTS_CD] |= (data & mask);
		write_signals(&outputs_cd, m_bus_status[BUSSTS_CD] ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_IO:
		m_bus_status[BUSSTS_IO] &= ~mask;
		m_bus_status[BUSSTS_IO] |= (data & mask);
		write_signals(&outputs_io, m_bus_status[BUSSTS_IO] ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_MSG:
		m_bus_status[BUSSTS_MSG] &= ~mask;
		m_bus_status[BUSSTS_MSG] |= (data & mask);
		write_signals(&outputs_msg, m_bus_status[BUSSTS_MSG] ? 0xffffffff : 0);
		break;

	case SIG_SCSI_REQ:
		{
			uint32_t prev_status = m_bus_status[BUSSTS_REQ];
			m_bus_status[BUSSTS_REQ] &= ~mask;
			m_bus_status[BUSSTS_REQ] |= (data & mask);
			
			if(!prev_status && m_bus_status[BUSSTS_REQ]) {
				// L -> H
//				if(bsy_status) {
					if(!m_bus_status[BUSSTS_CD] && !m_bus_status[BUSSTS_MSG]) {
						// data phase
						set_drq(true);
//						access = true;
					} else if(m_bus_status[BUSSTS_CD]) {
						// command/status/message phase
						set_irq(true);
					}
//				}
			} else if(prev_status && !m_bus_status[BUSSTS_REQ]) {
				// H -> L
				set_drq(false);
				set_irq(false);
				#ifdef SCSI_HOST_AUTO_ACK
					this->write_signal(SIG_SCSI_ACK, 0, 0);
				#endif
			}
			write_signals(&outputs_req, m_bus_status[BUSSTS_REQ] ? 0xffffffff : 0);
		}
		break;
	default:
		break;
	}
#endif
}

void SCSI::event_callback(int event_id, int err)
{
	m_register_id[event_id] = -1;
	switch(event_id) {
	case EVENT_DRQ:
		set_drq(m_pre_drq);
		break;
	case EVENT_NEXT_PHASE:
		switch(m_event_arg[EVENT_ARG_NEXT_PHASE]) {
		case SCSI_CTRL::PHASE_SELECTION:
			selection();
			break;
		default:
			break;
		}
		break;
	case EVENT_SELECTED:
		selected_device();
		break;
	case EVENT_STATUS:
		m_regs[SCSI_REG_INTS] = m_pre_ints;
		m_pre_ints = 0;
		update_irq();
		break;
	default:
		break;
	}
}


void SCSI::process_spc_control(uint32_t data)
{
	if ((m_regs[SCSI_REG_SCTL] & SCTL_RESET_ALL) != 0
		&& (data & SCTL_RESET_ALL) == 0) {
		// negate reset flags
		warm_reset(false);
	}
	OUT_DEBUG_CMD(_T("%s H: WREG SCTL:0x%02x -> 0x%02x"), DEBUG_PREFIX, m_regs[SCSI_REG_SCTL], data);

	m_regs[SCSI_REG_SCTL] = data;
	update_irq();

}

/// writing INTS
void SCSI::reset_interrupt_status(uint32_t data)
{
	uint32_t prev = m_regs[SCSI_REG_INTS]; 
	m_regs[SCSI_REG_INTS] = (m_regs[SCSI_REG_INTS] & ~data);
	update_irq();
	OUT_DEBUG_WREG(_T("%s H: WREG INTS:0x%02x -> Clear:0x%02x"), DEBUG_PREFIX, data, m_regs[SCSI_REG_INTS]);
	if ((prev & data & INTS_TIMEOUT) != 0) {
		// clear timeout
		update_bus_status(m_bdid, BUSTYPE_ATN, false);
		update_ssts(0, SSTS_CONNECTED);
		// restart selection phase
		if (m_trans > 0) {
			m_retry_selection = 1;
//			m_phase = PHASE_BUSFREE;
			process_spc_command_select();
		} else {
			// go busfree phase
			go_idle();
		}
	}
}

/// PCTL
void SCSI::write_pctl_register(uint32_t data)
{
	m_regs[SCSI_REG_PCTL] = (data & 0x87);

	OUT_DEBUG_WREG(_T("%s H: WREG PCTL:0x%02x"), DEBUG_PREFIX, m_regs[SCSI_REG_PCTL]);
}

/// DATA Reg
void SCSI::write_data_register(uint32_t data)
{
	if (!m_buffer.full()) {
		// store wrote data to buffer
		m_buffer.write(data & 0xff);
		m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_EMPTY;
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_FULL, m_buffer.full());

		OUT_DEBUG_BUFF(_T("%s H: Store to buffer (Trans Remain:%u  Buffer Remain:%d)"), DEBUG_PREFIX, m_trans, m_buffer.remain());
	}

	// decrease transfer count
	if (m_trans) {
		m_trans--;
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);
	}

	if (m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) {
		// assert ACK and send data to target
		update_bus_status(m_bdid, BUSTYPE_ACK, true);

		// send ACK and data to target
		// target will negate REQ signal
		if (d_selected_ctrl) {
			d_selected_ctrl->accept_ack();
		}

		// write data to target
		set_drq(false);

		// send data
		m_regs[SCSI_REG_TEMP_SEND] = m_buffer.read();
		m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_FULL;
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_EMPTY, m_buffer.empty());

		OUT_DEBUG_BUFF(_T("%s H: Load from buffer (Trans Remain:%u  Buffer Remain:%d)"), DEBUG_PREFIX, m_trans, m_buffer.remain());

		if (d_selected_ctrl) {
			d_selected_ctrl->write_data(m_regs[SCSI_REG_TEMP_SEND]);
		}

		// negate ACK
		// accept_req() will call from target
		// and REQ and drq will assert
		update_bus_status(m_bdid, BUSTYPE_ACK, false);
		// next request
		if (d_selected_ctrl) {
			if ((m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) == 0) {
				// set next signal and phase
				// accept_req() will call from target
				d_selected_ctrl->request_next();
			}
		}

		// trans count is zero
		if (!m_trans) {
			// all data transfered
			if (!(m_regs[SCSI_REG_SCMD] & SCMD_TERMINATION_MODE)) {
				// transfer complete
				command_complete();
			}
		}
	}

	if ((m_regs[SCSI_REG_SSTS] & SSTS_TRANSFER_IN_PROGRESS) != 0 && (m_regs[SCSI_REG_SCMD] & SCMD_PROGRAM_TRANSFER) == 0) {
		// dma transfer
		set_drq_delay(!m_buffer.full());
	} else {
		// program transfer
		set_drq(false);
	}
}

/// TEMP Reg
void SCSI::write_temp_register(uint32_t data)
{
	m_regs[SCSI_REG_TEMP_SEND] = (data & 0xff);
	if (!(m_regs[SCSI_REG_SCMD] & SCMD_INTERCEPT_TRANSFER)) {
		// clear FIFO buffer
		m_buffer.clear();
	}
	// need set zero here after timeout on selection phase
	if (m_regs[SCSI_REG_PSNS] & BUSSTS_SEL) {
		// selection phase complete
		update_bus_status(m_bdid, BUSTYPE_SEL, false);
//		m_phase = PHASE_BUSFREE;
	}
	OUT_DEBUG_WREG(_T("%s H: WREG TEMP_S:0x%02x buff remain:%d"), DEBUG_PREFIX, m_regs[SCSI_REG_TEMP_SEND], m_buffer.remain());
}

uint32_t SCSI::read_temp_register()
{
	// TEMP data is latched in process_spc_command_ack_req()
	uint32_t data = m_regs[SCSI_REG_TEMP_RECV];
	if (!(m_regs[SCSI_REG_SCMD] & SCMD_INTERCEPT_TRANSFER)) {
		// clear FIFO buffer
		m_buffer.clear();
	}
	OUT_DEBUG_RREG(_T("%s H: RREG TEMP_R:0x%02x buffer Count:%d"), DEBUG_PREFIX, data, m_buffer.count());
	return data;
}

uint32_t SCSI::read_data_register()
{
	uint32_t data = 0;
	
	// read data from buffer
	if (!m_buffer.empty()) {
		data = m_buffer.read();
		m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_FULL;
		BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_EMPTY, m_buffer.empty());

		OUT_DEBUG_BUFF(_T("%s H: Load from buffer (Trans Remain:%u  Buffer Count:%d)"), DEBUG_PREFIX, m_trans, m_buffer.count());
	}

	// decrease transfer count
	if (m_trans) m_trans--;
	BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);

#if 0
	OUT_DEBUG_RTC(_T("%s H: RREG DREG:0x%02x PSNS:0x%02x SSTS:0x%02x TC:%04x Buffer Count:%d"), DEBUG_PREFIX
		, data
		, m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]
		, m_trans, m_buffer.count());
#endif

	if (m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) {
		// data accepted
		update_bus_status(m_bdid, BUSTYPE_ACK, true);

		// send ACK and data to target
		// target will negate REQ signal
		if (d_selected_ctrl) {
			d_selected_ctrl->accept_ack();
		}

		if (!m_trans) {
			// all data transfered
			if (!(m_regs[SCSI_REG_SCMD] & SCMD_TERMINATION_MODE)) {
				// transfer complete
				command_complete();
			}
		}

		set_drq(false);
		// negate ACK
		// accept_req() will call from target
		// and REQ and drq will assert
		update_bus_status(m_bdid, BUSTYPE_ACK, false);
		// next request
		if (d_selected_ctrl) {
			if ((m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) == 0) {
				// set next signal and phase
				// accept_req() will call from target on DATA_IN, STATUS, MESSAGE_IN 
				d_selected_ctrl->request_next();
			}
		}
	}

	return data;
}

static const _TCHAR *c_spc_commands[] = {
	_T("Bus Release"),
	_T("Select"),
	_T("Reset ATN"),
	_T("Set ATN"),
	_T("Transfer"),
	_T("Transfer Pause"),
	_T("Reset ACK/REQ"),
	_T("Set ACK/REQ")
};

void SCSI::process_spc_command(uint32_t data)
{
	if (m_regs[SCSI_REG_SCTL] & SCTL_RESET_ALL) {
		// SPC is reset now
		OUT_DEBUG_WREG(_T("%s H: WREG SCMD:0x%02x Now reset, so no process spc command."), DEBUG_PREFIX, data);
		return;
	}
	OUT_DEBUG_WREG(_T("%s H: WREG SCMD:0x%02x %s"), DEBUG_PREFIX, data, c_spc_commands[(data & SCMD_COMMAND_MODE) >> SCMD_COMMAND_SFT]);

	m_regs[SCSI_REG_SCMD] = (data & 0xff);

	switch(data & SCMD_COMMAND_MODE) {
	case SCMD_COMMAND_BUSRELEASE:
		break;
	case SCMD_COMMAND_SELECT:
		m_retry_selection = 0;
		process_spc_command_select();
		break;
	case SCMD_COMMAND_RESET_ATN:
		process_spc_command_atn(false);
		break;
	case SCMD_COMMAND_SET_ATN:
		process_spc_command_atn(true);
		break;
	case SCMD_COMMAND_TRANSFER:
		process_spc_command_transfer();
		break;
	case SCMD_COMMAND_TRANSFER_PAUSE:
		break;
	case SCMD_COMMAND_RESET_ACK_REQ:
		process_spc_command_ack_req(false);
		break;
	case SCMD_COMMAND_SET_ACK_REQ:
		process_spc_command_ack_req(true);
		break;
	}
}

/// @brief select harddisk by id
void SCSI::process_spc_command_select()
{
	if (m_regs[SCSI_REG_PSNS] & BUSSTS_BSY) {
		// cannot selection phase
		return;
	}

	update_ssts(SSTS_SPC_BUSY, SSTS_CONNECTED | SSTS_TRANSFER_IN_PROGRESS);

	double delay = 1.0;
	if (m_regs[SCSI_REG_SCTL] & SCTL_ARBITRATION_ENABLE) {
		arbitration();
		delay += 32000000. / SCSI_CLOCK;
	}

	cancel_my_event(EVENT_NEXT_PHASE);
	m_event_arg[EVENT_ARG_NEXT_PHASE] = SCSI_CTRL::PHASE_SELECTION;
	register_my_event(EVENT_NEXT_PHASE, delay);
}

void SCSI::arbitration()
{
	// go arbitration
//	m_phase = PHASE_ARBITRATION;
	update_bus_status(m_bdid, BUSTYPE_BSY, true);
//	for(int id = 0; id<(int)d_ctrls->size(); id++) {
//		d_ctrls->arbitration();
//	}
}

/// process selection phase
void SCSI::selection() 
{
//	m_phase = PHASE_SELECTION;

	uint32_t next_ssts = 0;
	if (m_regs[SCSI_REG_PSNS] & BUSSTS_IO) {
		// reselect
		next_ssts |= SSTS_CONNECTED_TARGET;
	} else {
		// select
		next_ssts |= SSTS_CONNECTED_INITIATOR;
	}
	update_ssts(next_ssts, SSTS_CONNECTED | SSTS_TRANSFER_IN_PROGRESS);

	update_bus_status(m_bdid, BUSTYPE_BSY, false);
	update_bus_status(m_bdid, BUSTYPE_SEL, true);
	update_bus_status(m_bdid, BUSTYPE_ATN, m_pre_atn);
	m_pre_atn = false;

	d_selected_ctrl = NULL;
	// select a target device
	uint32_t data = (m_regs[SCSI_REG_TEMP_SEND] & ~m_regs[SCSI_REG_BDID]);
	int id = 0;
	for(; id<(int)d_ctrls->size(); id++) {
		uint32_t mask = (uint32_t)(1 << id);
		if (data == mask) {
			// select ID
			d_selected_ctrl = d_ctrls->at(id);
			break;
		}
	}

	if (d_selected_ctrl) {
		if (!d_selected_ctrl->select()) {
			// not mounted
			d_selected_ctrl = NULL;
		}
	}

	double delay = 1.0;
	if (!d_selected_ctrl) {
		// no device found

		// timeout (us)
		if (m_retry_selection) {
			// retry
			delay = (double)m_trans;
		} else {
			// first time
			delay = (double)((m_trans & 0xffff00) | 0xf);
		}
		// timeout is zero means infinity
		if (delay > 0.0) {
			delay = delay * 2000000.0 / SCSI_CLOCK;
			if (delay < 1.0) delay = 1.0;
		}
		if (delay > 0.0) {
			cancel_my_event(EVENT_SELECTED);
			m_pre_ints = INTS_TIMEOUT;
			register_my_event(EVENT_SELECTED, delay);
		}
		return;
	}

	// device is ok
	cancel_my_event(EVENT_SELECTED);
	m_pre_ints = INTS_COMMAND_COMPLETE;
	register_my_event(EVENT_SELECTED, 2.0);

	OUT_DEBUG_SEL(_T("%s H: Select: Req:%02X Select:%d Status:%04X"), DEBUG_PREFIX
		, data
		, d_selected_ctrl ? id : -1
		, m_regs[SCSI_REG_PSNS]); 
}

void SCSI::selected_device()
{
	m_regs[SCSI_REG_INTS] = m_pre_ints;
	m_pre_ints = 0;

	update_ssts(0, SSTS_SPC_BUSY);

	if (m_regs[SCSI_REG_INTS] & INTS_COMMAND_COMPLETE) {
		update_bus_status(m_bdid, BUSTYPE_SEL, false);
		m_trans = ((m_trans & 0xffff00) / 2) | (m_trans & 0xff);
		if (m_regs[SCSI_REG_PSNS] & BUSSTS_ATN) {
			// go to message out phase
//			m_phase = PHASE_MESSAGE_OUT;
			if (d_selected_ctrl) {
				d_selected_ctrl->go_message_out_phase();
			}
		} else {
			// go to command phase
//			m_phase = PHASE_COMMAND;
			if (d_selected_ctrl) {
				d_selected_ctrl->go_command_phase();
			}
		}
	} else {
		// time out
		// go to busfree
		m_trans = 0;
	}
	BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_TC_IS_ZERO, m_trans == 0);
	update_irq();
}

/// re-selection phase from target
void SCSI::reselection(int id)
{
	if (id <= 0 && id < (int)d_ctrls->size() && id != m_bdid) {
		// select ID
		d_selected_ctrl = d_ctrls->at(id);
	}
}

/// @brief set/reset ATN signal
void SCSI::process_spc_command_atn(bool is_set)
{
	m_pre_atn = is_set;

	if (!is_set || (m_regs[SCSI_REG_PSNS] & BUSSTS_BSY) != 0) {
		update_bus_status(m_bdid, BUSTYPE_ATN, is_set);
	}
}

/// @brief start transfer
void SCSI::process_spc_command_transfer()
{
	if (!d_selected_ctrl || (m_regs[SCSI_REG_PCTL] & PCTL_PHASE_MASK) != (m_regs[SCSI_REG_PSNS] & PCTL_PHASE_MASK)) {
		// error : service required
		m_pre_ints = INTS_SERVICE_REQUIRED;
		cancel_my_event(EVENT_STATUS);
		register_my_event(EVENT_STATUS, 2);

		return;
	}

	// start transfer mode
	update_ssts(SSTS_SPC_BUSY | SSTS_TRANSFER_IN_PROGRESS, 0);

	OUT_DEBUG_CMD(_T("%s H: Start Transfer PSNS:0x%02X SSTS:0x%02X TC:%d"), DEBUG_PREFIX, m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS], m_trans);

	if (m_regs[SCSI_REG_PSNS] & BUSSTS_IO) {
		// DATA_IN
		// SCSI bus data is latched in accept_req() on DATA_IN phase
		data_in_and_update_drq();
	} else {
		// DATA_OUT
		if (m_regs[SCSI_REG_SCMD] & SCMD_PROGRAM_TRANSFER) {
			// program transfer
			set_drq(false);
		} else {
			// dma transfer
			set_drq_delay(!m_buffer.full());
		}
	}
}

void SCSI::process_spc_command_ack_req(bool is_set)
{
	switch(m_regs[SCSI_REG_SSTS] & SSTS_CONNECTED) {
	case SSTS_CONNECTED_INITIATOR:
		update_bus_status(m_bdid, BUSTYPE_ACK, is_set);
		// send ACK and data to target
		if (d_selected_ctrl) {
			if (is_set) {
#ifdef _DEBUG
				if (d_selected_ctrl->get_phase() == SCSI_CTRL::PHASE_COMMAND && d_selected_ctrl->get_command_pos() >= 9) {
					OUT_DEBUG_ACK(_T("%s H: Set ACK. PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
						, m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
				}
#endif
				if (m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) {
					// negate REQ
					d_selected_ctrl->accept_ack();

					if (!(m_regs[SCSI_REG_PSNS] & BUSSTS_IO)) {
						// write
						d_selected_ctrl->write_data(m_regs[SCSI_REG_TEMP_SEND]);
					}

					if (m_regs[SCSI_REG_PSNS] & BUSSTS_MSG) {
						update_bus_status(m_bdid, BUSTYPE_ATN, false);
					}
				}
#ifdef _DEBUG
				else {
					// REQ signal is alreay off
					OUT_DEBUG_ACK(_T("%s H: Set ACK but REQ is already off. PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
						, m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
				}
#endif

			} else {
#ifdef _DEBUG
				if (d_selected_ctrl->get_phase() == SCSI_CTRL::PHASE_COMMAND && d_selected_ctrl->get_command_pos() >= 9) {
					OUT_DEBUG_ACK(_T("%s H: Reset ACK. PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
						, m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
				}
#endif
				if ((m_regs[SCSI_REG_PSNS] & BUSSTS_REQ) == 0) {
					d_selected_ctrl->request_next();
				}
			}
		}
		break;
	case SSTS_CONNECTED_TARGET:
		update_bus_status(m_bdid, BUSTYPE_REQ, is_set);
		// TODO: not implemented as target mode
		break;
	default:
		break;
	}
}

/// store temp data to buffer in transfer mode
void SCSI::data_in_and_update_drq()
{
	// DATA_IN
	// now transfer mode
	if (m_regs[SCSI_REG_SSTS] & SSTS_TRANSFER_IN_PROGRESS) {
		// store to buffer
		if (!m_buffer.full()) {
			m_buffer.write(m_regs[SCSI_REG_TEMP_RECV]);
			m_regs[SCSI_REG_SSTS] &= ~SSTS_DREG_EMPTY;
			BIT_ONOFF(m_regs[SCSI_REG_SSTS], SSTS_DREG_FULL, m_buffer.full());
	
			OUT_DEBUG_BUFF(_T("%s H: Store to buffer (Trans Remain:%04x Buffer Count+:%d)"), DEBUG_PREFIX, m_trans, m_buffer.count());
		}

		if (m_regs[SCSI_REG_SCMD] & SCMD_PROGRAM_TRANSFER) {
			// program transfer
			set_drq(false);
		} else {
			// dma transfer
			set_drq_delay(!m_buffer.empty());
		}
	} else {
		set_drq(false);
	}
}

/// asserted REQ signal on the bus
/// receive data from target and store buffer on DATA_IN
void SCSI::accept_req(uint32_t flags)
{
	if (m_regs[SCSI_REG_PSNS] & BUSSTS_IO) {
		// DATA_IN
		if (d_selected_ctrl) {
			// receive data
			m_regs[SCSI_REG_TEMP_RECV] = d_selected_ctrl->read_data();
		}

		// if now transfer mode then above temp data stores to buffer soon 
		data_in_and_update_drq();

	}
	if (flags & AR_COMPLETED) {
		OUT_DEBUG_STAT(_T("%s H: Accept Req Transfer Complete INTS:0x%02X PSNS:0x%02X SSTS:0x%02X TC:%d Buffer Count:%d"), DEBUG_PREFIX
			, m_regs[SCSI_REG_INTS], m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS], m_trans, m_buffer.count());
		command_complete();
	}
}

void SCSI::update_ssts(uint32_t on, uint32_t off)
{
	m_regs[SCSI_REG_SSTS] &= ~off;
	m_regs[SCSI_REG_SSTS] |=  on;
}

const uint8_t SCSI::bus_status_2_sig_map[BUSTYPE_MAX] = {
	BUSSTS_IO,	///< I/O (from control)
	BUSSTS_CD,	///< Control/Data (from control)
	BUSSTS_MSG,	///< Message (from control)
	BUSSTS_BSY,	///< Busy (from control)
	BUSSTS_SEL,	///< Select (to control)
	BUSSTS_ATN,	///< ATN (to control)
	BUSSTS_ACK,	///< ACK (to control)
	BUSSTS_REQ,	///< Request (from control)
};

/// update SCSI bus signals (PSNS)
void SCSI::update_bus_status(int ctrl_num, SXSI_SIGNAL_STATUS_TYPE sig_type, bool onoff)
{
#ifdef _DEBUG
	uint32_t prev = m_bus_status[sig_type];
#endif

	BIT_ONOFF(m_bus_status[sig_type], 1 << ctrl_num, onoff);
	BIT_ONOFF(m_regs[SCSI_REG_PSNS], bus_status_2_sig_map[sig_type], m_bus_status[sig_type] != 0);

#ifdef _DEBUG
	if (prev != m_bus_status[sig_type]) {
		OUT_DEBUG_SIG(_T("%s H: ID%d %s %s PSTS:%02X"), DEBUG_PREFIX
			, ctrl_num
			, c_sigtype_names[sig_type]
			, onoff ? _T("on ") : _T("off")
			, (int)m_regs[SCSI_REG_PSNS]);
	}
#endif
}

void SCSI::update_irq()
{
	set_irq((m_regs[SCSI_REG_SCTL] & SCTL_INTERRUPT_ENABLE) != 0 && m_regs[SCSI_REG_INTS] != 0);
}

void SCSI::set_irq(bool onoff)
{
	write_signals(&outputs_irq, onoff ? 0xffffffff : 0);

	OUT_DEBUG_INTR(_T("%s H: IRQ[%s] SCTL:0x%02X INTS:0x%02X PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
		, onoff ? _T("on") : _T("off")
		, m_regs[SCSI_REG_SCTL], m_regs[SCSI_REG_INTS], m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
}

#if 0
void SCSI::update_drq()
{
	bool onoff = ((m_regs[SCSI_REG_SCMD] & SCMD_PROGRAM_TRANSFER) == 0);
	if (onoff) {
		if (m_regs[SCSI_REG_PSNS] & BUSSTS_IO) {
			onoff &= ((m_regs[SCSI_REG_SSTS] & (SSTS_TRANSFER_IN_PROGRESS | SSTS_DREG_EMPTY)) == (SSTS_TRANSFER_IN_PROGRESS | SSTS_DREG_EMPTY));
		} else {
			onoff &= ((m_regs[SCSI_REG_SSTS] & (SSTS_TRANSFER_IN_PROGRESS | SSTS_DREG_FULL)) == (SSTS_TRANSFER_IN_PROGRESS));
		}
	}
	set_drq(onoff);
}
#endif

void SCSI::set_drq(bool onoff)
{
	write_signals(&outputs_drq, onoff ? 0xffffffff : 0);
	m_pre_drq = onoff;

	OUT_DEBUG_DRQ(_T("%s H: DRQ[%s] SCTL:0x%02X INTS:0x%02X PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
		, onoff ? _T("on") : _T("off")
		, m_regs[SCSI_REG_SCTL], m_regs[SCSI_REG_INTS], m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
}

void SCSI::set_drq_delay(bool onoff)
{
	cancel_my_event(EVENT_DRQ);
	register_my_event_by_clock(EVENT_DRQ, 8);
	m_pre_drq = onoff;
}

void SCSI::command_complete()
{
	update_ssts(0, SSTS_SPC_BUSY | SSTS_TRANSFER_IN_PROGRESS);
	m_regs[SCSI_REG_INTS] = INTS_COMMAND_COMPLETE;
	update_irq();
	OUT_DEBUG_STAT(_T("%s H: Command Complete INTS:0x%02X PSNS:0x%02X SSTS:0x%02X"), DEBUG_PREFIX
		, m_regs[SCSI_REG_INTS], m_regs[SCSI_REG_PSNS], m_regs[SCSI_REG_SSTS]);
}

void SCSI::go_idle()
{
	// busfree phase
//	m_phase = PHASE_BUSFREE;
	update_ssts(0, SSTS_CONNECTED | SSTS_SPC_BUSY | SSTS_TRANSFER_IN_PROGRESS);
	update_bus_status(m_bdid, BUSTYPE_ACK, false);
	update_bus_status(m_bdid, BUSTYPE_ATN, false);
	update_bus_status(m_bdid, BUSTYPE_SEL, false);
	OUT_DEBUG_STAT(_T("%s H: Go BUSFREE phase"), DEBUG_PREFIX);
	// interrupt
	if (m_regs[SCSI_REG_PCTL] & PCTL_BUSFREE_INT_ENABLE) {
		m_regs[SCSI_REG_INTS] |= INTS_DISCONNECTED;
		update_irq();
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

void SCSI::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);	// Version
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	SXSI_HOST::save_state_parts(vm_state.m_sxsi_host);

	if (d_selected_ctrl) {
		vm_state.m_selected_ctrl_id = Int32_LE(d_selected_ctrl->get_ctrl_id());
	} else {
		vm_state.m_selected_ctrl_id = Int32_LE(-1);
	}
	for(int i=0; i<SCSI_REGS_END; i++) {
		SET_Byte(m_regs[i]);
	}
	SET_Byte(m_bdid);	// ID 0 - 7
	SET_Byte(m_pre_ints);
	vm_state.m_flags = m_pre_atn ? 1 : 0;
	vm_state.m_flags |= m_pre_drq ? 2 : 0;
//	vm_state.m_flags |= m_now_iack ? 4 : 0;
	SET_Byte(m_retry_selection);
	SET_Uint32_LE(m_trans);		///< TC
//	SET_Int32_LE(m_phase);
	vm_state.m_buffer_count = (uint8_t)m_buffer.remain();
	for(uint32_t i=0; i<vm_state.m_buffer_count; i++) {
		vm_state.m_buffer[i] = m_buffer.read_not_remove(i);
	}
	for(int i=0; i<EVENT_IDS_MAX; i++) {
		SET_Int32_LE(m_register_id[i]);
	}
	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		SET_Uint32_LE(m_event_arg[i]);
	}

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

bool SCSI::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	SXSI_HOST::load_state_parts(vm_state.m_sxsi_host);

	int ctrl_id = Int32_LE(vm_state.m_selected_ctrl_id);
	if (ctrl_id >= 0 && ctrl_id < (int)d_ctrls->size()) {
		d_selected_ctrl = d_ctrls->at(ctrl_id);
	} else {
		d_selected_ctrl = NULL;
	}
	for(int i=0; i<SCSI_REGS_END; i++) {
		GET_Byte(m_regs[i]);
	}
	GET_Byte(m_bdid);	// ID 0 - 7
	GET_Byte(m_pre_ints);
	m_pre_atn = ((vm_state.m_flags & 1) != 0);
	m_pre_drq = ((vm_state.m_flags & 2) != 0);
//	m_now_iack = ((vm_state.m_flags & 4) != 0);
	GET_Byte(m_retry_selection);
	GET_Uint32_LE(m_trans);		///< TC
//	GET_Int32_LE(m_phase);
	m_buffer.clear();
	for(uint32_t i=0; i<vm_state.m_buffer_count; i++) {
		m_buffer.write(vm_state.m_buffer[i]);
	}
	for(int i=0; i<EVENT_IDS_MAX; i++) {
		GET_Int32_LE(m_register_id[i]);
	}
	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		GET_Uint32_LE(m_event_arg[i]);
	}

	return true;
}

// ----------------------------------------------------------------------------

bool SCSI::open_disk(int drv, const _TCHAR *path, uint32_t flags)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->open_disk(unit_num, path, flags);
}

bool SCSI::close_disk(int drv, uint32_t flags)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->close_disk(unit_num, flags);
}

bool SCSI::disk_mounted(int drv)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->disk_mounted(unit_num);
}

void SCSI::toggle_disk_write_protect(int drv)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	bool val = d_ctrls->at(id)->write_protected(unit_num);
	d_ctrls->at(id)->set_write_protect(unit_num, !val);
}

bool SCSI::disk_write_protected(int drv) const
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->write_protected(unit_num);
}

bool SCSI::is_same_disk(int drv, const _TCHAR *path)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return false;
	int unit_num = (drv % SCSI_UNITS_PER_CTRL);
	return d_ctrls->at(id)->is_same_disk(unit_num, path);
}

/// Is the disk already mounted on another drive?
int SCSI::mounted_disk_another_drive(int drv, const _TCHAR *path)
{
	int curr_id = (drv / SCSI_UNITS_PER_CTRL);
	int curr_unit = (drv % SCSI_UNITS_PER_CTRL);
	int match = -1;
	for(int id=0; id<(int)d_ctrls->size() && match < 0; id++) {
		for(int unit_num=0; unit_num<SCSI_UNITS_PER_CTRL && match < 0; unit_num++) {
			if (curr_id == id && curr_unit == unit_num) continue;
			if (d_ctrls->at(id)->is_same_disk(unit_num, path)) {
				match = id * SCSI_UNITS_PER_CTRL + unit_num;
			}
		}
	}
	return match;
}

void SCSI::change_device_type(int drv, int num)
{
	int id = (drv / SCSI_UNITS_PER_CTRL);
	if (id >= (int)d_ctrls->size()) return;
	return d_ctrls->at(id)->change_device_type(num);
}


/// b12 : hdbusy
uint32_t SCSI::get_led_status() const
{
	return (m_regs[SCSI_REG_PSNS] & BUSSTS_BSY) << 9;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void SCSI::debug_write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t SCSI::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	// addr is $EA0000 >> 1 (CZ-6BS1)
	// addr is $E96020 >> 1 (inner)
	switch(addr & 0xf) {
	case 0x00:
		// Bus Device ID (BDID)
		data = m_regs[SCSI_REG_BDID];
		break;
	case 0x01:
		// SPC control (SCTL)
		data = m_regs[SCSI_REG_SCTL];
		break;
	case 0x02:
		// Command (SCMD)
		data = m_regs[SCSI_REG_SCMD];
		break;
	case 0x04:
		// Interrupt Sense (INTS)
		data = m_regs[SCSI_REG_INTS];
		break;
	case 0x05:
		// Phase Sense (PSNS)
		data = m_regs[SCSI_REG_PSNS];
		break;
	case 0x06:
		// SPC Status (SSTS)
		data = m_regs[SCSI_REG_SSTS];
		break;
	case 0x07:
		// SPC Error Status (SERR)
		data = m_regs[SCSI_REG_SERR];
		break;
	case 0x08:
		// Phase Control (PCTL)
		data = m_regs[SCSI_REG_PCTL];
		break;
	case 0x09:
		// Modified Byte Counter (MBC)
		data = m_regs[SCSI_REG_MBC];
		break;
	case 0x0a:
		// Data Register (DREG)
		data = m_regs[SCSI_REG_TEMP_RECV];
		break;
	case 0x0b:
		// Temporary Register (TEMP)
		data = m_regs[SCSI_REG_TEMP_RECV];
		break;
	case 0x0c:
		// Transfer Counter High (TCH)
		data = ((m_trans >> 16) & 0xff);
		break;
	case 0x0d:
		// Transfer Counter Middle (TCM)
		data = ((m_trans >> 8) & 0xff);
		break;
	case 0x0e:
		// Transfer Counter Low (TCL)
		data = (m_trans & 0xff);
		break;
	default:
		break;
	}
	return data;
}

bool SCSI::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool SCSI::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	return false;
}

void SCSI::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::strcat(buffer, buffer_len, _T("SCSI\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T("Current Target ID:%d")
		, d_selected_ctrl ? d_selected_ctrl->get_ctrl_id() : -1
	);
	UTILITY::sntprintf(buffer, buffer_len, _T(" BDID:%d (%02x)\n"), m_bdid, m_regs[SCSI_REG_BDID]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" SCTL:%02x"), m_regs[SCSI_REG_SCTL]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" SCMD:%02x"), m_regs[SCSI_REG_SCMD]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" INTS:%02x"), m_regs[SCSI_REG_INTS]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" PSNS:%02x\n"), m_regs[SCSI_REG_PSNS]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" SSTS:%02x"), m_regs[SCSI_REG_SSTS]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" SERR:%02x"), m_regs[SCSI_REG_SERR]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" PCTL:%02x"), m_regs[SCSI_REG_PCTL]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" MBC :%02x\n"), m_regs[SCSI_REG_MBC]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" TEMP R:%02x"), m_regs[SCSI_REG_TEMP_RECV]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" TEMP S:%02x"), m_regs[SCSI_REG_TEMP_SEND]);
	UTILITY::sntprintf(buffer, buffer_len, _T(" TC:%06x\n"), m_trans);
	UTILITY::strcat(buffer, buffer_len, _T(" Buffer:"));
	int siz = m_buffer.size();
	int pos = m_buffer.read_pos();
	for(int i=0; i<siz; i++) {
		uint8_t data = m_buffer.peek(pos);
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02x"), data);
		pos++;
		if (pos >= siz) pos -= siz;
	}
	UTILITY::strcat(buffer, buffer_len, _T("\n"));

	for(int id=0; id<(int)d_ctrls->size(); id++) {
		d_ctrls->at(id)->debug_regs_info(buffer, buffer_len);
	}
}
#endif


// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SCSI_CTRL::SCSI_CTRL(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, SXSI_CTRL_RELAY *relay, int ctrl_id)
	: SXSI_CTRL(parent_vm, parent_emu, identifier, host, relay, ctrl_id)
{
	set_class_name("SCSI_CTRL");
}

SCSI_CTRL::~SCSI_CTRL()
{
}

void SCSI_CTRL::clear()
{
	m_phase = PHASE_BUSFREE;
	m_command_pos = 0;
	m_command_class = 0;
	m_command_code = CMD_NONE;
	m_command_len = 6;
	m_unit_number = 0;
	m_curr_block = 0;
	m_num_blocks = 0;
	m_send_message = MSG_COMPLETE;

	m_send_data_pos = 0;
	m_send_data_len = 0;
	m_send_data[0] = 0;
	m_recv_data_pos = 0;
	m_recv_data_len = 0;
	m_recv_data[0] = 0;

	memset(m_command_data, 0, sizeof(m_command_data));

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		m_register_id[id] = -1;
	}
	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		m_event_arg[i] = 0;
	}

	d_current_disk = d_relay->get_disk_unit(m_unit_number);
}

void SCSI_CTRL::initialize()
{
	clear();
}

void SCSI_CTRL::reset()
{
	warm_reset(true);
}

void SCSI_CTRL::warm_reset(bool por)
{
	if (!por) {
		for(int id=0; id<EVENT_IDS_MAX; id++) {
			cancel_my_event(id);
		}
	}
	clear();
}

// ----------------------------------------------------------------------------

const _TCHAR *SCSI_CTRL::c_phase_labels[] = {
	_T("BUSFREE"),
	_T("ARBITRATION"),
	_T("SELECTION"),
	_T("COMMAND"),
	_T("DATA IN"),
	_T("DATA OUT"),
	_T("STATUS"),
	_T("MESSAGE IN"),
	_T("MESSAGE OUT"),
	_T("COMMAND Next"),
	NULL
};

/// @brief identify
void SCSI_CTRL::set_identify(uint32_t data)
{
	m_identify = (data & 0xff);

	// next phase
	m_phase = PHASE_COMMAND_NEXT;
	OUT_DEBUG_SEL(_T("%s C: Ctrl:%d Identified Data:%02X"), DEBUG_PREFIX, m_ctrl_id, m_identify); 
}

/// @brief selected this disk
void SCSI_CTRL::go_command_phase()
{
	// go to command phase
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
	update_bus_status(SXSI_HOST::BUSTYPE_CD,  true);

	m_identify = 0;
	m_phase = PHASE_COMMAND;
	OUT_DEBUG_SEL(_T("%s C: Ctrl:%d Selected"), DEBUG_PREFIX, m_ctrl_id); 
}

/// @brief go message out phase
void SCSI_CTRL::go_message_out_phase()
{
	// go to message out phase
	update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
	update_bus_status(SXSI_HOST::BUSTYPE_MSG, true);
	// ready to receive command
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);

	m_identify = 0;
	m_phase = PHASE_MESSAGE_OUT;
	OUT_DEBUG_SEL(_T("%s C: Ctrl:%d Go Message Out Phase"), DEBUG_PREFIX, m_ctrl_id); 
}

/// accept ACK signal from host
void SCSI_CTRL::accept_ack()
{
	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
}

void SCSI_CTRL::request_next()
{
	if (m_register_id[EVENT_REQ] >= 0) {
		// Don't assert REQ signal yet
		return;
	}

	switch(m_phase) {
	case PHASE_BUSFREE:
		update_bus_status(SXSI_HOST::BUSTYPE_BSY, false);
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, false);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);
		// BUSFREE phase on host
		d_host->go_idle();

		break;

	case PHASE_COMMAND:
		if (m_command_pos < m_command_len) {
			// ready to next data
			update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		}
		break;

	case PHASE_DATA_IN:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, false);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, true);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_TRANSFERRING);
		break;

	case PHASE_DATA_OUT:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, false);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_TRANSFERRING);
		break;

	case PHASE_STATUS:
		if (m_register_id[EVENT_STATUS] < 0) {
			update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
			update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
			update_bus_status(SXSI_HOST::BUSTYPE_IO, true);
			update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
			d_host->accept_req(SXSI_HOST::AR_COMPLETED);
		}
		break;

	case PHASE_MESSAGE_IN:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, true);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, true);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_NONE);
		break;

	case PHASE_MESSAGE_OUT:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, true);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		d_host->accept_req(SXSI_HOST::AR_NONE);
		break;

	case PHASE_COMMAND_NEXT:
		update_bus_status(SXSI_HOST::BUSTYPE_MSG, false);
		update_bus_status(SXSI_HOST::BUSTYPE_CD, true);
		update_bus_status(SXSI_HOST::BUSTYPE_IO, false);
		update_bus_status(SXSI_HOST::BUSTYPE_REQ, true);
		m_phase = PHASE_COMMAND;
		d_host->accept_req(SXSI_HOST::AR_TRANSFERRING);
		break;
		
	default:
		break;
	}

	OUT_DEBUG_REQN(_T("%s C: Ctrl:%d-%d Next Phase:[%s] (SIG:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, c_phase_labels[m_phase], d_host->get_bus_signal()); 
}

const _TCHAR *SCSI_CTRL::c_command_labels[CMD_UNKNOWN+1] = {
	_T("NONE"),
	_T("TEST UNIT READY"),
	_T("REZERO UNIT"),
	_T("REQUEST SENSE"),
	_T("FORMAT UNIT"),
	_T("READ CAPACITY"),
	_T("READ"),
	_T("WRITE"),
	_T("SEEK"),
	_T("INQUIRY"),
	_T("MODE SENSE"),
	_T("MODE SELECT"),
	_T("START/STOP UNIT"),
	_T("PREVENT/ALLOW MEDIA REMOVAL"),
	_T("VERIFY"),
	_T("Unknown")
};

void SCSI_CTRL::write_data(uint32_t data)
{
	switch(m_phase) {
	case PHASE_COMMAND:
		// command phase
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d WR Command Phase Cmd:[%s] Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, c_command_labels[m_command_code < CMD_UNKNOWN ? m_command_code : CMD_UNKNOWN]
			, data, m_command_pos);
		write_control(data);
		break;
	case PHASE_DATA_OUT:
		// transfer phase
		switch(m_command_code) {
		case CMD_WRITE:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// one block data wrote
				recv_next_disk_data();
			}
			break;
		case CMD_VERIFY:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// one block data wrote
				verify_next_disk_data();
			}
			break;
#if 0
		case CMD_EXTEND_ARGS:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				finish_extended_args();
			}
			break;
#endif
		default:
			m_recv_data[m_recv_data_pos] = data;
			OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_recv_data_pos);
			m_recv_data_pos++;
			if (m_recv_data_pos >= m_recv_data_len) {
				// go to status phase
				send_status(STA_GOOD, SENSE_NO_SENSE);
			}
			break;
		}
		break;
	case PHASE_MESSAGE_OUT:
		// message out phase
		// get identify
		set_identify(data);
		break;
	default:
		// unknown phase
		OUT_DEBUG_DATW(_T("%s C: Ctrl:%d-%d WR Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_phase, m_command_code, data, m_recv_data_pos);
		break;
	}
}

void SCSI_CTRL::write_control(uint32_t data)
{
	switch(m_command_pos) {
	case 0:
		// command code
		parse_command_code(data & 0xff);
		break;
	case 1:
		// logical unit number and address (h)
		m_unit_number = ((data & 0xe0) >> 5);
		d_current_disk = d_relay->get_disk_unit(m_unit_number);
		break;
	default:
		break;
	}

	m_command_data[m_command_pos] = (data & 0xff);
	m_command_pos++;

	update_bus_status(SXSI_HOST::BUSTYPE_REQ, false);	// off request

	if (m_command_pos == m_command_len) {
		// parse parameter
		switch(m_command_class) {
		case 5:
			parse_command_param_group_5();
			break;
		case 1:
			parse_command_param_group_1();
			break;
		default:
			parse_command_param_group_0();
			break;
		}
		// start operation
		process_command();
	}
}

void SCSI_CTRL::parse_command_param_group_0()
{
	// logical block address (h)
	m_curr_block = ((uint32_t)(m_command_data[1] & 0x1f) << 16);
	// logical block address (m)
	m_curr_block |= ((uint32_t)m_command_data[2] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[3];
	// number of blocks
	m_num_blocks = m_command_data[4];
	// control bytes
}

void SCSI_CTRL::parse_command_param_group_1()
{
	// unit address is relative?
	m_is_relative = (m_command_data[1] & 0x01);
	// logical block address (h)
	m_curr_block = ((uint32_t)m_command_data[2] << 24);
	// logical block address (m0)
	m_curr_block |= ((uint32_t)m_command_data[3] << 16);
	// logical block address (m1)
	m_curr_block |= ((uint32_t)m_command_data[4] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[5];

	// control bytes (h)
	m_num_blocks = ((uint32_t)m_command_data[7] << 8);
	// control bytes (l)
	m_num_blocks |= (uint32_t)m_command_data[8];
	// control bytes
}

void SCSI_CTRL::parse_command_param_group_5()
{
	// unit address is relative?
	m_is_relative = (m_command_data[1] & 0x01);
	// logical block address (h)
	m_curr_block = ((uint32_t)m_command_data[2] << 24);
	// logical block address (m0)
	m_curr_block |= ((uint32_t)m_command_data[3] << 16);
	// logical block address (m1)
	m_curr_block |= ((uint32_t)m_command_data[4] << 8);
	// logical block address (l)
	m_curr_block |= (uint32_t)m_command_data[5];

	// control bytes (h)
	m_num_blocks = ((uint32_t)m_command_data[9] << 8);
	// control bytes (l)
	m_num_blocks |= (uint32_t)m_command_data[10];
	// control bytes
}

uint32_t SCSI_CTRL::read_data()
{
	uint32_t data = 0;

	switch(m_phase) {
	case PHASE_DATA_IN:
		switch(m_command_code) {
		case CMD_READ:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_send_data_pos);
			m_send_data_pos++;
			if (m_send_data_pos >= m_send_data_len) {
				send_next_disk_data();
			}
			break;
		default:
			data = m_send_data[m_send_data_pos];
			OUT_DEBUG_DATRE(_T("%s C: Ctrl:%d-%d RD Transfer Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, m_command_code, data, m_send_data_pos);
			m_send_data_pos++;
			if (m_send_data_pos >= m_send_data_len) {
				// go to status phase
				send_status(STA_GOOD, SENSE_NO_SENSE);
			}
			break;
		}
		break;
	case PHASE_STATUS:
		// read status
		data = m_send_data[m_send_data_pos];
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Status Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		// go to message in phase
		send_message_in();
		break;
	case PHASE_MESSAGE_IN:
		// read message
		data = m_send_data[m_send_data_pos];
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Message Phase Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_command_code, data, m_send_data_pos);
		m_send_data_pos++;
		switch(m_next_phase) {
		case PHASE_DATA_IN:
		case PHASE_DATA_OUT:
			// sent identify, so go to next phase
			m_phase = m_next_phase;
			request_next();
			break;
		default:
			// complete, so go to busfree phase
			go_idle();
			break;
		}
		break;
	default:
		// unknown phase
		OUT_DEBUG_DATR(_T("%s C: Ctrl:%d-%d RD Unknown Phase:%d Cmd:%d Data:%02X Pos:%d"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_phase, m_command_code, data, m_send_data_pos);
		break;
	}

	return data;
}

// class 0,1 commands
const uint8_t SCSI_CTRL::c_command_table[64] = {
	// 0                 1                2                3
	CMD_TEST_UNIT_READY, CMD_REZERO_UNIT, CMD_UNSUPPORTED, CMD_REQUEST_SENSE,
	// 4             5                6                7:Reassign Blocks
	CMD_FORMAT_UNIT, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 8             9                A                B
	CMD_READ,        CMD_UNSUPPORTED, CMD_WRITE,       CMD_SEEK,
	// C             D                E                F
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 10            11               12               13:Release Unit
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_INQUIRY,     CMD_UNSUPPORTED,
	// 14            15               16:Reserve       17:Release
	CMD_UNSUPPORTED, CMD_MODE_SELECT, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 18:Copy       19:Compare       1A               1B
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_MODE_SENSE,  CMD_START_STOP_UNIT,
	// 1C:Receive Diag 1D:Send Diag     1E                 1F
	CMD_UNSUPPORTED,   CMD_UNSUPPORTED, CMD_MEDIA_REMOVAL, CMD_UNSUPPORTED,
	// 20            21               22               23
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 24            25                 26               27
	CMD_UNSUPPORTED, CMD_READ_CAPACITY, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 28            29               2A               2B
	CMD_READ,        CMD_UNSUPPORTED, CMD_WRITE,       CMD_SEEK,
	// 2C            2D               2E:Write & Verify 2F
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,  CMD_VERIFY,
	// 30            31               32               33
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 34            35:Sync Cache    36               37:Read Defect Data
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 38            39               3A               3B:Write Buffer
	CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED,
	// 3C:Read Buffer 3D               3E:Read Long     3F:Write Long
	CMD_UNSUPPORTED,  CMD_UNSUPPORTED, CMD_UNSUPPORTED, CMD_UNSUPPORTED
};

void SCSI_CTRL::parse_command_code(uint8_t data)
{
	m_command_class = ((data >> 5) & 7);
	switch(m_command_class) {
	case 3:
		switch(data & 0x3f) {
		case 0x15:
		case 0x1a:
			m_command_len = 10;
			break;
		default:
			break;
		}
		break;
	default:
		m_command_code = c_command_table[data & 0x3f];
		break;
	}
	if (m_command_class == 1) {
		m_command_len = 10;
	} else {
		m_command_len = 6;
	}
}

void SCSI_CTRL::process_command()
{
	switch(m_command_code) {
	case CMD_TEST_UNIT_READY:
		// test unit ready
		test_unit_ready();
		break;
	case CMD_REZERO_UNIT:
		// rezero unit
		rezero_unit();
		break;
	case CMD_REQUEST_SENSE:
		// request sense
		request_sense();
		break;
	case CMD_FORMAT_UNIT:
		// format unit
		format_unit();
		break;
	case CMD_READ_CAPACITY:
		// read capacity
		read_capacity();
		break;
	case CMD_READ:
		// read data, go to transfer phase
		send_first_disk_data();
		break;
	case CMD_WRITE:
		// write data, go to transfer phase
		recv_first_disk_data();
		break;
	case CMD_SEEK:
		// seek
		seek();
		break;
	case CMD_INQUIRY:
		// inquiry
		inquiry();
		break;
	case CMD_MODE_SENSE:
		// mode sense
		mode_sense();
		break;
	case CMD_MODE_SELECT:
		// mode select
		mode_select();
		break;
	case CMD_START_STOP_UNIT:
		// start/stop unit
		start_stop_unit();
		break;
	case CMD_MEDIA_REMOVAL:
		// prevent/allow media removal
		media_removal();
		break;
	case CMD_VERIFY:
		// verify data, go to transfer phase
		verify_first_disk_data();
	default:
		// unsupport command, so go to status phase
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:Unknown"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number);
		send_status(STA_CHECK_CONDITION, SENSE_ILLEGAL_REQUEST);
		break;
	}
}

/// TEST UNIT READY (00)
///
void SCSI_CTRL::test_unit_ready()
{
	int device_type = d_relay->get_device_type();
	switch(device_type) {
	case HARDDISK::DEVICE_TYPE_SCSI_MO:
		if (!d_current_disk) {
			send_status(STA_CHECK_CONDITION, SENSE_NOT_READY);
			return;
		}
		break;
	default:
		if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
			send_status(STA_CHECK_CONDITION, SENSE_NOT_READY);
			return;
		}
		break;
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[TEST UNIT READY]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	send_status(STA_GOOD, SENSE_NO_SENSE);
}

/// REZERO UNIT (01)
///
void SCSI_CTRL::rezero_unit()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	int cylinder_diff = 0;
	bool rc = d_current_disk->seek(0, &cylinder_diff);
	if (!rc) {
		// error?!
		send_status(STA_CHECK_CONDITION, SENSE_MEDIUM_ERROR);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[REZERO UNIT] (Diff:%d Delay:%d)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, cylinder_diff, delay);

	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_GOOD, SENSE_NO_SENSE);
}

/// REQUEST SENSE (03)
///
void SCSI_CTRL::request_sense()
{
	m_send_data_pos = 0;
	m_send_data_len = MIN(m_command_data[4], 16);
	memset(m_send_data, 0, 16);
	m_send_data[0] = 0x70;
	m_send_data[1] = 0;
	m_send_data[2] = m_sense_code & 0xf;
	m_send_data[3] = (m_curr_block >> 24) & 0xff;
	m_send_data[4] = (m_curr_block >> 16) & 0xff;
	m_send_data[5] = (m_curr_block >> 8) & 0xff;
	m_send_data[6] = m_curr_block & 0xff;
	m_send_data[7] = 0;

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[REQUEST SENSE] Code:%02X"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_sense_code);

	set_request_delay(8, PHASE_DATA_IN);
}

void SCSI_CTRL::init_send_buffer(HARDDISK *disk)
{
	m_send_data_pos = 0;
	m_send_data_len = disk->get_sector_size();
	if (m_send_data_len > BUFFER_MAX) m_send_data_len = BUFFER_MAX;
}

void SCSI_CTRL::send_disk_data(HARDDISK *disk)
{
	// read from hard disk
	int cylinder_diff = 0;
	bool rc = disk->read_buffer(m_curr_block, m_send_data_len, m_send_data, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Read Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// read error
		send_status(STA_CHECK_CONDITION, SENSE_ABORTED_COMMAND);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_diff);
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Start (Diff:%d Delay:%d SIG:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff, delay
		, d_host->get_bus_signal()
		);

	play_seek_sound(cylinder_diff);

	// set request signal to read
	set_request_delay(delay, PHASE_DATA_IN);
}

/// READ (08)(28)
/// send data to initiator
void SCSI_CTRL::send_first_disk_data()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	init_send_buffer(d_current_disk);
	send_disk_data(d_current_disk);
}

void SCSI_CTRL::send_next_disk_data()
{
	// one block was read
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		init_send_buffer(d_current_disk);
		send_disk_data(d_current_disk);

	} else {
		// complete
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ] Block:%d Finish"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_curr_block);
		send_status(STA_GOOD, SENSE_NO_SENSE);
	}
}

void SCSI_CTRL::init_recv_buffer(HARDDISK *disk)
{
	m_recv_data_pos = 0;
	m_recv_data_len = disk->get_sector_size();
	if (m_recv_data_len > BUFFER_MAX) m_recv_data_len = BUFFER_MAX;
}

/// WRITE (0A)(2A)
/// receive data from initiator
void SCSI_CTRL::recv_first_disk_data()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Start (BUS:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, d_host->get_bus_signal());

	init_recv_buffer(d_current_disk);

	// set REQ signal
	set_request_delay(DELAY_TRANSFER_PHASE, PHASE_DATA_OUT);
}

void SCSI_CTRL::recv_next_disk_data()
{
	// one block was written
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	// write to hard disk
	bool rc = d_current_disk->is_write_protected();
	if (rc) {
		// write protected
		send_status(STA_CHECK_CONDITION, SENSE_DATA_PROTECT);
		return;
	}

	recv_disk_data(d_current_disk);
}

void SCSI_CTRL::recv_disk_data(HARDDISK *disk)
{
	int cylinder_diff = 0;
	bool rc = disk->write_buffer(m_curr_block, m_recv_data_len, m_recv_data, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d Wrote Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// write error
		send_status(STA_CHECK_CONDITION, SENSE_ABORTED_COMMAND);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;

	// set drq and request to write
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_diff);
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d (Diff:%d Delay:%d SIG:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff, delay
		, d_host->get_bus_signal()
		);

	play_seek_sound(cylinder_diff);

	// next block
	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		init_recv_buffer(d_current_disk);
		// set REQ signal
		set_request_delay(delay, PHASE_DATA_OUT);

	} else {
		// complete
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[WRITE] Block:%d Finish"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_curr_block);
		send_status_delay(delay, STA_GOOD, SENSE_NO_SENSE);
	}
}

/// VERIFY (2F)
///
void SCSI_CTRL::verify_first_disk_data()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[VERIFY] Start (BUS:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, d_host->get_bus_signal());

	init_recv_buffer(d_current_disk);

	// set REQ signal
	set_request_delay(DELAY_TRANSFER_PHASE, PHASE_DATA_OUT);
}

void SCSI_CTRL::verify_next_disk_data()
{
	// one block was written
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	verify_disk_data(d_current_disk);
}

void SCSI_CTRL::verify_disk_data(HARDDISK *disk)
{
	int cylinder_diff = 0;
	bool rc = disk->verify_buffer(m_curr_block, m_recv_data_len, m_recv_data, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[VERIFY] Block:%d Wrote Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// verify error
		send_status(STA_CHECK_CONDITION, SENSE_ABORTED_COMMAND);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;

	// set drq and request to write
	if (!FLG_DELAY_HDSEEK) {
		delay += disk->calc_access_time(cylinder_diff);
	}
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[VERIFY] Block:%d (Diff:%d Delay:%d SIG:%02X)"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, cylinder_diff, delay
		, d_host->get_bus_signal()
		);

	play_seek_sound(cylinder_diff);

	// next block
	m_num_blocks--;
	if (m_num_blocks > 0) {
		// next block
		m_curr_block++;
		init_recv_buffer(d_current_disk);
		// set REQ signal
		set_request_delay(delay, PHASE_DATA_OUT);

	} else {
		// complete
		OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[VERIFY] Block:%d Finish"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, m_curr_block);
		send_status_delay(delay, STA_GOOD, SENSE_NO_SENSE);
	}
}

/// SEEK (0B)
///
void SCSI_CTRL::seek()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	int cylinder_diff = 0;
	bool rc = d_current_disk->seek(m_curr_block, &cylinder_diff);
	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[SEEK] Block:%d Result:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_curr_block, rc ? _T("OK") : _T("NG"));
	if (!rc) {
		// seek error
		// may be recoverable by rezero unit 
		send_status(STA_CHECK_CONDITION, SENSE_ABORTED_COMMAND);
		return;
	}

	int delay = DELAY_TRANSFER_PHASE;
	if (!FLG_DELAY_HDSEEK) {
		delay += d_current_disk->calc_access_time(cylinder_diff);
	}
	play_seek_sound(cylinder_diff);

	send_status_delay(delay, STA_GOOD, SENSE_NO_SENSE);
}

/// FORMAT UNIT (04)
///
void SCSI_CTRL::format_unit()
{
	int device_type = d_relay->get_device_type();
	if (!d_current_disk || !d_current_disk->is_valid_disk(device_type)) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[FORMAT UNIT]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);
	d_current_disk->format_disk();
//	send_status(STA_GOOD, SENSE_NO_SENSE);
	send_status_delay(5 * 1000 * 1000, STA_GOOD, SENSE_NO_SENSE);
}

/// READ CAPACITY (25)
///
void SCSI_CTRL::read_capacity()
{
	if (!d_current_disk) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	m_send_data_pos = 0;
	m_send_data_len = 8;
	uint32_t lba = (uint32_t)d_current_disk->get_sector_total() - 1; // max size
	uint32_t blen = (uint32_t)d_current_disk->get_sector_size();

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[READ CAPACITY] Lba:%u Size:%u"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, lba, blen);

	m_send_data[3] = (lba & 0xff); lba >>= 8; 
	m_send_data[2] = (lba & 0xff); lba >>= 8;
	m_send_data[1] = (lba & 0xff); lba >>= 8;
	m_send_data[0] = (lba & 0xff);
	m_send_data[7] = (blen & 0xff); blen >>= 8; 
	m_send_data[6] = (blen & 0xff); blen >>= 8;
	m_send_data[5] = (blen & 0xff); blen >>= 8;
	m_send_data[4] = (blen & 0xff);

	set_request_delay(8, PHASE_DATA_IN);
}

/// INQUIRY (12)
///
void SCSI_CTRL::inquiry()
{
	m_send_data_pos = 0;
	m_send_data_len = MIN(m_command_data[4], 64);

//	if (!d_current_disk) {
//		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
//		return;
//	}

	int device_type = d_relay->get_device_type();
	memset(m_send_data, 0, 64);
	m_send_data[0] = (device_type == HARDDISK::DEVICE_TYPE_SCSI_MO ? 0x07 : 0); // Peripheral Device Type (HDD:0 CD-ROM:0x05 Optical Device:0x07) 
	m_send_data[1] = (device_type == HARDDISK::DEVICE_TYPE_SCSI_MO ? 0x80 : 0); // bit7:RMB(Removable) bit6-0:Device-Type Qualifier
	m_send_data[2] = 2; // ANSI X3T9.2/86-109(SCSI-2):2 ANSI-Approved Version:1
	m_send_data[3] = 0;
	m_send_data[4] = 0;

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[INQUIRY] Data:%02X %02X %02X"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_send_data[0], m_send_data[1], m_send_data[2]);

	set_request_delay(8, PHASE_DATA_IN);
}

/// MODE SENSE (1A)
///
void SCSI_CTRL::mode_sense()
{
	m_send_data_pos = 0;
	if (m_command_class == 3) m_send_data_len = ((int)m_command_data[7] << 8) | m_command_data[8];
	else m_send_data_len = m_command_data[4];
	m_send_data_len = MIN(m_send_data_len, 64);

	if (!d_current_disk) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	memset(m_send_data, 0, 64);
	m_send_data[0] = 3;	// length 
	m_send_data[1] = 0; // media type
	m_send_data[2] = d_current_disk->is_write_protected() ? 0x80 : 0;	// write protected
	m_send_data[3] = 0;

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[MODE SENSE] Data:%02X %02X %02X"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_send_data[0], m_send_data[1], m_send_data[2]);

	set_request_delay(8, PHASE_DATA_IN);
}

/// MODE SELECT (15)
///
void SCSI_CTRL::mode_select()
{
	m_recv_data_pos = 0;
	if (m_command_class == 3) m_recv_data_pos = ((int)m_command_data[7] << 8) | m_command_data[8];
	else m_recv_data_pos = m_command_data[4];

	if (!d_current_disk) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[MODE SELECT]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	set_request_delay(8, PHASE_DATA_OUT);
}

/// START/STOP UNIT (1B)
///
void SCSI_CTRL::start_stop_unit()
{
	if (!d_current_disk) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[START/STOP UNIT]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	send_status(STA_GOOD, SENSE_NO_SENSE);
}

/// PREVENT/ALLOW MEDIA REMOVAL (1E)
///
void SCSI_CTRL::media_removal()
{
	if (!d_current_disk) {
		send_status(STA_CHECK_CONDITION, SENSE_UNIT_ATTENTION);
		return;
	}

	OUT_DEBUG_CMD(_T("%s C: Ctrl:%d-%d CMD:[PREVENT/ALLOW MEDIA REMOVAL]"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number);

	send_status(STA_GOOD, SENSE_NO_SENSE);
}

void SCSI_CTRL::send_status(uint8_t error_occured, uint8_t sense_code, bool from_event)
{
	// go to status phase
	m_phase = PHASE_STATUS;

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = (error_occured << 1);
	m_send_message = MSG_COMPLETE;
	m_sense_code = sense_code;

	OUT_DEBUG_STAT(_T("%s C: Ctrl:%d-%d CMD:[%s] Status:%02X Sense:%02X FromEvent:%s"), DEBUG_PREFIX
		, m_ctrl_id, m_unit_number
		, m_command_code < CMD_UNKNOWN ? c_command_labels[m_command_code] : _T("Unknown")
		, m_send_data[0], sense_code
		, from_event ? _T("Yes") : _T("No"));

	if (from_event) {
		// Update bus signal
		request_next();
	}
}

void SCSI_CTRL::send_status_delay(int delay, uint8_t error_occured, uint8_t sense_code)
{
	// go to status phase
	m_phase = PHASE_STATUS;

	cancel_my_event(EVENT_STATUS);
	m_event_arg[EVENT_ARG_STATUS_TYPE] = error_occured;
	m_event_arg[EVENT_ARG_STATUS_CODE] = sense_code;
	register_my_event(EVENT_STATUS, delay);
}

void SCSI_CTRL::send_message_in()
{
	// go to message in phase
	m_phase = PHASE_MESSAGE_IN;

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = m_send_message;
}

void SCSI_CTRL::go_disconnect()
{
	// go to message in phase
	m_phase = PHASE_MESSAGE_IN;

	m_send_data_pos = 0;
	m_send_data_len = 1;
	m_send_data[0] = 0x04;
}

/// go busfree phase
void SCSI_CTRL::go_idle()
{
	// go to busfree phase
	m_phase = PHASE_BUSFREE;

	m_send_data_pos = 0;
	m_send_data_len = 0;
	m_send_data[0] = 0;
	m_recv_data_pos = 0;
	m_recv_data_len = 0;
	m_recv_data[0] = 0;
}

void SCSI_CTRL::reselection(int next_phase)
{
	// arbitration
	d_host->reselection(m_ctrl_id);

	// next phase after sent identify
	m_next_phase = next_phase;

	// message in phase
	// send identify
	m_send_message = (IDT_ENABLE | m_unit_number);
	send_message_in();

	request_next();
}

// ----------------------------------------------------------------------------

void SCSI_CTRL::play_seek_sound(int sum)
{
	if (sum > 0) {
		d_host->play_seek_sound(d_relay->get_device_type());
		if (sum > 1 && !FLG_DELAY_HDSEEK) {
			register_seek_sound(sum, HARDDISK::get_cylinder_to_cylinder_delay());
		}
	}
}

void SCSI_CTRL::register_seek_sound(int sum, int delay)
{
	m_event_arg[EVENT_ARG_SEEK_TIME] = (uint32_t)sum;
	cancel_my_event(EVENT_SEEK);
	register_my_event(EVENT_SEEK, delay);
}

// ----------------------------------------------------------------------------

#if 0
void SCSI_CTRL::update_bus_status(SCSI::SCSI_SIGNAL_STATUS_TYPE bus_type, bool onoff)
{
	d_host->update_bus_status(m_ctrl_id, bus_type, onoff);
}
#endif

void SCSI_CTRL::set_request_delay(int delay, int next_phase)
{
	cancel_my_event(EVENT_REQ);

	// disconnect if need
	if ((m_identify & (IDT_ENABLE | IDT_DISCONNECT)) == (IDT_ENABLE | IDT_DISCONNECT)) {
		go_disconnect();
		request_next();
	}

	m_event_arg[EVENT_ARG_NEXT_PHASE] = next_phase;
	register_my_event(EVENT_REQ, delay);
}

void SCSI_CTRL::send_request()
{
	if (d_host->is_active_bus_status(SXSI_HOST::BUSTYPE_ACK)) {
		// ACK signal is not off yet, so keep current phase until ACK signal become off.
		OUT_DEBUG_REQ(_T("%s C: Ctrl:%d-%d REQ Event Phase:[%s] Keep current phase because ACK is on (SIG:%02X)"), DEBUG_PREFIX
			, m_ctrl_id, m_unit_number
			, c_phase_labels[m_phase]
			, d_host->get_bus_signal());
		register_my_event(EVENT_REQ, 8);

	} else {
		if ((m_identify & (IDT_ENABLE | IDT_DISCONNECT)) == (IDT_ENABLE | IDT_DISCONNECT)
		  && m_phase == PHASE_BUSFREE) {
			// reselection phase
			reselection(m_event_arg[EVENT_ARG_NEXT_PHASE]);

		} else {
			// assert REQ signal on the SCSI BUS
			m_phase = m_event_arg[EVENT_ARG_NEXT_PHASE];
			OUT_DEBUG_REQ(_T("%s C: Ctrl:%d-%d REQ Event Phase:[%s] (SIG:%02X)"), DEBUG_PREFIX
				, m_ctrl_id, m_unit_number
				, c_phase_labels[m_phase]
				, d_host->get_bus_signal());
			request_next();

		}
	}
}

// ----------------------------------------------------------------------------

void SCSI_CTRL::cancel_my_event(int event_id)
{
	if (m_register_id[event_id] >= 0) {
		cancel_event(this, m_register_id[event_id]);
		m_register_id[event_id] = -1;
	}
}

void SCSI_CTRL::register_my_event(int event_id, int usec)
{
	register_event(this, event_id, usec, false, &m_register_id[event_id]);
}

// ----------------------------------------------------------------------------

void SCSI_CTRL::event_callback(int event_id, int err)
{
	m_register_id[event_id] = -1;
	switch(event_id) {
	case EVENT_REQ:
		send_request();
		break;
	case EVENT_STATUS:
		send_status(m_event_arg[EVENT_ARG_STATUS_TYPE], m_event_arg[EVENT_ARG_STATUS_CODE], true);
		break;
	case EVENT_SEEK:
		m_event_arg[EVENT_ARG_SEEK_TIME]--;
		play_seek_sound((int)m_event_arg[EVENT_ARG_SEEK_TIME]);
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void SCSI_CTRL::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// save config flags
	memset(&vm_state, 0, sizeof(vm_state));

	SXSI_CTRL::save_state_parts(vm_state.m_sxsi_ctrl);

	SET_Byte(m_send_message);
	SET_Byte(m_sense_code);
	SET_Byte(m_is_relative);
	SET_Byte(m_identify);
	SET_Int32_LE(m_next_phase);

	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		SET_Uint32_LE(m_event_arg[i]);
	}

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		SET_Int32_LE(m_register_id[id]);
	}

	SET_Int32_LE(m_send_data_pos);
	SET_Int32_LE(m_send_data_len);
	memcpy(vm_state.m_send_data, m_send_data, sizeof(vm_state.m_send_data));

	SET_Int32_LE(m_recv_data_pos);
	SET_Int32_LE(m_recv_data_len);
	memcpy(vm_state.m_recv_data, m_recv_data, sizeof(vm_state.m_recv_data));

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool SCSI_CTRL::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	SXSI_CTRL::load_state_parts(vm_state.m_sxsi_ctrl);

	GET_Byte(m_send_message);
	GET_Byte(m_sense_code);
	GET_Byte(m_is_relative);
	GET_Byte(m_identify);
	GET_Int32_LE(m_next_phase);

	for(int i=0; i<EVENT_ARGS_MAX; i++) {
		GET_Uint32_LE(m_event_arg[i]);
	}

	for(int id=0; id<EVENT_IDS_MAX; id++) {
		GET_Int32_LE(m_register_id[id]);
	}

	GET_Int32_LE(m_send_data_pos);
	GET_Int32_LE(m_send_data_len);
	memcpy(m_send_data, vm_state.m_send_data, sizeof(m_send_data));

	GET_Int32_LE(m_recv_data_pos);
	GET_Int32_LE(m_recv_data_len);
	memcpy(m_recv_data, vm_state.m_recv_data, sizeof(m_recv_data));

	d_current_disk = d_relay->get_disk_unit(m_unit_number);

	return true;
}

// ----------------------------------------------------------------------------
#if 0
bool SCSI_CTRL::open_disk(int unit_num, const _TCHAR *path, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->open(path, 512, flags);
}

bool SCSI_CTRL::close_disk(int unit_num, uint32_t flags)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	disk->close();
	return true;
}

bool SCSI_CTRL::disk_mounted(int unit_num) const
{
	return unit_mounted(unit_num);
}

void SCSI_CTRL::set_write_protect(int unit_num, bool val)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return;
	}
	disk->set_write_protect(val);
}

bool SCSI_CTRL::write_protected(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_write_protected();
}

bool SCSI_CTRL::is_same_disk(int unit_num, const _TCHAR *path)
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->is_same_file(path);
}

void SCSI_CTRL::change_device_type(int num)
{
	if (num < 0 || num >= DEVICE_TYPE_MAX) return;

	m_device_type = num;
}
#endif

// ----------------------------------------------------------------------------
#if 0
HARDDISK *SCSI_CTRL::get_disk_unit(int unit_num) const
{
	if (d_units->Count() <= unit_num) {
		return NULL;
	}
	return d_units->Item(unit_num);
}

bool SCSI_CTRL::unit_mounted(int unit_num) const
{
	HARDDISK *disk = get_disk_unit(unit_num);
	if (!disk) {
		return false;
	}
	return disk->mounted();
}

bool SCSI_CTRL::unit_mounted_at_least() const
{
	return d_relay->unit_mounted_at_least();
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void SCSI_CTRL::debug_write_data(uint32_t data)
{
	m_recv_data[m_recv_data_pos] = data & 0xff;
}

uint32_t SCSI_CTRL::debug_read_data()
{
	return m_send_data[m_send_data_pos];
}

void SCSI_CTRL::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::sntprintf(buffer, buffer_len, _T("SCSI CTRL: ID:%d Unit:%d\n"), m_ctrl_id, m_unit_number);
	UTILITY::tcscat(buffer, buffer_len, _T("  Current Phase:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_phase < PHASE_UNKNOWN ? c_phase_labels[m_phase] : _T("Unknown")
	);
	UTILITY::tcscat(buffer, buffer_len, _T("  Command:"));
	UTILITY::tcscat(buffer, buffer_len,
		m_command_code < CMD_UNKNOWN ? c_command_labels[m_command_code] : _T("Unknown")
	);
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	for(int i=0; i<m_command_pos; i++) {
		UTILITY::sntprintf(buffer, buffer_len, _T(" %02X"), m_command_data[i]);
	}
	UTILITY::tcscat(buffer, buffer_len, _T("\n"));
}
#endif

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SCSI_CTRL_RELAY::SCSI_CTRL_RELAY(VM* parent_vm, EMU* parent_emu, const char *identifier, SXSI_HOST *host, int ctrl_id)
	: SXSI_CTRL_RELAY(parent_vm, parent_emu, identifier, host, ctrl_id, SCSI_UNITS_PER_CTRL)
{
	set_class_name("SCSI_CTRL_RE");
	m_device_type = HARDDISK::DEVICE_TYPE_SCSI_HDD;

	SASI_CTRL *ctrl0 = new SASI_CTRL(vm, emu, "C", host, this, ctrl_id);
	SCSI_CTRL *ctrl1 = new SCSI_CTRL(vm, emu, "C", host, this, ctrl_id);
	add_ctrl(ctrl0);
	add_ctrl(ctrl1);
}

void SCSI_CTRL_RELAY::change_device_type(int num)
{
	if (num < 0 || num >= HARDDISK::DEVICE_TYPE_MAX) {
		num = HARDDISK::DEVICE_TYPE_SCSI_HDD;
	}

	// change control
	switch(num) {
	case HARDDISK::DEVICE_TYPE_SCSI_HDD:
	case HARDDISK::DEVICE_TYPE_SCSI_MO:
		d_selected_ctrl = d_ctrls.at(1);
		break;
	default:
		d_selected_ctrl = d_ctrls.at(0);
		break;
	}

	m_device_type = num;
}

bool SCSI_CTRL_RELAY::unit_mounted_at_least() const
{
	bool rc = false;
	for(int unit=0; unit<d_units->Count(); unit++) {
		switch(m_device_type) {
		case HARDDISK::DEVICE_TYPE_SCSI_MO:
			// MO is removable
			rc = true;
			break;
		default:
			rc |= d_units->Item(unit)->is_valid_disk(m_device_type);
			break;
		}
	}
	return rc;
}

#if 0
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SCSI_CTRLS::SCSI_CTRLS()
	: std::vector<SCSI_CTRL *>()
{
}

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

SCSI_UNITS::SCSI_UNITS(int alloc_size)
	: CPtrList<HARDDISK>(alloc_size)
{
}
#endif
