/** @file cchar.cpp

	@author Sasaji
	@date   2014.12.01

	@brief 文字列ポインタリスト
*/

#include "cchar.h"
#include "common.h"
#ifdef _WIN32
#include "depend.h"
#include <windows.h>
#endif
#include <ctype.h>
#include <wctype.h>
#include "utility.h"

//
//
//
//
CNchar::CNchar() : CCharTemp<char>()
{
	init();
}
CNchar::CNchar(const CNchar &src) : CCharTemp<char>(src)
{
	init();
}
CNchar::CNchar(const char *src_str) : CCharTemp<char>(src_str, src_str ? (int)strlen(src_str) : 0)
{
	init();
}
CNchar::CNchar(const char *src_str, int src_len) : CCharTemp<char>(src_str, src_len)
{
	init();
}
CNchar::CNchar(const wchar_t *src_str) : CCharTemp<char>()
{
	init();
	this->SetW(src_str); 
}
CNchar::CNchar(const wchar_t *src_str, int src_len) : CCharTemp<char>()
{
	init();
	this->SetW(src_str, src_len); 
}
CNchar::CNchar(int size) : CCharTemp<char>(size)
{
	init();
}
CNchar::~CNchar()
{
	term();
}
/// @brief 初期化
void CNchar::init()
{
#if defined(USE_UTF8_ON_MBCS)
	m_mbuf = NULL;
#endif
	m_wbuf = NULL;
}
/// @brief 後処理
void CNchar::term()
{
#if defined(USE_UTF8_ON_MBCS)
	delete m_mbuf;
	m_mbuf = NULL;
#endif
	delete m_wbuf;
	m_wbuf = NULL;
}

/// @brief 文字列をセット
/// @param[in] src_str 値
void CNchar::Set(const char *src_str)
{
	CCharTemp<char>::Set(src_str, src_str ? (int)strlen(src_str) : 0);
}
/// @brief 文字列をセット
/// @param[in] src_str 値
void CNchar::SetN(const char *src_str)
{
	Set(src_str);
}
/// @brief 文字列をセット
/// @param[in] src_str 値
/// @param[in] src_len 長さ
void CNchar::SetN(const char *src_str, int src_len)
{
	CCharTemp<char>::Set(src_str, src_len);
}
/// @brief 文字列が一致するか
/// @param[in] value 値
/// @return true / false
bool CNchar::MatchString(const char *value) const
{
	if (m_str != NULL && value != NULL && strcmp(m_str, value) == 0) {
		return true;
	}
	return false;
}
/// @brief 文字列の一部が一致するか
/// @param[in] value 値
/// @return true / false
bool CNchar::MatchSubString(const char *value) const
{
	if (m_str != NULL && value != NULL && (strstr(m_str, value) != NULL || strstr(value, m_str) != NULL)) {
		return true;
	}
	return false;
}
#if 0
/// @brief 文字列を代入
/// @param[in] src 値
/// @return 文字列
CNchar &CNchar::operator=(const CNchar &src)
{
	CCharTemp<char>::Set(src);
	return *this;
}
/// @brief 文字列をセット
/// @param[in] src_str 値
/// @return 文字列
CNchar &CNchar::operator=(const char *src_str)
{
	CCharTemp<char>::Set(src_str, src_str ? (int)strlen(src_str) : 0);
	return *this;
}
#endif
/// @brief 文字列を返す
/// @return 文字列
const char *CNchar::GetN() const
{
	return Get();
}
/// @brief 文字列をコピー
/// @param[out] dst_str コピー先
/// @param[in]  dst_len コピー先バッファサイズ
void CNchar::GetN(char *dst_str, int dst_len) const
{
	memset(dst_str, 0, sizeof(char) * dst_len);
	UTILITY::strncpy(dst_str, dst_len, Get(), Length());
}
/// @brief 文字列をセット(変換あり)
/// @param[in] src_str ワイド文字
/// @return true / false
void CNchar::SetW(const wchar_t *src_str)
{
	int src_len = (int)wcslen(src_str);
	SetW(src_str, src_len);
}
/// @brief 文字列をセット(変換あり)
/// @param[in]  src_str ワイド文字
/// @param[in]  src_len ワイド文字長さ
/// @attention Windows only
void CNchar::SetW(const wchar_t *src_str, int src_len)
{
	CNchar buf(src_len * 4);
	int len = UTILITY::conv_wcs_to_mbs(src_str, src_len, buf.Ptr(), src_len * 4);
	CopyData(buf.Get(), len);
}
/// @brief MBCS->ワイド文字に変換して返す
/// @return 文字列へのポインタ
/// @attention Windows only
const wchar_t *CNchar::GetW()
{
	delete m_wbuf;
	m_wbuf = new CWchar(Length());
	UTILITY::conv_mbs_to_wcs(Get(), Length(), m_wbuf->Ptr(), Length());
	return m_wbuf->Get();
}
/// @brief MBCS->ワイド文字に変換してコピー
/// @param[out] dst_str コピー先
/// @param[in]  dst_len コピー先バッファサイズ
void CNchar::GetW(wchar_t *dst_str, int dst_len) const
{
	UTILITY::conv_mbs_to_wcs(Get(), Length(), dst_str, dst_len);
}

