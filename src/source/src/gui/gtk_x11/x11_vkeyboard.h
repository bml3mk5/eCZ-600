/** @file x11_vkeyboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.21 -

	@brief [ virtual keyboard ]
*/

#ifndef X11_VKEYBOARD_H
#define X11_VKEYBOARD_H

#include "../vkeyboard.h"

#if (defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_VKEYBOARD))

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define NO_TITLEBAR
#define USE_XINITIMAGE 1
//#define VKEYBOARD_DEBUG 1

#define VKEYBOARD_EXPOSE_MAXNUM 10

namespace Vkbd {

/**
	@brief Virtual keyboard
*/
class VKeyboard : public Base
{
private:
	Display	   *display;
	int			screen;
	Window		parent;
	Window		window;
	GC			gc;
	long		event_masks;
	XImage		image;

	Atom		a_wm_protocol;
	Atom		a_delete_window;

	bool		event_working;

	bool		need_expose;
	VmRectWH	expose_rect[VKEYBOARD_EXPOSE_MAXNUM];
	int			expose_num;

	void set_dist();

	void need_update_window(PressedInfo_t *, bool);
	void need_update_window();
	void update_window();
	void paint_window(int, int, int, int);

	Window find_toplevel_window(Window, Window);

public:
	VKeyboard();
	~VKeyboard();

	bool Create(const char *);
	void Show(bool = true);
	void Close();

	void ProcessEvent();
	void PostProcessEvent();

};

} /* namespace Vkbd */

#endif /* SDL_VIDEO_DRIVER_X11 && USE_X11_VKEYBOARD */

#endif /* X11_VKEYBOARD_H */
