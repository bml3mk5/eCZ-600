/** @file ym2151.cpp

	Skelton for retropc emulator

	@author : Takeda.Toshiya
	@date   : 2009.03.08-

	@note
	Modified by Sasaji at 2022.02.22

	@brief [ YM2151 ]
*/

#include "ym2151.h"
#ifdef USE_DEBUGGER
//#include "debugger.h"
#include "../utility.h"
#endif
#include "../fileio.h"
#include "../logging.h"

//#define OUT_DEBUG_REGW logging->out_debugf
#define OUT_DEBUG_REGW(...)
//#define OUT_DEBUG_STATUS logging->out_debugf
#define OUT_DEBUG_STATUS(...)

#ifdef SUPPORT_MAME_FM_DLL
// thanks PC8801MA改
#include "fmdll/fmdll.h"
static CFMDLL* fmdll = NULL;
static int chip_reference_counter = 0;
static bool dont_create_multiple_chips = false;
#endif

YM2151::YM2151(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name(_T("YM2151"));

	init_output_signals(&outputs_irq);
	init_output_signals(&outputs_ct1);
	init_output_signals(&outputs_ct2);
	base_decibel = 0;

//#ifdef USE_DEBUGGER
//	d_debugger = NULL;
//#endif
}

void YM2151::initialize()
{
	opm = new FM::OPM;
#ifdef SUPPORT_MAME_FM_DLL
	if(!fmdll) {
//		fmdll = new CFMDLL(_T("mame2151.dll"));
		fmdll = new CFMDLL(pConfig->mame2151_dll_path);
	}
	dllchip = NULL;
#endif
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;

#ifdef USE_DEBUGGER
//	if(d_debugger != NULL) {
//		d_debugger->set_device_name(_T("Debugger (YM2151 OPM)"));
//		d_debugger->set_context_mem(this);
//		d_debugger->set_context_io(vm->dummy);
//	}
#endif
}

void YM2151::release()
{
	delete opm;
	opm = NULL;

#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Release(dllchip);
		dllchip = NULL;
		chip_reference_counter--;
	}
	if(fmdll && !chip_reference_counter) {
		delete fmdll;
		fmdll = NULL;
	}
#endif
}

void YM2151::reset()
{
//	touch_sound();
	opm->Reset();
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Reset(dllchip);
	}
#endif
	memset(port_log, 0, sizeof(port_log));
	write_signals(&outputs_ct1, 0);
	write_signals(&outputs_ct2, 0);
	timer_event_id = -1;
	irq_prev = busy = false;
}

void YM2151::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		OUT_DEBUG_REGW(_T("clk:%lld OPM CH%02X %02X"), get_current_clock(), ch, data);

//		if(ch < 0x100) {
			update_count();
			this->set_reg(ch, data);
			if(ch == 0x14) {
				update_event();
			} else if (ch == 0x1b) {
				// CT1, CT2
				write_signals(&outputs_ct1, data & 0x40 ? 0xffffffff : 0);
				write_signals(&outputs_ct2, data & 0x80 ? 0xffffffff : 0);
			}
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
//		}
	} else {
		ch = data;
	}
}

uint32_t YM2151::read_io8(uint32_t addr)
{
	if(addr & 1) {
		update_count();
		update_interrupt();
		uint32_t sts = read_status();
		OUT_DEBUG_STATUS(_T("clk:%lld OPM STATUS:%02X"), get_current_clock(), sts);
		return sts;
	}
	return 0xff;
}

void YM2151::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_YM2151_MUTE:
		mute = ((data & mask) != 0);
		break;
	case SIG_CPU_RESET:
		now_reset = ((data & mask) != 0);
		reset();
		break;
	}
}

void YM2151::event_vline(int v, int clock)
{
	update_count();
	update_interrupt();
}

void YM2151::event_callback(int event_id, int error)
{
	update_count();
	update_interrupt();
	timer_event_id = -1;
	update_event();
}

void YM2151::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = (uint32_t)(clock_accum >> 20);
	if(count) {
		opm->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void YM2151::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}

	int count = opm->GetNextEvent();
	if(count > 0) {
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	}
}

void YM2151::update_interrupt()
{
	bool irq = opm->ReadIRQ();
	if(!irq_prev && irq) {
		write_signals(&outputs_irq, 0xffffffff);
	} else if(irq_prev && !irq) {
		write_signals(&outputs_irq, 0);
	}
	irq_prev = irq;
}

void YM2151::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		opm->Mix(buffer, cnt);
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->Mix(dllchip, buffer, cnt);
		}
#endif
	}
}

void YM2151::set_volume(int decibel_l, int decibel_r, bool vol_mute)
{
	if (vol_mute) {
		decibel_l = -192;
		decibel_r = -192;
	}
	opm->SetVolume(base_decibel + decibel_l, base_decibel + decibel_r);

#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetVolumeFM(dllchip, base_decibel + decibel_l);
	}
#endif
}