/// @brief UTF-8文字をMBCSに変換して渡す
/// @note Windows MBCS environment only
const char *CNchar::GetM()
{
#if defined(USE_UTF8_ON_MBCS)
	delete m_mbuf;
	m_mbuf = new CNchar(Length() * 2);
	UTILITY::conv_utf8_to_mbs(Get(), Length(), m_mbuf->Ptr(), Length() * 2);
	return m_mbuf->Get();
#else
	return Get();
#endif
}

/// @brief MBCSをUTF-8文字に変換してセット
/// @note Windows MBCS environment only
void CNchar::SetM(const char *src_str)
{
#if defined(USE_UTF8_ON_MBCS)
	int src_len = (int)strlen(src_str);
	CNchar buf(src_len * 2);
	UTILITY::conv_mbs_to_utf8(src_str, src_len, buf.Ptr(), src_len * 2);
	Set(buf.m_str);
#else
	Set(src_str);
#endif
}

/// @brief MBCS/UTF-8->ワイド文字に変換して返す
/// @return 文字列へのポインタ
/// @attention Windows only
const wchar_t *CNchar::GetWM()
{
	delete m_wbuf;
	m_wbuf = new CWchar(Length());
#if defined(USE_WIN)
	// MBCS -> WideChar
	UTILITY::conv_mbs_to_wcs(Get(), Length(), m_wbuf->Ptr(), Length());
#else
	// UTF-8 -> WideChar
	UTILITY::conv_utf8_to_wcs(Get(), Length(), m_wbuf->Ptr(), Length());
#endif
	return m_wbuf->Get();
}

/// @brief 大文字にする
void CNchar::ToUpper()
{
	for(int i = 0; i < m_len; i++) {
		m_str[i] = toupper(m_str[i]);
	}
}

/// @brief 小文字にする
void CNchar::ToLower()
{
	for(int i = 0; i < m_len; i++) {
		m_str[i] = tolower(m_str[i]);
	}
}

//
//
//
//
CWchar::CWchar() : CCharTemp<wchar_t>()
{
	init();
}
CWchar::CWchar(const CWchar &src) : CCharTemp<wchar_t>(src)
{
	init();
}
CWchar::CWchar(const wchar_t *src_str) : CCharTemp<wchar_t>(src_str, src_str ? (int)wcslen(src_str) : 0)
{
	init();
}
CWchar::CWchar(const wchar_t *src_str, int src_len) : CCharTemp<wchar_t>(src_str, src_len)
{
	init();
}
CWchar::CWchar(int size) : CCharTemp<wchar_t>(size)
{
	init();
}
CWchar::~CWchar()
{
	term();
}
/// @brief 初期化
void CWchar::init()
{
	m_nbuf = NULL;
}
/// @brief 後処理
void CWchar::term()
{
	delete m_nbuf;
	m_nbuf = NULL;
}

