/** @file mc68000.cpp

	Skelton for retropc emulator

	@par Origin MAME 0.293, 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@brief [ MC68000 ]
*/

#include "mc68000.h"
//#include "../emu.h"
#include "../depend.h"
#include "../config.h"
#include "../fileio.h"
#include "../logging.h"
#include "../utility.h"
#include "mc68000_consts.h"
#include "mc68000ops.h"
#include "mc68000mmu.h"
#include "mc68000fpu.h"
#ifdef _DEBUG
#include <cassert>
#endif

#define OUT_ERRORLOG logging->out_logf

#ifdef _DEBUG
//#define OUT_DEBUG_EX_INTR logging->out_debugf
#define OUT_DEBUG_EX_INTR(...)
//#define OUT_DEBUG_EX_TRAPE logging->out_debugf
//#define OUT_DEBUG_INT_MSK logging->out_debugf
//#define OUT_DEBUG_BUSREQ logging->out_debugf
#define OUT_DEBUG_BUSREQ(...)
#else
#define OUT_DEBUG_EX_INTR(...)
#define OUT_DEBUG_BUSREQ(...)
#endif

/****************************************************************************/

bool MC68000BASE::m_emulation_initialized;
uint16_t MC68000BASE::m68ki_instruction_state_table[NUM_CPU_TYPES][0x10000]; /* opcode handler state numbers */
int16_t  MC68000BASE::m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

/****************************************************************************/

MC68000ABERR::MC68000ABERR()
{
	clear();
}
MC68000ABERR::MC68000ABERR(uint32_t num, uint32_t address, uint32_t mode, uint32_t fc)
{
	m_occured = true;
	m_num = num;
	m_address = address;
	m_mode = mode;
	m_fc = fc;
	m_size = 0;
}
MC68000ABERR::MC68000ABERR(uint32_t num, uint32_t address, uint32_t mode, uint32_t fc, uint32_t size)
{
	m_occured = true;
	m_num = num;
	m_address = address;
	m_mode = mode;
	m_fc = fc;
	m_size = size;
}
void MC68000ABERR::clear()
{
	m_occured = false;
	m_num = 0;
	m_address = 0;
	m_mode = 0;
	m_fc = 0;
	m_size = 0;
}

void MC68000ABERR::save_state(struct vm_state &v)
{
	v.m_num = Uint32_LE(m_num);
	v.m_address = Uint32_LE(m_address);
	v.m_mode = Uint32_LE(m_mode);
	v.m_fc =Uint32_LE( m_fc);
	v.m_size = Uint32_LE(m_size);
	v.m_occured = m_occured ? 1 : 0;
}

bool MC68000ABERR::load_state(const struct vm_state &v)
{
	m_num = Uint32_LE(v.m_num);
	m_address = Uint32_LE(v.m_address);
	m_mode = Uint32_LE(v.m_mode);
	m_fc = Uint32_LE(v.m_fc);
	m_size = Uint32_LE(v.m_size);
	m_occured = (v.m_occured != 0);
	return true;
}

size_t MC68000ABERR::get_state_size() const
{
	return sizeof(struct vm_state);
}

/****************************************************************************/

MC68000BASE::MC68000BASE(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	init_output_signals(&outputs_res);	// software reset 
	init_output_signals(&outputs_halt);	// stopped by bus/address error
	init_output_signals(&outputs_fc);	// function code
	init_output_signals(&outputs_bg);	// bus ground

	m_emulation_initialized = false;
//	m_interrupt_mixer = true;
	for(int i=0; i<8; i++) {
		d_mem[i] = NULL;
#ifdef USE_DEBUGGER
		d_mem_stored[i] = NULL;
#endif
	}
}

void MC68000BASE::initialize()
{
#ifdef USE_DEBUGGER
	dasm.set_context_cpu(this);
	dasm.set_regptrs(
		&m_pc,
		m_dar,
		m_sp,
#ifdef USE_MC68000VBR
		&m_vbr,
		&m_sfc,
		&m_dfc,
#endif
#ifdef USE_MC68000CACHE
		&m_cacr,
		&m_caar,
#endif
#ifdef USE_MC68000FPU
		&m_fpiar,
		&m_fpsr,
		&m_fpcr,
#endif
		&m_signals
	);
	m_now_debugging = false;
#endif

	// Set each memory space of function code
	if (!d_mem[M68K_FC_USER_PROGRAM]) d_mem[M68K_FC_USER_PROGRAM] = d_mem[M68K_FC_SUPERVISOR_PROGRAM];
	if (!d_mem[M68K_FC_USER_DATA]) d_mem[M68K_FC_USER_DATA] = d_mem[M68K_FC_SUPERVISOR_DATA];
	DEVICE *d_dmem = NULL;
	for(int i=7; i>=0; i--) {
		if (d_mem[i]) {
			d_dmem = d_mem[i];
			break;
		}
	}
#ifdef _DEBUG
	assert(d_dmem);
#endif
	for(int i=0; i<8; i++) {
		if (!d_mem[i]) d_mem[i] = d_dmem;
	}

#ifdef USE_DEBUGGER
	// memory access through debugger
	for(int i=0; i<8; i++) {
		d_mem_stored[i] = new DEBUGGER_BUS(vm, emu, NULL);
		d_mem_stored[i]->rerate_to_cpu(true);
		d_mem_stored[i]->set_parent(d_debugger);
		d_mem_stored[i]->set_context_mem(d_mem[i]);
	}
#endif

//	m_cpu_clock = 1000000;
	m_cpu_type = 0;
	for (int i=0;i<16;i++)
		m_dar[i] = 0;
	m_ppc = 0;
	m_pc = 0;
	for (int i=0;i<4;i++)
		m_sp[i] = 0;
#ifdef USE_MC68000VBR
	m_vbr = 0;
	m_sfc = 0;
	m_dfc = 0;
#endif
#ifdef USE_MC68000CACHE
	m_cacr = 0;
	m_caar = 0;
#endif
	m_ir = 0;
#ifdef USE_MC68000FPU
	m_fpiar = 0;
	m_fpsr = 0;
	m_fpcr = 0;
#endif
	m_t1_flag = 0;
	m_t0_flag = 0;
	m_s_flag = 0;
	m_m_flag = 0;
	m_x_flag = 0;
	m_n_flag = 0;
	m_not_z_flag = 0;
	m_v_flag = 0;
	m_c_flag = 0;
	m_int_mask = 0;
	m_int_level = 0;
	m_int_level_pref = 0;
	m_int_vec_num = 0;
	m_stopped = 0;
	m_pref_flags = 0;
	m_pref_addr = 0;
	m_pref_data = 0;
	m_sr_mask = 0;
	m_instr_mode = 0;
	m_run_mode = RUN_MODE_NORMAL;
	m_proc_mode = PROC_MODE_NORMAL;
	m_signals = 0;
	m_fc = 0xffff;
#ifdef USE_MC68000MMU
	m_has_pmmu = false;
	m_has_hmmu = false;
	m_pmmu_enabled = 0;
	m_hmmu_enabled = 0;
#endif
#ifdef USE_MC68000FPU
	m_has_fpu= false;
	m_fpu_just_reset= 0;
#endif

	m_cyc_bcc_notake_b = 0;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp = 0;
	m_cyc_scc_r_true = 0;
	m_cyc_movem_w = 0;
	m_cyc_movem_l = 0;
	m_cyc_shift = 0;
	m_cyc_reset = 0;

	m_initial_count = 0;
	m_icount = 0;
	m_multiple_cycle = 1;
	m_reset_cycles = 0;
	m_tracing = 0;
	m_total_icount = 0;

	m_aberr.clear();

	m_nmi_pending = false;

	m_cyc_instruction = NULL;
	m_cyc_exception = NULL;

#ifdef USE_MC68000MMU
	m_mmu_crp_aptr = m_mmu_crp_limit = 0;
	m_mmu_srp_aptr = m_mmu_srp_limit = 0;
	m_mmu_urp_aptr = 0;
	m_mmu_tc = 0;
	m_mmu_sr = 0;
	m_mmu_sr_040 = 0;

	for (int i=0; i<MMU_ATC_ENTRIES;i++)
		m_mmu_atc_tag[i] = m_mmu_atc_data[i] = 0;

	m_mmu_atc_rr = 0;
	m_mmu_tt0 = m_mmu_tt1 = 0;
	m_mmu_itt0 = m_mmu_itt1 = m_mmu_dtt0 = m_mmu_dtt1 = 0;
	m_mmu_acr0 = m_mmu_acr1 = m_mmu_acr2 = m_mmu_acr3 = 0;
	m_mmu_tmp_sr = 0;
	m_mmu_tmp_fc = 0;
	m_mmu_tmp_rw = 0;
//	m_mmu_tmp_buserror_address = 0;
//	m_mmu_tmp_buserror_occurred = 0;
//	m_mmu_tmp_buserror_fc = 0;
//	m_mmu_tmp_buserror_rw = 0;
#endif

#ifdef USE_MC68000CACHE
	for (int i=0;i<M68K_IC_SIZE;i++) {
		m_ic_address[i] = 0;
		m_ic_data[i] = 0;
		m_ic_valid[i] = false;
	}
#endif

	/* The first call to this function initializes the opcode handler jump table */
	build_opcode_table();
}

//
void MC68000BASE::release()
{
	// don't care deleteing DEBUGGER_BUS so that auto released in VM
}

/// power on reset
///
///
void MC68000BASE::reset()
{
	warm_reset(true);
}

/// asserted reset signal
///
///
void MC68000BASE::warm_reset(bool por)
{
	if (por) {
		/* Clear registers */
		for (int i=0;i<16;i++)
			m_dar[i] = M68KI_REG_DEFAULT_VAL;
		for (int i=0;i<4;i++)
			m_sp[i] = M68KI_REG_DEFAULT_VAL;
#ifdef USE_DEBUGGER
		dasm.update_reg(get_sr());
#endif
	}

	/* Disable the PMMU/HMMU on reset, if any */
#ifdef USE_MC68000MMU
	m_pmmu_enabled = 0;
	m_hmmu_enabled = 0;

	m_mmu_tc = 0;
	m_mmu_tt0 = 0;
	m_mmu_tt1 = 0;
#endif

	/* Clear all stop levels and eat up all remaining cycles */
	m_stopped &= (STOP_LEVEL_OUTER_HALT | STOP_LEVEL_BUSREQ);
	m_signals &= ~SIG_MASK_M68K_SOFTALL;
#ifndef USE_MEM_REAL_MACHINE_CYCLE
	m_initial_count = 0;
	m_icount = 0;
#endif

	m_run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Turn off tracing */
	m_t1_flag = m_t0_flag = 0;
	clear_trace();
	/* Interrupt mask to level 7 */
	m_int_mask = 0x0700;
	m_int_level = 0;
	m_int_level_pref = 0;
	m_int_vec_num = 0;
#ifdef USE_MC68000VBR
	/* Reset VBR */
	m_vbr = 0;
#endif
	/* Go to supervisor mode */
	set_sm_flag(SFLAG_SET | MFLAG_CLEAR);

	/* Invalidate the prefetch queue */
	/* Set to arbitrary number since our first fetch is from 0 */
	m_pref_flags = 0;

	/* Read the initial stack pointer and program counter */
	jump(0);
	m_run_mode = now_reset ? RUN_MODE_BERR_AERR_RESET : RUN_MODE_NORMAL;
	m_proc_mode = now_reset ? PROC_MODE_ASSERT_RESET :PROC_MODE_NEGATE_RESET;
	m_reset_cycles = m_cyc_exception[M68K_EXCEPTION_CATEGORY_SSP];

	/* flush the MMU's cache */
	pmmu_atc_flush();

#ifdef USE_MC68000CACHE
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		// clear instruction cache
		ic_clear();
	}
#endif
}

void MC68000BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifdef USE_DEBUGGER
	int int_flags_id = -1;
	uint32_t old_signals = m_signals;
#endif

	switch(id) {
#ifdef USE_MC68000_IRQ_LEVEL
	case SIG_CPU_IRQ:
		prefetch_irq_line(data, mask);
//		set_irq_line(data, mask);
# ifdef USE_DEBUGGER
		// data means interrupt number
		int_flags_id = (m_signals & SIG_MASK_M68K_IPLALL) | id;
# endif
		break;
#else
	case SIG_M68K_IPL0:
	case SIG_M68K_IPL1:
	case SIG_M68K_IPL2:
		prefetch_irq_line(id, (data & mask) != 0);
//		set_irq_line(id, (data & mask) != 0);
# ifdef USE_DEBUGGER
		int_flags_id = id;
# endif
		break;
#endif
	case SIG_M68K_BUSERR:
		if(data & mask) {
			m_signals |= SIG_MASK_M68K_BUSERR;
		}
		else {
			m_signals &= ~SIG_MASK_M68K_BUSERR;
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
#endif
		break;
	case SIG_M68K_ADDRERR:
		if(data & mask) {
			m_signals |= SIG_MASK_M68K_ADDRERR;
		}
		else {
			m_signals &= ~SIG_MASK_M68K_ADDRERR;
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
#endif
		break;
	case SIG_M68K_VPA_AVEC:
		if(data & mask) {
			m_signals |= SIG_MASK_M68K_VPA_AVEC;
			DASM_UPDATE_REG(get_sr());
		}
		else {
			m_signals &= ~SIG_MASK_M68K_VPA_AVEC;
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
#endif
		break;
	case SIG_CPU_RESET:
		if(data & mask) {
			m_signals |= SIG_MASK_M68K_RESET;
			if (!now_reset) {
				now_reset = true;
				DASM_PUSH_RESET(get_sr());
				m_proc_mode = PROC_MODE_ASSERT_RESET;
			}
		}
		else {
			m_signals &= ~SIG_MASK_M68K_RESET;
			if (now_reset) {
				now_reset = false;
				warm_reset(false);
			}
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
#endif
		break;
	case SIG_CPU_BUSREQ:
		// output bus ground signal to devices
		// bgack signal is ignored
		if(data & mask) {
			if (!(m_stopped & STOP_LEVEL_BUSREQ)) {
				write_signals(&outputs_bg, 0xffffffff);
				DASM_PUSH_BUSREQ(REG_PC(), get_sr());
				OUT_DEBUG_BUSREQ(_T("clk:%d MC68000 BUSREQ Asserted"), (int)get_current_clock());
			}
			m_stopped |= STOP_LEVEL_BUSREQ;
			m_signals |= SIG_MASK_M68K_BUSREQ;
		}
		else {
			if (m_stopped & STOP_LEVEL_BUSREQ) {
				write_signals(&outputs_bg, 0);
				OUT_DEBUG_BUSREQ(_T("clk:%d MC68000 BUSREQ Negated"), (int)get_current_clock());
			}
			m_stopped &= ~STOP_LEVEL_BUSREQ;
			m_signals &= ~SIG_MASK_M68K_BUSREQ;
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
		DASM_SET_SIGNALS(m_signals);
#endif
		break;
	case SIG_CPU_HALT:
		if(data & mask) {
			if (!(m_stopped & STOP_LEVEL_OUTER_HALT)) {
				m_signals |= SIG_MASK_M68K_HALT;
				DASM_PUSH_HALT(get_sr());
			}
			m_stopped |= STOP_LEVEL_OUTER_HALT;
//			m_now_halt = true;
		}
		else {
			if (m_stopped & STOP_LEVEL_OUTER_HALT) {
				m_signals &= ~SIG_MASK_M68K_HALT;
			}
			m_stopped &= ~STOP_LEVEL_OUTER_HALT;
//			m_now_halt = false;
		}
#ifdef USE_DEBUGGER
		int_flags_id = id;
#endif
		break;
	default:
		break;
	}

#ifdef USE_DEBUGGER
	// check break point
	if (int_flags_id >= 0) {
		if(d_debugger->now_debugging() && old_signals != m_signals) {
			d_debugger->check_intr_break_points((uint32_t)int_flags_id, old_signals < m_signals ? 1 : 0);
		}
//		dasm.set_signal(m_signals, int_flags_id);
	}
#endif
}

/// @param[in] clock : -1: process one code / >0: continue processing codes until spend given clock
/// @return : spent clock for processing code
int MC68000BASE::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		m_initial_count = 0;
		m_icount = 0;
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
		bool now_debugging = d_debugger->now_debugging();
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended()) {
				emu->mute_sound(true);
				while(d_debugger->now_debugging() && d_debugger->now_suspended()) {
					emu->sleep(10);
				}
			}
			if(d_debugger->now_debugging()) {
				d_mem = d_debugger;
			} else {
				now_debugging = false;
			}

			run_one_opecode();

			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended(true);
				}
				d_mem = d_mem_stored;
			}
		} else
#else
		if (d_debugger->now_debugging()) {
			if (!m_now_debugging) {
				for(int i=0; i<8; i++) {
					d_mem[i] = d_mem_stored[i];
				}
			}
			m_now_debugging = true;
			if (d_debugger->now_suspended()) {
				return 0;
			}
		} else {
			if (m_now_debugging) {
				// memory without debugger
				for(int i=0; i<8; i++) {
					d_mem[i] = d_mem_stored[i]->get_context_mem();
				}
			}
			m_now_debugging = false;
		}
#endif
#endif
		{
			run_one_opecode();
		}
#ifdef USE_MEM_REAL_MACHINE_CYCLE
		m_total_icount += m_icount;

		return m_icount;
#else
		m_total_icount += (m_initial_count - m_icount);

		return m_initial_count - m_icount;
#endif
	}
	else {
		// run cpu while given clocks
#ifdef USE_MEM_REAL_MACHINE_CYCLE
		int final_count = m_initial_count + clock;

		if (final_count > m_icount)
#else
		m_icount += clock;
		m_initial_count = m_icount;

		if (m_icount > 0)
#endif
		{
			do {
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
				bool now_debugging = d_debugger->now_debugging();
				if(now_debugging) {
					d_debugger->check_break_points(PC);
					if(d_debugger->now_suspended()) {
						emu->mute_sound(true);
						while(d_debugger->now_debugging() && d_debugger->now_suspended()) {
							emu->sleep(10);
						}
					}
					if(d_debugger->now_debugging()) {
						d_mem = d_debugger;
					} else {
						now_debugging = false;
					}

					run_one_opecode();

					if(now_debugging) {
						if(!d_debugger->now_going) {
							d_debugger->now_suspended(true);
						}
						d_mem = d_mem_stored;
					}
				} else
#else
				if (d_debugger->now_debugging()) {
					if (!m_now_debugging) {
						for(int i=0; i<8; i++) {
							d_mem[i] = d_mem_stored[i];
						}
					}
					m_now_debugging = true;
					if (d_debugger->now_suspended()) {
						return 0;
					}
				} else {
					if (m_now_debugging) {
						// memory without debugger
						for(int i=0; i<8; i++) {
							d_mem[i] = d_mem_stored[i]->get_context_mem();
						}
					}
					m_now_debugging = false;
				}
#endif
#endif
				{
					run_one_opecode();
				}
			}
#ifdef USE_MEM_REAL_MACHINE_CYCLE
			while(final_count > m_icount);

			final_count = m_icount - m_initial_count;
			m_initial_count += clock;
			if (m_initial_count >= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO)) {
				m_initial_count -= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO);
				m_icount -= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO);
			}
			m_total_icount += clock;
			return final_count;
