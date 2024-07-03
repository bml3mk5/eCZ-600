/** @file gui_base.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.01

	@brief [ gui base class ]
*/

#if defined(USE_WX) || defined(USE_WX2)
#include <wx/wx.h>
#endif
#include "gui_base.h"
#include "../config.h"
#include "../emu_osd.h"
#include "../emumsg.h"
#include "../vm/vm.h"
#include "../keycode.h"
#include "../res/resource.h"
#include "../main.h"
#ifdef USE_LEDBOX
#include "../depend.h"
#include "ledbox.h"
#endif
#ifdef USE_VKEYBOARD
#include "vkeyboard.h"
#endif
#include "../msgs.h"
#include "../labels.h"
#include "../utility.h"
// -------------------------------------
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_ttf.h>
#if defined(USE_SOCKET) && defined(USE_SDL_NET)
#include <SDL_net.h>
#endif
// -------------------------------------
#elif defined(USE_WX) || defined(USE_WX2)
#include "../csurface.h"
//#include "../utils.h"

wxDECLARE_EXPORTED_EVENT(,wxEVT_USER1, wxCommandEvent);
// -------------------------------------
#endif

//
// GUI base class
//
GUI_BASE::GUI_BASE(int argc, char **argv, EMU *new_emu)
{
	emu = new_emu;
	need_update_screen = 6;
	globalkey_enable = 0;
#ifdef USE_LEDBOX
	ledbox = NULL;
#endif
#ifdef USE_VKEYBOARD
	vkeyboard = NULL;
#endif
#if defined(_WIN32)
	hWindow = NULL;
#endif
#ifdef USE_GTK
	exit_program = false;
#endif
	next_sound_frequency = 0;
	next_sound_latency = 0;
}

GUI_BASE::~GUI_BASE()
{
#ifdef USE_LEDBOX
	delete ledbox;
#endif
#ifdef USE_VKEYBOARD
	delete vkeyboard;
#endif
}

int GUI_BASE::Init()
{
	return 0;
}

/// called by initialize() on EMU class
void GUI_BASE::InitializedEmu()
{
	next_sound_frequency = pConfig->sound_frequency;
	next_sound_latency = pConfig->sound_latency;
}

#if defined(_WIN32)
void GUI_BASE::SetWindowHandle()
{
#if defined(USE_SDL) || defined(USE_SDL2)
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
#ifndef USE_SDL2
	SDL_GetWMInfo(&info);
	hWindow = info.window;
#else
	SDL_GetWindowWMInfo(((EMU_OSD *)emu)->get_window(), &info);
	hWindow = info.info.win.window;
#endif
#endif
}
#endif

#ifdef GUI_USE_FOREIGN_WINDOW
void *GUI_BASE::CreateWindow(int width, int height)
{
	return NULL;
}
void *GUI_BASE::GetWindowData()
{
	return NULL;
}
#endif

#if defined(USE_WIN)
int GUI_BASE::CreateWidget(HWND hWnd, int width, int height)
{
	hWindow = hWnd;
	return 0;
}
#elif defined(USE_SDL)
int GUI_BASE::CreateWidget(SDL_Surface *screen, int width, int height)
{
#if defined(_WIN32)
	SetWindowHandle();
#endif
	return 0;
}
#elif defined(USE_SDL2)
int GUI_BASE::CreateWidget(SDL_Window *window, int width, int height)
{
#if defined(_WIN32)
	SetWindowHandle();
#endif
	return 0;
}
#elif defined(USE_QT)
int GUI_BASE::CreateWidget(QRect *screen, int width, int height)
{
	return 0;
}
#elif defined(USE_WX) || defined(USE_WX2)
int GUI_BASE::CreateWidget(wxWindow *window, int width, int height)
{
	return 0;
}
#endif

int GUI_BASE::CreateMenu()
{
#ifdef USE_SDL2
	// enable drag and drop operation
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
#endif
	return 0;
}
void GUI_BASE::ShowMenu()
{
}

void GUI_BASE::HideMenu()
{
}

int GUI_BASE::MixSurface()
{
	return 0;
}

