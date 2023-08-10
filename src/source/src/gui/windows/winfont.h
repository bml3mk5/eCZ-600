/** @file winfont.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21

	@brief [ font ]
*/

#ifndef WINFONT_H
#define WINFONT_H

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
//#include "common.h"

/// フォント設定
class CFont
{
private:
	LOGFONT m_lf;		///< 論理フォント
	HFONT   hFont;		///< フォントハンドル
	COLORREF mColor;	///< フォントカラー

	void set_default_font();
	void set_font(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight = FW_DONTCARE);

public:
	CFont();
	~CFont();

	void SetDefaultFont(HWND hWnd = NULL, const _TCHAR *font_name = NULL, double font_size = 9, COLORREF font_color = 0);
	void SetFont(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight = FW_DONTCARE, COLORREF font_color = 0);
	HFONT GetFont() { return hFont; }
	void GetFontName(_TCHAR *font_name, size_t name_len);
	const _TCHAR *GetFontNamePtr();
	void SetLogFont(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight = FW_DONTCARE);
	void SetLogFont(const LOGFONT *plf);
	LOGFONT *GetLogFont() { return &m_lf; }
	LONG GetHeight() { return abs(m_lf.lfHeight); }
	LONG GetTextWidth(HWND hWnd, const _TCHAR *text);
	void GetTextSize(HWND hWnd, const _TCHAR *text, SIZE *size);
	void SetFontColor(COLORREF value);
	COLORREF GetFontColor() { return mColor; }

	void GetLogFontOnNonClientArea();
	void GetFontSizeOnDC(HWND hWnd, SIZE *size);

	static LONG CalcHeightFromPoint(HWND hWnd, double font_size);
	static double CalcPointFromHeight(HWND hWnd, LONG height);

	static int AddFontPath(const _TCHAR *font_path);

#ifdef USE_CFONT_REGISTORY
	static bool GetFileNameFromFontNameEx(const _TCHAR *font_name, _TCHAR *file_name, int size);
	static bool GetFileNameFromFontName(const _TCHAR *font_name, _TCHAR *file_name, int size);
	static bool GetMappedFontName(const _TCHAR *font_name, _TCHAR *new_font_name, int size);
	static bool GetFontNameFromFileName(const _TCHAR *file_name, _TCHAR *font_name, int size);
#endif
};

#endif /* WINFONT_H */
