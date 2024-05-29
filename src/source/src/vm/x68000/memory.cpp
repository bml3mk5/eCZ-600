/** @file memory.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ memory ]
*/

#include "memory.h"
#include <stdlib.h>
#include <time.h>
//#include "../../emu.h"
#include "../vm.h"
#include "../../config.h"
#include "crtc.h"
#include "dmac.h"
#include "scc.h"
#include "../ym2151.h"
#include "adpcm.h"
#include "sprite_bg.h"
//#include "cmt.h"
#include "../i8255.h"
#include "rtc.h"
#include "sasi.h"
#include "../../fileio.h"
#include "../../logging.h"
#include "../../utility.h"
#include "../mc68000_consts.h"
#ifdef USE_DEBUGGER
//#include "l3basic.h"
#include "../../osd/debugger_console.h"
#endif

//#define _DEBUG_RAM
//#define _DEBUG_CRAM

#ifdef _DEBUG
//#define OUT_DEBUG_TVRAM logging->out_debugf
#define OUT_DEBUG_TVRAM(...)
//#define OUT_DEBUG_GVRAM logging->out_debugf
#define OUT_DEBUG_GVRAM(...)
//#define OUT_DEBUG_CGROM logging->out_debugf
#define OUT_DEBUG_CGROM(...)
//#define OUT_DEBUG_IVEC logging->out_debugf
#define OUT_DEBUG_IVEC(...)
#else
#define OUT_DEBUG_TVRAM(...)
#define OUT_DEBUG_GVRAM(...)
#define OUT_DEBUG_CGROM(...)
#define OUT_DEBUG_IVEC(...)
#endif

#define ADDRH_MASK(ah, x) ((ah) & ((x >> 1) - 1))

#define STORE_DATA(mem, dat, msk) mem = (((mem) & (msk)) | (dat))

// unit size is word
#define AREA_SET_SIZE m_area_set_size = 0x1000 * (m_area_set + 1)

MEMORY::MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("MEMORY");

	d_cpu = NULL;
	d_crtc = NULL;
	d_dmac = NULL;
	d_mfp = NULL;
	d_rtc = NULL;
	d_scc = NULL;
	d_sp_bg = NULL;
	d_sysport = NULL;
	d_disp = NULL;
	d_opm = NULL;
	d_fdd = NULL;
	d_fdc = NULL;
	d_sasi = NULL;
	d_comm = NULL;
	d_printer = NULL;
	d_board = NULL;
	d_scsi = NULL;

	p_crtc_regs = NULL;

	m_ram = NULL;
	m_ram_size = 0;	// word
	m_buserr = 0;
	m_is_xvi = false;

#ifdef USE_DEBUGGER
//	dc  = NULL;
//	bas = NULL;
	d_debugger = NULL;
#endif
}

MEMORY::~MEMORY()
{
#ifdef USE_DEBUGGER
//	delete bas;
#endif
	delete [] m_ram;
}

void MEMORY::initialize()
{
	rom_loaded[0] = 0;
	rom_loaded[1] = 0;
	rom_loaded_at_first = false;
	scsi_rom_loaded[0] = 0;
	scsi_rom_loaded[1] = 0;

	sram_loaded = 0;
	sram_saved = true;

	memset(m_rom, 0xff, sizeof(m_rom));
	memset(m_scsi_rom, 0xff, sizeof(m_scsi_rom));

	// read rom image from file
	load_rom_files();

	clear_sram();
	if (!FLG_ORIG_SRAM_CLR_PWRON) {
		// load SRAM
		load_sram_file();
	}

	m_ipl_mapping = false;
	m_sram_writable = false;
	m_buserr = 0;

	update_config();

//	register_vline_event(this);
#ifdef USE_DEBUGGER
//	bas = new L3Basic(this);
#endif
}

void MEMORY::release()
{
	if (FLG_ORIG_SRAM_SAVE_PWROFF) {
		// save SRAM
		sram_saved = false;
		save_sram_file();
	}
}

/// power on reset
/// @note reset() is called when processing power off and on.
void MEMORY::reset()
{
//	int val = 0;

	// read rom image from file
	load_rom_files();

	// resize main ram size and (re)allocate it
	set_main_ram();

	clear_main_memory(pConfig->ram_initialize, m_ram, m_ram_size);
	clear_graphic_memory(pConfig->vram_initialize, m_gvram, sizeof(m_gvram) / sizeof(m_gvram[0]));
	clear_graphic_memory(pConfig->vram_initialize, m_tvram, sizeof(m_tvram) / sizeof(m_tvram[0]));
	clear_graphic_memory(pConfig->spram_initialize, m_spram, sizeof(m_spram) / sizeof(m_spram[0]));
	memset(u_tvram , 0x00, sizeof(u_tvram));
	memset(u_pcg , 0x00, sizeof(u_pcg));
//	memset(u_gvram , 0x00, sizeof(u_gvram));
//	memset(u_spram , 0x00, sizeof(u_spram));
//	memset(u_bg , 0x00, sizeof(u_bg));

	if (pConfig->now_power_off) {
		if (FLG_ORIG_SRAM_SAVE_PWROFF) {
			// save SRAM
			save_sram_file();
		}
		sram_loaded = false;
	} else {
		if (FLG_ORIG_SRAM_CLR_PWRON) {
			// clear SRAM
			clear_sram();
		} else {
			// load SRAM
			load_sram_file();
		}
		sram_saved = false;
	}

	warm_reset(true);

#ifdef USE_DEBUGGER
//	if (bas) bas->Reset(0);
#endif
}

void MEMORY::warm_reset(bool por)
{
	m_ipl_mapping = true;
	m_sram_writable = false;
	m_buserr = 0;

	m_area_set = 0;
	AREA_SET_SIZE; // words
}

void MEMORY::clear_main_memory(int method, uint16_t *buf, size_t size)
{
	int val = 0;
	switch(method) {
	case 1:
		val = 0xff;
		/* :through: */
	case 2:
		memset(buf , val, size * sizeof(uint16_t));
		break;
	default:
		clear_ram(buf, size);
		break;
	}
}

void MEMORY::clear_graphic_memory(int method, uint16_t *buf, size_t size)
{
	int val = 0;
	switch(method) {
	case 1:
		val = 0xff;
		/* :through: */
	default:
		memset(buf , val, size * sizeof(uint16_t));
		break;
	}
}

/// clear memory such as stripe
/// @param[in] buf  : buffer
/// @param[in] size : size in words
void MEMORY::clear_ram(uint16_t *buf, size_t size)
{
	for(size_t i=0; i<size; i+=8) {
		for(size_t n=i; n<(i+2) && n<size; n++) {
			buf[n] = 0xffff;
		}
		for(size_t n=(i+2); n<(i+6) && n<size; n++) {
			buf[n] = 0;
		}
		for(size_t n=(i+6); n<(i+8) && n<size; n++) {
			buf[n] = 0xffff;
		}
	}
}

void MEMORY::load_rom_files()
{
	// load rom
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = pConfig->rom_path.Get();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!rom_loaded[0]) {
			rom_loaded[0] = emu->load_data_from_file_i(app_path, _T("IPLROM.DAT"), (uint8_t *)&m_rom[0x70000], 0x20000);
			if (rom_loaded[0]) {
				swap16_data(&m_rom[0x70000], 0x10000);
			} else {
				rom_loaded[0] = emu->load_data_from_file_i(app_path, _T("IPLROMXVI.DAT"), (uint8_t *)&m_rom[0x70000], 0x20000);
				if (rom_loaded[0]) {
					swap16_data(&m_rom[0x70000], 0x10000);
					m_is_xvi = true;
				}
			}
		}
		if (!rom_loaded[1]) {
			rom_loaded[1] = emu->load_data_from_file_i(app_path, _T("CGROM.DAT"), (uint8_t *)m_rom, 0xc0000);
			if (rom_loaded[1]) swap16_data(m_rom, 0x60000);
		}
		if (!scsi_rom_loaded[0]) {
			// read SCSI IPL ROM on EX board CZ-6BS1
			for(size_t offset = 0; offset <= 0x20; offset+=0x20) {
				scsi_rom_loaded[0] = emu->load_data_from_file_i(app_path, _T("SCSIEXROM.DAT"), (uint8_t *)&m_scsi_rom[offset >> 1], 0x2000 - offset
					, (const uint8_t *)"SCSIEX", 6, 0x44 - offset);
				if (scsi_rom_loaded[0] == 1) break;
			}
			if (scsi_rom_loaded[0]) {
				swap16_data(m_scsi_rom, 0x1000);
			}
		}
#ifdef USE_SCSI_TYPE_IN
		if (!scsi_rom_loaded[1]) {
			// read SCSI IPL ROM in main ROM  
			scsi_rom_loaded[1] = emu->load_data_from_file_i(app_path, _T("SCSIINROM.DAT"), (uint8_t *)&m_rom[0x60000], 0x2000
				, (const uint8_t *)"SCSIIN", 6, 0x24);
			if (scsi_rom_loaded[1]) {
				swap16_data(&m_rom[0x60000], 0x1000);
			}
		}
#endif
	}

	if (!rom_loaded_at_first) {
		if (!rom_loaded[0]) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("IPLROM.DAT"));
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("IPLROMXVI.DAT"));
		}
		if (!rom_loaded[1]) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("CGROM.DAT"));
		}
		if (!scsi_rom_loaded[0]) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("SCSIEXROM.DAT"));
		}
#ifdef USE_SCSI_TYPE_IN
		if (!scsi_rom_loaded[1]) {
			logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("SCSIINROM.DAT"));
		}
