/** @file win_dialogbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21

	@brief [ dialog box ]
*/

#include "win_dialogbox.h"
#include "win_gui.h"
#include "../../emu.h"
#include "../../utility.h"


namespace GUI_WIN
{

CDialogBox::CDialogBox(HINSTANCE new_inst, DWORD new_id, CFont *new_font, EMU *new_emu, GUI *new_gui)
{
	hInstance = new_inst;
	dialogId  = new_id;
	hDlg      = NULL;
	font      = new_font;
	emu       = new_emu;
	gui       = new_gui;
	isModeless = false;

	padding   = 2;
	margin    = 10;

	// size of dialog
	re_max.left = 0;
	re_max.top = 0;
	re_max.right = 0;
	re_max.bottom = 0;

	curStaticId = IDC_STATIC_AUTO;
	maxStaticId = curStaticId + 100;
//	hToolTips = NULL;
}

CDialogBox::~CDialogBox()
{
	Close();
}

/// create modal dialog box
INT_PTR CDialogBox::Show(HWND hWnd)
{
	return ::DialogBoxParam(hInstance, MAKEINTRESOURCE(dialogId), hWnd, Proc, (LPARAM)this);
}

/// create modeless dialog box
HWND CDialogBox::Create(HWND hWnd)
{
	if (hDlg == NULL) {
		hDlg = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(dialogId), hWnd, Proc, (LPARAM)this);
		if (hDlg != NULL) {
			isModeless = true;
		}
	}
	return hDlg;
}

/// close modeless dialog box
void CDialogBox::Close()
{
	if (isModeless && hDlg != NULL) {
		::DestroyWindow(hDlg);
		isModeless = false;
	}
	hDlg = NULL;
}

INT_PTR CALLBACK CDialogBox::Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	CDialogBox *myObj = (CDialogBox *)GetWindowLongPtr(hDlg, DWLP_USER);

	switch (message) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hDlg, DWLP_USER, lParam);
			myObj = (CDialogBox *)lParam;
			myObj->hDlg = hDlg;

			return myObj->onInitDialog(message, wParam, lParam);
		case WM_COMMAND:
			return myObj->onCommand(message, wParam, lParam);
		case WM_NOTIFY:
			return myObj->onNotify(message, wParam, lParam);
		case WM_MOUSEWHEEL:
			return myObj->onMouseWheel(message, wParam, lParam);
		case WM_HSCROLL:
			return myObj->onHScroll(message, wParam, lParam);
		case WM_VSCROLL:
			return myObj->onVScroll(message, wParam, lParam);
		case WM_CLOSE:
			return myObj->onClose(message, wParam, lParam);
		case WM_HELP:
			return myObj->onHelp(message, wParam, lParam);
		case WM_CTLCOLORSTATIC:
			return myObj->onControlColorStatic(message, wParam, lParam);
		case WM_CTLCOLORDLG:
			return myObj->onControlColorDialog(message, wParam, lParam);
		case WM_SIZE:
			return myObj->onSize(message, wParam, lParam);
		case WM_GETMINMAXINFO:
			return myObj->onMinMaxInfo(message, wParam, lParam);
		break;

	}
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	// get client size on window
//	::GetClientRect(hDlg, &re_max);
	memset(&re_max, 0, sizeof(re_max));

	// set font and translate text
	SetFontAndTranslateText();

	// create tooltip
//	DWORD dwStyle = WS_POPUP
//		| TTS_NOPREFIX; // | TTS_BALLOON | TTS_ALWAYSTIP | TTS_NOANIMATE | TTS_NOFADE;
//	hToolTips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, dwStyle
//		, 0, 0, 0, 0
//		, hDlg, NULL, hInstance, NULL);

	return 0;
}

INT_PTR CDialogBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
		::EndDialog(hDlg, LOWORD(wParam));
		hDlg = NULL;
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onNotify(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)TRUE;
}

INT_PTR CDialogBox::onMouseWheel(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)TRUE;
}

INT_PTR CDialogBox::onHScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onVScroll(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)TRUE;
}

INT_PTR CDialogBox::onClose(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onHelp(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onSize(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onControlColorStatic(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

INT_PTR CDialogBox::onControlColorDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)FALSE;
}

void CDialogBox::SetFontAndTranslateText()
{
	// set font
	SendMessage(hDlg, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
#ifdef USE_GETTEXT
	// translate title string on window
	_TCHAR buf[_MAX_PATH];
	if (GetWindowText(hDlg, buf, sizeof(buf))) {
		SetWindowText(hDlg, _tgettext(buf));
	}
#endif
	// set font and translate text string in child controls
	EnumChildWindows(hDlg, ChildProc, (LPARAM)this);
}

BOOL CALLBACK CDialogBox::ChildProc(HWND hCtrl, LPARAM lParam)
{
	CDialogBox *myObj = (CDialogBox *)lParam;
	// set font
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)myObj->font->GetFont(), MAKELPARAM(TRUE, 0));
#ifdef USE_GETTEXT
	// translate text string
	_TCHAR buf[_MAX_PATH];
	if (GetWindowText(hCtrl, buf, sizeof(buf))) {
		SetWindowText(hCtrl, _tgettext(buf));
	}
#endif
	return TRUE;
}

#if 0
void CDialogBox::AddToolTipText(const t_tooltip_text *texts)
{
	if (!hToolTips) return;

	TOOLINFO ti;

	memset(&ti, 0, sizeof(ti));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hDlg;
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	for(int n = 0; texts[n].text != NULL; n++) {
		ti.uId = (UINT_PTR)GetDlgItem(hDlg, texts[n].id);
		ti.lpszText = (LPTSTR)_tgettext(texts[n].text);
	    SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}
}
#endif

void CDialogBox::Larger(RECT *re)
{
	if (re_max.right < re->right) {
		re_max.right = re->right;
	}
	if (re_max.bottom < re->bottom) {
		re_max.bottom = re->bottom;
	}
}

void CDialogBox::Larger(LONG width, LONG height)
{
	if (re_max.right < width) {
		re_max.right = width;
	}
	if (re_max.bottom < height) {
		re_max.bottom = height;
	}
}

int CDialogBox::FindEmptyID()
{
	int num = -1;
	for(;curStaticId < maxStaticId; curStaticId++) {
		HWND hwnd = GetDlgItem(hDlg, curStaticId);
		if (!hwnd) {
			num = curStaticId;
			break;
		}
	}
	return num;
}

/// @brief Create a control.
/// @param[in] box : layout box object
/// @param[in] class_name : class name
/// @param[in] nItemId : control id
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @param[in] style : style on control
/// @param[in] exstyle : extent style for CreateWindowEx
/// @param[in] scrollstyle : scroll style
/// @return : handle of static text
HWND CDialogBox::CreateControl(CBox *box, const _TCHAR *class_name, int nItemId, int min_w, int min_h, int style, int exstyle, int scrollstyle)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | style;
	HWND hCtrl = CreateWindowEx(exstyle, class_name, _T(""), dwStyle, 0, 0, min_w, min_h, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustControl(box, nItemId, min_w, min_h, scrollstyle);
	}
	return hCtrl;
}

