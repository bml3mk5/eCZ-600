/** @file cocoa_gui.h

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2015.04.24 -

	@brief [ gui for macosx ]
*/

#ifndef COCOA_GUI_H
#define COCOA_GUI_H

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import "cocoa_loggingpanel.h"
#endif /* __OBJC__ */

#include "../gui_base.h"
#include "../../common.h"
#include "../../config.h"


#ifdef __OBJC__
//
@interface NSApplication (SDLApplication)
- (void)orderFrontStandardAboutPanel:(id)sender;
@end

#endif /* __OBJC__ */

#ifdef GUI_TYPE_COCOA

class GUI;

#ifdef __OBJC__

/**
	@brief Menu item
*/
@interface CocoaMenuItem : NSMenuItem
{
	int drv;
	int num;
}
@property (nonatomic) int drv;
@property (nonatomic) int num;

- (id)initWithTitle:(NSString *)new_title action:(SEL)new_action drv:(int)new_drv num:(int)new_num keyEquivalent:(NSString *)new_accl;
- (id)initWithTitleById:(CMsg::Id)new_titleid action:(SEL)new_action drv:(int)new_drv num:(int)new_num keyEquivalent:(NSString *)new_accl;
- (void)setTitleById:(CMsg::Id)new_titleid;
@end

/**
	@brief Menu
*/
@interface CocoaMenu : NSMenu
+ (CocoaMenu *)create_menu:(const char *)new_title;
+ (CocoaMenu *)create_menu_by_id:(CMsg::Id)new_titleid;
+ (CocoaMenu *)create_menu:(const char *)new_title :(id)new_delegate;
+ (CocoaMenu *)create_menu_by_id:(CMsg::Id)new_titleid :(id)new_delegate;
- (CocoaMenuItem *)add_menu_item:(const char *)new_title :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num :(unichar)new_accl;
- (CocoaMenuItem *)add_menu_item_by_id:(CMsg::Id)new_titleid :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num :(unichar)new_accl;
- (void)add_sub_menu:(CocoaMenu *)submenu :(const char *)new_title;
- (void)add_sub_menu_by_id:(CocoaMenu *)submenu :(CMsg::Id)new_titleid;
- (void)add_sub_menu:(CocoaMenu *)submenu :(const char *)new_title :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num;
- (void)add_sub_menu_by_id:(CocoaMenu *)submenu :(CMsg::Id)new_titleid :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num;
- (void)setTitleById:(CMsg::Id)new_titleid;
@end

/**
	@brief Delegate for Menu
*/
@interface CocoaMenuDelegate : NSObject <NSMenuDelegate>
+ (CocoaMenuDelegate *)create;
- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)new_target action:(SEL *)new_action;
@end

/**
	@brief GUI class for Cocoa
*/
@interface CocoaController : NSObject
{
	GUI *gui;
}
@property (nonatomic) GUI *gui;

- (void)UpdateRecentFiles:(CocoaMenuItem *)menuItem :(CRecentPathList &)list :(int)drv :(SEL)action;

- (void)Reset:(id)sender;
- (void)ForceReset:(id)sender;
- (void)SpecialReset:(id)sender;
- (void)WarmReset:(id)sender;
- (void)Interrupt:(id)sender;
- (void)Dipswitch:(id)sender;
- (void)TogglePause:(id)sender;
- (void)CPUPower:(id)sender;
#ifdef _MBS1
- (void)ChangeSystemMode:(id)sender;
#endif

- (void)ChangeFddType:(id)sender;
- (void)ToggleSyncIRQ:(id)sender;
#ifdef _MBS1
- (void)ToggleMemNoWait:(id)sender;
#endif

- (void)ShowOpenAutoKeyDialog:(id)sender;
- (void)StartAutoKey:(id)sender;
- (void)StopAutoKey:(id)sender;

- (void)ShowPlayRecKeyDialog:(id)sender;

- (void)StopPlayRecKey:(id)sender;
- (void)ShowRecordRecKeyDialog:(id)sender;
- (void)StopRecordRecKey:(id)sender;

- (void)ShowRecordStateAndRecKeyDialog:(id)sender;

- (void)ShowLoadStateDialog:(id)sender;
- (void)ShowSaveStateDialog:(id)sender;
- (void)LoadRecentState:(id)sender;

- (void)UpdateRecentStateList:(id)sender;

#ifdef USE_DATAREC
- (void)ShowLoadDataRecDialog:(id)sender;
- (void)ShowSaveDataRecDialog:(id)sender;
- (void)RewindDataRec:(id)sender;
- (void)FastForwardDataRec:(id)sender;
- (void)CloseDataRec:(id)sender;
- (void)StopDataRec:(id)sender;
- (void)ToggleRealModeDataRec:(id)sender;
- (void)LoadRecentDataRec:(id)sender;

