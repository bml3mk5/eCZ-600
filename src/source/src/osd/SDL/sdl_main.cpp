/** @file sdl_main.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.2.15

	@brief [ sdl_main ]

	@note
	This code is based on winmain.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include <sys/types.h>
#include "../../common.h"
#include "../../config.h"
#include "sdl_emu.h"
#include "../../emumsg.h"
#include "../../gui/gui.h"
#include "../../clocale.h"
#include "../../utility.h"
#include "sdl_main.h"
#include "sdl_parseopt.h"
#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#endif

#undef LOG_MEASURE

#define MIN_INTERVAL	4

inline void calc_next_interval(int &now_ms, int &interval_ms, int delta_ms, double frames_per_sec)
{
	interval_ms = (int)(now_ms / frames_per_sec);
	now_ms = now_ms - (int)(interval_ms * frames_per_sec) + delta_ms;
}

// variable

/// emulation core
EMU *emu = NULL;
GUI *gui = NULL;

/// ALT key
int *key_mod = NULL;

bool g_need_update_screen = false;
bool need_update_title = false;
SDL_mutex *mux_allow_update_screen = NULL;
SDL_cond *g_cond_allow_update_screen = NULL;

/// for calculate frame rate
typedef struct st_frame_count {
	int total;
	int draw;
	int skip;
} t_frame_count;

t_frame_count frames_result;

/// timing control
#define MAX_SKIP_FRAMES 10


/// emulator thread
SDL_Thread *emu_thread = NULL;
bool emu_thread_working = false;
// emulator thread procedure
static int emu_event_loop(void *param);

// main

static void main_event_loop();

static int event_proc(SDL_Event *);

/**
 *	main
 */
