/** @file gtk_recaudbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.30 -

	@brief [ record audio box ]
*/

#ifndef GUI_GTK_RECAUDBOX_H
#define GUI_GTK_RECAUDBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../vm/vm.h"

namespace GUI_GTK_X11
{

#define GTK_RECAUDEO_LIBS 5

/**
	@brief Record audio dialog box
*/
class RecAudioBox : public DialogBox
{
private:
	int typnum;
	int codnums[GTK_RECAUDEO_LIBS];
//	int quanums[GTK_RECAUDEO_LIBS];
	bool enables[GTK_RECAUDEO_LIBS];
	int fpsnum;

	GtkWidget *notebook;
	GtkWidget *comCod[GTK_RECAUDEO_LIBS];
//	GtkWidget *comQua[GTK_RECAUDEO_LIBS];
	GtkWidget *btnOk;

	bool SetData();
	void ChangeTab(int idx);

	static gboolean OnChangeTab(GtkNotebook *nb, gint idx, gpointer user_data);

	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	RecAudioBox(GUI *new_gui);
	~RecAudioBox();
	bool Show(GtkWidget *parent_window, int vid_fps_num);
	void Hide();

};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_RECAUDBOX_H */
