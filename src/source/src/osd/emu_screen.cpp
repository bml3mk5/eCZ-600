/** @file emu_screen.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.12.01

	@brief [ emu screen ]
*/

#include "../emu.h"
#include "../depend.h"
#include "../config.h"
#include "../cpixfmt.h"
#include "../csurface.h"
#include "../video/rec_video.h"
#include "../cmutex.h"


void EMU::EMU_SCREEN()
{
	RECT_IN(screen_size, SCREEN_DEST_X, SCREEN_DEST_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
	SIZE_IN(screen_aspect_size, SCREEN_WIDTH_ASPECT, SCREEN_HEIGHT_ASPECT);
//	window_width = WINDOW_WIDTH;
//	window_height = WINDOW_HEIGHT;
//	SIZE_IN(min_window_size, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	screen_size_changed = true;
	SIZE_IN(desktop_size, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	SIZE_IN(display_size, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	desktop_bpp = 24;
	display_margin.left = 0;
	display_margin.top = 0;
	display_margin.right = 0;
	display_margin.bottom = 0;

	source_size = screen_size;
	SIZE_IN(source_aspect_size, -1, -1);
#ifdef USE_SCREEN_ROTATE
	// clockwise rotate
	MATRIX_IN(rotate_matrix[0],  1,  0,  0,  1);
	MATRIX_IN(rotate_matrix[1],  0, -1,  1,  0);
	MATRIX_IN(rotate_matrix[2], -1,  0,  0, -1);
	MATRIX_IN(rotate_matrix[3],  0,  1, -1,  0);
#endif
#ifdef USE_SMOOTH_STRETCH
	SIZE_IN(stretch_power, -1, -1);
	stretch_screen = false;
#endif
	RECT_IN(stretched_size, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	POINT_IN(stretched_dest_real, 0, 0);
	mixed_size = source_size;
	SIZE_IN(mixed_ratio, 1, 1);

	RECT_IN(vm_screen_size, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	SIZE_IN(vm_display_size, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

	// screen mode
	now_resizing = false;
	window_mode_power = 10;
	now_screenmode = NOW_WINDOW;
	prev_screenmode = NOW_WINDOW;
	prev_window_mode = 0;
	window_dest.x = 0;
	window_dest.y = 0;
	first_change_screen = true;

	// initialize video recording
	rec_video_size[0].w = MIN_WINDOW_WIDTH;
	rec_video_size[0].h = MIN_WINDOW_HEIGHT;
	rec_video_size[0].x = (MAX_WINDOW_WIDTH - MIN_WINDOW_WIDTH) / 2;
	rec_video_size[0].y = (MAX_WINDOW_HEIGHT - MIN_WINDOW_HEIGHT) / 2;
	rec_video_size[1].w = MAX_WINDOW_WIDTH;
	rec_video_size[1].h = MAX_WINDOW_HEIGHT;
	rec_video_size[1].x = 0;
	rec_video_size[1].y = 0;
	rec_video_stretched_size = rec_video_size[0];

	rec_video = new REC_VIDEO(this);
	now_recording_video = rec_video->GetNowRecordingPtr();

	// initialize update flags
	disable_screen = DISABLE_SURFACE;
	first_invalidate_default = false;
	first_invalidate = false;
	self_invalidate = false;
	skip_frame = false;

	//
	sufOrigin = new CSurface();
#ifdef USE_SCREEN_ROTATE
	sufRotate = new CSurface();
#endif
	sufSource = NULL;
#ifdef USE_SCREEN_MIX_SURFACE
	sufMixed = new CSurface();
#endif
#ifdef USE_SMOOTH_STRETCH
	sufStretch1 = new CSurface();
	sufStretch2 = new CSurface();
#endif

	pixel_format = new CPixelFormat();

	// for lock screen
	mux_update_screen = new CMutex();
	screen_changing = false;
}

///
/// initialize screen
///
void EMU::initialize_screen()
{
	logging->out_debug(_T("EMU::initialize_screen"));

	change_pixel_aspect(pConfig->pixel_aspect);
	change_rec_video_size(pConfig->screen_video_size);
}

///
/// release screen
///
void EMU::release_screen()
{
	delete sufOrigin;
	sufOrigin = NULL;
#ifdef USE_SCREEN_ROTATE
	delete sufRotate;
	sufRotate = NULL;
#endif
#ifdef USE_SCREEN_MIX_SURFACE
	delete sufMixed;
	sufMixed = NULL;
#endif
#ifdef USE_SMOOTH_STRETCH
	delete sufStretch1;
	sufStretch1 = NULL;
	delete sufStretch2;
	sufStretch2 = NULL;
#endif

	delete pixel_format;
	pixel_format = NULL;

	delete mux_update_screen;
	mux_update_screen = NULL;

	delete rec_video;
	rec_video = NULL;
}

///
/// store window position to ini file
///
void EMU::resume_window_placement()
{
}

///
/// get RGB format in a pixeldata
///
void EMU::get_rgbformat(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift)
{
	pixel_format->Get(r_mask, g_mask, b_mask, r_shift, g_shift, b_shift);
}

///
/// get RGB + alpha format in a pixeldata
///
void EMU::get_rgbaformat(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint32_t *a_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift, uint8_t *a_shift)
{
	pixel_format->Get(r_mask, g_mask, b_mask, a_mask, r_shift, g_shift, b_shift, a_shift);
}

///
/// get R,G,B in a pixel
///
void EMU::get_rgbcolor(scrntype pixel, uint8_t *r, uint8_t *g, uint8_t *b)
{
	pixel_format->Get(pixel, *r, *g, *b);
}

///
/// create a pixeldata from R,G,B 
///
scrntype EMU::map_rgbcolor(uint8_t r, uint8_t g, uint8_t b)
{
	return pixel_format->Map(r, g, b);
}

void EMU::release_screen_on_emu_thread()
{
	// stop video recording
	stop_rec_video();
}

/// calculate the client size of window or fullscreen
///
/// @param [in] width : new width or -1 set current width
/// @param [in] height : new height or -1 set current height
/// @param [in] power : magnify x 10
/// @param [in] now_window : true:window / false:fullscreen
void EMU::set_display_size(int width, int height, int power, bool now_window)
{
}

/// set vm drawing area
void EMU::set_vm_display_size()
{
	if (vm) {
//		int new_size = pConfig->screen_video_size;
//		int l = mixed_size.x;
//		int t = mixed_size.y;
//		int r = l + mixed_size.w;
//		int b = t + mixed_size.h;
		int l = source_size.x;
		int t = source_size.y;
		int r = source_size.w - source_size.x;
		int b = source_size.h - source_size.y;

		// vm drawing area is same as area to capture and record screen
//		if (rec_video_size[new_size].w > mixed_size.w) {
//			l = rec_video_size[new_size].x;
//			r = rec_video_size[new_size].x + rec_video_size[new_size].w;
//		}

//		if (rec_video_size[new_size].h > mixed_size.h) {
//			t = rec_video_size[new_size].y;
//			b = rec_video_size[new_size].y + rec_video_size[new_size].h;
//		}

#ifdef USE_SCREEN_ROTATE
		l = screen_size.x;
		t = screen_size.y;
		r = l + screen_size.w;
		b = t + screen_size.h;
#endif
		vm->set_display_size(l,t,r,b);
	}
}

/// reserve top margin area for ledbox
int EMU::adjust_y_position(int h, int y)
{
	if (20 < h - LIMIT_MIN_WINDOW_HEIGHT && h - LIMIT_MIN_WINDOW_HEIGHT < 42) {
		y -= (42 - h + LIMIT_MIN_WINDOW_HEIGHT) / 2;
	}
	return y;
}

void EMU::draw_screen()
{
}

bool EMU::mix_screen()
{
	return false;
}

void EMU::fill_gray()
{
	scrntype color = map_rgbcolor(0x40, 0x40, 0x40);
	for(int y=0; y<source_size.h; y++) {
		scrntype *line = screen_buffer(y);
		for(int x=0; x<source_size.w; x++) {
			*line = color;
			line++;
		}
	}
}

scrntype* EMU::screen_buffer(int y)
{
	return NULL;
}

int EMU::screen_buffer_offset()
{
	return 0;
}

bool EMU::now_skip_frame()
{
	return skip_frame && !now_rec_video();
}

void EMU::capture_screen()
{
}

///
/// change screen size on vm
///
void EMU::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	SIZE_IN(vm_display_size, screen_width, screen_height);
}

///
/// start recording video
///
bool EMU::start_rec_video(int type, int fps_no, bool show_dialog)
{
#ifdef USE_REC_VIDEO
	int size = pConfig->screen_video_size;
	return rec_video->Start(type, fps_no, rec_video_size[size], sufOrigin, show_dialog);
#else
	return false;
#endif
}

///
/// record video
///
void EMU::record_rec_video()
{
}

///
/// stop recording video
///
void EMU::stop_rec_video()
{
#ifdef USE_REC_VIDEO
	rec_video->Stop();
#endif
}

///
/// restart recording video
///
void EMU::restart_rec_video()
{
#ifdef USE_REC_VIDEO
	rec_video->Restart();
#endif
}

/// fps number
/// @return 0 - 5 or -1
int EMU::get_rec_video_fps_num()
{
#ifdef USE_REC_VIDEO
	return rec_video->GetFpsNum();
#else
	return -1;
#endif
}

bool EMU::now_rec_video()
{
#ifdef USE_REC_VIDEO
	return *now_recording_video;
#else
	return false;
#endif
}

void EMU::resize_rec_video(int num)
{
#ifdef USE_EMU_INHERENT_SPEC
	pConfig->screen_video_size = num;
	change_rec_video_size(num);

	// send display size to vm
	set_vm_display_size();
#endif
}
void EMU::change_rec_video_size(int num)
{
	rec_video_stretched_size = rec_video_size[num];
	if (mixed_ratio.w < mixed_ratio.h) {
		rec_video_stretched_size.h = rec_video_stretched_size.h * mixed_ratio.w / mixed_ratio.h;
		rec_video_stretched_size.y += (rec_video_size[num].h - rec_video_stretched_size.h) / 2; 
	} else {
		rec_video_stretched_size.w = rec_video_stretched_size.w * mixed_ratio.h / mixed_ratio.w;
		rec_video_stretched_size.x += (rec_video_size[num].w - rec_video_stretched_size.w) / 2; 
	}
	rec_video_stretched_size.y = adjust_y_position(rec_video_stretched_size.h, rec_video_stretched_size.y);

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("          rec_video_size: num:%d x:%d y:%d w:%d h:%d")
		,num,rec_video_size[num].x,rec_video_size[num].y,rec_video_size[num].w,rec_video_size[num].h);
	logging->out_debugf(_T("rec_video_stretched_size:       x:%d y:%d w:%d h:%d")
		,rec_video_stretched_size.x,rec_video_stretched_size.y,rec_video_stretched_size.w,rec_video_stretched_size.h);
#endif
}

bool EMU::rec_video_enabled(int type)
{
#ifdef USE_REC_VIDEO
	return rec_video->IsEnabled(type);
#else
	return false;
#endif
}

const _TCHAR **EMU::get_rec_video_codec_list(int type)
{
#ifdef USE_REC_VIDEO
	return rec_video->GetCodecList(type);
#else
	return NULL;
#endif
}

const CMsg::Id *EMU::get_rec_video_quality_list(int type)
{
#ifdef USE_REC_VIDEO
	return rec_video->GetQualityList(type);
#else
	return NULL;
#endif
}

//----------

/// enumerate mode of screen and window
void EMU::init_screen_mode()
{
	screen_mode.Enum();
	screen_mode.GetVirtualDispSize(&desktop_size.w, &desktop_size.h, &desktop_bpp);
	window_mode.Enum(desktop_size.w, desktop_size.h);
	find_screen_mode();
}

/// enumerate screen mode for window
/// calcurate magnify range
/// @param [in] max_width
/// @param [in] max_height
void EMU::enum_window_mode(int max_width, int max_height)
{
	window_mode.Enum(max_width, max_height);
}


/// matching window or fullscreen size
void EMU::find_screen_mode()
{
	if (pConfig->window_mode < 8) {
		// matching window size at last time
		int find = window_mode.Find(pConfig->screen_width, pConfig->screen_height);
		if (find >= 0) {
			pConfig->window_mode = find;
		} else {
			pConfig->window_mode = 0;
		}
	} else {
		// matching fullscreen size at last time
		int find = screen_mode.FindMode(pConfig->disp_device_no, pConfig->screen_width, pConfig->screen_height);
		if (find >= 0) {
			pConfig->window_mode = find + (pConfig->disp_device_no * VIDEO_MODE_MAX) + 8;
		} else {
			// default window
			pConfig->window_mode = 0;
		}
	}
}

/// enumerate size of screen
void EMU::enum_screen_mode(uint32_t flags)
{
	screen_mode.Enum(desktop_size.w, desktop_size.h, flags);
}

/// change window size / switch over fullscreen and window
/// @param[in] mode 0 - 7: window size  8 -:  fullscreen size  -1: switch over  -2: shift window mode
void EMU::change_screen_mode(int mode)
{
//	logging->out_debugf(_T("change_screen_mode: mode:%d cwmode:%d pwmode:%d w:%d h:%d"),mode,pConfig->window_mode,prev_window_mode,desktop_size.w,desktop_size.h);
//	if (mode == pConfig->window_mode) return;
	if (now_resizing) {
		// ignore events
		return;
	}

	if (mode == -1) {
		// switch over fullscreen and window
		if (now_screenmode != NOW_WINDOW) {
			// go window mode
			mode = prev_window_mode;
		}
	} else if (mode == -2) {
		// shift window mode 
		if (now_screenmode != NOW_WINDOW) {
			// no change
			return;
		} else {
			mode = ((pConfig->window_mode + 1) % window_mode.Count());
		}
	}
	if (now_screenmode != NOW_FULLSCREEN) {
		prev_window_mode = pConfig->window_mode;
	}
//	logging->out_debugf(_T("change_screen_mode: mode:%d cwmode:%d pwmode:%d w:%d h:%d"),mode,pConfig->window_mode,prev_window_mode,desktop_size.w,desktop_size.h);
	set_window(mode, desktop_size.w, desktop_size.h);
}

/// change maximize window
///
/// @param[in] width
/// @param[in] height
/// @param[in] maximize
void EMU::change_maximize_window(int width, int height, bool maximize)
{
	EnumScreenMode next_mode = (maximize ? NOW_MAXIMIZE : NOW_WINDOW);
	if (now_screenmode != NOW_FULLSCREEN && now_screenmode != next_mode) {
		// change screen
		now_screenmode = next_mode;
		set_display_size(width, height, 10, now_screenmode == NOW_WINDOW);
	}
}

/// change screen stretching on fullscreen/maximize mode
///
/// @param[in] num : 0:no expand  1:normal expand  2:maximum expand
///
/// @attention should be called by main thread
void EMU::change_stretch_screen(int num)
{
	if (num < 0) {
		num = (pConfig->stretch_screen + 1);
	}
	num = (num % 3);
	if (pConfig->stretch_screen == num) {
		num = 0;
	}
	pConfig->stretch_screen = num;
	if (now_screenmode != NOW_WINDOW) {
		// change screen
		set_display_size(-1, -1, 10, now_screenmode == NOW_WINDOW);
	}
}

/// setting window or fullscreen size to display
///
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
void EMU::set_window(int mode, int cur_width, int cur_height)
{
}

/// change aspect ratio
///
/// @param[in] mode : aspect 0:square 1: 2:
void EMU::change_pixel_aspect(int mode)
{
	if (mode < 0) {
		mode = (pConfig->pixel_aspect + 1) % get_pixel_aspect_count();
	}
	pConfig->pixel_aspect = get_pixel_aspect(mode, &mixed_ratio.w, &mixed_ratio.h);

	set_display_size(pConfig->screen_width, pConfig->screen_height, now_screenmode != NOW_WINDOW ? 10 : window_mode_power, now_screenmode == NOW_WINDOW);
}

/// kind of aspect ratio
///
/// @return number
int EMU::get_pixel_aspect_count()
{
	return 5;
}

/// get horizontal and vertical of aspect ratio
///
/// @param[in]  mode 0, 1, 2, 3, 4
/// @param[out] wratio width
/// @param[out] hratio height
/// @return mode
int EMU::get_pixel_aspect(int mode, int *wratio, int *hratio)
{
	int new_mode = mode;
	switch(mode) {
	case 1:
		*wratio = 10;
		*hratio = 11;
		break;
	case 2:
		*wratio = 10;
		*hratio = 12;
		break;
	case 3:
		*wratio = 11;
		*hratio = 10;
		break;
	case 4:
		*wratio = 12;
		*hratio = 10;
		break;
	default:
		*wratio = 1;
		*hratio = 1;
		new_mode = 0;
		break;
	}
#ifdef USE_SCREEN_ROTATE
	if (pConfig->monitor_type & 1) {
		SWAP(int, *wratio, *hratio);
	}
#endif
	return new_mode;
}

/// number of window mode
///
/// @return number
int EMU::get_window_mode_count() const
{
	return window_mode.Count();
}

/// window mode
///
/// @param[in] num index of window mode
/// @return pointer of window mode structure
const CWindowMode *EMU::get_window_mode(int num) const
{
	return window_mode.Get(num);
}

/// number of display
///
/// @return number
int EMU::get_display_device_count() const
{
	return screen_mode.CountDisp();
}

/// display monitor information
///
/// @param[in] disp_no display number
/// @return pointer of display information
const CDisplayDevice *EMU::get_display_device(int disp_no) const
{
	return screen_mode.GetDisp(disp_no);
}

/// number of screen mode
///
/// @param[in] disp_no display number
/// @return number
int EMU::get_screen_mode_count(int disp_no) const
{
	return screen_mode.CountMode(disp_no);
}

/// screen mode
///
/// @param[in] disp_no display number
/// @param[in] num     index of screen mode
/// @return pointer of screen mode on specified display
const CVideoMode *EMU::get_screen_mode(int disp_no, int num) const
{
	return screen_mode.GetMode(disp_no, num);
}

/// fullscreen now ?
bool EMU::is_fullscreen() const
{
	return (now_screenmode == NOW_FULLSCREEN);
}

/// screen size of video
VmRectWH *EMU::get_screen_record_size(int num)
{
	if (0 <= num && num <= 1) {
		return &rec_video_size[num];
	}
	return NULL;
}

void EMU::lock_screen()
{
	mux_update_screen->lock();
	screen_changing = true;
}

void EMU::unlock_screen()
{
	screen_changing = false;
	mux_update_screen->unlock();
}

