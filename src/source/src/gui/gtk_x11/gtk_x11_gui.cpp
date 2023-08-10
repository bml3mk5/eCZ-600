/** @file gtk_x11_gui.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.05 -

	@brief [ gui for gtk+ x11 ]
*/

#include "gtk_x11_gui.h"
#include <SDL_syswm.h>
#include <gdk/gdkkeysyms.h>
#include "../../config.h"
#include "../../emu.h"
#include "../../emumsg.h"
#include "gtk_configbox.h"
#include "gtk_filebox.h"
#include "gtk_volumebox.h"
#include "gtk_keybindbox.h"
#include "gtk_joysetbox.h"
#include "gtk_recvidbox.h"
#include "gtk_recaudbox.h"
#include "gtk_aboutbox.h"
#include "../../depend.h"
#include "../../utility.h"
#include "../../labels.h"
#include "../../msgs.h"
#include "../../res/resource.h"
#ifdef USE_GTK
#include "../../osd/gtk/gtk_main.h"
#endif
#ifdef USE_VKEYBOARD
#include "gtk_vkeyboard.h"
#endif

using namespace GUI_GTK_X11;

#ifdef GUI_TYPE_GTK_X11

static bool first_menu_showed = false;

int check_supported_file(const _TCHAR *file_path);

#define SKIP_WHEN_MENU_OPENING(widget) { \
	if (g_object_get_data(G_OBJECT(widget), "menu-open") != NULL) { \
		return; \
	} \
}

#define TRIM_STRING_SIZE	72

CB_PARAM::CB_PARAM()
{
	emu = NULL;
	gui = NULL;
	type = 0;
	drv = 0;
	num = 0;
}
CB_PARAM::CB_PARAM(EMU *new_emu, GUI *new_gui, int new_type, int new_drv, int new_num)
{
	emu = new_emu;
	gui = new_gui;
	type = new_type;
	drv = new_drv;
	num = new_num;
}
CB_PARAM::~CB_PARAM()
{
}
void CB_PARAM::Set(EMU *new_emu, GUI *new_gui, int new_type, int new_drv, int new_num)
{
	emu = new_emu;
	gui = new_gui;
	type = new_type;
	drv = new_drv;
	num = new_num;
}

/****************************************

 GUI for windows class

 ****************************************/

GUI::GUI(int argc, char **argv, EMU *new_emu)
 : GUI_BASE(argc, argv, new_emu), DialogBox(this)
{
#if !defined(USE_GTK)
	gtk_init(&argc, &argv);

#if GTK_CHECK_VERSION(3,8,0)
	GtkApplication *app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);	
#elif GTK_CHECK_VERSION(2,28,0)
	GApplication *app = g_application_new(NULL, G_APPLICATION_FLAGS_NONE);	
#endif
#endif

	sdl_native_window = 0;
	view_width = 0;
	view_height = 0;
	view_top = 0;
	sdl_window = NULL;
	widget_window = NULL;
	menubar = NULL;

	now_menu = false;
	now_menuloop = false;

	volumebox = new VolumeBox(this);
	configbox = new ConfigBox(this);
	keybindbox = new KeybindBox(this);
	joysetbox = new JoySettingBox(this);
	recvidbox = new RecVideoBox(this);
	recaudbox = new RecAudioBox(this);
	filebox = new FileBox();
	filebox_cont = false;
}

GUI::~GUI()
{
	if (menubar) g_object_unref(G_OBJECT(menubar));

	delete volumebox;
	delete configbox;
	delete keybindbox;
	delete joysetbox;
	delete recvidbox;
	delete recaudbox;
	delete filebox;
}

#if defined(USE_GTK)
int GUI::CreateWidget(GtkWidget *window, int width, int height)
{
	return 0;
}
#elif !defined(USE_SDL2)
int GUI::CreateWidget(SDL_Surface *screen, int width, int height)
{
	view_width = width;
	view_height = height;

	if (sdl_native_window == 0) {

		SDL_SysWMinfo winfo;
		SDL_VERSION(&winfo.version);

		SDL_GetWMInfo(&winfo);
		logging->out_debugf("SDL: window:%lx  fswindow:%lx  wmwindow:%lx"
			,winfo.info.x11.window
			,winfo.info.x11.fswindow
			,winfo.info.x11.wmwindow);

		sdl_native_window = winfo.info.x11.wmwindow;

#ifndef GUI_USE_FOREIGN_WINDOW
		GdkDisplay *display = gdk_display_get_default();
		if (display == NULL) {
			logging->out_log(LOG_ERROR, "gdk_display_get_default error.\n");
			return 1;
		}

		//
		// create GdkWindow using SDL native window
		//
#if GTK_CHECK_VERSION(2,24,0)
		sdl_window = gdk_x11_window_foreign_new_for_display(display, sdl_native_window);
#else
		sdl_window = gdk_window_foreign_new_for_display(display, sdl_native_window);
#endif

#if 1
		// catch events on GTK+
		GdkEventMask event_masks = gdk_window_get_events(sdl_window);
		event_masks = (GdkEventMask)((int)GDK_EXPOSURE_MASK |
					   GDK_BUTTON_MOTION_MASK |
					   GDK_BUTTON_PRESS_MASK |
					   GDK_BUTTON_RELEASE_MASK |
					   GDK_ENTER_NOTIFY_MASK |
					   GDK_LEAVE_NOTIFY_MASK |
					   GDK_FOCUS_CHANGE_MASK |
//					GDK_KEY_PRESS_MASK |
//					GDK_KEY_RELEASE_MASK |
//		       GDK_POINTER_MOTION_MASK |
					   GDK_STRUCTURE_MASK
		);
		gdk_window_set_events(sdl_window, event_masks);
		event_masks = gdk_window_get_events(sdl_window);

		logging->out_debugf("Event masks: %x\n", event_masks);
#endif

#if 0
		GdkRGBA rgba;
		rgba.red = 0.8;
		rgba.green = 0.8;
		rgba.blue = 0.8;
		rgba.alpha = 1.0;
		gdk_window_set_background_rgba(sdl_window, &rgba);
#endif

#if 0
		GdkScreen *screen1 = gdk_screen_get_default();
		if (screen1 == NULL) {
			fprintf(stderr, "gdk_screen_get_default error.\n");
			return 0;
		}
#endif
#endif /* !GUI_USE_FOREIGN_WINDOW */
	}
	return 0;
}
#else /* USE_SDL2 */
int GUI::CreateWidget(SDL_Window *window, int width, int height)
{
	view_width = width;
	view_height = height;

	if (sdl_native_window == 0) {

		SDL_SysWMinfo winfo;
		SDL_VERSION(&winfo.version);

		SDL_GetWindowWMInfo(window, &winfo);
		logging->out_debugf("SDL: window:%lx" ,winfo.info.x11.window);
		sdl_native_window = winfo.info.x11.window;

#ifndef GUI_USE_FOREIGN_WINDOW
		GdkDisplay *display = gdk_display_get_default();
		if (display == NULL) {
			logging->out_log(LOG_ERROR, "gdk_display_get_default error.\n");
			return 1;
		}

		//
		// create GdkWindow using SDL native window
		//
#if GTK_CHECK_VERSION(2,24,0)
		sdl_window = gdk_x11_window_foreign_new_for_display(display, sdl_native_window);
#else
		sdl_window = gdk_window_foreign_new_for_display(display, sdl_native_window);
#endif

#if 1
		// catch events on GTK+
		GdkEventMask event_masks = gdk_window_get_events(sdl_window);
		event_masks = (GdkEventMask)((int)GDK_EXPOSURE_MASK |
					   GDK_BUTTON_MOTION_MASK |
					   GDK_BUTTON_PRESS_MASK |
					   GDK_BUTTON_RELEASE_MASK |
					   GDK_ENTER_NOTIFY_MASK |
					   GDK_LEAVE_NOTIFY_MASK |
					   GDK_FOCUS_CHANGE_MASK |
//					GDK_KEY_PRESS_MASK |
//					GDK_KEY_RELEASE_MASK |
//		       GDK_POINTER_MOTION_MASK |
					   GDK_STRUCTURE_MASK
		);
		gdk_window_set_events(sdl_window, event_masks);
		event_masks = gdk_window_get_events(sdl_window);

		logging->out_debugf("Event masks: %x\n", event_masks);
#endif

#if 0
		GdkRGBA rgba;
		rgba.red = 0.8;
		rgba.green = 0.8;
		rgba.blue = 0.8;
		rgba.alpha = 1.0;
		gdk_window_set_background_rgba(sdl_window, &rgba);
#endif

#if 0
		GdkScreen *screen1 = gdk_screen_get_default();
		if (screen1 == NULL) {
			fprintf(stderr, "gdk_screen_get_default error.\n");
			return 0;
		}
#endif
#endif /* !GUI_USE_FOREIGN_WINDOW */
	}
	return 0;
}
#endif

#ifdef GUI_USE_FOREIGN_WINDOW
void *GUI::CreateWindow(int width, int height)
{
	char id[32];

	view_width = width;
	view_height = height;

	widget_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if (widget_window == NULL) {
		logging->out_log(LOG_ERROR, "gtk_window_new error.\n");
		return NULL;
	}
	gtk_widget_set_size_request(widget_window, view_width, view_height);

	g_signal_connect(G_OBJECT(widget_window), "destroy", G_CALLBACK(OnExit), (gpointer)this);

	gtk_widget_realize(widget_window);

	GdkWindow *gdk_window = gtk_widget_get_window(widget_window);
	GdkEventMask event_masks = gdk_window_get_events(gdk_window);
	event_masks = (GdkEventMask)((int)GDK_EXPOSURE_MASK |
				   GDK_BUTTON_MOTION_MASK |
				   GDK_BUTTON_PRESS_MASK |
				   GDK_BUTTON_RELEASE_MASK |
				   GDK_ENTER_NOTIFY_MASK |
				   GDK_LEAVE_NOTIFY_MASK |
				   GDK_FOCUS_CHANGE_MASK |
//					GDK_KEY_PRESS_MASK |
//					GDK_KEY_RELEASE_MASK |
//		       GDK_POINTER_MOTION_MASK |
				   GDK_STRUCTURE_MASK
	);
	gdk_window_set_events(gdk_window, event_masks);

	gtk_widget_show_all(widget_window);

#ifdef USE_SDL
	// set environment to attach SDL
	sprintf(id,"%ld",
		GDK_WINDOW_XID(gdk_window));
	g_print("env: %s\n", id);
	setenv("SDL_WINDOWID",id,1);
#endif

	// process gtk event to show real window at first
	while (gtk_events_pending()) {
		gtk_main_iteration();
	}
	return (void *)GDK_WINDOW_XID(gdk_window);
}

