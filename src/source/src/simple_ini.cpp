/** @file simple_ini.cpp

	@author Sasaji
	@date   2019.09.01

	@brief simple ini
*/

#include "simple_ini.h"
#include "utility.h"


//

CSimpleValue::CSimpleValue()
{
	type = ValTypeUnknown;
	value.str = NULL;
}
CSimpleValue::CSimpleValue(const CSimpleValue &src)
{
	Copy(src);
}
CSimpleValue::~CSimpleValue()
{
	Clear();
}
/// @brief copy value
///
/// @param[in] src
/// @return value
CSimpleValue &CSimpleValue::operator=(const CSimpleValue &src)
{
	Copy(src);
	return *this;
}
/// @brief clear value
void CSimpleValue::Clear()
{
	if (type == ValTypeString) {
		delete value.str;
	}
	value.str = NULL;
	type = ValTypeUnknown;
}
/// @brief copy value
///
/// @param[in] src
void CSimpleValue::Copy(const CSimpleValue &src)
{
	switch(src.type) {
	case ValTypeString:
		SetString(src.value.str->Get());
		break;
	case ValTypeLong:
		SetLong(src.value.lng, false);
		break;
	case ValTypeLongHex:
		SetLong(src.value.lng, true);
		break;
	case ValTypeDouble:
		SetDouble(src.value.dbl);
		break;
	case ValTypeBool:
		SetBool(src.value.lng != 0); 
		break;
	default:
		break;
	}
}
/// @brief set value in which type is decided by format
///
/// @param[in] src
void CSimpleValue::SetAuto(const char *src)
{
	// check type of value
	if (IsBoolString(src)) {
		SetBool(ConvBool(src));
	} else if (IsDoubleString(src)) {
		SetDouble(ConvDouble(src));
	} else if (IsHexDigitString(src)) {
		SetLong(ConvDigit(src, true), true);
	} else if (IsDigitString(src)) {
		SetLong(ConvDigit(src, false), false);
	} else {
#ifdef _UNICODE
		wchar_t tsrc[_MAX_PATH];
		UTILITY::conv_to_native_path(src, tsrc, _MAX_PATH);
#else
		const char *tsrc = src;
#endif
		SetString(tsrc);
	}
}
/// @brief set string value
///
/// @param[in] src
void CSimpleValue::SetString(const _TCHAR *src)
{
	Clear();

	type = ValTypeString;
	value.str = new CTchar(src);
}
/// @brief set long value
///
/// @param[in] src
/// @param[in] use_hex
void CSimpleValue::SetLong(long src, bool use_hex)
{
	Clear();

	type = use_hex ? ValTypeLongHex : ValTypeLong;
	value.lng = src;
}
/// @brief set double value
///
/// @param[in] src
void CSimpleValue::SetDouble(double src)
{
	Clear();

	type = ValTypeDouble;
	value.dbl = src;
}
/// @brief set bool value
///
/// @param[in] src
void CSimpleValue::SetBool(bool src)
{
	Clear();

	type = ValTypeBool;
	value.lng = src ? 1 : 0;
}
/// @brief get string value
///
/// @return value
const _TCHAR *CSimpleValue::GetString() const
{
	return value.str->Get();
}
/// @brief get long value
///
/// @return value
long CSimpleValue::GetLong() const
{
	return value.lng;
}
/// @brief get double value
///
/// @return value
double CSimpleValue::GetDouble() const
{
	return value.dbl;
}
/// @brief get bool value
///
/// @return value
bool CSimpleValue::GetBool() const
{
	return value.lng ? true : false;
}
/// @brief type is string ?
///
/// @return true / false
bool CSimpleValue::IsString() const
{
	return (type == ValTypeString);
}
/// @brief type is long ?
///
/// @return true / false
bool CSimpleValue::IsLong() const
{
	return (type == ValTypeLong || type == ValTypeLongHex);
}
/// @brief type is double ?
///
/// @return true / false
bool CSimpleValue::IsDouble() const
{
	return (type == ValTypeDouble);
}
/// @brief type is bool ?
///
/// @return true / false
bool CSimpleValue::IsBool() const
{
	return (type == ValTypeBool);
}
/// @brief string is bool ?
///
/// @param[in] src
/// @return true / false
bool CSimpleValue::IsBoolString(const char *src)
{
	bool match =
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
		(_stricmp(src, "true") == 0
		|| _stricmp(src, "false") == 0);
#else
		(strcasecmp(src, "true") == 0
		|| strcasecmp(src, "false") == 0);
#endif
	return match;
}
/// @brief string is digit ?
///
/// @param[in] src
/// @return true / false
bool CSimpleValue::IsDigitString(const char *src)
{
	const char *p = src;
	bool match = (*p != '\0');
	while(match && *p != '\0') {
		if (*p != '-' && (*p < '0' || '9' < *p)) {
			match = false;
			break;
		}
		p++;
	}
	return match;
}
/// @brief string is hexa ?
///
/// @param[in] src
/// @return true / false
bool CSimpleValue::IsHexDigitString(const char *src)
{
	int len = (int)strlen(src);
	bool match = (len > 2);
	if (match) {
		match = (strstr(src, "0x") != NULL);
	}
	if (match) {
		const char *p = src + 2;
		match = (*p != '\0');
		while(match && *p != '\0') {
			if ((*p < '0' || '9' < *p) && (*p < 'A' || 'F' < *p) && (*p < 'a' || 'f' < *p)) {
				match = false;
				break;
			}
			p++;
		}
	}
	return match;
}
/// @brief string is double
///
/// @param[in] src
/// @return true / false
bool CSimpleValue::IsDoubleString(const char *src)
{
	bool match = true;
	const char *p = strchr(src, '.');
	if (!p) return false;

	p++;
	while(match && *p != '\0') {
		if (*p < '0' || '9' < *p) {
			match = false;
			break;
		}
		p++;
	}
	return match;
}
/// @brief convert string to bool
///
/// @param[in] src
/// @return bool
bool CSimpleValue::ConvBool(const char *src)
{
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
	bool val = (_stricmp(src, "true") == 0);
	if (!val) {
		val = (_stricmp(src, "false") == 0);
		val = !val;
	}
#else
	bool val = (strcasecmp(src, "true") == 0);
	if (!val) {
		val = (strcasecmp(src, "false") == 0);
		val = !val;
	}
#endif
	return val;
}
/// @brief convert string to digit
///
/// @param[in] src
/// @param[in] use_hex : string is hexa
/// @return digit
long CSimpleValue::ConvDigit(const char *src, bool use_hex)
{
	int val = 0;
	if (use_hex) {
		sscanf(src + 2, "%x", &val);
	} else {
		sscanf(src, "%d", &val);
	}
	return (long)val;
}
/// @brief convert string to double
///
/// @param[in] src
/// @return double
double CSimpleValue::ConvDouble(const char *src)
{
	double val = 0;
	sscanf(src, "%lf", &val);
	return val;
}
/// @brief write value string to file
///
/// @param[in] fio : file object
void CSimpleValue::Write(FILEIO &fio) const
{
	switch(type) {
	case ValTypeString:
	{
#ifdef _UNICODE
		char tvalue[_MAX_PATH];
		UTILITY::cconv_from_native_path(value.str->Get(), tvalue, _MAX_PATH);
#else
		const char *tvalue = value.str->Get();
#endif
		fio.Fputs(tvalue);
		break;
	}
	case ValTypeLong:
		fio.Fprintf("%d", (int)value.lng);
		break;
	case ValTypeLongHex:
		fio.Fprintf("0x%x", (int)value.lng);
		break;
	case ValTypeDouble:
		fio.Fprintf("%lf", value.dbl);
		break;
	case ValTypeBool:
		fio.Fputs(value.lng ? "true" : "false");
		break;
	default:
		break;
	}
}

