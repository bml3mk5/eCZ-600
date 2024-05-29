/** @file win_screen.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya, Sasaji
	@date   2006.08.18 -

	@brief [ win32 screen ]
*/

#include "win_emu.h"
#include "win_main.h"
#include "../../vm/vm.h"
#include "../../gui/gui.h"
#include "../../config.h"
#ifdef USE_MESSAGE_BOARD
#include "../../msgboard.h"
#endif
#ifdef USE_LEDBOX
#include "../../gui/ledbox.h"
#endif
#include "win_csurface.h"
#include "../../video/rec_video.h"

void EMU_OSD::EMU_SCREEN()
{
	dwStyle = 0;
	hWindow = NULL;

#ifdef USE_DIRECT3D
	pD3D = NULL;
	pD3Device = NULL;
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf = NULL;
#endif
#endif
	pD3Dorigin = NULL;
	pD3Dsource = NULL;
#ifdef USE_SCREEN_ROTATE
	pD3Drotate = NULL;
#endif
	lpD3DBmp = NULL;
#endif
#ifdef USE_LEDBOX
	ledbox = NULL;
#endif
}

void EMU_OSD::initialize_screen()
{
	EMU::initialize_screen();

#ifdef USE_DIRECT3D
	initialize_d3device(hMainWindow);
	create_d3device(hMainWindow);
//	create_d3dofflinesurface();
#endif

#ifdef USE_MESSAGE_BOARD
	msgboard = new MsgBoard(hMainWindow, this);
	if (msgboard) {
		msgboard->InitScreen(screen_size.w, screen_size.h);
		msgboard->SetVisible(FLG_SHOWMSGBOARD ? true : false);
	}
#endif

#ifdef USE_LEDBOX
	if (gui) {
		ledbox = gui->CreateLedBox();
	}
#endif
}

///
/// release screen
///
void EMU_OSD::release_screen()
{
#ifdef USE_DIRECT3D
	release_d3device();
	terminate_d3device();
#endif
	if (gui) {
		gui->ReleaseLedBox();
	}

	EMU::release_screen();
}

///
/// create / recreate window
///
/// @param[in] disp_no : (unused)
/// @param[in] x       : window position x
/// @param[in] y       : window position y
/// @param[in] width   : window width
/// @param[in] height  : window height
/// @param[in] flags   : (unused)
/// @return true / false
bool EMU_OSD::create_screen(int disp_no, int x, int y, int width, int height, uint32_t flags)
{
	if (screen_mode.WithinDisp(x, y) < 0) {
		x = CW_USEDEFAULT;
		y = CW_USEDEFAULT;
	}

	//  | WS_THICKFRAME
	dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
	dwExStyle = WS_EX_COMPOSITED | WS_EX_TRANSPARENT;

	hWindow = ::CreateWindowEx(dwExStyle, _T(CLASS_NAME), _T(DEVICE_NAME), dwStyle,
	                         x, y, width, height, NULL, NULL, hInstance, NULL);

	hMainWindow = hWindow;

	return (hWindow != NULL);
}
///
/// create / recreate offline surface
///
bool EMU_OSD::create_offlinesurface()
{
	if (!pixel_format) return false;

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
	if (!create_mixedsurface()) {
		return false;
	}
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

#ifdef USE_DIRECT3D
	create_d3dofflinesurface();
#endif

	disable_screen &= ~DISABLE_SURFACE;

	return true;
}
///
/// create / recreate mixed surface
///
bool EMU_OSD::create_mixedsurface()
{
	if (sufMixed) {
		sufMixed->Release();
		if (pConfig->double_buffering) {
			if (!sufMixed->Create(display_size.w, display_size.h, *pixel_format)) {
				logging->out_log(LOG_ERROR, _T("EMU_OSD::create_offlinesurface sufMixed failed."));
				return false;
			}
		}
	}
	return true;
}

/// calculate the client size of window or fullscreen
///
/// @param [in] width : new width or -1 set current width
/// @param [in] height : new height or -1 set current height
/// @param [in] power : magnify x 10
/// @param [in] now_window : true:window / false:fullscreen
void EMU_OSD::set_display_size(int width, int height, int power, bool now_window)
{
	bool display_size_changed = false;
	bool stretch_changed = false;

	if(width != -1 && (display_size.w != width || display_size.h != height)) {
		display_size.w = width;
		display_size.h = height;
		display_size_changed = stretch_changed = true;
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
			stretched_dest_real.x = - mixed_size.x * display_size.h / source_aspect_size.h;
			stretched_dest_real.y = - mixed_size.y * display_size.h / source_aspect_size.h;
			if(stretched_size.w > display_size.w) {
				stretched_size.w = display_size.w;
				stretched_size.h = (display_size.w * source_aspect_size.h) / source_aspect_size.w;
				stretched_dest_real.x = - mixed_size.x * display_size.w / source_aspect_size.w;
				stretched_dest_real.y = - mixed_size.y * display_size.w / source_aspect_size.w;
			}
			if (mixed_ratio.w < mixed_ratio.h) {
				stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
			} else {
				stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
			}
			stretched_size.x = (display_size.w - stretched_size.w) / 2;
			stretched_size.y = (display_size.h - stretched_size.h) / 2;
			stretched_dest_real.x += stretched_size.x;
			stretched_dest_real.y += stretched_size.y;
#ifdef USE_SCREEN_D3D_TEXTURE
			int ply_w = stretched_size.w;
			int ply_h = stretched_size.h;
			if (mixed_ratio.w < mixed_ratio.h) {
				ply_h = ply_h * mixed_ratio.h / mixed_ratio.w;
			} else {
				ply_w = ply_w * mixed_ratio.w / mixed_ratio.h;
			}
			reD3Dply.left = (display_size.w - ply_w) / 2;
			reD3Dply.top  = (display_size.h - ply_h) / 2;
			reD3Dply.right = reD3Dply.left + ply_w;
			reD3Dply.bottom = reD3Dply.top + ply_h;
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
				stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.w / min_rsize.w)) / 2;

				if ((mixed_size.h * min_rsize.h * display_size.w / min_rsize.w / min_size.h) >= display_size.h) {
					int mh = display_size.h * min_rrsize.h * min_rsize.w / display_size.w / min_size.h;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = 0;
					stretched_size.h = display_size.h;
					stretched_dest_real.y = (display_size.h - (mixed_rsize.h * display_size.w / min_rsize.w)) / 2;
#ifdef USE_SCREEN_D3D_TEXTURE
					reD3Dply.top  = stretched_dest_real.y;
					reD3Dply.bottom = reD3Dply.top + mixed_rsize.h * display_size.w / min_rsize.w;
#endif
				} else {
					int mh = mixed_rrsize.h;
					int sh = mixed_size.h * display_size.w / min_rsize.w;
					mixed_size.y = (mixed_size.h - mh) / 2;
					mixed_size.h = mh;
					stretched_size.y = (display_size.h - sh) / 2;
					stretched_size.h = sh;
					stretched_dest_real.y = stretched_size.y;
#ifdef USE_SCREEN_D3D_TEXTURE
					reD3Dply.top  = stretched_dest_real.y;
					reD3Dply.bottom = reD3Dply.top + stretched_size.h;
#endif
				}
