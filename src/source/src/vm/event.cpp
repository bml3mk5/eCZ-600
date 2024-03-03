/** @file event.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.11.29-

	@brief [ event manager ]
*/

#include <math.h>
#include <stdlib.h>
#include "event.h"
#include "../emu.h"
#include "../fileio.h"
#include "../config.h"
#ifdef _DEBUG
#include <cassert>
#endif

#ifndef EVENT_CONTINUOUS_SOUND
//#ifdef PCM1BIT_HIGH_QUALITY
#define EVENT_CONTINUOUS_SOUND
//#endif
#endif

//#define _DEBUG_SOUND_ADJUST

EVENT::EVENT(VM* parent_vm, EMU* parent_emu, char* identifier)
	: DEVICE(parent_vm, parent_emu, identifier)
{
	set_class_name("EVENT");

	dcount_cpu = ncount_cpu = 0;
	dcount_sound = 0;
	frame_event_count = vline_event_count = 0;

	// initialize event
	memset(frame_event, 0, sizeof(frame_event));
	memset(vline_event, 0, sizeof(vline_event));

	memset(d_cpu, 0, sizeof(d_cpu));

	for(int i = 0; i < MAX_EVENT; i++) {
		event[i].device = NULL;
		event[i].event_id = -1;
		event[i].expired_clock = 0;
		event[i].loop_clock = 0;
		event[i].active = false;
		event[i].index = i;
		event[i].next = (i + 1 < MAX_EVENT) ? &event[i + 1] : NULL;
		event[i].prev = (i - 1 >= 0) ? &event[i - 1] : NULL;
	}
	first_free_event = &event[0];
	first_fire_event = NULL;

	event_clocks = 0;

	// force update timing in the first frame
	frames_per_sec = 0.0;
	lines_per_frame = 0;
	next_frames_per_sec = FRAMES_PER_SEC;
	next_lines_per_frame = LINES_PER_FRAME;

	update_samples_real = 0.0;
	update_samples = 0;
	update_samples_adjust = 0;

#ifdef _DEBUG_LOG
	initialize_done = false;
#endif
#ifdef USE_EMU_INHERENT_SPEC
	dcount_display = 0;
#endif
}

void EVENT::initialize()
{
	// load config
	if(!(0 <= pConfig->cpu_power && pConfig->cpu_power <= 5)) {
		pConfig->cpu_power = 1;
	}

#ifdef USE_EMU_INHERENT_SPEC
	if (pConfig->sync_irq) {
		cpu_power = 1;
		event_power = pConfig->cpu_power;
	} else {
		cpu_power = pConfig->cpu_power;
		event_power = 1;
	}
	vm_pause = emu->get_pause_ptr();
#else
	cpu_power = pConfig->cpu_power;
	event_power = 0;
#endif
#ifdef USE_CPU_HALF_SPEED
	cpu_speed_half = cpu_speed_half_res = 0;
#endif

	memset(vclocks, 0, sizeof(vclocks));
	memset(vline_split, 0, sizeof(vline_split));

	frame_split_num = 0;
	first_frame = true;

	draw_count_per_frame = 1;
	draw_count_when_pause = 0;

	// initialize sound buffer
	sound_buffer = NULL;
	sound_tmp = NULL;

	vprev = -1;
	vsuspend = -1;
	epowsuspend = -1;
	frame_split_num_suspend = -1;

	now_rec_sound = emu->get_now_rec_sound_ptr();
}

void EVENT::initialize_sound(int rate, int samples)
{
	// initialize sound
	sound_rate = rate;
	sound_samples = samples;
	sound_samples_25 = rate * 25 / 1000;
#ifdef EVENT_CONTINUOUS_SOUND
	sound_tmp_samples = samples * 3;
#else
	sound_tmp_samples = samples;
#endif
	if (frames_per_sec > 0.0 && lines_per_frame > 0) {
		update_samples_real = (double)sound_rate / frames_per_sec / (double)lines_per_frame;
		update_samples = (int)(1024.0 * update_samples_real);
	} else {
		update_samples_real = 0.0;
		update_samples = 0;
	}
//	update_config();
	sound_buffer = (audio_sample_t*)malloc(sound_samples * sizeof(audio_sample_t) * 2);
	memset(sound_buffer, 0, sound_samples * sizeof(audio_sample_t) * 2);
	sound_empty = (audio_sample_t*)malloc(sound_samples * sizeof(audio_sample_t) * 2);
#ifdef USE_AUDIO_U8
	memset(sound_empty, 128, sound_samples * sizeof(audio_sample_t) * 2);
#else
	memset(sound_empty, 0, sound_samples * sizeof(audio_sample_t) * 2);
#endif
	sound_tmp = (int32_t*)malloc(sound_tmp_samples * sizeof(int32_t) * 2);
	memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	sound_buffer_ptr = accum_samples = 0;
	sound_buffer_overflow = 0;
	set_volume(0, false);
#ifdef _DEBUG_SOUND_LATE
	sound_tmp_clocks = (uint64_t*)malloc(sound_tmp_samples * sizeof(uint64_t));
	memset(sound_tmp_clocks, 0, sound_tmp_samples * sizeof(uint64_t));
#endif
}

void EVENT::set_context_cpu(DEVICE* device, int clocks)
{
	int index = dcount_cpu++;
#ifdef _DEBUG
	assert(index < MAX_CPU);
#endif
	d_cpu[index].device = device;
	d_cpu[index].cpu_clocks = clocks;
	d_cpu[index].accum_clocks = 0;
	ncount_cpu = dcount_cpu;
}

