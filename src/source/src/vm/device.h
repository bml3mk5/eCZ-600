/** @file device.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5/MBS1 by Sasaji at 2011.06.17

	@brief [ device base class ]
*/

#ifndef DEVICE_H
#define DEVICE_H

#include "../common.h"
#include "vm_defs.h"

class VM;
class EMU;
class FILEIO;

#ifdef USE_DEBUGGER
class DebuggerConsole;
#endif

/// @ingroup Enums
/// @brief common signal ids
enum SIG_CPU_IDS {
	SIG_CPU_IRQ				= 101,	///< IRQ signal
	SIG_CPU_FIRQ			= 102,	///< FIRQ signal
	SIG_CPU_NMI				= 103,	///< NMI signal
	SIG_CPU_BUSREQ			= 104,	///< BUSREQ signal
	SIG_CPU_DEBUG			= 201,	///< for DEBUG
#ifdef USE_EMU_INHERENT_SPEC
	SIG_CPU_ADD_DEAD_CYCLE	= 197,	///< dead cycle
	SIG_CPU_HALT			= 198,	///< HALT signal
	SIG_CPU_RESET			= 199	///< RESET signal
#endif
};

/**
	@brief device base class
*/
class DEVICE
{
protected:
	VM* vm;
	EMU* emu;
public:
	DEVICE(VM* parent_vm, EMU* parent_emu, const char *identifier);
	DEVICE(EMU* parent_emu, const char *identifier);
	virtual ~DEVICE();

	/// @name device information
	///@{
	void set_class_name(const char *name);
	const char *get_class_name() const;
	const char *get_identifier() const;
	void set_identifier(const char *identifier);
	void set_vm_state_class_name(const char *name);

	virtual void get_edition_string(char *buffer, size_t buffer_len) const;
	///@}

	/// @name device chain
	///@{
	DEVICE *get_prev_device();
	DEVICE *get_next_device();
	int     get_id() const;
	///@}

	/// @name control
	///@{
	virtual void initialize();
	virtual void release();

	virtual void update_config();
	virtual void save_state(FILEIO* fio);
	virtual bool load_state(FILEIO* fio);

	virtual void reset();
	virtual void special_reset();
	virtual void enable(bool value);
	virtual bool enable() const;
	///@}

	/// @name memory bus
	///@{
	virtual void    write_data_n(uint32_t addr, uint32_t data, int width);
	virtual uint32_t read_data_n(uint32_t addr, int width);

	virtual void    write_data8(uint32_t addr, uint32_t data);
	virtual uint32_t read_data8(uint32_t addr);
	virtual void    write_data16(uint32_t addr, uint32_t data);
	virtual uint32_t read_data16(uint32_t addr);
	virtual void    write_data24(uint32_t addr, uint32_t data);
	virtual uint32_t read_data24(uint32_t addr);
	virtual void    write_data32(uint32_t addr, uint32_t data);
	virtual uint32_t read_data32(uint32_t addr);
	virtual void    write_data8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_data8w(uint32_t addr, int* wait);
	virtual void    write_data16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_data16w(uint32_t addr, int* wait);
	virtual void    write_data24w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_data24w(uint32_t addr, int* wait);
	virtual void    write_data32w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_data32w(uint32_t addr, int* wait);

	virtual uint32_t fetch_op(uint32_t addr, int *wait);
	virtual void    latch_address(uint32_t addr, int *wait);

	virtual void    write_dma_data_n(uint32_t addr, uint32_t data, int width);
	virtual uint32_t read_dma_data_n(uint32_t addr, int width);

