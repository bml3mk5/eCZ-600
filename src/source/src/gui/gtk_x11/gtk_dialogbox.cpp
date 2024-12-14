/** @file gtk_dialogbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.10 -

	@brief [ dialog box ]
*/

#include "gtk_dialogbox.h"
#include "gtk_x11_gui.h"
#include "../../utility.h"

namespace GUI_GTK_X11
{

//
//
//

Box::Box()
{
	cellspace = 4;
}

Box::~Box()
{
}

GtkWidget *Box::create_hbox(GtkWidget *parent, int cell_space)
{
//	g_print("create_hbox\n");
	int cs = (cell_space >= 0 ? cell_space : cellspace);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,cs);
#else
	GtkWidget *box = gtk_hbox_new(FALSE, cs);
#endif
	add_widget(parent, box);
	return box;
}

GtkWidget *Box::create_vbox(GtkWidget *parent, int cell_space)
{
//	g_print("create_vbox\n");
	int cs = (cell_space >= 0 ? cell_space : cellspace);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,cs);
#else
	GtkWidget *box = gtk_vbox_new(FALSE, cs);
#endif
	add_widget(parent, box);
	return box;
}

void Box::add_widget(GtkWidget *parent, GtkWidget *child, gboolean expand, gboolean fill)
{
	if (!parent || !child) return;
	if (GTK_IS_BOX(parent)) {
		gtk_box_pack_start(GTK_BOX(parent), child, expand, fill, cellspace);
#if !GTK_CHECK_VERSION(3,8,0)
	} else if (GTK_IS_SCROLLED_WINDOW(parent)) {
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(parent), child);
#endif
	} else {
		gtk_container_add(GTK_CONTAINER(parent), child);
	}
}


//

Control::Control()
{
	cellspace = 4;
}

Control::~Control()
{
}

//
GtkWidget *Control::create_notebook(GtkWidget *parent)
{
	GtkWidget *nb = gtk_notebook_new();
	add_widget(parent, nb);
	return nb;
}

void Control::add_note(GtkWidget *parent, GtkWidget *child, const char *label)
{
	GtkWidget *lbl = gtk_label_new(label);
	gtk_notebook_append_page(GTK_NOTEBOOK(parent), child, lbl);
}
void Control::add_note(GtkWidget *parent, GtkWidget *child, CMsg::Id labelid)
{
	const char *label = CMSGV(labelid);
	add_note(parent, child, label);
}

//
GtkWidget *Control::create_frame(GtkWidget *parent, const char *label, GtkWidget **vbox, GtkWidget **hbox)
{
//	g_print("create_frame: %s\n",label);
	GtkWidget *frm = gtk_frame_new(label);
	add_widget(parent, frm);

	if (vbox != NULL) {
		*vbox = create_vbox(frm);
		if (hbox != NULL) {
			*hbox = create_hbox(*vbox);
		}
	}
	return frm;
}
GtkWidget *Control::create_frame(GtkWidget *parent, CMsg::Id labelid, GtkWidget **vbox, GtkWidget **hbox)
{
	const char *label = CMSGV(labelid);
	return create_frame(parent, label, vbox, hbox);
}

//
GtkWidget *Control::create_check_box(GtkWidget *parent, const char *label, bool chkd, int num, GCallback handler)
{
//	g_print("create_check_box: %s\n",label);
	GtkWidget *chk = gtk_check_button_new_with_label(label);
	add_widget(parent, chk);
	set_check_state(chk, chkd);
	if (handler) {
		g_signal_connect(G_OBJECT(chk), "toggled", handler, (gpointer)this);
		g_object_set_data(G_OBJECT(chk), "num", (gpointer)(intptr_t)num);
	}
	return chk;
}
//
GtkWidget *Control::create_check_box(GtkWidget *parent, const CMsg::Id labelid, bool chkd, int num, GCallback handler)
{
	const char *label = CMSGV(labelid);
	return create_check_box(parent, label, chkd, num, handler);
}
bool Control::get_check_state(GtkWidget *check)
{
	return (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)) == TRUE ? true : false);
}
int Control::get_check_state_num(GtkWidget **checks, int cnt)
{
	int bit = 0;
	for(int idx = 0; idx < cnt; idx++) {
		if (checks[idx] != NULL && get_check_state(checks[idx])) bit |= (1 << idx);
	}
	return bit;
}
void Control::set_check_state(GtkWidget *check, bool value)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), value ? TRUE : FALSE);
}

