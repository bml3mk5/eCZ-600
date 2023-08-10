/** @file winfont.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.01.21

	@brief [ font ]
*/

#include "winfont.h"
#include "../../common.h"
#include "../../cchar.h"
#include "../../utility.h"

CFont::CFont()
{
	memset(&m_lf, 0, sizeof(LOGFONT));
	hFont = NULL;
	set_default_font();
	SetFontColor(0xffffffff);
}

CFont::~CFont()
{
	if (hFont != NULL) DeleteObject(hFont);
}

/// @brief デフォルトフォントの設定
/// @param [in] hWnd
/// @param [in] font_name フォントファミリ名
/// @param [in] font_size フォントサイズ
/// @param [in] font_color フォントの色
void CFont::SetDefaultFont(HWND hWnd, const _TCHAR *font_name, double font_size, COLORREF font_color)
{
	set_default_font();
	SetFontColor(font_color);
}

/// @brief デフォルトフォントの設定
void CFont::set_default_font()
{
	GetLogFontOnNonClientArea();
	if (hFont != NULL) DeleteObject(hFont);
	hFont = CreateFontIndirect(&m_lf);
}

/// @brief フォントの設定
/// @param [in] hWnd
/// @param [in] font_name フォントファミリ名
/// @param [in] font_size フォントサイズ
/// @param [in] weight フォントの太さ
/// @param [in] font_color フォントの色
void CFont::SetFont(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight, COLORREF font_color)
{
	if (_tcslen(font_name) > 0) {
		set_font(hWnd, font_name, font_size, weight);
	} else {
		set_font(hWnd, NULL, font_size, weight);
	}
	SetFontColor(font_color);
}

/// @brief フォントの設定
void CFont::set_font(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight)
{
	SetLogFont(hWnd, font_name, font_size, weight);
	if (hFont != NULL) DeleteObject(hFont);
	hFont = CreateFontIndirect(&m_lf);
}

/// @brief フォントの設定（論理フォント指定）
/// @param [in] plf 論理フォント
void CFont::SetLogFont(const LOGFONT *plf)
{
	m_lf = *plf;
	if (hFont != NULL) DeleteObject(hFont);
	hFont = CreateFontIndirect(&m_lf);
}

/// @brief 論理フォントの設定
/// @param [in] hWnd
/// @param [in] font_name フォントファミリ名
/// @param [in] font_size フォントサイズ
/// @param [in] weight フォントの太さ
void CFont::SetLogFont(HWND hWnd, const _TCHAR *font_name, double font_size, LONG weight)
{
	if (font_name != NULL) {
//		memset(&m_lf, 0, sizeof(LOGFONT));
		// フォント名
		UTILITY::tcscpy(m_lf.lfFaceName, LF_FACESIZE, font_name);
		m_lf.lfWidth = 0;							// 平均文字幅
		m_lf.lfEscapement = 0;					// 文字送りの方向とX軸との角度 1/10°単位で指定。
		m_lf.lfOrientation = m_lf.lfEscapement;		// ベースラインとX軸との角度
		m_lf.lfItalic = 0;						// イタリック体指定
		m_lf.lfUnderline = 0;						// 下線付き指定
		m_lf.lfStrikeOut = 0;						// 打ち消し線付き指定
		m_lf.lfCharSet = DEFAULT_CHARSET;			// キャラクタセット
		m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;		// 出力精度
		m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;	// クリッピングの精度
		m_lf.lfQuality = DEFAULT_QUALITY;				// 出力品質
		m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;	// ピッチとファミリ
	}
	if (weight >= 0) m_lf.lfWeight = weight;				// フォントの太さ
	if (font_size >= 0.0) m_lf.lfHeight = CalcHeightFromPoint(hWnd, font_size);	// 文字セルまたは文字の高さ
}

/// @brief フォント名を取得
void CFont::GetFontName(_TCHAR *font_name, size_t name_len)
{
	UTILITY::tcscpy(font_name, name_len, m_lf.lfFaceName);
}

/// @brief フォント名を取得
const _TCHAR *CFont::GetFontNamePtr()
{
	return m_lf.lfFaceName;
}

