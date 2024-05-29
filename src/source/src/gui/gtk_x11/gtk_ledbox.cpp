/** @file x11_ledbox.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2016.01.21 -

	@brief [ led box ]
*/

#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))

#include "gtk_ledbox.h"

#if defined(USE_GTK_LEDBOX)

#include "../gui.h"

extern GUI *gui;

//
// for GTK+
//

LedBox::LedBox() : LedBoxBase()
{
	parent = NULL;
	window = NULL;

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
		if (visible && !inside) {
			// show
			gtk_widget_show_all(window);
//			need_update_window();
		} else {
			// hide
			gtk_widget_hide(window);
		}
	}
}

void LedBox::CreateDialogBox()
{
	if (!enable) return;

	parent = gui->GetWindow();

	window = gtk_window_new(GTK_WINDOW_POPUP);
	if (!window) return;

//	gtk_window_set_title(GTK_WINDOW(window), "LedBox");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);

	gtk_widget_set_app_paintable(window, TRUE);
	gtk_widget_set_size_request(window, Width() + 2, Height() + 2);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

#if !GTK_CHECK_VERSION(3,0,0)
	gint mask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK | GDK_EXPOSURE_MASK);
	gtk_widget_add_events(window, mask);
#endif

	g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(OnMouseDown), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "button-release-event", G_CALLBACK(OnMouseUp), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "motion-notify-event", G_CALLBACK(OnMouseMove), (gpointer)this);
#if GTK_CHECK_VERSION(3,0,0)
	g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(OnDraw), (gpointer)this);
#else
	g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(OnExpose), (gpointer)this);
#endif
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(OnDelete), (gpointer)this);
}

void LedBox::destroy_dialog()
{
#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::destroy_dialog()\n");
#endif

	if (window) {
//		gtk_widget_destroy(window);
		window = NULL;
	}
}

void LedBox::need_update_dialog()
{
	if (!window) return;

	gtk_widget_queue_draw(window);
}

void LedBox::Move()
{
	if (!window) return;

#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::Move()\n");
#endif

	gint px = 0;
	gint py = 0;
	gtk_window_get_position(GTK_WINDOW(parent), &px, &py);

	int x = px + dist.x;
	int y = py + dist.y;

	gtk_window_move(GTK_WINDOW(window), x, y);
}

void LedBox::move_in_place(int place)
{
	if (!window) return;

#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::move_in_place()\n");
#endif

	gint px = 0;
	gint py = 0;
	gint pw = 1;
	gint ph = 1;
	gint wx = 0;
	gint wy = 0;
	gint ww = 1;
	gint wh = 1;
	gtk_window_get_position(GTK_WINDOW(parent), &px, &py);
	gtk_window_get_size(GTK_WINDOW(parent), &pw, &ph);
	gtk_window_get_position(GTK_WINDOW(window), &wx, &wy);
	gtk_window_get_size(GTK_WINDOW(window), &ww, &wh);

	int w = ww;
	int h = wh;

	if (win_pt.place != place) {
		if (place & 1) {
			dist.x = pw - w;
		} else {
			dist.x = 0;
		}
		if (place & 2) {
			dist.y = ph - (place & 0x10 ? h : 4);
		} else {
			dist.y = 0 - (place & 0x10 ? 0 : h - 4);
		}
	}

	int x = px + dist.x;
	int y = py + dist.y;

	gtk_window_move(GTK_WINDOW(window), x, y);
}

#ifdef NO_TITLEBAR
void LedBox::mouse_move(GdkEventMotion *event)
{
	if (!window) return;

	gint px = 0;
	gint py = 0;
	gint wx = 0;
	gint wy = 0;
	gtk_window_get_position(GTK_WINDOW(parent), &px, &py);
	gtk_window_get_position(GTK_WINDOW(window), &wx, &wy);

	int x = event->x - pStart.x + wx;
	int y = event->y - pStart.y + wy;
	dist.x = wx - px;
	dist.y = wy - py;

#ifdef LEDBOX_DEBUG
	fprintf(stderr, "LedBox::mouse_move() wx:%d wy:%d mx:%d my:%d x:%d y:%d dx:%d dy:%d\n"
		,wx,wy,
		,event->x,event->y
		,x,y,dist.x,dist.y);
#endif

	gtk_window_move(GTK_WINDOW(window), x, y);
}
#endif

#ifndef NO_TITLEBAR
void LedBox::set_dist()
{	
	if (!window) return;

	gint px = 0;
	gint py = 0;
	gint x = 0;
	gint y = 0;
	gtk_window_get_position(GTK_WINDOW(parent), &px, &py);
	gtk_window_get_position(GTK_WINDOW(window), &x, &y);

	dist.x = x - px;
	dist.y = y - py;
}
#endif

gboolean LedBox::OnMouseDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
#ifdef NO_TITLEBAR
	GdkEventButton *e = (GdkEventButton *)event;
	LedBox *obj = (LedBox *)user_data;
	if (e->button == 1) {
		obj->pStart.x = e->x;
		obj->pStart.y = e->y;
	}
#endif
	return FALSE;
}

gboolean LedBox::OnMouseUp(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return FALSE;
}

gboolean LedBox::OnMouseMove(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventMotion *e = (GdkEventMotion *)event;
	LedBox *obj = (LedBox *)user_data;
#ifdef NO_TITLEBAR
	obj->mouse_move(e);
#else
	obj->set_dist();
#endif
	return FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
gboolean LedBox::OnDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	LedBox *obj = (LedBox *)user_data;
//	GdkRectangle re;
//	gdk_cairo_get_clip_rectangle(cr, &re);
//printf("OnDraw: re:%d:%d:%d:%d\n",re.x,re.y,re.width,re.height);
//	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	obj->cairosuf->BlitC(cr);
//	cairo_set_source_surface(cr, obj->cairosuf, 1.0, 1.0);
	cairo_paint(cr);
	return FALSE;
}
#else
gboolean LedBox::OnExpose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	LedBox *obj = (LedBox *)user_data;
	cairo_t *cr = gdk_cairo_create(widget->window);
//	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	obj->cairosuf->BlitC(cr);
//	cairo_set_source_surface(cr, obj->cairosuf, 1.0, 1.0);
	cairo_paint(cr);
	cairo_destroy(cr);
	return FALSE;
}
#endif

gboolean LedBox::OnDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	LedBox *obj = (LedBox *)user_data;
	obj->destroy_dialog();
	return FALSE;
}

#endif /* USE_GTK_LEDBOX */
#endif /* !(USE_SDL2 && USE_SDL2_LEDBOX) */
