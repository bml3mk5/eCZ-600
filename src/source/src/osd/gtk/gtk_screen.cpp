/** @file gtk_screen.cpp

	Skelton for retropc emulator
	GTK+ + SDL edition

	@author Sasaji
	@date   2017.01.27

	@brief [ gtk screen ]

	@note
	This code is based on win32_screen.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "../../emu_osd.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "../../config.h"
#ifdef USE_LEDBOX
#include "../../gui/ledbox.h"
#endif
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#include "../../csurface.h"
#include "../../video/rec_video.h"
#include "../../utils.h"
#include <cairo/cairo.h>

void EMU_OSD::EMU_SCREEN()
{
	screen = NULL;
	window = NULL;

	screen_flags = 0;

#ifdef USE_OPENGL
	glscreen = NULL;
	texGLMixed = NULL;
//	mix_texture_name = 0;
	src_tex_l = src_tex_t = 0.0;
	src_tex_r = src_tex_b = 1.0;
	src_pyl_l = src_pyl_t = -1.0;
	src_pyl_r = src_pyl_b = 1.0;
	opengl = NULL;
	next_use_opengl = 0;

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	sufMain = new CSurface();
	texGLLedBox = NULL;
	texGLMsgBoard = NULL;
#endif
#endif

#ifdef USE_LEDBOX
	ledbox = NULL;
#endif

	casSource = new CCairoSurface();
	casMixed = new CCairoSurface();
}

///
/// initialize screen
///
void EMU_OSD::initialize_screen()
{
	EMU::initialize_screen();

#ifdef USE_OPENGL
	next_use_opengl = pConfig->use_opengl;

	initialize_opengl();
#endif

#ifdef USE_MESSAGE_BOARD
	msgboard = new MsgBoard(this);
	if (msgboard) {
		msgboard->InitScreen(pixel_format, screen_size.w, screen_size.h);
		msgboard->SetMessagePos(4 + reMix.x, -4 - reMix.y, 2);
		msgboard->SetInfoPos(-4 - reMix.x, 4 + reMix.y, 1);
		msgboard->SetVisible(FLG_SHOWMSGBOARD ? true : false);
	}
#endif
#ifdef USE_LEDBOX
	if (gui) {
		ledbox = gui->CreateLedBox(res_path.GetN(), pixel_format);
	}
#endif
}

///
/// release screen
///
void EMU_OSD::release_screen()
{
	delete casMixed; casMixed = NULL;
	delete casSource; casSource = NULL;

#ifdef USE_OPENGL
	release_opengl();
#endif
	if (gui) {
		gui->ReleaseLedBox();
	}

	EMU::release_screen();
}

///
/// create / recreate window
///
bool EMU_OSD::create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags)
{
	bool rc = true;
//	uint32_t set_flags = 0;
	VmRect margin = { 0, 0, 0, 0 };

	lock_screen();

	if (now_screenmode == NOW_FULLSCREEN) {
		flags |= SDL_WINDOW_FULLSCREEN;
	} else {
		flags &= ~SDL_WINDOW_FULLSCREEN;
		margin = display_margin;
	}
	flags &= ~SDL_WINDOW_OPENGL;

#ifdef USE_OPENGL
	if (use_opengl) {
		flags |= SDL_WINDOW_OPENGL;
//		release_opengl_texture();
//		if (opengl) opengl->Terminate();
	}
#endif

	if (screen) {
		//
		// change window
		//
		GdkGeometry geo;
		if ((flags & SDL_WINDOW_FULLSCREEN) == 0) {
			// go window mode. So, set window size.
			geo.min_width = margin.left + width + margin.right;
			geo.min_height = margin.top + height + margin.bottom;
			geo.max_width = geo.min_width;
			geo.max_height = geo.min_height;
#if GTK_CHECK_VERSION(3,24,0)
			gtk_window_unfullscreen(GTK_WINDOW(window));
//			gtk_window_set_default_size(GTK_WINDOW(window), geo.min_width, geo.min_height);
			gtk_window_resize(GTK_WINDOW(window), geo.min_width, geo.min_height);
			gtk_window_move(GTK_WINDOW(window), pConfig->window_position_x, pConfig->window_position_y);
#else
			GdkWindow *dwindow = gtk_widget_get_window(window);
			gtk_window_unfullscreen(GTK_WINDOW(window));
			GdkWindowHints ghint = (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
			gdk_window_set_geometry_hints(dwindow, &geo, ghint);
			gdk_window_get_geometry(dwindow, NULL, NULL, &geo.base_width, &geo.base_height);
			gtk_widget_set_size_request(window, geo.min_width, geo.min_height);
			gdk_window_move(dwindow, pConfig->window_position_x, pConfig->window_position_y);
#endif
#if 0
			GdkWindowHints ghint = (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
			gdk_window_set_geometry_hints(dwindow, &geo, ghint);
			gdk_window_get_geometry(dwindow, NULL, NULL, &geo.base_width, &geo.base_height);
			gtk_window_unfullscreen(GTK_WINDOW(window));
			gtk_window_set_default_size(GTK_WINDOW(window), geo.min_width, geo.min_height);
#endif
		} else {
			// go fullscreen mode. So, set screen mode
			gtk_window_get_position(GTK_WINDOW(window), &pConfig->window_position_x, &pConfig->window_position_y);
			geo.min_width = width;
			geo.min_height = height;
			geo.max_width = width + 1;
			geo.max_height = height + 1;
#if GTK_CHECK_VERSION(3,24,0)
			GdkDisplay *ddisplay = gdk_display_get_default();
			GdkScreen *dscreen = gdk_display_get_default_screen(ddisplay);
//			gtk_window_set_default_size(GTK_WINDOW(window), geo.min_width, geo.min_height);
			gtk_window_fullscreen_on_monitor(GTK_WINDOW(window), dscreen, pConfig->disp_device_no);
#else
			const CDisplayDevice *dd = screen_mode.GetDisp(pConfig->disp_device_no);
			GdkWindow *dwindow = gtk_widget_get_window(window);
			gdk_window_move(dwindow, dd->re.x, dd->re.y);
			GdkWindowHints ghint = (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
			gdk_window_set_geometry_hints(dwindow, &geo, ghint);
			gdk_window_get_geometry(dwindow, NULL, NULL, &geo.base_width, &geo.base_height);
			gtk_window_fullscreen(GTK_WINDOW(window));
#endif
		}
	} else {
		//
		// create window
		//
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		if (window == NULL) {
			logging->out_log(LOG_ERROR, "gtk_window_new error.");
			rc = false;
			goto FIN;
		}
#ifdef WINDOW_MOVE_POSITION
		if (screen_mode.WithinDisplay(x, y)) {
			window_dest.x = x;
			window_dest.y = y;
		} else
#endif
		{
			gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
			window_dest.x = 0x7fffffff;
			window_dest.y = 0x7fffffff;
		}

#if GTK_CHECK_VERSION(3,24,0)
//		gtk_window_set_default_size(GTK_WINDOW(window), width, height);
		gtk_widget_set_size_request(window, width, height);
#else
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
#endif
#if 0
		gtk_window_set_default_size(GTK_WINDOW(window), width, height);
//		gtk_widget_set_size_request(window, width, height);
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
#endif

		screen = gtk_drawing_area_new();
		if (screen == NULL) {
			logging->out_log(LOG_ERROR, "gtk_drawing_area_new error.");
			rc = false;
			goto FIN;
		}

		pixel_format->PresetBGRA();

#ifdef USE_OPENGL
#ifdef USE_GTK_GLAREA
		glscreen = gtk_gl_area_new();
#else
		glscreen = NULL;
#endif
		if (glscreen == NULL) {
			logging->out_log(LOG_ERROR, "gtk_gl_area_new error.");
			use_opengl = 0;
			flags &= ~SDL_WINDOW_OPENGL;
		}
		if (use_opengl && glscreen) {
			g_signal_connect(G_OBJECT(glscreen), "realize", G_CALLBACK(on_gl_realize), (gpointer)this);
			g_signal_connect(G_OBJECT(glscreen), "unrealize", G_CALLBACK(on_gl_unrealize), (gpointer)this);
			g_signal_connect(G_OBJECT(glscreen), "render", G_CALLBACK(on_gl_render), (gpointer)this);
		} else
#endif
		{
#if GTK_CHECK_VERSION(3,0,0)
			g_signal_connect(G_OBJECT(screen), "realize", G_CALLBACK(on_realize), (gpointer)this);
			g_signal_connect(G_OBJECT(screen), "draw", G_CALLBACK(on_draw), (gpointer)this);
#else
			g_signal_connect(G_OBJECT(screen), "expose-event", G_CALLBACK(on_expose), (gpointer)this);
#endif
		}
	}
	if (flags != screen_flags) {
		logging->out_debugf(_T("different flags: 0x%x -> 0x%x"), screen_flags, flags);
		screen_flags = flags;
	}

	// create gui window
	if (gui->CreateWidget(screen, width, height) == -1) {
		rc = false;
		goto FIN;
	}

FIN:
	unlock_screen();

	return rc;
}

void EMU_OSD::realize_window(GtkWidget *window)
{
#ifdef WINDOW_MOVE_POSITION
	if (window_dest.x != 0x7fffffff && window_dest.y != 0x7fffffff) {
		// Many window manager ignore moving a window by application.
		gtk_window_move(GTK_WINDOW(window), window_dest.x, window_dest.y);
	}
#endif
}

///
/// create / recreate offline surface
///
bool EMU_OSD::create_offlinesurface()
{
	if (!screen) return true;

	if (sufOrigin && !sufOrigin->IsEnable()) {
		if (!sufOrigin->Create(screen_size.w, screen_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufOrigin failed."));
			return false;
		}
	}
#ifdef USE_SCREEN_ROTATE
	if (sufRotate) {
		sufRotate->Release();
		if (!sufRotate->Create(source_size.w, source_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufRotate failed."));
			return false;
		}
	}
#endif
	if (casSource) {
		casSource->CreateC(*sufSource, screen_size.w, screen_size.h);
	}
	if (!create_mixedsurface()) {
		return false;
	}

#ifdef USE_OPENGL
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (!sufMain->Create(mixed_max_size.w, mixed_max_size.h, *pixel_format)) {
		logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufMain failed."));
		return false;
	}
#endif
#endif

#ifdef USE_SMOOTH_STRETCH
	if (sufStretch1 && sufStretch2) {
		sufStretch1->Release();
		sufStretch2->Release();
		stretch_screen = false;
		if(stretch_power.w != 1 || stretch_power.h != 1) {
			sufStretch1->Create(source_size.w * stretch_power.w, source_size.h * stretch_power.h);
			SetStretchBltMode(sufStretch1->GetDC(), COLORONCOLOR);

			sufStretch2->Create(stretched_size.w, stretched_size.h);
			SetStretchBltMode(sufStretch2->GetDC(), COLORONCOLOR);

			stretch_screen = true;
		}
	}
#endif


	disable_screen &= ~DISABLE_SURFACE;

	return true;
}

bool EMU_OSD::create_mixedsurface()
{
	if (sufMixed) {
		sufMixed->Release();
		if (!sufMixed->Create(display_size.w, display_size.h, *pixel_format)) {
			logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufMixed failed."));
			return false;
		}
	}
	if (casMixed) {
		casMixed->CreateC(*sufMixed, display_size.w, display_size.h);
	}
	return true;
}

/// setting window or fullscreen size
///
/// @param [in] width : new width or -1 set current width
/// @param [in] height : new height or -1 set current height
/// @param [in] power : magnify x 10
/// @param [in] now_window : true:window / false:fullscreen
void EMU_OSD::set_display_size(int width, int height, int power, bool now_window)
{
//	bool display_size_changed = false;
	bool stretch_changed = false;

	if(width != -1 && (display_size.w != width || display_size.h != height)) {
		display_size.w = width;
		display_size.h = height;
//		display_size_changed = true;
		stretch_changed = true;
	}

#ifdef USE_SCREEN_ROTATE
	VmRectWH prev_source_size = source_size;

	if(pConfig->monitor_type & 1) {
		stretch_changed |= (source_size.w != screen_size.h);
		stretch_changed |= (source_size.h != screen_size.w);
		stretch_changed |= (source_aspect_size.w != screen_aspect_size.h);
		stretch_changed |= (source_aspect_size.h != screen_aspect_size.w);

		source_size.w = screen_size.h;
		source_size.h = screen_size.w;
		source_size.x = screen_size.y;
		source_size.y = screen_size.x;
		source_aspect_size.w = screen_aspect_size.h;
		source_aspect_size.h = screen_aspect_size.w;
	} else
#endif
	{
		stretch_changed |= (source_size.w != screen_size.w);
		stretch_changed |= (source_size.h != screen_size.h);
		stretch_changed |= (source_aspect_size.w != screen_aspect_size.w);
		stretch_changed |= (source_aspect_size.h != screen_aspect_size.h);

		source_size = screen_size;
		source_aspect_size = screen_aspect_size;
	}

	// fullscreen and stretch screen
	if(pConfig->stretch_screen && !now_window && display_size.w >= source_size.w && display_size.h >= source_size.h) {
		if (pConfig->stretch_screen == 1) {
			// fit to full screen
			mixed_size = source_size;
			if (mixed_ratio.w < mixed_ratio.h) {
				mixed_size.h = mixed_size.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				mixed_size.w = mixed_size.w * mixed_ratio.h / mixed_ratio.w;
			}
			mixed_size.x = (source_size.w - mixed_size.w) / 2;
			mixed_size.y = (source_size.h - mixed_size.h) / 2;
			mixed_size.y = adjust_y_position(mixed_size.h, mixed_size.y);

			stretched_size.w = (display_size.h * source_aspect_size.w) / source_aspect_size.h;
			stretched_size.h = display_size.h;
#ifndef USE_GTK
			stretched_dest_real.x = - mixed_size.x * display_size.h / source_aspect_size.h;
			stretched_dest_real.y = - mixed_size.y * display_size.h / source_aspect_size.h;
#endif
			if(stretched_size.w > display_size.w) {
				stretched_size.w = display_size.w;
				stretched_size.h = (display_size.w * source_aspect_size.h) / source_aspect_size.w;
#ifndef USE_GTK
				stretched_dest_real.x = - mixed_size.x * display_size.w / source_aspect_size.w;
				stretched_dest_real.y = - mixed_size.y * display_size.w / source_aspect_size.w;
#endif
			}
			if (mixed_ratio.w < mixed_ratio.h) {
				stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
			} else {
				stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
			}
			stretched_size.x = (display_size.w - stretched_size.w) / 2;
			stretched_size.y = (display_size.h - stretched_size.h) / 2;
#ifndef USE_GTK
			stretched_dest_real.x += stretched_size.x;
			stretched_dest_real.y += stretched_size.y;
#else
			stretched_dest_real.x = - mixed_size.x;
			stretched_dest_real.y = - mixed_size.y;
#endif

		} else {
			// fit text area to full screen (cut off padding area)
			VmSize min_size;
			SIZE_IN(min_size, LIMIT_MIN_WINDOW_WIDTH, LIMIT_MIN_WINDOW_HEIGHT);
#ifdef USE_SCREEN_ROTATE
			if(pConfig->monitor_type & 1) {
				SWAP(int, min_size.w, min_size.h);
			}
#endif
			VmSize mixed_rsize, mixed_rrsize;
			mixed_size = source_size;
			SIZE_IN(mixed_rsize, mixed_size.w, mixed_size.h);
			mixed_rrsize = mixed_rsize;

			VmSize min_rsize, min_rrsize;
			min_rsize = min_size;
			min_rrsize = min_size;
			if (mixed_ratio.w < mixed_ratio.h) {
				min_rsize.h = min_rsize.h * mixed_ratio.h / mixed_ratio.w;
				min_rrsize.h = min_rrsize.h * mixed_ratio.w / mixed_ratio.h;
				mixed_rsize.h = mixed_rsize.h * mixed_ratio.h / mixed_ratio.w;
				mixed_rrsize.h = mixed_rrsize.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				min_rsize.w = min_rsize.w * mixed_ratio.w / mixed_ratio.h;
				min_rrsize.w = min_rrsize.w * mixed_ratio.h / mixed_ratio.w;
				mixed_rsize.w = mixed_rsize.w * mixed_ratio.w / mixed_ratio.h;
				mixed_rrsize.w = mixed_rrsize.w * mixed_ratio.h / mixed_ratio.w;
			}

			double magx = (double)display_size.w / min_rsize.w;
			double magy = (double)display_size.h / min_rsize.h;

			bool mag_based_w = (magx < magy);

			stretched_dest_real.x = 0;
			stretched_dest_real.y = 0;
			if(mag_based_w) {
				// magnify = display_size.w / min_rsize.w

				mixed_size.x = (mixed_size.w - min_size.w) / 2;
				mixed_size.w = min_size.w;
				stretched_size.x = 0;
				stretched_size.w = display_size.w;
#ifndef USE_GTK
				stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.w / min_rsize.w)) / 2;
#else
				stretched_dest_real.x = - mixed_size.x;
#endif

				if ((mixed_size.h * min_rsize.h * display_size.w / min_rsize.w / min_size.h) >= display_size.h) {
					int mh = display_size.h * min_rrsize.h * min_rsize.w / display_size.w / min_size.h;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = 0;
					stretched_size.h = display_size.h;
#ifndef USE_GTK
					stretched_dest_real.y = (display_size.h - (mixed_size.h * display_size.w / min_rsize.w)) / 2;
#else
					stretched_dest_real.y = - mixed_size.y;
#endif
				} else {
					int mh = mixed_rrsize.h;
					int sh = mixed_size.h * display_size.w / min_rsize.w;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = (display_size.h - sh) / 2;
					stretched_size.h = sh;
#ifndef USE_GTK
					stretched_dest_real.y = stretched_size.y;
#else
					stretched_dest_real.y = - mixed_size.y;
#endif
				}

			} else {
				// magnify = display_size.h / min_rsize.h

				mixed_size.y = (mixed_size.h - min_size.h) / 2;
				mixed_size.h = min_size.h;
				stretched_size.y = 0;
				stretched_size.h = display_size.h;
#ifndef USE_GTK
				stretched_dest_real.y = (display_size.h - (mixed_rsize.h * display_size.h / min_rsize.h)) / 2;
#else
				stretched_dest_real.y = - mixed_size.y;
#endif

				if ((mixed_size.w * min_rsize.w * display_size.h / min_rsize.h / min_size.w) >= display_size.w) {
					int mw = display_size.w * min_rrsize.w * min_rsize.h / display_size.h / min_size.w;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = 0;
					stretched_size.w = display_size.w;
#ifndef USE_GTK
					stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.h / min_rsize.h)) / 2;
#else
					stretched_dest_real.x = - mixed_size.x;
#endif
				} else {
					int mw = mixed_rrsize.w;
					int sw = mixed_size.w * display_size.h / min_rsize.h;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = (display_size.w - sw) / 2;
					stretched_size.w = sw;
#ifndef USE_GTK
					stretched_dest_real.x = stretched_size.x;
#else
					stretched_dest_real.x = - mixed_size.x;
#endif
				}

			}
		}
	}
	// window or non-streach mode
	else {
		for(int n = 0; n <= 1; n++) {
			if (n == 0) {
				mixed_size.w = display_size.w * 10 / power;
				mixed_size.h = display_size.h * 10 / power;
			} else {
				mixed_size = source_size;
			}
			if (mixed_ratio.w < mixed_ratio.h) {
				mixed_size.h = mixed_size.h * mixed_ratio.w / mixed_ratio.h;
			} else {
				mixed_size.w = mixed_size.w * mixed_ratio.h / mixed_ratio.w;
			}
			mixed_size.x = (source_size.w - mixed_size.w) / 2;
			mixed_size.y = (source_size.h - mixed_size.h) / 2;
			if (mixed_size.x >= 0 && mixed_size.y >= 0) {
				break;
			}
		}
		mixed_size.y = adjust_y_position(mixed_size.h, mixed_size.y);

		stretched_size.w = source_aspect_size.w * power / 10;
		stretched_size.h = source_aspect_size.h * power / 10;
		stretched_size.x = (display_size.w - stretched_size.w) / 2;
		stretched_size.y = (display_size.h - stretched_size.h) / 2;
#ifndef USE_GTK
		stretched_dest_real.x = - mixed_size.x * power / 10;
		stretched_dest_real.y = - mixed_size.y * power / 10;
		if (mixed_ratio.w < mixed_ratio.h) {
			stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
		} else {
			stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
		}
#else
		stretched_dest_real.x = - mixed_size.x;
		stretched_dest_real.y = - mixed_size.y;
#endif
		if (stretched_size.x < 0) {
			stretched_size.x = 0;
			stretched_size.w = display_size.w;
		}
		if (stretched_size.y < 0) {
			stretched_size.y = 0;
			stretched_size.h = display_size.h;
		}
#ifndef USE_GTK
		stretched_dest_real.x += stretched_size.x;
		stretched_dest_real.y += stretched_size.y;
#endif
	}

	// sufMixed size normalize 2^n for opengl
	mixed_max_size.w = 0;
	mixed_max_size.h = 0;
	// 65536,32768,16384,8192,4096,2048,1024,512,256,128,64,32,16
	for(int i=65536; i>=16; i = (i >> 1)) {
		if (source_size.w <= i) mixed_max_size.w = i;
		if (source_size.h <= i) mixed_max_size.h = i;
	}

	reMix = mixed_size;
	reSuf = stretched_size;

#ifdef USE_OPENGL
#ifndef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	src_tex_l = (GLfloat)mixed_size.x / mixed_max_size.w;
	src_tex_t = (GLfloat)mixed_size.y / mixed_max_size.h;
	src_tex_r = (GLfloat)(mixed_size.x + mixed_size.w) / mixed_max_size.w;
	src_tex_b = (GLfloat)(mixed_size.y + mixed_size.h) / mixed_max_size.h;
#endif
	if (stretched_size.w < display_size.w) src_pyl_r = (GLfloat)stretched_size.w / display_size.w;
	else src_pyl_r = 1.0f;
	if (stretched_size.h < display_size.h) src_pyl_b = (GLfloat)stretched_size.h / display_size.h;
	else src_pyl_b = 1.0f;
	src_pyl_l = -src_pyl_r;
	src_pyl_t = src_pyl_b;
	src_pyl_b = -src_pyl_b;

	rePyl.left = stretched_size.x;
	rePyl.top = stretched_size.y;
	rePyl.right = stretched_size.x + stretched_size.w;
	rePyl.bottom = stretched_size.y + stretched_size.h;
#endif

	change_rec_video_size(pConfig->screen_video_size);

	first_invalidate = true;
	screen_size_changed = false;
#ifdef _DEBUG_LOG
	logging->out_debugf(_T("set_display_size: w:%d h:%d power:%d %s"),width,height,power,now_window ? _T("window") : _T("fullscreen"));
	logging->out_debugf(_T("         display: w:%d h:%d"),display_size.w, display_size.h);
	logging->out_debugf(_T("          screen: w:%d h:%d"), screen_size.w, screen_size.h);
	logging->out_debugf(_T("   screen aspect: w:%d h:%d"), screen_aspect_size.w, screen_aspect_size.h);
	logging->out_debugf(_T("          source: w:%d h:%d"), source_size.w, source_size.h);
	logging->out_debugf(_T("   source aspect: w:%d h:%d"), source_aspect_size.w, source_aspect_size.h);
	logging->out_debugf(_T("           mixed: w:%d h:%d"), mixed_size.w, mixed_size.h);
	logging->out_debugf(_T("       mixed max: w:%d h:%d"), mixed_max_size.w, mixed_max_size.h);
	logging->out_debugf(_T("         stretch: w:%d h:%d"), stretched_size.w, stretched_size.h);
	logging->out_debugf(_T("     screen dest: x:%d y:%d"), screen_size.x, screen_size.y);
	logging->out_debugf(_T("     source dest: x:%d y:%d"), source_size.x, source_size.y);
	logging->out_debugf(_T("      mixed dest: x:%d y:%d"), mixed_size.x, mixed_size.y);
	logging->out_debugf(_T("    stretch dest: x:%d y:%d"), stretched_size.x, stretched_size.y);
	logging->out_debugf(_T(" stretch dest re: x:%d y:%d"), stretched_dest_real.x, stretched_dest_real.y);
#ifdef USE_OPENGL
	logging->out_debugf(_T("     src polygon: l:%d t:%d r:%d b:%d"), rePyl.left, rePyl.top, rePyl.right, rePyl.bottom);
	logging->out_debugf(_T("     src polygon: l:%.5f t:%.5f r:%.5f b:%.5f"), src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b);
	logging->out_debugf(_T("     src texture: l:%.5f t:%.5f r:%.5f b:%.5f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif

	calc_vm_screen_size();

	if (now_window) {
#ifndef USE_GTK
		stretched_size.x += display_margin.left;
		stretched_size.y += display_margin.top;
#endif
		reSuf.x += display_margin.left;
		reSuf.y += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T(" margin      suf: l:%d t:%d w:%d h:%d"),reSuf.x,reSuf.y,reSuf.w,reSuf.h);
#endif
#ifdef USE_OPENGL
//		reTex.x += display_margin.left;
//		reTex.y += display_margin.top;
		rePyl.left += display_margin.left;
		rePyl.top += display_margin.top;
		rePyl.right += display_margin.left;
		rePyl.bottom += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T("     src polygon: l:%d t:%d r:%d b:%d"), rePyl.left, rePyl.top, rePyl.right, rePyl.bottom);
		logging->out_debugf(_T("     src polygon: l:%.5f t:%.5f r:%.5f b:%.5f"), src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b);
		logging->out_debugf(_T("     src texture: l:%.5f t:%.5f r:%.5f b:%.5f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif
	}

	lock_screen();

#ifdef USE_SCREEN_ROTATE
	if(pConfig->monitor_type) {
		sufSource = sufRotate;
	} else
#endif
	{
		sufSource = sufOrigin;
	}

#ifdef USE_SCREEN_ROTATE
	if ((source_size.w != prev_source_size.w && source_size.h != prev_source_size.h) || stretch_changed) {
		create_offlinesurface();
	}
#endif
	create_mixedsurface();

	// send display size to vm
	set_vm_display_size();

	set_ledbox_position(now_window);

	set_msgboard_position();

	unlock_screen();
}

void EMU_OSD::set_ledbox_position(bool now_window)
{
#ifdef USE_LEDBOX
	if (gui) {
#ifdef USE_OPENGL
		if (use_opengl > 0) {
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
			gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
#else
			gui->SetLedBoxPosition(now_window, mixed_size.x, mixed_size.y, mixed_size.w, mixed_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
#endif
		} else
#endif
		{
			gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
		}
	}
#endif
}

void EMU_OSD::set_msgboard_position()
{
#ifdef USE_MESSAGE_BOARD
	if (msgboard) {
#ifdef USE_OPENGL
		if (use_opengl > 0) {
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
			msgboard->SetSize(display_size.w, display_size.h);
			msgboard->SetMessagePos(4, -4, 2);
			msgboard->SetInfoPos(-4, 4, 1);
#else
			msgboard->SetSize(source_size.w, source_size.h);
			msgboard->SetMessagePos(4 + mixed_size.x,  - 4 - source_size.h + mixed_size.y + mixed_size.h, 2);
			msgboard->SetInfoPos(-4 - mixed_size.x, 4 + mixed_size.y, 1);
#endif
		} else
#endif
		{
			msgboard->SetSize(display_size.w, display_size.h);
			msgboard->SetMessagePos(4, -4, 2);
			msgboard->SetInfoPos(-4, 4, 1);
		}
	}
#endif
}

///
/// draw src screen from virtual machine
///
void EMU_OSD::draw_screen()
{
	// don't draw screen before new screen size is applied to buffers
	if(screen_size_changed) {
		return;
	}

	lock_screen();

	if (sufOrigin->Lock()) {
		if (!pConfig->now_power_off) {
			vm->draw_screen();
		} else {
			fill_gray();
		}
		sufOrigin->Unlock();
	}

	// screen size was changed in vm->draw_screen()
	if(screen_size_changed) {
		unlock_screen();
		return;
	}

#ifdef USE_SCREEN_ROTATE
	// rotate screen
	// right turn
	// src and dst should be the same size
	if(pConfig->monitor_type) {
		int rtype = (pConfig->monitor_type & 3);
		VmSize ss, ds;
		SIZE_IN(ss, screen_size.w, screen_size.h);
		SIZE_IN(ds, source_size.w, source_size.h);
		{
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] < 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] < 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = sufOrigin->GetBuffer() + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 + rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 + rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = sufRotate->GetBuffer() + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
		}
	}
#endif

#if 0
	{
		if (FLG_SHOWLEDBOX && ledbox) {
			ledbox->Draw(*sufSource);
		}
	}
#endif

	unlock_screen();
}

///
///
///
void EMU_OSD::on_realize(GtkWidget *widget, gpointer user_data)
{
	EMU_OSD *emu = (EMU_OSD *)user_data;
	emu->realize_window(widget);
}

///
///
///
gboolean EMU_OSD::on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	EMU *emu = (EMU *)user_data;
	GUI *gui = emu->get_gui();
	if (gui) {
		gui->UpdateScreen(cr);
	}
	return FALSE;
}

///
///
///
gboolean EMU_OSD::on_expose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	EMU *emu = (EMU *)user_data;
	GUI *gui = emu->get_gui();
	if (gui) {
		GdkWindow *dwindow = gtk_widget_get_window(widget);
#if GTK_CHECK_VERSION(3,22,0)
		GdkDrawingContext *context = gdk_window_begin_draw_frame(dwindow, NULL);
		cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
		gui->UpdateScreen(cr);
		gdk_window_end_draw_frame(dwindow, context);
#else
		cairo_t *cr = gdk_cairo_create(dwindow);
		gui->UpdateScreen(cr);
		cairo_destroy(cr);
#endif
	}
	return FALSE;
}

#ifdef USE_OPENGL
void EMU_OSD::on_gl_realize(GtkGLArea *area, gpointer user_data)
{
	EMU_OSD *emu_osd = (EMU_OSD *)user_data;
	if (emu_osd) {
		emu_osd->realize_opengl(area);
	}
}

void EMU_OSD::on_gl_unrealize(GtkGLArea *area, gpointer user_data)
{
	EMU_OSD *emu_osd = (EMU_OSD *)user_data;
	if (emu_osd) {
		emu_osd->unrealize_opengl(area);
	}
}

gboolean EMU_OSD::on_gl_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
	EMU *emu = (EMU *)user_data;
	GUI *gui = emu->get_gui();
	if (gui) {
		gui->UpdateScreen(context);
	}
	return TRUE;
}
#endif

///
/// copy src screen to mix screen
///
void EMU_OSD::mix_screen_sub()
{
#if 1 /* USE_SCREEN_MIX_SURFACE */
	lock_screen();

	sufSource->StretchBlit(vm_screen_size, *sufMixed, stretched_size);

	unlock_screen();

	if (gui) {
		gui->UpdateIndicator(update_led());
	}

