/** @file gnu_clocale.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2014.12.01

	@brief [i18n with gettext]
*/


#include "gnu_clocale.h"
#include "../fileio.h"
#include "../utility.h"
//#ifndef _WIN32
//#include <dirent.h>
//#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning( disable : 4996 )
#endif

#ifdef _WIN32
#include <shlwapi.h>

static const char *locale_for_search;
static LCID decide_lcid;
#endif

#if defined(__MACH__) && defined(__APPLE__)
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>
#endif

static const _TCHAR *sub_dir[] = {
	_T(""),
#ifdef _WIN32
	_T("..\\"),
	_T("..\\..\\"),
	_T("..\\..\\..\\"),
	_T("..\\..\\..\\..\\"),
#else
	_T("../"),
	_T("../../"),
	_T("../../../"),
	_T("../../../../"),
#endif
	NULL
};

//

CLocale::CLocale()
{
	Clear();
}
CLocale::CLocale(const CLocale &)
{
	Clear();
}
CLocale &CLocale::operator=(const CLocale &)
{
	Clear();
	return *this;
}
CLocale::CLocale(const _TCHAR *new_app_path, const _TCHAR *new_package_name, const _TCHAR *new_language)
{
	Clear();
	SetLocale(new_app_path, new_package_name, new_language);
}

CLocale::~CLocale()
{
	if (mPackageName.Length() > 0) {
#ifdef USE_GETTEXT
#if defined(_WIN32) && !defined(__MINGW32__)
		libintl_tfreeres();
#endif
#endif
	}
}

void CLocale::Clear()
{
	mRegionLoaded = false;

#ifdef _WIN32
	lcid = 0;
#endif

#ifdef _WIN32
	locale_for_search = NULL;
	decide_lcid = 0;
#endif
}

/// @brief Initialize and set a locale
bool CLocale::SetLocale(const _TCHAR *new_app_path, const _TCHAR *new_package_name, const _TCHAR *new_language)
{
	if (new_package_name == NULL) {
		return false;
	}

	// make file path
	if (!FindLocalePath(new_app_path)) {
		return false;
	}

	// read region list
	mRegionLoaded = ReadRegion(mLocalePath.Get());
	if (!mRegionLoaded) {
		return false;
	}

	return SetLocale(new_package_name, new_language);
}

bool CLocale::SetLocale(const _TCHAR *new_package_name, const _TCHAR *new_language)
{
	mPackageName.Set(new_package_name);

#ifdef USE_GETTEXT
	// find locale
	const CLocaleRegion *region = NULL;
	if (new_language != NULL && new_language[0] != '\0') {
		region = FindRegionByName(new_language);
	}

	// use system locale
	const char *locale = NULL;
	if (region == NULL) {
#if defined(_WIN32)
		locale = setlocale(LC_ALL, "");
#else
		locale = setlocale(LC_ALL, "");
		if (locale != NULL && strstr(locale, "LC_") != NULL) {
			locale = setlocale(LC_MESSAGES, "");
		}
#endif
		if (locale == NULL) {
			mLocaleName.Clear();
	        return false;
		}
		mLocaleName.SetN(locale);

		region = FindRegionByLocale(locale);
	}

	if (region == NULL) {
		mLanguageName.Clear();
		return false;
	}

	mLanguageName.Set(region->GetName().Get());

	locale = SetLocaleSub(locale, LC_ALL, region);
	if (locale == NULL) {
		mLocaleName.Clear();
        return false;
    }

	mLocaleName.SetN(locale);

#ifdef _WIN32
	set_lcid(locale);
#endif

	if (!SetDomain(new_package_name, mLocalePath.Get())) {
		mLocaleName.Clear();
		return false;
	}
#endif	// USE_GETTEXT

	return true;
}

const char *CLocale::SetLocaleSub(const char *locale, int category, const CLocaleRegion *region)
{
#if defined(_WIN32)
	locale = setlocale(LC_ALL, region->GetWin().GetN());
#elif defined(__MACH__) && defined(__APPLE__)
	locale = setlocale(LC_ALL, region->GetPosix().GetN());
#elif defined(linux) || defined(__UNIX__)
	char locale_full[256];
	const char *locale_charcodes[] = {
			".UTF-8",
			".utf-8",
			".UTF8",
			".utf8",
			NULL
	};
	const char *p = NULL;
	const char *new_locale = NULL;

	if (locale) {
		p = strchr(locale, '.');
	}
	if (p) {
		strcpy(locale_full, region->GetPosix().GetN());
		strcat(locale_full, p);
		new_locale = setlocale(LC_ALL, locale_full);
		if (!new_locale) p = NULL;
	}
	if (!p) {
		for(int i=0; locale_charcodes[i] != NULL; i++) {
			strcpy(locale_full, region->GetPosix().GetN());
			strcat(locale_full, locale_charcodes[i]);
			new_locale = setlocale(LC_ALL, locale_full);
			if (new_locale) break;
		}
	}
	locale = new_locale;
#else
	locale = setlocale(LC_ALL, region->GetPosix().GetN());
#endif
	return locale;
}

