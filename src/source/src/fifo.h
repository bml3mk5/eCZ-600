/** @file fifo.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.01-

	@brief [ fifo buffer ]

	@note Original author is Takeda.Toshiya
*/

#ifndef FIFO_H
#define FIFO_H

#include "common.h"
#include <stdlib.h>

/**
	@brief fifo buffer template
*/
template <class TYPE>
class FIFOBase
{
protected:
	TYPE* buf;
	int cnt, rpt, wpt, siz;
public:
	FIFOBase();
	FIFOBase(int s);
	virtual ~FIFOBase();
	/// @brief release data buffer from memory
	void release();
	/// @brief allocate memory for specified size
	bool allocate(int s);
	/// @brief clear read and write position
	void clear();
	/// @brief write a data and increase written position
	void write(TYPE val);
	/// @brief write datas
	int write(const TYPE *buffer, int size);
	/// @brief read a data and inclease read position
	TYPE read();
	/// @brief read datas and store to buffer
	int read(TYPE *buffer, int size);
	/// @brief read a data at specified position
	TYPE read_not_remove(int pt);
	/// @brief rollback a data at last read
	void rollback();
	/// @brief peek a data at specified position
	TYPE peek(int pt) const;
	/// @brief return data buffer starting on specified position
	TYPE* data(int pt) const;
	/// @brief return count of written data
	int  count() const;
	/// @brief return size of remaining buffer
	int  remain() const;
	/// @brief return size of buffer
	int  size() const;
	/// @brief return writing position
	int  write_pos() const;
	/// @brief return reading position
	int  read_pos() const;
	/// @brief is buffer full ?
	bool full() const;
	/// @brief is buffer empty ?
	bool empty() const;
};

template <class TYPE>
FIFOBase<TYPE>::FIFOBase()
{
	cnt = rpt = wpt = siz = 0;
	buf = NULL;
}

template <class TYPE>
FIFOBase<TYPE>::FIFOBase(int s)
{
	cnt = rpt = wpt = siz = 0;
	buf = NULL;
	allocate(s);
}

template <class TYPE>
FIFOBase<TYPE>::~FIFOBase()
{
	release();
}

/// release data buffer from memory
template <class TYPE>
void FIFOBase<TYPE>::release()
{
	if (buf) {
		free(buf);
		buf = NULL;
	}
	cnt = rpt = wpt = siz = 0;
}

/// allocate memory for specified size
/// @param[in] s : size
template <class TYPE>
bool FIFOBase<TYPE>::allocate(int s)
{
	if (siz < s) {
		if (!buf) buf = (TYPE *)malloc(sizeof(TYPE) * s);
		else buf = (TYPE *)realloc(buf, sizeof(TYPE) * s);
		if (!buf) siz = 0;
		else siz = s;
	}
	return (buf != NULL);
}

/// clear read and write position
/// no clear data buffer
template <class TYPE>
void FIFOBase<TYPE>::clear()
{
	cnt = rpt = wpt = 0;
}

/// write a data and increase written position
/// @param[in] val : a data
template <class TYPE>
void FIFOBase<TYPE>::write(TYPE val)
{
	if(cnt < siz) {
		buf[wpt++] = val;
		if(wpt >= siz) {
			wpt = 0;
		}
		cnt++;
	}
}

/// write datas
/// @param[in] buffer : store data
/// @param[in] size   : data size (items)
/// @return stored length
template <class TYPE>
int FIFOBase<TYPE>::write(const TYPE *buffer, int size)
{
	int stored = 0;
	TYPE *dst;

	int len = siz - wpt;
	if (cnt > 0 && wpt <= rpt) len = rpt - wpt;
	if (size < len) len = size;
	while(size > 0 && len > 0) {
		dst = &buf[wpt];
		memcpy(dst, buffer, len * sizeof(TYPE));
		size -= len;
		cnt += len;
		wpt += len;
		if (wpt >= siz) {
			wpt -= siz;
		}
		stored += len;
		buffer += len;
		len = rpt - wpt;
		if (size < len) len = size;
	}
	return stored;
}

/// read a data and increase read position
/// @return a data
template <class TYPE>
TYPE FIFOBase<TYPE>::read()
{
	int val = 0;
	if(cnt) {
		val = buf[rpt++];
		if(rpt >= siz) {
			rpt = 0;
		}
		cnt--;
	}
	return val;
}

/// read datas and store to buffer
/// @param[out] buffer : buffer to store data
/// @param[in]  size   : buffer size (items)
/// @return stored length
template <class TYPE>
int FIFOBase<TYPE>::read(TYPE *buffer, int size)
{
	int stored = 0;
	TYPE *src;

	int len = cnt < size ? cnt : size;
	if (rpt + len >= siz) len = siz - rpt;
	while(size > 0 && cnt > 0) {
		src = &buf[rpt];
		memcpy(buffer, src, len * sizeof(TYPE));
		size -= len;
		cnt -= len;
		rpt += len;
		if (rpt >= siz) {
			rpt -= siz;
		}
		stored += len;
		buffer += len;
		len = cnt < size ? cnt : size;
	}
	return stored;
}

/// read a data at specified position
/// @param[in] pt : position of buffer (relative from letest read position)
/// @return a data
template <class TYPE>
TYPE FIFOBase<TYPE>::read_not_remove(int pt)
{
	if(pt >= 0 && pt < cnt) {
		pt += rpt;
		if(pt >= siz) {
			pt -= siz;
		}
		return buf[pt];
	}
	return (TYPE)0;
}

/// rollback a data at last read
template <class TYPE>
void FIFOBase<TYPE>::rollback()
{
	if (rpt > 0) {
		rpt--;
	} else {
		rpt = siz - 1;
	}
	cnt++;
}

/// peek a data at specified position
/// @param[in] pt : position of buffer
/// @return a data
template <class TYPE>
TYPE FIFOBase<TYPE>::peek(int pt) const
{
	return buf[pt];
}

/// return data buffer starting on specified position
/// @param[in] pt : start position of buffer
/// @return pointer of data buffer
template <class TYPE>
TYPE* FIFOBase<TYPE>::data(int pt) const
{
	return &buf[pt];
}

/// return count of written data
template <class TYPE>
int FIFOBase<TYPE>::count() const
{
	return cnt;
}

/// return size of remaining buffer
template <class TYPE>
int FIFOBase<TYPE>::remain() const
{
	return siz - cnt;
}

/// return size of buffer
template <class TYPE>
int FIFOBase<TYPE>::size() const
{
	return siz;
}

/// return writing position
template <class TYPE>
int FIFOBase<TYPE>::write_pos() const
{
	return wpt;
}

/// return reading position
template <class TYPE>
int FIFOBase<TYPE>::read_pos() const
{
	return rpt;
}

/// is buffer full ?
template <class TYPE>
bool FIFOBase<TYPE>::full() const
{
	return (cnt >= siz);
}

/// is buffer empty ?
template <class TYPE>
bool FIFOBase<TYPE>::empty() const
{
	return (cnt == 0);
}

/**
	@brief fifo byte buffer
*/
class FIFOBYTE : public FIFOBase<uint8_t>
{
public:
	FIFOBYTE();
	FIFOBYTE(int s);
};

/**
	@brief fifo char buffer
*/
class FIFOCHAR : public FIFOBase<char>
{
public:
	FIFOCHAR();
	FIFOCHAR(int s);
};

/**
	@brief fifo integer buffer
*/
class FIFOINT : public FIFOBase<int>
{
public:
	FIFOINT();
	FIFOINT(int s);
};

#endif /* FIFO_H */

