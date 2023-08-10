/// @file paw_datas.cpp
///

#include "paw_datas.h"
#include <string.h>


namespace PARSEWAV 
{

//

CSampleData::CSampleData()
{
	Clear();
}
CSampleData::CSampleData(const CSampleData &src)
{
	Set(src);
}
#ifdef USE_SAMPLEDATA_USEC
CSampleData::CSampleData(uint8_t data, uint32_t usec, int spos, uint8_t attr, uint8_t flgs)
#else
CSampleData::CSampleData(uint8_t data, int spos, uint8_t attr, uint8_t flgs)
#endif
{
	Data(data);
#ifdef USE_SAMPLEDATA_USEC
	USec(usec);
#endif
	SPos(spos);
	Attr(attr);
	Flgs(flgs);
}
CSampleData &CSampleData::operator=(const CSampleData &src)
{
	Set(src);
	return *this;
}
void CSampleData::Clear()
{
	m_data = 0;
#ifdef USE_SAMPLEDATA_USEC
	m_usec = 0;
#endif
#ifdef USE_SAMPLEDATA_SPOS
	m_spos = 0;
#endif
	m_attr = 0;
	m_flgs = 0;
}
uint8_t CSampleData::Data() const
{
	return m_data;
}
#ifdef USE_SAMPLEDATA_USEC
uint32_t CSampleData::USec() const
{
	return m_usec;
}
#endif
int CSampleData::SPos() const
{
#ifdef USE_SAMPLEDATA_SPOS
	return m_spos;
#else
	return 0;
#endif
}
uint8_t CSampleData::Attr() const
{
	return m_attr;
}
uint8_t CSampleData::Flgs() const
{
	return m_flgs;
}
void CSampleData::Set(const CSampleData &src)
{
	Data(src.Data());
#ifdef USE_SAMPLEDATA_USEC
	USec(src.USec());
#endif
	SPos(src.SPos());
	Attr(src.Attr());
	Flgs(src.Flgs());
}
void CSampleData::Data(uint8_t val)
{
	m_data = val;
}
#ifdef USE_SAMPLEDATA_USEC
void CSampleData::USec(uint32_t val)
{
	m_usec = val;
}
#endif
void CSampleData::SPos(int val)
{
#ifdef USE_SAMPLEDATA_SPOS
	m_spos = val;
#endif
}
void CSampleData::Attr(uint8_t val)
{
	m_attr = val;
}
void CSampleData::Flgs(uint8_t val)
{
	m_flgs = val;
}

//

CSampleArray::CSampleArray(int init_size)
{
#ifdef USE_SAMPLEARRAY_POINTER
	m_datas = NULL;
	m_size = 0;
	Init();

	if (init_size > 0) {
		m_size = init_size;
		m_datas = new CSampleData[init_size];
	}
#else
	Init();
	m_size = DATA_ARRAY_SIZE;
#endif
}

CSampleArray::CSampleArray(const CSampleArray &src)
{
#ifdef USE_SAMPLEARRAY_POINTER
	m_datas = NULL;
#endif
	m_size = 0;
	Init();
}

CSampleArray::~CSampleArray()
{
#ifdef USE_SAMPLEARRAY_POINTER
	delete [] m_datas;
#endif
}

void CSampleArray::Init()
{
	Clear();
	m_rate = 0.0;
	m_total_w_pos = 0;
	m_total_r_pos = 0;
	m_last_data = false;
}

void CSampleArray::Clear()
{
//	for(int i=0; i<m_size; i++) {
//		m_datas[i].Clear();
//	}
	m_w_pos = 0;
	m_r_pos = 0;
	m_start_pos = 0;
}

const CSampleData &CSampleArray::At(int pos) const
{
	if (m_datas && 0 <= pos && pos < m_size) {
		return m_datas[pos];
	} else {
		return m_dummy;
	}
}
const CSampleData &CSampleArray::GetRead(int offset) const
{
	return At(m_r_pos + offset);
}
const CSampleData &CSampleArray::GetWrite(int offset) const
{
	return At(m_w_pos + offset);
}
/// スタート位置からの長さ
int CSampleArray::Length() const
{
	return m_w_pos - m_start_pos;
}
/// 残りフリーサイズ
int CSampleArray::FreeSize() const
{
	return m_size - m_w_pos;
}
/// 指定位置のデータポインタを返す
CSampleData *CSampleArray::GetPtr(int pos)
{
	if (m_datas && 0 <= pos && pos < m_size) {
		return &m_datas[pos];
	} else {
		return NULL;
	}
}
CSampleData *CSampleArray::GetStartPtr(int offset)
{
	return GetPtr(m_start_pos + offset);
}
CSampleData *CSampleArray::GetReadPtr(int offset)
{
	return GetPtr(m_r_pos + offset);
}
CSampleData *CSampleArray::GetWritePtr(int offset)
{
	return GetPtr(m_w_pos + offset);
}

/// 追加
void CSampleArray::Add(const CSampleData &val)
{
	if (m_datas && m_w_pos < m_size) {
		m_datas[m_w_pos] = val;
		m_w_pos++;
		m_total_w_pos++;
	}
}

/// 追加
#ifdef USE_SAMPLEDATA_USEC
void CSampleArray::Add(uint8_t data, uint32_t usec, int spos, uint8_t attr, uint8_t flgs)
{
	Add(CSampleData(data, usec, spos, attr, flgs));
}
#else
void CSampleArray::Add(uint8_t data, int spos, uint8_t attr, uint8_t flgs)
{
	Add(CSampleData(data, spos, attr, flgs));
}
#endif

/// 文字列を追加
/// @return 追加した文字数
#ifdef USE_SAMPLEDATA_USEC
int CSampleArray::AddString(const uint8_t *str, int len, uint32_t usec, int spos, uint8_t attr, uint8_t flgs)
{
	int n = 0;
	while(*str != 0 && n < len && m_w_pos < m_size) {
		Add(*str, usec, spos, attr, flgs);
		str++;
		n++;
	}
	return n;
}
#else
int CSampleArray::AddString(const uint8_t *str, int len, int spos, uint8_t attr, uint8_t flgs)
{
	int n = 0;
	while(*str != 0 && n < len && m_w_pos < m_size) {
		Add(*str, spos, attr, flgs);
		str++;
		n++;
	}
	return n;
}
#endif

/// 繰り返しセット
/// @return セットした文字数
#ifdef USE_SAMPLEDATA_USEC
int CSampleArray::Repeat(uint8_t data, int len, uint32_t usec, int spos, uint8_t attr, uint8_t flgs)
{
	int n = 0;
	while(n < len && m_w_pos < m_size) {
		Add(data, usec, spos, attr, flgs);
		n++;
	}
	return n;
}
#else
int CSampleArray::Repeat(uint8_t data, int len, int spos, uint8_t attr, uint8_t flgs)
{
	int n = 0;
	while(n < len && m_w_pos < m_size) {
		Add(data, spos, attr, flgs);
		n++;
	}
	return n;
}
#endif

/// バッファが書き込めないか
bool CSampleArray::IsFull(int offset) const
{
	return (m_w_pos + offset >= m_size);
}
/// 末尾まで読み込んでいるか
bool CSampleArray::IsTail(int offset) const
{
	return (m_r_pos + offset >= m_w_pos);
}
/// 残りのデータサイズ
int CSampleArray::RemainLength()
{
	return m_w_pos - m_r_pos;
}
/// 最後のデータか
bool CSampleArray::IsLastData() const
{
	return m_last_data;
}
/// 最後のデータ
void CSampleArray::LastData(bool val)
{
	m_last_data = val;
}
//
double CSampleArray::GetRate() const
{
	return m_rate;
}
int CSampleArray::GetSize() const
{
	return m_size;
}
int CSampleArray::GetStartPos() const
{
	return m_start_pos;
}
int CSampleArray::GetReadPos() const
{
	return m_r_pos;
}
int CSampleArray::GetWritePos() const
{
	return m_w_pos;
}
int CSampleArray::GetTotalReadPos() const
{
	return m_total_r_pos;
}
int CSampleArray::GetTotalWritePos() const
{
	return m_total_w_pos;
}
//
void CSampleArray::SetRate(double val)
{
	m_rate = val;
}
void CSampleArray::SetStartPos(int pos)
{
	m_start_pos = pos;
}
//
int CSampleArray::AddReadPos(int num)
{
	m_r_pos += num;
	m_total_r_pos += num;
	return m_r_pos;
}
int CSampleArray::AddWritePos(int num)
{
	m_w_pos += num;
	m_total_w_pos += num;
	return m_w_pos;
}
int CSampleArray::IncreaseReadPos()
{
	m_r_pos++;
	m_total_r_pos++;
	return m_r_pos;
}
int CSampleArray::IncreaseWritePos()
{
	m_w_pos++;
	m_total_w_pos++;
	return m_w_pos;
}
//
void CSampleArray::SkipReadPos()
{
	m_total_r_pos += m_w_pos - m_r_pos;
	m_r_pos = m_w_pos;
}
void CSampleArray::SkipToTail()
{
	m_total_r_pos += m_w_pos;
}
/// 未処理のデータをバッファの先頭にシフトする。
void CSampleArray::Shift()
{
	Shift(m_r_pos);
}
void CSampleArray::Shift(int offset)
{
	if (offset <= 0) return;
	for(int i=0; i<(m_w_pos - offset); i++) {
		m_datas[i] = m_datas[i + offset];
	}
	m_w_pos -= offset;
	if (m_w_pos < 0) m_w_pos = 0;
//	for(int i=m_w_pos; i<m_size; i++) {
//		m_datas[i].Clear();
//	}
	m_r_pos -= offset;
	if (m_r_pos < 0) m_r_pos = 0;

	m_start_pos -= offset;
	if (m_start_pos < 0) m_start_pos = 0;
}

/// 比較
/// @return -1: 引数の方が小さい 0:同じ 1:引数の方が大きい
int CSampleArray::Compare(int offset, const CSampleArray &dst, int dst_offset, int len)
{
	int cmp = 0;
	if (offset + len >= m_w_pos) {
		cmp = 1;
	} else if (dst_offset + len >= dst.m_w_pos) {
		cmp = -1;
	} else {
		for(int n = 0; n < len; n++) {
			cmp = ((int)dst.m_datas[dst_offset + n].Data() - (int)m_datas[offset + n].Data());
			if (cmp != 0) break;
		}
	}
	return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}

/// 比較
/// @return -1: 引数の方が小さい 0:同じ 1:引数の方が大きい
int CSampleArray::Compare(int offset, const uint8_t *ptn, int len)
{
	int cmp = 0;
	if (offset + len > m_w_pos) {
		cmp = 1;
	} else {
		for(int n = 0; n < len; n++) {
			cmp = ((int)ptn[n] - (int)m_datas[offset + n].Data());
			if (cmp != 0) break;
		}
	}
	return cmp > 0 ? 1 : cmp < 0 ? -1 : 0;
}
/// 比較
int CSampleArray::CompareRead(int offset, const uint8_t *ptn, int len)
{
	return Compare(m_r_pos + offset, ptn, len);
}

/// パターンに一致するデータ位置をさがす
/// @return 見つかった場合 (offset)を基準とした位置 <0:なし
int CSampleArray::Find(int offset, const uint8_t *ptn, int len)
{
	int idx = -1;
	int tail = m_w_pos - len;
	for(int pos = offset; pos <= tail; pos++) {
		if (Compare(pos, ptn, len) == 0) {
			idx = pos - offset;
			break;
		}
	}
	return idx;
}
/// パターンに一致するデータ位置をさがす
/// @return 見つかった場合 (ReadPos + offset)を基準とした位置 <0:なし
int CSampleArray::FindRead(int offset, const uint8_t *ptn, int len)
{
	return Find(m_r_pos + offset, ptn, len);
}

/// 値が指定した範囲で同じデータか
bool CSampleArray::SameAsRead(int offset, int len, const CSampleData &dat)
{
	bool same = true;
	for(int i=offset; i<(offset+len); i++) {
		if (m_datas[m_r_pos + i].Data() != dat.Data()) {
			same = false;
			break;
		}
	}
	return same;
}

/// spos位置をさがす
int CSampleArray::FindSPos(int offset, int spos)
{
	int pos = -1;
	for(int i=offset; i<m_w_pos; i++) {
		if (spos <= At(i).SPos()) {
			pos = i;
			break;
		}
	}
	return pos;
}
/// 末尾からspos位置をさがす
int CSampleArray::FindRevSPos(int offset, int spos)
{
	int pos = -1;
	for(int i=(m_w_pos-1); i>=offset; i--) {
		if (spos > At(i).SPos()) {
			pos = i + 1;
			if (pos >= m_w_pos) {
				pos = m_w_pos - 1;
			}
			break;
		}
	}
	if (pos < 0) {
		if (spos == At(offset).SPos()) {
			pos = offset;
		}
	}
	return pos;
}

//

CSampleString::CSampleString()
{
}
CSampleString::CSampleString(const CSampleString &src)
{
}
CSampleString::CSampleString(const CSampleArray &data, int offset, int len)
{
	m_str = new char[len + 1];
	memset(m_str, 0, len + 1);
	m_len = len;
	for(int i=0; i<len; i++) {
		m_str[i] = (char)data.At(i + offset).Data();
	}
}
CSampleString::~CSampleString()
{
	delete [] m_str;
}
const char *CSampleString::Get() const
{
	return m_str;
}
int CSampleString::Length() const
{
	return m_len;
}

//

CSampleBytes::CSampleBytes()
{
}
CSampleBytes::CSampleBytes(const CSampleBytes &src)
{
}
CSampleBytes::CSampleBytes(const CSampleArray &data, int offset, int len)
{
	m_str = new uint8_t[len + 1];
	memset(m_str, 0, len + 1);
	m_len = len;
	for(int i=0; i<len; i++) {
		m_str[i] = (uint8_t)data.At(i + offset).Data();
	}
}
CSampleBytes::~CSampleBytes()
{
	delete [] m_str;
}
const uint8_t *CSampleBytes::Get() const
{
	return m_str;
}
int CSampleBytes::Length() const
{
	return m_len;
}

//

CSampleList::CSampleList(const CSampleList &src)
	: std::vector<CSampleData>(src)
{
}
CSampleList::CSampleList()
	: std::vector<CSampleData>()
{
}
CSampleList::CSampleList(size_t size)
	: std::vector<CSampleData>(size)
{
}


//

WaveData::WaveData(const WaveData &src)
	: CSampleArray(src)
{
}

WaveData::WaveData(int init_size)
	: CSampleArray(init_size)
{
}

//

CarrierData::CarrierData(const CarrierData &src)
	: CSampleArray(src)
{
}

CarrierData::CarrierData(int init_size)
	: CSampleArray(init_size)
{
}

}; /* namespace PARSEWAV */