#ifdef USE_LAYOUT_RECT
HWND CDialogBox::CreateStatic(int nItemId, const _TCHAR *label, RECT *re, bool align_right)
{
	if (nItemId < 0) nItemId = FindEmptyID();
	if (nItemId < 0) return (HWND)0;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | SS_LEFT;
	HWND hCtrl = CreateWindowEx(0, WC_STATIC, label, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustStatic(nItemId, re, align_right);
	}
	return hCtrl;
}
#endif

/// @brief Create a static text.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of static text
/// @param[in] label : string
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @param[in] align : align string
/// @param[in] exstyle : extent style for CreateWindowEx
/// @return : handle of static text
HWND CDialogBox::CreateStatic(CBox *box, int nItemId, const _TCHAR *label, int min_w, int min_h, int align, int exstyle)
{
	if (nItemId < 0) nItemId = FindEmptyID();
	if (nItemId < 0) return (HWND)0;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | align;
	HWND hCtrl = CreateWindowEx(exstyle, WC_STATIC, label, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustStatic(box, nItemId, min_w, min_h);
	}
	return hCtrl;
}

/// @brief Create a static text.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of static text
/// @param[in] label : string
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @param[in] align : align string
/// @param[in] exstyle : extent style on CreateWindowEx
/// @return : handle of static text
HWND CDialogBox::CreateStatic(CBox *box, int nItemId, CMsg::Id label, int min_w, int min_h, int align, int exstyle)
{
	if (nItemId < 0) nItemId = FindEmptyID();
	if (nItemId < 0) return (HWND)0;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | align;
	HWND hCtrl = CreateWindowEx(exstyle, WC_STATIC, CMSGVM(label), dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustStatic(box, nItemId, min_w, min_h);
	}
	return hCtrl;
}

/// @brief Create a group box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of group box
/// @param[in] label : label string
/// @param[in] orient : CBox::VerticalBox / CBox::HorizontalBox
/// @param[in] align : text align 
/// @param[out] hCtrl : return handle of group box object 
/// @return : layout box object
CBox *CDialogBox::CreateGroup(CBox *box, int nItemId, const _TCHAR *label, int orient, int align, HWND *hCtrl)
{
	CBox *nbox = NULL;

	if (nItemId < 0) nItemId = FindEmptyID();
	if (nItemId < 0) return nbox;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | SS_LEFT | BS_GROUPBOX;
	HWND hGroup = CreateWindowEx(0, WC_BUTTON, label, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hGroup) {
		SendMessage(hGroup, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		nbox = AdjustGroup(box, nItemId, orient, align);
	}
	if (hCtrl) *hCtrl = hGroup;
	return nbox;
}

/// @brief Create a group box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of group box
/// @param[in] label : label string
/// @param[in] orient : CBox::VerticalBox / CBox::HorizontalBox
/// @param[in] align : text align 
/// @param[out] hCtrl : return handle of group box object 
/// @return : layout box object
CBox *CDialogBox::CreateGroup(CBox *box, int nItemId, const CMsg::Id label, int orient, int align, HWND *hCtrl)
{
	CBox *nbox = NULL;

	if (nItemId < 0) nItemId = FindEmptyID();
	if (nItemId < 0) return nbox;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | SS_LEFT | BS_GROUPBOX;
	HWND hGroup = CreateWindowEx(0, WC_BUTTON, CMSGVM(label), dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hGroup) {
		SendMessage(hGroup, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		nbox = AdjustGroup(box, nItemId, orient, align);
	}
	if (hCtrl) *hCtrl = hGroup;
	return nbox;
}

#ifdef USE_LAYOUT_RECT
HWND CDialogBox::CreateComboBox(int nItemId, const _TCHAR **list, int selnum, int nMinSize, RECT *re, bool align_right, bool translate)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, selnum, translate);
		AdjustComboBox(nItemId, nMinSize, re, align_right);
	}
	return hCtrl;
}
#endif

/// @brief Create a combo box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @param[in] translate : translate strings
/// @return : window handle
HWND CDialogBox::CreateComboBox(CBox *box, int nItemId, const _TCHAR **list, int selnum, int nMinSize, bool translate)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, selnum, translate);
		AdjustComboBox(box, nItemId, nMinSize);
	}
	return hCtrl;
}

/// @brief Create a combo box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @param[in] appendnum : index number of item to append string 
/// @param[in] appendstr : append string
/// @return : window handle
HWND CDialogBox::CreateComboBox(CBox *box, int nItemId, const CMsg::Id *list, int selnum, int nMinSize, int appendnum, CMsg::Id appendstr)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, selnum, appendnum, appendstr);
		AdjustComboBox(box, nItemId, nMinSize);
	}
	return hCtrl;
}

/// @brief Create a combo box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @return : window handle
HWND CDialogBox::CreateComboBox(CBox *box, int nItemId, const CPtrList<CTchar> &list, int selnum, int nMinSize)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, selnum);
		AdjustComboBox(box, nItemId, nMinSize);
	}
	return hCtrl;
}

/// @brief Create a combo box with static label.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] label : label string
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @return : window handle of combo box
HWND CDialogBox::CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const _TCHAR **list, int selnum, int nMinSize)
{
	CreateStatic(box, IDC_STATIC, label);
	HWND hCtrl = CreateComboBox(box, nItemId, list, selnum, nMinSize);
	return hCtrl;
}

/// @brief Create a combo box with static label.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] label : label string
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @return : window handle
HWND CDialogBox::CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const CMsg::Id *list, int selnum, int nMinSize)
{
	CreateStatic(box, IDC_STATIC, label);
	HWND hCtrl = CreateComboBox(box, nItemId, list, selnum, nMinSize);
	return hCtrl;
}

/// @brief Create a combo box with static label.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] label : label string
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] nMinSize : width of combo box
/// @return : window handle
HWND CDialogBox::CreateComboBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const CPtrList<CTchar> &list, int selnum, int nMinSize)
{
	CreateStatic(box, IDC_STATIC, label);
	HWND hCtrl = CreateComboBox(box, nItemId, list, selnum, nMinSize);
	return hCtrl;
}

/// @brief Create a combo text box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] defnum : default index number
/// @param[in] nMinSize : width of combo box
/// @param[in] translate : translate strings
/// @return : window handle
HWND CDialogBox::CreateComboTextBox(CBox *box, int nItemId, const _TCHAR **list, int defnum, int nMinSize, bool translate)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, 0, translate);
		SetDlgItemInt(hDlg, nItemId, defnum, TRUE);
		AdjustComboBox(box, nItemId, nMinSize);
	}
	return hCtrl;
}

/// @brief Create a combo text box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] deftext : default index number
/// @param[in] nMinSize : width of combo box
/// @param[in] translate : translate strings
/// @return : window handle
HWND CDialogBox::CreateComboTextBox(CBox *box, int nItemId, const _TCHAR **list, const _TCHAR *deftext, int nMinSize, bool translate)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS;
	HWND hCtrl = CreateWindow(WC_COMBOBOX, "", dwStyle , 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetComboBoxItems(hCtrl, list, 0, translate);
		SetDlgItemText(hDlg, nItemId, deftext);
		AdjustComboBox(box, nItemId, nMinSize);
	}
	return hCtrl;
}