#else
			while(m_icount > 0);

			m_total_icount += (m_initial_count - m_icount);
			return m_initial_count - m_icount;
#endif
		} else {
#ifdef USE_MEM_REAL_MACHINE_CYCLE
			final_count = m_icount - m_initial_count;
			m_initial_count += clock;
			if (m_initial_count >= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO)) {
				m_initial_count -= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO);
				m_icount -= (CLOCKS_CYCLE << MAIN_SUB_CLOCK_RATIO);
			}
			m_total_icount += clock;
			return 0;
#else
			m_total_icount += (m_initial_count - m_icount);
			return 0;
#endif
		}
	}
}

#ifdef USE_CPU_REAL_MACHINE_CYCLE
/// @param[in] clock : -1: process one code / >0: continue processing codes until spend given clock
/// @param[in] accum : current accumulated clock
/// @param[in] cycle : spent clock is multiplied by cycle
/// @return : spent clock for processing code
int MC68000BASE::run(int clock, int accum, int cycle)
{
	// run only one opcode
	m_initial_count = accum;
	m_icount = accum;
	m_multiple_cycle = cycle;
#if defined(USE_DEBUGGER)
#if defined(USE_SUSPEND_IN_CPU)
	bool now_debugging = d_debugger->now_debugging();
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended()) {
			emu->mute_sound(true);
			while(d_debugger->now_debugging() && d_debugger->now_suspended()) {
				emu->sleep(10);
			}
		}
		if(d_debugger->now_debugging()) {
			d_mem = d_debugger;
		} else {
			now_debugging = false;
		}

		run_one_opecode();

		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended() = true;
			}
			d_mem = d_mem_stored;
		}
	} else
#else
	if (d_debugger->now_debugging()) {
		if (!m_now_debugging) {
			for(int i=0; i<8; i++) {
				d_mem[i] = d_mem_stored[i];
			}
		}
		m_now_debugging = true;
		if (d_debugger->now_suspended()) {
			return 0;
		}
	} else {
		if (m_now_debugging) {
			// memory without debugger
			for(int i=0; i<8; i++) {
				d_mem[i] = d_mem_stored[i]->get_context_mem();
			}
		}
		m_now_debugging = false;
	}
#endif
#endif
	{
		run_one_opecode();
	}
	m_total_icount += (m_icount - m_initial_count);
	return m_icount - m_initial_count;
}
#endif

/// process one instruction, prefetch or exception 
void MC68000BASE::run_one_opecode()
{
	try {
		if (m_stopped) {
			/* now suspended */
			if (m_stopped == STOP_LEVEL_STOP) {
				/* now stop by instruction */
				/* check interrupt signal */
				if (check_interrupts()) SET_ICOUNT(4);
			} else {
				/* now halt or bus request/ack */
				d_mem[m_fc & 7]->update_intr_condition();
				SET_ICOUNT(4);
			}

		} else {
			switch(m_proc_mode & 0x1f) {
			case PROC_MODE_ASSERT_RESET:
				/* Suspend cpu when asserting reset signal */
				d_mem[m_fc & 7]->update_intr_condition();
				SET_ICOUNT(4);
				break;

			case PROC_MODE_NEGATE_RESET:
				{
				/* Read the initial stack pointer and program counter */
					uint32_t count = 0;

					/* eat up any reset cycles */
					SET_ICOUNT(m_reset_cycles);
					m_reset_cycles = 0;
					DASM_SET_REG(0, ACCUM_ICOUNT(), get_sr());

					DASM_PUSH_VECTOR(M68K_EXCEPTION_CATEGORY_SSP, 0, 0);
					count = m_icount;
					set_current_sp(read_32(0));
					count = m_icount - count;
					DASM_SET_REG(0, count, get_sr());

					DASM_PUSH_VECTOR(M68K_EXCEPTION_CATEGORY_PC, 1, 4);
					count = m_icount;
					REG_PC() = read_32(4);
					count = m_icount - count;
					DASM_SET_REG(0, count, get_sr());

					jump(m_pc);

					m_proc_mode = PROC_MODE_NORMAL;
				}
				break;

//			case PROC_MODE_INTERNAL_HALT:
//				/* Duplicate address/bus error */
//				/* normally never enter in it */
//				SET_ICOUNT(4);
//				break;

			case PROC_MODE_EXCEPTION_BUS_ERROR:
				/* Bus error */
				m_aberr.m_occured = false;
				exception_bus_error();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_ADDRESS_ERROR:
				/* Address error */
				m_aberr.m_occured = false;
				exception_address_error();
				write_signal(SIG_M68K_ADDRERR, 0, 1);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION:
				exception_illegal();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_INTERRUPT:
				/* Interrupt */
				exception_interrupt(m_proc_mode & 0xff00);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_ZERO_DIVIDE:
				/* divide by zero */
				exception_trap(M68K_EXCEPTION_CATEGORY_ZERO_DIVIDE, M68K_EXCEPTION_ZERO_DIVIDE);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_CHK:
				/* chk */
				exception_trap(M68K_EXCEPTION_CATEGORY_CHK, M68K_EXCEPTION_CHK);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_TRAPV:
				/* trapv */
				exception_trap(M68K_EXCEPTION_CATEGORY_TRAPV, M68K_EXCEPTION_TRAPV);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_TRAPN:
				/* trap N */
				exception_trapN(M68K_EXCEPTION_TRAP_BASE, (m_proc_mode >> 8) & 0xf);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION:
				exception_privilege_violation();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_TRACE:
				/* Trace */
				exception_trace();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_1010:
				exception_1010();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_1111:
				exception_1111();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_BREAKPOINT:
				exception_breakpoint();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_FORMAT_ERROR:
				exception_format_error();
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			case PROC_MODE_EXCEPTION_MMU_CONFIGURATION:
				/* MMU configuration */
				exception_trap(M68K_EXCEPTION_CATEGORY_OTHER, M68K_EXCEPTION_MMU_CONFIGURATION);
				m_proc_mode = PROC_MODE_NORMAL;
				break;

			default:
				/* See if interrupts came in */
				if (!check_interrupts()) {
					break;
				}

//				/* Stopped ? */
//				if(m_stopped) {
//					SET_ICOUNT(4);
//					break;
//				}

				/* Set tracing accodring to T1. (T0 is done inside instruction) */
				trace_t1(); /* auto-disable (see m68kcpu.h) */

				/* Record previous program counter */
				REG_PPC() = REG_PC();

				if((REG_PC() != m_pref_addr) || !(m_pref_flags & PREF_ENABLE))
				{
					/* Prefetch the instruction code */
					/* Always process it after branch/jump/return. */

					set_fc(m_s_flag | M68K_FC_USER_PROGRAM);

					SET_MMU_TMP_FC(m_fc);
					SET_MMU_TMP_RW(1);
					SET_MMU_TMP_RW(M68K_SZ_WORD);

					has_address_error_for_prog_read(REG_PC(), REG_PC(), m_fc); /* auto-disable (see m68kcpu.h) */

					// process bus error logic in ic_readimm16()
					m_pref_data = ic_readimm16(REG_PC());
					m_pref_addr = REG_PC();
					m_pref_flags = PREF_ENABLE;	// enable and no ignore

					DASM_PUSH_PREF(m_pref_addr, m_pref_data, ACCUM_ICOUNT(), get_sr());

				} else {

					/* Read an instruction and call its handler */
					m_ir = read_first_16();

					m_run_mode = RUN_MODE_NORMAL;

					uint16_t state = m_state_table[m_ir];
					(this->*m68k_handler_table[state])();

					SET_ICOUNT(m_cyc_instruction[m_ir]);

					update_current_sp();

					DASM_SET_REG(0, ACCUM_ICOUNT(), get_sr());

					/* Trace m68k_exception, if necessary */
					exception_if_trace(); /* auto-disable (see m68kcpu.h) */

				}
				break;
			}
		}
		// assert prefetched signal
		{
			uint32_t old_level = m_int_level;

			m_int_level = m_int_level_pref;

			/* A transition from < 7 to 7 always interrupts (NMI) */
			/* Note: Level 7 can also level trigger like a normal IRQ */
			// FIXME: This may cause unintended level 7 interrupts if one or two IPL lines are asserted
			// immediately before others are cleared. The actual 68000 imposes an input hold time.
			if(old_level != SIG_MASK_M68K_IPLALL && m_int_level == SIG_MASK_M68K_IPLALL)
				m_nmi_pending = true;
			else if(old_level == SIG_MASK_M68K_IPLALL && m_int_level != SIG_MASK_M68K_IPLALL)
				m_nmi_pending = false;
		}

	} catch (MC68000ABERR &error) {
		// bus error or address error
		m_aberr = error;
	}
}

/// @param [in] data : interrupt level bit7-bit1 
/// @param [in] mask : mask
void MC68000BASE::prefetch_irq_line(uint32_t data, uint32_t mask)
{
//	uint32_t old_level = m_int_level;

	data &= mask;

	uint32_t new_level;
	for(new_level=0x700; new_level>0x000; new_level-=0x100) {
		if (data & 0x80) {
			break;
		}
		data <<= 1;		
	}
	m_int_level_pref = new_level;

	m_signals = (m_signals & ~SIG_MASK_M68K_IPLALL) | m_int_level_pref;
#ifdef USE_DEBUGGER
	m_signals &= ~SIG_MASK_M68K_DEBUG_INTR; 
	m_signals |= ((data & mask) << 16);

	dasm.set_signals(m_signals);
#endif
	OUT_DEBUG_EX_INTR(_T("clk:%d MC68000 PREF INT Pref:%04X Now:%04X Msk:%04X PROC:%04X")
		, (int)get_current_clock()
		, m_int_level_pref, m_int_level, m_int_mask, m_proc_mode);
}

/// @param [in] id    : SIG_M68K_IPL0 - SIG_M68K_IPL2
/// @param [in] onoff : true: assert the signal / false: negate
void MC68000BASE::prefetch_irq_line(uint32_t id, bool onoff)
{
//	uint32_t old_level = m_int_level;

	int v = id - SIG_M68K_IPL0;
	m_int_level_pref = (onoff ? m_int_level | (0x100 << v) : m_int_level & ~(0x100 << v));

	m_signals = (m_signals & ~SIG_MASK_M68K_IPLALL) | m_int_level_pref;
#ifdef USE_DEBUGGER
	m_signals &= ~SIG_MASK_M68K_DEBUG_INTR; 
	m_signals |= (1 << (m_int_level >> 8)); 

	dasm.set_signals(m_signals);
#endif
}

#if 0
/// @param [in] data : interrupt level bit7-bit1 
/// @param [in] mask : mask
void MC68000BASE::set_irq_line(uint32_t data, uint32_t mask)
{
	uint32_t old_level = m_int_level;

	data &= mask;

	uint32_t new_level;
	for(new_level=0x700; new_level>0x000; new_level-=0x100) {
		if (data & 0x80) {
			break;
		}
		data <<= 1;		
	}
	m_int_level = new_level;

	m_signals = (m_signals & ~SIG_MASK_M68K_IPLALL) | m_int_level;
#ifdef USE_DEBUGGER
	m_signals &= ~SIG_MASK_M68K_DEBUG_INTR; 
	m_signals |= ((data & mask) << 16);
#endif

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	// FIXME: This may cause unintended level 7 interrupts if one or two IPL lines are asserted
	// immediately before others are cleared. The actual 68000 imposes an input hold time.
	if(old_level != SIG_MASK_M68K_IPLALL && m_int_level == SIG_MASK_M68K_IPLALL)
		m_nmi_pending = true;
}

/// @param [in] id    : SIG_M68K_IPL0 - SIG_M68K_IPL2
/// @param [in] onoff : true: assert the signal / false: negate
void MC68000BASE::set_irq_line(uint32_t id, bool onoff)
{
	uint32_t old_level = m_int_level;

	int v = id - SIG_M68K_IPL0;
	m_int_level = (onoff ? m_int_level | (0x100 << v) : m_int_level & ~(0x100 << v));

	m_signals = (m_signals & ~SIG_MASK_M68K_IPLALL) | m_int_level;
#ifdef USE_DEBUGGER
	m_signals &= ~SIG_MASK_M68K_DEBUG_INTR; 
	m_signals |= (1 << (m_int_level >> 8)); 
#endif

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	// FIXME: This may cause unintended level 7 interrupts if one or two IPL lines are asserted
	// immediately before others are cleared. The actual 68000 imposes an input hold time.
	if(old_level != SIG_MASK_M68K_IPLALL && m_int_level == SIG_MASK_M68K_IPLALL)
		m_nmi_pending = true;
}
#endif

/// software reset
void MC68000BASE::reset_peripherals()
{
	write_signals(&outputs_res, 1);
	write_signals(&outputs_res, 0);
}

// ----------------------------------------------------------------------------

uint64_t MC68000BASE::get_current_clock()
{
#ifdef USE_MEM_REAL_MACHINE_CYCLE
	return m_icount;
#else
	return m_total_icount;
#endif
}

// ----------------------------------------------------------------------------

#define SET_Bool(v) vm_state.v = (v ? 1 : 0)
#define SET_Byte(v) vm_state.v = v
#define SET_Uint16_LE(v) vm_state.v = Uint16_LE(v)
#define SET_Uint32_LE(v) vm_state.v = Uint32_LE(v)
#define SET_Uint64_LE(v) vm_state.v = Uint64_LE(v)
#define SET_Int32_LE(v) vm_state.v = Int32_LE(v)

void MC68000BASE::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	SET_Uint32_LE(m_cpu_clock);
	SET_Uint32_LE(m_cpu_type);     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
	SET_Uint32_LE(m_ppc);          /* Previous program counter */
	SET_Uint32_LE(m_pc);           /* Program Counter */
	for(int i=0; i<16; i++) SET_Uint32_LE(m_dar[i]);      /* Data and Address Registers */
	for(int i=0; i<4; i++) SET_Uint32_LE(m_sp[i]);        /* User, Interrupt, and Master Stack Pointers */
	SET_Uint32_LE(m_fc);			 /* Function Code */
#ifdef USE_MC68000VBR
	SET_Uint32_LE(m_vbr);          /* Vector Base Register (m68010+) */
	SET_Uint32_LE(m_sfc);          /* Source Function Code Register (m68010+) */
	SET_Uint32_LE(m_dfc);          /* Destination Function Code Register (m68010+) */
#endif
#ifdef USE_MC68000CACHE
	SET_Uint32_LE(m_cacr);         /* Cache Control Register (m68020, unemulated) */
	SET_Uint32_LE(m_caar);         /* Cache Address Register (m68020, unemulated) */
#endif
	SET_Uint32_LE(m_ir);           /* Instruction Register */
#ifdef USE_MC68000FPU
	for(int i=0; i<8; i++) {
		SET_Uint64_LE(m_fpr[i].high);       /* FPU Data Register (m68030/040) */
		SET_Uint64_LE(m_fpr[i].low);        /* FPU Data Register (m68030/040) */
	}
	SET_Uint32_LE(m_fpiar);        /* FPU Instruction Address Register (m68040) */
	SET_Uint32_LE(m_fpsr);         /* FPU Status Register (m68040) */
	SET_Uint32_LE(m_fpcr);         /* FPU Control Register (m68040) */
#endif
	SET_Uint32_LE(m_t1_flag);      /* Trace 1 */
	SET_Uint32_LE(m_t0_flag);      /* Trace 0 */
	SET_Uint32_LE(m_s_flag);       /* Supervisor:0x8 / User:0x0 */
	SET_Uint32_LE(m_m_flag);       /* Master: 0x04 / Interrupt:0x0 */
	SET_Uint32_LE(m_x_flag);       /* Extend */
	SET_Uint32_LE(m_n_flag);       /* Negative */
	SET_Uint32_LE(m_not_z_flag);   /* Zero, inverted for speedups */
	SET_Uint32_LE(m_v_flag);       /* Overflow */
	SET_Uint32_LE(m_c_flag);       /* Carry */
	SET_Uint32_LE(m_int_mask);     /* I0-I2 */
	SET_Uint32_LE(m_int_level);    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	SET_Uint32_LE(m_int_level_pref); /* Prefetch state of interrupt pins IPL0-IPL2 */
	SET_Uint32_LE(m_int_vec_num);  /* Interrupt vector number */
	SET_Uint32_LE(m_stopped);      /* Stopped state */
	SET_Uint32_LE(m_pref_flags);	 /* Valid data in prefetch queue */
	SET_Uint32_LE(m_pref_addr);    /* Last prefetch address */
	SET_Uint32_LE(m_pref_data);    /* Data in the prefetch queue */
	SET_Uint32_LE(m_sr_mask);      /* Implemented status register bits */
	SET_Uint32_LE(m_instr_mode);   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	SET_Uint32_LE(m_run_mode);     /* Stores whether we are processing a reset, bus error, address error, or something else */
	SET_Uint32_LE(m_proc_mode);    /* State of processing mode b7-b0:proc mode(m68ki_proc_mode_t) b11-b8:intr level */
	SET_Uint32_LE(m_signals);		 /* Asserted signals */
#ifdef USE_MC68000MMU
	SET_Bool(m_has_pmmu);	        /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	SET_Bool(m_has_hmmu);           /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	SET_Int32_LE(m_pmmu_enabled);   /* Indicates if the PMMU is enabled */
	SET_Int32_LE(m_hmmu_enabled);   /* Indicates if the HMMU is enabled */
#endif
#ifdef USE_MC68000FPU
	SET_Bool(m_has_fpu);            /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */
	SET_Int32_LE(m_fpu_just_reset); /* Indicates the FPU was just reset */
#endif
	SET_Uint32_LE(m_cyc_bcc_notake_b);
	SET_Uint32_LE(m_cyc_bcc_notake_w);
	SET_Uint32_LE(m_cyc_dbcc_f_noexp);
	SET_Uint32_LE(m_cyc_dbcc_f_exp);
	SET_Uint32_LE(m_cyc_scc_r_true);
	SET_Uint32_LE(m_cyc_movem_w);
	SET_Uint32_LE(m_cyc_movem_l);
	SET_Uint32_LE(m_cyc_shift);
	SET_Uint32_LE(m_cyc_reset);

	SET_Int32_LE(m_initial_count);
	SET_Int32_LE(m_icount);                     /* Current clock / Number of clocks remaining */
	SET_Int32_LE(m_multiple_cycle);
	SET_Int32_LE(m_reset_cycles);
	SET_Uint32_LE(m_tracing);
	SET_Uint64_LE(m_total_icount);

	SET_Bool(m_nmi_pending);

