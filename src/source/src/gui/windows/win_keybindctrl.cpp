/// @file win_keybindctrl.cpp
///
/// @brief キーコード割り付けコントロール
///
/// @author Sasaji
/// @date   2012.3.31
///

#include "win_keybindctrl.h"
#include "../../res/resource.h"
#include "win_gui.h"
#include "../../emu.h"
#include "../../clocale.h"
#include "../../utility.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include "win_key_trans.h"
#endif

#define TIMER_JOYPAD 1

extern EMU *emu;

namespace GUI_WIN
{

ATOM KeybindControl::RegisterClass(HINSTANCE hInst)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= KeybindControl::CustomCtrlProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("KeyBindCtrl");
	wcex.hIconSm		= NULL;

	return ::RegisterClassEx(&wcex);
}

void KeybindControl::UnregisterClass(HINSTANCE hInst)
{
	::UnregisterClass(TEXT("KeyBindCtrl"), hInst);
}

KeybindControl *KeybindControl::GetPtr(HWND hWnd)
{
	return (KeybindControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

LRESULT CALLBACK KeybindControl::CustomCtrlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInst;
	KeybindControl *kbctl;
	LRESULT re;

	if (message == WM_CREATE) {
		// KeyBindCtrlウィンドウにKeybindControlのインスタンスを関連付ける。
		hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		kbctl = new KeybindControl(hInst, hWnd);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)kbctl);
	} else {
		kbctl = (KeybindControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	// インスタンスのプロシージャを呼ぶ
	re = kbctl->Proc(hWnd, message, wParam, lParam);

	if (message == WM_NCDESTROY) {
		// KeyBindCtrlウィンドウからKeybindControlのインスタンスを破棄する。
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
		delete kbctl;
	}
	return re;
}
LRESULT CALLBACK KeybindControl::CustomCtrlMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hMainCtrl = GetParent(hWnd);
	KeybindControl *kbctl = (KeybindControl *)GetWindowLongPtr(hMainCtrl, GWLP_USERDATA);

	return kbctl->MainProc(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK KeybindControl::CustomCtrlEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hMainCtrl = GetParent(GetParent(hWnd));
	KeybindControl *kbctl = (KeybindControl *)GetWindowLongPtr(hMainCtrl, GWLP_USERDATA);

	return kbctl->EditProc(hWnd, message, wParam, lParam);
}

KeybindControl::KeybindControl(HINSTANCE hInst, HWND hWnd) : KeybindData()
{
	hInstance = hInst;
	hMainCtrl = hWnd;
	font = NULL;

	hStaticMain = NULL;
	memset(table, 0, sizeof(table));

	memset(&vScrMain, 0, sizeof(vScrMain));
	vScrMain.cbSize = sizeof(vScrMain);

	cell_height = 18;
	cell_width = 80;
	margin = 5;
	padding = 2;

	ctrl_width = 90;
	ctrl_height = 30;

	use_timer = false;
}

KeybindControl::~KeybindControl()
{
}

void KeybindControl::Init(EMU *emu, int new_tabnum, HFONT new_font)
{
	font = new_font;
	KeybindData::Init(emu, new_tabnum);
	if (m_devtype == DEVTYPE_JOYPAD) {
		// use joypad
		init_joypad(hMainCtrl);
	}
}

void KeybindControl::init_joypad(HWND hWnd)
{
	// joypad
	use_timer = true;
	SetTimer(hWnd, TIMER_JOYPAD, 100, NULL);
}

void KeybindControl::term_joypad(HWND hWnd)
{
	if (use_timer) {
		KillTimer(hWnd, TIMER_JOYPAD);
	}
}

void KeybindControl::update_joypad(ctrlcols_t *obj)
{
	emu->update_joystick();
	uint32_t *joy_stat = emu->joy_real_buffer(obj->col);
	if ((joy_stat[0] & *p_joy_mask) | joy_stat[1]) {
		SetJoyCell(obj, joy_stat[0] & *p_joy_mask, joy_stat[1]);
	}
}

LRESULT KeybindControl::Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
//	HDC hdc;
//	_TCHAR str[1000];
//	_stprintf(str,_T("m:%x w:%d l:%d\n"),message,wParam,lParam);
//	OutputDebugString(str);

	switch (message) {
		case WM_CREATE:
			create_children(hWnd);
			break;
		case WM_DESTROY:
			term_joypad(hWnd);
			destroy_children(hWnd);
			break;
		case WM_SETFONT:
			setfont_children(hWnd, wParam);
			break;
		case WM_CTLCOLORSTATIC:
			return ctlcolor_children(hWnd, wParam, lParam);
			break;
		case WM_PAINT:
			GetScrollInfo(hWnd, SB_VERT, &vScrMain);

			BeginPaint(hWnd, &ps);
			move_children(hWnd, 0, -vScrMain.nPos);
			EndPaint(hWnd, &ps);

			break;
		// 縦スクロールバー
		case WM_VSCROLL:
			switch(LOWORD(wParam)) {
			case SB_TOP:
				vScrMain.nPos = vScrMain.nMin;
				break;
			case SB_BOTTOM:
				vScrMain.nPos = vScrMain.nMax;
				break;
			case SB_LINEUP:
				vScrMain.nPos -= 10;
				if (vScrMain.nPos < vScrMain.nMin) vScrMain.nPos = vScrMain.nMin;
				break;
			case SB_LINEDOWN:
				vScrMain.nPos += 10;
				if (vScrMain.nPos > vScrMain.nMax) vScrMain.nPos = vScrMain.nMax;
				break;
			case SB_PAGEUP:
				vScrMain.nPos -= vScrMain.nPage;
				if (vScrMain.nPos < vScrMain.nMin) vScrMain.nPos = vScrMain.nMin;
				break;
			case SB_PAGEDOWN:
				vScrMain.nPos += vScrMain.nPage;
				if (vScrMain.nPos > vScrMain.nMax) vScrMain.nPos = vScrMain.nMax;
				break;
			case SB_THUMBTRACK:
				vScrMain.nPos = HIWORD(wParam);
				break;
			case SB_THUMBPOSITION:
				vScrMain.nPos = HIWORD(wParam);
				break;
			}
			SetScrollInfo(hWnd , SB_VERT , &vScrMain , TRUE);

			InvalidateRect(hWnd , NULL , TRUE);
			break;
		// ホイールマウス
		case WM_MOUSEWHEEL:
			{
				// ホイールを回した量
				int delta_y = GET_WHEEL_DELTA_WPARAM(wParam);
				set_scroll_bar_pos(hWnd, -delta_y);
				InvalidateRect(hWnd , NULL , TRUE);
			}
			return 1;
			break;

		// JOYPAD
//		case MM_JOY1MOVE:
//		case MM_JOY1BUTTONDOWN:
		case WM_TIMER:
			{
				HWND hF = GetFocus();
				HWND hPF = GetParent(hF);

				if (hF != NULL && hPF == hStaticMain) {
					// フォーカスがあれば送信
					SendMessage(hF, message, wParam, lParam);
				}
			}
			return 1;
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
				{
					HWND hF = GetFocus();
					HWND hPF = GetParent(hF);

					if (hF == hWnd || hPF == hWnd) {
						return 1;	// フォーカスがある場合
					} else {
						return 0;
					}
				}
				break;
			}
			break;
	}

	return DefWindowProc(hWnd , message , wParam , lParam);
}

