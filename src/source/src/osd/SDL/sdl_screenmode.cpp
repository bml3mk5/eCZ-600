/** @file sdl_screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/


#include "../screenmode.h"
#include <SDL.h>
#include "../../vm/vm_defs.h"
#include "../../logging.h"


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
	int w,h,bpp;

	// enumerate display device
#ifndef USE_SDL2
	uint32_t flags = SDL_FULLSCREEN;

	CDisplayDevice *item = new CDisplayDevice();
	item->re.x = 0;
	item->re.y = 0;
	item->re.w = desktop_width;
	item->re.h = desktop_height;
	disp_devices.Add(item);

	const SDL_VideoInfo *video = SDL_GetVideoInfo();
#else
	int disp_device_count = SDL_GetNumVideoDisplays();
	if (disp_device_count > DISP_DEVICE_MAX) {
		disp_device_count = DISP_DEVICE_MAX;
	}
	for(int disp_no = 0; disp_no < disp_device_count; disp_no++) {
		CDisplayDevice *item = new CDisplayDevice();
		item->name.SetN(SDL_GetDisplayName(disp_no));
		SDL_Rect re;
		SDL_GetDisplayBounds(disp_no, &re);
		RECT_IN(item->re, re.x, re.y, re.w, re.h);
		disp_devices.Add(item);
	}
#endif

	for(int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		CDisplayDevice *dd = disp_devices[disp_no];
#ifndef USE_SDL2
		SDL_Rect **list = SDL_ListModes(video->vfmt, flags);
		if (list == NULL || list == (SDL_Rect **)-1) break;
		for(int i = 0; list[i] != NULL && dd->modes.Count() < VIDEO_MODE_MAX; i++) {
			w = list[i]->w;
			h = list[i]->h;
			bpp = video->vfmt->BitsPerPixel;
#else
		int mode_count = SDL_GetNumDisplayModes(disp_no);
		for(int i = 0; i < mode_count && dd->modes.Count() < VIDEO_MODE_MAX; i++) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(disp_no, i, &mode) < 0) {
				continue;
			}
			w = mode.w;
			h = mode.h;
			bpp = SDL_BITSPERPIXEL(mode.format);
#endif
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
#ifdef USE_SDL2
	/* SDL 2 */
	SDL_DisplayMode display_mode;

	memset(&display_mode, 0, sizeof(display_mode));
	if (SDL_GetDesktopDisplayMode(0, &display_mode)) {
		logging->out_logf(LOG_ERROR, _T("SDL_GetDesktopDisplayMode: %s."), SDL_GetError());
	}
	if (width) *width = display_mode.w;
	if (height) *height = display_mode.h;
	if (bpp) *bpp = SDL_BITSPERPIXEL(display_mode.format);
#else
	/* SDL 1 */
	const SDL_VideoInfo *video = NULL;

	// get current video mode
	if((video = SDL_GetVideoInfo()) == NULL) {
		logging->out_logf(LOG_ERROR, _T("SDL_GetVideoInfo: %s."), SDL_GetError());
		if (width) *width = 0;
		if (height) *height = 0;
		if (bpp) *bpp = 1;
		return;
	}
	if (width) *width = video->current_w;
	if (height) *height = video->current_h;
	if (bpp) *bpp = video->vfmt->BitsPerPixel;

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("video->hw_available:%d"),video->hw_available);
	logging->out_debugf(_T("video->wm_available:%d"),video->wm_available);
	logging->out_debugf(_T("video->blit_hw:%d"),video->blit_hw);
	logging->out_debugf(_T("video->blit_hw_CC:%d"),video->blit_hw_CC);
	logging->out_debugf(_T("video->blit_hw_A:%d"),video->blit_hw_A);
	logging->out_debugf(_T("video->blit_sw:%d"),video->blit_sw);
	logging->out_debugf(_T("video->blit_sw_CC:%d"),video->blit_sw_CC);
	logging->out_debugf(_T("video->blit_sw_A:%d"),video->blit_sw_A);
	logging->out_debugf(_T("video->blit_fill:%d"),video->blit_fill);
	logging->out_debugf(_T("video->video_mem:%d"),video->video_mem);
	logging->out_debugf(_T("video->vfmt->BitsPerPixel:%d"),video->vfmt->BitsPerPixel);
	logging->out_debugf(_T("video->vfmt->BytesPerPixel:%d"),video->vfmt->BytesPerPixel);
	logging->out_debugf(_T("video->vfmt->Rmask:%x, Gmask:%x, Bmask:%x, Amask:%x"),video->vfmt->Rmask,video->vfmt->Gmask,video->vfmt->Bmask,video->vfmt->Amask);
	logging->out_debugf(_T("video->vfmt->Rshift:%d, Gshift:%d, Bshift:%d, Ashift:%d"),video->vfmt->Rshift,video->vfmt->Gshift,video->vfmt->Bshift,video->vfmt->Ashift);
	logging->out_debugf(_T("video->vfmt->Rloss:%d, Gloss:%d, Bloss:%d, Aloss:%d"),video->vfmt->Rloss,video->vfmt->Gloss,video->vfmt->Bloss,video->vfmt->Aloss);
	logging->out_debugf(_T("video->vfmt->colorkey:%d"),video->vfmt->colorkey);
	logging->out_debugf(_T("video->vfmt->alpha:%d"),video->vfmt->alpha);
	logging->out_debugf(_T("video->current_w:%d"),video->current_w);
	logging->out_debugf(_T("video->current_h:%d"),video->current_h);
#endif

#endif
}