#ifdef USE_SCREEN_D3D_TEXTURE
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = - reD3Dply.left + stretched_size.w;
#endif

			} else {
				// magnify = display_size.h / min_rsize.h

				mixed_size.y = (mixed_size.h - min_size.h) / 2;
				mixed_size.h = min_size.h;
				stretched_size.y = 0;
				stretched_size.h = display_size.h;
				stretched_dest_real.y = (display_size.h - (mixed_rsize.h * display_size.h / min_rsize.h)) / 2;

				if ((mixed_size.w * min_rsize.w * display_size.h / min_rsize.h / min_size.w) >= display_size.w) {
					int mw = display_size.w * min_rrsize.w * min_rsize.h / display_size.h / min_size.w;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = 0;
					stretched_size.w = display_size.w;
					stretched_dest_real.x = (display_size.w - (mixed_rsize.w * display_size.h / min_rsize.h)) / 2;
#ifdef USE_SCREEN_D3D_TEXTURE
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = reD3Dply.left + mixed_rsize.w * display_size.h / min_rsize.h;
#endif
				} else {
					int mw = mixed_rrsize.w;
					int sw = mixed_size.w * display_size.h / min_rsize.h;
					mixed_size.x = (mixed_size.w - mw) / 2;
					mixed_size.w = mw;
					stretched_size.x = (display_size.w - sw) / 2;
					stretched_size.w = sw;
					stretched_dest_real.x = stretched_size.x;
#ifdef USE_SCREEN_D3D_TEXTURE
					reD3Dply.left = stretched_dest_real.x;
					reD3Dply.right = reD3Dply.left + stretched_size.w;
#endif
				}
#ifdef USE_SCREEN_D3D_TEXTURE
				reD3Dply.top  = stretched_dest_real.y;
				reD3Dply.bottom = - reD3Dply.top + stretched_size.h;
#endif

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
		stretched_dest_real.x = - mixed_size.x * power / 10;
		stretched_dest_real.y = - mixed_size.y * power / 10;
		if (mixed_ratio.w < mixed_ratio.h) {
			stretched_dest_real.y = stretched_dest_real.y * mixed_ratio.h / mixed_ratio.w;
		} else {
			stretched_dest_real.x = stretched_dest_real.x * mixed_ratio.w / mixed_ratio.h;
		}
#ifdef USE_SCREEN_D3D_TEXTURE
		int ply_w = stretched_size.w;
		int ply_h = stretched_size.h;
		if (mixed_ratio.w < mixed_ratio.h) {
			ply_h = ply_h * mixed_ratio.h / mixed_ratio.w;
		} else {
			ply_w = ply_w * mixed_ratio.w / mixed_ratio.h;
		}
		reD3Dply.left = (display_size.w - ply_w) / 2;
		reD3Dply.top  = (display_size.h - ply_h) / 2;
		reD3Dply.right = reD3Dply.left + ply_w;
		reD3Dply.bottom = reD3Dply.top + ply_h;
#endif
		if (stretched_size.x < 0) {
			stretched_size.x = 0;
			stretched_size.w = display_size.w;
		}
		if (stretched_size.y < 0) {
			stretched_size.y = 0;
			stretched_size.h = display_size.h;
		}
		stretched_dest_real.x += stretched_size.x;
		stretched_dest_real.y += stretched_size.y;
	}

#ifdef USE_SMOOTH_STRETCH
	int new_pow_x = 1, new_pow_y = 1;
	while(stretched_size.w > source_size.w * new_pow_x) {
		new_pow_x++;
	}
	while(stretched_size.h > source_size.h * new_pow_y) {
		new_pow_y++;
	}

	// support high quality stretch only for x1 window size in gdi mode
	if(new_pow_x > 1 && new_pow_y > 1) {
		new_pow_x = new_pow_y = 1;
	}

//	if(stretched_size.w == source_width * new_pow_x && stretched_size.h == source_height * new_pow_y) {
//		new_pow_x = new_pow_y = 1;
//	}
	if(stretch_power.w != new_pow_x || stretch_power.h != new_pow_y) {
		stretch_power.w = new_pow_x;
		stretch_power.h = new_pow_y;
		stretch_changed = true;
	}

	// re create surface later 
