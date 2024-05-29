/** @file win_main.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@note
	Modified for BML3MK5 by Sasaji at 2011.06.17
	Modified for MBS1 by Sasaji at 2015.08.11
	Modified for X68000 by Sasaji at 2022.02.22

	@brief [ win32 main ]
*/

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <commdlg.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include "win_main.h"
#include "../../gui/windows/winfont.h"
#include "../../clocale.h"
#include "../../res/resource.h"
#include "../../config.h"
#include "win_emu.h"
#include "../../emumsg.h"
#include "../../gui/gui.h"
#include "win_parseopt.h"

#undef LOG_MEASURE

#ifdef EXCEPTION_ILLEGAL_INSTRUCTION
#endif

#define MIN_INTERVAL	4

inline void calc_next_interval(int &now_ms, int &interval_ms, int delta_ms, double frames_per_sec)
{
	interval_ms = (int)(now_ms / frames_per_sec);
	now_ms = now_ms - (int)(interval_ms * frames_per_sec) + delta_ms;
}

#undef ENABLE_FREE_SIZING

// variable

/// emulation core
EMU *emu = NULL;
GUI *gui = NULL;

HINSTANCE hInstance = NULL;
HWND hMainWindow = NULL;

// buttons
#ifdef USE_BUTTON
#define MAX_FONT_SIZE 32
HFONT hFont[MAX_FONT_SIZE];
HWND hButton[MAX_BUTTONS];
WNDPROC buttonWndProc[MAX_BUTTONS];
#endif

/// key
//int *key_mod = NULL;

/// screen
//DWORD dwStyle;

/// client width on the window
int window_client_width;
/// client height on the window
int window_client_height;

#ifdef ENABLE_FREE_SIZING
int min_window_width;
int min_window_height;
int max_window_width;
int max_window_height;
#endif

bool need_update_title = false;

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
HANDLE emu_thread = NULL;
DWORD emu_thread_id = 0;
bool emu_thread_working = false;
// emulator thread procedure
static DWORD WINAPI EmuProc(LPVOID lpParameter);

// windows main
int WINAPI _tWinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance_, LPTSTR szCmdLine, int iCmdShow);
void register_my_class(HINSTANCE hInstance_);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
#ifdef USE_BUTTON
LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
#endif

/**
 *	main
 */