void YM2151::initialize_sound(int rate, int clock, int samples, int decibel)
{
	opm->Init(clock, rate, false);
	opm->SetVolume(decibel, decibel);
	base_decibel = decibel;

#ifdef SUPPORT_MAME_FM_DLL
	if(!dont_create_multiple_chips) {
		fmdll->Create((LPVOID*)&dllchip, clock, rate);
		if(dllchip) {
			chip_reference_counter++;

			fmdll->SetVolumeFM(dllchip, decibel);

			DWORD mask = 0;
			DWORD dwCaps = fmdll->GetCaps(dllchip);
			if((dwCaps & SUPPORT_MULTIPLE) != SUPPORT_MULTIPLE) {
				dont_create_multiple_chips = true;
			}
			if((dwCaps & SUPPORT_FM_A) == SUPPORT_FM_A) {
				mask = 0x07;
			}
			if((dwCaps & SUPPORT_FM_B) == SUPPORT_FM_B) {
				mask |= 0x38;
			}
			if((dwCaps & SUPPORT_FM_C) == SUPPORT_FM_C) {
				mask |= 0xc0;
			}
			opm->SetChannelMask(mask);
			fmdll->SetChannelMask(dllchip, ~mask);
		}
	}
#endif

	chip_clock = clock;
}

void YM2151::set_reg(uint32_t addr, uint32_t data)
{
//	touch_sound();
	opm->SetReg(addr, data);
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetReg(dllchip, addr, data);
	}
#endif
//	port_log[addr].written = true;
	port_log[addr].data = data;
}

uint32_t YM2151::read_status()
{
	uint32_t status;

	/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
	status = opm->ReadStatus() & ~0x80;

	if(busy) {
		// from PC-88 machine language master bible (XM8 version 1.00)
		// TODO: waiting time on busy
		if (get_passed_usec(clock_busy) < 2.13) {
			status |= 0x80;
		} else {
			busy = false;
		}
	}
	return status;
}

void YM2151::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

// ----------------------------------------------------------------------------

void YM2151::save_state(FILEIO *fio)
{
	struct vm_state_st vm_state;
	size_t state_size = 0;

	//
	vm_state_ident.version = Uint16_LE(1);

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	// reserved header
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	for(int i=0; i<0x100; i++) {
		vm_state.port_log[i].data = port_log[i].data;
	}
	vm_state.base_decibel = Int32_LE(base_decibel);

	vm_state.chip_clock = Int32_LE(chip_clock);

	vm_state.ch = ch;
	vm_state.irq_prev = irq_prev ? 1 : 0;
	vm_state.mute = mute ? 1 : 0;
	vm_state.busy = busy ? 1 : 0;

	vm_state.clock_prev = Uint64_LE(clock_prev);
	vm_state.clock_accum = Uint64_LE(clock_accum);
	vm_state.clock_const = Uint64_LE(clock_const);

	vm_state.clock_busy = Uint64_LE(clock_busy);

	vm_state.timer_event_id = Int32_LE(timer_event_id);


	fio->Fwrite(&vm_state, sizeof(vm_state), 1);

	opm->SaveState((void *)fio, &state_size);

	// set total size
	state_size += sizeof(vm_state);
	vm_state_ident.size = Uint32_LE((uint32_t)(sizeof(vm_state_ident) + state_size));

	// overwrite header
	fio->Fseek(-(long)(state_size + sizeof(vm_state_ident)), FILEIO::SEEKCUR);
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fio->Fseek((long)state_size, FILEIO::SEEKCUR);
}

bool YM2151::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	uint32_t i_size = Uint32_LE(vm_state_i.size);
	memset(&vm_state, 0, sizeof(vm_state));
	if (i_size >= (sizeof(vm_state) + sizeof(vm_state_i))) {
		fio->Fread(&vm_state, sizeof(vm_state), 1);
	}

	for(int i=0; i<0x100; i++) {
		port_log[i].data = vm_state.port_log[i].data;
	}
	base_decibel = Int32_LE(vm_state.base_decibel);

	chip_clock = Int32_LE(vm_state.chip_clock);

	ch = vm_state.ch;
	irq_prev = vm_state.irq_prev != 0;
	mute = vm_state.mute != 0;
	busy = vm_state.busy != 0;

	clock_prev = Uint64_LE(vm_state.clock_prev);
	clock_accum = Uint64_LE(vm_state.clock_accum);
	clock_const = Uint64_LE(vm_state.clock_const);

	clock_busy = Uint64_LE(vm_state.clock_busy);

	timer_event_id = Int32_LE(vm_state.timer_event_id);

	return opm->LoadState((void *)fio);
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
uint32_t YM2151::debug_read_io8(uint32_t addr)
{
	return read_io8(addr);
}

bool YM2151::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	 return false;
}

bool YM2151::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	 set_reg(reg_num, data);
	 return true;
}

void YM2151::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	int val = 0;
	buffer[0] = _T('\0');

	// for opm register
	// status
	{
		val = (int)read_status();
		UTILITY::sntprintf(buffer, buffer_len, _T(" STATUS:%02X\n"), val);
	}
	for(uint32_t ad=0; ad<256; ad++) {
		bool valid = false;
		bool lf = false;

		if (ad == 0x01 || ad == 0x08 || ad == 0x0f
		 || ad == 0x10 || ad == 0x11 || ad == 0x12 || ad == 0x14
		 || ad == 0x18 || ad == 0x19 || ad == 0x1b
		 || ad >= 0x20
		) {
			valid = true;
		}
		if ((ad & 0x0f) == 0x0f) {
			lf = true;
		}

		if (valid) {
			val = port_log[ad].data;
			UTILITY::sntprintf(buffer, buffer_len, _T(" %02X:%02X"), ad, val);
		}
		if (_tcslen(buffer) + 6 > buffer_len) break;
		if (lf) UTILITY::tcscat(buffer, buffer_len, _T("\n"));
	}
}
#endif

