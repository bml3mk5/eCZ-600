/** @file gui_base.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.04.01

	@brief [ gui base ]
*/

#ifndef GUI_BASE_H
#define GUI_BASE_H

#include "../common.h"
// -------------------------------------
#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#include <SDL_syswm.h>
#elif defined(USE_WX)
#include <SDL.h>
#include <SDL_syswm.h>
#include <wx/wx.h>
#elif  defined(USE_WX2)
#include <SDL.h>
#include <wx/wx.h>
#elif defined(USE_QT)
#include <QEvent>
#endif
// -------------------------------------
#include "../emu_osd.h"
#include "../vm/vm_defs.h"
#include "../rec_video_defs.h"

class EMU;
#ifdef USE_LEDBOX
class LedBox;
#endif
#ifdef USE_VKEYBOARD
namespace Vkbd {
	class VKeyboard;
}
#endif

#if defined(USE_QT)
class MyUserEvent;
#elif defined(USE_WX) || defined(USE_WX2)
class MyApp;
#endif

/// GUI wrapper class
class GUI_BASE
{
protected:
	EMU *emu;
	int need_update_screen;
	int globalkey_enable;

#ifdef USE_LEDBOX
	LedBox *ledbox;
#endif
#ifdef USE_VKEYBOARD
	Vkbd::VKeyboard *vkeyboard;
#endif
#if defined(_WIN32)
	HWND hWindow;
	void SetWindowHandle();
#endif

#ifdef USE_GTK
	bool exit_program;
#endif

	int next_sound_frequency;
	int next_sound_latency;

public:
	GUI_BASE(int argc, char **argv, EMU *new_emu);
	virtual ~GUI_BASE();

	/// @name send to gui from emu class or main.
	//@{

	/// @return -1:error 0:normal ok 1:ok but use default locale
	virtual int Init();
	virtual void InitializedEmu();
#ifdef GUI_USE_FOREIGN_WINDOW
	/// create window without SDL library
	virtual void *CreateWindow(int width, int height);
	virtual void *GetWindowData();
#endif
	/// create widget if need
#if defined(USE_WIN)
	virtual int CreateWidget(HWND new_hwnd, int width, int height);
#elif defined(USE_SDL)
	virtual int CreateWidget(SDL_Surface *screen, int width, int height);
#elif defined(USE_SDL2)
	virtual int CreateWidget(SDL_Window *window, int width, int height);
#elif defined(USE_QT)
	virtual int CreateWidget(QRect *screen, int width, int height);
#elif defined(USE_WX) || defined(USE_WX2)
	virtual int CreateWidget(wxWindow *window, int width, int height);
#endif
	/// create menu if need
	/// @return -1:error 0:normal ok 1:ok (need window resize)
	virtual int CreateMenu();
	/// show gui menu
	virtual void ShowMenu();
	/// hide gui menu
	virtual void HideMenu();

	/// draw gui widget on surface
	/// @return 1:processed SDL_UpdateRect() in this function
	virtual int MixSurface();

	/// post update screen request message
	/// @attention this function is called by emu thread
	/// @return false:did not call UpdateScreen in main thread
	virtual bool NeedUpdateScreen();
	/// post finished updateing screen request message
	virtual void UpdatedScreen();
	/// draw screen on window
	/// @note this function should be called by main thread
#if defined(USE_WIN)
	virtual void UpdateScreen(HDC hdc);
#elif defined(USE_SDL) || defined(USE_SDL2) || defined(USE_QT) || defined(USE_WX) || defined(USE_WX2)
	virtual void UpdateScreen();
	virtual void DecreaseUpdateScreenCount();
#endif

	virtual int CreateGlobalKeys();

	/// @return true : processed global key
	virtual bool ExecuteGlobalKeys(int code, uint32_t status);
	/// @return true : processed global key
	virtual bool ReleaseGlobalKeys(int code, uint32_t status);

