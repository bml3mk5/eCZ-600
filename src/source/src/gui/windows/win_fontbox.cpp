/** @file win_fontbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ font box ]
*/

#include "../../common.h"
#include "win_fontbox.h"
#include "winfont.h"
#include "../../utility.h"

namespace GUI_WIN
{

FontBox::FontBox(HWND parent_window)
{
	hWnd = parent_window;
}

FontBox::~FontBox()
{
}

/// @brief show font dialog
/// @param[in] title (not used)
/// @param[in,out] lf logfont
/// @param[in,out] font_name font name
/// @param[in]     name_size buffer size of font_name
/// @param[in,out] font_size font size
bool FontBox::Show(const _TCHAR *title, LOGFONT *lf, _TCHAR *font_name, size_t name_size, double *font_size)
{
	LOGFONT new_lf;
	memset(&new_lf, 0, sizeof(new_lf));
	if (lf) {
		new_lf = *lf;
	}
	if (font_name && font_name[0] != _T('\0')) {
		UTILITY::tcscpy(new_lf.lfFaceName, sizeof(new_lf.lfFaceName), font_name);
	}
	if (font_size) {
		new_lf.lfHeight = CFont::CalcHeightFromPoint(hWnd, *font_size);
	}

	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_LIMITSIZE | CF_NOVERTFONTS; // | CF_EFFECTS;
	cf.hwndOwner = hWnd;
	cf.hDC = NULL;
	cf.lpLogFont = &new_lf;
	cf.nSizeMin = 6;
	cf.nSizeMax = 60;
	//	cf.rgbColors = mColor;

	// display font dialog
	BOOL rc = ChooseFont(&cf);
	if (rc == TRUE) {
		if (lf) *lf = new_lf;
		if (font_name) UTILITY::tcscpy(font_name, name_size, cf.lpLogFont->lfFaceName);
		if (font_size) *font_size = (double)cf.iPointSize / 10.0;
//		if (font_color && cf.rgbColors != mColor) *font_color = cf.rgbColors;
	}
	return (rc == TRUE);
}

}; /* namespace GUI_WIN */