void EVENT::set_context_cpu(DEVICE* device)
{
	set_context_cpu(device, CPU_CLOCKS);
}

void EVENT::release()
{
	// release sound
	if(sound_buffer) {
		free(sound_buffer);
		sound_buffer = NULL;
	}
	if(sound_empty) {
		free(sound_empty);
		sound_empty = NULL;
	}
	if(sound_tmp) {
		free(sound_tmp);
		sound_tmp = NULL;
	}
#ifdef _DEBUG_SOUND_LATE
	if (sound_tmp_clocks) {
		free(sound_tmp_clocks);
		sound_tmp_clocks = NULL;
	}
#endif
}

void EVENT::reset()
{
	// clear events except loop event
	for(int i = 0; i < MAX_EVENT; i++) {
		if(event[i].active && event[i].loop_clock == 0) {
			cancel_event(NULL, i);
		}
	}

	event_remain = 0;
	cpu_remain = cpu_accum = cpu_done = 0;

	frame_split_num = 0;
	first_frame = true;

#if 0
	// reset sound
	if(sound_buffer) {
		memset(sound_buffer, 0, sound_samples * sizeof(audio_sample_t) * 2);
	}
	if(sound_tmp) {
		memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	}
//	sound_buffer_ptr = 0;
#endif

#ifdef _DEBUG_LOG
	initialize_done = true;
#endif
}

