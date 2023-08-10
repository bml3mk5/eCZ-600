/** @file utility.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.04.01 -

	@brief [ utility ]
*/

#include "utility.h"
#include <stdio.h>
#include <ctype.h>
#include "curtime.h"
#if !defined(_WIN32)
#include "ConvertUTF.h"
#endif
#include <wchar.h>

#ifdef _WIN32
// #define USE_WINDOWS_API
#include "depend.h"
#include <Windows.h>
#include <Shlwapi.h>

#define PATH_SEP_CHAR1	_T('\\')
#define PATH_SEP_STR1	_T("\\")
#define PATH_GPARENT_STR1	_T("...\\")
#define PATH_PARENT_STR1	_T("..\\")
#define PATH_CURRENT_STR1	_T(".\\")

#define PATH_SEP_CHAR2	_T('/')
#define PATH_SEP_STR2	_T("/")
#define PATH_GPARENT_STR2	_T(".../")
#define PATH_PARENT_STR2	_T("../")
#define PATH_CURRENT_STR2	_T("./")

#else	/* !_WIN32 */

#define PATH_SEP_CHAR1	_T('/')
#define PATH_SEP_STR1	_T("/")
#define PATH_GPARENT_STR1	_T(".../")
#define PATH_PARENT_STR1	_T("../")
#define PATH_CURRENT_STR1	_T("./")

#define PATH_SEP_CHAR2	_T('\\')
#define PATH_SEP_STR2	_T("\\")
#define PATH_GPARENT_STR2	_T("...\\")
#define PATH_PARENT_STR2	_T("..\\")
#define PATH_CURRENT_STR2	_T(".\\")

#endif

