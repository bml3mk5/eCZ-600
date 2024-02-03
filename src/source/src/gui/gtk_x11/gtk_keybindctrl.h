/** @file gtk_keybindctrl.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.01.07 -

	@brief [ keybind control ]
*/

#ifndef GUI_GTK_KEYBINDCTRL_H
#define GUI_GTK_KEYBINDCTRL_H

#include <gtk/gtk.h>
#include "gtk_dialogbox.h"
#include "../gui_keybinddata.h"
#include "../../vm/vm.h"
#include "../../cptrlist.h"

namespace GUI_GTK_X11
{

/**
  @brief Keybind data control
 */
class KeybindDataControl : public KeybindData, public Control
{
public:
	GtkWidget *grid;
	GtkWidget *cells[KBCTRL_MAX_LINES][KBCTRL_MAX_COLS];
	GtkWidget *chkCombi;

public:
	KeybindDataControl();
	virtual ~KeybindDataControl();
	void ClearAllControl();
	void Create(Control *parent, GtkWidget* box, int tab_num, int width, int height
			,GCallback on_key_down, GCallback on_double_click, GCallback on_focus_in);

	void Update();
	void SetData();

	static int TranslateCode(int code, int scancode);
	bool ClearKeyCellByCode(int code, int scancode);
	bool SetKeyCell(int row, int col, int code, int scancode, GtkWidget *widget);
	bool ClearKeyCell(int row, int col, GtkWidget *widget);
	bool SetJoyCell(int row, int col, uint32_t code, GtkWidget *widget);
	bool ClearJoyCell(int row, int col, GtkWidget *widget);
	bool ClearCell(int row, int col, GtkWidget *widget);

};

/**
  @brief Keybind control base
 */
class KeybindControlBox : public DialogBox
{
protected:
	CPtrList<KeybindDataControl> ctrls;
	guint m_timeout_id;
	GtkWidget *selected_cell;
	uint32_t enable_axes;

	virtual void ShowAfter(GtkWidget *boxall);

	virtual void Update();

	virtual bool SetData();

//	virtual bool ClearKeyCellByCode(int tab, int code, int scancode);
//	virtual bool SetKeyCell(int tab, int row, int col, int code, int scancode, GtkWidget *widget);
//	virtual bool ClearKeyCell(int tab, int row, int col, GtkWidget *widget);
//	virtual bool SetJoyCell(int tab, int row, int col, uint32_t code, GtkWidget *widget);
//	virtual bool ClearJoyCell(int tab, int row, int col, GtkWidget *widget);
	virtual bool ClearCell(GtkWidget *widget);

	virtual void UpdateJoy();

//	virtual void LoadPreset(int tab, int idx);
//	virtual void SavePreset(int tab, int idx);
	virtual void ToggleAxis(GtkWidget *widget);

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
	KeybindControlBox(GUI *new_gui);
	virtual ~KeybindControlBox();
	virtual bool Show(GtkWidget *parent_window);
	virtual void Hide();
	virtual void SetSelectedCell(GtkWidget *widget) { selected_cell = widget; }
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_KEYBINDCTRL_H */
