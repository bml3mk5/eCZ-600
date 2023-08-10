/** @file sdl_vkeyboardbase.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.09.22 -

	@brief [ virtual keyboard ]
*/

#include <string.h>
//#include <malloc.h>
#include "sdl_vkeyboardbase.h"
#include "../../emu.h"
#include "../../fifo.h"
//#include "../../logging.h"

//#if defined(_X68000)
//#include "../../gui/vkeyboard_x68000.h"
//#else
//namespace Vkbd {
//const struct stBitmap_t cBmpParts[] = {
//	{ -1, 0, 0, 0, 0 }
//};
//const struct stLedPos_t cLedPos[] = {
//	{ -1, 0, 0 }
//};
//const Pos_t cVkbdKeyPos[] = {
//	{ 0, 0, NULL }
//};
//}
//#endif

#include "../../utils.h"
#include "../../utility.h"

extern EMU *emu;

namespace Vkbd {

const _TCHAR *csBitmapFileNames[BITMAPIDS_END] = {
	_T("x68000_keyboard_1.png"),
	_T("x68000_keyboard_1_led0.png"),
	_T("x68000_keyboard_1_led1.png"),
	_T("x68000_keyboard_1_led2.png"),
	_T("x68000_keyboard_1_led3.png"),
};

OSDBase::OSDBase() : Base()
{
}

OSDBase::~OSDBase()
{
}

bool OSDBase::load_bitmap(const _TCHAR *res_path)
{
	bool rc = true;
	for(int i=0; i<BITMAPIDS_END; i++) {
		if (!pBitmaps[i]) {
			rc = (rc && create_bitmap(res_path, csBitmapFileNames[i], &pBitmaps[i]));
		}
	}
	if (rc) {
		create_surface();
	}
	return rc;
}

bool OSDBase::create_surface()
{
	if (pSurface) return true;
	if (!pBitmaps[BITMAPIDS_BASE]) return false;
	if (!pBitmaps[BITMAPIDS_BASE]->IsEnable()) return false;

	pSurface = new CSurface(pBitmaps[BITMAPIDS_BASE]->Width(), pBitmaps[BITMAPIDS_BASE]->Height(),
#if defined(_WIN32)
		CPixelFormat::BGRA32
#elif defined(__APPLE__) && defined(__MACH__)
		CPixelFormat::ARGB32
#else
		CPixelFormat::BGRA32
#endif
	);
	if (pSurface == NULL) return false;

	pBitmaps[BITMAPIDS_BASE]->Blit(*pSurface);
	return true;
}

bool OSDBase::create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CBitmap **suf)
{
	_TCHAR path[_MAX_PATH];
	UTILITY::stprintf(path, _MAX_PATH, _T("%s%s") ,res_path, bmp_file);
	*suf = new CBitmap(path, NULL);
	if (*suf == NULL || !(*suf)->IsEnable()) return false;
	return true;
}

void OSDBase::unload_bitmap()
{
	if (pSurface) {
		delete pSurface;
		pSurface = NULL;
	}
	for(int i=0; i<BITMAPIDS_END; i++) {
		if (pBitmaps[i]) {
			delete pBitmaps[i];
			pBitmaps[i] = NULL;
		}
	}
}

/// @param[in] info : information of mouse
/// @param[in] onoff : pressed a key?
void OSDBase::need_update_window_base(PressedInfo_t *info, bool onoff)
{
	const Bitmap_t *bp = NULL;

	SDL_Rect dstrect;
	dstrect.x = info->re.left;
	dstrect.y = info->re.top;
	dstrect.w = info->re.right - info->re.left + 1;
	dstrect.h = info->re.bottom - info->re.top + 1;
	int parts_num = info->parts_num;

	if (onoff) {
		// key pressed
		SDL_Rect srcrect;
		if (parts_num >= 0) {
			// another parts
			bp = &cBmpParts[parts_num];
			srcrect.x = bp->x;
			srcrect.y = bp->y;
			srcrect.w = bp->w;
			srcrect.h = bp->h;
			if (pBitmaps[bp->idx + led_bright]) blit_surface(pBitmaps[bp->idx + led_bright], srcrect, dstrect);
		} else {
			// set base parts shifted down 3px
			fill_rect(&dstrect);
			dstrect.h -= 3;
			srcrect = dstrect;
			dstrect.y += 3;
			if (info->led_bitmap_num >= 0) {
				// led turn on, so set another parts
				bp = &cBmpParts[info->led_parts_num];
				srcrect.x = bp->x;
				srcrect.y = bp->y;
				srcrect.w = bp->w;
				srcrect.h = bp->h;
				if (pBitmaps[info->led_bitmap_num + led_bright]) blit_surface(pBitmaps[info->led_bitmap_num + led_bright], srcrect, dstrect);
			} else {
				blit_surface(pBitmaps[BITMAPIDS_BASE], srcrect, dstrect);
			}
		}
	} else {
		// key released
		if (info->led_bitmap_num >= 0) {
			// led turn on, so set another parts
			SDL_Rect srcrect;
			bp = &cBmpParts[info->led_parts_num];
			srcrect.x = bp->x;
			srcrect.y = bp->y;
			srcrect.w = bp->w;
			srcrect.h = bp->h;
			if (pBitmaps[info->led_bitmap_num + led_bright]) blit_surface(pBitmaps[info->led_bitmap_num + led_bright], srcrect, dstrect);
		} else {
			// set base parts
			blit_surface(pBitmaps[BITMAPIDS_BASE], dstrect, dstrect);
		}
	}
}

void OSDBase::fill_rect(SDL_Rect *re)
{
	if (!pSurface) return;

#if defined(_BML3MK5)
	SDL_FillRect(pSurface->Get(), re, 0x80808080);
#else
	SDL_FillRect(pSurface->Get(), re, 0x30303030);
#endif
}

bool OSDBase::blit_surface(CBitmap *pSrcBmp, SDL_Rect &src_re, SDL_Rect &dst_re)
{
	if (!pSurface || !pSrcBmp) return false;

	pSrcBmp->Blit(src_re, *pSurface, dst_re);

	return true;
}

} /* namespace Vkbd */