void EVENT::drive(int split_num)
{
	int v;

	int epowstart = (epowsuspend <= 0 ? (1 << event_power) : epowsuspend);
	int draw_count = ((epowstart / FRAME_SPLIT_NUM / 2) * (FRAME_SPLIT_NUM - split_num));
	if (frame_split_num_suspend >= 0) frame_split_num = frame_split_num_suspend;

	for(int epow = epowstart; epow > 0; epow--) {
		if (first_frame) {
			// raise pre frame events to update timing settings
			for(int i = 0; i < frame_event_count; i++) {
				frame_event[i]->event_pre_frame();
			}
			// generate clocks per line
			if(frames_per_sec != next_frames_per_sec || lines_per_frame != next_lines_per_frame) {
				frames_per_sec = next_frames_per_sec;
				lines_per_frame = next_lines_per_frame;

				int sum = (int)((double)d_cpu[0].cpu_clocks / frames_per_sec + 0.5);
				int remain = sum;

				for(int i = 0; i < lines_per_frame; i++) {
					vclocks[i] = (sum / lines_per_frame);
					remain -= vclocks[i];
				}
				for(int i = 0; i < remain; i++) {
					int index = (lines_per_frame * i / remain);
					vclocks[index]++;
				}
				sum = lines_per_frame;
				remain = sum;
				for(int sp = 1; sp <= FRAME_SPLIT_NUM*2; sp++) {
					vline_split[sp] = (sum / FRAME_SPLIT_NUM / 2);
					remain -= vline_split[sp];
				}
				for(int sp = 0; sp < remain; sp++) {
					int index = FRAME_SPLIT_NUM * 2 * sp / remain;
					vline_split[index+1]++;
				}
				for(int sp = 1; sp <= FRAME_SPLIT_NUM*2; sp++) {
					vline_split[sp] += vline_split[sp-1];
				}
				d_cpu[0].update_clocks = 1024;
				d_cpu[0].device->set_cpu_clock(d_cpu[0].cpu_clocks);
				for(int i = 1; i < dcount_cpu; i++) {
					d_cpu[i].update_clocks = (int)(1024.0 * (double)d_cpu[i].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5);
					d_cpu[i].device->set_cpu_clock((uint32_t)((double)d_cpu[i].update_clocks * d_cpu[0].cpu_clocks / 1024.0));
				}
				for(DEVICE* device = vm->first_device; device; device = device->get_next_device()) {
					if(device->event_manager_id() == this_device_id) {
						device->update_timing(d_cpu[0].cpu_clocks, frames_per_sec, lines_per_frame);
					}
				}
				update_samples_real = (double)sound_rate / frames_per_sec / (double)lines_per_frame;
				update_samples = (int)(1024.0 * update_samples_real);
			}

			// run virtual machine for 1 frame period
			for(int i = 0; i < frame_event_count; i++) {
				frame_event[i]->event_frame();
			}

#ifdef EVENT_CONTINUOUS_SOUND
			// adjusting count of samples per vline
#ifdef _DEBUG_SOUND_ADJUST
			int prev_adjust = update_samples_adjust;
#endif
			if ((sound_samples + sound_samples_25) > sound_buffer_ptr) {
				// samples left little
				update_samples_adjust = 32;
			} else {
				update_samples_adjust = 0;
			}
#ifdef _DEBUG_SOUND_ADJUST
			if (prev_adjust != update_samples_adjust) logging->out_logf(LOG_DEBUG,_T("EVENT::drive samples:%d buf_ptr:%d samples_adjust:%d"),sound_samples,sound_buffer_ptr,update_samples_adjust);
#endif
#endif
			first_frame = false;
		}

		int vstart = (vsuspend < 0 ? vline_split[frame_split_num] : vsuspend);
#ifdef USE_DEBUGGER
		bool now_suspend = false;
		for(int i=0; i<ncount_cpu; i++) {
			now_suspend |= d_cpu[i].device->get_debugger()->now_suspend();
		}
		if (!now_suspend) {
			vsuspend = -1;
			epowsuspend = -1;
			frame_split_num_suspend = -1;
			*vm_pause &= ~VM_SUSPEND_MASK;
		}
#endif
		if (!(*vm_pause)) {
			for(v = vstart; v < vline_split[frame_split_num+1]; v++) {
				// run virtual machine per line
				if (v != vprev) {
					for(int i = 0; i < vline_event_count; i++) {
						vline_event[i]->event_vline(v, vclocks[v]);
					}
					if(event_remain < 0) {
						if(-event_remain > vclocks[v]) {
							update_event(vclocks[v]);
						}
						else {
							update_event(-event_remain);
						}
					}
					event_remain += vclocks[v];
					cpu_remain += ((vclocks[v] << cpu_power) >> 1);

					vprev = v;
				}

				while(event_remain > 0 && vsuspend < 0) {
					int event_done = event_remain;
					if(cpu_remain > 0) {
						// run one opecode on primary cpu
						int cpu_done_tmp;

#if (NUMBER_OF_CPUS > 1)
						if(ncount_cpu == 1)
#endif
						{
#ifdef USE_CPU_REAL_MACHINE_CYCLE
#ifdef USE_CPU_HALF_SPEED
							cpu_done_tmp = d_cpu[0].device->run(-1, (event_clocks % CLOCKS_CYCLE), cpu_speed_half + 1);
#else
							cpu_done_tmp = d_cpu[0].device->run(-1, (event_clocks % CLOCKS_CYCLE), 1);
#endif
#else
							cpu_done_tmp = d_cpu[0].device->run(-1);
#endif
#ifdef USE_CPU_HALF_SPEED
							cpu_speed_half = cpu_speed_half_res;
#endif
#ifdef USE_DEBUGGER
							if (d_cpu[0].device->reach_break_point()) {
								vsuspend = v;
								epowsuspend = epow;
								frame_split_num_suspend = frame_split_num;
								draw_count_when_pause = 0;
								*vm_pause |= VM_SUSPEND_MASK;
#if 0
								logging->out_debugf(_T("BREAK: v:%3d epow:%d fsn:%d ev_clk:%10lld ev_rem:%3d cpu_rem:%3d pause:%x"),vsuspend,epowsuspend,frame_split_num_suspend
								,event_clocks,event_remain,cpu_remain
								,(*vm_pause));
#endif
							}
#endif /* USE_DEBUGGER */
						}
#if (NUMBER_OF_CPUS > 1)
						else {
							// sync to sub cpus
#ifdef USE_DEBUGGER
							now_suspend = false;
#endif /* USE_DEBUGGER */
							if(cpu_done == 0) {
#ifdef USE_CPU_REAL_MACHINE_CYCLE
								cpu_done = d_cpu[0].device->run(-1, (event_clocks % CLOCKS_CYCLE), cpu_speed_half + 1);
#else
								cpu_done = d_cpu[0].device->run(-1);
#endif
#ifdef USE_DEBUGGER
								now_suspend |= d_cpu[0].device->reach_break_point();
#endif /* USE_DEBUGGER */
							}
							cpu_done_tmp = 1; //(cpu_done < 4) ? cpu_done : 4;
#ifdef USE_CPU_HALF_SPEED
							cpu_speed_half = cpu_speed_half_res;
#endif
							cpu_done -= cpu_done_tmp;
							if (cpu_done < 0) cpu_done = 0;

							for(int i = 1; i < ncount_cpu; i++) {
								// run sub cpus
								d_cpu[i].accum_clocks += d_cpu[i].update_clocks * cpu_done_tmp;
								int sub_clock = d_cpu[i].accum_clocks >> 10;
								if(sub_clock) {
									d_cpu[i].accum_clocks -= sub_clock << 10;
									sub_clock = d_cpu[i].device->run(sub_clock);
#ifdef USE_DEBUGGER
									now_suspend |= (sub_clock > 0 ? d_cpu[i].device->reach_break_point() : d_cpu[i].device->get_debugger()->now_suspend());
#endif /* USE_DEBUGGER */
								}
							}
#ifdef USE_DEBUGGER
							if (now_suspend) {
								// all cpus are suspended if one of cpu become suspending.
								vsuspend = v;
								epowsuspend = epow;
								frame_split_num_suspend = frame_split_num;
								draw_count_when_pause = 0;
								*vm_pause |= VM_SUSPEND_MASK;
								for(int i = 0; i < ncount_cpu; i++) {
									d_cpu[i].device->get_debugger()->go_suspend();
								}
							}
#endif /* USE_DEBUGGER */
						}
#endif /* NUMBER_OF_CPUS */
						cpu_remain -= cpu_done_tmp;
						cpu_accum += cpu_done_tmp;
						event_done = ((cpu_accum << 1) >> cpu_power);
						cpu_accum -= ((event_done << cpu_power) >> 1);
					}
					if(event_done > 0) {
						if(event_done > event_remain) {
							update_event(event_remain);
						}
						else {
							update_event(event_done);
						}
						event_remain -= event_done;
					}
				} // while(event_remain ...
				if (event_remain <= 0) {
#ifdef USE_EMU_INHERENT_SPEC
					for(int i = 0; i < dcount_display; i++) {
						d_display[i]->update_display(v, vclocks[v]);
					}
#endif
					update_sound(v);
				}
				if (vsuspend >= 0) {
					break;
				}
			} // for(v ...
			if (event_remain <= 0 && v == lines_per_frame) {
				// draw screen if need
				if (draw_count <= draw_count_per_frame) {
					emu->draw_screen();
				}
				draw_count--;
				// update autokey if need
				emu->update_autokey();
			}
		} else if ((*vm_pause) & VM_SUSPEND_MASK) {
			// now suspend (for debugger)
#ifdef USE_DEBUGGER
			// run debug event frame while suspending
			for(int i = 0; i < frame_event_count; i++) {
				frame_event[i]->debug_event_frame();
			}
#endif
			v = vstart;
			update_sound(v);
			if (draw_count_when_pause == 0) {
				emu->draw_screen();
			}
			draw_count_when_pause = (draw_count_when_pause + 1) % 6;

		} else if ((*vm_pause) & (VM_SYSPAUSE_MASK | VM_USRPAUSE_MASK)) {
			// now pausing
#ifdef USE_DEBUGGER
			// When start debugging in pausing 
			for(int i=0; i<ncount_cpu; i++) {
				DEVICE *dbg = d_cpu[i].device->get_debugger();
				dbg->go_suspend_at_first();
			}
#endif
			for(v = vstart; v < vline_split[frame_split_num+1]; v++) {
				update_sound(v);
			}
			if (draw_count_when_pause < 2) {
#ifdef USE_EMU_INHERENT_SPEC
				for(v = vstart; v < vline_split[frame_split_num+1]; v++) {
					for(int i = 0; i < dcount_display; i++) {
						d_display[i]->update_display(v, vclocks[v]);
					}
				}
#endif
				emu->draw_screen();
			}
			draw_count_when_pause = (draw_count_when_pause + 1) % 6;

		} else {
			// now power off

			for(v = vstart; v < vline_split[frame_split_num+1]; v++) {
				if (v != vprev) {
					for(int i = 0; i < vline_event_count; i++) {
						vline_event[i]->event_vline(v, vclocks[v]);
					}
					if(event_remain < 0) {
						if(-event_remain > vclocks[v]) {
							update_event(vclocks[v]);
						}
						else {
							update_event(-event_remain);
						}
					}
					event_remain += vclocks[v];
					cpu_remain += ((vclocks[v] << cpu_power) >> 1);

					vprev = v;
				}

				while(event_remain > 0) {
					int event_done = event_remain;
					if(cpu_remain > 0) {
						cpu_remain--;
						cpu_accum++;
						event_done = ((cpu_accum << 1) >> cpu_power);
						cpu_accum -= ((event_done << cpu_power) >> 1);
					}
					if(event_done > 0) {
						if(event_done > event_remain) {
							update_event(event_remain);
						}
						else {
							update_event(event_done);
						}
						event_remain -= event_done;
					}
				} // while(event_remain ...
				if (event_remain <= 0) {
					update_sound(v);
				}
			} // for(v ...
			if (draw_count_when_pause == 0) {
				emu->draw_screen();
			}
			draw_count_when_pause = (draw_count_when_pause + 1) % 6;

		}
		if (event_remain <= 0 && v == vline_split[frame_split_num+1]) {
			frame_split_num = (frame_split_num + 1) & ((FRAME_SPLIT_NUM << 1) - 1);
			first_frame = (frame_split_num == 0);
		}
		if (epowsuspend > 0) {
			break;
		}
	} // for(int epow ...
}