/// @brief Add a string in itemlist on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] str : item string
/// @param[in] translate : translate strings
void CDialogBox::AddComboBoxItem(int nItemId, const _TCHAR *str, bool translate)
{
	HWND combo = GetDlgItem(hDlg, nItemId);
	int n = (int)SendMessage(combo, CB_GETCOUNT, 0, 0);
	if (translate) str = _tgettext(str);
	SendMessage(combo, CB_ADDSTRING, n, (LPARAM)(LPTSTR)str);
}

/// @brief Set itemlist on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] translate : translate strings
void CDialogBox::SetComboBoxItems(int nItemId, const _TCHAR **list, int selnum, bool translate)
{
	SetComboBoxItems(GetDlgItem(hDlg, nItemId), list, selnum, translate);
}

/// @brief Set itemlist on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] appendnum : index number of item to append string 
/// @param[in] appendstr : append string
void CDialogBox::SetComboBoxItems(int nItemId, const CMsg::Id *list, int selnum, int appendnum, CMsg::Id appendstr)
{
	SetComboBoxItems(GetDlgItem(hDlg, nItemId), list, selnum, appendnum, appendstr);
}

/// @brief Set itemlist on combo box.
/// @param[in] combo : handle of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] translate : translate strings
void CDialogBox::SetComboBoxItems(HWND combo, const _TCHAR **list, int selnum, bool translate)
{
	for(int n=0; list[n] != NULL; n++) {
		const _TCHAR *label = list[n];
		if (translate) label = _tgettext(label);
		SendMessage(combo, CB_ADDSTRING, n, (LPARAM)(LPTSTR)label);
	}
	SendMessage(combo, CB_SETCURSEL, selnum, 0);
}

/// @brief Set itemlist on combo box.
/// @param[in] combo : handle of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
/// @param[in] appendnum : index number of item to append string 
/// @param[in] appendstr : append string
void CDialogBox::SetComboBoxItems(HWND combo, const CMsg::Id *list, int selnum, int appendnum, CMsg::Id appendstr)
{
	for(int n=0; list[n] != CMsg::Null && list[n] != CMsg::End; n++) {
		if (n == appendnum) {
			_TCHAR label[_MAX_PATH];
			UTILITY::tcscpy(label, _MAX_PATH, CMSGVM(list[n]));
			UTILITY::tcscat(label, _MAX_PATH, CMSGVM(appendstr));
			SendMessage(combo, CB_ADDSTRING, n, (LPARAM)(LPTSTR)label);
		} else {
			const _TCHAR *label = CMSGVM(list[n]);
			SendMessage(combo, CB_ADDSTRING, n, (LPARAM)(LPTSTR)label);
		}
	}
	SendMessage(combo, CB_SETCURSEL, selnum, 0);
}

/// @brief Set itemlist on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
void CDialogBox::SetComboBoxItems(int nItemId, const CPtrList<CTchar> &list, int selnum)
{
	SetComboBoxItems(GetDlgItem(hDlg, nItemId), list, selnum);
}

/// @brief Set itemlist on combo box.
/// @param[in] combo : handle of combo box
/// @param[in] list : item string list
/// @param[in] selnum : select index number
void CDialogBox::SetComboBoxItems(HWND combo, const CPtrList<CTchar> &list, int selnum)
{
	for(int n=0; n<list.Count(); n++) {
		const _TCHAR *label = list.Item(n)->Get();
		SendMessage(combo, CB_ADDSTRING, n, (LPARAM)(LPTSTR)label);
	}
	SendMessage(combo, CB_SETCURSEL, selnum, 0);
}

/// @brief Select item on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] selnum : select index number
void CDialogBox::SelectComboBoxItem(int nItemId, int selnum)
{
	HWND combo = GetDlgItem(hDlg, nItemId);
	SendMessage(combo, CB_SETCURSEL, selnum, 0);
}

/// @brief Replace or append string in selected item on combo box.
/// @param[in] nItemId : control id of combo box
/// @param[in] selnum : select index number
/// @param[in] id : replace or append string 
/// @param[in] append_text : true if append string
void CDialogBox::ReplaceComboBoxItem(int nItemId, int selnum, const CMsg::Id id, bool append_text)
{
	HWND combo = GetDlgItem(hDlg, nItemId);
	ReplaceComboBoxItem(combo, selnum, id, append_text);
}

/// @brief Replace or append string in selected item on combo box.
/// @param[in] combo : handle of combo box
/// @param[in] selnum : select index number
/// @param[in] id : replace or append string 
/// @param[in] append_text : true if append string
void CDialogBox::ReplaceComboBoxItem(HWND combo, int selnum, const CMsg::Id id, bool append_text)
{
	_TCHAR label[_MAX_PATH];
	label[0] = _T('\0');
	if (append_text) {
		int len = (int)SendMessage(combo, CB_GETLBTEXTLEN, selnum, 0);
		if (len < _MAX_PATH - 1) {
			SendMessage(combo, CB_GETLBTEXT, selnum, (LPARAM)(LPTSTR)label);
		}
	}
	UTILITY::tcscat(label, _MAX_PATH, CMSGVM(id));
	int cursel = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);
	SendMessage(combo, CB_DELETESTRING, selnum, 0);
	SendMessage(combo, CB_INSERTSTRING, selnum, (LPARAM)(LPTSTR)label);
	if (cursel != CB_ERR) {
		SendMessage(combo, CB_SETCURSEL, cursel, 0);
	}
}

#ifdef USE_LAYOUT_RECT
HWND CDialogBox::CreateDefaultButton(int nItemId, const _TCHAR *caption, int nSize, RECT *re, bool align_right)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON;
	HWND hCtrl = CreateWindowEx(0, WC_BUTTON, caption, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustButton(nItemId, nSize, re, align_right);
	}
	return hCtrl;
}

HWND CDialogBox::CreateButton(int nItemId, const _TCHAR *caption, int nSize, RECT *re, bool align_right)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
	HWND hCtrl = CreateWindowEx(0, WC_BUTTON, caption, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustButton(nItemId, nSize, re, align_right);
	}
	return hCtrl;
}
#endif

/// @brief Create a button.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of button
/// @param[in] caption : string on button
/// @param[in] nSize : width
/// @param[in] default_button : default selected button
/// @return : handle of button
HWND CDialogBox::CreateButton(CBox *box, int nItemId, const _TCHAR *caption, int nSize, bool default_button)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	if (default_button) dwStyle |= BS_DEFPUSHBUTTON;
	else dwStyle |= BS_PUSHBUTTON;
	HWND hCtrl = CreateWindowEx(0, WC_BUTTON, caption, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustButton(box, nItemId, nSize);
	}
	return hCtrl;
}

/// @brief Create a button.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of button
/// @param[in] caption : string on button
/// @param[in] nSize : width
/// @param[in] default_button : default selected button
/// @return : handle of button
HWND CDialogBox::CreateButton(CBox *box, int nItemId, CMsg::Id caption, int nSize, bool default_button)
{
	return CreateButton(box, nItemId, CMSGVM(caption), nSize, default_button);
}