//
GtkWidget *Control::create_radio_box(GtkWidget *parent, GtkWidget **gradio, const char *label, int num, int sel_num, GCallback handler)
{
//	g_print("create_radio_box 1: %s\n",label);
	GtkWidget *radio;
	if (num == 0) {
		radio = gtk_radio_button_new_with_label(NULL,label);
	} else {
		radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(gradio[num-1]),label);
	}
	add_widget(parent, radio);
	if (num == sel_num) set_radio_state(radio, true);
	if (handler) {
		g_signal_connect(G_OBJECT(radio), "toggled", handler, (gpointer)this);
		g_object_set_data(G_OBJECT(radio), "num", (gpointer)(intptr_t)num);
	}
	return radio;
}
GtkWidget *Control::create_radio_box(GtkWidget *parent, GtkWidget **gradio, const CMsg::Id labelid, int num, int sel_num, GCallback handler)
{
	const char *label = CMSGV(labelid);
	return create_radio_box(parent, gradio, label, num, sel_num, handler);
}
void Control::create_radio_box(GtkWidget *parent, const char **list, int cnt, GtkWidget **radio, int sel_num)
{
//	g_print("create_radio_box 2\n");
	if (list == NULL || radio == NULL) return;
	for(int i=0; i<cnt && list[i] != NULL; i++) {
		if (i == 0) {
			radio[i] = gtk_radio_button_new_with_label(NULL,list[i]);
		} else {
			radio[i] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio[i-1]),list[i]);
		}
		add_widget(parent, radio[i]);
	}
	set_radio_state(radio[sel_num], true);
}
void Control::create_radio_box(GtkWidget *parent, const CMsg::Id *list, int cnt, GtkWidget **radio, int sel_num)
{
//	g_print("create_radio_box 2\n");
	if (list == NULL || radio == NULL) return;
	for(int i=0; i<cnt && list[i] != 0 && list[i] != CMsg::End; i++) {
		const char *label = CMSGV(list[i]);
		if (i == 0) {
			radio[i] = gtk_radio_button_new_with_label(NULL, label);
		} else {
			radio[i] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio[i-1]), label);
		}
		add_widget(parent, radio[i]);
	}
	set_radio_state(radio[sel_num], true);
}
bool Control::get_radio_state(GtkWidget *radio)
{
	return (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == TRUE ? true : false);
}
int Control::get_radio_state_idx(GtkWidget **radios, int cnt)
{
	int idx = 0;
	for(; idx < cnt && radios[idx] != NULL; idx++) {
		if (get_radio_state(radios[idx])) break;
	}
	return idx;
}
void Control::set_radio_state(GtkWidget *radio, bool value)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), value ? TRUE : FALSE);
}

//
GtkWidget *Control::create_combo_box(GtkWidget *parent, const char *label, const char **list, int sel_num)
{
//	g_print("create_combo_box: %s\n",label);
	GtkWidget *lbl = gtk_label_new(label);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *com = gtk_combo_box_text_new();
	for(int i=0; list[i] != NULL; i++) {
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com),NULL,list[i]);
	}
#else
	GtkWidget *com = gtk_combo_box_new_text();
	for(int i=0; list[i] != NULL; i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(com),list[i]);
	}
#endif
	add_widget(parent, lbl);
	add_widget(parent, com);
	set_combo_sel_num(com,sel_num);
	return com;
}
GtkWidget *Control::create_combo_box(GtkWidget *parent, CMsg::Id labelid, const char **list, int sel_num)
{
	const char *label = CMSGV(labelid);
	return create_combo_box(parent, label, list, sel_num);
}
GtkWidget *Control::create_combo_box(GtkWidget *parent, const char *label, const CMsg::Id *ids, int sel_num, int appendnum, CMsg::Id appendstr)
{
	GtkWidget *lbl = gtk_label_new(label);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *com = gtk_combo_box_text_new();
	for(int i=0; ids[i] != 0 && ids[i] != CMsg::End; i++) {
		if (i == appendnum) {
			char label[_MAX_PATH];
			UTILITY::tcscpy(label, _MAX_PATH, CMSGV(ids[i]));
			UTILITY::tcscat(label, _MAX_PATH, CMSGV(appendstr));
			gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com),NULL,label);
		} else {
			gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com),NULL,CMSGV(ids[i]));
		}
	}