#ifdef USE_MC68000MMU
	/* PMMU registers */
	SET_Uint32_LE(m_mmu_crp_aptr);
	SET_Uint32_LE(m_mmu_crp_limit);
	SET_Uint32_LE(m_mmu_srp_aptr);
	SET_Uint32_LE(m_mmu_srp_limit);
	SET_Uint32_LE(m_mmu_urp_aptr);    /* 040 only */
	SET_Uint32_LE(m_mmu_tc);
	SET_Uint32_LE(m_mmu_sr_040;
	for(int i=0; i<MMU_ATC_ENTRIES; i++) {
		SET_Uint32_LE(m_mmu_atc_tag[i]);
		SET_Uint32_LE(m_mmu_atc_data[i]);
	}
	SET_Uint32_LE(m_mmu_atc_rr);
	SET_Uint32_LE(m_mmu_tt0, m_mmu_tt1);
	SET_Uint32_LE(m_mmu_itt0, m_mmu_itt1);
	SET_Uint32_LE(m_mmu_dtt0, m_mmu_dtt1);
	SET_Uint32_LE(m_mmu_acr0, m_mmu_acr1);
	SET_Uint32_LE(m_mmu_acr2, m_mmu_acr3);
	SET_Uint32_LE(m_mmu_last_page_entry);
	SET_Uint32_LE(m_mmu_last_page_entry_addr);

	SET_Uint16_LE(m_mmu_sr);
	SET_Uint16_LE(m_mmu_tmp_sr);      /* temporary hack: status code for ptest and to handle write protection */
	SET_Uint16_LE(m_mmu_tmp_fc);      /* temporary hack: function code for the mmu (moves) */
	SET_Uint16_LE(m_mmu_tmp_rw);      /* temporary hack: read/write (1/0) for the mmu */
	SET_Byte(m_mmu_tmp_sz);           /* temporary hack: size for mmu */
	SET_Bool(m_mmu_tablewalk);        /* set when MMU walks page tables */
	SET_Uint32_LE(m_mmu_last_logical_addr);
#endif
#ifdef USE_MC68000CACHE
	for(int i=0; i<M68K_IC_SIZE; i++) {
		SET_Uint32_LE(m_ic_address[i]);   /* instruction cache address data */
		SET_Uint32_LE(m_ic_data[i]);      /* instruction cache content data */
		Set_Bool(m_ic_valid[i]);          /* instruction cache valid flags */
	}
#endif

	m_aberr.save_state(vm_state.m_aberr);

	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fwrite(&vm_state, sizeof(vm_state), 1);

}

#define GET_Bool(v) v = (vm_state.v != 0)
#define GET_Byte(v) v = vm_state.v
#define GET_Uint16_LE(v) v = Uint16_LE(vm_state.v)
#define GET_Uint32_LE(v) v = Uint32_LE(vm_state.v)
#define GET_Uint64_LE(v) v = Uint64_LE(vm_state.v)
#define GET_Int32_LE(v) v = Int32_LE(vm_state.v)

bool MC68000BASE::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fio, vm_state_i, vm_state);

	// copy values
	GET_Uint32_LE(m_cpu_clock);
	GET_Uint32_LE(m_cpu_type);     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
	GET_Uint32_LE(m_ppc);          /* Previous program counter */
	GET_Uint32_LE(m_pc);           /* Program Counter */
	for(int i=0; i<16; i++) GET_Uint32_LE(m_dar[i]);      /* Data and Address Registers */
	for(int i=0; i<4; i++) GET_Uint32_LE(m_sp[i]);        /* User, Interrupt, and Master Stack Pointers */
	GET_Uint32_LE(m_fc);			 /* Function Code */
#ifdef USE_MC68000VBR
	GET_Uint32_LE(m_vbr);          /* Vector Base Register (m68010+) */
	GET_Uint32_LE(m_sfc);          /* Source Function Code Register (m68010+) */
	GET_Uint32_LE(m_dfc);          /* Destination Function Code Register (m68010+) */
#endif
#ifdef USE_MC68000CACHE
	GET_Uint32_LE(m_cacr);         /* Cache Control Register (m68020, unemulated) */
	GET_Uint32_LE(m_caar);         /* Cache Address Register (m68020, unemulated) */
#endif
	GET_Uint32_LE(m_ir);           /* Instruction Register */
#ifdef USE_MC68000FPU
	for(int i=0; i<8; i++) {
		GET_Uint64_LE(m_fpr[i].high);       /* FPU Data Register (m68030/040) */
		GET_Uint64_LE(m_fpr[i].low);        /* FPU Data Register (m68030/040) */
	}
	GET_Uint32_LE(m_fpiar);        /* FPU Instruction Address Register (m68040) */
	GET_Uint32_LE(m_fpsr);         /* FPU Status Register (m68040) */
	GET_Uint32_LE(m_fpcr);         /* FPU Control Register (m68040) */
#endif
	GET_Uint32_LE(m_t1_flag);      /* Trace 1 */
	GET_Uint32_LE(m_t0_flag);      /* Trace 0 */
	GET_Uint32_LE(m_s_flag);       /* Supervisor:0x8 / User:0x0 */
	GET_Uint32_LE(m_m_flag);       /* Master: 0x04 / Interrupt:0x0 */
	GET_Uint32_LE(m_x_flag);       /* Extend */
	GET_Uint32_LE(m_n_flag);       /* Negative */
	GET_Uint32_LE(m_not_z_flag);   /* Zero, inverted for speedups */
	GET_Uint32_LE(m_v_flag);       /* Overflow */
	GET_Uint32_LE(m_c_flag);       /* Carry */
	GET_Uint32_LE(m_int_mask);     /* I0-I2 */
	GET_Uint32_LE(m_int_level);    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	GET_Uint32_LE(m_int_level_pref); /* Prefetch state of interrupt pins IPL0-IPL2 */
	GET_Uint32_LE(m_int_vec_num);  /* Interrupt vector number */
	GET_Uint32_LE(m_stopped);      /* Stopped state */
	GET_Uint32_LE(m_pref_flags);	 /* Valid data in prefetch queue */
	GET_Uint32_LE(m_pref_addr);    /* Last prefetch address */
	GET_Uint32_LE(m_pref_data);    /* Data in the prefetch queue */
	GET_Uint32_LE(m_sr_mask);      /* Implemented status register bits */
	GET_Uint32_LE(m_instr_mode);   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	m_run_mode = (m68ki_run_mode_t)Uint32_LE(vm_state.m_run_mode);     /* Stores whether we are processing a reset, bus error, address error, or something else */
	GET_Uint32_LE(m_proc_mode);    /* State of processing mode b7-b0:proc mode(m68ki_proc_mode_t) b11-b8:intr level */
	GET_Uint32_LE(m_signals);		 /* Asserted signals */
#ifdef USE_MC68000MMU
	GET_Bool(m_has_pmmu);	        /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	GET_Bool(m_has_hmmu);           /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	GET_Int32_LE(m_pmmu_enabled);   /* Indicates if the PMMU is enabled */
	GET_Int32_LE(m_hmmu_enabled);   /* Indicates if the HMMU is enabled */
#endif
#ifdef USE_MC68000FPU
	GET_Bool(m_has_fpu);            /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */
	GET_Int32_LE(m_fpu_just_reset); /* Indicates the FPU was just reset */
#endif
	GET_Uint32_LE(m_cyc_bcc_notake_b);
	GET_Uint32_LE(m_cyc_bcc_notake_w);
	GET_Uint32_LE(m_cyc_dbcc_f_noexp);
	GET_Uint32_LE(m_cyc_dbcc_f_exp);
	GET_Uint32_LE(m_cyc_scc_r_true);
	GET_Uint32_LE(m_cyc_movem_w);
	GET_Uint32_LE(m_cyc_movem_l);
	GET_Uint32_LE(m_cyc_shift);
	GET_Uint32_LE(m_cyc_reset);

	GET_Int32_LE(m_initial_count);
	GET_Int32_LE(m_icount);                     /* Current clock / Number of clocks remaining */
	GET_Int32_LE(m_multiple_cycle);
	GET_Int32_LE(m_reset_cycles);
	GET_Uint32_LE(m_tracing);
	GET_Uint64_LE(m_total_icount);

	GET_Bool(m_nmi_pending);

#ifdef USE_MC68000MMU
	/* PMMU registers */
	GET_Uint32_LE(m_mmu_crp_aptr);
	GET_Uint32_LE(m_mmu_crp_limit);
	GET_Uint32_LE(m_mmu_srp_aptr);
	GET_Uint32_LE(m_mmu_srp_limit);
	GET_Uint32_LE(m_mmu_urp_aptr);    /* 040 only */
	GET_Uint32_LE(m_mmu_tc);
	GET_Uint32_LE(m_mmu_sr_040;
	for(int i=0; i<MMU_ATC_ENTRIES; i++) {
		GET_Uint32_LE(m_mmu_atc_tag[i]);
		GET_Uint32_LE(m_mmu_atc_data[i]);
	}
	GET_Uint32_LE(m_mmu_atc_rr);
	GET_Uint32_LE(m_mmu_tt0, m_mmu_tt1);
	GET_Uint32_LE(m_mmu_itt0, m_mmu_itt1);
	GET_Uint32_LE(m_mmu_dtt0, m_mmu_dtt1);
	GET_Uint32_LE(m_mmu_acr0, m_mmu_acr1);
	GET_Uint32_LE(m_mmu_acr2, m_mmu_acr3);
	GET_Uint32_LE(m_mmu_last_page_entry);
	GET_Uint32_LE(m_mmu_last_page_entry_addr);

	GET_Uint16_LE(m_mmu_sr);
	GET_Uint16_LE(m_mmu_tmp_sr);      /* temporary hack: status code for ptest and to handle write protection */
	GET_Uint16_LE(m_mmu_tmp_fc);      /* temporary hack: function code for the mmu (moves) */
	GET_Uint16_LE(m_mmu_tmp_rw);      /* temporary hack: read/write (1/0) for the mmu */
	GET_Byte(m_mmu_tmp_sz);           /* temporary hack: size for mmu */
	GET_Bool(m_mmu_tablewalk);        /* set when MMU walks page tables */
	GET_Uint32_LE(m_mmu_last_logical_addr);
#endif
#ifdef USE_MC68000CACHE
	for(int i=0; i<M68K_IC_SIZE; i++) {
		GET_Uint32_LE(m_ic_address[i]);   /* instruction cache address data */
		GET_Uint32_LE(m_ic_data[i]);      /* instruction cache content data */
		Get_Bool(m_ic_valid[i]);          /* instruction cache valid flags */
	}
#endif
	return m_aberr.load_state(vm_state.m_aberr);
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
//void MC68000BASE::debug_write_data8(uint32_t addr, uint32_t data)
//{
//	int wait;
//	d_mem_stored->write_data8w(addr, data, &wait);
//}

uint32_t MC68000BASE::debug_read_data16(int type, uint32_t addr)
{
	return d_mem[m_fc & 7]->debug_read_data16(type, addr);
}

uint32_t MC68000BASE::DEBUG_RM8(uint32_t addr, uint32_t fc)
{
	return d_mem[fc & 7]->debug_read_data8(0, addr);
}
uint32_t MC68000BASE::DEBUG_RM16(uint32_t addr, uint32_t fc)
{
	return d_mem[fc & 7]->debug_read_data16(0, addr);
}

#define DEBUG_WRITE_REG(reg_name, reg_variable) \
	if(!wrote && _tcsicmp(reg, reg_name) == 0) { \
		reg_variable = data; \
		wrote = true; \
	}
#define DEBUG_WRITE_A7_REG() \
	if(!wrote && _tcsicmp(reg, _T("A7")) == 0) { \
		m_dar[15] = data; \
		m_sp[(m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1] = data; \
		wrote = true; \
	}
#define DEBUG_WRITE_SR_REG() \
	if(!wrote && _tcsicmp(reg, _T("SR")) == 0) { \
		m_debug_sr = data; \
		set_sr_noint(m_debug_sr); \
		dasm.update_reg(get_sr()); \
		wrote = true; \
	}
#define DEBUG_WRITE_SP_REG(reg_name, num) \
	if(!wrote && _tcsicmp(reg, reg_name) == 0) { \
		m_sp[num] = data; \
		if (((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1) == num) m_dar[15] = data; \
		wrote = true; \
	}

bool MC68000BASE::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	_TCHAR name[4];
	bool wrote = false;

	DEBUG_WRITE_REG(_T("PC"), m_pc)
	name[0] = _T('D'); name[2] = 0;
	for(int i=0; !wrote && i<8; i++) {
		name[1] = i + 0x30;
		DEBUG_WRITE_REG(name, m_dar[i])
	}
	name[1] = '*';
	if(!wrote && _tcsicmp(reg, name) == 0) {
		for(int i=0; !wrote && i<8; i++) {
			m_dar[i] = data;
		}
		wrote = true;
	}
	name[0] = _T('A');
	for(int i=0; !wrote && i<7; i++) {
		name[1] = i + 0x30;
		DEBUG_WRITE_REG(name, m_dar[i + 8])
	}
	name[1] = '*';
	if(!wrote && _tcsicmp(reg, name) == 0) {
		for(int i=0; !wrote && i<7; i++) {
			m_dar[i + 8] = data;
		}
		wrote = true;
	}
	DEBUG_WRITE_A7_REG()
	DEBUG_WRITE_SP_REG(_T("USP"), 0)
	if (CPU_TYPE_IS_020_PLUS()) {
		DEBUG_WRITE_SP_REG(_T("ISP"), 2)
		DEBUG_WRITE_SP_REG(_T("MSP"), 3)
	} else {
		DEBUG_WRITE_SP_REG(_T("SSP"), 2)
	}
	DEBUG_WRITE_SR_REG()
#ifdef USE_MC68000VBR
	if (CPU_TYPE_IS_010_PLUS()) {
		DEBUG_WRITE_REG(_T("VBR"), m_vbr)
		DEBUG_WRITE_REG(_T("SFC"), m_sfc)
		DEBUG_WRITE_REG(_T("DFC"), m_dfc)
	}
#endif
#ifdef USE_MC68000CACHE
	if (CPU_TYPE_IS_020_PLUS()) {
		DEBUG_WRITE_REG(_T("CACR"), m_cacr)
		DEBUG_WRITE_REG(_T("CAAR"), m_caar)
	}
#endif

	if (wrote) {
		dasm.update_reg(get_sr());
	}

	return wrote;
}

void MC68000BASE::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
//	dasm.print_regs_current(m_pc, m_dar, m_sp, get_sr(), m_signals);
	dasm.print_regs_current();
	UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
}

#define SET_DEBUG_REG_PTR(reg_name, reg_variable) \
	if(_tcsicmp(reg, reg_name) == 0) { \
		UTILITY::tcscpy(reg, regsiz, reg_name); \
		regptr = reinterpret_cast<void *>(&reg_variable); \
		reglen = static_cast<int>(sizeof(reg_variable)); \
		return true; \
	}

bool MC68000BASE::get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen)
{
	_TCHAR name[4];
	m_debug_sr = get_sr();
	SET_DEBUG_REG_PTR(_T("PC"), m_pc)
	name[0] = _T('D'); name[2] = 0;
	for(int i=0; i<8; i++) {
		name[1] = i + 0x30;
		SET_DEBUG_REG_PTR(name, m_dar[i])
	}
	name[0] = _T('A');
	for(int i=0; i<8; i++) {
		name[1] = i + 0x30;
		SET_DEBUG_REG_PTR(name, m_dar[i + 8])
	}
	SET_DEBUG_REG_PTR(_T("USP"), m_sp[0])
	if (CPU_TYPE_IS_020_PLUS()) {
		SET_DEBUG_REG_PTR(_T("ISP"), m_sp[2])
		SET_DEBUG_REG_PTR(_T("MSP"), m_sp[3])
	} else {
		SET_DEBUG_REG_PTR(_T("SSP"), m_sp[2])
	}
	SET_DEBUG_REG_PTR(_T("SR"), m_debug_sr)
#ifdef USE_MC68000VBR
	if (CPU_TYPE_IS_010_PLUS()) {
		SET_DEBUG_REG_PTR(_T("VBR"), m_vbr)
		SET_DEBUG_REG_PTR(_T("SFC"), m_sfc)
		SET_DEBUG_REG_PTR(_T("DFC"), m_dfc)
	}
#endif
#ifdef USE_MC68000CACHE
	if (CPU_TYPE_IS_020_PLUS()) {
		SET_DEBUG_REG_PTR(_T("CACR"), m_cacr)
		SET_DEBUG_REG_PTR(_T("CAAR"), m_caar)
	}
#endif
	return false;
}

int MC68000BASE::debug_dasm(int type, uint32_t addr, _TCHAR *buffer, size_t buffer_len, int flags)
{
	int opspos;
	if ((flags & 1) == 0) {
		// next opcode
		opspos = dasm.print_dasm_preprocess(type, addr, flags);
		if (buffer) UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	} else {
		// current opcode
		opspos = dasm.print_dasm_processed();
		if (buffer) UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return (opspos << 1);
}

int MC68000BASE::debug_dasm_label(int type, uint32_t addr, _TCHAR *buffer, size_t buffer_len)
{
	int len = dasm.print_dasm_label(type, addr);
	if (len) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return len;
}

int MC68000BASE::debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_regs_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

int MC68000BASE::debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len)
{
	int next = dasm.print_dasm_traceback(index);
	if (next >= -1) {
		UTILITY::tcscpy(buffer, buffer_len, dasm.get_line());
	}
	return next;
}

bool MC68000BASE::reach_break_point()
{
	return d_debugger->reach_break_point_at(m_pc & debug_prog_addr_mask());
}

#if 0
void MC68000BASE::go_suspend()
{
	d_debugger->go_suspend();
}

bool MC68000BASE::now_suspend() const
{
	return d_debugger->now_suspend();
}
#endif

uint32_t MC68000BASE::get_debug_pc(int type) {
	return m_pc;
}

uint32_t MC68000BASE::get_debug_next_pc(int type) {
	return m_ppc;
}

uint32_t MC68000BASE::get_debug_branch_pc(int type) {
	uint16_t addr = m_debug_npc != m_ppc ? m_debug_npc : m_pc;
	return addr;
}

/// signal names
const MC68000BASE::names_map_t MC68000BASE::signal_names_map[] = {
	{ _T("RESET"), SIG_CPU_RESET },
	{ _T("HALT"), SIG_CPU_HALT },
#ifdef USE_MC68000_IRQ_LEVEL
	{ _T("INT1"), SIG_MASK_M68K_IPL0 | SIG_CPU_IRQ },
	{ _T("INT2"), SIG_MASK_M68K_IPL1 | SIG_CPU_IRQ },
	{ _T("INT3"), SIG_MASK_M68K_IPL1 | SIG_MASK_M68K_IPL0 | SIG_CPU_IRQ },
	{ _T("INT4"), SIG_MASK_M68K_IPL2 | SIG_CPU_IRQ },
	{ _T("INT5"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL0 | SIG_CPU_IRQ },
	{ _T("INT6"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL1 | SIG_CPU_IRQ },
	{ _T("INT7"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL1 | SIG_MASK_M68K_IPL0 | SIG_CPU_IRQ },
#else
	{ _T("IPL0"), SIG_M68K_IPL0 },
	{ _T("IPL1"), SIG_M68K_IPL1 },
	{ _T("IPL2"), SIG_M68K_IPL2 },
#endif
	{ _T("BUSERR"), SIG_M68K_BUSERR },
	{ _T("ADDRERR"), SIG_M68K_ADDRERR },
	{ _T("BUSREQ"), SIG_CPU_BUSREQ },
	{ _T("VPA"), SIG_M68K_VPA_AVEC },
	{ _T("AVEC"), SIG_M68K_VPA_AVEC },
	{ NULL, 0 }
};

bool MC68000BASE::get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, signal_names_map[i].name) == 0) {
			if (num) *num = signal_names_map[i].num;
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

bool MC68000BASE::get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; signal_names_map[i].name != NULL; i++) {
		if (signal_names_map[i].num == num) {
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = signal_names_map[i].name;
			return true;
		}
	}
	return false;
}

