/** @file simple_ini.h

	@author Sasaji
	@date   2019.09.01

	@brief simple ini
*/

#ifndef SIMPLE_INI_H
#define SIMPLE_INI_H

#include "common.h"
#include "fileio.h"
#include "cchar.h"
#include "cptrlist.h"

/// store value of string/long/bool type
class CSimpleValue
{
private:
	enum {
		ValTypeUnknown = 0,
		ValTypeString,
		ValTypeLong,
		ValTypeLongHex,
		ValTypeDouble,
		ValTypeBool
	} type;
	union {
		CTchar *str;
		long    lng;
		double  dbl;
	} value;

	void Copy(const CSimpleValue &src);

	static bool IsBoolString(const char *src);
	static bool IsDigitString(const char *src);
	static bool IsHexDigitString(const char *src);
	static bool IsDoubleString(const char *src);
	static bool ConvBool(const char *src);
	static long ConvDigit(const char *src, bool use_hex);
	static double ConvDouble(const char *src);

public:
	CSimpleValue();
	CSimpleValue(const CSimpleValue &src);
	~CSimpleValue();
	CSimpleValue &operator=(const CSimpleValue &src);
	void Clear();
	void SetAuto(const char *src);
	void SetString(const _TCHAR *src);
	void SetLong(long src, bool use_hex = false);
	void SetDouble(double src);
	void SetBool(bool src);

	const _TCHAR *GetString() const;
	long GetLong() const;
	double GetDouble() const;
	bool GetBool() const;

	bool IsString() const;
	bool IsLong() const;
	bool IsDouble() const;
	bool IsBool() const;

	void Write(FILEIO &fio) const;
};

/// one hash item
class CSimpleIniItem
{
private:
	CTchar key;
	CSimpleValue value;
	CTchar comment;
public:
	CSimpleIniItem();
#ifdef _UNICODE
	CSimpleIniItem(const wchar_t *key_, const wchar_t *value_, const wchar_t *comment_ = NULL, bool auto_ = false);
	CSimpleIniItem(const wchar_t *key_, const char *value_, const wchar_t *comment_ = NULL, bool auto_ = true);
#else
	CSimpleIniItem(const char *key_, const char *value_, const char *comment_ = NULL, bool auto_ = false);
#endif
	CSimpleIniItem(const _TCHAR *key_, long value_, const _TCHAR *comment_ = NULL, bool use_hex_ = false);
	CSimpleIniItem(const _TCHAR *key_, double value_, const _TCHAR *comment_ = NULL);
	CSimpleIniItem(const _TCHAR *key_, bool value_, const _TCHAR *comment_ = NULL);
	~CSimpleIniItem();
	void SetAuto(const _TCHAR *key_, const char *value_, const _TCHAR *comment_ = NULL);
	void SetString(const _TCHAR *key_, const _TCHAR *value_, const _TCHAR *comment_ = NULL);
	void SetLong(const _TCHAR *key_, long value_, const _TCHAR *comment_ = NULL, bool use_hex_ = false);
	void SetDouble(const _TCHAR *key_, double value_, const _TCHAR *comment_ = NULL);
	void SetBool(const _TCHAR *key_, bool value_, const _TCHAR *comment_ = NULL);

	const _TCHAR *GetString(const _TCHAR *default_value) const;
	long GetLong(long default_value) const;
	double GetDouble(double default_value) const;
	bool GetBool(bool default_value) const;

	const CTchar &GetKey() const { return key; }

	void Write(FILEIO &fio) const;
};

/// list of hash items
class CSimpleIniItems : public CPtrList<CSimpleIniItem>
{
public:
	CSimpleIniItems();
	virtual ~CSimpleIniItems();
	CSimpleIniItem *Find(const _TCHAR *key_);
	int FindIndex(const _TCHAR *key_);
	const CSimpleIniItem *Find(const _TCHAR *key_) const;

	void SetAutoItem(const char *key_, const char *value_, const _TCHAR *comment_ = NULL);
	void SetStringItem(const _TCHAR *key_, const _TCHAR *value_, const _TCHAR *comment_ = NULL);
	void SetLongItem(const _TCHAR *key_, long value_, const _TCHAR *comment_ = NULL, bool use_hex_ = false);
	void SetDoubleItem(const _TCHAR *key_, double value_, const _TCHAR *comment_ = NULL);
	void SetBoolItem(const _TCHAR *key_, bool value_, const _TCHAR *comment_ = NULL);

	const _TCHAR *GetStringItem(const _TCHAR *key_, const _TCHAR *default_value) const;
	long GetLongItem(const _TCHAR *key_, long default_value) const;
	double GetDoubleItem(const _TCHAR *key_, double default_value) const;
	bool GetBoolItem(const _TCHAR *key_, bool default_value) const;

	bool DeleteItem(const _TCHAR *key_);

	void Write(FILEIO &fio) const;
};

/// one section included hash items
class CSimpleIniSection : public CSimpleIniItems
{
private:
	CTchar section;

public:
	CSimpleIniSection();
	CSimpleIniSection(const _TCHAR *section_);
	~CSimpleIniSection();
	void SetSection(const _TCHAR *section_);

	const CTchar &GetSection() const { return section; }

	void Write(FILEIO &fio) const;
};

/// list of section
class CSimpleIniSections : public CPtrList<CSimpleIniSection>
{
public:
	CSimpleIniSections();
	virtual ~CSimpleIniSections();
	CSimpleIniSection *Find(const _TCHAR *section_);
	const CSimpleIniSection *Find(const _TCHAR *section_) const;

	void Write(FILEIO &fio) const;
};

/// access to simple ini file
class CSimpleIni : public CSimpleIniSections
{
private:
	void SetSection(const _TCHAR &section_);
	CSimpleIniSection *FindAndSetSection(const char *section_);
#ifdef _UNICODE
	CSimpleIniSection *FindAndSetSection(const wchar_t *section_);
#endif
	void SetComment(const char *str, CTchar &comment);
	void Delete(int num);	// overload

public:
	CSimpleIni();
	~CSimpleIni();

	bool LoadFile(const _TCHAR *file_path);
	bool SaveFile(const _TCHAR *file_path);

	const _TCHAR *GetValue(const _TCHAR *section, const _TCHAR *key, const _TCHAR *default_value = NULL) const;
	long GetLongValue(const _TCHAR *section, const _TCHAR *key, long default_value = 0) const;
	double GetDoubleValue(const _TCHAR *section, const _TCHAR *key, double default_value = 0.0) const;
	bool GetBoolValue(const _TCHAR *section, const _TCHAR *key, bool default_value = false) const;

	bool Delete(const _TCHAR *section, const _TCHAR *key);

	bool SetValue(const _TCHAR *section, const _TCHAR *key, const _TCHAR *value, const _TCHAR *comment = NULL);
	bool SetLongValue(const _TCHAR *section, const _TCHAR *key, long value, const _TCHAR *comment = NULL, bool use_hex = false);
	bool SetDoubleValue(const _TCHAR *section, const _TCHAR *key, double value, const _TCHAR *comment = NULL);
	bool SetBoolValue(const _TCHAR *section, const _TCHAR *key, bool value, const _TCHAR *comment = NULL);
};

#endif /* SIMPLE_INI_H */
