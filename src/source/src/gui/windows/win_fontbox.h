/** @file win_fontbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ font box ]
*/

#ifndef WIN_FONTBOX_H
#define WIN_FONTBOX_H

#include <Windows.h>
#include <tchar.h>

namespace GUI_WIN
{

/**
	@brief Font dialog box
*/
class FontBox
{
private:
	HWND hWnd;

public:
	FontBox(HWND parent_window);
	~FontBox();
	bool Show(const _TCHAR *title, LOGFONT *lf, _TCHAR *font_name, size_t name_size, double *font_size);
};

}; /* namespace GUI_WIN */

#endif /* WIN_FONTBOX_H */
