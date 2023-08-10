/** @file sdl_main.h

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2015.2.18

	@brief [ sdl_main ]
*/

#ifndef SDL_MAIN_H
#define SDL_MAIN_H

#include <SDL.h>

// signal to main loop
extern bool g_need_update_screen;
extern SDL_cond *g_cond_allow_update_screen;

// frame rate
//extern const int fps[6];
//extern const int fskip[6];
//extern int rec_fps_no;

#endif	/* SDL_MAIN_H */