LRESULT KeybindControl::MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC origProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch(message) {
	case WM_CTLCOLORSTATIC:
		{
			HWND hCtrl = (HWND)lParam;
			HWND hF = GetFocus();
			HWND hPF = GetParent(hF);

			if (hF == hCtrl && hPF == hWnd) {
				// フォーカスが当たっているコントロールの背景色を変更
//				HDC hDC = (HDC)wParam;
				HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);

				return (LRESULT)hBrush;	// 背景の色
			}
		}
		break;
	}

	return CallWindowProc(origProc, hWnd , message , wParam , lParam);
}

LRESULT KeybindControl::EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ctrlcols_t *obj = (ctrlcols_t *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	WNDPROC origProc;
//	_TCHAR str[1000];
//	_stprintf(str,_T("m:%x w:%d l:%d\n"),message,wParam,lParam);
//	OutputDebugString(str);

	if (obj == NULL) {
		return 0;
	}

	origProc = obj->proc;

	switch (message) {
		case WM_DESTROY:
			return 0;
			break;
		case WM_GETDLGCODE:
			{
				LRESULT iRet = CallWindowProc(origProc, hWnd , message , wParam , lParam);
				if (lParam) {
					iRet |= DLGC_WANTALLKEYS;
				}
				return iRet;
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				ClearCell(obj);
			}
			break;
		case WM_SYSCHAR:
		case WM_CHAR:
			return 0;
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (m_devtype != DEVTYPE_JOYPAD && (wParam < 0x88 || wParam > 0x8f)) {
				wParam = translate_keycode(wParam, lParam);
				ClearCellByVkCode((uint32_t)wParam);
				SetKeyCell(obj, (uint32_t)wParam);
			}
			return 0;
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			return 0;
			break;
//		case MM_JOY1MOVE:
//		case MM_JOY1BUTTONDOWN:
		case WM_TIMER:
			if (m_devtype == DEVTYPE_JOYPAD) {
				update_joypad(obj);
			}
			return 0;
			break;
	}
	return CallWindowProc(origProc, hWnd , message , wParam , lParam);
}