	virtual void    write_dma_data8(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_data8(uint32_t addr);
	virtual void    write_dma_data16(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_data16(uint32_t addr);
	virtual void    write_dma_data32(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_data32(uint32_t addr);
	///@}

	/// @name i/o bus
	///@{
	virtual void    write_io_n(uint32_t addr, uint32_t data, int width);
	virtual uint32_t read_io_n(uint32_t addr, int width);
	virtual void    write_io_m(uint32_t addr, uint32_t data, uint32_t mask);
	virtual uint32_t read_io_m(uint32_t addr, uint32_t mask);

	virtual void    write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t read_io8(uint32_t addr);
	virtual void    write_io16(uint32_t addr, uint32_t data);
	virtual uint32_t read_io16(uint32_t addr);
	virtual void    write_io32(uint32_t addr, uint32_t data);
	virtual uint32_t read_io32(uint32_t addr);
	virtual void    write_io8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_io8w(uint32_t addr, int* wait);
	virtual void    write_io16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_io16w(uint32_t addr, int* wait);
	virtual void    write_io32w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_io32w(uint32_t addr, int* wait);

	virtual void    write_dma_io_n(uint32_t addr, uint32_t data, int width);
	virtual uint32_t read_dma_io_n(uint32_t addr, int width);

	virtual void    write_dma_io8(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_io8(uint32_t addr);
	virtual void    write_dma_io16(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_io16(uint32_t addr);
	virtual void    write_dma_io32(uint32_t addr, uint32_t data);
	virtual uint32_t read_dma_io32(uint32_t addr);
	///@}

	/// @name memory mapped i/o (classify memory bus more detail)
	///@{
	virtual void    write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual uint32_t read_memory_mapped_io8(uint32_t addr);
	virtual void    write_memory_mapped_io16(uint32_t addr, uint32_t data);
	virtual uint32_t read_memory_mapped_io16(uint32_t addr);
	virtual void    write_memory_mapped_io32(uint32_t addr, uint32_t data);
	virtual uint32_t read_memory_mapped_io32(uint32_t addr);
	virtual void    write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_memory_mapped_io8w(uint32_t addr, int* wait);
	virtual void    write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_memory_mapped_io16w(uint32_t addr, int* wait);
	virtual void    write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t read_memory_mapped_io32w(uint32_t addr, int* wait);
	///@}

	/// device information to send signal
	typedef struct {
		DEVICE *device;
		int id;
		uint32_t mask;
		int shift;
		uint32_t negative;
	} output_t;

	/// device list to send signal
	typedef struct {
		int count;
		output_t item[MAX_OUTPUT];
	} outputs_t;

	/// @name device to device
	///@{
	virtual void init_output_signals(outputs_t *items);
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, int shift_left, uint32_t negative = 0);
	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask, uint32_t negative = 0);
//	virtual void register_output_signal(outputs_t *items, DEVICE *device, int id, uint32_t mask);
	virtual void write_signals(outputs_t *items, uint32_t data);
	virtual void write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t read_signal(int ch);
	///@}

	/// @name z80 daisy chain
	///@{
	virtual void set_context_intr(DEVICE* device, uint32_t bit);
	virtual void set_context_child(DEVICE* device);
	///@}

	/// @name interrupt device to device
	///@{
	virtual void set_intr_iei(bool val);
	///@}

	/// @name interrupt device to cpu
	///@{
	virtual void set_intr_line(bool line, bool pending, uint32_t bit);
	///@}

	/// @name interrupt cpu to device
	///@{
	virtual uint32_t get_intr_ack();
	virtual void notify_intr_reti();
	virtual void notify_intr_ei();
	virtual void update_intr_condition();
	///@}

	/// @name dma
	///@{
	virtual void do_dma();
	///@}

	/// @name cpu
	///@{
	virtual int run(int clock);
#ifdef USE_CPU_REAL_MACHINE_CYCLE
	virtual int run(int clock, int accum, int cycles);
#endif
	virtual uint32_t get_pc();
	virtual uint32_t get_next_pc();
	///@}

	/// @name bios
	///@{
	virtual bool bios_call(uint32_t pc, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	virtual bool bios_int(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	///@}

	/// @name floppy disk controller
	///@{
	virtual bool search_track(int channel);
	virtual uint32_t verify_track();
	virtual bool verify_track(int channel, int track);
	virtual int  get_current_track_number(int channel);
	virtual uint32_t search_sector(int side, bool compare);
	virtual int  search_sector(int channel);
	virtual int  search_sector(int channel, int track, int sect, bool compare_side, int side);
	virtual bool make_track();
	virtual bool make_track(int channel);
	virtual bool parse_track();
	virtual bool parse_track(int channel);
	///@}

	/// @name floppy disk drive
	///@{
	virtual int get_a_round_clock(int channel);
	virtual int get_head_loading_clock(int channel);
	virtual int get_index_hole_remain_clock();
	virtual int calc_index_hole_search_clock(int channel);
	virtual int get_clock_arrival_sector(int channel, int sect, int delay);
	virtual int calc_sector_search_clock(int channel, int sect);
	virtual int calc_next_sector_clock(int channel);
	///@}

	/// processing event device
	DEVICE* event_manager;

	/// @name event manager
	///@{
	virtual void set_context_event_manager(DEVICE* device);
	virtual int  event_manager_id();
	virtual void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id, uint64_t *expire_clock = NULL);
	virtual void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id, uint64_t *expire_clock = NULL);
	virtual void modify_event_by_clock(int register_id, int clock);
	virtual void cancel_event(DEVICE* device, int register_id);
	virtual int  is_registerd_event(const char *class_name, const char *identifier, int register_id);
	virtual void register_frame_event(DEVICE* device);
	virtual	bool is_registerd_frame_event(const char *class_name, const char *identifier);
	virtual void register_vline_event(DEVICE* device);
	virtual	bool is_registerd_vline_event(const char *class_name, const char *identifier);
	virtual uint64_t get_current_clock();
	virtual uint64_t get_passed_clock(uint64_t prev);
	virtual double get_passed_usec(uint64_t prev);
	virtual void set_cpu_clock(uint32_t freq);
	virtual uint32_t get_cpu_clock() const;

	virtual void set_number_of_cpu(int nums);
	virtual uint32_t get_cpu_pc(int index);
	virtual void set_frames_per_sec(double frames);
	virtual void set_lines_per_frame(int lines);
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	///@}

	/// @name event callback
	///@{
	virtual void event_callback(int event_id, int err);
	virtual void event_pre_frame();	// this event is to update timing settings
	virtual void event_frame();
	virtual void event_vline(int v, int clock);
	virtual void event_hsync(int v, int h, int clock);
	///@}

	/// @name sound
	///@{
	virtual void mix(int32_t* buffer, int cnt);
//	virtual void set_volume(int volume);
//	virtual void set_volume(int ch, int decibel_l, int decibel_r);
	///@}

	/// @name network and communication device
	///@{
	virtual void network_connected(int ch);
	virtual void network_disconnected(int ch);
	virtual void network_writeable(int ch);
	virtual void network_readable(int ch);
	virtual void network_accepted(int ch, int new_ch);
	virtual uint8_t* get_sendbuffer(int ch, int* size, int* flags);
	virtual void inc_sendbuffer_ptr(int ch, int size);
	virtual uint8_t* get_recvbuffer0(int ch, int* size0, int* size1, int* flags);
	virtual uint8_t* get_recvbuffer1(int ch);
	virtual void inc_recvbuffer_ptr(int ch, int size);
	virtual double get_send_speed_usec(int ch, bool is_byte);
	virtual double get_recv_speed_usec(int ch, bool is_byte);
	///@}

#ifdef USE_DEBUGGER
	/// @name debugger
	///@{
	virtual DEVICE *get_debugger();
	virtual void set_debugger(DEVICE *dbg);
	virtual void set_debugger_console(DebuggerConsole *dc);
	virtual DEVICE *get_context_mem() const;

	virtual uint32_t debug_prog_addr_mask();
	virtual uint32_t debug_data_addr_mask();
	virtual uint32_t debug_physical_addr_mask(int type);
	virtual uint32_t debug_io_addr_mask();
	virtual uint32_t debug_data_mask();
	virtual bool debug_physical_addr_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	virtual void debug_write_data8(int type, uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_data8(int type, uint32_t addr);
	virtual void debug_write_data16(int type, uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_data16(int type, uint32_t addr);
	virtual void debug_write_data32(int type, uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_data32(int type, uint32_t addr);
	virtual void debug_write_data8w(int type, uint32_t addr, uint32_t data, int *wait);
	virtual uint32_t debug_read_data8w(int type, uint32_t addr, int *wait);

	virtual bool debug_ioport_is_supported() const;
	virtual bool debug_exception_is_supported() const;

	virtual void debug_write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_io8(uint32_t addr);
	virtual void debug_write_io16(uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_io16(uint32_t addr);
	virtual void debug_write_io32(uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_io32(uint32_t addr);
	virtual uint32_t debug_latch_address(uint32_t addr);
	virtual void debug_write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual uint32_t debug_read_memory_mapped_io8(uint32_t addr);

	virtual void debug_event_frame();

	virtual uint32_t debug_read_bank(uint32_t addr);
	virtual bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	virtual bool debug_write_reg(uint32_t reg_num, uint32_t data);
	virtual bool debug_write_reg(int type, const _TCHAR *reg, uint32_t data);
	virtual bool debug_write_reg(int type, uint32_t reg_num, uint32_t data);
	static uint32_t find_debug_reg_name(const _TCHAR *list[], const _TCHAR *name);

	virtual void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual void debug_regs_info(int type, _TCHAR *buffer, size_t buffer_len);
	virtual bool get_debug_reg_ptr(_TCHAR *reg, size_t regsiz, void * &regptr, int &reglen);
	virtual void debug_memory_map_info(DebuggerConsole *dc);
	virtual int debug_address_map_info(DebuggerConsole *dc, int index);
	virtual int debug_address_map_edit(DebuggerConsole *dc, int index, int *values, int count);
	virtual int debug_address_map_get_prev(DebuggerConsole *dc, int index, int *values, int &count);
	virtual int debug_memory_space_map_info(DebuggerConsole *dc, int index);
	virtual int debug_memory_space_map_edit(DebuggerConsole *dc, int index, int *values, int count);
	virtual int debug_memory_space_map_get(DebuggerConsole *dc, int index, int *values, int &count);
	virtual int debug_dasm(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len, int flags);
	virtual int debug_dasm_label(int type, uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	virtual int debug_trace_back_regs(int index, _TCHAR *buffer, size_t buffer_len);
	virtual int debug_trace_back(int index, _TCHAR *buffer, size_t buffer_len);
	virtual uint32_t debug_address_mapping_rev(uint32_t addr);
	virtual bool reach_break_point();
//	virtual void now_debugging(bool val);
//	virtual bool now_debugging() const;
	virtual void go_suspend();
	virtual void go_suspend_at_first();
	virtual bool now_suspend() const;

	virtual	uint32_t get_debug_pc(int type);
	virtual	uint32_t get_debug_next_pc(int type);
	virtual	uint32_t get_debug_branch_pc(int type);

	virtual bool get_debug_signal_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	virtual bool get_debug_signal_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	virtual void get_debug_signal_names_str(_TCHAR *buffer, size_t buffer_len);

	virtual bool get_debug_exception_name_index(const _TCHAR *param, uint32_t *num, uint32_t *mask, int *idx, const _TCHAR **name);
	virtual bool get_debug_exception_name_index(uint32_t num, uint32_t *mask, int *idx, const _TCHAR **name);
	virtual void get_debug_exception_names_str(_TCHAR *buffer, size_t buffer_len);

	virtual int  get_debug_graphic_memory_size(int type, int *width, int *height);
	virtual bool debug_graphic_type_name(int type, _TCHAR *buffer, size_t buffer_len);
	virtual bool debug_draw_graphic(int type, int width, int height, scrntype *buffer);

	virtual uint32_t debug_basic_get_line_number_ptr();
	virtual uint32_t debug_basic_get_line_number();

	virtual bool debug_basic_is_supported();
	virtual void debug_basic_variables(DebuggerConsole *dc, int name_cnt, const _TCHAR **names);
	virtual void debug_basic_list(DebuggerConsole *dc, int st_line, int ed_line);
	virtual void debug_basic_trace_onoff(DebuggerConsole *dc, bool enable);
	virtual void debug_basic_trace_current();
	virtual void debug_basic_trace_back(DebuggerConsole *dc, int num);
	virtual void debug_basic_command(DebuggerConsole *dc);
	virtual void debug_basic_error(DebuggerConsole *dc, int num);

//	virtual void debug_basic_set_break_point(uint32_t &addr);
	virtual bool debug_basic_check_break_point(uint32_t line, int len);
	///@}
#endif /* USE_DEBUGGER */

#ifdef USE_EMU_INHERENT_SPEC
	/// @name display
	///@{
	virtual void update_display(int vline, int clock);
	///@}
#endif


protected:
	DEVICE* prev_device;	///< device chain
	DEVICE* next_device;	///< device chain
	int this_device_id;

	char this_class_name[13];
	char this_identifier[5];

	bool available;

#ifdef USE_EMU_INHERENT_SPEC
	bool now_reset;
#endif

#pragma pack(1)
	/// for resume (state file)
	typedef struct vm_state_ident_st {
		char   class_name[12];
		char   identifier[4];
		uint32_t size;
		uint16_t version;
		int    device_id;
		char   reserved[6];
	} vm_state_ident_t;
	vm_state_ident_t vm_state_ident;
#pragma pack()

	/// @name processing resume state file
	///@{
	bool find_state_chunk(FILEIO *fio, const char *class_name, const char *identifier, vm_state_ident_t *vm_state_i);
	bool find_state_chunk(FILEIO *fio, vm_state_ident_t *vm_state_i);

	void adjust_chunk_size(vm_state_ident_t *vm_state_i, uint32_t &chunk_size);
	///@}

#define FIND_STATE_CHUNK(p_fio, st_i) \
	{ \
		if (find_state_chunk(p_fio, &st_i) != true) { \
			return true; \
		} \
		p_fio->Fseek(-(long)sizeof(st_i), FILEIO::SEEKCUR); \
	}

#define READ_STATE_CHUNK(p_fio, st_i, st_v) \
	{ \
		if (find_state_chunk(p_fio, &st_i) != true) { \
			return true; \
		} \
		uint32_t i_size = Uint32_LE(st_i.size); \
		memset(&st_v, 0, sizeof(st_v)); \
		if (i_size >= (sizeof(st_v) + sizeof(st_i))) { \
			p_fio->Fread(&st_v, sizeof(st_v), 1); \
			p_fio->Fseek(i_size - sizeof(st_v) - sizeof(st_i), FILEIO::SEEKCUR); \
		} else { \
			p_fio->Fread(&st_v, i_size - sizeof(st_i), 1); \
		} \
	}

#define READ_STATE_VARIABLE_CHUNK(p_fio, st_i, st_v) \
	{ \
		if (find_state_chunk(p_fio, &st_i) != true) { \
			return; \
		} \
		uint32_t i_size = Uint32_LE(st_i.size); \
		memset(&st_v, 0, sizeof(st_v)); \
		if (i_size >= (sizeof(st_v) + sizeof(st_i))) { \
			p_fio->Fread(&st_v, sizeof(st_v), 1); \
		} else { \
			p_fio->Fread(&st_v, i_size - sizeof(st_i), 1); \
		} \
	}

	/// @name suppress debug log
	///@{
	virtual void dummyf(const void *format, ...);
	///@}
};

#endif /* DEVICE_H */
