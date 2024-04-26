/** @file win_dialogbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21

	@brief [ dialog box ]
*/

#ifndef WIN_DIALOGBOX_H
#define WIN_DIALOGBOX_H

#include <windows.h>
#include <commctrl.h>
#include "../../clocale.h"
#include "winfont.h"
#include "../../common.h"
#include "../../res/resource.h"
#include "../../msgs.h"
#include "../../cchar.h"
#include "../../cptrlist.h"

#undef  USE_LAYOUT_RECT
#define USE_LAYOUT_CBOX

class EMU;
class GUI;

/// @namespace GUI_WIN
/// @brief Dialogs for Windows
namespace GUI_WIN
{

#ifdef USE_LAYOUT_CBOX
class CBox;
class CTabItems;
#endif

/**
	@brief dialog box template

	Wrapper creating a widget control
*/
class CDialogBox
{
protected:
	HINSTANCE   hInstance;
	DWORD       dialogId;
	HWND        hDlg;
	bool        isModeless;
	CFont      *font;
//	HWND		hToolTips;
	EMU        *emu;
	GUI        *gui;

	int padding;
	int margin;
	RECT re_max;

	int curStaticId;
	int maxStaticId;

	virtual INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	virtual INT_PTR onCommand(UINT, WPARAM, LPARAM);
	virtual INT_PTR onNotify(UINT, WPARAM, LPARAM);
	virtual INT_PTR onMouseWheel(UINT, WPARAM, LPARAM);
	virtual INT_PTR onHScroll(UINT, WPARAM, LPARAM);
	virtual INT_PTR onVScroll(UINT, WPARAM, LPARAM);
	virtual INT_PTR onSize(UINT, WPARAM, LPARAM);
	virtual INT_PTR onMinMaxInfo(UINT, WPARAM, LPARAM);
	virtual INT_PTR onOK(UINT, WPARAM, LPARAM);
	virtual INT_PTR onClose(UINT, WPARAM, LPARAM);
	virtual INT_PTR onHelp(UINT, WPARAM, LPARAM);
	virtual INT_PTR onControlColorStatic(UINT, WPARAM, LPARAM);
	virtual INT_PTR onControlColorDialog(UINT, WPARAM, LPARAM);

	static INT_PTR CALLBACK Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//	virtual void AddToolTipText(const t_tooltip_text *texts);

private:
	static BOOL CALLBACK ChildProc(HWND hCtrl, LPARAM lParam);

public:
	CDialogBox(HINSTANCE new_inst, DWORD new_id, CFont *new_font, EMU *new_emu, GUI *new_gui);
	virtual ~CDialogBox();

