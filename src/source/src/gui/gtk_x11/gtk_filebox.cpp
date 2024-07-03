/** @file gtk_filebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ file box ]
*/

#include "gtk_filebox.h"
#include <ctype.h>
#include "../../utility.h"

namespace GUI_GTK_X11
{

FileBox::FileBox()
{
	window = NULL;
	dialog = NULL;
	selected_file = NULL;
	cb_handler = NULL;
	cb_data = NULL;
}

FileBox::~FileBox()
{
	if (selected_file) g_free(selected_file);
	if (dialog) gtk_widget_destroy(dialog);
}

/// @brief Show a file dialog
/// @param[in] parent_window
/// @param[in] filter : message string list (NULL terminate)
/// @param[in] title : title string
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @param[in] callback_handler : callback function after submitted button
/// @param[out] callback_data : callback data after submitted button
/// @return true if pressed ok button
bool FileBox::Show(GtkWidget *parent_window, const char *filter[], const char *title, const char *dir, bool save, const char *path, CbFileBox callback_handler, void *callback_data)
{
	if (create_chooser(parent_window,
		title,
		dir, save, path) == NULL) return false;

	parse_filter(GTK_FILE_CHOOSER(dialog), filter);

	return set_handler(callback_handler, callback_data);
}

/// @brief Show a file dialog
/// @param[in] parent_window
/// @param[in] filter : message id list (CMsg::End terminate)
/// @param[in] title : title string
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @param[in] callback_handler : callback function after submitted button
/// @param[out] callback_data : callback data after submitted button
/// @return true if pressed ok button
bool FileBox::Show(GtkWidget *parent_window, const CMsg::Id *filter, const char *title, const char *dir, bool save, const char *path, CbFileBox callback_handler, void *callback_data)
{
	if (create_chooser(parent_window,
		title,
		dir, save, path) == NULL) return false;

	parse_filter(GTK_FILE_CHOOSER(dialog), filter);

	return set_handler(callback_handler, callback_data);
}

/// @brief Show a file dialog
/// @param[in] parent_window
/// @param[in] filter : message id list (CMsg::End terminate)
/// @param[in] titleid : title id
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @param[in] callback_handler : callback function after submitted button
/// @param[out] callback_data : callback data after submitted button
/// @return true if pressed ok button
bool FileBox::Show(GtkWidget *parent_window, const CMsg::Id *filter, CMsg::Id titleid, const char *dir, bool save, const char *path, CbFileBox callback_handler, void *callback_data)
{
	if (create_chooser(parent_window,
		gMessages.Get(titleid),
		dir, save, path) == NULL) return false;

	parse_filter(GTK_FILE_CHOOSER(dialog), filter);

	return set_handler(callback_handler, callback_data);
}

/// @brief Show a file dialog
/// @param[in] parent_window
/// @param[in] filter : string such as "foo;bar;baz"
/// @param[in] title : title string
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @param[in] callback_handler : callback function after submitted button
/// @param[out] callback_data : callback data after submitted button
/// @return true if pressed ok button
bool FileBox::Show(GtkWidget *parent_window, const char *filter, const char *title, const char *dir, bool save, const char *path, CbFileBox callback_handler, void *callback_data)
{
	if (create_chooser(parent_window,
		title,
		dir, save, path) == NULL) return false;

	parse_filter(GTK_FILE_CHOOSER(dialog), filter, save);

	return set_handler(callback_handler, callback_data);
}

/// @brief Show a file dialog
/// @param[in] parent_window
/// @param[in] filter : string such as "foo;bar;baz"
/// @param[in] titleid : title id
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @param[in] callback_handler : callback function after submitted button
/// @param[out] callback_data : callback data after submitted button
/// @return true if pressed ok button
bool FileBox::Show(GtkWidget *parent_window, const char *filter, CMsg::Id titleid, const char *dir, bool save, const char *path, CbFileBox callback_handler, void *callback_data)
{
	if (create_chooser(parent_window,
		gMessages.Get(titleid),
		dir, save, path) == NULL) return false;

	parse_filter(GTK_FILE_CHOOSER(dialog), filter, save);

	return set_handler(callback_handler, callback_data);
}

/// @brief Create a file dialog
/// @param[in] parent
/// @param[in] title : title string
/// @param[in] dir : directory
/// @param[in] save : is a save dialog?
/// @param[in] path : path to open/save a file
/// @return created the file dialog
GtkWidget *FileBox::create_chooser(GtkWidget *parent, const char *title, const char *dir, bool save, const char *path)
{
	if (dialog) return dialog;
	window = parent;
	if (window == NULL) return NULL;
	dialog = gtk_file_chooser_dialog_new(
		title,
		GTK_WINDOW(window),
		save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
		CMSG(Cancel), GTK_RESPONSE_CANCEL,
		save ? CMSG(Save) : CMSG(Open), GTK_RESPONSE_ACCEPT,
		NULL);
	if (dialog == NULL) return NULL;

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
	if (save) {
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
	}
	if(dir != NULL && dir[0]) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);
	}
	if (path != NULL && path[0]) {
//		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), path);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), path);
//	} else {
//		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), app);
	}

	return dialog;
}

