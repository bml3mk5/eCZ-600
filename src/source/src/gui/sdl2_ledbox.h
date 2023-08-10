/** @file sdl2_ledbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#if defined(USE_SDL2) && defined(USE_SDL2_LEDBOX)

#ifndef SDL2_LEDBOX_H
#define SDL2_LEDBOX_H

#include "ledbox.h"

/**
	@brief LedBox is the window that display the access indicator outside the main window.
*/
class LedBox : public LedBoxBase
{
private:
	SDL_Window *wLedBox;
	SDL_Window *wParent;

	VmPoint pOffset;

	void create_dialog_box_sub();
	void show_dialog();
	void mouse_move(int ox, int oy);
	void move_in_place(int place);
	void need_update_dialog();
public:
	LedBox();
	~LedBox();

	void CreateDialogBox();
	void Move();
	void SetParentWindow(SDL_Window *parent);
	int  ProcessEvent(SDL_Event *e);
	Uint32 GetWindowID();
	void SetDist();
};

#endif /* SDL2_LEDBOX_H */

#endif /* USE_SDL2 && USE_SDL2_LEDBOX */