int main(int argc, char* argv[])
{
#if defined(_DEBUG) && defined(_WIN32) && !defined(__MINGW32__)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	int rc = 0;
	int need_resize = 0;

	Uint32 init_flags = 0;
	Uint32 video_flags = 0;
	int status = 0;

#ifdef _WIN32
	CoInitialize(NULL);
#endif

	// get and parse options
	CParseOptions *options = new CParseOptions(argc, argv);

	// logging
	logging = new Logging(options->get_ini_path());

	// i18n
#if defined(__APPLE__) && defined(__MACH__)
	clocale = new CLocale(options->get_res_path(), CONFIG_NAME, "");
#else
	clocale = new CLocale(options->get_app_path(), CONFIG_NAME, "");
#endif

	// create a instance of emulation core
	emu = new EMU_OSD(options->get_app_path(), options->get_ini_path(), options->get_res_path());
	logging->set_receiver(emu);

	// load config
	pConfig = new Config;
	pConfig->load(options->get_ini_file());

	// change language if need
	clocale->ChangeLocaleIfNeed(pConfig->language);
	logging->out_logc(LOG_INFO, _T("Locale:["), clocale->GetLocaleName(), _T("] Lang:["), clocale->GetLanguageName(), _T("]"), NULL);

	// gui class
	gui = new GUI(argc, argv, emu);
	rc = gui->Init();
	if (rc == -1) {
		rc = 1;
		goto ERROR_FIN;
	} else if (rc == 1) {
		// use english message.
		clocale->UnsetLocale();
		logging->out_log(LOG_WARN, _T("Use default locale message."));
	}
	emu->set_gui(gui);

	// disable keylock
//	SDL_putenv("SDL_DISABLE_LOCK_KEYS=1");

#ifdef GUI_USE_FOREIGN_WINDOW
	// create window without SDL library
	if (!gui->CreateWindow(MIN_WINDOW_WIDTH,MIN_WINDOW_HEIGHT)) {
		rc = 1;
		goto ERROR_FIN;
	}
#endif

	// init SDL
	init_flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
#ifndef _DEBUG
	init_flags |= SDL_INIT_NOPARACHUTE;
#endif
	if(SDL_Init(init_flags)) {
		logging->out_logf(LOG_ERROR, _T("SDL_Init: %s."), SDL_GetError());
		rc = 1;
		goto ERROR_FIN;
	}

#ifdef USE_SDL2
	/* SDL 2 */
#ifdef USE_OPENGL
	emu->set_use_opengl(pConfig->use_opengl);
#endif
	// create window
	emu->init_screen_mode();
	if (!emu->create_screen(pConfig->disp_device_no, pConfig->window_position_x, pConfig->window_position_y, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, video_flags)) {
		rc = 1;
		goto ERROR_FIN;
	}

#if 0
#if !(defined(__APPLE__) && defined(__MACH__))
	// window icon
	char buf[_MAX_PATH];
	SDL_Surface *icon;
	sprintf(buf, "%s%s.bmp", options->get_res_path(), CONFIG_NAME);
	icon = SDL_LoadBMP(buf);
	if (icon) {
		SDL_SetWindowIcon(((EMU_OSD *)emu)->get_window(), icon);
		SDL_FreeSurface(icon);
	}
#endif
#endif

	SDL_StopTextInput();

#else /* USE_SDL2 */
	/* SDL 1 */
	video_flags |= SDL_HWSURFACE;

#if (defined(__APPLE__) && defined(__MACH__))
	// for mac
	video_flags |= SDL_ASYNCBLIT;
#endif

#ifdef USE_OPENGL
	emu->set_use_opengl(pConfig->use_opengl);
#endif
	// create window
	emu->init_screen_mode();
	if (!emu->create_screen(pConfig->disp_device_no, pConfig->window_position_x, pConfig->window_position_y, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, video_flags)) {
		rc = 1;
		goto ERROR_FIN;
	}

	// Window title
	SDL_WM_SetCaption(DEVICE_NAME, NULL);

#if 0
#if !(defined(__APPLE__) && defined(__MACH__))
	// window icon
	char buf[_MAX_PATH];
	SDL_Surface *icon;
	sprintf(buf, "%s%s.bmp", options->get_res_path(), CONFIG_NAME);
	icon = SDL_LoadBMP(buf);
	if (icon) {
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface(icon);
	}
#endif
#endif

#endif /* !USE_SDL2 */

	// create gui menu
	need_resize = gui->CreateMenu();
	if (need_resize == -1) {
		rc = 1;
		goto ERROR_FIN;
	}
	// create global keys
	if (gui->CreateGlobalKeys() == -1) {
		rc = 1;
		goto ERROR_FIN;
	}

	// initialize emulation core
	emu->initialize();
	// restore screen mode
	if(pConfig->window_mode >= 0 && pConfig->window_mode < 8) {
		emu->change_screen_mode(pConfig->window_mode);
	}
	else if(pConfig->window_mode >= 8) {
		int prev_mode = pConfig->window_mode;
		pConfig->window_mode = 0;	// initialize window mode
		emu->change_screen_mode(prev_mode);
	}
	// use offscreen surface
	if (!emu->create_offlinesurface()) {
		rc = 1;
		goto ERROR_FIN;
	}

	key_mod = emu->get_key_mod_ptr();

	// vm start
	if (emu) {
		emu->reset();
	}

	// open files specified on command line path
	options->open_recent_file(gui);

	// init mutex
	mux_allow_update_screen = SDL_CreateMutex();
	g_cond_allow_update_screen =SDL_CreateCond();

	/// create emulator thread
#ifndef USE_SDL2
	if ((emu_thread = SDL_CreateThread(emu_event_loop, (void *)NULL)) == NULL) {
#else
	if ((emu_thread = SDL_CreateThread(emu_event_loop, "emu_thread", (void *)NULL)) == NULL) {
#endif
		logging->out_logf(LOG_ERROR, _T("SDL_CreateThread: %s."), SDL_GetError());
		rc = 1;
		goto FIN;
	}

	// main event loop
	rc = 0;
	main_event_loop();

	// wait thread
	emu_thread_working = false;
	if (emu_thread) {
		SDL_WaitThread(emu_thread, &status);
//		if (0) {
//			// kill thread by force
//			SDL_KillThread(emu_thread);
//		}
	}
	emu_thread = NULL;

FIN:
	// delete mutex
	SDL_DestroyCond(g_cond_allow_update_screen);
	SDL_DestroyMutex(mux_allow_update_screen);

ERROR_FIN:
	// release emulation core
	if(emu) {
		emu->release();
	}

	// save config
	pConfig->save();
	pConfig->release();

	delete pConfig;
	delete gui;
	logging->set_receiver(NULL);
	delete emu;
	delete clocale;
	delete logging;
	delete options;

//#if defined(_DEBUG) && defined(_WIN32) && !defined(__MINGW32__)
//	_CrtDumpMemoryLeaks();
//#endif

	SDL_Quit();

#ifdef _WIN32
	CoUninitialize();
#endif

	return rc;
}

/**
 *	main event loop
 */
static void main_event_loop()
{
	char buf[256];

	SDL_Event event;
	int msgcnt = 0;
	bool working = true;
	int rc = 0;

//	t_frame_count main_frame = {0,0,0};

	SDL_mutexP(mux_allow_update_screen);
	while (working) {
		gui->PreProcessEvent();
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG,"M1: need_update:%d",need_update ? 1:0);
#endif
		msgcnt = 100;
		while (working && SDL_PollEvent(&event) && msgcnt > 0) {
//			logging->out_debugf("event: %d", event.type);
			rc = gui->ProcessEvent(&event);
			if (rc > 0) {
				rc = event_proc(&event);
			}
			if (rc < 0) {
				working = false;
				break;
			}
			msgcnt--;
//			main_frame.draw++;
		}
		gui->PostProcessEvent();

#ifdef USE_SOCKET
		emu->check_socket();
#endif

		if (need_update_title) {
			need_update_title = false;
//			int ratio = 0;
//			if (frames_result.total) ratio = (int)(100.0 * (double)frames_result.draw / (double)frames_result.total + 0.5);
			sprintf(buf, "%s - %d/%dfps", DEVICE_NAME, frames_result.draw, frames_result.total);
#ifndef USE_SDL2
			SDL_WM_SetCaption(buf, NULL);
#else
			SDL_Window *window = ((EMU_OSD *)emu)->get_window();
			if (window) SDL_SetWindowTitle(window, buf);
#endif
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG,"M2: need_update:%d",need_update ? 1:0);
#endif
		if (!g_need_update_screen) {
			if (SDL_CondWaitTimeout(g_cond_allow_update_screen, mux_allow_update_screen, 17) != 0) {
//				// when no request update screen from emu thread
//				gui->DecreaseUpdateScreenCount();
			}
		}
//		main_frame.total++;
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG,"M3: need_update:%d",need_update ? 1:0);
#endif
	}
	SDL_mutexV(mux_allow_update_screen);
	return;
}