	/// something do before process event
	virtual void PreProcessEvent();
	/// event processing
#if defined(USE_WIN)
	virtual LRESULT ProcessEvent(UINT iMsg, WPARAM wParam, LPARAM lParam);
#elif defined(USE_SDL) || defined(USE_SDL2)
	/// @return -1: exit event loop 0: processed myself 1: no processing
	virtual int ProcessEvent(SDL_Event *e);
#elif defined(USE_QT)
	/// @return -1: exit event loop 0: processed myself 1: no processing
	virtual int ProcessEvent(MyUserEvent *e);
#elif defined(USE_WX) || defined(USE_WX2)
	/// @return -1: exit event loop 0: processed myself 1: no processing
	virtual int ProcessEvent(wxCommandEvent &e);
#endif
	/// something do after process event
	virtual void PostProcessEvent();

	/// callback from emu screen change
	virtual void ScreenModeChanged(bool fullscreen);

	///
	virtual void PostDrive();

	/// post to WM_COMMAND/SDL event message
	virtual void PostCommandMessage(int id, void *data1 = NULL, void *data2 = NULL);

	/// process command
	virtual int ProcessCommand(int id, void *data1, void *data2);

	///
	virtual void KeyDown(int code, uint32_t mod);
	///
	virtual void KeyUp(int code);

	///
	virtual void SetFocusToMainWindow();

	/// ledbox
#if defined(USE_WIN)
	virtual LedBox *CreateLedBox();
#elif defined(USE_SDL) || defined(USE_SDL2)
	virtual LedBox *CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format);
#elif defined(USE_QT)
	virtual LedBox *CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format);
#elif defined(USE_WX) || defined(USE_WX2)
	virtual LedBox *CreateLedBox(const _TCHAR *res_path, CPixelFormat *src_format);
#endif
	virtual void CreateLedBoxSub();
	virtual void ReleaseLedBox();
	virtual void SetLedBoxPosition(bool mode, int left, int top, int width, int height, int place);
#if 0
#if defined(USE_WIN)
	virtual void DrawLedBox(HDC hdc);
	virtual void DrawLedBox(LPDIRECT3DSURFACE9 suf);
#elif defined(USE_SDL) || defined(USE_SDL2)
	virtual void DrawLedBox(CSurface *screen);
# if defined(USE_SDL2)
	virtual void DrawLedBox(CTexture &texture);
	virtual void DrawLedBox(COpenGLTexture &texture);
# endif
#elif defined(USE_QT)
	virtual void DrawLedBox(QImage *screen);
#elif defined(USE_WX) || defined(USE_WX2)
	virtual void DrawLedBox(CSurface *screen);
#endif
#endif
	virtual void MoveLedBox();

	virtual void UpdateIndicator(uint64_t flag);

	//@}

	/// @name send to ui class from ui (user / operator).
	//@{
	virtual void Exit(void);

	// Show Dialog

#ifdef USE_DATAREC
	virtual bool ShowLoadDataRecDialog(void);
	virtual bool ShowSaveDataRecDialog(void);
#endif

#ifdef USE_FD1
	virtual bool ShowOpenFloppyDiskDialog(int drv);
	virtual int  ShowSelectFloppyDriveDialog(int drv);
	virtual bool ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type);
#endif

#ifdef USE_HD1
	virtual bool ShowOpenHardDiskDialog(int drv);
	virtual bool ShowOpenBlankHardDiskDialog(int drv, uint8_t type);
	virtual bool ShowSelectHardDiskDeviceTypeDialog(int drv);
#endif

#ifdef USE_CART1
	virtual bool ShowOpenCartridgeDialog(int drv);
#endif

#ifdef USE_QD1
	virtual bool ShowOpenQuickDiskDialog(int drv);
#endif

#ifdef USE_MEDIA
	virtual bool ShowOpenMediaDialog(void);
#endif

#ifdef USE_BINARY_FILE1
	virtual bool ShowLoadBinaryDialog(int drv);
	virtual bool ShowSaveBinaryDialog(int drv);
