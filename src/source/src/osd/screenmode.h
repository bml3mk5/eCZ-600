/** @file screenmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#ifndef SCREEN_MODE_H
#define SCREEN_MODE_H

#include "../common.h"
#include "../cchar.h"
#include "../cptrlist.h"
#include "../res/resource.h"


#define VIDEO_MODE_MAX 20
/// Display monitor size
class CVideoMode
{
public:
	CVideoMode();
	~CVideoMode();

	int width;
	int height;

	void Set(int width_, int height_);
	bool Match(int width_, int height_) const;
};

/// Manage display monitor size
class CVideoModes : public CPtrList<CVideoMode>
{
private:
	static bool greater(const CVideoMode *a, const CVideoMode *b);

public:
	CVideoModes();

	void Sort();
	int Find(int width_, int height_) const;
	static bool IsValidSize(int width_, int height_);
};


#define DISP_DEVICE_MAX 6
/// display monitor
class CDisplayDevice
{
public:
	CDisplayDevice();
	~CDisplayDevice();

	int      disp_no;
	CTchar   name;
	VmRectWH re;
	CVideoModes modes;
};

/**
	@brief Remain size on each displays
*/
class ScreenModeBase
{
protected:
	CPtrList<CDisplayDevice> disp_devices;

	int major_bits_per_pixel;

public:
	ScreenModeBase();
	virtual ~ScreenModeBase();

	virtual void Enum() = 0;
	virtual void Enum(int desktop_width, int desktop_height, int bits_per_pixel) = 0;

	virtual int CountMode(int disp_no) const;
	virtual const CVideoMode *GetMode(int disp_no, int num) const;

	virtual int FindMode(int disp_no, int width, int height) const;

	virtual int CountDisp() const;
	virtual const CDisplayDevice *GetDisp(int num) const;
	virtual void GetDispSize(int num, int *width, int *height, int *bits_per_pixel) const;

	virtual int WithinDisp(int x, int y) const;
	virtual void GetVirtualDispSize(int *width, int *height, int *bits_per_pixel) const;
};


#if defined(USE_WIN)
#include "windows/win_screenmode.h"
#elif defined(USE_GTK)
#include "gtk/gtk_screenmode.h"
#elif defined(USE_SDL) || defined(USE_SDL2)
#include "SDL/sdl_screenmode.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "wxwidgets/wxw_screenmode.h"
#elif defined(USE_QT)
#include "qt/qt_screenmode.h"
#endif

#endif /* SCREEN_MODE_H */
