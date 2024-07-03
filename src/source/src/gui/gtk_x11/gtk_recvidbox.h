/** @file gtk_recvidbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.30 -

	@brief [ record video box ]
*/

#ifndef GUI_GTK_RECVIDBOX_H
#define GUI_GTK_RECVIDBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../vm/vm.h"

namespace GUI_GTK_X11
{

#define GTK_RECVIDEO_LIBS 5

/**
	@brief Record video dialog box
*/
class RecVideoBox : public DialogBox
{
private:
	int fpsnum;
	int typnum;
	int codnums[GTK_RECVIDEO_LIBS];
	int quanums[GTK_RECVIDEO_LIBS];
	bool enables[GTK_RECVIDEO_LIBS];

	GtkWidget *notebook;
	GtkWidget *comCod[GTK_RECVIDEO_LIBS];
	GtkWidget *comQua[GTK_RECVIDEO_LIBS];
	GtkWidget *btnOk;

	bool conti;

	bool SetData();
	void ChangeTab(int idx);
	void HideB(bool canceled);

	static gboolean OnChangeTab(GtkNotebook *nb, gint idx, gpointer user_data);

	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	RecVideoBox(GUI *new_gui);
	~RecVideoBox();
	bool Show(GtkWidget *parent_window, int num, bool continuous);
	void Hide();

};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_RECVIDBOX_H */