/// 子コントロールを作成する
///
///
void KeybindControl::create_children(HWND hWnd)
{
	_TCHAR label[KBLABEL_MAXLEN];
	int y = margin;
	int x = margin;

	vScrMain.cbSize = sizeof(SCROLLINFO);

	memset(table, 0, sizeof(table));

	hStaticMain = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE,	 0, 0, 1, 1, hWnd, NULL, hInstance, NULL);
	if (hStaticMain == NULL) {
		return;
	}
	WNDPROC proc = (WNDPROC)GetWindowLongPtr(hStaticMain, GWLP_WNDPROC);
	SetWindowLongPtr(hStaticMain, GWLP_WNDPROC, (LONG_PTR)CustomCtrlMainProc);
	SetWindowLongPtr(hStaticMain, GWLP_USERDATA, (LONG_PTR)proc);

	// title
	hTitles[0] = CreateWindow(_T("STATIC"), _T("vm key"), WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
				 x, y, cell_width, cell_height, hStaticMain, NULL, hInstance, NULL);
	x += cell_width + padding;
	for(int j=1; j<=KBCTRL_MAX_COLS; j++) {
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("bind%d"), j);
		hTitles[j] = CreateWindow(_T("STATIC"), label, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
				 x, y, cell_width, cell_height, hStaticMain, NULL, hInstance, NULL);
		x += cell_width + padding;
	}
	y += cell_height + padding;

	x += margin - padding;
	y += margin - padding;
	ctrl_width = x;
	ctrl_height = y;
	MoveWindow(hStaticMain, 0, 0, x, y, false);

	// スクロールバー設定
	set_scroll_bar_range(hWnd, y);
}

/// 子コントロールのフォントをセット
///
///
void KeybindControl::setfont_children(HWND hWnd, WPARAM wParam)
{
	HDC hdc = GetDC(hWnd);
	SelectObject(hdc, (HFONT)wParam);
	SIZE siz;
	GetTextExtentPoint32(hdc, _T("0"), 1, &siz);
	ReleaseDC(hWnd, hdc);
	if (siz.cx > 0) cell_width = siz.cx * 10 + padding * 2;
	if (siz.cy > 0) cell_height = siz.cy + padding * 2;

	for(int j=0; j<=KBCTRL_MAX_COLS; j++) {
		if (hTitles[j]) SendMessage(hTitles[j],WM_SETFONT,wParam,MAKELPARAM(TRUE, 0));
	}
	for(int i=0; i<KBCTRL_MAX_LINES; i++) {
		if (table[i].hSta1) SendMessage(table[i].hSta1,WM_SETFONT,wParam,MAKELPARAM(TRUE, 0));
		for(int j=0; j<KBCTRL_MAX_COLS; j++) {
			if (table[i].cols[j].hEdt) SendMessage(table[i].cols[j].hEdt,WM_SETFONT,wParam,MAKELPARAM(TRUE, 0));
		}
	}
}
/// 子コントロールを削除する
///
///
void KeybindControl::destroy_children(HWND hWnd)
{
	for(int i=0; i<KBCTRL_MAX_LINES; i++) {
		for(int j=0; j<KBCTRL_MAX_COLS; j++) {
			SetWindowLongPtr(table[i].cols[j].hEdt, GWLP_WNDPROC, (LONG_PTR)table[i].cols[j].proc);
		}
	}
	WNDPROC proc = (WNDPROC)GetWindowLongPtr(hStaticMain, GWLP_USERDATA);
	SetWindowLongPtr(hStaticMain, GWLP_WNDPROC, (LONG_PTR)proc);
}

