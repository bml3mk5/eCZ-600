/** @file memory.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ memory ]
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;
#ifdef USE_DEBUGGER
//class L3Basic;
#endif

#define BANK_SIZE	4

#define MEM_ADDRH(x) ((x) >> 1)

/**
	@brief Memory access
*/
class MEMORY : public DEVICE
{
public:
	/// @brief signals on MEMORY
	enum SIG_MEMORY_IDS {
		SIG_MEMORY_DISP		= 0,
		SIG_MEMORY_VSYNC,
		SIG_MEMORY_PIA_PA,
		SIG_SRAM_WRITABLE,
	};

private:
	DEVICE *d_cpu;
	DEVICE *d_mfp;
	DEVICE *d_crtc;
	DEVICE *d_dmac;
	DEVICE *d_scc;
	DEVICE *d_sysport;
	DEVICE *d_disp;
	DEVICE *d_sp_bg;
	DEVICE *d_opm;
	DEVICE *d_adpcm;
	DEVICE *d_fdc, *d_fdd;
	DEVICE *d_sasi;
	DEVICE *d_pio;
	DEVICE *d_rtc;
	DEVICE *d_printer;
	DEVICE *d_comm;
	DEVICE *d_board;

	uint32_t *p_fc;							///< function code on MPU

	uint16_t m_area_set;					///< set supervisor area
	uint32_t m_area_set_size;				///< set supervisor area (real size (word))

	uint16_t m_rom[MEM_ADDRH(0x100000)];	///< CGROM 768KB + (reserved) + IPLROM 128KB

	uint16_t *m_ram;						///< main RAM
	uint32_t  m_ram_size;					///< main RAM size (word)

	uint16_t m_gvram[MEM_ADDRH(0x80000)];	///< Graphic VRAM 512KB b15-b12:G3, b11-b8:G2, b7-b4:G1, b3-b0:G0
//	uint8_t  u_gvram[MEM_ADDRH(0x80000)];	///< Graphic VRAM update flag
	uint16_t m_tvram[MEM_ADDRH(0x80000)];	///< Text VRAM 128 * 4 = 512KB
	uint8_t  u_tvram[MEM_ADDRH(0x20000)];	///< Text VRAM update flag

	uint16_t m_spram[MEM_ADDRH(0x8000)];	///< Sprite/PCG/BG RAM 32KB
//	uint8_t  u_spram[MEM_ADDRH(0x8000)];	///< Sprite/PCG/BG update flag
	uint16_t u_pcg[256];					///< PCG(16x16) update flag
//	uint16_t u_bg[2][64 * 64 * 2];			///< BG(Area0, 1) update flag

	uint16_t m_sram[MEM_ADDRH(0x4000)];		///< SRAM 16KB

	uint32_t m_pc, m_pc_prev;

	bool  m_ipl_mapping;	// mapping $ffxxxx -> $00xxxx after reset

	bool  m_sram_writable;	// I/O $e8e00d

	bool  m_is_xvi;

	uint16_t *p_crtc_regs;

	bool  rom_loaded[2];
	bool  rom_loaded_at_first;

	bool sram_loaded;
	bool sram_saved;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint16_t m_gvram[MEM_ADDRH(0x80000)];	///< Graphic VRAM 512KB
		uint16_t m_tvram[MEM_ADDRH(0x80000)];	///< Text VRAM 128 * 4 = 512KB
		uint16_t m_spram[MEM_ADDRH(0x8000)];	///< Sprite/PCG/BG RAM 32KB
		uint16_t m_sram[MEM_ADDRH(0x4000)];		///< SRAM 16KB

		uint32_t m_pc;
		uint32_t m_pc_prev;
		uint16_t m_area_set;					///< set supervisor area
		uint8_t  m_ipl_mapping;
		uint8_t  m_sram_writable;
		uint8_t  m_is_xvi;
		char reserved[3];

		char signature[10];
		uint8_t   next_ram_size_num;
		uint8_t   main_ram_size_num;
		uint32_t  m_ram_size;					///< main RAM size (word)
	};
