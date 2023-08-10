/** @file gtk_keybindbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ keybind box ]
*/

#ifndef GUI_GTK_KEYBINDBOX_H
#define GUI_GTK_KEYBINDBOX_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../gui_keybinddata.h"
#include "../../vm/vm.h"
#include "../../cptrlist.h"

namespace GUI_GTK_X11
{

class KeybindControl : public KeybindData
{
public:
	GtkWidget *grid;
	GtkWidget *cells[KBCTRL_MAX_LINES][KBCTRL_MAX_COLS];
	GtkWidget *chkCombi;
public:
	KeybindControl();
	virtual ~KeybindControl();
	void ClearAllControl();
};

/**
	@brief Keybind dialog box
*/
class KeybindBox : public DialogBox
{
private:
	CPtrList<KeybindControl> ctrls;
	guint m_timeout_id;
	GtkWidget *selected_cell;
	uint32_t enable_axes;

	void Update();

	bool SetData();

	int TranslateCode(int code, int scancode);
	bool ClearKeyCellByCode(int tab, int code, int scancode);
	bool SetKeyCell(int tab, int row, int col, int code, int scancode, GtkWidget *widget);
	bool ClearKeyCell(int tab, int row, int col, GtkWidget *widget);
	bool SetJoyCell(int tab, int row, int col, uint32_t code, GtkWidget *widget);
	bool ClearJoyCell(int tab, int row, int col, GtkWidget *widget);
	bool ClearCell(GtkWidget *widget);

	void UpdateJoy();

	void LoadPreset(int tab, int idx);
	void SavePreset(int tab, int idx);
	void ToggleAxis(GtkWidget *widget);

	static gboolean OnKeyDown(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
	static gboolean OnDoubleClick(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
	static gboolean OnFocusIn(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
	static void OnClickLoadDefault(GtkButton *button, gpointer user_data);
	static void OnClickLoadPreset(GtkButton *button, gpointer user_data);
	static void OnClickSavePreset(GtkButton *button, gpointer user_data);
	static void OnClickAxis(GtkToggleButton *button, gpointer user_data);
	static void OnResponse(GtkWidget *widget, gint response_id, gpointer user_data);
	static gboolean OnTimeout(gpointer user_data);

public:
	KeybindBox(GUI *new_gui);
	~KeybindBox();
	bool Show(GtkWidget *parent_window);
	void Hide();
	void SetSelectedCell(GtkWidget *widget) { selected_cell = widget; }
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_KEYBINDBOX_H */
