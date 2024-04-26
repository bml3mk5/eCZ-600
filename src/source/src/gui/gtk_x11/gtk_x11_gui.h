/** @file gtk_x11_gui.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.05 -

	@brief [ gui for gtk x11 ]
*/

#ifndef GTK_X11_GUI_H
#define GTK_X11_GUI_H

#include <vector>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "gtk_keysym_compat.h"
#include "../gui_base.h"
#include "gtk_dialogbox.h"
#include "../../common.h"
#include "../../rec_video_defs.h"
#ifdef USE_GTK
#include <cairo/cairo.h>
#endif

class GUI;

#ifdef GUI_TYPE_GTK_X11

typedef void (*CbActivate)(GtkWidget *, gpointer);
typedef void (*CbShow)(GtkWidget *, gpointer);

namespace GUI_GTK_X11
{
class VolumeBox;
class ConfigBox;
class KeybindBox;
class RecVideoBox;
class RecAudioBox;
class FileBox;
class JoySettingBox;
class LoggingBox;
}; /* namespace GUI_GTK_X11 */

/**
	@brief Callback parameter
*/
class CB_PARAM
{
public:
	EMU *emu;
	GUI *gui;
	int type;
	int drv;
	int num;
	CB_PARAM();
	CB_PARAM(EMU *new_emu, GUI *new_gui, int new_type, int new_drv, int new_num);
	~CB_PARAM();
	void Set(EMU *new_emu, GUI *new_gui, int new_type, int new_drv, int new_num);
};

/**
	@brief GUI class
*/
class GUI : public GUI_BASE, public GUI_GTK_X11::DialogBox
{
private:
	Window sdl_native_window;

	//
	int view_width;
	int view_height;
	int view_top;

	GdkWindow *sdl_window;
	GtkWidget *widget_window;
	GtkWidget *menubar;
	GtkAccelGroup *accel_group;

	GUI_GTK_X11::VolumeBox *volumebox;
	GUI_GTK_X11::ConfigBox *configbox;
	GUI_GTK_X11::KeybindBox *keybindbox;
	GUI_GTK_X11::JoySettingBox *joysetbox;
	GUI_GTK_X11::RecVideoBox *recvidbox;
	GUI_GTK_X11::RecAudioBox *recaudbox;
	GUI_GTK_X11::FileBox *filebox;
	GUI_GTK_X11::LoggingBox *loggingbox;
	CB_PARAM filebox_param;
	bool filebox_cont;

	/// menu
	bool now_menu;
	bool now_menuloop;

	static void cb_realize(GtkWidget *widget, gpointer user_data);

	static void OnUpdateSubmenu(GtkWidget *widget, gpointer user_data);
	static void OnUpdateItemInSubmenu(GtkWidget *widget, gpointer user_data);
	static void OnSelectRecentFile(GtkWidget *widget, gpointer user_data);
	static void OnUpdateRecentFile(GtkWidget *widget, gpointer user_data);
	static void OnUpdateMultiVolumeList(GtkWidget *widget, gpointer user_data);
	static void OnSelectMultiVolumeItem(GtkWidget *widget, gpointer user_data);
	static void OnUpdateMultiVolumeItem(GtkWidget *widget, gpointer user_data);
	static void RemoveAllItems(GtkWidget *container);
	static void OnRemoveItem(GtkWidget *item, gpointer user_data);

	static void OnExit(GtkWidget *widget, gpointer user_data);