/// post update screen request message
/// @attention this function is called by emu thread
/// @return false:did not call UpdateScreen in main thread
bool GUI_BASE::NeedUpdateScreen()
{
#if defined(USE_WIN) || defined(USE_SDL) || defined(USE_SDL2)
	if (need_update_screen > 0) {
		need_update_screen = 0;
#if defined(USE_WIN)
		// post dummy meesage so that process ProcessEvent fuction
		emu->need_update_screen();
#elif defined(USE_SDL) || defined(USE_SDL2)
		// send signal to main loop
		g_need_update_screen = true;
		SDL_CondSignal(g_cond_allow_update_screen);
#endif
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

/// post finished updateing screen request message
void GUI_BASE::UpdatedScreen()
{
}

/// draw screen on window
/// @note this function should be called by main thread
#if defined(USE_WIN)
void GUI_BASE::UpdateScreen(HDC hdc)
{
//	logging->out_debug("GUI_BASE::UpdateScreen");
	emu->update_screen(hdc);
	need_update_screen = 6;
}
#elif defined(USE_SDL) || defined(USE_SDL2)
void GUI_BASE::UpdateScreen()
{
#ifndef USE_GTK
//	logging->out_log(LOG_DEBUG, "GUI_BASE::UpdateScreen: Start");
	emu->update_screen();
	need_update_screen = 6;
	g_need_update_screen = false;
//	logging->out_logf(LOG_DEBUG, "GUI_BASE::UpdateScreen: Updated: %d", need_update_screen);
#endif
}
#else
void GUI_BASE::UpdateScreen()
{
}
#endif

#if defined(USE_SDL) || defined(USE_SDL2) || defined(USE_QT) || defined(USE_WX) || defined(USE_WX2)
void GUI_BASE::DecreaseUpdateScreenCount()
{
	if (need_update_screen > 0) need_update_screen--;
//	logging->out_logf(LOG_DEBUG, "GUI_BASE::Decrease %d",need_update_screen);
}
#endif

int GUI_BASE::CreateGlobalKeys()
{
	return 0;
}

/**
 * global keys
 *
 * @param [in] code : GLOBALKEY_*
 * @param [in] status : 2: from emu thread (vm)
 * @return true : processed global key
 */
bool GUI_BASE::ExecuteGlobalKeys(int code, uint32_t status)
{
	bool result = true;

	if (globalkey_enable) return result;

	// Alt + ...
	switch(code) {
		case GLOBALKEY_RETURN:	// screen mode
			PostCommandMessage(ID_ACCEL_SCREEN);
			break;
#ifdef USE_LIGHTPEN
		case GLOBALKEY_CONTROL:	// enable lightpen
			// ignore when pressed this in reckey playdata
			if (status != 2) PostCommandMessage(ID_OPTIONS_LIGHTPEN);
			break;
#endif
#ifdef USE_MOUSE
		case GLOBALKEY_CONTROL:	// enable mouse
			// ignore when pressed this in reckey playdata
			if (status != 2) PostCommandMessage(ID_OPTIONS_MOUSE);
			break;
#endif
		case GLOBALKEY_0:	// sync IRQ
			PostEtToggleSyncIRQ();
			break;
		case GLOBALKEY_1:	// CPU x1
			PostEtCPUPower(1);
			break;
		case GLOBALKEY_2:	// CPU x2
			PostEtCPUPower(2);
			break;
		case GLOBALKEY_3:	// CPU x4
			PostEtCPUPower(3);
			break;
		case GLOBALKEY_4:	// CPU x8
			PostEtCPUPower(4);
			break;
		case GLOBALKEY_5:	// CPU x16
			PostEtCPUPower(5);
			break;
//		case GLOBALKEY_6:	// CPU x32
//			PostEtCPUPower(6);
//			break;
		case GLOBALKEY_9:	// CPU x0.5
			PostEtCPUPower(0);
			break;
		case GLOBALKEY_A:	// aspect ratio
			PostCommandMessage(ID_SCREEN_PIXEL_ASPECT_A);
			break;
		case GLOBALKEY_C:	// configure...
//			ShowConfigureDialog();
			PostCommandMessage(ID_OPTIONS_CONFIG);
			break;
#ifdef USE_DEBUGGER
		case GLOBALKEY_D:	// debugger...
			PostMtOpenDebugger();
			break;
#endif
		case GLOBALKEY_E:	// load record key...
//			ShowPlayRecKeyDialog();
			PostCommandMessage(ID_RECKEY_PLAY);
			break;
		case GLOBALKEY_F:	// change fdd mode
			if (status == 2) ChangeFddType(-1);
			else PostCommandMessage(ID_OPTIONS_FDD_TYPE_A);
			break;
		case GLOBALKEY_G:	// capture screen
			if (status != 2) PostEtCaptureScreen();
			break;
		case GLOBALKEY_K:	// key bind...
//			ShowKeybindDialog();
			PostCommandMessage(ID_OPTIONS_KEYBIND);
			break;
		case GLOBALKEY_J:	// enable joystick
			// ignore when pressed this in reckey playdata
			if (status != 2) PostCommandMessage(ID_OPTIONS_JOYPAD_A);
			break;
		case GLOBALKEY_L:	// show ledbox
			if (status == 2) ToggleLedBox();
			else PostCommandMessage(ID_OPTIONS_LEDBOX_A);
			break;
#ifdef _BML3MK5
		case GLOBALKEY_M:	// Mode switch
			if (status == 2) Dipswitch(2);
			else PostCommandMessage(ID_DIPSWITCH3);
			break;
#endif
#ifdef _MBS1
		case GLOBALKEY_m:	// Dip switch
			if (status == 2) Dipswitch(2);
			else PostCommandMessage(ID_DIPSWITCH3);
			break;
		case GLOBALKEY_M:	// Mode switch
			if (status == 2) ChangeSystemMode(-1);
			else PostCommandMessage(ID_SYSTEM_MODE_A);
			break;
#endif
		case GLOBALKEY_O:	// load state...
//			ShowLoadStateDialog();
			PostCommandMessage(ID_LOAD_STATE);
			break;
		case GLOBALKEY_P:	// pos change ledbox
			if (status == 2) ChangeLedBoxPosition(-1);
			else PostCommandMessage(ID_OPTIONS_LEDBOX_POS_A);
			break;
		case GLOBALKEY_Q:	// pause
			PostEtTogglePause();
			break;
		case GLOBALKEY_R:	// reset switch
			PostEtWarmReset(1);
			break;
		case GLOBALKEY_S:	// scanline
			PostEtChangeDrawMode(-1);
			break;
#ifdef USE_AFTERIMAGE
		case GLOBALKEY_T:	// afterimage1,2
			PostEtChangeAfterImage();
			break;
#endif
		case GLOBALKEY_V:	// volume
//			ShowVolumeDialog();
			PostCommandMessage(ID_SOUND_VOLUME);
			break;
		case GLOBALKEY_X:	// stretch on fullscreen
			PostCommandMessage(ID_SCREEN_STRETCH_A);
			break;
		case GLOBALKEY_W:	// change window size
			PostCommandMessage(ID_SCREEN_WINDOW_A);
			break;
#ifdef USE_DIRECT3D
		case GLOBALKEY_U:	// direct3d filter
			PostCommandMessage(ID_SCREEN_D3D_FILTER_A);
			break;
		case GLOBALKEY_Y:	// use direct3d1,2
			PostCommandMessage(ID_SCREEN_D3D_A);
			break;
#endif
#ifdef USE_OPENGL
		case GLOBALKEY_U:	// opengl filter
			PostCommandMessage(ID_SCREEN_OPENGL_FILTER_A);
			break;
		case GLOBALKEY_Y:	// use opengl1,2
			PostCommandMessage(ID_SCREEN_OPENGL_A);
			break;
#endif
		case GLOBALKEY_Z:	// show Message
			if (status == 2) ToggleMessageBoard();
			else PostCommandMessage(ID_OPTIONS_MSGBOARD);
			break;
		case GLOBALKEY_F3:	// Power Switch
			PostEtSpecialReset();
			break;
		case GLOBALKEY_F4:	// exit
//			Exit();
			PostCommandMessage(ID_EXIT);
			break;
#ifdef USE_DATAREC
		case GLOBALKEY_F5:	// Rewind
			PostEtRewindDataRecMessage();
			break;
		case GLOBALKEY_F6:	// Stop
//			PostCommandMessage(ID_REC_DATAREC);
			PostEtStopDataRecMessage();
			break;
		case GLOBALKEY_F7:	// Play...
			PostCommandMessage(ID_PLAY_DATAREC);
			break;
		case GLOBALKEY_F8:	// F.F.
			PostEtFastForwardDataRecMessage();
			break;
#endif
#ifdef USE_FD1
		case GLOBALKEY_F9:	// Open FDD0
//			ShowOpenFloppyDiskDialog(0);
			PostCommandMessage(ID_OPEN_FD1);
			break;
#endif
#ifdef USE_FD2
		case GLOBALKEY_F10:	// Open FDD1
//			ShowOpenFloppyDiskDialog(1);
			PostCommandMessage(ID_OPEN_FD2);
			break;
#endif
#ifdef USE_FD3
		case GLOBALKEY_F11:	// Open FDD2
//			ShowOpenFloppyDiskDialog(1);
			PostCommandMessage(ID_OPEN_FD3);
			break;
#endif
#ifdef USE_FD4
		case GLOBALKEY_F12:	// Open FDD3
//			ShowOpenFloppyDiskDialog(1);
			PostCommandMessage(ID_OPEN_FD4);
			break;
#endif
#ifdef _X68000
		case GLOBALKEY_SFT_0:
			ToggleShowScreen(0);
			break;
		case GLOBALKEY_SFT_1:
		case GLOBALKEY_SFT_2:
		case GLOBALKEY_SFT_3:
		case GLOBALKEY_SFT_4:
		case GLOBALKEY_SFT_5:
		case GLOBALKEY_SFT_6:
		case GLOBALKEY_SFT_7:
		case GLOBALKEY_SFT_8:
		case GLOBALKEY_SFT_9:
			ToggleShowScreen(1 << (code - GLOBALKEY_SFT_1));
			break;
#endif
		default:
			result = false;
			break;
	}
	if (result) {
		globalkey_enable |= 0x7;
	}
	return result;
}

/**
 * global keys
 *
 * @return true : processed global key
 */
bool GUI_BASE::ReleaseGlobalKeys(int code, uint32_t status)
{
	bool result = true;

	if (globalkey_enable & 0xf0) return result;

	// Alt + ...
	switch(code) {
		case GLOBALKEY_R:	// reset switch
			PostEtWarmReset(0);
			break;
		default:
			result = false;
			break;
	}
	return result;
}

/// something do before process event
void GUI_BASE::PreProcessEvent()
{
#if defined(USE_SDL) || defined(USE_SDL2)
#ifndef USE_GTK
	if (need_update_screen <= 0) {
		UpdateScreen();
	}
#endif
#endif
#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_LEDBOX)
	if (ledbox) {
		ledbox->ProcessEvent();
	}
#endif
#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_VKEYBOARD)
	if (vkeyboard) {
		vkeyboard->ProcessEvent();
	}
#endif
}

/// event processing

#if defined(USE_WIN)
/// @return 0: processed myself 1: no processing
LRESULT GUI_BASE::ProcessEvent(UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if (iMsg == WM_COMMAND) {
		int id = LOWORD(wParam);

		return ProcessCommand(id, NULL, NULL);

	}
	return 1;
}
#elif defined(USE_SDL) || defined(USE_SDL2)
/// @return -1: exit event loop 0: processed myself 1: no processing
int GUI_BASE::ProcessEvent(SDL_Event *e)
{
#if defined(USE_SDL2) && defined(USE_SDL2_LEDBOX)
	if (ledbox) {
		int rc = ledbox->ProcessEvent(e);
		if (rc <= 0) return rc;
	}
#endif
	switch(e->type) {
	case SDL_USEREVENT_COMMAND: {
		int id = e->user.code;
		void *data1 = e->user.data1;
		void *data2 = e->user.data2;

		return ProcessCommand(id, data1, data2);
	}
	break;
#ifdef USE_SDL2
	case SDL_WINDOWEVENT: {
		switch (e->window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			Exit();
			return 0;
		case SDL_WINDOWEVENT_MOVED:
#ifdef USE_SDL2_LEDBOX
			if (e->window.windowID == ledbox->GetWindowID()) {
				// ledbox dialog was moved
				ledbox->SetDist();
				return 0;
			} else
#endif
			{
				// main window was moved
				ledbox->Move();
				return 0;
			}
		}
	}
	break;
	case SDL_DROPFILE: {
		OpenDroppedFile(e->drop.file);
		SDL_free(e->drop.file);
		return 0;
	}
	break;
#endif
	}
	return 1;
}
#elif defined(USE_QT)
/// @return -1: exit event loop 0: processed myself 1: no processing
int GUI_BASE::ProcessEvent(MyUserEvent *e)
{
	switch(e->type()) {
    case QEvent::User:
		int id = e->code;
		return ProcessCommand(id, NULL, NULL);
	}
	return 1;
}
#elif defined(USE_WX) || defined(USE_WX2)
/// @return -1: exit event loop 0: processed myself 1: no processing
int GUI_BASE::ProcessEvent(wxCommandEvent &e)
{
	wxEventType type = e.GetEventType();
	if (type == wxEVT_USER1 || type == wxEVT_MENU) {
		int id = e.GetId();
		return ProcessCommand(id, NULL, NULL);
	}
	return 1;
}
#endif

/// something do after process event
void GUI_BASE::PostProcessEvent()
{
#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_LEDBOX)
	if (ledbox) {
		ledbox->PostProcessEvent();
	}
#endif
#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_VKEYBOARD)
	if (vkeyboard) {
		vkeyboard->PostProcessEvent();
	}
#endif
}

void GUI_BASE::ScreenModeChanged(bool fullscreen)
{
}

void GUI_BASE::PostDrive()
{
	if (globalkey_enable & 0xf) globalkey_enable--;
}

/// post message to main thread
void GUI_BASE::PostCommandMessage(int id, void *data1, void *data2)
{
#if defined(USE_WIN)
	// post window message to main thread
	PostMessage(hWindow, WM_COMMAND, MAKELONG(id, 1), 0);
#elif defined(USE_SDL) || defined(USE_SDL2)
	SDL_Event e;
	e.type = SDL_USEREVENT_COMMAND;
	e.user.code = id;
	e.user.data1 = data1;
	e.user.data2 = data2;

	SDL_PushEvent(&e);
#endif
}

/// process command
int GUI_BASE::ProcessCommand(int id, void *data1, void *data2)
{
	switch(id) {
#ifdef USE_MOUSE
		case ID_ENABLE_MOUSE_TEMP:
			emu->enable_mouse(1);
			return 0;
		case ID_DISABLE_MOUSE_TEMP:
			emu->disable_mouse(1);
			return 0;
#endif		
		case ID_RESET:
			PostEtReset();
			return 0;
#ifdef USE_SPECIAL_RESET
		case ID_FORCE_RESET:
			PostEtForceReset();
			return 0;
		case ID_SPECIAL_RESET:
			PostEtSpecialReset();
			return 0;
		case ID_WARM_RESET:
			PostEtWarmReset(-1);
			return 0;
#endif
#ifdef USE_DIPSWITCH
		case ID_DIPSWITCH1:
		case ID_DIPSWITCH2:
		case ID_DIPSWITCH3:
		case ID_DIPSWITCH4:
		case ID_DIPSWITCH5:
		case ID_DIPSWITCH6:
		case ID_DIPSWITCH7:
		case ID_DIPSWITCH8:
			Dipswitch(id - ID_DIPSWITCH1);
			return 0;
#endif
#ifdef USE_EMU_INHERENT_SPEC
#ifdef _MBS1
		case ID_SYSTEM_MODE_1:
		case ID_SYSTEM_MODE_2:
			ChangeSystemMode(ID_SYSTEM_MODE_2 - id);
			return 0;
		case ID_SYSTEM_MODE_A:
			ChangeSystemMode(-1);
			return 0;
#endif
		case ID_FDD_TYPE_1:
		case ID_FDD_TYPE_2:
		case ID_FDD_TYPE_3:
		case ID_FDD_TYPE_4:
			ChangeFddType(id - ID_FDD_TYPE_1);
			return 0;
		case ID_INTERRUPT0:
		case ID_INTERRUPT1:
		case ID_INTERRUPT2:
		case ID_INTERRUPT3:
		case ID_INTERRUPT4:
		case ID_INTERRUPT5:
		case ID_INTERRUPT6:
		case ID_INTERRUPT7:
			PostEtInterrupt(id - ID_INTERRUPT0);
			return 0;
#endif
#ifdef _HC80
		case ID_HC80_RAMDISK0:
		case ID_HC80_RAMDISK1:
		case ID_HC80_RAMDISK2:
			pConfig->device_type = LOWORD(wParam) - ID_HC80_RAMDISK0;
			break;
#endif
#ifdef _MZ800
		case ID_MZ800_MODE_MZ800:
		case ID_MZ800_MODE_MZ700:
			pConfig->boot_mode = LOWORD(wParam) - ID_MZ800_MODE_MZ800;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef _PASOPIA
		case ID_PASOPIA_MODE_TBASIC_V1_0:
		case ID_PASOPIA_MODE_TBASIC_V1_1:
		case ID_PASOPIA_MODE_OABASIC:
		case ID_PASOPIA_MODE_OABASIC_NO_DISK:
		case ID_PASOPIA_MODE_MINI_PASCAL:
			pConfig->boot_mode = LOWORD(wParam) - ID_PASOPIA_MODE_TBASIC_V1_0;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_PASOPIA_DEVICE_RAM_PAC:
		case ID_PASOPIA_DEVICE_KANJI_ROM:
		case ID_PASOPIA_DEVICE_JOYSTICK:
			pConfig->device_type = LOWORD(wParam) - ID_PASOPIA_DEVICE_RAM_PAC;
			break;
#endif
#ifdef _PC98DO
		case ID_PC98DO_MODE_PC98:
		case ID_PC8801_MODE_V1S:
		case ID_PC8801_MODE_V1H:
		case ID_PC8801_MODE_V2:
		case ID_PC8801_MODE_N:
			pConfig->boot_mode = LOWORD(wParam) - ID_PC98DO_MODE_PC98;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef _PC8801MA
		case ID_PC8801_MODE_V1S:
		case ID_PC8801_MODE_V1H:
		case ID_PC8801_MODE_V2:
		case ID_PC8801_MODE_N:
			pConfig->boot_mode = LOWORD(wParam) - ID_PC8801_MODE_V1S;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_PC8801_DEVICE_JOYSTICK:
		case ID_PC8801_DEVICE_MOUSE:
		case ID_PC8801_DEVICE_JOYMOUSE:
			pConfig->device_type = LOWORD(wParam) - ID_PC8801_DEVICE_JOYSTICK;
			break;
#endif
#if defined(_PC9801E) || defined(_PC9801VM) || defined(_PC98DO) || defined(_PC8801MA)
		case ID_PC9801_CPU_CLOCK_HIGH:
			pConfig->cpu_clock_low = false;
			break;
		case ID_PC9801_CPU_CLOCK_LOW:
			pConfig->cpu_clock_low = true;
			break;
#endif
		case ID_CPU_POWER0:
		case ID_CPU_POWER1:
		case ID_CPU_POWER2:
		case ID_CPU_POWER3:
		case ID_CPU_POWER4:
		case ID_CPU_POWER5:
		case ID_CPU_POWER6:
			PostEtCPUPower(id - ID_CPU_POWER0);
			return 0;
#ifdef USE_EMU_INHERENT_SPEC
		case ID_PAUSE:
			PostEtTogglePause();
			return 0;
		case ID_SYNC_IRQ:
			PostEtToggleSyncIRQ();
			return 0;
#ifdef _MBS1
		case ID_MEMORY_NOWAIT:
			ToggleMemNoWait();
			return 0;
#endif
#endif
#ifdef USE_AUTO_KEY
		case ID_AUTOKEY_START:
			PostEtStartAutoKeyMessage();
			return 0;
		case ID_AUTOKEY_OPEN:
			ShowOpenAutoKeyDialog();
			return 0;
		case ID_AUTOKEY_STOP:
			PostEtStopAutoKeyMessage();
			return 0;
#endif
#ifdef USE_KEY_RECORD
		case ID_RECKEY_PLAY:
			ShowPlayRecKeyDialog();
			return 0;
		case ID_RECKEY_REC:
			ShowRecordStateAndRecKeyDialog();
			return 0;
		case ID_RECKEY_STOP_PLAY:
			StopPlayRecKey();
			return 0;
		case ID_RECKEY_STOP_REC:
			StopRecordRecKey();
			return 0;
#endif
#ifdef USE_EMU_INHERENT_SPEC
		case ID_LOAD_STATE:
			ShowLoadStateDialog();
			return 0;
		case ID_SAVE_STATE:
			ShowSaveStateDialog(false);
			return 0;
#endif
#ifdef USE_DEBUGGER
		case ID_OPEN_DEBUGGER0:
		case ID_OPEN_DEBUGGER1:
		case ID_OPEN_DEBUGGER2:
		case ID_OPEN_DEBUGGER3:
			OpenDebugger();
			return 0;
		case ID_CLOSE_DEBUGGER:
			CloseDebugger();
			return 0;
#endif
#ifdef USE_FD1
#define FD_MENU_PREITEMS(drv, ID_OPEN_FD, ID_CLOSE_FD, ID_CHANGE_FD, ID_WRITEPROTECT_FD, ID_OPEN_BLANK_2D_FD, ID_OPEN_BLANK_2DD_FD, ID_OPEN_BLANK_2HD_FD) \
		case ID_OPEN_FD: \
			ShowOpenFloppyDiskDialog(drv); \
			return 0; \
		case ID_CLOSE_FD: \
			PostEtCloseFloppyMessage(drv); \
			return 0; \
		case ID_CHANGE_FD: \
			PostEtChangeSideFloppyDisk(drv); \
			return 0; \
		case ID_WRITEPROTECT_FD: \
			PostEtToggleWriteProtectFloppyDisk(drv); \
			return 0; \
		case ID_OPEN_BLANK_2D_FD: \
			ShowOpenBlankFloppyDiskDialog(drv, 0x00); \
			return 0; \
		case ID_OPEN_BLANK_2DD_FD: \
			ShowOpenBlankFloppyDiskDialog(drv, 0x10); \
			return 0; \
		case ID_OPEN_BLANK_2HD_FD: \
			ShowOpenBlankFloppyDiskDialog(drv, 0x20); \
			return 0;
		FD_MENU_PREITEMS(0, ID_OPEN_FD1, ID_CLOSE_FD1, ID_CHANGE_FD1, ID_WRITEPROTECT_FD1, ID_OPEN_BLANK_2D_FD1, ID_OPEN_BLANK_2DD_FD1, ID_OPEN_BLANK_2HD_FD1)
#endif
#ifdef USE_FD2
		FD_MENU_PREITEMS(1, ID_OPEN_FD2, ID_CLOSE_FD2, ID_CHANGE_FD2, ID_WRITEPROTECT_FD2, ID_OPEN_BLANK_2D_FD2, ID_OPEN_BLANK_2DD_FD2, ID_OPEN_BLANK_2HD_FD2)
#endif
#ifdef USE_FD3
		FD_MENU_PREITEMS(2, ID_OPEN_FD3, ID_CLOSE_FD3, ID_CHANGE_FD3, ID_WRITEPROTECT_FD3, ID_OPEN_BLANK_2D_FD3, ID_OPEN_BLANK_2DD_FD3, ID_OPEN_BLANK_2HD_FD3)
#endif
#ifdef USE_FD4
		FD_MENU_PREITEMS(3, ID_OPEN_FD4, ID_CLOSE_FD4, ID_CHANGE_FD4, ID_WRITEPROTECT_FD4, ID_OPEN_BLANK_2D_FD4, ID_OPEN_BLANK_2DD_FD4, ID_OPEN_BLANK_2HD_FD4)
#endif
#ifdef USE_FD5
		FD_MENU_PREITEMS(4, ID_OPEN_FD5, ID_CLOSE_FD5, ID_CHANGE_FD5, ID_WRITEPROTECT_FD5, ID_OPEN_BLANK_2D_FD5, ID_OPEN_BLANK_2DD_FD5, ID_OPEN_BLANK_2HD_FD5)
#endif
#ifdef USE_FD6
		FD_MENU_PREITEMS(5, ID_OPEN_FD6, ID_CLOSE_FD6, ID_CHANGE_FD6, ID_WRITEPROTECT_FD6, ID_OPEN_BLANK_2D_FD6, ID_OPEN_BLANK_2DD_FD6, ID_OPEN_BLANK_2HD_FD6)
#endif
#ifdef USE_HD1
#define HD_MENU_PREITEMS_A(drv, ID_OPEN_HD, ID_CLOSE_HD, ID_WRITEPROTECT_HD) \
		case ID_OPEN_HD: \
			ShowOpenHardDiskDialog(drv); \
			return 0; \
		case ID_CLOSE_HD: \
			PostEtCloseHardDiskMessage(drv); \
			return 0; \
		case ID_WRITEPROTECT_HD: \
			PostEtToggleWriteProtectHardDisk(drv); \
			return 0;
#define HD_MENU_PREITEMS_B(drv, ID_OPEN_BLANK_10MB_HD, ID_OPEN_BLANK_20MB_HD, ID_OPEN_BLANK_40MB_HD) \
		case ID_OPEN_BLANK_10MB_HD: \
			ShowOpenBlankHardDiskDialog(drv, 0); \
			return 0; \
		case ID_OPEN_BLANK_20MB_HD: \
			ShowOpenBlankHardDiskDialog(drv, 1); \
			return 0; \
		case ID_OPEN_BLANK_40MB_HD: \
			ShowOpenBlankHardDiskDialog(drv, 2); \
			return 0;
#define HD_MENU_PREITEMS_C(drv, ID_DEVICE_TYPE_HD) \
		case ID_DEVICE_TYPE_HD: \
			ShowSelectHardDiskDeviceTypeDialog(drv); \
			return 0;
		HD_MENU_PREITEMS_A(0, ID_OPEN_HD1, ID_CLOSE_HD1, ID_WRITEPROTECT_HD1)
		HD_MENU_PREITEMS_B(0, ID_OPEN_BLANK_10MB_HD1, ID_OPEN_BLANK_20MB_HD1, ID_OPEN_BLANK_40MB_HD1)
		HD_MENU_PREITEMS_C(0, ID_DEVICE_TYPE_HD1)
#endif
#ifdef USE_HD2
		HD_MENU_PREITEMS_A(1, ID_OPEN_HD2, ID_CLOSE_HD2, ID_WRITEPROTECT_HD2)
		HD_MENU_PREITEMS_B(1, ID_OPEN_BLANK_10MB_HD2, ID_OPEN_BLANK_20MB_HD2, ID_OPEN_BLANK_40MB_HD2)
		HD_MENU_PREITEMS_C(1, ID_DEVICE_TYPE_HD2)
#endif
#ifdef USE_HD3
		HD_MENU_PREITEMS_A(2, ID_OPEN_HD3, ID_CLOSE_HD3, ID_WRITEPROTECT_HD3)
		HD_MENU_PREITEMS_C(2, ID_DEVICE_TYPE_HD3)
#endif
#ifdef USE_HD4
		HD_MENU_PREITEMS_A(3, ID_OPEN_HD4, ID_CLOSE_HD4, ID_WRITEPROTECT_HD4)
		HD_MENU_PREITEMS_C(3, ID_DEVICE_TYPE_HD4)
#endif
#ifdef USE_HD5
		HD_MENU_PREITEMS_A(4, ID_OPEN_HD5, ID_CLOSE_HD5, ID_WRITEPROTECT_HD5)
		HD_MENU_PREITEMS_C(4, ID_DEVICE_TYPE_HD5)
#endif
#ifdef USE_HD6
		HD_MENU_PREITEMS_A(5, ID_OPEN_HD6, ID_CLOSE_HD6, ID_WRITEPROTECT_HD6)
		HD_MENU_PREITEMS_C(5, ID_DEVICE_TYPE_HD6)
#endif
#ifdef USE_HD17
		HD_MENU_PREITEMS_A(16, ID_OPEN_HD17, ID_CLOSE_HD17, ID_WRITEPROTECT_HD17)
		HD_MENU_PREITEMS_C(16, ID_DEVICE_TYPE_HD17)
#endif
#ifdef USE_HD18
		HD_MENU_PREITEMS_A(17, ID_OPEN_HD18, ID_CLOSE_HD18, ID_WRITEPROTECT_HD18)
		HD_MENU_PREITEMS_C(17, ID_DEVICE_TYPE_HD18)
#endif
#ifdef USE_HD19
		HD_MENU_PREITEMS_A(18, ID_OPEN_HD19, ID_CLOSE_HD19, ID_WRITEPROTECT_HD19)
		HD_MENU_PREITEMS_C(18, ID_DEVICE_TYPE_HD19)
#endif
#ifdef USE_HD20
		HD_MENU_PREITEMS_A(19, ID_OPEN_HD20, ID_CLOSE_HD20, ID_WRITEPROTECT_HD20)
		HD_MENU_PREITEMS_C(19, ID_DEVICE_TYPE_HD20)
#endif
#ifdef USE_HD21
		HD_MENU_PREITEMS_A(20, ID_OPEN_HD21, ID_CLOSE_HD21, ID_WRITEPROTECT_HD21)
		HD_MENU_PREITEMS_C(20, ID_DEVICE_TYPE_HD21)
#endif
#ifdef USE_HD22
		HD_MENU_PREITEMS_A(21, ID_OPEN_HD22, ID_CLOSE_HD22, ID_WRITEPROTECT_HD22)
		HD_MENU_PREITEMS_C(21, ID_DEVICE_TYPE_HD22)
#endif
#ifdef USE_DATAREC
		case ID_PLAY_DATAREC:
			ShowLoadDataRecDialog();
			return 0;
		case ID_REC_DATAREC:
			ShowSaveDataRecDialog();
			return 0;
		case ID_CLOSE_DATAREC:
			PostEtCloseDataRecMessage();
			return 0;
		case ID_REWIND_DATAREC:
			PostEtRewindDataRecMessage();
			return 0;
		case ID_FAST_FORWARD_DATAREC:
			PostEtFastForwardDataRecMessage();
			return 0;
		case ID_STOP_DATAREC:
			PostEtStopDataRecMessage();
			return 0;
		case ID_REAL_DATAREC:
			PostEtToggleRealModeDataRecMessage();
			return 0;
#endif
#ifdef USE_DATAREC_BUTTON
		case ID_PLAY_BUTTON:
			if(emu) {
				emu->push_play();
			}
			break;
		case ID_STOP_BUTTON:
			if(emu) {
				emu->push_stop();
			}
			break;
#endif
#ifdef USE_CART1
		case ID_OPEN_CART:
			ShowOpenCartridgeDialog();
			return 0;
		case ID_CLOSE_CART:
			PostEtCloseCartridgeMessage(0);
			return 0;
#endif
#ifdef USE_QD1
		case ID_OPEN_QD:
			ShowOpenQuickDiskDialog();
			return 0;
		case ID_CLOSE_QD:
			PostEtCloseQuickDiskMessage();
			return 0;
#endif
#ifdef USE_MEDIA
		case ID_OPEN_MEDIA:
			ShowOpenMediaDialog();
			return 0;
		case ID_CLOSE_MEDIA:
			PostEtCloseMediaMessage();
			return 0;
#endif
#ifdef USE_BINARY_FILE1
		#define BINARY_MENU_ITEMS(drv, ID_LOAD_BINARY, ID_SAVE_BINARY) \
		case ID_LOAD_BINARY: \
			ShowLoadBinaryDialog(drv); \
			return 0; \
		case ID_SAVE_BINARY: \
			ShowSaveBinaryDialog(drv); \
			return 0;
		BINARY_MENU_ITEMS(0, ID_LOAD_BINARY1, ID_SAVE_BINARY1)
#endif
#ifdef USE_BINARY_FILE2
		BINARY_MENU_ITEMS(1, ID_LOAD_BINARY2, ID_SAVE_BINARY2)
#endif
#ifdef USE_EMU_INHERENT_SPEC
		case ID_SCREEN_REC_SIZE1:
		case ID_SCREEN_REC_SIZE2:
			PostEtResizeRecordVideoSurface(id - ID_SCREEN_REC_SIZE1);
			return 0;
#endif
		case ID_SCREEN_REC60:
		case ID_SCREEN_REC30:
		case ID_SCREEN_REC20:
		case ID_SCREEN_REC15:
		case ID_SCREEN_REC12:
		case ID_SCREEN_REC10:
//				StartRecordVideo(id - ID_SCREEN_REC60);
//				ShowRecordVideoDialog(id - ID_SCREEN_REC60);
			ShowRecordVideoAndAudioDialog(id - ID_SCREEN_REC60);
			return 0;
		case ID_SCREEN_STOP:
			PostEtStopRecordVideo();
			return 0;
		case ID_SCREEN_CAPTURE:
			PostEtCaptureScreen();
			return 0;
#ifdef USE_EMU_INHERENT_SPEC
		case ID_SCREEN_VFRAME:
			ChangeFrameRate(-1);
			return 0;
		case ID_SCREEN_FPS60:
		case ID_SCREEN_FPS30:
		case ID_SCREEN_FPS20:
		case ID_SCREEN_FPS15:
		case ID_SCREEN_FPS12:
		case ID_SCREEN_FPS10:
			ChangeFrameRate(id - ID_SCREEN_FPS60);
			return 0;
#endif
		case ID_SCREEN_WINDOW1:
		case ID_SCREEN_WINDOW2:
		case ID_SCREEN_WINDOW3:
		case ID_SCREEN_WINDOW4:
		case ID_SCREEN_WINDOW5:
		case ID_SCREEN_WINDOW6:
		case ID_SCREEN_WINDOW7:
		case ID_SCREEN_WINDOW8:
			ChangeWindowMode(id - ID_SCREEN_WINDOW1);
			return 0;
		case ID_SCREEN_WINDOW_A:
			ChangeWindowMode(-2);
			return 0;
#if 0
		case ID_SCREEN_FULLSCREEN1:
		case ID_SCREEN_FULLSCREEN2:
		case ID_SCREEN_FULLSCREEN3:
		case ID_SCREEN_FULLSCREEN4:
		case ID_SCREEN_FULLSCREEN5:
		case ID_SCREEN_FULLSCREEN6:
		case ID_SCREEN_FULLSCREEN7:
		case ID_SCREEN_FULLSCREEN8:
		case ID_SCREEN_FULLSCREEN9:
		case ID_SCREEN_FULLSCREEN10:
		case ID_SCREEN_FULLSCREEN11:
		case ID_SCREEN_FULLSCREEN12:
		case ID_SCREEN_FULLSCREEN13:
		case ID_SCREEN_FULLSCREEN14:
		case ID_SCREEN_FULLSCREEN15:
		case ID_SCREEN_FULLSCREEN16:
		case ID_SCREEN_FULLSCREEN17:
		case ID_SCREEN_FULLSCREEN18:
		case ID_SCREEN_FULLSCREEN19:
		case ID_SCREEN_FULLSCREEN20:
		case ID_SCREEN_FULLSCREEN21:
		case ID_SCREEN_FULLSCREEN22:
		case ID_SCREEN_FULLSCREEN23:
		case ID_SCREEN_FULLSCREEN24:
			ChangeFullScreenMode(id - ID_SCREEN_FULLSCREEN1);
			return 0;
#endif
		case ID_SCREEN_STRETCH:
			ChangeStretchScreen(1);
			return 0;
		case ID_SCREEN_CUTOUT:
			ChangeStretchScreen(2);
			return 0;
		case ID_SCREEN_STRETCH_A:
			ChangeStretchScreen(-1);
			return 0;
		// accelerator
		case ID_ACCEL_SCREEN:
			TogglgScreenMode();
			return 0;
#ifdef USE_MOUSE
		case ID_ACCEL_MOUSE:
			ToggleUseMouse();
			return 0;
#endif
#if defined(USE_MONITOR_TYPE) || defined(USE_SCREEN_ROTATE)
		case ID_SCREEN_MONITOR_TYPE0:
		case ID_SCREEN_MONITOR_TYPE1:
		case ID_SCREEN_MONITOR_TYPE2:
		case ID_SCREEN_MONITOR_TYPE3:
			pConfig->monitor_type = (id - ID_SCREEN_MONITOR_TYPE0);
			if(emu) {
				emu->update_config();
#ifdef USE_SCREEN_ROTATE
				emu->change_screen_mode(-1);
#endif
			}
			break;
#endif
#ifdef USE_SCANLINE
		case ID_SCREEN_SCANLINE0:
		case ID_SCREEN_SCANLINE1:
		case ID_SCREEN_SCANLINE2:
		case ID_SCREEN_SCANLINE3:
			PostEtChangeDrawMode(id - ID_SCREEN_SCANLINE0);
			return 0;
		case ID_SCREEN_SCANLINE_A:
			PostEtChangeDrawMode(-1);
			return 0;
#endif
#ifdef USE_DIRECT3D
		case ID_SCREEN_D3D_SYNC:
			ChangeUseDirect3D(1);
			return 0;
		case ID_SCREEN_D3D_ASYNC:
			ChangeUseDirect3D(2);
			return 0;
		case ID_SCREEN_D3D_A:
			ChangeUseDirect3D();
			return 0;
		case ID_SCREEN_D3D_FILTER0:
		case ID_SCREEN_D3D_FILTER1:
		case ID_SCREEN_D3D_FILTER2:
			ChangeDirect3DFilter(id - ID_SCREEN_D3D_FILTER0);
			return 0;
		case ID_SCREEN_D3D_FILTER_A:
			ChangeDirect3DFilter(-1);
			return 0;
#endif
#ifdef USE_OPENGL
		case ID_SCREEN_OPENGL_SYNC:
			ChangeUseOpenGL(1);
			return 0;
		case ID_SCREEN_OPENGL_ASYNC:
			ChangeUseOpenGL(2);
			return 0;
		case ID_SCREEN_OPENGL_A:
			ChangeUseOpenGL();
			return 0;
		case ID_SCREEN_OPENGL_FILTER0:
		case ID_SCREEN_OPENGL_FILTER1:
		case ID_SCREEN_OPENGL_FILTER2:
			ChangeOpenGLFilter(id - ID_SCREEN_OPENGL_FILTER0);
			return 0;
		case ID_SCREEN_OPENGL_FILTER_A:
			ChangeOpenGLFilter(-1);
			return 0;
#endif
#ifdef USE_EMU_INHERENT_SPEC
		case ID_SCREEN_PIXEL_ASPECT0:
		case ID_SCREEN_PIXEL_ASPECT1:
		case ID_SCREEN_PIXEL_ASPECT2:
		case ID_SCREEN_PIXEL_ASPECT3:
		case ID_SCREEN_PIXEL_ASPECT4:
			ChangePixelAspect(id - ID_SCREEN_PIXEL_ASPECT0);
			return 0;
		case ID_SCREEN_PIXEL_ASPECT_A:
			ChangePixelAspect(-1);
			return 0;
#ifdef USE_AFTERIMAGE
		case ID_SCREEN_AFTERIMAGE1:
			PostEtChangeAfterImage(1);
			return 0;
		case ID_SCREEN_AFTERIMAGE2:
			PostEtChangeAfterImage(2);
			return 0;
		case ID_SCREEN_AFTERIMAGE_A:
			PostEtChangeAfterImage();
			return 0;
#endif
#ifdef USE_KEEPIMAGE
		case ID_SCREEN_KEEPIMAGE1:
			PostEtChangeKeepImage(1);
			return 0;
		case ID_SCREEN_KEEPIMAGE2:
			PostEtChangeKeepImage(2);
			return 0;
		case ID_SCREEN_KEEPIMAGE_A:
			PostEtChangeKeepImage();
			return 0;
#endif
#ifdef _MBS1
		case ID_SCREEN_DIGITAL:
			ChangeRGBType(0);
			return 0;
		case ID_SCREEN_ANALOG:
			ChangeRGBType(1);
			return 0;
#endif
#ifdef _X68000
		case ID_SCREEN_SHOW_ALL:
			ToggleShowScreen(0);
			return 0;
		case ID_SCREEN_SHOW_01:
		case ID_SCREEN_SHOW_02:
		case ID_SCREEN_SHOW_03:
		case ID_SCREEN_SHOW_04:
		case ID_SCREEN_SHOW_05:
		case ID_SCREEN_SHOW_06:
		case ID_SCREEN_SHOW_07:
		case ID_SCREEN_SHOW_08:
		case ID_SCREEN_SHOW_09:
			ToggleShowScreen(1 << (id - ID_SCREEN_SHOW_01));
			return 0;
#endif

#ifdef USE_DEBUG_SOUND_FILTER
		case ID_SOUND_FILTER:
			ShowSndFilterDialog();
			return 0;
#endif
#endif
		case ID_SOUND_VOLUME:
			ShowVolumeDialog();
			return 0;
		case ID_SOUND_REC:
//				PostEtStartRecordSound();
			ShowRecordAudioDialog();
			return 0;
		case ID_SOUND_STOP:
			PostEtStopRecordSound();
			return 0;
		case ID_SOUND_FREQ0:
		case ID_SOUND_FREQ1:
		case ID_SOUND_FREQ2:
		case ID_SOUND_FREQ3:
		case ID_SOUND_FREQ4:
		case ID_SOUND_FREQ5:
		case ID_SOUND_FREQ6:
		case ID_SOUND_FREQ7:
			ChangeSoundFrequency(id - ID_SOUND_FREQ0);
			return 0;
		case ID_SOUND_LATE0:
		case ID_SOUND_LATE1:
		case ID_SOUND_LATE2:
		case ID_SOUND_LATE3:
		case ID_SOUND_LATE4:
		case ID_SOUND_LATE5:
			ChangeSoundLatency(id - ID_SOUND_LATE0);
			return 0;
#ifdef USE_SOUND_DEVICE_TYPE
		case ID_SOUND_DEVICE_TYPE0:
		case ID_SOUND_DEVICE_TYPE1:
		case ID_SOUND_DEVICE_TYPE2:
		case ID_SOUND_DEVICE_TYPE3:
			pConfig->sound_device_type = LOWORD(wParam) - ID_SOUND_DEVICE_TYPE0;
			//if(emu) {
			//	emu->update_config();
			//}
			break;
#endif
#ifdef USE_MIDI
		case ID_MIDI_RESET_GM:
		case ID_MIDI_RESET_GS:
		case ID_MIDI_RESET_LA:
		case ID_MIDI_RESET_XG:
			SendMidiResetMessage(id - ID_MIDI_RESET_GM);
			return 0;
#endif
#ifdef USE_BUTTON
		case ID_BUTTON +  0:
		case ID_BUTTON +  1:
		case ID_BUTTON +  2:
		case ID_BUTTON +  3:
		case ID_BUTTON +  4:
		case ID_BUTTON +  5:
		case ID_BUTTON +  6:
		case ID_BUTTON +  7:
		case ID_BUTTON +  8:
		case ID_BUTTON +  9:
		case ID_BUTTON + 10:
		case ID_BUTTON + 11:
		case ID_BUTTON + 12:
		case ID_BUTTON + 13:
		case ID_BUTTON + 14:
		case ID_BUTTON + 15:
		case ID_BUTTON + 16:
		case ID_BUTTON + 17:
		case ID_BUTTON + 18:
		case ID_BUTTON + 19:
		case ID_BUTTON + 20:
		case ID_BUTTON + 21:
		case ID_BUTTON + 22:
		case ID_BUTTON + 23:
		case ID_BUTTON + 24:
		case ID_BUTTON + 25:
		case ID_BUTTON + 26:
		case ID_BUTTON + 27:
		case ID_BUTTON + 28:
		case ID_BUTTON + 29:
		case ID_BUTTON + 30:
		case ID_BUTTON + 31:
			if(emu) {
				emu->press_button(LOWORD(wParam) - ID_BUTTON);
			}
			break;
#endif
#ifdef USE_PRINTER
#define PRINTER_MENU_PREITEMS(drv, ID_PRINTER_SAVE, ID_PRINTER_PRINT, ID_PRINTER_CLEAR, ID_PRINTER_DIRECT, ID_PRINTER_ONLINE) \
		case ID_PRINTER_SAVE: \
			ShowSavePrinterDialog(drv); \
			return 0; \
		case ID_PRINTER_PRINT: \
/*			PostEtPrintPrinterMessage(drv);*/ \
			PrintPrinter(drv); \
			return 0; \
		case ID_PRINTER_CLEAR: \
			PostEtClearPrinterBufferMessage(drv); \
			return 0; \
		case ID_PRINTER_DIRECT: \
/*			PostEtEnablePrinterDirectMessage(drv);*/ \
			EnablePrinterDirect(drv); \
			return 0; \
		case ID_PRINTER_ONLINE: \
			PostEtTogglePrinterOnlineMessage(drv); \
			return 0;
		PRINTER_MENU_PREITEMS(0, ID_PRINTER0_SAVE, ID_PRINTER0_PRINT, ID_PRINTER0_CLEAR, ID_PRINTER0_DIRECT, ID_PRINTER0_ONLINE)
		PRINTER_MENU_PREITEMS(1, ID_PRINTER1_SAVE, ID_PRINTER1_PRINT, ID_PRINTER1_CLEAR, ID_PRINTER1_DIRECT, ID_PRINTER1_ONLINE)
		PRINTER_MENU_PREITEMS(2, ID_PRINTER2_SAVE, ID_PRINTER2_PRINT, ID_PRINTER2_CLEAR, ID_PRINTER2_DIRECT, ID_PRINTER2_ONLINE)
#endif
#ifdef USE_EMU_INHERENT_SPEC
#define COMM_MENU_PREITEMS(drv, ID_COMM_SERVER, ID_COMM_CONNECT, ID_COMM_THROUGH, ID_COMM_WILLECHO, ID_COMM_BINARY) \
		case ID_COMM_SERVER: \
/*			PostEtEnableCommServerMessage(drv);*/ \
			ToggleEnableCommServer(drv); \
			return 0; \
		case ID_COMM_CONNECT: \
/*			PostEtConnectCommMessage(drv);*/ \
			ToggleConnectComm(drv, 0); \
			return 0; \
		case ID_COMM_THROUGH: \
			ToggleCommThroughMode(drv); \
			return 0; \
		case ID_COMM_WILLECHO: \
			SendCommTelnetCommand(drv, 1); \
			return 0; \
		case ID_COMM_BINARY: \
			ToggleCommBinaryMode(drv); \
			return 0;
		COMM_MENU_PREITEMS(0, ID_COMM0_SERVER, ID_COMM0_CONNECT, ID_COMM0_THROUGH, ID_COMM0_WILLECHO, ID_COMM0_BINARY)
		COMM_MENU_PREITEMS(1, ID_COMM1_SERVER, ID_COMM1_CONNECT, ID_COMM1_THROUGH, ID_COMM1_WILLECHO, ID_COMM1_BINARY)

		case ID_OPTIONS_LEDBOX_A:
			ToggleLedBox();
			return 0;
		case ID_OPTIONS_LEDBOX_SHOW:
			ToggleShowLedBox();
			return 0;
		case ID_OPTIONS_LEDBOX_INSIDE:
			ToggleInsideLedBox();
			return 0;
		case ID_OPTIONS_LEDBOX_POS_A:
			ChangeLedBoxPosition(-1);
			return 0;
		case ID_OPTIONS_MSGBOARD:
			ToggleMessageBoard();
			return 0;
#ifdef USE_PERFORMANCE_METER
		case ID_OPTIONS_PMETER:
			TogglePMeter();
			return 0;
#endif
		case ID_OPTIONS_JOYPAD0:
			ChangeUseJoypad(1);
			return 0;
		case ID_OPTIONS_JOYPAD1:
			ChangeUseJoypad(2);
			return 0;
		case ID_OPTIONS_JOYPAD_A:
			ChangeUseJoypad(-1);
			return 0;
#ifdef USE_KEY2JOYSTICK
		case ID_OPTIONS_KEY2JOYPAD:
			ToggleEnableKey2Joypad();
			return 0;
#endif
		case ID_OPTIONS_JOYSETTING:
			ShowJoySettingDialog();
			return 0;
#ifdef USE_MOUSE
		case ID_OPTIONS_MOUSE:
			ToggleUseMouse();
			return 0;
#endif
#ifdef USE_LIGHTPEN
		case ID_OPTIONS_LIGHTPEN:
			ToggleEnableLightpen();
			return 0;
#endif
#ifdef USE_DIRECTINPUT
		case ID_OPTIONS_USE_DINPUT:
			ToggleUseDirectInput();
			return 0;
#endif
		case ID_OPTIONS_LOOSEN_KEY:
			ToggleLoosenKeyStroke();
			return 0;
		case ID_OPTIONS_KEYBIND:
			ShowKeybindDialog();
			return 0;
		case ID_OPTIONS_VKEYBOARD:
			ShowVirtualKeyboard();
			return 0;
		case ID_OPTIONS_CONFIG:
			ShowConfigureDialog();
			return 0;
		case ID_OPTIONS_LOGGING:
			ShowLoggingDialog();
			return 0;
		case ID_OPTIONS_FDD_TYPE_A:
			ChangeFddType(-1);
			return 0;
#endif
		case ID_HELP_ABOUT:
			ShowAboutDialog();
			return 0;
		case ID_EXIT:
			Exit();
			return 0;
		default:
			if (id == 0) {
				break;
			}
			else if (id >= ID_SCREEN_FULLSCREEN0_01 && id < (ID_SCREEN_FULLSCREEN5_01 + VIDEO_MODE_MAX)) {
				ChangeFullScreenMode(id - ID_SCREEN_FULLSCREEN0_01);
				return 0;
			}
#define MENU_RECENT(ID_RECENT, FUNCTION) \
			else if (id >= ID_RECENT && id < (ID_RECENT + MAX_HISTORY)) { \
				FUNCTION(id - ID_RECENT); \
				return 0; \
			}
#define MENU_RECENT_WITH_DRV(drv, ID_RECENT, FUNCTION) \
			else if (id >= ID_RECENT && id < (ID_RECENT + MAX_HISTORY)) { \
				FUNCTION(drv, id - ID_RECENT); \
				return 0; \
			}
#ifdef USE_DATAREC
			MENU_RECENT(ID_RECENT_DATAREC, PostEtLoadRecentDataRecMessage)
#endif
#ifdef USE_FD1
#define FD_MENU_RECENT(drv, ID_RECENT_FD, ID_SELECT_D88_BANK) \
			MENU_RECENT_WITH_DRV(drv, ID_RECENT_FD, PostEtOpenRecentFloppyMessage) \
			else if (id >= ID_SELECT_D88_BANK && id < (ID_SELECT_D88_BANK + 50)) { \
				PostEtOpenFloppySelectedVolume(drv, id - ID_SELECT_D88_BANK); \
				return 0; \
			}
			FD_MENU_RECENT(0, ID_RECENT_FD1, ID_SELECT_D88_BANK1)
#endif
#ifdef USE_FD2
			FD_MENU_RECENT(1, ID_RECENT_FD2, ID_SELECT_D88_BANK2)
#endif
#ifdef USE_FD3
			FD_MENU_RECENT(2, ID_RECENT_FD3, ID_SELECT_D88_BANK3)
#endif
#ifdef USE_FD4
			FD_MENU_RECENT(3, ID_RECENT_FD4, ID_SELECT_D88_BANK4)
#endif
#ifdef USE_FD5
			FD_MENU_RECENT(4, ID_RECENT_FD5, ID_SELECT_D88_BANK5)
#endif
#ifdef USE_FD6
			FD_MENU_RECENT(5, ID_RECENT_FD6, ID_SELECT_D88_BANK6)
#endif
#ifdef USE_HD1
#define HD_MENU_RECENT(drv, ID_RECENT_HD) \
			MENU_RECENT_WITH_DRV(drv, ID_RECENT_HD, PostEtOpenRecentHardDiskMessage)
			HD_MENU_RECENT(0, ID_RECENT_HD1)
#ifdef USE_HD2
			HD_MENU_RECENT(1, ID_RECENT_HD2)
#endif
#ifdef USE_HD3
			HD_MENU_RECENT(2, ID_RECENT_HD3)
#endif
#ifdef USE_HD4
			HD_MENU_RECENT(3, ID_RECENT_HD4)
#endif
#ifdef USE_HD5
			HD_MENU_RECENT(4, ID_RECENT_HD5)
#endif
#ifdef USE_HD6
			HD_MENU_RECENT(5, ID_RECENT_HD6)
#endif
#ifdef USE_HD17
			HD_MENU_RECENT(16, ID_RECENT_HD17)
#endif
#ifdef USE_HD18
			HD_MENU_RECENT(17, ID_RECENT_HD18)
#endif
#ifdef USE_HD19
			HD_MENU_RECENT(18, ID_RECENT_HD19)
#endif
#ifdef USE_HD20
			HD_MENU_RECENT(19, ID_RECENT_HD20)
#endif
#ifdef USE_HD21
			HD_MENU_RECENT(20, ID_RECENT_HD21)
#endif
#ifdef USE_HD22
			HD_MENU_RECENT(21, ID_RECENT_HD22)
#endif
#endif
#ifdef USE_CART1
			MENU_RECENT_WITH_DRV(0, ID_RECENT_CART, PostEtOpenRecentCartridgeMessage)
#endif
#ifdef USE_QD1
			MENU_RECENT_WITH_DRV(0, ID_RECENT_QD, PostEtOpenRecentQuickDiskMessage)
#endif
#ifdef USE_MEDIA
			MENU_RECENT(ID_RECENT_MEDIA, PostEtOpenRecentMediaMessage)
#endif
#ifdef USE_BINARY_FILE1
			MENU_RECENT_WITH_DRV(0, ID_RECENT_BINARY1, PostEtLoadRecentBinaryMessage)
#endif
#ifdef USE_BINARY_FILE2
			MENU_RECENT_WITH_DRV(1, ID_RECENT_BINARY2, PostEtLoadRecentBinaryMessage)
#endif
#ifdef USE_UART
#define COMM_MENU_UART(drv, ID_START, ID_END) \
			else if (id >= ID_START && id < ID_END) { \
				ToggleConnectComm(drv, id - ID_START + 1); \
				return 0; \
			}
			COMM_MENU_UART(0, ID_COMM0_PORT1, ID_COMM0_PORT_BOTTOM)
			COMM_MENU_UART(1, ID_COMM1_PORT1, ID_COMM1_PORT_BOTTOM)
#endif
#ifdef USE_MIDI
			else if (id >= ID_MIDIIN_PORT1 && id < ID_MIDIIN_PORT_BOTTOM) {
				ToggleConnectMidiIn(id - ID_MIDIIN_PORT1);
				return 0;
			}
			else if (id >= ID_MIDIOUT_PORT1 && id < ID_MIDIOUT_PORT_BOTTOM) {
				ToggleConnectMidiOut(id - ID_MIDIOUT_PORT1);
				return 0;
			}
			else if (id >= ID_MIDIOUT_LATE1 && id <= ID_MIDIOUT_LATE6) {
				ChangeMidiOutLatencyByNum(id - ID_MIDIOUT_LATE1);
				return 0;
			}
			else if (id == ID_MIDIOUT_LATE_OTHER) {
				ShowMIDIOutLatencyDialog();
				return 0;
			}
#endif
			MENU_RECENT(ID_RECENT_STATE, PostEtLoadRecentStatusMessage)
			break;
	}
	return 0;
}

///
void GUI_BASE::KeyDown(int code, uint32_t mod)
{
}

///
void GUI_BASE::KeyUp(int code)
{
}

///
void GUI_BASE::SetFocusToMainWindow()
{
}

void GUI_BASE::GetLibVersionString(_TCHAR *str, int max_len, const _TCHAR *sep_str)
{
#if defined(USE_WIN)

	*str = _T('\0');

#elif defined(USE_SDL) || defined(USE_SDL2)

	_TCHAR buf[_MAX_PATH];
	size_t len = 0;
	SDL_version sdl_ver;
#if defined(USE_SDL2) || defined(USE_WX2)
	SDL_GetVersion(&sdl_ver);
#else
	const SDL_version *p = SDL_Linked_Version();
	sdl_ver = *p;
#endif
	UTILITY::stprintf(&buf[len], _MAX_PATH - len, _T("SDL Version %d.%d.%d")
		, sdl_ver.major, sdl_ver.minor, sdl_ver.patch);

	const SDL_version *sdl_ttf_ver = TTF_Linked_Version();

	UTILITY::tcscat(buf, _MAX_PATH, sep_str);
	len = _tcslen(buf);
	UTILITY::stprintf(&buf[len], _MAX_PATH - len, _T("SDL_ttf Version %d.%d.%d")
		, sdl_ttf_ver->major, sdl_ttf_ver->minor, sdl_ttf_ver->patch);

#if defined(USE_SOCKET) && defined(USE_SDL_NET)
#ifndef USE_SDL2
	const SDL_version *sdl_net_ver;
#else
	const SDLNet_version *sdl_net_ver;
#endif
	sdl_net_ver = SDLNet_Linked_Version();

	UTILITY::tcscat(buf, _MAX_PATH, sep_str);
	len = _tcslen(buf);
	UTILITY::stprintf(&buf[len], _MAX_PATH - len, _T("SDL_Net Version %d.%d.%d")
		, sdl_net_ver->major, sdl_net_ver->minor, sdl_net_ver->patch);
#endif

	UTILITY::tcsncpy(str, max_len, buf, _MAX_PATH);

#endif
}

/// exit program
void GUI_BASE::Exit(void)
{
#if defined(USE_WIN)
	SendMessage(hWindow, WM_CLOSE, 0, 0L);
#elif defined(USE_SDL) || defined(USE_SDL2)
#ifdef USE_GTK
	exit_program = true;
#else
	SDL_Event e;
	e.type = SDL_QUIT;

	SDL_PushEvent(&e);
#endif
#endif
}

#ifdef USE_DATAREC
bool GUI_BASE::ShowLoadDataRecDialog(void)
{
	return false;
}

bool GUI_BASE::ShowSaveDataRecDialog(void)
{
	return false;
}
#endif

#ifdef USE_FD1
bool GUI_BASE::ShowOpenFloppyDiskDialog(int drv)
{
	return false;
}

int GUI_BASE::ShowSelectFloppyDriveDialog(int drv)
{
	return drv;
}

bool GUI_BASE::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
	return false;
}
#endif

#ifdef USE_HD1
bool GUI_BASE::ShowOpenHardDiskDialog(int drv)
{
	return false;
}

bool GUI_BASE::ShowOpenBlankHardDiskDialog(int drv, uint8_t type)
{
	return false;
}

bool GUI_BASE::ShowSelectHardDiskDeviceTypeDialog(int drv)
{
	return false;
}
#endif

#ifdef USE_CART1
bool GUI_BASE::ShowOpenCartridgeDialog(int drv)
{
	return false;
}
#endif

#ifdef USE_QD1
bool GUI_BASE::ShowOpenQuickDiskDialog(int drv)
{
	return false;
}
#endif

#ifdef USE_MEDIA
bool GUI_BASE::ShowOpenMediaDialog(void)
{
	return false;
}
#endif

#ifdef USE_BINARY_FILE1
bool GUI_BASE::ShowLoadBinaryDialog(void)
{
	return false;
}
bool GUI_BASE::ShowSaveBinaryDialog(void)
{
	return false;
}
#endif

bool GUI_BASE::ShowLoadStateDialog(void)
{
	return false;
}

bool GUI_BASE::ShowSaveStateDialog(bool cont)
{
	return false;
}

bool GUI_BASE::ShowOpenAutoKeyDialog(void)
{
	return false;
}

bool GUI_BASE::ShowPlayRecKeyDialog(void)
{
	return false;
}

bool GUI_BASE::ShowRecordRecKeyDialog(void)
{
	return false;
}

bool GUI_BASE::ShowRecordStateAndRecKeyDialog(void)
{
	return false;
}

bool GUI_BASE::ShowSavePrinterDialog(int drv)
{
	return false;
}

bool GUI_BASE::ShowRecordVideoDialog(int fps_num)
{
	return false;
}

bool GUI_BASE::ShowRecordAudioDialog(void)
{
	return false;
}

bool GUI_BASE::ShowRecordVideoAndAudioDialog(int fps_num)
{
	return false;
}

bool GUI_BASE::ShowVolumeDialog(void)
{
	return false;
}

#ifdef USE_DEBUG_SOUND_FILTER
bool GUI_BASE::ShowSndFilterDialog(void)
{
	return false;
}
#endif

#ifdef USE_MIDI
bool GUI_BASE::ShowMIDIOutLatencyDialog(void)
{
	return false;
}
#endif

bool GUI_BASE::ShowJoySettingDialog(void)
{
	return false;
}

bool GUI_BASE::ShowKeybindDialog(void)
{
	return false;
}

bool GUI_BASE::ShowConfigureDialog(void)
{
	return false;
}

bool GUI_BASE::ShowLoggingDialog(void)
{
	return false;
}

bool GUI_BASE::ShowAboutDialog(void)
{
	return false;
}

bool GUI_BASE::ShowVirtualKeyboard(void)
{
	return false;
}

void GUI_BASE::UpdateVirtualKeyboard(uint32_t flag)
{
#ifdef USE_VKEYBOARD
	if (vkeyboard) {
		if (!vkeyboard->UpdateStatus(flag)) {
			delete vkeyboard;
			vkeyboard = NULL;
		}
	}
#endif
}

bool GUI_BASE::IsShownVirtualKeyboard(void)
{
#ifdef USE_VKEYBOARD
	return (vkeyboard != NULL);
#else
	return false;
#endif
}

/// Reset
void GUI_BASE::PostEtReset(void)
{
	emumsg.Send(EMUMSG_ID_RESET);
}
bool GUI_BASE::NowPowerOff(void)
{
	return pConfig->now_power_off;
}
/// ForceReset
void GUI_BASE::PostEtForceReset(void)
{
	emumsg.Send(EMUMSG_ID_FORCE_RESET);
}
/// SpecialReset
void GUI_BASE::PostEtSpecialReset(void)
{
	emumsg.Send(EMUMSG_ID_SPECIAL_RESET);
}
bool GUI_BASE::NowSpecialReset(void)
{
	return emu->now_special_reset();
}
/// WarmReset
void GUI_BASE::PostEtWarmReset(int onoff)
{
	emumsg.Send(EMUMSG_ID_WARM_RESET, onoff);
}
/// change dipswitch
void GUI_BASE::Dipswitch(int bit)
{
	emu->change_dipswitch(bit);
}
uint8_t GUI_BASE::GetDipswitch(void)
{
#ifdef USE_DIPSWITCH
	return pConfig->dipswitch;
#else
	return 0;
#endif
}
#ifdef _MBS1
/// change system mode
void GUI_BASE::ChangeSystemMode(int val)
{
	emu->change_archtecture(VM::ArchSysMode, val, false);
}
void GUI_BASE::ToggleSystemMode(void)
{
	emu->change_archtecture(VM::ArchSysMode, 1 - GetSystemMode(), false);
}
int GUI_BASE::GetSystemMode(void)
{
	return (emu->get_parami(VM::ParamSysMode) & SYS_MODE_S1L3);
}
#endif

/// post to assert interrupt message to emu thread
void GUI_BASE::PostEtInterrupt(int num)
{
	emumsg.Send(EMUMSG_ID_INTERRUPT, num);
}

/// post pause message to emu thread
void GUI_BASE::PostEtTogglePause(void)
{
	emumsg.Send(EMUMSG_ID_PAUSE);
}
/// change pause by user (called by emu thread)
void GUI_BASE::TogglePause(void)
{
	bool vm_pause = emu->get_pause(2);
	vm_pause = !vm_pause;
	emu->set_pause(2, vm_pause);
}
/// now pausing by user
bool GUI_BASE::NowPause(void)
{
	return emu->get_pause(2);
}
/// post system pause message to emu thread
void GUI_BASE::PostEtSystemPause(bool val)
{
	emumsg.Send(EMUMSG_ID_SYSTEM_PAUSE, val ? 1 : 0);
}
/// change pause by system
void GUI_BASE::SystemPause(bool val)
{
	emu->set_pause(1, val);
	globalkey_enable = val ? 0x80 : 0;	// reverse
	emu->mute_sound(val);
}
/// now pausing by user or system
bool GUI_BASE::NowSystemPause(void)
{
	return emu->get_pause(3);
}

// change cpu power
void GUI_BASE::PostEtCPUPower(int num)
{
	emumsg.Send(EMUMSG_ID_CPU_POWER, num);
}
int GUI_BASE::GetCPUPower(void)
{
	return pConfig->cpu_power;
}
// change FDD Type
void GUI_BASE::ChangeFddType(int num)
{
	emu->change_archtecture(VM::ArchFddType, num, false);
}
int GUI_BASE::GetFddType(void)
{
	return 0; //pConfig->fdd_type;
}
int GUI_BASE::NextFddType(void)
{
	return 0;// emu->get_parami(VM::ParamFddType);
}
void GUI_BASE::PostEtToggleSyncIRQ(void)
{
	emumsg.Send(EMUMSG_ID_SYNC_IRQ);
}
bool GUI_BASE::NowSyncIRQ(void)
{
	return pConfig->sync_irq;
}
#ifdef _MBS1
void GUI_BASE::ToggleMemNoWait(void)
{
	pConfig->mem_nowait = !pConfig->mem_nowait;
	emu->out_info_x(pConfig->mem_nowait ? CMsg::Memory_Without_Wait : CMsg::Memory_With_Wait);
}
bool GUI_BASE::NowMemNoWait(void)
{
	return pConfig->mem_nowait;
}
#endif

// Screen

/// Frame Rate
void GUI_BASE::ChangeFrameRate(int num)
{
	pConfig->fps_no = num;
//	gui->emu->update_config();
}
int GUI_BASE::GetFrameRateNum(void)
{
	return pConfig->fps_no;
}
/// Toggle Window <-> Fullscreen
void GUI_BASE::TogglgScreenMode(void)
{
	emu->change_screen_mode(-1);
}
/// Change Window Mode
void GUI_BASE::ChangeWindowMode(int num)
{
	emu->change_screen_mode(num >= 0 ? num : -2);
}
/// Change Screen Mode
void GUI_BASE::ChangeFullScreenMode(int num)
{
	if (!IsFullScreen())
		emu->change_screen_mode(num + 8);
}
/// Get current window mode number
int GUI_BASE::GetWindowMode(void)
{
	int num = pConfig->window_mode;
	return num < 8 ? num : -1;
}
/// Get current fullscreen mode number
int GUI_BASE::GetFullScreenMode(void)
{
	int num = pConfig->window_mode - 8;
	return num >= 0 ? num : -1;
}
/// Get number of window modes
int GUI_BASE::GetWindowModeCount(void)
{
	return emu->get_window_mode_count();
}
/// Get string of specified window mode
/// @param[in]  num window mode number
/// @param[out] str string
bool GUI_BASE::GetWindowModeStr(int num, _TCHAR *str) const
{
	const CWindowMode * mode = emu->get_window_mode(num);
	if (mode != NULL) {
		UTILITY::stprintf(str, 32, _T("%dx%d x%.1f"),
			mode->width,
			mode->height,
			(double)mode->power / 10.0);
		return true;
	} else {
		*str = _T('\0');
	}
	return false;
}
/// Get number of displays
int GUI_BASE::GetDisplayDeviceCount(void)
{
	return emu->get_display_device_count();
}
/// Get string of specified display
/// @param[in]  prefix
/// @param[in]  num display number
/// @param[out] str string
bool GUI_BASE::GetDisplayDeviceStr(const _TCHAR *prefix, int num, _TCHAR *str) const
{
	const CDisplayDevice *dd = emu->get_display_device(num);
	if (dd != NULL) {
		UTILITY::tcscpy(str, 32, prefix);
		int len = (int)_tcslen(str);
		UTILITY::stprintf(&str[len], 32, _T(" %d"), num);
		return true;
	} else {
		*str = _T('\0');
	}
	return false;
}
/// Get number of fullscreen modes
/// @param[in] disp_no display number
int GUI_BASE::GetFullScreenModeCount(int disp_no)
{
	return emu->get_screen_mode_count(disp_no);
}
/// Get string of specified fullscreen mode
/// @param[in]  disp_no display number
/// @param[in]  num fullscreen mode number
/// @param[out] str string
bool GUI_BASE::GetFullScreenModeStr(int disp_no, int num, _TCHAR *str) const
{
	const CVideoMode *mode = emu->get_screen_mode(disp_no, num);
	if (mode != NULL) {
		UTILITY::stprintf(str, 32, _T("[%d] %dx%d"),
			disp_no,
			mode->width,
			mode->height);
		return true;
	} else {
		*str = _T('\0');
	}
	return false;
}
/// Get string of specified video size
/// @param[in]  num video size number
/// @param[out] str string
bool GUI_BASE::GetRecordVideoSizeStr(int num, _TCHAR *str) const
{
	VmRectWH *mode = emu->get_screen_record_size(num);
	if (mode != NULL) {
		UTILITY::stprintf(str, 32, _T("%dx%d"),
			mode->w,
			mode->h);
		return true;
	} else {
		*str = _T('\0');
	}
	return false;
}
/// Fullscreen now?
bool GUI_BASE::IsFullScreen(void)
{
	return emu->is_fullscreen();
}
/// Change Scan Line
void GUI_BASE::PostEtChangeDrawMode(int num)
{
	emumsg.Send(EMUMSG_ID_SCREEN_SCANLINE1, num);
}
/// Rotate Scan Line
void GUI_BASE::PostEtChangeDrawMode(void)
{
	emumsg.Send(EMUMSG_ID_SCREEN_SCANLINE_A);
}
int GUI_BASE::GetDrawMode(void)
{
	return pConfig->scan_line;
}
/// Change Stretch Screen
void GUI_BASE::ChangeStretchScreen(int num)
{
	emu->change_stretch_screen(num);
}
int GUI_BASE::GetStretchScreen(void)
{
	return pConfig->stretch_screen;
}
#if 0
/// Change Cutoff Screen
void GUI_BASE::ToggleCutoutScreen(void)
{
	emu->change_cutout_screen();
}
bool GUI_BASE::NowCutoutedScreen(void)
{
	return pConfig->cutout_screen;
}
#endif
void GUI_BASE::ChangePixelAspect(int num)
{
	emu->change_pixel_aspect(num);
//	emu->change_screen_mode(pConfig->window_mode);
}
int GUI_BASE::GetPixelAspectMode(void)
{
	return pConfig->pixel_aspect;
}
int GUI_BASE::GetPixelAspectModeCount(void)
{
	return emu->get_pixel_aspect_count();
}
void GUI_BASE::GetPixelAspectModeStr(int num, _TCHAR *str)
{
	int w, h;
	num = emu->get_pixel_aspect(num, &w, &h);
	UTILITY::stprintf(str, 32, _T("%d:%d"), w, h);
}

void GUI_BASE::PostEtStartRecordVideo(int num)
{
	emumsg.Send(EMUMSG_ID_SCREEN_START_RECORD, num);
}
void GUI_BASE::PostEtStopRecordVideo(void)
{
	emumsg.Send(EMUMSG_ID_SCREEN_STOP_RECORD);
}
bool GUI_BASE::NowRecordingVideo(void)
{
	return emu->now_rec_video();
}
int GUI_BASE::GetRecordVideoFrameNum(void)
{
	return emu->get_rec_video_fps_num();
}
void GUI_BASE::PostEtResizeRecordVideoSurface(int num)
{
	emumsg.Send(EMUMSG_ID_SCREEN_RESIZE_RECORD, num);
}
int GUI_BASE::GetRecordVideoSurfaceNum()
{
	return pConfig->screen_video_size;
}
void GUI_BASE::PostEtCaptureScreen(void)
{
	emumsg.Send(EMUMSG_ID_SCREEN_CAPTURE);
}

#ifdef USE_AFTERIMAGE
/// Change After Image
void GUI_BASE::PostEtChangeAfterImage(int num)
{
	emumsg.Send(EMUMSG_ID_SCREEN_AFTERIMAGE1, num);
}
void GUI_BASE::PostEtChangeAfterImage(void)
{
	emumsg.Send(EMUMSG_ID_SCREEN_AFTERIMAGE_A);
}
int GUI_BASE::GetAfterImageMode(void)
{
	return pConfig->afterimage;
}
#endif
#ifdef USE_KEEPIMAGE
/// Change Keep Image
void GUI_BASE::PostEtChangeKeepImage(int num)
{
	emumsg.Send(EMUMSG_ID_SCREEN_KEEPIMAGE1, num);
}
void GUI_BASE::PostEtChangeKeepImage(void)
{
	emumsg.Send(EMUMSG_ID_SCREEN_KEEPIMAGE_A);
}
int GUI_BASE::GetKeepImageMode(void)
{
	return pConfig->keepimage;
}
#endif
#ifdef _MBS1
/// Change RGB Type
void GUI_BASE::ChangeRGBType(int num)
{
	switch(num) {
	case 1:
		pConfig->tvsuper = (pConfig->tvsuper | 0x10);
		break;
	default:
		pConfig->tvsuper = (pConfig->tvsuper & ~0x10);
		break;
	}
}
void GUI_BASE::ChangeRGBType(void)
{
	int num = 1 - GetRGBTypeMode();
	ChangeRGBType(num);
}
int GUI_BASE::GetRGBTypeMode(void)
{
	return ((pConfig->tvsuper & 0x10) >> 4);
}
#endif
#ifdef _X68000
void GUI_BASE::ToggleShowScreen(int flags)
{
	int val = emu->get_parami(VM::ParamHideScreen);
	if (flags == 0)
		val = 0;
	else
		val ^= flags;

	static const _TCHAR *lbl[] = {
		_T(" G0"), _T(" G1"), _T(" G2"), _T(" G3"), _T(" G4"),
		_T(" TX"), _T(" SP"), _T(" B0"), _T(" B1"),
		NULL
	};
	_TCHAR str[64];
	UTILITY::tcscpy(str, 64, _T("ShowScreen"));
	for(int i=0; i<9; i++) {
		UTILITY::tcscat(str, 64, (val & (1 << i)) ? _T(" --") : lbl[i]);
	}

	emu->out_info(str);
	emu->set_parami(VM::ParamHideScreen, val);
}
int GUI_BASE::GetShowScreen()
{
	return ~emu->get_parami(VM::ParamHideScreen);
}
#endif

#ifdef USE_DIRECT3D
/// Change Use Direct3D
void GUI_BASE::ChangeUseDirect3D(int num)
{
	emu->change_screen_use_direct3d(num);
}
void GUI_BASE::ChangeUseDirect3D(void)
{
	emu->change_screen_use_direct3d(-1);
}
int GUI_BASE::GetDirect3DMode(void)
{
	return pConfig->use_direct3d;
}
void GUI_BASE::ChangeDirect3DFilter(int num)
{
	const CMsg::Id list[] = {
		CMsg::None_,
		CMsg::Point,
		CMsg::Linear,
		CMsg::End
	};

	if (num < 0) {
		num = (pConfig->d3d_filter_type + 1) % 3;
	}
	pConfig->d3d_filter_type = num;

	emu->out_infoc_x(CMsg::Direct3D_Filter, CMsg::Colon_Space, list[num], 0);
}
int GUI_BASE::GetDirect3DFilter(void)
{
	return pConfig->d3d_filter_type;
}
#endif
#ifdef USE_OPENGL
/// Change Use OpenGL
void GUI_BASE::ChangeUseOpenGL(int num)
{
	emu->change_screen_use_opengl(num);
}
void GUI_BASE::ChangeUseOpenGL(void)
{
	emu->change_screen_use_opengl(-1);
}
int GUI_BASE::GetOpenGLMode(void)
{
	return pConfig->use_opengl;
}
void GUI_BASE::ChangeOpenGLFilter(int num)
{
	const CMsg::Id list[] = {
		CMsg::Nearest_Neighbour,
		CMsg::Linear,
		CMsg::End
	};

	if (num < 0) {
		num = (pConfig->gl_filter_type + 1) % 2;
	}
	pConfig->gl_filter_type = num;

	emu->change_opengl_attr();

	emu->out_infoc_x(CMsg::OpenGL_Filter, CMsg::Colon_Space, list[num], 0);
}
int GUI_BASE::GetOpenGLFilter(void)
{
	return pConfig->gl_filter_type;
}
#endif

// Sound

void GUI_BASE::PostEtStartRecordSound(void)
{
	emumsg.Send(EMUMSG_ID_SOUND_START_RECORD);
//	emu->start_rec_sound();
}
void GUI_BASE::PostEtStopRecordSound(void)
{
	emumsg.Send(EMUMSG_ID_SOUND_STOP_RECORD);
//	emu->stop_rec_sound();
}
bool GUI_BASE::NowRecordingSound(void)
{
	return emu->now_rec_sound();
}
void GUI_BASE::ChangeSoundFrequency(int num)
{
	const CMsg::Id list[] = {
		CMsg::F2000Hz,
		CMsg::F4000Hz,
		CMsg::F8000Hz,
		CMsg::F11025Hz,
		CMsg::F22050Hz,
		CMsg::F44100Hz,
		CMsg::F48000Hz,
		CMsg::F96000Hz,
		CMsg::End
	};
	CMsg::Id need_restart = CMsg::Null;
	if (next_sound_frequency != num) {
		need_restart = CMsg::LB_Need_restart_program_RB;
	}

	emu->out_infoc_x(CMsg::Frequency, CMsg::Colon_Space, list[num], need_restart, 0);

	pConfig->sound_frequency = num;
//	gui->emu->update_config();
}
int GUI_BASE::GetSoundFrequencyNum()
{
	return pConfig->sound_frequency;
}
void GUI_BASE::ChangeSoundLatency(int num)
{
	const CMsg::Id list[] = {
		CMsg::S50msec,
		CMsg::S75msec,
		CMsg::S100msec,
		CMsg::S200msec,
		CMsg::S300msec,
		CMsg::S400msec,
		CMsg::End
	};
	CMsg::Id need_restart = CMsg::Null;
	if (next_sound_latency != num) {
		need_restart = CMsg::LB_Need_restart_program_RB;
	}

	emu->out_infoc_x(CMsg::Latency, CMsg::Colon_Space, list[num], need_restart, 0);

	pConfig->sound_latency = num;
//	gui->emu->update_config();
}
int GUI_BASE::GetSoundLatencyNum()
{
	return pConfig->sound_latency;
}

#ifdef USE_DATAREC
// Data Recorder

void GUI_BASE::PostEtLoadDataRecMessage(const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_PLAY_DATAREC, path);
}
void GUI_BASE::PostEtLoadRecentDataRecMessage(int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_DATAREC, pConfig->GetRecentDataRecPathString(num));
}