#ifdef USE_LEDBOX
	if (FLG_SHOWLEDBOX && ledbox) {
		ledbox->Draw(*sufMixed);
	}
#endif

#ifdef USE_MESSAGE_BOARD
	if (msgboard && FLG_SHOWMSGBOARD) {
		msgboard->Draw(*sufMixed);
	}
#endif
#endif /* USE_SCREEN_MIX_SURFACE */
}

///
/// copy src screen to mix screen and overlap a message
///
/// @return false: cannot mix (no allocate mix surface)
bool EMU_OSD::mix_screen_pa(cairo_t *cr)
{
	if (disable_screen) return false;

#ifdef USE_OPENGL
	if (screen_flags & SDL_WINDOW_OPENGL) return false;
#endif

	if (pConfig->double_buffering) {
		mix_screen_sub();

		// render screen

//		// fill black on screen
//		if (first_invalidate) {
//			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
//		}

		// paint surface
		casMixed->BlitC(cr);

		cairo_paint(cr);

		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		cairo_fill(cr);

		if (first_invalidate) {
			first_invalidate = false;
#ifdef _DEBUG
			cairo_matrix_t m2;
			cairo_get_matrix(cr, &m2);
			logging->out_debugf(_T("matrix:[%.2f, %.2f, %.2f, %.2f] %.2f, %.2f"),
				m2.xx, m2.xy, m2.yx, m2.yy, m2.x0, m2.y0);
#endif
		}

	}
	else
	{
		// render screen
		gint w, h;

		gtk_window_get_size(GTK_WINDOW(window), &w, &h);

		// fill black on screen
//		if (first_invalidate) {
//			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
//		}

//		cairo_antialias_t ar = CAIRO_ANTIALIAS_DEFAULT;
//		if (pConfig->gl_filter_type == 0) ar = CAIRO_ANTIALIAS_NONE;
//		cairo_set_antialias(cr, ar);

		if (is_fullscreen()) {
			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
			cairo_fill(cr);
			cairo_paint(cr);
		}

#if 0
			// fill black on frame area
			cairo_rectangle(cr
				, (double)-stretched_size.x
				, (double)-stretched_size.y
				, (double)w
				, (double)stretched_size.y);
			cairo_rectangle(cr
				, (double)-stretched_size.x
				, 0.0
				, (double)stretched_size.x
				, (double)mixed_size.h);
			cairo_rectangle(cr
				, (double)mixed_size.w
				, 0.0
				, (double)w - mixed_size.w
				, (double)mixed_size.h);
			cairo_rectangle(cr
				, (double)-stretched_size.x
				, (double)mixed_size.h
				, (double)w
				, (double)h - mixed_size.h);
#endif

			// paint surface
			lock_screen();
			casSource->StretchBlitC(vm_screen_size, cr, stretched_size, pConfig->gl_filter_type);
			unlock_screen();

			cairo_paint(cr);

			if (gui) {
				gui->UpdateIndicator(update_led());
			}

#ifdef USE_LEDBOX
			if (FLG_SHOWLEDBOX && ledbox) {
				ledbox->Draw(cr);
			}
#endif

#ifdef USE_MESSAGE_BOARD
			if (msgboard && FLG_SHOWMSGBOARD) {
				msgboard->Draw(cr);
			}
#endif

//			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
//			cairo_fill(cr);

			if (first_invalidate) {
				first_invalidate = false;
#ifdef _DEBUG
				cairo_matrix_t m2;
				cairo_get_matrix(cr, &m2);
				logging->out_debugf(_T("matrix:[%.2f, %.2f, %.2f, %.2f] %.2f, %.2f"),
					m2.xx, m2.xy, m2.yx, m2.yy, m2.x0, m2.y0);
#endif
			}
	}

	// invalidate window
	self_invalidate = true;
//	skip_frame = false;

	return true;
}