#if 0
	if(stretch_changed) {
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
	logging->out_debugf(_T("         stretch: w:%d h:%d"), stretched_size.w, stretched_size.h);
	logging->out_debugf(_T("     screen dest: x:%d y:%d"), screen_size.x, screen_size.y);
	logging->out_debugf(_T("     source dest: x:%d y:%d"), source_size.x, source_size.y);
	logging->out_debugf(_T("      mixed dest: x:%d y:%d"), mixed_size.x, mixed_size.y);
	logging->out_debugf(_T("    stretch dest: x:%d y:%d"), stretched_size.x, stretched_size.y);
	logging->out_debugf(_T(" stretch dest re: x:%d y:%d"), stretched_dest_real.x, stretched_dest_real.y);
#endif

	SetRect(&reD3Dmix, mixed_size.x, mixed_size.y, mixed_size.x + mixed_size.w, mixed_size.y + mixed_size.h);
	SetRect(&reD3Dsuf, stretched_size.x, stretched_size.y, stretched_size.x + stretched_size.w, stretched_size.y + stretched_size.h);

#ifdef _DEBUG_LOG
	logging->out_debugf(_T("D3D          mix: l:%d t:%d r:%d b:%d w:%d h:%d"),reD3Dmix.left,reD3Dmix.top,reD3Dmix.right,reD3Dmix.bottom,mixed_size.w,mixed_size.h);
	logging->out_debugf(_T("             suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
#ifdef USE_SCREEN_D3D_TEXTURE
	logging->out_debugf(_T("             ply: l:%d t:%d r:%d b:%d"),reD3Dply.left,reD3Dply.top,reD3Dply.right,reD3Dply.bottom);
#endif
#endif
	if (now_window) {
		stretched_size.x += display_margin.left;
		stretched_size.y += display_margin.top;
		reD3Dsuf.left += display_margin.left;
		reD3Dsuf.top += display_margin.top;
		reD3Dsuf.right += display_margin.left;
		reD3Dsuf.bottom += display_margin.top;
#ifdef _DEBUG_LOG
		logging->out_debugf(_T(" margin      suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
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

#ifdef USE_DIRECT3D
	if (pD3Device) {
		HRESULT hre;

		if (now_window) {
			d3dpp.BackBufferWidth = 0;
			d3dpp.BackBufferHeight = 0;
			d3dpp.Windowed = TRUE;
		} else {
			d3dpp.BackBufferWidth = display_size.w;
			d3dpp.BackBufferHeight = display_size.h;
			d3dpp.Windowed = TRUE;
		}
		// save buffer to dib temporary
		if (pConfig->use_direct3d && pD3Dorigin && sufOrigin && sufOrigin->IsEnable()) {
#ifdef USE_SCREEN_D3D_TEXTURE
			copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
			copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
		}
		// re create back buffer to fit to the main screen(window)
		hre = reset_d3device(hMainWindow);
		if (hre == D3D_OK) {
			pD3Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 0.0, 0);
			// restore buffer from dib
			if (pD3Dorigin && sufOrigin && sufOrigin->IsEnable()) {
#ifdef USE_SCREEN_D3D_TEXTURE
				copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), false);
#else
				copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), false);
#endif
			}
		}
		if (pD3Device) {
			LPDIRECT3DSURFACE9 suf = NULL;
			D3DSURFACE_DESC desc;
			pD3Device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&suf);
			if (suf != NULL) {
				suf->GetDesc(&desc);
				logging->out_debugf(_T("d3dbackbuffer: w:%d h:%d"),desc.Width,desc.Height);
				suf->Release();

				if (desc.Width < (UINT)(reD3Dsuf.right - reD3Dsuf.left)) {
					// adjust width to fit the surface
					uint32_t new_mw = (uint32_t)((uint64_t)desc.Width * (reD3Dmix.right - reD3Dmix.left) / (reD3Dsuf.right - reD3Dsuf.left));
					uint32_t new_sw = (uint32_t)((uint64_t)new_mw * (reD3Dsuf.right - reD3Dsuf.left) / (reD3Dmix.right - reD3Dmix.left));
					reD3Dmix.right = reD3Dmix.left + new_mw;
					reD3Dsuf.right = reD3Dsuf.left + new_sw;
				}
				if (desc.Height < (UINT)(reD3Dsuf.bottom - reD3Dsuf.top)) {
					// adjust height to fit the surface
					uint32_t new_mh = (uint32_t)((uint64_t)desc.Height * (reD3Dmix.bottom - reD3Dmix.top) / (reD3Dsuf.bottom - reD3Dsuf.top));
					uint32_t new_sh = (uint32_t)((uint64_t)new_mh * (reD3Dsuf.bottom - reD3Dsuf.top) / (reD3Dmix.bottom - reD3Dmix.top));
					reD3Dmix.bottom = reD3Dmix.top + new_mh;
					reD3Dsuf.bottom = reD3Dsuf.top + new_sh;
				}

				logging->out_debugf(_T("          mix: l:%d t:%d r:%d b:%d"),reD3Dmix.left,reD3Dmix.top,reD3Dmix.right,reD3Dmix.bottom);
				logging->out_debugf(_T("          suf: l:%d t:%d r:%d b:%d"),reD3Dsuf.left,reD3Dsuf.top,reD3Dsuf.right,reD3Dsuf.bottom);
#ifdef USE_SCREEN_D3D_TEXTURE
				logging->out_debugf(_T("          ply: l:%d t:%d r:%d b:%d"),reD3Dply.left,reD3Dply.top,reD3Dply.right,reD3Dply.bottom);
#endif
			}
		}
#ifdef USE_SCREEN_ROTATE
		if(pConfig->monitor_type) {
			pD3Dsource = pD3Drotate;
		} else
#endif
		{
			pD3Dsource = pD3Dorigin;
		}

#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dsource->SetD3DTexturePosition(reD3Dply);
#endif
	}

#endif	/* USE_DIRECT3D */

#ifdef USE_SCREEN_ROTATE
	if ((source_size.w != prev_source_size.w && source_size.h != prev_source_size.h) || stretch_changed) {
		create_offlinesurface();
	}
#else
	create_mixedsurface();
#endif

	// send display size to vm
	set_vm_display_size();

	set_ledbox_position(now_window);

	set_msgboard_position();

	unlock_screen();

	calc_vm_screen_size();
}

void EMU_OSD::set_ledbox_position(bool now_window)
{
#ifdef USE_LEDBOX
	if (gui) {
#ifdef USE_SCREEN_D3D_TEXTURE
		if (pConfig->use_direct3d) {
			gui->SetLedBoxPosition(now_window, 0, 0, display_size.w, display_size.h, pConfig->led_pos | (is_fullscreen() ? 0x10 : 0));
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
#ifdef USE_SCREEN_D3D_TEXTURE
		if (pConfig->use_direct3d) {
			msgboard->SetSize(display_size.w, display_size.h);
			msgboard->SetMessagePos(4, -4, 2);
			msgboard->SetInfoPos(-4, 4, 1);
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

#ifdef USE_DIRECT3D
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;
#ifdef USE_SCREEN_D3D_TEXTURE
	UINT level = 0;
#endif

	lpD3DBmp = NULL;
	if (pD3Dorigin) {
#ifdef USE_SCREEN_D3D_TEXTURE
		hre = pD3Dorigin->GetD3DTexture()->LockRect(level, &pLockedRect, NULL, 0 /* D3DLOCK_DISCARD */);
#else
		hre = pD3Dorigin->GetD3DSurface()->LockRect(&pLockedRect, NULL, 0);
#endif
		if (hre == D3D_OK) {
			lpD3DBmp = (scrntype *)pLockedRect.pBits;
		}
	}
#endif
	// draw screen
	if (!pConfig->now_power_off) {
		vm->draw_screen();
	} else {
		fill_gray();
	}

#ifdef USE_DIRECT3D
	if (pD3Dorigin)	{
#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dorigin->GetD3DTexture()->UnlockRect(level);
#else
		pD3Dorigin->GetD3DSurface()->UnlockRect();
#endif
	}
#endif

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
#ifdef USE_DIRECT3D
		if (pConfig->use_direct3d) {
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] < 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] < 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			D3DLOCKED_RECT pLockedRectOrigin;
			D3DLOCKED_RECT pLockedRectRotate;
			hre = pD3Dorigin->LockRect(&pLockedRectOrigin, NULL, 0);
			hre = pD3Drotate->LockRect(&pLockedRectRotate, NULL, 0);
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = (scrntype *)pLockedRectOrigin.pBits + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 + rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 + rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = (scrntype *)pLockedRectRotate.pBits + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
			pD3Drotate->UnlockRect();
			pD3Dorigin->UnlockRect();
		} else
#endif
		{
			int nw0 = rotate_matrix[rtype].x[0] < 0 ? (ss.w - 1) : 0;
			int nh0 = rotate_matrix[rtype].y[0] >= 0 ? (ss.h - 1) : 0;
			int nw1 = rotate_matrix[rtype].x[1] >= 0 ? (ss.w - 1) : 0;
			int nh1 = rotate_matrix[rtype].y[1] < 0 ? (ss.h - 1) : 0;
			for(int sy = 0; sy < ss.h; sy++) {
				scrntype* src = sufOrigin->GetBuffer() + ss.w * sy;
				for(int sx = 0; sx < ss.w; sx++) {
					int dx = nw0 + rotate_matrix[rtype].x[0] * sx + nh0 - rotate_matrix[rtype].y[0] * sy; 
					int dy = nw1 - rotate_matrix[rtype].x[1] * sx + nh1 + rotate_matrix[rtype].y[1] * sy;
					scrntype* dst = sufRotate->GetBuffer() + ds.w * dy + dx;
					*dst = *src;
					src++;
				}
			}
		}
	}
#endif

#if 0
	// ledbox is rendered to source surface, because this is the surface to record a video 
#ifdef USE_DIRECT3D
#ifndef USE_SCREEN_D3D_TEXTURE
	if (pConfig->use_direct3d) {
		if (FLG_SHOWLEDBOX && gui) {
			gui->DrawLedBox(pD3Dsource->GetD3DSurface());
		}
	} else
#endif
#endif
	{
		if (FLG_SHOWLEDBOX && gui) {
			gui->DrawLedBox(sufSource->GetDC());
		}
	}
#endif

	unlock_screen();
}

