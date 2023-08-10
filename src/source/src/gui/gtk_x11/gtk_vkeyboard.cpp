/** @file gtk_vkeyboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.21 -

	@brief [ virtual keyboard ]
*/

#include "gtk_vkeyboard.h"

#if defined(USE_GTK_VKEYBOARD)

#include "../gui.h"

extern GUI *gui;

namespace Vkbd {

//
// for GTK+
//

VKeyboard::VKeyboard() : OSDBase()
{
	parent = NULL;
	window = NULL;
	drawing = NULL;
	surface = NULL;
}

VKeyboard::~VKeyboard()
{
}

void VKeyboard::Show(bool show)
{
	if (window) {
		Base::Show(show);
		if (show) {
			// show
			gtk_widget_show_all(window);
//			need_update_window();
		} else {
			// hide
			gtk_widget_hide(window);
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

	parent = gui->GetWindow();

//	int flags = GTK_DIALOG_DESTROY_WITH_PARENT;
//	window = gtk_dialog_new_with_buttons("Virtual Keyboard"
//		, GTK_WINDOW(parent)
//		, (GtkDialogFlags)flags
//		, NULL
//	);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if (!window) {
		closed = true;
		return false;
	}

	gtk_window_set_title(GTK_WINDOW(window), "Virtual Keyboard");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);

//	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(window));
//	gtk_box_set_spacing(GTK_BOX(cont), 0);
//	gtk_box_pack_start(GTK_BOX(cont), drawing, TRUE, TRUE, 0);
	drawing = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing, pSurface->Width(), pSurface->Height());
	gtk_container_add(GTK_CONTAINER(window), drawing);

	gtk_widget_set_app_paintable(window, TRUE);
//	gtk_widget_set_size_request(window, pSurface->Width(), pSurface->Height());
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

#if !GTK_CHECK_VERSION(3,0,0)
	gint mask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK);
	gtk_widget_add_events(window, mask);
#endif

	surface = cairo_image_surface_create_for_data((unsigned char *)pSurface->GetBuffer()
		, CAIRO_FORMAT_RGB24
		, pSurface->Width(), pSurface->Height()
		, cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, pSurface->Width())
	);

//	g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(OnMouseDown), (gpointer)this);
//	g_signal_connect(G_OBJECT(window), "button-release-event", G_CALLBACK(OnMouseUp), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "button-press-event", G_CALLBACK(OnMouseDown), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "button-release-event", G_CALLBACK(OnMouseUp), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(OnKeyDown), (gpointer)this);
	g_signal_connect(G_OBJECT(window), "key-release-event", G_CALLBACK(OnKeyUp), (gpointer)this);
#if GTK_CHECK_VERSION(3,0,0)
//	g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(OnDraw), (gpointer)this);
	g_signal_connect(G_OBJECT(drawing), "draw", G_CALLBACK(OnDraw), (gpointer)this);
#else
	g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(OnExpose), (gpointer)this);
#endif
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(OnDelete), (gpointer)this);
//	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(OnResponse), (gpointer)this);

	gtk_widget_set_events(drawing,
			gtk_widget_get_events(drawing) | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

//	adjust_window_size();
	set_dist();
	closed = false;

	return true;
}

void VKeyboard::Close()
{
#ifdef VKEYBOARD_DEBUG
	fprintf(stderr, "VKeyboard::Close()\n");
#endif

	if (surface) {
		cairo_surface_destroy(surface);
		surface = NULL;
	}

	if (window) {
		gtk_widget_destroy(window);
		window = NULL;
	}

	unload_bitmap();

	CloseBase();
}

void VKeyboard::set_dist()
{
	if (!window) return;

	gint xp = 0;
	gint yp = 0;
	gint wp = 1;
	gint hp = 1;
	gint w = 1;
	gint h = 1;
	gtk_window_get_position(GTK_WINDOW(parent), &xp, &yp);
	gtk_window_get_size(GTK_WINDOW(parent), &wp, &hp);
	gtk_window_get_default_size(GTK_WINDOW(window), &w, &h);
	if (w < 0 || h < 0) {
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
	}

	int x = (wp - w) / 2 + xp;
	int y = yp;

	gtk_window_move(GTK_WINDOW(window), x, y);
}

void VKeyboard::need_update_window(PressedInfo_t *info, bool onoff)
{
	if (!window) return;

	need_update_window_base(info, onoff);

//	gtk_widget_queue_draw_area(window
	gtk_widget_queue_draw_area(drawing
		, info->re.left
		, info->re.top
		, info->re.right - info->re.left
		, info->re.bottom - info->re.top
	);
}

//void VKeyboard::need_update_window()
//{
//	if (!window) return;
//
//	gtk_widget_queue_draw(window);
//}

gboolean VKeyboard::OnMouseDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *e = (GdkEventButton *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	if (e->button == 1) {
		obj->MouseDown(e->x, e->y);
	}
	return TRUE;
}

gboolean VKeyboard::OnMouseUp(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *e = (GdkEventButton *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	if (e->button == 1) {
		obj->MouseUp();
	}
	return TRUE;
}

gboolean VKeyboard::OnKeyDown(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey *key_event = (GdkEventKey *)event;
	emu->key_down_up(0, (int)key_event->keyval
		, (short)key_event->hardware_keycode);
	return FALSE;
}

gboolean VKeyboard::OnKeyUp(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventKey *key_event = (GdkEventKey *)event;
	emu->key_down_up(1, (int)key_event->keyval
		, (short)key_event->hardware_keycode);
	return FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
gboolean VKeyboard::OnDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	VKeyboard *obj = (VKeyboard *)user_data;
//	GdkRectangle re;
//	gdk_cairo_get_clip_rectangle(cr, &re);
//printf("OnDraw: re:%d:%d:%d:%d\n",re.x,re.y,re.width,re.height);
//	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_source_surface(cr, obj->surface, 0.0, 0.0);
	cairo_paint(cr);
	return FALSE;
}
#else
gboolean VKeyboard::OnExpose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventExpose *e = (GdkEventExpose *)event;
	VKeyboard *obj = (VKeyboard *)user_data;
	cairo_t *cr = gdk_cairo_create(widget->window);
	cairo_set_source_surface(cr, obj->surface, 0.0, 0.0);
	cairo_paint(cr);
	cairo_destroy(cr);
	return FALSE;
}
#endif

//void VKeyboard::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
//{
//	g_print("OnResponse\n");
//	VKeyboard *obj = (VKeyboard *)user_data;
//	obj->Close();
//}

gboolean VKeyboard::OnDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	VKeyboard *obj = (VKeyboard *)user_data;
	obj->Close();
	return FALSE;
}

} /* namespace Vkbd */

#endif /* USE_GTK_VKEYBOARD */

