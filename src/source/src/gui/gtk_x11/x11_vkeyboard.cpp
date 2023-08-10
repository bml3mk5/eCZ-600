/** @file x11_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.21 -

	@brief [ virtual keyboard ]
*/

#include "x11_vkeyboard.h"

#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_VKEYBOARD)

#include <SDL_syswm.h>
#include "../../emu.h"

extern EMU *emu;

namespace Vkbd {

//
// for X window system
//

VKeyboard::VKeyboard() : Base()
{
	display = NULL;
	screen = 0;
	parent = 0;
	window = 0;
	gc = 0;
	event_masks = 0;
	memset(&image, 0, sizeof(image));
	a_wm_protocol = None;
	a_delete_window = None;

	event_working = false;
	need_expose = false;
	expose_num = 0;
}

VKeyboard::~VKeyboard()
{
}

void VKeyboard::Show(bool show)
{
	if (window) {
#ifdef VKEYBOARD_DEBUG
		fprintf(stderr, "VKeyboard::Show()\n");
#endif

		XWindowAttributes attr;
		XGetWindowAttributes(display, window, &attr);
		if (show) {
			// show
			if (attr.map_state == IsUnmapped) {
				XMapWindow(display, window);
//				XMapRaised(display, window);
				need_update_window();
				update_window();
			}
		} else {
			// hide
			if (attr.map_state != IsUnmapped) {
				XUnmapWindow(display, window);
//				XWithdrawWindow(display, window, screen);
			}
		}
	}
}

bool VKeyboard::Create(const char *res_path)
{
	if (window) return true;

	if (!load_bitmap(res_path)) {
		closed = true;
		return false;
	}

	// Get X11 Display from SDL
#ifndef USE_GTK
	SDL_SysWMinfo sdl_info;
	SDL_VERSION(&sdl_info.version);
#ifndef USE_SDL2
	SDL_GetWMInfo(&sdl_info);
#else
	SDL_GetWindowWMInfo(emu->get_window(), &sdl_info);
#endif

	display = sdl_info.info.x11.display;
	if (!display) {
		closed = true;
		return false;
	}
#ifndef USE_SDL2
	parent = sdl_info.info.x11.wmwindow;
#else
	parent = sdl_info.info.x11.window;
#endif
#else /* !USE_GTK */
	GtkWidget *gtk_win = emu->get_window();
	GdkWindow *gdk_win = gtk_widget_get_window(gtk_win);
	parent = GDK_WINDOW_XID(gdk_win);
	GdkDisplay *gdk_dis = gdk_window_get_display(gdk_win);
	display = GDK_DISPLAY_XDISPLAY(gdk_dis);
#endif /* USE_GTK */

	parent = find_toplevel_window(DefaultRootWindow(display), parent);

	screen = DefaultScreen(display);

	unsigned long black;

	black = BlackPixel(display, screen);

	// Set size hint
	XSizeHints hints;
	hints.x = 0; hints.y = 0;
	hints.width = pSurface->Width();
	hints.height = pSurface->Height();

	// No Resizeable
	hints.base_width = hints.min_width = hints.max_width = hints.width;
	hints.base_height = hints.min_height = hints.max_height = hints.height;
	hints.width_inc = hints.height_inc = 0;

	hints.flags = PSize | PBaseSize | PMinSize | PMaxSize | PResizeInc; // | PPosition;

	// Create Window
	window = XCreateSimpleWindow(display
		, DefaultRootWindow(display)
//		, parent
		, hints.x, hints.y, hints.width, hints.height, 0, black, black);
	if (!window) {
		closed = true;
		return false;
	}

	// Set size hints
	XSetWMNormalHints(display, window, &hints);

//	// Ignore management by window manager
//	XSetWindowAttributes attr;
//	attr.override_redirect = True;
//	XChangeWindowAttributes(display, window, CWOverrideRedirect, &attr);

	// Create Graphic Context
	gc = XCreateGC(display, window, 0, 0);
	XSetBackground(display, gc, black);
	XSetForeground(display, gc, black);

	// Attach pixel buffer to XImage structure.
	image.width = pSurface->Width();
	image.height = pSurface->Height();
	image.xoffset = 0;
	image.format = ZPixmap;
	image.data = (char *)pSurface->GetBuffer();
	image.byte_order = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? MSBFirst : LSBFirst;
	image.bitmap_unit = pSurface->BitsPerPixel();
	image.bitmap_bit_order = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? MSBFirst : LSBFirst;
	image.bitmap_pad = pSurface->BitsPerPixel();
	image.depth = DefaultDepth(display, screen);
	image.bytes_per_line = pSurface->BytesPerLine();
	image.bits_per_pixel = pSurface->BitsPerPixel();
	CPixelFormat format = pSurface->GetPixelFormat();
	image.red_mask = format.Rmask;
	image.green_mask = format.Gmask;
	image.blue_mask = format.Bmask;
	XInitImage(&image);

	// Set Acceptable Event
	event_masks = FocusChangeMask | Button1MotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | StructureNotifyMask;
	XSelectInput(display, window, event_masks);

	// Title
	XStoreName(display, window, "Virtual Keyboard");

	// catch delete window message from window manager
	a_wm_protocol = XInternAtom(display, "WM_PROTOCOLS", True);
	a_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(display, window, &a_delete_window, 1);

	// Mapping Window on display
//	XMapWindow(display, window);

	// Wait Raised window
//	XFlush(display);

#if 0
	// See "Xm/MwmUtils.h" about MOTIF definitions.
	#define MWM_HINTS_FUNCTIONS (1L << 0)
	#define MWM_HINTS_DECORATIONS (1L << 1)
	#define MWM_FUNC_CLOSE (1L << 5)
	#define MWM_DECOR_BORDER (1L << 1)

	// Send non border window styles to window manager using Motif property.
	Atom _MOTIF_WM_HINTS = XInternAtom(display, "_MOTIF_WM_HINTS", True);
	if (_MOTIF_WM_HINTS != None) {
		struct {
			unsigned long flags;
			unsigned long functions;
			unsigned long decorations;
			long input_mode;
			unsigned long status;
		} MWMHints = {
			MWM_HINTS_DECORATIONS,
			 0,
			 0,	// no border
			 0, 0
		};

		XChangeProperty(display, window,
			_MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace,
			(unsigned char *)&MWMHints, sizeof(MWMHints) / 4
		);
	}
#endif

	// accept event
	event_working = true;

	//
	expose_num = 0;

//	adjust_window_size();
	set_dist();
	Base::Create();

	return true;
}

void VKeyboard::Close()
{
#ifdef VKEYBOARD_DEBUG
	fprintf(stderr, "VKeyboard::Close()\n");
#endif

	event_working = false;

	if (window) {
		XDestroyWindow(display, window);
		window = 0;
	}
	if (gc) {
		XFreeGC(display, gc);
		gc = 0;
	}

	unload_bitmap();

	Base::Close();
}

/// process pending event
///
/// @note must be called before processing SDL_Event on SDL1
///       because SDL1 catches all events using XNextEvent.
void VKeyboard::ProcessEvent()
{
	XEvent event;

	while (event_working
	&& XCheckWindowEvent(display, window, event_masks, &event)) {
		switch(event.type) {
		case Expose:
			paint_window(event.xexpose.x, event.xexpose.y
				, event.xexpose.width, event.xexpose.height);
			break;
		case ButtonPress:
			if (event.xbutton.button == 1) {
				MouseDown(event.xbutton.x, event.xbutton.y);
			}
			break;
		case ButtonRelease:
			if (event.xbutton.button == 1) {
				MouseUp();
			}
			break;
		case ClientMessage:
//fprintf(stderr, "ClientMessage\n");
			if (event.xclient.message_type == a_wm_protocol
			 && event.xclient.data.l[0] == a_delete_window) {
				Close();
			}
			break;
		default:
//fprintf(stderr, "Event: %d\n",event.type);
			break;
		}
	}
}

void VKeyboard::PostProcessEvent()
{
	if (need_expose) {
		if (window) {
			for(int n=0; n<expose_num; n++) {
				VmRectWH *re = &expose_rect[n];
				XClearArea(display, window, re->x, re->y, re->w, re->h, True);	// send expose event
			}
		}
		expose_num = 0;
		need_expose = false;
	}
}

void VKeyboard::set_dist()
{
	if (!window) return;

	XWindowAttributes attr, pattr;

	XGetWindowAttributes(display, window, &attr);
	XGetWindowAttributes(display, parent, &pattr);

	int wp = pattr.width;
	int hp = pattr.height;
	int w = attr.width;
	int h = attr.height;

	int x = (wp - w) / 2 + pattr.x;
	int y = pattr.y;

//	if (y + h > desktop_height) {
//		y = (desktop_height - h);
//	}
	XMoveWindow(display, window, x, y);
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!window) return;