	virtual INT_PTR Show(HWND hWnd);
	virtual HWND Create(HWND hWnd);
	virtual void Close();
	virtual void SetFontAndTranslateText();
#ifdef USE_LAYOUT_RECT
	virtual HWND CreateStatic(int nItemId, const _TCHAR *label, RECT *re, bool align_right = false);
	virtual HWND CreateComboBox(int nItemId, const _TCHAR **list, int selnum, int nMinSize, RECT *re, bool align_right = false, bool translate = false);
	virtual HWND CreateDefaultButton(int nItemId, const _TCHAR *caption, int nSize, RECT *re, bool align_right = false);
	virtual HWND CreateButton(int nItemId, const _TCHAR *caption, int nSize, RECT *re, bool align_right = false);
	virtual void AdjustButton(int nItemId, int nSize, RECT *re, bool align_right = false);
	virtual void AdjustStatic(int nItemId, RECT *re, bool align_right = false);
	virtual void AdjustCheckBox(int nItemId, RECT *re, bool align_right = false);
	virtual void AdjustEditBox(int nItemId, int nMaxSize, RECT *re, bool align_right = false, const _TCHAR ch = _T('m'));
	virtual void AdjustEditBox(int nItemId, RECT *re, const _TCHAR *cLenStr, bool align_right = false);
	virtual void AdjustEditBoxWithStatic(int nStaticItemId, int nEditItemId, int nEditMaxSize, RECT *re, bool align_right = false);
	virtual void AdjustComboBox(int nItemId, int nMinSize, RECT *re, bool align_right = false);
	virtual void AdjustComboBoxWithStatic(int nStaticItemId, int nComboItemId, int nComboMinSize, RECT *re, bool align_right = false);
	virtual void AdjustControl(int nItemId, RECT *re, int w, int h, bool align_right = false, int style = 0);
#endif
	virtual void AddComboBoxItem(int nItemId, const _TCHAR *str, bool translate = false);
	virtual void SetComboBoxItems(HWND combo, const _TCHAR **list, int selnum, bool translate = false);
	virtual void SetComboBoxItems(int nItemId, const _TCHAR **list, int selnum, bool translate = false);
	virtual void SetComboBoxItems(HWND combo, const CMsg::Id *list, int selnum, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	virtual void SetComboBoxItems(int nItemId, const CMsg::Id *list, int selnum, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	virtual void SetComboBoxItems(HWND combo, const CPtrList<CTchar> &list, int selnum);
	virtual void SetComboBoxItems(int nItemId, const CPtrList<CTchar> &list, int selnum);
	virtual void SelectComboBoxItem(int nItemId, int selnum);
	virtual void ReplaceComboBoxItem(HWND combo, int selnum, const CMsg::Id id, bool append_text = false);
	virtual void ReplaceComboBoxItem(int nItemId, int selnum, const CMsg::Id id, bool append_text = false);
	virtual void AdjustWindow(int lmargin);

	virtual void ResizeControl(int nItemId, int w, int h);
	virtual void AdjustControlSize(int nItemId, int diff_x, int diff_y);
	virtual void AdjustControlPos(int nItemId, int diff_x, int diff_y);
	virtual int  GetControlXdiff(int nItemId, int new_width);
	virtual void GetControlPos(int nItemId, RECT *re);
	virtual void GetControlSize(int nItemId, SIZE *size);

	//
#ifdef USE_LAYOUT_CBOX
	virtual HWND CreateControl(CBox *box, const _TCHAR *class_name, int nItemId, int min_w, int min_h, int style = 0, int exstyle = 0, int scrollstyle = 0);
	virtual HWND CreateStatic(CBox *box, int nItemId, const _TCHAR *label, int min_w = 0, int min_h = 0, int align = 0, int exstyle = 0);
	virtual HWND CreateStatic(CBox *box, int nItemId, CMsg::Id label, int min_w = 0, int min_h = 0, int align = 0, int exstyle = 0);
	virtual CBox *CreateGroup(CBox *box, int nItemId, const _TCHAR *label, int orient, int align = 0, HWND *hCtrl = NULL);
	virtual CBox *CreateGroup(CBox *box, int nItemId, CMsg::Id label, int orient, int align = 0, HWND *hCtrl = NULL);
	virtual HWND CreateComboBox(CBox *box, int nItemId, const _TCHAR **list, int selnum, int nMinSize, bool translate = false);
	virtual HWND CreateComboBox(CBox *box, int nItemId, const CMsg::Id *list, int selnum, int nMinSize, int appendnum = -1, CMsg::Id appendstr = CMsg::End);
	virtual HWND CreateComboBox(CBox *box, int nItemId, const CPtrList<CTchar> &list, int selnum, int nMinSize);
	virtual HWND CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const _TCHAR **list, int selnum, int nMinSize);
	virtual HWND CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const CMsg::Id *list, int selnum, int nMinSize);
	virtual HWND CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const CPtrList<CTchar> &list, int selnum, int nMinSize);
	virtual HWND CreateComboTextBox(CBox *box, int nItemId, const _TCHAR **list, int defnum, int nMinSize, bool translate = false);
	virtual HWND CreateComboTextBox(CBox *box, int nItemId, const _TCHAR **list, const _TCHAR *deftext, int nMinSize, bool translate = false);
	virtual HWND CreateButton(CBox *box, int nItemId, const _TCHAR *caption, int nSize, bool default_button = false);
	virtual HWND CreateButton(CBox *box, int nItemId, CMsg::Id caption, int nSize, bool default_button = false);
	virtual HWND CreateCheckBox(CBox *box, int nItemId, const _TCHAR *caption, bool value, int min_w = 0, int min_h = 0);
	virtual HWND CreateCheckBox(CBox *box, int nItemId, CMsg::Id caption, bool value, int min_w = 0, int min_h = 0);
	virtual HWND CreateRadioButton(CBox *box, int nItemId, const _TCHAR *caption, bool first = false, int min_w = 0, int min_h = 0);
	virtual HWND CreateRadioButton(CBox *box, int nItemId, CMsg::Id caption, bool first = false, int min_w = 0, int min_h = 0);
	virtual HWND CreateEditBox(CBox *box, int nItemId, const _TCHAR *text, int nMaxSize, int align = 0, const _TCHAR ch = _T('m'));
	virtual HWND CreateEditBox(CBox *box, int nItemId, int digit, int nMaxSize, int align = 0, const _TCHAR ch = _T('m'));
	virtual HWND CreateEditBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const _TCHAR *text, int nMaxSize, int align = 0, const _TCHAR ch = _T('m'));
	virtual HWND CreateEditBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, int digit, int nMaxSize, int align = 0, const _TCHAR ch = _T('m'));
	virtual HWND CreateTextControl(CBox *box, int nItemId, bool multi_line = false, bool read_only = false, int min_w = 0, int min_h = 0);
	virtual HWND CreateSlider(CBox *box, int nItemId, int min_w, int min_h, int range_min, int range_max, int ticks, int value, bool vertical = true);
	virtual HWND CreateUpDown(CBox *box, int nItemId, HWND hEdit, int range_min, int range_max, int value);

	virtual void AdjustButton(CBox *box, int nItemId, int nSize);
	virtual void AdjustStatic(CBox *box, int nItemId, int min_w = 0, int min_h = 0);
	virtual CBox *AdjustGroup(CBox *box, int nItemId, int orient, int align = 0);
	virtual void AdjustCheckBox(CBox *box, int nItemId, int min_w = 0, int min_h = 0);
	virtual void AdjustEditBox(CBox *box, int nItemId, int nMaxSize, const _TCHAR ch = _T('m'));
	virtual void AdjustComboBox(CBox *box, int nItemId, int nMinSize);
	virtual void AdjustControl(CBox *box, int nItemId, int w, int h, int style = 0);
	virtual CBox *AdjustTabControl(CBox *box, int nItemId, int nBGItemId);
