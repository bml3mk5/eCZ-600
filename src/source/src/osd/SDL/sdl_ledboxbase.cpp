/** @file sdl_ledboxbase.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ led box ]
*/
#include "sdl_ledboxbase.h"
#include "sdl_cbitmap.h"
#include "../../utils.h"
#include "../../cchar.h"
#include "../../utility.h"
#ifdef USE_PERFORMANCE_METER
#include "../../config.h"
#endif

LedBoxBase::LedBoxBase() : CSurface()
{
	for(int i=0; i<LED_TYPE_END; i++) {
		led[i] = NULL;
	}

	flag_old = -1;
	dist.x = 0; dist.y = 0;
	mode = 0;
	prev_mode = -1;
	dist_set[0].x = 2; dist_set[0].y = 2;
	dist_set[1].x = 2; dist_set[1].y = 2;
	memset(&win_pt, 0, sizeof(win_pt));
	memset(&parent_pt, 0, sizeof(parent_pt));
	changed_position = false;

	enable = false;
	visible = false;
	inside = false;
}

LedBoxBase::~LedBoxBase()
{
	for(int i=0; i<LED_TYPE_END; i++) {
		delete led[i];
	}
}

// 画面初期化
bool LedBoxBase::InitScreen(const _TCHAR *res_path, CPixelFormat &pixel_format)
{
	enable = false;

	// LED背景
	if (!create_bitmap(res_path, _T("ledbox_x68k_1.png"), pixel_format, &led[LED_TYPE_BASE])) return enable;
	// LEDパーツ作成
	if (!create_bitmap(res_path, _T("ledparts_x68k_1.png"), pixel_format, &led[LED_TYPE_PARTS])) return enable;

	// horiz
	rect_in(led_ps[LED_PARTS_GRAY0H],  0 * 16,  0, 10, 7);
	rect_in(led_ps[LED_PARTS_GRAY1H],  0 * 16, 10, 10, 7);
	rect_in(led_ps[LED_PARTS_G0H],     1 * 16,  0, 10, 7);
	rect_in(led_ps[LED_PARTS_G1H],     1 * 16, 10, 10, 7);
	rect_in(led_ps[LED_PARTS_G2H],     2 * 16,  0, 10, 7);
	rect_in(led_ps[LED_PARTS_G3H],     2 * 16, 10, 10, 7);
	rect_in(led_ps[LED_PARTS_R0H],     3 * 16,  0, 10, 7);
	rect_in(led_ps[LED_PARTS_R1H],     3 * 16, 10, 10, 7);
	rect_in(led_ps[LED_PARTS_R2H],     4 * 16,  0, 10, 7);
	rect_in(led_ps[LED_PARTS_R3H],     4 * 16, 10, 10, 7);
	rect_in(led_ps[LED_PARTS_GRAY0H],  5 * 16,  0,  7, 7);
	rect_in(led_ps[LED_PARTS_GRAY1H],  5 * 16, 10,  7, 7);
	rect_in(led_ps[LED_PARTS_G0C],     6 * 16,  0,  7, 7);
	rect_in(led_ps[LED_PARTS_R0C],     7 * 16,  0,  7, 7);
	rect_in(led_ps[LED_PARTS_GRAY0V],  8 * 16,  0,  5, 9);
	rect_in(led_ps[LED_PARTS_GRAY1V],  8 * 16, 10,  5, 9);
	rect_in(led_ps[LED_PARTS_G0V],     9 * 16,  0,  5, 9);
	rect_in(led_ps[LED_PARTS_G1V],     9 * 16, 10,  5, 9);
	rect_in(led_ps[LED_PARTS_R0V],    10 * 16,  0,  5, 9);
	rect_in(led_ps[LED_PARTS_R1V],    10 * 16, 10,  5, 9);
	rect_in(led_ps[LED_PARTS_R0Q],    11 * 16,  0,  7, 7);
	rect_in(led_ps[LED_PARTS_R1Q],    11 * 16, 10,  7, 7);
	rect_in(led_ps[LED_PARTS_R2Q],    12 * 16,  0,  7, 7);
	rect_in(led_ps[LED_PARTS_R3Q],    12 * 16, 10,  7, 7);

	// 位置設定
	point_in(led_pt[LED_POS_BASE],      0,  0, led[LED_TYPE_BASE]);
	point_in(led_pt[LED_POS_HDBUSY],   13, 13, led_ps[LED_PARTS_G0C]);
	point_in(led_pt[LED_POS_TIMER],    45, 13, led_ps[LED_PARTS_R0C]);
	point_in(led_pt[LED_POS_POWER],    78, 13, led_ps[LED_PARTS_G0C]);
	point_in(led_pt[LED_POS_HIRA],    110, 14, led_ps[LED_PARTS_G0H]);
	point_in(led_pt[LED_POS_ZENKAKU], 139, 14, led_ps[LED_PARTS_G0H]);
	point_in(led_pt[LED_POS_KANA],    171, 14, led_ps[LED_PARTS_R0H]);
	point_in(led_pt[LED_POS_ROMA],    195, 14, led_ps[LED_PARTS_R0H]);
	point_in(led_pt[LED_POS_CODE],    219, 14, led_ps[LED_PARTS_R0H]);
	point_in(led_pt[LED_POS_INS],     243, 14, led_ps[LED_PARTS_R0H]);
	point_in(led_pt[LED_POS_CAPS],    267, 14, led_ps[LED_PARTS_R0H]);
	point_in(led_pt[LED_POS_FDD0],    296,  3, led_ps[LED_PARTS_G0C]);
	point_in(led_pt[LED_POS_FDD0E],   297, 13, led_ps[LED_PARTS_G0V]);
	point_in(led_pt[LED_POS_FDD1],    316,  3, led_ps[LED_PARTS_G0C]);
	point_in(led_pt[LED_POS_FDD1E],   317, 13, led_ps[LED_PARTS_G0V]);
	point_in(led_pt[LED_POS_HDD0],    349, 14, led_ps[LED_PARTS_R0Q]);
	point_in(led_pt[LED_POS_HIRES1],  349, 13, led_ps[LED_PARTS_G0C]);

	if (!create_surface(pixel_format)) {
		return false;
	}

	parent_pt.w = Width();
	parent_pt.h = Height();

	enable = true;
	return enable;
}