void MC68000BASE::get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	buffer[0] = _T('\0');
	for(; signal_names_map[i].name != NULL; i++) {
		if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(","));
		UTILITY::tcscat(buffer, buffer_len, signal_names_map[i].name);
	}
}

/// exception names
const MC68000BASE::names_map_t MC68000BASE::exception_names_map[] = {
	{ _T("BUSERR"), M68K_EXCEPTION_CATEGORY_BUS_ERROR },
	{ _T("ADDRERR"), M68K_EXCEPTION_CATEGORY_ADDRESS_ERROR },
	{ _T("ILLEGAL"), M68K_EXCEPTION_CATEGORY_ILLEGAL_INSTRUCTION },
	{ _T("ZERODIV"), M68K_EXCEPTION_CATEGORY_ZERO_DIVIDE },
	{ _T("CHK"), M68K_EXCEPTION_CATEGORY_CHK },
	{ _T("VIOLATION"), M68K_EXCEPTION_CATEGORY_PRIVILEGE_VIOLATION },
	{ _T("TRACE"), M68K_EXCEPTION_CATEGORY_TRACE },
	{ _T("C1010"), M68K_EXCEPTION_CATEGORY_1010 },
	{ _T("C1111"), M68K_EXCEPTION_CATEGORY_1111 },
	{ _T("FORMAT"), M68K_EXCEPTION_CATEGORY_FORMAT_ERROR },
	{ _T("TRAPV"), M68K_EXCEPTION_CATEGORY_TRAPV },
	{ _T("TRAP0"), 0x000 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP1"), 0x100 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP2"), 0x200 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP3"), 0x300 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP4"), 0x400 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP5"), 0x500 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP6"), 0x600 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP7"), 0x700 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP8"), 0x800 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAP9"), 0x900 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPA"), 0xa00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPB"), 0xb00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPC"), 0xc00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPD"), 0xd00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPE"), 0xe00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("TRAPF"), 0xf00 | M68K_EXCEPTION_CATEGORY_TRAP },
	{ _T("INT1"), SIG_MASK_M68K_IPL0 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT2"), SIG_MASK_M68K_IPL1 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT3"), SIG_MASK_M68K_IPL0 | SIG_MASK_M68K_IPL1 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT4"), SIG_MASK_M68K_IPL2 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT5"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL0 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT6"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL1 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ _T("INT7"), SIG_MASK_M68K_IPL2 | SIG_MASK_M68K_IPL1 | SIG_MASK_M68K_IPL0 | M68K_EXCEPTION_CATEGORY_INTERRUPT },
	{ NULL, 0 }
};

bool MC68000BASE::get_debug_exception_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; exception_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, exception_names_map[i].name) == 0) {
			if (num) *num = exception_names_map[i].num;
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = exception_names_map[i].name;
			return true;
		}
	}
	return false;
}

bool MC68000BASE::get_debug_exception_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; exception_names_map[i].name != NULL; i++) {
		if (exception_names_map[i].num == num) {
			if (mask) *mask = d_debugger->get_stored_mask();
			if (idx) *idx = i;
			if (name) *name = exception_names_map[i].name;
			return true;
		}
	}
	return false;
}

void MC68000BASE::get_debug_exception_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	buffer[0] = _T('\0');
	for(; exception_names_map[i].name != NULL; i++) {
		if (i > 0) UTILITY::tcscat(buffer, buffer_len, _T(","));
		UTILITY::tcscat(buffer, buffer_len, exception_names_map[i].name);
	}
}

#endif /* USE_DEBUGGER */

/****************************************************************************/

/****************************************************************************/

/* ---------------------------- Read Program ------------------------------ */

/// clear the instruction cache
void MC68000BASE::ic_clear()
{
#ifdef USE_MC68000CACHE
	int i;
	for (i=0; i< M68K_IC_SIZE; i++) {
		m_ic_address[i] = ~0;
	}
#endif
}

/// read immediate word using the instruction cache
uint32_t MC68000BASE::ic_readimm16sub(uint32_t address)
{
#ifdef USE_MC68000CACHE
	// 68020 series I-cache (MC68020 User's Manual, Section 4 - On-Chip Cache Memory)
	if (CPU_TYPE_IS_020())
	{
		uint32_t tag = (address >> 8) | (m_s_flag ? 0x1000000 : 0);
		int idx = (address >> 2) & 0x3f;    // 1-of-64 select

		// do a cache fill if the line is invalid or the tags don't match
		if ((!m_ic_valid[idx]) || (m_ic_address[idx] != tag))
		{
			// if the cache is frozen, don't update it
			if (m_cacr & M68K_CACR_FI)
			{
				return RM_PROG_16(address, m_fc);
			}

			//printf("m68k: doing cache fill at %08x (tag %08x idx %d)\n", address, tag, idx);

			// if no buserror occurred, validate the tag
			if (m_signals & SIG_MASK_M68K_BUSERRALL)
			{
				return RM_PROG_16(address, m_fc);
			}
			else
			{
				uint32_t data = RM_PROG_32(address & ~3, m_fc);

				m_ic_address[idx] = tag;
				m_ic_data[idx] = data;
				m_ic_valid[idx] = true;
			}
		}

		// at this point, the cache is guaranteed to be valid, either as
		// a hit or because we just filled it.
		if (address & 2)
		{
			return m_ic_data[idx] & 0xffff;
		}
		else
		{
			return m_ic_data[idx] >> 16;
		}
	}
#endif /* USE_MC68000CACHE */
	return RM_PROG_16(address, m_fc);
}

/// read immediate word (instruction code and operand)
///
/// @note If bus error occurred then throw exception and jump to exception process in caller function.
uint32_t MC68000BASE::ic_readimm16(uint32_t address)
{
	uint32_t value;
#ifdef USE_MC68000CACHE
	if (m_cacr & M68K_CACR_EI)
	{
		value = ic_readimm16sub(address);
	}
	else
#endif /* USE_MC68000CACHE */
	{
		value = RM_PROG_16(address, m_fc);
	}

	has_bus_error_for_read(address, m_fc, value);

	return value;
}

/// read immediate word (instruction code and operand)
///
/// Handles all immediate reads, does address error check, function code setting and prefetching.
uint32_t MC68000BASE::read_imm_16_base()
{
	uint32_t value;

	set_fc(m_s_flag | M68K_FC_USER_PROGRAM);

	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_WORD);

	has_address_error_for_prog_read(REG_PC(), REG_PC(), m_fc); /* auto-disable (see m68kcpu.h) */

#if 0
	if((REG_PC() != m_pref_addr) || !m_pref_enable)
	{
		// process bus error logic in ic_readimm16()
		m_pref_data = ic_readimm16(REG_PC(), m_fc);
		m_pref_addr = REG_PC();
		m_pref_enable = true;
	}
#endif

	value = MASK_OUT_ABOVE_16(m_pref_data);
	REG_PC() += 2;

	DASM_ADD_CODE(value);

	return value;
}

/// read immediate word (instruction code and operand) and use above 1byte
uint32_t MC68000BASE::read_imm_8()
{
	return MASK_OUT_ABOVE_8(read_imm_16());
}

/// read immediate word (instruction code and operand)
uint32_t MC68000BASE::read_imm_16()
{
	uint32_t value = read_imm_16_base();

	if (!(m_pref_flags & PREF_IGNORE)) {
		// prefetch
		// process bus error logic in ic_readimm16()
		m_pref_data = ic_readimm16(REG_PC());
		m_pref_addr = REG_PC();
	}
	m_pref_flags &= ~PREF_IGNORE;

	return value;
}

/// read immediate word (instruction code and operand)
uint32_t MC68000BASE::read_imm_32()
{
	uint32_t value = read_imm_16_base();

	// prefetch
	// process bus error logic in ic_readimm16()
	m_pref_data = ic_readimm16(REG_PC());
	m_pref_addr = REG_PC();

	value = MASK_OUT_ABOVE_32((value << 16) | MASK_OUT_ABOVE_16(m_pref_data));
	REG_PC() += 2;

	DASM_ADD_CODE(value);

	if (!(m_pref_flags & PREF_IGNORE)) {
		// prefetch
		// process bus error logic in ic_readimm16()
		m_pref_data = ic_readimm16(REG_PC());
		m_pref_addr = REG_PC();
	}
	m_pref_flags &= ~PREF_IGNORE;

	return value;
}

uint32_t MC68000BASE::read_first_16()
{
	DASM_PUSH_ADDR(m_pc);
	return read_imm_16();
}

uint32_t MC68000BASE::read_dummy_imm_16()
{
	set_fc(m_s_flag | M68K_FC_USER_PROGRAM);
	return ic_readimm16(REG_PC());
}

/****************************************************************************/

/* ------------------------- Top level read/write ------------------------- */

/* Handles all memory accesses (except for immediate reads).
 * All memory accesses must go through these top level functions.
 * These functions will also check for address error and set the function
 * code.
 */
uint32_t MC68000BASE::read_8_bc(uint32_t address)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_BYTE);

	uint32_t value = RM8(address, m_fc);
	DASM_ADD_MEM2(address, m_fc, (uint8_t)value, false); 

	has_bus_error_for_read(address, m_fc, value);

	return value;
}
uint32_t MC68000BASE::read_16_bc(uint32_t address)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_WORD);

	uint32_t value = RM16(address, m_fc);
	DASM_ADD_MEM2(address, m_fc, (uint16_t)value, false); 

	has_bus_error_for_read(address, m_fc, value);

	return value;
}
uint32_t MC68000BASE::read_32_bc(uint32_t address)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	uint32_t value = RM32(address, m_fc);
	DASM_ADD_MEM2(address, m_fc, (uint32_t)value, false); 

	has_bus_error_for_read(address, m_fc, value);

	return value;
}

void MC68000BASE::write_8_bc(uint32_t address, uint32_t value)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_BYTE);

	WM8(address, m_fc, value);
	DASM_ADD_MEM2(address, m_fc, (uint8_t)value, true); 

	has_bus_error_for_write(address, m_fc, value);
}
void MC68000BASE::write_16_bc(uint32_t address, uint32_t value)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_WORD);

	WM16(address, m_fc, value);
	DASM_ADD_MEM2(address, m_fc, (uint16_t)value, true); 

	has_bus_error_for_write(address, m_fc, value);
}
void MC68000BASE::write_32_bc(uint32_t address, uint32_t value)
{
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	WM32(address, m_fc, value);
	DASM_ADD_MEM2(address, m_fc, (uint32_t)value, true); 

	has_bus_error_for_write(address, m_fc, value);
}

uint32_t MC68000BASE::read_8_ac(uint32_t address)
{
	// no need address error check
	return read_8_bc(address);
}
uint32_t MC68000BASE::read_16_ac(uint32_t address)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_data_read(REG_PPC(), address, m_fc);
	}
	return read_16_bc(address);
}
uint32_t MC68000BASE::read_32_ac(uint32_t address)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_data_read(REG_PPC(), address, m_fc);
	}
	return read_32_bc(address);
}

void MC68000BASE::write_8_ac(uint32_t address, uint32_t value)
{
	// no need address error check
	write_8_bc(address, value);
}
void MC68000BASE::write_16_ac(uint32_t address, uint32_t value)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_data_write(REG_PPC(), address, m_fc, value);
	}
	write_16_bc(address, value);
}
void MC68000BASE::write_32_ac(uint32_t address, uint32_t value)
{
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_data_write(REG_PPC(), address, m_fc, value);
	}
	write_32_bc(address, value);
}

/* ---------------------------- Read/Write data --------------------------- */

/// Read from the current address space
uint32_t MC68000BASE::read_8(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_8_ac(address);
}
/// Read from the current address space
uint32_t MC68000BASE::read_16(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_16_ac(address);
}
/// Read from the current address space
uint32_t MC68000BASE::read_32(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_32_ac(address);
}

/// Write to the current data space
void MC68000BASE::write_8(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	write_8_ac(address, value);
}
/// Write to the current data space
void MC68000BASE::write_16(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	write_16_ac(address, value);
}
/// Write to the current data space
void MC68000BASE::write_32(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	write_32_ac(address, value);
}

uint32_t MC68000BASE::read_naerr_8(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_8_bc(address);
}

uint32_t MC68000BASE::read_naerr_16(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_16_bc(address);
}

uint32_t MC68000BASE::read_naerr_32(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	return read_32_bc(address);
}

/* ----------------------- Read/Write stack data -------------------------- */

#if 0
uint32_t MC68000BASE::read_stack_8(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_BYTE);

	uint32_t value = RM8(address, m_fc);

	has_bus_error_for_read(address, m_fc, value);

	return value;
}
#endif
uint32_t MC68000BASE::read_stack_16(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_read(REG_PPC(), address, m_fc);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_WORD);

	uint32_t value = RM16(address, m_fc);

	has_bus_error_for_read(address, m_fc, value);

	return value;
}
uint32_t MC68000BASE::read_stack_32(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_read(REG_PPC(), address, m_fc);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	uint32_t value = RM32(address, m_fc);

	has_bus_error_for_read(address, m_fc, value);

	return value;
}
uint32_t MC68000BASE::read_stack_32_rev(uint32_t address)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_read(REG_PPC(), address, m_fc);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(1);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	uint32_t value = RM32REV(address, m_fc);

	has_bus_error_for_read(address, m_fc, value);

	return value;
}

#if 0
void MC68000BASE::write_stack_8(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_BYTE);
	WM8(address, m_fc, value);
	has_bus_error_for_write(address, m_fc, value);
}
#endif
void MC68000BASE::write_stack_16(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_write(REG_PPC(), address, m_fc, value);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_WORD);

	WM16(address, m_fc, value);

	has_bus_error_for_write(address, m_fc, value);
}
void MC68000BASE::write_stack_32(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_write(REG_PPC(), address, m_fc, value);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	WM32(address, m_fc, value);

	has_bus_error_for_write(address, m_fc, value);
}
void MC68000BASE::write_stack_32_rev(uint32_t address, uint32_t value)
{
	set_fc(m_s_flag | M68K_FC_USER_DATA);
	if (CPU_TYPE_IS_010_LESS())
	{
		has_address_error_for_stack_write(REG_PPC(), address, m_fc, value);
	}
	SET_MMU_TMP_FC(m_fc);
	SET_MMU_TMP_RW(0);
	SET_MMU_TMP_SZ(M68K_SZ_LONG);

	WM32REV(address, m_fc, value);

	has_bus_error_for_write(address, m_fc, value);
}

uint32_t MC68000BASE::read_intr_8(uint32_t address)
{
	set_fc(M68K_FC_CPU_SPACE);
	return RM8(address, m_fc);
	// This does not catch the bus error signal.
}
uint32_t MC68000BASE::read_intr_16(uint32_t address)
{
	set_fc(M68K_FC_CPU_SPACE);
	return RM16(address, m_fc);
	// This does not catch the bus error signal.
}

/****************************************************************************/

/* --------------------- Effective Address Calculation -------------------- */

/* The program counter relative addressing modes cause operands to be
 * retrieved from program space, not data space.
 */
uint32_t MC68000BASE::get_ea_aw()
{
	uint32_t ea = read_imm_16();

	return MAKE_INT_16(ea);
}
uint32_t MC68000BASE::get_ea_al()
{
	uint32_t ea = read_imm_32();

	return ea;
}
uint32_t MC68000BASE::get_ea_pcdi()
{
	uint32_t old_pc = REG_PC();
	uint32_t ea = read_imm_16();

	return old_pc + MAKE_INT_16(ea);
}
uint32_t MC68000BASE::get_ea_pcix()
{
	return get_ea_ix(REG_PC());
}

/** Indexed addressing modes on MC68000/008/010 are encoded as follows:
 *
 * Base instruction format:
 * F E D C B A 9 8 7 6 | 5 4 3 | 2 1 0
 * x x x x x x x x x x | 1 1 0 | BASE REGISTER      (An)
 *
 * Brief extension format:
 *  F  |  E D C   |  B  | A 9 8 | 7 6 5 4 3 2 1 0
 * D/A | REGISTER | W/L | 0 0 0 |  DISPLACEMENT
 *
 */
uint32_t MC68000BASE::get_ea_ix(uint32_t An)
{
	/* An = base register */
	uint32_t extension = read_imm_16();
	uint32_t Xn = 0;                        /* Index register */

	/* Calculate index */
	Xn = REG_DA()[extension>>12];     /* Xn */
	if(!BIT_B(extension))           /* W/L */
		Xn = MAKE_INT_16(Xn);

	/* Add base register and displacement and return */
	return An + Xn + MAKE_INT_8(extension);
}

uint32_t MC68000BASE::get_ea_di(uint32_t An)
{
	uint32_t ea = read_imm_16();

	return An + MAKE_INT_16(ea);
}

/* ----------------------------- Fetch operands --------------------------- */