/// @brief change locale
bool CLocale::SetLocale(const char *new_locale)
{
#ifdef USE_GETTEXT
#if defined(__MACH__) && defined(__APPLE__)
	char n_new_locale[100];
	int len = (int)strlen(new_locale);
	if (len > 0 && strchr(new_locale, '_') == NULL) {
		sprintf(n_new_locale, "%s_", new_locale);
	} else {
		strcpy(n_new_locale, new_locale);
	}
	CFStringRef nlf = CFStringCreateWithCString(NULL, n_new_locale, kCFStringEncodingASCII);
	CFStringRef new_locale_ref;
	CFArrayRef locales = CFLocaleCopyAvailableLocaleIdentifiers();
	bool match = false;
	for(CFIndex i=0; i<CFArrayGetCount(locales); i++) {
		CFStringRef nl = (CFStringRef)CFArrayGetValueAtIndex(locales, i);
		if (CFStringCompare(nl, nlf, 0) == kCFCompareEqualTo) {
			new_locale_ref = nl;
			match = true;
			break;
		}
	}
	if (!match) {
		for(CFIndex i=0; i<CFArrayGetCount(locales); i++) {
			CFStringRef nl = (CFStringRef)CFArrayGetValueAtIndex(locales, i);
			if (CFStringHasPrefix(nl, nlf)) {
				new_locale_ref = nl;
				match = true;
				break;
			}
		}
	}
	if (!match) {
		new_locale_ref = nlf;
	}
	CFStringGetCString(new_locale_ref, n_new_locale, sizeof(n_new_locale), kCFStringEncodingASCII);

	CFRelease(locales);
	CFRelease(nlf);
	const char *locale = setlocale(LC_ALL, n_new_locale);
#else
	const char *locale = setlocale(LC_ALL, new_locale);
#endif
	if (locale == NULL) {
        return false;
    }

#ifdef _WIN32
	set_lcid(locale);
#endif

	if (!SetDomain(mPackageName.Get(), mLocalePath.Get())) {
		return false;
	}

	mLocaleName.SetN(locale);

#endif /* USE_GETTEXT */

	return true;
}

/// @brief set dummy locale. So, no translate.
bool CLocale::UnsetLocale()
{
#ifdef USE_GETTEXT
	const _TCHAR *package = _T("dummy_locale");
	if (_ttextdomain(package) == NULL) {
        return false;
    }
#endif
	mLanguageName.Clear();
	return true;
}

bool CLocale::SetDomain(const _TCHAR *new_package_name, const _TCHAR *new_locale_path)
{
	if (_tbindtextdomain(new_package_name, new_locale_path) == NULL) {
        return false;
    }
#if defined(UNICODE) || defined(_UNICODE)
    if (wbind_textdomain_codeset(new_package_name, _T("UTF-16LE")) == NULL) {
        return false;
    }
#elif defined(USE_SDL) || defined(USE_SDL2)
	if (bind_textdomain_codeset(new_package_name, _T("UTF-8")) == NULL) {
		return false;
	}
#endif
	if (_ttextdomain(new_package_name) == NULL) {
        return false;
    }
	return true;
}

/// @brief set locale
/// @note SetThreadLocale WinAPI set a locale on current thread only. So, you need call this on every threads.
bool CLocale::SetThreadLocale()
{
#ifdef _WIN32
	if (lcid == 0) {
		lcid = ::GetThreadLocale();
	}
	return (::SetThreadLocale(lcid) == TRUE);
#else
	return true;
#endif
}

bool CLocale::FindLocalePath(const _TCHAR *new_app_path)
{
	_TCHAR sdir[_MAX_PATH];
	_TCHAR new_locale_path[_MAX_PATH];
	bool exist = false;
	for(int i=0; sub_dir[i] != NULL; i++) {
		UTILITY::tcscpy(sdir, _MAX_PATH, new_app_path);
		UTILITY::add_path_separator(sdir, _MAX_PATH);
		UTILITY::tcscat(sdir, _MAX_PATH, sub_dir[i]);
		UTILITY::tcscat(sdir, _MAX_PATH, _T("locale"));

		UTILITY::slim_path(sdir, new_locale_path, _MAX_PATH);
		if (FILEIO::IsDirExists(new_locale_path)) {
			exist = true;
			break;
		}
	}
	if (exist) {
		mLocalePath.Set(new_locale_path);
	}
	return exist;
}