void EVENT::update_event(int clock)
{
	uint64_t event_clocks_tmp = event_clocks + clock;
	event_t *event_handle = NULL;

	while(first_fire_event != NULL && first_fire_event->expired_clock <= event_clocks_tmp) {
		event_handle = first_fire_event;

		first_fire_event = event_handle->next;
		if(first_fire_event != NULL) {
			first_fire_event->prev = NULL;
		}
		if(event_handle->loop_clock != 0) {
			event_handle->expired_clock += event_handle->loop_clock;
			insert_event(event_handle);
		}
		else {
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
		event_clocks = event_handle->expired_clock;
		event_handle->device->event_callback(event_handle->event_id, 0);
	}
	event_clocks = event_clocks_tmp;
}

uint64_t EVENT::get_current_clock()
{
	return event_clocks;
}

uint64_t EVENT::get_passed_clock(uint64_t prev)
{
	uint64_t current = get_current_clock();
	return (current > prev) ? current - prev : current + (0xffffffffffffffffLL - prev) + 1;
}

double EVENT::get_passed_usec(uint64_t prev)
{
	return 1000000.0 * get_passed_clock(prev) / d_cpu[0].cpu_clocks;
}

/// set number of CPUs for use
///
/// @note should only set in initialize or reset sequence
void EVENT::set_number_of_cpu(int nums)
{
	if (nums > 0) {
		ncount_cpu = nums;

//#ifdef USE_DEBUGGER
//		for(int i = ncount_cpu; i < dcount_cpu; i++) {
//			d_cpu[i].device->get_debugger()->go_suspend();
//		}
//#endif
	}
}

uint32_t EVENT::get_cpu_pc(int index)
{
	return d_cpu[index].device->get_pc();
}

void EVENT::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id, uint64_t *expire_clock)
{
	int clock = (int)((double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
	register_event_by_clock(device, event_id, clock, loop, register_id, expire_clock);
}

void EVENT::register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id, uint64_t *expire_clock)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		logging->out_debugf(_T("EVENT: non-loop event is registered before initialize is done"));
	}
#endif

	// register event
	if(first_free_event == NULL) {

		logging->out_logf(LOG_ERROR,_T("EVENT: too many events !!! device_id:%d [%s:%s] event_id:%d")
			,device->get_id(), device->get_class_name(), device->get_identifier() ,event_id);

		if(register_id != NULL) {
			*register_id = -1;
		}
		return;
	}
	event_t *event_handle = first_free_event;
	first_free_event = first_free_event->next;

	if(register_id != NULL) {
		*register_id = event_handle->index;
	}
	event_handle->active = true;
	event_handle->device = device;
	event_handle->event_id = event_id;
	event_handle->expired_clock = event_clocks + clock;
	event_handle->loop_clock = loop ? clock : 0;

	if (expire_clock) *expire_clock = event_handle->expired_clock;

	insert_event(event_handle);
}

