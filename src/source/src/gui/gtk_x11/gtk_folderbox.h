/** @file gtk_folderbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ folder box ]
*/

#ifndef GUI_GTK_FOLDERBOX_H
#define GUI_GTK_FOLDERBOX_H

#include <gtk/gtk.h>

namespace GUI_GTK_X11
{

/**
	@brief Folder dialog box
*/
class FolderBox
{
private:
	GtkWidget *window;
	GtkWidget *dialog;
	char *selected_folder;

public:
	FolderBox(GtkWidget *parent_window);
	~FolderBox();
	bool Show(const char *title, const char *path = NULL);

	const char *GetPath();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_FOLDERBOX_H */