uint32_t MC68000BASE::oper_ay_ai_8()  {uint32_t ea = EA_AY_AI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ay_ai_16() {uint32_t ea = EA_AY_AI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ay_ai_32() {uint32_t ea = EA_AY_AI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ay_pi_8()  {uint32_t ea = EA_AY_PI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ay_pi_16() {uint32_t ea = EA_AY_PI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ay_pi_32() {uint32_t ea = EA_AY_PI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ay_pd_8()  {uint32_t ea = EA_AY_PD_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ay_pd_16() {uint32_t ea = EA_AY_PD_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ay_pd_32() {uint32_t ea = EA_AY_PD_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ay_di_8()  {uint32_t ea = EA_AY_DI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ay_di_16() {uint32_t ea = EA_AY_DI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ay_di_32() {uint32_t ea = EA_AY_DI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ay_ix_8()  {uint32_t ea = EA_AY_IX_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ay_ix_16() {uint32_t ea = EA_AY_IX_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ay_ix_32() {uint32_t ea = EA_AY_IX_32(); return read_32(ea);}

uint32_t MC68000BASE::oper_ax_ai_8()  {uint32_t ea = EA_AX_AI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ax_ai_16() {uint32_t ea = EA_AX_AI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ax_ai_32() {uint32_t ea = EA_AX_AI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ax_pi_8()  {uint32_t ea = EA_AX_PI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ax_pi_16() {uint32_t ea = EA_AX_PI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ax_pi_32() {uint32_t ea = EA_AX_PI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ax_pd_8()  {uint32_t ea = EA_AX_PD_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ax_pd_16() {uint32_t ea = EA_AX_PD_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ax_pd_32() {uint32_t ea = EA_AX_PD_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ax_di_8()  {uint32_t ea = EA_AX_DI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ax_di_16() {uint32_t ea = EA_AX_DI_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ax_di_32() {uint32_t ea = EA_AX_DI_32(); return read_32(ea);}
uint32_t MC68000BASE::oper_ax_ix_8()  {uint32_t ea = EA_AX_IX_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_ax_ix_16() {uint32_t ea = EA_AX_IX_16(); return read_16(ea);}
uint32_t MC68000BASE::oper_ax_ix_32() {uint32_t ea = EA_AX_IX_32(); return read_32(ea);}

uint32_t MC68000BASE::oper_a7_pi_8()  {uint32_t ea = EA_A7_PI_8();  return read_8(ea); }
uint32_t MC68000BASE::oper_a7_pd_8()  {uint32_t ea = EA_A7_PD_8();  return read_8(ea); }

uint32_t MC68000BASE::oper_aw_8()     {uint32_t ea = EA_AW_8();     return read_8(ea); }
uint32_t MC68000BASE::oper_aw_16()    {uint32_t ea = EA_AW_16();    return read_16(ea);}
uint32_t MC68000BASE::oper_aw_32()    {uint32_t ea = EA_AW_32();    return read_32(ea);}
uint32_t MC68000BASE::oper_al_8()     {uint32_t ea = EA_AL_8();     return read_8(ea); }
uint32_t MC68000BASE::oper_al_16()    {uint32_t ea = EA_AL_16();    return read_16(ea);}
uint32_t MC68000BASE::oper_al_32()    {uint32_t ea = EA_AL_32();    return read_32(ea);}
uint32_t MC68000BASE::oper_pcdi_8()   {uint32_t ea = EA_PCDI_8();   return read_naerr_8(ea); }
uint32_t MC68000BASE::oper_pcdi_16()  {uint32_t ea = EA_PCDI_16();  return read_naerr_16(ea);}
uint32_t MC68000BASE::oper_pcdi_32()  {uint32_t ea = EA_PCDI_32();  return read_naerr_32(ea);}
uint32_t MC68000BASE::oper_pcix_8()   {uint32_t ea = EA_PCIX_8();   return read_naerr_8(ea); }
uint32_t MC68000BASE::oper_pcix_16()  {uint32_t ea = EA_PCIX_16();  return read_naerr_16(ea);}
uint32_t MC68000BASE::oper_pcix_32()  {uint32_t ea = EA_PCIX_32();  return read_naerr_32(ea);}
//uint32_t MC68000BASE::oper_pcdi_8()   {uint32_t ea = EA_PCDI_8();   return read_pcrel_8(ea); }
//uint32_t MC68000BASE::oper_pcdi_16()  {uint32_t ea = EA_PCDI_16();  return read_pcrel_16(ea);}
//uint32_t MC68000BASE::oper_pcdi_32()  {uint32_t ea = EA_PCDI_32();  return read_pcrel_32(ea);}
//uint32_t MC68000BASE::oper_pcix_8()   {uint32_t ea = EA_PCIX_8();   return read_pcrel_8(ea); }
//uint32_t MC68000BASE::oper_pcix_16()  {uint32_t ea = EA_PCIX_16();  return read_pcrel_16(ea);}
//uint32_t MC68000BASE::oper_pcix_32()  {uint32_t ea = EA_PCIX_32();  return read_pcrel_32(ea);}

/* ---------------------------- Stack Functions --------------------------- */

/* Push/pull data from the stack */
void MC68000BASE::push_16(uint32_t value)
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() - 2));
	write_stack_16(REG_SP(), value);
}

void MC68000BASE::push_32(uint32_t value)
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() - 4));
	write_stack_32(REG_SP(), value);
}

uint32_t MC68000BASE::pull_16()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + 2));
	return read_stack_16(REG_SP()-2);
}

uint32_t MC68000BASE::pull_32()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + 4));
	return read_stack_32(REG_SP()-4);
}


/* Increment/decrement the stack as if doing a push/pull but
 * don't do any memory access.
 */
void MC68000BASE::fake_push_16()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() - 2));
}

void MC68000BASE::fake_push_32()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() - 4));
}

void MC68000BASE::fake_pull_16()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + 2));
}

void MC68000BASE::fake_pull_32()
{
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + 4));
}

/* ----------------------------- Program Flow ----------------------------- */

/* Jump to a new program location or vector.
 * These functions will also call the pc_changed callback if it was enabled
 * in m68kconf.h.
 */
void MC68000BASE::jump(uint32_t new_pc)
{
	REG_PC() = new_pc;
}

void MC68000BASE::jump_vector(uint32_t category, uint32_t vector)
{
	REG_PC() = (vector<<2);
#ifdef USE_MC68000VBR
	REG_PC() += m_vbr;
#endif
	DASM_PUSH_VECTOR(category, vector, REG_PC());
	REG_PC() = read_32(REG_PC());
}

/* Branch to a new memory location.
 * The 32-bit branch will call pc_changed if it was enabled in m68kconf.h.
 * So far I've found no problems with not calling pc_changed for 8 or 16
 * bit branches.
 */
void MC68000BASE::branch_8(uint32_t offset)
{
	REG_PC() += MAKE_INT_8(offset);
}

void MC68000BASE::branch_16(uint32_t offset)
{
	REG_PC() += MAKE_INT_16(offset);
}

void MC68000BASE::branch_32(uint32_t offset)
{
	REG_PC() += offset;
}

void MC68000BASE::non_branch_8(uint32_t offset)
{
	// nothing to do
}

void MC68000BASE::non_branch_16(uint32_t offset)
{
	DASM_ADD_CODE(offset);
}

void MC68000BASE::non_branch_32(uint32_t offset)
{
	DASM_ADD_CODE(offset >> 16);
	DASM_ADD_CODE(offset & 0xffff);
}

/* ---------------------------- Status Register --------------------------- */

/// update current stack pointer
void MC68000BASE::update_current_sp()
{
	int pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP_BASE()[pos] = REG_SP();
}

/// set a value of current stack pointer and backup it
void MC68000BASE::set_current_sp(uint32_t value)
{
	REG_SP() = value;
	int pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP_BASE()[pos] = REG_SP();
}

/// Set the S flag and change the active stack pointer.
/// @note value MUST be 2 or 0.
void MC68000BASE::set_s_flag(uint32_t value)
{
	/* Backup the old stack pointer */
	int pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP_BASE()[pos] = REG_SP();
	/* Set the S flag */
	m_s_flag = value;
	/* Set the new stack pointer */
	pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP() = REG_SP_BASE()[pos];
}

/// Set the S and M flags and change the active stack pointer.
/// @note value MUST be 0, 1, 2, or 3 (bit1 = S, bit0 = M).
void MC68000BASE::set_sm_flag(uint32_t value)
{
	/* Backup the old stack pointer */
	int pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP_BASE()[pos] = REG_SP();
	/* Set the S and M flags */
	m_s_flag = value & SFLAG_SET;
	m_m_flag = value & MFLAG_SET;
	/* Set the new stack pointer */
	pos = ((m_s_flag | ((m_s_flag >> 1) & m_m_flag)) >> 1);
	REG_SP() = REG_SP_BASE()[pos];
}

/* Set the S and M flags.  Don't touch the stack pointer. */
void MC68000BASE::set_sm_flag_nosp(uint32_t value)
{
	/* Set the S and M flags */
	m_s_flag = value & SFLAG_SET;
	m_m_flag = value & MFLAG_SET;
}

/* Set the condition code register */
void MC68000BASE::set_ccr(uint32_t value)
{
	m_x_flag = BIT_4(value)  << 4;
	m_n_flag = BIT_3(value)  << 4;
	m_not_z_flag = !BIT_2(value);
	m_v_flag = BIT_1(value)  << 6;
	m_c_flag = BIT_0(value)  << 8;
}

/* Set the status register but don't check for interrupts */
void MC68000BASE::set_sr_noint(uint32_t value)
{
#ifdef OUT_DEBUG_INT_MSK
	uint32_t prev = m_int_mask;
#endif
	/* Mask out the "unimplemented" bits */
	value &= m_sr_mask;

	/* Now set the status register */
	m_t1_flag = BIT_F(value);
	m_t0_flag = BIT_E(value);
	m_int_mask = value & 0x0700;
#ifdef OUT_DEBUG_INT_MSK
	if (prev != m_int_mask) {
		OUT_DEBUG_INT_MSK(_T("clk:%d MC68000 INT MSK %04X -> %04X(%04X)"), (int)get_current_clock(), prev, m_int_mask_pref, m_int_mask);
	}
#endif
	set_ccr(value);
	set_sm_flag((value >> 11) & 6);
}

/* Set the status register but don't check for interrupts nor
 * change the stack pointer
 */
void MC68000BASE::set_sr_noint_nosp(uint32_t value)
{
#ifdef OUT_DEBUG_INT_MSK
	uint32_t prev = m_int_mask;
#endif
	/* Mask out the "unimplemented" bits */
	value &= m_sr_mask;

	/* Now set the status register */
	m_t1_flag = BIT_F(value);
	m_t0_flag = BIT_E(value);
	m_int_mask = value & 0x0700;
#ifdef OUT_DEBUG_INT_MSK
	if (prev != m_int_mask) {
		OUT_DEBUG_INT_MSK(_T("clk:%d MC68000 INT MSK %04X -> %04X(%04X)"), (int)get_current_clock(), prev, m_int_mask_pref, m_int_mask);
	}
#endif
	set_ccr(value);
	set_sm_flag_nosp((value >> 11) & 6);
}

/* Set the status register and check for interrupts */
void MC68000BASE::set_sr(uint32_t value)
{
	set_sr_noint(value);
// interrput is checked before next operation
//	check_interrupts();
}

/* Get the condition code register */
uint32_t MC68000BASE::get_ccr()
{
	return ((COND_XS() >> 4) |
		(COND_MI() >> 4) |
		(COND_EQ() << 2) |
		(COND_VS() >> 6) |
		(COND_CS() >> 8));
}

/* Get the status register */
uint32_t MC68000BASE::get_sr()
{
	return (m_t1_flag |
		m_t0_flag |
		(m_s_flag << 11) |
		(m_m_flag << 11) |
		m_int_mask |
		get_ccr());
}

/* Set the fuction code */
void MC68000BASE::set_fc(uint32_t new_fc)
{
	if (m_fc != new_fc) {
		write_signals(&outputs_fc, new_fc);
		m_fc = new_fc;
	}
}

/****************************************************************************/

/* ------------------------- Exception Processing ------------------------- */

/* Initiate exception processing */
uint32_t MC68000BASE::init_exception(uint32_t category, uint32_t vector)
{
	/* Save the old status register */
	uint32_t sr = get_sr();

	/* Turn off trace flag, clear pending traces */
	m_t1_flag = m_t0_flag = 0;
	clear_trace();
	/* Enter supervisor mode */
	set_s_flag(SFLAG_SET);

	m_signals |= SIG_MASK_M68K_EXCEPTION;

#ifdef USE_DEBUGGER
	if(d_debugger->now_debugging()) {
		d_debugger->check_exception_break_points(category, vector);
	}
#endif
	return sr;
}

void MC68000BASE::term_exception()
{
	read_dummy_imm_16();

	DASM_SET_REG(0, ACCUM_ICOUNT(), get_sr());

	m_signals &= ~SIG_MASK_M68K_EXCEPTION;
}

/* ----------------------- Stack Frame Processing ------------------------- */

/// 3 word stack frame (68000 only)
void MC68000BASE::stack_frame_3word(uint32_t pc, uint32_t sr)
{
	push_32(pc);
	push_16(sr);
	update_current_sp();
}

/// Format 0 stack frame.
/// This is the standard stack frame for 68010+.
///
void MC68000BASE::stack_frame_0000(uint32_t pc, uint32_t sr, uint32_t vector)
{
	/* Stack a 3-word frame if we are 68000 */
	if(CPU_TYPE_IS_000())
	{
		stack_frame_3word(pc, sr);
		return;
	}
	push_16(vector<<2);
	push_32(pc);
	push_16(sr);
	update_current_sp();
}

/// Format 1 stack frame (68020).
/// For 68020, this is the 4 word throwaway frame.
///
void MC68000BASE::stack_frame_0001(uint32_t pc, uint32_t sr, uint32_t vector)
{
	push_16(0x1000 | (vector<<2));
	push_32(pc);
	push_16(sr);
	update_current_sp();
}

/// Format 2 stack frame.
/// This is used only by 68020 for trap exceptions.
///
void MC68000BASE::stack_frame_0010(uint32_t sr, uint32_t vector)
{
	push_32(REG_PPC());
	push_16(0x2000 | (vector<<2));
	push_32(REG_PC());
	push_16(sr);
	update_current_sp();
}


/// Bus error stack frame (68000 only).
///
void MC68000BASE::stack_frame_buserr(uint32_t sr)
{
	push_32(REG_PC());
	push_16(sr);
	push_16(m_ir);
	push_32(m_aberr.m_address);    /* access address */
	/* 0 0 0 0 0 0 0 0 0 0 0 R/W I/N FC
	 * R/W  0 = write, 1 = read
	 * I/N  0 = instruction, 1 = not
	 * FC   3-bit function code
	 */
	push_16((m_aberr.m_mode << 4) | m_instr_mode | m_aberr.m_fc);
	update_current_sp();
}

/// Format 8 stack frame (68010).
/// 68010 only.  This is the 29 word bus/address error frame.
///
void MC68000BASE::stack_frame_1000(uint32_t pc, uint32_t sr, uint32_t vector)
{
	/* VERSION
	 * NUMBER
	 * INTERNAL INFORMATION, 16 WORDS
	 */
	fake_push_32();
	fake_push_32();
	fake_push_32();
	fake_push_32();
	fake_push_32();
	fake_push_32();
	fake_push_32();
	fake_push_32();

	/* INSTRUCTION INPUT BUFFER */
	push_16(0);

	/* UNUSED, RESERVED (not written) */
	fake_push_16();

	/* DATA INPUT BUFFER */
	push_16(0);

	/* UNUSED, RESERVED (not written) */
	fake_push_16();

	/* DATA OUTPUT BUFFER */
	push_16(0);

	/* UNUSED, RESERVED (not written) */
	fake_push_16();

	/* FAULT ADDRESS */
	push_32(0);

	/* SPECIAL STATUS WORD */
	push_16(0);

	/* 1000, VECTOR OFFSET */
	push_16(0x8000 | (vector<<2));

	/* PROGRAM COUNTER */
	push_32(pc);

	/* STATUS REGISTER */
	push_16(sr);

	update_current_sp();
}

/// Format A stack frame (short bus fault).
/// This is used only by 68020 for bus fault and address error
/// if the error happens at an instruction boundary.
/// PC stacked is address of next instruction.
///
void MC68000BASE::stack_frame_1010(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address)
{
	int orig_rw = m_aberr.m_mode;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_aberr.m_fc;
	int orig_sz = m_aberr.m_size;

	/* INTERNAL REGISTER */
	push_16(0);

	/* INTERNAL REGISTER */
	push_16(0);

	/* DATA OUTPUT BUFFER (2 words) */
	push_32(0);

	/* INTERNAL REGISTER */
	push_16(0);

	/* INTERNAL REGISTER */
	push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	push_32(fault_address);

	/* INSTRUCTION PIPE STAGE B */
	push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	push_16(0);

	/* SPECIAL STATUS REGISTER */
	// set bit for: Rerun Faulted bus Cycle, or run pending prefetch
	// set FC
	push_16(0x0100 | orig_fc | (orig_rw<<6) | (orig_sz<<4));

	/* INTERNAL REGISTER */
	push_16(0);

	/* 1010, VECTOR OFFSET */
	push_16(0xa000 | (vector<<2));

	/* PROGRAM COUNTER */
	push_32(pc);

	/* STATUS REGISTER */
	push_16(sr);

	update_current_sp();
}

/// Format B stack frame (long bus fault).
/// This is used only by 68020 for bus fault and address error
/// if the error happens during instruction execution.
/// PC stacked is address of instruction in progress.
///
void MC68000BASE::stack_frame_1011(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address)
{
	int orig_rw = m_aberr.m_mode;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_aberr.m_fc;
	int orig_sz = m_aberr.m_size;

	/* INTERNAL REGISTERS (18 words) */
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);

	/* VERSION# (4 bits), INTERNAL INFORMATION */
	push_16(0);

	/* INTERNAL REGISTERS (3 words) */
	push_32(0);
	push_16(0);

	/* DATA INTPUT BUFFER (2 words) */
	push_32(0);

	/* INTERNAL REGISTERS (2 words) */
	push_32(0);

	/* STAGE B ADDRESS (2 words) */
	push_32(0);

	/* INTERNAL REGISTER (4 words) */
	push_32(0);
	push_32(0);

	/* DATA OUTPUT BUFFER (2 words) */
	push_32(0);

	/* INTERNAL REGISTER */
	push_16(0);

	/* INTERNAL REGISTER */
	push_16(0);

	/* DATA CYCLE FAULT ADDRESS (2 words) */
	push_32(fault_address);

	/* INSTRUCTION PIPE STAGE B */
	push_16(0);

	/* INSTRUCTION PIPE STAGE C */
	push_16(0);

	/* SPECIAL STATUS REGISTER */
	push_16(0x0100 | orig_fc | (orig_rw<<6) | (orig_sz<<4));

	/* INTERNAL REGISTER */
	push_16(0);

	/* 1011, VECTOR OFFSET */
	push_16(0xb000 | (vector<<2));

	/* PROGRAM COUNTER */
	push_32(pc);

	/* STATUS REGISTER */
	push_16(sr);

	update_current_sp();
}

/// Type 7 stack frame (access fault).
/// This is used by the 68040 for bus fault and mmu trap
/// 30 words
///
void MC68000BASE::stack_frame_0111(uint32_t sr, uint32_t vector, uint32_t pc, uint32_t fault_address, bool in_mmu)
{
	int orig_rw = m_aberr.m_mode;    // this gets splatted by the following pushes, so save it now
	int orig_fc = m_aberr.m_fc;

	/* INTERNAL REGISTERS (18 words) */
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);
	push_32(0);

	/* FAULT ADDRESS (2 words) */
	push_32(fault_address);

	/* INTERNAL REGISTERS (3 words) */
	push_32(0);
	push_16(0);

	/* SPECIAL STATUS REGISTER (1 word) */
	push_16((in_mmu ? 0x400 : 0) | orig_fc | (orig_rw<<8));

	/* EFFECTIVE ADDRESS (2 words) */
	push_32(fault_address);

	/* 0111, VECTOR OFFSET (1 word) */
	push_16(0x7000 | (vector<<2));

	/* PROGRAM COUNTER (2 words) */
	push_32(pc);

	/* STATUS REGISTER (1 word) */
	push_16(sr);

	update_current_sp();
}

/* ------------------------- Exception Details ---------------------------- */

/// Used for Group 2 exceptions.
/// These stack a type 2 frame on the 020.
///
void MC68000BASE::exception_trap(uint32_t category, uint32_t vector)
{
	uint32_t sr = init_exception(category, vector);

	if(CPU_TYPE_IS_010_LESS())
		stack_frame_0000(REG_PC(), sr, vector);
	else
		stack_frame_0010(sr, vector);

	jump_vector(category, vector);

	/* Use up some clock cycles */
	SET_ICOUNT(m_cyc_exception[category]);

	term_exception();
}

/// Trap#n stacks a 0 frame but behaves like group2 otherwise
///
void MC68000BASE::exception_trapN(uint32_t vector_base, uint32_t number)
{
#ifdef OUT_DEBUG_EX_TRAPE
	if (number == 0xe) {
		OUT_DEBUG_EX_TRAPE(_T("clk:%d MC68000 TRAPE"), (int)get_current_clock());
	}
#endif

	uint32_t sr = init_exception(number << 8 | M68K_EXCEPTION_CATEGORY_TRAP, vector_base + number);

	stack_frame_0000(REG_PC(), sr, vector_base + number);

	jump_vector(M68K_EXCEPTION_CATEGORY_TRAP, vector_base + number);

	/* Use up some clock cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_TRAP]);

	term_exception();
}

/// Check trace flag
void MC68000BASE::exception_if_trace()
{
	if(m_tracing) {
		if ((m_proc_mode & 0xff) == PROC_MODE_NORMAL) {
			m_proc_mode = PROC_MODE_EXCEPTION_TRACE;
		}
	}
}

/// Exception for trace mode
void MC68000BASE::exception_trace()
{
	uint32_t sr = init_exception(M68K_EXCEPTION_CATEGORY_TRACE, M68K_EXCEPTION_TRACE);

	if(CPU_TYPE_IS_010_LESS())
	{
		if(CPU_TYPE_IS_000())
		{
			m_instr_mode = INSTRUCTION_NO;
		}
		stack_frame_0000(REG_PC(), sr, M68K_EXCEPTION_TRACE);
	}
	else
	{
		stack_frame_0010(sr, M68K_EXCEPTION_TRACE);
	}

	jump_vector(M68K_EXCEPTION_CATEGORY_TRACE, M68K_EXCEPTION_TRACE);

	/* Trace nullifies a STOP instruction */
	m_stopped &= ~STOP_LEVEL_STOP;
	m_signals &= ~SIG_MASK_M68K_STOPBYSW;

	/* Use up some clock cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_TRACE]);

	term_exception();
}

/// Exception for privilege violation
void MC68000BASE::exception_privilege_violation()
{
	uint32_t sr = init_exception(M68K_EXCEPTION_CATEGORY_PRIVILEGE_VIOLATION, M68K_EXCEPTION_PRIVILEGE_VIOLATION);

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	stack_frame_0000(REG_PPC(), sr, M68K_EXCEPTION_PRIVILEGE_VIOLATION);
	jump_vector(M68K_EXCEPTION_CATEGORY_PRIVILEGE_VIOLATION, M68K_EXCEPTION_PRIVILEGE_VIOLATION);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_PRIVILEGE_VIOLATION] - m_cyc_instruction[m_ir]);

	term_exception();
}

/// Exception for A-Line instructions
void MC68000BASE::exception_1010()
{
	uint32_t sr;

	sr = init_exception(M68K_EXCEPTION_CATEGORY_1010, M68K_EXCEPTION_1010);
	stack_frame_0000(REG_PPC(), sr, M68K_EXCEPTION_1010);
	jump_vector(M68K_EXCEPTION_CATEGORY_1010, M68K_EXCEPTION_1010);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_1010] - m_cyc_instruction[m_ir]);

	term_exception();
}

/// Exception for F-Line instructions
void MC68000BASE::exception_1111()
{
	uint32_t sr;

	sr = init_exception(M68K_EXCEPTION_CATEGORY_1111, M68K_EXCEPTION_1111);
	stack_frame_0000(REG_PPC(), sr, M68K_EXCEPTION_1111);
	jump_vector(M68K_EXCEPTION_CATEGORY_1111, M68K_EXCEPTION_1111);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_1111] - m_cyc_instruction[m_ir]);

	term_exception();
}

/// before exception for illegal instructions
void MC68000BASE::exception_breakpoint()
{
	// set address bus on cpu space
	if (CPU_TYPE_IS_020_PLUS()) {
		read_intr_16((m_ir & 7) << 2);
	} else if (CPU_TYPE_IS_010()) {
		read_intr_16(0);
	}

	exception_illegal();
}

/// Exception for illegal instructions
void MC68000BASE::exception_illegal()
{
	uint32_t sr;

	sr = init_exception(M68K_EXCEPTION_CATEGORY_ILLEGAL_INSTRUCTION, M68K_EXCEPTION_ILLEGAL_INSTRUCTION);

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	stack_frame_0000(REG_PPC(), sr, M68K_EXCEPTION_ILLEGAL_INSTRUCTION);
	jump_vector(M68K_EXCEPTION_CATEGORY_ILLEGAL_INSTRUCTION, M68K_EXCEPTION_ILLEGAL_INSTRUCTION);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_ILLEGAL_INSTRUCTION] - m_cyc_instruction[m_ir]);

	term_exception();
}

/// Exception for format errror in RTE
void MC68000BASE::exception_format_error()
{
	uint32_t sr = init_exception(M68K_EXCEPTION_CATEGORY_FORMAT_ERROR, M68K_EXCEPTION_FORMAT_ERROR);

	stack_frame_0000(REG_PC(), sr, M68K_EXCEPTION_FORMAT_ERROR);
	jump_vector(M68K_EXCEPTION_CATEGORY_FORMAT_ERROR, M68K_EXCEPTION_FORMAT_ERROR);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_FORMAT_ERROR] - m_cyc_instruction[m_ir]);

	term_exception();
}

/* --------------------------- Address Error ------------------------------ */

/// @param[in] pc : program counter read instruction
/// @param[in] address : address to jump or branch
/// @param[in] fc : function code
void MC68000BASE::has_address_error_for_prog_read(uint32_t pc, uint32_t address, uint32_t fc)
{
	if(address & 1) {
		write_signal(SIG_M68K_ADDRERR, 1, 1);
		DASM_PUSH_AERR(address);
		show_error_message(pc, address);
		m_proc_mode = PROC_MODE_EXCEPTION_ADDRESS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_ADDRESS_ERROR, address, MODE_READ, fc);
	}
}
/// @param[in] pc : program counter read instruction
/// @param[in] address : address to jump or branch
/// @param[in] fc : function code
/// @param[in] value : data on address
void MC68000BASE::has_address_error_for_data_write(uint32_t pc, uint32_t address, uint32_t fc, uint32_t value)
{
	if(address & 1) {
		write_signal(SIG_M68K_ADDRERR, 1, 1);
		DASM_ADD_MEM2(address, fc, value, true);
		DASM_PUSH_AERR(address);
		show_error_message(pc, address);
		m_proc_mode = PROC_MODE_EXCEPTION_ADDRESS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_ADDRESS_ERROR, address, MODE_WRITE, fc);
	}
}
/// @param[in] pc : program counter read instruction
/// @param[in] address : address to jump or branch
/// @param[in] fc : function code
void MC68000BASE::has_address_error_for_data_read(uint32_t pc, uint32_t address, uint32_t fc)
{
	if(address & 1) {
		write_signal(SIG_M68K_ADDRERR, 1, 1);
		DASM_ADD_MEM2(address, fc, DEBUG_RM8(address, fc), false);
		DASM_PUSH_AERR(address);
		show_error_message(pc, address);
		m_proc_mode = PROC_MODE_EXCEPTION_ADDRESS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_ADDRESS_ERROR, address, MODE_READ, fc);
	}
}
/// @param[in] pc : program counter read instruction
/// @param[in] address : address to jump or branch
/// @param[in] fc : function code
/// @param[in] value : data on address
void MC68000BASE::has_address_error_for_stack_write(uint32_t pc, uint32_t address, uint32_t fc, uint32_t value)
{
	if(address & 1) {
		write_signal(SIG_M68K_ADDRERR, 1, 1);
		DASM_PUSH_AERR2(address, fc, value, true);
		show_error_message(pc, address);
		m_proc_mode = PROC_MODE_EXCEPTION_ADDRESS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_ADDRESS_ERROR, address, MODE_WRITE, fc);
	}
}
/// @param[in] pc : program counter read instruction
/// @param[in] address : address to jump or branch
/// @param[in] fc : function code
void MC68000BASE::has_address_error_for_stack_read(uint32_t pc, uint32_t address, uint32_t fc)
{
	if(address & 1) {
		write_signal(SIG_M68K_ADDRERR, 1, 1);
		DASM_PUSH_AERR2(address, fc, DEBUG_RM16(address, fc), false);
		show_error_message(pc, address);
		m_proc_mode = PROC_MODE_EXCEPTION_ADDRESS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_ADDRESS_ERROR, address, MODE_READ, fc);
	}
}