int WINAPI _tWinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance_, LPTSTR szCmdLine, int iCmdShow)
{
	int rc = 0;

	InitCommonControls();
	CoInitialize(NULL);
	// get and parse options
	CParseOptions *options = new CParseOptions(szCmdLine);

	// logging
	logging = new Logging(options->get_ini_path());

	// i18n
	clocale = new CLocale(options->get_app_path(), CONFIG_NAME, "");

	// create a instance of emulation core
	emu = new EMU_OSD(options->get_app_path(), options->get_ini_path(), options->get_ini_path());
	logging->set_receiver(emu);

	// load config
	pConfig = new Config;
	pConfig->load(options->get_ini_file());

	// change language if need
	clocale->ChangeLocaleIfNeed(pConfig->language);
	logging->out_logc(LOG_INFO, _T("Locale:["), clocale->GetLocaleName(), _T("] Lang:["), clocale->GetLanguageName(), _T("]"), NULL);

	hInstance = hInstance_;

	// gui class
	gui = new GUI(0, NULL, emu);
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

	//
	register_my_class(hInstance_);

	// create window

	// get window client size

#ifdef ENABLE_FREE_SIZING
	RECT rect;

	SetRect(&rect, 0, 0, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	AdjustWindowRectEx(&rect, dwStyle, TRUE, 0);
	min_window_width = (rect.right - rect.left);
	min_window_height = (rect.bottom - rect.top);
	SetRect(&rect, 0, 0, MAX_WINDOW_WIDTH, MAX_WINDOW_HEIGHT);
	AdjustWindowRectEx(&rect, dwStyle, TRUE, 0);
	max_window_width = (rect.right - rect.left);
	max_window_height = (rect.bottom - rect.top);
#endif

	// get window position
	window_client_width = MIN_WINDOW_WIDTH;
	window_client_height = MIN_WINDOW_HEIGHT;
//	SetRect(&rect, 0, 0, window_client_width, window_client_height);

	//	AdjustWindowRectEx(&rect, dwStyle, TRUE, 0);

	// enumerate screen mode
	emu->init_screen_mode();

	// show window

	if (!emu->create_screen(0, pConfig->window_position_x, pConfig->window_position_y, window_client_width, window_client_height, 0))
	{
		rc = 1;
		goto ERROR_FIN;
	}

//	hMainWindow = hWnd;

//	emu->set_handle(hWnd, hInstance_);
	if (gui->CreateWidget(hMainWindow, window_client_width, window_client_height) == -1) {
		rc = 1;
		goto ERROR_FIN;
	}
	ShowWindow(hMainWindow, iCmdShow);
	UpdateWindow(hMainWindow);

	// create menu
	if (gui->CreateMenu() == -1) {
		rc = 1;
		goto ERROR_FIN;
	}

	// show menu
	gui->ShowMenu();

	// accelerator
	HACCEL hAccel = LoadAccelerators(hInstance_, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	// disable ime
	ImmAssociateContext(hMainWindow, NULL);

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

	// open files specified on command line path
	options->open_recent_file(gui);

	timeBeginPeriod(MIN_INTERVAL);

	/// create emulator thread
	if ((emu_thread = ::CreateThread(NULL, 0, EmuProc, (LPVOID)hMainWindow, 0, &emu_thread_id)) == NULL) {
		logging->out_logf(LOG_ERROR, _T("CreateThread failed: %d"), GetLastError());
		rc = 1;
		goto FIN;
	}

	// main message loop
	MSG msg;
	int msgcnt = 0;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator(hMainWindow, hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	rc = (int)msg.wParam;

	// wait thread
	emu_thread_working = false;
	if (WaitForSingleObject(emu_thread, 10000) == WAIT_TIMEOUT) {
		// kill thread by force
		if (emu_thread) {
			TerminateThread(emu_thread, 0);
		}
	}

FIN:
	timeEndPeriod(MIN_INTERVAL);

ERROR_FIN:
	if(emu) {
		// quit fullscreen mode
		if(emu->is_fullscreen()) {
			ChangeDisplaySettings(NULL, 0);
		}
	}
#ifdef USE_BUTTON
	for(int i = 0; i < MAX_FONT_SIZE; i++) {
		if(hFont[i]) {
			DeleteObject(hFont[i]);
		}
	}
#endif
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

	CoUninitialize();

	// exit process
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return rc;
}

void register_my_class(HINSTANCE hInstance_)
{
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance_;
	wndclass.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
//	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wndclass.lpszMenuName = NULL;	// menu bar create later
	wndclass.lpszClassName = _T(CLASS_NAME);
	wndclass.hIconSm = NULL;
	RegisterClassEx(&wndclass);
}

/**
 *	main event procedure
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int no;
	LRESULT result = 1;

	switch(iMsg) {
	case WM_PAINT:
		if (need_update_title) {
			need_update_title = false;
			_TCHAR buf[256];
//			int ratio = 0;
//			if (frames_result.total) ratio = (int)(100.0 * (double)frames_result.draw / (double)frames_result.total + 0.5);
			_stprintf(buf, _T("%s - %d/%dfps"), _T(DEVICE_NAME), frames_result.draw, frames_result.total);
			SetWindowText(hWnd, buf);
		}
		hdc = BeginPaint(hWnd, &ps);
		if(gui) {
#ifdef LOG_MEASURE
			logging->out_logf(LOG_DEBUG,"WM_PAINT ST: thread_id:%ld %ld",GetCurrentThreadId(), timeGetTime());
#endif
			gui->UpdateScreen(hdc);
#ifdef LOG_MEASURE
			logging->out_logf(LOG_DEBUG,"WM_PAINT ED: %ld",timeGetTime());
#endif
		}
		EndPaint(hWnd, &ps);
		return 0;
//	case WM_ERASEBKGND:
//		return 0;
	case WM_CREATE:
#ifdef USE_BUTTON
		memset(hFont, 0, sizeof(hFont));
		for(int i = 0; i < MAX_BUTTONS; i++) {
			hButton[i] = CreateWindow(_T("BUTTON"), buttons[i].caption,
			                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_MULTILINE,
			                          buttons[i].x, buttons[i].y,
			                          buttons[i].width, buttons[i].height,
			                          hWnd, (HMENU)(ID_BUTTON + i), (HINSTANCE)GetModuleHandle(0), NULL);
			buttonWndProc[i] = (WNDPROC)(LONG_PTR)GetWindowLong(hButton[i], GWL_WNDPROC);
			SetWindowLong(hButton[i], GWL_WNDPROC, (LONG)(LONG_PTR)ButtonWndProc);
			//HFONT hFont = GetWindowFont(hButton[i]);
			if(!hFont[buttons[i].font_size]) {
				LOGFONT logfont;
				logfont.lfEscapement = 0;
				logfont.lfOrientation = 0;
				logfont.lfWeight = FW_NORMAL;
				logfont.lfItalic = FALSE;
				logfont.lfUnderline = FALSE;
				logfont.lfStrikeOut = FALSE;
				logfont.lfCharSet = DEFAULT_CHARSET;
				logfont.lfOutPrecision = OUT_TT_PRECIS;
				logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
				logfont.lfQuality = DEFAULT_QUALITY;
				logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
				_tcscpy(logfont.lfFaceName, _T("Arial"));
				logfont.lfHeight = buttons[i].font_size;
				logfont.lfWidth = buttons[i].font_size >> 1;
				hFont[buttons[i].font_size] = CreateFontIndirect(&logfont);
			}
			SetWindowFont(hButton[i], hFont[buttons[i].font_size], TRUE);
		}
#endif
		// enable to accept drag and drops
		DragAcceptFiles(hWnd, TRUE);

		break;
	case WM_CLOSE:
		if(emu) {
			emu->resume_window_placement();
#ifdef USE_POWER_OFF
			// notify power off
			static int notified = 0;
			if(!notified) {
				emu->notify_power_off();
				notified = 1;
				return 0;
			}
#endif
		}
#if 0
		// quit fullscreen mode
		if(emu->is_fullscreen()) {
			ChangeDisplaySettings(NULL, 0);
		}
#ifdef USE_BUTTON
		for(int i = 0; i < MAX_FONT_SIZE; i++) {
			if(hFont[i]) {
				DeleteObject(hFont[i]);
			}
		}
#endif
		// delete gui class
		delete gui;
		gui = NULL;
		// release emulation core
		if(emu) {
			emu->release();
		}

		save_config();
		release_config();

		if(emu) {
			delete emu;
		}
		emu = NULL;

		CoUninitialize();
#endif
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
#ifdef USE_BITMAP
	case WM_SIZE:
		if(emu) {
			emu->reload_bitmap();
		}
		break;
#endif
	case WM_KILLFOCUS:
		if(emu) {
			emu->key_lost_focus();
		}
		break;
	case WM_ENTERSIZEMOVE:
		if(emu) {
//			emu->mute_sound(true);
			emu->set_pause(1, true);
		}
		break;
	case WM_EXITSIZEMOVE:
		if(emu) {
			emu->key_lost_focus();
//			emu->mute_sound(false);
			emu->set_pause(1, false);
		}
		break;
	case WM_MOVING:
		if(emu) {
//			emu->mute_sound(true);
			emu->set_pause(1, true);
		}
		break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
#ifdef USE_ALT_F10_KEY
		return 0;	// not activate menu when hit ALT/F10
#endif
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (emu) {
			uint8_t type = (iMsg & 1);
#ifdef USE_DIRECTINPUT
			type |= (pConfig->use_direct_input == (Config::DIRECTINPUT_ENABLE | Config::DIRECTINPUT_AVAIL)) ? 4 : 0;
#endif
			if (!emu->key_down_up(type, LOBYTE(wParam), (long)lParam)) return 0;
		}
		break;
	case WM_SYSCHAR:
		return 0;	// not activate menu
		break;
	case WM_SIZING:
#ifdef ENABLE_FREE_SIZING
		{
			RECT *re = (RECT*)lParam;
			int x = re->left;
			int y = re->top;
			int w = re->right - x;
			int h = re->bottom - y;
			if (w < min_window_width) {
				re->right = x + min_window_width;
			}
			if (max_window_width < w) {
				re->right = x + max_window_width;
			}
			if (h < min_window_height) {
				re->bottom = y + min_window_height;
			}
			if (max_window_height < h) {
				re->bottom = y + max_window_height;
			}
		}
#endif
		break;
	case WM_SIZE:
		{
			window_client_width = LOWORD(lParam);
			window_client_height = HIWORD(lParam);
			if (emu) {
				emu->change_maximize_window(window_client_width, window_client_height, wParam == SIZE_MAXIMIZED);
			}
		}
		break;
	case WM_RESIZE:
		if(emu) {
			emu->change_screen_mode(pConfig->window_mode);
		}
		break;
	case WM_RBUTTONDOWN:
		if(emu) {
			if (emu->is_fullscreen()) {
				if (GET_X_LPARAM(lParam) < 8 && GET_Y_LPARAM(lParam) < 8) {
					// goto window mode
					emu->change_screen_mode(0);
				}
			}
		}
		break;
#ifdef USE_SOCKET
	case WM_SOCKET0:
	case WM_SOCKET1:
	case WM_SOCKET2:
	case WM_SOCKET3:
	case WM_SOCKET4:
	case WM_SOCKET5:
		no = iMsg - WM_SOCKET0;
		if(!emu) {
			break;
		}
		if(WSAGETSELECTERROR(lParam) != 0) {
			emu->disconnect_socket(no);
			emu->socket_disconnected(no);
			break;
		}
		if(emu->get_socket(no) != (void *)wParam) {
			break;
		}
		switch(WSAGETSELECTEVENT(lParam)) {
		case FD_CONNECT:
			emu->socket_connected(no);
			break;
		case FD_CLOSE:
			emu->socket_disconnected(no);
			break;
		case FD_WRITE:
			emu->socket_writeable(no);
			emu->send_data(no);
			break;
		case FD_READ:
			emu->recv_data(no);
			emu->socket_readable(no);
			break;
		case FD_ACCEPT:
			emu->socket_accept(no);
			break;
		}
		return 0;
#endif
#if 0
	case WM_MOUSEMOVE:
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = 0;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
			emu->mouse_move((int)GET_X_LPARAM(lParam), (int)GET_Y_LPARAM(lParam));
		}
		return 0;
	case WM_MOUSELEAVE:
		emu->mouse_leave();
		return 0;
#endif
	case WM_DROPFILES:
		gui->OpenDroppedFile((void *)wParam);
		return 0;
	case WM_INITMENUPOPUP:
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
	case WM_COMMAND:
	case WM_MOVE:
		result = gui->ProcessEvent(iMsg, wParam, lParam);
		break;
	}
	if (result > 0) {
		result = DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return result;
}

#ifdef USE_BUTTON
LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_BUTTONS; i++) {
		if(hWnd == hButton[i]) {
			switch(iMsg) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if(emu) {
					emu->key_down(LOBYTE(wParam), false);
				}
				return 0;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if(emu) {
					emu->key_up(LOBYTE(wParam));
				}
				return 0;
			}
			return CallWindowProc(buttonWndProc[i], hWnd, iMsg, wParam, lParam);
		}
	}
	return 0;
}
#endif

/**
 *  emulator thread event loop
 *
 *  @attention called by another thread
 */
static DWORD WINAPI EmuProc(LPVOID lpParameter)
{
	HWND hWnd = (HWND)lpParameter;
	emu_thread_working = true;

	const int fskip[6] = {1, 2, 3, 4, 5, 6};
	int fskip_remain = 0;

	t_frame_count frames = { 0, 0, 0 };

	int ms = 1000;
	int next_interval;
	calc_next_interval(ms, next_interval, 1000, FRAMES_PER_SEC);

	int split_num = 0;

	DWORD current_time = 0;
	int   remain_time = 0;
	DWORD ideal_next_time = 0;
	DWORD real_next_time = 0;
#ifdef LOG_MEASURE
	DWORD skip_reason = 0;
#endif
#ifdef USE_PERFORMANCE_METER
	LARGE_INTEGER lpCount1,lpCount2,lpCount3;
	lpCount1.QuadPart = lpCount2.QuadPart = lpCount3.QuadPart = 0;
#endif

	// vm start
	emu->reset();

	// wait a sec.
//	CDelay(475);
	emu->adjust_sound_pos(500);

	DWORD update_fps_time = timeGetTime() + 1000;

	// play sound
	emu->mute_sound(false);

	// process coming messages at first.
	while (emumsg.Process()) {}

	ideal_next_time = timeGetTime();

#ifdef USE_PERFORMANCE_METER
//	QueryPerformanceCounter(&lpCount1);
	lpCount1.QuadPart = ideal_next_time;
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
		current_time = timeGetTime();
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
		logging->out_logf(LOG_DEBUG, "A2: rti:%ld nt:%ld",timeGetTime(),ideal_next_time);
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
				current_time = timeGetTime();
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
//					real_next_time = timeGetTime();
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
		current_time = timeGetTime();
#ifdef USE_PERFORMANCE_METER
		if (pConfig->show_pmeter) {
			lpCount2.QuadPart = current_time;
//			QueryPerformanceCounter(&lpCount2);
		}
#endif
		remain_time = real_next_time - current_time;
		if (remain_time > 3) {
			CDelay(remain_time - 1);
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A4: rti:%ld nt:%ld %ld",timeGetTime(),ideal_next_time,real_next_time);
#endif
		// calc frame rate
		current_time = timeGetTime();
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
//			QueryPerformanceFrequency(&lpFreq);
//			QueryPerformanceCounter(&lpCount3);
			lpCount3.QuadPart =  current_time;
			if (lpCount3.QuadPart > lpCount1.QuadPart) {
				gdPMvalue = ((lpCount2.QuadPart - lpCount1.QuadPart) * 100 / (lpCount3.QuadPart - lpCount1.QuadPart)) & 0xfff;
			}
			lpCount1 = lpCount3;
		}
#endif
	}

	emu->release_on_emu_thread();

	return 0;
}
