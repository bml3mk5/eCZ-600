/** @file simple_clocale.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [simple i18n]
*/

#ifndef SIMPLE_CLOCALE_H
#define SIMPLE_CLOCALE_H

#include "../common.h"
#include "../cchar.h"
#include "../cptrlist.h"
#ifdef _WIN32
#include "../depend.h"
#include <windows.h>
#endif

/**
	@brief Locale Catalog
*/
class CLocaleCatalog
{
private:
	CTchar src;
	CTchar dst;

public:
	CLocaleCatalog(const _TCHAR *n_src, const _TCHAR *n_dst);

	const CTchar &GetSrc() const { return src; }
	const CTchar &GetDst() const { return dst; }
};

/**
	@brief Locale Catalog List
*/
class CLocaleCatalogList : public CPtrList<CLocaleCatalog>
{
private:
	const CLocaleCatalog *MatchSourceCyc(const _TCHAR *str, int len, int index, int start, int end, int depth) const;

public:
	CLocaleCatalogList();
	void SortBySource();
	const CLocaleCatalog *MatchSource(const _TCHAR *str) const;
};

/**
	@brief Locale Region
*/
class CLocaleRegion
{
private:
	CTchar file;
	CTchar name;
	CTchar win;
	CTchar posix;

public:
	CLocaleRegion(const char *n_file, const char *n_name, const char *n_win, const char *n_posix);

	const CTchar &GetFile() const { return file; }
	const CTchar &GetName() const { return name; }
	const CTchar &GetWin() const { return win; }
	const CTchar &GetPosix() const { return posix; }
};

/**
	@brief Language Localization
*/
class CLocale
{
private:
	CTchar mLanguageName;
	CTchar mLocaleName;
	CTchar mPackageName;
	CTchar mLocalePath;
	CTchar mLocaleFile;

#ifdef _WIN32
	LCID  lcid;
#endif

	bool mBigEndien;
	CLocaleCatalogList mCatalog;
	CPtrList<CLocaleRegion> mRegion;
	bool mRegionLoaded;

	_TCHAR mTempMessage[_MAX_PATH];

	/// Cannot copy
	CLocale(const CLocale &);
	CLocale &operator=(const CLocale &);

	const char *SetLocaleSub(const char *locale, int category, const CLocaleRegion *region);

	bool FindLocalePath(const _TCHAR *new_app_path);
	bool MakeLocaleFilePath(const _TCHAR *locale_name, const _TCHAR *domain_name);
	const CLocaleRegion *FindRegionByName(const _TCHAR *language);
	const CLocaleRegion *FindRegionByLocale(const char *locale);
	bool ReadRegion(const _TCHAR *filename);
	bool ReadCatalog(const _TCHAR *filename);

	static void swap_buf4(uint8_t *buf);
	uint32_t to_uint32(uint8_t *buf);
	static bool get_attr(const char *buf, const char *attr, char *out, int len);

#ifdef _WIN32
	static BOOL CALLBACK enum_lcids_proc(LPSTR lcid_string);
	LCID get_lcid(const char *locale);
	void set_lcid(const char *locale);
#endif

public:
	enum enumCharCodes {
		CODE_DEFAULT = 0,
		CODE_ACP,
	};

public:
	CLocale();
	CLocale(const _TCHAR *new_app_path, const _TCHAR *new_package_name, const _TCHAR *new_language);
	~CLocale();
	void Clear();

	bool SetLocale(const _TCHAR *new_app_path, const _TCHAR *new_package_name, const _TCHAR *new_language);
	bool SetLocale(const _TCHAR *new_package_name, const _TCHAR *new_language);
	bool UnsetLocale();

	const _TCHAR *GetText(const _TCHAR *str);
	const _TCHAR *GetText(const _TCHAR *str, enumCharCodes char_code);

#ifdef _WIN32
	LCID GetLcid() { return lcid; }
#endif
	bool SetThreadLocale();

	bool IsOk() const;

	const _TCHAR *GetLocaleName() const;
	const _TCHAR *GetLanguageName() const;

	void ChangeLocaleIfNeed(const CTchar &new_language);
	bool GetLocaleNames(CPtrList<CTchar> &arr) const;
	bool GetLocaleNamesWithDefault(CPtrList<CTchar> &arr) const;
	int  SelectLocaleNameIndex(const CPtrList<CTchar> &arr, const CTchar &name) const;
	void ChooseLocaleName(const CPtrList<CTchar> &arr, int selidx, CTchar &name) const;
};

extern CLocale *clocale;

#if defined(UNICODE) || defined(_UNICODE)
/* if variable string, use _tgettext instead of _(). */
#define _(text) clocale->GetText(L##text)
#define wgettext(text) clocale->GetText(L##text)
#define _tgettext(text) clocale->GetText(L##text)
#else /* MBCS */
#define _(text) clocale->GetText(text)
#define gettext(text) clocale->GetText(text)
#define _tgettext(text) clocale->GetText(text)
#endif

/// Specify a text that xgettext should get and translate.
/// (When use xgettext, set --keyword=_TX.)
#define _TX _T

#endif /* SIMPLE_CLOCALE_H */
