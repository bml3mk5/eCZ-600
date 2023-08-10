/** @file x11_ledbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.01.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#ifndef X11_LEDBOX_H
#define X11_LEDBOX_H

#include "../ledbox.h"

#if (defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_LEDBOX))

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define NO_TITLEBAR
#define USE_XINITIMAGE 1
//#define LEDBOX_DEBUG 1

/**
	@brief LedBox
*/
class LedBox : public LedBoxBase
{
private:
	Display	   *display;
	int			screen;
	Window		parent;
	Window		window;
	GC			gc;
	long		event_masks;
#ifdef USE_XINITIMAGE
	XImage		image;
#else
	XImage	   *image;
#endif

	bool		event_working;
	bool		need_expose;

	VmPoint		pStart;

	void destroy_dialog();
	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

#ifdef NO_TITLEBAR
	void mouse_move(const XEvent &event);
#else
	void set_dist();
#endif
	void update_dialog();
	Window find_toplevel_window(Window, Window);

	static Bool is_map_notify(Display *, XEvent *, XPointer);
	static Bool is_unmap_notify(Display *, XEvent *, XPointer);

public:
	LedBox();
	~LedBox();

	void CreateDialogBox();
	void Move();
	void ProcessEvent();
	void PostProcessEvent();
};

#endif /* SDL_VIDEO_DRIVER_X11 && USE_X11_LEDBOX */

#endif /* X11_LEDBOX_H */

#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
