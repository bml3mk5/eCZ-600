/** @file gtk_filebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ file box ]
*/

#ifndef GUI_GTK_FILEBOX_H
#define GUI_GTK_FILEBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "../../msgs.h"

namespace GUI_GTK_X11
{
class FileBox;

typedef void (*CbFileBox)(FileBox *, bool, void *);

/**
	@brief File dialog box
*/
class FileBox
{
private:
	GtkWidget *window;
	GtkWidget *dialog;
	char *selected_file;
	CbFileBox cb_handler;
	void *cb_data;

	void parse_filter(GtkFileChooser *chooser, const char *filter_list[]);
	void parse_filter(GtkFileChooser *chooser, const CMsg::Id *filter_list);
	void parse_filter(GtkFileChooser *chooser, const char *filter_str, bool save);

	GtkWidget *create_chooser(GtkWidget *parent, const char *title, const char *dir, bool save, const char *path);
	bool set_handler(CbFileBox callback_handler, void *callback_data);

	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	FileBox();
	~FileBox();
	bool Show(GtkWidget *parent_window, const char *filter[], const char *title, const char *dir, bool save, const char *path = NULL, CbFileBox callback_handler = NULL, void *callback_data = NULL);
	bool Show(GtkWidget *parent_window, const CMsg::Id *filter, const char *title, const char *dir, bool save, const char *path = NULL, CbFileBox callback_handler = NULL, void *callback_data = NULL);
	bool Show(GtkWidget *parent_window, const CMsg::Id *filter, CMsg::Id titleid, const char *dir, bool save, const char *path = NULL, CbFileBox callback_handler = NULL, void *callback_data = NULL);
	bool Show(GtkWidget *parent_window, const char *filter, const char *title, const char *dir, bool save, const char *path = NULL, CbFileBox callback_handler = NULL, void *callback_data = NULL);
	bool Show(GtkWidget *parent_window, const char *filter, CMsg::Id titleid, const char *dir, bool save, const char *path = NULL, CbFileBox callback_handler = NULL, void *callback_data = NULL);
	void Hide();

	const char *GetPath();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_FILEBOX_H */