/// @brief Parse a filter string list
/// @param[in] chooser
/// @param[in] filter_list : string list terminated NULL
void FileBox::parse_filter(GtkFileChooser *chooser, const char *filter_list[])
{
	if (!filter_list) return;

	char fil[_MAX_PATH];
	char subext[_MAX_PATH];

	GtkFileFilter *filter;

	for(int i=0; filter_list[i] != NULL; i++) {
		memset(fil, 0, sizeof(fil));
		if (UTILITY::substr_in_bracket(filter_list[i], fil) <= 0) continue;
		int fil_pos = 0;
		filter = gtk_file_filter_new();
		do {
			fil_pos = UTILITY::get_token(fil, fil_pos, (int)strlen(fil), subext, _MAX_PATH, _T(';'));
			gtk_file_filter_add_pattern(filter, subext);
			for(char *p=subext; *p != '\0'; p++) *p = toupper(*p);
			gtk_file_filter_add_pattern(filter, subext);
		} while (fil_pos >= 0);

		gtk_file_filter_set_name(filter, filter_list[i]);

		gtk_file_chooser_add_filter(chooser, filter);
	}
}

/// @brief Parse a filter id list
/// @param[in] chooser
/// @param[in] filter_list : id list terminated CMsg::End
void FileBox::parse_filter(GtkFileChooser *chooser, const CMsg::Id *filter_list)
{
	if (!filter_list) return;

	char fil[_MAX_PATH];
	char subext[_MAX_PATH];

	GtkFileFilter *filter;

	for(int i=0; filter_list[i] != 0 && filter_list[i] != CMsg::End; i++) {
		const char *label = gMessages.Get(filter_list[i]);
		memset(fil, 0, sizeof(fil));
		if (UTILITY::substr_in_bracket(label, fil) <= 0) continue;
		int fil_pos = 0;
		filter = gtk_file_filter_new();
		do {
			fil_pos = UTILITY::get_token(fil, fil_pos, (int)strlen(fil), subext, _MAX_PATH, _T(';'));
			gtk_file_filter_add_pattern(filter, subext);
			for(char *p=subext; *p != '\0'; p++) *p = toupper(*p);
			gtk_file_filter_add_pattern(filter, subext);
		} while (fil_pos >= 0);

		gtk_file_filter_set_name(filter, label);

		gtk_file_chooser_add_filter(chooser, filter);
	}
}