/// 子コントロールを配置する
///
///
void KeybindControl::dispose_children(HWND hWnd)
{
	_TCHAR label[KBLABEL_MAXLEN];
	int x = margin;
	int y = margin;

	// title
	for(int j=0; j<=KBCTRL_MAX_COLS; j++) {
		MoveWindow(hTitles[j], x, y, cell_width, cell_height, false);
		x += cell_width + padding;
	}

	y += padding + cell_height + padding;
	x = margin;

	// create cell control
	int rows = GetNumberOfRows();

	for(int row=0; row<rows; row++) {
		x = margin;

		if (m_vm_type == VM_TYPE_PIOBITASSIGN) {
			KeybindData::GetVmJoyBitLabel(row2idx_map[row], label, true);
		} else if (m_vm_type == VM_TYPE_PIOJOYASSIGN) {
			KeybindData::GetVmJoyLabel(row2idx_map[row], label, true);
		} else {
			KeybindData::GetVmKeyLabel(row2idx_map[row], label, true);
		}
		UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);

		table[row].hSta1 = CreateWindow(_T("STATIC"), label, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
				 x, y, cell_width, cell_height, hStaticMain, NULL, hInstance, NULL);
		SendMessage(table[row].hSta1, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		x += cell_width + padding;

		for(int col=0; col<KBCTRL_MAX_COLS; col++) {
			table[row].cols[col].row = row;
			table[row].cols[col].col = col;
			table[row].cols[col].hEdt = CreateWindow(_T("edit"),_T(""),WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_READONLY,
					 x, y, cell_width, cell_height, hStaticMain, NULL, hInstance, NULL);
			SendMessage(table[row].cols[col].hEdt, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
			table[row].cols[col].proc = (WNDPROC)GetWindowLongPtr(table[row].cols[col].hEdt, GWLP_WNDPROC);
			SetWindowLongPtr(table[row].cols[col].hEdt, GWLP_WNDPROC, (LONG_PTR)CustomCtrlEditProc);
			SetWindowLongPtr(table[row].cols[col].hEdt, GWLP_USERDATA, (LONG_PTR)&table[row].cols[col]);
			x += cell_width + padding;
		}

		table[row].enabled = true;

		y += cell_height + padding;
	}

	x += margin - padding;
	y += margin - padding;
	MoveWindow(hStaticMain, 0, 0, x, y, true);

	ctrl_width = x;
	ctrl_height = y;

	// スクロールバー設定
	set_scroll_bar_range(hWnd, y);
}
//
//
//
LRESULT KeybindControl::ctlcolor_children(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
//	HDC hDC = (HDC)wParam;
//	HWND hCtrl = (HWND)lParam;
	HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);

	return (LRESULT)hBrush;	// テキストが書かれていない部分の背景の色
}

//
//
//
void KeybindControl::move_children(HWND hWnd, int x, int y)
{
	WINDOWINFO wid;
	RECT re;

	GetWindowInfo(hStaticMain, &wid);

	re.left = x;
	re.top = y;
	re.right = wid.rcClient.right - wid.rcClient.left;
	re.bottom = wid.rcClient.bottom - wid.rcClient.top;
	MoveWindow(hStaticMain, re.left, re.top, re.right, re.bottom, true);


}

UINT32 KeybindControl::translate_keycode(WPARAM wParam, LPARAM lParam) {
	int new_code = 0;
#ifdef USE_SDL2
	UINT32 scan_code = translate_to_sdlkey(wParam, lParam);
	emu->translate_keysym(0, (int)wParam, (short)scan_code, &new_code);
#else
	emu->translate_keysym(0, (int)wParam, (long)lParam, &new_code);
#endif
	return new_code;
}

void KeybindControl::set_scroll_bar_range(HWND hWnd, int y)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	vScrMain.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE;
	vScrMain.nMin = 0;
	vScrMain.nMax = y;
	vScrMain.nPage = rt.bottom - rt.top;
	SetScrollInfo(hWnd , SB_VERT , &vScrMain , TRUE);
	vScrMain.fMask = SIF_POS;
}

/// スクロールバーの位置設定
void KeybindControl::set_scroll_bar_pos(HWND hWnd, int dy)
{
	vScrMain.fMask = SIF_POS;
	vScrMain.nPos += dy;
	if (vScrMain.nPos > vScrMain.nMax) vScrMain.nPos = vScrMain.nMax;
	if (vScrMain.nPos < vScrMain.nMin) vScrMain.nPos = vScrMain.nMin;
	SetScrollInfo(hWnd , SB_VERT , &vScrMain , TRUE);
}

