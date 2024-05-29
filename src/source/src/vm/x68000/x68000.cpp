/** @file x68000.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ virtual machine ]
*/

#include "x68000.h"
#include "../../emu.h"
#include "../../config.h"
#include "../../emumsg.h"
#include "../device.h"
#include "../event.h"

#include "crtc.h"
#include "../ym2151.h"
#include "adpcm.h"
#include "../mc68000.h"

#include "board.h"
#include "mfp.h"
#include "dmac.h"
#include "display.h"
#include "sprite_bg.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "scc.h"
#include "sysport.h"
#include "comm.h"
#include "../i8255.h"
#ifdef USE_FD1
#include "fdc.h"
#include "floppy.h"
#endif
#ifdef USE_HD1
#include "sasi.h"
#include "scsi.h"
#endif
#include "rtc.h"
#ifdef USE_PRINTER
#include "printer.h"
#endif
//#include "keyrecord.h"
#ifdef USE_DEBUGGER
#include "../debugger.h"
#include "../mc68000dasm.h"
#endif
#include "../../depend.h"
#include "../../utility.h"
#include "../../version.h"
#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	//
	emu->set_parami(ParamIOPort, pConfig->io_port);
	emu->set_parami(ParamMainRamSizeNum, pConfig->main_ram_size_num);
#ifdef USE_HD1
	emu->set_parami(ParamSCSIType, pConfig->scsi_type);
	for(int drv = 0; drv < MAX_HARD_DISKS; drv++) {
		set_hard_disk_device_type(drv, pConfig->GetHardDiskDeviceType(drv));
	}
#endif

	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu, NULL);	// must be 1st device
	main_event = new EVENT(this, emu, NULL);	// must be 2nd device
	main_event->initialize();		// must be initialized first

	crtc = new CRTC(this, emu, NULL);
	opm = new YM2151(this, emu, NULL);
	adpcm = new ADPCM(this, emu, NULL);

	board = new BOARD(this, emu, NULL);
	mfp = new MFP(this, emu, NULL);
	dmac = new DMAC(this, emu, NULL);
	display = new DISPLAY(this, emu, NULL);
	sp_bg = new SPRITE_BG(this, emu, NULL);
	key = new KEYBOARD(this, emu, NULL);
	mouse = new MOUSE(this, emu, NULL);
	memory = new MEMORY(this, emu, NULL);
	scc = new SCC(this, emu, NULL);
	sysport = new SYSPORT(this, emu, NULL);
	comm[0] = new COMM(this, emu, NULL, 0);
	pio = new I8255(this, emu, NULL);
#ifdef USE_FD1
	fdc = new FDC(this, emu, NULL);
	fdd = new FLOPPY(this, emu, NULL);
#endif
#ifdef USE_HD1
	sasi = new SASI(this, emu, NULL);
	scsi = new SCSI(this, emu, NULL); 
#endif
	rtc = new RTC(this, emu, NULL);
#ifdef USE_PRINTER
	printer[0] = new PRINTER(this, emu, NULL, 0);
#endif

	//
#if defined(USE_MC68030)
	cpu = new MC68030(this, emu, NULL);
#elif defined(USE_MC68EC030)
	cpu = new MC68EC030(this, emu, NULL);
#elif defined(USE_MC68020MMU)
	cpu = new MC68020MMU(this, emu, NULL);
#elif defined(USE_MC68020)
	cpu = new MC68020(this, emu, NULL);
#elif defined(USE_MC68010)
	cpu = new MC68010(this, emu, NULL);
#elif defined(USE_MC68008)
	cpu = new MC68008(this, emu, NULL);
#else
	cpu = new MC68000(this, emu, NULL);
#endif

#ifdef USE_KEY_RECORD
	reckey = new KEYRECORD(emu);
#endif

	// set contexts
	main_event->set_context_cpu(cpu, CPU_CLOCKS);
	main_event->set_context_sound(opm);
	main_event->set_context_sound(adpcm);
#ifdef USE_FD1
	main_event->set_context_sound(fdd);
#endif
#ifdef USE_HD1
	main_event->set_context_sound(sasi);
	main_event->set_context_sound(scsi);
#endif
	main_event->set_context_display(display);
#ifdef USE_KEY_RECORD
	reckey->set_context_event(main_event);
#endif

	// crtc
	crtc->set_context_vdisp(mfp, MFP::SIG_TAI, 1);
	crtc->set_context_vdisp(mfp, MFP::SIG_GPIP, 0x10);
//	crtc->set_context_vsync(mfp, MFP::SIG_TAI, 1, 0xffffffff);	// negative
//	crtc->set_context_vsync(mfp, MFP::SIG_GPIP, 0x10, 0xffffffff);	// negative
	crtc->set_context_raster(mfp, MFP::SIG_GPIP, 0x40, 0xffffffff);	// negative
	crtc->set_context_hsync(mfp, MFP::SIG_GPIP, 0x80);
//	crtc->set_context_change_size(display, DISPLAY::SIG_DISPLAY_SIZE, 0xffff);
	crtc->set_context_display(display);
	crtc->set_vtram_ptr(memory->get_tvram_ptr(), memory->get_tvram_flag_ptr());
	crtc->set_gvram_ptr(memory->get_gvram_ptr() /*, memory->get_gvram_flag_ptr() */);
	crtc->set_vc_gr_pripage(display->get_gr_pripage());

	// display
	display->set_context_crtc(crtc);
	display->set_context_sprite_bg(sp_bg);
	display->set_cgrom_ptr(memory->get_rom_ptr());
	display->set_tvram_ptr(memory->get_tvram_ptr(), memory->get_tvram_flag_ptr());
	display->set_gvram_ptr(memory->get_gvram_ptr() /*, memory->get_gvram_flag_ptr() */);
	display->set_spram_ptr(memory->get_spram_ptr(), memory->get_pcg_flag_ptr() /*, memory->get_bg_flag_ptr(0), memory->get_bg_flag_ptr(1)*/);

	display->set_crtc_hz_ptr(crtc->get_hz_total_ptr(), crtc->get_hz_disp_ptr());
	display->set_crtc_vt_ptr(crtc->get_vt_total_ptr(), crtc->get_vt_disp_ptr());

//	display->set_crtc_ma_ra_ptr(crtc->get_ma_ptr(),crtc->get_ra_ptr());
//	display->set_crtc_max_ra_ptr(crtc->get_max_ra_ptr());
//	display->set_crtc_odd_line_ptr(crtc->get_video_odd_line_ptr());

	// sprite bg

	// keyboard
	key->set_context_cpu(cpu);
//	key->set_context_disp(display);
	key->set_context_board(board);
#ifdef USE_KEY_RECORD
	key->set_keyrecord(reckey);
//	reckey->set_context_keyboard(key);
#endif
	key->set_context_mfp(mfp);
	key->set_context_pio(pio);

	// mouse
	mouse->set_context_board(board);
#ifdef USE_KEY_RECORD
	mouse->set_keyrecord(reckey);
//	reckey->set_context_mouse(mouse);
#endif
	mouse->set_context_mfp(mfp);
	mouse->set_context_scc(scc);

	// memory
	memory->set_context_cpu(cpu);
	memory->set_context_mfp(mfp);
	memory->set_context_dmac(dmac);
	memory->set_context_crtc(crtc);
	memory->set_context_sprite_bg(sp_bg);
	memory->set_context_display(display);
	memory->set_context_opm(opm);
	memory->set_context_adpcm(adpcm);
	memory->set_context_comm(comm[0]);