//

CSimpleIniItem::CSimpleIniItem()
{
}
#ifdef _UNICODE
/// @brief set value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] auto_ : type is decided by format in value if true, otherwise value type is always string
CSimpleIniItem::CSimpleIniItem(const wchar_t *key_, const wchar_t *value_, const wchar_t *comment_, bool auto_)
{
	if (!auto_) SetString(key_, value_, comment_);
}
/// @brief set value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] auto_ : type is decided by format in value if true, otherwise value type is always string
CSimpleIniItem::CSimpleIniItem(const wchar_t *key_, const char *value_, const wchar_t *comment_, bool auto_)
{
	if (auto_) SetAuto(key_, value_, comment_);
}
#else
/// @brief set value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] auto_ : type is decided by format in value if true, otherwise value type is always string
CSimpleIniItem::CSimpleIniItem(const char *key_, const char *value_, const char *comment_, bool auto_)
{
	if (auto_) SetAuto(key_, value_, comment_);
	else SetString(key_, value_, comment_);
}
#endif
/// @brief set long value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] use_hex_ : output as hexa string
CSimpleIniItem::CSimpleIniItem(const _TCHAR *key_, long value_, const _TCHAR *comment_, bool use_hex_)
{
	SetLong(key_, value_, comment_, use_hex_);
}
/// @brief set double value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
CSimpleIniItem::CSimpleIniItem(const _TCHAR *key_, double value_, const _TCHAR *comment_)
{
	SetDouble(key_, value_, comment_);
}
/// @brief set bool value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
CSimpleIniItem::CSimpleIniItem(const _TCHAR *key_, bool value_, const _TCHAR *comment_)
{
	SetBool(key_, value_, comment_);
}
CSimpleIniItem::~CSimpleIniItem()
{
}
/// @brief set value
///
/// value type is decided by format in value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItem::SetAuto(const _TCHAR *key_, const char *value_, const _TCHAR *comment_)
{
	key.Set(key_);
	value.SetAuto(value_);
	if (comment_) comment.Set(comment_);
}
/// @brief set string value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItem::SetString(const _TCHAR *key_, const _TCHAR *value_, const _TCHAR *comment_)
{
	key.Set(key_);
	value.SetString(value_);
	if (comment_) comment.Set(comment_);
}
/// @brief set long value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] use_hex_ : output as hexa string
void CSimpleIniItem::SetLong(const _TCHAR *key_, long value_, const _TCHAR *comment_, bool use_hex_)
{
	key.Set(key_);
	value.SetLong(value_, use_hex_);
	if (comment_) comment.Set(comment_);
}
/// @brief set double value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItem::SetDouble(const _TCHAR *key_, double value_, const _TCHAR *comment_)
{
	key.Set(key_);
	value.SetDouble(value_);
	if (comment_) comment.Set(comment_);
}
/// @brief set bool value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItem::SetBool(const _TCHAR *key_, bool value_, const _TCHAR *comment_)
{
	key.Set(key_);
	value.SetBool(value_);
	if (comment_) comment.Set(comment_);
}
/// @brief get string value
///
/// @param[in] default_value : return value if mismatch type
/// @return value
const _TCHAR *CSimpleIniItem::GetString(const _TCHAR *default_value) const
{
	return value.IsString() ? value.GetString() : default_value;
}
/// @brief get long value
///
/// @param[in] default_value : return value if mismatch type
/// @return value
long CSimpleIniItem::GetLong(long default_value) const
{
	return value.IsLong() ? value.GetLong() : default_value;
}
/// @brief get double value
///
/// @param[in] default_value : return value if mismatch type
/// @return value
double CSimpleIniItem::GetDouble(double default_value) const
{
	return value.IsDouble() ? value.GetDouble() : default_value;
}
/// @brief get bool value
///
/// @param[in] default_value : return value if mismatch type
/// @return value
bool CSimpleIniItem::GetBool(bool default_value) const
{
	return value.IsBool() ? value.GetBool() : default_value;
}
/// @brief write item string to file
///
/// @param[in] fio : file object
void CSimpleIniItem::Write(FILEIO &fio) const
{
	if (comment.Length() > 0) {
#ifdef _UNICODE
		char tcomment[_MAX_PATH];
		UTILITY::cconv_from_native_path(comment.Get(), tcomment, _MAX_PATH);
#else
		const char *tcomment = comment.Get();
#endif
		fio.Fputs(tcomment);
		fio.Fputs("\n");
	}
#ifdef _UNICODE
	char tkey[_MAX_PATH];
	UTILITY::cconv_from_native_path(key.Get(), tkey, _MAX_PATH);
#else
	const char *tkey = key.Get();
#endif
	fio.Fputs(tkey);
	fio.Fputs(" = ");
	value.Write(fio);
	fio.Fputs("\n");
}