- (void)UpdateRecentDataRecList:(id)sender;
#endif

#ifdef USE_FD1
- (void)ShowOpenFloppyDiskDialog:(id)sender;
- (void)ChangeSideFloppyDisk:(id)sender;
- (void)CloseFloppy:(id)sender;
- (void)ShowOpenBlankFloppyDiskDialog:(id)sender;
- (void)ToggleWriteProtectFloppyDisk:(id)sender;
- (void)OpenRecentFloppy:(id)sender;
- (void)OpenFloppySelectedVolume:(id)sender;

- (void)UpdateRecentFloppyList:(id)sender;
- (void)UpdateVolumeFloppyList:(id)sender;
#endif

#ifdef USE_HD1
- (void)ShowOpenHardDiskDialog:(id)sender;
- (void)CloseHardDisk:(id)sender;
- (void)ShowOpenBlankHardDiskDialog:(id)sender;
- (void)ToggleWriteProtectHardDisk:(id)sender;
- (void)ShowSelectHardDiskDeviceTypeDialog:(id)sender;
- (void)OpenRecentHardDisk:(id)sender;

- (void)UpdateRecentHardDiskList:(id)sender;
#endif

- (void)ChangeFrameRate:(id)sender;
- (void)ResizeRecordVideoSurface:(id)sender;
- (void)ChangeRecordVideoFrameRate:(id)sender;
- (void)ShowRecordVideoDialog:(id)sender;
- (void)StopRecordVideo:(id)sender;
- (void)CaptureScreen:(id)sender;
- (void)ChangeWindowMode:(id)sender;
- (void)ChangeFullScreenMode:(id)sender;
- (void)ToggleStretchScreen:(id)sender;
/* - (void)ToggleCutoutScreen:(id)sender; */
- (void)ChangePixelAspectMode:(id)sender;
- (void)ChangeScanLine:(id)sender;
#ifdef USE_AFTERIMAGE
- (void)ChangeAfterImage:(id)sender;
#endif
#ifdef USE_KEEPIMAGE
- (void)ChangeKeepImage:(id)sender;
#endif
#ifdef _MBS1
- (void)ChangeRGBType:(id)sender;
#endif
#ifdef _X68000
- (void)ChangeShowScreen:(id)sender;
#endif
- (void)ChangeUseOpenGL:(id)sender;
- (void)ChangeOpenGLFilter:(id)sender;


- (void)ShowVolumeDialog:(id)sender;
- (void)ShowRecordAudioDialog:(id)sender;
- (void)StopRecordSound:(id)sender;
- (void)ChangeSoundFrequency:(id)sender;
- (void)ChangeSoundLatency:(id)sender;


- (void)ShowSavePrinterDialog:(id)sender;
- (void)PrintPrinter:(id)sender;
- (void)ClearPrinterBuffer:(id)sender;
- (void)EnablePrinterDirect:(id)sender;
- (void)TogglePrinterOnline:(id)sender;

- (void)EnableCommServer:(id)sender;
- (void)ConnectComm:(id)sender;
- (void)CommThroughMode:(id)sender;
- (void)CommBinaryMode:(id)sender;
- (void)SendCommTelnetCommand:(id)sender;

- (void)UpdateCommConnectList:(id)sender;


- (void)ToggleLedBox:(id)sender;
/* - (void)ToggleShowLedBox:(id)sender; */
/* - (void)ToggleInsideLedBox:(id)sender; */
- (void)ToggleMessageBoard:(id)sender;
#ifdef USE_PERFORMANCE_METER
- (void)TogglePMeter:(id)sender;
#endif
- (void)ChangeUseJoypad:(id)sender;
#ifdef USE_KEY2JOYSTICK
- (void)ToggleEnableKey2Joypad:(id)sender;
#endif
#ifdef USE_LIGHTPEN
- (void)ToggleEnableLightpen:(id)sender;
#endif
#ifdef USE_MOUSE
- (void)ToggleUseMouse:(id)sender;
#endif
- (void)ToggleLoosenKeyStroke:(id)sender;
- (void)ShowJoySettingDialog:(id)sender;
- (void)ShowKeybindDialog:(id)sender;
- (void)ShowConfigureDialog:(id)sender;
- (void)ShowVirtualKeyboard:(id)sender;
- (void)ShowLoggingDialog:(id)sender;

#ifdef USE_DEBUGGER
- (void)OpenDebugger:(id)sender;
- (void)CloseDebugger:(id)sender;
#endif


- (void)PerformUpdateScreen;

- (BOOL)validateMenuItem:(CocoaMenuItem *)menuItem;