#ifdef USE_FD1
	memory->set_context_fdc(fdc);
	memory->set_context_fdd(fdd);
#endif
#ifdef USE_HD1
	memory->set_context_sasi(sasi);
	memory->set_context_scsi(scsi);
#endif
	memory->set_context_pio(pio);
	memory->set_context_rtc(rtc);
	memory->set_context_board(board);
	memory->set_context_scc(scc);
	memory->set_context_sysport(sysport);
#ifdef USE_PRINTER
	memory->set_context_printer(printer[0]);
#endif
	memory->set_crtc_regs_ptr(crtc->get_regs());
	memory->set_fc_ptr(cpu->get_fc_ptr());

	// cpu bus
	cpu->set_cpu_clock(CPU_CLOCKS);
	cpu->set_context_mem(M68K_FC_SUPERVISOR_PROGRAM, memory);
	cpu->set_context_fc(board, SIG_M68K_FC, 0xffffffff);
	cpu->set_context_bg(dmac, DMAC::SIG_BG, 0xffffffff);
#ifdef USE_DEBUGGER
	DEBUGGER *debugger = new DEBUGGER(this, emu, NULL);
	cpu->set_context_debugger(debugger);
#endif

	// dmac
	dmac->set_context_memory(memory);
	// channel 0 is fdd
	dmac->set_context_device(0, memory);
	dmac->set_context_ack(0, fdc, FDC::SIG_DACK, 0x01);
	// channel 1 is hdd
	dmac->set_context_device(1, memory);
	dmac->set_context_ack(1, sasi, FDC::SIG_DACK, 0x01);
	// channel 2 is memory (or extended device)
	dmac->set_context_device(2, memory);
	// channel 3 is adpcm
	dmac->set_context_device(3, adpcm);
	dmac->set_context_ack(3, adpcm, ADPCM::SIG_DACK, 0x01);

	dmac->set_context_busreq(cpu, SIG_CPU_BUSREQ, 0x01);
	dmac->set_context_irq(board, SIG_CPU_IRQ, 0x08);	// IRQ to IPL3
#ifdef USE_DEBUGGER
	dmac->set_context_debugger(debugger);
#endif

	// scc
	scc->set_context_channel_a(comm[0]);
	scc->set_context_channel_b(mouse);
	scc->set_context_irq(board, SIG_CPU_IRQ, 0x20);	// IRQ to IPL5

	// sysport
	sysport->set_context_crtc(crtc);
	sysport->set_context_display(display);
	sysport->set_context_memory(memory);
	sysport->set_context_board(board);
	sysport->set_context_keyboard(key);

	// comm
	comm[0]->set_context_ctrl(scc);

	// mfp
	mfp->set_context_serial(key);
	mfp->set_context_irq(board, SIG_CPU_IRQ, 0x40);	// IRQ to IPL6
	mfp->set_context_timer(1, mfp, MFP::SIG_TXC, 0xffffffff);	// TimerB output

	// pio
	pio->set_context_port_c(adpcm, ADPCM::SIG_SEL_FREQUENCY, 0x0f);
	pio->set_context_port_c(key, KEYBOARD::SIG_JOY_ENABLE_SEND, 0xf0);

	// fm opm
	opm->set_context_irq(mfp, MFP::SIG_GPIP, 0x08, 0xffffffff);	// GPIP3 (negative logic)
	opm->set_context_ct1(fdd, SIG_FLOPPY_FORCE_READY, 1);
	opm->set_context_ct2(adpcm, ADPCM::SIG_SEL_CLOCK, 1);

	// adpcm
	adpcm->set_context_mck(dmac, DMAC::SIG_REQ_3, 1);

#ifdef USE_FD1
	// fdc for 5inch mini floppy
	fdc->set_context_irq(board, SIG_CPU_IRQ, 0x800002);	// IRQ to IPL1
	fdc->set_context_drq(dmac, DMAC::SIG_REQ_0, 1);
	fdc->set_context_hdu(fdd, SIG_FLOPPY_HEAD_SELECT, 0x04);
	fdc->set_context_fdd(fdd);

	// fdd
	fdd->set_context_irq(board, SIG_CPU_IRQ, 0x400002);	// IRQ to IPL1
//	fdd->set_context_drq(board, SIG_CPU_HALT, SIG_HALT_FD_MASK);
	fdd->set_context_fdc(fdc);
	fdd->set_context_board(board);
#endif

//#ifdef USE_HD1
//	// sasi hdd
//	sasi->set_context_irq(board, SIG_CPU_IRQ, 0x100002);	// IRQ to IPL1
//	sasi->set_context_drq(dmac, DMAC::SIG_REQ_1, 1);
//#endif

#ifdef USE_PRINTER
	printer[0]->set_context_busy(board, SIG_CPU_IRQ, 0x200002, 0xffffffff);	// IRQ to IPL1 (negative)
#endif

	rtc->set_context_alarm(mfp, MFP::SIG_GPIP, 0x01);	// GPIP0
//	rtc->set_context_pulse(mfp, MFP::SIG_GPIP, 0x20);	// GPIP5

	// software reset by cpu
	cpu->set_context_reset(memory, SIG_CPU_RESET, 1);
	cpu->set_context_reset(key, SIG_CPU_RESET, 1);
	cpu->set_context_reset(dmac, SIG_CPU_RESET, 1);
	cpu->set_context_reset(mfp, SIG_CPU_RESET, 1);
	cpu->set_context_reset(scc, SIG_CPU_RESET, 1);
	cpu->set_context_reset(opm, SIG_CPU_RESET, 1);
	cpu->set_context_reset(adpcm, SIG_CPU_RESET, 1);
#ifdef USE_FD1
	cpu->set_context_reset(fdd, SIG_CPU_RESET, 1);
#endif
#ifdef USE_HD1
	cpu->set_context_reset(sasi, SIG_CPU_RESET, 1);
	cpu->set_context_reset(scsi, SIG_CPU_RESET, 1);
#endif
	cpu->set_context_reset(pio, SIG_CPU_RESET, 1);

	// main board
	// reset signal
	// send reset to memory at first
	board->set_context_reset(memory, SIG_CPU_RESET, 1);
	board->set_context_reset(key, SIG_CPU_RESET, 1);
	board->set_context_reset(dmac, SIG_CPU_RESET, 1);
	board->set_context_reset(mfp, SIG_CPU_RESET, 1);
	board->set_context_reset(scc, SIG_CPU_RESET, 1);
	board->set_context_reset(opm, SIG_CPU_RESET, 1);
	board->set_context_reset(adpcm, SIG_CPU_RESET, 1);
#ifdef USE_FD1
	board->set_context_reset(fdd, SIG_CPU_RESET, 1);
#endif
#ifdef USE_HD1
	board->set_context_reset(sasi, SIG_CPU_RESET, 1);
	board->set_context_reset(scsi, SIG_CPU_RESET, 1);
#endif
	board->set_context_reset(pio, SIG_CPU_RESET, 1);
	board->set_context_reset(rtc, SIG_CPU_RESET, 1);

	// send reset to cpu at last
	board->set_context_reset(cpu, SIG_CPU_RESET, 1);

	// power signal
	board->set_context_power(mfp, MFP::SIG_GPIP, 0x04, 0xffffffff);	// GPIP2 (negative)

	// irq signal
	board->set_context_irq(cpu, SIG_CPU_IRQ, 0xffffffff);
	// halt signal
	board->set_context_halt(cpu, SIG_CPU_HALT, 0xffffffff);
	// iack signal