#endif
		rom_loaded_at_first = true;
	}
}

void MEMORY::load_sram_file()
{
	const _TCHAR *app_path, *rom_path[2];

	rom_path[0] = vm->initialize_path();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!sram_loaded) {
			sram_loaded = vm->load_data_from_file(app_path, _T("sram.dat"), (uint8_t *)&m_sram, sizeof(m_sram));
			if (sram_loaded) {
				swap16_data(m_sram, sizeof(m_sram) / sizeof(m_sram[0]));
			}
		}
	}
	if (sram_loaded) {
		// modify main ram size current setting.
		patch_sram_data();
	} else {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, _T("sram.dat"));
	}
}

void MEMORY::save_sram_file()
{
	const _TCHAR *app_path, *rom_path[2];
	_TCHAR file_path[_MAX_PATH];

	rom_path[0] = vm->initialize_path();
	rom_path[1] = vm->application_path();

	for(int i=0; i<2; i++) {
		app_path = rom_path[i];

		if (!sram_saved) {
			FILEIO fio;
			UTILITY::tcscpy(file_path, _MAX_PATH, app_path);
			UTILITY::tcscat(file_path, _MAX_PATH, _T("sram.dat"));
			if(fio.Fopen(file_path, FILEIO::WRITE_BINARY)) {
				fwrite16_data(&fio, m_sram, sizeof(m_sram) / sizeof(m_sram[0]));
				fio.Fclose();
				logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_saved, _T("sram.dat"));
				sram_saved = true;
			}
		}
	}
	if (!sram_saved) {
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_saved, _T("sram.dat"));
	}
}

void MEMORY::load_sram_file_forcely()
{
	sram_loaded = false;
	load_sram_file();
}

void MEMORY::save_sram_file_forcely()
{
	sram_saved = false;
	save_sram_file();
}

void MEMORY::clear_sram()
{
	memset(m_sram , 0xff, sizeof(m_sram));
}

void MEMORY::patch_sram_data()
{
	// patch SRAM data

	// modify main ram size
	if (!FLG_ORIG_SRAM_RAM_SIZE) {
		m_sram[4] = (m_ram_size >> 15);
		m_sram[5] = 0;
	}
}

void MEMORY::swap16_data(uint16_t *data, size_t len)
{
#ifndef USE_BIG_ENDIAN
	// convert to big endian
	for(size_t i=0; i<len; i++) {
		data[i] = swap16(data[i]);
	}
#endif
}

void MEMORY::fwrite16_data(FILEIO *fio, const uint16_t *data, size_t len)
{
	enum {
		BUFSIZE = 512
	};

	uint16_t buffer[BUFSIZE];
	for(size_t n=0; n<len; n+=BUFSIZE) {
		memcpy(buffer, data, sizeof(uint16_t) * BUFSIZE);
		swap16_data(buffer, BUFSIZE);
		fio->Fwrite(buffer, sizeof(uint16_t), BUFSIZE);
		data += BUFSIZE;
	}
}

size_t MEMORY::fread16_data(FILEIO *fio, uint16_t *data, size_t len)
{
	size_t size = fio->Fread(data, sizeof(uint16_t), len);
	swap16_data(data, size);
	return size;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data8w(addr, data, &wait);
}
uint32_t MEMORY::read_data8(uint32_t addr)
{
	int wait = 0;
	return read_data8w(addr, &wait);
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	write_data_nw(addr, data, wait, 1);
}
uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
{
	return read_data_nw(addr, wait, 1);
}

/// write the 16bit data using DMA controller
///
/// @param[in]     addr : address
/// @param[in]     data : write data
void MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, 0x10002);
}

/// read a 16bit data using DMA controller
///
/// @param[in]     addr : address
/// @return        data : read data
uint32_t MEMORY::read_dma_data16(uint32_t addr)
{
	int wait = 0;
	return read_data_nw(addr, &wait, 0x10002);
}

/// write the data using DMA controller
///
/// @param[in]     addr : address
/// @param[in]     data : write data
/// @param[in]     width : data width in bytes (1 or 2)
void MEMORY::write_dma_data_n(uint32_t addr, uint32_t data, int width)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, width | 0x10000);
}

/// read a data using DMA controller
///
/// @param[in]     addr : address
/// @param[in]     width : data width in bytes (1 or 2)
/// @return        data : read data
uint32_t MEMORY::read_dma_data_n(uint32_t addr, int width)
{
	int wait = 0;
	return read_data_nw(addr, &wait, width | 0x10000);
}

/// write the 8bit data using DMA controller
///
/// @param[in]     addr : address
/// @param[in]     data : write data
void MEMORY::write_dma_io8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, 0x10001);
}

/// read an 8bit data using DMA controller
///
/// @param[in]     addr : address
/// @return        data : read data
uint32_t MEMORY::read_dma_io8(uint32_t addr)
{
	int wait = 0;
	return read_data_nw(addr, &wait, 0x10001);
}

/// write the 16bit data using DMA controller
///
/// @param[in]     addr : address
/// @param[in]     data : write data
void MEMORY::write_dma_io16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, 0x10002);
}

/// read a 16bit data using DMA controller
///
/// @param[in]     addr : address
/// @return        data : read data
uint32_t MEMORY::read_dma_io16(uint32_t addr)
{
	int wait = 0;
	return read_data_nw(addr, &wait, 0x10002);
}

/// write the 16bit data
///
/// @param[in]     addr : address
/// @param[in]     data : write data
void MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data16w(addr, data, &wait);
}

/// write the 16bit data
///
/// @param[in]     addr : address
/// @param[in]     data : write data
/// @param[in,out] wait : current clock, this function need add spent cycles to it
void MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	write_data_nw(addr, data, wait, 2);
}