/// Exception for address error
void MC68000BASE::exception_address_error()
{
	uint32_t sr = init_exception(M68K_EXCEPTION_CATEGORY_ADDRESS_ERROR, M68K_EXCEPTION_ADDRESS_ERROR);

	/* If we were processing a bus error, address error, or reset,
	 * this is a catastrophic failure.
	 * Halt the CPU
	 */
	if(m_run_mode == RUN_MODE_BERR_AERR_RESET)
	{
//		read_data8(0x00ffff01);
		m_stopped |= STOP_LEVEL_INTER_HALT;
//		m_proc_mode = PROC_MODE_INTERNAL_HALT;
		m_signals |= SIG_MASK_M68K_INTER_HALT;
		write_signals(&outputs_halt, 0x80000000);

		term_exception();
		return;
	}
	m_run_mode = RUN_MODE_BERR_AERR_RESET;

	if (CPU_TYPE_IS_000())
	{
		/* Note: This is implemented for 68000 only! */
		stack_frame_buserr(sr);
	}
	else if (CPU_TYPE_IS_010())
	{
		/* only the 68010 throws this unique type-1000 frame */
		stack_frame_1000(m_ppc, sr, M68K_EXCEPTION_ADDRESS_ERROR);
	}
	else
	{
		stack_frame_1010(sr, M68K_EXCEPTION_ADDRESS_ERROR, m_ppc, m_aberr.m_address);
	}

	jump_vector(M68K_EXCEPTION_CATEGORY_ADDRESS_ERROR, M68K_EXCEPTION_ADDRESS_ERROR);

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_ADDRESS_ERROR]);

	term_exception();
}

/* ----------------------------- Bus Error -------------------------------- */

void MC68000BASE::has_bus_error_for_write(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_signals & SIG_MASK_M68K_BUSERRALL) {
		DASM_PUSH_BERR(address, value);
		m_proc_mode = PROC_MODE_EXCEPTION_BUS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_BUS_ERROR, address, MODE_WRITE, m_fc);
	}
}
void MC68000BASE::has_bus_error_for_read(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_signals & SIG_MASK_M68K_BUSERRALL) {
		DASM_PUSH_BERR(address, value);
		m_proc_mode = PROC_MODE_EXCEPTION_BUS_ERROR;
		throw MC68000ABERR(M68K_EXCEPTION_BUS_ERROR, address, MODE_READ, m_fc);
	}
}

/// Exception for bus error
void MC68000BASE::exception_bus_error()
{
//	SET_MMU_TMP_BUSERROR_FC(GET_MMU_TMP_FC());
//	SET_MMU_TMP_BUSERROR_RW(GET_MMU_TMP_RW());
//	SET_MMU_TMP_BUSERROR_SZ(GET_MMU_TMP_SZ());

	// Halt the cpu on berr when writing the stack frame.
	if (m_run_mode >= RUN_MODE_BERR_AERR_RESET_WSF)
	{
		m_stopped |= STOP_LEVEL_INTER_HALT;
//		m_proc_mode = PROC_MODE_INTERNAL_HALT;
		m_signals |= SIG_MASK_M68K_INTER_HALT;
		write_signals(&outputs_halt, 0x80000000);
		DASM_SET_REG(0, ACCUM_ICOUNT(), get_sr());

		term_exception();
		return;
	}

	uint32_t sr = init_exception(M68K_EXCEPTION_CATEGORY_BUS_ERROR, M68K_EXCEPTION_BUS_ERROR);

	m_run_mode = RUN_MODE_BERR_AERR_RESET_WSF;

	if (CPU_TYPE_IS_000())
	{
		/* Note: This is implemented for 68000 only! */
		stack_frame_buserr(sr);
	}
	else if (CPU_TYPE_IS_010())
	{
		/* only the 68010 throws this unique type-1000 frame */
		stack_frame_1000(m_ppc, sr, M68K_EXCEPTION_BUS_ERROR);
	}
#ifdef USE_MC68070
	else if (CPU_TYPE_IS_070())
	{
		/* only the 68070 throws this unique type-1111 frame */
		stack_frame_1111(m_ppc, sr, M68K_EXCEPTION_BUS_ERROR);
	}
#endif
	else if (m_aberr.m_address == m_ppc)
	{
		stack_frame_1010(sr, M68K_EXCEPTION_BUS_ERROR, m_ppc, m_aberr.m_address);
	}
	else
	{
		stack_frame_1011(sr, M68K_EXCEPTION_BUS_ERROR, m_ppc, m_aberr.m_address);
	}

	jump_vector(M68K_EXCEPTION_CATEGORY_BUS_ERROR, M68K_EXCEPTION_BUS_ERROR);
	m_run_mode = RUN_MODE_BERR_AERR_RESET;

#ifdef USE_MC68000MMU
//	m_mmu_tmp_buserror_occurred = 0;
	m_signals &= ~SIG_MASK_M68K_MMUBUSERR;
#endif

	/* Use up some clock cycles and undo the instruction's cycles */
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_BUS_ERROR]);

	term_exception();
}

/* ---------------------------- Interrupts -------------------------------- */

/// Check for interrupts
bool MC68000BASE::check_interrupts()
{
	uint32_t int_level = 0;
	uint32_t intaddr = 0;

	if(m_nmi_pending) {
		m_nmi_pending = false;
		int_level = 0x0700;
		m_proc_mode = (PROC_MODE_EXCEPTION_INTERRUPT | int_level);
	} else if(m_int_level > m_int_mask) {
		int_level = m_int_level;
		m_proc_mode = (PROC_MODE_EXCEPTION_INTERRUPT | int_level);
	}

	if ((m_proc_mode & 0xff) != PROC_MODE_EXCEPTION_INTERRUPT) {
		// no interrupt
		return true;
	}

	OUT_DEBUG_EX_INTR(_T("clk:%d MC68000 INT%d EXCEPTION cur:%04X msk:%04X PROC:%04X")
		, (int)get_current_clock()
		, int_level >> 8, m_int_level, m_int_mask, m_proc_mode);

	DASM_UPDATE_REG(get_sr());

	// read interrupt vector number from a device
	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	/* Turn off the stopped state */
	m_stopped &= ~STOP_LEVEL_STOP;
	m_signals &= ~SIG_MASK_M68K_STOPBYSW;

#if 0
	/* If we are halted, don't do anything */
	if(m_stopped) {
#ifdef _DEBUG
		if ((m_proc_mode & 0xff) == PROC_MODE_EXCEPTION_INTERRUPT) {
			OUT_DEBUG_EX_INTR(_T("clk:%d MC68000 INT%d BUT NOW STOPPED:%04X")
				, (int)get_current_clock()
				, int_level >> 8, m_stopped);
		}
#endif
		return true;
	}
#endif

	int_level >>= 8;

	// 68000 and 68010 assert UDS as well as LDS for IACK cycles but disregard D8-D15
	// 68008 definitely reads only one byte from CPU space, and the 68020 byte-sizes the request
//	if(CPU_TYPE_IS_EC020_PLUS() || m_cpu_type == CPU_TYPE_008) {
		intaddr = 0xfffffff1 | (int_level << 1);
		DASM_PUSH_INTVEC(intaddr, get_sr());
		m_int_vec_num = read_intr_8(intaddr);
		DASM_ADD_MEM2(intaddr, m_fc, (uint8_t)m_int_vec_num, false);
//	} else {
//		intaddr = 0xfffffff0 | (int_level << 1);
//		DASM_PUSH_INTVEC(intaddr, get_sr());
//		m_int_vec_num = read_intr_16(intaddr) & 0xff;
//		DASM_ADD_MEM2(intaddr, m_fc, (uint8_t)m_int_vec_num, false);
//	}
	if (m_signals & SIG_MASK_M68K_VPA_AVEC) {
		m_int_vec_num = M68K_INT_ACK_AUTOVECTOR;
#ifdef USE_MEM_REAL_MACHINE_CYCLE
		if(CPU_TYPE_IS_010_LESS()) {
			// When VPA signal is asserted, bus cycle is incleased until falling edge of E signal.
			SET_ICOUNT(6);
			int count = m_icount % 10;
			SET_ICOUNT(10 - count);
		}
#endif
	} else if (m_signals & SIG_MASK_M68K_BUSERR) {
		m_int_vec_num = M68K_INT_ACK_SPURIOUS;
	}

	/* Get the interrupt vector */
	if(m_int_vec_num == M68K_INT_ACK_AUTOVECTOR) {
		/* Use the autovectors.  This is the most commonly used implementation */
		m_int_vec_num = M68K_EXCEPTION_INTERRUPT_AUTOVECTOR + int_level;
	} else if (m_int_vec_num == M68K_INT_ACK_SPURIOUS) {
		/* Called if no devices respond to the interrupt acknowledge */
		m_int_vec_num = M68K_EXCEPTION_SPURIOUS_INTERRUPT;
	} else if (m_int_vec_num > 255) {
		m_int_vec_num &= 0xff;
	}

	DASM_SET_CYCLES(0, ACCUM_ICOUNT());

	return false;
}

/// Service an interrupt request and start exception processing
/// @param [in] int_level : level << 8 : 0x100 - 0x700
void MC68000BASE::exception_interrupt(uint32_t int_level)
{
//	uint32_t vector = 0;
	uint32_t intaddr = 0;
	uint32_t sr;
	uint32_t new_pc;
//	int vector_cycles = 0;

#if 0
	int_level >>= 8;

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	/* Turn off the stopped state */
	m_stopped &= ~STOP_LEVEL_STOP;
	m_signals &= ~SIG_MASK_M68K_STOPBYSW;

	/* If we are halted, don't do anything */
	if(m_stopped) return; 

	// 68000 and 68010 assert UDS as well as LDS for IACK cycles but disregard D8-D15
	// 68008 definitely reads only one byte from CPU space, and the 68020 byte-sizes the request
	if(CPU_TYPE_IS_EC020_PLUS() || m_cpu_type == CPU_TYPE_008) {
		intaddr = 0xfffffff1 | (int_level << 1);
		DASM_PUSH_INTVEC(intaddr);
		vector = read_intr_8(intaddr);
		DASM_ADD_MEM2(intaddr, m_fc, (uint8_t)vector, false);
	} else {
		intaddr = 0xfffffff0 | (int_level << 1);
		DASM_PUSH_INTVEC(intaddr);
		vector = read_intr_16(intaddr) & 0xff;
		DASM_ADD_MEM2(intaddr, m_fc, (uint8_t)vector, false);
	}
	if (m_signals & SIG_MASK_M68K_VPA_AVEC) {
		vector = M68K_INT_ACK_AUTOVECTOR;
	} else if (m_signals & SIG_MASK_M68K_BUSERR) {
		vector = M68K_INT_ACK_SPURIOUS;
	}

	/* Get the interrupt vector */
	if(vector == M68K_INT_ACK_AUTOVECTOR) {
		/* Use the autovectors.  This is the most commonly used implementation */
		vector = M68K_EXCEPTION_INTERRUPT_AUTOVECTOR+int_level;
	} else if (vector == M68K_INT_ACK_SPURIOUS) {
		/* Called if no devices respond to the interrupt acknowledge */
		vector = M68K_EXCEPTION_SPURIOUS_INTERRUPT;
	} else if (vector > 255) {
		vector &= 0xff;
	}
#endif
	/* Start exception processing */
	sr = init_exception((int_level & SIG_MASK_M68K_IPLALL) | M68K_EXCEPTION_CATEGORY_INTERRUPT, m_int_vec_num);

	/* Set the interrupt mask to the level of the one being serviced */
//	m_int_mask = int_level << 8;
	m_int_mask = int_level;

//	vector_cycles = ACCUM_ICOUNT();
//	DASM_SET_REG(0, vector_cycles, get_sr());

	/* Get the new PC */
	intaddr = (m_int_vec_num << 2);
#ifdef USE_MC68000VBR
	intaddr += m_vbr;
#endif
	DASM_PUSH_VECTOR(M68K_EXCEPTION_CATEGORY_INTERRUPT, m_int_vec_num, intaddr);
	new_pc = read_32(intaddr);

	/* If vector is uninitialized, call the uninitialized interrupt vector */
	if(new_pc == 0) {
		intaddr = (M68K_EXCEPTION_UNINITIALIZED_INTERRUPT<<2);
#ifdef USE_MC68000VBR
		intaddr += m_vbr;
#endif
		DASM_PUSH_VECTOR(M68K_EXCEPTION_CATEGORY_INTERRUPT, m_int_vec_num, intaddr);
		new_pc = read_32(intaddr);
	}

	/* Generate a stack frame */
	stack_frame_0000(REG_PC(), sr, m_int_vec_num);
	if(m_m_flag && CPU_TYPE_IS_EC020_PLUS())
	{
		/* Create throwaway frame */
		set_sm_flag(m_s_flag);  /* clear M */
		sr |= 0x2000; /* Same as SR in master stack frame except S is forced high */
		stack_frame_0001(REG_PC(), sr, m_int_vec_num);
	}

	jump(new_pc);

	/* Defer cycle counting until later */
//	SET_ICOUNT(m_cyc_exception[m_int_vec_num] - vector_cycles);
	SET_ICOUNT(m_cyc_exception[M68K_EXCEPTION_CATEGORY_INTERRUPT]);

	term_exception();
}