void *GUI::GetWindowData()
{
	void *data = NULL;
	if (widget_window) {
		GdkWindow *gdk_window = gtk_widget_get_window(widget_window);
		data = (void *)GDK_WINDOW_XID(gdk_window);
	}
	return data;
}
#endif

GtkWidget *GUI::GetWindow()
{
	return widget_window;
}

#ifdef USE_GTK
void GUI::UpdateScreen(cairo_t *cr)
{
	((EMU_OSD *)emu)->update_screen_pa(cr);
	need_update_screen = 6;
	g_need_update_screen = false;
}

#ifdef USE_OPENGL
void GUI::UpdateScreen(GdkGLContext *context)
{
	((EMU_OSD *)emu)->update_screen_gl(context);
	need_update_screen = 6;
	g_need_update_screen = false;
}
#endif

#endif

int GUI::CreateMenu()
{
	// create new user defined signal for menu item
	g_signal_new("user-show",
		  GTK_TYPE_MENU_ITEM,
		  G_SIGNAL_ACTION,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

#ifndef USE_GTK

	// Create Menu using GtkWidget
#if !defined(GUI_USE_FOREIGN_WINDOW)
	/* !GUI_USE_FOREIGN_WINDOW */
#if GTK_CHECK_VERSION(3,0,0) && !defined(NO_ATTACH_SDL)
	widget_window = gtk_offscreen_window_new();
//	widget_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#else
	widget_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
	if (widget_window == NULL) {
		logging->out_log(LOG_ERROR, "gtk_window_new error.\n");
		return 1;
	}

	gtk_widget_set_size_request(widget_window, view_width, view_height);

	g_signal_connect(G_OBJECT(widget_window), "realize", G_CALLBACK(GUI::cb_realize), (gpointer)sdl_window);
	g_signal_connect(G_OBJECT(widget_window), "destroy", G_CALLBACK(OnExit), (gpointer)this);

	gtk_widget_realize(widget_window);
#else
	/* GUI_USE_FOREIGN_WINDOW */
	if (widget_window == NULL) {
		logging->out_log(LOG_ERROR, "gtk_window_new error.\n");
		return 1;
	}
#endif /* GUI_USE_FOREIGN_WINDOW */

	// set icon
	char buf[_MAX_PATH];
	sprintf(buf, "%s%s.bmp", emu->resource_path(), CONFIG_NAME);
	gtk_window_set_default_icon_from_file(buf, NULL);

#else /* !USE_GTK */

	widget_window = ((EMU_OSD *)emu)->get_window();

	// create new user defined signal
	g_signal_new("user-command",
		  GTK_TYPE_WIDGET,
		  G_SIGNAL_RUN_LAST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__INT,
		  G_TYPE_NONE, 1, G_TYPE_INT);

	g_signal_connect(G_OBJECT(widget_window), "user-command",
		G_CALLBACK(OnUserCommand), (gpointer)this);

#endif /* USE_GTK */

	//

	GtkWidget *vbox;
	vbox = create_vbox(NULL, 0);

	gtk_container_add(GTK_CONTAINER(widget_window), vbox);

	GtkWidget *menu;
	GtkWidget *submenu;
	GtkWidget *subsubmenu;
//	GtkWidget *item;
//	GSList *group = NULL;
	char name[128];

	menubar = gtk_menu_bar_new();
	g_object_ref(G_OBJECT(menubar));

	accel_group = gtk_accel_group_new();

	menu = create_sub_menu(menubar, CMsg::Control);
	{
		create_check_menu_item(menu, CMsg::Power_Switch, OnSelectSpecialReset, OnUpdateSpecialReset, 0, 0, GDK_KEY_F3);
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Power_Off_Forcely);
		{
			create_menu_item(submenu, CMsg::Do_it, OnSelectForceReset, NULL, 0, 0, 0);
		}
		create_separator_menu(menu);
		create_menu_item(menu, CMsg::Reset_Switch, OnSelectWarmReset, NULL, 0, 0, GDK_KEY_R);
		create_menu_item(menu, CMsg::Interrupt_Switch, OnSelectInterrupt, NULL, 7);
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Pause, OnSelectPause, OnUpdatePause, 0, 0, GDK_KEY_Q);
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::CPU_Speed);
		{
			create_radio_menu_item(submenu, CMsg::CPU_x0_5, OnSelectCPUPower, OnUpdateCPUPower, 0, 0, GDK_KEY_9);
			create_radio_menu_item(submenu, CMsg::CPU_x1, OnSelectCPUPower, OnUpdateCPUPower, 0, 1, GDK_KEY_1);
			create_radio_menu_item(submenu, CMsg::CPU_x2, OnSelectCPUPower, OnUpdateCPUPower, 0, 2, GDK_KEY_2);
			create_radio_menu_item(submenu, CMsg::CPU_x4, OnSelectCPUPower, OnUpdateCPUPower, 0, 3, GDK_KEY_3);
			create_radio_menu_item(submenu, CMsg::CPU_x8, OnSelectCPUPower, OnUpdateCPUPower, 0, 4, GDK_KEY_4);
			create_radio_menu_item(submenu, CMsg::CPU_x16, OnSelectCPUPower, OnUpdateCPUPower, 0, 5, GDK_KEY_5);
			create_separator_menu(submenu);
			create_check_menu_item(submenu, CMsg::Sync_With_CPU_Speed, OnSelectSyncIRQ, OnUpdateSyncIRQ, 0, 0, GDK_KEY_0);
		}
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Auto_Key);
		{
			create_menu_item(submenu, CMsg::Open_, OnSelectOpenAutoKey, OnUpdateOpenAutoKey);
			create_menu_item(submenu, CMsg::Paste, OnSelectStartAutoKey, OnUpdateStartAutoKey);
			create_menu_item(submenu, CMsg::Stop, OnSelectStopAutoKey);
		}
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Record_Key);
		{
			create_check_menu_item(submenu, CMsg::Play_, OnSelectPlayRecKey, OnUpdatePlayRecKey, 0, 0, GDK_KEY_E);
			create_menu_item(submenu, CMsg::Stop_Playing, OnSelectStopPlayingRecKey);
			create_separator_menu(submenu);
			create_check_menu_item(submenu, CMsg::Record_, OnSelectRecordRecKey, OnUpdateRecordRecKey);
			create_menu_item(submenu, CMsg::Stop_Recording, OnSelectStopRecordingRecKey);
		}
		create_separator_menu(menu);
		create_menu_item(menu, CMsg::Load_State_, OnSelectLoadState, NULL, 0, 0, GDK_KEY_O);
		create_menu_item(menu, CMsg::Save_State_, OnSelectSaveState);
		create_separator_menu(menu);
		create_recent_menu(menu, STATE, 0);
		create_separator_menu(menu);
		create_menu_item(menu, CMsg::Exit_, OnExit, NULL, 0, 0, GDK_KEY_F4);
	}
#ifdef USE_DATAREC
	menu = create_sub_menu(menubar, CMsg::Tape);
	{
		create_check_menu_item(menu, CMsg::Play_, OnSelectLoadDataRec, OnUpdateLoadDataRec, 0, 0, GDK_KEY_F7);
		create_check_menu_item(menu, CMsg::Rec_, OnSelectSaveDataRec, OnUpdateSaveDataRec);
		create_menu_item(menu, CMsg::Eject, OnSelectCloseDataRec);
		create_separator_menu(menu);
		create_menu_item(menu, CMsg::Rewind, OnSelectRewindDataRec, NULL, 0, 0, GDK_KEY_F5);
		create_menu_item(menu, CMsg::F_F_, OnSelectFastFowardDataRec, NULL, 0, 0, GDK_KEY_F8);
		create_menu_item(menu, CMsg::Stop, OnSelectStopDataRec, NULL, 0, 0, GDK_KEY_F6);
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Real_Mode, OnSelectRealModeDataRec, OnUpdateRealModeDataRec);
		create_separator_menu(menu);
		create_recent_menu(menu, DATAREC, 0);
	}
