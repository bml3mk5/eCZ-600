/** @file cchar.h

	@author Sasaji
	@date   2014.12.01

	@brief 文字列リスト
*/

#ifndef CCHAR_H
#define CCHAR_H

#include "common.h"
#include <wchar.h>

/// @brief 文字列テンプレート
template <class TYPE>
class CCharTemp
{
protected:
	TYPE *str;
	int   len;

public:
	CCharTemp() {
		this->str = NULL;
		this->len = -1;
		this->CopyData(NULL, 0);
	}
	CCharTemp(const CCharTemp<TYPE> &src) {
		this->str = NULL;
		this->len = -1;
		this->CopyData(src.str, src.len);
	}
	CCharTemp(const TYPE *src_str, int src_len) {
		this->str = NULL;
		this->len = -1;
		this->CopyData(src_str, src_len);
	}
	virtual ~CCharTemp() {
		delete[] str;
	}
	virtual int Length() const {
		return len;
	}
	virtual void Clear() {
		CopyData(NULL, 0);
	}
	virtual void Set(const CCharTemp<TYPE> &src) {
		CopyData(src.str, src.len);
	}
	virtual void Set(const TYPE *src_str) = 0;
	virtual void Set(const TYPE *src_str, int src_len) {
		CopyData(src_str, src_len);
	}
	virtual CCharTemp<TYPE> &operator=(const CCharTemp<TYPE> &src) {
		CopyData(src.str, src.len);
		return *this;
	}
	virtual const TYPE *Get() const {
		return str;
	}
	virtual operator const TYPE *() const {
		return str;
	}
	virtual bool operator==(const CCharTemp<TYPE> &src) const {
		return Equal(src.str, src.len);
	}
	virtual int Compare(const CCharTemp<TYPE> &src) const {
		size_t l = (len > src.len ? src.len : len);
		int c = memcmp(str, src.str, l * sizeof(TYPE));
		if (c == 0) c = (len - src.len);
		return c;
	}
	virtual int Compare(const TYPE *src_str, int src_len) const {
		size_t l = (len > src_len ? src_len : len);
		int c = memcmp(str, src_str, l * sizeof(TYPE));
		if (c == 0) c = (len - src_len);
		return c;
	}

protected:
	CCharTemp(int size) {
		this->str = new TYPE[size + 1];
		this->len = 0;
		memset(this->str, 0, (size + 1) * sizeof(TYPE));
	}
	virtual void CopyData(const TYPE *src_str, int src_len) {
		if (len != src_len) {
			delete[] str;
			str = new TYPE[src_len + 1];
			len = src_len;
		}
		if (src_str && src_len > 0) memcpy(str, src_str, src_len * sizeof(TYPE));
		str[src_len] = (TYPE)0;
	}
	virtual bool Equal(const TYPE *src_str, int src_len) const {
		if (src_str == NULL || str == NULL || src_len != len) return false;
		return (memcmp(src_str, str, src_len * sizeof(TYPE)) == 0);
	}
	TYPE *Ptr() { return str; }

private:
	CCharTemp(const TYPE *src_str) {
		this->str = NULL;
		this->len = -1;
	}
	operator TYPE *() const { return str; }
};

class CWchar;

/// @brief 文字列ナロー
class CNchar : public CCharTemp<char>
{
public:
	friend class CWchar;

	CNchar();
	CNchar(const CNchar &src);
	CNchar(const char *src_str);
	CNchar(const char *src_str, int src_len);
	CNchar(const wchar_t *src_str);
	CNchar(const wchar_t *src_str, int src_len);
	CNchar(int size);
	virtual ~CNchar();
	virtual void Set(const char *src_str);
	virtual void SetN(const char *src_str);
	virtual void SetN(const char *src_str, int src_len);
	virtual const char *GetN() const;
	virtual void GetN(char *dst_str, int dst_len) const;
	virtual void SetW(const wchar_t *src_str);
	virtual void SetW(const wchar_t *src_str, int src_len);
	virtual CWchar GetW() const;
	virtual void GetW(wchar_t *dst_str, int dst_len) const;
	virtual bool MatchString(const char *value) const;
	virtual bool MatchSubString(const char *value) const;

	virtual CNchar GetM() const;
	virtual void SetM(const char *src_str);
	virtual CWchar GetWM() const;

private:
	virtual CNchar &operator=(const CNchar &src);
	virtual CNchar &operator=(const char *src_str);
};

/// @brief 文字列ワイド
class CWchar : public CCharTemp<wchar_t>
{
public:
	friend class CNchar;

	CWchar();
	CWchar(const CWchar &src);
	CWchar(const wchar_t *src_str);
	CWchar(const wchar_t *src_str, int src_len);
	CWchar(const char *src_str);
	CWchar(const char *src_str, int src_len);
	CWchar(int size);
	virtual ~CWchar();
	virtual void Set(const wchar_t *src_str);
	virtual const wchar_t *GetW() const;
	virtual void GetW(wchar_t *dst_str, int dst_len) const;
	virtual void SetN(const char *src_str);
	virtual void SetN(const char *src_str, int src_len);
	virtual CNchar GetN(int codepage = -1) const;
	virtual void GetN(char *dst_str, int dst_len, int codepage = -1) const;
	virtual bool MatchString(const wchar_t *value) const;
	virtual bool MatchSubString(const wchar_t *value) const;

	virtual const wchar_t *GetM() const;
	virtual void SetM(const wchar_t *src_str);
	virtual const wchar_t *GetWM() const;

private:
	virtual CWchar &operator=(const CWchar &src);
	virtual CWchar &operator=(const wchar_t *src_str);
};

#if defined(_UNICODE)
#define CTchar CWchar
#else
#define CTchar CNchar
#endif

#endif /* CCHAR_H */