/****************************************************************************/

void MC68000BASE::show_error_message(uint32_t pc, uint32_t address)
{
	if (FLG_SHOWMSG_ADDRERR) {
		OUT_ERRORLOG(LOG_ERROR,_T("Address error occured at $%06X"), pc);
	}
}

/****************************************************************************/

/* ------------------------- Effective Address ---------------------------- */

uint8_t MC68000BASE::read_ea_8(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return REG_D()[reg];
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			return read_8(ea);
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_8();
			return read_8(ea);
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_8();
			return read_8(ea);
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_8();
			return read_8(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_8();
			return read_8(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					uint32_t ea = (uint32_t)OPER_I_16();
					return read_8(ea);
				}
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					return read_8(ea);
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_8();
					return read_8(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					uint32_t ea =  EA_PCIX_8();
					return read_8(ea);
				}
				case 4:     // #<data>
				{
					return  OPER_I_8();
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_8: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_8: unhandled mode %d, reg %d at %08X"
						, mode, reg, REG_PC());
	}

	return 0;
}

uint16_t MC68000BASE::read_ea_16(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return (uint16_t)(REG_D()[reg]);
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			return read_16(ea);
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_16();
			return read_16(ea);
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_16();
			return read_16(ea);
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_16();
			return read_16(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_16();
			return read_16(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					uint32_t ea = (uint32_t)OPER_I_16();
					return read_16(ea);
				}
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					return read_16(ea);
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_16();
					return read_16(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					uint32_t ea =  EA_PCIX_16();
					return read_16(ea);
				}
				case 4:     // #<data>
				{
					return OPER_I_16();
				}

				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_16: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_16: unhandled mode %d, reg %d at %08X"
						, mode, reg, REG_PC());
	}

	return 0;
}

uint32_t MC68000BASE::read_ea_32(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return REG_D()[reg];
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			return read_32(ea);
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_32();
			return read_32(ea);
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_32();
			return read_32(ea);
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_32();
			return read_32(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			return read_32(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					uint32_t ea = (uint32_t)OPER_I_16();
					return read_32(ea);
				}
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					return read_32(ea);
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_32();
					return read_32(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					uint32_t ea =  EA_PCIX_32();
					return read_32(ea);
				}
				case 4:     // #<data>
				{
					return  OPER_I_32();
				}
				default:    logging->out_logf(LOG_ERROR, "M68000BASE: READ_EA_32: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "M68000BASE: READ_EA_32: unhandled mode %d, reg %d at %08X"
						, mode, reg, REG_PC());
	}
	return 0;
}

uint64_t MC68000BASE::read_ea_64(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	uint32_t h1, h2;

	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			h1 = read_32(ea+0);
			h2 = read_32(ea+4);
			return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
		}
		case 3:     // (An)+
		{
			uint32_t ea = REG_A()[reg];
			REG_A()[reg] += 8;
			h1 = read_32(ea+0);
			h2 = read_32(ea+4);
			return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
		}
		case 4:     // -(An)
		{
			uint32_t ea = REG_A()[reg]-8;
			REG_A()[reg] -= 8;
			h1 = read_32(ea+0);
			h2 = read_32(ea+4);
			return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_32();
			h1 = read_32(ea+0);
			h2 = read_32(ea+4);
			return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			h1 = read_32(ea+0);
			h2 = read_32(ea+4);
			return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					return (uint64_t)(read_32(ea)) << 32 | (uint64_t)(read_32(ea+4));
				}
				case 3:     // (PC) + (Xn) + d8
				{
					uint32_t ea =  EA_PCIX_32();
					h1 = read_32(ea+0);
					h2 = read_32(ea+4);
					return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
				}
				case 4:     // #<data>
				{
					h1 = OPER_I_32();
					h2 = OPER_I_32();
					return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_32();
					h1 = read_32(ea+0);
					h2 = read_32(ea+4);
					return  (uint64_t)(h1) << 32 | (uint64_t)(h2);
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_64: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: READ_EA_64: unhandled mode %d, reg %d at %08X"
						, mode, reg, REG_PC());
	}

	return 0;
}

