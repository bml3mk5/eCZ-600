/** @file win_screenmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#ifndef WIN_SCREEN_MODE_H
#define WIN_SCREEN_MODE_H

#include <windows.h>
#include "../../common.h"


/**
	@brief Remain size on each displays
*/
class ScreenMode : public ScreenModeBase
{
private:
	int monitor_count;

	static int qsort_by_n_w_h(const void *a, const void *b);
	static BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
	BOOL get_monitor_device(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor);
public:
	ScreenMode();
	~ScreenMode();

	void Enum();
	void Enum(int desktop_width, int desktop_height, int bits_per_pixel);

	static void GetDesktopSize(int *width, int *height, int *bpp);
};

#endif /* WIN_SCREEN_MODE_H */