void EVENT::insert_event(event_t *event_handle)
{
//	logging->out_debugf(_T("EVENT: insert_event: device_id:%d event_id:%d register_id:%d")
//		,event_handle->device->this_device_id,event_handle->event_id,event_handle->index);
	if(first_fire_event == NULL) {
		first_fire_event = event_handle;
		event_handle->prev = event_handle->next = NULL;
	}
	else {
		for(event_t *insert_pos = first_fire_event; insert_pos != NULL; insert_pos = insert_pos->next) {
			if(insert_pos->expired_clock > event_handle->expired_clock) {
				if(insert_pos->prev != NULL) {
					// insert
					insert_pos->prev->next = event_handle;
					event_handle->prev = insert_pos->prev;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				}
				else {
					// add to head
					first_fire_event = event_handle;
					event_handle->prev = NULL;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				}
			}
			else if(insert_pos->next == NULL) {
				// add to tail
				insert_pos->next = event_handle;
				event_handle->prev = insert_pos;
				event_handle->next = NULL;
				break;
			}
		}
	}
}

void EVENT::modify_event_by_clock(int register_id, int clock)
{
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(event_handle->active) {
			event_handle->expired_clock += clock;
			if (event_handle->loop_clock) event_handle->loop_clock += clock;
		}
		if (clock > 0) {
			// sort
			for(event_t *pos = event_handle->next; pos != NULL; pos = pos->next) {
				if (pos->expired_clock >= event_handle->expired_clock) break;
				// swap
				event_handle->prev->next = pos;
				pos->next->prev = event_handle;
				pos->prev = event_handle->prev;
				event_handle->next = pos->next;
				event_handle->prev = pos;
				pos->next = event_handle;
				if (pos->prev == NULL) {
					first_fire_event = pos;
				}

				pos = event_handle;
			}
		} else if (clock < 0) {
			// sort
			for(event_t *pos = event_handle->prev; pos != NULL; pos = pos->prev) {
				if (pos->expired_clock <= event_handle->expired_clock) break;
				// swap
				event_handle->next->prev = pos;
				pos->prev->next = event_handle;
				pos->next = event_handle->next;
				event_handle->prev = pos->prev;
				event_handle->next = pos;
				pos->prev = event_handle;
				if (event_handle->prev == NULL) {
					first_fire_event = event_handle;
				}

				pos = event_handle;
			}
		}
	}
}

void EVENT::cancel_event(DEVICE *device, int register_id)
{
	// cancel registered event
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(device != NULL && device != event_handle->device) {
			logging->out_debugf(_T("EVENT: event cannot be canceled by non owned device!!! device_id:%d [%s:%s]")
				, device->get_id(), device->get_class_name(), device->get_identifier());
			return;
		}
		if(event_handle->active) {
//			logging->out_debugf(_T("EVENT: cancel_event: device_id:%d event_id:%d register_id:%d")
//				,event_handle->device->this_device_id,event_handle->event_id,event_handle->index);
			if(event_handle->prev != NULL) {
				event_handle->prev->next = event_handle->next;
			} else {
				first_fire_event = event_handle->next;
			}
			if(event_handle->next != NULL) {
				event_handle->next->prev = event_handle->prev;
			}
			event_handle->device = NULL;
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
	}
}

int EVENT::is_registerd_event(const char *class_name, const char *identifier, int register_id)
{
	int found = -1;
	int id = 0;
	int ide = MAX_EVENT;
	if (register_id >= 0) {
		id = register_id;
		ide = id + 1;
	}
	for(; id < ide; id++) {
		event_t *event_handle = &event[id];
		if(event_handle->active
		 && strncmp(event_handle->device->get_class_name(), class_name, 12) == 0
		 && strncmp(event_handle->device->get_identifier(), identifier, 4) == 0) {
			 found = id;
		}
	}
	return found;
}

void EVENT::register_frame_event(DEVICE* dev)
{
	if(frame_event_count < MAX_EVENT) {
		frame_event[frame_event_count++] = dev;
	}
	else {
		logging->out_log(LOG_ERROR,_T("EVENT: too many frame events !!!"));
	}
}

bool EVENT::is_registerd_frame_event(const char *class_name, const char *identifier)
{
	bool found = false;
	for(int i=0; i < frame_event_count; i++) {
		DEVICE *device = frame_event[i];
		if(strncmp(device->get_class_name(), class_name, 12) == 0
		&& strncmp(device->get_identifier(), identifier, 4) == 0) {
			 found = true;
			 break;
		}
	}
	return found;
}

void EVENT::register_vline_event(DEVICE* dev)
{
	if(vline_event_count < MAX_EVENT) {
		vline_event[vline_event_count++] = dev;
	}
	else {
		logging->out_log(LOG_ERROR,_T("EVENT: too many vline events !!!"));
	}
}

bool EVENT::is_registerd_vline_event(const char *class_name, const char *identifier)
{
	bool found = false;
	for(int i=0; i < vline_event_count; i++) {
		DEVICE *device = vline_event[i];
		if(strncmp(device->get_class_name(), class_name, 12) == 0
		&& strncmp(device->get_identifier(), identifier, 4) == 0) {
			 found = true;
			 break;
		}
	}
	return found;
}