//

CSimpleIniItems::CSimpleIniItems()
	: CPtrList<CSimpleIniItem>()
{
}
CSimpleIniItems::~CSimpleIniItems()
{
}
/// @brief find key
///
/// @param[in] key_
/// @return item if found, otherwise NULL
CSimpleIniItem *CSimpleIniItems::Find(const _TCHAR *key_)
{
	CSimpleIniItem *match = NULL;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetKey().MatchString(key_)) {
			match = Item(i);
			break;
		}
	}
	return match;
}
/// @brief find key
///
/// @param[in] key_
/// @return item if found, otherwise NULL
const CSimpleIniItem *CSimpleIniItems::Find(const _TCHAR *key_) const
{
	const CSimpleIniItem *match = NULL;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetKey().MatchString(key_)) {
			match = Item(i);
			break;
		}
	}
	return match;
}
/// @brief find key and return index if found
///
/// @param[in] key_
/// @return index if found, otherwise -1
int CSimpleIniItems::FindIndex(const _TCHAR *key_)
{
	int num = -1;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetKey().MatchString(key_)) {
			num = i;
			break;
		}
	}
	return num;
}
/// @brief set value
///
/// value type is decided by format in value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItems::SetAutoItem(const char *key_, const char *value_, const _TCHAR *comment_)
{
#ifdef _UNICODE
	wchar_t tkey[_MAX_PATH];
	UTILITY::conv_to_native_path(key_, tkey, _MAX_PATH);
#else
	const char *tkey = key_;
#endif
	CSimpleIniItem *match = Find(tkey);
	if (match) {
		match->SetAuto(tkey, value_, comment_);
	} else {
		Add(new CSimpleIniItem(tkey, value_, comment_, true));
	}
}
/// @brief set string value
///
/// update value if key found, otherwise set new key and value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItems::SetStringItem(const _TCHAR *key_, const _TCHAR *value_, const _TCHAR *comment_)
{
	CSimpleIniItem *match = Find(key_);
	if (match) {
		match->SetString(key_, value_, comment_);
	} else {
		Add(new CSimpleIniItem(key_, value_, comment_, false));
	}
}
/// @brief set long value
///
/// update value if key found, otherwise set new key and value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
/// @param[in] use_hex_ : output as hexa string
void CSimpleIniItems::SetLongItem(const _TCHAR *key_, long value_, const _TCHAR *comment_, bool use_hex_)
{
	CSimpleIniItem *match = Find(key_);
	if (match) {
		match->SetLong(key_, value_, comment_, use_hex_);
	} else {
		Add(new CSimpleIniItem(key_, value_, comment_, use_hex_));
	}
}
/// @brief set double value
///
/// update value if key found, otherwise set new key and value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItems::SetDoubleItem(const _TCHAR *key_, double value_, const _TCHAR *comment_)
{
	CSimpleIniItem *match = Find(key_);
	if (match) {
		match->SetDouble(key_, value_, comment_);
	} else {
		Add(new CSimpleIniItem(key_, value_, comment_));
	}
}
/// @brief set bool value
///
/// update value if key found, otherwise set new key and value
///
/// @param[in] key_
/// @param[in] value_
/// @param[in] comment_ : nullable
void CSimpleIniItems::SetBoolItem(const _TCHAR *key_, bool value_, const _TCHAR *comment_)
{
	CSimpleIniItem *match = Find(key_);
	if (match) {
		match->SetBool(key_, value_, comment_);
	} else {
		Add(new CSimpleIniItem(key_, value_, comment_));
	}
}
/// @brief get string value
///
/// @param[in] key_
/// @param[in] default_value : return value if key is not found
/// @return value
const _TCHAR *CSimpleIniItems::GetStringItem(const _TCHAR *key_, const _TCHAR *default_value) const
{
	const CSimpleIniItem *match_item = Find(key_);
	if (!match_item) {
		return default_value;
	}
	return match_item->GetString(default_value);
}
/// @brief get long value
///
/// @param[in] key_
/// @param[in] default_value : return value if key is not found
/// @return value
long CSimpleIniItems::GetLongItem(const _TCHAR *key_, long default_value) const
{
	const CSimpleIniItem *match_item = Find(key_);
	if (!match_item) {
		return default_value;
	}
	return match_item->GetLong(default_value);
}
/// @brief get double value
///
/// @param[in] key_
/// @param[in] default_value : return value if key is not found
/// @return value
double CSimpleIniItems::GetDoubleItem(const _TCHAR *key_, double default_value) const
{
	const CSimpleIniItem *match_item = Find(key_);
	if (!match_item) {
		return default_value;
	}
	return match_item->GetDouble(default_value);
}
/// @brief get bool value
///
/// @param[in] key_
/// @param[in] default_value : return value if key is not found
/// @return value
bool CSimpleIniItems::GetBoolItem(const _TCHAR *key_, bool default_value) const
{
	const CSimpleIniItem *match_item = Find(key_);
	if (!match_item) {
		return default_value;
	}
	return match_item->GetBool(default_value);
}
/// @brief delete item
///
/// @param[in] key_
/// @return true if success
bool CSimpleIniItems::DeleteItem(const _TCHAR *key_)
{
	int match_idx = FindIndex(key_);
	if (match_idx >= 0) {
		Delete(match_idx);
	}
	return (match_idx >= 0);
}
/// @brief write items to file
///
/// @param[in] fio : file object
void CSimpleIniItems::Write(FILEIO &fio) const
{
	for(int i=0; i<Count(); i++) {
		const CSimpleIniItem *item = Item(i);
		item->Write(fio);
	}
}