///
/// copy src screen to mix screen and overlap a message
///
/// @return false: cannot mix (no allocate mix surface)
bool EMU_OSD::mix_screen()
{
	lock_screen();

#ifdef USE_DIRECT3D
	if (pD3Device != NULL && pConfig->use_direct3d) {
		// mix d3d screen buffer
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
#if 0
		pD3Device->UpdateSurface(pD3Dsource, NULL, pD3Dmixsuf, NULL);

		RECT src_re;
		src_re.left = vm_screen_size.x;
		src_re.top = vm_screen_size.y;
		src_re.right = vm_screen_size.w - vm_screen_size.x - vm_screen_size.x;
		src_re.bottom = vm_screen_size.h - vm_screen_size.y - vm_screen_size.y;
		RECT dst_re;
		dst_re.left = dst_re.top = 0;
		dst_re.right = sufMixed->Width();
		dst_re.bottom = sufMixed->Height();
		pD3Device->StretchRect(pD3Dsource, &src_re, pD3Dmixsuf, &dst_re, D3DTEXF_NONE);
#else
		HDC src_dc;
		HDC dst_dc;
		pD3Dsource->GetD3DSurface()->GetDC(&src_dc);
		pD3Dmixsuf->GetD3DSurface()->GetDC(&dst_dc);
		StretchBlt(dst_dc, 0, 0, sufMixed->Width(), sufMixed->Height(), src_dc, vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h, SRCCOPY);
		pD3Dsource->GetD3DSurface()->ReleaseDC(src_dc);
		pD3Dmixsuf->GetD3DSurface()->ReleaseDC(dst_dc);
#endif
		if (FLG_SHOWLEDBOX && gui) {
			gui->DrawLedBox(pD3Dmixsuf->GetD3DSurface());
		}

#ifdef USE_MESSAGE_BOARD
		if (msgboard) {
			msgboard->Draw(pD3Dmixsuf->GetD3DSurface());
		}
#endif
#endif /* USE_SCREEN_D3D_MIX_SURFACE */
#endif /* !USE_SCREEN_D3D_TEXTURE */
	} else
#endif /* !USE_DIRECT3D */
	{
		// mix dib screen buffer
#ifdef USE_SCREEN_MIX_SURFACE
		if (pConfig->double_buffering) {
			//copy to render buffer with stretched size
			sufSource->StretchBlit(vm_screen_size, *sufMixed, stretched_size);
#ifdef USE_LEDBOX
			if (FLG_SHOWLEDBOX && gui) {
				ledbox->Draw(sufMixed->GetDC());
			}
#endif
#ifdef USE_MESSAGE_BOARD
			if (msgboard) {
				msgboard->Draw(sufMixed->GetDC());
			}
#endif
		}
#endif /* USE_SCREEN_MIX_SURFACE */
	}

	// stretch screen
#ifdef USE_SMOOTH_STRETCH
	//	if(stretch_screen) {
	if(stretch_screen && (!pConfig->use_direct3d)) {
#if 0
		StretchBlt(sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, sufSource->GetDC(), 0, 0, source_size.w, source_size.h, SRCCOPY);
#else
		// about 50% faster than StretchBlt()
		scrntype* src = sufSource->GetBuffer() + source_size.w * (source_size.h - 1);
		scrntype* dst = sufStretch1->GetBuffer() + source_size.w * stretch_power.w * (source_size.h * stretch_power.h - 1);
		int data_len = source_size.w * stretch_power.w;

		for(int y = 0; y < source_size.h; y++) {
			if(stretch_power.w != 1) {
				scrntype* dst_tmp = dst;
				for(int x = 0; x < source_size.w; x++) {
					scrntype c = src[x];
					for(int px = 0; px < stretch_power.w; px++) {
						dst_tmp[px] = c;
					}
					dst_tmp += stretch_power.w;
				}
			}
			else {
				// faster than memcpy()
				for(int x = 0; x < source_size.w; x++) {
					dst[x] = src[x];
				}
			}
			if(stretch_power.h != 1) {
				scrntype* src_tmp = dst;
				for(int py = 1; py < stretch_power.h; py++) {
					dst -= data_len;
					// about 10% faster than memcpy()
					for(int x = 0; x < data_len; x++) {
						dst[x] = src_tmp[x];
					}
				}
			}
			src -= source_size.w;
			dst -= data_len;
		}
#endif

		StretchBlt(sufStretch2->GetDC(), 0, 0, stretched_size.w, stretched_size.h, sufStretch1->GetDC(), 0, 0, source_size.w * stretch_power.w, source_size.h * stretch_power.h, SRCCOPY);
	}
#endif

	unlock_screen();

	return true;
}

