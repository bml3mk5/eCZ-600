/** @file gtk_midlatebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.12 -

	@brief [ midi latency box ]
*/

#ifndef GUI_GTK_MIDLATEBOX_H
#define GUI_GTK_MIDLATEBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "gtk_dialogbox.h"

namespace GUI_GTK_X11
{

/**
	@brief hd device type dialog box
*/
class MidLateBox : public DialogBox
{
private:
	GtkWidget *spnMIDIOutDelay;

public:
	MidLateBox(GUI *new_gui);
	~MidLateBox();
	bool Show(GtkWidget *parent_window);
	void SetData();
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_MIDLATEBOX_H */
