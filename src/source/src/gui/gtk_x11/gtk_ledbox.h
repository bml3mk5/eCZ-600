/** @file x11_ledbox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.01.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#ifndef GTK_LEDBOX_H
#define GTK_LEDBOX_H

#include "../ledbox.h"

#if defined(USE_GTK_LEDBOX)

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
//#include <cairo/cairo.h>

#define NO_TITLEBAR
//#define LEDBOX_DEBUG 1

/**
	@brief LedBox is the window that display the access indicator outside the main window.
*/
class LedBox : public LedBoxBase
{
private:
	GtkWidget  *parent;
	GtkWidget  *window;
//	cairo_surface_t *cairosuf;

	VmPoint		pStart;

//	void create_dialog();
	void destroy_dialog();
	void show_dialog();
	void move_in_place(int place);
	void need_update_dialog();

#ifdef NO_TITLEBAR
	void mouse_move(GdkEventMotion *event);
#else
	void set_dist();
#endif


	static gboolean OnMouseDown(GtkWidget *, GdkEvent *, gpointer);
	static gboolean OnMouseUp(GtkWidget *, GdkEvent *, gpointer);
	static gboolean OnMouseMove(GtkWidget *, GdkEvent *, gpointer);
#if GTK_CHECK_VERSION(3,0,0)
	static gboolean OnDraw(GtkWidget *, cairo_t *, gpointer);
#else
	static gboolean OnExpose(GtkWidget *, GdkEvent *, gpointer);
#endif
	static gboolean OnDelete(GtkWidget *, GdkEvent *, gpointer);

public:
	LedBox();
	~LedBox();

	void CreateDialogBox();
	void Move();
};

#endif /* USE_GTK_LEDBOX */

#endif /* GTK_LEDBOX_H */

#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