	static void OnSelectForceReset(GtkWidget *widget, gpointer user_data);
	static void OnSelectSpecialReset(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSpecialReset(GtkWidget *widget, gpointer user_data);
	static void OnSelectPowerOn(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePowerOn(GtkWidget *widget, gpointer user_data);
	static void OnSelectDipswitch(GtkWidget *widget, gpointer user_data);
	static void OnUpdateDipswitch(GtkWidget *widget, gpointer user_data);
	static void OnSelectWarmReset(GtkWidget *widget, gpointer user_data);
	static void OnSelectInterrupt(GtkWidget *widget, gpointer user_data);
	static void OnSelectPause(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePause(GtkWidget *widget, gpointer user_data);
	static void OnSelectFddType(GtkWidget *widget, gpointer user_data);
	static void OnUpdateFddType(GtkWidget *widget, gpointer user_data);
	static void OnSelectCPUPower(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCPUPower(GtkWidget *widget, gpointer user_data);
	static void OnSelectSyncIRQ(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSyncIRQ(GtkWidget *widget, gpointer user_data);
#ifdef _MBS1
	static void OnSelectMemNoWait(GtkWidget *widget, gpointer user_data);
	static void OnUpdateMemNoWait(GtkWidget *widget, gpointer user_data);
#endif
	static void OnSelectOpenAutoKey(GtkWidget *widget, gpointer user_data);
	static void OnUpdateOpenAutoKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectStartAutoKey(GtkWidget *widget, gpointer user_data);
	static void OnUpdateStartAutoKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectStopAutoKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectPlayRecKey(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePlayRecKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectStopPlayingRecKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectRecordRecKey(GtkWidget *widget, gpointer user_data);
	static void OnUpdateRecordRecKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectStopRecordingRecKey(GtkWidget *widget, gpointer user_data);
	static void OnSelectLoadState(GtkWidget *widget, gpointer user_data);
	static void OnSelectSaveState(GtkWidget *widget, gpointer user_data);

	static void OnSelectOpenAutoKeyFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectSaveStateFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectLoadStateFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectLoadRecKeyFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectSaveRecKeyFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);

	//
#ifdef USE_DATAREC
	static void OnSelectLoadDataRec(GtkWidget *widget, gpointer user_data);
	static void OnUpdateLoadDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectSaveDataRec(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSaveDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectRewindDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectFastFowardDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectCloseDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectStopDataRec(GtkWidget *widget, gpointer user_data);
	static void OnSelectRealModeDataRec(GtkWidget *widget, gpointer user_data);
	static void OnUpdateRealModeDataRec(GtkWidget *widget, gpointer user_data);

	static void OnSelectLoadDataRecFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectSaveDataRecFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
#endif

	//
#ifdef USE_FD1
	static void OnSelectOpenFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnUpdateOpenFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectChangeSideFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnUpdateChangeSideFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectCloseFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectOpenBlankFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectWriteProtectFloppyDisk(GtkWidget *widget, gpointer user_data);
	static void OnUpdateWriteProtectFloppyDisk(GtkWidget *widget, gpointer user_data);

	static void OnSelectOpenFloppyFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectOpenBlankFloppyFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
#endif

	//
#ifdef USE_HD1
	static void OnSelectOpenHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnUpdateOpenHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectCloseHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectOpenBlankHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectWriteProtectHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnUpdateWriteProtectHardDisk(GtkWidget *widget, gpointer user_data);
	static void OnSelectHardDiskDeviceType(GtkWidget *widget, gpointer user_data);
	static void OnUpdateHardDiskDeviceType(GtkWidget *widget, gpointer user_data);

	static void OnSelectOpenHardDiskFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
	static void OnSelectOpenBlankHardDiskFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);
#endif

	//
	static void OnSelectFrameRate(GtkWidget *widget, gpointer user_data);
	static void OnUpdateFrameRate(GtkWidget *widget, gpointer user_data);
	static void OnSelectScreenRecordSize(GtkWidget *widget, gpointer user_data);
	static void OnUpdateScreenRecordSize(GtkWidget *widget, gpointer user_data);
	static void OnSelectScreenRecordFrameRate(GtkWidget *widget, gpointer user_data);
	static void OnUpdateScreenRecordFrameRate(GtkWidget *widget, gpointer user_data);
	static void OnSelectStopScreenRecord(GtkWidget *widget, gpointer user_data);
	static void OnSelectScreenCapture(GtkWidget *widget, gpointer user_data);

	static void OnSelectWindowMode(GtkWidget *widget, gpointer user_data);
	static void OnUpdateWindowMode(GtkWidget *widget, gpointer user_data);
	static void OnSelectStretchScreen(GtkWidget *widget, gpointer user_data);
	static void OnUpdateStretchScreen(GtkWidget *widget, gpointer user_data);
	static void OnSelectCutoutScreen(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCutoutScreen(GtkWidget *widget, gpointer user_data);

	static void OnSelectScreenMode(GtkWidget *widget, gpointer user_data);
	static void OnUpdateScreenMode(GtkWidget *widget, gpointer user_data);

	static void OnSelectPixelAspect(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePixelAspect(GtkWidget *widget, gpointer user_data);

	static void OnSelectScanLine(GtkWidget *widget, gpointer user_data);
	static void OnUpdateScanLine(GtkWidget *widget, gpointer user_data);
#ifdef USE_AFTERIMAGE
	static void OnSelectAfterImage(GtkWidget *widget, gpointer user_data);
	static void OnUpdateAfterImage(GtkWidget *widget, gpointer user_data);
#endif
#ifdef _MBS1
	static void OnSelectRGBType(GtkWidget *widget, gpointer user_data);
	static void OnUpdateRGBType(GtkWidget *widget, gpointer user_data);
#endif
#ifdef USE_KEEPIMAGE
	static void OnSelectKeepImage(GtkWidget *widget, gpointer user_data);
	static void OnUpdateKeepImage(GtkWidget *widget, gpointer user_data);
#endif
#ifdef _X68000
	static void OnSelectShowScreen(GtkWidget *widget, gpointer user_data);
	static void OnUpdateShowScreen(GtkWidget *widget, gpointer user_data);
#endif
#ifdef USE_OPENGL
	static void OnSelectUseOpenGL(GtkWidget *widget, gpointer user_data);
	static void OnUpdateUseOpenGL(GtkWidget *widget, gpointer user_data);
	static void OnSelectOpenGLFilter(GtkWidget *widget, gpointer user_data);
	static void OnUpdateOpenGLFilter(GtkWidget *widget, gpointer user_data);
#endif

	//
	static void OnSelectSoundVolume(GtkWidget *widget, gpointer user_data);
	static void OnSelectSoundStartRecord(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSoundStartRecord(GtkWidget *widget, gpointer user_data);
	static void OnSelectSoundStopRecord(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSoundStopRecord(GtkWidget *widget, gpointer user_data);
	static void OnSelectSoundRate(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSoundRate(GtkWidget *widget, gpointer user_data);
	static void OnSelectSoundLatency(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSoundLatency(GtkWidget *widget, gpointer user_data);

	//
	static void OnSelectSavePrinter(GtkWidget *widget, gpointer user_data);
	static void OnUpdateSavePrinter(GtkWidget *widget, gpointer user_data);
	static void OnSelectClearPrinter(GtkWidget *widget, gpointer user_data);
	static void OnUpdateClearPrinter(GtkWidget *widget, gpointer user_data);
	static void OnSelectPrintPrinter(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePrintPrinter(GtkWidget *widget, gpointer user_data);
	static void OnSelectDirectPrinter(GtkWidget *widget, gpointer user_data);
	static void OnUpdateDirectPrinter(GtkWidget *widget, gpointer user_data);
	static void OnSelectPrinterOnline(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePrinterOnline(GtkWidget *widget, gpointer user_data);

	static void OnSelectSavePrinterFileBox(GUI_GTK_X11::FileBox *fbox, bool rc, void *data);

	static void OnSelectCommServer(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCommServer(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCommConnectMenu(GtkWidget *widget, gpointer user_data);
	static void OnSelectCommConnect(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCommConnect(GtkWidget *widget, gpointer user_data);
	static void OnSelectCommThroughMode(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCommThroughMode(GtkWidget *widget, gpointer user_data);
	static void OnSelectCommBinaryMode(GtkWidget *widget, gpointer user_data);
	static void OnUpdateCommBinaryMode(GtkWidget *widget, gpointer user_data);
	static void OnSelectCommSendTelnetCommand(GtkWidget *widget, gpointer user_data);

	//
	static void OnSelectLedBox(GtkWidget *widget, gpointer user_data);
	static void OnUpdateLedBox(GtkWidget *widget, gpointer user_data);
	static void OnSelectInsideLed(GtkWidget *widget, gpointer user_data);
	static void OnUpdateInsideLed(GtkWidget *widget, gpointer user_data);
	static void OnSelectMsgBoard(GtkWidget *widget, gpointer user_data);
	static void OnUpdateMsgBoard(GtkWidget *widget, gpointer user_data);
#ifdef USE_PERFORMANCE_METER
	static void OnSelectPMeter(GtkWidget *widget, gpointer user_data);
	static void OnUpdatePMeter(GtkWidget *widget, gpointer user_data);
#endif
	static void OnSelectUseJoypad(GtkWidget *widget, gpointer user_data);
	static void OnUpdateUseJoypad(GtkWidget *widget, gpointer user_data);
#ifdef USE_KEY2JOYSTICK
	static void OnSelectEnableKey2Joypad(GtkWidget *widget, gpointer user_data);
	static void OnUpdateEnableKey2Joypad(GtkWidget *widget, gpointer user_data);
#endif
	static void OnSelectJoySetting(GtkWidget *widget, gpointer user_data);
#ifdef USE_LIGHTPEN
	static void OnSelectEnableLightpen(GtkWidget *widget, gpointer user_data);
	static void OnUpdateEnableLightpen(GtkWidget *widget, gpointer user_data);
#endif
#ifdef USE_MOUSE
	static void OnSelectEnableMouse(GtkWidget *widget, gpointer user_data);
	static void OnUpdateEnableMouse(GtkWidget *widget, gpointer user_data);
#endif
	static void OnSelectLoosenKeyStroke(GtkWidget *widget, gpointer user_data);
	static void OnUpdateLoosenKeyStroke(GtkWidget *widget, gpointer user_data);
	static void OnSelectKeybindBox(GtkWidget *widget, gpointer user_data);
	static void OnSelectConfigureBox(GtkWidget *widget, gpointer user_data);
	static void OnSelectVirtualKeyboard(GtkWidget *widget, gpointer user_data);
	static void OnUpdateVirtualKeyboard(GtkWidget *widget, gpointer user_data);
	static void OnSelectLoggingBox(GtkWidget *widget, gpointer user_data);
	static void OnUpdateLoggingBox(GtkWidget *widget, gpointer user_data);
#ifdef USE_DEBUGGER
	static void OnSelectOpenDebugger(GtkWidget *widget, gpointer user_data);
	static void OnUpdateOpenDebugger(GtkWidget *widget, gpointer user_data);
	static void OnSelectCloseDebugger(GtkWidget *widget, gpointer user_data);
#endif

	static void OnSelectAbout(GtkWidget *widget, gpointer user_data);

#ifdef USE_GTK
	static void OnUserCommand(GtkWidget *widget, gint id, gpointer user_data);
#endif
	static void OnModifyMenuOpenFlag(GtkWidget *widget, gpointer user_data);
	static gboolean OnDragDrop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time_, gpointer user_data);
	static void OnDragDataReceived(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time_, gpointer user_data);

	//
	GtkWidget *create_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_check_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_check_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_radio_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_radio_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show = NULL, int drv = 0, int num = 0, guint key = 0);
	GtkWidget *create_separator_menu(GtkWidget *menu);
	GtkWidget *create_sub_menu(GtkWidget *menu, const char *label, CbShow cb_show = OnUpdateSubmenu, int drv = 0, int num = 0);
	GtkWidget *create_sub_menu(GtkWidget *menu, CMsg::Id labelid, CbShow cb_show = OnUpdateSubmenu, int drv = 0, int num = 0);
	GtkWidget *create_recent_menu(GtkWidget *menu, int type, int drv);
	GtkWidget *create_recent_menu_item(GtkWidget *menu, const char *label, int type = 0, int drv = 0, int num = 0);
	GtkWidget *create_multi_volume_menu(GtkWidget *menu, int drv);
	GtkWidget *create_multi_volume_menu_item(GtkWidget *menu, const char *label, int drv, int num);
	GtkWidget *create_comm_connect_menu(GtkWidget *menu, int drv);
	GtkWidget *create_comm_connect_menu_item(GtkWidget *menu, const char *label, int drv, int num);
	inline void add_accelerator(GtkWidget *menu_item, guint key);

	void modify_menu_open_flag(GtkWidget *menu, bool val);

public:
	GUI(int argc, char **argv, EMU *new_emu);
	virtual ~GUI();

#ifdef GUI_USE_FOREIGN_WINDOW
	virtual void *CreateWindow(int width, int height);
	virtual void *GetWindowData();
#endif
	virtual GtkWidget *GetWindow();

#if defined(USE_GTK)
	virtual int CreateWidget(GtkWidget *window, int width, int height);
#elif !defined(USE_SDL2)
	virtual int CreateWidget(SDL_Surface *screen, int width, int height);
#else
	virtual int CreateWidget(SDL_Window *window, int width, int height);
#endif

	virtual int CreateMenu();

	virtual void ShowMenu();
	virtual void HideMenu();

	virtual int ProcessEvent(SDL_Event *e);
	virtual int ProcessEventExtra(SDL_Event *e);
	virtual void PostProcessEvent();

	virtual void ScreenModeChanged(bool fullscreen);
#ifdef USE_GTK
	virtual void UpdateScreen(cairo_t *);
#ifdef USE_OPENGL
	virtual void UpdateScreen(GdkGLContext *);
#endif
	/// post to GTK signal
	virtual void PostCommandMessage(int id, void *data1 = NULL, void *data2 = NULL);
#endif

	virtual bool ShowAboutDialog();
	virtual bool ShowRecordVideoDialog(int fps_num);
	virtual bool ShowRecordAudioDialog();
	virtual bool ShowRecordVideoAndAudioDialog(int fps_num);
#ifdef USE_EMU_INHERENT_SPEC
	virtual bool ShowJoySettingDialog();
	virtual bool ShowConfigureDialog();
	virtual bool ShowKeybindDialog();
	virtual bool ShowVolumeDialog();
	virtual bool ShowLoggingDialog();
	virtual bool IsShownLoggingDialog();
#endif
#ifdef USE_DATAREC
	virtual bool ShowLoadDataRecDialog();
	virtual bool ShowSaveDataRecDialog();
	virtual void set_datarec_file_menu(uint32_t uItem);
#endif

#ifdef USE_FD1
	virtual bool ShowOpenFloppyDiskDialog(int drv);
	void set_disk_file_menu(uint32_t uItem, int drv);
	virtual bool ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type);
#ifdef USE_EMU_INHERENT_SPEC
	void set_disk_side_menu(uint32_t uItem, int drv);
#endif
#endif

#ifdef USE_HD1
	virtual bool ShowOpenHardDiskDialog(int drv);
	virtual bool ShowOpenBlankHardDiskDialog(int drv, uint8_t type);
	virtual bool ShowSelectHardDiskDeviceTypeDialog(int drv);
#endif

#ifdef USE_AUTO_KEY
	virtual bool ShowOpenAutoKeyDialog();
	virtual bool StartAutoKey(void);
#endif

#ifdef USE_EMU_INHERENT_SPEC
	virtual bool ShowSaveStateDialog();
	virtual bool ShowLoadStateDialog();
#ifdef USE_KEY_RECORD
	virtual bool ShowPlayRecKeyDialog();
	virtual bool ShowRecordRecKeyDialog();
	virtual bool ShowRecordStateAndRecKeyDialog();
#endif
#endif

#ifdef USE_PRINTER
	virtual bool ShowSavePrinterDialog(int drv);
#endif

	virtual bool OpenDroppedFile(void *param);

	virtual bool ShowVirtualKeyboard(void);

	enum en_device_type {
		DATAREC = 0,
		FLOPPY,
		HARDDISK,
		STATE
	};
};

#endif /* GUI_TYPE_GTK_X11 */

#endif /* GTK_X11_GUI_H */
