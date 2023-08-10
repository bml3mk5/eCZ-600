/** @file gtk_aboutbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ about box ]
*/

#ifndef GUI_GTK_ABOUTBOX_H
#define GUI_GTK_ABOUTBOX_H

#include <gtk/gtk.h>

namespace GUI_GTK_X11
{

/**
	@brief About dialog box
*/
class AboutBox
{
private:
	GtkWidget *dialog;

	GdkPixbuf *SetIcon();

public:
	AboutBox(GtkWidget *parent_window);
	~AboutBox();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_ABOUTBOX_H */