///
/// post request screen updating to draw it on main thread
///
void EMU_OSD::need_update_screen()
{
	// invalidate window
	UINT flags = RDW_INVALIDATE | RDW_INTERNALPAINT | (first_invalidate ? RDW_ERASE : RDW_NOERASE);
	RedrawWindow(hMainWindow, NULL, NULL, flags);
	self_invalidate = true;
//	skip_frame = false;
}

///
/// pointer on source screen
///
/// @return pointer on source screen
scrntype* EMU_OSD::screen_buffer(int y)
{
#ifdef USE_DIRECT3D
	if (lpD3DBmp && pConfig->use_direct3d) {
		return lpD3DBmp + screen_size.w * y;
	} else
#endif
	return sufOrigin->GetBuffer() + screen_size.w * (screen_size.h - y - 1);
}

///
/// offset on source screen
///
/// @return offset on source screen
int EMU_OSD::screen_buffer_offset()
{
#ifdef USE_DIRECT3D
	if (lpD3DBmp && pConfig->use_direct3d) {
		return screen_size.w;
	} else
#endif
	return -screen_size.w;
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

#ifdef USE_SCREEN_D3D_TEXTURE
	if (pD3Dorigin) {
		float uw = (float)vm_display_size.w / MIN_WINDOW_WIDTH;
		float vh = (float)vm_display_size.h / MIN_WINDOW_HEIGHT;

		float ux = 0.0;
		if (vm_display_size.w < SCREEN_WIDTH) {
			ux = ((float)screen_size.x - (float)screen_size.x * uw) / (float)screen_size.w;
		} else {
			ux = - (float)screen_size.x * uw / (float)screen_size.w;
		}
		float vy = ((float)screen_size.y - (float)screen_size.y * vh) / (float)screen_size.h;

		pD3Dorigin->SetD3DTexturePositionUV(ux, vy, uw, vh);

#ifdef _DEBUG
		logging->out_debugf(_T("D3DTexturePositionUV: ux:%.3f vy:%.3f uw:%.3f vh:%.3f"), ux, vy, uw, vh);
#endif
	}
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

#ifdef USE_DIRECT3D
///
/// update screen using Direct3D
///
void EMU_OSD::update_screen_d3d()
{
	HRESULT hre = D3D_OK;

	lock_screen();

	if (lost_d3device) {
		// can use a device?
		hre = pD3Device->TestCooperativeLevel();
		if (hre == D3DERR_DEVICELOST) {
			// now lost device
			lost_d3device = true;
		} else if (hre == D3DERR_DEVICENOTRESET) {
			// reset device to use
			if (reset_d3device(hMainWindow) == D3D_OK) {
				lost_d3device = false;
			}
		} else {
			lost_d3device = false;
		}
	} else {
		if(!disable_screen && pD3Device) {
#ifdef USE_SCREEN_D3D_TEXTURE
			// set texture and draw screen
//			pD3Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x10, 0x10, 0x10), 0.0f, 0);

			pD3Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
//			pD3Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//			pD3Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			pD3Device->SetSamplerState(0, D3DSAMP_MAGFILTER, pConfig->d3d_filter_type);
			pD3Device->SetSamplerState(0, D3DSAMP_MINFILTER, pConfig->d3d_filter_type);

			hre = pD3Device->BeginScene();
			if (hre == D3D_OK) {
				DWORD fvf = D3DFVF_XYZRHW | D3DFVF_TEX1;
				// notice x,y,z,rhw,u,v to screen
				hre = pD3Device->SetFVF(fvf);

				pD3Dsource->DrawD3DTexture(pD3Device);

#ifdef USE_LEDBOX
				ledbox->Draw(pD3Device);
#endif
#ifdef USE_MESSAGE_BOARD
				msgboard->Draw(pD3Device);
#endif

				pD3Device->EndScene();

			}
			if (hre == D3D_OK) {
				hre = pD3Device->Present(NULL,NULL,NULL,NULL);
			}

#else /* !USE_SCREEN_D3D_TEXTURE */
			// get back buffer
			PDIRECT3DSURFACE9 pD3Dsuf = NULL;
			pD3Device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pD3Dsuf);
			if (pD3Dsuf) {
#ifdef USE_SCREEN_D3D_MIX_SURFACE
				// copy to back buffer from offline surface
				hre = pD3Device->StretchRect(pD3Dmixsuf->GetD3DSurface(), &reD3Dmix, pD3Dsuf, &reD3Dsuf, (_D3DTEXTUREFILTERTYPE)pConfig->d3d_filter_type);
#else
				// copy to back buffer from offline surface
				RECT src_re;
				src_re.left = vm_screen_size.x;
				src_re.top = vm_screen_size.y;
				src_re.right = vm_screen_size.w - vm_screen_size.x - vm_screen_size.x;
				src_re.bottom = vm_screen_size.h - vm_screen_size.y - vm_screen_size.y;
				hre = pD3Device->StretchRect(pD3Dsource->GetD3DSurface(), &src_re, pD3Dsuf, &reD3Dsuf, (_D3DTEXTUREFILTERTYPE)pConfig->d3d_filter_type);
#ifdef USE_LEDBOX
				if (FLG_SHOWLEDBOX && gui) {
					gui->DrawLedBox(pD3Dsuf);
				}
#endif
#ifdef USE_MESSAGE_BOARD
				if (msgboard) {
					msgboard->Draw(pD3Dsuf);
				}
#endif

#endif
				pD3Dsuf->Release();
			}
			// draw
			hre = pD3Device->Present(NULL,NULL,NULL,NULL);
#endif
			first_invalidate = self_invalidate = false;
		}
		if (hre == D3DERR_DEVICELOST) {
			// now lost device
			lost_d3device = true;
		}
	}

	unlock_screen();
}
#endif

///
/// render screen and display window
///
/// @note must be called by main thread
void EMU_OSD::update_screen(HDC hdc)
{
	if (!initialized) return;

//	lock_screen();

	if (gui) {
		gui->UpdateIndicator(update_led());
	}

	mix_screen();

#ifdef USE_DIRECT3D
	if (pD3Device != NULL && pConfig->use_direct3d) {
		update_screen_d3d();
	} else
#endif
	{
		update_screen_dc(hdc);
	}

//	unlock_screen();
}

///
/// update screen using DC
///
void EMU_OSD::update_screen_dc(HDC hdc)
{
#ifdef USE_BITMAP
	if(first_invalidate || !self_invalidate) {
		HDC hmdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = LoadBitmap(hInstance, _T("IDI_BITMAP1"));
		BITMAP bmp;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		int w = (int)bmp.bmWidth;
		int h = (int)bmp.bmHeight;
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hmdc, hBitmap);
		BitBlt(hdc, 0, 0, w, h, hmdc, 0, 0, SRCCOPY);
		SelectObject(hmdc, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hmdc);
	}
