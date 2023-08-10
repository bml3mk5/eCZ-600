/** @file gtk_main.h

	Skelton for retropc emulator
	GTK+ + SDL edition

	@author Sasaji
	@date   2017.1.27

	@brief [ gtk_main ]
*/

#ifndef GTK_MAIN_H
#define GTK_MAIN_H

#include <SDL.h>
//#include <gtk/gtk.h>
//#include <gdk/gdk.h>
//#include <gdk/gdkx.h>

// signal to main loop
extern bool g_need_update_screen;
extern SDL_cond *g_cond_allow_update_screen;

// frame rate
//extern const int fps[6];
//extern const int fskip[6];
//extern int rec_fps_no;

#endif	/* GTK_MAIN_H */