bool LedBoxBase::create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CPixelFormat &format, CBitmap **suf)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::stprintf(path, _MAX_PATH, _T("%s%s"), res_path, bmp_file);
	*suf = new CBitmap(path, &format);
	if (*suf == NULL || !(*suf)->IsEnable()) return false;
	return true;
}

bool LedBoxBase::create_surface(CPixelFormat &format)
{
	if (led[LED_TYPE_BASE] == NULL || !led[LED_TYPE_BASE]->IsEnable()) return false;
	Release();

	int w, h;
	w = led[LED_TYPE_BASE]->Width();
	h = led[LED_TYPE_BASE]->Height();
#ifdef USE_PERFORMANCE_METER
	if (pConfig->show_pmeter) {
		w += 108;
	}
#endif
	bool rc = Create(w, h, format);
	if (!rc) return false;

#if defined(USE_SDL)
	// See SDL_ConvertSurface source code
	SDL_Surface *led_suf = led[LED_TYPE_BASE]->Get();
	led_suf->flags &= ~SDL_SRCALPHA;
	SDL_Rect sre;
	sre.x = 0; sre.y =0;
	sre.w = led_suf->w;	sre.h = led_suf->h;
	SDL_BlitSurface(led_suf, &sre, suf, &sre);
	SDL_SetAlpha(suf, SDL_SRCALPHA, 0xff);
	led_suf->flags |= SDL_SRCALPHA;
#endif

	return true;
}

void LedBoxBase::rect_in(SDL_Rect &re, int x, int y, int w, int h)
{
	re.x = x;
	re.y = y;
	re.w = w;
	re.h = h;
}

void LedBoxBase::point_in(SDL_Rect &dst, int x, int y, SDL_Rect &src)
{
	dst.x = x;
	dst.y = y;
	dst.w = src.w;
	dst.h = src.h;
}

void LedBoxBase::point_in(SDL_Rect &dst, int x, int y, CBitmap *src)
{
	dst.x = x;
	dst.y = y;
	dst.w = src->Width();
	dst.h = src->Height();
}

void LedBoxBase::Show(int flag)
{
	visible = ((flag & 1) != 0);
	inside = ((flag & 8) != 0);

	show_dialog();
}

