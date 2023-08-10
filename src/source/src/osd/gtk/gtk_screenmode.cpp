/** @file gtk_screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.11

	@brief [ screen mode ]
*/


#include "../screenmode.h"
#include "../../logging.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo/cairo.h>

ScreenMode::ScreenMode()
	: ScreenModeBase()
{
}

ScreenMode::~ScreenMode()
{
}

void ScreenMode::Enum()
{
	int w, h, bpp;
	GetDesktopSize(&w, &h, &bpp);
	Enum(w, h, bpp);
}

void ScreenMode::Enum(int desktop_width, int desktop_height, int bits_per_pixel)
{
//	int max_desktop_width = 0;
//	int max_desktop_height = 0;

	major_bits_per_pixel = bits_per_pixel;

	// enumerate screen mode for fullscreen
	GdkDisplay *display = gdk_display_get_default();
#if GTK_CHECK_VERSION(3,22,0)
	if (display) {
		int nums = gdk_display_get_n_monitors(display);
		for(int disp_no = 0; disp_no < nums; disp_no++) {
			GdkMonitor *monitor = gdk_display_get_monitor(display, disp_no);
			GdkRectangle re;
			gdk_monitor_get_geometry(monitor, &re);
			CDisplayDevice *item = new CDisplayDevice();
			item->re.x = re.x;
			item->re.y = re.y;
			item->re.w = re.width;
			item->re.h = re.height;
			disp_devices.Add(item);
		}
	} else
#else
	GdkScreen *screen = NULL;
	if (display) {
		screen = gdk_display_get_default_screen(display);
	}
	if (screen) {
		int nums = gdk_screen_get_n_monitors(screen);
		for(int disp_no = 0; disp_no < nums; disp_no++) {
			GdkRectangle re;
			gdk_screen_get_monitor_geometry(screen, disp_no, &re);
			CDisplayDevice *item = new CDisplayDevice();
			item->re.x = re.x;
			item->re.y = re.y;
			item->re.w = re.width;
			item->re.h = re.height;
			disp_devices.Add(item);
		}
	} else
#endif
	{
		CDisplayDevice *item = new CDisplayDevice();
		item->re.x = 0;
		item->re.y = 0;
		item->re.w = desktop_width;
		item->re.h = desktop_height;
		disp_devices.Add(item);
	}

	for(int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		CDisplayDevice *dd = disp_devices[disp_no];
		// if cannot get modes, add default screen size mode.
		if (dd->modes.Count() == 0) {
			CVideoMode *item = new CVideoMode();
			item->Set(dd->re.w, dd->re.h);
			dd->modes.Add(item);

//			max_desktop_width = desktop_width;
//			max_desktop_height = desktop_height;
		}
	}
}

void ScreenMode::GetDesktopSize(int *width, int *height, int *bpp)
{
	// desktop size on the primary monitor
#if GTK_CHECK_VERSION(3,22,0)
	GdkDisplay *display = gdk_display_get_default();
	GdkMonitor *primary = NULL;
	if (display) {
		primary = gdk_display_get_primary_monitor(display);
	}
	if (primary) {
		GdkRectangle re;
		gdk_monitor_get_geometry(primary, &re);

		if (width) *width = re.width;
		if (height) *height = re.height;
	} else {
		if (width) *width = 0;
		if (height) *height = 0;
	}
	if (bpp) {
		GdkVisual *visual = NULL;
# if GTK_CHECK_VERSION(3,26,0)
		GdkWindow *root = gdk_get_default_root_window();
		if (root) {
			visual = gdk_window_get_visual(root);
		}
# else
		GdkScreen *screen = gdk_display_get_default_screen(display);
		if (screen) {
			visual = gdk_screen_get_system_visual(screen);
		}
# endif
		if (visual) {
			*bpp = gdk_visual_get_depth(visual);
		} else {
			*bpp = _RGB888;
		}
	}
#else
	GdkDisplay *display = gdk_display_get_default();
	GdkScreen *screen = NULL;
	gint primary = -1;
	if (display) {
		screen = gdk_display_get_default_screen(display);
	}
	if (screen) {
		primary = gdk_screen_get_primary_monitor(screen);
	}
	if (primary >= 0) {
		GdkRectangle re;
		gdk_screen_get_monitor_geometry(screen, primary, &re);

		if (width) *width = re.width;
		if (height) *height = re.height;
		if (bpp) {
			GdkVisual *visual = gdk_screen_get_system_visual(screen);
			if (visual) {
				*bpp = gdk_visual_get_depth(visual);
			} else {
				*bpp = _RGB888;
			}
		}
	} else {
		if (width) *width = 0;
		if (height) *height = 0;
		if (bpp) *bpp = 1;
	}
#endif

#if 0
	GdkWindow *root = gdk_get_default_root_window();
	if (root) {
		if (width) *width = gdk_window_get_width(root);
		if (height) *height = gdk_window_get_height(root);
		if (bpp) {
			GdkVisual *visual = gdk_window_get_visual(root);
			if (visual) {
				*bpp = gdk_visual_get_depth(visual);
			} else {
				*bpp = _RGB888;
			}
		}
	} else {
		if (width) *width = 0;
		if (height) *height = 0;
		if (bpp) *bpp = 1;
	}
#endif
}
