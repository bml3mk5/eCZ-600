/** @file gnu_clocale.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.12.01

	@brief [i18n with gettext]

	@note char code policy
		Win32: MBCS (maybe shift jis on Japanese Edition)
		Mac  : utf-8
		Linux: should be utf-8
*/

#ifndef _GNU_CLOCALE_H_
#define _GNU_CLOCALE_H_

#include "../depend.h"

#ifdef USE_GETTEXT

#include "../common.h"
#include "../cchar.h"
#include "../cptrlist.h"

#if defined(_WIN32) && !defined(__MINGW32__)

#include "libintl.h"
#if defined(UNICODE) || defined(_UNICODE)
#ifdef _DEBUG
#pragma comment(lib, "libintlud.lib")
#else
#pragma comment(lib, "libintlu.lib")
#endif
#include "wlibintl.h"
#else
#ifdef _DEBUG
#pragma comment(lib, "libintld.lib")
#else
#pragma comment(lib, "libintl.lib")
#endif
#endif

#else

#include <locale.h>
#include <libintl.h>

#endif

#if defined(UNICODE) || defined(_UNICODE)
/* if variable string, use _tgettext instead of _(). */
#define _(text) wgettext(L##text)
#define _tgettext wgettext
#define _tdgettext dwgettext
#define _tdcgettext dcwgettext
#define _tngettext nwgettext
#define _tdngettext dnwgettext
#define _tdcngettext dcnwgettext

#define _tbindtextdomain wbindtextdomain
#define _tbind_textdomain_codeset wbind_textdomain_codeset

#define _ttextdomain wtextdomain

#define libintl_tfreeres libintl_wfreeres
#else /* MBCS */
#define _(text) gettext(text)
#define _tgettext gettext
#define _tdgettext dgettext
#define _tdcgettext dcgettext
#define _tngettext ngettext
#define _tdngettext dngettext
#define _tdcngettext dcngettext

#define _tbindtextdomain bindtextdomain
#define _tbind_textdomain_codeset wbind_textdomain_codeset

#define _ttextdomain textdomain

#define libintl_tfreeres libintl_freeres
#endif

#else	// ! USE_GETTEXT

#include <locale.h>

#if defined(UNICODE) || defined(_UNICODE)
#define _(text) (L##text)
#define wgettext
#else
#define _(text) (text)
#define gettext
#endif
#define _tgettext

#endif	// USE_GETTEXT

/// Specify a text that xgettext should get and translate.
/// (When use xgettext, set --keyword=_TX.)
#define _TX _T

#ifdef _WIN32
#include <windows.h>
#endif

class CLocaleRegion;

/// locale class with gnu gettext
class CLocale
{
private:
	CTchar mLanguageName;
	CTchar mLocaleName;
	CTchar mPackageName;
	CTchar mLocalePath;
#ifdef _WIN32
	LCID  lcid;
#endif

	CPtrList<CLocaleRegion> mRegion;
	bool mRegionLoaded;

	_TCHAR mTempMessage[_MAX_PATH];

	/// Cannot copy
	CLocale(const CLocale &);
	CLocale &operator=(const CLocale &);

	const char *SetLocaleSub(const char *locale, int category, const CLocaleRegion *region);

	bool FindLocalePath(const _TCHAR *new_app_path);
	const CLocaleRegion *FindRegionByName(const _TCHAR *language);
	const CLocaleRegion *FindRegionByLocale(const char *locale);
	bool ReadRegion(const _TCHAR *filename);

	bool SetDomain(const _TCHAR *new_package_name, const _TCHAR *new_locale_path);
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
	bool SetLocale(const char *new_locale);
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

class CLocaleRegion
{
private:
	CTchar file;
	CTchar name;
	CTchar win;
	CTchar posix;

public:
	CLocaleRegion(const _TCHAR *n_file, const _TCHAR *n_name, const _TCHAR *n_win, const _TCHAR *n_posix);

	const CTchar &GetFile() const { return file; }
	const CTchar &GetName() const { return name; }
	const CTchar &GetWin() const { return win; }
	const CTchar &GetPosix() const { return posix; }
};

// set in main.
extern CLocale *clocale;

#endif /* _GNU_CLOCALE_H_ */