/// write the data
///
/// @param[in]     addr : the address
/// @param[in]     data : write data
/// @param[in,out] wait : wait cycle
/// @param[in]     width : data width
/// @note bit16 on width means accessing from DMA
/// @note bit17 on width means accessing from debugger
///
///	@note The data bus width is 16bits.
///	So a read/write data b15-b8 always exists at even address, and b7-b0 does at odd. 
void MEMORY::write_data_nw(uint32_t addr, uint32_t data, int *wait, int width)
{
	addr &= 0xffffff;	// 24bits
	uint32_t addrh;
	uint32_t addrio;
	m_buserr = 0;
	bool from_dmac = (width & 0x10000) != 0;
#ifdef USE_DEBUGGER
/// @note bit17 on width means accessing from debugger
	bool from_debugger = (width & 0x20000) != 0;
#endif
	bool is_supervisor = ((*p_fc & 4) != 0 || from_dmac);
	width &= 0xffff;

	uint32_t mask;
	if (width == 1) {
		if (addr & 1) {
			// odd
			mask = 0xff00;
			data &= 0x00ff;
		} else {
			// even
			mask = 0x00ff;
			data &= 0xff00;
		}
	} else {
		mask = 0x0000;
		data &= 0xffff;
	}
	addrh = MEM_ADDRH(addr);

#ifdef _DEBUG_RAM
	if (addr >= 0xff70 && addr <= 0xff7f) {
		logging->out_debugf("mw %06X = %04X", addr, data);
	}
#endif

	switch((addrh >> 15) & 0xf8) {
	case 0x00:
	case 0x08:
	case 0x10:
	case 0x18:
		// Main RAM
		if (is_supervisor) {
			// supervisor mode
			if (addrh < m_ram_size) {
				STORE_DATA(m_ram[addrh], data, mask);
			} else {
				// buserror
				m_buserr = 1;
			}
		} else {
			// user mode
			if (m_area_set_size <= addrh && addrh < m_ram_size) {
				STORE_DATA(m_ram[addrh], data, mask);
			} else {
				// buserror
				m_buserr = 1;
			}
		}
		break;
	case 0x20:
	case 0x28:
	case 0x30:
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
	case 0x60:
	case 0x68:
	case 0x70:
	case 0x78:
	case 0x80:
	case 0x88:
	case 0x90:
	case 0x98:
	case 0xa0:
	case 0xa8:
	case 0xb0:
	case 0xb8:
		// Main RAM
		if (addrh < m_ram_size) {
			STORE_DATA(m_ram[addrh], data, mask);
		} else {
			// buserror
			m_buserr = 1;
		}
		break;
	case 0xc0:
		// Graphic VRAM
		if (is_supervisor) {
			write_gvram0(addrh, data, mask);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xc8:
		// Graphic VRAM
		if (is_supervisor) {
			write_gvram1(addrh, data, mask);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xd0:
		// Graphic VRAM
		if (is_supervisor) {
			write_gvram2(addrh, data, mask);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xd8:
		// Graphic VRAM
		if (is_supervisor) {
			write_gvram3(addrh, data, mask);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xe0:
		// Text VRAM
		if (is_supervisor) {
			write_tvram(addrh, data, mask);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xe8:
		// System I/O
		if (is_supervisor) {
			addrio = ((addr >> 12) & 0x7e);
			switch(addrio) {
			case 0x00:
				// CRTC
				d_crtc->write_io_m(addr, data, mask);
				break;
			case 0x02:
				// Palette and Video Control
				d_disp->write_io_m(addr, data, mask);
				break;
			case 0x04:
				// DMAC
				d_dmac->write_io_n(addr, data, width);
				break;
			case 0x06:
				// AREA SET
				STORE_DATA(m_area_set, data, mask);
				m_area_set &= 0xff;
				AREA_SET_SIZE; // words
				break;
			case 0x08:
				// MFP
				d_mfp->write_io8(addrh, data);
				break;
			case 0x0a:
				// RTC
				d_rtc->write_io8(addrh, data);
				break;
			case 0x0c:
				// PRINTER
				d_printer->write_io8(addrh, data);
				break;
			case 0x0e:
				// SYS.PORT
				d_sysport->write_io8(addrh, data);
				break;
			case 0x10:
				// FM Synth
				d_opm->write_io8(addrh, data);
				break;
			case 0x12:
				// ADPCM Voice
				d_adpcm->write_io8(addrh, data);
				break;
			case 0x14:
				// FD
				d_fdd->write_io8(addrh, data);
				break;
			case 0x16:
				// HD
				if (pConfig->scsi_type == SCSI_TYPE_IN && ((addr & 0x20) != 0)) {
					d_scsi->write_io8(addrh, data);
				} else {
					d_sasi->write_io8(addrh, data);
				}
				break;
			case 0x18:
				// SCC
				d_scc->write_io8(addrh, data);
				break;
			case 0x1a:
				// 8255
				d_pio->write_io8(addrh, data);
				break;
			case 0x1c:
				// INT
				d_board->write_io8(addrh, data);
				break;
			case 0x1e:
				// (Ex) FPU
				m_buserr = 1;
				break;
			case 0x20:
				// (Ex) SCSI
				if (pConfig->scsi_type == SCSI_TYPE_EX) {
					if ((addr & 0x1fe0) == 0) {
						// I/O 0x00-0x1f
						d_scsi->write_io8(addrh, data);
					}
				} else {
					m_buserr = 1;
				}
				break;
			case 0x22:
			case 0x24:
			case 0x26:
			case 0x28:
			case 0x2a:
			case 0x2c:
			case 0x2e:
				// (Ex)
				m_buserr = 1;
				break;
			case 0x30:
			case 0x32:
			case 0x34:
			case 0x36:
				// Sprite/PCG/BG
				d_sp_bg->write_io_m(addr, data, mask);
				break;
			case 0x38:
			case 0x3a:
				// PCG VRAM
				STORE_DATA(m_spram[ADDRH_MASK(addrh, 0x8000)], data, mask);
				// set update flag
				u_pcg[ADDRH_MASK(addrh, 0x8000) >> 6] |= (SPRITE_BG::SP_PRW_UPD_MASK | 3);
				break;
			case 0x3c:
			case 0x3e:
				// PCG/BG VRAM
				STORE_DATA(m_spram[ADDRH_MASK(addrh, 0x8000)], data, mask);
				// set update flag
				u_pcg[ADDRH_MASK(addrh, 0x8000) >> 6] |= (SPRITE_BG::SP_PRW_UPD_MASK | 3);
				break;
			case 0x40:
			case 0x42:
			case 0x44:
			case 0x46:
			case 0x48:
			case 0x4a:
			case 0x4c:
			case 0x4e:
				// (Ex) User I/O
				m_buserr = 1;
				break;
			case 0x50:
			case 0x52:
				// SRAM 16KB
				if (m_sram_writable) {
					write_sram(addrh, data, mask);
				}
				break;
			default:
				m_buserr = 1;
				break;
			}
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xf0:
		// CGROM
		if (is_supervisor) {
			// TODO
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xf8:
		// CGROM + IPLROM
		if (is_supervisor) {
			if (m_is_xvi && (addr & 0xfe0000) == 0xfc0000) {
				m_buserr = 1;
			}
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	default:
		// buserror
		m_buserr = 1;
		break;
	}

#ifdef USE_DEBUGGER
	if (!from_debugger) {
#endif
		if (from_dmac) {
			// send BERR signal to DMAC
			d_dmac->write_signal(DMAC::SIG_BUSERR, m_buserr, 1);
		} else {
			// send BERR signal to MPU
			d_cpu->write_signal(SIG_M68K_BUSERR, m_buserr, 1);
		}
#ifdef USE_DEBUGGER
	}
#endif
	m_buserr = 0;

//#define WRITE_IO8 write_io8
//#include "memory_writeio.h"

#ifdef USE_DEBUGGER
//	bas->SetTraceBack(addr);
#endif
}

void MEMORY::write_sram(uint32_t addrh, uint32_t data, uint32_t mask)
{
	uint32_t addrs = ADDRH_MASK(addrh, 0x4000);

	if (FLG_ORIG_SRAM_CHG_BOOT_DEV) {
		// check warning
		switch(addrs) {
		case (0x18 >> 1):
			write_sram_and_check_warn(addrs, data, mask, 0xff00, CMsg::The_setting_of_boot_device_in_SRAM_was_changed);
			break;
		case (0x10 >> 1):
		case (0x12 >> 1):
			write_sram_and_check_warn(addrs, data, mask, 0xffff, CMsg::The_start_address_in_SRAM_was_changed);
			break;
		default:
			STORE_DATA(m_sram[addrs], data, mask);
			break;
		}
	} else {
		STORE_DATA(m_sram[addrs], data, mask);
	}
	patch_sram_data();
}

void MEMORY::write_sram_and_check_warn(uint32_t addrs, uint32_t data, uint32_t mask, uint16_t check_mask, int msg_id)
{
	uint16_t orig_data = (m_sram[addrs] & check_mask);
	STORE_DATA(m_sram[addrs], data, mask);
	uint16_t new_data = (m_sram[addrs] & check_mask);

	if (new_data != 0 && orig_data != new_data) {
		logging->out_log_x(LOG_WARN, CMSGV((CMsg::Id)msg_id)); 
	}
}

/// $E00000-$E7FFFF
void MEMORY::write_tvram(uint32_t addrh, uint32_t data, uint32_t mask)
{
	if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_MEN) {
		// mask bit enable
		mask |= p_crtc_regs[CRTC::CRTC_BIT_MASK];
		data &= ~p_crtc_regs[CRTC::CRTC_BIT_MASK];
	}
//	if ((addrh & 0xffff) == 0) {
//		OUT_DEBUG_TVRAM(_T("W_TVRAM: %06X Data:%04X Mask:%04X"), addrh << 1, data, mask);
//	}
	u_tvram[ADDRH_MASK(addrh, 0x20000)] |= 3;
	if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_SA) {
		// accress parallel
		if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_AP0) {
			STORE_DATA(m_tvram[ADDRH_MASK(addrh, 0x20000)], data, mask);
		}
		if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_AP1) {
			STORE_DATA(m_tvram[ADDRH_MASK(addrh, 0x20000) | 0x10000], data, mask);
		}
		if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_AP2) {
			STORE_DATA(m_tvram[ADDRH_MASK(addrh, 0x20000) | 0x20000], data, mask);
		}
		if (p_crtc_regs[CRTC::CRTC_CONTROL1] & CRTC::CONTROL1_AP3) {
			STORE_DATA(m_tvram[ADDRH_MASK(addrh, 0x20000) | 0x30000], data, mask);
		}
	} else {
		STORE_DATA(m_tvram[ADDRH_MASK(addrh, 0x80000)], data, mask);
	}
}

/// $C00000-$C7FFFF
void MEMORY::write_gvram0(uint32_t addrh, uint32_t data, uint32_t mask)
{
	uint32_t addrg;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		STORE_DATA(m_gvram[addrg], data, mask);
//		u_gvram[addrg] |= 0xf;
		OUT_DEBUG_GVRAM(_T("WG2BY-0: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		if ((addrh & 0x200) != 0) {
			// page 1
			mask = 0xff0f;
			data <<= 4;
			data &= 0x00f0;
		} else {
			// page 0
			mask = 0xfff0;
			data &= 0x000f;
		}
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
//		u_gvram[addrg] |= 0xf;
		STORE_DATA(m_gvram[addrg], data, mask);
		OUT_DEBUG_GVRAM(_T("WGT16-0: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0xf;
			OUT_DEBUG_GVRAM(_T("WG64K-0: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 0
			mask = 0xff00;
			data &= 0x00ff;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0x3;
			OUT_DEBUG_GVRAM(_T("WG256-0: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 0
			mask = 0xfff0;
			data &= 0x000f;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0x1;
			OUT_DEBUG_GVRAM(_T("WG016-0: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		}
	}
}

/// $C80000-$CFFFFF
void MEMORY::write_gvram1(uint32_t addrh, uint32_t data, uint32_t mask)
{
	uint32_t addrg;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		STORE_DATA(m_gvram[addrg], data, mask);
//		u_gvram[addrg] |= 0xf;
		OUT_DEBUG_GVRAM(_T("WG2BY-1: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		if ((addrh & 0x200) != 0) {
			// page 1
			mask = 0xff0f;
			data <<= 4;
			data &= 0x00f0;
		} else {
			// page 0
			mask = 0xfff0;
			data &= 0x000f;
		}
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
//		u_gvram[addrg] |= 0xf;
		STORE_DATA(m_gvram[addrg], data, mask);
		OUT_DEBUG_GVRAM(_T("WGT16-1: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
//			STORE_DATA(m_gvram[addrg], data, mask);
			m_buserr = 1;
//			u_gvram[addrg] |= 0xf;
			OUT_DEBUG_GVRAM(_T("WG64K-1: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 1
			mask = 0x00ff;
			data <<= 8;
			data &= 0xff00;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0xc;
			OUT_DEBUG_GVRAM(_T("WG256-1: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 1
			mask = 0xff0f;
			data <<= 4;
			data &= 0x00f0;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0x2;
			OUT_DEBUG_GVRAM(_T("WG016-1: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		}
	}
}

/// $D00000-$D7FFFF
void MEMORY::write_gvram2(uint32_t addrh, uint32_t data, uint32_t mask)
{
	uint32_t addrg;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		STORE_DATA(m_gvram[addrg], data, mask);
//		u_gvram[addrg] |= 0xf;
		OUT_DEBUG_GVRAM(_T("WG2BY-2: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		if ((addrh & 0x200) != 0) {
			// page 3
			mask = 0x0fff;
			data <<= 12;
			data &= 0xf000;
		} else {
			// page 2
			mask = 0xf0ff;
			data <<= 8;
			data &= 0x0f00;
		}
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
//		u_gvram[addrg] |= 0xf;
		STORE_DATA(m_gvram[addrg], data, mask);
		OUT_DEBUG_GVRAM(_T("WGT16-2: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
//			STORE_DATA(m_gvram[addrg], data, mask);
			m_buserr = 1;
//			u_gvram[addrg] |= 0xf;
			OUT_DEBUG_GVRAM(_T("WG64K-2: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 0
			mask = 0xff00;
			data &= 0x00ff;
//			STORE_DATA(m_gvram[addrg], data, mask);
			m_buserr = 1;
//			u_gvram[addrg] |= 0x3;
			OUT_DEBUG_GVRAM(_T("WG256-2: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 2
			mask = 0xf0ff;
			data <<= 8;
			data &= 0x0f00;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0x4;
			OUT_DEBUG_GVRAM(_T("WG016-2: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		}
	}
}

/// $D80000-$DFFFFF
void MEMORY::write_gvram3(uint32_t addrh, uint32_t data, uint32_t mask)
{
	uint32_t addrg;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		STORE_DATA(m_gvram[addrg], data, mask);
//		u_gvram[addrg] |= 0xf;
		OUT_DEBUG_GVRAM(_T("WG2BY-3: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		if ((addrh & 0x200) != 0) {
			// page 3
			mask = 0x0fff;
			data <<= 12;
			data &= 0xf000;
		} else {
			// page 2
			mask = 0xf0ff;
			data <<= 8;
			data &= 0x0f00;
		}
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
//		u_gvram[addrg] |= 0xf;
		STORE_DATA(m_gvram[addrg], data, mask);
		OUT_DEBUG_GVRAM(_T("WGT16-3: %06X %05X %04X"), addrh << 1, addrg, m_gvram[addrg]);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
//			STORE_DATA(m_gvram[addrg], data, mask);
			m_buserr = 1;
//			u_gvram[addrg] |= 0xf;
			OUT_DEBUG_GVRAM(_T("WG64K-3: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 1
			mask = 0x00ff;
			data <<= 8;
			data &= 0xff00;
//			STORE_DATA(m_gvram[addrg], data, mask);
			m_buserr = 1;
//			u_gvram[addrg] |= 0xc;
			OUT_DEBUG_GVRAM(_T("WG256-3: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			// page 3
			mask = 0x0fff;
			data <<= 12;
			data &= 0xf000;
			STORE_DATA(m_gvram[addrg], data, mask);
//			u_gvram[addrg] |= 0x8;
			OUT_DEBUG_GVRAM(_T("WG016-3: %06X %05X %04X & %04X = %04X"), addrh << 1, addrg, data, ~mask, m_gvram[addrg]);
			break;
		}
	}
}

/// read a 16bit data
///
/// @param[in]     addr : the address
/// @return data
uint32_t MEMORY::read_data16(uint32_t addr)
{
	int wait = 0;
	return read_data16w(addr, &wait);
}

/// read a 16bit data
///
/// @param[in]     addr : the address
/// @param[in,out] wait : wait cycle
/// @return data
uint32_t MEMORY::read_data16w(uint32_t addr, int *wait)
{
	return read_data_nw(addr, wait, 2);
}

/// read a data
///
/// @param[in]     addr : the address
/// @param[in,out] wait : wait cycle
/// @param[in]     width : data width
/// @note bit16 on width means accessing from DMA
/// @note bit17 on width means accessing from debugger
///
/// @return data
///	@note The data bus width is 16bits.
///	So a read/write data b15-b8 always exists at even address, and b7-b0 does at odd. 
uint32_t MEMORY::read_data_nw(uint32_t addr, int *wait, int width)
{
	uint32_t data = 0xffff;
	addr &= 0xffffff;	// 24bits
	uint32_t addrh;
	uint32_t addrio;
	m_buserr = 0;
	bool from_dmac = (width & 0x10000) != 0;
#ifdef USE_DEBUGGER
	bool from_debugger = (width & 0x20000) != 0;
#endif
	bool is_supervisor = ((*p_fc & 4) != 0 || from_dmac);
	bool is_8bits = false;
	width &= 0xffff;
	addrh = MEM_ADDRH(addr);

	switch((addrh >> 15) & 0xf8) {
	case 0x00:
	case 0x08:
		if (is_supervisor) {
			// supervisor mode
			if (m_ipl_mapping && addrh < 0x8000) {
				// Readable IPLROM area on boot
				data = m_rom[addrh + 0x78000];
			} else {
				// Main RAM
				if (addrh < m_ram_size) {
					data = m_ram[addrh];
				} else {
					// buserror
					m_buserr = 1;
				}
			}
		} else {
			// user mode
			if (m_area_set_size <= addrh && addrh < m_ram_size) {
				data = m_ram[addrh];
			} else {
				// buserror
				m_buserr = 1;
			}
		}
		break;
	case 0x10:
	case 0x18:
		// Main RAM
		if (is_supervisor) {
			// supervisor mode
			if (addrh < m_ram_size) {
				data = m_ram[addrh];
			} else {
				// buserror
				m_buserr = 1;
			}
		} else {
			// user mode
			if (m_area_set_size <= addrh && addrh < m_ram_size) {
				data = m_ram[addrh];
			} else {
				// buserror
				m_buserr = 1;
			}
		}
		break;
	case 0x20:
	case 0x28:
	case 0x30:
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
	case 0x60:
	case 0x68:
	case 0x70:
	case 0x78:
	case 0x80:
	case 0x88:
	case 0x90:
	case 0x98:
	case 0xa0:
	case 0xa8:
	case 0xb0:
	case 0xb8:
		// Main RAM
		if (addrh < m_ram_size) {
			data = m_ram[addrh];
		} else {
			// buserror
			m_buserr = 1;
		}
		break;
	case 0xc0:
		// Graphic VRAM
		if (is_supervisor) {
			data = read_gvram0(addrh);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xc8:
		// Graphic VRAM
		if (is_supervisor) {
			data = read_gvram1(addrh);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xd0:
		// Graphic VRAM
		if (is_supervisor) {
			data = read_gvram2(addrh);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xd8:
		// Graphic VRAM
		if (is_supervisor) {
			data = read_gvram3(addrh);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xe0:
		// Text VRAM
		if (is_supervisor) {
			data = m_tvram[ADDRH_MASK(addrh, 0x80000)];
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xe8:
		// System I/O
		if (is_supervisor) {
			addrio = ((addr >> 12) & 0x7e);
			switch(addrio) {
			case 0x00:
				// CRTC
				data = d_crtc->read_io16(addr);
				break;
			case 0x02:
				// Palette and Video Control
				data = d_disp->read_io16(addr);
				break;
			case 0x04:
				// DMAC
				data = d_dmac->read_io_n(addr, width);
				break;
			case 0x06:
				// AREA SET (write only)
//				data = m_area_set & 0x00ff;
				break;
			case 0x08:
				// MFP
				is_8bits = true;
				data = d_mfp->read_io8(addrh);
				break;
			case 0x0a:
				// RTC
				is_8bits = true;
				data = d_rtc->read_io8(addrh);
				break;
			case 0x0c:
				// PRINTER
				// cannot read
				break;
			case 0x0e:
				// SYS.PORT
				is_8bits = true;
				data = d_sysport->read_io8(addrh);
				break;
			case 0x10:
				// FM Synth
				is_8bits = true;
				data = d_opm->read_io8(addrh);
				break;
			case 0x12:
				// ADPCM Voice
				is_8bits = true;
				data = d_adpcm->read_io8(addrh);
				break;
			case 0x14:
				// FD
				is_8bits = true;
				data = d_fdd->read_io8(addrh);
				break;
			case 0x16:
				// HD
				if (pConfig->scsi_type == SCSI_TYPE_IN && ((addr & 0x20) != 0)) {
					is_8bits = true;
					data = d_scsi->read_io8(addrh);
				} else {
					is_8bits = true;
					data = d_sasi->read_io8(addrh);
				}
				break;
			case 0x18:
				// SCC
				is_8bits = true;
				data = d_scc->read_io8(addrh);
				break;
			case 0x1a:
				// 8255
				is_8bits = true;
				data = d_pio->read_io8(addrh);
				break;
			case 0x1c:
				// INT
				is_8bits = true;
				data = d_board->read_io8(addrh);
				break;
			case 0x1e:
				// FPU
				m_buserr = 1;
				break;
			case 0x20:
				// (Ex) SCSI
				if (pConfig->scsi_type == SCSI_TYPE_EX) {
					if ((addr & 0x1fe0) == 0) {
						// I/O 0x00-0x1f
						is_8bits = true;
						data = d_scsi->read_io8(addrh);
					} else {
						// ROM 0x20-0x1fff
						data = m_scsi_rom[ADDRH_MASK(addrh, 0x2000)];
					}
				} else {
					m_buserr = 1;
				}
				break;
			case 0x22:
			case 0x24:
			case 0x26:
			case 0x28:
			case 0x2a:
			case 0x2c:
			case 0x2e:
				// (Ex)
				m_buserr = 1;
				break;
			case 0x30:
			case 0x32:
			case 0x34:
			case 0x36:
				// Sprite/PCG/BG
				data = d_sp_bg->read_io16(addr);
				break;
			case 0x38:
			case 0x3a:
			case 0x3c:
			case 0x3e:
				// Sprite/PCG/BG VRAM
				data = m_spram[ADDRH_MASK(addrh, 0x8000)];
				break;
			case 0x40:
			case 0x42:
			case 0x44:
			case 0x46:
			case 0x48:
			case 0x4a:
			case 0x4c:
			case 0x4e:
				// (Ex) User I/O
				m_buserr = 1;
				break;
			case 0x50:
			case 0x52:
				// SRAM 16KB
				data = m_sram[ADDRH_MASK(addrh, 0x4000)];
				break;
			default:
				m_buserr = 1;
				break;
			}
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xf0:
		// CGROM
		if (is_supervisor) {
			data = m_rom[ADDRH_MASK(addrh, 0x100000)];
//			OUT_DEBUG_CGROM(_T("CGROM1: %06X %04X"), addr, data);
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	case 0xf8:
		// CGROM + IPLROM
		if (is_supervisor) {
			m_ipl_mapping = false;
			if (m_is_xvi && (addr & 0xfe0000) == 0xfc0000) {
				m_buserr = 1;
				break;
			}
			data = m_rom[ADDRH_MASK(addrh, 0x100000)];
//			if (addr < 0xfc0000) {
//				OUT_DEBUG_CGROM(_T("CGROM2: %06X %04X"), addr, data);
//			}
			if (d_board->get_intr_ack() == 7 &&
				(addr & 0xfffff0) == 0xfffff0) {
				switch(addrh & 7) {
				case 1:
					// INT1 read vector number (I/O control)
					is_8bits = true;
					data = d_board->read_external_data8(addrh);
					break;
				case 2:
					// INT2 read vector number (Ex SCSI board)
					is_8bits = true;
					// (Ex) SCSI
					if (pConfig->scsi_type == SCSI_TYPE_EX) {
						data = d_scsi->read_external_data8(addrh);
					} else {
						data = 0xff;
					}
					break;
				case 3:
					// INT3 read vector number (DMAC)
					is_8bits = true;
					data = d_dmac->read_external_data8(addrh);
					break;
				case 4:
					// INT4 read vector number (no device)
					is_8bits = true;
					data = 0xff;
					break;
				case 5:
					// INT5 read vector number (SCC)
					is_8bits = true;
					data = d_scc->read_external_data8(addrh);
					break;
				case 6:
					// INT6 read vector number (MFP)
					is_8bits = true;
					data = d_mfp->read_external_data8(addrh);
					break;
				}
				OUT_DEBUG_IVEC(_T("clk:%d IRQ READ INT%d VEC:%02X"), (int)get_current_clock(), addrh & 7, data);
			}
		} else {
			// buserror on user mode
			m_buserr = 1;
		}
		break;
	default:
		m_buserr = 1;
		break;
	}

#ifdef USE_DEBUGGER
	if (!from_debugger) {
#endif
		if (from_dmac) {
			// send BERR to DMAC
			d_dmac->write_signal(DMAC::SIG_BUSERR, m_buserr, 1);
		} else {
			// send BERR to MPU
			d_cpu->write_signal(SIG_M68K_BUSERR, m_buserr, 1);
		}
#ifdef USE_DEBUGGER
	}
#endif
	m_buserr = 0;

//	// memory mapped i/o
//#define READ_IO8 read_io8
//#include "memory_readio.h"

	if (is_8bits && width == 1) {
		if (!(addr & 1)) {
			// even
			data <<= 8;
		}
	}
	data &= 0xffff;

#ifdef _DEBUG_RAM
	if (addr >= 0xff70 && addr <= 0xff7f) {
		uint8_t ch = (data < 0x20 || data > 0x7f) ? 0x20 : data;
		logging->out_debugf("mr %04x=%02x %c",addr,data,ch);
	}
#endif

	return data;
}

/// $C00000-$C7FFFF
uint32_t MEMORY::read_gvram0(uint32_t addrh)
{
	uint32_t addrg;
	uint32_t data;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		data = m_gvram[addrg];
		OUT_DEBUG_GVRAM(_T("RG2BY: %06X %05X %04X"), addrh << 1, addrg, data);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		int page = (addrh & 0x80000) >> 16;
		if (addrh & 0x200) page += 4;
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
		data = m_gvram[addrg];
		data >>= page;
		OUT_DEBUG_GVRAM(_T("RGT16: %06X %05X %04X"), addrh << 1, addrg, data);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			OUT_DEBUG_GVRAM(_T("RG64K: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0x40000) >> 15);
			data &= 0x00ff;
			OUT_DEBUG_GVRAM(_T("RG256: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0xc0000) >> 16);
			data &= 0xf;
			OUT_DEBUG_GVRAM(_T("RG016: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		}
	}
	return data;
}

/// $C80000-$CFFFFF
uint32_t MEMORY::read_gvram1(uint32_t addrh)
{
	uint32_t addrg;
	uint32_t data;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		data = m_gvram[addrg];
		OUT_DEBUG_GVRAM(_T("RG2BY: %06X %05X %04X"), addrh << 1, addrg, data);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		int page = (addrh & 0x80000) >> 16;
		if (addrh & 0x200) page += 4;
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
		data = m_gvram[addrg];
		data >>= page;
		OUT_DEBUG_GVRAM(_T("RGT16: %06X %05X %04X"), addrh << 1, addrg, data);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			m_buserr = 1;
			OUT_DEBUG_GVRAM(_T("RG64K: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0x40000) >> 15);
			data &= 0x00ff;
			OUT_DEBUG_GVRAM(_T("RG256: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0xc0000) >> 16);
			data &= 0xf;
			OUT_DEBUG_GVRAM(_T("RG016: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		}
	}
	return data;
}

/// $D00000-$D7FFFF
uint32_t MEMORY::read_gvram2(uint32_t addrh)
{
	uint32_t addrg;
	uint32_t data;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		data = m_gvram[addrg];
		OUT_DEBUG_GVRAM(_T("RG2BY: %06X %05X %04X"), addrh << 1, addrg, data);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		int page = (addrh & 0x80000) >> 16;
		if (addrh & 0x200) page += 4;
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
		data = m_gvram[addrg];
		data >>= page;
		OUT_DEBUG_GVRAM(_T("RGT16: %06X %05X %04X"), addrh << 1, addrg, data);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			m_buserr = 1;
			OUT_DEBUG_GVRAM(_T("RG64K: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0x40000) >> 15);
			data &= 0x00ff;
			m_buserr = 1;
			OUT_DEBUG_GVRAM(_T("RG256: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0xc0000) >> 16);
			data &= 0xf;
			OUT_DEBUG_GVRAM(_T("RG016: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		}
	}
	return data;
}

/// $D80000-$DFFFFF
uint32_t MEMORY::read_gvram3(uint32_t addrh)
{
	uint32_t addrg;
	uint32_t data;

	if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_VRAM_ACCS) {
		// 16bits map mode (such as 65536colors mode)
		addrg = ADDRH_MASK(addrh, 0x80000);
		data = m_gvram[addrg];
		OUT_DEBUG_GVRAM(_T("RG2BY: %06X %05X %04X"), addrh << 1, addrg, data);

	} else if (p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_SIZE1024) {
		// 1024 x 1024 mode (16colors)
		int page = (addrh & 0x80000) >> 16;
		if (addrh & 0x200) page += 4;
		addrg = (addrh & 0x1ff) | ((addrh >> 1) & 0x3fe00);
		data = m_gvram[addrg];
		data >>= page;
		OUT_DEBUG_GVRAM(_T("RGT16: %06X %05X %04X"), addrh << 1, addrg, data);

	} else {
		switch(p_crtc_regs[CRTC::CRTC_CONTROL0] & CRTC::CONTROL0_COLOR) {
		case CRTC::CONTROL0_COLOR65536:
			// 65536colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			m_buserr = 1;
			OUT_DEBUG_GVRAM(_T("RG64K: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		case CRTC::CONTROL0_COLOR256:
			// 256colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0x40000) >> 15);
			data &= 0x00ff;
			m_buserr = 1;
			OUT_DEBUG_GVRAM(_T("RG256: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		default:
			// 16colors mode
			addrg = ADDRH_MASK(addrh, 0x80000);
			data = m_gvram[addrg];
			data >>= ((addrh & 0xc0000) >> 16);
			data &= 0xf;
			OUT_DEBUG_GVRAM(_T("RG016: %06X %05X %04X"), addrh << 1, addrg, data);
			break;
		}
	}
	return data;
}

uint32_t MEMORY::read_signal(int id)
{
	uint32_t data = 0xff;
	switch(id) {
		case SIG_SRAM_WRITABLE:
			data = m_sram_writable ? 0x31 : 0;
			break;
	}
	return data;
}

#ifdef USE_DEBUGGER

void MEMORY::debug_write_data8(int type, uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, 0x20001);
}

uint32_t MEMORY::debug_read_data8(int type, uint32_t addr)
{
	uint32_t data = debug_read_data_nw(type, addr, 1);
	if (!(addr & 1)) {
		data >>= 8;
	}
	return data & 0xff;
}

void MEMORY::debug_write_data16(int type, uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data_nw(addr, data, &wait, 0x20002);
}

uint32_t MEMORY::debug_read_data16(int type, uint32_t addr)
{
	return debug_read_data_nw(type, addr, 2);
}

uint32_t MEMORY::debug_read_data_nw(int type, uint32_t addr, int width)
{
	uint32_t data = 0xffff;
	addr &= 0xffffff;	// 24bits
	uint32_t addrh;
	uint32_t addrio;
	m_buserr = 0;
	bool is_8bits = false;
	width &= 0xffff;
	addrh = MEM_ADDRH(addr);

	switch((addrh >> 15) & 0xf8) {
	case 0x00:
	case 0x08:
		if (m_ipl_mapping && addrh < 0x8000) {
			// Readable IPLROM area on boot
			data = m_rom[addrh + 0x78000];
		} else {
			// Main RAM
			if (addrh < m_ram_size) {
				data = m_ram[addrh];
			} else {
				// buserror
				m_buserr = 1;
			}
		}
		break;
	case 0x10:
	case 0x18:
	case 0x20:
	case 0x28:
	case 0x30:
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
	case 0x60:
	case 0x68:
	case 0x70:
	case 0x78:
	case 0x80:
	case 0x88:
	case 0x90:
	case 0x98:
	case 0xa0:
	case 0xa8:
	case 0xb0:
	case 0xb8:
		// Main RAM
		if (addrh < m_ram_size) {
			data = m_ram[addrh];
		} else {
			// buserror
			m_buserr = 1;
		}
		break;
	case 0xc0:
		// Graphic VRAM
		data = read_gvram0(addrh);
		break;
	case 0xc8:
		// Graphic VRAM
		data = read_gvram1(addrh);
		break;
	case 0xd0:
		// Graphic VRAM
		data = read_gvram2(addrh);
		break;
	case 0xd8:
		// Graphic VRAM
		data = read_gvram3(addrh);
		break;
	case 0xe0:
		// Text VRAM
		data = m_tvram[ADDRH_MASK(addrh, 0x80000)];
		break;
	case 0xe8:
		// System I/O
		addrio = ((addr >> 12) & 0x7e);
		switch(addrio) {
		case 0x00:
			// CRTC
			data = d_crtc->debug_read_io16(addr);
			break;
		case 0x02:
			// Palette and Video Control
			data = d_disp->debug_read_io16(addr);
			break;
		case 0x04:
			// DMAC
			data = d_dmac->debug_read_io16(addr);
			break;
		case 0x06:
			// AREA SET
			data = m_area_set;
			break;
		case 0x08:
			// MFP
			is_8bits = true;
			data = d_mfp->debug_read_io8(addrh);
			break;
		case 0x0a:
			// RTC
			is_8bits = true;
			data = d_mfp->debug_read_io8(addrh);
			break;
		case 0x0c:
			// PRINTER
			is_8bits = true;
			data = d_printer->debug_read_io8(addrh);
			break;
		case 0x0e:
			// SYS.PORT
			is_8bits = true;
			data = d_sysport->debug_read_io8(addrh);
			break;
		case 0x10:
			// FM Synth
			is_8bits = true;
			data = d_opm->debug_read_io8(addrh);
			break;
		case 0x12:
			// ADPCM Voice
			is_8bits = true;
			data = d_adpcm->debug_read_io8(addrh);
			break;
		case 0x14:
			// FD
			is_8bits = true;
			data = d_fdd->debug_read_io8(addrh);
			break;
		case 0x16:
			// HD
			if (pConfig->scsi_type == SCSI_TYPE_IN && ((addr & 0x20) != 0)) {
				is_8bits = true;
				data = d_scsi->debug_read_io8(addrh);
			} else {
				is_8bits = true;
				data = d_sasi->debug_read_io8(addrh);
			}
			break;
		case 0x18:
			// SCC
			is_8bits = true;
			data = d_scc->debug_read_io8(addrh);
			break;
		case 0x1a:
			// 8255
			is_8bits = true;
			data = d_pio->debug_read_io8(addrh);
			break;
		case 0x1c:
			// INT
			is_8bits = true;
			data = d_board->debug_read_io8(addrh);
			break;
		case 0x1e:
			// FPU
			m_buserr = 1;
			break;
		case 0x20:
			// (Ex) SCSI
			if (pConfig->scsi_type == SCSI_TYPE_EX) {
				if ((addr & 0x1fe0) == 0) {
					// I/O 0x00-0x1f
					is_8bits = true;
					data = d_scsi->debug_read_io8(addrh);
				} else {
					// ROM 0x20-0x1fff
					data = m_scsi_rom[ADDRH_MASK(addrh, 0x2000)];
				}
			} else {
				m_buserr = 1;
			}
			break;
		case 0x22:
		case 0x24:
		case 0x26:
		case 0x28:
		case 0x2a:
		case 0x2c:
		case 0x2e:
			// (Ex)
			m_buserr = 1;
			break;
		case 0x30:
		case 0x32:
		case 0x34:
		case 0x36:
			// Sprite/PCG/BG
			data = d_sp_bg->debug_read_io16(addr);
			break;
		case 0x38:
		case 0x3a:
		case 0x3c:
		case 0x3e:
			// Sprite/PCG/BG VRAM
			data = m_spram[ADDRH_MASK(addrh, 0x8000)];
			break;
		case 0x40:
		case 0x42:
		case 0x44:
		case 0x46:
		case 0x48:
		case 0x4a:
		case 0x4c:
		case 0x4e:
			// (Ex) User I/O
			m_buserr = 1;
			break;
		case 0x50:
		case 0x52:
			// SRAM 16KB
			data = m_sram[ADDRH_MASK(addrh, 0x4000)];
			break;
		default:
			m_buserr = 1;
			break;
		}
		break;
	case 0xf0:
	case 0xf8:
		// CGROM + IPLROM
		data = m_rom[ADDRH_MASK(addrh, 0x100000)];
		break;
	default:
		m_buserr = 1;
		break;
	}

	m_buserr = 0;

	// memory mapped i/o
//#undef READ_IO8
//#undef WRITE_SIGNAL
//#undef DEBUG_READ_OK
//#define READ_IO8 debug_read_io8
//#include "memory_readio.h"

	if (is_8bits && width == 1) {
		if (!(addr & 1)) {
			// even
			data <<= 8;
		}
	}
	data &= 0xffff;

	return data;
}

void MEMORY::debug_write_data32(int type, uint32_t addr, uint32_t data)
{
	// big endien
	debug_write_data16(type, addr, (data >> 16) & 0xffff);
	debug_write_data16(type, addr + 2, data & 0xffff);
}

uint32_t MEMORY::debug_read_data32(int type, uint32_t addr)
{
	// big endien
	uint32_t val = debug_read_data16(type,addr) << 16;
	val |= debug_read_data16(type,addr + 2);
	return val;
}

#endif /* USE_DEBUGGER */

#ifdef USE_CPU_REAL_MACHINE_CYCLE
void MEMORY::latch_address(uint32_t addr, int *wait)
{

}
#endif

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{

}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
		case SIG_CPU_RESET:
			now_reset = ((data & mask) != 0);
			if (!now_reset) {
				warm_reset(false);
			}
			break;
		case SIG_SRAM_WRITABLE:
			m_sram_writable = ((data & mask) == 0x31);
			break;
	}
}

// 0:1MB 1:2MB 2:4MB 3:8MB 4:10MB
static const uint32_t c_memory_size[] = {
	1, 2, 4, 6, 8, 10, 12
};

/// (re)allocate main ram 
void MEMORY::set_main_ram()
{
	if (pConfig->main_ram_size_num > 6) pConfig->main_ram_size_num = 0;
	uint32_t size = c_memory_size[pConfig->main_ram_size_num] * 1024 * 1024 / (uint32_t)sizeof(uint16_t);
	set_main_ram(size);
}

void MEMORY::set_main_ram(uint32_t size)
{
	if (m_ram_size != size) {
		if (m_ram != NULL) {
			delete [] m_ram;
			m_ram_size = 0;
			m_ram = NULL;
		}
		if (size > 0) {
			m_ram = new uint16_t[size];
			m_ram_size = size;
		}
	}
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void MEMORY::event_callback(int event_id, int err)
{
}

// ----------------------------------------------------------------------------

#if 0
void MEMORY::event_vline(int v, int clock)
{
	for(int i=0; i<256; i++) {
		if (u_pcg[i]) u_pcg[i]--;
	}
}
#endif

// ----------------------------------------------------------------------------

void MEMORY::update_config()
{
}

// ----------------------------------------------------------------------------

void MEMORY::save_state(FILEIO *fio)
{
	struct vm_state_st *vm_state = NULL;

	// save header
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st));
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	// save values
	fwrite16_data(fio, m_gvram, sizeof(m_gvram) / sizeof(m_gvram[0]));
	fwrite16_data(fio, m_tvram, sizeof(m_tvram) / sizeof(m_tvram[0]));
	fwrite16_data(fio, m_spram, sizeof(m_spram) / sizeof(m_spram[0]));
	fwrite16_data(fio, m_sram, sizeof(m_sram) / sizeof(m_sram[0]));

	fio->FputUint32_LE(m_pc);
	fio->FputUint32_LE(m_pc_prev);
	fio->FputUint16_LE(m_area_set);
	fio->FputUint8(m_ipl_mapping ? 1 : 0);
	fio->FputUint8(m_sram_writable ? 1 : 0);

	fio->FputUint8(m_is_xvi ? 1 : 0);
	fio->FputUint8(m_buserr);
	fio->Fsets(0, sizeof(vm_state->reserved));
	fio->Fwrite("main_ram\0\0", sizeof(vm_state->signature), 1);

	fio->FputUint8((uint8_t)emu->get_parami(VM::ParamMainRamSizeNum));
	fio->FputUint8(pConfig->main_ram_size_num);
	fio->FputUint32_LE(m_ram_size);

	fwrite16_data(fio, m_ram, m_ram_size);
}

bool MEMORY::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st *vm_state = NULL;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	// load values
	fread16_data(fio, m_gvram, sizeof(m_gvram) / sizeof(m_gvram[0]));
	fread16_data(fio, m_tvram, sizeof(m_tvram) / sizeof(m_tvram[0]));
	fread16_data(fio, m_spram, sizeof(m_spram) / sizeof(m_spram[0]));
	fread16_data(fio, m_sram, sizeof(m_sram) / sizeof(m_sram[0]));

	m_pc = fio->FgetUint32_LE();
	m_pc_prev = fio->FgetUint32_LE();
	m_area_set = fio->FgetUint16_LE();
	AREA_SET_SIZE; // words

	m_ipl_mapping = fio->FgetUint8() != 0;
	m_sram_writable = fio->FgetUint8() != 0;

	m_is_xvi = fio->FgetUint8() != 0;
	m_buserr = fio->FgetUint8();

	fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);
	fio->Fseek(sizeof(vm_state->signature), FILEIO::SEEKCUR);

	emu->set_parami(VM::ParamMainRamSizeNum, fio->FgetUint8());
	pConfig->main_ram_size_num = fio->FgetUint8();
	uint32_t new_ram_size = fio->FgetUint32_LE();

	set_main_ram(new_ram_size);

	fread16_data(fio, m_ram, new_ram_size);

	// set update flag
	memset(u_tvram , 0x03, sizeof(u_tvram));
	memset(u_pcg, (SPRITE_BG::SP_PRW_UPD_MASK | 3), sizeof(u_pcg));

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
void MEMORY::set_debugger_console(DebuggerConsole *dc)
{
//	this->dc = dc;
//	if (bas) bas->SetDebuggerConsole(dc);
}

uint32_t MEMORY::debug_physical_addr_mask(int type)
{
	uint32_t data = 0;
	switch(type) {
	case 0:
		data = 0x7fff;
		break;
	default:
		break;
	}
	return data;
}

bool MEMORY::debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	bool exist = true;
	switch(type) {
	case 0:
		UTILITY::tcscpy(buffer, buffer_len, _T("RAM"));
		break;
	default:
		exist = false;
		break;
	}
	return exist;
}

enum en_bank_kind {
	BANK_NONE = 0,
	BANK_MAIN_RAM_SO,
	BANK_MAIN_RAM_SU,
	BANK_GRAPHIC_VRAM,
	BANK_TEXT_VRAM,
	BANK_SPRITE_VRAM,
	BANK_SRAM,
	BANK_IPL_ROM,
	BANK_CG_ROM,
	BANK_IO_CRTC,
	BANK_IO_VIDEO,
	BANK_IO_DMAC,
	BANK_IO_AREASET,
	BANK_IO_MFP,
	BANK_IO_RTC,
	BANK_IO_PRINTER,
	BANK_IO_SYSPORT,
	BANK_IO_OPM,
	BANK_IO_ADPCM,
	BANK_IO_FDC,
	BANK_IO_SASI,
	BANK_IO_SCSI_IN,
	BANK_SCSI_IN_ROM,
	BANK_IO_SCSI_EX,
	BANK_SCSI_EX_ROM,
	BANK_IO_SCC,
	BANK_IO_8255,
	BANK_IO_INT,
	BANK_IO_FPU,
	BANK_IO_SPRITE,
	BANK_IO_UNKNOWN
};

static const _TCHAR *c_bank_desc[] = {
	_T("(no assign)"),
	_T("Main RAM (S)"),
	_T("Main RAM (S,U)"),
	_T("Graphic VRAM"),
	_T("Text VRAM"),
	_T("Sprite VRAM"),
	_T("SRAM"),
	_T("IPL ROM"),
	_T("CG ROM"),
	_T("CRTC I/O"),
	_T("VIDEO CTRL I/O"),
	_T("DMAC I/O"),
	_T("AREA SET I/O"),
	_T("MFP I/O"),
	_T("RTC I/O"),
	_T("PRINTER I/O"),
	_T("SYSTEM PORT I/O"),
	_T("FM OPM I/O"),
	_T("ADPCM I/O"),
	_T("FDC I/O"),
	_T("HDC(SASI) I/O"),
	_T("HDC(In SCSI) I/O"),
	_T("In SCSI ROM"),
	_T("HDC(Ex SCSI) I/O"),
	_T("Ex SCSI ROM"),
	_T("SCC I/O"),
	_T("I8255 I/O"),
	_T("Interrupt I/O"),
	_T("FPU I/O"),
	_T("Sprite I/O"),
	_T("(unknown)"),
	NULL
};

#define SET_BANK_KIND(read, write) (write << 16 | read)

uint32_t MEMORY::debug_read_bank(uint32_t addr)
{
	uint32_t data = 0;
	addr &= 0xffffff;	// 24bits
	uint32_t addrh;
	uint32_t addrio;
	addrh = MEM_ADDRH(addr);

	switch((addrh >> 15) & 0xf8) {
	case 0x00:
	case 0x08:
	case 0x10:
	case 0x18:
	case 0x20:
	case 0x28:
	case 0x30:
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
	case 0x60:
	case 0x68:
	case 0x70:
	case 0x78:
	case 0x80:
	case 0x88:
	case 0x90:
	case 0x98:
	case 0xa0:
	case 0xa8:
	case 0xb0:
	case 0xb8:
		if (m_ipl_mapping && addrh < 0x8000) {
			// Readable IPLROM area on boot
			data = SET_BANK_KIND(BANK_IPL_ROM, BANK_NONE);
		} else {
			// Main RAM
			if (addrh < m_area_set_size && addrh < m_ram_size) {
				data = SET_BANK_KIND(BANK_MAIN_RAM_SO, BANK_MAIN_RAM_SO);
			} else if (addrh < m_ram_size) {
				data = SET_BANK_KIND(BANK_MAIN_RAM_SU, BANK_MAIN_RAM_SU);
			}
		}
		break;
	case 0xc0:
	case 0xc8:
	case 0xd0:
	case 0xd8:
		// Graphic VRAM
		data = SET_BANK_KIND(BANK_GRAPHIC_VRAM, BANK_GRAPHIC_VRAM);
		break;
	case 0xe0:
		// Text VRAM
		data = SET_BANK_KIND(BANK_TEXT_VRAM, BANK_TEXT_VRAM);
		break;
	case 0xe8:
		// System I/O
		addrio = ((addr >> 12) & 0x7e);
		switch(addrio) {
		case 0x00:
			// CRTC
			data = SET_BANK_KIND(BANK_IO_CRTC, BANK_IO_CRTC);
			break;
		case 0x02:
			// Palette and Video
			data = SET_BANK_KIND(BANK_IO_VIDEO, BANK_IO_VIDEO);
			break;
		case 0x04:
			// DMAC
			data = SET_BANK_KIND(BANK_IO_DMAC, BANK_IO_DMAC);
			break;
		case 0x06:
			// AREA SET
			data = SET_BANK_KIND(BANK_NONE, BANK_IO_AREASET);
			break;
		case 0x08:
			// MFP
			data = SET_BANK_KIND(BANK_IO_MFP, BANK_IO_MFP);
			break;
		case 0x0a:
			// RTC
			data = SET_BANK_KIND(BANK_IO_RTC, BANK_IO_RTC);
			break;
		case 0x0c:
			// PRINTER
			data = SET_BANK_KIND(BANK_IO_PRINTER, BANK_IO_PRINTER);
			break;
		case 0x0e:
			// SYS.PORT
			data = SET_BANK_KIND(BANK_IO_SYSPORT, BANK_IO_SYSPORT);
			break;
		case 0x10:
			// FM Synth
			data = SET_BANK_KIND(BANK_IO_OPM, BANK_IO_OPM);
			break;
		case 0x12:
			// Voice
			data = SET_BANK_KIND(BANK_IO_ADPCM, BANK_IO_ADPCM);
			break;
		case 0x14:
			// FD
			data = SET_BANK_KIND(BANK_IO_FDC, BANK_IO_FDC);
			break;
		case 0x16:
			// HD
			if (pConfig->scsi_type == SCSI_TYPE_IN && ((addr & 0x20) != 0)) {
				data = SET_BANK_KIND(BANK_IO_SCSI_IN, BANK_IO_SCSI_IN);
			} else {
				data = SET_BANK_KIND(BANK_IO_SASI, BANK_IO_SASI);
			}
			break;
		case 0x18:
			// SCC
			data = SET_BANK_KIND(BANK_IO_SCC, BANK_IO_SCC);
			break;
		case 0x1a:
			// 8255
			data = SET_BANK_KIND(BANK_IO_8255, BANK_IO_8255);
			break;
		case 0x1c:
			// INT
			data = SET_BANK_KIND(BANK_IO_INT, BANK_IO_INT);
			break;
		case 0x1e:
			// FPU
			data = SET_BANK_KIND(BANK_IO_FPU, BANK_IO_FPU);
			break;
		case 0x20:
			// (Ex) SCSI
			if (pConfig->scsi_type == SCSI_TYPE_EX) {
				if ((addr & 0x1fe0) == 0) {
					// I/O 0x00-0x1f
					data = SET_BANK_KIND(BANK_IO_SCSI_EX, BANK_IO_SCSI_EX);
				} else {
					// ROM 0x20-0x1fff
					data = SET_BANK_KIND(BANK_SCSI_EX_ROM, BANK_NONE);
				}
			} else {
				data = SET_BANK_KIND(BANK_NONE, BANK_NONE);
			}
			break;
		case 0x30:
		case 0x32:
		case 0x34:
		case 0x36:
			// Sprite
			data = SET_BANK_KIND(BANK_IO_SPRITE, BANK_IO_SPRITE);
			break;
		case 0x38:
		case 0x3a:
		case 0x3c:
		case 0x3e:
			// Sprite VRAM
			data = SET_BANK_KIND(BANK_SPRITE_VRAM, BANK_SPRITE_VRAM);
			break;
		case 0x50:
		case 0x52:
			// SRAM 16KB
			data = SET_BANK_KIND(BANK_SRAM, (m_sram_writable ? BANK_SRAM : BANK_NONE));
			break;
		default:
			break;
		}
		break;
	case 0xf0:
		// CGROM
		data = SET_BANK_KIND(BANK_CG_ROM, BANK_NONE);
		break;
	case 0xf8:
		if (addr < 0xfc0000) {
			// CGROM
			data = SET_BANK_KIND(BANK_CG_ROM, BANK_NONE);
		} else if (addr < 0xfe0000) {
			if (pConfig->scsi_type == SCSI_TYPE_IN) {
				// SCSI ROM
				data = SET_BANK_KIND(BANK_SCSI_IN_ROM, BANK_NONE);
			} else {
				// reserved area
				data = SET_BANK_KIND(BANK_NONE, BANK_NONE);
			}
		} else {
			// IPLROM
			data = SET_BANK_KIND(BANK_IPL_ROM, BANK_NONE);
		}
		break;
	default:
		break;
	}
	return data;
}

void MEMORY::debug_memory_map_info(DebuggerConsole *dc)
{
	uint32_t prev_addr = 0;
	uint32_t prev_data = 0;
	uint32_t end_addr = 0x1000000;
	uint32_t inc_addr = 16;

	for(uint32_t addr=0; addr <= end_addr; addr+=inc_addr) {
		uint32_t data = debug_read_bank(addr);
		if (addr == 0) {
			prev_data = data;
		}
		if (data != prev_data || addr == end_addr) {
			dc->Printf(_T("%06X - %06X : Read:"), prev_addr, addr-1);
			print_memory_bank(prev_data, false, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Print(_T("  Write:"), false);
			print_memory_bank(prev_data, true, dc->GetBuffer(true), dc->GetBufferSize());
			dc->Out(false);
			dc->Cr();
			prev_addr = addr;
			prev_data = data;
		}
	}
}

void MEMORY::print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len)
{
	const _TCHAR *str;
	if (w) {
		data >>= 16;
	}
	data &= 0xffff;
	if (data >= BANK_IO_UNKNOWN) {
		data = BANK_IO_UNKNOWN;
	}
	str = c_bank_desc[data];
	UTILITY::stprintf(buffer, buffer_len, _T("%-15s"), str); 
}

bool MEMORY::debug_write_reg(int type, const _TCHAR *reg, uint32_t data)
{
	return false;
}

static const struct st_debug_sram_map {
	uint32_t addr;
	uint32_t size;
	const _TCHAR *desc;
} c_debug_sram_map[] = {
	{ 0x0008, 4, _T("Main memory size") },
	{ 0x000c, 4, _T("ROM start up address") },
	{ 0x0010, 4, _T("SRAM start up address") },
	{ 0x0014, 4, _T("Spent time from powered on by alarm to off in minutes") },
	{ 0x0018, 2, _T("Boot device") },
	{ 0x001a, 2, _T("RS-232C configuration") },
	{ 0x001c, 1, _T("Status of keyboard LED when power off") },
	{ 0x001d, 1, _T("Screen mode when power on") },
	{ 0x001e, 4, _T("Behavior when power on by alarm") },
	{ 0x0022, 4, _T("Time of alarm") },
	{ 0x0026, 1, _T("Alarm on/off") },
	{ 0x0027, 1, _T("Control TV using OPT.2 key") },
	{ 0x0028, 1, _T("Contrast of monitor") },
	{ 0x0029, 1, _T("Eject FD media when power off") },
	{ 0x002a, 1, _T("Control code to send to TV when power off") },
	{ 0x002b, 1, _T("Key layout of keyboard") },
	{ 0x002c, 1, _T("Char font of calculator") },
	{ 0x002d, 1, _T("SRAM Usage") },
	{ 0x002e, 2, _T("Text palette #0") },
	{ 0x0030, 2, _T("Text palette #1") },
	{ 0x0032, 2, _T("Text palette #2") },
	{ 0x0034, 2, _T("Text palette #3") },
	{ 0x0036, 2, _T("Text palette #4 - #7") },
	{ 0x0038, 2, _T("Text palette #8 - #15") },
	{ 0x003a, 1, _T("Time of beginning key repeat") },
	{ 0x003b, 1, _T("Key repeat time") },
	{ 0x003c, 4, _T("Timeout period of printer") },
	{ 0x0040, 4, _T("Operating minutes after initialized SRAM") },
	{ 0x0044, 4, _T("Times of turning off the power switch after initialized SRAM") },
	{ 0, 0, NULL }
};

bool MEMORY::debug_write_reg(int type, uint32_t reg_num, uint32_t data)
{
	switch(type) {
	case 1:
		for(int i=0; c_debug_sram_map[i].desc; i++) {
			uint32_t addr = c_debug_sram_map[i].addr;
			if (reg_num != addr) continue;
			switch(c_debug_sram_map[i].size) {
			case 4: // dword
				addr >>= 1;
				m_sram[addr] = (data >> 16);
				m_sram[addr + 1] = (data & 0xffff);
				break;
			case 2: // word
				addr >>= 1;
				m_sram[addr] = (data & 0xffff);
				break;
			default: // byte
				if (addr & 1) {
					addr >>= 1;
					m_sram[addr] = (data & 0xff) | (m_sram[addr] & 0xff00);
				} else {
					addr >>= 1;
					m_sram[addr] = ((data & 0xff) << 8) | (m_sram[addr] & 0xff);
				}
				break;
			}
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

void MEMORY::debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len)
{
	_TCHAR unit[16];
	buffer[0] = _T('\0');
	switch(type) {
	case 1:
		UTILITY::tcscat(buffer, buffer_len, _T("SRAM:\n"));
		for(int i=0; c_debug_sram_map[i].desc; i++) {
			uint32_t addr = c_debug_sram_map[i].addr;
			uint32_t data;
			switch(c_debug_sram_map[i].size) {
			case 4: // dword
				data = m_sram[addr >> 1];
				data <<= 16;
				data |= m_sram[(addr >> 1) + 1];
				UTILITY::tcscpy(unit, 16, _T("%08X"));
				break;
			case 2: // word
				data = m_sram[addr >> 1];
				UTILITY::tcscpy(unit, 16, _T("    %04X"));
				break;
			default: // byte
				data = m_sram[addr >> 1];
				if (!(addr & 1)) data >>= 8;
				data &= 0xff;
				UTILITY::tcscpy(unit, 16, _T("      %02X"));
				break;
			}
			const _TCHAR *desc = c_debug_sram_map[i].desc;
			UTILITY::sntprintf(buffer, buffer_len, _T(" %04X:"), addr);
			UTILITY::sntprintf(buffer, buffer_len, unit, data);
			UTILITY::tcscat(buffer, buffer_len, _T(" : "));
			UTILITY::tcscat(buffer, buffer_len, desc);
			UTILITY::tcscat(buffer, buffer_len, _T("\n"));
		}
		break;
	default:
		break;
	}
}

int MEMORY::get_debug_graphic_memory_size(int num, int type, int *width, int *height)
{
	return d_disp->get_debug_graphic_memory_size(num, type, width, height);
}

bool MEMORY::debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len)
{
	return d_disp->debug_graphic_type_name(type, buffer, buffer_len);
}

bool MEMORY::debug_draw_graphic(int type, int width, int height, scrntype *buffer)
{
	return d_disp->debug_draw_graphic(type, width, height, buffer);
}

bool MEMORY::debug_dump_graphic(int type, int width, int height, uint16_t *buffer)
{
	return d_disp->debug_dump_graphic(type, width, height, buffer);
}

bool MEMORY::debug_basic_is_supported()
{
	return false;
}

uint32_t MEMORY::debug_basic_get_line_number_ptr()
{
//	return bas->GetLineNumberPtr();
	return 0;
}

uint32_t MEMORY::debug_basic_get_line_number()
{
//	return bas->GetLineNumber();
	return 0;
}

void MEMORY::debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names)
{
//	bas->PrintVariable(name_cnt, names);
}

void MEMORY::debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line)
{
//	if (st_line == -1 || st_line == ed_line) {
//		bas->PrintCurrentLine(st_line);
//	} else {
//		bas->PrintList(st_line, ed_line, 0);
//	}
}

void MEMORY::debug_basic_trace_onoff(DebuggerConsole *dc, bool enable)
{
//	bas->TraceOnOff(enable);
}

void MEMORY::debug_basic_trace_current()
{
//	bas->PrintCurrentTrace();
}

void MEMORY::debug_basic_trace_back(DebuggerConsole *dc, int num)
{
//	bas->PrintTraceBack(num);
}

void MEMORY::debug_basic_command(DebuggerConsole *dc)
{
//	bas->PrintCommandList();
}

void MEMORY::debug_basic_error(DebuggerConsole *dc, int num)
{
//	bas->PrintError(num);
}

bool MEMORY::debug_basic_check_break_point(uint32_t line, int len)
{
//	return bas->IsCurrentLine(line, line + len);
	return false;
}

#endif