//	board->set_context_iack(6, mfp, MFP::SIG_IACK, 0xffffffff);
//	board->set_context_iack(3, dmac, DMAC::SIG_IACK, 0xffffffff);
//	board->set_context_iack(5, scc, SCC::SIG_IACK, 0xffffffff);
//	board->set_context_iack(2, scsi, SCSI::SIG_IACK, 0xffffffff);

	board->set_context_cpu(cpu);
	board->set_context_mfp(mfp);

	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(device->get_id() != main_event->get_id()) {
			device->initialize();
		}
	}
}

VM::~VM()
{
	// delete all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->release();
	}
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->get_next_device();
		delete device;
		device = next_device;
	}
#ifdef USE_KEY_RECORD
	delete reckey;
#endif
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(device->get_id() == id) {
			return device;
		}
	}
	return NULL;
}

DEVICE* VM::get_device(char *name, char *identifier)
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		if(strncmp(name, device->get_class_name(), 12) == 0
		&& strncmp(identifier, device->get_identifier(), 4) == 0) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// power on / off
	emu->out_info_x(pConfig->now_power_off ? CMsg::PowerOff : CMsg::PowerOn);

	// reset all devices
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->reset();
	}

	// update power on/off signal on MFP
	board->write_signal(BOARD::SIG_BOARD_POWER, board->get_front_power_on() ? 1 : 0, 1);
	// update alarm on/off signal on MFP
	rtc->write_signal(RTC::SIG_UPDATE_ALARM, 1, 1);
}

void VM::force_reset()
{
	if (!pConfig->now_power_off) {
		// power off forcely
		board->write_signal(BOARD::SIG_BOARD_POWER, 0, 1);
		emumsg.Send(EMUMSG_ID_RESET);
	}
}

/// pressed front power switch and send to interrupt signal
void VM::special_reset()
{
	// toggle on <-> off
	if (pConfig->now_power_off) {
		// power on
		board->write_signal(BOARD::SIG_BOARD_POWER, 1, 1);
		emumsg.Send(EMUMSG_ID_RESET);
	} else {
		// toggle front power switch on/off
		uint32_t pwr_sw = board->get_front_power_on();
		// When turn off, request power off sequence to cpu
		// If turn on while power off sequence, restart after finished the process.
		board->write_signal(BOARD::SIG_BOARD_POWER, 1 - pwr_sw, 1);
	}
}

bool VM::now_special_reset()
{
	return board->get_front_power_on() != 0;
}

void VM::warm_reset(int onoff)
{
	// send reset signal
	if (onoff < 0) {
		board->write_signal(SIG_CPU_HALT, 1, 1);
		board->write_signal(SIG_CPU_RESET, 1, 1);
		board->write_signal(SIG_CPU_RESET, 0, 1);
		board->write_signal(SIG_CPU_HALT, 0, 1);
	} else {
		if (onoff) {
			board->write_signal(SIG_CPU_HALT, onoff, 1);
			board->write_signal(SIG_CPU_RESET, onoff, 1);
		} else {
			board->write_signal(SIG_CPU_RESET, onoff, 1);
			board->write_signal(SIG_CPU_HALT, onoff, 1);
		}
	}
}

void VM::assert_interrupt(int num)
{
	// send nmi signal
	if (num == 7) board->write_signal(SIG_CPU_IRQ, 0xffffffff, 0x80);
}

void VM::run(int split_num)
{
	main_event->drive(split_num);

}

double VM::get_frame_rate()
{
	return main_event->get_frame_rate();
}

void VM::change_dipswitch(int num)
{
#ifdef USE_DIPSWITCH
	emu->out_infoc_x(CMsg::MODE_Switch_, (pConfig->dipswitch & 4) ? CMsg::ON : CMsg::OFF, NULL);
#endif
}

bool VM::now_skip()
{
	return false;
}

void VM::update_params()
{
//	change_fdd_type(emu->get_parami(ParamFddType), true);
	pConfig->main_ram_size_num = emu->get_parami(ParamMainRamSizeNum);
#ifdef USE_HD1
	pConfig->scsi_type = emu->get_parami(ParamSCSIType);
	for(int drv = 0; drv < MAX_HARD_DISKS; drv++) {
		pConfig->SetHardDiskDeviceType(drv, get_hard_disk_device_type(drv));
	}

	change_hdd_type();
#endif

	set_volume();
}

