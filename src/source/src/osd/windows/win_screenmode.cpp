/** @file win_screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#include "../screenmode.h"
#include "../../vm/vm_defs.h"
#include "../../logging.h"
//#include "win_main.h"


ScreenMode::ScreenMode()
	: ScreenModeBase()
{
	monitor_count = 0;
}

ScreenMode::~ScreenMode()
{
}

BOOL CALLBACK ScreenMode::MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	ScreenMode *obj = (ScreenMode *)dwData;
	return obj->get_monitor_device(hMonitor, hdcMonitor, lprcMonitor);
}

BOOL ScreenMode::get_monitor_device(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor)
{
	if (monitor_count < disp_devices.Count()) {
		CDisplayDevice *item = disp_devices[monitor_count];
		RECT re;
		CopyRect(&re, lprcMonitor);
		RECT_IN(item->re, re.left, re.top, re.right - re.left, re.bottom - re.top);
	}
	monitor_count++;

	return TRUE;
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

	// enumerate display device
	for(int i = 0; disp_devices.Count() < DISP_DEVICE_MAX; i++) {
		DISPLAY_DEVICE disp;
		ZeroMemory(&disp, sizeof(disp));
		disp.cb = sizeof(disp);
		if (!EnumDisplayDevices(NULL, i, &disp, 0)) {
			break;
		}
		if (disp.StateFlags & (DISPLAY_DEVICE_ACTIVE | DISPLAY_DEVICE_PRIMARY_DEVICE)) {
			CDisplayDevice *item = new CDisplayDevice();
			item->disp_no = i;
			item->name.Set(disp.DeviceName);
			disp_devices.Add(item);
		}
	}
	major_bits_per_pixel = bits_per_pixel;

	// enumerate monitor size
	EnumDisplayMonitors(NULL, NULL, MonitorProc, (LPARAM)this);

	// enumerate screen mode for fullscreen
	int w,h,bpp;
	for(int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		CDisplayDevice *dd = disp_devices[disp_no];
		for(int i = 0; dd->modes.Count() < VIDEO_MODE_MAX; i++) {
			DEVMODE dev;
			ZeroMemory(&dev, sizeof(dev));
			dev.dmSize = sizeof(dev);
			if(EnumDisplaySettings(dd->name, i, &dev) == 0) {
				break;
			}
			w = dev.dmPelsWidth;
			h = dev.dmPelsHeight;
			bpp = dev.dmBitsPerPel;

			if (!dd->modes.IsValidSize(w, h)) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp ignored"), disp_no, w, h, bpp);
				continue;
			}

			int found = dd->modes.Find(w, h);
			if (found >= 0) {
				logging->out_debugf(_T("screen_mode:-- [%d] %dx%d %dbpp already exist"), disp_no, w, h, bpp);
			} else {
				CVideoMode *item = new CVideoMode();
				item->Set(w, h);
				dd->modes.Add(item);
				logging->out_debugf(_T("screen_mode:%2d [%d] %dx%d %dbpp"), dd->modes.Count(), disp_no, w, h, bpp);

//				if(max_desktop_width <= w) {
//					max_desktop_width = w;
//				}
//				if(max_desktop_height <= h) {
//					max_desktop_height = h;
//				}
			}
		}
		dd->modes.Sort();

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
	HDC hdcScr = GetDC(NULL);
	if (width) *width = GetDeviceCaps(hdcScr, HORZRES);
	if (height) *height = GetDeviceCaps(hdcScr, VERTRES);
	if (bpp) *bpp = GetDeviceCaps(hdcScr, BITSPIXEL);
	ReleaseDC(NULL, hdcScr);
}
