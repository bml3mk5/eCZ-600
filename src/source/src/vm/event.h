/** @file event.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.11.29-

	@brief [ event manager ]
*/

#ifndef EVENT_H
#define EVENT_H

#include "vm.h"
#include "device.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define MAX_CPU		2
#define MAX_LINES	(LINES_PER_FRAME * 8)
#define MAX_EVENT	64
#define NO_EVENT	-1
#ifdef USE_EMU_INHERENT_SPEC
#define MAX_DISPLAY	2
#ifdef USE_CPU_HALF_SPEED
#define SIG_EVENT_CPU_HALF_SPEED 1
#endif
#endif

#ifdef _DEBUG_LOG
//#define _DEBUG_SOUND_LATE
#endif

class EMU;

/**
	@brief Event manager
*/
class EVENT : public DEVICE
{
private:
	// event manager
	typedef struct {
		DEVICE* device;
		int cpu_clocks;
		int update_clocks;
		int accum_clocks;
	} cpu_t;
	cpu_t d_cpu[MAX_CPU];
	int dcount_cpu;	///< number of CPUs
	int ncount_cpu;	///< number of CPUs on current architecture

	int vclocks[MAX_LINES];
	int event_power;
	int cpu_power;
	int event_remain;
	int cpu_remain, cpu_accum, cpu_done;
	uint64_t event_clocks;
	int frame_split_num;
	bool first_frame;
#ifdef USE_CPU_HALF_SPEED
	uint8_t cpu_speed_half, cpu_speed_half_res;
#endif

	typedef struct event_t {
		DEVICE* device;
		int event_id;
		uint64_t expired_clock;
		uint32_t loop_clock;
		bool active;
		int index;
		event_t *next;
		event_t *prev;
	} event_t;
	event_t event[MAX_EVENT];
	event_t *first_free_event;
	event_t *first_fire_event;

	DEVICE* frame_event[MAX_EVENT];
	DEVICE* vline_event[MAX_EVENT];
	int frame_event_count, vline_event_count;

	double frames_per_sec, next_frames_per_sec;
	int lines_per_frame, next_lines_per_frame;

	int vline_split[FRAME_SPLIT_NUM*2+1];

	void update_event(int clock);
	void insert_event(event_t *event_handle);

	// sound manager
	DEVICE* d_sound[MAX_SOUND];
	int dcount_sound;

	audio_sample_t* sound_buffer;
	audio_sample_t* sound_empty;

	int32_t* sound_tmp;
	int sound_buffer_ptr;
	int sound_buffer_overflow;
	int sound_rate;
	int sound_samples;
	int sound_samples_25;
	int sound_tmp_samples;
	int accum_samples;
	double update_samples_real;
	int update_samples;
	int update_samples_adjust;
//	int update_sound_rate_f;
//	int sound_rate_f;
	int master_volume;
	bool *now_rec_sound;
	void mix_sound(int samples);
	void update_sound(int v);
	void record_sound(int samples);

#ifdef _DEBUG_LOG
	bool initialize_done;
#endif
#ifdef _DEBUG_SOUND_LATE
	uint64_t* sound_tmp_clocks;
#endif

#ifdef USE_EMU_INHERENT_SPEC
	// display
	DEVICE *d_display[MAX_DISPLAY];
	int dcount_display;

	int *vm_pause;
#endif
	int draw_count_per_frame;
	int draw_count_when_pause;

	int vprev;
	int vsuspend;
	int epowsuspend;
	int frame_split_num_suspend;

	// for resume
#pragma pack(1)
	struct vm_state_st {
		uint64_t event_clocks;
		int first_free_event_index;
		int first_fire_event_index;

		struct event_t {
			char class_name[12];
			char identifier[4];
			int event_id;
			uint64_t expired_clock;
			uint32_t loop_clock;
			uint8_t  active;
			int index;
			int next_index;
			int prev_index;

			char reserved[3];
		} event[MAX_EVENT];

		uint8_t cpu_speed_half;
		uint8_t ncount_cpu;
		char reserved[2];

		int event_remain;
		int cpu_remain;
		int cpu_accum;

		uint64_t next_frames_per_sec;
		int next_lines_per_frame;

		int frame_split_num;
	};
#pragma pack()

public:
	EVENT(VM* parent_vm, EMU* parent_emu, char* identifier);
	~EVENT() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void update_config();

	// common event functions
	int event_manager_id() {
		return this_device_id;
	}
	void set_frames_per_sec(double new_frames_per_sec) {
		next_frames_per_sec = new_frames_per_sec;
	}
	void set_lines_per_frame(int new_lines_per_frame) {
		next_lines_per_frame = new_lines_per_frame;
		if (next_lines_per_frame >= MAX_LINES) next_lines_per_frame = MAX_LINES;
	}
	void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id, uint64_t *expire_clock = NULL);
	void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id, uint64_t *expire_clock = NULL);
	void modify_event_by_clock(int register_id, int clock);
	void cancel_event(DEVICE* device, int register_id);
	int  is_registerd_event(const char *class_name, const char *identifier, int register_id);
	void register_frame_event(DEVICE* device);
	bool is_registerd_frame_event(const char *class_name, const char *identifier);
	void register_vline_event(DEVICE* device);
	bool is_registerd_vline_event(const char *class_name, const char *identifier);
	uint64_t get_current_clock();
	uint64_t get_passed_clock(uint64_t prev);
	double get_passed_usec(uint64_t prev);
	int  get_current_power();
	void set_number_of_cpu(int nums);
	uint32_t get_cpu_pc(int index);

	// unique functions
	double get_frame_rate() {
		return next_frames_per_sec;
	}
	void drive(int split_num);

	void initialize_sound(int rate, int samples);
	audio_sample_t* create_sound(int* extra_frames, int samples);
	void set_volume(int decibel, bool vol_mute);

	void set_context_cpu(DEVICE* device, int clocks);
	void set_context_cpu(DEVICE* device);
	void set_context_sound(DEVICE* device) {
		d_sound[dcount_sound++] = device;
	}
#ifdef USE_EMU_INHERENT_SPEC
	void set_context_display(DEVICE* device) {
		d_display[dcount_display++] = device;
	}
	void set_draw_count_per_frame(int value) {
		draw_count_per_frame = value;
	}
	void write_signal(int id, uint32_t data, uint32_t mask);
#endif
	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

#ifdef USE_DEBUGGER
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* EVENT_H */