void EVENT::mix_sound(int samples)
{
	int32_t *p;

	if (!((*vm_pause) & ~VM_POWEROFF_MASK)) {
		if(samples > 0) {
			emu->lock_sound_buffer();
			memset(sound_tmp + sound_buffer_ptr * 2, 0, samples * sizeof(int32_t) * 2);
			for(int i = 0; i < dcount_sound; i++) {
				d_sound[i]->mix(sound_tmp + sound_buffer_ptr * 2, samples);
			}
			// control volume
			p = sound_tmp + sound_buffer_ptr * 2;
			for(int i = 0; i < (samples << 1); i++) {
				*p = *p * master_volume / 16384;
				// limit
				if ((*p) >= 32768) *p = 32767;
				else if ((*p) < -32768) *p = -32768;
				p++;
			}
			// record sound if need
			record_sound(samples);

			sound_buffer_ptr += samples;
			emu->unlock_sound_buffer();
		}
		else {
			// notify to sound devices
			emu->lock_sound_buffer();
			for(int i = 0; i < dcount_sound; i++) {
				d_sound[i]->mix(sound_tmp + sound_buffer_ptr * 2, 0);
			}
			emu->unlock_sound_buffer();
		}
	} else {
		// now pausing
		if(samples > 0) {
			emu->lock_sound_buffer();
			memset(sound_tmp + sound_buffer_ptr * 2, 0, samples * sizeof(int32_t) * 2);

			// record sound if need
			record_sound(samples);

			sound_buffer_ptr += samples;
			emu->unlock_sound_buffer();
		}
	}
}

void EVENT::update_sound(int v)
{
	accum_samples += (update_samples + update_samples_adjust);
	int samples = (accum_samples >> 9 >> event_power);
	accum_samples -= (samples << 9 << event_power);
//	int samples = (int)(update_sound_rate_f / lines_per_frame / frames_per_sec / 16.0);
//	update_sound_rate_f = sound_rate_f + (update_sound_rate_f - (int)(samples * lines_per_frame * frames_per_sec * 16.0));

	// mix sound
	if(sound_tmp_samples - sound_buffer_ptr < samples) {
		samples = sound_tmp_samples - sound_buffer_ptr;
		sound_buffer_overflow++;
	}
#ifdef _DEBUG_SOUND_LATE
//	logging->out_debugf(_T("EVENT::update_sound: clock:%lld"), event_clocks);
	for(int i=0; i<samples; i++) {
		sound_tmp_clocks[sound_buffer_ptr+i] = event_clocks;
	}
#endif
//	logging->out_debugf("EVENT::update_sound accum_samples:%d samples:%d sound_tmp_samples:%d sound_buffer_ptr:%d",accum_samples,samples,sound_tmp_samples,sound_buffer_ptr);
	mix_sound(samples);
}

/// @attention called by another thread (EMU::update_sound).
/// @param[in] extra_frames 0
/// @param[in] samples      requested samples by DirectSound / SDLSound
audio_sample_t* EVENT::create_sound(int* extra_frames, int samples)
{
	int frames = 0;

#ifndef EVENT_CONTINUOUS_SOUND
	// fill sound buffer
	int samples = sound_samples - sound_buffer_ptr;
	mix_sound(samples);
#endif
#ifdef LOW_PASS_FILTER
	// low-pass filter
	for(int i = 0; i < samples - 1; i++) {
		sound_tmp[i * 2    ] = (sound_tmp[i * 2    ] + sound_tmp[i * 2 + 2]) / 2; // L
		sound_tmp[i * 2 + 1] = (sound_tmp[i * 2 + 1] + sound_tmp[i * 2 + 3]) / 2; // R
	}
#endif
#ifdef _DEBUG_SOUND_LATE
	logging->out_logf(LOG_DEBUG,_T("EVENT::create_sound 1 samples:%d buf_ptr:%d ovflow:%d"),samples,sound_buffer_ptr,sound_buffer_overflow);
#endif
#ifdef EVENT_CONTINUOUS_SOUND
	//  skip if buffer is not fill
	if (samples > sound_buffer_ptr) {
#ifdef _DEBUG_SOUND_ADJUST
		if (sound_buffer_ptr > 0) logging->out_logf(LOG_DEBUG,_T("EVENT::create_sound underflow: samples:%d buf_ptr:%d"),samples,sound_buffer_ptr);
#endif
		*extra_frames = frames;
		return sound_empty;
	}
#endif
#ifdef _DEBUG_SOUND_LATE
	logging->out_logf(LOG_DEBUG,_T("EVENT::create_sound: bufclk:%lld-%lld nowclk:%lld d:%lld"),sound_tmp_clocks[0],sound_tmp_clocks[sound_buffer_ptr-1],event_clocks,event_clocks-sound_tmp_clocks[0]);
#endif
	// copy to buffer
	for(int i = 0; i < samples * 2; i++) {
#ifdef USE_AUDIO_U8
		sound_buffer[i] = (audio_sample_t)(sound_tmp[i] / 256) + 128;
#else
		sound_buffer[i] = (audio_sample_t)sound_tmp[i];
#endif
	}
	if(sound_buffer_ptr > samples && sound_buffer_overflow < 60) {
		if (sound_buffer_overflow < 10) {
			sound_buffer_ptr -= samples;
		} else {
			sound_buffer_ptr -= samples + samples / 2;
#ifdef _DEBUG_SOUND_ADJUST
			logging->out_logf(LOG_DEBUG,_T("EVENT::create_sound overflow: %d samples:%d buf_ptr:%d"),sound_buffer_overflow,samples,sound_buffer_ptr);
#endif
		}
		// shift buffer (samples * stereo(2))
		memcpy(sound_tmp, sound_tmp + samples * 2, sound_buffer_ptr * sizeof(int32_t) * 2);
#ifdef _DEBUG_SOUND_LATE
		memcpy(sound_tmp_clocks, sound_tmp_clocks + samples, sound_buffer_ptr * sizeof(uint64_t));
#endif
	}
	else {
		sound_buffer_ptr = 0;
	}
#ifdef _DEBUG_SOUND_LATE
	logging->out_debugf("EVENT::create_sound:2 sound_samples:%d samples:%d buf_ptr:%d", sound_samples, samples, sound_buffer_ptr);
#endif

	sound_buffer_overflow = 0;

	*extra_frames = frames;
	return sound_buffer;
}

