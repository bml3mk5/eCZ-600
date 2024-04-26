/** @file win_loggingbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.04.13 -

	@brief [ logging box ]
*/

#ifndef WIN_LOGGINGBOX_H
#define WIN_LOGGINGBOX_H

#include <windows.h>
#include "win_dialogbox.h"

namespace GUI_WIN
{
/**
	@brief View log dialog box
*/
class LoggingBox : public CDialogBox
{
private:
	static INT_PTR CALLBACK LoggingBoxProc(HWND, UINT, WPARAM, LPARAM);

	INT_PTR onInitDialog(UINT, WPARAM, LPARAM);
	INT_PTR onClose(UINT, WPARAM, LPARAM);
	INT_PTR onCommand(UINT, WPARAM, LPARAM);
	INT_PTR onSize(UINT, WPARAM, LPARAM);
	INT_PTR onMinMaxInfo(UINT, WPARAM, LPARAM);

	_TCHAR *p_buffer;
	int m_buffer_size;

	RECT m_client_re;

	bool m_initialized;

	void SetData();
	void Alloc(int size);
	void Free();

	void AdjustButtonPosition();

public:
	LoggingBox(HINSTANCE, CFont *, EMU *, GUI *);
	~LoggingBox();
	void Close();

};

}; /* namespace GUI_WIN */

#endif /* WIN_LOGGINGBOX_H */