#endif
	for(int drv=0; drv<USE_DRIVE; drv++) {
		sprintf(name,CMSG(FDDVDIGIT),drv);
		menu = create_sub_menu(menubar, name);
		{
			create_check_menu_item(menu, CMsg::Insert_, OnSelectOpenFloppyDisk, OnUpdateOpenFloppyDisk, drv, 0, GDK_KEY_F9 + drv);
			create_menu_item(menu, CMsg::Eject, OnSelectCloseFloppyDisk, NULL, drv);
			submenu = create_sub_menu(menu, CMsg::New);
			{
				create_menu_item(submenu, CMsg::Insert_Blank_2HD_, OnSelectOpenBlankFloppyDisk, NULL, drv, 0x20);
			}
			create_separator_menu(menu);
			create_check_menu_item(menu, CMsg::Write_Protect, OnSelectWriteProtectFloppyDisk, OnUpdateWriteProtectFloppyDisk, drv);
			create_separator_menu(menu);
			create_multi_volume_menu(menu, drv);
			create_separator_menu(menu);
			create_recent_menu(menu, FLOPPY, drv);
		}
	}
	menu = create_sub_menu(menubar, CMsg::HDD);
	{
		for(int drv=0; drv<MAX_HARD_DISKS; drv++) {
			sprintf(name,CMSG(SASIVDIGIT),drv);
			submenu = create_sub_menu(menu, name);
			{
				create_check_menu_item(submenu, CMsg::Mount_, OnSelectOpenHardDisk, OnUpdateOpenHardDisk, drv, 0);
				create_menu_item(submenu, CMsg::Unmount, OnSelectCloseHardDisk, NULL, drv);
				subsubmenu = create_sub_menu(submenu, CMsg::New);
				{
					create_menu_item(subsubmenu, CMsg::Mount_Blank_10MB_, OnSelectOpenBlankHardDisk, NULL, drv, 0);
					create_menu_item(subsubmenu, CMsg::Mount_Blank_20MB_, OnSelectOpenBlankHardDisk, NULL, drv, 1);
					create_menu_item(subsubmenu, CMsg::Mount_Blank_40MB_, OnSelectOpenBlankHardDisk, NULL, drv, 2);
				}
				create_separator_menu(submenu);
				create_recent_menu(submenu, HARDDISK, drv);
			}
		}
	}
	menu = create_sub_menu(menubar, CMsg::Screen);
	{
		submenu = create_sub_menu(menu, CMsg::Frame_Rate);
		{
			create_radio_menu_item(submenu, CMsg::Auto, OnSelectFrameRate, OnUpdateFrameRate, 0, -1);
			create_radio_menu_item(submenu, CMsg::F_1_1fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 0);
			create_radio_menu_item(submenu, CMsg::F_1_2fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 1);
			create_radio_menu_item(submenu, CMsg::F_1_3fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 2);
			create_radio_menu_item(submenu, CMsg::F_1_4fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 3);
			create_radio_menu_item(submenu, CMsg::F_1_5fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 4);
			create_radio_menu_item(submenu, CMsg::F_1_6fps, OnSelectFrameRate, OnUpdateFrameRate, 0, 5);
		}
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Record_Screen);
		{
			for(int i = 0; i < 2; i++) {
				GetRecordVideoSizeStr(i, name);
				create_radio_menu_item(submenu, name, OnSelectScreenRecordSize, OnUpdateScreenRecordSize, 0, i);
			}
			create_separator_menu(submenu);
#ifdef USE_REC_VIDEO
			create_radio_menu_item(submenu, CMsg::Rec_60fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 0);
			create_radio_menu_item(submenu, CMsg::Rec_30fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 1);
			create_radio_menu_item(submenu, CMsg::Rec_20fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 2);
			create_radio_menu_item(submenu, CMsg::Rec_15fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 3);
			create_radio_menu_item(submenu, CMsg::Rec_12fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 4);
			create_radio_menu_item(submenu, CMsg::Rec_10fps, OnSelectScreenRecordFrameRate, OnUpdateScreenRecordFrameRate, 0, 5);
			create_menu_item(submenu, CMsg::Stop, OnSelectStopScreenRecord);
			create_separator_menu(submenu);
#endif
			create_menu_item(submenu, CMsg::Capture, OnSelectScreenCapture);
		}
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Window);
		{
			for(int i = 0; i < GetWindowModeCount(); i++) {
				GetWindowModeStr(i, name);
				create_radio_menu_item(submenu, name, OnSelectWindowMode, OnUpdateWindowMode, 0, i);
			}
		}
		submenu = create_sub_menu(menu, CMsg::Fullscreen);
		{
			create_check_menu_item(submenu, CMsg::Stretch_Screen, OnSelectStretchScreen, OnUpdateStretchScreen, 0, 1, GDK_KEY_X);
			create_check_menu_item(submenu, CMsg::Cutout_Screen, OnSelectCutoutScreen, OnUpdateCutoutScreen, 0, 2, GDK_KEY_X);
			create_separator_menu(submenu);
			for(int disp_no = 0; disp_no < GetDisplayDeviceCount(); disp_no++) {
				GetDisplayDeviceStr(CMSG(Display), disp_no, name);
				GtkWidget *ssmenu = create_sub_menu(submenu, name);
				for(int i = 0; i < GetFullScreenModeCount(disp_no); i++) {
					GetFullScreenModeStr(disp_no, i, name);
					create_radio_menu_item(ssmenu, name, OnSelectScreenMode, OnUpdateScreenMode, 0, disp_no * VIDEO_MODE_MAX + i);
				}
			}
		}
		submenu = create_sub_menu(menu, CMsg::Aspect_Ratio);
		{
			for(int i = 0; i < GetPixelAspectModeCount(); i++) {
				GetPixelAspectModeStr(i, name);
				create_radio_menu_item(submenu, name, OnSelectPixelAspect, OnUpdatePixelAspect, 0, i);
			}
		}
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Drawing_Mode);
			create_radio_menu_item(submenu, CMsg::Full_Draw, OnSelectScanLine, OnUpdateScanLine, 0, 0, GDK_KEY_S);
			create_radio_menu_item(submenu, CMsg::Scanline, OnSelectScanLine, OnUpdateScanLine, 0, 1, GDK_KEY_S);
			create_radio_menu_item(submenu, CMsg::Stripe, OnSelectScanLine, OnUpdateScanLine, 0, 2, GDK_KEY_S);
			create_radio_menu_item(submenu, CMsg::Stripe_Strongly, OnSelectScanLine, OnUpdateScanLine, 0, 3, GDK_KEY_S);
#ifdef USE_AFTERIMAGE
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Afterimage1, OnSelectAfterImage, OnUpdateAfterImage, 0, 1, GDK_KEY_T);
		create_check_menu_item(menu, CMsg::Afterimage2, OnSelectAfterImage, OnUpdateAfterImage, 0, 2, GDK_KEY_T);
#endif
#ifdef USE_KEEPIMAGE
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Keepimage1, OnSelectKeepImage, OnUpdateKeepImage, 0, 1);
		create_check_menu_item(menu, CMsg::Keepimage2, OnSelectKeepImage, OnUpdateKeepImage, 0, 2);
#endif
#ifdef _X68000
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Show_Screen);
			create_check_menu_item(submenu, CMsg::Graphic0, OnSelectShowScreen, OnUpdateShowScreen, 0, 1, GDK_KEY_1 + 0x100);
			create_check_menu_item(submenu, CMsg::Graphic1, OnSelectShowScreen, OnUpdateShowScreen, 0, 2, GDK_KEY_2 + 0x100);
			create_check_menu_item(submenu, CMsg::Graphic2, OnSelectShowScreen, OnUpdateShowScreen, 0, 3, GDK_KEY_3 + 0x100);
			create_check_menu_item(submenu, CMsg::Graphic3, OnSelectShowScreen, OnUpdateShowScreen, 0, 4, GDK_KEY_4 + 0x100);
			create_check_menu_item(submenu, CMsg::Graphic4, OnSelectShowScreen, OnUpdateShowScreen, 0, 5, GDK_KEY_5 + 0x100);
			create_check_menu_item(submenu, CMsg::Text, OnSelectShowScreen, OnUpdateShowScreen, 0, 6, GDK_KEY_6 + 0x100);
			create_check_menu_item(submenu, CMsg::Sprite, OnSelectShowScreen, OnUpdateShowScreen, 0, 7, GDK_KEY_7 + 0x100);
			create_check_menu_item(submenu, CMsg::BG0, OnSelectShowScreen, OnUpdateShowScreen, 0, 8, GDK_KEY_8 + 0x100);
			create_check_menu_item(submenu, CMsg::BG1, OnSelectShowScreen, OnUpdateShowScreen, 0, 9, GDK_KEY_9 + 0x100);
			create_separator_menu(submenu);
			create_menu_item(submenu, CMsg::Show_All, OnSelectShowScreen, OnUpdateShowScreen, 0, 0, GDK_KEY_0 + 0x100);
#endif
#ifdef USE_OPENGL
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Use_OpenGL_Sync, OnSelectUseOpenGL, OnUpdateUseOpenGL, 0, 1, GDK_KEY_Y);
		create_check_menu_item(menu, CMsg::Use_OpenGL_Async, OnSelectUseOpenGL, OnUpdateUseOpenGL, 0, 2, GDK_KEY_Y);
		submenu = create_sub_menu(menu, CMsg::OpenGL_Filter);
			create_radio_menu_item(submenu, CMsg::Nearest_Neighbour, OnSelectOpenGLFilter, OnUpdateOpenGLFilter, 0, 0, GDK_KEY_U);
			create_radio_menu_item(submenu, CMsg::Linear, OnSelectOpenGLFilter, OnUpdateOpenGLFilter, 0, 1, GDK_KEY_U);
#endif
	}
	menu = create_sub_menu(menubar, CMsg::Sound);
	{
		create_menu_item(menu, CMsg::Volume_, OnSelectSoundVolume);
		create_separator_menu(menu);
		submenu = create_sub_menu(menu, CMsg::Record_Sound);
		{
			create_menu_item(submenu, CMsg::Start_, OnSelectSoundStartRecord,  OnUpdateSoundStartRecord);
			create_menu_item(submenu, CMsg::Stop, OnSelectSoundStopRecord, OnUpdateSoundStopRecord);
		}
		create_separator_menu(menu);
//		create_radio_menu_item(menu, CMsg::F2000Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 0);
//		create_radio_menu_item(menu, CMsg::F4000Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 1);
		create_radio_menu_item(menu, CMsg::F8000Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 2);
		create_radio_menu_item(menu, CMsg::F11025Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 3);
		create_radio_menu_item(menu, CMsg::F22050Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 4);
		create_radio_menu_item(menu, CMsg::F44100Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 5);
		create_radio_menu_item(menu, CMsg::F48000Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 6);
		create_radio_menu_item(menu, CMsg::F96000Hz, OnSelectSoundRate, OnUpdateSoundRate, 0, 7);
		create_separator_menu(menu);
		create_radio_menu_item(menu, CMsg::S50msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 0);
		create_radio_menu_item(menu, CMsg::S75msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 1);
		create_radio_menu_item(menu, CMsg::S100msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 2);
		create_radio_menu_item(menu, CMsg::S200msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 3);
		create_radio_menu_item(menu, CMsg::S300msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 4);
		create_radio_menu_item(menu, CMsg::S400msec, OnSelectSoundLatency, OnUpdateSoundLatency, 0, 5);
	}
	menu = create_sub_menu(menubar, CMsg::Devices);
	{
		for(int drv = 0; drv < MAX_PRINTER; drv++) {
			sprintf(name,CMSG(Printer), drv);
			submenu = create_sub_menu(menu, name);
			{
				create_menu_item(submenu, CMsg::Save_, OnSelectSavePrinter, OnUpdateSavePrinter, drv);
				create_menu_item(submenu, CMsg::Print_to_mpprinter, OnSelectPrintPrinter, OnUpdatePrintPrinter, drv);
				create_menu_item(submenu, CMsg::Clear, OnSelectClearPrinter, NULL, drv);
				create_separator_menu(submenu);
				create_check_menu_item(submenu, CMsg::Online, OnSelectPrinterOnline, OnUpdatePrinterOnline, drv);
				create_separator_menu(submenu);
				create_check_menu_item(submenu, CMsg::Send_to_mpprinter_concurrently, OnSelectDirectPrinter, OnUpdateDirectPrinter, drv);
			}
		}
		create_separator_menu(menu);
		for(int drv = 0; drv < MAX_COMM; drv++) {
			sprintf(name,CMSG(Communication), drv);
			submenu = create_sub_menu(menu, name);
			{
				create_check_menu_item(submenu, CMsg::Enable_Server, OnSelectCommServer, OnUpdateCommServer, drv);
				create_sub_menu(submenu, CMsg::Connect, OnUpdateCommConnectMenu, drv);
				create_separator_menu(submenu);
				create_check_menu_item(submenu, CMsg::Comm_With_Byte_Data, OnSelectCommThroughMode, OnUpdateCommThroughMode, drv);
				create_separator_menu(submenu);
				subsubmenu = create_sub_menu(submenu, CMsg::Options_For_Telnet);
				{
					create_check_menu_item(subsubmenu, CMsg::Binary_Mode, OnSelectCommBinaryMode, OnUpdateCommBinaryMode, drv);
					create_separator_menu(subsubmenu);
					create_menu_item(subsubmenu, CMsg::Send_WILL_ECHO, OnSelectCommSendTelnetCommand, NULL, drv, 1);
				}
			}
		}
	}
	menu = create_sub_menu(menubar, CMsg::Options);
	{
		create_check_menu_item(menu, CMsg::Show_LED, OnSelectLedBox, OnUpdateLedBox, 0, 0, GDK_KEY_L);
#ifdef USE_OUTSIDE_LEDBOX
		create_check_menu_item(menu, CMsg::Inside_LED, OnSelectInsideLed, OnUpdateInsideLed, 0, 0, GDK_KEY_L);
#endif
		create_check_menu_item(menu, CMsg::Show_Message, OnSelectMsgBoard, OnUpdateMsgBoard, 0, 0, GDK_KEY_Z);
#ifdef USE_PERFORMANCE_METER
		create_check_menu_item(menu, CMsg::Show_Performance_Meter, OnSelectPMeter, OnUpdatePMeter, 0, 0, 0);
#endif
#ifdef USE_LIGHTPEN
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Enable_Lightpen, OnSelectEnableLightpen, OnUpdateEnableLightpen, 0, 0, GDK_KEY_Control_L);
#endif
#ifdef USE_MOUSE
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Enable_Mouse, OnSelectEnableMouse, OnUpdateEnableMouse, 0, 0, GDK_KEY_Control_L);
#endif
#ifdef USE_JOYSTICK
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Use_Joypad_Key_Assigned, OnSelectUseJoypad, OnUpdateUseJoypad, 0, 1, GDK_KEY_J);
#ifdef USE_PIAJOYSTICK
		create_check_menu_item(menu, CMsg::Use_Joypad_Port_Connected, OnSelectUseJoypad, OnUpdateUseJoypad, 0, 2, GDK_KEY_J);
