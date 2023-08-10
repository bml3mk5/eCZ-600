/** @file gtk_aboutbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21 -

	@brief [ about box ]
*/

#include "gtk_aboutbox.h"
#include "../gui.h"
#include "../../version.h"
#ifdef USE_GTK
#include <cairo/cairo.h>
#endif

extern EMU *emu;
extern GUI *gui;

namespace GUI_GTK_X11
{

AboutBox::AboutBox(GtkWidget *parent_window)
{
	char prog_name[128];
	char prog_ver[128];
	char lib_ver[256];

	sprintf(prog_name,"%s",APP_NAME);
	// version
	sprintf(prog_ver,"\nVersion: %s \"%s\"",APP_VERSION,PLATFORM);
#ifdef _DEBUG
	strcat(prog_ver, " (DEBUG Version)");
#endif
	// edition
	emu->get_edition_string(lib_ver, sizeof(lib_ver));
	// library
	strcat(lib_ver, "\n using ");
	for(int i=0; i<3; i++) {
		if (i != 0) {
			strcat(lib_ver, "\n and ");
		}
		size_t vlen = strlen(lib_ver);
		switch(i) {
#ifdef USE_GTK
		case 0:
#else
		case 1:
#endif
			sprintf(&lib_ver[vlen],"GTK+ Version %d.%d.%d",
				gtk_major_version,gtk_minor_version,gtk_micro_version);
			break;
#ifdef USE_GTK
		case 1:
			sprintf(&lib_ver[vlen],"cairo Version %d.%d.%d",
				CAIRO_VERSION_MAJOR,CAIRO_VERSION_MINOR,CAIRO_VERSION_MICRO);
			break;
#else
		case 2:
			lib_ver[vlen-5] = '\0';
			break;
#endif
		default:
			gui->GetLibVersionString(&lib_ver[vlen], sizeof(lib_ver) - vlen);
			break;
		}
	}

	GdkPixbuf *icon = SetIcon();
	gtk_show_about_dialog(GTK_WINDOW(parent_window),
		"program-name", prog_name,
		"version", prog_ver,
		"copyright", APP_COPYRIGHT,
		"comments", lib_ver,
		"logo", icon,
		NULL);
	if (icon) {
		g_object_unref(icon);
	}
}

AboutBox::~AboutBox()
{
}

GdkPixbuf *AboutBox::SetIcon()
{
	char buf[_MAX_PATH];

	sprintf(buf, "%s%s.png", emu->resource_path(), CONFIG_NAME);
	GError *error = NULL;
	GdkPixbuf *icon = gdk_pixbuf_new_from_file(buf, &error);
	if (error) {
		g_error_free(error);
	}
	return icon;
}

}; /* namespace GUI_GTK_X11 */