#pragma pack()

	static void clear_main_memory(int method, uint16_t *buf, size_t size);
	static void clear_graphic_memory(int method, uint16_t *buf, size_t size);
	static void clear_ram(uint16_t *buf, size_t size);

	void load_rom_files();

	void load_sram_file();
	void save_sram_file();
	void clear_sram();
	void patch_sram_data();

	void swap16_data(uint16_t *data, size_t len);
	void fwrite16_data(FILEIO *fio, const uint16_t *data, size_t len);
	size_t fread16_data(FILEIO *fio, uint16_t *data, size_t len);
	void set_main_ram();
	void set_main_ram(uint32_t size);

	inline void write_data_nw(uint32_t addr, uint32_t data, int *wait, int width);
	inline uint32_t read_data_nw(uint32_t addr, int *wait, int width);
	inline uint32_t debug_read_data_nw(int type, uint32_t addr, int width);

	inline void write_sram(uint32_t addrh, uint32_t data, uint32_t mask);
	inline void write_sram_and_check_warn(uint32_t addrh, uint32_t data, uint32_t mask, uint16_t check_mask, int msg_id);

	inline void write_tvram(uint32_t addrh, uint32_t data, uint32_t mask);
	inline void write_gvram0(uint32_t addrh, uint32_t data, uint32_t mask);
	inline void write_gvram1(uint32_t addrh, uint32_t data, uint32_t mask);
	inline void write_gvram2(uint32_t addrh, uint32_t data, uint32_t mask);
	inline void write_gvram3(uint32_t addrh, uint32_t data, uint32_t mask);
	inline uint32_t read_gvram(uint32_t addrh);

#ifdef USE_DEBUGGER
//	DebuggerConsole *dc;
	DEVICE *d_debugger;
//	L3Basic *bas;
#endif

public:
	MEMORY(VM* parent_vm, EMU* parent_emu, const char* identifier);
	~MEMORY();

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);

	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);

#ifdef USE_CPU_REAL_MACHINE_CYCLE
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
	void latch_address(uint32_t addr, int *wait);
#endif

	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);

	void write_data24(uint32_t addr, uint32_t data) {
		write_data16(addr, data >> 8);
		write_data8(addr + 2, data & 0xff);
	}
	uint32_t read_data24(uint32_t addr) {
		return (read_data16(addr) << 8) | read_data8(addr + 2);
	}
	void write_data32(uint32_t addr, uint32_t data) {
		write_data16(addr, data >> 16);
		write_data16(addr + 2, data & 0xffff);
	}
	uint32_t read_data32(uint32_t addr) {
		return (read_data16(addr) << 16) | read_data16(addr + 2);
	}

	void write_data16w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data16w(uint32_t addr, int *wait);

	void write_data24w(uint32_t addr, uint32_t data, int *wait) {
		write_data16w(addr, data >> 16, wait);
		write_data8(addr + 2, data & 0xff);
	}
	uint32_t read_data24w(uint32_t addr, int *wait) {
		return (read_data16w(addr, wait) << 8) | read_data8(addr + 2);
	}
	void write_data32w(uint32_t addr, uint32_t data, int *wait) {
		write_data16w(addr, data >> 16, wait);
		write_data16(addr + 2, data & 0xffff);
	}
	uint32_t read_data32w(uint32_t addr, int *wait) {
		return (read_data16w(addr, wait) << 16) | read_data16(addr + 2);
	}

	void write_dma_data_n(uint32_t addr, uint32_t data, int width);
	uint32_t read_dma_data_n(uint32_t addr, int width);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io16(uint32_t addr);

	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);

	// unique function
	void set_context_cpu(DEVICE* device) {
		d_cpu = device;
	}
	void set_context_mfp(DEVICE* device) {
		d_mfp = device;
	}
	void set_context_crtc(DEVICE* device) {
		d_crtc = device;
	}
	void set_context_dmac(DEVICE* device) {
		d_dmac = device;
	}
	void set_context_scc(DEVICE* device) {
		d_scc = device;
	}
	void set_context_sysport(DEVICE* device) {
		d_sysport = device;
	}
	void set_context_display(DEVICE* device) {
		d_disp = device;
	}
	void set_context_opm(DEVICE* device) {
		d_opm = device;
	}
	void set_context_adpcm(DEVICE* device) {
		d_adpcm = device;
	}
	void set_context_fdc(DEVICE* device) {
		d_fdc = device;
	}
	void set_context_fdd(DEVICE* device) {
		d_fdd = device;
	}
	void set_context_sasi(DEVICE* device) {
		d_sasi = device;
	}
	void set_context_pio(DEVICE* device) {
		d_pio = device;
	}
	void set_context_sprite_bg(DEVICE* device) {
		d_sp_bg = device;
	}
	void set_context_rtc(DEVICE* device) {
		d_rtc = device;
	}
	void set_context_printer(DEVICE* device) {
		d_printer = device;
	}
	void set_context_comm(DEVICE* device) {
		d_comm = device;
	}
	void set_context_board(DEVICE* device) {
		d_board = device;
	}
	void set_crtc_regs_ptr(uint16_t* regs) {
		p_crtc_regs = regs;
	}

	uint16_t* get_rom_ptr() {
		return m_rom;
	}
	uint16_t* get_tvram_ptr() {
		return m_tvram;
	}
	uint8_t* get_tvram_flag_ptr() {
		return u_tvram;
	}
	uint16_t* get_gvram_ptr() {
		return m_gvram;
	}
