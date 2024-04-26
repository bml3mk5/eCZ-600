/** @file gtk_hdtypebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.03.20 -

	@brief [ hd device type box ]
*/

#include "gtk_hdtypebox.h"
#include "../gui.h"
#include "../../msgs.h"
#include "../../labels.h"
#include "../../utility.h"
#ifdef USE_GTK
#include <cairo/cairo.h>
#endif

//extern EMU *emu;
//extern GUI *gui;

namespace GUI_GTK_X11
{

HDTypeBox::HDTypeBox(GUI *new_gui, int drv, int type) : DialogBox(new_gui)
{
	m_drive = drv;
	m_device_type = type;
}

HDTypeBox::~HDTypeBox()
{
}

bool HDTypeBox::Show(GtkWidget *parent_window)
{
	DialogBox::Show(parent_window);

	if (dialog) return true;

	create_dialog(window, CMsg::Select_Device_Type);
	add_accept_button(CMsg::OK);
	add_reject_button(CMsg::Cancel);

	GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget *vboxall;
	GtkWidget *hbox;

	vboxall = create_vbox(cont);

	char label[_MAX_PATH];
	if (m_drive < MAX_SASI_HARD_DISKS) {
		UTILITY::sprintf(label, _MAX_PATH, CMSG(Select_a_device_type_on_SASI_disk_VDIGIT), m_drive);
	} else {
		UTILITY::sprintf(label, _MAX_PATH, CMSG(Select_a_device_type_on_SCSI_disk_VDIGIT), m_drive - MAX_SASI_HARD_DISKS);
	}
	create_label(vboxall, label);

	hbox = create_hbox(vboxall);
	create_radio_box(hbox, LABELS::hd_device_type, 4, radio, m_device_type);

	create_label(vboxall, CMsg::Need_restart_program_or_PowerOn);

	//

	gtk_widget_show_all(dialog);

	return true;
}

int HDTypeBox::GetDeviceType()
{
	return get_radio_state_idx(radio, 4);
}

}; /* namespace GUI_GTK_X11 */