#endif
#ifdef USE_KEY2JOYSTICK
		create_check_menu_item(menu, CMsg::Enable_Key_to_Joypad, OnSelectEnableKey2Joypad, OnUpdateEnableKey2Joypad, 0, 0, 0);
#endif
#endif
		create_separator_menu(menu);
		create_check_menu_item(menu, CMsg::Virtual_Keyboard_, OnSelectVirtualKeyboard, OnUpdateVirtualKeyboard, 0, 0, 0);
#ifdef USE_DEBUGGER
		create_separator_menu(menu);
		create_menu_item(menu, CMsg::Start_Debugger, OnSelectOpenDebugger, OnUpdateOpenDebugger, 0, 0, GDK_KEY_D);
		create_menu_item(menu, CMsg::Stop_Debugger, OnSelectCloseDebugger, NULL);
#endif
		create_separator_menu(menu);
#ifdef USE_JOYSTICK
		create_menu_item(menu, CMsg::Joypad_Setting_, OnSelectJoySetting, NULL, 0, 0, 0);
#endif
		create_menu_item(menu, CMsg::Keybind_, OnSelectKeybindBox, NULL, 0, 0, GDK_KEY_K);
		create_menu_item(menu, CMsg::Configure_, OnSelectConfigureBox, NULL, 0, 0, GDK_KEY_C);
	}
	menu = create_sub_menu(menubar, CMsg::Help);
	{
		create_menu_item(menu, CMsg::About_, OnSelectAbout);
	}

	/* set menubar on vbox */
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

#ifdef USE_GTK
	GtkWidget *screen = ((EMU_OSD *)emu)->get_screen();
	gtk_box_pack_start(GTK_BOX(vbox), screen, TRUE, TRUE, 0);
	g_object_ref(G_OBJECT(screen));

//	const GtkTargetEntry targets[] = {
//			{ "text/uri-list", 0, 0 },
//			{ "text/plain", 0, 1 }
//	};

	// connect drop signal
	g_signal_connect(G_OBJECT(widget_window), "drag-drop",
		G_CALLBACK(OnDragDrop), (gpointer)this);
	g_signal_connect(G_OBJECT(widget_window), "drag-data-received",
		G_CALLBACK(OnDragDataReceived), (gpointer)this);
	// enable drop on window
	gtk_drag_dest_set(widget_window, GTK_DEST_DEFAULT_ALL,
			NULL, 0, (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
	gtk_drag_dest_add_uri_targets(widget_window);
//	gtk_drag_dest_add_text_targets(widget_window);
#endif

//#if !defined(GUI_USE_FOREIGN_WINDOW)
	/* show window */
	gtk_widget_show_all(widget_window);
//#endif

	/* menubar height */
	GtkAllocation menubar_size;
	gtk_widget_get_allocation(menubar, &menubar_size);
	if (menubar_size.height > 1) {
//		g_print("on height: %d\n", menubar_size.height);
		view_top = menubar_size.height;
		first_menu_showed = true;
	} else {
//		g_print("off height: %d\n", menubar_size.height);
//		gtk_widget_hide(widget_window);
	}

//	gtk_window_resize(GTK_WINDOW(widget_window), view_width, view_height + view_top);
	emu->set_display_margin(0,view_top,0,0);

	// need resize window
	return 1;
}

void GUI::ShowMenu()
{
	if(!now_menu) {
		if (menubar) {
			gtk_widget_show(menubar);
			now_menu = true;
		}
	}
}

void GUI::HideMenu()
{
	if(now_menu) {
		if (menubar) {
			gtk_widget_hide(menubar);
			now_menu = false;
		}
	}
}

int GUI::ProcessEvent(SDL_Event *e)
{
#ifndef USE_GTK
//	if (e->type == SDL_SYSWMEVENT) {
//		SDL_SysWMmsg *msg = ((SDL_SysWMEvent *)e)->msg;
//		fprintf(stderr,"Event: %d\n", msg->event.xevent.type);
//	}
	return GUI_BASE::ProcessEvent(e);
#else
	return (exit_program ? -1 : 0);
#endif
}

int GUI::ProcessEventExtra(SDL_Event *e)
{
	return 1;
}

void GUI::PostProcessEvent()
{
	GUI_BASE::PostProcessEvent();

#ifndef USE_GTK
	int msg = 0;
//	logging->out_debug("GUI::PostProcessEvent() in");
	while (gtk_events_pending() && msg < 10) {
		gtk_main_iteration();
		msg++;
	}
#endif
//	logging->out_debug("GUI::PostProcessEvent() out");
	if (!first_menu_showed) {
//		GtkWidget *vbox = gtk_bin_get_child(GTK_BIN(widget_window));
//		gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
//		gtk_widget_show_all(widget_window);
//		gtk_widget_grab_focus(widget_window);
		first_menu_showed = true;
	}
}

void GUI::ScreenModeChanged(bool fullscreen)
{
	if (fullscreen) {
		HideMenu();
	} else {
		ShowMenu();
	}
}

#ifdef USE_GTK
/// post message to main thread
void GUI::PostCommandMessage(int id, void *data1, void *data2)
{
	g_signal_emit_by_name(G_OBJECT(widget_window), "user-command"
		, id
		, data1
		, data2
	);
}
#endif

#ifdef USE_EMU_INHERENT_SPEC
bool GUI::ShowJoySettingDialog()
{
	// create joypad setting dialog
	joysetbox->Show(widget_window);
	return true;
}

bool GUI::ShowConfigureDialog()
{
	// create configure dialog
	configbox->Show(widget_window);
	return true;
}

bool GUI::ShowKeybindDialog()
{
	// create keybind box
	keybindbox->Show(widget_window);
	return true;
}

bool GUI::ShowVolumeDialog()
{
	// create volume dialog
	volumebox->Show(widget_window);
	return true;
}
#endif
bool GUI::ShowRecordVideoDialog(int fps_num)
{
	// create record video dialog
#ifdef USE_REC_VIDEO
	recvidbox->Show(widget_window, fps_num, true);
#endif
	return true;
}
bool GUI::ShowRecordAudioDialog()
{
	// create record audio dialog
#ifdef USE_REC_AUDIO
	recaudbox->Show(widget_window, -1);
#endif
	return true;
}
// call by record video dialog
bool GUI::ShowRecordVideoAndAudioDialog(int fps_num)
{
	// create record audio dialog
#ifdef USE_REC_AUDIO
	recaudbox->Show(widget_window, fps_num);
#endif
	return true;
}

/// create about dialog
bool GUI::ShowAboutDialog()
{
//	SystemPause(true);
	AboutBox aboutbox(widget_window);
//	if(emu) SystemPause(false);
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
//		{ CMsg::Supported_Files_wav_cas_tap, CMsg::All_Files_, CMsg::End };
//#elif defined(DATAREC_MZT)
//		{ CMsg::Supported_Files_wav_cas_mzt_m12, CMsg::All_Files_, CMsg::End };
//#elif defined(USE_EMU_INHERENT_SPEC)
//		{ CMsg::Supported_Files_l3_l3b_l3c_wav_t9x, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_wav_cas),CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::datarec_exts;

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Play_Data_Recorder_Tape,
		pConfig->initial_datarec_path,
//		_T("l3"),
		false,
		NULL,
		OnSelectLoadDataRecFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectLoadDataRecFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtLoadDataRecMessage(fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
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

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Record_Data_Recorder_Tape,
		pConfig->initial_datarec_path,
//		_T("l3"),
		true,
		NULL,
		OnSelectSaveDataRecFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectSaveDataRecFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtSaveDataRecMessage(fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
}

void GUI::set_datarec_file_menu(uint32_t uItem)
{
}
#endif

#ifdef USE_FD1
bool GUI::ShowOpenFloppyDiskDialog(int drv)
{
//	const CMsg::Id filter[] =
//#ifndef USE_EMU_INHERENT_SPEC
//		{ CMsg::Supported_Files_d88_d77_td0_imd_dsk_fdi_hdm_tfd_xdf_2d_sf7, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_d88_td0_imd_dsk_fdi_hdm_tfd_xdf_2d_sf7, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::floppy_disk_exts;

	_TCHAR title[128];
	_stprintf(title, CMSG(Open_Floppy_Disk_VDIGIT), drv);

	SystemPause(true);
	filebox_param.Set(emu, this, 0, drv, 0);
	bool rc = filebox->Show(widget_window,
		filter,
		title,
		pConfig->initial_disk_path,
//		_T("d88"),
		false,
		NULL,
		OnSelectOpenFloppyFileBox,
		(void *)&filebox_param
	);
	return rc;
}
void GUI::OnSelectOpenFloppyFileBox(FileBox *fbox, bool rc, void *data)
{
	CB_PARAM *param = (CB_PARAM *)data;
	GUI *gui = param->gui;
	int drv = param->drv;
	if (rc) {
		gui->PostEtOpenFloppyMessage(drv, fbox->GetPath(), 0, 0, true);
	} else {
		gui->SystemPause(false);
	}
}

void GUI::set_disk_file_menu(uint32_t uItem, int drv)
{
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

	_TCHAR title[128];
	_stprintf(title, CMSG(New_Floppy_Disk_VDIGIT), drv);

	_TCHAR file_name[_MAX_PATH];
	UTILITY::create_date_file_path(NULL, file_name, _MAX_PATH, _T("d88"));

	SystemPause(true);
	filebox_param.Set(emu, this, 0, drv, type);
	bool rc = filebox->Show(widget_window,
		filter,
		title,
		pConfig->initial_disk_path,	
//		_T("d88"),
		true,
		file_name,
		OnSelectOpenBlankFloppyFileBox,
		(void *)&filebox_param
	);
	return rc;
}
void GUI::OnSelectOpenBlankFloppyFileBox(FileBox *fbox, bool rc, void *data)
{
	CB_PARAM *param = (CB_PARAM *)data;
	EMU *emu = param->emu;
	GUI *gui = param->gui;
	int drv = param->drv;
	int num = param->num;
	if(rc) {
		rc = emu->create_blank_floppy_disk(fbox->GetPath(), (uint8_t)num);
	}
	if (rc) {
		gui->PostEtOpenFloppyMessage(drv, fbox->GetPath(), 0, 0, true);
	} else {
		gui->SystemPause(false);
	}
}

#ifdef USE_EMU_INHERENT_SPEC
void GUI::set_disk_side_menu(uint32_t uItem, int drv)
{
}
#endif

#endif	// USE_FD1

#ifdef USE_HD1
bool GUI::ShowOpenHardDiskDialog(int drv)
{
//	const CMsg::Id filter[] =
//#ifndef USE_EMU_INHERENT_SPEC
//		{ CMsg::Supported_Files_hdf, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_hdf, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::hard_disk_exts;

	_TCHAR title[128];
	_stprintf(title, CMSG(Open_Hard_Disk_VDIGIT), drv);

	SystemPause(true);
	filebox_param.Set(emu, this, 0, drv, 0);
	bool rc = filebox->Show(widget_window,
		filter,
		title,
		pConfig->initial_hard_disk_path,
//		_T("hdf"),
		false,
		NULL,
		OnSelectOpenHardDiskFileBox,
		(void *)&filebox_param
	);
	return rc;
}
void GUI::OnSelectOpenHardDiskFileBox(FileBox *fbox, bool rc, void *data)
{
	CB_PARAM *param = (CB_PARAM *)data;
	GUI *gui = param->gui;
	int drv = param->drv;
	if (rc) {
		gui->PostEtOpenHardDiskMessage(drv, fbox->GetPath(), 0);
	} else {
		gui->SystemPause(false);
	}
}
bool GUI::ShowOpenBlankHardDiskDialog(int drv, uint8_t type)
{
//	const CMsg::Id filter[] =
//#ifndef USE_EMU_INHERENT_SPEC
//		{ CMsg::Supported_Files_hdf, CMsg::All_Files_, CMsg::End };
//#else
//		{ CMsg::Supported_Files_hdf, CMsg::All_Files_, CMsg::End };
//#endif
	const char *filter = LABELS::blank_hard_disk_exts;

	_TCHAR title[128];
	_stprintf(title, CMSG(New_Hard_Disk_VDIGIT), drv);

	_TCHAR file_name[_MAX_PATH];
	UTILITY::create_date_file_path(NULL, file_name, _MAX_PATH, _T("hdf"));

	SystemPause(true);
	filebox_param.Set(emu, this, 0, drv, type);
	bool rc = filebox->Show(widget_window,
		filter,
		title,
		pConfig->initial_hard_disk_path,	
//		_T("hdf"),
		true,
		file_name,
		OnSelectOpenBlankHardDiskFileBox,
		(void *)&filebox_param
	);
	return rc;
}
void GUI::OnSelectOpenBlankHardDiskFileBox(FileBox *fbox, bool rc, void *data)
{
	CB_PARAM *param = (CB_PARAM *)data;
	EMU *emu = param->emu;
	GUI *gui = param->gui;
	int drv = param->drv;
	int num = param->num;
	if(rc) {
		rc = emu->create_blank_hard_disk(fbox->GetPath(), (uint8_t)num);
	}
	if (rc) {
		gui->PostEtOpenHardDiskMessage(drv, fbox->GetPath(), 0);
	} else {
		gui->SystemPause(false);
	}
}
#endif	// USE_HD1

#ifdef USE_AUTO_KEY
bool GUI::ShowOpenAutoKeyDialog()
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_txt_bas_lpt, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::autokey_file_exts;

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Open_Text_File,
		pConfig->initial_autokey_path,
//		_T("txt"),
		false,
		NULL,
		OnSelectOpenAutoKeyFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectOpenAutoKeyFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtLoadAutoKeyMessage(fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
}
#endif

#ifdef USE_EMU_INHERENT_SPEC
bool GUI::ShowSaveStateDialog()
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_l3r, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::state_file_exts;

	SystemPause(true);
	pConfig->saved_state_path.Clear();
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Save_Status_Data,
		pConfig->initial_state_path,
//		_T("l3r"),
		true,
		NULL,
		OnSelectSaveStateFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectSaveStateFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtSaveStatusMessage(fbox->GetPath(), gui->filebox_cont);
		if (gui->filebox_cont) {
			// show Record RecKey Dialog
			gui->PostCommandMessage(ID_RECKEY_REC);
		}
		gui->filebox_cont = false;
	} else {
		gui->SystemPause(false);
	}
}

bool GUI::ShowLoadStateDialog()
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_l3r, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::state_file_exts;

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Load_Status_Data,
		pConfig->initial_state_path,
//		_T("l3r"),
		false,
		NULL,
		OnSelectLoadStateFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectLoadStateFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtLoadStatusMessage(fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
}

#ifdef USE_KEY_RECORD
bool GUI::ShowPlayRecKeyDialog()
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_l3k, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::key_rec_file_exts;

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Play_Recorded_Keys,
		pConfig->initial_state_path,
//		_T("l3k"),
		false,
		NULL,
		OnSelectLoadRecKeyFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectLoadRecKeyFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtLoadRecKeyMessage(fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
}

bool GUI::ShowRecordRecKeyDialog()
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_l3k, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::key_rec_file_exts;

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Record_Input_Keys,
		pConfig->initial_state_path,
//		_T("l3k"),
		true,
		NULL,
		OnSelectSaveRecKeyFileBox,
		(void *)this
	);
	return rc;
}
void GUI::OnSelectSaveRecKeyFileBox(FileBox *fbox, bool rc, void *data)
{
	GUI *gui = (GUI *)data;
	if (rc) {
		gui->PostEtSaveRecKeyMessage(fbox->GetPath(), false);
	} else {
		gui->SystemPause(false);
	}
}