#endif
	if(!disable_screen) {
#ifdef USE_LED
		// 7-seg LEDs
		for(int i = 0; i < MAX_LEDS; i++) {
			int x = leds[i].x;
			int y = leds[i].y;
			int w = leds[i].width;
			int h = leds[i].height;
			BitBlt(hdc, x, y, w, h, hdcDib, x, y, SRCCOPY);
		}
#else
		// standard screen
#ifdef USE_SMOOTH_STRETCH
		if(stretch_screen) {
			BitBlt(hdc, screen_size.x, screen_size.y, stretched_size.w, stretched_size.h, sufStretch2->GetDC(), 0, 0, SRCCOPY);
		}
		else {
#endif
#ifdef USE_SCREEN_MIX_SURFACE
			if (pConfig->double_buffering) {
				// copy from mixed buffer
				BitBlt(hdc, 0, 0, display_size.w, display_size.h, sufMixed->GetDC(), 0, 0, SRCCOPY);
			} else
#endif
			{
				// draw to device context directly
				BeginPath(hdc);
				if(stretched_size.w == vm_screen_size.w && stretched_size.h == vm_screen_size.h) {
					BitBlt(hdc, stretched_size.x, stretched_size.y, stretched_size.w, stretched_size.h, sufSource->GetDC(), vm_screen_size.x, vm_screen_size.y, SRCCOPY);
				}
				else {
					StretchBlt(hdc, stretched_size.x, stretched_size.y, stretched_size.w, stretched_size.h, sufSource->GetDC(), vm_screen_size.x, vm_screen_size.y, vm_screen_size.w, vm_screen_size.h, SRCCOPY);
				}
#ifdef USE_LEDBOX
				ledbox->Draw(hdc);
#endif
#ifdef USE_MESSAGE_BOARD
				msgboard->Draw(hdc);
#endif
				EndPath(hdc);
			}
#ifdef USE_SMOOTH_STRETCH
		}
#endif

#ifdef USE_ACCESS_LAMP
		// draw access lamps of drives
		int status = vm->access_lamp() & 7;
		static int prev_status = 0;
		bool render_in = (status != 0);
		bool render_out = (prev_status != status);
		prev_status = status;

		if(render_in || render_out) {
			COLORREF crColor = RGB((status & 1) ? 255 : 0, (status & 2) ? 255 : 0, (status & 4) ? 255 : 0);
			int right_bottom_x = screen_dest_x + stretched_size.w;
			int right_bottom_y = screen_dest_y + stretched_size.h;

			for(int y = display_size.h - 6; y < display_size.h; y++) {
				for(int x = display_size.w - 6; x < display_size.w; x++) {
					if((x < right_bottom_x && y < right_bottom_y) ? render_in : render_out) {
						SetPixelV(hdc, x, y, crColor);
					}
				}
			}
		}
#endif

#endif
		first_invalidate = self_invalidate = false;
	}
}

#ifdef USE_DIRECT3D
#ifdef USE_SCREEN_D3D_TEXTURE
void EMU_OSD::copy_d3dtex_dib(PDIRECT3DTEXTURE9 tex, scrntype *buf, bool to_dib)
{
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;
	UINT level = 0;

	hre = tex->LockRect(level, &pLockedRect, NULL, 0);
	if (hre == D3D_OK) {
		scrntype *src;
		scrntype *out;
		if (to_dib) {
			src = (scrntype *)pLockedRect.pBits;
			out = buf;
		} else {
			src = buf;
			out = (scrntype *)pLockedRect.pBits;
		}
		out += screen_size.w * (screen_size.h - 1);
		int data_len = screen_size.w;

		for(int y = 0; y < screen_size.h; y++) {
			for(int i = 0; i < data_len; i++) {
				out[i] = src[i];
			}
			src += data_len;
			out -= data_len;
		}
		tex->UnlockRect(level);
	}
}
#else
void EMU_OSD::copy_d3dsuf_dib(PDIRECT3DSURFACE9 suf, scrntype *buf, bool to_dib)
{
	HRESULT hre;
	D3DLOCKED_RECT pLockedRect;

	hre = suf->LockRect(&pLockedRect, NULL, 0);
	if (hre == D3D_OK) {
		scrntype *src;
		scrntype *out;
		if (to_dib) {
			src = (scrntype *)pLockedRect.pBits;
			out = buf;
		} else {
			src = buf;
			out = (scrntype *)pLockedRect.pBits;
		}
		out += screen_size.w * (screen_size.h - 1);
		int data_len = screen_size.w;

		for(int y = 0; y < screen_size.h; y++) {
			for(int i = 0; i < data_len; i++) {
				out[i] = src[i];
			}
			src += data_len;
			out -= data_len;
		}
		suf->UnlockRect();
	}
}
#endif
#endif

///
/// capture current screen and save to a file
///
void EMU_OSD::capture_screen()
{
	lock_screen();

#ifdef USE_DIRECT3D
	if (pD3Dorigin != NULL && pConfig->use_direct3d) {
#ifdef USE_SCREEN_D3D_TEXTURE
		copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
		copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
	}
#endif
//	copy_dib_rec_video();
	calc_vm_screen_size_sub(rec_video_size[pConfig->screen_video_size], vm_screen_size_for_rec);

//	rec_video->Capture(CAPTURE_SCREEN_TYPE, rec_video_stretched_size, sufOrigin, rec_video_size[pConfig->screen_video_size]);
	rec_video->Capture(CAPTURE_SCREEN_TYPE, vm_screen_size_for_rec, sufOrigin, rec_video_size[pConfig->screen_video_size]);

	unlock_screen();
}

///
/// start recording video
///
//bool EMU::start_rec_video(int type, int fps_no, bool show_dialog)
//{
//#ifdef USE_REC_VIDEO
//	int size = pConfig->screen_video_size;
//	return rec_video->Start(type, fps_no, rec_video_size[size], sufOrigin, show_dialog);
//#else
//	return false;
//#endif
//}