#ifdef USE_OPENGL
///
/// copy src screen to mix screen and overlap a message using OpenGL
///
/// @return false: cannot mix (no allocate mix surface)
bool EMU_OSD::mix_screen_gl(GdkGLContext *context)
{
	/* OpenGL ********************/
	if (!(sufMixed && sufMixed->IsEnable())) return false;

	if (!(screen_flags & SDL_WINDOW_OPENGL)) return false;

#ifndef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (texGLMixed) {
		mix_screen_sub();

		if (first_invalidate) {
#ifdef USE_OPENGL_WH_ORTHO
			if (opengl->Version() <= 1) {
				src_pyl_l = (GLfloat)rePyl.left;
				src_pyl_t = (GLfloat)rePyl.top;
				src_pyl_r = (GLfloat)rePyl.right;
				src_pyl_b = (GLfloat)rePyl.bottom;
			}
#endif
			texGLMixed->SetPos(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
			first_invalidate = false;
		}

		// draw texture using screen pixel buffer
		texGLMixed->Draw(sufMixed->Width(), sufMixed->Height(), sufMixed->GetBuffer());

		if (!self_invalidate) {
			// call this only once at first.
			change_opengl_attr();
		}
	}

#else /*  USE_SCREEN_OPENGL_MIX_ON_RENDERER */
	if (texGLMixed) {
		lock_screen();

		SDL_Rect re;
		RECT_IN(re, 0, 0, vm_screen_size.x + vm_screen_size.w, vm_screen_size.y + vm_screen_size.h);
		sufSource->Blit(*sufMain, re);

		unlock_screen();

		if (first_invalidate) {
#ifdef USE_OPENGL_WH_ORTHO
			if (opengl->Version() <= 1) {
				src_pyl_l = (GLfloat)rePyl.left;
				src_pyl_t = (GLfloat)rePyl.top;
				src_pyl_r = (GLfloat)rePyl.right;
				src_pyl_b = (GLfloat)rePyl.bottom;
			}
#endif
			texGLMixed->SetPos(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
			first_invalidate = false;
		}

		// draw texture using screen pixel buffer
		texGLMixed->Draw(sufMain->Width(), sufMain->Height(), sufMain->GetBuffer());

		if (gui) {
			gui->UpdateIndicator(update_led());
		}

#ifdef USE_LEDBOX
		if (FLG_SHOWLEDBOX && ledbox && texGLLedBox) {
			ledbox->Draw(*texGLLedBox);
		}
#endif
#ifdef USE_MESSAGE_BOARD
		if (FLG_SHOWMSGBOARD && msgboard && texGLMsgBoard) {
			msgboard->Draw(*texGLMsgBoard);
		}
#endif
		if (!self_invalidate) {
			// call this only once at first.
			change_opengl_attr();
		}
	}

#endif /* USE_SCREEN_OPENGL_MIX_ON_RENDERER */

	// invalidate window
	self_invalidate = true;
//	skip_frame = false;

	return true;
}
#endif /* USE_OPENGL */

///
/// post request screen updating to draw it on main thread
///
void EMU_OSD::need_update_screen()
{
#ifdef USE_OPENGL
	if (use_opengl && glscreen) {
#ifdef USE_GTK_GLAREA
		gtk_gl_area_queue_render(GTK_GL_AREA(glscreen));
#endif
	} else
#endif
	{
		gtk_widget_queue_draw(screen);
	}
}

///
/// pointer on source screen
///
/// @return pointer on source screen
scrntype* EMU_OSD::screen_buffer(int y)
{
	return sufOrigin->GetBuffer(y);
}

///
/// offset on source screen
///
/// @return offset on source screen
int EMU_OSD::screen_buffer_offset()
{
	return screen_size.w;
}

///
/// change screen size on vm
///
void EMU_OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	EMU::set_vm_screen_size(screen_width, screen_height, window_width, window_height, window_width_aspect, window_height_aspect);

	calc_vm_screen_size();

	first_invalidate = true;
}