//	uint8_t* get_gvram_flag_ptr() {
//		return u_gvram;
//	}
	uint16_t* get_spram_ptr() {
		return m_spram;
	}
//	uint8_t* get_spram_flag_ptr() {
//		return u_spram;
//	}
	uint16_t* get_pcg_flag_ptr() {
		return u_pcg;
	}
//	uint16_t* get_bg_flag_ptr(int num) {
//		return u_bg[num];
//	}
	uint16_t* get_sram_ptr() {
		return m_sram;
	}
	uint8_t get_sram8(int addr) const {
		if (addr & 1) {
			addr >>= 1;
			return (m_sram[addr] & 0xff);
		} else {
			addr >>= 1;
			return ((m_sram[addr] >> 8) & 0xff);
		}
	}
	uint16_t get_sram16(int addr) const {
		addr >>= 1;
		return m_sram[addr];
	}
	uint32_t get_sram32(int addr) const {
		addr >>= 1;
		return ((uint32_t)m_sram[addr] << 16) | m_sram[addr + 1];
	}
	void set_sram8(int addr, uint32_t data) {
		if (addr & 1) {
			addr >>= 1;
			m_sram[addr] = (data & 0xff) | (m_sram[addr] & 0xff00);
		} else {
			addr >>= 1;
			m_sram[addr] = ((data & 0xff) << 8) | (m_sram[addr] & 0x00ff);
		}
	}
	void set_sram16(int addr, uint32_t data) {
		addr >>= 1;
		m_sram[addr] = (data & 0xffff);
	}
	void set_sram32(int addr, uint32_t data) {
		addr >>= 1;
		m_sram[addr] = ((data >> 16) & 0xffff);
		m_sram[addr + 1] = (data & 0xffff);
	}
	void load_sram_file_forcely();
	void save_sram_file_forcely();
	void set_fc_ptr(uint32_t *fc) {
		p_fc = fc;
	}

	void event_callback(int event_id, int err);
//	void event_vline(int v, int clock);
	void update_config();

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);


#ifdef USE_DEBUGGER
	void set_debugger_console(DebuggerConsole *dc);
	DEVICE *get_debugger() {
		return d_debugger;
	}
	void set_debugger(DEVICE *dbg) {
		d_debugger = dbg;
	}
	DEVICE *get_context_mem() const {
		return (DEVICE *)this;
	}

	void debug_write_data8(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data8(int type, uint32_t addr);
	void debug_write_data16(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data16(int type, uint32_t addr);
	void debug_write_data32(int type, uint32_t addr, uint32_t data);
	uint32_t debug_read_data32(int type, uint32_t addr);

	uint32_t debug_physical_addr_mask(int type);
	bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);

	uint32_t debug_read_bank(uint32_t addr);
	void debug_memory_map_info(DebuggerConsole *dc);
	void print_memory_bank(uint32_t data, bool w, _TCHAR *buffer, size_t buffer_len);

	bool debug_write_reg(int type, const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(int type, uint32_t reg_num, uint32_t data);
	void debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len);

	int  get_debug_graphic_memory_size(int num, int type, int *width, int *height);
	bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);
	bool debug_dump_graphic(int type, int width, int height, uint16_t *buffer);

	uint32_t debug_basic_get_line_number_ptr();
	uint32_t debug_basic_get_line_number();

	bool debug_basic_is_supported();
	void debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names);
	void debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line);
	void debug_basic_trace_onoff(DebuggerConsole *dc, bool enable);
	void debug_basic_trace_current();
	void debug_basic_trace_back(DebuggerConsole *dc, int num);
	void debug_basic_command(DebuggerConsole *dc);
	void debug_basic_error(DebuggerConsole *dc, int num);

	bool debug_basic_check_break_point(uint32_t line, int len);

#endif

};

#endif /* MEMORY_H */