void VM::pause(int value)
{
//	msm58321->pause(value);
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
int VM::get_cpus() const
{
	int nums = 1;
	return nums;
}

DEVICE *VM::get_cpu(int index)
{
	switch(index) {
	case 0:
		return cpu;
	case 1:
		return dmac;
	}
	return NULL;
}

DEVICE *VM::get_memory(int index)
{
	switch(index) {
	case 0:
		return memory;
	case 1:
		return memory;
	}
	return NULL;
}

/// device names
const struct VM::st_device_name_map VM::c_device_names_map[] = {
	{ _T("MFP"), DNM_MFP },
	{ _T("SCC"), DNM_SCC },
	{ _T("DMAC"), DNM_DMAC },
	{ _T("CRTC"), DNM_CRTC },
	{ _T("PALETTE"), DNM_PALETTE },
	{ _T("SPRITE"), DNM_SPRITE_BG },
	{ _T("VIDCTRL"), DNM_VID_CTL },
	{ _T("FDC"), DNM_FDC },
	{ _T("FDD"), DNM_FDD },
	{ _T("SASI"), DNM_SASI },
	{ _T("SCSI"), DNM_SCSI },
	{ _T("OPM"), DNM_OPM },
	{ _T("ADPCM"), DNM_ADPCM },
	{ _T("RTC"), DNM_RTC },
	{ _T("INTCTRL"), DNM_BOARD },
	{ _T("SYSPORT"), DNM_SYSPORT },
	{ _T("KEYBOARD"), DNM_KEYBOARD },
	{ _T("SRAM"), DNM_SRAM },
	{ _T("EVENT"), DNM_EVENT },
	{ NULL, 0 }
};

bool VM::get_debug_device_name(const _TCHAR *param, uint32_t *num, int *idx, const _TCHAR **name)
{
	int i = 0; 
	for(; c_device_names_map[i].name != NULL; i++) {
		if (_tcsicmp(param, c_device_names_map[i].name) == 0) {
			if (num) *num = c_device_names_map[i].num;
			if (idx) *idx = i;
			if (name) *name = c_device_names_map[i].name;
			return true;
		}
	}
	return false;
}

void VM::get_debug_device_names_str(_TCHAR *buffer, size_t buffer_len)
{
	int i = 0;
	int len = 2;
	UTILITY::tcscpy(buffer, buffer_len, _T("  "));
	for(; c_device_names_map[i].name != NULL; i++) {
		if (i > 0) {
			UTILITY::tcscat(buffer, buffer_len, _T(","));
			len++;
		}
		int siz = (int)_tcslen(c_device_names_map[i].name);
		if (len + siz >= 80) {
			UTILITY::tcscat(buffer, buffer_len, _T("\n  "));
			len = 2;
		}
		UTILITY::tcscat(buffer, buffer_len, c_device_names_map[i].name);
		len += siz;
	}
}

bool VM::debug_write_reg(uint32_t num, uint32_t reg_num, uint32_t data)
{
	bool valid = false;
	switch(num) {
	case DNM_MFP:
		// MFP
		if (mfp) valid = mfp->debug_write_reg(reg_num, data);
		break;
	case DNM_SCC:
		// SCC
		if (scc) valid = scc->debug_write_reg(reg_num, data);
		break;
	case DNM_DMAC:
		// DMAC
		if (dmac) valid = dmac->debug_write_reg(reg_num, data);
		break;
	case DNM_CRTC:
		// CRTC
		if (crtc) valid = crtc->debug_write_reg(reg_num, data);
		break;
	case DNM_PALETTE:
		// Palette
		if (display) valid = display->debug_write_reg(0, reg_num, data);
		break;
	case DNM_SPRITE_BG:
		// sprite / BG
		if (sp_bg) valid = sp_bg->debug_write_reg(reg_num, data);
		break;
	case DNM_VID_CTL:
		// Video Control
		if (display) valid = display->debug_write_reg(1, reg_num, data);
		break;
	case DNM_FDC:
		// FDC
		if (fdc) valid = fdc->debug_write_reg(reg_num, data);
		break;
	case DNM_FDD:
		// FDD
		if (fdd) valid = fdd->debug_write_reg(reg_num, data);
		break;
	case DNM_SASI:
		// SASI HDD
		if (sasi) valid = sasi->debug_write_reg(reg_num, data);
		break;
	case DNM_SCSI:
		// SCSI HDD
		if (scsi) valid = scsi->debug_write_reg(reg_num, data);
		break;
	case DNM_OPM:
		// OPM
		if (opm) valid = opm->debug_write_reg(reg_num, data);
		break;
	case DNM_ADPCM:
		// ADPCM
		if (adpcm) valid = adpcm->debug_write_reg(reg_num, data);
		break;
	case DNM_RTC:
		// RTC
		if (rtc) valid = rtc->debug_write_reg(reg_num, data);
		break;
	case DNM_BOARD:
		// interrupt control
		if (board) valid = board->debug_write_reg(reg_num, data);
		break;
	case DNM_SYSPORT:
		// system port
		if (sysport) valid = sysport->debug_write_reg(reg_num, data);
		break;
	case DNM_KEYBOARD:
		// keyboard
		if (key) valid = key->debug_write_reg(reg_num, data);
		break;
	case DNM_SRAM:
		// SRAM
		if (memory) valid = memory->debug_write_reg(1, reg_num, data);
		break;
	}
	return valid;
}

bool VM::debug_write_reg(uint32_t num, const _TCHAR *reg, uint32_t data)
{
	bool valid = false;
	switch(num) {
	case DNM_MFP:
		// MFP
		if (mfp) valid = mfp->debug_write_reg(reg, data);
		break;
	case DNM_SCC:
		// SCC
		if (scc) valid = scc->debug_write_reg(reg, data);
		break;
	case DNM_DMAC:
		// DMAC
		if (dmac) valid = dmac->debug_write_reg(reg, data);
		break;
	case DNM_CRTC:
		// CRTC
		if (crtc) valid = crtc->debug_write_reg(reg, data);
		break;
	case DNM_PALETTE:
		// Palette
		if (display) valid = display->debug_write_reg(0, reg, data);
		break;
	case DNM_SPRITE_BG:
		// sprite / BG
		if (sp_bg) valid = sp_bg->debug_write_reg(reg, data);
		break;
	case DNM_VID_CTL:
		// Video Control
		if (display) valid = display->debug_write_reg(1, reg, data);
		break;
	case DNM_FDC:
		// FDC
		if (fdc) valid = fdc->debug_write_reg(reg, data);
		break;
	case DNM_FDD:
		// FDD
		if (fdd) valid = fdd->debug_write_reg(reg, data);
		break;
	case DNM_SASI:
		// SASI HDD
		if (sasi) valid = sasi->debug_write_reg(reg, data);
		break;
	case DNM_SCSI:
		// SCSI HDD
		if (scsi) valid = scsi->debug_write_reg(reg, data);
		break;
	case DNM_OPM:
		// OPM
		if (opm) valid = opm->debug_write_reg(reg, data);
		break;
	case DNM_ADPCM:
		// ADPCM
		if (adpcm) valid = adpcm->debug_write_reg(reg, data);
		break;
	case DNM_RTC:
		// RTC
		if (rtc) valid = rtc->debug_write_reg(reg, data);
		break;
	case DNM_BOARD:
		// interrupt control
		if (board) valid = board->debug_write_reg(reg, data);
		break;
	case DNM_SYSPORT:
		// system port
		if (sysport) valid = sysport->debug_write_reg(reg, data);
		break;
	case DNM_SRAM:
		// SRAM
		if (memory) valid = memory->debug_write_reg(1, reg, data);
		break;
	}
	return valid;
}

void VM::debug_regs_info(uint32_t num, _TCHAR *buffer, size_t buffer_len)
{
	switch(num) {
	case DNM_MFP:
		// MFP
		if (mfp) mfp->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_SCC:
		// SCC
		if (scc) scc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_DMAC:
		// DMAC
		if (dmac) dmac->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_CRTC:
		// CRTC
		if (crtc) crtc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_PALETTE:
		// Palette
		if (display) display->debug_regs_info(0, buffer, buffer_len);
		break;
	case DNM_SPRITE_BG:
		// sprite / BG
		if (sp_bg) sp_bg->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_VID_CTL:
		// Video Control
		if (display) display->debug_regs_info(1, buffer, buffer_len);
		break;
	case DNM_FDC:
		// FDC
		if (fdc) fdc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_FDD:
		// FDD
		if (fdd) fdd->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_SASI:
		// SASI HDD
		if (sasi) sasi->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_SCSI:
		// SCSI HDD
		if (scsi) scsi->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_OPM:
		// OPM
		if (opm) opm->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_ADPCM:
		// ADPCM
		if (adpcm) adpcm->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_RTC:
		// RTC
		if (rtc) rtc->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_BOARD:
		// interrupt control
		if (board) board->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_SYSPORT:
		// system port
		if (sysport) sysport->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_KEYBOARD:
		// keyboard
		if (key) key->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_EVENT:
		// event
		if (main_event) main_event->debug_regs_info(buffer, buffer_len);
		break;
	case DNM_SRAM:
		// SRAM
		if (memory) memory->debug_regs_info(1, buffer, buffer_len);
		break;
	}
}
#endif	/* USE_DEBUGGER */

// ----------------------------------------------------------------------------
// event manager
// ----------------------------------------------------------------------------

void VM::register_event(DEVICE* dev, int event_id, int usec, bool loop, int* register_id)
{
	main_event->register_event(dev, event_id, usec, loop, register_id);
}

void VM::register_event_by_clock(DEVICE* dev, int event_id, int clock, bool loop, int* register_id)
{
	main_event->register_event_by_clock(dev, event_id, clock, loop, register_id);
}

void VM::cancel_event(DEVICE *dev, int register_id)
{
	main_event->cancel_event(dev, register_id);
}

void VM::register_frame_event(DEVICE* dev)
{
	main_event->register_frame_event(dev);
}

void VM::register_vline_event(DEVICE* dev)
{
	main_event->register_vline_event(dev);
}

uint64_t VM::get_current_clock()
{
	return main_event->get_current_clock();
}

uint64_t VM::get_passed_clock(uint64_t prev)
{
	return main_event->get_passed_clock(prev);
}

//uint32_t VM::get_pc()
//{
//	return cpu->get_pc();
//}

void VM::set_lines_per_frame(int lines) {
	main_event->set_lines_per_frame(lines);
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::set_display_size(int left, int top, int right, int bottom)
{
	display->set_display_size(left, top, right, bottom);
}

void VM::draw_screen()
{
//	display->set_vram_ptr(memory->get_vram());
	display->draw_screen();
}

// ----------------------------------------------------------------------------
uint64_t VM::update_led()
{
	uint64_t status = 0;

	// b0-b8: kbd led (negative logic)
	status |= key->get_led_status();
	// b9-b10: power on/off
	status |= board->get_led_status();
	// b9: alarm  b11: timer
	status |= rtc->get_led_status();
#ifdef USE_HD1
	// b12: sasi hdd
	status |= (sasi->get_led_status());
	// b12: scsi hdd
	status |= (scsi->get_led_status());
#endif
	// b13: hireso
	status |= crtc->get_led_status();
#ifdef USE_FD1
	// b14-b25: fdd status
	status |= (fdd->get_led_status() << 14);
#endif
	return status;
}

// ----------------------------------------------------------------------------
// sound manager
// ----------------------------------------------------------------------------

/// Initialize sound device at starting application
/// @param [in] rate : sampling rate
/// @param [in] samples : sample number per second
void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	main_event->initialize_sound(rate, samples);

	// init sound gen
	opm->initialize_sound(rate, 4000000, samples, 0);
	adpcm->initialize_sound(rate, 4000000, samples, 0);

#ifdef USE_FD1
	fdd->initialize_sound(rate, 0);
#endif
#ifdef USE_HD1
	sasi->initialize_sound(rate, 0);
	scsi->initialize_sound(rate, 0);
#endif

	//
	set_volume();
}

/// Re-initialize sound device under power-on operation
/// @param [in] rate : sampling rate
/// @param [in] samples : sample number per second
void VM::reset_sound(int rate, int samples)
{
}

/// @attention called by another thread
audio_sample_t* VM::create_sound(int* extra_frames, int samples)
{
	return main_event->create_sound(extra_frames, samples);
}

void VM::set_volume()
{
	int vol = 0;
	main_event->set_volume(pConfig->volume - 81, pConfig->mute);
	vol = pConfig->opm_volume - 81;
	opm->set_volume(vol, vol, pConfig->opm_mute);
	vol = pConfig->adpcm_volume - 81;
	adpcm->set_volume(vol, vol, pConfig->adpcm_mute);
#ifdef USE_FD1
	fdd->set_volume(pConfig->fdd_volume - 81, pConfig->fdd_mute);
#endif
#ifdef USE_HD1
	sasi->set_volume(pConfig->hdd_volume - 81, pConfig->hdd_mute);
	scsi->set_volume(pConfig->hdd_volume - 81, pConfig->hdd_mute);
#endif
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code)
{
}

void VM::key_up(int code)
{
}

void VM::system_key_down(int code)
{
	key->system_key_down(code);
}

void VM::system_key_up(int code)
{
	key->system_key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

#ifdef USE_DATAREC
bool VM::play_datarec(const _TCHAR* file_path)
{
//	return cmt->play_datarec(file_path);
	return false;
}

bool VM::rec_datarec(const _TCHAR* file_path)
{
//	return cmt->rec_datarec(file_path);
}

void VM::close_datarec()
{
//	cmt->close_datarec();
}

void VM::rewind_datarec()
{
//	cmt->rewind_datarec();
}

void VM::fast_forward_datarec()
{
//	cmt->fast_forward_datarec();
}

void VM::stop_datarec()
{
//	cmt->stop_datarec();
}

void VM::realmode_datarec()
{
//	cmt->realmode_datarec();
}

bool VM::datarec_opened(bool play_mode)
{
//	return cmt->datarec_opened(play_mode);
	return false;
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_FD1
bool VM::open_floppy_disk(int drv, const _TCHAR* file_path, int offset, uint32_t flags)
{
	bool rc = fdd->open_disk(drv, file_path, offset, flags);
	if (rc) {
		if (!(flags & OPEN_DISK_FLAGS_FORCELY)) {
			int sdrv = fdd->inserted_disk_another_drive(drv, file_path, offset);
			if (sdrv >= 0) {
				int drvmin = MIN(drv, sdrv);
				int drvmax = MAX(drv, sdrv);
				logging->out_logf_x(LOG_WARN, CMsg::There_is_the_same_floppy_disk_in_drive_VDIGIT_and_VDIGIT, drvmin, drvmax);
			}
		}
	}
	return rc;
}

bool VM::close_floppy_disk(int drv, uint32_t flags)
{
	return fdd->close_disk(drv, flags);
}

int VM::change_floppy_disk(int drv)
{
	return 0;
}

bool VM::floppy_disk_inserted(int drv)
{
	return fdd->disk_inserted(drv);
}

int VM::get_floppy_disk_side(int drv)
{
	return fdd->get_disk_side(drv);
}

void VM::toggle_floppy_disk_write_protect(int drv)
{
	fdd->toggle_disk_write_protect(drv);
}

bool VM::floppy_disk_write_protected(int drv)
{
	return fdd->disk_write_protected(drv);
}

bool VM::is_same_floppy_disk(int drv, const _TCHAR *file_path, int offset)
{
	return fdd->is_same_disk(drv, file_path, offset);
}
#endif

// ----------------------------------------------------------------------------

#ifdef USE_HD1
bool VM::open_hard_disk(int drv, const _TCHAR* file_path, uint32_t flags)
{
	bool rc = false;
	if (IS_SASI_DRIVE(drv)) {
		rc = sasi->open_disk(drv, file_path, flags);
	} else {
		rc = scsi->open_disk(TO_SCSI_DRIVE(drv), file_path, flags);
	}
	if (rc) {
		if (!(flags & OPEN_DISK_FLAGS_FORCELY)) {
			int samedrv = -1;
			if (samedrv < 0) {
				samedrv = sasi->mounted_disk_another_drive(drv, file_path);
			}
			if (samedrv < 0) {
				samedrv = scsi->mounted_disk_another_drive(TO_SCSI_DRIVE(drv), file_path);
				if (samedrv >= 0) samedrv = FROM_SCSI_DRIVE(samedrv);
			}
			if (samedrv >= 0) {
				_TCHAR str1[128];
				_TCHAR str2[128];
				if (IS_SASI_DRIVE(drv)) {
					int sdrv = drv / SASI_UNITS_PER_CTRL;
					int sunit = drv % SASI_UNITS_PER_CTRL;
					UTILITY::stprintf(str1, 128, _T("SASI%du%d"), sdrv, sunit);
				} else {
					UTILITY::stprintf(str1, 128, _T("SCSI%d"), TO_SCSI_DRIVE(drv));
				}
				if (IS_SASI_DRIVE(samedrv)) {
					int sdrv = samedrv / SASI_UNITS_PER_CTRL;
					int sunit = samedrv % SASI_UNITS_PER_CTRL;
					UTILITY::stprintf(str2, 128, _T("SASI%du%d"), sdrv, sunit);
				} else {
					UTILITY::stprintf(str2, 128, _T("SCSI%d"), TO_SCSI_DRIVE(samedrv));
				}
				logging->out_logf_x(LOG_WARN, CMsg::There_is_the_same_hard_disk_in_VSTR_and_VSTR
					, str1
					, str2);
			}
		}
	} else {
		// open error message
		if (IS_SASI_DRIVE(drv)) {
			int sdrv = drv / SASI_UNITS_PER_CTRL;
			int sunit = drv % SASI_UNITS_PER_CTRL;
			logging->out_logf_x(LOG_ERROR, CMsg::Disk_image_on_SASI_VDIGIT_unit_VDIGIT_couldn_t_be_opened, sdrv, sunit);
		} else {
			logging->out_logf_x(LOG_ERROR, CMsg::Disk_image_on_SCSI_VDIGIT_couldn_t_be_opened, TO_SCSI_DRIVE(drv));
		}
	}
	return rc;
}

bool VM::close_hard_disk(int drv, uint32_t flags)
{
	if (IS_SASI_DRIVE(drv)) {
		return sasi->close_disk(drv, flags);
	} else {
		return scsi->close_disk(TO_SCSI_DRIVE(drv), flags);
	}
}

bool VM::hard_disk_mounted(int drv)
{
	if (IS_SASI_DRIVE(drv)) {
		return sasi->disk_mounted(drv);
	} else {
		return scsi->disk_mounted(TO_SCSI_DRIVE(drv));
	}
}

void VM::toggle_hard_disk_write_protect(int drv)
{
	if (IS_SASI_DRIVE(drv)) {
		sasi->toggle_disk_write_protect(drv);
	} else {
		scsi->toggle_disk_write_protect(TO_SCSI_DRIVE(drv));
	}
}

bool VM::hard_disk_write_protected(int drv)
{
	if (IS_SASI_DRIVE(drv)) {
		return sasi->disk_write_protected(drv);
	} else {
		return scsi->disk_write_protected(TO_SCSI_DRIVE(drv));
	}
}

void VM::set_hard_disk_device_type(int drv, int num)
{
	int idx = drv / 4;
	int sft = (drv % 4) * 8;
	int val = emu->get_parami(VM::ParamHDDeviceType0 + idx);
	val &= ~(0xff << sft);
	val |= (num << sft);
	emu->set_parami(VM::ParamHDDeviceType0 + idx, val);
}

int VM::get_hard_disk_device_type(int drv)
{
	int idx = drv / 4;
	int sft = (drv % 4) * 8;
	return (emu->get_parami(VM::ParamHDDeviceType0 + idx) >> sft) & 0xff;
}

void VM::change_hard_disk_device_type(int drv, int num)
{
	// set on config only
	int units;
	if (IS_SASI_DRIVE(drv)) {
		units = SASI_UNITS_PER_CTRL;
	} else {
		units = SCSI_UNITS_PER_CTRL;
	}
	drv /= units;
	drv *= units;
	for(int unit = 0; unit < units; unit++) {
		set_hard_disk_device_type(drv, num);
		drv++;
	}
}

int VM::get_current_hard_disk_device_type(int drv)
{
	return pConfig->GetHardDiskDeviceType(drv);
}

bool VM::is_same_hard_disk(int drv, const _TCHAR *file_path)
{
	if (IS_SASI_DRIVE(drv)) {
		return sasi->is_same_disk(drv, file_path);
	} else {
		return scsi->is_same_disk(TO_SCSI_DRIVE(drv), file_path);
	}
}
#endif

// ----------------------------------------------------------------------------
void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->get_next_device()) {
		device->update_config();
	}
}

// ----------------------------------------------------------------------------
#ifdef USE_PRINTER
bool VM::save_printer(int dev, const _TCHAR* filename)
{
	return printer[dev]->save_printer(filename);
}

void VM::clear_printer(int dev)
{
	printer[dev]->reset();
}

int VM::get_printer_buffer_size(int dev)
{
	return printer[dev]->get_buffer_size();
}

uint8_t* VM::get_printer_buffer(int dev)
{
	return printer[dev]->get_buffer();
}

void VM::enable_printer_direct(int dev)
{
	printer[dev]->set_direct_mode();
}

bool VM::print_printer(int dev)
{
	return printer[dev]->print_printer();
}

void VM::toggle_printer_online(int dev)
{
	return printer[dev]->toggle_printer_online();
}
#endif

// ----------------------------------------------------------------------------
/// @note called by main thread
void VM::enable_comm_server(int dev)
{
	comm[dev]->enable_server();
}

/// @note called by main thread
void VM::enable_comm_connect(int dev, int num)
{
	comm[dev]->enable_connect(num);
}

/// @note called by main thread
bool VM::now_comm_connecting(int dev, int num)
{
	return comm[dev]->now_connecting(num);
}

/// @note called by main thread
void VM::send_comm_telnet_command(int dev, int num)
{
	comm[dev]->send_telnet_command(num);
}

// ----------------------------------------------------------------------------
void VM::modify_joytype()
{
	key->modify_joytype();
}

void VM::save_keybind()
{
//	key->save_keybind();
}

void VM::clear_joy2joyk_map()
{
	key->clear_joy2joyk_map();
}

void VM::set_joy2joyk_map(int num, int idx, uint32_t joy_code)
{
	key->set_joy2joyk_map(num, idx, joy_code);
}

// ----------------------------------------------------------------------------
void VM::load_sram_file()
{
	memory->load_sram_file_forcely();
}

void VM::save_sram_file()
{
	memory->save_sram_file_forcely();
}

uint32_t VM::get_sram_ram_size() const
{
	return memory->get_sram32(SRAM_MAIN_RAM_SIZE);
}

uint32_t VM::get_sram_rom_start_address() const
{
	return memory->get_sram32(SRAM_ROM_START_ADDRESS);
}

void VM::set_sram_rom_start_address(uint32_t val)
{
	memory->set_sram32(SRAM_ROM_START_ADDRESS, val);
}

uint32_t VM::get_sram_sram_start_address() const
{
	return memory->get_sram32(SRAM_SRAM_START_ADDRESS);
}

void VM::set_sram_sram_start_address(uint32_t val)
{
	memory->set_sram32(SRAM_SRAM_START_ADDRESS, val);
}

/// @return position
int VM::get_sram_boot_device() const
{
	int pos = BOOT_DEVICE_UNKNOWN;
	uint16_t val = memory->get_sram16(SRAM_BOOT_DEVICE);
	if (val == 0x0000) {
		pos = BOOT_DEVICE_STD;
	} else if ((val & 0xf0ff) == 0x8000) {
		if ((val & 0x0f00) < 0x0400) {
			pos = BOOT_DEVICE_HD0 + ((val & 0x0f00) >> 16);
		}
	} else if ((val & 0xf0ff) == 0x9070) {
		if ((val & 0x0f00) < 0x0400) {
			pos = BOOT_DEVICE_2HD0 + ((val & 0x0f00) >> 16);
		}
	} else if (val == 0xa000) {
		pos = BOOT_DEVICE_ROM;
	} else if (val == 0xb000) {
		pos = BOOT_DEVICE_SRAM;
	}
	return pos;
}

uint32_t VM::conv_sram_boot_device(int pos) const
{
	uint32_t val = 0;
	if (pos >= BOOT_DEVICE_HD0 && pos <= BOOT_DEVICE_HD3) {
		val = 0x8000 | ((pos - BOOT_DEVICE_HD0) << 8);
	} else if (pos >= BOOT_DEVICE_2HD0 && pos <= BOOT_DEVICE_2HD3) {
		val = 0x9070 | ((pos - BOOT_DEVICE_2HD0) << 8);
	} else if (pos == BOOT_DEVICE_ROM) {
		val = 0xa000;
	} else if (pos == BOOT_DEVICE_SRAM) {
		val = 0xb000;
	}
	return val;
}

void VM::set_sram_boot_device(int pos)
{
	uint32_t val = conv_sram_boot_device(pos);
	memory->set_sram16(SRAM_BOOT_DEVICE, val);
}

uint16_t VM::get_sram_rs232c() const
{
	return memory->get_sram16(SRAM_RS232C);
}

void VM::set_sram_rs232c(uint32_t val)
{
	memory->set_sram16(SRAM_RS232C, val);
}

int VM::get_sram_rs232c_baud_rate(uint32_t value) const
{
	// bit0-3
	int pos = (value & 0xf);
	if (pos > BAUD_RATE_UNKNOWN) {
		pos = BAUD_RATE_UNKNOWN;
	}
	return pos;
}

uint32_t VM::conv_sram_rs232c_baud_rate(int pos) const
{
	// bit0 - bit3
	return (pos & 0xf);
}

int VM::get_sram_rs232c_databit(uint32_t value) const
{
	// bit10-11
	return ((value >> 10) & 0x3);
}

uint32_t VM::conv_sram_rs232c_databit(int pos) const
{
	// bit10-11
	return ((pos & 0x3) << 10);
}

int VM::get_sram_rs232c_parity(uint32_t value) const
{
	// bit12-13
	int pos = ((value >> 12) & 0x3);
	switch(pos) {
	case 3:
		pos = SRAM_PARITY_EVEN;
		break;
	case 2:
		pos = SRAM_PARITY_NONE;
		break;
	default:
		break;
	}
	return pos;
}

uint32_t VM::conv_sram_rs232c_parity(int pos) const
{
	// bit12-13
	switch(pos) {
	case SRAM_PARITY_EVEN:
		pos = 3;
		break;
	case SRAM_PARITY_ODD:
		pos = 1;
		break;
	default:
		pos = 0;
		break;
	}
	return (pos << 12);
}

int VM::get_sram_rs232c_stopbit(uint32_t value) const
{
	// bit14-15
	int pos = ((value >> 14) & 0x3);
	switch(pos) {
	case 1:
		pos = STOPBIT_1BIT;
		break;
	case 2:
		pos = STOPBIT_1_5BITS;
		break;
	default:
		pos = STOPBIT_2BITS;
		break;
	}
	return pos;
}

uint32_t VM::conv_sram_rs232c_stopbit(int pos) const
{
	// bit14-15
	switch(pos) {
	case STOPBIT_1BIT:
		pos = 1;
		break;
	case STOPBIT_1_5BITS:
		pos = 2;
		break;
	default:
		pos = 0;
		break;
	}
	return (pos << 14);
}

int VM::get_sram_rs232c_flowctrl(uint32_t value) const
{
	// bit8-9
	return ((value >> 8) & 0x3);
}

uint32_t VM::conv_sram_rs232c_flowctrl(int pos) const
{
	// bit8-9
	return ((pos & 0x3) << 8);
}

bool VM::get_sram_alarm_onoff() const
{
	return memory->get_sram8(SRAM_ALARM_ON_OFF) == 0;
}

void VM::set_sram_alarm_onoff(bool val)
{
	memory->set_sram8(SRAM_ALARM_ON_OFF, val ? 0 : 7);
}

uint32_t VM::get_sram_alarm_time() const
{
	return memory->get_sram32(SRAM_ALARM_ON_TIME);
}

int VM::get_sram_alarm_duration() const
{
	return (int)memory->get_sram32(SRAM_ALARM_DURATION);
}

int VM::get_sram_contrast() const
{
	return memory->get_sram8(SRAM_CONTRAST) & 0xf;
}

void VM::set_sram_contrast(int val)
{
	memory->set_sram8(SRAM_CONTRAST, val & 0xf);
}

int VM::get_sram_fd_eject() const
{
	return memory->get_sram8(SRAM_FD_EJECT);
}

void VM::set_sram_fd_eject(int val)
{
	memory->set_sram8(SRAM_FD_EJECT, val ? 0x01 : 0x00);
}

int VM::get_sram_purpose() const
{
	return memory->get_sram8(SRAM_PURPOSE) & 0x03;
}

void VM::set_sram_purpose(int val)
{
	memory->set_sram8(SRAM_PURPOSE, val & 0x03);
}

int VM::get_sram_key_repeat_delay() const
{
	return memory->get_sram8(SRAM_KEY_REPEAT_DELAY) & 0xf;
}

void VM::set_sram_key_repeat_delay(int pos)
{
	memory->set_sram8(SRAM_KEY_REPEAT_DELAY, pos & 0xf);
}

int VM::get_sram_key_repeat_rate() const
{
	return memory->get_sram8(SRAM_KEY_REPEAT_RATE) & 0xf;
}

void VM::set_sram_key_repeat_rate(int pos)
{
	memory->set_sram8(SRAM_KEY_REPEAT_RATE, pos & 0xf);
}

int VM::get_sram_key_led() const
{
	return memory->get_sram8(SRAM_KEY_LED) & 0x7f;
}

void VM::set_sram_key_led(int val)
{
	memory->set_sram8(SRAM_KEY_LED, val & 0x7f);
}

int VM::get_sram_sasi_hdd_nums() const
{
	return memory->get_sram8(SRAM_SASI_HDD_NUMS) & 0xf;
}

void VM::set_sram_sasi_hdd_nums(int val)
{
	memory->set_sram8(SRAM_SASI_HDD_NUMS, val & 0xf);
}

bool VM::get_sram_scsi_enable_flag() const
{
	return (memory->get_sram8(SRAM_SCSI_ENABLE_FLAG) == 0x56);
}

void VM::set_sram_scsi_enable_flag(bool val)
{
	memory->set_sram8(SRAM_SCSI_ENABLE_FLAG, val ? 0x56 : 0);
}

int VM::get_sram_scsi_host_id() const
{
	return memory->get_sram8(SRAM_SCSI_HOST_ID) & 0x7;
}

void VM::set_sram_scsi_host_id(int val)
{
	val &= 0x7;
	val |= memory->get_sram8(SRAM_SCSI_HOST_ID) & ~0x7;
	memory->set_sram8(SRAM_SCSI_HOST_ID, val & 0xff);
}

uint8_t VM::get_sram_sasi_hdd_on_scsi() const
{
	return memory->get_sram8(SRAM_SASI_HDD_ON_SCSI);
}

void VM::set_sram_sasi_hdd_on_scsi(uint8_t val)
{
	memory->set_sram8(SRAM_SASI_HDD_ON_SCSI, val);
}

// ----------------------------------------------------------------------------
void VM::change_archtecture(int id, int num, bool reset)
{
}

void VM::change_fdd_type(int num, bool reset)
{
}

void VM::change_hdd_type()
{
#ifdef USE_HD1
	sasi->init_context_irq();
	sasi->init_context_drq();
	scsi->init_context_irq();
	scsi->init_context_drq();
	switch(pConfig->scsi_type) {
	case SCSI_TYPE_IN:
		scsi->set_context_irq(board, SIG_CPU_IRQ, 0x100002);	// INT1 // same as SASI
		scsi->set_context_drq(dmac, DMAC::SIG_REQ_1, 1);
		break;
	case SCSI_TYPE_EX:
		scsi->set_context_irq(board, SIG_CPU_IRQ, 0x100004);	// INT2
		// [: through :]
	default:
		sasi->set_context_irq(board, SIG_CPU_IRQ, 0x100002);	// IRQ to IPL1
		sasi->set_context_drq(dmac, DMAC::SIG_REQ_1, 1);
		break;
	}
	for(int drv=0; drv<MAX_HARD_DISKS; drv++) {
		int idx = pConfig->GetHardDiskIndex(drv);
		if (idx < 0) continue;
		if (IS_SASI_DRIVE(drv)) {
			sasi->change_device_type(drv, pConfig->GetHardDiskDeviceType(drv));
		} else {
			scsi->change_device_type(TO_SCSI_DRIVE(drv), pConfig->GetHardDiskDeviceType(drv));
		}
	}
#endif
}

// ----------------------------------------------------------------------------

/// @brief load a data from file
///
/// @param[in]  file_path : directory
/// @param[in]  file_name : data file
/// @param[out] data      : loaded data
/// @param[in]  size      : buffer size of data
/// @param[in]  first_data      : (nullable) first pattern to compare to loaded data
/// @param[in]  first_data_size : size of first_data
/// @param[in]  first_data_pos  : comparing position in loaded data
/// @param[in]  last_data       : (nullable) last pattern to compare to loaded data
/// @param[in]  last_data_size  : size of last_data
/// @param[in]  last_data_pos   : comparing position in loaded data
/// @return 1:successfully loaded  2:data loaded but unmatch pattern or size  0:unloaded
int VM::load_data_from_file(const _TCHAR *file_path, const _TCHAR *file_name
	, uint8_t *data, size_t size
	, const uint8_t *first_data, size_t first_data_size, size_t first_data_pos
	, const uint8_t *last_data,  size_t last_data_size, size_t last_data_pos)
{
	return EMU::load_data_from_file(file_path, file_name, data, size
		, first_data, first_data_size, first_data_pos
		, last_data, last_data_size, last_data_pos);
}

/// @brief get VM specific parameter
///
/// @param[in] id
/// @return parameter
int VM::get_parami(enumParamiId id) const
{
	return emu->get_parami(id);
}

/// @brief set VM specific parameter
///
/// @param[in] id
/// @param[in] val : parameter
void VM::set_parami(enumParamiId id, int val)
{
	emu->set_parami(id, val);
}

/// @brief get VM specific object
///
/// @param[in] id
/// @return object
void *VM::get_paramv(enumParamvId id) const
{
	return emu->get_paramv(id);
}

/// @brief set VM specific object
///
/// @param[in] id
/// @param[in] val : object
void VM::set_paramv(enumParamvId id, void *val)
{
	emu->set_paramv(id, val);
}

const _TCHAR *VM::application_path() const
{
	return emu->application_path();
}

const _TCHAR *VM::initialize_path() const
{
	return emu->initialize_path();
}

bool VM::get_pause(int idx) const
{
	return emu->get_pause(idx);
}

void VM::set_pause(int idx, bool val)
{
	emu->set_pause(idx, val);
}

//
void VM::get_edition_string(char *buffer, size_t buffer_len) const
{
	*buffer = '\0';
#ifdef USE_DEBUGGER
	UTILITY::strcat(buffer, buffer_len, *buffer == '\0' ? " with " : ", ");
	UTILITY::strcat(buffer, buffer_len, "Debugger");
#endif
}

// ----------------------------------------------------------------------------

bool VM::save_state(const _TCHAR* filename)
{
	vm_state_header_t vm_state_h;

	// header
	memset(&vm_state_h, 0, sizeof(vm_state_h));
	UTILITY::strncpy(vm_state_h.v1.header, sizeof(vm_state_h.v1.header), RESUME_FILE_HEADER, 16);
	vm_state_h.v1.version = Uint16_LE(RESUME_FILE_VERSION);
	vm_state_h.v1.revision = Uint16_LE(RESUME_FILE_REVISION);
	vm_state_h.v1.param = Uint32_LE(0);
	vm_state_h.v1.emu_major = Uint16_LE(APP_VER_MAJOR);
	vm_state_h.v1.emu_minor = Uint16_LE(APP_VER_MINOR);
	vm_state_h.v1.emu_rev   = Uint16_LE(APP_VER_REV);
	vm_state_h.v1.emu_build = Uint16_LE(APP_VER_BUILD);
	// version 2
	vm_state_h.v2.scsi_type = (pConfig->scsi_type & 0xf) | ((emu->get_parami(ParamSCSIType) & 0xf) << 4);
	for(int drv=0; drv<MAX_HARD_DISKS; drv++) {
		vm_state_h.v2.hdd_device_type[drv] = (pConfig->GetHardDiskDeviceType(drv) & 0xf) | ((get_hard_disk_device_type(drv) & 0xf) << 4);
	}

	FILEIO fio;
	bool rc = false;
	if(fio.Fopen(filename, FILEIO::WRITE_BINARY)) {
		// write header
		fio.Fwrite(&vm_state_h, sizeof(vm_state_h), 1);
		// write data
		for(DEVICE* device = first_device; device; device = device->get_next_device()) {
			device->save_state(&fio);
		}
		fio.Fclose();
		rc = true;
	}
	return rc;
}

bool VM::load_state(const _TCHAR* filename)
{
	vm_state_header_t vm_state_h;

	FILEIO fio;
	bool rc = false;
	do {
		if(!fio.Fopen(filename, FILEIO::READ_BINARY)) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Cannot_open);
			break;
		}
		// read header
		fio.Fread(&vm_state_h.v1, sizeof(vm_state_h.v1), 1);
		// check header
		if (strncmp(vm_state_h.v1.header, RESUME_FILE_HEADER, 16) != 0) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Unsupported_file);
			break;
		}

		uint16_t version = Uint16_LE(vm_state_h.v1.version);
		if (version == 2) {
			// version 2
			// read more
			fio.Fread(&vm_state_h.v2, sizeof(vm_state_h.v2), 1);

			pConfig->scsi_type = (vm_state_h.v2.scsi_type & 0xf);
			emu->set_parami(ParamSCSIType, (vm_state_h.v2.scsi_type >> 4) & 0xf);
			for(int drv=0; drv<MAX_HARD_DISKS; drv++) {
				pConfig->SetHardDiskDeviceType(drv, vm_state_h.v2.hdd_device_type[drv] & 0xf);
				set_hard_disk_device_type(drv, (vm_state_h.v2.hdd_device_type[drv] >> 4) & 0xf);
			}

		} else if (version == 1) {
			// version 1
			memset(&vm_state_h.v2, 0, sizeof(vm_state_h.v2));

			pConfig->scsi_type = (vm_state_h.v2.scsi_type & 0xf);
			for(int drv=0; drv<MAX_HARD_DISKS; drv++) {
				pConfig->SetHardDiskDeviceType(drv, vm_state_h.v2.hdd_device_type[drv] & 0xf);
			}

		} else {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Invalid_version);
			break;
		}

		// read data
		rc = true;
		for(DEVICE* device = first_device; rc && device != NULL; device = device->get_next_device()) {
			rc = device->load_state(&fio);
		}
		//
		if (!rc) {
			logging->out_log_x(LOG_ERROR, CMsg::Load_State_Invalid_version);
			break;
		}
		//
		change_hdd_type();
		//
		set_volume();
	} while(0);

	fio.Fclose();


	return rc;
}

// ----------------------------------------------------------------------------

#ifdef USE_KEY_RECORD
bool VM::play_reckey(const _TCHAR* filename)
{
	return key->play_reckey(filename);
}

bool VM::record_reckey(const _TCHAR* filename)
{
	return key->record_reckey(filename);
}

void VM::stop_reckey(bool stop_play, bool stop_record)
{
	key->stop_reckey(stop_play, stop_record);
}
#endif