void EMU_OSD::calc_vm_screen_size()
{
	calc_vm_screen_size_sub(mixed_size, vm_screen_size);

#ifdef _DEBUG
	logging->out_debugf(_T("vm_display_size: w:%d h:%d"), vm_display_size.w, vm_display_size.h);
	logging->out_debugf(_T("vm_screen_size: x:%d y:%d w:%d h:%d"), vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h);
#endif

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	src_tex_l = (float)vm_screen_size.x / (float)mixed_max_size.w;
	src_tex_r = (float)(vm_screen_size.x + vm_screen_size.w) / (float)mixed_max_size.w;
	src_tex_t = (float)vm_screen_size.y / (float)mixed_max_size.h;
	src_tex_b = (float)(vm_screen_size.y + vm_screen_size.h) / (float)mixed_max_size.h;

#ifdef _DEBUG
	logging->out_debugf(_T("src tex: l:%.3f t:%.3f r:%.3f b:%.3f"), src_tex_l, src_tex_t, src_tex_r, src_tex_b);
#endif
#endif
}

void EMU_OSD::calc_vm_screen_size_sub(const VmRectWH &src_size, VmRectWH &vm_size)
{
	double sw = (double)vm_display_size.w * src_size.w / MIN_WINDOW_WIDTH;
	vm_size.w = (int)(sw + 0.5);
	if (vm_display_size.w < SCREEN_WIDTH) {
		vm_size.x = (int)((double)screen_size.x - ((sw - vm_display_size.w) / 2.0) + 0.5);
	} else {
		vm_size.x = (int)(- ((sw - vm_display_size.w) / 2.0) + 0.5);
	}
	double sh = (double)vm_display_size.h * src_size.h / MIN_WINDOW_HEIGHT;
	vm_size.h = (int)(sh + 0.5);
	vm_size.y = (int)((double)screen_size.y - ((sh - vm_display_size.h) / 2.0) + 0.5);
}