void MC68000BASE::write_ea_8(int ea, uint8_t data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			write_8(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_8();
			write_8(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_8();
			write_8(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_8();
			write_8(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_8();
			write_8(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).B
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					write_8(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_16();
					write_8(ea, data);
					break;
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_8: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_8: unhandled mode %d, reg %d, data %08X at %08X"
						, mode, reg, data, REG_PC());
	}
}

void MC68000BASE::write_ea_16(int ea, uint16_t data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			write_16(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_16();
			write_16(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_16();
			write_16(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_16();
			write_16(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_16();
			write_16(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).W
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					write_16(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_16();
					write_16(ea, data);
					break;
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_16: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_16: unhandled mode %d, reg %d, data %08X at %08X"
						, mode, reg, data, REG_PC());
	}
}

void MC68000BASE::write_ea_32(int ea, uint32_t data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 1:     // An
		{
			REG_A()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			write_32(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_32();
			write_32(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			uint32_t ea = EA_AY_PD_32();
			write_32(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_32();
			write_32(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			write_32(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					write_32(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_32();
					write_32(ea, data);
					break;
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_32: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_32: unhandled mode %d, reg %d, data %08X at %08X"
						, mode, reg, data, REG_PC());
	}
}

void MC68000BASE::write_ea_64(int ea, uint64_t data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			write_32(ea, (uint32_t)(data >> 32));
			write_32(ea+4, (uint32_t)(data));
			break;
		}
		case 3:     // (An)+
		{
			uint32_t ea = REG_A()[reg];
			REG_A()[reg] += 8;
			write_32(ea+0, (uint32_t)(data >> 32));
			write_32(ea+4, (uint32_t)(data));
			break;
		}
		case 4:     // -(An)
		{
			uint32_t ea;
			REG_A()[reg] -= 8;
			ea = REG_A()[reg];
			write_32(ea+0, (uint32_t)(data >> 32));
			write_32(ea+4, (uint32_t)(data));
			break;
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_32();
			write_32(ea+0, (uint32_t)(data >> 32));
			write_32(ea+4, (uint32_t)(data));
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			write_32(ea+0, (uint32_t)(data >> 32));
			write_32(ea+4, (uint32_t)(data));
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					write_32(ea+0, (uint32_t)(data >> 32));
					write_32(ea+4, (uint32_t)(data));
					break;
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_32();
					write_32(ea+0, (uint32_t)(data >> 32));
					write_32(ea+4, (uint32_t)(data));
					break;
				}
				default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_64: unhandled mode %d, reg %d at %08X"
								, mode, reg, REG_PC());
			}
			break;
		}
		default:    logging->out_logf(LOG_ERROR, "MC68000BASE: WRITE_EA_64: unhandled mode %d, reg %d, data %08X%08X at %08X"
						, mode, reg, (uint32_t)(data >> 32), (uint32_t)(data), REG_PC());
	}
}

void MC68000BASE::set_one(uint16_t opcode, uint16_t state, const opcode_handler_struct &s)
{
	for(int i=0; i<NUM_CPU_TYPES; i++) {
#ifndef USE_MEM_REAL_MACHINE_CYCLE
		if(s.cycles[i].a != 0xff) {
			m68ki_cycles[i][opcode] = s.cycles[i].a;
			m68ki_instruction_state_table[i][opcode] = state;
		}
#else
		if(s.cycles[i].n != 0xff) {
			m68ki_cycles[i][opcode] = s.cycles[i].n;
			m68ki_instruction_state_table[i][opcode] = state;
		}
#endif
	}
}

void MC68000BASE::build_opcode_table()
{
	if (m_emulation_initialized) return;

	for(int i = 0; i < 0x10000; i++)
	{
		/* default to illegal */
		for(int k=0;k<NUM_CPU_TYPES;k++)
		{
			m68ki_instruction_state_table[k][i] = m68k_state_illegal;
			m68ki_cycles[k][i] = 0;
		}
	}

	for(uint16_t state = 0; m68k_opcode_table[state].mask; state++)
	{
		const opcode_handler_struct &os = m68k_opcode_table[state];
		uint16_t mask = os.mask;
		uint16_t extraval = 0;
		do {
			set_one(os.match | extraval, state, os);
			extraval = ((extraval | mask) + 1) & ~mask;
		} while(extraval);
	}

	m_emulation_initialized = true;
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68000
MC68000::MC68000(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68000BASE(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68000");
}

void MC68000::initialize()
{
	MC68000BASE::initialize();

	m_cpu_type         = CPU_TYPE_000;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init16(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[0];
	m_cyc_instruction  = m68ki_cycles[0];
	m_cyc_exception    = m68ki_exception_cycle_table[0];
#ifndef USE_MEM_REAL_MACHINE_CYCLE
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 132;
#else
	m_cyc_bcc_notake_b = 2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 128;	// 132 - (1read * 4)
#endif
}

/* ------------------------------------------------------------------------ */

/// Read data from memory
///
///	@note The data bus width is 16bits.
///	So a read/write data b15-b8 always exists at even address, and b7-b0 does at odd. 
uint32_t MC68000::RM8(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data8w(address, &m_icount);
	SET_MEM_ICOUNT_000();
	if (WORD_ALIGNED(address)) value >>= 8;
	return value & 0xff;
}
/// Write data to memory
///
///	@note The data bus width is 16bits.
///	So a read/write data b15-b8 always exists at even address, and b7-b0 does at odd. 
void MC68000::WM8(uint32_t address, uint32_t fc, uint32_t value)
{
	if (WORD_ALIGNED(address)) value <<= 8;
	d_mem[fc & 7]->write_data8w(address, value, &m_icount);
	SET_MEM_ICOUNT_000();
}
uint32_t MC68000::RM16(uint32_t address, uint32_t fc)
{
	uint32_t value;
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		value = d_mem[fc & 7]->read_data16w(address, &m_icount);
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		value = (d_mem[fc & 7]->read_data8w(address, &m_icount) << 8);
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data8w(address + 1, &m_icount) >> 8);
		SET_MEM_ICOUNT_000();
	}
	return value & 0xffff;
}
void MC68000::WM16(uint32_t address, uint32_t fc, uint32_t value)
{
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		d_mem[fc & 7]->write_data16w(address, value, &m_icount);
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		d_mem[fc & 7]->write_data8w(address, (value >> 8), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data8w(address + 1, (value << 8), &m_icount);
		SET_MEM_ICOUNT_000();
	}
}
uint32_t MC68000::RM32(uint32_t address, uint32_t fc)
{
	uint32_t value;
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		value = (d_mem[fc & 7]->read_data16w(address, &m_icount) << 16);
		SET_MEM_ICOUNT_000();
		value |= d_mem[fc & 7]->read_data16w(address + 2, &m_icount); 
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		value = (d_mem[fc & 7]->read_data8w(address, &m_icount) << 24);
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data16w(address + 1, &m_icount) << 8);
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data8w(address + 3, &m_icount) >> 8);
		SET_MEM_ICOUNT_000();
	}
	return value;
}
void MC68000::WM32(uint32_t address, uint32_t fc, uint32_t value)
{
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		d_mem[fc & 7]->write_data16w(address, (value >> 16), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data16w(address + 2, value, &m_icount);
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		d_mem[fc & 7]->write_data8w(address, (value >> 24), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data16w(address + 1, (value >> 8), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data8w(address + 3, (value << 8), &m_icount);
		SET_MEM_ICOUNT_000();
	}
}

uint32_t MC68000::RM32REV(uint32_t address, uint32_t fc)
{
	uint32_t value;
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		value = d_mem[fc & 7]->read_data16w(address + 2, &m_icount); 
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data16w(address, &m_icount) << 16); 
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		value = (d_mem[fc & 7]->read_data8w(address + 3, &m_icount) >> 8);
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data16w(address + 1, &m_icount) << 8); 
		SET_MEM_ICOUNT_000();
		value |= (d_mem[fc & 7]->read_data8w(address, &m_icount) << 24);
		SET_MEM_ICOUNT_000();
	}
	return value;
}
void MC68000::WM32REV(uint32_t address, uint32_t fc, uint32_t value)
{
	// check alignment / word (2bytes) boundary 
	if (WORD_ALIGNED(address)) {
		// even
		d_mem[fc & 7]->write_data16w(address + 2, value, &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data16w(address, (value >> 16), &m_icount);
		SET_MEM_ICOUNT_000();
	} else {
		// odd
		d_mem[fc & 7]->write_data8w(address + 3, (value << 8), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data16w(address + 1, (value >> 8), &m_icount);
		SET_MEM_ICOUNT_000();
		d_mem[fc & 7]->write_data8w(address, (value >> 24), &m_icount);
		SET_MEM_ICOUNT_000();
	}
}

uint32_t MC68000::RM_PROG_16(uint32_t address, uint32_t fc)
{
	// no need alignment check
	uint32_t value = d_mem[fc & 7]->read_data16w(address, &m_icount);
	SET_MEM_ICOUNT_000();
	return value & 0xffff;
}
uint32_t MC68000::RM_PROG_32(uint32_t address, uint32_t fc)
{
	// no need alignment check
	uint32_t value = d_mem[fc & 7]->read_data16w(address, &m_icount); 
	SET_MEM_ICOUNT_000();
	value <<= 16;
	value |= d_mem[fc & 7]->read_data16w(address + 2, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value;
}
#endif /* USE_MC68000 */

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68008
MC68008::MC68008(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68000BASE(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68008");
}

void MC68008::initialize()
{
	MC68000BASE::initialize();

	m_cpu_type         = CPU_TYPE_008;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init8(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[0];
	m_cyc_instruction  = m68ki_cycles[0];
	m_cyc_exception    = m68ki_exception_cycle_table[0];
#ifndef USE_MEM_REAL_MACHINE_CYCLE
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 136;
#else
	m_cyc_bcc_notake_b = 2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 128;	// 136 - (2read * 4) 
#endif
}

/* ------------------------------------------------------------------------ */

/// Read data from memory
uint32_t MC68008::RM8(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data8w(address, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value & 0xff;
}
/// Write data to memory
void MC68008::WM8(uint32_t address, uint32_t fc, uint32_t value)
{
	d_mem[fc & 7]->write_data8w(address, value, &m_icount);
	SET_MEM_ICOUNT_000();
}
uint32_t MC68008::RM16(uint32_t address, uint32_t fc)
{
	uint32_t value = (d_mem[fc & 7]->read_data8w(address, &m_icount) << 8);
	SET_MEM_ICOUNT_000();
	value |= d_mem[fc & 7]->read_data8w(address + 1, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value & 0xffff;
}
void MC68008::WM16(uint32_t address, uint32_t fc, uint32_t value)
{
	d_mem[fc & 7]->write_data8w(address, (value >> 8) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 1, value & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
}
uint32_t MC68008::RM32(uint32_t address, uint32_t fc)
{
	uint32_t value = (d_mem[fc & 7]->read_data8w(address, &m_icount) << 24);
	SET_MEM_ICOUNT_000();
	value |= (d_mem[fc & 7]->read_data8w(address + 1, &m_icount) << 16); 
	SET_MEM_ICOUNT_000();
	value |= (d_mem[fc & 7]->read_data8w(address + 2, &m_icount) << 8); 
	SET_MEM_ICOUNT_000();
	value |= d_mem[fc & 7]->read_data8w(address + 3, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value;
}
void MC68008::WM32(uint32_t address, uint32_t fc, uint32_t value)
{
	d_mem[fc & 7]->write_data8w(address, (value >> 24) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 1, (value >> 16) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 2, (value >> 8) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 3, value & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
}

uint32_t MC68008::RM32REV(uint32_t address, uint32_t fc)
{
	uint32_t value = (d_mem[fc & 7]->read_data8w(address + 2, &m_icount) << 8);
	SET_MEM_ICOUNT_000();
	value |= d_mem[fc & 7]->read_data8w(address + 3, &m_icount); 
	SET_MEM_ICOUNT_000();
	value |= (d_mem[fc & 7]->read_data8w(address, &m_icount) << 24); 
	SET_MEM_ICOUNT_000();
	value |= (d_mem[fc & 7]->read_data8w(address + 1, &m_icount) << 16); 
	SET_MEM_ICOUNT_000();
	return value;
}
void MC68008::WM32REV(uint32_t address, uint32_t fc, uint32_t value)
{
	d_mem[fc & 7]->write_data8w(address + 2, (value >> 8) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 3, value & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address, (value >> 24) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
	d_mem[fc & 7]->write_data8w(address + 1, (value >> 16) & 0xff, &m_icount);
	SET_MEM_ICOUNT_000();
}

uint32_t MC68008::RM_PROG_16(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data8w(address, &m_icount);
	SET_MEM_ICOUNT_000();
	value <<= 8;
	value |= d_mem[fc & 7]->read_data8w(address + 1, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value & 0xffff;
}
uint32_t MC68008::RM_PROG_32(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data8w(address, &m_icount); 
	SET_MEM_ICOUNT_000();
	value <<= 8;
	value |= d_mem[fc & 7]->read_data8w(address + 1, &m_icount); 
	SET_MEM_ICOUNT_000();
	value <<= 8;
	value |= d_mem[fc & 7]->read_data8w(address + 2, &m_icount); 
	SET_MEM_ICOUNT_000();
	value <<= 8;
	value |= d_mem[fc & 7]->read_data8w(address + 3, &m_icount); 
	SET_MEM_ICOUNT_000();
	return value;
}
#endif /* USE_MC68008 */

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68010
MC68010::MC68010(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68000(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68010");
}

void MC68010::initialize()
{
	MC68000BASE::initialize();

	m_cpu_type         = CPU_TYPE_010;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init16(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[2];
	m_cyc_instruction  = m68ki_cycles[2];
	m_cyc_exception    = m68ki_exception_cycle_table[2];
#ifndef USE_MEM_REAL_MACHINE_CYCLE
	m_cyc_bcc_notake_b = -4;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 6;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 130;
#else
	m_cyc_bcc_notake_b = 0;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 6;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 126;
#endif
}
#endif /* USE_MC68010 */

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

#if defined(USE_MC68020) || defined(USE_MC68000MMU)
MC68020::MC68020(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68000BASE(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68020");
}

void MC68020::initialize()
{
	MC68000BASE::initialize();

	m_cpu_type         = CPU_TYPE_020;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[3];
	m_cyc_instruction  = m68ki_cycles[3];
	m_cyc_exception    = m68ki_exception_cycle_table[3];
#ifndef USE_MEM_REAL_MACHINE_CYCLE
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
#else
	// cycles are incorrect
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
#endif
}

/* ------------------------------------------------------------------------ */

/// Read data from memory
uint32_t MC68020::RM8(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data8w(address, &m_icount); 
	SET_MEM_ICOUNT_020();
	return value & 0xff;
}
/// Write data to memory
void MC68020::WM8(uint32_t address, uint32_t fc, uint32_t value)
{
	d_mem[fc & 7]->write_data8w(address, value, &m_icount);
	SET_MEM_ICOUNT_020();
}
uint32_t MC68020::RM16(uint32_t address, uint32_t fc)
{
	uint32_t value;
	// check alignment / dword (4bytes) boundary 
	if ((address & 3) != 3) {
		value = d_mem[fc & 7]->read_data16w(address, &m_icount); 
		SET_MEM_ICOUNT_020();
	} else {
		// over boundary
		value = ((d_mem[fc & 7]->read_data8w(address, &m_icount) & 0xff) << 8);
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data8w(address + 1, &m_icount) & 0xff); 
		SET_MEM_ICOUNT_020();
	}
	return value & 0xffff;
}
void MC68020::WM16(uint32_t address, uint32_t fc, uint32_t value)
{
	// check alignment / dword (4bytes) boundary 
	if ((address & 3) != 3) {
		d_mem[fc & 7]->write_data16w(address, value, &m_icount);
		SET_MEM_ICOUNT_020();
	} else {
		// over boundary
		d_mem[fc & 7]->write_data8w(address, (value >> 8) & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data8w(address + 1, value & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
	}
}
uint32_t MC68020::RM32(uint32_t address, uint32_t fc)
{
	uint32_t value;
	switch(address & 3) {
	case 0:
		value = d_mem[fc & 7]->read_data32w(address, &m_icount); 
		SET_MEM_ICOUNT_020();
		break;
	case 1:
		value = ((d_mem[fc & 7]->read_data24w(address, &m_icount) & 0xffffff) << 8); 
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data8w(address + 3, &m_icount) & 0xff); 
		SET_MEM_ICOUNT_020();
		break;
	case 2:
		value = ((d_mem[fc & 7]->read_data16w(address, &m_icount) & 0xffff) << 16);
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data16w(address + 2, &m_icount) & 0xffff); 
		SET_MEM_ICOUNT_020();
		break;
	case 3:
		value = ((d_mem[fc & 7]->read_data8w(address, &m_icount) & 0xff) << 24);
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data24w(address + 1, &m_icount) & 0xffffff); 
		SET_MEM_ICOUNT_020();
		break;
	}
	return value;
}
void MC68020::WM32(uint32_t address, uint32_t fc, uint32_t value)
{
	switch(address & 3) {
	case 0:
		d_mem[fc & 7]->write_data32w(address, value, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 1:
		d_mem[fc & 7]->write_data24w(address, (value >> 8) & 0xffffff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data8w(address + 3, value & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 2:
		d_mem[fc & 7]->write_data16w(address, (value >> 16) & 0xffff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data16w(address + 2, value & 0xffff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 3:
		d_mem[fc & 7]->write_data8w(address, (value >> 24) & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data24w(address + 1, value & 0xffffff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	}
}

uint32_t MC68020::RM32REV(uint32_t address, uint32_t fc)
{
	uint32_t value;
	switch(address & 3) {
	case 0:
		value = d_mem[fc & 7]->read_data32w(address, &m_icount); 
		SET_MEM_ICOUNT_020();
		break;
	case 1:
		value = ((d_mem[fc & 7]->read_data8w(address + 3, &m_icount) & 0xff) << 24); 
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data24w(address, &m_icount) & 0xffffff); 
		SET_MEM_ICOUNT_020();
		break;
	case 2:
		value = ((d_mem[fc & 7]->read_data16w(address + 2, &m_icount) & 0xffff) << 16); 
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data16w(address, &m_icount) & 0xffff); 
		SET_MEM_ICOUNT_020();
		break;
	case 3:
		value = ((d_mem[fc & 7]->read_data24w(address + 1, &m_icount) & 0xffffff) << 8); 
		SET_MEM_ICOUNT_020();
		value |= (d_mem[fc & 7]->read_data8w(address, &m_icount) & 0xff); 
		SET_MEM_ICOUNT_020();
		break;
	}
	return value;
}
void MC68020::WM32REV(uint32_t address, uint32_t fc, uint32_t value)
{
	switch(address & 3) {
	case 0:
		d_mem[fc & 7]->write_data32w(address, value, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 1:
		d_mem[fc & 7]->write_data8w(address + 3, value & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data24w(address, (value >> 8) & 0xffffff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 2:
		d_mem[fc & 7]->write_data16w(address + 2, value & 0xffff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data16w(address, (value >> 16) & 0xffff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	case 3:
		d_mem[fc & 7]->write_data24w(address + 1, value & 0xffffff, &m_icount);
		SET_MEM_ICOUNT_020();
		d_mem[fc & 7]->write_data8w(address, (value >> 24) & 0xff, &m_icount);
		SET_MEM_ICOUNT_020();
		break;
	}
}

uint32_t MC68020::RM_PROG_16(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data16w(address, &m_icount);
	SET_MEM_ICOUNT_020();
	return value & 0xffff;
}
uint32_t MC68020::RM_PROG_32(uint32_t address, uint32_t fc)
{
	uint32_t value = d_mem[fc & 7]->read_data32w(address, &m_icount);
	SET_MEM_ICOUNT_020();
	return value;
}

/** Indexed addressing modes are encoded as follows:
 *
 * Base instruction format:
 * F E D C B A 9 8 7 6 | 5 4 3 | 2 1 0
 * x x x x x x x x x x | 1 1 0 | BASE REGISTER      (An)
 *
 * Base instruction format for destination EA in move instructions:
 * F E D C | B A 9    | 8 7 6 | 5 4 3 2 1 0
 * x x x x | BASE REG | 1 1 0 | X X X X X X       (An)
 *
 * Brief extension format:
 *  F  |  E D C   |  B  |  A 9  | 8 | 7 6 5 4 3 2 1 0
 * D/A | REGISTER | W/L | SCALE | 0 |  DISPLACEMENT
 *
 * Full extension format:
 *  F     E D C      B     A 9    8   7    6    5 4       3   2 1 0
 * D/A | REGISTER | W/L | SCALE | 1 | BS | IS | BD SIZE | 0 | I/IS
 * BASE DISPLACEMENT (0, 16, 32 bit)                (bd)
 * OUTER DISPLACEMENT (0, 16, 32 bit)               (od)
 *
 * D/A:     0 = Dn, 1 = An                          (Xn)
 * W/L:     0 = W (sign extend), 1 = L              (.SIZE)
 * SCALE:   00=1, 01=2, 10=4, 11=8                  (*SCALE)
 * BS:      0=add base reg, 1=suppress base reg     (An suppressed)
 * IS:      0=add index, 1=suppress index           (Xn suppressed)
 * BD SIZE: 00=reserved, 01=NULL, 10=Word, 11=Long  (size of bd)
 *
 * IS I/IS Operation
 * 0  000  No Memory Indirect
 * 0  001  indir prex with null outer
 * 0  010  indir prex with word outer
 * 0  011  indir prex with long outer
 * 0  100  reserved
 * 0  101  indir postx with null outer
 * 0  110  indir postx with word outer
 * 0  111  indir postx with long outer
 * 1  000  no memory indirect
 * 1  001  mem indir with null outer
 * 1  010  mem indir with word outer
 * 1  011  mem indir with long outer
 * 1  100-111  reserved
 */
uint32_t MC68020::get_ea_ix(uint32_t An)
{
	/* An = base register */
	uint32_t extension = read_imm_16();
	uint32_t Xn = 0;                        /* Index register */
	uint32_t bd = 0;                        /* Base Displacement */
	uint32_t od = 0;                        /* Outer Displacement */

	/* Brief extension format */
	if(!BIT_8(extension))
	{
		/* Calculate index */
		Xn = REG_DA()[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		/* Add scale if proper CPU type */
		if(CPU_TYPE_IS_EC020_PLUS())
			Xn <<= (extension>>9) & 3;  /* SCALE */

		/* Add base register and displacement and return */
		return An + Xn + MAKE_INT_8(extension);
	}

	/* Full extension format */

	SET_ICOUNT(m68ki_ea_idx_cycle_table[extension & 0x3f]);

	/* Check if base register is present */
	if(BIT_7(extension))                /* BS */
		An = 0;                         /* An */

	/* Check if index is present */
	if(!BIT_6(extension))               /* IS */
	{
		Xn = REG_DA()[extension>>12];     /* Xn */
		if(!BIT_B(extension))           /* W/L */
			Xn = MAKE_INT_16(Xn);
		Xn <<= (extension>>9) & 3;      /* SCALE */
	}

	/* Check if base displacement is present */
	if(BIT_5(extension))                /* BD SIZE */
		bd = BIT_4(extension) ? read_imm_32() : MAKE_INT_16(read_imm_16());

	/* If no indirect action, we are done */
	if(!(extension&7))                  /* No Memory Indirect */
		return An + bd + Xn;

	/* Check if outer displacement is present */
	if(BIT_1(extension))                /* I/IS:  od */
		od = BIT_0(extension) ? read_imm_32() : MAKE_INT_16(read_imm_16());

	/* Postindex */
	if(BIT_2(extension))                /* I/IS:  0 = preindex, 1 = postindex */
		return read_32(An + bd) + Xn + od;

	/* Preindex */
	return read_32(An + bd + Xn) + od;
}
#endif /* defined(USE_MC68020) || defined(USE_MC68000MMU) */

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68000MMU
MC68020MMU::MC68020MMU(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68020(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68020MMU");
}

void MC68020MMU::initialize()
{
	MC68020::initialize();

	m_has_pmmu         = true;
}

/* ------------------------------------------------------------------------ */

/// Read data from memory
/// with translating logical address to physical address
///
/// @note throw exception when bus error occured
uint32_t MC68020MMU::RM8(uint32_t address, uint32_t fc)
{
	uint32_t value;
	if (m_pmmu_enabled) {
		address = pmmu_translate_addr(address, 1);
	}
	value = d_mem[fc & 7]->read_data8w(address, &m_icount);
	SET_MEM_ICOUNT_020();
	return value & 0xff;
}
/// Write data to memory
/// with translating logical address to physical address
///
/// @note throw exception when bus error occured
void MC68020MMU::WM8(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_pmmu_enabled) {
		address = pmmu_translate_addr(address, 0);
	}
	d_mem[fc & 7]->write_data8w(address, value, &m_icount);
	SET_MEM_ICOUNT_020();
}
uint32_t MC68020MMU::RM16(uint32_t address, uint32_t fc)
{
	uint32_t value;
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 1);
		if ((address & 0xff) != 0xff) {
			// not at page boundary; use default code
			value = MC68020::RM16(address, fc);
		} else {
			value = (d_mem[fc & 7]->read_data8w(address0, &m_icount) << 16);
			SET_MEM_ICOUNT_020();
			uint32_t address1 = pmmu_translate_addr(address + 1, 1);
			value |= d_mem[fc & 7]->read_data8w(address1, &m_icount);
			SET_MEM_ICOUNT_020();
		}
	} else {
		value = MC68020::RM16(address, fc);
	}
	return value & 0xffff;
}
void MC68020MMU::WM16(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 0);
		if ((address & 0xff) != 0xff) {
			// not at page boundary; use default code
			MC68020::WM16(address, fc, value);
		} else {
			d_mem[fc & 7]->write_data8w(address0, (value >> 8) & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
			uint32_t address1 = pmmu_translate_addr(address + 1, 1);
			d_mem[fc & 7]->write_data8w(address1, value & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
		}
	} else {
		MC68020::WM16(address, fc, value);
	}
}
uint32_t MC68020MMU::RM32(uint32_t address, uint32_t fc)
{
	uint32_t value;
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 1);
		switch(address & 0xff) {
		default:
			// not at page boundary; use default code
			value = MC68020::RM32(address, fc);
			break;
		case 0xff: {
			value = (d_mem[fc & 7]->read_data8w(address0, &m_icount) << 24);
			SET_MEM_ICOUNT_020();
			uint32_t address1 = pmmu_translate_addr(address + 1, 1);
			value |= d_mem[fc & 7]->read_data24w(address1, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfe: {
			value = (d_mem[fc & 7]->read_data16w(address0, &m_icount) << 16);
			SET_MEM_ICOUNT_020();
			uint32_t address2 = pmmu_translate_addr(address + 2, 1);
			value |= d_mem[fc & 7]->read_data16w(address2, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfd: {
			value = (d_mem[fc & 7]->read_data24w(address0, &m_icount) << 8);
			SET_MEM_ICOUNT_020();
			uint32_t address3 = pmmu_translate_addr(address + 3, 1);
			value |= d_mem[fc & 7]->read_data8w(address3, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		}
	} else {
		value = MC68020::RM32(address, fc);
	}
	return value;
}
void MC68020MMU::WM32(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 0);
		switch(address & 0xff) {
		default:
			// not at page boundary; use default code
			MC68020::WM32(address, fc, value);
			break;
		case 0xff: {
			d_mem[fc & 7]->write_data8w(address0, (value >> 24) & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
			uint32_t address1 = pmmu_translate_addr(address + 1, 0);
			d_mem[fc & 7]->write_data24w(address1, value & 0xffffff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfe: {
			d_mem[fc & 7]->write_data16w(address0, (value >> 16) & 0xffff, &m_icount);
			SET_MEM_ICOUNT_020();
			uint32_t address2 = pmmu_translate_addr(address + 2, 0);
			d_mem[fc & 7]->write_data16w(address2, value & 0xffff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfd: {
			d_mem[fc & 7]->write_data24w(address0, (value >> 8) & 0xffffff, &m_icount);
			SET_MEM_ICOUNT_020();
			uint32_t address3 = pmmu_translate_addr(address + 3, 0);
			d_mem[fc & 7]->write_data8w(address3, value & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		}
	} else {
		MC68020::WM32(address, fc, value);
	}
}

uint32_t MC68020MMU::RM32REV(uint32_t address, uint32_t fc)
{
	uint32_t value;
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 1);
		switch(address & 0xff) {
		default:
			// not at page boundary; use default code
			value = MC68020::RM32REV(address, fc);
			break;
		case 0xff: {
			uint32_t address1 = pmmu_translate_addr(address + 1, 1);
			value = d_mem[fc & 7]->read_data24w(address1, &m_icount);
			SET_MEM_ICOUNT_020();
			value |= (d_mem[fc & 7]->read_data8w(address0, &m_icount) << 24);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfe: {
			uint32_t address2 = pmmu_translate_addr(address + 2, 1);
			value |= d_mem[fc & 7]->read_data16w(address2, &m_icount);
			SET_MEM_ICOUNT_020();
			value = (d_mem[fc & 7]->read_data16w(address0, &m_icount) << 16);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfd: {
			uint32_t address3 = pmmu_translate_addr(address + 3, 1);
			value |= d_mem[fc & 7]->read_data8w(address3, &m_icount);
			SET_MEM_ICOUNT_020();
			value = (d_mem[fc & 7]->read_data24w(address0, &m_icount) << 8);
			SET_MEM_ICOUNT_020();
			} break;
		}
	} else {
		value = MC68020::RM32REV(address, fc);
	}
	return value;
}
void MC68020MMU::WM32REV(uint32_t address, uint32_t fc, uint32_t value)
{
	if (m_pmmu_enabled) {
		uint32_t address0 = pmmu_translate_addr(address, 0);
		switch(address & 0xff) {
		default:
			// not at page boundary; use default code
			MC68020::WM32REV(address, fc, value);
			break;
		case 0xff: {
			uint32_t address1 = pmmu_translate_addr(address + 1, 0);
			d_mem[fc & 7]->write_data24w(address1, value & 0xffffff, &m_icount);
			SET_MEM_ICOUNT_020();
			d_mem[fc & 7]->write_data8w(address0, (value >> 24) & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfe: {
			uint32_t address2 = pmmu_translate_addr(address + 2, 0);
			d_mem[fc & 7]->write_data16w(address2, value & 0xffff, &m_icount);
			SET_MEM_ICOUNT_020();
			d_mem[fc & 7]->write_data16w(address0, (value >> 16) & 0xffff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		case 0xfd: {
			uint32_t address3 = pmmu_translate_addr(address + 3, 0);
			d_mem[fc & 7]->write_data8w(address3, value & 0xff, &m_icount);
			SET_MEM_ICOUNT_020();
			d_mem[fc & 7]->write_data24w(address0, (value >> 8) & 0xffffff, &m_icount);
			SET_MEM_ICOUNT_020();
			} break;
		}
	} else {
		MC68020::WM32REV(address, fc, value);
	}
}

uint32_t MC68020MMU::RM_PROG_16(uint32_t address, uint32_t fc)
{
	uint32_t value = RM16(address, fc);
	return value & 0xffff;
}
uint32_t MC68020MMU::RM_PROG_32(uint32_t address, uint32_t fc)
{
	uint32_t value = RM32(address, fc);
	return value;
}
#endif /*  USE_MC68000MMU */

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68EC030
MC68EC030::MC68EC030(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68020(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68EC030");
}

void MC68EC030::initialize()
{
	MC68020::initialize();

	m_cpu_type         = CPU_TYPE_EC030;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[4];
	m_cyc_instruction  = m68ki_cycles[4];
	m_cyc_exception    = m68ki_exception_cycle_table[4];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
//	m_has_pmmu         = false; /* EC030 lacks the PMMU and is effectively a die-shrink 68020 */
#ifdef USE_MC68000FPU
	m_has_fpu          = true;
#endif

//	define_state();
}
#endif /* USE_MC68EC030 */

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_MC68000MMU
MC68030::MC68030(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: MC68020MMU(parent_vm, parent_emu, identifier)
{
	set_class_name("MC68030");
}

void MC68030::initialize()
{
	MC68020MMU::initialize();

	m_cpu_type         = CPU_TYPE_030;
	DASM_SET_CPU_TYPE(m_cpu_type);

//	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[4];
	m_cyc_instruction  = m68ki_cycles[4];
	m_cyc_exception    = m68ki_exception_cycle_table[4];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = true;
#ifdef USE_MC68000FPU
	m_has_fpu          = true;
#endif
}

#endif /* USE_MC68000MMU */