#else
	GtkWidget *com = gtk_combo_box_new_text();
	for(int i=0; ids[i] != 0 && ids[i] != CMsg::End; i++) {
		if (i == appendnum) {
			char label[_MAX_PATH];
			UTILITY::tcscpy(label, _MAX_PATH, CMSGV(ids[i]));
			UTILITY::tcscat(label, _MAX_PATH, CMSGV(appendstr));
			gtk_combo_box_append_text(GTK_COMBO_BOX(com),label);
		} else {
			gtk_combo_box_append_text(GTK_COMBO_BOX(com),CMSGV(ids[i]));
		}
	}
#endif
	add_widget(parent, lbl);
	add_widget(parent, com);
	set_combo_sel_num(com,sel_num);
	return com;
}
GtkWidget *Control::create_combo_box(GtkWidget *parent, CMsg::Id labelid, const CMsg::Id *ids, int sel_num, int appendnum, CMsg::Id appendstr)
{
	const char *label = CMSGV(labelid);
	return create_combo_box(parent, label, ids, sel_num, appendnum, appendstr);
}
GtkWidget *Control::create_combo_box(GtkWidget *parent, const char *label, const CPtrList<CTchar> &list, int sel_num)
{
	GtkWidget *lbl = gtk_label_new(label);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *com = gtk_combo_box_text_new();
	for(int i=0; i<list.Count(); i++) {
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com),NULL,list.Item(i)->GetN());
	}
#else
	GtkWidget *com = gtk_combo_box_new_text();
	for(int i=0; i<list.Count(); i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(com),list.Item(i)->GetN());
	}
#endif
	add_widget(parent, lbl);
	add_widget(parent, com);
	set_combo_sel_num(com,sel_num);
	return com;
}
GtkWidget *Control::create_combo_box(GtkWidget *parent, CMsg::Id labelid, const CPtrList<CTchar> &list, int sel_num)
{
	const char *label = CMSGV(labelid);
	return create_combo_box(parent, label, list, sel_num);
}

int Control::get_combo_sel_num(GtkWidget *combo)
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
}
void Control::set_combo_sel_num(GtkWidget *combo, int sel_num)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo),sel_num);
}

//
GtkWidget *Control::create_text(GtkWidget *parent, int width)
{
	GtkWidget *txt = gtk_entry_new();
	gtk_widget_set_size_request(txt, width, -1);
	add_widget(parent, txt);
	return txt;
}
GtkWidget *Control::create_text(GtkWidget *parent, const char *text, int len)
{
//	g_print("create_text:\n");
	GtkWidget *txt = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(txt), len);
	if (text != NULL) gtk_entry_set_text(GTK_ENTRY(txt), text);
	add_widget(parent, txt);
	return txt;
}
GtkWidget *Control::create_text_with_label(GtkWidget *parent, const char *label, const char *text, int len)
{
//	g_print("create_text_with_label: %s\n",label);
	GtkWidget *lbl = gtk_label_new(label);
	GtkWidget *txt = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(txt), len);
	if (text != NULL) gtk_entry_set_text(GTK_ENTRY(txt), text);
	add_widget(parent, lbl);
	add_widget(parent, txt);
	return txt;
}
GtkWidget *Control::create_text_with_label(GtkWidget *parent, CMsg::Id labelid, const char *text, int len)
{
	const char *label = CMSGV(labelid);
	return create_text_with_label(parent, label, text, len);
}

const char *Control::get_text(GtkWidget *entry)
{
	return gtk_entry_get_text(GTK_ENTRY(entry));
}
void Control::set_text(GtkWidget *entry, const char *text)
{
	if (GTK_IS_ENTRY(entry)) {
		gtk_entry_set_text(GTK_ENTRY(entry), text ? text : "");
	}
}

//
GtkWidget *Control::create_text_view(GtkWidget *parent, int width, int height)
{
	GtkWidget *txtv = gtk_text_view_new();
	gtk_widget_set_size_request(txtv, width, height);
	add_widget(parent, txtv);
	return txtv;
}