//

CSimpleIniSection::CSimpleIniSection()
	: CSimpleIniItems()
{
}
CSimpleIniSection::CSimpleIniSection(const _TCHAR *section_)
	: CSimpleIniItems()
{
	SetSection(section_);
}
CSimpleIniSection::~CSimpleIniSection()
{
}
/// @brief set section
///
/// @param[in] section_
void CSimpleIniSection::SetSection(const _TCHAR *section_)
{
	section.Set(section_);
}
/// @brief write section string to file
///
/// @param[in] fio : file object
void CSimpleIniSection::Write(FILEIO &fio) const
{
#ifdef _UNICODE
	char tsection[_MAX_PATH];
	UTILITY::cconv_from_native_path(section.Get(), tsection, _MAX_PATH);
#else
	const char *tsection = section.Get();
#endif
	fio.Fputs("\n");
	if (section.Length() > 0) {
		fio.Fputs("\n[");
		fio.Fputs(tsection);
		fio.Fputs("]\n");
	}
	CSimpleIniItems::Write(fio);
}

//

CSimpleIniSections::CSimpleIniSections()
	: CPtrList<CSimpleIniSection>()
{
}
CSimpleIniSections::~CSimpleIniSections()
{
}
/// @brief find section
///
/// @param[in] section_
/// @return section if found
CSimpleIniSection *CSimpleIniSections::Find(const _TCHAR *section_)
{
	CSimpleIniSection *match = NULL;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetSection().MatchString(section_)) {
			match = Item(i);
			break;
		}
	}
	return match;
}
/// @brief find section
///
/// @param[in] section_
/// @return section if found
const CSimpleIniSection *CSimpleIniSections::Find(const _TCHAR *section_) const
{
	const CSimpleIniSection *match = NULL;
	for(int i=0; i<Count(); i++) {
		if (Item(i)->GetSection().MatchString(section_)) {
			match = Item(i);
			break;
		}
	}
	return match;
}
/// @brief write to file
///
/// @param[in] fio : file object
void CSimpleIniSections::Write(FILEIO &fio) const
{
	for(int sec=0; sec<Count(); sec++) {
		const CSimpleIniSection *section = Item(sec);
		section->Write(fio);
	}
}