void GUI_BASE::PostEtSaveDataRecMessage(const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_REC_DATAREC, path);
}
void GUI_BASE::PostEtCloseDataRecMessage(void)
{
	emumsg.Send(EMUMSG_ID_CLOSE_DATAREC);
}
void GUI_BASE::PostEtRewindDataRecMessage(void)
{
	emumsg.Send(EMUMSG_ID_REWIND_DATAREC);
}
void GUI_BASE::PostEtFastForwardDataRecMessage(void)
{
	emumsg.Send(EMUMSG_ID_FAST_FORWARD_DATAREC);
}
void GUI_BASE::PostEtStopDataRecMessage(void)
{
	emumsg.Send(EMUMSG_ID_STOP_DATAREC);
}
void GUI_BASE::PostEtToggleRealModeDataRecMessage(void)
{
	emumsg.Send(EMUMSG_ID_REAL_DATAREC);
}
bool GUI_BASE::NowRealModeDataRec(void)
{
	return pConfig->NowRealModeDataRec();
}
bool GUI_BASE::IsOpenedLoadDataRecFile(void)
{
	return emu->datarec_opened(true);
}
bool GUI_BASE::IsOpenedSaveDataRecFile(void)
{
	return emu->datarec_opened(false);
}
#endif

#ifdef USE_FD1
// Floppy disk