void EVENT::set_volume(int decibel, bool vol_mute)
{
	master_volume = int(16384.0 * pow(10.0, decibel / 40.0));
	if (vol_mute) master_volume = 0;
}

void EVENT::record_sound(int samples)
{
	if (*now_rec_sound) {
		emu->record_rec_sound(&sound_tmp[sound_buffer_ptr << 1], samples);
	}
}

void EVENT::update_config()
{
#ifdef USE_EMU_INHERENT_SPEC
	if (pConfig->sync_irq) {
		event_power = pConfig->cpu_power;
		cpu_power = 1;
		cpu_accum = 0;
	} else {
		cpu_power = pConfig->cpu_power;
		event_power = 1;
		cpu_accum = 0;
	}

	draw_count_per_frame = 1;
#ifdef USE_AFTERIMAGE
	if (pConfig->afterimage != 0) {
		draw_count_per_frame = 2;
	}
#endif
#ifdef USE_KEEPIMAGE
	if (pConfig->keepimage != 0) {
		draw_count_per_frame = 2;
	}
#endif

#endif

//	sound_rate_f = ((sound_rate << 5) >> event_power);
//	update_sound_rate_f = sound_rate_f;
}

#ifdef USE_EMU_INHERENT_SPEC
void EVENT::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifdef USE_CPU_HALF_SPEED
	switch(id) {
	case SIG_EVENT_CPU_HALF_SPEED:
		cpu_speed_half_res = ((data & mask) ? 1 : 0);
		break;
	}
#endif
}
#endif

// ----------------------------------------------------------------------------

void EVENT::save_state(FILEIO *fio)
{
	struct vm_state_st *vm_state = NULL;

	//
#ifdef USE_CPU_HALF_SPEED
	vm_state_ident.version = Uint16_LE(0x42);
#else
	vm_state_ident.version = Uint16_LE(2);
#endif
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(struct vm_state_st));
	fio->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);

	// copy events
	fio->FputUint64_LE(event_clocks);

	int first_free_event_index = -1;
	for(int j=0; j<MAX_EVENT; j++) {
		if (first_free_event == &event[j]) {
			first_free_event_index = j;
			break;
		}
	}
	fio->FputInt32_LE(first_free_event_index);

	int first_fire_event_index = -1;
	for(int j=0; j<MAX_EVENT; j++) {
		if (first_fire_event == &event[j]) {
			first_fire_event_index = j;
			break;
		}
	}
	fio->FputInt32_LE(first_fire_event_index);

	for(int i=0; i<MAX_EVENT; i++) {
		if (event[i].device != NULL) {
			fio->Fwrite(event[i].device->get_class_name(), 12, 1);
			fio->Fwrite(event[i].device->get_identifier(), 4, 1);
		} else {
			fio->Fsets(0, sizeof(vm_state->event[i].class_name));
			fio->Fsets(0, sizeof(vm_state->event[i].identifier));
		}
		fio->FputInt32_LE(event[i].event_id);
		fio->FputUint64_LE(event[i].expired_clock);
		fio->FputUint32_LE(event[i].loop_clock);
		fio->FputUint8(event[i].active ? 1 : 0);
		fio->FputInt32_LE(event[i].index);

		int next_index = -1;
		for(int j=0; j<MAX_EVENT; j++) {
			if (event[i].next == &event[j]) {
				next_index = j;
				break;
			}
		}
		fio->FputInt32_LE(next_index);

		int prev_index = -1;
		for(int j=0; j<MAX_EVENT; j++) {
			if (event[i].prev == &event[j]) {
				prev_index = j;
				break;
			}
		}
		fio->FputInt32_LE(prev_index);

		fio->Fsets(0, sizeof(vm_state->event[i].reserved));
	}

#ifdef USE_CPU_HALF_SPEED
	// 0x42
	fio->FputUint8(cpu_speed_half | (cpu_speed_half_res << 1));
#else
	// Version 2
	fio->FputUint8(0);
#endif
	fio->FputUint8((uint8_t)ncount_cpu);
	fio->Fsets(0, sizeof(vm_state->reserved));

	fio->FputInt32_LE(event_remain);
	fio->FputInt32_LE(cpu_remain);
	fio->FputInt32_LE(cpu_accum);

	fio->FputDouble_LE(next_frames_per_sec);
	fio->FputInt32_LE(next_lines_per_frame);
	fio->FputInt32_LE(frame_split_num);
}