bool CLocale::ReadRegion(const _TCHAR *filename)
{
	bool rc = true;
	FILEIO fio;

	char buf[_MAX_PATH];
	char *p;

	char file[32];
	char name[64];
	char win[64];
	char posix[64];

	do {
		UTILITY::tcscpy(buf, _MAX_PATH, filename);
		UTILITY::add_path_separator(buf, _MAX_PATH);
		UTILITY::tcscat(buf, _MAX_PATH, _T("list.xml"));

		if (!fio.Fopen(buf, FILEIO::READ_ASCII)) {
			rc = false;
			break;
		}

		mRegion.Clear();

		int phase = 0;
		bool comment = false;

		// read XML
		while((p = fio.Fgets(buf, _MAX_PATH)) != NULL && mRegion.Count() < 200) {
			p = UTILITY::lskip(buf);
			if (strstr(p, "<!--") != NULL) {
				comment = true;
			}
			if (!comment) {
				switch(phase & 0xff) {
				case 1:
					if (strstr(p, "<LocaleList ") != NULL) {
						phase = 2;
					}
					break;
				case 2:
					if (strstr(p, "</LocaleList>") != NULL) {
						phase = 1;
					} else if (strstr(p, "<Locale ") != NULL) {
						p += 8;
						get_attr(p, "file", file, sizeof(file));
						get_attr(p, "name", name, sizeof(name));
						get_attr(p, "win", win, sizeof(win));
						get_attr(p, "posix", posix, sizeof(posix));

						mRegion.Add(new CLocaleRegion(file, name, win, posix));
					}
					break;
				default:
					if (strstr(p, "<?xml ") != NULL) {
						phase = 1;
					}
					break;
				}
			}
			if (strstr(p, "-->") != NULL) {
				comment = false;
			}
		}
	} while(0);

	return rc;
}

bool CLocale::get_attr(const char *buf, const char *attr, char *out, int len)
{
	memset(out, 0, len);
	const char *p = strstr(buf, attr);
	if (p == NULL) return false;
	p += strlen(attr);
	p = UTILITY::lskip(p);
	if (*p != '=') return false;
	p++;
	p = UTILITY::lskip(p);
	if (*p != '"') return false;
	p++;
	const char *ep = strchr(p, '"');
	if (ep == NULL) return false;
	int nlen = static_cast<int>(len - 1 > ep - p ? ep - p : len - 1);
	strncpy(out, p, nlen);
	return true;
}

const CLocaleRegion *CLocale::FindRegionByName(const _TCHAR *language)
{
	bool match = false;
	CLocaleRegion *itm = NULL;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
		if (_tcscmp(language, itm->GetName().Get()) == 0) {
			match = true;
			break;
		}
	}
	return (match ? itm : NULL);
}

const CLocaleRegion *CLocale::FindRegionByLocale(const char *locale)
{
	bool match = false;
	CLocaleRegion *itm = NULL;
	for(int i=0; i<mRegion.Count(); i++) {
		itm = mRegion[i];
#if defined(_WIN32)
		if (strstr(locale, itm->GetWin().GetN()) != NULL)
#else
		if (strstr(locale, itm->GetPosix().GetN()) != NULL)
#endif
		{
			match = true;
			break;
		}
	}
	return (match ? itm : NULL);
}

const _TCHAR *CLocale::GetText(const _TCHAR *str)
{
	return (const _TCHAR *)gettext(str);
}

const _TCHAR *CLocale::GetText(const _TCHAR *str, enumCharCodes char_code)
{
	if (char_code == CODE_ACP) {
		UTILITY::conv_utf8_to_mbs(gettext(str), _MAX_PATH, mTempMessage, _MAX_PATH);
		return mTempMessage;
	} else {
		return (const _TCHAR *)gettext(str);
	}
}

bool CLocale::IsOk() const
{
	return (mPackageName.Length() > 0 && mLocaleName.Length() > 0);
}

const _TCHAR *CLocale::GetLocaleName() const
{
	return mLocaleName.Get();
}

const _TCHAR *CLocale::GetLanguageName() const
{
	return mLanguageName.Get();
}

void CLocale::ChangeLocaleIfNeed(const CTchar &new_language)
{
//	if (!IsOk()) return;
	if (new_language.Length() <= 0) return;

	if (new_language.MatchString(_T("default"))) {
		UnsetLocale();
		return;
	}

	SetLocale(mPackageName.Get(), new_language.Get());
}