//
GtkWidget *Control::create_spin(GtkWidget *parent, int range_min, int range_max, int val)
{
	GtkWidget *spin = gtk_spin_button_new_with_range((gdouble)range_min, (gdouble)range_max, 1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), (gdouble)val);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 0);
	add_widget(parent, spin);
	return spin;
}
gdouble Control::get_spin_value(GtkWidget *spin)
{
	return gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));
}

//
GtkWidget *Control::create_label(GtkWidget *parent, const char *label)
{
//	g_print("create_label: %s\n",label);
	GtkWidget *lbl = gtk_label_new(label);
	add_widget(parent, lbl);
	return lbl;
}
GtkWidget *Control::create_label(GtkWidget *parent, CMsg::Id labelid)
{
	const char *label = CMSGV(labelid);
	return create_label(parent, label);
}

//
GtkWidget *Control::create_button(GtkWidget *parent, const char *label, GCallback handler)
{
//	g_print("create_button: %s\n",label);
	GtkWidget *btn = gtk_button_new_with_label(label);
	add_widget(parent, btn);
	if (handler) g_signal_connect(G_OBJECT(btn), "clicked", handler, (gpointer)this);
	return btn;
}
GtkWidget *Control::create_button(GtkWidget *parent, CMsg::Id labelid, GCallback handler)
{
	const char *label = CMSGV(labelid);
	return create_button(parent, label, handler);
}

//
GtkWidget *Control::create_volume_box(GtkWidget *parent, int val, int num, GCallback handler)
{
#if GTK_CHECK_VERSION(3,2,0)
	GtkWidget *vol = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 100.0, 1.0);
#else
	GtkWidget *vol = gtk_vscale_new_with_range(0.0, 100.0, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(vol), (gdouble)val);
	gtk_range_set_inverted(GTK_RANGE(vol), TRUE);
	if (handler) {
		g_signal_connect(G_OBJECT(vol), "value-changed", handler, (gpointer)this);
		g_object_set_data(G_OBJECT(vol), "num", (gpointer)(intptr_t)num);
	}
	add_widget(parent, vol);
	return vol;
}
gdouble Control::get_volume_value(GtkWidget *volume)
{
	return gtk_range_get_value(GTK_RANGE(volume));
}

//
GtkWidget *Control::create_scale_box(GtkWidget *parent, int range_min, int range_max, int val, bool vertical, int num, GCallback handler)
{
	GtkWidget *vol = NULL;
#if GTK_CHECK_VERSION(3,2,0)
	vol = gtk_scale_new_with_range(vertical ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL, range_min, range_max, 1.0);
#else
	if (vertical) {
		vol = gtk_vscale_new_with_range(range_min, range_max, 1.0);
	} else {
		vol = gtk_hscale_new_with_range(range_min, range_max, 1.0);
	}
#endif
	gtk_range_set_value(GTK_RANGE(vol), (gdouble)val);
	gtk_range_set_inverted(GTK_RANGE(vol), vertical ? TRUE : FALSE);
	if (handler) {
		g_signal_connect(G_OBJECT(vol), "value-changed", handler, (gpointer)this);
		g_object_set_data(G_OBJECT(vol), "num", (gpointer)(intptr_t)num);
	}
	add_widget(parent, vol);
	return vol;
}
gdouble Control::get_scale_value(GtkWidget *scale)
{
	return gtk_range_get_value(GTK_RANGE(scale));
}

GtkWidget *Control::create_hbox(GtkWidget *parent, int cell_space, gboolean expand)
{
//	g_print("create_hbox\n");
	int cs = (cell_space >= 0 ? cell_space : cellspace);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,cs);
#else
	GtkWidget *box = gtk_hbox_new(FALSE, cs);
#endif
	add_widget(parent, box, expand, expand);
	return box;
}

GtkWidget *Control::create_vbox(GtkWidget *parent, int cell_space, gboolean expand)
{
//	g_print("create_vbox\n");
	int cs = (cell_space >= 0 ? cell_space : cellspace);
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,cs);
#else
	GtkWidget *box = gtk_vbox_new(FALSE, cs);
#endif
	add_widget(parent, box, expand, expand);
	return box;
}

