/** @file simple_clocale.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.09.01

	@brief [simple i18n]
*/


#include "simple_clocale.h"
#include "../fileio.h"
#include "../utility.h"
#include <locale.h>

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

/// identifier in .mo file (little endien)
static const uint8_t mofile_ident[4] = { 0x95, 0x04, 0x12, 0xde };

/// static struct on CLocale
class CKeyTable
{
public:
	uint32_t len;
	uint32_t pos;
	CTchar   data;
public:
	CKeyTable(uint32_t n_len, uint32_t n_pos) {
		len = n_len;
		pos = n_pos;
	}
};

//

CLocaleCatalog::CLocaleCatalog(const _TCHAR *n_src, const _TCHAR *n_dst)
{
	src.Set(n_src);
	dst.Set(n_dst);
}

//

CLocaleCatalogList::CLocaleCatalogList()
	: CPtrList<CLocaleCatalog>()
{
}

void CLocaleCatalogList::SortBySource()
{
	if (Count() < 1) return;

	for(int y = 0; y < (Count()-1); y++) {
		for(int x = (y+1); x < Count(); x++) {
			int c = Item(y)->GetSrc().Compare(Item(x)->GetSrc());
			if (c > 0) {
				Exchange(x, y);
			}
		}
	}
}

/// @brief Search translated string in this catalog
///
/// @param[in] str   : string
/// @param[in] len   : length of string
/// @param[in] index : position in a catalog
/// @param[in] start : first position in searching a catalog
/// @param[in] end   : last position in searching a catalog
/// @param[in] depth : depth of recursive
/// @return matched catalog or NULL
///
/// @note this function is called recursively.
const CLocaleCatalog *CLocaleCatalogList::MatchSourceCyc(const _TCHAR *str, int len, int index, int start, int end, int depth) const
{
	if (len <= 0 || depth > 20) {
		return NULL;
	}
	int c = Item(index)->GetSrc().Compare(str, len);
	if (c == 0) {
		return Item(index);
	} else if (c > 0) {
		end = index - 1;
		index = (end - start) / 2 + start;
	} else {
		start = index + 1;
		index = (end - start) / 2 + start;
	}
	if (end < start) {
		return NULL;
	}
	return MatchSourceCyc(str, len, index, start, end, depth + 1);
}

/// @brief Search translated string in this catalog
///
/// @param[in] str : string
/// @return matched catalog or NULL
const CLocaleCatalog *CLocaleCatalogList::MatchSource(const _TCHAR *str) const
{
	const CLocaleCatalog *itm = NULL;
#if 0
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetSrc().MatchString(str)) {
			itm = Item(i);
			break;
		}
	}
#else
	if (Count() > 0) itm = MatchSourceCyc(str, (int)_tcslen(str), Count() / 2, 0, Count()-1, 0);
#endif
	return itm;
}

//

CLocaleRegion::CLocaleRegion(const char *n_file, const char *n_name, const char *n_win, const char *n_posix)
{
	file.Set(n_file);
	name.Set(n_name);
	win.Set(n_win);
	posix.Set(n_posix);
}

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
}

void CLocale::Clear()
{
	mBigEndien = false;
	mRegionLoaded = false;

#ifdef _WIN32
	lcid = 0;
#endif

#ifdef _WIN32
	locale_for_search = NULL;
	decide_lcid = 0;
#endif
}

/// @brief set locale and translated catalog
///
/// @param [in] new_app_path     : directory path of application
/// @param [in] new_package_name : package name
/// @param [in] new_language     : language name (ex. "Japanese" "English"
/// @return true / false
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

/// @brief set locale and translated catalog
///
/// @param [in] new_package_name : package name
/// @param [in] new_language     : language name (ex. "Japanese" "English"
/// @return true / false
bool CLocale::SetLocale(const _TCHAR *new_package_name, const _TCHAR *new_language)
{
	mPackageName.Set(new_package_name);

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
#elif defined(__MACH__) && defined(__APPLE__)
		char current_locale[128];
		CFLocaleRef cf_locale_ref = CFLocaleCopyCurrent();
		CFStringRef cf_locale_string_ref = CFLocaleGetIdentifier(cf_locale_ref);
		CFStringGetCString(cf_locale_string_ref, current_locale, sizeof(current_locale), kCFStringEncodingASCII);
		CFRelease(cf_locale_ref);

		char *p = strchr(current_locale, '.');
		if (p != NULL) {
			*p = 0;
		}
		locale = current_locale;
#elif defined(linux) || defined(__UNIX__)
		locale = setlocale(LC_ALL, "");
		// If diffrent from LC_ALL and LC_MESSAGES, string is enumrated each locale.
		if (locale != NULL && strstr(locale, "LC_") != NULL) {
			locale = setlocale(LC_MESSAGES, "");
		}
#else
		locale = setlocale(LC_ALL, "");
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

	// load catalog
	MakeLocaleFilePath(region->GetFile().Get(), new_package_name);

	if (!ReadCatalog(mLocaleFile.Get())) {
		mLocaleName.Clear();
		return false;
	}

	return true;
}