/// @brief Parse a filter string
/// @param[in] chooser
/// @param[in] filter_str : string such as "foo;bar;baz"
/// @param[in] save : is a save dialog?
void FileBox::parse_filter(GtkFileChooser *chooser, const char *filter_str, bool save)
{
	if (!filter_str) return;

	char subext[8];
	char label[_MAX_PATH];

	GtkFileFilter *filter = NULL;

	int fil_pos = 0;
	int fil_len = (int)strlen(filter_str);
	int sub_len = 0;
	int ext_nums = 0;
	strcpy(subext, "*.");
	if (!save) {
		// for load dialog
		// "Supported Files (*.foo;*.bar)"
		UTILITY::strcpy(label, sizeof(label), CMSG(Supported_Files));
		UTILITY::strcat(label, sizeof(label), " (");
		do {
			sub_len = 0;
			fil_pos = UTILITY::get_token(filter_str, fil_pos, fil_len, &subext[2], 6, ';', &sub_len);
			if (fil_len > 0) {
				if (!filter) {
					filter = gtk_file_filter_new();
				}
				for(char *p=&subext[2]; *p != '\0'; p++) *p = tolower(*p);
				if (ext_nums > 0) {
					UTILITY::strcat(label, sizeof(label), ";");
				}
				UTILITY::strcat(label, sizeof(label), subext);
				gtk_file_filter_add_pattern(filter, subext);
				for(char *p=&subext[2]; *p != '\0'; p++) *p = toupper(*p);
				UTILITY::strcat(label, sizeof(label), ";");
				UTILITY::strcat(label, sizeof(label), subext);
				gtk_file_filter_add_pattern(filter, subext);
				ext_nums++;
			}
		} while(fil_pos >= 0);
		UTILITY::strcat(label, sizeof(label), ")");
		if (ext_nums > 0) {
			gtk_file_filter_set_name(filter, label);
			gtk_file_chooser_add_filter(chooser, filter);
		}
		// "All Files (*.*)"
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.*");
		UTILITY::strcpy(label, sizeof(label), CMSG(All_Files));
		UTILITY::strcat(label, sizeof(label), " (*.*)");
		gtk_file_filter_set_name(filter, label);
		gtk_file_chooser_add_filter(chooser, filter);

	} else {
		// for save dialog
		// "Foo File (*.foo)"
		// "Bar File (*.bar)"
		do {
			sub_len = 0;
			fil_pos = UTILITY::get_token(filter_str, fil_pos, fil_len, &subext[2], 6, ';', &sub_len);
			if (fil_len > 0) {
				for(int i=0; i<2; i++) {
					filter = gtk_file_filter_new();
					if (i == 0) {
						for(char *p=&subext[2]; *p != '\0'; p++) *p = tolower(*p);
					} else {
						for(char *p=&subext[2]; *p != '\0'; p++) *p = toupper(*p);
					}
					gtk_file_filter_add_pattern(filter, subext);
					UTILITY::strcpy(label, sizeof(label), &subext[2]);
					UTILITY::strcat(label, sizeof(label), " ");
					UTILITY::strcat(label, sizeof(label), CMSG(File));
					UTILITY::strcat(label, sizeof(label), " (");
					UTILITY::strcat(label, sizeof(label), subext);
					UTILITY::strcat(label, sizeof(label), ")");
					gtk_file_filter_set_name(filter, label);
					gtk_file_chooser_add_filter(chooser, filter);
				}
				ext_nums++;
			}
		} while(fil_pos >= 0);
	}
}

bool FileBox::set_handler(CbFileBox callback_handler, void *callback_data)
{
	if (callback_handler) {
		// modeless
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(OnResponse), (gpointer)this);
		cb_handler = callback_handler;
		cb_data = callback_data;
		gtk_widget_show_all(dialog);
		return true;
	} else {
		// modal
		gint rc = gtk_dialog_run(GTK_DIALOG(dialog));
		if (rc == GTK_RESPONSE_ACCEPT) {
			selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		}
		return (rc == GTK_RESPONSE_ACCEPT);
	}
}

void FileBox::Hide()
{
	if (selected_file) {
		g_free(selected_file);
		selected_file = NULL;
	}
	if (dialog) {
		gtk_widget_destroy(dialog);
		dialog = NULL;
		cb_handler = NULL;
		cb_data = NULL;
	}
}

const char *FileBox::GetPath()
{
	return selected_file;
}

void FileBox::OnResponse(GtkWidget *widget, gint response_id, gpointer user_data)
{
//	g_print("OnResponse\n");
	FileBox *obj = (FileBox *)user_data;
	if (response_id == GTK_RESPONSE_ACCEPT) {
		obj->selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	}
	if (obj->cb_handler) {
		obj->cb_handler(obj, response_id == GTK_RESPONSE_ACCEPT, obj->cb_data);
	}
	obj->Hide();
}

}; /* namespace GUI_GTK_X11 */


