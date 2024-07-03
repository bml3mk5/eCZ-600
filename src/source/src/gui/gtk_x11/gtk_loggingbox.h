/** @file gtk_loggingbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.14 -

	@brief [ log box ]
*/

#ifndef GUI_GTK_LOGGINGBOX_H
#define GUI_GTK_LOGGINGBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"

namespace GUI_GTK_X11
{

/**
	@brief Log dialog box
*/
class LoggingBox : public DialogBox
{
private:
	GtkWidget *txtPath;
	GtkWidget *winLog;
	GtkWidget *txtLog;
	GtkWidget *btnUpdate;
	GtkWidget *btnClose;

	gint m_client_size_w;
	gint m_client_size_h;

	char *p_buffer;
	int m_buffer_size;

	void SetData();
//	void ResizeWidgets(GtkWidget *widget, GtkAllocation *allocation);

	static void OnClickUpdate(GtkWidget *widget, gpointer user_data);
	static void OnClickClose(GtkWidget *widget, gpointer user_data);
	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);
	static gboolean OnScrollToEnd(gpointer user_data);
//	static void OnResize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);

public:
	LoggingBox(GUI *new_gui);
	~LoggingBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_LOGGINGBOX_H */
