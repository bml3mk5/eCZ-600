/** @file sdl2_ledbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.12.21 -

	@brief [ led box ]
*/

#if defined(USE_SDL2) && defined(USE_SDL2_LEDBOX)

#include "sdl2_ledbox.h"

//
// for sdl2
//

LedBox::LedBox() : LedBoxBase()
{
	wLedBox = NULL;
	wParent = NULL;
	pOffset.x = 0;
	pOffset.y = 0;
}

LedBox::~LedBox()
{
	if (wLedBox) SDL_DestroyWindow(wLedBox);
}

void LedBox::SetParentWindow(SDL_Window *parent)
{
	wParent = parent;
}

void LedBox::show_dialog()
{
	if (wLedBox) {
		if (visible && !inside) {
			flag_old = -1;	// need update
			SDL_ShowWindow(wLedBox);
		} else {
			SDL_HideWindow(wLedBox);
		}
		SDL_RaiseWindow(wParent);
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

	int width = surface->w;
	int height = surface->h;

	wLedBox = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS);

	create_dialog_box_sub();
}

int LedBox::ProcessEvent(SDL_Event *e)
{
	if (e->window.windowID != SDL_GetWindowID(wLedBox)) return 1;
	switch(e->type) {
	case SDL_MOUSEMOTION:
		if (e->motion.state & SDL_BUTTON_LMASK) {
			mouse_move(e->motion.x, e->motion.y);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (e->button.button == SDL_BUTTON_LEFT) {
			pOffset.x = e->button.x;
			pOffset.y = e->button.y;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (e->button.button == SDL_BUTTON_LEFT) {
			mouse_move(e->button.x, e->button.y);
		}
		break;
	}
	return 0;
}

Uint32 LedBox::GetWindowID()
{
	return SDL_GetWindowID(wLedBox);
}

void LedBox::mouse_move(int ox, int oy)
{
	if (wLedBox) {
		int x,y,px,py;
		SDL_GetWindowPosition(wLedBox, &x, &y);
		SDL_GetWindowPosition(wParent, &px, &py);
		x = x + ox - pOffset.x;
		y = y + oy - pOffset.y;
		dist.x = x - px;
		dist.y = y - py;
		SDL_SetWindowPosition(wLedBox, x, y);
	}
}

void LedBox::Move()
{
	if (wLedBox) {
		int px,py;
		SDL_GetWindowPosition(wParent, &px, &py);
		int x = px + dist.x;
		int y = py + dist.y;
		SDL_SetWindowPosition(wLedBox, x, y);
	}
}

void LedBox::move_in_place(int place)
{
	if (wLedBox) {
		int w,h;
		int pw,ph;
		SDL_GetWindowSize(wLedBox, &w, &h);
		SDL_GetWindowSize(wParent, &pw, &ph);

		if (win_pt.place != place) {
			if (place & 1) {
				dist.x = pw - w;
			} else {
				dist.x = 0;
			}
			if (place & 2) {
				dist.y = ph - h;
			} else {
				dist.y = 0;
			}
		}
		int x,y;
		SDL_GetWindowPosition(wParent, &x, &y);
		x = x + dist.x;
		y = y + dist.y;
		SDL_SetWindowPosition(wLedBox, x, y);
	}
}

void LedBox::SetDist()
{
	if (wLedBox) {
		int x,y;
		int px,py;
		SDL_GetWindowPosition(wLedBox, &x, &y);
		SDL_GetWindowPosition(wParent, &px, &py);
		dist.x = x - px;
		dist.y = y - py;
	}
}

void LedBox::need_update_dialog()
{
	if (wLedBox) {
		SDL_Surface *sLedBox = SDL_GetWindowSurface(wLedBox);
		SDL_BlitSurface(surface, NULL, sLedBox, NULL);
		SDL_UpdateWindowSurface(wLedBox);
	}
}

#include <SDL_syswm.h>

void LedBox::create_dialog_box_sub()
{
#if defined(_WIN32)
	if (wLedBox) {
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		SDL_GetWindowWMInfo(wLedBox, &info);

		HWND hLedBox = info.info.win.window;
		SDL_GetWindowWMInfo(wParent, &info);
		HWND hParent = info.info.win.window;

		// attach parent window
		::SetWindowLongPtr(hLedBox, GWLP_HWNDPARENT, (LONG_PTR)hParent);
#if 0
		// set window style
		LONG dwStyle = (LONG)::GetWindowLongPtr(hLedBox, GWL_STYLE);
		dwStyle &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
		::SetWindowLongPtr(hLedBox, GWL_STYLE, dwStyle);
		LONG dwExStyle = (LONG)::GetWindowLongPtr(hLedBox, GWL_EXSTYLE);
		dwExStyle |= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
		::SetWindowLongPtr(hLedBox, GWL_EXSTYLE, dwExStyle);
#endif
	}
#endif
}

#endif /* USE_SDL2 && USE_SDL2_LEDBOX */