/// send message to emu thread in order to open floppy disk image.
void GUI_BASE::PostEtOpenFloppyMessage(int drv, const _TCHAR *file_path, int bank, uint32_t flags, bool multiopen)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_OPEN_FD, drv, path, bank, flags, multiopen);
}
/// send message to emu thread in order to open floppy disk image.
void GUI_BASE::PostEtOpenRecentFloppyMessage(int drv, int num)
{
	int new_drv = ShowSelectFloppyDriveDialog(drv);
	emumsg.Send(EMUMSG_ID_RECENT_FD, new_drv, pConfig->GetRecentFloppyDiskPathString(drv, num), pConfig->GetRecentFloppyDiskPathNumber(drv, num), 0, true);
}
/// send message to emu thread in order to open floppy disk image.
void GUI_BASE::PostEtOpenFloppySelectedVolume(int drv, int bank_num)
{
	emumsg.Send(EMUMSG_ID_SELECT_D88_BANK, drv, bank_num);
}
void GUI_BASE::PostEtCloseFloppyMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_CLOSE_FD, drv, 0);
}
void GUI_BASE::PostEtChangeSideFloppyDisk(int drv)
{
	emumsg.Send(EMUMSG_ID_CHANGE_FD, drv, 0);
}
int GUI_BASE::GetSideFloppyDisk(int drv)
{
	return emu->get_floppy_disk_side(drv);
}
void GUI_BASE::PostEtToggleWriteProtectFloppyDisk(int drv)
{
	emumsg.Send(EMUMSG_ID_WRITEPROTECT_FD, drv, 0);
}
bool GUI_BASE::InsertedFloppyDisk(int drv)
{
	return emu->floppy_disk_inserted(drv);
}
bool GUI_BASE::WriteProtectedFloppyDisk(int drv)
{
	return emu->floppy_disk_write_protected(drv);
}

