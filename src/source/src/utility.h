/** @file utility.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.04.01 -

	@brief [ utility ]
*/

#ifndef UTILITY_H
#define UTILITY_H

#include "common.h"
#include <stdio.h>
#include <stdarg.h>

class CurTime;

namespace UTILITY
{
	// ----------------------------------------------------------------------
	// file path conversion

	bool check_file_extension(const _TCHAR *file_path, const _TCHAR *ext);
	bool check_file_extensions(const _TCHAR *file_path, ...);
	bool check_file_extensions(const _TCHAR *file_path, const char *exts);

	void convert_path_separator(_TCHAR *path);
	void add_path_separator(_TCHAR *path, size_t maxlen = _MAX_PATH);
	void get_parent_dir(_TCHAR *path);
	void get_ancestor_dir(_TCHAR *path, int ancestor_num);

	void get_dir_and_basename(const _TCHAR *path, _TCHAR *dir, _TCHAR *name, size_t dir_size = _MAX_PATH, size_t name_size = _MAX_PATH);
	void get_dir_and_basename(_TCHAR *path, _TCHAR *name, size_t name_size = _MAX_PATH);

	_TCHAR *trim_center(const _TCHAR *str, int maxlen);

	bool make_relative_path(const _TCHAR *base_path, _TCHAR *path, size_t path_size = _MAX_PATH);
	bool make_absolute_path(const _TCHAR *base_path, _TCHAR *path, size_t path_size = _MAX_PATH);
	void get_long_full_path_name(const _TCHAR *src, _TCHAR *dst, size_t dst_size = _MAX_PATH);

	void slim_path(const _TCHAR *path, _TCHAR *new_path, size_t maxlen, int *parent_num = NULL);
//	_TCHAR *slim_path(const _TCHAR *path, int *parent_num = NULL);

	int copy_path(_TCHAR *dst, const _TCHAR *src, size_t maxSize);

	size_t chomp_crlf(char *str);
	size_t chomp_crlf(wchar_t *str);

	void create_date_file_path(const _TCHAR *dir, _TCHAR *file_path, size_t maxlen, const char *extensions);

	// ----------------------------------------------------------------------
	// string conversion

	int  conv_wcs_to_utf8(const wchar_t *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);
	int  conv_utf8_to_wcs(const char *srcStart, int srcMaxSize, wchar_t *dstStart, int dstMaxSize);

	int  conv_wcs_to_mbs(const wchar_t *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);
	int  conv_mbs_to_wcs(const char *srcStart, int srcMaxSize, wchar_t *dstStart, int dstMaxSize);

#ifdef _UNICODE
	/* _UNICODE */

	/// native string to UTF-8 string
	void wconv_from_native_path(const wchar_t *src, wchar_t *dst, int maxSize = _MAX_PATH);
	/// native string to UTF-8 string
	void cconv_from_native_path(const wchar_t *src, char *dst, int maxSize = _MAX_PATH);
	/// native string to UTF-8 string
	int  tcs_to_mbs(char *dst, const wchar_t *src, int maxSize = _MAX_PATH);
	/// UTF-8 string to native string
	void conv_to_native_path(const wchar_t *src, wchar_t *dst, int maxSize = _MAX_PATH);
	/// UTF-8 string to native string
	void conv_to_native_path(const char *src, wchar_t *dst, int maxSize = _MAX_PATH);

#define conv_from_native_path wconv_from_native_path

#else	/* !_UNICODE */

	int  conv_mbs_to_utf8(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);
	int  conv_utf8_to_mbs(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);

	/// native string to UTF-8 string (under SDL only)
	int  conv_from_api_string(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);
	/// UTF-8 string to native string (under SDL only)
	int  conv_to_api_string(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize);
	/// native string to UTF-8 string
	int  tcs_to_mbs(char *dst, const char *src, int maxSize = _MAX_PATH);
	/// native string to UTF-8 string (under Windows only)
	void cconv_from_native_path(const char *src, char *dst, int maxSize = _MAX_PATH);
	/// UTF-8 string to native string (under Windows only)
	void conv_to_native_path(const char *src, char *dst, int maxSize = _MAX_PATH);

#define conv_from_native_path cconv_from_native_path

#endif	/* _UNICODE */