bool GUI::ShowRecordStateAndRecKeyDialog()
{
	filebox_cont = true;
	ShowSaveStateDialog();
	return true;
}

#endif
#endif	// USE_EMU_INHERENT_SPEC

#ifdef USE_PRINTER
bool GUI::ShowSavePrinterDialog(int drv)
{
//	const CMsg::Id filter[] =
//		{ CMsg::Supported_Files_lpt, CMsg::All_Files_, CMsg::End };
	const char *filter = LABELS::printing_file_exts;

	SystemPause(true);
	filebox_param.Set(emu, this, 0, drv, 0);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Save_Printing_Data,
		pConfig->initial_printer_path,
//		_T("lpt"),
		true,
		NULL,
		OnSelectSavePrinterFileBox,
		(void *)&filebox_param
	);
	return rc;
}
void GUI::OnSelectSavePrinterFileBox(FileBox *fbox, bool rc, void *data)
{
	CB_PARAM *param = (CB_PARAM *)data;
	GUI *gui = param->gui;
	int drv = param->drv;
	if (rc) {
		gui->PostEtSavePrinterMessage(drv, fbox->GetPath());
	} else {
		gui->SystemPause(false);
	}
}
#endif

#ifdef USE_CART
void open_cart_dialog()
{
	const CMsg::Id filter[] =
#ifdef _X1TWIN
		{ CMsg::Supported_Files_pce, CMsg::All_Files_, CMsg::End };
#else
		{ CMsg::Supported_Files_rom_bin, CMsg::All_Files_, CMsg::End };
#endif

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
#ifdef _X1TWIN
		CMsg::HuCARD,
#else
		CMsg::Game_Cartridge,
#endif
		pConfig->initial_cart_path
	);
	if(rc) {
		UPDATE_HISTORY(path, pConfig->recent_cart_path);
		emu->open_cart(filebox->GetPath());
	}
	SystemPause(false);
}
#endif

#ifdef USE_QUICKDISK
void open_quickdisk_dialog()
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_mzt, CMsg::All_Files_, CMsg::End };

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Quick_Disk,
		pConfig->initial_quickdisk_path
	);
	if(rc) {
		UPDATE_HISTORY(path, pConfig->recent_quickdisk_path);
		emu->open_quickdisk(filebox->GetPath());
	}
	SystemPause(false);
}
#endif

#ifdef USE_MEDIA
void open_media_dialog()
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_m3u, CMsg::All_Files_, CMsg::End };

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
		CMsg::Sound_Cassette_Tape,
		pConfig->initial_media_path
	);
	if(rc) {
		UPDATE_HISTORY(path, pConfig->recent_media_path);
		emu->open_media(filebox->GetPath());
	}
	SystemPause(false);
}
#endif

#ifdef USE_BINARY_FILE1
void open_binary_dialog(int drv, bool load)
{
	const CMsg::Id filter[] =
		{ CMsg::Supported_Files_ram_bin, CMsg::All_Files_, CMsg::End };

	SystemPause(true);
	bool rc = filebox->Show(widget_window,
		filter,
#if defined(_PASOPIA) || defined(_PASOPIA7)
		CMsg::RAM_Pack_Cartridge,
#else
		CMsg::Memory_Dump,
#endif
		pConfig->initial_binary_path
	);
	if(rc) {
		UPDATE_HISTORY(path, pConfig->recent_binary_path[drv]);
		if(load) {
			emu->load_binary(drv, filebox->GetPath());
		}
		else {
			emu->save_binary(drv, filebox->GetPath());
		}
	}
	SystemPause(false);
}
#endif

/**
 *	open dropped file
 */
bool GUI::OpenDroppedFile(void *param)
{
	_TCHAR dropped_file[_MAX_PATH];
	const gchar *file_path = g_filename_from_uri((const char *)param, NULL, NULL);
	if (file_path) {
		UTILITY::strcpy(dropped_file, sizeof(dropped_file), file_path);
	} else {
		dropped_file[0] = _T('\0');
	}
	return OpenFileByExtention(dropped_file);
}