bool GUI_BASE::OpenFloppyDisk(int drv, const _TCHAR* path, int bank_num, uint32_t flags, bool multiopen)
{
	return emu->open_floppy_disk_by_bank_num(drv, path, bank_num, flags, multiopen);
}
void GUI_BASE::CloseFloppyDisk(int drv)
{
	emu->close_floppy_disk(drv);
}
bool GUI_BASE::OpenFloppyDiskSelectedVolume(int drv, int bank_num)
{
	return emu->open_floppy_disk_with_sel_bank(drv, bank_num);
}

void GUI_BASE::GetMultiVolumeStr(int num, const _TCHAR *name, _TCHAR *str, size_t slen)
{
	UTILITY::stprintf(str, slen, _T("%2d: %s"), num + 1, name[0] != _T('\0') ? name : CMSG(no_label));
}

//d88_file_t *GUI_BASE::GetD88File(int drv)
D88File *GUI_BASE::GetD88File(int drv)
{
	return emu->get_d88_file(drv);
}
#endif

#ifdef USE_HD1
/// send message to emu thread in order to open hard disk image.
void GUI_BASE::PostEtOpenHardDiskMessage(int drv, const _TCHAR *file_path, uint32_t flags)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_OPEN_HD, drv, path, 0, flags, false);
}
/// send message to emu thread in order to open hard disk image.
void GUI_BASE::PostEtOpenRecentHardDiskMessage(int drv, int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_HD, drv, pConfig->GetRecentHardDiskPathString(drv, num), pConfig->GetRecentHardDiskPathNumber(drv, num), 0, false);
}
void GUI_BASE::PostEtCloseHardDiskMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_CLOSE_HD, drv, 0);
}
bool GUI_BASE::MountedHardDisk(int drv)
{
	return emu->hard_disk_mounted(drv);
}
void GUI_BASE::PostEtToggleWriteProtectHardDisk(int drv)
{
	emumsg.Send(EMUMSG_ID_WRITEPROTECT_HD, drv, 0);
}
bool GUI_BASE::WriteProtectedHardDisk(int drv)
{
	return emu->hard_disk_write_protected(drv);
}
int GUI_BASE::GetHardDiskDeviceType(int drv)
{
	return emu->get_hard_disk_device_type(drv);
}
void GUI_BASE::ChangeHardDiskDeviceType(int drv, int num)
{
	emu->change_hard_disk_device_type(drv, num);
}
int GUI_BASE::GetCurrentHardDiskDeviceType(int drv)
{
	return emu->get_current_hard_disk_device_type(drv);
}
#endif