///
/// record video
///
void EMU_OSD::record_rec_video()
{
#ifdef USE_REC_VIDEO
	if (rec_video->IsRecordFrame()) {
#ifdef USE_DIRECT3D
		if (pD3Device != NULL && pConfig->use_direct3d) {
#ifdef USE_SCREEN_D3D_TEXTURE
			copy_d3dtex_dib(pD3Dorigin->GetD3DTexture(), sufOrigin->GetBuffer(), true);
#else
			copy_d3dsuf_dib(pD3Dorigin->GetD3DSurface(), sufOrigin->GetBuffer(), true);
#endif
		}
#endif
		rec_video->Record(rec_video_stretched_size, sufOrigin, rec_video_size[pConfig->screen_video_size]);
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
		WINDOWINFO wid;
		GetWindowInfo(hMainWindow, &wid);
		pConfig->window_position_x = wid.rcWindow.left;
		pConfig->window_position_y = wid.rcWindow.top;
	}
}

/// show window and move position
/// @param [in] dest_x        : move to position x (unless set SWP_NOMOVE on flags)
/// @param [in] dest_y        : move to position y (unless set SWP_NOMOVE on flags)
/// @param [in] client_width  : current desktop width
/// @param [in] client_height : current desktop height
/// @param [in] flags         : SetWindowPos flags
void EMU_OSD::set_client_pos(int dest_x, int dest_y, int client_width, int client_height, UINT flags)
{
	WINDOWINFO wid;
	GetWindowInfo(hMainWindow, &wid);

	int width = (wid.rcClient.left - wid.rcWindow.left) + client_width + (wid.rcWindow.right - wid.rcClient.right);
	int height = (wid.rcClient.top - wid.rcWindow.top) + client_height + (wid.rcWindow.bottom - wid.rcClient.bottom);

	flags |= SWP_NOZORDER;

#ifdef _DEBUG
	logging->out_debugf(_T("set_client_pos: x:%d y:%d w:%d h:%d flags:%x"), dest_x, dest_y, width, height, flags);
#endif
	SetWindowPos(hMainWindow, HWND_TOP, dest_x, dest_y, width, height, flags);
}

/// setting window or fullscreen size to display
/// @param [in] mode 0 .. 7 window mode / 8 .. 23 fullscreen mode / -1 want to go fullscreen, but unknown mode
/// @param [in] cur_width  current desktop width if mode is -1
/// @param [in] cur_height current desktop height if mode is -1
void EMU_OSD::set_window(int mode, int cur_width, int cur_height)
{
	static LONG style = WS_VISIBLE;
	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);

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
//		RECT rect = {0, 0, width, height};
		RECT rect = {-display_margin.left, -display_margin.top, width + display_margin.right, height + display_margin.bottom};
//		AdjustWindowRectEx(&rect, dwStyle, TRUE, 0);
		int dest_x = 0;
		int dest_y = 0;

		if(now_screenmode == NOW_FULLSCREEN) {
			// change fullscreen to window
			ChangeDisplaySettings(NULL, 0);
			SetWindowLong(hMainWindow, GWL_STYLE, style);
			if (!first_change_screen) {
				// rollback position of window mode
				dest_x = (int)window_dest.x;
				dest_y = (int)window_dest.y;
			}
#ifdef _DEBUG
			logging->out_debugf(_T("set_window: f->w x:%d y:%d w:%d h:%d"), dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top);
#endif

			now_screenmode = NOW_WINDOW;
			// show menu
			gui->ScreenModeChanged(now_screenmode == NOW_FULLSCREEN);
			// resize client
			set_client_pos(dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		}
		else {
			if (now_screenmode == NOW_MAXIMIZE) {
				// restore from maximize
				ShowWindow(hMainWindow, SW_RESTORE);
				now_screenmode = NOW_WINDOW;
			}
			// get current position of window
			WINDOWINFO wid;
			GetWindowInfo(hMainWindow, &wid);
			dest_x = wid.rcWindow.left;
			dest_y = wid.rcWindow.top;

#ifdef _DEBUG
			logging->out_debugf(_T("set_window: w->w x:%d y:%d w:%d h:%d"), dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top);
#endif
			// resize client
			set_client_pos(dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
		}
		pConfig->window_mode = prev_window_mode = mode;
		pConfig->window_position_x = dest_x;
		pConfig->window_position_y = dest_y;
		pConfig->disp_device_no = 0;
		pConfig->screen_width = width;
		pConfig->screen_height = height;
		window_mode_power = wm->power;

		// set screen size to emu class
		set_display_size(width, height, window_mode_power, true);
//#ifdef USE_MOUSE_ABSOLUTE
//		set_mouse_position();
//#endif
	}
	else if(now_screenmode != NOW_FULLSCREEN) {
		// go fullscreen

		// get current position of window
		WINDOWINFO wid;
		GetWindowInfo(hMainWindow, &wid);
		window_dest.x = wid.rcWindow.left;
		window_dest.y = wid.rcWindow.top;

		const CVideoMode *sm = NULL;
		const TCHAR *dev_name = NULL;
		int disp_no = 0;
		int width = 0;
		int height = 0;
		int left = 0;
		int top = 0;

		if (mode >= 8) {
			// check mode number is valid
			if (screen_mode.GetMode((mode - 8) / VIDEO_MODE_MAX, (mode - 8) % VIDEO_MODE_MAX) < 0) {
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
			dev_name = dd->name.Get();
			left = dd->re.x;
			top = dd->re.y;
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
			dev_name = dd->name.Get();
			left =  dd->re.x;
			top = dd->re.y;
			width = sm ? sm->width : dd->re.w;
			height = sm ? sm->height : dd->re.h;
		}

		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = desktop_bpp;
		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;

		// try fullscreen
		if(ChangeDisplaySettingsEx(dev_name, &dev, NULL, CDS_TEST, NULL) == DISP_CHANGE_SUCCESSFUL) {
			if (now_screenmode == NOW_MAXIMIZE) {
				// restore from maximize
				ShowWindow(hMainWindow, SW_RESTORE);
			}
			GetWindowPlacement(hMainWindow, &place);
			ChangeDisplaySettingsEx(dev_name, &dev, NULL, CDS_FULLSCREEN, NULL);
			style = GetWindowLong(hMainWindow, GWL_STYLE);
			SetWindowLong(hMainWindow, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hMainWindow, HWND_TOP, left, top, width, height, SWP_SHOWWINDOW);
			SetCursorPos(width / 2, height / 2);
			prev_screenmode = now_screenmode;
			now_screenmode = NOW_FULLSCREEN;

			pConfig->window_mode = mode;
			pConfig->disp_device_no = disp_no;
			pConfig->screen_width = width;
			pConfig->screen_height = height;

			// remove menu
			gui->ScreenModeChanged(true);

			// set screen size to emu class
			set_display_size(width, height, 10, false);

//#ifdef USE_MOUSE_ABSOLUTE
//			set_mouse_position();
//#endif
		}
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
	first_change_screen = false;
	now_resizing = false;
//	mute_sound(false);
	set_pause(1, false);
}

// ----------
#ifdef USE_DIRECT3D
void EMU_OSD::initialize_d3device(HWND hWnd)
{
	ZeroMemory(&d3dpp , sizeof(D3DPRESENT_PARAMETERS));

	// D3D
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D) {
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT , &d3ddm);

		d3dpp.BackBufferWidth = 0;
		d3dpp.BackBufferHeight = 0;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality = 0;
		d3dpp.EnableAutoDepthStencil = FALSE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = TRUE;

		set_d3dpresent_interval();
	}

#ifdef USE_SCREEN_D3D_TEXTURE
	pD3Dorigin = new CD3DTexture();
#else
	pD3Dorigin = new CD3DSurface();
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf = new CD3DSurface();
#endif
#endif

}