/**
 *	main event procedure
 *
 * @return -1:exit program 0:processed event 1: no process
 */
static int event_proc(SDL_Event *e)
{
	int rc = 1;
	switch(e->type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if(emu) {
			rc = emu->key_down_up(e->type & 1, (int)e->key.keysym.sym, (long)e->key.keysym.scancode);
		}
		break;
//	case SDL_VIDEORESIZE:
//		break;
	case SDL_MOUSEBUTTONDOWN:
		if (emu) {
			SDL_MouseButtonEvent *me = (SDL_MouseButtonEvent *)e;
			// When right click at lefttop side in fullscreen, go to window mode
			if (me->button == SDL_BUTTON_RIGHT) {
				if (emu->is_fullscreen()) {
					if (me->x < 8 && me->y < 8) {
						// goto window mode
						emu->change_screen_mode(0);
					}
				}
			}
		}
		break;
	case SDL_MOUSEMOTION:
		if (emu) {
			emu->mouse_move(e->motion.x, e->motion.y);
		}
		break;
	case SDL_QUIT:
		if (emu) {
			emu->resume_window_placement();
		}
		logging->out_debug(_T("SDL_QUIT"));
		rc = -1;
		break;
#ifdef USE_SOCKET
	case SDL_USEREVENT_SOCKET:
		if (emu) {
			int code = e->user.code;
			int no = (int)(intptr_t)e->user.data1;
			switch(code) {
				case SDL_USEREVENT_SOCKET_CONNECTED:
					emu->socket_connected(no);
					break;
				case SDL_USEREVENT_SOCKET_DISCONNECTED:
					emu->socket_disconnected(no);
					break;
				case SDL_USEREVENT_SOCKET_WRITEABLE:
					emu->socket_writeable(no);
					emu->send_data(no);
					break;
//				case SDL_USEREVENT_SOCKET_READABLE:
//					emu->recv_data(no);
//					emu->socket_readable(no);
//					break;
//				case SDL_USEREVENT_SOCKET_ACCEPT:
//					emu->socket_accept(no);
//					break;
			}
		}
		rc = 0;
		break;
#endif
	}
	return rc;
}

/**
 *  emulator thread event loop
 *
 *  @attention called by another thread
 */
