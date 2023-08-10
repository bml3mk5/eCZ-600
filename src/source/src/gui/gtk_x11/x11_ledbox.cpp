/** @file x11_ledbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.01.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#include "x11_ledbox.h"

#if defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_LEDBOX)

#include <SDL_syswm.h>
#include "../../emu.h"

extern EMU *emu;

//
// for X window system
//

LedBox::LedBox() : LedBoxBase()
{
	display = NULL;
	screen = 0;
	parent = 0;
	window = 0;
	gc = 0;
	event_masks = 0;
#ifdef USE_XINITIMAGE
	memset(&image, 0, sizeof(image));
#else
	image = NULL;
#endif

	event_working = false;
	need_expose = false;

	pStart.x = 0;
	pStart.y = 0;
}

LedBox::~LedBox()
{
	destroy_dialog();
}

void LedBox::show_dialog()
{
	if (window) {
#ifdef LEDBOX_DEBUG
		fprintf(stderr, "LedBox::show_dialog()\n");
#endif

		XWindowAttributes attr;
//		XEvent event;

		XGetWindowAttributes(display, window, &attr);
		if (visible && !inside) {
			// show
			if (attr.map_state == IsUnmapped) {
				XMapWindow(display, window);
//				XMapRaised(display, window);
				need_update_dialog();
				// wait until unmapped
//				XIfEvent(display, &event, &is_map_notify, (XPointer)&window);
//				XSync(display, False);
//				XFlush(display);
//				XSetInputFocus(display, parent, RevertToPointerRoot, CurrentTime);
			}
		} else {
			// hide
			if (attr.map_state != IsUnmapped) {
				XUnmapWindow(display, window);
//				XWithdrawWindow(display, window, screen);
				// wait until unmapped
//				XIfEvent(display, &event, &is_unmap_notify, (XPointer)&window);
//				XSync(display, False);
//				XFlush(display);
			}
		}
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

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
	if (!display) return;
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
	hints.width = Width(); hints.height = Height();

	// No Resizeable
	hints.base_width = hints.min_width = hints.max_width = hints.width + 2;
	hints.base_height = hints.min_height = hints.max_height = hints.height + 2;
	hints.width_inc = hints.height_inc = 0;

	hints.flags = PSize | PBaseSize | PMinSize | PMaxSize | PResizeInc; // | PPosition;

	// Create Window
	window = XCreateSimpleWindow(display
		, DefaultRootWindow(display)
//		, parent
		, hints.x, hints.y, hints.width, hints.height, 1, black, black);
	if (!window) return;

	// Set size hints
	XSetWMNormalHints(display, window, &hints);

	// Ignore management by window manager
	XSetWindowAttributes attr;
	attr.override_redirect = True;
	XChangeWindowAttributes(display, window, CWOverrideRedirect, &attr);

	// Create Graphic Context
	gc = XCreateGC(display, window, 0, 0);
	XSetBackground(display, gc, black);
	XSetForeground(display, gc, black);

	// Attach pixel buffer to XImage structure.
#ifdef USE_XINITIMAGE
//	char *data = (char *)malloc(sizeof(char) * surface->h * surface->pitch);
	image.width = Width();
	image.height = Height();
	image.xoffset = 0;
	image.format = ZPixmap;
	image.data = (char *)GetBuffer();
	image.byte_order = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? MSBFirst : LSBFirst;
	image.bitmap_unit = BitsPerPixel();
	image.bitmap_bit_order = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? MSBFirst : LSBFirst;
	image.bitmap_pad = BitsPerPixel();
	image.depth = DefaultDepth(display, screen);
	image.bytes_per_line = BytesPerLine();
	image.bits_per_pixel = BitsPerPixel();
	image.red_mask = suf->format->Rmask;
	image.green_mask = suf->format->Gmask;
	image.blue_mask = suf->format->Bmask;
	XInitImage(&image);
#else
	char *data = (char *)malloc(sizeof(char) * Height() * BytesPerLine());
	int depth = DefaultDepth(display, screen);
	image = XCreateImage(display,
		CopyFromParent, 
		depth / 8,
		ZPixmap, 0, data,
		Width(), Height(), BitsPerPixel(),
		BytesPerLine());
#endif

	// Set Acceptable Event
	event_masks = FocusChangeMask | Button1MotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | StructureNotifyMask;
	XSelectInput(display, window, event_masks);

	// Mapping Window on display
//	XMapWindow(display, window);

	// Wait Raised window
//	XFlush(display);

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

	// accept event
	event_working = true;
}

void LedBox::destroy_dialog()
{
#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::destroy_dialog()\n");
#endif

	event_working = false;

	if (gc) {
		XFreeGC(display, gc);
		gc = 0;
	}
	if (window) {
		XDestroyWindow(display, window);
		window = 0;
	}
#ifdef USE_XINITIMAGE
//	if (image.data) {
//		free(image.data);
//		image.data = NULL;
//	}
#else
	if (image) {
		XDestroyImage(image);
		image = NULL;
	}
#endif
}

/// process pending event
///
/// @note must be called before processing SDL_Event on SDL1
///       because SDL1 catches all events using XNextEvent.
void LedBox::ProcessEvent()
{
	XEvent event;

	while (event_working
	&& XCheckWindowEvent(display, window, event_masks, &event)) {
		switch(event.type) {
		case Expose:
#ifdef LEDBOX_DEBUG
			fprintf(stderr, "LedBox::ProcessEvent() Expose\n");
#endif
			if (event.xexpose.count == 0) {
				update_dialog();
			}
			break;
#ifdef NO_TITLEBAR
		case ButtonPress:
			if (event.xbutton.button == 1) {
				pStart.x = event.xbutton.x;
				pStart.y = event.xbutton.y;
			}
#ifdef LEDBOX_DEBUG
			fprintf(stderr, "LedBox::ProcessEvent() ButtonPress x:%d y:%d\n",pStart.x,pStart.y);
#endif
			break;
		case ButtonRelease:
			break;
		case MotionNotify:
			mouse_move(event);
			break;
#else
		case MotionNotify:
			set_dist();
			break;
#endif
		default:
#ifdef LEDBOX_DEBUG
			fprintf(stderr, "LedBox::ProcessEvent() type:%d\n", event.type);
#endif
			break;
		}
	}
}

void LedBox::PostProcessEvent()
{
	if (need_expose) {
		if (window) {
//			XClearWindow(display, window); // expose event
			XClearArea(display, window, 0, 0, 0, 0, True);	// send expose event
		}
		need_expose = false;
	}
}

Bool LedBox::is_map_notify(Display *d, XEvent *e, XPointer w)
{
	return e->type == MapNotify && e->xmap.window == *((Window *)w);
}

Bool LedBox::is_unmap_notify(Display *d, XEvent *e, XPointer w)
{
	return e->type == UnmapNotify && e->xunmap.window == *((Window *)w);
}

void LedBox::Move()
{
	if (window) {
#ifdef LEDBOX_DEBUG
		fprintf(stderr, "LedBox::Move()\n");
#endif

		XWindowAttributes pattr;
		XGetWindowAttributes(display, parent, &pattr);

		int x = pattr.x + dist.x;
		int y = pattr.y + dist.y;

		XMoveWindow(display, window, x, y);
	}
}

void LedBox::move_in_place(int place)
{
	if (window) {
#ifdef LEDBOX_DEBUG
		fprintf(stderr, "LedBox::move_in_place()\n");
#endif

		XWindowAttributes attr, pattr;
		XGetWindowAttributes(display, window, &attr);
		XGetWindowAttributes(display, parent, &pattr);

		int w = attr.width;
		int h = attr.height;

		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = pattr.width - w;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = pattr.height - h;
			} else {
				dist.y = 0;
			}
		}

		int x = pattr.x + dist.x;
		int y = pattr.y + dist.y;

		XMoveWindow(display, window, x, y);
	}
}

#ifdef NO_TITLEBAR
void LedBox::mouse_move(const XEvent &event)
{
	XWindowAttributes attr, pattr;

	XGetWindowAttributes(display, window, &attr);
	XGetWindowAttributes(display, parent, &pattr);

	int x = event.xmotion.x - pStart.x + attr.x;
	int y = event.xmotion.y - pStart.y + attr.y;
	dist.x = attr.x - pattr.x;
	dist.y = attr.y - pattr.y;

#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::mouse_move() wx:%d wy:%d ww:%d wh:%d mx:%d my:%d x:%d y:%d dx:%d dy:%d\n"
		,attr.x,attr.y,attr.width,attr.height
		,event.xmotion.x,event.xmotion.y
		,x,y,dist.x,dist.y);
#endif

	XMoveWindow(display, window, x, y);
}
#endif

#ifndef NO_TITLEBAR
void LedBox::set_dist()
{
	XWindowAttributes attr, pattr;

	XGetWindowAttributes(display, window, &attr);
	XGetWindowAttributes(display, parent, &pattr);

	dist.x = attr.x - pattr.x;
	dist.y = attr.y - pattr.y;
}
#endif

void LedBox::need_update_dialog()
{
	need_expose = true;
}

void LedBox::update_dialog()
{
#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::update_dialog()\n");
#endif

#ifdef USE_XINITIMAGE
	if (image.width <= 0 || image.height <= 0) return;

//	char *src = (char *)surface->pixels;
//	char *dst = image.data;
//	memcpy(dst, src, surface->pitch * surface->h);
	XPutImage(display, window, gc, &image,
		0, 0, 0, 0,
		image.width, image.height
	);
#else
	if (image->width <= 0 || image->height <= 0) return;

	char *src = (char *)surface->pixels;
	char *dst = image->data;
	memcpy(dst, src, surface->pitch * surface->h);
	XPutImage(display, window, gc, image,
		0, 0, 1, 1,
		image->width, image->height
	);
#endif
}

/// get toplevel window id
/// Toplevel window means a window just under the root window.
///
/// @note Some window manager wrap a window in a window to attach a title bar.
Window LedBox::find_toplevel_window(Window start, Window window)
{
	Window root;
	Window parent;
	Window *children = NULL;
	Window match = 0;
	unsigned int num_children = 0;
	unsigned int i;

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

//	fprintf(stderr, "LedBox::find_toplevel_window() match:%lx\n", match);

	return match;
}

#endif /* SDL_VIDEO_DRIVER_X11 && USE_X11_LEDBOX */
#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