/// @brief Create a check box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of check box
/// @param[in] caption : string on check box
/// @param[in] value : on/off value on check box
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @return : handle of check box
HWND CDialogBox::CreateCheckBox(CBox *box, int nItemId, const _TCHAR *caption, bool value, int min_w, int min_h)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;
	HWND hCtrl = CreateWindowEx(0, WC_BUTTON, caption, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustCheckBox(box, nItemId, min_w, min_h);
//		CheckDlgButton(hDlg, nItemId, value ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hCtrl, BM_SETCHECK, (WPARAM)(value ? BST_CHECKED : BST_UNCHECKED), 0L);
	}
	return hCtrl;
}

/// @brief Create a check box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of check box
/// @param[in] caption : string on check box
/// @param[in] value : on/off value on check box
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @return : handle of check box
HWND CDialogBox::CreateCheckBox(CBox *box, int nItemId, CMsg::Id caption, bool value, int min_w, int min_h)
{
	return CreateCheckBox(box, nItemId, CMSGVM(caption), value, min_w, min_h);
}

/// @brief Create a radio button.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of radio button
/// @param[in] caption : string on radio button
/// @param[in] first : first item on radio box
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @return : handle of radio button
HWND CDialogBox::CreateRadioButton(CBox *box, int nItemId, const _TCHAR *caption, bool first, int min_w, int min_h)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON;
	if (first) dwStyle |= WS_GROUP;
	HWND hCtrl = CreateWindowEx(0, WC_BUTTON, caption, dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustCheckBox(box, nItemId, min_w, min_h);
	}
	return hCtrl;
}

/// @brief Create a radio button.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of radio button
/// @param[in] caption : string on radio button
/// @param[in] first : first item on radio box
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @return : handle of radio button
HWND CDialogBox::CreateRadioButton(CBox *box, int nItemId, CMsg::Id caption, bool first, int min_w, int min_h)
{
	return CreateRadioButton(box, nItemId, CMSGVM(caption), first, min_w, min_h);
}

/// @brief Create a edit box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of edit box
/// @param[in] text : default string on edit box
/// @param[in] nMaxSize : width of edit box
/// @param[in] align : align string
/// @param[in] ch : a char using size hint of width on edit box 
/// @return : handle of edit box
HWND CDialogBox::CreateEditBox(CBox *box, int nItemId, const _TCHAR *text, int nMaxSize, int align, const _TCHAR ch)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
	align |= WS_EX_CLIENTEDGE;
	HWND hCtrl = CreateWindowEx(align, WC_EDIT, _T(""), dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetDlgItemText(hDlg, nItemId, text);
		AdjustEditBox(box, nItemId, nMaxSize, ch);
	}
	return hCtrl;
}

/// @brief Create a edit box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of edit box
/// @param[in] digit : default string on edit box
/// @param[in] nMaxSize : width of edit box
/// @param[in] align : align string
/// @param[in] ch : a char using size hint of width on edit box 
/// @return : handle of edit box
HWND CDialogBox::CreateEditBox(CBox *box, int nItemId, int digit, int nMaxSize, int align, const _TCHAR ch)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
	align |= WS_EX_CLIENTEDGE;
	HWND hCtrl = CreateWindowEx(align, WC_EDIT, _T(""), dwStyle, 0, 0, 10, 10, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SetDlgItemInt(hDlg, nItemId, digit, TRUE);
		AdjustEditBox(box, nItemId, nMaxSize, ch);
	}
	return hCtrl;
}

/// @brief Create a edit box with static label.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of edit box
/// @param[in] label : label string
/// @param[in] text : default string on edit box
/// @param[in] nMaxSize : width of edit box
/// @param[in] align : align string
/// @param[in] ch : a char using size hint of width on edit box 
/// @return : handle of edit box
HWND CDialogBox::CreateEditBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, const _TCHAR *text, int nMaxSize, int align, const _TCHAR ch)
{
	CreateStatic(box, IDC_STATIC, label);
	HWND hCtrl = CreateEditBox(box, nItemId, text, nMaxSize, align, ch);
	return hCtrl;
}

/// @brief Create a edit box with static label.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of edit box
/// @param[in] label : label string
/// @param[in] digit : default string on edit box
/// @param[in] nMaxSize : width of edit box
/// @param[in] align : align string
/// @param[in] ch : a char using size hint of width on edit box 
/// @return : handle of edit box
HWND CDialogBox::CreateEditBoxWithLabel(CBox *box, int nItemId, CMsg::Id label, int digit, int nMaxSize, int align, const _TCHAR ch)
{
	CreateStatic(box, IDC_STATIC, label);
	HWND hCtrl = CreateEditBox(box, nItemId, digit, nMaxSize, align, ch);
	return hCtrl;
}

/// @brief Create a text control.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of radio button
/// @param[in] multi_line : control has multi line?
/// @param[in] read_only : control is read only?
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @return : handle of control
HWND CDialogBox::CreateTextControl(CBox *box, int nItemId, bool multi_line, bool read_only, int min_w, int min_h)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
	DWORD align = WS_EX_CLIENTEDGE;
	if (multi_line) {
		dwStyle |= (ES_MULTILINE | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL);
	}
	if (read_only) {
		dwStyle |= ES_READONLY;
	}
	HWND hCtrl = CreateWindowEx(align, WC_EDIT, _T(""), dwStyle, 0, 0, min_w, min_h, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		AdjustControl(box, nItemId, min_w, min_h);
	}
	return hCtrl;
}

/// @brief Create a slider.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of slider
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
/// @param[in] range_min : minimum value
/// @param[in] range_max : maximum value
/// @param[in] ticks : ticks of range
/// @param[in] value : default value 
/// @param[in] vertical : true if vertical slider  
/// @return : handle of slider
HWND CDialogBox::CreateSlider(CBox *box, int nItemId, int min_w, int min_h, int range_min, int range_max, int ticks, int value, bool vertical)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_AUTOTICKS | TBS_BOTH;
	if (vertical) dwStyle |= TBS_VERT;
	else dwStyle |= TBS_HORZ;
	HWND hCtrl = CreateWindowEx(0, TRACKBAR_CLASS, _T(""), dwStyle, 0, 0, min_w, min_h, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SendMessage(hCtrl, TBM_SETRANGE, TRUE, MAKELPARAM(range_min, range_max));
		SendMessage(hCtrl, TBM_SETTICFREQ, ticks, 0);
		SendMessage(hCtrl, TBM_SETPOS, TRUE, value);
		AdjustControl(box, nItemId, min_w, min_h);
	}
	return hCtrl;
}