@end

#ifdef USE_DELEGATE
/**
	@brief Recent file list for datarec
*/
@interface CocoaRecentDataRecList : NSObject <NSMenuDelegate>
{
	CocoaController *target;
}
- (id)initWithTarget:(CocoaController *)new_target;
- (void)menuWillOpen:(CocoaMenu *)menu;
- (void)menuDidClose:(CocoaMenu *)menu;
@end

/**
	@brief Recent file list for floppy
*/
@interface CocoaRecentFloppyDiskList : NSObject <NSMenuDelegate>
{
	CocoaController *target;
	int drv;
}
- (id)initWithTarget:(CocoaController *)new_target :(int)new_drv;
- (void)menuWillOpen:(CocoaMenu *)menu;
- (void)menuDidClose:(CocoaMenu *)menu;
@end

/**
	@brief Multi volume list on floppy disk
*/
@interface CocoaVolumeFloppyDiskList : NSObject <NSMenuDelegate>
{
	CocoaController *target;
	int drv;
}
- (id)initWithTarget:(CocoaController *)new_target :(int)new_drv;
- (void)menuWillOpen:(CocoaMenu *)menu;
- (void)menuDidClose:(CocoaMenu *)menu;
@end
#endif /* USE_DELEGATE */

//
void add_main_menu(CocoaMenu *submenu, const char *new_title);
void add_main_menu_by_id(CocoaMenu *submenu, CMsg::Id new_titleid);
NSArray *get_file_filter(const char *str);

#endif /* __OBJC__ */
/**
	@brief GUI class
*/
class GUI : public GUI_BASE
{
private:
#ifdef __OBJC__
	CocoaController *recv;
	CocoaLoggingPanel *logging_dlg;
/*	CocoaMenu *popupMenu; */
#else
	void *recv;
	void *logging_dlg;
/*	void *popupMenu; */
#endif

	void setup_menu(void);
public:
	GUI(int argc, char **argv, EMU *new_emu);
	virtual ~GUI();

#ifndef USE_SDL2
	virtual int CreateWidget(SDL_Surface *screen, int width, int height);
#else
	virtual int CreateWidget(SDL_Window *window, int width, int height);
#endif
	virtual int CreateMenu();
	virtual void ShowMenu();
	virtual void HideMenu();
	virtual void PreProcessEvent();
	virtual int ProcessEvent(SDL_Event *e);

	virtual bool NeedUpdateScreen();
	virtual void DecreaseUpdateScreenCount();
	virtual void ScreenModeChanged(bool fullscreen);

	virtual void SetFocusToMainWindow();

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

	virtual bool ShowLoadStateDialog(void);
	virtual bool ShowSaveStateDialog(bool cont);

	virtual bool ShowOpenAutoKeyDialog(void);
	virtual bool StartAutoKey(void);

	virtual bool ShowPlayRecKeyDialog(void);
	virtual bool ShowRecordRecKeyDialog(void);

	virtual bool ShowSavePrinterDialog(int drv);

	virtual bool ShowRecordVideoDialog(int fps_num);
	virtual bool ShowRecordAudioDialog(void);
	virtual bool ShowRecordVideoAndAudioDialog(int fps_num);

	virtual bool ShowVolumeDialog(void);

	virtual bool ShowJoySettingDialog(void);
	virtual bool ShowKeybindDialog(void);
	virtual bool ShowConfigureDialog(void);
	virtual bool ShowLoggingDialog(void);
	virtual bool IsShownLoggingDialog(void);

	virtual bool ShowVirtualKeyboard(void);

	virtual void GoWindowMode(void);

#ifdef USE_MOUSE
	virtual void ToggleUseMouse(void);
#endif
};

#endif /* GUI_TYPE_COCOA */

void remove_window_menu(void);
void translate_apple_menu(void);
void set_delegate_to_sdl_window(GUI_BASE *new_gui);

#ifdef __OBJC__
/// SDL1 doesn't support drag and drop operation.
/// override SDL_QuartzWindowDelegate for drag and drop
/// see SDL/src/video/quartz/SDL_QuartsWindow.h
///
/// SDL2 doesn't support drag and drop operation.
/// override Cocoa_WindowListener for drag and drop
/// see SDL/src/video/cocoa/SDL_cocoawindow.h
@interface CocoaWindowDelegate : NSObject <NSWindowDelegate>
{
	id parentObject;
	GUI_BASE *guiObject;
}
@property (assign) id parentObject;
@property (assign) GUI_BASE *guiObject;
+ (id)allocWithParent:(id)new_obj gui:(GUI_BASE *)new_gui;
@end
#endif /* __OBJC__ */

#endif /* COCOA_GUI_H */