#if 0
void KeybindControl::SetTitleLabel(const _TCHAR *vmlabel, const _TCHAR *vklabel)
{
	_TCHAR label[KBLABEL_MAXLEN];

	UTILITY::conv_to_api_string(vmlabel, (int)_tcslen(vmlabel), label, KBLABEL_MAXLEN);
	SetWindowText(hTitles[0], label);
	for(int j=1; j<=KBCTRL_MAX_COLS; j++) {
		_stprintf(label, vklabel, j);
		UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);
		SetWindowText(hTitles[j], label);
	}
}
#endif

void KeybindControl::SetTitleLabel(CMsg::Id vmlabelid, CMsg::Id vklabelid)
{
	_TCHAR label[KBLABEL_MAXLEN];
	const _TCHAR *p;

	p = CMSGVM(vmlabelid);
	SetWindowText(hTitles[0], p);
	p = CMSGVM(vklabelid);
	for(int j=1; j<=KBCTRL_MAX_COLS; j++) {
		UTILITY::stprintf(label, KBLABEL_MAXLEN, p, j);
		SetWindowText(hTitles[j], label);
	}
}

void KeybindControl::SetCellSize(int w, int h)
{
	cell_width = w;
	cell_height = h;
}

void KeybindControl::Update()
{
	int rows = GetNumberOfRows();
	int cols = GetNumberOfColumns();
	for(int row=0; row<rows; row++) {
		for(int col=0; col<cols; col++) {
			SetVkLabel(row, col);
		}
	}
}

bool KeybindControl::SetVkLabel(int row, int col)
{
	_TCHAR label[KBLABEL_MAXLEN];
	if (row < 0 || row >= KBCTRL_MAX_LINES) {
		return false;
	}
	if (col < 0 || col >= KBCTRL_MAX_COLS) {
		return false;
	}
	if (table[row].enabled != true) {
		return false;
	}
	ctrlcols_t *obj = &table[row].cols[col];
	GetCellString(row, col, label);
	UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);
	SetWindowText(obj->hEdt, label);
	return true;
}

bool KeybindControl::SetKeyCell(ctrlcols_t *obj, uint32_t code)
{
	_TCHAR label[KBLABEL_MAXLEN];
	int row = obj->row;
	int col = obj->col;

	if (m_devtype != DEVTYPE_KEYBOARD) return false;

	SetVkKeyCode(row, col, code, label);

	UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);
	SetWindowText(obj->hEdt, label);
	return true;
}

bool KeybindControl::SetJoyCell(ctrlcols_t *obj, uint32_t code0, uint32_t code1)
{
	_TCHAR label[KBLABEL_MAXLEN];
	int row = obj->row;
	int col = obj->col;

	if (m_devtype != DEVTYPE_JOYPAD) return false;

	SetVkJoyCode(row, col, code0, code1, label);

	UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);
	SetWindowText(obj->hEdt, label);
	return true;
}

bool KeybindControl::ClearCell(ctrlcols_t *obj)
{
	_TCHAR label[KBLABEL_MAXLEN];
	int row = obj->row;
	int col = obj->col;

	if (m_devtype == DEVTYPE_JOYPAD) {
		ClearVkJoyCode(row, col, label);
	} else {
		ClearVkKeyCode(row, col, label);
	}
	UTILITY::conv_to_api_string(label, KBLABEL_MAXLEN, label, KBLABEL_MAXLEN);
	SetWindowText(obj->hEdt, label);
	return true;
}

bool KeybindControl::ClearCellByVkCode(uint32_t code)
{
	if (m_flags & FLAG_DENY_DUPLICATE) {
		int row = vkkey2rowcol_map[code].row;
		int col = vkkey2rowcol_map[code].col;
		if (row >= 0 && col >= 0) {
			return ClearCell(&table[row].cols[col]);
		}
	}
	return false;
}

bool KeybindControl::MapDefaultVmKey()
{
	dispose_children(hMainCtrl);
	return true;
}

void KeybindControl::LoadDefaultPreset()
{
	KeybindData::LoadDefaultPreset();
	Update();
}

void KeybindControl::LoadPreset(int idx)
{
	KeybindData::LoadPreset(idx);
	Update();
}

}; /* namespace GUI_WIN */