	// ----------------------------------------------------------------------
	// string format

#ifdef _UNICODE
	const wchar_t *concat(const wchar_t *src, ...);
	void concatv(wchar_t *dst, size_t max_len, const wchar_t *src, va_list ap);
	void concat(wchar_t *dst, size_t max_len, const wchar_t *src, ...);
	void wcscpy(wchar_t *dst, size_t max_len, const wchar_t *src);
	void wcscat(wchar_t *dst, size_t max_len, const wchar_t *src);
	void wcsncpy(wchar_t *dst, size_t max_len, const wchar_t *src, size_t src_count);
	void wcsncat(wchar_t *dst, size_t max_len, const wchar_t *src, size_t src_count);

#endif /* _UNICODE */
	const char *concat(const char *src, ...);
	void concatv(char *dst, size_t max_len, const char *src, va_list ap);
	void concat(char *dst, size_t max_len, const char *src, ...);

	void strcpy(char *dst, size_t max_len, const char *src);
	void strcat(char *dst, size_t max_len, const char *src);
	void strncpy(char *dst, size_t max_len, const char *src, size_t src_count);
	void strncat(char *dst, size_t max_len, const char *src, size_t src_count);

#ifdef _UNICODE
#define tcscpy(dst, max_len, src) wcscpy(dst, max_len, src)
#define tcscat(dst, max_len, src) wcscat(dst, max_len, src)
#define tcsncpy(dst, max_len, src, src_count) wcsncpy(dst, max_len, src, src_count)
#define tcsncat(dst, max_len, src, src_count) wcsncat(dst, max_len, src, src_count)
#else
#define tcscpy(dst, max_len, src) strcpy(dst, max_len, src)
#define tcscat(dst, max_len, src) strcat(dst, max_len, src)
#define tcsncpy(dst, max_len, src, src_count) strncpy(dst, max_len, src, src_count)
#define tcsncat(dst, max_len, src, src_count) strncat(dst, max_len, src, src_count)
#endif

	void conv_format(const _TCHAR *format, _TCHAR *wformat, size_t wsize);

	int vftprintf(FILE *fp, const _TCHAR *format, va_list ap);

	int sntprintf(_TCHAR *dest, size_t count, const _TCHAR *format, ...);

	int stprintf(_TCHAR *dest, size_t count, const _TCHAR *format, ...);
	int vstprintf(_TCHAR *dest, size_t count, const _TCHAR *format, va_list ap);

	int sprintf(char *dest, size_t count, const char *format, ...);
	int vsprintf(char *dest, size_t count, const char *format, va_list ap);

//	int sscanf(const char *source, const char *format, ...);
//	int vsscanf(const char *source, const char *format, va_list ap);

	int sprintf_utf8(char *dest, size_t count, const char *format, ...);
	int vsprintf_utf8(char *dest, size_t count, const char *format, va_list ap);

	int utf8firstpos(const char *str);

	/// substring in last circle bracket
	int substr_in_bracket(const _TCHAR *src, _TCHAR *dst, int maxSize = _MAX_PATH);

#ifdef _UNICODE
	wchar_t *lskip(wchar_t *str);
	const wchar_t *lskip(const wchar_t *str);
	void rtrim(wchar_t *str, const wchar_t *sep = NULL);

	int  get_token(const wchar_t *str, int start_pos, int max_len, const wchar_t sep, int &st, int &ed);
	int  get_token(const wchar_t *str, int start_pos, int max_len, wchar_t *word, int word_max_len, const wchar_t sep = _T(','), int *word_len = NULL);
	wchar_t *wcstok(wchar_t *str, const wchar_t *delimiter, wchar_t **next);
	int  get_parameters(wchar_t *str, int max_len, wchar_t **params, int max_params);
	int  get_parameters(const wchar_t *str, int max_len, const wchar_t **params, int max_params, int *paramslen);
#endif /* _UNICODE */
	char *lskip(char *str);
	const char *lskip(const char *str);
	void rtrim(char *str, const char *sep = NULL);

	int  get_token(const char *str, int start_pos, int max_len, const char sep, int &st, int &ed);
	int  get_token(const char *str, int start_pos, int max_len, char *word, int word_max_len, const char sep = _T(','), int *word_len = NULL);
	char *strtok(char *str, const char *delimiter, char **next);
	int  get_parameters(char *str, int max_len, char **params, int max_params);
	int  get_parameters(const char *str, int max_len, const char **params, int max_params, int *paramslen);

#ifdef _UNICODE
#define tcstok(str, delimiter, next) wcstok(str, delimiter, next)
#else
#define tcstok(str, delimiter, next) strtok(str, delimiter, next)
#endif

	uint16_t sjis_to_jis(uint16_t code);

};

#endif /* UTILITY_H */