#endif

	virtual bool ShowLoadStateDialog(void);
	virtual bool ShowSaveStateDialog(bool cont);

	virtual bool ShowOpenAutoKeyDialog(void);

	virtual bool ShowPlayRecKeyDialog(void);
	virtual bool ShowRecordRecKeyDialog(void);

	virtual bool ShowRecordStateAndRecKeyDialog(void);

	virtual bool ShowSavePrinterDialog(int drv);

	virtual bool ShowRecordVideoDialog(int fps_num);
	virtual bool ShowRecordAudioDialog(void);
	virtual bool ShowRecordVideoAndAudioDialog(int fps_num);

	virtual bool ShowVolumeDialog(void);
#ifdef USE_DEBUG_SOUND_FILTER
	virtual bool ShowSndFilterDialog(void);
#endif

	virtual bool ShowJoySettingDialog(void);

	virtual bool ShowKeybindDialog(void);
	virtual bool ShowConfigureDialog(void);
	virtual bool ShowLoggingDialog(void);

	virtual bool ShowAboutDialog(void);

	virtual bool ShowVirtualKeyboard(void);
	//@}

	/// @name callback to emu class from ui (user / operator).
	//@{

	// Control

	virtual void PostEtReset(void);
	virtual bool NowPowerOff(void);
	virtual void PostEtForceReset(void);
	virtual void PostEtSpecialReset(void);
	virtual bool NowSpecialReset(void);
	virtual void PostEtWarmReset(int onoff);
	virtual void Dipswitch(int bit);
	virtual uint8_t GetDipswitch(void);
#ifdef _MBS1
	virtual void ChangeSystemMode(int val);
	virtual void ToggleSystemMode(void);
	virtual int  GetSystemMode(void);
#endif
	virtual void PostEtInterrupt(int num);
	virtual void PostEtTogglePause(void);
	virtual void TogglePause(void);
	virtual bool NowPause(void);
	virtual void PostEtSystemPause(bool val);
	virtual void SystemPause(bool val);
	virtual bool NowSystemPause(void);
	virtual void PostEtCPUPower(int num);
	virtual int  GetCPUPower(void);

	virtual void ChangeFddType(int num);
	virtual int  GetFddType(void);
	virtual int  NextFddType(void);
	virtual void PostEtToggleSyncIRQ(void);
	virtual bool NowSyncIRQ(void);
#ifdef _MBS1
	virtual void ToggleMemNoWait(void);
	virtual bool NowMemNoWait(void);
#endif

	// Screen

	virtual void ChangeFrameRate(int num);
	virtual int  GetFrameRateNum(void);
	virtual void TogglgScreenMode(void);
	virtual void ChangeWindowMode(int num);
	virtual void ChangeFullScreenMode(int num);
	virtual int  GetWindowMode(void);
	virtual int  GetFullScreenMode(void);
	virtual int  GetWindowModeCount(void);
	virtual bool GetWindowModeStr(int num, _TCHAR *str) const;
	virtual int  GetDisplayDeviceCount(void);
	virtual bool GetDisplayDeviceStr(const _TCHAR *prefix, int num, _TCHAR *str) const;
	virtual int  GetFullScreenModeCount(int disp_no);
	virtual bool GetFullScreenModeStr(int disp_no, int num, _TCHAR *str) const;
	virtual bool IsFullScreen(void);
	virtual void PostEtChangeDrawMode(int num);
	virtual void PostEtChangeDrawMode(void);
	virtual int  GetDrawMode(void);
	virtual void ChangeStretchScreen(int num);
	virtual int  GetStretchScreen(void);
//	virtual void ToggleCutoutScreen(void);
//	virtual bool NowCutoutedScreen(void);
	virtual void ChangePixelAspect(int num);
	virtual int  GetPixelAspectMode(void);
	virtual int  GetPixelAspectModeCount(void);
	virtual void GetPixelAspectModeStr(int num, _TCHAR *str);

	virtual void PostEtStartRecordVideo(int num);
	virtual void PostEtStopRecordVideo(void);
	virtual bool NowRecordingVideo(void);
	virtual int  GetRecordVideoFrameNum(void);
	virtual void PostEtResizeRecordVideoSurface(int num);
	virtual int  GetRecordVideoSurfaceNum(void);
	virtual bool GetRecordVideoSizeStr(int num, _TCHAR *str) const;
	virtual void PostEtCaptureScreen(void);