#ifdef USE_CART1
// Cartridge

void GUI_BASE::PostEtOpenCartridgeMessage(int drv, const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_OPEN_CART, drv, path);
}
void GUI_BASE::PostEtOpenRecentCartridgeMessage(int drv, int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_CART, drv, pConfig->recent_datarec_path[drv][num]->path);
}
void GUI_BASE::PostEtCloseCartridgeMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_CLOSE_CART, drv, 0);
}
#endif

#ifdef USE_QD1
// Quick Disk

void GUI_BASE::PostEtOpenQuickDiskMessage(int drv, const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_OPEN_QD, drv, path);
}
void GUI_BASE::PostEtToggleWriteProtectQuickDisk(int drv)
{
	emumsg.Send(EMUMSG_ID_WRITEPROTECT_QD, drv, 0);
}
void GUI_BASE::PostEtOpenRecentQuickDiskMessage(int drv, int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_QD, drv, pConfig->recent_quickdisk_path[drv][num]->path);
}
void GUI_BASE::PostEtCloseQuickDiskMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_CLOSE_QD, drv, 0);
}
#endif

#ifdef USE_MEDIA
// Media

void GUI_BASE::PostEtOpenMediaMessage(const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_OPEN_MEDIA, path);
}
void GUI_BASE::PostEtOpenRecentMediaMessage(int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_MEDIA, pConfig->recent_media_path[num]->path);
}
void GUI_BASE::PostEtCloseMediaMessage()
{
	emumsg.Send(EMUMSG_ID_CLOSE_MEDIA);
}
#endif

#ifdef USE_BINARY_FILE1
// Binary File

void GUI_BASE::PostEtLoadBinaryMessage(int drv, const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_LOAD_BINARY, drv, path);
}
void GUI_BASE::PostEtSaveBinaryMessage(int drv, const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_SAVE_BINARY, drv, path);
}
void GUI_BASE::PostEtOpenRecentBinaryMessage(int drv, int num)
{
	emumsg.Send(EMUMSG_ID_RECENT_BINARY, drv, pConfig->recent_binary_path[drv][num]->path);
}
void GUI_BASE::PostEtCloseBinaryMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_CLOSE_BINARY, drv, 0);
}
#endif

#ifdef USE_PRINTER
// Printer