/// @param[in] flag :
/// b00: kana on keyboard (negative)
/// b01: romaji on keyboard (negative)
/// b02: code input on keyboard (negative)
/// b03: caps on keyboard (negative)
/// b04: insert on keyboard (negative)
/// b05: hiragana on keyboard (negative)
/// b06: zenkaku on keyboard (negative)
/// b07: led bright 0
/// b08: led bright 1
/// b09: power led green
/// b10: power led red
/// b11: timer
/// b12: hdbusy
/// b13: hireso (red)
/// b14: fd0 access green
/// b15: fd0 access red
/// b16: fd0 eject green
/// b17: fd1 access green
/// b18: fd1 access red
/// b19: fd1 eject green
/// b20: fd2 access green
/// b21: fd2 access red
/// b22: fd2 eject green
/// b23: fd3 access green
/// b24: fd3 access red
/// b25: fd3 eject green
void LedBoxBase::Update(uint64_t flag)
{
	if (!enable) return;

	if (flag != flag_old) {
		int n = ((flag & 0x180) >> 7);

		led[LED_TYPE_BASE]->Blit(*this);

		if (!(flag & 0x01)) {
			copy_led_s(led_pt[LED_POS_KANA], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0H + n]);
		}
		if (!(flag & 0x02)) {
			copy_led_s(led_pt[LED_POS_ROMA], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0H + n]);
		}
		if (!(flag & 0x04)) {
			copy_led_s(led_pt[LED_POS_CODE], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0H + n]);
		}
		if (!(flag & 0x08)) {
			copy_led_s(led_pt[LED_POS_CAPS], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0H + n]);
		}
		if (!(flag & 0x10)) {
			copy_led_s(led_pt[LED_POS_INS], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0H + n]);
		}
		if (!(flag & 0x20)) {
			copy_led_s(led_pt[LED_POS_HIRA], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0H + n]);
		}
		if (!(flag & 0x40)) {
			copy_led_s(led_pt[LED_POS_ZENKAKU], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0H + n]);
		}
		// power on (green)
		if (flag & 0x200) {
			copy_led_s(led_pt[LED_POS_POWER], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0C]);
		}
		// power off (red)
		if (flag & 0x400) {
			copy_led_s(led_pt[LED_POS_POWER], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// timer
		if (flag & 0x800) {
			copy_led_s(led_pt[LED_POS_TIMER], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// hdbusy
		if (flag & 0x1000) {
			if (1) {
				// origin
				copy_led_s(led_pt[LED_POS_HDD0], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0Q]);
			} else {
				// inner hd version (ACE HD etc.)
				copy_led_s(led_pt[LED_POS_HDBUSY], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
			}
		}
		// hireso
		if (flag & 0x2000) {
			if (1) {
				// origin
				copy_led_s(led_pt[LED_POS_HDBUSY], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
			} else {
				// inner hd version (ACE HD etc.)
				copy_led_s(led_pt[LED_POS_HIRES1], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
			}
		}
#ifdef USE_FD1
		// fdd0 green
		if (flag & 0x04000) {
			copy_led_s(led_pt[LED_POS_FDD0], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0C]);
		}
		// fdd0 red
		if (flag & 0x08000) {
			copy_led_s(led_pt[LED_POS_FDD0], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// fdd0 eject
		if (flag & 0x10000) {
			copy_led_s(led_pt[LED_POS_FDD0E], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0V]);
		}
#endif
#ifdef USE_FD2
		// fdd1 green
		if (flag & 0x20000) {
			copy_led_s(led_pt[LED_POS_FDD1], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0C]);
		}
		// fdd1 red
		if (flag & 0x40000) {
			copy_led_s(led_pt[LED_POS_FDD1], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// fdd1 eject
		if (flag & 0x80000) {
			copy_led_s(led_pt[LED_POS_FDD1E], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0V]);
		}
#endif
#ifdef USE_FD3
		// fdd2 green
		if (flag & 0x0100000) {
			copy_led_s(led_pt[LED_POS_FDD2], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0C]);
		}
		// fdd2 red
		if (flag & 0x0200000) {
			copy_led_s(led_pt[LED_POS_FDD2], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// fdd2 eject
		if (flag & 0x0400000) {
			copy_led_s(led_pt[LED_POS_FDD2E], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0V]);
		}
#endif
#ifdef USE_FD4
		// fdd3 green
		if (flag & 0x0800000) {
			copy_led_s(led_pt[LED_POS_FDD3], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0C]);
		}
		// fdd3 red
		if (flag & 0x1000000) {
			copy_led_s(led_pt[LED_POS_FDD3], led[LED_TYPE_PARTS], led_ps[LED_PARTS_R0C]);
		}
		// fdd3 eject
		if (flag & 0x2000000) {
			copy_led_s(led_pt[LED_POS_FDD3E], led[LED_TYPE_PARTS], led_ps[LED_PARTS_G0V]);
		}
#endif
#ifdef USE_PERFORMANCE_METER
		// for debug
		if (pConfig->show_pmeter) {
			re.x = suf->w - 108; re.y = 0;
			re.w = 108; re.h = suf->h;
			SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0x40, 0x40, 0x40, 0xff));
			for(int pos=0; pos < 11; pos++) {
				re.x = suf->w - 104 + (pos * 10); re.y = suf->h * ((pos % 5 == 0) ? 1 : 2) / 8;
				re.w = 1; re.h = suf->h * ((pos % 5 == 0) ? 3 : 2) / 8;
				SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0xff, 0xff, 0xff, 0xff));
			}
			int value = (flag >> 48);
			if (value > 100) value = 100;
			re.x = suf->w - 104; re.y = suf->h / 2 + 2;
			re.w = value; re.h = 2;
			SDL_FillRect(suf, &re, SDL_MapRGBA(suf->format, 0xff, 0x0, 0x0, 0xff));
		}
#endif

		if (visible && !inside) {
			need_update_dialog();
		}
	}

	flag_old = flag;
}