#ifdef USE_AFTERIMAGE
	virtual void PostEtChangeAfterImage(int num);
	virtual void PostEtChangeAfterImage(void);
	virtual int  GetAfterImageMode(void);
#endif
#ifdef USE_KEEPIMAGE
	virtual void PostEtChangeKeepImage(int num);
	virtual void PostEtChangeKeepImage(void);
	virtual int  GetKeepImageMode(void);
#endif
#ifdef _MBS1
	virtual void ChangeRGBType(int num);
	virtual void ChangeRGBType(void);
	virtual int  GetRGBTypeMode(void);
#endif
#ifdef _X68000
	virtual void ToggleShowScreen(int flags);
	virtual int  GetShowScreen();
#endif

#ifdef USE_DIRECT3D
	virtual void ChangeUseDirect3D(int num);
	virtual void ChangeUseDirect3D(void);
	virtual int  GetDirect3DMode(void);
	virtual void ChangeDirect3DFilter(int num);
	virtual int  GetDirect3DFilter(void);
#endif
#ifdef USE_OPENGL
	virtual void ChangeUseOpenGL(int num);
	virtual void ChangeUseOpenGL(void);
	virtual int  GetOpenGLMode(void);
	virtual void ChangeOpenGLFilter(int num);
	virtual int  GetOpenGLFilter(void);
#endif

	// Sound

	virtual void PostEtStartRecordSound(void);
	virtual void PostEtStopRecordSound(void);
	virtual bool NowRecordingSound(void);
	virtual void ChangeSoundFrequency(int num);
	virtual int  GetSoundFrequencyNum();
	virtual void ChangeSoundLatency(int num);
	virtual int  GetSoundLatencyNum();

#ifdef USE_DATAREC
	// Data Recorder Tape

	virtual void PostEtLoadDataRecMessage(const _TCHAR *file_path);
	virtual void PostEtLoadRecentDataRecMessage(int num);
	virtual void PostEtSaveDataRecMessage(const _TCHAR *file_path);
	virtual void PostEtCloseDataRecMessage(void);
	virtual void PostEtRewindDataRecMessage(void);
	virtual void PostEtFastForwardDataRecMessage(void);
	virtual void PostEtStopDataRecMessage(void);
	virtual void PostEtToggleRealModeDataRecMessage(void);
	virtual bool NowRealModeDataRec(void);
	virtual bool IsOpenedLoadDataRecFile(void);
	virtual bool IsOpenedSaveDataRecFile(void);
#endif

#ifdef USE_FD1
	// Floppy disk

	virtual void PostEtOpenFloppyMessage(int drv, const _TCHAR *file_path, int bank, uint32_t flags, bool multiopen);
	virtual void PostEtOpenRecentFloppyMessage(int drv, int num);
	virtual void PostEtOpenFloppySelectedVolume(int drv, int bank_num);
	virtual void PostEtCloseFloppyMessage(int drv);
	virtual void PostEtChangeSideFloppyDisk(int drv);
	virtual int  GetSideFloppyDisk(int drv);
	virtual void PostEtToggleWriteProtectFloppyDisk(int drv);
	virtual bool InsertedFloppyDisk(int drv);
	virtual bool WriteProtectedFloppyDisk(int drv);

	virtual bool OpenFloppyDisk(int drv, const _TCHAR* filename, int bank_num, uint32_t flags, bool multiopen);
	virtual void CloseFloppyDisk(int drv);

	virtual bool OpenFloppyDiskSelectedVolume(int drv, int bank_num);

	virtual void GetMultiVolumeStr(int num, const _TCHAR *name, _TCHAR *str, size_t slen);

//	virtual d88_file_t *GetD88File(int drv);
	virtual D88File *GetD88File(int drv);
