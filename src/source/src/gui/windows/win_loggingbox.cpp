/** @file win_loggingbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.13 -

	@brief [ logging box ]
*/
#include "win_loggingbox.h"
#include <WindowsX.h>
#include "../../emu.h"
#include "win_gui.h"
#include "../../logging.h"
#include "../../labels.h"
#include "../../utility.h"

namespace GUI_WIN
{

LoggingBox::LoggingBox(HINSTANCE hInst, CFont *new_font, EMU *new_emu, GUI *new_gui)
	: CDialogBox(hInst, IDD_LOGGING, new_font, new_emu, new_gui)
{
	p_buffer = NULL;
	m_buffer_size = 0;
	m_initialized  = false;
}

LoggingBox::~LoggingBox()
{
	Free();
}

void LoggingBox::Close()
{
	CDialogBox::Close();
	Free();
}

void LoggingBox::Alloc(int size)
{
	m_buffer_size = size;
	p_buffer = new _TCHAR[m_buffer_size];
	p_buffer[0] = 0;
}

void LoggingBox::Free()
{
	delete [] p_buffer;
	p_buffer = NULL;
	m_buffer_size = 0;
}

INT_PTR LoggingBox::onInitDialog(UINT message, WPARAM wParam, LPARAM lParam)
{
	m_initialized  = false;

	CDialogBox::onInitDialog(message, wParam, lParam);

	const _TCHAR *onechar = _T("m");
	SIZE siz = { 0, 0 };

	// edit box
	font->GetTextSize(hDlg, onechar, &siz);

	// layout
	CBox *box_all = new CBox(CBox::VerticalBox, 0, 1);
	// file
	CreateTextControl(box_all, IDC_TEXT_LOGPATH, false, true, 480, siz.cy + 4);
	// text
	CreateTextControl(box_all, IDC_TEXT_LOG, true, true, 480, 320);
	// button
	CBox *box_btn = new CBox(CBox::HorizontalBox, CBox::CenterPos, padding);
	box_all->AddBox(box_btn);
	CreateButton(box_btn, IDC_BTN_UPDATE, CMsg::Update, 8, false);
	CreateButton(box_btn, IDOK, CMsg::Close, 8, true);

	box_all->Realize(*this);

	delete box_all;

	// path
	SetDlgItemText(hDlg, IDC_TEXT_LOGPATH, logging->get_log_path());

	GetClientRect(hDlg, &m_client_re);

	// buttons
	AdjustButtonPosition();

	m_initialized  = true;

	return (INT_PTR)TRUE;
}

INT_PTR LoggingBox::onClose(UINT message, WPARAM wParam, LPARAM lParam)
{
	Close();
	return (INT_PTR)TRUE;
}

INT_PTR LoggingBox::onCommand(UINT message, WPARAM wParam, LPARAM lParam)
{
	int id = (int)LOWORD(wParam);
	if (id == IDC_BTN_UPDATE) {
		SetData();
		return (INT_PTR)TRUE;
	} else if (id == IDOK) {
		Close();
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR LoggingBox::onSize(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!m_initialized) {
		return (INT_PTR)FALSE;
	}

	RECT re;
	GetClientRect(hDlg, &re);

	int s_w = re.right - re.left - m_client_re.right + m_client_re.left;
	int s_h = re.bottom - re.top - m_client_re.bottom + m_client_re.top;

	m_client_re = re;

	// path
	GetWindowRect(GetDlgItem(hDlg, IDC_TEXT_LOGPATH), &re);
	SetWindowPos(GetDlgItem(hDlg, IDC_TEXT_LOGPATH), HWND_TOP, 0, 0, re.right - re.left + s_w, re.bottom - re.top, SWP_NOMOVE | SWP_NOZORDER);
	// log
	GetWindowRect(GetDlgItem(hDlg, IDC_TEXT_LOG), &re);
	SetWindowPos(GetDlgItem(hDlg, IDC_TEXT_LOG), HWND_TOP, 0, 0, re.right - re.left + s_w, re.bottom - re.top + s_h, SWP_NOMOVE | SWP_NOZORDER);
	// buttons
	AdjustButtonPosition();

	return (INT_PTR)FALSE;
}

INT_PTR LoggingBox::onMinMaxInfo(UINT message, WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO *info = (MINMAXINFO *)lParam;

	info->ptMinTrackSize.x = 480;
	info->ptMinTrackSize.y = 320;
	return (INT_PTR)FALSE;
}

void LoggingBox::AdjustButtonPosition()
{
	RECT re;

	GetWindowRect(GetDlgItem(hDlg, IDC_BTN_UPDATE), &re);
	SetWindowPos(GetDlgItem(hDlg, IDC_BTN_UPDATE), HWND_TOP, 4, m_client_re.bottom - re.bottom + re.top - 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, IDOK), &re);
	SetWindowPos(GetDlgItem(hDlg, IDOK), HWND_TOP, m_client_re.right - re.right + re.left - 4, m_client_re.bottom - re.bottom + re.top - 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void LoggingBox::SetData()
{
	if (!p_buffer) {
		m_buffer_size = 1024 * 1024;
		Alloc(m_buffer_size);
	}
	p_buffer[0] = 0;
	logging->get_log(p_buffer, m_buffer_size);
	SetDlgItemText(hDlg, IDC_TEXT_LOG, p_buffer);
	int pos = (int)SendDlgItemMessage(hDlg, IDC_TEXT_LOG, EM_GETLINECOUNT, 0, 0);
	SendDlgItemMessage(hDlg, IDC_TEXT_LOG, EM_LINESCROLL, 0, pos);
}

}; /* namespace GUI_WIN */