///
/// render screen and display window
///
/// @note must be called by main thread
void EMU_OSD::update_screen_pa(cairo_t *cr)
{
//	lock_screen();

	if (mix_screen_pa(cr)) {
		gui->MixSurface();
	}
//	unlock_screen();
}

#ifdef USE_OPENGL
///
/// render screen and display window
///
/// @note must be called by main thread
void EMU_OSD::update_screen_gl(GdkGLContext *context)
{
//	lock_screen();

	if (mix_screen_gl(context)) {
		gui->MixSurface();
	}
//	unlock_screen();
}
#endif

///
/// capture current screen and save to a file
///
void EMU_OSD::capture_screen()
{
	int size = pConfig->screen_video_size;

	calc_vm_screen_size_sub(rec_video_size[size], vm_screen_size_for_rec);

	rec_video->Capture(CAPTURE_SCREEN_TYPE, vm_screen_size_for_rec, sufSource, rec_video_size[size]);
}

///
/// start recording video
///
bool EMU_OSD::start_rec_video(int type, int fps_no, bool show_dialog)
{
#ifdef USE_REC_VIDEO
	int size = pConfig->screen_video_size;
	return rec_video->Start(type, fps_no, rec_video_size[size], sufSource, show_dialog);
#else
	return false;
#endif
}