//

CSimpleIni::CSimpleIni()
	: CSimpleIniSections()
{
}

CSimpleIni::~CSimpleIni()
{
}

/// @brief load from file
///
/// @param[in] file_path
/// @return true if success, otherwise false
bool CSimpleIni::LoadFile(const _TCHAR *file_path)
{
	FILEIO fio;
	if (!fio.Fopen(file_path, FILEIO::READ_ASCII)) {
		return false;
	}

	Clear();

	// add empty section
	CSimpleIniSection *current_section = new CSimpleIniSection();
	Add(current_section);

	CTchar comment;

	char line[1024];
	char *cp = NULL;
	char *ep = NULL;
	while(1) {
		cp = fio.Fgets(line, (int)sizeof(line));
		if (cp == NULL) break;

		UTILITY::chomp_crlf(line);
		cp = UTILITY::lskip(line);

		if (*cp == '\0') {
			// empty line
			continue;
		}
		if (*cp == '[') {
			// section
			cp = UTILITY::lskip(cp + 1);
			ep = strchr(cp, ']');
			if (ep != NULL) {
				*ep = '\0';
				UTILITY::rtrim(cp);
				current_section = FindAndSetSection(cp);
			}
			comment.Clear();
			continue;
		}

		ep = strchr(cp + 1, '=');
		if (*cp == ';' || ep == NULL) {
			// comment line
			SetComment(cp, comment);
			continue;

		}

		// key = value
		*ep = '\0';
		char *kp = cp;
		char *vp = UTILITY::lskip(ep + 1);
		UTILITY::rtrim(kp);
		UTILITY::rtrim(vp);

		current_section->SetAutoItem(kp, vp, comment.Length() > 0 ? comment.Get() : NULL);

		comment.Clear();
	}

	fio.Fclose();
	return true;
}