void GUI::cb_realize(GtkWidget *widget, gpointer user_data)
{
#ifndef GUI_USE_FOREIGN_WINDOW
	/* !GUI_USE_FOREIGN_WINDOW */
	GtkWindow *window = GTK_WINDOW(widget);
	GdkWindow *sdl_window = GDK_WINDOW(user_data);

	if (!window) return;

	GdkWindow *orig_window = gtk_widget_get_window(widget);

#ifdef _DEBUG
	Window x_win = 0;
	Window x_pwin = 0;
	Window x_twin = 0;
	if (orig_window) {
		x_win = gdk_x11_window_get_xid(orig_window);
		GdkWindow *wpa = gdk_window_get_parent(orig_window);
		if (wpa) x_pwin = gdk_x11_window_get_xid(wpa);
		GdkWindow *wtl = gdk_window_get_toplevel(orig_window);
		if (wtl) x_twin = gdk_x11_window_get_xid(wtl);
	}
	Window x_sdl_win = gdk_x11_window_get_xid(sdl_window);
	Window x_sdl_pwin = 0;
	Window x_sdl_twin = 0;
	GdkWindow *pa = gdk_window_get_parent(sdl_window);
	if (pa) {
		x_sdl_pwin = gdk_x11_window_get_xid(pa);
	}
	GdkWindow *ta = gdk_window_get_toplevel(sdl_window);
	if (ta) {
		x_sdl_twin = gdk_x11_window_get_xid(ta);
	}
	fprintf(stderr, "realize: orig:%lx p:%lx t:%lx sdl:%lx p:%lx t:%lx", x_win, x_pwin, x_twin, x_sdl_win, x_sdl_pwin, x_sdl_twin);
#endif

#ifndef NO_ATTACH_SDL
	// attach window built by SDL.
//	gtk_widget_set_visual(widget, gdk_window_get_visual(sdl_window));
	gtk_widget_set_window(widget, sdl_window);
#if GTK_CHECK_VERSION(3,8,0)
	gtk_widget_register_window(widget, sdl_window);
//	gtk_style_context_set_background(gtk_widget_get_style_context(widget), sdl_window);
#else
	gdk_window_set_user_data(sdl_window, (gpointer)widget);
#endif
//	gdk_window_set_decorations(sdl_window, GDK_DECOR_ALL);
//	gdk_window_set_decorations(sdl_window, GDK_DECOR_MENU);
#if 0
	if (gtk_window_get_accept_focus(window))
		gdk_window_set_accept_focus(sdl_window, TRUE);
	else
		gdk_window_set_accept_focus(sdl_window, FALSE);
#endif
	if (gtk_window_get_focus_on_map(window))
		gdk_window_set_focus_on_map(sdl_window, TRUE);
	else
		gdk_window_set_focus_on_map(sdl_window, FALSE);

	if (orig_window != NULL) {
#if GTK_CHECK_VERSION(3,8,0)
		gtk_widget_unregister_window(widget, orig_window);
#else
		gdk_window_set_user_data(orig_window, NULL);
#endif
		gdk_window_destroy(orig_window);
	}
#endif /* NO_ATTACH_SDL */
#endif /* !GUI_USE_FOREIGN_WINDOW */
}

//
//
//
//
//
GtkWidget *GUI::create_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	GtkWidget *item;
	item = gtk_menu_item_new_with_mnemonic(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	add_accelerator(item, key);

	if (cb_activate != NULL) {
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(cb_activate), (gpointer)this);
	}
	if (cb_show != NULL) {
		g_signal_connect(G_OBJECT(item), "user-show", G_CALLBACK(cb_show), (gpointer)this);
	}
	g_object_set_data(G_OBJECT(item), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(item), "num", (gpointer)(intptr_t)num);
	g_object_set_data(G_OBJECT(item), "menu-open", NULL);
	return item;
}
GtkWidget *GUI::create_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	const char *label = gMessages.Get(labelid);
	return create_menu_item(menu, label, cb_activate, cb_show, drv, num, key);
}

//
GtkWidget *GUI::create_check_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	GtkWidget *item;
	item = gtk_check_menu_item_new_with_mnemonic(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	add_accelerator(item, key);

	if (cb_activate != NULL) {
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(cb_activate), (gpointer)this);
	}
	if (cb_show != NULL) {
		g_signal_connect(G_OBJECT(item), "user-show", G_CALLBACK(cb_show), (gpointer)this);
	}
	g_object_set_data(G_OBJECT(item), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(item), "num", (gpointer)(intptr_t)num);
	g_object_set_data(G_OBJECT(item), "menu-open", NULL);
	return item;
}
GtkWidget *GUI::create_check_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	const char *label = gMessages.Get(labelid);
	return create_check_menu_item(menu, label, cb_activate, cb_show, drv, num, key);
}

//
GtkWidget *GUI::create_radio_menu_item(GtkWidget *menu, const char *label, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	GtkWidget *item = create_check_menu_item(menu, label, cb_activate, cb_show, drv, num, key);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
	return item;
}
GtkWidget *GUI::create_radio_menu_item(GtkWidget *menu, CMsg::Id labelid, CbActivate cb_activate, CbShow cb_show, int drv, int num, guint key)
{
	const char *label = gMessages.Get(labelid);
	return create_radio_menu_item(menu, label, cb_activate, cb_show, drv, num, key);
}

//
GtkWidget *GUI::create_separator_menu(GtkWidget *menu)
{
	GtkWidget *item;
	item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menu), item);
	return item;
}

//
GtkWidget *GUI::create_sub_menu(GtkWidget *menu, const char *label, CbShow cb_show, int drv, int num)
{
	GtkWidget *item;
	GtkWidget *submenu;
	item = gtk_menu_item_new_with_mnemonic(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	g_object_set_data(G_OBJECT(submenu), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(submenu), "num", (gpointer)(intptr_t)num);
	g_signal_connect(G_OBJECT(submenu), "show", G_CALLBACK(cb_show), (gpointer)this);
	return submenu;
}
GtkWidget *GUI::create_sub_menu(GtkWidget *menu, CMsg::Id labelid, CbShow cb_show, int drv, int num)
{
	const char *label = gMessages.Get(labelid);
	return create_sub_menu(menu, label, cb_show, drv, num);
}

//
GtkWidget *GUI::create_recent_menu(GtkWidget *menu, int type, int drv)
{
	return create_sub_menu(menu
		, type == STATE ? CMsg::Recent_State_Files : CMsg::Recent_Files
		, OnUpdateRecentFile, drv, type);
}
GtkWidget *GUI::create_recent_menu_item(GtkWidget *menu, const char *label, int type, int drv, int num)
{
	GtkWidget *item;
	item = gtk_menu_item_new_with_label(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(OnSelectRecentFile), (gpointer)this);
	g_object_set_data(G_OBJECT(item), "type", (gpointer)(intptr_t)type);
	g_object_set_data(G_OBJECT(item), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(item), "num", (gpointer)(intptr_t)num);
	g_object_set_data(G_OBJECT(item), "menu-open", NULL);
	return item;
}
GtkWidget *GUI::create_multi_volume_menu(GtkWidget *menu, int drv)
{
	return create_sub_menu(menu, CMsg::Multi_Volume, OnUpdateMultiVolumeList, drv);
}
GtkWidget *GUI::create_multi_volume_menu_item(GtkWidget *menu, const char *label, int drv, int num)
{
	GtkWidget *item;
	item = gtk_check_menu_item_new_with_label(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(OnSelectMultiVolumeItem), (gpointer)this);
	g_signal_connect(G_OBJECT(item), "show", G_CALLBACK(OnUpdateMultiVolumeItem), (gpointer)this);
	g_object_set_data(G_OBJECT(item), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(item), "num", (gpointer)(intptr_t)num);
	return item;
}
GtkWidget *GUI::create_comm_connect_menu(GtkWidget *menu, int drv)
{
	char buf[128];
	int uarts = EnumUarts();

	RemoveAllItems(menu);
	create_comm_connect_menu_item(menu, CMSG(Ethernet), drv, 0);
	if (uarts > 0) {
		create_separator_menu(menu);
	}
	for(int i=0; i<uarts; i++) {
		GetUartDescription(i, buf, sizeof(buf));
		create_comm_connect_menu_item(menu, buf, drv, i + 1);
	}
	modify_menu_open_flag(menu, false);
	return menu;
}
GtkWidget *GUI::create_comm_connect_menu_item(GtkWidget *menu, const char *label, int drv, int num)
{
	GtkWidget *item = gtk_check_menu_item_new_with_label(label);
	gtk_container_add(GTK_CONTAINER(menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(OnSelectCommConnect), (gpointer)this);
	g_signal_connect(G_OBJECT(item), "show", G_CALLBACK(OnUpdateCommConnect), (gpointer)this);
	g_object_set_data(G_OBJECT(item), "drv", (gpointer)(intptr_t)drv);
	g_object_set_data(G_OBJECT(item), "num", (gpointer)(intptr_t)num);
	g_object_set_data(G_OBJECT(item), "menu-open", (gpointer)1);
	gtk_widget_show(item);
	return item;
}
void GUI::add_accelerator(GtkWidget *menu_item, guint key)
{
	if (key != 0) {
		int mod = GDK_MOD1_MASK;
		if (key >= 0x100 && key < 0x200) {
			key -= 0x100;
			mod |= GDK_SHIFT_MASK;
		}
		gtk_widget_add_accelerator(menu_item, "activate", accel_group, key, (GdkModifierType)mod, GTK_ACCEL_VISIBLE);
	}
}
void GUI::modify_menu_open_flag(GtkWidget *menu, bool val)
{
	gtk_container_foreach(GTK_CONTAINER(menu), OnModifyMenuOpenFlag, (gpointer)val);
}
void GUI::OnModifyMenuOpenFlag(GtkWidget *item, gpointer user_data)
{
	bool val = (bool)user_data;
	g_object_set_data(G_OBJECT(item), "menu-open", val ? (gpointer)1 : NULL);
}

//
//
//
//
//
void GUI::OnUpdateSubmenu(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkWidget *submenu = widget;
	if (submenu == NULL) return;

	gtk_container_foreach(GTK_CONTAINER(submenu),OnUpdateItemInSubmenu,(gpointer)gui);
}
void GUI::OnUpdateItemInSubmenu(GtkWidget *widget, gpointer user_data)
{
//printf("OnUpdateItemInSubmenu in\n");
	g_object_set_data(G_OBJECT(widget), "menu-open", (gpointer)1);
	g_signal_emit_by_name(G_OBJECT(widget), "user-show");
	g_object_set_data(G_OBJECT(widget), "menu-open", NULL);
//printf("OnUpdateItemInSubmenu out\n");
}
void GUI::OnSelectRecentFile(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int type = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"type");
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	switch(type) {
#ifdef USE_DATAREC
		case DATAREC:
			gui->PostEtLoadDataRecMessage(pConfig->recent_datarec_path[num]->path);
			break;
#endif
#ifdef USE_FD1
		case FLOPPY:
			gui->PostEtOpenFloppyMessage(drv, pConfig->recent_disk_path[drv][num]->path, 0, 0, true);
			break;
#endif
#ifdef USE_HD1
		case HARDDISK:
			gui->PostEtOpenHardDiskMessage(drv, pConfig->recent_hard_disk_path[drv][num]->path, 0);
			break;
#endif
		case STATE:
			gui->PostEtLoadStatusMessage(pConfig->recent_state_path[num]->path);
			break;
	}
}
void GUI::OnUpdateRecentFile(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int type = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkWidget *submenu = widget;
	if (submenu == NULL) return;

	GtkWidget *item = NULL;
	char path[_MAX_PATH+8];
	const char *file = NULL;
	bool flag = false;
	int bank_num = 0;
	int max_history = 0;
	switch(type) {
#ifdef USE_DATAREC
	case DATAREC:
		max_history = pConfig->recent_datarec_path.Count();
		break;
#endif
#ifdef USE_FD1
	case FLOPPY:
		max_history = pConfig->recent_disk_path[drv].Count();
		break;
#endif
#ifdef USE_HD1
	case HARDDISK:
		max_history = pConfig->recent_hard_disk_path[drv].Count();
		break;
#endif
	case STATE:
		max_history = pConfig->recent_state_path.Count();
		break;
	}

	RemoveAllItems(submenu);
	for(int num=0; num<max_history; num++) {
		switch(type) {
#ifdef USE_DATAREC
		case DATAREC:
			file = pConfig->recent_datarec_path[num]->path;
			break;
#endif
#ifdef USE_FD1
		case FLOPPY:
			file = pConfig->recent_disk_path[drv][num]->path;
			bank_num = pConfig->recent_disk_path[drv][num]->num;
			break;
#endif
#ifdef USE_HD1
		case HARDDISK:
			file = pConfig->recent_hard_disk_path[drv][num]->path;
			break;
#endif
		case STATE:
			file = pConfig->recent_state_path[num]->path;
			break;
		}
		if (file == NULL || file[0] == '\0') break;

		UTILITY::conv_from_native_path(file, path, _MAX_PATH);
		strcpy(path, UTILITY::trim_center(path, 64));
		if (bank_num > 0) {
			sprintf(&path[strlen(path)], " : %d", bank_num + 1);
		}
		item = gui->create_recent_menu_item(submenu, path, type, drv, num);
		gtk_widget_show(item);

		flag = true;
	}
	if (!flag) {
		item = gui->create_menu_item(submenu, CMsg::None_, NULL);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);
	}
}
void GUI::OnUpdateMultiVolumeList(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkWidget *submenu = widget;
	if (submenu == NULL) return;

	GtkWidget *item = NULL;
	char name[32];
	bool flag = false;

	RemoveAllItems(submenu);
	D88File *d88_file = gui->GetD88File(drv);
	int bank_nums = d88_file->GetBanks().Count();
	for(int num=0; num<bank_nums; num++) {
		const D88Bank *d88_bank = d88_file->GetBank(num);
		sprintf(name, "%02d:%s"
			, num+1
			, d88_bank->GetNameLength() > 0 ? d88_bank->GetName() : CMSG(no_label));

		item = gui->create_multi_volume_menu_item(submenu, name, drv, num);
		gtk_widget_show(item);
		flag = true;
	}
	if (bank_nums == 1) {
		gtk_widget_set_sensitive(item, FALSE);
	}
	if (!flag) {
		item = gui->create_menu_item(submenu, CMsg::None_, NULL);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);
	}
}
void GUI::OnSelectMultiVolumeItem(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int bank_num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtOpenFloppySelectedVolume(drv, bank_num);
}
void GUI::OnUpdateMultiVolumeItem(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int bank_num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkWidget *item = widget;
	D88File *d88_file = gui->GetD88File(drv);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), bank_num == d88_file->GetCurrentBank());
}