///
/// record video
///
void EMU_OSD::record_rec_video()
{
#ifdef USE_REC_VIDEO
	if (rec_video->IsRecordFrame()) {
		int size = pConfig->screen_video_size;
		rec_video->Record(rec_video_stretched_size, sufSource, rec_video_size[size]);
	}
#endif
}

/// store window position to ini file
void EMU_OSD::resume_window_placement()
{
	if (now_screenmode == NOW_FULLSCREEN) {
		pConfig->window_position_x = window_dest.x;
		pConfig->window_position_y = window_dest.y;
	} else {
		int x = 0;
		int y = 0;
		gtk_window_get_position(GTK_WINDOW(window), &x, &y);
		pConfig->window_position_x = x;
		pConfig->window_position_y = y;
	}
}

/// change window size / switch over fullscreen and window
/// @param[in] mode 0 - 7: window size  8 -:  fullscreen size  -1: switch over  -2: shift window mode
void EMU_OSD::change_screen_mode(int mode)
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
	if (!create_screen(pConfig->disp_device_no, 0, 0, pConfig->screen_width, pConfig->screen_height, screen_flags)) {
		exit(1);
	}
	first_change_screen = false;
}

/// setting window or fullscreen size to display
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
void EMU_OSD::set_window(int mode, int cur_width, int cur_height)
{
	logging->out_debugf(_T("set_window: mode:%d"), mode);

	now_resizing = true;

	if(mode >= 0 && mode < 8) {
		// go window
		if (mode >= window_mode.Count()) mode = 0;
		const CWindowMode *wm = window_mode.Get(mode);
		int width = wm->width;
		int height = wm->height;
#ifdef USE_SCREEN_ROTATE
		if (pConfig->monitor_type & 1) {
			int v = width;
			width = height;
			height = v;
		}
#endif

		pConfig->window_mode = mode;
		pConfig->disp_device_no = 0;
		pConfig->screen_width = width;
		pConfig->screen_height = height;
		window_mode_power = wm->power;

		now_screenmode = NOW_WINDOW;

		gui->ScreenModeChanged(false);

		logging->out_debugf(_T("set_window: f->w mode:%d w:%d h:%d nf:%d"), mode, width, height, (int)now_screenmode);

		// set screen size to emu class
		set_display_size(width, height, window_mode_power, true);
	}
	else if(now_screenmode != NOW_FULLSCREEN) {
		// go fullscreen

		// get current position of window
		gtk_window_get_position(GTK_WINDOW(window), &window_dest.x, &window_dest.y);

		const CVideoMode *sm = NULL;
		int disp_no = 0;
		int width = 0;
		int height = 0;
//		int left = 0;
//		int top = 0;

		if (mode >= 8) {
			// check mode number is valid
			if (!screen_mode.GetMode((mode - 8) / VIDEO_MODE_MAX, (mode - 8) % VIDEO_MODE_MAX)) {
				mode = -1;
			}
		}

		if(mode == -1) {
			// search current monitor
			disp_no = screen_mode.WithinDisp(window_dest.x, window_dest.y);
			if (disp_no < 0) {
				disp_no = 0;
			}
			// get width and height
			const CDisplayDevice *dd = screen_mode.GetDisp(disp_no);
//			left = dd->re.x;
//			top = dd->re.y;
			width = dd->re.w;
			height = dd->re.h;

			// matching width and height
			int find = screen_mode.FindMode(disp_no, width, height);
			if (find >= 0) {
				mode = find + (disp_no * VIDEO_MODE_MAX) + 8;
			} else {
				mode = 8;
			}
		} else {
			disp_no = (mode - 8) / VIDEO_MODE_MAX;
			sm = screen_mode.GetMode(disp_no, (mode - 8) % VIDEO_MODE_MAX);

			const CDisplayDevice *dd = screen_mode.GetDisp(disp_no);
//			left =  dd->re.x;
//			top = dd->re.y;
			width = sm ? sm->width : dd->re.w;
			height = sm ? sm->height : dd->re.h;
		}

		pConfig->window_mode = mode;
		pConfig->disp_device_no = disp_no;
		pConfig->screen_width = width;
		pConfig->screen_height = height;

		now_screenmode = NOW_FULLSCREEN;

		gui->ScreenModeChanged(true);

		logging->out_debugf(_T("set_window: w->f mode:%d w:%d h:%d nf:%d"), mode, width, height, (int)now_screenmode);

		// set screen size to emu class
		set_display_size(width, height, 10, false);
	} else {
		// now fullscreen
		if (mode >= screen_mode.CountMode(0) + 8) {
			mode = -1;
		}
		const CVideoMode *sm = screen_mode.GetMode(0, mode - 8);
		int width = sm ? sm->width : cur_width;
		int height = sm ? sm->height : cur_height;

		logging->out_debugf(_T("set_window: f->f mode:%d w:%d h:%d nf:%d"), mode, width, height, (int)now_screenmode);

		// set screen size to emu class
		set_display_size(width, height, 10, false);
	}
	now_resizing = false;
}