static int emu_event_loop(void *param)
{
	emu_thread_working = true;

	const int fskip[6] = {1, 2, 3, 4, 5, 6};
	int fskip_remain = 0;

	t_frame_count frames = { 0, 0, 0 };

	int ms = 1000;
	int next_interval;
	calc_next_interval(ms, next_interval, 1000, FRAMES_PER_SEC);

	int split_num = 0;

	uint32_t current_time = 0;
	int    remain_time = 0;
	uint32_t ideal_next_time = 0;
	uint32_t real_next_time = 0;
#ifdef LOG_MEASURE
	uint32_t skip_reason = 0;
#endif
#ifdef USE_PERFORMANCE_METER
	uint32_t lpCount1,lpCount2,lpCount3;
	lpCount1 = lpCount2 = lpCount3 = 0;
#endif

	// wait a sec.
	CDelay(500);

	uint32_t update_fps_time = SDL_GetTicks() + 1000;

	// play sound
	emu->mute_sound(false);

	// process coming messages at first.
	while (emumsg.Process()) {}

	ideal_next_time = SDL_GetTicks();

#ifdef USE_PERFORMANCE_METER
	lpCount1 = ideal_next_time;
#endif

	// loop
	while (emu_thread_working) {
		split_num = 0;
		// get next period
		calc_next_interval(ms, next_interval, 1000, emu->get_frame_rate());

//		start_time = next_time;
//		next_time += emu->now_skip() ? 0 : next_interval;
		ideal_next_time += next_interval;
		real_next_time += next_interval;
		current_time = SDL_GetTicks();
#ifdef LOG_MEASURE
		skip_reason = 0;
#endif

		// reset ideal time when proccesing is too late...
		if (current_time > ideal_next_time + 200) {
			ideal_next_time = current_time + next_interval;
#ifdef LOG_MEASURE
			skip_reason = 0x0200;
#endif
		}
		// sync next time when the real time have one frame difference from ideal time.
		if ((real_next_time > (next_interval + ideal_next_time)) || (ideal_next_time > (next_interval + real_next_time))) {
			real_next_time = ideal_next_time;
#ifdef LOG_MEASURE
			skip_reason = 0x0100;
#endif
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A1: cur:%ld nt:%ld %ld ms:%d int:%d st:%04x", current_time, ideal_next_time, real_next_time, ms, next_interval, skip_reason);
#endif
#if (FRAME_SPLIT_NUM > 1)
		for(int i=0; i < (FRAME_SPLIT_NUM - 1); i++) {
			// drive machine
			if(emu)	emu->run(split_num);
			split_num++;
			while (emumsg.Process()) {}
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A2: rti:%ld nt:%ld",SDL_GetTicks(),ideal_next_time);
#endif
#endif
		if(emu) {
			// drive machine
			emu->run(split_num);

			// record video
			emu->record_rec_video();

			frames.total++;

			if(pConfig->fps_no >= 0) {
				if (fskip_remain <= 0) {
					// constant frames per 1 second
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
					fskip_remain = fskip[pConfig->fps_no];
#ifdef LOG_MEASURE
					skip_reason |= 0x11;
#endif
				} else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x12;
#endif
				}
				if (fskip_remain > 0) fskip_remain--;
			}
			else {
				current_time = SDL_GetTicks();
				if(real_next_time > current_time) {
					// update window if enough time
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x01;
#endif
				}
				else if(frames.skip > MAX_SKIP_FRAMES) {
					// update window at least once per 10 frames
					if (gui->NeedUpdateScreen()) {
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x02;
#endif
//					real_next_time = SDL_GetTicks();
				}
				else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x03;
#endif
				}
			}
			emu->skip_screen(frames.skip > 0);
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A3: cur:%ld nt:%ld (total:%d fps:%d skip:%d) st:%04x",current_time,ideal_next_time,frames.total,frames.draw,frames.skip,skip_reason);
#endif
		gui->PostDrive();

		while (emumsg.Process()) {}
		if (frames.skip > 0) {
			real_next_time -= 2;
		}
		current_time = SDL_GetTicks();
#ifdef USE_PERFORMANCE_METER
		if (pConfig->show_pmeter) {
			lpCount2 = current_time;
		}
#endif
		remain_time = real_next_time - current_time;
#if defined(__APPLE__) && defined(__MACH__)
		if (remain_time > 3) {
			CDelay(remain_time - 2);
		}
#else
		if (remain_time > 3) {
			CDelay(remain_time - 1);
		}
#endif
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A4: rti:%ld nt:%ld %ld fu:%ld",SDL_GetTicks(),ideal_next_time,real_next_time,update_fps_time);
#endif
		// calc frame rate
		current_time = SDL_GetTicks();
		if(update_fps_time <= current_time + 8) {
			frames_result = frames;
			need_update_title = true;

			update_fps_time += 1000;
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			frames.total = frames.draw = 0;
		}
#ifdef USE_PERFORMANCE_METER
		if (pConfig->show_pmeter) {
			lpCount3 =  current_time;
			if (lpCount3 > lpCount1) {
				gdPMvalue = ((lpCount2 - lpCount1) * 100 / (lpCount3 - lpCount1)) & 0xfff;
			}
			lpCount1 = lpCount3;
		}
#endif
	}

	emu->release_on_emu_thread();

	return 0;
}
