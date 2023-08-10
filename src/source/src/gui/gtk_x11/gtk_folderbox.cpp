/** @file gtk_folderbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ folder box ]
*/

#include "gtk_folderbox.h"
#include "../../common.h"
#include "../../msgs.h"
#include "../../utility.h"

namespace GUI_GTK_X11
{

FolderBox::FolderBox(GtkWidget *parent_window)
{
	window = parent_window;
	dialog = NULL;
	selected_folder = NULL;
}

FolderBox::~FolderBox()
{
	if (selected_folder) g_free(selected_folder);
	if (dialog) gtk_widget_destroy(dialog);
}

bool FolderBox::Show(const char *title, const char *path)
{
	if (window == NULL) return false;
	dialog = gtk_file_chooser_dialog_new(
		title,
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		CMSG(Cancel), GTK_RESPONSE_CANCEL,
		CMSG(Open), GTK_RESPONSE_ACCEPT,
		NULL);
	if (dialog == NULL) return false;

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
	if (path != NULL && path[0]) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
	}

	gint rc = gtk_dialog_run(GTK_DIALOG(dialog));
	if (rc == GTK_RESPONSE_ACCEPT) {
		selected_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
	}
	return (rc == GTK_RESPONSE_ACCEPT);
}

const char *FolderBox::GetPath()
{
	return selected_folder;
}

}; /* namespace GUI_GTK_X11 */