// ----------
#ifdef USE_OPENGL

void EMU_OSD::initialize_opengl()
{
}

void EMU_OSD::realize_opengl(GtkGLArea *area)
{
	if (!use_opengl) {
		return;
	}
#ifdef USE_GTK_GLAREA
	gtk_gl_area_make_current(area);
	GError *err = gtk_gl_area_get_error(area);
	if (err != NULL) {
		logging->out_logf(LOG_ERROR, _T("gtk_gl_area_make_current: %s"), err->message);
		return;
	}
#endif
	if (!opengl) {
		opengl = COpenGL::New();
		if (!texGLMixed) {
			texGLMixed = COpenGLTexture::New(opengl, 0);
		}
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
		if (!texGLLedBox) {
			texGLLedBox = COpenGLTexture::New(opengl, 1);
		}
		if (!texGLMsgBoard) {
			texGLMsgBoard = COpenGLTexture::New(opengl, 2);
		}
#endif
	}

	// set pixel format
//	SDL_UTILS::set_pixel_format_for_opengl(pixel_format);
	pixel_format->PresetRGBA();

	//
	opengl->Initialize();

	// create texture
	create_opengl_texture();

	opengl->InitViewport(mixed_size.w, mixed_size.h);

	GdkWindow *gwin = gtk_widget_get_window(window);
	GdkDisplay *gdisp = gdk_window_get_display(gwin);
	pConfig->use_opengl = opengl->SetInterval(pConfig->use_opengl, GDK_DISPLAY_XDISPLAY(gdisp));

//	opengl->CreateBuffer(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);
}