void GUI_BASE::PostEtSavePrinterMessage(int drv, const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_PRINTER_SAVE, drv, path);
}
int GUI_BASE::GetPrinterBufferSize(int drv)
{
	return emu->get_printer_buffer_size(drv);
}
void GUI_BASE::PostEtClearPrinterBufferMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_PRINTER_CLEAR, drv, 0);
}
void GUI_BASE::PostEtPrintPrinterMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_PRINTER_PRINT, drv, 0);
}
void GUI_BASE::PrintPrinter(int drv)
{
	if (emu) {
		emu->print_printer(drv);
	}
}
void GUI_BASE::PostEtEnablePrinterDirectMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_PRINTER_DIRECT, drv, 0);
}
void GUI_BASE::EnablePrinterDirect(int drv)
{
	if (emu) {
		emu->enable_printer_direct(drv);
	}
}
bool GUI_BASE::IsEnablePrinterDirect(int drv)
{
	return pConfig->printer_direct[drv];
}
void GUI_BASE::PostEtTogglePrinterOnlineMessage(int drv)
{
	emumsg.Send(EMUMSG_ID_PRINTER_ONLINE, drv, 0);
}
//void GUI_BASE::TogglePrinterOnline(int drv)
//{
//	if (emu) {
//		emu->toggle_printer_online(drv);
//	}
//}
bool GUI_BASE::IsOnlinePrinter(int drv)
{
	return pConfig->printer_online[drv];
}
#endif

// Comm

//void GUI_BASE::PostEtEnableCommServerMessage(int drv)
//{
//	emumsg.Send(EMUMSG_ID_COMM_SERVER, drv, 0);
//}
void GUI_BASE::ToggleEnableCommServer(int drv)
{
#ifdef MAX_COMM
	if(emu) {
		emu->enable_comm_server(drv);
	}
#endif
}
bool GUI_BASE::IsEnableCommServer(int drv)
{
#ifdef MAX_COMM
	return pConfig->comm_server[drv];
#else
	return false;
#endif
}
//void GUI_BASE::PostEtToggleConnectCommMessage(int drv, int num)
//{
//	emumsg.Send(EMUMSG_ID_COMM_CONNECT, drv, num);
//}
void GUI_BASE::ToggleConnectComm(int drv, int num)
{
#ifdef MAX_COMM
	if(emu) {
		emu->enable_comm_connect(drv, num);
	}
#endif
}
bool GUI_BASE::NowConnectingComm(int drv, int num)
{
#ifdef MAX_COMM
	if(emu) {
		return emu->now_comm_connecting(drv, num);
	} else {
		return false;
	}
#else
	return false;
#endif
}
void GUI_BASE::ToggleCommThroughMode(int drv)
{
#ifdef MAX_COMM
	if (drv <= 0 && drv < MAX_COMM) {
		pConfig->comm_through[drv] = !pConfig->comm_through[drv];
	}
#endif
}
bool GUI_BASE::NowCommThroughMode(int drv)
{
#ifdef MAX_COMM
	if (drv <= 0 && drv < MAX_COMM) {
		return pConfig->comm_through[drv];
	}
#endif
	return false;
}
void GUI_BASE::ToggleCommBinaryMode(int drv)
{
#ifdef MAX_COMM
	if (pConfig->comm_binary[drv]) {
		SendCommTelnetCommand(drv, 0x10);
	} else {
		SendCommTelnetCommand(drv, 0x00);
	}
#endif
}
bool GUI_BASE::NowCommBinaryMode(int drv)
{
#ifdef MAX_COMM
	return pConfig->comm_binary[drv];
#else
	return false;
#endif
}
void GUI_BASE::SendCommTelnetCommand(int drv, int num)
{
#ifdef MAX_COMM
	emu->send_comm_telnet_command(drv, num);
#endif
}

int GUI_BASE::EnumUarts()
{
#ifdef USE_UART
	return emu->enum_uarts();
#else
	return 0;
#endif
}
void GUI_BASE::GetUartDescription(int ch, _TCHAR *buf, size_t size)
{
#ifdef USE_UART
	emu->get_uart_description(ch, buf, size);
#endif
}

// MIDI

void GUI_BASE::ConnectMidiIn(int idx)
{
#ifdef USE_MIDI
	if (idx < 0) {
		emu->close_midiin();
	} else {
		if (!emu->now_midiin_connecting(idx)) {
			emu->close_midiin();
			emu->open_midiin(idx);
		}
	}
#endif
}
void GUI_BASE::ToggleConnectMidiIn(int idx)
{
#ifdef USE_MIDI
	if (emu->now_midiin_connecting(idx)) {
		// close if alreay connected
		emu->close_midiin();
	} else {
		// connect
		emu->close_midiin();
		emu->open_midiin(idx);
	}
#endif
}
bool GUI_BASE::NowConnectingMidiIn(int idx)
{
#ifdef USE_MIDI
	return emu->now_midiin_connecting(idx);
#else
	return false;
#endif
}

void GUI_BASE::ConnectMidiOut(int idx)
{
#ifdef USE_MIDI
	if (idx < 0) {
		emu->close_midiout();
	} else {
		if (!emu->now_midiout_connecting(idx)) {
			emu->close_midiout();
			emu->open_midiout(idx);
		}
	}
#endif
}
void GUI_BASE::ToggleConnectMidiOut(int idx)
{
#ifdef USE_MIDI
	if (emu->now_midiout_connecting(idx)) {
		// close if alreay connected
		emu->close_midiout();
	} else {
		// connect
		emu->close_midiout();
		emu->open_midiout(idx);
	}
#endif
}
bool GUI_BASE::NowConnectingMidiOut(int idx)
{
#ifdef USE_MIDI
	return emu->now_midiout_connecting(idx);
#else
	return false;
#endif
}

int GUI_BASE::EnumMidiIns()
{
#ifdef USE_MIDI
	return emu->enum_midiins();
#else
	return 0;
#endif
}
void GUI_BASE::GetMidiInDescription(int idx, _TCHAR *buf, size_t size)
{
#ifdef USE_MIDI
	emu->get_midiin_description(idx, buf, size);
#endif
}

int GUI_BASE::EnumMidiOuts()
{
#ifdef USE_MIDI
	return emu->enum_midiouts();
#else
	return 0;
#endif
}
void GUI_BASE::GetMidiOutDescription(int idx, _TCHAR *buf, size_t size)
{
#ifdef USE_MIDI
	emu->get_midiout_description(idx, buf, size);
#endif
}

void GUI_BASE::ChangeMidiOutLatencyByNum(int num)
{
#ifdef USE_MIDI
	if (0 <= num && num <= 5) {
		pConfig->midiout_delay = (25 << num);
		emu->set_midiout_delay_time(pConfig->midiout_delay);
	}
#endif
}
int GUI_BASE::GetMidiOutLatencyNum() const
{
#ifdef USE_MIDI
	int match = -1;
	for(int i=0; i<6; i++) {
		if (pConfig->midiout_delay == (25 << i)) {
			match = i;
			break;
		}
	}
	return match;
#endif
}

void GUI_BASE::SendMidiResetMessage(int num)
{
#ifdef USE_MIDI
	if (MIDI_TYPE_GM <= num && num <= MIDI_TYPE_XG) {
		pConfig->midi_send_reset_type = num;
	}
	emu->send_midi_reset_message();
#endif
}

// Status VM

void GUI_BASE::PostEtLoadStatusMessage(const _TCHAR *file_path)
{
#ifdef USE_STATE
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_LOAD_STATE, path);
#endif
}
void GUI_BASE::PostEtLoadRecentStatusMessage(int num)
{
#ifdef USE_STATE
	emumsg.Send(EMUMSG_ID_RECENT_STATE, pConfig->GetRecentStatePathString(num));
#endif
}
void GUI_BASE::PostEtSaveStatusMessage(const _TCHAR *file_path, bool sys_pause)
{
#ifdef USE_STATE
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_SAVE_STATE, path, sys_pause);
	pConfig->SetInitialStatePathFrom(path);
#endif
}

// Auto key

void GUI_BASE::PostEtLoadAutoKeyMessage(const _TCHAR *file_path)
{
#ifdef USE_AUTO_KEY
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_AUTOKEY_OPEN, path);
#endif
}
void GUI_BASE::PostEtStartAutoKeyMessage(void)
{
#ifdef USE_AUTO_KEY
	emumsg.Send(EMUMSG_ID_AUTOKEY_START);
#endif
}
void GUI_BASE::PostEtStopAutoKeyMessage(void)
{
#ifdef USE_AUTO_KEY
	emumsg.Send(EMUMSG_ID_AUTOKEY_STOP);
#endif
}
bool GUI_BASE::IsRunningAutoKey(void)
{
#ifdef USE_AUTO_KEY
	return emu->now_auto_key();
#else
	return false;
#endif
}
/// @attention called by emu thread
bool GUI_BASE::StartAutoKey(void)
{
	return false;
}

// Record key

void GUI_BASE::PostEtLoadRecKeyMessage(const _TCHAR *file_path)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_RECKEY_PLAY, path);
}

void GUI_BASE::PostEtSaveRecKeyMessage(const _TCHAR *file_path, bool sys_pause)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::get_long_full_path_name(file_path, path);
	emumsg.Send(EMUMSG_ID_RECKEY_REC, path, sys_pause);
}
void GUI_BASE::StopPlayRecKey(void)
{
#ifdef USE_KEY_RECORD
	emu->stop_reckey(true, false);
#endif
}
void GUI_BASE::StopRecordRecKey(void)
{
#ifdef USE_KEY_RECORD
	emu->stop_reckey(false, true);
#endif
}
bool GUI_BASE::NowPlayingRecKey(void)
{
	return pConfig->reckey_playing;
}
bool GUI_BASE::NowRecordingRecKey(void)
{
	return pConfig->reckey_recording;
}


// Misc

void GUI_BASE::ChangeLedBox(int phase)
{
#ifdef USE_LEDBOX
	if (!ledbox) return;

	switch(phase) {
	case 2:
#ifdef USE_OUTSIDE_LEDBOX
		pConfig->misc_flags |= MSK_SHOWLEDBOX;
		pConfig->misc_flags &= ~MSK_INSIDELEDBOX;
		break;
#endif
	case 1:
		pConfig->misc_flags |= MSK_SHOWLEDBOX;
		pConfig->misc_flags |= MSK_INSIDELEDBOX;
		break;
	default: // hide
		pConfig->misc_flags &= ~MSK_SHOWLEDBOX;
		pConfig->misc_flags &= ~MSK_INSIDELEDBOX;
		break;
	}

	ledbox->Show(FLG_LEDBOX_ALL);
#endif
}

/// ShowInside->ShowOutside->Hide Ledbox
void GUI_BASE::ToggleLedBox(void)
{
#ifdef USE_LEDBOX
	if (!ledbox) return;

	switch(GetLedBoxPhase(-1)) {
	case 1:
#ifdef USE_OUTSIDE_LEDBOX
		pConfig->misc_flags |= MSK_SHOWLEDBOX;
		pConfig->misc_flags &= ~MSK_INSIDELEDBOX;
		break;
#endif
	case 2:
		pConfig->misc_flags &= ~MSK_SHOWLEDBOX;
		pConfig->misc_flags |= MSK_INSIDELEDBOX;
		break;
	case 0:
		pConfig->misc_flags |= MSK_SHOWLEDBOX;
		pConfig->misc_flags |= MSK_INSIDELEDBOX;
		break;
	default:
		break;
	}

	ledbox->Show(FLG_LEDBOX_ALL);

	switch(GetLedBoxPhase(-1)) {
	case 1:
		emu->out_info_x(CMsg::Show_LED_Inside);
		break;
	case 2:
		emu->out_info_x(CMsg::Show_LED_Outside);
		break;
	case 0:
		emu->out_info_x(CMsg::Hide_LED);
		break;
	default:
		logging->out_log_x(LOG_WARN, CMsg::LED_is_disable);
		break;
	}
#endif
}
/// Show/Hide Ledbox
void GUI_BASE::ToggleShowLedBox(void)
{
#ifdef USE_LEDBOX
	if (!ledbox) return;
//	emu->show_led();
	pConfig->misc_flags ^= MSK_SHOWLEDBOX;
	ledbox->Show(FLG_LEDBOX_ALL);

	switch(GetLedBoxPhase(0)) {
	case 1:
		emu->out_infof_x(CMsg::Show_LED);
		break;
	case 0:
		emu->out_infof_x(CMsg::Hide_LED);
		break;
	default:
		logging->out_log_x(LOG_WARN, CMsg::LED_is_disable);
		break;
	}
#endif
}
/// Inside/Outside Ledbox
void GUI_BASE::ToggleInsideLedBox(void)
{
#ifdef USE_LEDBOX
	if (!ledbox) return;
//	emu->inside_led();
	pConfig->misc_flags ^= MSK_INSIDELEDBOX;
	ledbox->Show(FLG_LEDBOX_ALL);

	switch(GetLedBoxPhase(1)) {
	case 1:
		emu->out_infof_x(CMsg::Inside_LED);
		break;
	case 0:
		emu->out_infof_x(CMsg::Outside_LED);
		break;
	default:
		logging->out_log_x(LOG_WARN, CMsg::LED_is_disable);
		break;
	}
#endif
}
/// @param[in] id
/// @return -1:ledbox is disable 0:false 1:true
int GUI_BASE::GetLedBoxPhase(int id)
{
#ifdef USE_LEDBOX
	// emu->is_shown_ledbox(id);
	if (ledbox && ledbox->IsEnable()) {
		switch(id) {
			case 0:
				return FLG_SHOWLEDBOX ? 1 : 0;
			case 1:
				return FLG_INSIDELEDBOX ? 1 : 0;
			default:
				return FLG_SHOWLEDBOX ? (FLG_INSIDELEDBOX ? 1 : 2) : 0;
		}
	}
#endif
	return -1;
}
bool GUI_BASE::IsShownLedBox(void)
{
#ifdef USE_LEDBOX
	return (GetLedBoxPhase(0) > 0 ? true : false);
#else
	return false;
#endif
}
bool GUI_BASE::IsInsidedLedBox(void)
{
#ifdef USE_LEDBOX
	return (GetLedBoxPhase(1) > 0 ? true : false);
#else
	return false;
#endif
}
void GUI_BASE::ChangeLedBoxPosition(int num)
{
#ifdef USE_LEDBOX
	if (!ledbox) return;
//	emu->change_pos_led(num);
	if (num == -1) {
		pConfig->led_pos = (pConfig->led_pos + 1) % 4;
	} else {
		pConfig->led_pos = num;
	}
	ledbox->SetPos(pConfig->led_pos | (IsFullScreen() ? 0x10 : 0));
#endif
}

/// Show/Hide MessageBoard
void GUI_BASE::ToggleMessageBoard(void)
{
	emu->show_message_board();
	switch(emu->is_shown_message_board()) {
	case 1:
		emu->out_infof_x(CMsg::Show_Message);
		break;
	case 0:
		emu->out_infof_x(CMsg::Hide_Message);
		break;
	default:
		logging->out_log_x(LOG_WARN, CMsg::Message_board_is_disable);
		break;
	}
}
bool GUI_BASE::IsShownMessageBoard(void)
{
	return (emu->is_shown_message_board() > 0 ? true : false);
}

bool GUI_BASE::IsShownLoggingDialog(void)
{
	return false;
}

#ifdef USE_PERFORMANCE_METER
void GUI_BASE::TogglePMeter(void)
{
	pConfig->show_pmeter = (pConfig->show_pmeter ? false : true);
}
bool GUI_BASE::IsShownPMeter(void)
{
	return pConfig->show_pmeter;
}
#endif

// Input device