	Base::need_update_window(info, onoff);

	if (expose_num < VKEYBOARD_EXPOSE_MAXNUM) {
		VmRectWH *re = &expose_rect[expose_num];
		re->x = info->re.left;
		re->y = info->re.top;
		re->w = info->re.right - info->re.left;
		re->h = info->re.bottom - info->re.top;
		expose_num++;
	}
}

void VKeyboard::need_update_window()
{
	if (!window) return;

	if (expose_num < VKEYBOARD_EXPOSE_MAXNUM) {
		VmRectWH *re = &expose_rect[expose_num];
		re->x = 0;
		re->y = 0;
		re->w = image.width;
		re->h = image.height;
		expose_num++;
	}
}

void VKeyboard::update_window()
{
	need_expose = true;
}

void VKeyboard::paint_window(int x, int y, int width, int height)
{
	if (!window) return;

	XPutImage(display, window, gc, &image,
		x, y, x, y,
		width, height
	);
}

/// get toplevel window id
/// Toplevel window means a window just under the root window.
///
/// @note Some window manager wrap a window in a window to attach a title bar.
Window VKeyboard::find_toplevel_window(Window start, Window window)
{
	Window root;
	Window parent;
	Window *children = NULL;
	Window match = 0;
	unsigned int num_children = 0;
	unsigned int i;

//	fprintf(stderr, "VKeyboard::find_toplevel_window() start:%lx window:%lx\n", start, window);

	if (XQueryTree(display, start
		, &root, &parent, &children, &num_children) == 0) {
		return 0;
	}
	if (num_children > 0) {
		for(i=0; i<num_children; i++) {
			if (children[i] == window) {
				match = start;
				break;
			}
		}
		for(i=0; i<num_children && match == 0; i++) {
			match = find_toplevel_window(children[i], window);
		}
	}
	if (children) {
		XFree(children);
	}

//	fprintf(stderr, "VKeyboard::find_toplevel_window() match:%lx\n", match);

	return match;
}

} /* namespace Vkbd */

#endif /* SDL_VIDEO_DRIVER_X11 && USE_X11_VKEYBOARD */
