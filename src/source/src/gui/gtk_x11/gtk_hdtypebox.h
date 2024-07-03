/** @file gtk_hdtypebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.20 -

	@brief [ hd device type box ]
*/

#ifndef GUI_GTK_HDTYPEBOX_H
#define GUI_GTK_HDTYPEBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include <vector>

namespace GUI_GTK_X11
{

/**
	@brief hd device type dialog box
*/
class HDTypeBox : public DialogBox
{
private:
	int m_drive;
	int m_device_type;

	GtkWidget *radio[4];

public:
	HDTypeBox(GUI *new_gui, int drv, int type);
	~HDTypeBox();
	bool Show(GtkWidget *parent_window);
	int GetDeviceType();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_HDTYPEBOX_H */