GtkWidget *Control::create_grid(GtkWidget *parent)
{
//	g_print("create_grid\n");
#if GTK_CHECK_VERSION(3,4,0)
	GtkWidget *grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	add_widget(parent, grid);
	return grid;
#else
	GtkWidget *tbl = gtk_table_new(3,3,TRUE);
	add_widget(parent, tbl);
	return tbl;
#endif
}

void Control::attach_to_grid(GtkWidget *grid, GtkWidget *child, int col, int row)
{
#if GTK_CHECK_VERSION(3,4,0)
	gtk_grid_attach(GTK_GRID(grid),child,col,row,1,1);
#else
	gtk_table_attach_defaults(GTK_TABLE(grid), child, col, col+1, row, row+1);
#endif
}

GtkWidget *Control::create_scroll_win(GtkWidget *parent, int width, int height, gboolean expand)
{
//	g_print("create_scroll_win\n");
	GtkWidget *swin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(swin, width, height);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin), GTK_SHADOW_IN);
	gtk_widget_set_vexpand(swin, expand);
	add_widget(parent, swin, expand, expand);
	return swin;
}

void Control::add_widget(GtkWidget *parent, GtkWidget *child, gboolean expand, gboolean fill)
{
	if (!parent || !child) return;
	if (GTK_IS_BOX(parent)) {
		gtk_box_pack_start(GTK_BOX(parent), child, expand, fill, cellspace);
#if !GTK_CHECK_VERSION(3,8,0)
	} else if (GTK_IS_SCROLLED_WINDOW(parent)) {
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(parent), child);
#endif
	} else {
		gtk_container_add(GTK_CONTAINER(parent), child);
	}
}

void Control::set_enable(GtkWidget *widget, bool enable)
{
	gtk_widget_set_sensitive(widget, enable);
}

//
//
//

DialogBox::DialogBox(GUI *new_gui)
	:Control()
{
	gui = new_gui;
	window = NULL;
	dialog = NULL;
}

DialogBox::~DialogBox()
{
	if (dialog) gtk_widget_destroy(dialog);
}

bool DialogBox::Show(GtkWidget *parent_window)
{
	window = parent_window;
	return true;
}

bool DialogBox::Show(GtkWidget *parent_window, int)
{
	return Show(parent_window);
}

bool DialogBox::Show(GtkWidget *parent_window, int, bool)
{
	return Show(parent_window);
}

bool DialogBox::ShowModal(GtkWidget *parent_window)
{
	bool rc = Show(parent_window);
	if (!rc) return rc;
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	return (response == GTK_RESPONSE_ACCEPT);
}

void DialogBox::Hide()
{
	if (dialog) {
		gtk_widget_destroy(dialog);
		dialog = NULL;
	}
}

bool DialogBox::IsVisible() const
{
	if (dialog) {
		gboolean rc = FALSE;
#if GTK_CHECK_VERSION(3,8,0)
		rc = gtk_widget_is_visible(dialog);
#else
		rc = gtk_widget_get_visible(dialog);
#endif
		return (rc == TRUE);
	}
	return false;
}

GtkWidget *DialogBox::create_dialog(GtkWidget *parent, const char *title)
{
	int flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(parent),
		(GtkDialogFlags)flags,
		NULL,
		NULL);
	return dialog;
}
GtkWidget *DialogBox::create_dialog(GtkWidget *parent, CMsg::Id titleid)
{
	const char *title = CMSGV(titleid);
	return create_dialog(parent, title);
}

GtkWidget *DialogBox::create_window(GtkWidget *parent, const char *title)
{
	int flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(parent),
		(GtkDialogFlags)flags,
		NULL,
		NULL);
	return dialog;
}
GtkWidget *DialogBox::create_window(GtkWidget *parent, CMsg::Id titleid)
{
	const char *title = CMSGV(titleid);
	return create_window(parent, title);
}

GtkWidget *DialogBox::add_accept_button(CMsg::Id labelid)
{
	const char *label = CMSGV(labelid);
	return gtk_dialog_add_button(GTK_DIALOG(dialog), label, GTK_RESPONSE_ACCEPT);
}
GtkWidget *DialogBox::add_reject_button(CMsg::Id labelid)
{
	const char *label = CMSGV(labelid);
	return gtk_dialog_add_button(GTK_DIALOG(dialog), label, GTK_RESPONSE_REJECT);
}

}; /* namespace GUI_GTK_X11 */