/// @brief set locale and get locale name
///
/// @param [in] locale   : locale
/// @param [in] category : category
/// @param [in] region   : region
/// @return locale name
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

/// @brief unset locale and get locale name
///
/// @return true
bool CLocale::UnsetLocale()
{
	mCatalog.Clear();
	mLanguageName.Clear();
	return true;
}

/// @brief set locale
///
/// @note SetThreadLocale WinAPI set a locale on current thread only. So, you need call this on every threads.
///
/// @return true
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

/// @brief find existing locale path
///
/// @param [in] new_app_path : application path
/// @return true if exist path
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

/// @brief make catalog file path
///
/// @param [in] locale_name
/// @param [in] package_name
/// @return true
bool CLocale::MakeLocaleFilePath(const _TCHAR *locale_name, const _TCHAR *package_name)
{
	_TCHAR buf[_MAX_PATH];
	UTILITY::tcscpy(buf, _MAX_PATH, mLocalePath.Get());
	UTILITY::add_path_separator(buf, _MAX_PATH);
	UTILITY::tcscat(buf, _MAX_PATH, locale_name);
	UTILITY::add_path_separator(buf, _MAX_PATH);
	UTILITY::tcscat(buf, _MAX_PATH, _T("LC_MESSAGES"));
	UTILITY::add_path_separator(buf, _MAX_PATH);
	UTILITY::tcscat(buf, _MAX_PATH, package_name);
	UTILITY::tcscat(buf, _MAX_PATH, _T(".mo"));
	mLocaleFile.Set(buf);
	return true;
}

/// @brief read .mo catalog
///
/// @param [in] filename : catalog file
/// @return true / false
bool CLocale::ReadCatalog(const _TCHAR *filename)
{
	bool rc = true;
	FILEIO fio;

	uint8_t buf[1024];
	size_t len;


	uint32_t key_count;
	uint32_t key_pos;
	uint32_t sig_pos;
//	uint32_t con_count;
//	uint32_t con_pos;
	uint32_t trans_start = 0;

	do {
		if (!fio.Fopen(filename, FILEIO::READ_BINARY)) {
			rc = false;
			break;
		}

		mCatalog.Clear();

		// read header + revision
		len = fio.Fread(buf, 1, 0x1c);
		if (len < 0x1c) {
			rc = false;
		}
		if (!memcmp(mofile_ident, buf, 4)) {
			swap_buf4(buf);
			if (memcmp(mofile_ident, buf, 4)) {
				mBigEndien = true;
			} else {
				rc = false;
				break;
			}
		}

		// key table
		key_count = to_uint32(&buf[8]) * 2;
		key_pos = to_uint32(&buf[0xc]);
		// signature
		sig_pos = to_uint32(&buf[0x10]);
		// hash table
//		con_count = to_uint32(&buf[0x14]);
//		con_pos = to_uint32(&buf[0x18]);

		if (key_count == 0) {
			rc = false;
			break;
		}

		// read key table
		uint32_t i;
		CPtrList<CKeyTable> key_table;
		fio.Fseek(key_pos, FILEIO::SEEKSET);
		for(i=0; i<key_count; i++) {
			uint32_t now_pos = fio.Ftell();
			if (sig_pos >= now_pos) {
					trans_start = i;
			}
			len = fio.Fread(buf, 1, 8);
			if (len == 0) {
				break;
			}
			key_table.Add(new CKeyTable(to_uint32(buf), to_uint32(&buf[4])));
		}

		// read string and push to keytable
		for(i=0; i<key_count; i++) {
			CKeyTable *itm = key_table[i];
			fio.Fseek(itm->pos, FILEIO::SEEKSET);
			if (itm->len > 0) {
				uint32_t blen = itm->len < 1023 ? itm->len : 1023;
				len = fio.Fread(buf, 1, blen);
				buf[blen] = '\0';
			} else {
				buf[0] = '\0';
			}
			itm->data.Set((const char *)buf);
		}

		// connect original string to translated string
		for(i=0; i<trans_start; i++) {
			CKeyTable *itm_src = key_table[i];
			CKeyTable *itm_dst = key_table[i + trans_start];
			mCatalog.Add(new CLocaleCatalog(itm_src->data.Get(), itm_dst->data.Get()));
		}

		// sort
		mCatalog.SortBySource();

	} while(0);

	return rc;
}