/// @brief Create a updown button.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of slider
/// @param[in] hEdit : handle of edit box
/// @param[in] range_min : minimum value
/// @param[in] range_max : maximum value
/// @param[in] value : default value 
/// @return : handle of updown button
HWND CDialogBox::CreateUpDown(CBox *box, int nItemId, HWND hEdit, int range_min, int range_max, int value)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | UDS_ALIGNRIGHT | UDS_SETBUDDYINT;
	HWND hCtrl = CreateWindowEx(0, UPDOWN_CLASS, _T(""), dwStyle, 0, 0, 16, 16, hDlg, (HMENU)(INT_PTR)nItemId, hInstance, NULL);
	if (hCtrl) {
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)font->GetFont(), MAKELPARAM(TRUE, 0));
		SendMessage(hCtrl, UDM_SETBUDDY, (WPARAM)hEdit, 0);
		SendMessage(hCtrl, UDM_SETRANGE, 0, MAKELPARAM(range_max, range_min));
		SendMessage(hCtrl, UDM_SETPOS, 0, value);

		SIZE siz = { 0, 0 };
		const _TCHAR *buf = _T("0");

		// edit box
		font->GetTextSize(hDlg, buf, &siz);

		AdjustControl(box, nItemId, padding + siz.cx * 2 + padding, siz.cy + padding * 4);
	}
	return hCtrl;
}

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustButton(int nItemId, int nSize, RECT *re, bool align_right)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	if (!GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		UTILITY::tcscpy(buf, _MAX_PATH, _T("0"));
	}
	font->GetTextSize(hDlg, buf, &siz);

	siz.cx += (siz.cy * 2);
	if (siz.cx < siz.cy * nSize) {
		siz.cx = siz.cy * nSize;
	}
	if (align_right) {
		re->left = re->right - siz.cx;
	} else {
		re->right = siz.cx + re->left;
	}
	re->bottom = siz.cy + padding * 4 + re->top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, re->top, re->right - re->left, re->bottom - re->top, TRUE);

	Larger(re);
}
#endif

/// @brief Adjust position and size of a button control.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of button
/// @param[in] nSize : minimum size
void CDialogBox::AdjustButton(CBox *box, int nItemId, int nSize)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	if (!GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		UTILITY::tcscpy(buf, _MAX_PATH, _T("0"));
	}
	font->GetTextSize(hDlg, buf, &siz);

	siz.cx += (siz.cy * 2);
	if (siz.cx < siz.cy * nSize) {
		siz.cx = siz.cy * nSize;
	}
	box->AddControl(nItemId, siz.cx, siz.cy + padding * 4);
}


#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustStatic(int nItemId, RECT *re, bool align_right)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// static text
	if (GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		font->GetTextSize(hDlg, buf, &siz);
	}
	if (align_right) {
		re->left = re->right - siz.cx;
	} else {
		re->right = siz.cx + re->left;
	}
	LONG top = re->top + padding;
	re->bottom = siz.cy + padding * 2 + top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, top, re->right - re->left, re->bottom - top, TRUE);
	re->bottom += padding;

	Larger(re);
}
#endif

/// @brief Adjust position and size of a static text control.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of static text
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
void CDialogBox::AdjustStatic(CBox *box, int nItemId, int min_w, int min_h)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// static text
	if (GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		font->GetTextSize(hDlg, buf, &siz);
	}
	if (siz.cx < min_w) siz.cx = min_w;
	if (siz.cy < min_h) siz.cy = min_h;
	box->AddControl(nItemId, siz.cx, siz.cy +  padding * 4, 0, padding * 2);
}

/// @brief Adjust position and size of a group box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of group box
/// @param[in] orient : CBox::VerticalBox / CBox::HorizontalBox
/// @param[in] align : text align
/// @return : layout box object
CBox *CDialogBox::AdjustGroup(CBox *box, int nItemId, int orient, int align)
{
	_TCHAR buf[_MAX_PATH];
	int h = font->GetHeight();

	GetDlgItemText(hDlg, nItemId, buf, sizeof(buf));

	CBox *cbox = new CBox(orient, align, margin, buf);
	cbox->SetTopMargin(h + padding + 4);
	box->AddControlWithBox(cbox, nItemId);

	return cbox;
}

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustCheckBox(int nItemId, RECT *re, bool align_right)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// checkbox
	if (GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		font->GetTextSize(hDlg, buf, &siz);
	}
	if (align_right) {
		re->left = re->right - siz.cx - 20;
	} else {
		re->right = siz.cx + 20 + re->left;
	}
	LONG top = re->top + padding;
	re->bottom = siz.cy + padding * 3 + top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, top, re->right - re->left, re->bottom - top, TRUE);

	Larger(re);
}
#endif

/// @brief Adjust position and size of a check box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of check box
/// @param[in] min_w : minimum width
/// @param[in] min_h : minimum height
void CDialogBox::AdjustCheckBox(CBox *box, int nItemId, int min_w, int min_h)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// checkbox
	if (GetDlgItemText(hDlg, nItemId, buf, sizeof(buf))) {
		font->GetTextSize(hDlg, buf, &siz);
	}
	if (siz.cx < min_w) siz.cx = min_w;
	if (siz.cy < min_h) siz.cy = min_h;
	box->AddControl(nItemId, siz.cx + 20, siz.cy + padding * 4, 0, padding);
}

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustEditBox(int nItemId, int nMaxSize, RECT *re, bool align_right, const _TCHAR ch)
{
	_TCHAR buf[2] = { ch, _T('\0') };
	SIZE siz = { 0, 0 };

	// edit box
	font->GetTextSize(hDlg, buf, &siz);

	if (align_right) {
		re->left = re->right - padding - siz.cx * nMaxSize - padding;
	} else {
		re->right = padding + siz.cx * nMaxSize + padding + re->left;
	}
	re->bottom = siz.cy + padding * 4 + re->top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, re->top, re->right - re->left, re->bottom - re->top, TRUE);

	Larger(re);
}

void CDialogBox::AdjustEditBox(int nItemId, RECT *re, const _TCHAR *cLenStr, bool align_right)
{
	SIZE siz = { 0, 0 };

	// edit box
	font->GetTextSize(hDlg, cLenStr, &siz);

	if (align_right) {
		re->left = re->right - padding - siz.cx - padding;
	} else {
		re->right = padding + siz.cx + padding + re->left;
	}
	re->bottom = siz.cy + padding * 3 + re->top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, re->top, re->right - re->left, re->bottom - re->top, TRUE);

	Larger(re);
}
#endif

/// @brief Adjust position and size of a edit box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of edit box
/// @param[in] nMaxSize : width of edit box
/// @param[in] ch : a char using size hint of width on edit box 
void CDialogBox::AdjustEditBox(CBox *box, int nItemId, int nMaxSize, const _TCHAR ch)
{
	_TCHAR buf[2] = { ch, _T('\0') };
	SIZE siz = { 0, 0 };

	// edit box
	font->GetTextSize(hDlg, buf, &siz);

	box->AddControl(nItemId,  padding + siz.cx * nMaxSize + padding, siz.cy + padding * 4);
}

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustEditBoxWithStatic(int nStaticItemId, int nEditItemId, int nEditMaxSize, RECT *re, bool align_right)
{
	LONG left = re->left;
	LONG right = re->right;

	// static text
	AdjustStatic(nStaticItemId, re, align_right);
	// edit box
	if (align_right) {
		re->right = re->left - padding;
	} else {
		re->left = re->right + padding;
	}
	AdjustEditBox(nEditItemId, nEditMaxSize, re, align_right);
	if (align_right) {
		re->right = right;
	} else {
		re->left = left;
	}
}
#endif

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustComboBox(int nItemId, int nMinSize, RECT *re, bool align_right)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// combo box
	int count = (int)SendDlgItemMessage(hDlg, nItemId, CB_GETCOUNT, 0, 0);
	int max_len = 0;
	for(int n = 0; n < count; n++) {
		int len = (int)SendDlgItemMessage(hDlg, nItemId, CB_GETLBTEXTLEN, (WPARAM)n, 0);
		if (len > max_len) {
			SendDlgItemMessage(hDlg, nItemId, CB_GETLBTEXT, (WPARAM)n, (LPARAM)buf);
			max_len = len;
		}
	}
	if (max_len == 0) {
		UTILITY::tcscpy(buf, _MAX_PATH, _T("m"));
	}
	font->GetTextSize(hDlg, buf, &siz);

	if (siz.cx < siz.cy * nMinSize) {
		siz.cx = siz.cy * nMinSize;
	}
	if (align_right) {
		re->left = re->right - padding - siz.cx - padding - siz.cy - padding;
	} else {
		re->right = padding + siz.cx + padding + siz.cy + padding + re->left;
	}
	re->bottom = siz.cy + padding * 4 + re->top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, re->top, re->right - re->left, re->bottom - re->top, TRUE);

	Larger(re);
}
#endif