/// @brief get locale names on locale directory
bool CLocale::GetLocaleNames(CPtrList<CTchar> &arr) const
{
	for(int i=0; i<mRegion.Count(); i++) {
		arr.Add(new CTchar(mRegion.Item(i)->GetName().Get()));
	}
	return true;
#if 0
	_TCHAR dir[_MAX_PATH];

	if (locale_path.Length() <= 0) {
		return false;
	}

#ifdef _WIN32
	WIN32_FIND_DATA FindFileData;

	_tcscpy(dir, locale_path);
	if (PathAppend(dir, _T("\\*")) == FALSE) {
		return false;
	}

	HANDLE hFind = FindFirstFile(dir, &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE) {
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && FindFileData.cFileName[0] != _T('.')) {
			CTchar *name = new CTchar(FindFileData.cFileName);
			arr.Add(name);
		}

		if (!FindNextFile(hFind, &FindFileData)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
			break;
		}
	}
	return true;
#else
	DIR *hdir;
	struct dirent *entry;
	hdir = opendir(locale_path);
	if (!hdir) {
		return false;
	}
	while(1) {
		entry = readdir(hdir);
		if (entry == NULL) {
			break;
		}
		if (entry->d_name[0] == '.' || entry->d_name[0] == '\0') {
			continue;
		}
		struct stat st;
		UTILITY::tcscpy(dir, _MAX_PATH, locale_path);
		UTILITY::add_path_separator(dir);
		UTILITY::tcscat(dir, _MAX_PATH, entry->d_name);
		if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode)) {
			CTchar *name = new CTchar(entry->d_name);
			arr.Add(name);
		}
	};

	closedir(hdir);

	return true;
#endif
#endif
}

bool CLocale::GetLocaleNamesWithDefault(CPtrList<CTchar> &arr) const
{
	arr.Add(new CTchar(_T("System dependent")));
	arr.Add(new CTchar(_T("Default (English)")));
	return GetLocaleNames(arr);
}

int CLocale::SelectLocaleNameIndex(const CPtrList<CTchar> &arr, const CTchar &name) const
{
	int selidx = 0;
	if (!name.MatchString(_T("default"))) {
		for(int i=0; i<arr.Count(); i++) {
			bool match = false;
			match = arr.Item(i)->MatchString(name.Get());
			if (match) {
				selidx = i;
				break;
			}
		}
	} else {
		selidx = 1;
	}
	return selidx;
}

void CLocale::ChooseLocaleName(const CPtrList<CTchar> &arr, int selidx, CTchar &name) const
{
	if (selidx == 0) {
		name.Set(_T(""));
	} else if (selidx == 1) {
		name.Set(_T("default"));
	} else {
		name.Set(arr.Item(selidx)->Get());
	}
}

#ifdef _WIN32

/// @brief Compare with specified locale name and supported locale name
BOOL CALLBACK CLocale::enum_lcids_proc(LPSTR lcid_string)
{
	if (locale_for_search == NULL) {
		return FALSE;
	}

	char language_name[128], country_name[128];
	char *endp;
	LCID lcid = strtoul(lcid_string, &endp, 16);
	GetLocaleInfoA(lcid, LOCALE_SENGLANGUAGE, language_name, sizeof(language_name));
	char *p = strrchr(language_name, '(');
	if (p != NULL) *p = '\0';
	GetLocaleInfoA(lcid, LOCALE_SENGCOUNTRY,  country_name, sizeof(country_name));
	if (strstr(locale_for_search, language_name) != NULL && strstr(locale_for_search, country_name) != NULL) {
		decide_lcid = lcid;
		return FALSE;
	}
	return TRUE;
}

/// @brief Get lcid from a locale name
LCID CLocale::get_lcid(const char *locale)
{
	locale_for_search = locale;
	decide_lcid = 0;
	EnumSystemLocalesA(enum_lcids_proc, LCID_SUPPORTED);
	locale_for_search = NULL;
	return decide_lcid;
}

void CLocale::set_lcid(const char *locale)
{
	if (locale != NULL && locale[0] != '\0') {
		// change language
		LCID new_lcid = get_lcid(locale);
		if (new_lcid > 0) {
			lcid = new_lcid;
			SetThreadLocale();
		}
	} else {
		lcid = ::GetThreadLocale();
	}
}

#endif

//

CLocaleRegion::CLocaleRegion(const _TCHAR *n_file, const _TCHAR *n_name, const _TCHAR *n_win, const _TCHAR *n_posix)
{
	file.Set(n_file);
	name.Set(n_name);
	win.Set(n_win);
	posix.Set(n_posix);
}

/// set in main.
CLocale *clocle = NULL;
