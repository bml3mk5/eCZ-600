/** @file gtk_dialogbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ dialog box ]
*/

#ifndef GUI_GTK_DIALOGBOX_H
#define GUI_GTK_DIALOGBOX_H

#include "../../common.h"
#include <gtk/gtk.h>
#include "../../cchar.h"
#include "../../cptrlist.h"
#include "../../msgs.h"

class GUI;

/// @namespace GUI_GTK_X11
/// @brief Dialogs for GTK+ on X11
namespace GUI_GTK_X11
{

/**
	@brief Layout control widgets on a dialog
*/
class Box
{
protected:
	int cellspace;

public:
	Box();
	virtual ~Box();
	GtkWidget *create_hbox(GtkWidget *parent, int cell_space = -1);
	GtkWidget *create_vbox(GtkWidget *parent, int cell_space = -1);
	void add_widget(GtkWidget *parent, GtkWidget *child, gboolean expand = FALSE, gboolean fill = FALSE);
};

/**
	@brief Control widget
*/
class Control
{
protected:
	int cellspace;

	GtkWidget *create_notebook(GtkWidget *parent);
	void add_note(GtkWidget *parent, GtkWidget *child, const char *label);
	void add_note(GtkWidget *parent, GtkWidget *child, CMsg::Id labelid);
	GtkWidget *create_frame(GtkWidget *parent, const char *label, GtkWidget **vbox = NULL, GtkWidget **hbox = NULL);
	GtkWidget *create_frame(GtkWidget *parent, CMsg::Id labelid, GtkWidget **vbox = NULL, GtkWidget **hbox = NULL);
	GtkWidget *create_check_box(GtkWidget *parent, const char *label, bool chkd, int num = 0, GCallback handler = NULL);
	GtkWidget *create_check_box(GtkWidget *parent, CMsg::Id labelid, bool chkd, int num = 0, GCallback handler = NULL);
	bool get_check_state(GtkWidget *check);
	int get_check_state_num(GtkWidget **checks, int cnt);
	void set_check_state(GtkWidget *check, bool value);
	GtkWidget *create_radio_box(GtkWidget *parent, GtkWidget **gradio, const char *label, int num, int sel_num, GCallback handler = NULL);
	GtkWidget *create_radio_box(GtkWidget *parent, GtkWidget **gradio, CMsg::Id labelid, int num, int sel_num, GCallback handler = NULL);
	void create_radio_box(GtkWidget *parent, const char **list, int cnt, GtkWidget **radio, int sel_num);
	void create_radio_box(GtkWidget *parent, const CMsg::Id *list, int cnt, GtkWidget **radio, int sel_num);
	bool get_radio_state(GtkWidget *radio);
	int get_radio_state_idx(GtkWidget **radios, int cnt);
	void set_radio_state(GtkWidget *radio, bool value);
	GtkWidget *create_combo_box(GtkWidget *parent, const char *label, const char **list, int sel_num);
	GtkWidget *create_combo_box(GtkWidget *parent, CMsg::Id labelid, const char **list, int sel_num);
	GtkWidget *create_combo_box(GtkWidget *parent, const char *label, const CMsg::Id *ids, int sel_num, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	GtkWidget *create_combo_box(GtkWidget *parent, CMsg::Id labelid, const CMsg::Id *ids, int sel_num, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	GtkWidget *create_combo_box(GtkWidget *parent, const char *label, const CPtrList<CTchar> &list, int sel_num);
	GtkWidget *create_combo_box(GtkWidget *parent, CMsg::Id labelid, const CPtrList<CTchar> &list, int sel_num);
	int get_combo_sel_num(GtkWidget *combo);
	void set_combo_sel_num(GtkWidget *combo, int sel_num);
	GtkWidget *create_text(GtkWidget *parent, int width);
	GtkWidget *create_text(GtkWidget *parent, const char *text, int len);
	GtkWidget *create_text_with_label(GtkWidget *parent, const char *label, const char *text, int len);
	GtkWidget *create_text_with_label(GtkWidget *parent, CMsg::Id labelid, const char *text, int len);
	const char *get_text(GtkWidget *entry);
	void set_text(GtkWidget *entry, const char *text);
	GtkWidget *create_text_view(GtkWidget *parent, int width, int height);
	GtkWidget *create_spin(GtkWidget *parent, int range_min, int range_max, int val);
	gdouble get_spin_value(GtkWidget *spin);
	GtkWidget *create_label(GtkWidget *parent, const char *label);
	GtkWidget *create_label(GtkWidget *parent, CMsg::Id labelid);
	GtkWidget *create_button(GtkWidget *parent, const char *label, GCallback handler = NULL);
	GtkWidget *create_button(GtkWidget *parent, CMsg::Id labelid, GCallback handler = NULL);
	GtkWidget *create_volume_box(GtkWidget *parent, int val, int num = 0, GCallback handler = NULL);
	gdouble get_volume_value(GtkWidget *volume);
	GtkWidget *create_scale_box(GtkWidget *parent, int range_min, int range_max, int val, bool vertical = true, int num = 0, GCallback handler = NULL);
	gdouble get_scale_value(GtkWidget *scale);
	GtkWidget *create_hbox(GtkWidget *parent, int cell_space = -1, gboolean expand = FALSE);
	GtkWidget *create_vbox(GtkWidget *parent, int cell_space = -1, gboolean expand = FALSE);
	GtkWidget *create_grid(GtkWidget *parent);
	void attach_to_grid(GtkWidget *grid, GtkWidget *child, int col, int row);
	GtkWidget *create_scroll_win(GtkWidget *parent, int width = -1, int height = -1, gboolean expand = FALSE);
	void add_widget(GtkWidget *parent, GtkWidget *child, gboolean expand = FALSE, gboolean fill = FALSE);
	void set_enable(GtkWidget *widget, bool enable);

public:
	Control();
	virtual ~Control();
};

/**
	@brief Dialog base box
*/
class DialogBox : public Control
{
protected:
	GUI *gui;

	GtkWidget *window;
	GtkWidget *dialog;

	GtkWidget *create_dialog(GtkWidget *parent, const char *title);
	GtkWidget *create_dialog(GtkWidget *parent, CMsg::Id titleid);
	GtkWidget *create_window(GtkWidget *parent, const char *title);
	GtkWidget *create_window(GtkWidget *parent, CMsg::Id titleid);
	GtkWidget *add_accept_button(CMsg::Id labelid);
	GtkWidget *add_reject_button(CMsg::Id labelid);

public:
	DialogBox(GUI *new_gui);
	virtual ~DialogBox();
	virtual bool Show(GtkWidget *parent_window);
	virtual bool Show(GtkWidget *parent_window, int);
	virtual bool Show(GtkWidget *parent_window, int, bool);
	virtual bool ShowModal(GtkWidget *parent_window);
	virtual void Hide();
	virtual bool IsVisible() const;
};

}; /* namespace GUI_GTK_X11 */

#endif /* GUI_GTK_DIALOGBOX_H */
