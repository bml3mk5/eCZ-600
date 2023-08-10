/** @file win_ledboxbase.h

	SHARP X68000 Emulator
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ led box ]
*/

#ifndef _WIN_LEDBOX_BASE_H_
#define _WIN_LEDBOX_BASE_H_

#include <windows.h>

#include "win_d3d.h"
#include "../../common.h"
#include "../../csurface.h"
#include "../../cbitmap.h"

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
	VmRectWH	led_ps[LED_PARTS_END];

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
	VmRectWH	led_pt[LED_POS_END];

	// 表示位置
	struct win_pt_st {
		int left;
		int top;
		int right;
		int bottom;
		int place;
	} win_pt;
	VmRectWH    parent_pt;

	int			mode;
	int         prev_mode;
	VmPoint		dist_set[2];
	VmPoint		dist;

	bool        visible;
	bool        inside;

#ifdef USE_SCREEN_D3D_TEXTURE
	CD3DTexture texture;
#endif

	bool create_bitmap(CBitmap **l, DWORD id);
//	bool create_parts(HDC hdc, const led_t &srcl, int x, int y, int w, int h, led_t *l);
	void rect_in(VmRectWH &re, int x, int y, int w, int h);
	void point_in(VmRectWH &dst, int x, int y, VmRectWH &src);
	void point_in(VmRectWH &dst, int x, int y, CBitmap *src);
	void copy_led(VmRectWH &, CBitmap *);
	void copy_led_s(VmRectWH &, CBitmap *, VmRectWH &);
	void copy_led_s(VmRectWH &, CBitmap *, POINT &);

	/* for dialog box */
	virtual void show_dialog() {}
	virtual void move_in_place(int) {}
	virtual void need_update_dialog() {}
public:
	LedBoxBase();
	virtual ~LedBoxBase();
	bool InitScreen();
	void Show(int);
	void SetMode(int);
	void SetPos(int left, int top, int right, int bottom, int place);
	void SetPos(int place);
	void Draw(HDC);
	void Draw(LPDIRECT3DSURFACE9);
	bool Draw(PDIRECT3DDEVICE9);
	void Update(uint64_t flag);

	void SetDistance(int place, const VmPoint *ndist);
	void GetDistance(VmPoint *ndist);

	HRESULT CreateTexture(PDIRECT3DDEVICE9 pD3Device);
	void ReleaseTexture();

	/* for dialog box */
	virtual void CreateDialogBox() {}
	virtual void Move() {}
};

#ifdef _WIN32
#include "../../gui/windows/win_ledbox.h"
#endif

#endif /* _WIN_LEDBOX_BASE_H_ */