#endif

#ifdef USE_HD1
	// Hard disk

	virtual void PostEtOpenHardDiskMessage(int drv, const _TCHAR *file_path, uint32_t flags);
	virtual void PostEtOpenRecentHardDiskMessage(int drv, int num);
	virtual void PostEtCloseHardDiskMessage(int drv);
	virtual bool MountedHardDisk(int drv);
	virtual void PostEtToggleWriteProtectHardDisk(int drv);
	virtual bool WriteProtectedHardDisk(int drv);
	virtual int  GetHardDiskDeviceType(int drv);
	virtual void ChangeHardDiskDeviceType(int drv, int num);
	virtual int  GetCurrentHardDiskDeviceType(int drv);
#endif

#ifdef USE_CART1
	// Cartridge

	virtual void PostEtOpenCartridgeMessage(const _TCHAR *file_path);
	virtual void PostEtOpenRecentCartridgeMessage(int drv, int num);
	virtual void PostEtCloseCartridgeMessage(int drv);
#endif

#ifdef USE_QD1
	// Quick Disk

	virtual void PostEtOpenQuickDiskMessage(int drv, const _TCHAR *file_path);
	virtual void PostEtToggleWriteProtectQuickDisk(int drv);
	virtual void PostEtOpenRecentQuickDiskMessage(int drv, int num);
	virtual void PostEtCloseQuickDiskMessage(int drv);
#endif

#ifdef USE_MEDIA
	// Media

	virtual void PostEtOpenMediaMessage(const _TCHAR *file_path);
	virtual void PostEtOpenRecentMediaMessage(int num);
	virtual void PostEtCloseMediaMessage(void);
#endif

#ifdef USE_BINARY_FILE1
	// Binary File

	virtual void PostEtLoadBinaryMessage(int drv, const _TCHAR *file_path);
	virtual void PostEtSaveBinaryMessage(int drv, const _TCHAR *file_path);
	virtual void PostEtLoadRecentBinaryMessage(int drv, int num);
	virtual void PostEtCloseBinaryMessage(int drv);
#endif

#ifdef USE_PRINTER
	// Printer

	virtual void PostEtSavePrinterMessage(int drv, const _TCHAR *file_path);
	virtual int  GetPrinterBufferSize(int drv);
	virtual void PostEtClearPrinterBufferMessage(int drv);
	virtual void PostEtPrintPrinterMessage(int drv);
	virtual void PrintPrinter(int drv);
	virtual void PostEtEnablePrinterDirectMessage(int drv);
	virtual void EnablePrinterDirect(int drv);
	virtual bool IsEnablePrinterDirect(int drv);
	virtual void PostEtTogglePrinterOnlineMessage(int drv);
//	virtual void TogglePrinterOnline(int drv);
	virtual bool IsOnlinePrinter(int drv);
#endif

	// Comm

//	virtual void PostEtEnableCommServerMessage(int drv);
	virtual void ToggleEnableCommServer(int drv);
	virtual bool IsEnableCommServer(int drv);