void EMU_OSD::unrealize_opengl(GtkGLArea *area)
{
	if (!use_opengl) return;

	release_opengl_texture();
	opengl->Terminate();

	if (glscreen) {
		glscreen = NULL;
	}
}

void EMU_OSD::create_opengl_texture()
{
	if (!window) return;

//	if (mix_texture_name) return;
	if (!texGLMixed) return;

	// create
//	mix_texture_name = opengl->CreateTexture(pConfig->gl_filter_type);
	texGLMixed->Create(pConfig->gl_filter_type);
	texGLMixed->CreateBuffer(src_pyl_l, src_pyl_t, src_pyl_r, src_pyl_b, src_tex_l, src_tex_t, src_tex_r, src_tex_b);

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	texGLLedBox->Create(pConfig->gl_filter_type);
# ifdef linux
	texGLLedBox->PresetBGRA();
# endif
	texGLMsgBoard->Create(pConfig->gl_filter_type);
#endif
}

void EMU_OSD::change_opengl_attr()
{
	if (!use_opengl) return;

	texGLMixed->SetFilter(pConfig->gl_filter_type);

#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	texGLLedBox->SetFilter(pConfig->gl_filter_type);
	texGLMsgBoard->SetFilter(pConfig->gl_filter_type);
#endif
}

void EMU_OSD::release_opengl()
{
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	delete texGLMsgBoard;
	texGLMsgBoard = NULL;
	delete texGLLedBox;
	texGLLedBox = NULL;
	delete sufMain;
	sufMain = NULL;
#endif

	delete texGLMixed;
	texGLMixed = NULL;

	delete opengl;
	opengl = NULL;
}

void EMU_OSD::release_opengl_texture()
{
#ifdef USE_SCREEN_OPENGL_MIX_ON_RENDERER
	if (texGLMsgBoard) {
		texGLMsgBoard->Release();
	}
	if (texGLLedBox) {
		texGLLedBox->Release();
	}
#endif

	if (texGLMixed) {
//		mix_texture_name = opengl->ReleaseTexture();
		texGLMixed->Release();
	}
}

void EMU_OSD::set_opengl_attr()
{
}

void EMU_OSD::set_opengl_poly(int width, int height)
{
}

#endif // USE_OPENGL
