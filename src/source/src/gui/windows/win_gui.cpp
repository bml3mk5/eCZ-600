/** @file win_gui.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2014.03.24 -

	@brief [ gui for windows ]
*/

#include <Windows.h>
#include "win_gui.h"
#include "../../config.h"
#include "winfont.h"
#include "../../emu.h"
#include "../../emumsg.h"
#include "win_filebox.h"
#include "win_seldrvbox.h"
#include "win_joysetbox.h"
#include "../../depend.h"
#include "../../labels.h"
#include "../../utility.h"
#include "../../main.h"
#ifdef USE_VKEYBOARD
#include "win_vkeyboard.h"
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_syswm.h>
#endif

#ifdef GUI_TYPE_WINDOWS

using namespace GUI_WIN;

#define TRIM_STRING_SIZE	72

void GUI::update_recent_menu(HMENU hMenu, int menu_id, CRecentPathList &list)
{
	bool flag = false;
	HMENU hSubMenu = get_sub_menu(hMenu, menu_id);
	_TCHAR str[_MAX_PATH];
	if (hSubMenu) {
		while(DeleteMenu(hSubMenu, 0, MF_BYPOSITION) != 0) {}
		for(int i = 0; i < MAX_HISTORY && i < list.Count(); i++) {
			if (!GetRecentFileStr(list[i]->path, list[i]->num, str, TRIM_STRING_SIZE)) break;
			AppendMenu(hSubMenu, MF_STRING, menu_id + i, str);
			flag = true;
		}
		if(!flag) {
			AppendMenu(hSubMenu, MF_GRAYED | MF_STRING, menu_id, CMSGM(None_));
		}
	}
}

void GUI::update_multivolume_menu(HMENU hMenu, int drv, int menu_id)
{
#ifdef USE_FD1
	bool flag = false;
	HMENU hSubMenu = get_sub_menu(hMenu, menu_id);
	_TCHAR str[32];
	if (hSubMenu) {
		while(DeleteMenu(hSubMenu, 0, MF_BYPOSITION) != 0) {}
		D88File *d88_file = GetD88File(drv);
		int bank_nums = d88_file->GetBanks().Count();
		if(bank_nums >= 1) {
			for(int i = 0; i < bank_nums; i++) {
				GetMultiVolumeStr(i, d88_file->GetBank(i)->GetName(), str, 32);
				AppendMenu(hSubMenu
					, MF_STRING | (bank_nums <= 1 ? MF_GRAYED : 0) | (i == d88_file->GetCurrentBank() ? MF_CHECKED : 0)
					, menu_id + i, str);
			}
			flag = true;
		}
		if(!flag) {
			AppendMenu(hSubMenu, MF_GRAYED | MF_STRING, menu_id, CMSGM(None_));
		}
	}
#endif
}

HMENU GUI::get_sub_menu(HMENU hMenu, int id)
{
	HMENU hSubMenu = NULL;
	int count = GetMenuItemCount(hMenu);
	for(int n=count-1; n>=0; n--) {
		hSubMenu = GetSubMenu(hMenu,n);
		if (!hSubMenu) continue;

		MENUITEMINFO mii;
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_ID;
		if (!GetMenuItemInfo(hSubMenu, id, FALSE, &mii)) {
			hSubMenu = NULL;
			continue;
		}
		break;
	}
	return hSubMenu;
}

/// @note recursive called by itself
HMENU GUI::get_sub_menu_new(HMENU hMenu, int id)
{
	HMENU hMatched = NULL;
	int count = GetMenuItemCount(hMenu);
	for(int n=0; n<count; n++) {
		HMENU hSubMenu = GetSubMenu(hMenu, n);
		if (hSubMenu) {
			hMatched = get_sub_menu_new(hSubMenu, id);
			if (hMatched) {
				break;
			}
		} else {
			MENUITEMINFO mii;
			memset(&mii, 0, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_FTYPE | MIIM_ID;
			if (GetMenuItemInfo(hMenu, n, TRUE, &mii)) {
				if (id == (int)mii.wID && (mii.fType & MFT_SEPARATOR) == 0) {
					hMatched = hMenu;
					break;
				}
			}
		}
	}
	return hMatched;
}

#if 0
#if defined(USE_SDL) || defined(USE_SDL2)
/// for locale
#define MSG_PTR_MAX 32
static char *msg_buf_ptr[MSG_PTR_MAX];
static int cur_msg_buf_ptr;
#endif
#endif

/****************************************

 GUI for windows class

 ****************************************/

GUI::GUI(int argc, char **argv, EMU *new_emu) : GUI_BASE(argc, argv, new_emu)
{
	hInstance = ::GetModuleHandle(NULL);

	font = new CFont();
	font->SetDefaultFont();

//	configbox = new ConfigBox(hInstance, font, emu, this);
//	keybindbox = new KeybindBox(hInstance, font, emu, this);
//	volumebox = new VolumeBox(hInstance, font, emu, this);
#ifdef USE_REC_VIDEO
	recvidbox = new RecVideoBox(hInstance, font, emu, this);
#endif
#ifdef USE_REC_AUDIO
	recaudbox = new RecAudioBox(hInstance, font, emu, this);
#endif
//	aboutbox = new AboutBox(hInstance, font, emu, this);

	KeybindControl::RegisterClass(hInstance);

	now_menu = false;
	now_menuloop = false;

#if defined(USE_SDL) || defined(USE_SDL2)
//	memset(msg_buf_ptr, 0, sizeof(msg_buf_ptr));
//	cur_msg_buf_ptr = 0;

	OrigProcPtr = NULL;
#endif
}

GUI::~GUI()
{
#if defined(USE_SDL) || defined(USE_SDL2)
	if (OrigProcPtr) {
		SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)OrigProcPtr);
	}

//	for(int i=0; i<MSG_PTR_MAX; i++) {
//		if (msg_buf_ptr[i]) delete [] msg_buf_ptr[i];
//	}
#endif

//	delete aboutbox;
#ifdef USE_REC_AUDIO
	delete recaudbox;
#endif
#ifdef USE_REC_VIDEO
	delete recvidbox;
#endif
//	delete volumebox;
//	delete keybindbox;
//	delete configbox;
	delete font;

	KeybindControl::UnregisterClass(hInstance);
}

#if defined(USE_SDL)
int GUI::CreateWidget(SDL_Surface *screen, int width, int height)
{
	GUI_BASE::CreateWidget(screen, width, height);
	// enable to accept drag and drops
	::DragAcceptFiles(hWindow, TRUE);
	return 0;
}
#endif

int GUI::CreateMenu()
{
#if defined(USE_SDL) || defined(USE_SDL2)
	GUI_BASE::CreateMenu();

	// Hook default window message procedure
	if (OrigProcPtr == NULL) {
		OrigProcPtr = (WNDPROC)GetWindowLongPtr(hWindow, GWLP_WNDPROC);
		SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG_PTR)this);
		SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)MessageProcStatic);
	}
	// disable system specific event because system event process on MessageProc
	SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);
	// enable menu
	ShowMenu();
	// need resize window
	return 1;
#else
	ShowMenu();
	return 0;
#endif
}