/// @brief find section, make new section if not found
///
/// @param[in] section_
/// @return section if found, otherwise make new section
CSimpleIniSection *CSimpleIni::FindAndSetSection(const char *section_)
{
#ifdef _UNICODE
	wchar_t tsection[_MAX_PATH];
	UTILITY::conv_to_native_path(section_, tsection, _MAX_PATH);
#else
	const char *tsection = section_;
#endif
	CSimpleIniSection *match = Find(tsection);
	if (!match) {
		// add section
		match = new CSimpleIniSection(tsection);
		Add(match);
	}
	return match;
}

#ifdef _UNICODE
/// @brief find section, make new section if not found
///
/// @param[in] section_
/// @return section if found, otherwise make new section
CSimpleIniSection *CSimpleIni::FindAndSetSection(const wchar_t *section_)
{
	CSimpleIniSection *match = Find(section_);
	if (!match) {
		// add section
		match = new CSimpleIniSection(section_);
		Add(match);
	}
	return match;
}
#endif

/// @brief set comment
///
/// @param[in]  str
/// @param[out] comment
void CSimpleIni::SetComment(const char *str, CTchar &comment)
{
#ifdef _UNICODE
	wchar_t tstr[_MAX_PATH];
	UTILITY::conv_to_native_path(str, tstr, _MAX_PATH);
#else
	const char *tstr = str;
#endif
	comment.Set(tstr);
}

/// @brief delete item (overloaded)
void CSimpleIni::Delete(int num)
{
	CSimpleIniSections::Delete(num);
}

/// @brief save to file
///
/// @param[in] file_path
/// @return true if success, otherwise false
bool CSimpleIni::SaveFile(const _TCHAR *file_path)
{
	FILEIO fio;
	if (!fio.Fopen(file_path, FILEIO::WRITE_ASCII)) {
		return false;
	}
	Write(fio);
	return true;
}

