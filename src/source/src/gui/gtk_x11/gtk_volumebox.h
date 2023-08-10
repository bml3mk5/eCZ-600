/** @file gtk_volumebox.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ volume box ]
*/

#ifndef GUI_GTK_VOLUMEBOX_H
#define GUI_GTK_VOLUMEBOX_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../../config.h"

namespace GUI_GTK_X11
{

/**
	@brief Volume dialog box
*/
class VolumeBox : public DialogBox
{
private:
	int  *p_volume[VOLUME_NUMS];
	bool *p_mute[VOLUME_NUMS];

	void SetPtr();
	void SetVolume(int index, int value);
	int  GetVolume(int index);
	void SetMute(int index, bool value);
	bool GetMute(int index);
	static void OnChangeVolume(GtkWidget *widget, gpointer user_data);
	static void OnChangeMute(GtkWidget *widget, gpointer user_data);
	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);

public:
	VolumeBox(GUI *new_gui);
	~VolumeBox();
	bool Show(GtkWidget *parent_window);
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_VOLUMEBOX_H */