void GUI::ShowMenu()
{
#if defined(USE_SDL) || defined(USE_SDL2)
	if(hWindow && !now_menu) {
#else
	if(!now_menu) {
#endif
		HMENU hMenu = LoadMenu((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1));
		translate_menu(hMenu, 0);
		SetMenu(hWindow, hMenu);
		now_menu = true;
	}
}

void GUI::HideMenu()
{
#if defined(USE_SDL) || defined(USE_SDL2)
	if(hWindow && now_menu) {
#else
	if(now_menu) {
#endif
		HMENU hMenu = GetMenu(hWindow);
		SetMenu(hWindow, NULL);
		DestroyMenu(hMenu);
		now_menu = false;
	}
}

#if defined(USE_SDL) || defined(USE_SDL2)
LRESULT CALLBACK GUI::MessageProcStatic(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	GUI *gui = (GUI *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (gui) return gui->MessageProc(hWnd, iMsg, wParam, lParam);
	else return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

/// post update screen request message
/// @attention this function is called by emu thread
/// @return false:did not call UpdateScreen in main thread
bool GUI::NeedUpdateScreen()
{
//	logging->out_debugf(_T("NeedUpdateScreen: %d"), need_update_screen);
	if (need_update_screen > 0) {
		need_update_screen = 0;
//		PostMessage(hWindow, WM_USER_PAINT, 0, 0);
		UINT flags = RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_NOERASE;
		RedrawWindow(hWindow, NULL, NULL, flags);
		// send signal to main loop
		g_need_update_screen = true;
		SDL_CondSignal(g_cond_allow_update_screen);
		return true;
	} else {
		return false;
	}
}

void GUI::PreProcessEvent()
{
	// nothing to do
}
#endif

#if defined(USE_WIN)
LRESULT GUI::ProcessEvent(UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = GUI_BASE::ProcessEvent(iMsg, wParam, lParam);
	if (result != 1) return result;

#elif defined(USE_SDL) || defined(USE_SDL2)
LRESULT GUI::MessageProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
#endif
	switch(iMsg) {
#if defined(USE_SDL) || defined(USE_SDL2)
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
//		logging->out_debugf("WM_PAINT: %lx", hWnd);
		UpdateScreen();
		EndPaint(hWnd, &ps);
		return 0;
   }
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		// convert to SDL user message and post again
		logging->out_debugf(_T("PostCommandMessage: %d"), id);
		PostCommandMessage(id);
		return 0;
	}
#endif
	case WM_INITMENUPOPUP:
//		if(emu) {
//			emu->mute_sound(true);
//			SystemPause(true);
//		}
		update_menu((HMENU)wParam, LOWORD(lParam));
		return 0;

	case WM_ENTERMENULOOP:
//		if(emu) {
//			SystemPause(true);
//		}
		now_menuloop = true;
		return 0;

	case WM_EXITMENULOOP:
		if(emu->is_fullscreen() && now_menuloop) {
			HideMenu();
		}
		if (emu) {
			emu->key_lost_focus();
//			emu->mute_sound(false);
			SystemPause(false);
		}
		now_menuloop = false;
		return 0;

#if defined(USE_WIN) || defined(USE_SDL)
	case WM_MOVE:
//		if(emu) {
//			emu->move_led();
//		}
		MoveLedBox();
		return 0;
#endif
#if defined(USE_WIN)
	case WM_MOUSEMOVE:
		if(emu->is_fullscreen() && !now_menuloop) {
			POINTS p = MAKEPOINTS(lParam);
			if(p.y == 0) {
				ShowMenu();
			}
			else if(p.y > 32) {
				HideMenu();
			}
		}
		return 0;
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
	case WM_DROPFILES:
		OpenDroppedFile((void *)wParam);
		return 0;
	case WM_ENTERIDLE:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
#endif
	}
#if defined(USE_WIN)
	return 1;
#elif defined(USE_SDL) || defined(USE_SDL2)
	if (OrigProcPtr) return CallWindowProc(OrigProcPtr, hWnd, iMsg, wParam, lParam);
	else return DefWindowProc(hWnd, iMsg, wParam, lParam);
#endif
}

#if defined(USE_SDL) || defined(USE_SDL2)
void GUI::PostProcessEvent()
{
#if defined(USE_SDL)
	// When fullscreen, no grab
	ClipCursor(NULL);
#endif
}
#endif

void GUI::ScreenModeChanged(bool fullscreen)
{
	if (fullscreen) {
		HideMenu();
	} else {
		ShowMenu();
	}
}

void GUI::SetFocusToMainWindow()
{
	::SetFocus(hWindow);
}

bool GUI::ShowRecordVideoDialog(int fps_num)
{
	bool rc = false;
#ifdef USE_REC_VIDEO
	PostEtSystemPause(true);
	if (recvidbox->Show(hWindow) == IDOK) {
		PostEtStartRecordVideo(fps_num);
		rc = true;
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
#endif
	return rc;
}

bool GUI::ShowRecordAudioDialog(void)
{
	bool rc = false;
#ifdef USE_REC_AUDIO
	PostEtSystemPause(true);
	if (recaudbox->Show(hWindow) == IDOK) {
		PostEtStartRecordSound();
		rc = true;
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
#endif
	return rc;
}

bool GUI::ShowRecordVideoAndAudioDialog(int fps_num)
{
	bool rc = false;
#ifdef USE_REC_VIDEO
	PostEtSystemPause(true);
	if (recvidbox->Show(hWindow, true) == IDOK && recaudbox->Show(hWindow) == IDOK) {
		PostEtStartRecordVideo(fps_num);
		rc = true;
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
#endif
	return rc;
}

#ifdef USE_EMU_INHERENT_SPEC
bool GUI::ShowConfigureDialog()
{
	bool rc = false;
	PostEtSystemPause(true);
	ConfigBox dlg(hInstance, font, emu, this);
	if (dlg.Show(hWindow) == IDOK) {
		rc = true;
	}
	PostEtSystemPause(false);
	SetFocusToMainWindow();
	return rc;
}

bool GUI::ShowKeybindDialog()
{
	bool rc = false;
	PostEtSystemPause(true);
	KeybindBox dlg(hInstance, font, emu, this);
	if (dlg.Show(hWindow) == IDOK) {
		emu->save_keybind();
		rc = true;
	}
	PostEtSystemPause(false);
	SetFocusToMainWindow();
	return rc;
}

bool GUI::ShowJoySettingDialog()
{
	bool rc = false;
	PostEtSystemPause(true);
	JoySettingBox dlg(hInstance, font, emu, this);
	rc = (dlg.Show(hWindow) == IDOK);
	PostEtSystemPause(false);
	SetFocusToMainWindow();
	return rc;
}

/// create volume dialog
bool GUI::ShowVolumeDialog()
{
	VolumeBox dlg(hInstance, font, emu, this);
	dlg.Show(hWindow);
	SetFocusToMainWindow();
	return true;
}

#ifdef USE_DEBUG_SOUND_FILTER
bool GUI::ShowSndFilterDialog()
{
	SndFilterBox dlg(hInstance, font, emu, this);
	dlg.Show(hWindow);
	return true;
}
#endif
#endif

/// create about dialog
bool GUI::ShowAboutDialog()
{
	AboutBox dlg(hInstance, font, emu, this);
	dlg.Show(hWindow);
	SetFocusToMainWindow();
	return true;
}

#ifdef USE_DATAREC
bool GUI::ShowLoadDataRecDialog()
{
//	const CMsg::Id filter[] =
//#if defined(DATAREC_PC8801)
//		{ CMsg::Supported_Files_cas_cmt_t88, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_BINARY_ONLY)
//		{ CMsg::Supported_Files_cas_cmt, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_TAP)
//		{ CMsg::Supported Files_wav_cas_tap, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_MZT)
//		{ CMsg::Supported Files_wav_cas_mzt_m12, CMsg::All_Files_, CMsg::End };
//#elif defined(USE_EMU_INHERENT_SPEC)
//		{ CMsg::Supported_Files_l3_l3b_l3c_wav_t9x, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_wav_cas, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::datarec_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Play_Data_Recorder_Tape),
		pConfig->initial_datarec_path.GetM(),
//		_T("l3"),
		false
	);
	if (rc) {
		PostEtLoadDataRecMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowSaveDataRecDialog()
{
//	const CMsg::Id filter[] =
//#if defined(DATAREC_PC8801)
//		{ CMsg::Supported_Files_cas_cmt, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_BINARY_ONLY)
//		{ CMsg::Supported_Files_cas_cmt, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_TAP)
//		{ CMsg::Supported_Files_wav_cas, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_MZT)
//		{ CMsg::Supported_Files_wav_cas, CMsg::All_Files_, CMsg::End };
//#elif defined(USE_EMU_INHERENT_SPEC)
//		{ CMsg::L3_File_l3, CMsg::L3B_File_l3b, CMsg::L3C_File_l3c, CMsg::Wave_File_wav, CMsg::T9X_File_t9x, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_wav_cas, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::datarec_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Record_Data_Recorder_Tape),
		pConfig->initial_datarec_path.GetM(),
//		_T("l3"),
		true
	);
	if (rc) {
		PostEtSaveDataRecMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

void GUI::set_datarec_file_menu(HMENU hMenu, UINT uItem)
{
	MENUITEMINFO minfo;
	_TCHAR str[_MAX_PATH];

	minfo.cbSize = sizeof(MENUITEMINFO);
	minfo.fMask = MIIM_TYPE;
	minfo.fType = MFT_STRING;

	bool ins = emu->datarec_opened(true);

	set_file_name_for_menu(ins, CMsg::Play_LB, CMsg::Play_, pConfig->opened_datarec_path.path, -1, str);

	minfo.dwTypeData = str;
	minfo.cch = (UINT)_tcslen(str);
	SetMenuItemInfo(hMenu, uItem, FALSE, &minfo);

	ins = emu->datarec_opened(false);

	set_file_name_for_menu(ins, CMsg::Rec_LB, CMsg::Rec_, pConfig->opened_datarec_path.path, -1, str);

	minfo.dwTypeData = str;
	minfo.cch = (UINT)_tcslen(str);
	SetMenuItemInfo(hMenu, uItem + 1, FALSE, &minfo);

//	CheckMenuRadioItem(hMenu, uItem, uItem + 1, uItem + opened_tape_type - 1, MF_BYCOMMAND);
	CheckMenuItem(hMenu, uItem, IsOpenedLoadDataRecFile() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, uItem + 1, IsOpenedSaveDataRecFile() ? MF_CHECKED : MF_UNCHECKED);
}
#endif

#ifdef USE_FD1
bool GUI::ShowOpenFloppyDiskDialog(int drv)
{
	const char *filter = LABELS::floppy_disk_exts;

	DWORD flags;
	_TCHAR title[128];
	UTILITY::stprintf(title, 128, CMSGM(Open_Floppy_Disk_VDIGIT), drv);
	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		pConfig->initial_disk_path.GetM(),
//		_T("d88"),
		false
	);
	flags = fbox.GetFlags();
	if(rc) {
		PostEtOpenFloppyMessage(drv, fbox.GetPathM(), 0, flags, true);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

int GUI::ShowSelectFloppyDriveDialog(int drv)
{
//	PostEtSystemPause(true);
	SelDrvBox dlg(hInstance, font, emu, this);
	dlg.SetDrive(drv);
	dlg.SetPrefix(CMSGM(FDD));
	int new_drv = (int)dlg.Show(hWindow);
	return new_drv >= 0 ? new_drv : drv;
}

bool GUI::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
//	const CMsg::Id filter[] =
//#ifndef USE_EMU_INHERENT_SPEC
//		{ CMsg::Supported_Files_d88_d77, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_d88, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::blank_floppy_disk_exts;

	DWORD flags;
	_TCHAR title[128];
	UTILITY::stprintf(title, 128, CMSGM(New_Floppy_Disk_VDIGIT), drv);
	FileBox fbox(hWindow);

	_TCHAR file_name[_MAX_PATH];
	UTILITY::create_date_file_path(pConfig->initial_disk_path, file_name, _MAX_PATH, filter);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		NULL,
//		_T("d88"),
		true,
		file_name
	);
	flags = fbox.GetFlags();
	if(rc) {
		rc = emu->create_blank_floppy_disk(fbox.GetPathM(), type);
	}
	if(rc) {
		PostEtOpenFloppyMessage(drv, fbox.GetPath(), 0, flags, true);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

void GUI::GetMultiVolumeStr(int num, const _TCHAR *name, _TCHAR *str, size_t slen)
{
	UTILITY::stprintf(str, slen, _T("%2d: %s"), num + 1, name[0] != _T('\0') ? name : CMSGM(no_label));
}

void GUI::set_disk_file_menu(HMENU hMenu, UINT uItem, int drv)
{
	MENUITEMINFO minfo;
	_TCHAR str[_MAX_PATH];

	minfo.cbSize = sizeof(MENUITEMINFO);
	minfo.fMask = MIIM_TYPE;
	minfo.fType = MFT_STRING;

	bool ins = InsertedFloppyDisk(drv);

	set_file_name_for_menu(ins, CMsg::Insert_LB, CMsg::Insert_, pConfig->opened_disk_path[drv].path, pConfig->opened_disk_path[drv].num, str);

	minfo.dwTypeData = str;
	minfo.cch = (UINT)_tcslen(str);

	SetMenuItemInfo(hMenu, uItem, FALSE, &minfo);
	CheckMenuItem(hMenu, uItem, ins ? MF_CHECKED : MF_UNCHECKED);
}

void GUI::set_disk_side_menu(HMENU hMenu, UINT uItem, int drv)
{
#if 0
	if (emu) {
		MENUITEMINFO minfo;
		_TCHAR str[40];

		minfo.cbSize = sizeof(MENUITEMINFO);
		minfo.fMask = MIIM_TYPE;
		minfo.fType = MFT_STRING;

		int side = GetSideFloppyDisk(drv);
		if (side == 1 && pConfig->fdd_type == FDD_TYPE_3FDD) {
			UTILITY::tcscpy(str, 40, CMSGM(Change_Side_to_A));
		} else {
			UTILITY::tcscpy(str, 40, CMSGM(Change_Side_to_B));
		}
		minfo.dwTypeData = str;
		minfo.cch = (UINT)_tcslen(str);

		SetMenuItemInfo(hMenu, uItem, FALSE, &minfo);
		EnableMenuItem(hMenu, uItem, side >= 0 && pConfig->fdd_type == FDD_TYPE_3FDD ? MF_ENABLED : MF_GRAYED);
	}
#endif
}

#endif	// USE_FD1

#ifdef USE_HD1
bool GUI::ShowOpenHardDiskDialog(int drv)
{
	const char *filter = LABELS::hard_disk_exts;

	DWORD flags;
	_TCHAR title[128];
	UTILITY::stprintf(title, 128, CMSGM(Open_Hard_Disk_VDIGIT), drv);
	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		pConfig->initial_disk_path.GetM(),
		false
	);
	flags = fbox.GetFlags();
	if(rc) {
		PostEtOpenHardDiskMessage(drv, fbox.GetPathM(), flags);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowOpenBlankHardDiskDialog(int drv, uint8_t type)
{
	const char *filter = LABELS::blank_hard_disk_exts;

	DWORD flags;
	_TCHAR title[128];
	UTILITY::stprintf(title, 128, CMSGM(New_Hard_Disk_VDIGIT), drv);
	FileBox fbox(hWindow);

	_TCHAR file_name[_MAX_PATH];
	UTILITY::create_date_file_path(pConfig->initial_hard_disk_path, file_name, _MAX_PATH, filter);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		NULL,
		true,
		file_name
	);
	flags = fbox.GetFlags();
	if(rc) {
		rc = emu->create_blank_hard_disk(fbox.GetPathM(), type);
	}
	if(rc) {
		PostEtOpenHardDiskMessage(drv, fbox.GetPath(), flags);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

void GUI::set_hard_disk_file_menu(HMENU hMenu, UINT uItem, int drv)
{
	MENUITEMINFO minfo;
	_TCHAR str[_MAX_PATH];

	minfo.cbSize = sizeof(MENUITEMINFO);
	minfo.fMask = MIIM_TYPE;
	minfo.fType = MFT_STRING;

	bool ins = MountedHardDisk(drv);

	set_file_name_for_menu(ins, CMsg::Mount_LB, CMsg::Mount_, pConfig->opened_hard_disk_path[drv].path, pConfig->opened_hard_disk_path[drv].num, str);

	minfo.dwTypeData = str;
	minfo.cch = (UINT)_tcslen(str);

	SetMenuItemInfo(hMenu, uItem, FALSE, &minfo);
	CheckMenuItem(hMenu, uItem, ins ? MF_CHECKED : MF_UNCHECKED);
}
#endif	// USE_HD1

void GUI::set_file_name_for_menu(bool ok, CMsg::Id ok_prefix, CMsg::Id ng_prefix, const _TCHAR *path, int num, _TCHAR *str)
{
	if (ok) {
#if defined(USE_UTF8_ON_MBCS)
		// convert UTF-8 to MBCS string
		_TCHAR tpath[_MAX_PATH];
		UTILITY::conv_to_native_path(path, tpath, _MAX_PATH);
#else
		const _TCHAR *tpath = path;
#endif
		UTILITY::tcscpy(str, _MAX_PATH, CMSGVM(ok_prefix));
		UTILITY::tcscat(str, _MAX_PATH, UTILITY::trim_center(tpath, TRIM_STRING_SIZE));
		if (num > 0) {
			size_t len = _tcslen(str);
			UTILITY::stprintf(&str[len], _MAX_PATH - len, _T(" : %d"), num + 1);
		}
		UTILITY::tcscat(str, _MAX_PATH, _T("]"));
	} else {
		UTILITY::tcscpy(str, _MAX_PATH, CMSGVM(ng_prefix));
	}
}

#ifdef USE_CART1
bool GUI::ShowOpenCartridgeDialog(int drv)
{
	const CMsg::Id filter[] =
#if defined(_GAMEGEAR)
		{ CMsg::Supported_Files_rom_bin_hex_gg_col, CMsg::All_Files_, CMsg::End };
#elif defined(_MASTERSYSTEM)
		{ CMsg::Supported_Files_rom_bin_hex_sms, CMsg::All_Files_, CMsg::End };
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		{ CMsg::Supported_Files_rom_bin_hex_60, CMsg::All_Files_, CMsg::End };
#elif defined(_PCENGINE) || defined(_X1TWIN)
		{ CMsg::Supported_Files_rom_bin_hex_pce, CMsg::All_Files_, CMsg::End };
#else
		{ CMsg::Supported_Files_rom_bin_hex, CMsg::All_Files_, CMsg::End };
#endif

	const _TCHAR *title =
#if defined(_PCENGINE) || defined(_X1TWIN)
		CMSGM(Open_HuCARD);
#else
		CMSGM(Open_Cartridge);
#endif

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		pConfig->initial_cart_path.GetM(),
		_T("rom"),
		false
	);
	if (rc) {
		PostEtOpenCartridgeMessage(drv, fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifdef USE_QD1
bool GUI::ShowOpenQuickDiskDialog(int drv)
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_mzt_q20_qdf, CMsg::All_Files_, CMsg::End };

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Open_Quick_Disk),
		pConfig->initial_quickdisk_path.GetM(),
		_T("mzt"),
		false
	);
	if (rc) {
		PostEtOpenQuickDiskMessage(drv, fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifdef USE_MEDIA
bool GUI::ShowOpenMediaDialog()
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_bin, CMsg::All_Files_, CMsg::End };

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Open_Media),
		pConfig->initial_media_path.GetM(),
		_T("bin"),
		false
	);
	if (rc) {
		PostEtOpenQuickDiskMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifdef USE_BINARY_FILE1
bool GUI::ShowLoadBinaryDialog(int drv)
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_ram_bin_hex, CMsg::All_Files_, CMsg::End };

	const _TCHAR *title =
#if defined(_PASOPIA) || defined(_PASOPIA7)
		CMSGM(Load_RAM_Pack_Cartridge);
#else
		CMSGM(Load_Memory_Dump);
#endif

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		pConfig->initial_binary_path.GetM(),
		_T("ram"),
		false
	);
	if (rc) {
		PostEtLoadBinaryMessage(drv, fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowSaveBinaryDialog(int drv)
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_ram_bin_hex, CMsg::All_Files_, CMsg::End };

	const _TCHAR *title =
#if defined(_PASOPIA) || defined(_PASOPIA7)
		CMSGM(Save_RAM_Pack_Cartridge);
#else
		CMSGM(Save_Memory_Dump);
#endif

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		title,
		pConfig->initial_binary_path.GetM(),
		_T("ram"),
		true
	);
	if (rc) {
		PostEtSaveBinaryMessage(drv, fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifdef USE_AUTO_KEY
bool GUI::ShowOpenAutoKeyDialog()
{
	const char *filter = LABELS::autokey_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Open_Text_File),
		pConfig->initial_autokey_path.GetM(),
		false
	);
	if(rc) {
		PostEtLoadAutoKeyMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifdef USE_STATE
bool GUI::ShowSaveStateDialog(bool cont)
{
	const char *filter = LABELS::state_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	pConfig->saved_state_path.Clear();
	bool rc = fbox.Show(
		filter,
		CMSGM(Save_Status_Data),
		pConfig->initial_state_path.GetM(),
		true
	);
	if (rc) {
		PostEtSaveStatusMessage(fbox.GetPathM(), cont);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowLoadStateDialog()
{
	const char *filter = LABELS::state_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Load_Status_Data),
		pConfig->initial_state_path.GetM(),
		false
		);
	if(rc) {
		PostEtLoadStatusMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

#ifdef USE_KEY_RECORD
bool GUI::ShowPlayRecKeyDialog()
{
	const char *filter = LABELS::key_rec_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Play_Recorded_Keys),
		pConfig->initial_state_path.GetM(),
		false
		);
	if(rc) {
		PostEtLoadRecKeyMessage(fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}

bool GUI::ShowRecordRecKeyDialog()
{
	const char *filter = LABELS::key_rec_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Record_Input_Keys),
		pConfig->initial_state_path.GetM(),
		true
		);
	if(rc) {
		PostEtSaveRecKeyMessage(fbox.GetPathM(), false);
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
bool GUI::ShowRecordStateAndRecKeyDialog()
{
	bool rc = ShowSaveStateDialog(true);
	if (!rc) return rc;
	return ShowRecordRecKeyDialog();
}
#endif
#endif /* USE_STATE */

#ifdef USE_PRINTER
bool GUI::ShowSavePrinterDialog(int drv)
{
	const char *filter = LABELS::printing_file_exts;

	FileBox fbox(hWindow);

	PostEtSystemPause(true);
	bool rc = fbox.Show(
		filter,
		CMSGM(Save_Printing_Data),
		pConfig->initial_printer_path.GetM(),
		true
	);
	if (rc) {
		PostEtSavePrinterMessage(drv, fbox.GetPathM());
	} else {
		PostEtSystemPause(false);
	}
	return rc;
}
#endif

#ifndef USE_SDL2
/**
 *	open dropped file
 */
bool GUI::OpenDroppedFile(void *param)
{
	_TCHAR dropped_file[_MAX_PATH];

	HDROP hDrop=(HDROP)param;
	// get first filename
	DragQueryFile(hDrop, 0, dropped_file, _MAX_PATH);
    DragFinish(hDrop);
	if (_tcslen(dropped_file) == 0) {
		return false;
	}

	return OpenFileByExtention(dropped_file);
}
#endif

//
//
void GUI::update_control_menu(HMENU hMenu)
{
//	CheckMenuItem(hMenu, ID_RESET, NowPowerOff() ? MF_UNCHECKED : MF_CHECKED); 

#ifndef _MBS1
#ifdef USE_DIPSWITCH
	for(int i = 0; i < 8; i++) {
#ifdef _BML3MK5
		CheckMenuItem(hMenu, ID_DIPSWITCH1 + i, (GetDipswitch() & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
#else
		CheckMenuItem(hMenu, ID_DIPSWITCH1 + i, !(GetDipswitch() & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
#endif
	}
#endif
#endif
#ifdef USE_EMU_INHERENT_SPEC
#ifdef _MBS1
	CheckMenuRadioItem(hMenu, ID_SYSTEM_MODE_1, ID_SYSTEM_MODE_2, ID_SYSTEM_MODE_2 - GetSystemMode(), MF_BYCOMMAND);
#endif
	CheckMenuRadioItem(hMenu, ID_FDD_TYPE_1, ID_FDD_TYPE_4, ID_FDD_TYPE_1 + NextFddType(), MF_BYCOMMAND);
#endif
#ifdef _HC80
	if(pConfig->device_type >= 0 && pConfig->device_type < 3) {
		CheckMenuRadioItem(hMenu, ID_HC80_RAMDISK0, ID_HC80_RAMDISK2, ID_HC80_RAMDISK0 + pConfig->device_type, MF_BYCOMMAND);
	}
#endif
#ifdef _MZ800
	if(pConfig->boot_mode >= 0 && pConfig->boot_mode < 2) {
		CheckMenuRadioItem(hMenu, ID_MZ800_MODE_MZ800, ID_MZ800_MODE_MZ700, ID_MZ800_MODE_MZ800 + pConfig->boot_mode, MF_BYCOMMAND);
	}
#endif
#ifdef _PASOPIA
	if(pConfig->boot_mode >= 0 && pConfig->boot_mode < 5) {
		CheckMenuRadioItem(hMenu, ID_PASOPIA_MODE_TBASIC_V1_0, ID_PASOPIA_MODE_MINI_PASCAL, ID_PASOPIA_MODE_TBASIC_V1_0 + pConfig->boot_mode, MF_BYCOMMAND);
	}
	if(pConfig->device_type >= 0 && pConfig->boot_mode < 3) {
		CheckMenuRadioItem(hMenu, ID_PASOPIA_DEVICE_RAM_PAC, ID_PASOPIA_DEVICE_JOYSTICK, ID_PASOPIA_DEVICE_RAM_PAC + pConfig->device_type, MF_BYCOMMAND);
	}
#endif
#ifdef _PC98DO
	if(pConfig->boot_mode >= 0 && pConfig->boot_mode < 5) {
		CheckMenuRadioItem(hMenu, ID_PC98DO_MODE_PC98, ID_PC8801_MODE_N, ID_PC98DO_MODE_PC98 + pConfig->boot_mode, MF_BYCOMMAND);
	}
#endif
#ifdef _PC8801MA
	if(pConfig->boot_mode >= 0 && pConfig->boot_mode < 4) {
		CheckMenuRadioItem(hMenu, ID_PC8801_MODE_V1S, ID_PC8801_MODE_N, ID_PC8801_MODE_V1S + pConfig->boot_mode, MF_BYCOMMAND);
	}
	if(pConfig->device_type >= 0 && pConfig->boot_mode < 3) {
//		CheckMenuRadioItem(hMenu, ID_PC8801_DEVICE_JOYSTICK, ID_PC8801_DEVICE_JOYMOUSE, ID_PC8801_DEVICE_JOYSTICK + pConfig->boot_mode, MF_BYCOMMAND);
		// joymouse is not supported yet...
		CheckMenuRadioItem(hMenu, ID_PC8801_DEVICE_JOYSTICK, ID_PC8801_DEVICE_MOUSE, ID_PC8801_DEVICE_JOYSTICK + pConfig->device_type, MF_BYCOMMAND);
	}
#endif
#if defined(_PC9801E) || defined(_PC9801VM) || defined(_PC98DO) || defined(_PC8801MA)
	if(pConfig->cpu_clock_low) {
		CheckMenuRadioItem(hMenu, ID_PC9801_CPU_CLOCK_HIGH, ID_PC9801_CPU_CLOCK_LOW, ID_PC9801_CPU_CLOCK_LOW, MF_BYCOMMAND);
	}
	else {
		CheckMenuRadioItem(hMenu, ID_PC9801_CPU_CLOCK_HIGH, ID_PC9801_CPU_CLOCK_LOW, ID_PC9801_CPU_CLOCK_HIGH, MF_BYCOMMAND);
	}
#endif
	if(pConfig->cpu_power >= 0 && pConfig->cpu_power <= 5) {
		CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER5, ID_CPU_POWER0 + GetCPUPower(), MF_BYCOMMAND);
	}
#ifdef USE_EMU_INHERENT_SPEC
	CheckMenuItem(hMenu, ID_PAUSE,    NowPause() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SYNC_IRQ, NowSyncIRQ() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SPECIAL_RESET, NowSpecialReset() ? MF_CHECKED : MF_UNCHECKED);
#ifdef _MBS1
	CheckMenuItem(hMenu, ID_MEMORY_NOWAIT, NowMemNoWait() ? MF_CHECKED : MF_UNCHECKED);
#endif
#endif
#ifdef USE_AUTO_KEY
	// auto key
	bool now_paste = true;
	if(emu) {
		now_paste = IsRunningAutoKey();
	}
	EnableMenuItem(hMenu, ID_AUTOKEY_OPEN, now_paste ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_AUTOKEY_START, now_paste ? MF_GRAYED : MF_ENABLED);
#endif
#ifdef USE_KEY_RECORD
	CheckMenuItem(hMenu, ID_RECKEY_PLAY, NowPlayingRecKey() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_RECKEY_REC,  NowRecordingRecKey() ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_STATE
	update_recent_menu(hMenu, ID_RECENT_STATE, pConfig->recent_state_path);
#endif
}

//
//
void GUI::update_fdd_menu(HMENU hMenu, int drv, int ID_OPEN_FD, int ID_CHANGE_FD, int ID_WRITEPROTECT_FD, int ID_RECENT_FD, int ID_SELECT_D88_BANK)
{
#ifdef USE_FD1
	update_multivolume_menu(hMenu, drv, ID_SELECT_D88_BANK);
	update_recent_menu(hMenu, ID_RECENT_FD, pConfig->recent_disk_path[drv]);
	set_disk_side_menu(hMenu, ID_CHANGE_FD, drv);
	set_disk_file_menu(hMenu, ID_OPEN_FD, drv);
	EnableMenuItem(hMenu, ID_WRITEPROTECT_FD, InsertedFloppyDisk(drv) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, ID_WRITEPROTECT_FD, WriteProtectedFloppyDisk(drv) ? MF_CHECKED : MF_UNCHECKED);
#endif
}

//
//
void GUI::update_hdd_menu(HMENU hMenu, int drv, int ID_OPEN_HD, int ID_RECENT_HD)
{
#ifdef USE_HD1
	set_hard_disk_file_menu(hMenu, ID_OPEN_HD, drv);
	for(int i=0; i<MAX_HARD_DISKS; i++) {
		HMENU hSubMenu = get_sub_menu(hMenu, ID_RECENT_HD);
		update_recent_menu(hSubMenu, ID_RECENT_HD, pConfig->recent_hard_disk_path[drv]);
	}
#endif
}

//
//
void GUI::update_tape_menu(HMENU hMenu)
{
#ifdef USE_DATAREC
	update_recent_menu(hMenu, ID_RECENT_DATAREC, pConfig->recent_datarec_path);

	set_datarec_file_menu(hMenu, ID_PLAY_DATAREC);
	// realmode
	CheckMenuItem(hMenu, ID_REAL_DATAREC, NowRealModeDataRec() ? MF_CHECKED : MF_UNCHECKED);
#endif
}

//
//
void GUI::update_screen_menu(HMENU hMenu)
{
	_TCHAR buf[_MAX_PATH];

	// recording
	bool now_rec = true;
	if(emu) {
		now_rec = NowRecordingVideo() | NowRecordingSound();
	}
#if defined(USE_REC_VIDEO) || defined(USE_CAPTURE_SCREEN_PNG)
	// rec video size
	InsertMenu(hMenu, ID_SCREEN_REC_SIZE1, MF_BYCOMMAND | MF_STRING, ID_SCREEN_BOTTOM, _T(""));
	for(int i = 0; i <= (ID_SCREEN_REC_SIZE2 - ID_SCREEN_REC_SIZE1); i++) {
		DeleteMenu(hMenu, ID_SCREEN_REC_SIZE1 + i, MF_BYCOMMAND);
	}
	for(int i = 0; i <= (ID_SCREEN_REC_SIZE2 - ID_SCREEN_REC_SIZE1); i++) {
		GetRecordVideoSizeStr(i, buf);
		InsertMenu(hMenu, ID_SCREEN_BOTTOM, MF_BYCOMMAND | MF_STRING, ID_SCREEN_REC_SIZE1 + i, buf);
	}
	DeleteMenu(hMenu, ID_SCREEN_BOTTOM, MF_BYCOMMAND);
	CheckMenuRadioItem(hMenu, ID_SCREEN_REC_SIZE1, ID_SCREEN_REC_SIZE2, ID_SCREEN_REC_SIZE1 + pConfig->screen_video_size, MF_BYCOMMAND);
	for(UINT i = ID_SCREEN_REC_SIZE1; i<= ID_SCREEN_REC_SIZE2; i++) {
		EnableMenuItem(hMenu, i, now_rec ? MF_GRAYED : MF_ENABLED);
	}
#endif
#if defined(USE_REC_VIDEO)
	for(UINT i = ID_SCREEN_REC60; i<= ID_SCREEN_REC10; i++) {
		EnableMenuItem(hMenu, i, now_rec ? MF_GRAYED : MF_ENABLED);
	}
	if (now_rec) {
		CheckMenuRadioItem(hMenu, ID_SCREEN_REC60, ID_SCREEN_REC10, ID_SCREEN_REC60 + GetRecordVideoFrameNum(), MF_BYCOMMAND);
	} else {
		CheckMenuRadioItem(hMenu, ID_SCREEN_REC60, ID_SCREEN_REC10, ID_SCREEN_REC60 - 1, MF_BYCOMMAND);
	}
//	EnableMenuItem(hMenu, ID_SCREEN_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
#endif
#ifdef USE_EMU_INHERENT_SPEC
	// frame rate
	CheckMenuRadioItem(hMenu, ID_SCREEN_VFRAME, ID_SCREEN_FPS10, ID_SCREEN_VFRAME + pConfig->fps_no + 1, MF_BYCOMMAND);
//	for(UINT i = ID_SCREEN_VFRAME; i<= ID_SCREEN_FPS10; i++) {
//		EnableMenuItem(hMenu, i, now_rec ? MF_GRAYED : MF_ENABLED);
//	}
#endif

	// window mode
	if (GetMenuState(hMenu, ID_SCREEN_WINDOW_A, MF_BYCOMMAND) != (UINT)-1) {
		for(int i = 0; i < GetWindowModeCount(); i++) {
			GetWindowModeStr(i, buf);
			InsertMenu(hMenu, ID_SCREEN_WINDOW_A, MF_BYCOMMAND | MF_STRING, ID_SCREEN_WINDOW1 + i, buf);
		}
		DeleteMenu(hMenu, ID_SCREEN_WINDOW_A, MF_BYCOMMAND);
	}
	if(pConfig->window_mode >= 0 && pConfig->window_mode < GetWindowModeCount()) {
		UINT first = ID_SCREEN_WINDOW1;
		UINT last = first + GetWindowModeCount() - 1;
		UINT chk = first + pConfig->window_mode;
		CheckMenuRadioItem(hMenu, first, last, chk, MF_BYCOMMAND);
	}

	// stretch screen
	CheckMenuItem(hMenu, ID_SCREEN_STRETCH, GetStretchScreen() == 1 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_CUTOUT, GetStretchScreen() == 2 ? MF_CHECKED : MF_UNCHECKED);

	// fullscreen mode
	if (GetMenuState(hMenu, ID_SCREEN_DISPLAY_A, MF_BYCOMMAND) != (UINT)-1) {
		// insert display submenu
		for(int disp_no = 0; disp_no < GetDisplayDeviceCount(); disp_no++) {
			HMENU hDMenu = ::CreateMenu();
			InsertMenu(hDMenu, 0, MF_BYPOSITION | MF_STRING, ID_SCREEN_BOTTOM, _T(""));
			for(int i = 0; i < GetFullScreenModeCount(disp_no); i++) {
				MENUITEMINFO info;
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				GetFullScreenModeStr(disp_no, i, buf);
				info.fMask = MIIM_TYPE | MIIM_ID;
				info.fType = MFT_STRING;
				info.wID = ID_SCREEN_FULLSCREEN0_01 + disp_no * VIDEO_MODE_MAX + i;
				info.dwTypeData = buf;
				InsertMenuItem(hDMenu, ID_SCREEN_BOTTOM, FALSE, &info);
			}
			DeleteMenu(hDMenu, ID_SCREEN_BOTTOM, MF_BYCOMMAND);
			GetDisplayDeviceStr(CMSGM(Display), disp_no, buf);
			InsertMenu(hMenu, ID_SCREEN_DISPLAY_A, MF_BYCOMMAND | MF_POPUP | MF_STRING, (UINT_PTR)hDMenu, buf);
		}
		DeleteMenu(hMenu, ID_SCREEN_DISPLAY_A, MF_BYCOMMAND);
	}
	UINT enable = IsFullScreen() ? MF_GRAYED : MF_ENABLED;
	for(int disp_no = 0; disp_no < GetDisplayDeviceCount(); disp_no++) {
		int cnt = GetFullScreenModeCount(disp_no);
		if(pConfig->window_mode >= (disp_no * VIDEO_MODE_MAX + 8) && pConfig->window_mode < (disp_no * VIDEO_MODE_MAX + cnt + 8)) {
			UINT first = ID_SCREEN_FULLSCREEN0_01 + disp_no * VIDEO_MODE_MAX;
			UINT last = first + cnt - 1;
			UINT chk = first + pConfig->window_mode - (disp_no * VIDEO_MODE_MAX) - 8;
			CheckMenuRadioItem(hMenu, first, last, chk, MF_BYCOMMAND);
		}
		for(int i = 0; i < GetFullScreenModeCount(disp_no); i++) {
			EnableMenuItem(hMenu, ID_SCREEN_FULLSCREEN0_01 + disp_no * VIDEO_MODE_MAX + i, enable);
		}
	}

	// pixel aspect ratio
	UINT last = GetPixelAspectModeCount();
	InsertMenu(hMenu, ID_SCREEN_PIXEL_ASPECT0, MF_BYCOMMAND | MF_STRING, ID_SCREEN_BOTTOM, _T(""));
	for(UINT i = 0; i < last; i++) {
		DeleteMenu(hMenu, ID_SCREEN_PIXEL_ASPECT0 + i, MF_BYCOMMAND);
	}
	for(UINT i = 0; i < last; i++) {
		GetPixelAspectModeStr(i, buf);
		InsertMenuA(hMenu, ID_SCREEN_BOTTOM, MF_BYCOMMAND | MF_STRING, ID_SCREEN_PIXEL_ASPECT0 + i, buf);
	}
	DeleteMenu(hMenu, ID_SCREEN_BOTTOM, MF_BYCOMMAND);
	CheckMenuRadioItem(hMenu, ID_SCREEN_PIXEL_ASPECT0, ID_SCREEN_PIXEL_ASPECT0 + last - 1, ID_SCREEN_PIXEL_ASPECT0 + GetPixelAspectMode(), MF_BYCOMMAND);

#ifdef USE_MONITOR_TYPE
	if(pConfig->monitor_type >= 0 && pConfig->monitor_type < USE_MONITOR_TYPE) {
		CheckMenuRadioItem(hMenu, ID_SCREEN_MONITOR_TYPE0, ID_SCREEN_MONITOR_TYPE0 + USE_MONITOR_TYPE - 1, ID_SCREEN_MONITOR_TYPE0 + pConfig->monitor_type, MF_BYCOMMAND);
	}
#elif defined(USE_SCREEN_ROTATE)
	if(pConfig->monitor_type >= 0 && pConfig->monitor_type < 4) {
		CheckMenuRadioItem(hMenu, ID_SCREEN_MONITOR_TYPE0, ID_SCREEN_MONITOR_TYPE1, ID_SCREEN_MONITOR_TYPE0 + pConfig->monitor_type, MF_BYCOMMAND);
	}
#endif
#ifdef USE_SCANLINE
	// scanline
	CheckMenuRadioItem(hMenu, ID_SCREEN_SCANLINE0, ID_SCREEN_SCANLINE3, ID_SCREEN_SCANLINE0 + GetDrawMode(), MF_BYCOMMAND);
#endif
#ifdef USE_AFTERIMAGE
	CheckMenuItem(hMenu, ID_SCREEN_AFTERIMAGE1, (GetAfterImageMode() & 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_AFTERIMAGE2, (GetAfterImageMode() & 2) ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_KEEPIMAGE
	CheckMenuItem(hMenu, ID_SCREEN_KEEPIMAGE1, (GetKeepImageMode() & 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_KEEPIMAGE2, (GetKeepImageMode() & 2) ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef _X68000
	{
		int val = GetShowScreen();
		for(int i=0; i<9; i++) {
			CheckMenuItem(hMenu, ID_SCREEN_SHOW_01 + i, (val & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
		}
	}
#endif
#if defined(USE_DIRECT3D)
	CheckMenuItem(hMenu, ID_SCREEN_D3D_SYNC, (pConfig->use_direct3d & 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_D3D_ASYNC, (pConfig->use_direct3d & 2) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuRadioItem(hMenu, ID_SCREEN_D3D_FILTER0, ID_SCREEN_D3D_FILTER2, ID_SCREEN_D3D_FILTER0 + GetDirect3DFilter(), MF_BYCOMMAND);
	bool enable_d3d = emu->enabled_direct3d();
	EnableMenuItem(hMenu, ID_SCREEN_D3D_SYNC, enable_d3d ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_ASYNC, enable_d3d ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_FILTER0, enable_d3d ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_FILTER1, enable_d3d ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_FILTER2, enable_d3d ? MF_ENABLED : MF_GRAYED);
#elif defined(USE_OPENGL)
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_SYNC, (GetOpenGLMode() & 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_ASYNC, (GetOpenGLMode() & 2) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuRadioItem(hMenu, ID_SCREEN_OPENGL_FILTER0, ID_SCREEN_OPENGL_FILTER2, ID_SCREEN_OPENGL_FILTER0 + GetOpenGLFilter(), MF_BYCOMMAND);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_SYNC, MF_ENABLED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_ASYNC, MF_ENABLED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER0, MF_ENABLED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER1, MF_ENABLED);
#else
#if defined(ID_SCREEN_OPENGL_SYNC)
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_SYNC, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_ASYNC, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER0, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER1, MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_SYNC, MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_ASYNC, MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER0, MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_OPENGL_FILTER1, MF_GRAYED);
#else
	CheckMenuItem(hMenu, ID_SCREEN_D3D_SYNC, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_D3D_ASYNC, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SCREEN_D3D_FILTER, MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_SYNC, MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_ASYNC, MF_GRAYED);
	EnableMenuItem(hMenu, ID_SCREEN_D3D_FILTER, MF_GRAYED);
#endif
#endif
#ifdef USE_D3D
	CheckMenuItem(hMenu, ID_SCREEN_WAIT_VSYNC, pConfig->wait_vsync ? MF_CHECKED : MF_UNCHECKED);
#endif
}

//
//
void GUI::update_sound_menu(HMENU hMenu)
{
	bool now_rec = false;
	if(emu) {
		now_rec = NowRecordingVideo() | NowRecordingSound();
	}
	EnableMenuItem(hMenu, ID_SOUND_REC, now_rec ? MF_GRAYED : MF_ENABLED);
	CheckMenuItem(hMenu, ID_SOUND_REC, now_rec ? MF_CHECKED : MF_UNCHECKED);
//		EnableMenuItem(hMenu, ID_SOUND_STOP, now_stop ? MF_GRAYED : MF_ENABLED);

	CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ7, ID_SOUND_FREQ0 + GetSoundFrequencyNum(), MF_BYCOMMAND);
	CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE5, ID_SOUND_LATE0 + GetSoundLatencyNum(), MF_BYCOMMAND);
#ifdef USE_SOUND_DEVICE_TYPE
	if(pConfig->sound_device_type >= 0 && pConfig->sound_device_type < USE_SOUND_DEVICE_TYPE) {
		CheckMenuRadioItem(hMenu, ID_SOUND_DEVICE_TYPE0, ID_SOUND_DEVICE_TYPE0 + USE_SOUND_DEVICE_TYPE - 1, ID_SOUND_DEVICE_TYPE0 + pConfig->sound_device_type, MF_BYCOMMAND);
	}
#endif
}

//
//
void GUI::update_device_menu(HMENU hMenu)
{
#ifdef MAX_PRINTER
	for(int i=0; i<MAX_PRINTER; i++) {
		int j = i * (ID_PRINTER1_SAVE - ID_PRINTER0_SAVE);
		int printer_buffer_size = 0;
		// printer menu
		if(emu) {
			printer_buffer_size = GetPrinterBufferSize(i);
		}
		EnableMenuItem(hMenu, ID_PRINTER0_SAVE + j,  printer_buffer_size <= 0 ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, ID_PRINTER0_PRINT + j, printer_buffer_size <= 0 ? MF_GRAYED : MF_ENABLED);
//		EnableMenuItem(hMenu, ID_PRINTER0_CLEAR + j, printer_buffer_size <= 0 ? MF_GRAYED : MF_ENABLED);
		CheckMenuItem(hMenu, ID_PRINTER0_ONLINE + j, IsOnlinePrinter(i) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_PRINTER0_DIRECT + j, IsEnablePrinterDirect(i) ? MF_CHECKED : MF_UNCHECKED);
	}
#endif
	int uarts = EnumUarts();
	for(int drv=0; drv<MAX_COMM; drv++) {
		int j = drv * (ID_COMM1_SERVER - ID_COMM0_SERVER);
		// comm menu
		CheckMenuItem(hMenu, ID_COMM0_SERVER + j, IsEnableCommServer(drv) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_COMM0_CONNECT + j, NowConnectingComm(drv, 0) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_COMM0_THROUGH + j, NowCommThroughMode(drv) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_COMM0_BINARY + j, NowCommBinaryMode(drv) ? MF_CHECKED : MF_UNCHECKED);

		// connect
		if (GetMenuState(hMenu, ID_COMM0_CONNECT + j, MF_BYCOMMAND) != (UINT)-1) {
			HMENU hSubMenu = get_sub_menu_new(hMenu, ID_COMM0_CONNECT + j);
			// delete menu
			while (GetMenuItemCount(hSubMenu) > 1) {
				DeleteMenu(hSubMenu, 1, MF_BYPOSITION);
			}
//			int pos = 1;
			if (uarts > 0) {
				AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
//				pos++;
			}
			// insert menu
			for(int idx = 0; idx < uarts && idx < (ID_COMM0_PORT_BOTTOM - ID_COMM0_PORT1); idx++) {
				_TCHAR buf[64];
				GetUartDescription(idx, buf, sizeof(buf));
				DWORD id = ID_COMM0_PORT1 + drv * (ID_COMM1_PORT1 - ID_COMM0_PORT1) + idx;
				DWORD flags = MF_STRING;
				if (NowConnectingComm(drv, idx + 1)) {
					flags |= MF_CHECKED;
				}
				AppendMenu(hSubMenu, flags, id, buf);

//				MENUITEMINFO info;
//				ZeroMemory(&info, sizeof(info));
//				info.cbSize = sizeof(info);
//				info.fMask = MIIM_TYPE | MIIM_ID;
//				info.fType = MFT_STRING;
//				info.wID = id;
//				info.dwTypeData = buf;
//				InsertMenuItem(hSubMenu, pos, TRUE, &info);
//				pos++;
//				if (NowConnectingComm(drv, idx + 1)) {
//					CheckMenuItem(hSubMenu, id, MF_CHECKED);
//				}
			}
		}
	}
}

//
//
void GUI::update_option_menu(HMENU hMenu)
{
	CheckMenuItem(hMenu, ID_OPTIONS_LEDBOX_SHOW, IsShownLedBox() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_OPTIONS_LEDBOX_INSIDE, IsInsidedLedBox() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_OPTIONS_MSGBOARD, IsShownMessageBoard() ? MF_CHECKED : MF_UNCHECKED);
#ifdef USE_PERFORMANCE_METER
	CheckMenuItem(hMenu, ID_OPTIONS_PMETER, IsShownPMeter() ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_DIRECTINPUT
	CheckMenuItem(hMenu, ID_OPTIONS_USE_DINPUT, NowUseDirectInput() ? MF_CHECKED : MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_OPTIONS_USE_DINPUT, IsEnableDirectInput() ? MF_ENABLED : MF_GRAYED);
#endif
	CheckMenuItem(hMenu, ID_OPTIONS_JOYPAD0, IsEnableJoypad(1) ? MF_CHECKED : MF_UNCHECKED);
#ifdef USE_PIAJOYSTICK
	CheckMenuItem(hMenu, ID_OPTIONS_JOYPAD1, IsEnableJoypad(2) ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_KEY2JOYSTICK
	CheckMenuItem(hMenu, ID_OPTIONS_KEY2JOYPAD, IsEnableKey2Joypad() ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_MOUSE
	CheckMenuItem(hMenu, ID_OPTIONS_MOUSE, IsEnableMouse() ? MF_CHECKED : MF_UNCHECKED);
#endif
	CheckMenuItem(hMenu, ID_OPTIONS_LOOSEN_KEY, IsLoosenKeyStroke() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_OPTIONS_VKEYBOARD, IsShownVirtualKeyboard() ? MF_CHECKED : MF_UNCHECKED);
#ifdef USE_DEBUGGER
	EnableMenuItem(hMenu, ID_OPEN_DEBUGGER0,  IsDebuggerOpened() ? MF_GRAYED : MF_ENABLED);
#endif
}

//
//
void GUI::update_menu(HMENU hMenu, int pos)
{
	if (hMenu != GetSubMenu(GetMenu(hWindow), pos)) return;

#ifdef MENU_POS_CONTROL
	if(pos == MENU_POS_CONTROL) {
		// control menu
		update_control_menu(hMenu);
	}
#endif
#ifdef MENU_POS_CART1
	else if(pos == MENU_POS_CART1) {
		// cartridge
		update_recent_menu(hMenu, ID_RECENT_CART1, pConfig->recent_cart_path[0]);
	}
#endif
#ifdef MENU_POS_FD1
	else if(pos == MENU_POS_FD1) {
		// floppy drive #1
		update_fdd_menu(hMenu, 0, ID_OPEN_FD1, ID_CHANGE_FD1, ID_WRITEPROTECT_FD1, ID_RECENT_FD1, ID_SELECT_D88_BANK1);
	}
#endif
#ifdef MENU_POS_FD2
	else if(pos == MENU_POS_FD2) {
		// floppy drive #2
		update_fdd_menu(hMenu, 1, ID_OPEN_FD2, ID_CHANGE_FD2, ID_WRITEPROTECT_FD2, ID_RECENT_FD2, ID_SELECT_D88_BANK2);
	}
#endif
#ifdef MENU_POS_FD3
	else if(pos == MENU_POS_FD3) {
		// floppy drive #3
		update_fdd_menu(hMenu, 2, ID_OPEN_FD3, ID_CHANGE_FD3, ID_WRITEPROTECT_FD3, ID_RECENT_FD3, ID_SELECT_D88_BANK3);
	}
#endif
#ifdef MENU_POS_FD4
	else if(pos == MENU_POS_FD4) {
		// floppy drive #4
		update_fdd_menu(hMenu, 3, ID_OPEN_FD4, ID_CHANGE_FD4, ID_WRITEPROTECT_FD4, ID_RECENT_FD4, ID_SELECT_D88_BANK4);
	}
#endif
#ifdef MENU_POS_FD5
	else if(pos == MENU_POS_FD5) {
		// floppy drive #5
		update_fdd_menu(hMenu, 4, ID_OPEN_FD5, ID_CHANGE_FD5, ID_WRITEPROTECT_FD5, ID_RECENT_FD5, ID_SELECT_D88_BANK5);
	}
#endif
#ifdef MENU_POS_FD6
	else if(pos == MENU_POS_FD6) {
		// floppy drive #6
		update_fdd_menu(hMenu, 5, ID_OPEN_FD6, ID_CHANGE_FD6, ID_WRITEPROTECT_FD6, ID_RECENT_FD6, ID_SELECT_D88_BANK6);
	}
#endif
#ifdef MENU_POS_HD1
	else if(pos == MENU_POS_HD1) {
		// hard disk drive #1
		update_hdd_menu(hMenu, 0, ID_OPEN_HD1, ID_RECENT_HD1);
	}
#endif
#ifdef MENU_POS_QD1
	else if(pos == MENU_POS_QD1) {
		// quick disk drive
		update_recent_menu(hMenu, ID_RECENT_QD1, pConfig->recent_quickdisk_path[0]);
	}
#endif
#ifdef MENU_POS_DATAREC
	else if(pos == MENU_POS_DATAREC) {
		// data recorder
		update_tape_menu(hMenu);
	}
#endif
#ifdef MENU_POS_MEDIA
	else if(pos == MENU_POS_MEDIA) {
		// media
		update_recent_menu(hMenu, ID_RECENT_MEDIA, pConfig->recent_media_path);
	}
#endif
#ifdef MENU_POS_BINARY1
	else if(pos == MENU_POS_BINARY1) {
		// binary #1
		update_recent_menu(hMenu, ID_RECENT_BINARY1, pConfig->recent_binary_path[0]);
	}
#endif
#ifdef MENU_POS_BINARY2
	else if(pos == MENU_POS_BINARY2) {
		// binary #2
		update_recent_menu(hMenu, ID_RECENT_BINARY2, pConfig->recent_binary_path[1]);
	}
#endif
#ifdef MENU_POS_SCREEN
	else if(pos == MENU_POS_SCREEN) {
		// screen menu
		update_screen_menu(hMenu);
	}
#endif
#ifdef MENU_POS_SOUND
	else if(pos == MENU_POS_SOUND) {
		// sound menu
		update_sound_menu(hMenu);
	}
#endif
#ifdef MENU_POS_DEVICE
	else if(pos == MENU_POS_DEVICE) {
		// device menu
		update_device_menu(hMenu);
	}
#endif
#ifdef MENU_POS_OPTIONS
	else if(pos == MENU_POS_OPTIONS) {
		// options menu
		update_option_menu(hMenu);
	}
#endif
	DrawMenuBar(hWindow);
}

/// translate text string on the menu
void GUI::translate_menu(HMENU hMenu, int depth)
{
	_TCHAR buf[_MAX_PATH];
	MENUITEMINFO mii;

	if (depth >= 10) return;

	int nums = GetMenuItemCount(hMenu);
	for(int i=0; i<nums; i++) {
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_SUBMENU | MIIM_STRING;
		mii.dwTypeData = buf;
		mii.cch = _MAX_PATH - 1;
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.cch > 0) {
			mii.fMask = MIIM_STRING;
#if defined(USE_SDL) || defined(USE_SDL2)
			const _TCHAR *tstr = clocale->GetText(mii.dwTypeData, CLocale::CODE_ACP);
#else
			const _TCHAR *tstr = _tgettext(mii.dwTypeData);
#endif
			if (tstr != buf) {
				UTILITY::tcscpy(buf, _MAX_PATH, tstr);
			}
			mii.cch = (UINT)_tcslen(buf);
			SetMenuItemInfo(hMenu, i, TRUE, &mii);
		}
		if (mii.hSubMenu != NULL) {
			translate_menu(mii.hSubMenu, depth + 1);
		}
	}
}

#if 0
#if defined(USE_SDL) || defined(USE_SDL2)
static const char *stock_str(const char *buf, int len)
{
	cur_msg_buf_ptr = ((cur_msg_buf_ptr + 1) % MSG_PTR_MAX);
	if (msg_buf_ptr[cur_msg_buf_ptr] != NULL) {
		delete [] msg_buf_ptr[cur_msg_buf_ptr];
	}
	msg_buf_ptr[cur_msg_buf_ptr] = new char[len+4];
	strcpy(msg_buf_ptr[cur_msg_buf_ptr], buf);

	return msg_buf_ptr[cur_msg_buf_ptr];
}

const char *GUI::gettext_acp(const char *src)
{
	char buf[_MAX_PATH];

	src = gettext(src); ///< UTF-8 string
	int len = strlen(src);
	if (len <= 0) return src;
	len = UTILITY::conv_to_api_string(src, len, buf, _MAX_PATH); 
	if (len <= 0) return src;

	return stock_str(buf, len);
}
const char *GUI::cmsg_acp(CMsg::Id id)
{
	char buf[_MAX_PATH];

	const char *src = gMessages.Get(id); ///< UTF-8 string
	int len = strlen(src);
	if (len <= 0) return src;
	len = UTILITY::conv_to_api_string(src, len, buf, _MAX_PATH); 
	if (len <= 0) return src;

	return stock_str(buf, len);
}
#endif
#endif

#endif /* GUI_TYPE_WINDOWS */

#ifdef GUI_TYPE_AGAR
#include "../agar/ag_gui.h"
#endif
#ifdef USE_AUTO_KEY
bool GUI::StartAutoKey(void)
{
	bool rc = true;
	if(OpenClipboard(NULL)) {
		HANDLE hClip = GetClipboardData(CF_TEXT);
		if(hClip) {
			char *str = (char*)GlobalLock(hClip);
			emu->start_auto_key(str);
		}
		CloseClipboard();
	} else {
		rc = false;
	}
	return rc;
}
#endif

bool GUI::ShowVirtualKeyboard()
{
#ifdef USE_VKEYBOARD
	if (!vkeyboard) {
		HINSTANCE hInstance = ::GetModuleHandle(NULL);
		vkeyboard = new Vkbd::VKeyboard(hInstance, hWindow);

		uint8_t *buf;
		int siz;
		emu->get_vm_key_status_buffer(&buf, &siz);
		FIFOINT *his = emu->get_vm_key_history();
		vkeyboard->SetStatusBufferPtr(buf, siz, VM_KEY_STATUS_VKEYBOARD);
		vkeyboard->SetHistoryBufferPtr(his);
#if defined(USE_SDL) || defined(USE_SDL2)
		if (!vkeyboard->Create(emu->resource_path())) {
			logging->out_log(LOG_ERROR, _T("Cannot open virtual keyboard window."));
		}
#else
		vkeyboard->Create();
#endif
		vkeyboard->Show();
	} else {
		vkeyboard->Close();
	}
	return true;
#else
	return false;
#endif
}