void GUI::RemoveAllItems(GtkWidget *container)
{
	gtk_container_foreach(GTK_CONTAINER(container), OnRemoveItem, (gpointer)container);
}
void GUI::OnRemoveItem(GtkWidget *item, gpointer user_data)
{
	GtkWidget *menu = GTK_WIDGET(user_data);
	gtk_container_remove(GTK_CONTAINER(menu), item);
}
//
//
//
//
//
void GUI::OnExit(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	gui->Exit();
}

void GUI::OnSelectForceReset(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtForceReset();
}
void GUI::OnSelectSpecialReset(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtSpecialReset();
}
void GUI::OnUpdateSpecialReset(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowSpecialReset() ? TRUE : FALSE);
}

void GUI::OnSelectPowerOn(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtReset();
}
void GUI::OnUpdatePowerOn(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowPowerOff() ? FALSE : TRUE);
}

void GUI::OnSelectDipswitch(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int bit = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->Dipswitch(bit);
}
void GUI::OnUpdateDipswitch(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int bit = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, (gui->GetDipswitch() & (1 << bit)) ? TRUE : FALSE);
}
void GUI::OnSelectWarmReset(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtWarmReset(-1);
}

void GUI::OnSelectInterrupt(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtInterrupt(num);
}

void GUI::OnSelectPause(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->TogglePause();
}
void GUI::OnUpdatePause(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;
	gtk_check_menu_item_set_active(item, gui->NowPause());
}
void GUI::OnSelectFddType(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

//printf("OnSelectFddType %d\n",num);
	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeFddType(num);
}
void GUI::OnUpdateFddType(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

//printf("OnUpdateFddType %d\n",num);
	gtk_check_menu_item_set_active(item, gui->NextFddType() == num);
}
void GUI::OnSelectCPUPower(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

//printf("OnSelectCPUPower %d\n",num);
	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtCPUPower(num);
}
void GUI::OnUpdateCPUPower(GtkWidget *widget, gpointer user_data)
{
//	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

//printf("OnUpdateCPUPower %d\n",num);
	gtk_check_menu_item_set_active(item, pConfig->cpu_power == num);
}
void GUI::OnSelectSyncIRQ(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtToggleSyncIRQ();
}
void GUI::OnUpdateSyncIRQ(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowSyncIRQ());
}
#ifdef _MBS1
void GUI::OnSelectMemNoWait(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleMemNoWait();
}
void GUI::OnUpdateMemNoWait(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowMemNoWait());
}
#endif
void GUI::OnSelectOpenAutoKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowOpenAutoKeyDialog();
}
void GUI::OnUpdateOpenAutoKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	gtk_widget_set_sensitive(widget, gui->IsRunningAutoKey() ? FALSE : TRUE);
}
void GUI::OnSelectStartAutoKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->StartAutoKey();
}
void GUI::OnUpdateStartAutoKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	gtk_widget_set_sensitive(widget, gui->IsRunningAutoKey() ? FALSE : TRUE);
}
void GUI::OnSelectStopAutoKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtStopAutoKeyMessage();
}
void GUI::OnSelectPlayRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowPlayRecKeyDialog();
}
void GUI::OnUpdatePlayRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowPlayingRecKey());
}
void GUI::OnSelectStopPlayingRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->StopPlayRecKey();
}
void GUI::OnSelectRecordRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
//	gui->ShowRecordRecKeyDialog();
	gui->ShowRecordStateAndRecKeyDialog();
}
void GUI::OnUpdateRecordRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowRecordingRecKey());
}
void GUI::OnSelectStopRecordingRecKey(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->StopRecordRecKey();
}
void GUI::OnSelectLoadState(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowLoadStateDialog();
}
void GUI::OnSelectSaveState(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowSaveStateDialog();
}

//
#ifdef USE_DATAREC
void GUI::OnSelectLoadDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowLoadDataRecDialog();
}
void GUI::OnUpdateLoadDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsOpenedLoadDataRecFile());
}
void GUI::OnSelectSaveDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowSaveDataRecDialog();
}
void GUI::OnUpdateSaveDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsOpenedSaveDataRecFile());
}
void GUI::OnSelectRewindDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtRewindDataRecMessage();
}
void GUI::OnSelectFastFowardDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtFastForwardDataRecMessage();
}
void GUI::OnSelectCloseDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtCloseDataRecMessage();
}
void GUI::OnSelectStopDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtStopDataRecMessage();
}
void GUI::OnSelectRealModeDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtToggleRealModeDataRecMessage();
}
void GUI::OnUpdateRealModeDataRec(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowRealModeDataRec());
}
#endif /* USE_DATAREC */

//
#ifdef USE_FD1
void GUI::OnSelectOpenFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowOpenFloppyDiskDialog(drv);
}
void GUI::OnUpdateOpenFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->InsertedFloppyDisk(drv));
}
void GUI::OnSelectChangeSideFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtChangeSideFloppyDisk(drv);
}
void GUI::OnUpdateChangeSideFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	int side = gui->GetSideFloppyDisk(drv);
	if (side) {
		gtk_menu_item_set_label(GTK_MENU_ITEM(item), CMSG(Change_Side_to_A));
	} else {
		gtk_menu_item_set_label(GTK_MENU_ITEM(item), CMSG(Change_Side_to_B));
	}
}
void GUI::OnSelectCloseFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtCloseFloppyMessage(drv);
}
void GUI::OnSelectOpenBlankFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowOpenBlankFloppyDiskDialog(drv, (uint8_t)num);
}
void GUI::OnSelectWriteProtectFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtToggleWriteProtectFloppyDisk(drv);
}
void GUI::OnUpdateWriteProtectFloppyDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->WriteProtectedFloppyDisk(drv));
	gtk_widget_set_sensitive(GTK_WIDGET(item), gui->InsertedFloppyDisk(drv));
}
#endif /* USE_FD1 */

//
#ifdef USE_HD1
void GUI::OnSelectOpenHardDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowOpenHardDiskDialog(drv);
}
void GUI::OnUpdateOpenHardDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->MountedHardDisk(drv));
}
void GUI::OnSelectCloseHardDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtCloseHardDiskMessage(drv);
}
void GUI::OnSelectOpenBlankHardDisk(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowOpenBlankHardDiskDialog(drv, (uint8_t)num);
}
#endif /* USE_HD1 */