/// Change UseJoypad
void GUI_BASE::ChangeUseJoypad(int num)
{
	if (num > 0 && emu->is_enable_joypad(num)) num = 0;
	emu->change_use_joypad(num);
	if (emu->is_enable_joypad(1)) {
		emu->out_info_x(CMsg::Enable_Joypad_to_Key);
	} else if (emu->is_enable_joypad(2)) {
		emu->out_info_x(CMsg::Enable_Joypad);
	} else {
		emu->out_info_x(CMsg::Disable_Joypad);
	}
}
bool GUI_BASE::IsEnableJoypad(int num)
{
	return emu->is_enable_joypad(num);
}

#ifdef USE_KEY2JOYSTICK
void GUI_BASE::ToggleEnableKey2Joypad(void)
{
	emu->toggle_enable_key2joy();
}
bool GUI_BASE::IsEnableKey2Joypad(void)
{
	return emu->is_enable_key2joy();
}
#endif

#ifdef USE_LIGHTPEN
/// Change EnableLightpen
void GUI_BASE::ToggleEnableLightpen(void)
{
	pConfig->misc_flags ^= MSK_USELIGHTPEN;
	if (FLG_USELIGHTPEN) {
		emu->out_info_x(CMsg::Enable_Lightpen);
	} else {
		emu->out_info_x(CMsg::Disable_Lightpen);
	}
}
bool GUI_BASE::IsEnableLightpen(void)
{
	return FLG_USELIGHTPEN ? true : false;
}
#endif
#ifdef USE_MOUSE
/// Change UseMouse
void GUI_BASE::ToggleUseMouse(void)
{
	emu->toggle_mouse();
	pConfig->misc_flags = (emu->get_mouse_enabled() ? (pConfig->misc_flags | MSK_USEMOUSE) : (pConfig->misc_flags & ~MSK_USEMOUSE));
	if (FLG_USEMOUSE) {
		emu->out_info_x(CMsg::Enable_Mouse);
	} else {
		emu->out_info_x(CMsg::Disable_Mouse);
	}
}
bool GUI_BASE::IsEnableMouse(void)
{
	return emu->get_mouse_enabled();
}
/// post enable/disable mouse message to main thread
void GUI_BASE::PostMtToggleUseMouse(void)
{
	PostCommandMessage(ID_OPTIONS_MOUSE);
}
/// post enable/disable mouse message to main thread
void GUI_BASE::PostMtEnableMouseTemp(bool val)
{
	PostCommandMessage(val ? ID_ENABLE_MOUSE_TEMP : ID_DISABLE_MOUSE_TEMP);
}
#endif
#ifdef USE_DIRECTINPUT
/// Use Direct Input
void GUI_BASE::ToggleUseDirectInput(void)
{
	pConfig->use_direct_input = (pConfig->use_direct_input ^ 1);
	if (pConfig->use_direct_input & 1) {
		emu->out_info_x(CMsg::Enable_DirectInput);
	} else {
		emu->out_info_x(CMsg::Disable_DirectInput);
	}
}
bool GUI_BASE::NowUseDirectInput(void)
{
	return ((pConfig->use_direct_input & 1) != 0);
}
bool GUI_BASE::IsEnableDirectInput(void)
{
	return ((pConfig->use_direct_input & 4) != 0);
}
#endif
///
void GUI_BASE::ToggleLoosenKeyStroke(void)
{
#if defined(_BML3MK5) || defined(_MBS1)
	pConfig->original = (pConfig->original ^ MSK_ORIG_LIMKEY);
#endif
	emu->update_config();
}
bool GUI_BASE::IsLoosenKeyStroke(void)
{
#if defined(_BML3MK5) || defined(_MBS1)
	return ((pConfig->original & MSK_ORIG_LIMKEY) != 0);
#else
	return false;
#endif
}
/// Processing Recent File Name String
bool GUI_BASE::GetRecentFileStr(const _TCHAR *file, int num, _TCHAR *str, int trimlen)
{
	if (file == NULL || file[0] == '\0') return false;

#if defined(USE_UTF8_ON_MBCS)
	// convert UTF-8 to MBCS
	UTILITY::conv_to_native_path(file, str, _MAX_PATH);
#else
	UTILITY::tcscpy(str, _MAX_PATH, file);
#endif
	UTILITY::tcscpy(str, _MAX_PATH, UTILITY::trim_center(str, trimlen));
	if (num > 0) {
		size_t len = _tcslen(str);
		UTILITY::stprintf(&str[len], _MAX_PATH - len, _T(" : %d"), num + 1);
	}
	return true;
}

// Drop

bool GUI_BASE::OpenDroppedFile(void *param)
{
#if defined(USE_SDL2)
	_TCHAR *dropped_file = (_TCHAR *)param;
	if (!dropped_file || _tcslen(dropped_file) == 0) {
		return false;
	}
	return OpenFileByExtention(dropped_file);
#elif defined(USE_WX) || defined(USE_WX2)
	return OpenFileByExtention((const _TCHAR *)param);
#else
	return false;
#endif
}

#ifdef USE_DEBUGGER

void GUI_BASE::OpenDebugger()
{
	emu->open_debugger();
}

void GUI_BASE::PostMtOpenDebugger()
{
	PostCommandMessage(ID_OPEN_DEBUGGER0);
}

void GUI_BASE::CloseDebugger()
{
	emu->close_debugger();
}

void GUI_BASE::PostMtCloseDebugger()
{
	PostCommandMessage(ID_CLOSE_DEBUGGER);
}

bool GUI_BASE::IsDebuggerOpened()
{
	return emu->now_debugging;
}

#endif

bool GUI_BASE::OpenFileByExtention(const _TCHAR *file_path)
{
	int rc = CheckSupportedFile(file_path);
	switch(rc) {
		case FILE_TYPE_DATAREC:
#ifdef USE_DATAREC
			// maybe tape image file
			PostEtLoadDataRecMessage(file_path);
#endif
			break;
		case FILE_TYPE_FLOPPY:
#ifdef USE_FD1
			// maybe disk image file
			PostEtOpenFloppyMessage(0, file_path, 0, 0, true);
#endif
			break;
		case FILE_TYPE_STATE:
			// maybe state file
			PostEtLoadStatusMessage(file_path);
			break;
		case FILE_TYPE_AUTO_KEY:
			// maybe text file (for autokey)
			PostEtLoadAutoKeyMessage(file_path);
			break;
		case FILE_TYPE_KEY_RECORD:
			// maybe record key file
			PostEtLoadRecKeyMessage(file_path);
			break;
		case FILE_TYPE_SASI_HARD_DISK:
#ifdef USE_HD1
			// maybe hard disk image file
			PostEtOpenHardDiskMessage(0, file_path, 0);
#endif
			break;
		case FILE_TYPE_SCSI_HARD_DISK:
#ifdef USE_HD17
			// maybe hard disk image file
			PostEtOpenHardDiskMessage(16, file_path, 0);
#endif
			break;
		default:
			return false;
	}
	return true;
}

/**
 *	check supported file
 */
int GUI_BASE::CheckSupportedFile(const _TCHAR *file_path)
{
	bool rc;

	rc = UTILITY::check_file_extensions(file_path, LABELS::datarec_exts);
//		, _T(".l3"), _T(".l3b"), _T(".l3c"), _T(".wav"), _T(".t9x"), NULL);
	if (rc) {
		// tape image
		return FILE_TYPE_DATAREC;
	}

#ifdef USE_FD1
	rc = UTILITY::check_file_extensions(file_path, LABELS::floppy_disk_exts);
//		, _T(".d88"), _T(".d68"), _T(".d77"), _T(".td0"), _T(".imd"), _T(".img"), _T(".dsk"), _T(".fdi"), _T(".hdm"), _T(".tfd"), _T(".xdf"), _T(".2d"), _T(".2hd"), _T(".sf7"), NULL);
	if (rc) {
		// disk image
		return FILE_TYPE_FLOPPY;
	}
#endif

	rc = UTILITY::check_file_extensions(file_path, LABELS::state_file_exts);
//		, _T(".l3r"), NULL);
	if (rc) {
		// maybe state file
		return FILE_TYPE_STATE;
	}

	rc = UTILITY::check_file_extensions(file_path, LABELS::autokey_file_exts);
//		, _T(".txt"), _T(".bas"), _T(".lpt"), NULL);
	if (rc) {
		// maybe text file (for autokey)
		return FILE_TYPE_AUTO_KEY;
	}

	rc = UTILITY::check_file_extension(file_path, _T(".ini"));
	if (rc) {
		// maybe ini file
		return FILE_TYPE_INITIALIZE;
	}

	rc = UTILITY::check_file_extensions(file_path, LABELS::key_rec_file_exts);
	if (rc) {
		// maybe record key file
		return FILE_TYPE_KEY_RECORD;
	}

#ifdef USE_HD1
	rc = UTILITY::check_file_extensions(file_path, LABELS::sasi_hard_disk_exts);
	if (rc) {
		// hard disk image
		return FILE_TYPE_SASI_HARD_DISK;
	}
#endif

#ifdef USE_HD17
	rc = UTILITY::check_file_extensions(file_path, LABELS::scsi_hard_disk_exts);
	if (rc) {
		// hard disk image
		return FILE_TYPE_SCSI_HARD_DISK;
	}
#endif

	return 0;
}

//
// ledbox
//

// ---------------------------------------------------------
#if defined(USE_WIN)
LedBox *GUI_BASE::CreateLedBox()
{
#ifdef USE_LEDBOX
	ledbox = new LedBox();

	if (ledbox) {
		CreateLedBoxSub();
//		HDC hdc = ::GetDC(hWindow);
		if (!ledbox->InitScreen()) {
			delete ledbox;
			ledbox = NULL;
		}
		if (ledbox) {
			ledbox->SetDistance(pConfig->led_pos, pConfig->led_dist);
			ledbox->CreateDialogBox();
			ledbox->Show(FLG_LEDBOX_ALL);
		}
//		::ReleaseDC(hWindow, hdc);
	}
	return ledbox;
#else
	return NULL;
#endif
}
// -------------------------------------
#elif defined(USE_SDL) || defined(USE_SDL2)
LedBox *GUI_BASE::CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format)
{
#ifdef USE_LEDBOX
	ledbox = new LedBox();

	if (ledbox) {
		CreateLedBoxSub();
		CPixelFormat pixel_format = (*src_format);
#if defined(__MACH__) && defined(__APPLE__)
		pixel_format.PresetRGBA();
#else
		pixel_format.PresetBGRA();
#endif
		if (!ledbox->InitScreen(res_path, pixel_format)) {
			logging->out_logf(LOG_WARN, _T("LedBox: Cannot load images from %s."), res_path);
			delete ledbox;
			ledbox = NULL;
		}
	}
	if (ledbox) {
		ledbox->SetDistance(pConfig->led_pos, pConfig->led_dist);
		ledbox->CreateDialogBox();
		ledbox->Show(FLG_LEDBOX_ALL);
	}
	return ledbox;
#else
	return NULL;
#endif
}
// -------------------------------------
#elif defined(USE_QT)
LedBox *GUI_BASE::CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format)
{
#ifdef USE_LEDBOX
	ledbox = new LedBox();

	if (ledbox) {
		CreateLedBoxSub();
		CPixelFormat pixel_format = (*src_format);
#if defined(__MACH__) && defined(__APPLE__)
        pixel_format.PresetBGRA();
#else
		pixel_format.PresetBGRA();
#endif
		if (!ledbox->InitScreen(res_path, pixel_format)) {
			logging->out_logf(LOG_WARN, _T("LedBox: Cannot load images from %s."), res_path);
			delete ledbox;
			ledbox = NULL;
		}
	}
	if (ledbox) {
		ledbox->SetDistance(pConfig->led_pos, pConfig->led_dist);
		ledbox->CreateDialogBox();
		ledbox->Show(FLG_LEDBOX_ALL);
	}
	return ledbox;
#else
	return NULL;
#endif
}
// -------------------------------------
#elif defined(USE_WX) || defined(USE_WX2)
LedBox *GUI_BASE::CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format)
{
#ifdef USE_LEDBOX
	ledbox = new LedBox();

	if (ledbox) {
		CreateLedBoxSub();
		CPixelFormat pixel_format = (*src_format);
#if defined(__WXGTK__)
		pixel_format.PresetRGBA();
#endif
		if (!ledbox->InitScreen(res_path, &pixel_format)) {
			logging->out_logf(LOG_WARN, _T("LedBox: Cannot load images from %s."), res_path);
			delete ledbox;
			ledbox = NULL;
		}
	}
	if (ledbox) {
		ledbox->SetDistance(pConfig->led_pos, pConfig->led_dist);
		ledbox->CreateDialogBox();
		ledbox->Show(FLG_LEDBOX_ALL);
	}
	return ledbox;
#else
	return NULL;
#endif
}
#endif
// ---------------------------------------------------------

void GUI_BASE::CreateLedBoxSub()
{
#ifdef USE_LEDBOX
#if defined(USE_WIN)
	ledbox->SetHandle(::GetModuleHandle(NULL), hWindow);
#elif defined(USE_SDL) || defined(USE_SDL2)
# if defined(USE_SDL2) && defined(USE_SDL2_LEDBOX)
	ledbox->SetParentWindow(emu->get_window());
# elif defined(_WIN32)
	ledbox->SetHandle(::GetModuleHandle(NULL), hWindow);
# endif
#endif
#endif
}

void GUI_BASE::ReleaseLedBox()
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->GetDistance(pConfig->led_dist);
	}
	delete ledbox;
	ledbox = NULL;
#endif
}

void GUI_BASE::SetLedBoxPosition(bool mode, int left, int top, int width, int height, int place)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->SetMode(mode ? 0 : 1);
		ledbox->SetPos(left, top, left + width, top + height, place);
		ledbox->GetDistance(pConfig->led_dist);
	}
#endif
}

void GUI_BASE::UpdateIndicator(uint64_t flag)
{
#ifdef USE_LEDBOX
	if (ledbox && FLG_SHOWLEDBOX) {
		ledbox->Update(flag);
	}
#endif
	UpdateVirtualKeyboard((uint32_t)flag);
}

// ---------------------------------------------------------
#if 0
#if defined(USE_WIN)
void GUI_BASE::DrawLedBox(HDC hdc)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(hdc);
	}
#endif
}

void GUI_BASE::DrawLedBox(LPDIRECT3DSURFACE9 suf)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(suf);
	}
#endif
}
// -------------------------------------
#elif defined(USE_SDL) || defined(USE_SDL2)
void GUI_BASE::DrawLedBox(CSurface *screen)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(*screen);
	}
#endif
}
#if defined(USE_SDL2)
void GUI_BASE::DrawLedBox(CTexture &texture)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(texture);
	}
#endif
}
void GUI_BASE::DrawLedBox(COpenGLTexture &texture)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(texture);
	}
#endif
}
#endif
// -------------------------------------
#elif defined(USE_QT)
void GUI_BASE::DrawLedBox(QImage *screen)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(screen);
	}
#endif
}
// -------------------------------------
#elif defined(USE_WX) || defined(USE_WX2)
void GUI_BASE::DrawLedBox(CSurface *screen)
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Draw(*screen);
	}
#endif
}
#endif
#endif
// ---------------------------------------------------------

void GUI_BASE::MoveLedBox()
{
#ifdef USE_LEDBOX
	if (ledbox) {
		ledbox->Move();
	}
#endif
}

#if defined(USE_QT)
//
MyUserEvent::MyUserEvent(Type type) :
	QEvent(type)
{
	code = 0;
	data1 = NULL;
	data2 = NULL;
}

MyUserEvent::~MyUserEvent()
{

}
#endif
