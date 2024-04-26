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
	TYPE *m_str;
	int   m_len;

public:
	CCharTemp() {
		m_str = NULL;
		m_len = -1;
		CopyData(NULL, 0);
	}
	CCharTemp(const CCharTemp<TYPE> &src) {
		m_str = NULL;
		m_len = -1;
		CopyData(src.m_str, src.m_len);
	}
	CCharTemp(const TYPE *src_str, int src_len) {
		m_str = NULL;
		m_len = -1;
		CopyData(src_str, src_len);
	}
	virtual ~CCharTemp() {
		delete[] m_str;
	}
	virtual int Length() const {
		return m_len;
	}
	virtual void Clear() {
		CopyData(NULL, 0);
	}
	virtual void Set(const CCharTemp<TYPE> &src) {
		CopyData(src.m_str, src.m_len);
	}
	virtual void Set(const TYPE *src_str) = 0;
	virtual void Set(const TYPE *src_str, int src_len) {
		CopyData(src_str, src_len);
	}
#if 0
	virtual CCharTemp<TYPE> &operator=(const CCharTemp<TYPE> &src) {
		Set(src.m_str, src.m_len);
		return *this;
	}
	virtual CCharTemp<TYPE> &operator=(const TYPE *src_str) {
		Set(src_str);
		return *this;
	}
#endif
	virtual const TYPE *Get() const {
		return m_str;
	}
#if 0
	virtual operator const TYPE *() const {
		return m_str;
	}
#endif
	virtual bool operator==(const CCharTemp<TYPE> &src) const {
		return Equal(src.m_str, src.m_len);
	}
	virtual int Compare(const CCharTemp<TYPE> &src) const {
		size_t l = (m_len > src.m_len ? src.m_len : m_len);
		int c = memcmp(m_str, src.m_str, l * sizeof(TYPE));
		if (c == 0) c = (m_len - src.m_len);
		return c;
	}
	virtual int Compare(const TYPE *src_str, int src_len) const {
		size_t l = (m_len > src_len ? src_len : m_len);
		int c = memcmp(m_str, src_str, l * sizeof(TYPE));
		if (c == 0) c = (m_len - src_len);
		return c;
	}

protected:
	CCharTemp(int size) {
		m_str = new TYPE[size + 1];
		m_len = 0;
		memset(m_str, 0, (size + 1) * sizeof(TYPE));
	}
	virtual void CopyData(const TYPE *src_str, int src_len) {
		if (m_len != src_len) {
			delete[] m_str;
			m_str = new TYPE[src_len + 1];
			m_len = src_len;
		}
		if (src_str && src_len > 0) memcpy(m_str, src_str, src_len * sizeof(TYPE));
		m_str[src_len] = (TYPE)0;
	}
	virtual bool Equal(const TYPE *src_str, int src_len) const {
		if (src_str == NULL || m_str == NULL || src_len != m_len) return false;
		return (memcmp(src_str, m_str, src_len * sizeof(TYPE)) == 0);
	}
	TYPE *Ptr() { return m_str; }

private:
	CCharTemp(const TYPE *src_str) {
		m_str = NULL;
		m_len = -1;
	}
#if 0
	operator TYPE *() const { return str; }
#endif
};

class CNchar;
class CWchar;

/// @brief 文字列ナロー
class CNchar : public CCharTemp<char>
{
private:
#if defined(USE_UTF8_ON_MBCS)
	CNchar *m_mbuf;
#endif
	CWchar *m_wbuf;
	void init();
	void term();

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
	virtual const wchar_t *GetW();
	virtual void GetW(wchar_t *dst_str, int dst_len) const;
	virtual bool MatchString(const char *value) const;
	virtual bool MatchSubString(const char *value) const;

	virtual const char *GetM();
	virtual void SetM(const char *src_str);
	virtual const wchar_t *GetWM();

	virtual void ToUpper();
	virtual void ToLower();
//protected:
//	virtual CNchar &operator=(const CNchar &src);
//	virtual CNchar &operator=(const char *src_str);
};

/// @brief 文字列ワイド
class CWchar : public CCharTemp<wchar_t>
{
private:
	CNchar *m_nbuf;
	void init();
	void term();

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
	virtual const char *GetN(int codepage = -1);
	virtual void GetN(char *dst_str, int dst_len, int codepage = -1) const;
	virtual bool MatchString(const wchar_t *value) const;
	virtual bool MatchSubString(const wchar_t *value) const;

	virtual const wchar_t *GetM() const;
	virtual void SetM(const wchar_t *src_str);
	virtual const wchar_t *GetWM() const;

	virtual void ToUpper();
	virtual void ToLower();
//protected:
//	virtual CWchar &operator=(const CWchar &src);
//	virtual CWchar &operator=(const wchar_t *src_str);
};

#if defined(_UNICODE)
#define CTchar CWchar
#else
#define CTchar CNchar
#endif

#endif /* CCHAR_H */