/// @brief テキストの幅を取得
LONG CFont::GetTextWidth(HWND hWnd, const _TCHAR *text)
{
	SIZE siz;
	GetTextSize(hWnd, text, &siz);
	return siz.cx;
}

/// @brief テキストのサイズを取得
void CFont::GetTextSize(HWND hWnd, const _TCHAR *text, SIZE *size)
{
	HDC hdc = ::GetDC(hWnd);
//	HFONT hOldFont = (HFONT)
	::SelectObject(hdc, hFont);
	int len = 0;
	if (text != NULL && (len = (int)_tcslen(text)) > 0) {
//		::GetTextExtentPoint32(hdc, text, len, size);
		RECT re = { 0, 0, 0, 0 };
		::DrawText(hdc, text, len, &re, DT_CALCRECT);
		size->cx = re.right - re.left;
		size->cy = re.bottom - re.top;
	} else {
		::GetTextExtentPoint32(hdc, _T("0"), 1, size);
	}
	::ReleaseDC(hWnd, hdc);
}

/// @brief 画面からデフォルトのシステムフォント（論理フォント）を得る
void CFont::GetLogFontOnNonClientArea()
{
	// OSバージョンを取得
	OSVERSIONINFO vi;
	memset(&vi, 0, sizeof(vi));
	vi.dwOSVersionInfoSize = sizeof(vi);
	GetVersionEx(&vi);

	// get logfont on non client area
	NONCLIENTMETRICS ncm;
	// Vista以降(Windows SDK 7.0以降)では構造体サイズが異なる
#if (WINVER >= 0x0600)
	ncm.cbSize = (vi.dwMajorVersion >= 6 ? sizeof(ncm) : sizeof(ncm) - 4);
#else
	ncm.cbSize = sizeof(ncm);
#endif
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	m_lf = ncm.lfMessageFont;
}

/// @brief 指定画面のフォントの幅と高さを得る
void CFont::GetFontSizeOnDC(HWND hWnd, SIZE *size)
{
	HDC hdc = GetDC(hWnd);
//	HFONT hOldFont = (HFONT)
	SelectObject(hdc, hFont);
	GetTextExtentPoint32(hdc, _T("0"), 1, size);
	ReleaseDC(hWnd, hdc);
}

/// @brief ポイントサイズから高さを計算
LONG CFont::CalcHeightFromPoint(HWND hWnd, double font_size)
{
	HDC hdc = GetDC(hWnd);
	int logdpiy = GetDeviceCaps(hdc, LOGPIXELSY);
	if (logdpiy == 0) logdpiy = 96;
	LONG nHeight = (LONG)(font_size * logdpiy / -72.0);
	ReleaseDC(hWnd, hdc);
	return nHeight;
}

/// @brief 高さからポイントサイズを計算
double CFont::CalcPointFromHeight(HWND hWnd, LONG height)
{
	HDC hdc = GetDC(hWnd);
	int logdpiy = GetDeviceCaps(hdc, LOGPIXELSY);
	if (logdpiy == 0) logdpiy = 96;
	double dSize = (double)height * -72.0 / logdpiy;
	ReleaseDC(hWnd, hdc);
	return dSize;
}

/// @brief フォントカラー設定
void CFont::SetFontColor(COLORREF value)
{
	if (value == 0xffffffff) {
		value = ::GetSysColor(COLOR_WINDOWTEXT);
	}
	mColor = value;
}

/// @brief 検索するフォントパスの追加
/// @param[in] font_path フォントパス
/// @return 0:失敗
int CFont::AddFontPath(const _TCHAR *font_path)
{
	int rc = 0;
	if (_tcslen(font_path) > 0) {
		rc = AddFontResourceEx(font_path, FR_PRIVATE, NULL);
	}
	return rc;
}