/// @brief Adjust position and size of a combo box.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id of combo box
/// @param[in] nMinSize : width of combo box
void CDialogBox::AdjustComboBox(CBox *box, int nItemId, int nMinSize)
{
	_TCHAR buf[_MAX_PATH];
	SIZE siz = { 0, 0 };

	// combo box
	int count = (int)SendDlgItemMessage(hDlg, nItemId, CB_GETCOUNT, 0, 0);
	int max_len = 0;
	for(int n = 0; n < count; n++) {
		int len = (int)SendDlgItemMessage(hDlg, nItemId, CB_GETLBTEXTLEN, (WPARAM)n, 0);
		if (len > max_len) {
			SendDlgItemMessage(hDlg, nItemId, CB_GETLBTEXT, (WPARAM)n, (LPARAM)buf);
			max_len = len;
		}
	}
	if (max_len == 0) {
		UTILITY::tcscpy(buf, _MAX_PATH, _T("m"));
	}
	font->GetTextSize(hDlg, buf, &siz);

	if (siz.cx < siz.cy * nMinSize) {
		siz.cx = siz.cy * nMinSize;
	}
	box->AddControl(nItemId, padding + siz.cx + padding + siz.cy + padding, siz.cy + padding * 4);
}

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustComboBoxWithStatic(int nStaticItemId, int nComboItemId, int nComboMinSize, RECT *re, bool align_right)
{
	LONG left = re->left;
	LONG right = re->right;

	// static text
	AdjustStatic(nStaticItemId, re, align_right);
	// combo box
	if (align_right) {
		re->right = re->left - padding;
	} else {
		re->left = re->right + padding;
	}
	AdjustComboBox(nComboItemId, nComboMinSize, re, align_right);
	if (align_right) {
		re->right = right;
	} else {
		re->left = left;
	}
}
#endif

#ifdef USE_LAYOUT_RECT
void CDialogBox::AdjustControl(int nItemId, RECT *re, int w, int h, bool align_right, int style)
{
	if (style != 0) {
		int val = GetSystemMetrics(style);
		switch(style) {
		case SM_CXVSCROLL:
			w += val;
			break;
		case SM_CYHSCROLL:
			h += val;
			break;
		}
	}
	if (align_right) {
		re->left = re->right - w;
	} else {
		re->right = w + re->left;
	}
	re->bottom = h + re->top;
	MoveWindow(GetDlgItem(hDlg, nItemId), re->left, re->top, re->right - re->left, re->bottom - re->top, TRUE);

	Larger(re);
}
#endif

/// @brief Adjust position and size of a control.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id
/// @param[in] w : minimum width
/// @param[in] h : minimum height
/// @param[in] style : style on control
void CDialogBox::AdjustControl(CBox *box, int nItemId, int w, int h, int style)
{
	if (style != 0) {
		int val = GetSystemMetrics(style);
		switch(style) {
		case SM_CXVSCROLL:
			w += val;
			break;
		case SM_CYHSCROLL:
			h += val;
			break;
		}
	}
	if (box) {
		box->AddControl(nItemId, w, h);
	}
}

/// @brief Adjust position and size of a tab control.
/// @param[in] box : layout box object
/// @param[in] nItemId : control id
/// @param[in] nBGItemId : control id of back ground
CBox *CDialogBox::AdjustTabControl(CBox *box, int nItemId, int nBGItemId)
{
	int h = font->GetHeight();

	CBox *cbox = new CBox(CBox::TabControlBox, 0, margin, _T("tab"));
	cbox->SetTopMargin(h + margin + margin);
	box->AddControlWithBox(cbox, nItemId);
	cbox->AddControl(nBGItemId, 1, 1, 1, 1);

	return cbox;
}

/// @brief Fit size of a window.
/// @param[in] lmargin : margin
void CDialogBox::AdjustWindow(int lmargin)
{
	WINDOWINFO wi;
	GetWindowInfo(hDlg, &wi);

//	RECT scn_re;
//	GetWindowRect(GetDesktopWindow(), &scn_re);
	re_max.right = re_max.right + lmargin;
	re_max.bottom = re_max.bottom + lmargin;

	RECT win_re;
	win_re.left = 0;
	win_re.top = 0;
	win_re.right = re_max.right;
	win_re.bottom = re_max.bottom;
	// get window size included border size
	AdjustWindowRectEx(&win_re, wi.dwStyle, FALSE, wi.dwExStyle);
	// resize window
	win_re.right = win_re.right - win_re.left;	// width
	win_re.bottom = win_re.bottom - win_re.top;	// height
//	win_re.left = (scn_re.right - win_re.right) / 2;
//	win_re.top  = (scn_re.bottom - win_re.bottom) / 2;
	win_re.left = wi.rcWindow.left;	// position on screen
	win_re.top  = wi.rcWindow.top;	// position on screen
	MoveWindow(hDlg, win_re.left, win_re.top, win_re.right, win_re.bottom, TRUE);

}