/// @brief read region defined file
///
/// @param [in] filename : region xml file
/// @return true / false
bool CLocale::ReadRegion(const _TCHAR *filename)
{
	bool rc = true;
	FILEIO fio;

	_TCHAR fpath[_MAX_PATH];
	char buf[_MAX_PATH];
	char *p;

	char file[32];
	char name[64];
	char win[64];
	char posix[64];

	do {
		UTILITY::tcscpy(fpath, _MAX_PATH, filename);
		UTILITY::add_path_separator(fpath, _MAX_PATH);
		UTILITY::tcscat(fpath, _MAX_PATH, _T("list.xml"));

		if (!fio.Fopen(fpath, FILEIO::READ_ASCII)) {
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
	UTILITY::strncpy(out, len, p, nlen);
	return true;
}

/// @brief find region
///
/// @param [in] language : language name
/// @return region
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

/// @brief find region
///
/// @param [in] locale : locale name
/// @return region
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

/// @brief find translated string in catalog and return it if found 
///
/// @param [in] str : string
/// @return translated string if found, otherwise pointer of str
///
/// @attention Need duplicate returned string, because clear it when invoke GetText at next time.
const _TCHAR *CLocale::GetText(const _TCHAR *str)
{
	const CLocaleCatalog *itm = mCatalog.MatchSource(str);
	if (itm) {
#if defined(USE_WIN)
		UTILITY::conv_utf8_to_mbs(itm->GetDst().Get(), itm->GetDst().Length(), mTempMessage, _MAX_PATH);
		return mTempMessage;
#else
		return itm->GetDst().Get();
#endif
	} else {
		return str;
	}
}

/// @brief find translated string in catalog and return it if found 
///
/// @param [in] str : string
/// @param [in] char_code : CODE_ACP: convert to MBCS
/// @return translated string if found, otherwise pointer of str
///
/// @attention Need duplicate returned string, because clear it when invoke GetText at next time.
const _TCHAR *CLocale::GetText(const _TCHAR *str, enumCharCodes char_code)
{
	const CLocaleCatalog *itm = mCatalog.MatchSource(str);
	if (itm) {
		if (char_code == CODE_ACP) {
			UTILITY::conv_utf8_to_mbs(itm->GetDst().Get(), itm->GetDst().Length(), mTempMessage, _MAX_PATH);
			return mTempMessage;
		} else {
			return itm->GetDst().Get();
		}
	} else {
		return str;
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

/// @brief change locale if possible
///
/// @param[in] new_language : language name
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
///
/// @param[out] arr : array of locale name
/// @return true
bool CLocale::GetLocaleNames(CPtrList<CTchar> &arr) const
{
	for(int i=0; i<mRegion.Count(); i++) {
		arr.Add(new CTchar(mRegion.Item(i)->GetName().Get()));
	}
	return true;
}

/// @brief get locale names on locale directory (for configure dialog box)
///
/// @param[out] arr : array of locale name
/// @return true
bool CLocale::GetLocaleNamesWithDefault(CPtrList<CTchar> &arr) const
{
	arr.Add(new CTchar(_T("System dependent")));
	arr.Add(new CTchar(_T("Default (English)")));
	return GetLocaleNames(arr);
}

/// @brief select locale name matches specified name (for configure dialog box)
///
/// @param[in] arr  : array of locale name
/// @param[in] name : locale name
/// @return index of matching name in array
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

/// @brief get locale name at specified index in array (for configure dialog box)
///
/// @param[in] arr    : array of locale name
/// @param[in] selidx : index in array
/// @param[out] name  : locale name
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

void CLocale::swap_buf4(uint8_t *buf)
{
	uint8_t sw = buf[0];
	buf[0] = buf[3];
	buf[3] = sw;
	sw = buf[1];
	buf[1] = buf[2];
	buf[2] = sw;
}

uint32_t CLocale::to_uint32(uint8_t *buf)
{
	if (mBigEndien) swap_buf4(buf);
	return (uint32_t)(*(uint32_t *)buf);
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

/// @brief Set lcid from a locale name
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

// set in main.
CLocale *clocale = NULL;