/// @brief get string value
///
/// @param[in] section
/// @param[in] key
/// @param[in] default_value : return value if section or key is not found
/// @return true if success, otherwise false
const _TCHAR *CSimpleIni::GetValue(const _TCHAR *section, const _TCHAR *key, const _TCHAR *default_value) const
{
	const CSimpleIniSection *match_section = Find(section);
	if (!match_section) {
		return default_value;
	}
	return match_section->GetStringItem(key, default_value);
}

/// @brief get long value
///
/// @param[in] section
/// @param[in] key
/// @param[in] default_value : return value if section or key is not found
/// @return true if success, otherwise false
long CSimpleIni::GetLongValue(const _TCHAR *section, const _TCHAR *key, long default_value) const
{
	const CSimpleIniSection *match_section = Find(section);
	if (!match_section) {
		return default_value;
	}
	return match_section->GetLongItem(key, default_value);
}

/// @brief get double value
///
/// @param[in] section
/// @param[in] key
/// @param[in] default_value : return value if section or key is not found
/// @return true if success, otherwise false
double CSimpleIni::GetDoubleValue(const _TCHAR *section, const _TCHAR *key, double default_value) const
{
	const CSimpleIniSection *match_section = Find(section);
	if (!match_section) {
		return default_value;
	}
	return match_section->GetDoubleItem(key, default_value);
}

/// @brief get bool value
///
/// @param[in] section
/// @param[in] key
/// @param[in] default_value : return value if section or key is not found
/// @return true if success, otherwise false
bool CSimpleIni::GetBoolValue(const _TCHAR *section, const _TCHAR *key, bool default_value) const
{
	const CSimpleIniSection *match_section = Find(section);
	if (!match_section) {
		return default_value;
	}
	return match_section->GetBoolItem(key, default_value);
}

/// @brief delete value of specified key
///
/// @param[in] section
/// @param[in] key
/// @return true if success, otherwise false
bool CSimpleIni::Delete(const _TCHAR *section, const _TCHAR *key)
{
	CSimpleIniSection *match_section = Find(section);
	if (!match_section) {
		return false;
	}
	return match_section->DeleteItem(key);
}

/// @brief set string value
///
/// @param[in] section : make new section if not found
/// @param[in] key
/// @param[in] value
/// @param[in] comment : nullable
/// @return true
bool CSimpleIni::SetValue(const _TCHAR *section, const _TCHAR *key, const _TCHAR *value, const _TCHAR *comment)
{
	CSimpleIniSection *match_section = FindAndSetSection(section);
	match_section->SetStringItem(key, value, comment);
	return true;
}

/// @brief set long value
///
/// @param[in] section : make new section if not found
/// @param[in] key
/// @param[in] value
/// @param[in] comment : nullable
/// @param[in] use_hex : output as hexa string
/// @return true
bool CSimpleIni::SetLongValue(const _TCHAR *section, const _TCHAR *key, long value, const _TCHAR *comment, bool use_hex)
{
	CSimpleIniSection *match_section = FindAndSetSection(section);
	match_section->SetLongItem(key, value, comment, use_hex);
	return true;
}

/// @brief set double value
///
/// @param[in] section : make new section if not found
/// @param[in] key
/// @param[in] value
/// @param[in] comment : nullable
/// @return true
bool CSimpleIni::SetDoubleValue(const _TCHAR *section, const _TCHAR *key, double value, const _TCHAR *comment)
{
	CSimpleIniSection *match_section = FindAndSetSection(section);
	match_section->SetDoubleItem(key, value, comment);
	return true;
}

/// @brief set bool value
///
/// @param[in] section : make new section if not found
/// @param[in] key
/// @param[in] value
/// @param[in] comment : nullable
/// @return true
bool CSimpleIni::SetBoolValue(const _TCHAR *section, const _TCHAR *key, bool value, const _TCHAR *comment)
{
	CSimpleIniSection *match_section = FindAndSetSection(section);
	match_section->SetBoolItem(key, value, comment);
	return true;
}