/// @brief Resize a control.
/// @param[in] nItemId : control id
/// @param[in] w : width
/// @param[in] h : height
void CDialogBox::ResizeControl(int nItemId, int w, int h)
{
	HWND hCtrl;
	if (nItemId > 0) {
		hCtrl = GetDlgItem(hDlg, nItemId);
	} else {
		hCtrl = hDlg;
	}
	if (hCtrl == NULL) return;
	SetWindowPos(hCtrl, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
}

/// @brief Adjust size of a control.
/// @param[in] nItemId : control id
/// @param[in] diff_x : difference of width
/// @param[in] diff_y : difference of height
void CDialogBox::AdjustControlSize(int nItemId, int diff_x, int diff_y)
{
	HWND hCtrl;
	if (nItemId > 0) {
		hCtrl = GetDlgItem(hDlg, nItemId);
	} else {
		hCtrl = hDlg;
	}
	if (hCtrl == NULL) return;
	WINDOWINFO wi;
	GetWindowInfo(hCtrl, &wi);
	int width = wi.rcWindow.right - wi.rcWindow.left + diff_x;
	int height = wi.rcWindow.bottom - wi.rcWindow.top + diff_y;
	SetWindowPos(hCtrl, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

	if (nItemId > 0) {
		RECT re;
		GetControlPos(nItemId, &re);
		Larger(&re);
	}
}

/// @brief Adjust position of a control.
/// @param[in] nItemId : control id
/// @param[in] diff_x : difference of x
/// @param[in] diff_y : difference of y
void CDialogBox::AdjustControlPos(int nItemId, int diff_x, int diff_y)
{
	HWND hCtrl = GetDlgItem(hDlg, nItemId);
	if (hCtrl == NULL) return;
	RECT re;
	GetControlPos(nItemId, &re);
	re.left += diff_x;
	re.top += diff_y;
	SetWindowPos(hCtrl, NULL, re.left, re.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

/// @brief Calcurate difference of with on a control.
/// @param[in] nItemId : control id
/// @param[in] new_width : width
/// @return difference of width
int CDialogBox::GetControlXdiff(int nItemId, int new_width)
{
	WINDOWINFO wi;
	HWND hCtrl = GetDlgItem(hDlg, nItemId);
	GetWindowInfo(hCtrl, &wi);
	return new_width - wi.rcClient.right + wi.rcClient.left;
}

/// @brief Get relative position of a control on a window
/// @param[in] nItemId : control id
/// @param[in] re : rectangle
void CDialogBox::GetControlPos(int nItemId, RECT *re)
{
	HWND hCtrl = GetDlgItem(hDlg, nItemId);
	if (hCtrl == NULL) return;
	WINDOWINFO wiDlg;
	GetWindowInfo(hDlg, &wiDlg);
	WINDOWINFO wi;
	GetWindowInfo(hCtrl, &wi);
	if (re != NULL) {
		re->left = wi.rcWindow.left - wiDlg.rcClient.left;
		re->top =  wi.rcWindow.top  - wiDlg.rcClient.top;
		re->right  = wi.rcWindow.right - wiDlg.rcClient.left;
		re->bottom = wi.rcWindow.bottom  - wiDlg.rcClient.top;
	}
}

/// @brief Get size of a control
/// @param[in] nItemId : control id
/// @param[in] size : size
void CDialogBox::GetControlSize(int nItemId, SIZE *size)
{
	HWND hCtrl = GetDlgItem(hDlg, nItemId);
	if (hCtrl == NULL) return;
	RECT re;
	::GetClientRect(hCtrl, &re);
	if (size != NULL) {
		size->cx = re.right - re.left;
		size->cy = re.bottom - re.top;
	}
}

void CDialogBox::EnableDlgItem(int nItemId, bool value)
{
	::EnableWindow(::GetDlgItem(hDlg, nItemId), value);
}

void CDialogBox::ShowDlgItem(int nItemId, bool value)
{
	::ShowWindow(::GetDlgItem(hDlg, nItemId), value);
}

//
// Layout for widget controls
//

#undef _DEBUG_CBOX

/// @brief layout box
/// @param[in] orient : CBox::VerticalBox, CBox::HorizontalBox, CBox::TabControlBox
/// @param[in] align : CBox::LeftPos,  CBox::CenterPos,  CBox::RightPos,  CBox::TopPos,  CBox::MiddlePos,  CBox::BottomPos
/// @param[in] margin : margin (px)
/// @param[in] name : unique name (optional)
CBox::CBox(int orient, int align, int margin, const _TCHAR *name)
{
	this->orient = (enOrient)orient;
	this->align = align;
	this->margin.left = margin;
	this->margin.top = margin;
	this->margin.right = margin;
	this->margin.bottom = margin;
	this->re.x = 0;
	this->re.y = 0;
	this->re.w = 0;
	this->re.h = 0;

	padding   = 2;

	parent_box = NULL;

	control_nums = 0;
	controls = NULL;

	realized = false;

	memset(this->name, 0, sizeof(this->name));
	if (name) {
		UTILITY::tcsncpy(this->name, sizeof(this->name)/sizeof(_TCHAR), name, 9);
	}

	tab_items = NULL;
}

CBox::~CBox()
{
	if (controls) {
		struct stControls *item = controls;
		while(item != NULL) {
			struct stControls *next = item->next;
			delete item->box;
			delete item;
			item = next;
		}
	}
	controls = NULL;
	control_nums = 0;
}

void CBox::AddWidth(int width)
{
	if (orient == HorizontalBox) {
		re.w += width;
	} else {
		if (width > re.w) {
			re.w = width;
		}
	}
}

void CBox::AddHeight(int height)
{
	if (orient == HorizontalBox) {
		if (height > re.h) {
			re.h = height;
		}
	} else {
		re.h += height;
	}
}

/// @brief Add a layout box
/// @param[in] box : a layout box
void CBox::AddBox(CBox *box)
{
	AddItem(box, 0, 0, 0, 0, 0);
}

/// @brief Create a new layout box
/// @param[in] orient : CBox::VerticalBox, CBox::HorizontalBox, CBox::TabControlBox
/// @param[in] align : CBox::LeftPos, CBox::CenterPos, CBox::RightPos, CBox::TopPos, CBox::MiddlePos, CBox::BottomPos
/// @param[in] margin : margin (px)
/// @param[in] name : unique name (optional)
/// @return new layout box object
CBox *CBox::AddBox(int orient, int align, int margin, const _TCHAR *name)
{
	CBox *box = new CBox(orient, align, margin, name);
	this->AddBox(box);
	return box;
}

/// @brief Add a control
/// @param[in] itemid : a control id
/// @param[in] width
/// @param[in] height
/// @param[in] px
/// @param[in] py
void CBox::AddControl(int itemid, int width, int height, int px, int py)
{
	AddItem(NULL, itemid, width, height, px, py);
}

/// @brief Add a box and control
/// @param[in] box : a layout box
/// @param[in] itemid : a control id
void CBox::AddControlWithBox(CBox *box, int itemid)
{
	AddItem(box, itemid, 0, 0, 0, 0);
}

/// @brief Add a spacer
/// @param[in] width
/// @param[in] height
/// @param[in] px
/// @param[in] py
void CBox::AddSpace(int width, int height, int px, int py)
{
	AddItem(NULL, 0, width, height, px, py);
}

/// @brief Add a box or control
/// @param[in] box : a layout box
/// @param[in] itemid : a control id
/// @param[in] width
/// @param[in] height
/// @param[in] px
/// @param[in] py
void CBox::AddItem(CBox *box, int itemid, int width, int height, int px, int py)
{
	if (box) box->parent_box = this;

	struct stControls *newitem = new struct stControls;
	newitem->box = box;
	newitem->itemid = itemid;
	newitem->x = -1;
	newitem->y = -1;
	newitem->px = px;
	newitem->py = py;
	newitem->w = width;
	newitem->h = height;
	newitem->next = NULL;

	if (controls) {
		// next item
		struct stControls *item = controls;
		while (item->next != NULL) {
			item = item->next;
		}
		item->next = newitem;
	} else {
		// first item
		controls = newitem;
	}
	control_nums++;

	if (itemid > 0) AddIdToTabItems(itemid);
}

/// @brief Adjust position and size of items in a dialog
/// @param[in] dlg : a dialog
void CBox::Realize(CDialogBox &dlg)
{
	if (realized) return;

	RealizeReal(dlg);
	MoveItems(NULL, dlg);
	dlg.Larger((LONG)re.w, (LONG)re.h);
	dlg.AdjustWindow(0);
}

void CBox::RealizeReal(CDialogBox &dlg)
{
	if (realized) return;

#ifdef _DEBUG_CBOX
	EMU *emu = dlg.GetEmu();
#endif

	// add left top margin
	re.w += margin.left;
	re.h += margin.top;

	int mw,mh;
	int nums = 0;
	struct stControls *item = controls;
	while(item != NULL) {
		if (item->box) {
			// box
			switch(orient) {
			case VerticalBox:
				if (nums > 0) {
					re.h += padding;
				}
				item->box->AdjustPosition(re.x + margin.left, re.y + re.h);
				item->x = margin.left;
				item->y = re.h;
				break;
			case HorizontalBox:
				if (nums > 0) {
					re.w += padding;
				}
				item->box->AdjustPosition(re.x + re.w, re.y + margin.top);
				item->x = re.w;
				item->y = margin.top;
				break;
			case TabControlBox:
				item->box->AdjustPosition(re.x + margin.left, re.y + margin.top);
				item->x = margin.left;
				item->y = margin.top;
				break;
			}

			item->box->RealizeReal(dlg);

			item->w = item->box->GetWidthWithMargin();
			item->h = item->box->GetHeightWithMargin();

			switch(orient) {
			case VerticalBox:
				re.h += item->h;
				mw = margin.left + item->w;
				if (re.w < mw) {
					re.w = mw;
				}
				break;
			case HorizontalBox:
				re.w += item->w;
				mh = margin.top + item->h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case TabControlBox:
				mw = margin.left + item->w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item->h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			}

#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s B%d x:%d y:%d w:%d h:%d"), name, nums, item->x, item->y, item->w, item->h);
#endif

		} else {
			// control or space
			switch(orient) {
			case VerticalBox:
				if (nums > 0) {
					re.h += padding;
				}
				item->x = margin.left;
				item->y = re.h;
				re.h += item->h;
				mw = margin.left + item->w;
				if (re.w < mw) {
					re.w = mw;
				}
				break;
			case HorizontalBox:
				if (nums > 0) {
					re.w += padding;
				}
				item->x = re.w;
				item->y = margin.top;
				re.w += item->w;
				mh = margin.top + item->h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case TabControlBox:
				item->x = margin.left;
				item->y = margin.top;
				mw = margin.left + item->w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item->h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			}
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s C%d x:%d y:%d w:%d h:%d"), name, nums, item->x, item->y, item->w, item->h);
#endif
		}
		item = item->next;
		nums++;
	}

	// add right bottom margin
	re.w += margin.right;
	re.h += margin.bottom;

#ifdef _DEBUG_CBOX
	logging->out_debugf(_T("Realize: %-8s re x:%d y:%d w:%d h:%d"), name, re.x, re.y, re.w, re.h);
#endif

	realized = true;
}

void CBox::AdjustPosition(int x, int y)
{
	re.x = x;
	re.y = y;
}

void CBox::MoveItems(CBox *parent, CDialogBox &dlg)
{
	if (!realized) return;

#ifdef _DEBUG_CBOX
	EMU *emu = dlg.GetEmu();
#endif

	if (parent) {
		switch(orient) {
		case VerticalBox:
			if ((align & 0xf0) == BottomPos) {
				re.y = parent->GetHeightWithTopMargin() - re.h;
			}
			break;
		case HorizontalBox:
			if ((align & 0x0f) == RightPos) {
				re.x = parent->GetWidthWithLeftMargin() - re.w;
			}
			break;
		case TabControlBox:
			controls->w = re.w - 6;
			controls->h = re.h - margin.top + 4;
			controls->x = 2;
			controls->y = margin.top - 8;
			break;
		}
	}

	int nums = 0;
	struct stControls *item = controls;
	while(item != NULL) {
		if (item->box) {
			item->box->MoveItems(this, dlg);
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("MoveItems: %-8s B%d x:%d y:%d w:%d h:%d"), name, nums, item->x, item->y, item->w, item->h);
#endif
		}
		if (item->itemid > 0) {

			switch((int)orient) {
			case VerticalBox:
				if ((align & 0x0f) == CenterPos) {
					item->x = (re.w - item->w) / 2;
				}
				break;
			case HorizontalBox:
				if ((align & 0xf0) == MiddlePos) {
					item->y = (re.h - item->h) / 2;
				}
				break;
			}

			MoveWindow(GetDlgItem(dlg.GetDlg(), item->itemid)
				, re.x + item->x + item->px
				, re.y + item->y + item->py
				, item->w - item->px
				, item->h - item->py
				, TRUE);
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("MoveItems: %-8s C%d x:%d y:%d w:%d h:%d"), name, nums, item->x, item->y, item->w, item->h);
#endif
		}
		item = item->next;
		nums++;
	}
#ifdef _DEBUG_CBOX
	logging->out_debugf(_T("MoveItems: %-8s re x:%d y:%d w:%d h:%d"), name, re.x, re.y, re.w, re.h);
#endif
}

int CBox::GetWidthWithMargin() const
{
	return re.w;
}

int CBox::GetHeightWithMargin() const
{
	return re.h;
}

int CBox::GetWidthWithLeftMargin() const
{
	return re.w - margin.right;
}

int CBox::GetHeightWithTopMargin() const
{
	return re.h - margin.bottom;
}

int CBox::GetWidth() const
{
	return re.w - margin.left - margin.right;
}

int CBox::GetHeight() const
{
	return re.h - margin.right - margin.bottom;
}

void CBox::GetPositionByItem(int num, int &x, int &y)
{
	int nums = 0;
	struct stControls *item = controls;
	while(item != NULL) {
		if (num == nums) {
			x = re.x + item->x + item->px;
			y = re.y + item->y + item->py;
			break;
		}
		item = item->next;
		nums++;
	}
}

void CBox::SetLeftMargin(int val)
{
	margin.left = val;
}

void CBox::SetTopMargin(int val)
{
	margin.top = val;
}

void CBox::SetTabItems(CTabItems *new_items)
{
	tab_items = new_items;
}

void CBox::AddIdToTabItems(int itemid)
{
	if (tab_items) {
		tab_items->AddItemId(itemid);
	} else {
		if (parent_box) parent_box->AddIdToTabItems(itemid);
	}
}

//

CTabItemIds::CTabItemIds()
	: std::vector<int>()
{
}

CTabItemIds::~CTabItemIds()
{
}

CTabItems::CTabItems()
	: CPtrList<CTabItemIds>()
{
	current_pos = -1;
}

CTabItems::~CTabItems()
{
}

void CTabItems::SetCurrentPosition(int val)
{
	current_pos = val;
}

void CTabItems::AddItemId(int itemid)
{
	if (current_pos < 0 || Count() <= current_pos) return;

	CTabItemIds *ids = Item(current_pos);
	if (ids) {
		bool match = false;
		for(int i=0; i<(int)ids->size(); i++) {
			if (itemid == ids->at(i)) {
				match = true;
				break;
			}
		}
		if (!match) {
			ids->push_back(itemid);
		}
	}
}

}; /* namespace GUI_WIN */