namespace UTILITY
{

// ----------------------------------------------------------------------
// file path conversion

/**
 * @brief match extension
 *
 * @param [in] file_path
 * @param [in] ext
 * @return true when include ext in file_path
 */
bool check_file_extension(const _TCHAR* file_path, const _TCHAR* ext)
{
	int nam_len = (int)_tcslen(file_path);
	int ext_len = (int)_tcslen(ext);

#if defined(_WIN32) && !defined(NO_USE_WINAPI)
	if(nam_len >= ext_len && _tcsnicmp(&file_path[nam_len - ext_len], ext, ext_len) == 0) {
#else
#ifdef _UNICODE
	if(nam_len >= ext_len && wcsncasecmp(&file_path[nam_len - ext_len], ext, ext_len) == 0) {
#else
	if(nam_len >= ext_len && strncasecmp(&file_path[nam_len - ext_len], ext, ext_len) == 0) {
#endif
#endif
		return true;
	}
	return false;
}

/**
 * @brief match one and more in extension list
 *
 * @param [in] file_path
 * @param [in] ...       extension (const _TCHAR *) list which is terminated by NULL
 * @return true when include ext in file_path
 */
bool check_file_extensions(const _TCHAR* file_path, ...)
{
	bool rc = false;
	const _TCHAR *ext = NULL;

	va_list ap;
	va_start(ap, file_path);
	while(!rc && (ext = va_arg(ap, const _TCHAR *)) != NULL) {
		rc = check_file_extension(file_path, ext);
	}
	va_end(ap);
	return rc;
}

/**
 * @brief match one and more in extension list
 *
 * @param [in] file_path
 * @param [in] exts       extension list such as "foo;bar;baz"
 * @return true when include ext in file_path
 */
bool check_file_extensions(const _TCHAR *file_path, const char *exts)
{
	bool rc = false;

	char ext[8];
	int len = (int)strlen(exts);
	int pos = 0;
	int ext_len;
	strcpy(ext, 8, ".");
	do {
		ext_len = 0;
		pos = get_token(exts, pos, len, &ext[1], 7, ';', &ext_len);
		if (ext_len >= 0) {
#ifdef _UNICODE
			wchar_t wext[8];
			conv_mbs_to_wcs(ext, 8, wext, 8);
			rc = check_file_extension(file_path, wext);
#else
			rc = check_file_extension(file_path, ext);
#endif
		}
	} while(pos >= 0 && !rc);

	return rc;
}

/**
 * @brief convert separator chars in path
 *
 * @param [in,out] path
 */
void convert_path_separator(_TCHAR *path)
{
	size_t len = _tcslen(path);

	for(size_t i=0; i<len; i++) {
		if (path[i] == PATH_SEP_CHAR2) {
			path[i]=PATH_SEP_CHAR1;
		}
	}
}

/**
 * @brief add a separator char in the end of path
 *
 * @param [in,out] path
 * @param [in]     maxlen : buffer size of path
 */
void add_path_separator(_TCHAR *path, size_t maxlen)
{
	size_t len = _tcslen(path);

	if (len > 0 && len < (maxlen - 1)) {
		if (path[len-1] != PATH_SEP_CHAR1) {
			path[len]=PATH_SEP_CHAR1;
			path[len+1]=_T('\0');
		}
	}
}

/**
 * @brief leave parent dir in path
 *
 * @param [in,out] path
 */
void get_parent_dir(_TCHAR *path)
{
	int pt = (int)_tcslen(path);
	while(pt >= 0 && path[pt] != PATH_SEP_CHAR1) {
		pt--;
	}
	path[pt + 1] = _T('\0');
}

/**
 * @brief leave ancestor dir (end of path must be sep char)
 *
 * @param [in,out] path
 * @param [in]     ancestor_num : ancestor number
 */
void get_ancestor_dir(_TCHAR *path, int ancestor_num)
{
	int pt = (int)_tcslen(path);
	int num = 0;

	ancestor_num++;
	while(pt > 0 && num < ancestor_num) {
		pt--;
		if (path[pt] == PATH_SEP_CHAR1) num++;
	}
	if (num == ancestor_num) path[pt + 1] = _T('\0');
}

/**
 * @brief get dirname and basename (pattern 1)
 *
 * @param [in]  path
 * @param [out] dir  : dir name
 * @param [out] name : base name
 * @param [in]  dir_size  : buffer size of dir
 * @param [in]  name_size : buffer size of name
 */
void get_dir_and_basename(const _TCHAR *path, _TCHAR *dir, _TCHAR *name, size_t dir_size, size_t name_size)
{
	const _TCHAR *p = NULL;

	if (dir != NULL)  {
		*dir = _T('\0');
	}
	if (name != NULL) {
		*name = _T('\0');
	}
	p = _tcsrchr(path, PATH_SEP_CHAR1);
	if (p != NULL) {
		if (dir  != NULL) {
			UTILITY::tcsncpy(dir, dir_size, path, p-path+1);
		}
		if (name != NULL) {
			UTILITY::tcscpy(name, name_size, p+1);
		}
	} else {
		if (name != NULL) {
			UTILITY::tcscpy(name, name_size, path);
		}
	}
	return;
}

/**
 * @brief get dirname and basename (pattern 2)
 *
 * @param [in,out] path : leave dir name (trim base name)
 * @param [out]    name : base name
 * @param [in]     name_size : buffer size of name
 */
void get_dir_and_basename(_TCHAR *path, _TCHAR *name, size_t name_size)
{
	_TCHAR *p = NULL;

	if (name != NULL) {
		*name = _T('\0');
	}
	p = _tcsrchr(path, PATH_SEP_CHAR1);
	if (p != NULL) {
		if (name != NULL) {
			UTILITY::tcscpy(name, name_size, p+1);
		}
		*(p+1) = _T('\0');
	} else {
		if (name != NULL) {
			UTILITY::tcscpy(name, name_size, path);
		}
		*path = _T('\0');
	}
	return;
}

/**
 * @brief trim center of long string
 *
 * @param [in] str
 * @param [in] maxlen : maximum length of trimmed string
 * @return trimmed string
 *
 * @attention this function is not thread safe.
 */
_TCHAR *trim_center(const _TCHAR *str, int maxlen)
{
	static _TCHAR buff[_MAX_PATH];

	int srclen = (int)_tcslen(str);
	if (srclen > maxlen + 4) {
		int len = maxlen / 2 - 2;
#if !defined(_UNICODE) && !defined(USE_WIN)
		len += utf8firstpos(&str[len-1]);
#endif
		UTILITY::tcsncpy(buff, _MAX_PATH, str, len);
		UTILITY::tcscat(buff, _MAX_PATH, _T("..."));
		int pos = srclen - len;
#if !defined(_UNICODE) && !defined(USE_WIN)
		pos += utf8firstpos(&str[pos]);
#endif
		UTILITY::tcscat(buff, _MAX_PATH, &str[pos]);
	} else {
		UTILITY::tcscpy(buff, _MAX_PATH, str);
	}
	return buff;
}

/**
 * @brief make relative path if match base_path
 *
 * @param [in] base_path
 * @param [in,out] path
 * @param [in] path_size
 * @return true
 *
 */
bool make_relative_path(const _TCHAR *base_path, _TCHAR *path, size_t path_size)
{
	_TCHAR new_path[_MAX_PATH];

#ifdef USE_WINDOWS_API	// Use Windows API //
	if (!PathRelativePathTo(new_path, base_path, FILE_ATTRIBUTE_DIRECTORY, path, 0)) {
		return false;
	}
	UTILITY::tcscpy(path, path_size, new_path);

#else	// ! Use Windows API //
	_TCHAR s_base_path[_MAX_PATH];
	_TCHAR s_path[_MAX_PATH];

	slim_path(base_path, s_base_path, _MAX_PATH);
	slim_path(path, s_path, _MAX_PATH);

	int len1 = (int)_tcslen(s_base_path);

	if (len1 == 0) {
		return true;
	}
	if (_tcsncmp(s_path, s_base_path, 2) != 0) {
		// if first 2 chars are different, stop convert it
		return true;
	}

	const _TCHAR *p1st = s_base_path;
	const _TCHAR *p1ed = NULL;
	_TCHAR *p2st = s_path;
	int parent = 0;

	while (p1st < s_base_path + len1 && (p1ed = _tcschr(p1st, PATH_SEP_CHAR1)) != NULL) {
		int len_part = (int)(p1ed - s_base_path + 1);
		if (_tcsncmp(s_path, s_base_path, len_part) == 0) {
			// same
			p2st = s_path + len_part;
		} else {
			// different
			parent++;
		}
		p1st = p1ed + 1;
	}

	// add relative path
	memset(new_path, 0, sizeof(new_path));
	for(; parent > 0; parent--) {
		UTILITY::tcscat(new_path, _MAX_PATH, PATH_PARENT_STR1);
	}
	UTILITY::tcscat(new_path, _MAX_PATH, p2st);

	UTILITY::tcscpy(path, path_size, new_path);

#endif	// end of Use Windows API //
	return true;
}

/**
 * @brief make absolute path
 *
 * @param [in] base_path
 * @param [in,out] path
 * @param [in] path_size
 * @return true
 *
 */
bool make_absolute_path(const _TCHAR *base_path, _TCHAR *path, size_t path_size)
{
	int len1 = (int)_tcslen(base_path);

	if (len1 == 0) {
		return false;
	}

	_TCHAR new_path[_MAX_PATH];
	_TCHAR p2st[_MAX_PATH];

#ifdef USE_WINDOWS_API	// Use Windows API //
	if (!PathCombine(new_path, base_path, path)) {
		return false;
	}
	UTILITY::tcscpy(path, path_size, new_path);

#else	// ! Use Windows API //

#ifdef _WIN32
	// drive letter or root
	if ((_istalpha(path[0]) && path[1] == _T(':')) || path[0] == PATH_SEP_CHAR1) {
#else
	// root
	if (path[0] == PATH_SEP_CHAR1) {
#endif
		return true;
	}

	slim_path(base_path, new_path, _MAX_PATH);

	int parent = 0;
	slim_path(path, p2st, _MAX_PATH, &parent);

	// trim base path
	_TCHAR *p1st = NULL;
	for(; parent >= 0; parent--) {
		p1st = _tcsrchr(new_path, PATH_SEP_CHAR1);
		if (p1st == NULL) {
			break;
		}
		*p1st = _T('\0');
	}
	if (p1st == NULL) {
		// parent of root dir ???
		// so root
		p1st = new_path;
#ifdef _WIN32
		// if drive letter
		if (_istalpha(p1st[0]) && p1st[1] == _T(':')) {
			p1st += 2;
		}
#endif
	}
	*p1st = PATH_SEP_CHAR1;
	p1st++;
	*p1st = _T('\0');

	// add
	UTILITY::tcscat(new_path, _MAX_PATH, p2st);
	UTILITY::tcscpy(path, path_size, new_path);
#endif	// end of Use Windows API //
	return true;
}

/**
 * @brief get long full pathname
 *
 * @param [in]  src
 * @param [out] dst
 * @param [in]  dst_size
 */
void get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_size)
{
	_TCHAR tmp[_MAX_PATH];

	memset(tmp, 0, sizeof(tmp));
#ifdef _WIN32
# ifdef _UNICODE
	if(GetFullPathNameW(src, _MAX_PATH, tmp, NULL) == 0)
# else
	if(GetFullPathNameA(src, _MAX_PATH, tmp, NULL) == 0)
# endif
	{
		// not restrict
		UTILITY::tcscpy(tmp, _MAX_PATH, src);
		UTILITY::tcscpy(dst, dst_size, tmp);
	}
# ifdef _UNICODE
	else if(GetLongPathNameW(tmp, dst, _MAX_PATH) == 0)
# else
	else if(GetLongPathNameA(tmp, dst, _MAX_PATH) == 0)
# endif
	{
		UTILITY::tcscpy(dst, dst_size, tmp);
	}
#else
	UTILITY::tcscpy(tmp, _MAX_PATH, src);
	UTILITY::tcscpy(dst, dst_size, tmp);
#endif
}

#if 0
/**
 * @brief slim path
 */
_TCHAR *slim_path(const _TCHAR *path, int *parent_num)
{
	static _TCHAR new_path[_MAX_PATH];
	slim_path(path, new_path, _MAX_PATH, parent_num);
	return new_path;
}
#endif

/**
 * @brief remove "." and ".." in the path
 *
 * @param [in] path         source
 * @param [out] new_path    dest
 * @param [in] maxlen       size of new_path
 * @param [out] parent_num  number of ".." in path  
 */
void slim_path(const _TCHAR *path, _TCHAR *new_path, size_t maxlen, int *parent_num)
{
	const _TCHAR *p2st = path;
//	const _TCHAR *p2ed = path;
	const _TCHAR *p2sep = NULL;
	int len2 = (int)_tcslen(path);
	int parent = 0;
	struct poslen_st {
		int pos;
		int len;
	} child_stack[64];
	int child = 0;

	memset(new_path, 0, maxlen * sizeof(_TCHAR));

	// if root
	if (_tcsncmp(p2st, PATH_SEP_STR1, 1) == 0) {
		// push path
		if (child < 64) {
			child_stack[child].pos = 0;
			child_stack[child].len = 1;
			child++;
		}
		p2st++;
//		p2ed = p2st;
	}

	// parse parent path
	while (p2st < path + len2) {
		if (_tcsncmp(p2st, PATH_GPARENT_STR1, 4) == 0) {
			child-=2;
			if (child < 0) {
				parent-=child;
				child=0;
			}
			p2st += 4;
//			p2ed = p2st;
		} else if (_tcsncmp(p2st, PATH_PARENT_STR1, 3) == 0) {
			child--;
			if (child < 0) {
				parent-=child;
				child=0;
			}
			p2st += 3;
//			p2ed = p2st;
		} else if (_tcsncmp(p2st, PATH_CURRENT_STR1, 2) == 0) {
			p2st += 2;
//			p2ed = p2st;
		} else if (_tcsncmp(p2st, PATH_SEP_STR1, 1) == 0) {
			p2st += 1;
//			p2ed = p2st;
		} else if ((p2sep = _tcschr(p2st, PATH_SEP_CHAR1)) != NULL) {
			if (p2st < p2sep) {
				// push path
				if (child < 64) {
					child_stack[child].pos = (int)(p2st - path);
					child_stack[child].len = (int)(p2sep - p2st + 1);
					child++;
				}
				p2st = p2sep + 1;
			} else {
				break;
			}
		} else {
			break;
		}
	}
	if (parent_num == NULL) {
		for(; parent > 0; parent--) {
			UTILITY::tcsncat(new_path, maxlen, PATH_PARENT_STR1, 3);
		}
	} else {
		*parent_num = parent;
	}
	for(int i=0; i<child; i++) {
		UTILITY::tcsncat(new_path, maxlen, path + child_stack[i].pos, child_stack[i].len);
	}
	UTILITY::tcscat(new_path, maxlen, p2st);
}

/**
 * @brief copy path with cutting out a cr and lf in the end of path
 *
 * @param [out] dst         dest
 * @param [in]  src         source
 * @param [in]  maxSize     buffer size of dest
 * return length of dst
 */
int copy_path(_TCHAR *dst, const _TCHAR *src, size_t maxSize)
{
	memset(dst, 0, sizeof(_TCHAR) * maxSize);

	UTILITY::tcscpy(dst, maxSize, src);

	// chomp cr lf
	return (int)chomp_crlf(dst);
}

/**
 * @brief cut out a cr and lf in the end of string
 *
 * @param [in,out] str
 * return length of str
 */
size_t chomp_crlf(char *str)
{
	// chomp cr lf
	size_t lastpos = strlen(str);
	if (lastpos == 0) return 0;
	do {
		lastpos--;
		if (str[lastpos] == '\n' || str[lastpos] == '\r') {
			str[lastpos] = '\0';
		} else {
			break;
		}
	} while(lastpos > 0);
	return strlen(str);
}

/**
 * @brief cut out a cr and lf in the end of string
 *
 * @param [in,out] str
 * return length of str
 */
size_t chomp_crlf(wchar_t *str)
{
	// chomp cr lf
	size_t lastpos = wcslen(str);
	if (lastpos == 0) return 0;
	do {
		lastpos--;
		if (str[lastpos] == L'\n' || str[lastpos] == L'\r') {
			str[lastpos] = L'\0';
		} else {
			break;
		}
	} while(lastpos > 0);
	return wcslen(str);
}

/**
 * @brief make date and time based file path
 *
 * @param [in] dir : directory (need a path separator in the end of dir) (nullable)
 *             If dir is NULL, append a file name to the end of file_path
 * @param [out] file_path : dir + file name + extension
 * @param [in] maxlen : buffer size of file path
 * @param [in] extensions : extension of file name (such as "txt;doc") (nullable)
 *             Append the first extension of specified parameter to the end of file_path
 */
void create_date_file_path(const _TCHAR *dir, _TCHAR *file_path, size_t maxlen, const char *extensions)
{
	_TCHAR file_name[64];

	char ext[8];
	int ext_len = 0;
	if (extensions) {
		int len = (int)strlen(extensions);
		get_token(extensions, 0, len, ext, 8, ';', &ext_len);
	}
	CurTime cur_time;
	cur_time.GetHostTime();
	cur_time.GetCurrTime();
	UTILITY::stprintf(file_name, 64, _T("%04d-%02d-%02d_%02d-%02d-%02d"),
		cur_time.GetYear(),
		cur_time.GetMonth(),
		cur_time.GetDay(),
		cur_time.GetHour(),
		cur_time.GetMin(),
		cur_time.GetSec()
	);
	if (ext_len > 0) {
		UTILITY::tcscat(file_name, 64, _T("."));
#ifdef _UNICODE
		_TCHAR wext[8];
		UTILITY::conv_mbs_to_wcs(ext, 8, wext, 8);
		UTILITY::tcscat(file_name, 64, wext);
#else
		UTILITY::tcscat(file_name, 64, ext);
#endif
	}
	size_t file_name_len = _tcslen(file_name);
	if (maxlen <= file_name_len) return;

	if (dir != NULL && _tcslen(dir) + file_name_len < maxlen) {
		UTILITY::tcscpy(file_path, maxlen, dir);
	}
	UTILITY::tcscat(file_path, maxlen, file_name);
}

// ----------------------------------------------------------------------
// string conversion

/// @brief convert to UTF-8 from Widechar
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_wcs_to_utf8(const wchar_t *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	// WideChar -> UTF-8
	len = WideCharToMultiByte(CP_UTF8, 0, srcStart, srcMaxSize, dstStart, dstMaxSize, NULL, NULL);
#else
	UTF8   *dst    = (UTF8 *)dstStart;
	UTF8  **dst_st = (UTF8  **)&dst;
	UTF8   *dst_ed = (UTF8  *)(dstStart + dstMaxSize);
	ConversionResult rc = conversionOK;
	if (sizeof(wchar_t) == 4) {
		const UTF32 **src_st = (const UTF32 **)&srcStart;
		const UTF32  *src_ed = (const UTF32  *)(srcStart + srcMaxSize);
		rc = ConvertUTF32toUTF8(src_st, src_ed
		, dst_st, dst_ed, lenientConversion);
		if (rc == conversionOK) {
			**dst_st = 0;
			len = (int)(*dst_st - (UTF8 *)dstStart);
		}
	} else if (sizeof(wchar_t) == 2) {
		const UTF16 **src_st = (const UTF16 **)&srcStart;
		const UTF16  *src_ed = (const UTF16  *)(srcStart + srcMaxSize);
		rc = ConvertUTF16toUTF8(src_st, src_ed
		, dst_st, dst_ed, lenientConversion);
		if (rc == conversionOK) {
			**dst_st = 0;
			len = (int)(*dst_st - (UTF8 *)dstStart);
		}
	}
#endif
	return len;
}

/// @brief convert to Widechar from UTF-8
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_utf8_to_wcs(const char *srcStart, int srcMaxSize, wchar_t *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	// UTF-8 -> WideChar
	len = MultiByteToWideChar(CP_UTF8, 0, srcStart, srcMaxSize, dstStart, dstMaxSize);
#else
	const UTF8  **src_st = (const UTF8 **)&srcStart;
	const UTF8   *src_ed = (const UTF8  *)(srcStart + srcMaxSize);
	ConversionResult rc = conversionOK;
	if (sizeof(wchar_t) == 4) {
		UTF32  *dst    = (UTF32 *)dstStart;
		UTF32 **dst_st = (UTF32 **)&dst;
		UTF32  *dst_ed = (UTF32 *)(dstStart + dstMaxSize);
		rc = ConvertUTF8toUTF32(src_st, src_ed
		, dst_st, dst_ed, lenientConversion);
		if (rc == conversionOK) {
			**dst_st = 0;
			len = (int)(*dst_st - (UTF32 *)dstStart);
		}
	} else if (sizeof(wchar_t) == 2) {
		UTF16  *dst    = (UTF16 *)dstStart;
		UTF16 **dst_st = (UTF16 **)&dst;
		UTF16  *dst_ed = (UTF16 *)(dstStart + dstMaxSize);
		rc = ConvertUTF8toUTF16(src_st, src_ed
		, dst_st, dst_ed, lenientConversion);
		if (rc == conversionOK) {
			**dst_st = 0;
			len = (int)(*dst_st - (UTF16 *)dstStart);
		}
	}
#endif
	return len;
}

/// @brief convert to Widechar from MBCS/UTF-8
///
/// @note MBCS code if under Windows, otherwise UTF-8 code 
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_mbs_to_wcs(const char *srcStart, int srcMaxSize, wchar_t *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	// MBS -> WideChar
	len = MultiByteToWideChar(CP_ACP, 0, srcStart, srcMaxSize, dstStart, dstMaxSize);
#else
	// UTF-8 -> WideChar
	len = conv_utf8_to_wcs(srcStart, srcMaxSize, dstStart, dstMaxSize);
#endif
	return len;
}

/// @brief convert to MBCS from Widechar
///
/// @note MBCS code if under Windows, otherwise UTF-8 code 
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_wcs_to_mbs(const wchar_t *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	// WideChar -> MBS
	len = WideCharToMultiByte(CP_ACP, 0, srcStart, srcMaxSize, dstStart, dstMaxSize, NULL, NULL);
#else
	// WideChar -> UTF-8
	len = conv_wcs_to_utf8(srcStart, srcMaxSize, dstStart, dstMaxSize);
#endif
	return len;
}

#ifdef _UNICODE
	/* _UNICODE */

/// @brief copy wide string
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void wconv_from_native_path(const wchar_t *src, wchar_t *dst, int maxSize)
{
	size_t len = wcslen(src);
	memset(dst, 0, maxSize * sizeof(wchar_t));
	len = (len >= (size_t)maxSize ? (size_t)maxSize-1 : len);
	::wcsncpy(dst, src, len);
	dst[len]=L'\0';
	return;
}

/// @brief convert wide string to UTF-8 string
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void cconv_from_native_path(const wchar_t *src, char *dst, int maxSize)
{
	memset(dst, 0, maxSize);

#ifdef _WIN32
//	int rc = 0;

	// WideChar -> UTF-8
	WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, maxSize, NULL, NULL);
#else
	bool rc = false;
	int len = (int)wcslen(src);
	len = len > maxSize ? maxSize : len;

	// WideChar -> UTF-8
	rc = conv_wcs_to_utf8(src, len, dst, maxSize);
#endif
	return;
}

/// @brief convert from wide char to UTF-8 char on the UNICODE environment
///
/// @param [out] dst
/// @param [in]  src
/// @param [in]  maxSize : buffer size of dst
/// @return length of dest string
int tcs_to_mbs(char *dst, const wchar_t *src, int maxSize)
{
	int len = 0;
	memset(dst, 0, sizeof(char) * maxSize);
	// WideChar -> UTF-8
	len = conv_wcs_to_utf8(src, (int)wcslen(src), dst, maxSize);
	return len;
}

/// @brief copy wide char string and cut off cr and lf
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void conv_to_native_path(const wchar_t *src, wchar_t *dst, int maxSize)
{
	size_t len = wcslen(src);
	memset(dst, 0, maxSize * sizeof(wchar_t));
	len = (len >= (size_t)maxSize ? (size_t)maxSize-1 : len);
	::wcsncpy(dst, src, len);
	dst[len]=L'\0';
	// chomp lf
	chomp_crlf(dst);
	return;
}

/// @brief convert UTF-8 to wide char string and cut off cr and lf
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void conv_to_native_path(const char *src, wchar_t *dst, int maxSize)
{
#ifdef _WIN32
//	int rc = 0;

	// UTF-8 -> WideChar
	MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, maxSize * sizeof(wchar_t));
#else
	bool rc = false;
	int len = (int)strlen(src);
	len = len > maxSize ? maxSize : len;

	// UTF-8 -> WideChar
	rc = conv_utf8_to_wcs(src, len, dst, maxSize);
#endif
	// chomp cr lf
	chomp_crlf(dst);
	return;
}

#else	/* !_UNICODE */

/// @brief convert native string to UTF-8 code string (under Windows only)
///
/// @note no conversion unless under Windows
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_mbs_to_utf8(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	wchar_t *dstTemp = new wchar_t[srcMaxSize + 1];
	// MBS -> WideChar
	len = MultiByteToWideChar(CP_ACP, 0, srcStart, -1, dstTemp, srcMaxSize + 1);
	if (len > 0) {
		// WideChar -> UTF-8
		len = WideCharToMultiByte(CP_UTF8, 0, dstTemp, -1, dstStart, dstMaxSize, NULL, NULL);
	}
	delete [] dstTemp;
#endif
	if (len == 0) {
		// no convert
		if (dstStart != srcStart) {
			memset(dstStart, 0, dstMaxSize);
			::strncpy(dstStart, srcStart, dstMaxSize);
		}
		len = (int)strlen(dstStart);
	}
	return len;
}

/// @brief convert native string to UTF-8 string (under SDL on Windows only)
///
/// @note Treat strings as UTF-8 character codes under SDL, but the Windows API requires MBCS character codes.
/// @note So, you need to convert it before passing it to APIs.
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_from_api_string(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if (defined(USE_SDL) || defined(USE_SDL2)) && defined(_WIN32)
	len = conv_mbs_to_utf8(srcStart, srcMaxSize, dstStart, dstMaxSize);
#else
	if (dstStart != srcStart) {
		memset(dstStart, 0, dstMaxSize);
		::strncpy(dstStart, srcStart, dstMaxSize);
	}
	len = (int)strlen(dstStart);
#endif
	return len;
}

/// @brief convert native string to UTF-8 string (under Windows only)
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void cconv_from_native_path(const char *src, char *dst, int maxSize)
{
	conv_mbs_to_utf8(src, (int)strlen(src), dst, maxSize);
	return;
}

/// @brief copy chars on the ASCII(MBCS) environment
///
/// @param [out] dst
/// @param [in]  src
/// @param [in]  maxSize : buffer size of dst
/// @return length of dest string
int tcs_to_mbs(char *dst, const char *src, int maxSize)
{
	memset(dst, 0, sizeof(char) * maxSize);
	int len = (int)strlen(src);
	len = (len >= maxSize ? maxSize-1 : len);
	::strncpy(dst, src, len);
	return len;
}

/// @brief convert to native code from UTF-8 in string (under Windows only)
///
/// @note no conversion unless under Windows
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int conv_utf8_to_mbs(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if defined(_WIN32)
	wchar_t dstTemp[1024];
	// UTF-8 -> WideChar
	len = MultiByteToWideChar(CP_UTF8, 0, srcStart, -1, dstTemp, 1024);
	if (len > 0) {
		// WideChar -> MBS
		len = WideCharToMultiByte(CP_ACP, 0, dstTemp, -1, dstStart, dstMaxSize, NULL, NULL);
	}
#endif
	if (len == 0) {
		// no convert
		if (srcStart != dstStart) {
			memset(dstStart, 0, dstMaxSize);
			::strncpy(dstStart, srcStart, dstMaxSize);
		}
		len = (int)strlen(dstStart);
	}
	return len;
}

/// @brief UTF-8 string to native string (under SDL on Windows only)
///
/// @note Treat strings as UTF-8 character codes under SDL, but the Windows API requires MBCS character codes.
/// @note So, you need to convert it before passing it to APIs.
///
/// @param [in]  srcStart
/// @param [in]  srcMaxSize : buffer size of srcStart
/// @param [out] dstStart
/// @param [in]  dstMaxSize : buffer size of dstStart
/// @return length of dest string
int  conv_to_api_string(const char *srcStart, int srcMaxSize, char *dstStart, int dstMaxSize)
{
	int len = 0;
#if (defined(USE_SDL) || defined(USE_SDL2)) && defined(_WIN32)
	len = conv_utf8_to_mbs(srcStart, srcMaxSize, dstStart, dstMaxSize);
#else
	if (srcStart != dstStart) {
		memset(dstStart, 0, dstMaxSize);
		::strncpy(dstStart, srcStart, dstMaxSize);
	}
	len = (int)strlen(dstStart);
#endif
	return len;
}

/// @brief convert UTF-8 to native string and cut off cr and lf (under Windows only)
///
/// @param [in]  src
/// @param [out] dst
/// @param [in]  maxSize : buffer size of dst
void conv_to_native_path(const char *src, char *dst, int maxSize)
{
	conv_utf8_to_mbs(src, (int)strlen(src), dst, maxSize);

	// chomp cr lf
	chomp_crlf(dst);
	return;
}

#endif	/* _UNICODE */

// ----------------------------------------------------------------------
// string format

#ifdef _UNICODE

/// @brief concatenate words
///
/// @param[in] src : 1st word
/// @param[in] ... : 2nd and later word
/// @return concatenated string
///
/// @attention need set NULL in last arg
/// @attention This function is not thread safe.
const wchar_t *concat(const wchar_t *src, ...) {
	static wchar_t str[1024];
	va_list ap;
	va_start(ap, src);
	concatv(str, 1024, src, ap);
	va_end(ap);
	return str;

}
/// @brief concatenate words
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : 1st word
/// @param[in] ap : 2nd and later word
///
/// @attention need set NULL in last arg
void concatv(wchar_t *dst, size_t max_len, const wchar_t *src, va_list ap) {
	const wchar_t *src2;
    if (dst != src) UTILITY::wcscpy(dst, max_len, src);
	for(size_t i=1; (src2 = va_arg(ap, const wchar_t *)) != NULL; i++) {
        if (wcslen(dst) + wcslen(src2) >= max_len) break;
        UTILITY::wcscat(dst, max_len, src2);
    }
}
/// @brief concatenate words
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : 1st word
/// @param[in] ... : 2nd and later word
///
/// @attention need set NULL in last
void concat(wchar_t *dst, size_t max_len, const wchar_t *src, ...) {
	va_list ap;
	va_start(ap, src);
	concatv(dst, max_len, src, ap);
	va_end(ap);
}

/// @brief copy string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
void wcscpy(wchar_t *dst, size_t max_len, const wchar_t *src) {
	if (max_len == 0) return;
	size_t src_len = wcslen(src);
	if (max_len <= src_len) src_len = max_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::wcsncpy_s(dst, max_len, src, src_len);
#else
	::wcsncpy(dst, src, src_len);
#endif
	dst[src_len] = L'\0';
}

/// @brief concatenate string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
void wcscat(wchar_t *dst, size_t max_len, const wchar_t *src) {
	if (max_len == 0) return;
	size_t dst_len = wcslen(dst);
	if (max_len <= dst_len) return;
	size_t src_len = wcslen(src);
	if (max_len <= src_len + dst_len) src_len = max_len - dst_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::wcsncat_s(dst, max_len, src, src_len);
#else
	::wcsncat(dst, src, src_len);
#endif
}

/// @brief copy string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
/// @param[in] src_count : source string count
void wcsncpy(wchar_t *dst, size_t max_len, const wchar_t *src, size_t src_count) {
	if (max_len == 0) return;
	if (max_len <= src_count) src_count = max_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::wcsncpy_s(dst, max_len, src, src_count);
#else
	::wcsncpy(dst, src, src_count);
#endif
	dst[src_count] = L'\0';
}

/// @brief concatenate string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
/// @param[in] src_count : source string count
void wcsncat(wchar_t *dst, size_t max_len, const wchar_t *src, size_t src_count) {
	if (max_len == 0) return;
	size_t dst_len = wcslen(dst);
	if (max_len <= dst_len) return;
	if (max_len <= src_count + dst_len) src_count = max_len - dst_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::wcsncat_s(dst, max_len, src, src_count);
#else
	::wcsncat(dst, src, src_count);
#endif
}

#endif /* _UNICODE */

/// @brief concatenate words
///
/// @param[in] src : 1st word
/// @param[in] ... : 2nd and later word
/// @return concatenated string
///
/// @attention need set NULL in last arg
/// @attention This function is not thread safe.
const char *concat(const char *src, ...) {
	static char str[1024];
	va_list ap;
	va_start(ap, src);
	concatv(str, 1024, src, ap);
	va_end(ap);
	return str;
}
/// @brief concatenate words
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : 1st word
/// @param[in] ap : 2nd and later word
///
/// @attention need set NULL in last arg
void concatv(char *dst, size_t max_len, const char *src, va_list ap) {
	const char *src2;
	if (dst != src) UTILITY::strcpy(dst, max_len, src);
	for(size_t i=1; (src2 = va_arg(ap, const char *)) != NULL; i++) {
		if (strlen(dst) + strlen(src2) >= max_len) break;
		UTILITY::strcat(dst, max_len, src2);
	}
}
/// @brief concatenate words
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : 1st word
/// @param[in] ... : 2nd and later word
///
/// @attention need set NULL in last arg
void concat(char *dst, size_t max_len, const char *src, ...) {
	va_list ap;
	va_start(ap, src);
	concatv(dst, max_len, src, ap);
	va_end(ap);
}

// -----

/// @brief copy string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
void strcpy(char *dst, size_t max_len, const char *src) {
	if (max_len == 0) return;
	size_t src_len = strlen(src);
	if (max_len <= src_len) src_len = max_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::strncpy_s(dst, max_len, src, src_len);
#else
	::strncpy(dst, src, src_len);
#endif
	dst[src_len] = '\0';
}

/// @brief concatenate string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
void strcat(char *dst, size_t max_len, const char *src) {
	if (max_len == 0) return;
	size_t dst_len = strlen(dst);
	if (max_len <= dst_len) return;
	size_t src_len = strlen(src);
	if (max_len <= src_len + dst_len) src_len = max_len - dst_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::strncat_s(dst, max_len, src, src_len);
#else
	::strncat(dst, src, src_len);
#endif
}

/// @brief copy string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
/// @param[in] src_count : source string count
void strncpy(char *dst, size_t max_len, const char *src, size_t src_count) {
	if (max_len == 0) return;
	if (max_len <= src_count) src_count = max_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::strncpy_s(dst, max_len, src, src_count);
#else
	::strncpy(dst, src, src_count);
#endif
	dst[src_count] = '\0';
}

/// @brief concatenate string with specified string size
///
/// @param[out] dst : concatenated string
/// @param[in] max_len : string size
/// @param[in] src : source string
/// @param[in] src_count : source string count
void strncat(char *dst, size_t max_len, const char *src, size_t src_count) {
	if (max_len == 0) return;
	size_t dst_len = strlen(dst);
	if (max_len <= dst_len) return;
	if (max_len <= src_count + dst_len) src_count = max_len - dst_len - 1;
#if defined(_WIN32) && defined(_MSC_VER)
	::strncat_s(dst, max_len, src, src_count);
#else
	::strncat(dst, src, src_count);
#endif
}

// -----

/// @brief convert to "%ls" from "%s"
///
/// @param[in]  format
/// @param[out] wformat
/// @param[in]  wsize : buffer size of wformat
void conv_format(const _TCHAR *format, _TCHAR *wformat, size_t wsize)
{
#if defined(_UNICODE) && !(defined(_WIN32) && defined(_MSC_VER))
	bool is_formatting = false;
	int wpos = 0;
	int len = (int)wcslen(format);
	wformat[wpos]=_T('\0');
	for(int pos=0; pos < len && wpos < ((int)wsize - 2); ) {
		if (!is_formatting) {
			if (format[pos] == _T('%')) {
				is_formatting = true;
			}
		} else {
			if ((format[pos] >= _T('a') && format[pos] <= _T('z')) || (format[pos] >= _T('A') && format[pos] <= _T('Z')) || format[pos] == _T('%')) {
				// convert %s -> %ls
				if (format[pos] == _T('s')) {
					wformat[wpos] = _T('l');
					wpos++;
				}
				is_formatting = false;
			}
		}
		wformat[wpos]=format[pos];
		pos++;
		wpos++;
	}
	wformat[wpos]=_T('\0');
#else
	size_t len = _tcslen(format);
	if (len >= wsize) len = wsize-1;
	::_tcsncpy(wformat, format, len);
	wformat[len]=_T('\0');
#endif
}

/// @brief vftprintf with specified string size
///
/// @param[in,out] fp : file pointer
/// @param[in] format : printf format
/// @param[in] ap : values
/// @return length
int vftprintf(FILE *fp, const _TCHAR *format, va_list ap) {
#ifdef _UNICODE
#if defined(_WIN32) && defined(_MSC_VER)
	return ::vfwprintf(fp, format, ap);
#else
	wchar_t wformat[1024];
	conv_format(format, wformat, 1024);
	return ::vfwprintf(fp, wformat, ap);
#endif
#else
	return ::vfprintf(fp, format, ap);
#endif
}

/// @brief snprintf
///
/// @param[out] dest : string
/// @param[in] count : string size
/// @param[in] format : printf format
/// @return length
int sntprintf(_TCHAR *dest, size_t count, const _TCHAR *format, ...) {
	va_list ap;
	va_start(ap, format);
	size_t dlen = _tcslen(dest);
	size_t drem = count - dlen;
	if (drem > count) return 0;
	int len = UTILITY::vstprintf(&dest[dlen], drem, format, ap);
	va_end(ap);
	return len;
}

/// @brief sprintf with specified string size
///
/// @param[out] dest : string
/// @param[in] count : string size
/// @param[in] format : printf format
/// @return length
int stprintf(_TCHAR *dest, size_t count, const _TCHAR *format, ...) {
	va_list ap;
	va_start(ap, format);
	int len = UTILITY::vstprintf(dest, count, format, ap);
	va_end(ap);
	return len;
}

/// @brief vsprintf with specified string size
///
/// @param[out] dest : string
/// @param[in] count : string size
/// @param[in] format : printf format
/// @param[in] ap : values
/// @return length
int vstprintf(_TCHAR *dest, size_t count, const _TCHAR *format, va_list ap) {
#ifdef _UNICODE
#if defined(_WIN32) && defined(_MSC_VER)
	return ::vswprintf(dest, count, format, ap);
#else
	wchar_t wformat[1024];
	conv_format(format, wformat, 1024);
	return ::vswprintf(dest, count, wformat, ap);
#endif
#else
#if defined(_WIN32) && defined(_MSC_VER)
	return ::vsprintf_s(dest, count, format, ap);
#else
	return ::vsnprintf(dest, count, format, ap);
#endif
#endif
}

// -----

/// @brief sprintf with specified string size
///
/// @param[out] dest : string
/// @param[in] count : string size
/// @param[in] format : ascii based printf style string
/// @return length
int sprintf(char *dest, size_t count, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int len = UTILITY::vsprintf(dest, count, format, ap);
	va_end(ap);
	return len;
}

/// @brief vsprintf with specified string size
///
/// @param[out] dest
/// @param[in] count buffer size on dest
/// @param[in] format ascii based printf style string
/// @param[in] ap
/// @return length
int vsprintf(char *dest, size_t count, const char *format, va_list ap) {
#if (defined(_WIN32) && defined(_MSC_VER))
	return ::vsprintf_s(dest, count, format, ap);
#else
	return ::vsnprintf(dest, count, format, ap);
#endif
}

#if 0
/// @brief sscanf wrapper
///
/// @param[in] source
/// @param[in] format ascii based printf style string
/// @return stored nums
int sscanf(const char *source, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int len = UTILITY::vsscanf(source, format, ap);
	va_end(ap);
	return len;
}

/// @brief vsscanf wrapper
///
/// @param[in] source
/// @param[in] format ascii based printf style string
/// @param[in] ap
/// @return stored nums
int vsscanf(const char *source, const char *format, va_list ap) {
#if (defined(_WIN32) && defined(_MSC_VER))
	return ::vsscanf_s(source, format, ap);
#else
	return ::vsscanf(source, format, ap);
#endif
}
#endif

/// @brief sprintf with specified string size (UTF-8 based string)
///
/// @param[out] dest : string
/// @param[in] count : buffer size of dest
/// @param[in] format : UTF-8 based printf style string
/// @return length
int sprintf_utf8(char *dest, size_t count, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int len = UTILITY::vsprintf_utf8(dest, count, format, ap);
	va_end(ap);
	return len;
}

#if (defined(_WIN32) && defined(_MSC_VER))

/// @brief vsprintf with specified string size (UTF-8 based string)
///
/// @note Char code in format is UTF-8. So, convert it to native language.
/// @note Because a program sometimes occur exception under Windows MBCS environment.
///
/// @param[out] dest
/// @param[in] count buffer size of dest
/// @param[in] format UTF-8 based printf style string
/// @param[in] ap
/// @return length
int vsprintf_utf8(char *dest, size_t count, const char *format, va_list ap) {
	wchar_t wformat[1024];
	wchar_t wdest[1024];
	char nformat[1024];
	char ndest[1024];
	int len;
	len = ::MultiByteToWideChar(CP_UTF8, 0, format, -1, wformat, 1024);
	if (len <= 0) return 0;
	len = ::WideCharToMultiByte(CP_ACP, 0, wformat, -1, nformat, 1024, NULL, NULL);
	if (len <= 0) return 0;
	len = vsprintf_s(ndest, count, nformat, ap);
	if (len <= 0) return 0;
	len = ::MultiByteToWideChar(CP_ACP, 0, ndest, -1, wdest, 1024);
	if (len <= 0) return 0;
	len = ::WideCharToMultiByte(CP_UTF8, 0, wdest, -1, dest, (int)count, NULL, NULL);
	return len;
}

#else	/* !_WIN32 || !_MSC_VER */

/// @brief vsprintf with specified string size (UTF-8 based string)
///
/// @param[out] dest
/// @param[in] count buffer size of dest
/// @param[in] format UTF-8 based printf style string
/// @param[in] ap
/// @return length
int vsprintf_utf8(char *dest, size_t count, const char *format, va_list ap) {
	return ::vsnprintf(dest, count, format, ap);
}

#endif	/* _WIN32 && _MSC_VER */

/// @brief find char boundary position of UTF-8 string
///
/// @param[in] str
/// @return boundary position
int utf8firstpos(const char *str)
{
	const char *p = str;
	int pos = 0;
	for(; *p != '\0'; p++, pos++) {
		if (((*p) & 0xc0) != 0x80) break;
	}
	return pos;
}

/// @brief get substring on last(most right) circle bracket in string
///
/// @param[in]  src
/// @param[out] dst
/// @param[in]  maxSize : buffer size of dst
/// @return length of substring  -1:not found
int substr_in_bracket(const _TCHAR *src, _TCHAR *dst, int maxSize)
{
	const _TCHAR *ps,*pe;
	int len = 0;

	pe = _tcsrchr(src, _T(')'));
	if (pe == NULL) return -1;
	ps = _tcsrchr(src, _T('('));
	if (ps == NULL) return -1;
	len = (int)(pe - ps - 1);
	if (len < 0) return -1;
	if (len >= maxSize-1) len = maxSize-1;

	::_tcsncpy(dst, ps+1, len);
	dst[len]=_T('\0');

	return len;
}

#ifdef _UNICODE
/// @brief skip space char in string at left side
///
/// @param[in] str
/// @return non space char at first
wchar_t *lskip(wchar_t *str)
{
	wchar_t *p = str;
	while(*p == L' ' || *p == L'\t') p++;
	return p;
}
/// @brief skip space char in string at left side
///
/// @param[in] str
/// @return non space char at first
const wchar_t *lskip(const wchar_t *str)
{
	const wchar_t *p = str;
	while(*p == L' ' || *p == L'\t') p++;
	return p;
}
/// @brief trimming char in string at right side
///
/// @param[in,out] str
/// @param[in] sep trimming char or NULL
void rtrim(wchar_t *str, const wchar_t *sep)
{
	// trim white space
	int len = (int)wcslen(str);
	for (int pos = len - 1; pos >= 0; pos--) {
		if (str[pos] == L' ' || str[pos] == L'\t'
		|| (sep != NULL && str[pos] != L'\0' && wcschr(sep, str[pos]) != NULL)) {
			str[pos]=L'\0';
		} else {
			break;
		}
	}
}

/// @brief get token from string that is separated by char
///
/// @param[in] str : string
/// @param[in] start_pos : start position of str to parse
/// @param[in] max_len : str size
/// @param[in] sep separated char
/// @param[out] st : start position of separated word in str
/// @param[out] ed : end position of separated word in str
/// @return >=0: next position / -1:last word
int get_token(const wchar_t *str, int start_pos, int max_len, const wchar_t sep, int &st, int &ed)
{
	if (start_pos < 0) {
		return start_pos;
	}

	st = start_pos;

	// ltrim
	for(; str[st] != L'\0' && st < max_len; st++) {
		if (str[st] != L' ' && str[st] != L'\t') {
			break;
		}
	}

	// search next separator
	ed = st;
	bool quote = false;
	for(; str[ed] != L'\0' && ed < max_len; ed++) {
		if (str[ed] == L'"') {
			quote = !quote;
		}
		if (quote == false && str[ed] == sep) {
			break;
		}
	}

	// end of line ?
	int next = ed;
	if (str[ed] == L'\0' || ed == max_len) {
		next = -1;
	} else {
		next++;
	}

	// rtrim
	if (!quote) {
		for (; ed >= 0; ed--) {
			if (str[ed] != L'\0' && str[ed] != L' ' && str[ed] != L'\t') {
				break;
			}
		}
	}

	// trim quote char
	if (str[st] == L'"') st++;
	if (str[ed] == L'"') ed--;

	return next;
}

/// @brief get token from string that is separated by char
///
/// @param[in] str
/// @param[in] start_pos : start position of str to parse
/// @param[in] max_len : str size
/// @param[out] word
/// @param[in] word_max_len
/// @param[in] sep separated char
/// @param[out] word_len
/// @return >=0: next position / -1:last word
int get_token(const wchar_t *str, int start_pos, int max_len, wchar_t *word, int word_max_len, const wchar_t sep, int *word_len)
{
	int st = 0, ed = 0;
	int next = get_token(str, start_pos, max_len, sep, st, ed);

	// copy word
	int len = ed - st;
	if (word) {
		if (len >= (word_max_len - 1)) len = (word_max_len - 1);
		word[0] = L'\0';
		if (len > 0) {
#if defined(_WIN32) && defined(_MSC_VER)
			::wcsncat_s(word, word_max_len, &str[st], len);
#else
			::wcsncat(word, &str[st], len);
#endif
		}
	}
	if (word_len) {
		*word_len = len;
	}

	return next;
}

/// @brief separate string
///
/// @param[in,out] str : replace separated char to null char in str
/// @param[in] delimiter
/// @param[in,out] next : start position / return next position
/// @return first position
wchar_t *wcstok(wchar_t *str, const wchar_t *delimiter, wchar_t **next)
{
	wchar_t *start = (next && *next ? *next : str);
	if (!start) return start;

	wchar_t *p = start;
	bool found = false;
	for(;*p != L'\0'; p++) {
		if (::wcschr(delimiter, *p)) {
			found = true;
			break;
		}
	}
	if (found) {
		*p = L'\0';
		if (next) *next = p + 1;
	} else {
		if (next) *next = NULL;
	}
	return start;
}

/// @brief get parameters from string
///
/// @param[in,out] str : replace separated char to null char in str
/// @param[in] max_len
/// @param[out] params : separated string (reference to str)
/// @param[in] max_params : array length of params
/// @return number of parameters
int get_parameters(wchar_t *str, int max_len, wchar_t **params, int max_params)
{
	int paramnum = 0;

	if (paramnum >= max_params) {
		return paramnum;
	}

	int st = 0, ed = 0;
	int pos = 0;
	do {
		pos = get_token(str, pos, max_len, L' ', st, ed);
		str[ed+1] = L'\0';
		params[paramnum++] = &str[st];
	} while (pos >= 0 && paramnum < max_params);

	return paramnum;
}

/// @brief get parameters from string
///
/// @param[in] str
/// @param[in] max_len
/// @param[out] params : separated word (reference to str)
/// @param[in] max_params
/// @param[out] paramslen : separated word length
/// @return number of parameters
int get_parameters(const wchar_t *str, int max_len, const wchar_t **params, int max_params, int *paramslen)
{
	int paramnum = 0;

	if (paramnum >= max_params) {
		return paramnum;
	}

	int st = 0, ed = 0;
	int pos = 0;
	do {
		pos = get_token(str, pos, max_len, L' ', st, ed);
		int len = (ed + 1 - st);
		if (len > 0) {
			params[paramnum] = &str[st];
			paramslen[paramnum] = len;
			paramnum++;
		}
	} while (pos >= 0 && paramnum < max_params);

	return paramnum;
}
#endif /* _UNICODE */

/// @brief skip space char in string at left side
///
/// @param[in] str
/// @return non space char at first
char *lskip(char *str)
{
	char *p = str;
	while(*p == ' ' || *p == '\t') p++;
	return p;
}
/// @brief skip space char in string at left side
///
/// @param[in] str
/// @return non space char at first
const char *lskip(const char *str)
{
	const char *p = str;
	while(*p == ' ' || *p == '\t') p++;
	return p;
}
/// @brief trimming char in string at right side
///
/// @param[in,out] str
/// @param[in] sep trimming char or NULL
void rtrim(char *str, const char *sep)
{
	// trim white space
	int len = (int)strlen(str);
	for (int pos = len - 1; pos >= 0; pos--) {
		if (str[pos] == ' ' || str[pos] == '\t'
		|| (sep != NULL && str[pos] != '\0' && strchr(sep, str[pos]) != NULL)) {
			str[pos]='\0';
		} else {
			break;
		}
	}
}

/// @brief get token from string that is separated by char
///
/// @param[in] str : string
/// @param[in] start_pos : start position of str to parse
/// @param[in] max_len : str size
/// @param[in] sep separated char
/// @param[out] st : start position of separated word in str
/// @param[out] ed : end position of separated word in str
/// @return >=0: next position / -1:last word
int get_token(const char *str, int start_pos, int max_len, const char sep, int &st, int &ed)
{
	if (start_pos < 0) {
		return start_pos;
	}

	st = start_pos;

	// ltrim
	for(; str[st] != '\0' && st < max_len; st++) {
		if (str[st] != ' ' && str[st] != '\t') {
			break;
		}
	}

	// search next separator
	ed = st;
	bool quote = false;
	for(; str[ed] != '\0' && ed < max_len; ed++) {
		if (str[ed] == '"') {
			quote = !quote;
		}
		if (quote == false && str[ed] == sep) {
			break;
		}
	}

	// end of line ?
	int next = ed;
	if (str[ed] == '\0' || ed == max_len) {
		next = -1;
	} else {
		next++;
	}

	// rtrim
	if (!quote) {
		for (; ed >= 0; ed--) {
			if (str[ed] != sep && str[ed] != '\0' && str[ed] != ' ' && str[ed] != '\t') {
				break;
			}
		}
	}

	// trim quote char
	if (str[st] == '"') st++;
	if (str[ed] == '"') ed--;

	return next;
}

/// @brief get token from string that is separated by char
///
/// @param[in] str
/// @param[in] start_pos : start position of str to parse
/// @param[in] max_len : str size
/// @param[out] word
/// @param[in] word_max_len
/// @param[in] sep separated char
/// @param[out] word_len
/// @return >=0: next position / -1:last word
int get_token(const char *str, int start_pos, int max_len, char *word, int word_max_len, const char sep, int *word_len)
{
	int st = 0, ed = 0;
	int next = get_token(str, start_pos, max_len, sep, st, ed);

	// copy word
	int len = ed - st + 1;
	if (word) {
		if (len >= (word_max_len - 1)) len = (word_max_len - 1);
		word[0] = '\0';
		if (len > 0) {
			::strncat(word, &str[st], len);
		}
	}
	if (word_len) {
		*word_len = len;
	}

	return next;
}

/// @brief separate string
///
/// @param[in,out] str : replace separated char to null char in str
/// @param[in] delimiter
/// @param[in,out] next : start position / return next position
/// @return first position
char *strtok(char *str, const char *delimiter, char **next)
{
	char *start = (next && *next ? *next : str);
	if (!start) return start;

	char *p = start;
	bool found = false;
	for(;*p != '\0'; p++) {
		if (::strchr(delimiter, *p)) {
			found = true;
			break;
		}
	}
	if (found) {
		*p = '\0';
		if (next) *next = p + 1;
	} else {
		if (next) *next = NULL;
	}
	return start;
}

/// @brief get parameters from string
///
/// @param[in,out] str : replace separated char to null char in str
/// @param[in] max_len
/// @param[out] params : separated string (reference to str)
/// @param[in] max_params : array length of params
/// @return number of parameters
int get_parameters(char *str, int max_len, char **params, int max_params)
{
	int paramnum = 0;

	if (!str) {
		return paramnum;
	}
	if (paramnum >= max_params) {
		return paramnum;
	}

	int st = 0, ed = 0;
	int pos = 0;
	do {
		pos = get_token(str, pos, max_len, ' ', st, ed);
		if (ed >= st) {
			str[ed+1] = '\0';
			params[paramnum++] = &str[st];
		}
	} while (pos >= 0 && paramnum < max_params);

	return paramnum;
}

/// @brief get parameters from string
///
/// @param[in] str
/// @param[in] max_len
/// @param[out] params : separated word (reference to str)
/// @param[in] max_params
/// @param[out] paramslen : separated word length
/// @return number of parameters
int get_parameters(const char *str, int max_len, const char **params, int max_params, int *paramslen)
{
	int paramnum = 0;

	if (paramnum >= max_params) {
		return paramnum;
	}

	int st = 0, ed = 0;
	int pos = 0;
	do {
		pos = get_token(str, pos, max_len, ' ', st, ed);
		int len = (ed + 1 - st);
		if (len > 0) {
			params[paramnum] = &str[st];
			paramslen[paramnum] = len;
			paramnum++;
		}
	} while (pos >= 0 && paramnum < max_params);

	return paramnum;
}

uint16_t sjis_to_jis(uint16_t code)
{
	uint8_t k1 = (code >> 8) & 0xff;
	uint8_t k2 = (code & 0xff);
	k1 <<= 1;
		if( k2 < 0x9f ){
		if( k1 < 0x3f ) k1 += 0x1f; else k1 -= 0x61;
		if( k2 > 0x7e ) k2 -= 0x20; else k2 -= 0x1f;
	}else{
		if( k1 < 0x3f ) k1 += 0x20; else k1 -= 0x60;
		k2 -= 0x7e;
	}

	return (((uint16_t)k1 << 8) | k2);
}

}	// end of UTILITY