#ifdef USE_CFONT_REGISTORY
/// @brief フォント名からファイル名を得る
/// レジストリを検索
bool CFont::GetFileNameFromFontNameEx(const _TCHAR *font_name, _TCHAR *file_name, int size)
{
	bool rc = GetFileNameFromFontName(font_name, file_name, size);
	if (!rc) {
		_TCHAR new_font_name[_MAX_PATH];
		// 無い場合は別名から得られるか
		rc = GetMappedFontName(font_name, new_font_name, _MAX_PATH);
		if (rc) {
			rc = GetFileNameFromFontName(new_font_name, file_name, size);
		}
	}
	return rc;
}

/// @brief フォント名からファイル名を得る
/// レジストリを検索
bool CFont::GetFileNameFromFontName(const _TCHAR *font_name, _TCHAR *file_name, int size)
{
	LSTATUS lsts;
	HKEY hKey = NULL;
	lsts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"), 0
		, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE
		| KEY_WOW64_32KEY 
		, &hKey);
	if (lsts != ERROR_SUCCESS) {
		return false;
	}
	bool rc = false;
	for(DWORD i=0; ; i++) {
		_TCHAR key[_MAX_PATH];
		BYTE value[_MAX_PATH];
		DWORD klen = _MAX_PATH;
		DWORD vlen = _MAX_PATH;
		DWORD type = 0;
		lsts = RegEnumValue(hKey, i, key, &klen, NULL, &type, value, &vlen);
		if (lsts != ERROR_SUCCESS) {
			break;
		}
		if (klen == 0) {
			continue;
		}
		if (_tcsstr(key, font_name) != NULL) {
			CTchar cfile_name((const char *)value, (int)vlen);
			UTILITY::tcscpy(file_name, size, cfile_name.Get());
			rc = true;
			break;
		}
	}
	RegCloseKey(hKey);
	return rc;
}

/// @brief フォント名の別名を得る
/// レジストリを検索
bool CFont::GetMappedFontName(const _TCHAR *font_name, _TCHAR *new_font_name, int size)
{
	LSTATUS lsts;
	HKEY hKey = NULL;
	lsts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontMapperFamilyFallback"), 0
		, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE
		| KEY_WOW64_32KEY 
		, &hKey);
	if (lsts != ERROR_SUCCESS) {
		return false;
	}
	bool rc = false;
	for(DWORD i=0; ; i++) {
		_TCHAR key[_MAX_PATH];
		BYTE value[_MAX_PATH];
		DWORD klen = _MAX_PATH;
		DWORD vlen = _MAX_PATH;
		DWORD type = 0;
		lsts = RegEnumValue(hKey, i, key, &klen, NULL, &type, value, &vlen);
		if (lsts != ERROR_SUCCESS) {
			break;
		}
		if (klen == 0) {
			continue;
		}
		if (_tcsstr(key, font_name) != NULL) {
			CTchar cnew_font_name((const char *)value, (int)vlen);
			UTILITY::tcscpy(new_font_name, size, cnew_font_name.Get());
			rc = true;
			break;
		}
	}
	RegCloseKey(hKey);
	return rc;
}

/// @brief ファイル名からフォント名を得る
/// レジストリを検索
bool CFont::GetFontNameFromFileName(const _TCHAR *file_name, _TCHAR *font_name, int size)
{
	LSTATUS lsts;
	HKEY hKey = NULL;
	lsts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"), 0
		, KEY_READ
		| KEY_WOW64_32KEY
		, &hKey);
	if (lsts != ERROR_SUCCESS) {
		return false;
	}
	bool rc = false;
	for(DWORD i=0; ; i++) {
		_TCHAR key[_MAX_PATH];
		BYTE value[_MAX_PATH];
		DWORD klen = _MAX_PATH;
		DWORD vlen = _MAX_PATH;
		DWORD type = 0;
		lsts = RegEnumValue(hKey, i, key, &klen, NULL, &type, value, &vlen);
		if (lsts != ERROR_SUCCESS) {
			break;
		}
		if (vlen == 0) {
			continue;
		}
		CTchar cvalue((const char *)value, (int)vlen);
		if (cvalue.MatchString(file_name)) {
			UTILITY::tcscpy(font_name, size, key);
			rc = true;
			break;
		}
	}
	RegCloseKey(hKey);
	return rc;
}
#endif