HRESULT EMU_OSD::create_d3device(HWND hWnd)
{
	D3DCAPS9 d3dcaps;
	HRESULT hre;

	lost_d3device = false;
	enable_direct3d = false;

	hre = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING ,&d3dpp, &pD3Device);
	if (hre != D3D_OK) {
		hre = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING ,&d3dpp, &pD3Device);
	}
	if (hre != D3D_OK) {
		// disable direct 3d
		pD3Device = NULL;
	}

	if (pD3Device) {
		enable_direct3d = true;

		pD3Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 0.0, 0);

		pD3Device->GetDeviceCaps(&d3dcaps);
		if ((d3dcaps.Caps & D3DCAPS_READ_SCANLINE) == 0 || (d3dcaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) == 0) {
			if (pConfig->use_direct3d != 0) pConfig->use_direct3d = 2;
		}
	}

	return hre;
}

void EMU_OSD::create_d3dofflinesurface()
{
	HRESULT hre;
	if (!pD3Device) return;

	do {
#ifdef USE_SCREEN_D3D_TEXTURE
		pD3Dorigin->ReleaseD3DTexture();
		lpD3DBmp = NULL;

//		hre = pD3Device->CreateTexture(screen_size.w, screen_size.h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pD3Dorigin, NULL);
		hre = pD3Dorigin->CreateD3DTexture(pD3Device, screen_size.w, screen_size.h);
		if (hre != D3D_OK) {
			break;
		}
		// set position
		pD3Dorigin->SetD3DTexturePosition(reD3Dply);
		// create polygon structure
		set_d3d_viewport();

#ifdef USE_LEDBOX
		ledbox->CreateTexture(pD3Device);
		gui->ChangeLedBoxPosition(pConfig->led_pos);
#endif
#ifdef USE_MESSAGE_BOARD
		msgboard->CreateTexture(pD3Device);
#endif

#else /* !USE_SCREEN_D3D_TEXTURE */
		pD3Dorigin->ReleaseD3DSurface();
		lpD3DBmp = NULL;

//		hre = pD3Dorigin->CreateD3DMemorySurface(pD3Device, screen_size.w, screen_size.h);
		hre = pD3Dorigin->CreateD3DSurface(pD3Device, screen_size.w, screen_size.h);
		if (hre != D3D_OK) {
			break;
		}
		hre = create_d3dmixedsurface();
		if (hre != D3D_OK) {
			break;
		}
#endif /* USE_SCREEN_D3D_TEXTURE */

#ifdef USE_SCREEN_ROTATE
		pD3Drotate->ReleaseD3DSurface();

		hre = pD3Drotate->CreateD3DMemorySurface(pD3Device, source_size.w, source_size.h);
		if (hre != D3D_OK) {
			break;
		}

		if(pConfig->monitor_type) {
			pD3Dsource = pD3Drotate;
		} else
#endif
		{
			pD3Dsource = pD3Dorigin;
		}
	} while(0);

	if (hre != D3D_OK) {
		release_d3device();
	}
}

HRESULT EMU_OSD::create_d3dmixedsurface()
{
	HRESULT hre = D3D_OK;
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf->ReleaseD3DSurface();

	hre = pD3Dmixsuf->CreateD3DSurface(pD3Device, source_size.w, source_size.h);
#endif
	return hre;
}

HRESULT EMU_OSD::reset_d3device(HWND hWnd)
{
	HRESULT hre = D3D_OK;

	hre = pD3Device->Reset(&d3dpp);
	if (hre == D3DERR_DEVICELOST) {
		return hre;
	}
	if (hre != D3D_OK) {
		// re create
		release_d3device();

		hre = create_d3device(hWnd);
		if (hre == D3D_OK) {
			create_d3dofflinesurface();
		}
	}
	return hre;
}

void EMU_OSD::release_d3device()
{
#ifdef USE_SCREEN_ROTATE
	pD3Drotate->ReleaseD3DSurface();
#endif
	pD3Dsource = NULL;

#ifdef USE_SCREEN_D3D_TEXTURE
	pD3Dorigin->ReleaseD3DTexture();
	lpD3DBmp = NULL;
#ifdef USE_LEDBOX
	ledbox->ReleaseTexture();
#endif
#ifdef USE_MESSAGE_BOARD
	msgboard->ReleaseTexture();
#endif
#else
	pD3Dorigin->ReleaseD3DSurface();
	lpD3DBmp = NULL;

#ifdef USE_SCREEN_D3D_MIX_SURFACE
	pD3Dmixsuf->ReleaseD3DSurface();
#endif
#endif
	if (pD3Device) {
		pD3Device->Release();
		pD3Device = NULL;
	}
}

void EMU_OSD::terminate_d3device()
{
	delete pD3Dorigin;
	pD3Dorigin = NULL;
#ifndef USE_SCREEN_D3D_TEXTURE
#ifdef USE_SCREEN_D3D_MIX_SURFACE
	delete pD3Dmixsuf;
	pD3Dmixsuf = NULL;
#endif
#endif
	if (pD3D) {
		pD3D->Release();
	}
}

void EMU_OSD::set_d3dpresent_interval()
{
	if (pConfig->use_direct3d == 2) {
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	} else {
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
}

#ifdef USE_SCREEN_D3D_TEXTURE
void EMU_OSD::set_d3d_viewport()
{
	D3DVIEWPORT9 vp;

	vp.X = 0;
	vp.Y = 0;
	vp.Width = display_size.w;
	vp.Height = display_size.h;
	vp.MinZ = 0.0;
	vp.MaxZ = 1.0;

	// view port
	pD3Device->SetViewport(&vp);

	logging->out_debugf(_T(" D3D Viewport x:%d y:%d w:%d h:%d"), vp.X, vp.Y, vp.Width, vp.Height);
}
#endif

#endif // USE_DIRECT3D