//	virtual void PostEtToggleConnectCommMessage(int drv, int num);
	virtual void ToggleConnectComm(int drv, int num);
	virtual bool NowConnectingComm(int drv, int num);
	virtual void ToggleCommThroughMode(int drv);
	virtual bool NowCommThroughMode(int drv);
	virtual void ToggleCommBinaryMode(int drv);
	virtual bool NowCommBinaryMode(int drv);
	virtual void SendCommTelnetCommand(int drv, int num);

	virtual int  EnumUarts();
	virtual void GetUartDescription(int ch, _TCHAR *buf, size_t size);

	// Status

	virtual void PostEtLoadStatusMessage(const _TCHAR *file_path);
	virtual void PostEtLoadRecentStatusMessage(int num);
	virtual void PostEtSaveStatusMessage(const _TCHAR *file_path, bool sys_pause);

	// Auto Key

	virtual void PostEtLoadAutoKeyMessage(const _TCHAR *file_path);
	virtual void PostEtStartAutoKeyMessage(void);
	virtual void PostEtStopAutoKeyMessage(void);
	virtual bool IsRunningAutoKey(void);

	virtual bool StartAutoKey(void);

	// Record key

	virtual void PostEtLoadRecKeyMessage(const _TCHAR *file_path);
	virtual void PostEtSaveRecKeyMessage(const _TCHAR *file_path, bool sys_pause);
	virtual void StopPlayRecKey(void);
	virtual void StopRecordRecKey(void);
	virtual bool NowPlayingRecKey(void);
	virtual bool NowRecordingRecKey(void);

	// Misc

	virtual void ChangeLedBox(int phase);
	virtual void ToggleLedBox(void);
	virtual void ToggleShowLedBox(void);
	virtual void ToggleInsideLedBox(void);
	virtual int  GetLedBoxPhase(int id);
	virtual bool IsShownLedBox(void);
	virtual bool IsInsidedLedBox(void);
	virtual void ChangeLedBoxPosition(int num);

	virtual void UpdateVirtualKeyboard(uint32_t flag);
	virtual bool IsShownVirtualKeyboard(void);

	virtual void ToggleMessageBoard(void);
	virtual bool IsShownMessageBoard(void);
	virtual bool IsShownLoggingDialog(void);
#ifdef USE_PERFORMANCE_METER
	virtual void TogglePMeter(void);
	virtual bool IsShownPMeter(void);
#endif
	virtual void ChangeUseJoypad(int num);
	virtual bool IsEnableJoypad(int num);
#ifdef USE_KEY2JOYSTICK
	virtual void ToggleEnableKey2Joypad(void);
	virtual bool IsEnableKey2Joypad(void);
#endif
#ifdef USE_LIGHTPEN
	virtual void ToggleEnableLightpen(void);
	virtual bool IsEnableLightpen(void);
#endif
#ifdef USE_MOUSE
	virtual void ToggleUseMouse(void);
	virtual bool IsEnableMouse(void);
	virtual void PostMtToggleUseMouse(void);
	virtual void PostMtEnableMouseTemp(bool val);
#endif
#ifdef USE_DIRECTINPUT
	virtual void ToggleUseDirectInput(void);
	virtual bool NowUseDirectInput(void);
	virtual bool IsEnableDirectInput(void);
#endif
	virtual void ToggleLoosenKeyStroke(void);
	virtual bool IsLoosenKeyStroke(void);

	virtual bool GetRecentFileStr(const _TCHAR *file, int num, _TCHAR *str, int trimlen);

#ifdef USE_DEBUGGER
	virtual void OpenDebugger();
	virtual void PostMtOpenDebugger();
	virtual void CloseDebugger();
	virtual void PostMtCloseDebugger();
	virtual bool IsDebuggerOpened();
#endif

	// Drop
	virtual bool OpenFileByExtention(const _TCHAR *file_path);
	virtual bool OpenDroppedFile(void *param);
	//@}

	/// @name utilities
	//@{
	enum enSupportedFileTypes {
		FILE_TYPE_NONE = 0,
		FILE_TYPE_DATAREC,
		FILE_TYPE_FLOPPY,
		FILE_TYPE_SASI_HARD_DISK,
		FILE_TYPE_SCSI_HARD_DISK,
		FILE_TYPE_STATE,
		FILE_TYPE_AUTO_KEY,
		FILE_TYPE_KEY_RECORD,
		FILE_TYPE_INITIALIZE,
	};
	static int CheckSupportedFile(const _TCHAR *file_path);
	virtual void GetLibVersionString(_TCHAR *str, int max_len = _MAX_PATH, const _TCHAR *sep_str = _T("\n"));
	//@}
};

#if defined(USE_QT)
///
/// @brief MyUserEvent
///
class MyUserEvent : public QEvent
{
public:
	explicit MyUserEvent(Type type);
	~MyUserEvent();

	int code;
	void *data1;
	void *data2;
};
#endif

#endif /* GUI_BASE_H */