//
void GUI::OnSelectFrameRate(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeFrameRate(num);
}
void GUI::OnUpdateFrameRate(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetFrameRateNum() == num);
}
void GUI::OnSelectScreenRecordSize(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtResizeRecordVideoSurface(num);
}
void GUI::OnUpdateScreenRecordSize(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetRecordVideoSurfaceNum() == num);
	gtk_widget_set_sensitive(GTK_WIDGET(item), !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
void GUI::OnSelectScreenRecordFrameRate(GtkWidget *widget, gpointer user_data)
{
#ifdef USE_REC_VIDEO
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowRecordVideoDialog(num);
#endif
}
void GUI::OnUpdateScreenRecordFrameRate(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetRecordVideoFrameNum() == num);
	gtk_widget_set_sensitive(GTK_WIDGET(item), !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
void GUI::OnSelectStopScreenRecord(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtStopRecordVideo();
}
void GUI::OnSelectScreenCapture(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtCaptureScreen();
}
void GUI::OnSelectWindowMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeWindowMode(num);
}
void GUI::OnUpdateWindowMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetWindowMode() == num);
}
void GUI::OnSelectStretchScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeStretchScreen(1);
}
void GUI::OnUpdateStretchScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetStretchScreen() == 1);
}
void GUI::OnSelectCutoutScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeStretchScreen(2);
}
void GUI::OnUpdateCutoutScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetStretchScreen() == 2);
}
void GUI::OnSelectScreenMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeFullScreenMode(num);
}
void GUI::OnUpdateScreenMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetFullScreenMode() == num);
	gtk_widget_set_sensitive(GTK_WIDGET(item), gui->IsFullScreen() == 0);
}
void GUI::OnSelectPixelAspect(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangePixelAspect(num);
}
void GUI::OnUpdatePixelAspect(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetPixelAspectMode() == num);
}
void GUI::OnSelectScanLine(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtChangeDrawMode(num);
}
void GUI::OnUpdateScanLine(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetDrawMode() == num);
}
#ifdef USE_AFTERIMAGE
void GUI::OnSelectAfterImage(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtChangeAfterImage(num);
}
void GUI::OnUpdateAfterImage(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetAfterImageMode() == num);
}
#endif
#ifdef _MBS1
void GUI::OnSelectRGBType(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeRGBType(num);
}
void GUI::OnUpdateRGBType(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetRGBTypeMode() == num);
}
#endif
#ifdef USE_KEEPIMAGE
void GUI::OnSelectKeepImage(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtChangeKeepImage(num);
}
void GUI::OnUpdateKeepImage(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetKeepImageMode() == num);
}
#endif
#ifdef _X68000
void GUI::OnSelectShowScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	if (num > 0) {
		num--;
		gui->ToggleShowScreen(1 << num);
	} else {
		gui->ToggleShowScreen(0);
	}
}
void GUI::OnUpdateShowScreen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	if (num > 0) {
		num--;
		gtk_check_menu_item_set_active(item, (gui->GetShowScreen() & (1 << num)) != 0);
	}
}
#endif
#ifdef USE_OPENGL
void GUI::OnSelectUseOpenGL(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeUseOpenGL(num);
}
void GUI::OnUpdateUseOpenGL(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetOpenGLMode() == num);
}
void GUI::OnSelectOpenGLFilter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeOpenGLFilter(num);
}
void GUI::OnUpdateOpenGLFilter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetOpenGLFilter() == num);
}
#endif

//
void GUI::OnSelectSoundVolume(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowVolumeDialog();
}
void GUI::OnSelectSoundStartRecord(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowRecordAudioDialog();
}
void GUI::OnUpdateSoundStartRecord(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_widget_set_sensitive(GTK_WIDGET(item), !(gui->NowRecordingVideo() | gui->NowRecordingSound()));
}
void GUI::OnSelectSoundStopRecord(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtStopRecordSound();
}
void GUI::OnUpdateSoundStopRecord(GtkWidget *widget, gpointer user_data)
{
//	GUI *gui = (GUI *)user_data;
//	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

//	gtk_widget_set_sensitive(GTK_WIDGET(item), gui->NowRecordingSound());
}
void GUI::OnSelectSoundRate(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeSoundFrequency(num);
}
void GUI::OnUpdateSoundRate(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetSoundFrequencyNum() == num);
}
void GUI::OnSelectSoundLatency(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeSoundLatency(num);
}
void GUI::OnUpdateSoundLatency(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->GetSoundLatencyNum() == num);
}

//
void GUI::OnSelectSavePrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowSavePrinterDialog(drv);
}
void GUI::OnUpdateSavePrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	int size = gui->GetPrinterBufferSize(drv);
	gtk_widget_set_sensitive(GTK_WIDGET(item), size > 0);
}
void GUI::OnSelectPrintPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtPrintPrinterMessage(drv);
}
void GUI::OnUpdatePrintPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	int size = gui->GetPrinterBufferSize(drv);
	gtk_widget_set_sensitive(GTK_WIDGET(item), size > 0);
}
void GUI::OnSelectClearPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtClearPrinterBufferMessage(drv);
}
void GUI::OnUpdateClearPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	int size = gui->GetPrinterBufferSize(drv);
	gtk_widget_set_sensitive(GTK_WIDGET(item), size > 0);
}
void GUI::OnSelectDirectPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtEnablePrinterDirectMessage(drv);
}
void GUI::OnUpdateDirectPrinter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsEnablePrinterDirect(drv));
}
void GUI::OnSelectPrinterOnline(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->PostEtTogglePrinterOnlineMessage(drv);
}
void GUI::OnUpdatePrinterOnline(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsOnlinePrinter(drv));
}

void GUI::OnSelectCommServer(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
//	gui->PostEtEnableCommServerMessage(drv);
	gui->ToggleEnableCommServer(drv);
}
void GUI::OnUpdateCommServer(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsEnableCommServer(drv));
}
void GUI::OnUpdateCommConnectMenu(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	gui->create_comm_connect_menu(widget, drv);
}
void GUI::OnSelectCommConnect(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
//	gui->PostEtConnectCommMessage(drv);
	gui->ToggleConnectComm(drv, num);
}
void GUI::OnUpdateCommConnect(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowConnectingComm(drv, num));
}
void GUI::OnSelectCommThroughMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleCommThroughMode(drv);
}
void GUI::OnUpdateCommThroughMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowCommThroughMode(drv));
}
void GUI::OnSelectCommBinaryMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleCommBinaryMode(drv);
}
void GUI::OnUpdateCommBinaryMode(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->NowCommBinaryMode(drv));
}
void GUI::OnSelectCommSendTelnetCommand(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int drv = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"drv");
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->SendCommTelnetCommand(drv, num);
}

//
void GUI::OnSelectLedBox(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleLedBox();
}
void GUI::OnUpdateLedBox(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsShownLedBox());
}
void GUI::OnSelectInsideLed(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleInsideLedBox();
}
void GUI::OnUpdateInsideLed(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsInsidedLedBox());
}
void GUI::OnSelectMsgBoard(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleMessageBoard();
}
void GUI::OnUpdateMsgBoard(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsShownMessageBoard());
}
#ifdef USE_PERFORMANCE_METER
void GUI::OnSelectPMeter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->TogglePMeter();
}
void GUI::OnUpdatePMeter(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsShownPMeter());
}
#endif
void GUI::OnSelectUseJoypad(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ChangeUseJoypad(num);
}
void GUI::OnUpdateUseJoypad(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;
	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	gtk_check_menu_item_set_active(item, gui->IsEnableJoypad(num));
}
#ifdef USE_KEY2JOYSTICK
void GUI::OnSelectEnableKey2Joypad(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleEnableKey2Joypad();
}
void GUI::OnUpdateEnableKey2Joypad(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsEnableKey2Joypad());
}
#endif
void GUI::OnSelectJoySetting(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowJoySettingDialog();
}
#ifdef USE_LIGHTPEN
void GUI::OnSelectEnableLightpen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleEnableLightpen();
}
void GUI::OnUpdateEnableLightpen(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsEnableLightpen());
}
#endif
#ifdef USE_MOUSE
void GUI::OnSelectEnableMouse(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleUseMouse();
}
void GUI::OnUpdateEnableMouse(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsEnableMouse());
}
#endif
void GUI::OnSelectLoosenKeyStroke(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ToggleLoosenKeyStroke();
}
void GUI::OnUpdateLoosenKeyStroke(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsLoosenKeyStroke());
}
void GUI::OnSelectKeybindBox(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowKeybindDialog();
}
void GUI::OnSelectConfigureBox(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowConfigureDialog();
}

void GUI::OnSelectVirtualKeyboard(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowVirtualKeyboard();
}
void GUI::OnUpdateVirtualKeyboard(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;

	gtk_check_menu_item_set_active(item, gui->IsShownVirtualKeyboard());
}

void GUI::OnSelectAbout(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->ShowAboutDialog();
}

#ifdef USE_DEBUGGER
void GUI::OnSelectOpenDebugger(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
//	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	SKIP_WHEN_MENU_OPENING(widget);
	gui->OpenDebugger();
}

void GUI::OnUpdateOpenDebugger(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	GtkCheckMenuItem *item = (GtkCheckMenuItem *)widget;
//	int num = (int)(intptr_t)g_object_get_data(G_OBJECT(widget),"num");

	gtk_widget_set_sensitive(GTK_WIDGET(item), !gui->IsDebuggerOpened());
}

void GUI::OnSelectCloseDebugger(GtkWidget *widget, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;

	SKIP_WHEN_MENU_OPENING(widget);
	gui->CloseDebugger();
}
#endif

#ifdef USE_GTK
void GUI::OnUserCommand(GtkWidget *widget, gint id, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	gui->ProcessCommand(id, NULL, NULL);
}
#endif

gboolean GUI::OnDragDrop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time_, gpointer user_data)
{
	GtkTargetList *list = gtk_drag_dest_get_target_list(widget);
	GdkAtom atom = gtk_drag_dest_find_target(widget, context, list);
	if (atom == GDK_NONE) {
		// out of target
		gtk_drag_finish(context, FALSE, FALSE, time_);
		return FALSE;
	}
	gtk_drag_get_data(widget, context, atom, time_);
	return TRUE;
}

void GUI::OnDragDataReceived(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time_, gpointer user_data)
{
	GUI *gui = (GUI *)user_data;
	gchar **list = gtk_selection_data_get_uris(data);
	if (!list) {
		gtk_drag_finish(context, FALSE, FALSE, time_);
		return;
	}
	gui->OpenDroppedFile(list[0]);
	g_strfreev(list);
	gtk_drag_finish(context, TRUE, FALSE, time_);
}

#endif /* GUI_TYPE_GTK_X11 */

#ifdef GUI_TYPE_AGAR
#include "../agar/ag_gui.h"
#endif
bool GUI::StartAutoKey(void)
{
	GtkClipboard *clip;
	gchar *str;

	clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	if (!clip) return false;

	str = gtk_clipboard_wait_for_text(clip);
	if (!str) return false;

	emu->start_auto_key(str);

	g_free(str);
	return true;
}

bool GUI::ShowVirtualKeyboard(void)
{
#ifdef USE_VKEYBOARD
	if (!vkeyboard) {
		vkeyboard = new Vkbd::VKeyboard();

		uint8_t *buf;
		int siz;
		emu->get_vm_key_status_buffer(&buf, &siz);
		FIFOINT *his = emu->get_vm_key_history();
		vkeyboard->SetStatusBufferPtr(buf, siz, VM_KEY_STATUS_VKEYBOARD);
		vkeyboard->SetHistoryBufferPtr(his);
		if (!vkeyboard->Create(emu->resource_path())) {
			logging->out_log(LOG_ERROR, _T("Cannot open virtual keyboard window."));
		}
		vkeyboard->Show();
	} else {
		vkeyboard->Close();
	}
	return true;
#else
	return false;
#endif
}
