/// @file win_keybindctrl.h
///
/// @brief キーコード割り付けコントロール
///
/// @author Sasaji
/// @date   2012.3.31
///

#ifndef WIN_KEYBINDCONTROL_H
#define WIN_KEYBINDCONTROL_H

#include <windows.h>
#include "../../common.h"
#include "../../msgs.h"
#include "../gui_keybinddata.h"

#define KBCTRL_MAX_LINES 128
#define KBCTRL_MAX_COLS  2

namespace GUI_WIN
{

class KeybindControl;
class CDialogBox;
class CBox;

typedef struct ctrlcols_st {
	int     row;
	int     col;
	HWND    hEdt;
	WNDPROC proc;
} ctrlcols_t;

typedef struct ctrltable_st {
	HWND       hSta1;
	ctrlcols_t cols[KBCTRL_MAX_COLS];
	bool       enabled;
} ctrltable_t;

/// @brief キーコード割り付けコントロール
class KeybindControl : public KeybindData
{
private:
	HINSTANCE hInstance;
	HWND hMainCtrl;
	HWND hStaticMain;
	HFONT font;

	bool use_timer;

	HWND       hTitles[KBCTRL_MAX_COLS + 1];
	ctrltable_t table[KBCTRL_MAX_LINES];

	HWND hChkCombi;

	int cell_height;
	int cell_width;
	int padding;
	int margin;

	int ctrl_width;
	int ctrl_height;

	SCROLLINFO vScrMain;

	void create_children(HWND hWnd);
	void setfont_children(HWND hWnd, WPARAM wParam);
	void destroy_children(HWND hWnd);
	void dispose_children(HWND hWnd);
	LRESULT ctlcolor_children(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void move_children(HWND hWnd, int x, int y);

	void init_joypad(HWND hWnd);
	void term_joypad(HWND hWnd);
	void update_joypad(ctrlcols_t *obj);

	UINT32 translate_keycode(WPARAM wParam, LPARAM lParam);

	void set_scroll_bar_range(HWND hWnd, int y);
	void set_scroll_bar_pos(HWND hWnd, int dy);

	static LRESULT CALLBACK CustomCtrlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CustomCtrlMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CustomCtrlEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	KeybindControl(HINSTANCE hInst, HWND hWnd);
	~KeybindControl();

	static ATOM RegisterClass(HINSTANCE);
	static void UnregisterClass(HINSTANCE);

	static KeybindControl *GetPtr(HWND);

	void Init(EMU *emu, int new_tabnum, HFONT new_font);

	LRESULT Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//	void SetTitleLabel(const _TCHAR *vmlabel, const _TCHAR *vklabel);
	void SetTitleLabel(CMsg::Id vmlabelid, CMsg::Id vklabelid);
	void SetCellSize(int w, int h);

	void   Update();
	bool   SetVkLabel(int row, int col);

	bool   SetKeyCell(ctrlcols_t *obj, uint32_t code);
	bool   SetJoyCell(ctrlcols_t *obj, uint32_t code0, uint32_t code1);
	bool   ClearCell(ctrlcols_t *obj);
	bool   ClearCellByVkCode(uint32_t code);

	bool   MapDefaultVmKey();

	void   LoadDefaultPreset();
	void   LoadPreset(int idx);
	void   SavePreset(int idx);

	void   SetData();

	HWND   AddCombiCheckButton(CDialogBox *dlg, CBox *box);
	HWND   GetCombiCheckButton();
	void   SetCombiCheckData();
	void   SetCombiCheckButton(int val);
	void   UpdateCombiCheckButton();
	int    GetCombiId();

	int    GetWidth() { return ctrl_width; };
	int    Getheight() { return ctrl_height; };
	int    GetCellWidth() { return cell_width; };
	int    GetCellHeight() { return cell_height; };
};

}; /* namespace GUI_WIN */

#endif /* WIN_KEYBINDCONTROL_H */