void LedBoxBase::copy_led(SDL_Rect &p, CBitmap *l)
{
	l->Blit(*this, p);
}

void LedBoxBase::copy_led_s(SDL_Rect &p, CBitmap *l, SDL_Rect &re)
{
	l->Blit(re, *this, p);
}

void LedBoxBase::SetPos(int left, int top, int right, int bottom, int place)
{
	win_pt.left = left;
	win_pt.top = top;
	win_pt.right = right;
	win_pt.bottom = bottom;

	if (place & 1) {
		// base right
		parent_pt.x = right - parent_pt.w;
	} else {
		// base left
		parent_pt.x = left;
	}
	if (place & 2) {
		// base bottom
		parent_pt.y = bottom - parent_pt.h;
	} else {
		// base top
		parent_pt.y = top;
	}

	move_in_place(place);

	win_pt.place = place;

	changed_position = true;
}

void LedBoxBase::SetPos(int place)
{
	SetPos(win_pt.left, win_pt.top, win_pt.right, win_pt.bottom, place);
}

void LedBoxBase::Draw(CSurface &screen)
{
	if (!visible || !inside || !enable) return;

	changed_position = false;
	Blit(screen, parent_pt);
}

#if defined(USE_SDL2)
void LedBoxBase::Draw(CTexture &texture)
{
	if (!visible || !inside || !enable) return;

	changed_position = false;
	SDL_Rect texture_re;
	RECT_IN(texture_re, 0, 0, parent_pt.w, parent_pt.h);
	SDL_UpdateTexture(texture.Get(), &texture_re, GetBuffer(), BytesPerLine());
	SDL_RenderCopy(texture.Renderer(), texture.Get(), &texture_re, &parent_pt);
}
#endif
#ifdef USE_OPENGL
void LedBoxBase::Draw(COpenGLTexture &texture)
{
	if (!visible || !inside || !enable) return;

	if (changed_position) {
		float pyl_l = -1.0f;
		float pyl_r = 1.0f;
		if (win_pt.place & 1) pyl_l = 1.0f - (float)parent_pt.w * 2.0f / (float)(win_pt.right - win_pt.left);
		else pyl_r = (float)parent_pt.w * 2.0f / (float)(win_pt.right - win_pt.left) - 1.0f;
		float pyl_t = 1.0f;
		float pyl_b = -1.0f;
		if (win_pt.place & 2) pyl_t = (float)parent_pt.h * 2.0f / (float)(win_pt.bottom - win_pt.top) - 1.0f;
		else pyl_b = 1.0f - (float)parent_pt.h * 2.0f / (float)(win_pt.bottom - win_pt.top);
		texture.SetPos(pyl_l, pyl_t, pyl_r, pyl_b, 0.0f, 0.0f, 1.0f, 1.0f);
		changed_position = false;
	}
	texture.Render(Width(), Height(), GetBuffer());
}
#endif

void LedBoxBase::SetMode(int val)
{
	val = (val < 0) ? 0 : (val > 1 ? 1 : val);
	dist_set[mode].x = dist.x;
	dist_set[mode].y = dist.y;
	prev_mode = mode;
	mode = val;
	dist.x = dist_set[mode].x;
	dist.y = dist_set[mode].y;
}

void LedBoxBase::SetDistance(int place, const VmPoint *ndist)
{
	win_pt.place = place;
	dist_set[0].x = ndist[0].x;
	dist_set[0].y = ndist[0].y;
	dist_set[1].x = ndist[1].x;
	dist_set[1].y = ndist[1].y;

	dist.x = dist_set[mode].x;
	dist.y = dist_set[mode].y;
}

void LedBoxBase::GetDistance(VmPoint *ndist)
{
	dist_set[mode].x = dist.x;
	dist_set[mode].y = dist.y;

	ndist[0].x = dist_set[0].x;
	ndist[0].y = dist_set[0].y;
	ndist[1].x = dist_set[1].x;
	ndist[1].y = dist_set[1].y;
}
