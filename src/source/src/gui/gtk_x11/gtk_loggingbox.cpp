/** @file gtk_loggingbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.14 -

	@brief [ log box ]
*/

#include "gtk_loggingbox.h"
#include "../../emu.h"
#include "../../logging.h"
#include "../../msgs.h"

namespace GUI_GTK_X11
{

LoggingBox::LoggingBox(GUI *new_gui) : DialogBox(new_gui)
{
	txtPath = NULL;
	winLog = NULL;
	txtLog = NULL;
	btnUpdate = NULL;
	btnClose = NULL;

	m_client_size_w = 0;
	m_client_size_h = 0;

	p_buffer = NULL;
	m_buffer_size = 0;
}

LoggingBox::~LoggingBox()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
}

bool LoggingBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_window(window, CMsg::Log);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vbox;
	GtkWidget *hbox;

	//

	vbox = create_vbox(cont, -1, TRUE);
	txtPath = create_text(vbox, 480);
//	gtk_entry_set_editable(GTK_ENTRY(txtPath), FALSE);
	g_object_set(G_OBJECT(txtPath), "editable", 0, NULL);
	winLog = create_scroll_win(vbox, 480, 320, TRUE);
	txtLog = create_text_view(NULL, 480, 320);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(txtLog), FALSE);
	gtk_container_add(GTK_CONTAINER(winLog), txtLog);
//	gtk_scrollable_set_vscroll_policy(GTK_SCROLLABLE(txtLog), GTK_SCROLL_NATURAL);
//	gtk_scrollable_set_hscroll_policy(GTK_SCROLLABLE(txtLog), GTK_SCROLL_NATURAL);

	hbox = create_hbox(vbox);
	btnUpdate = create_button(hbox, CMsg::Update, G_CALLBACK(OnClickUpdate));
	btnClose  = create_button(hbox, CMsg::Close, G_CALLBACK(OnClickClose));

	//

	gtk_widget_show_all(dialog);

	//

	gtk_window_get_size(GTK_WINDOW(dialog), &m_client_size_w, &m_client_size_h);

	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
//	g_signal_connect(G_OBJECT(dialog), "size-allocate", G_CALLBACK(OnResize), (gpointer)this);

	//
	gtk_entry_set_text(GTK_ENTRY(txtPath), logging->get_log_path());

	return true;
}

void LoggingBox::Hide()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;

	DialogBox::Hide();
}

#if 0
void LoggingBox::ResizeWidgets(GtkWidget *widget, GtkAllocation *allocation)
{
	if (widget != dialog) return;

	gint w, h;
//	gtk_window_get_size(GTK_WINDOW(dialog), &w, &h);
	w = allocation->width;
	h = allocation->height;
	int s_w = w - m_client_size_w;
	int s_h = h - m_client_size_h;
	m_client_size_w = w;
	m_client_size_h = h;

	GtkAllocation re;
	gtk_widget_get_allocated_size(winLog, &re, NULL);
	re.width += s_w;
	re.height += s_h;
	gtk_widget_size_allocate(winLog, &re);
}
#endif

void LoggingBox::OnClickUpdate(GtkWidget *widget, gpointer user_data)
{
	LoggingBox *obj = (LoggingBox *)user_data;
	obj->SetData();
}

void LoggingBox::OnClickClose(GtkWidget *widget, gpointer user_data)
{
	LoggingBox *obj = (LoggingBox *)user_data;
	obj->Hide();
}

void LoggingBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
	LoggingBox *obj = (LoggingBox *)user_data;
	obj->Hide();
}

#if 0
void LoggingBox::OnResize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	LoggingBox *obj = (LoggingBox *)user_data;
	obj->ResizeWidgets(widget, allocation);
}
#endif

gboolean LoggingBox::OnScrollToEnd(gpointer user_data)
{
	GtkWidget *tv = (GtkWidget *)user_data;

	GtkTextBuffer *tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
	if (!tbuffer) return G_SOURCE_REMOVE;

	GtkTextIter iend;
	gtk_text_buffer_get_end_iter(tbuffer, &iend);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tv), &iend, 0.0, TRUE, 0.0, 0.0);

	return G_SOURCE_REMOVE;
}

void LoggingBox::SetData()
{
	if (!p_buffer) {
		m_buffer_size = 1024 * 1024;
		p_buffer = new char[m_buffer_size];
	}
	p_buffer[0] = 0;
	logging->get_log(p_buffer, m_buffer_size);

	GtkTextBuffer *tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtLog));
	if (!tbuffer) {
		tbuffer = gtk_text_buffer_new(NULL);
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(txtLog), tbuffer);
	}
	gtk_text_buffer_set_text(tbuffer, p_buffer, -1);

	g_idle_add(&OnScrollToEnd, txtLog);
}

}; /* namespace GUI_GTK_X11 */