bool EVENT::load_state(FILEIO *fio)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st *vm_state = NULL;

	if (find_state_chunk(fio, &vm_state_i) != true) {
		return true;
	}

	// restore events
	event_clocks = fio->FgetUint64_LE();

	int first_free_event_index = fio->FgetInt32_LE();
	if (first_free_event_index < 0) {
		first_free_event = NULL;
	} else {
		first_free_event = &event[first_free_event_index];
	}

	int first_fire_event_index = fio->FgetInt32_LE();
	if (first_fire_event_index < 0) {
		first_fire_event = NULL;
	} else {
		first_fire_event = &event[first_fire_event_index];
	}

	for(int i=0; i<MAX_EVENT; i++) {
		char class_name[13];
		char identifier[5];

		memset(class_name, 0, sizeof(class_name));
		fio->Fread(class_name, 12, 1);
		memset(identifier, 0, sizeof(identifier));
		fio->Fread(identifier, 4, 1);
		if (class_name[0] == '\0') {
			event[i].device = NULL;
		} else {
			event[i].device = vm->get_device(class_name, identifier);
		}

		event[i].event_id = fio->FgetInt32_LE();
		event[i].expired_clock = fio->FgetUint64_LE();
		event[i].loop_clock = fio->FgetUint32_LE();
		event[i].active = fio->FgetUint8() ? true : false;
		event[i].index = fio->FgetInt32_LE();

		int next_index = fio->FgetInt32_LE();
		if (next_index < 0) {
			event[i].next = NULL;
		} else {
			event[i].next = &event[next_index];
		}

		int prev_index = fio->FgetInt32_LE();
		if (prev_index < 0) {
			event[i].prev = NULL;
		} else {
			event[i].prev = &event[prev_index];
		}

		fio->Fseek(sizeof(vm_state->event[i].reserved), FILEIO::SEEKCUR);

#ifdef USE_CPU_HALF_SPEED
		if (vm_state_i.version < 0x41) {
			// L3 compatible
			event[i].expired_clock <<= 1;
			event[i].loop_clock <<= 1;
		}
#endif
	}

#ifdef USE_CPU_HALF_SPEED
	cpu_speed_half = 0;
	if (Uint16_LE(vm_state_i.version) >= 0x42) {
		// 0x42
		cpu_speed_half = fio->FgetUint8();
		ncount_cpu = fio->FgetUint8();
		fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);
		event_remain = fio->FgetInt32_LE();
		cpu_remain = fio->FgetInt32_LE();
		cpu_accum = fio->FgetInt32_LE();
	} else if (Uint16_LE(vm_state_i.version) >= 0x41) {
		cpu_speed_half = fio->FgetUint8();
		ncount_cpu = fio->FgetUint8();
		fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);
		fio->Fseek(sizeof(vm_state->event_remain), FILEIO::SEEKCUR);
		fio->Fseek(sizeof(vm_state->cpu_remain), FILEIO::SEEKCUR);
		fio->Fseek(sizeof(vm_state->cpu_accum), FILEIO::SEEKCUR);
	} else {
		// L3 compatible
		event_clocks <<= 1;
		if (Uint16_LE(vm_state_i.version) >= 2) {
			fio->Fseek(sizeof(vm_state->cpu_speed_half), FILEIO::SEEKCUR);
			ncount_cpu = fio->FgetUint8();
			fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);
			event_remain = fio->FgetInt32_LE();
			cpu_remain = fio->FgetInt32_LE();
			cpu_accum = fio->FgetInt32_LE();
		}
	}

	cpu_speed_half_res = ((cpu_speed_half & 0x02) >> 1);
	cpu_speed_half &= 0x01;

#else
	if (Uint16_LE(vm_state_i.version) >= 2) {
		// Version 2
		fio->FgetUint8();
		ncount_cpu = fio->FgetUint8();
		fio->Fseek(sizeof(vm_state->reserved), FILEIO::SEEKCUR);
		event_remain = fio->FgetInt32_LE();
		cpu_remain = fio->FgetInt32_LE();
		cpu_accum = fio->FgetInt32_LE();

		next_frames_per_sec = fio->FgetDouble_LE();
		next_lines_per_frame = fio->FgetInt32_LE();
		frame_split_num = fio->FgetInt32_LE();

		frames_per_sec = next_frames_per_sec + 1;
		lines_per_frame = next_lines_per_frame + 1;
		first_frame = (frame_split_num == 0);
	}
#endif

	if (ncount_cpu <= 0 || ncount_cpu > 2) {
		ncount_cpu = 1;
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER

#include "../utility.h"

void EVENT::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	UTILITY::tcscpy(buffer, buffer_len, _T("EVENT:\n"));
	int last_idx = MAX_EVENT;
	for(int i=MAX_EVENT-1; i>=0; i--) {
		event_t *h = &event[i];
		if (h->device != NULL || h->event_id >= 0 || h->expired_clock != 0 || h->loop_clock != 0) {
			last_idx = i + 1;
			break;
		}
	}
	for(int i=0; i<last_idx; i++) {
		event_t *h = &event[i];
		UTILITY::sntprintf(buffer, buffer_len, _T("%02d:"), i);
		if (h->active) {
			// active
			UTILITY::tcscat(buffer, buffer_len, _T(" on:  "));
		} else {
			// inactive
			UTILITY::tcscat(buffer, buffer_len, _T(" off: "));
		}
		if (h->device) {
			UTILITY::sntprintf(buffer, buffer_len, _T("%-12s(%-4s) ")
				, h->device->get_class_name(), h->device->get_identifier());
		} else {
			UTILITY::tcscat(buffer, buffer_len, _T("            (    ) "));
		}
		UTILITY::sntprintf(buffer, buffer_len, _T("id:%d expire:%lld loop:%d  prev:%02d next:%02d\n")
			, h->event_id, h->expired_clock, h->loop_clock
			, h->prev ? h->prev->index : -1
			, h->next ? h->next->index : -1
		);
	}
}
#endif
