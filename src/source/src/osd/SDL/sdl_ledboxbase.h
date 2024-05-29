/** @file sdl_ledboxbase.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ led box ]
*/

#ifndef SDL_LEDBOX_BASE_H
#define SDL_LEDBOX_BASE_H

#include "../../vm/vm.h"
#include <SDL.h>
#include "../../common.h"
#include "../../csurface.h"
#ifdef USE_OPENGL
#include "../opengl.h"
#endif
#ifdef USE_GTK
class CCairoSurface;
#endif

class CBitmap;

/**
	@brief LedBoxBase is the class that display the access indicator on the screen.
*/
class LedBoxBase : public CSurface
{
protected:
	uint64_t	flag_old;

	enum enum_led_type {
		LED_TYPE_DIGIT = 0,
		LED_TYPE_PARTS,
		LED_TYPE_BASE,
		LED_TYPE_END
	};
	// ビットマップ
	CBitmap *led[LED_TYPE_END];

	enum enum_led_parts {
		LED_PARTS_GRAY0H = 0,
		LED_PARTS_GRAY1H,
		LED_PARTS_G0H,
		LED_PARTS_G1H,
		LED_PARTS_G2H,
		LED_PARTS_G3H,
		LED_PARTS_R0H,
		LED_PARTS_R1H,
		LED_PARTS_R2H,
		LED_PARTS_R3H,
		LED_PARTS_GRAY0C,
		LED_PARTS_GRAY1C,
		LED_PARTS_G0C,
		LED_PARTS_R0C,
		LED_PARTS_GRAY0V,
		LED_PARTS_GRAY1V,
		LED_PARTS_G0V,
		LED_PARTS_G1V,
		LED_PARTS_R0V,
		LED_PARTS_R1V,
		LED_PARTS_R0Q,
		LED_PARTS_R1Q,
		LED_PARTS_R2Q,
		LED_PARTS_R3Q,
		LED_PARTS_END
	};
	// LEDパーツ内位置
	SDL_Rect	led_ps[LED_PARTS_END];

	enum enum_led_pos {
		LED_POS_BASE = 0,
		LED_POS_HDBUSY,
		LED_POS_TIMER,
		LED_POS_POWER,
		LED_POS_HIRA,
		LED_POS_ZENKAKU,
		LED_POS_KANA,
		LED_POS_ROMA,
		LED_POS_CODE,
		LED_POS_INS,
		LED_POS_CAPS,
		LED_POS_FDD0,
		LED_POS_FDD0E,
		LED_POS_FDD1,
		LED_POS_FDD1E,
		LED_POS_HDD0,
		LED_POS_HIRES1,
		LED_POS_END
	};
	// LED位置
	SDL_Rect	led_pt[LED_POS_END];

	// 表示位置
	struct win_pt_st {
		int left;
		int top;
		int right;
		int bottom;
		int place;
	} win_pt;
	SDL_Rect    parent_pt;
	bool		changed_position;

	int			mode;
	int         prev_mode;
	VmPoint		dist_set[2];
	VmPoint		dist;

	bool        enable;
	bool        visible;
	bool        inside;

#ifdef USE_GTK
	CCairoSurface *cairosuf;
#endif

	bool create_surface(CPixelFormat &);
	bool create_bitmap(const _TCHAR *, const _TCHAR *, CPixelFormat &, CBitmap **);
	void rect_in(SDL_Rect &re, int x, int y, int w, int h);
	void point_in(SDL_Rect &dst, int x, int y, SDL_Rect &src);
	void point_in(SDL_Rect &dst, int x, int y, CBitmap *src);
	void copy_led(SDL_Rect &, CBitmap *);
	void copy_led_s(SDL_Rect &, CBitmap *, SDL_Rect &);

	/* for dialog box */
	virtual void create_dialog_box_sub() {}
	virtual void show_dialog() {}
	virtual void move_in_place(int) {}
	virtual void need_update_dialog() {}
public:
	LedBoxBase();
	virtual ~LedBoxBase();

	bool InitScreen(const _TCHAR *res_path, CPixelFormat &pixel_format);
	void Show(int);
	void SetMode(int);
	void SetPos(int left, int top, int right, int bottom, int place);
	void SetPos(int place);
	void Draw(CSurface &);
	void Draw(SDL_Surface &);
#ifdef USE_GTK
	void Draw(cairo_t *);
#endif
#if defined(USE_SDL2)
	void Draw(CTexture &);
#endif
#ifdef USE_OPENGL
	void Draw(COpenGLTexture &);
#endif
	void Update(uint64_t flag);

	void SetDistance(int place, const VmPoint *ndist);
	void GetDistance(VmPoint *ndist);

	bool IsEnable() { return enable; }

	/* for dialog box */
	virtual void CreateDialogBox() {}
	virtual void Move() {}
};

#if defined(USE_SDL2) && defined(USE_SDL2_LEDBOX)
#include "sdl2_ledbox.h"
#elif defined(USE_WX) || defined(USE_WX2)
#include "../../gui/wxwidgets/wx_ledbox.h"
#elif defined(_WIN32)
#include "../../gui/windows/win_ledbox.h"
#elif defined(__MACH__) && defined(__APPLE__)
#include "../../gui/cocoa/cocoa_ledbox.h"
#elif defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_LEDBOX)
#include "../../gui/gtk_x11/x11_ledbox.h"
#elif defined(USE_GTK_LEDBOX)
#include "../../gui/gtk_x11/gtk_ledbox.h"
#else
class LedBox : public LedBoxBase
{
public:
	LedBox() : LedBoxBase() {};
};
#endif

#endif /* SDL_LEDBOX_BASE_H */