#endif

	void EnableDlgItem(int nItemId, bool value);
	void ShowDlgItem(int nItemId, bool value);

	int GetRight() { return re_max.right; }
	int GetBottom() { return re_max.bottom; }

	HWND GetDlg() { return hDlg; }
	void Larger(RECT *re);
	void Larger(LONG width, LONG height);

	int FindEmptyID();

#ifdef _DEBUG
	EMU *GetEmu() { return emu; }
#endif
};

#ifdef USE_LAYOUT_CBOX
/**
	@brief Manage layouting for widget controls on a dialog
*/
class CBox
{
public:
	enum enOrient {
		VerticalBox = 0,
		HorizontalBox,
		TabControlBox,
	};
	enum enAlign {
		LeftPos   = 0x00,
		CenterPos = 0x01,
		RightPos  = 0x02,
		TopPos    = 0x00,
		MiddlePos = 0x10,
		BottomPos = 0x20
	};
private:
	enOrient orient;
	int      align;
	VmRectWH re;	// absolute position
	bool realized;

	int padding;
	VmRect margin;

	_TCHAR name[10];

	CBox *parent_box;
	struct stControls {
		CBox *box;
		int itemid;
		int x;	// relative position x from re.x
		int y;	// relative position y from re.y
		int px;	// parts position x from x
		int py;	// parts position y from y
		int w;
		int h;
		struct stControls *next;
	} *controls;
	int control_nums;

	CTabItems *tab_items;

	void AddItem(CBox *box, int itemid, int width, int height, int px, int py);
	void AddWidth(int width);
	void AddHeight(int height);

	void RealizeReal(CDialogBox &dlg);
	void MoveItems(CBox *parent, CDialogBox &dlg);

public:
	CBox(int orient, int align = LeftPos | TopPos, int margin = 0, const _TCHAR *name = NULL);
	virtual ~CBox();

	void AddBox(CBox *box);
	CBox *AddBox(int orient, int align = LeftPos | TopPos, int margin = 0, const _TCHAR *name = NULL);
	void AddControl(int itemid, int width, int height, int px = 0, int py = 0);
	void AddControlWithBox(CBox *box, int itemid);
	void AddSpace(int width, int height, int px = 0, int py = 0);

	void Realize(CDialogBox &dlg);
	void AdjustPosition(int x, int y);

	int GetWidthWithMargin() const;
	int GetHeightWithMargin() const;
	int GetWidthWithLeftMargin() const;
	int GetHeightWithTopMargin() const;
	int GetWidth() const;
	int GetHeight() const;

	void GetPositionByItem(int num, int &x, int &y);

	void SetLeftMargin(int val);
	void SetTopMargin(int val);

	void SetTabItems(CTabItems *new_items);
	void AddIdToTabItems(int itemid);
};
#endif

// for tab control

/**
	@brief Relate control items to a page on Tab control
*/
class CTabItemIds : public std::vector<int>
{
public:
	CTabItemIds();
	virtual ~CTabItemIds();
};

/**
	@brief Manage showing/hiding control items for Tab control on a dialog
*/
class CTabItems : public CPtrList<CTabItemIds>
{
private:
	int current_pos;
public:
	CTabItems();
	virtual ~CTabItems();
	void SetCurrentPosition(int val);
	void AddItemId(int itemid);
};

}; /* namespace GUI_WIN */

#endif /* WIN_DIALOGBOX_H */