/// @brief 文字列をセット(変換あり)
/// @param[in] src_str 値
/// @return true / false
CWchar::CWchar(const char *src_str)
{
	init();
	SetN(src_str);
}
/// @brief 文字列をセット(変換あり)
/// @param[in] src_str 値
/// @param[in] src_len 長さ
/// @return true / false
CWchar::CWchar(const char *src_str, int src_len)
{
	init();
	SetN(src_str, src_len);
}
/// @brief 文字列をセット
/// @param[in] src_str 値
/// @return true / false
void CWchar::Set(const wchar_t *src_str)
{
	CCharTemp<wchar_t>::Set(src_str, src_str ? (int)wcslen(src_str) : 0);
}
/// @brief 文字列を取得
/// @return 文字列
const wchar_t *CWchar::GetW() const
{
	return Get();
}
/// @brief 文字列をコピー
/// @param[out] dst_str コピー先
/// @param[in]  dst_len コピー先バッファサイズ
void CWchar::GetW(wchar_t *dst_str, int dst_len) const
{
	memset(dst_str, 0, sizeof(wchar_t) * dst_len);
	UTILITY::wcsncpy(dst_str, dst_len, Get(), Length());
}

/// @brief 文字列をセット(変換あり)
/// @param[in] src_str 値
/// @return true / false
void CWchar::SetN(const char *src_str)
{
	int src_len = (int)strlen(src_str);
	SetN(src_str, src_len);
}
/// @brief 文字列をセット(変換あり)
/// @param[in] src_str 値
/// @param[in] src_len 長さ
/// @return true / false
void CWchar::SetN(const char *src_str, int src_len)
{
	CWchar buf(src_len);
	int len = UTILITY::conv_mbs_to_wcs(src_str, src_len, buf.Ptr(), src_len);
	CopyData(buf.Get(), len);
}
/// @brief 文字列を取得(変換あり)
/// @param[in] codepage コードページ
/// @return 文字列
const char *CWchar::GetN(int codepage)
{
	delete m_nbuf;
	m_nbuf = new CNchar(Length() * 4);
#ifdef _WIN32
	UINT acp = (codepage < 0 ? GetACP() : codepage);
	// convert to narrow char from wide char
	// copy data
	WideCharToMultiByte(acp, 0, Get(), Length(), m_nbuf->Ptr(), Length() * 4, NULL, NULL);
#else
	UTILITY::conv_wcs_to_utf8(Get(), Length(), m_nbuf->Ptr(), Length() * 4);
#endif
	return m_nbuf->Get();
}
/// @brief 文字列をコピー(変換あり)
/// @param[out] dst_str コピー先
/// @param[in]  dst_len コピー先バッファサイズ
/// @param[in] codepage コードページ
void CWchar::GetN(char *dst_str, int dst_len, int codepage) const
{
#ifdef _WIN32
	UINT acp = (codepage < 0 ? GetACP() : codepage);
	WideCharToMultiByte(acp, 0, Get(), Length(), dst_str, dst_len, NULL, NULL);
#else
	UTILITY::conv_wcs_to_utf8(Get(), Length(), dst_str, dst_len);
#endif
}
/// @brief 文字列が一致するか
/// @param[in] value 値
/// @return true / false
bool CWchar::MatchString(const wchar_t *value) const
{
	if (m_str != NULL && wcscmp(m_str, value) == 0) {
		return true;
	}
	return false;
}
/// @brief 文字列の一部が一致するか
/// @param[in] value 値
/// @return true / false
bool CWchar::MatchSubString(const wchar_t *value) const
{
	if (m_str != NULL && (wcsstr(m_str, value) != NULL || wcsstr(value, m_str) != NULL)) {
		return true;
	}
	return false;
}
#if 0
/// @brief 文字列を代入
/// @param[in] src 値
/// @return 文字列
CWchar &CWchar::operator=(const CWchar &src)
{
	CCharTemp<wchar_t>::Set(src);
	return *this;
}
/// @brief 文字列をセット
/// @param[in] src_str 値
/// @return 文字列
CWchar &CWchar::operator=(const wchar_t *src_str)
{
	CCharTemp<wchar_t>::Set(src_str, src_str ? (int)wcslen(src_str) : 0);
	return *this;
}
#endif
const wchar_t *CWchar::GetM() const
{
	return Get();
}
void CWchar::SetM(const wchar_t *src_str)
{
	Set(src_str);
}
const wchar_t *CWchar::GetWM() const
{
	return Get();
}

/// @brief 大文字にする
void CWchar::ToUpper()
{
	for(int i = 0; i < m_len; i++) {
		m_str[i] = towupper(m_str[i]);
	}
}

/// @brief 小文字にする
void CWchar::ToLower()
{
	for(int i = 0; i < m_len; i++) {
		m_str[i] = towlower(m_str[i]);
	}
}
