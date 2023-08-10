/// @file paw_datas.h
///

#ifndef _PARSEWAV_DATAS_H_
#define _PARSEWAV_DATAS_H_

#include "../common.h"
#include <vector>


namespace PARSEWAV
{

#ifndef DATA_ARRAY_SIZE
#define DATA_ARRAY_SIZE	256
#endif

//#define USE_SAMPLEDATA_USEC
//#define USE_SAMPLEDATA_SPOS
#define USE_SAMPLEARRAY_POINTER

/// サンプルデータ保持用クラス
class CSampleData
{
protected:
#ifdef USE_SAMPLEDATA_USEC
	uint32_t m_usec;
#endif
#ifdef USE_SAMPLEDATA_SPOS
	int      m_spos;	///< サンプル位置
#endif
	uint8_t  m_data;	///< サンプルデータ
	uint8_t  m_attr;	///< 属性(ボーレートなど)
	uint8_t  m_flgs;	///< フラグ(エラー判定に使用)
public:
	CSampleData();
	CSampleData(const CSampleData &src);
#ifdef USE_SAMPLEDATA_USEC
	CSampleData(uint8_t data, uint32_t usec, int spos, uint8_t attr, uint8_t flgs);
#else
	CSampleData(uint8_t data, int spos, uint8_t attr, uint8_t flgs);
#endif
	CSampleData &operator=(const CSampleData &src);
	void Clear();
	uint8_t  Data() const;
#ifdef USE_SAMPLEDATA_USEC
	uint32_t USec() const;
#endif
	int      SPos() const;
	uint8_t  Attr() const;
	uint8_t  Flgs() const;
	void Set(const CSampleData &src);
	void Data(uint8_t val);
#ifdef USE_SAMPLEDATA_USEC
	void USec(uint32_t val);
#endif
	void SPos(int val);
	void Attr(uint8_t val);
	void Flgs(uint8_t val);
};

/// サンプルデータ配列クラス
class CSampleArray
{
protected:
	int     m_size;			///< 配列サイズ(要素数)

#ifdef USE_SAMPLEARRAY_POINTER
	CSampleData *m_datas;
#else
	CSampleData  m_datas[DATA_ARRAY_SIZE];
#endif
	CSampleData  m_dummy;

	double  m_rate;

	int     m_w_pos;		///< 書き込んだ位置
	int     m_r_pos;		///< 読み込んだ位置
	int     m_total_w_pos;	///< 書き込んだ位置の合計
	int     m_total_r_pos;	///< 読み込んだ位置の合計
	int     m_start_pos;	///< 書き込み開始位置（ファイル出力時に使用）
	bool    m_last_data;	///< 最後のデータ

	CSampleArray(const CSampleArray &src);

public:
	CSampleArray(int init_size = DATA_ARRAY_SIZE);
	~CSampleArray();

	void Init();
	void Clear();
	const CSampleData &At(int pos = 0) const;
	const CSampleData &GetRead(int offset = 0) const;
	const CSampleData &GetWrite(int offset = 0) const;

	/// スタート位置からの長さ
	int Length() const;
	/// 残りフリーサイズ
	int FreeSize() const;

	CSampleData *GetPtr(int pos = 0);
	CSampleData *GetStartPtr(int offset = 0);
	CSampleData *GetReadPtr(int offset = 0);
	CSampleData *GetWritePtr(int offset = 0);

	/// 追加
	void Add(const CSampleData &val);
#ifdef USE_SAMPLEDATA_USEC
	void Add(uint8_t data, uint32_t usec, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#else
	void Add(uint8_t data, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#endif
	/// 文字列を追加
#ifdef USE_SAMPLEDATA_USEC
	int AddString(const uint8_t *str, int len, uint32_t usec, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#else
	int AddString(const uint8_t *str, int len, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#endif
	/// 繰り返しセット
#ifdef USE_SAMPLEDATA_USEC
	int Repeat(uint8_t data, int len, uint32_t usec, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#else
	int Repeat(uint8_t data, int len, int spos, uint8_t attr = 0, uint8_t flgs = 0);
#endif
	/// バッファが書き込めないか
	bool IsFull(int offset = 0) const;
	/// 末尾まで読み込んでいるか
	bool IsTail(int offset = 0) const;
	/// 残りのデータサイズ
	int RemainLength();

	/// 最後のデータか
	bool IsLastData() const;
	/// 最後のデータ
	void LastData(bool val);

	//
	double GetRate() const;
	int GetSize() const;
	int GetStartPos() const;
	int GetReadPos() const;
	int GetWritePos() const;
	int GetTotalReadPos() const;
	int GetTotalWritePos() const;
	//
	void SetRate(double val);
	void SetStartPos(int pos);
	//
	int AddReadPos(int num);
	int AddWritePos(int num);
	int IncreaseReadPos();
	int IncreaseWritePos();
	//
	void SkipReadPos();
	void SkipToTail();
	/// 未処理のデータをバッファの先頭にシフトする。
	void Shift();
	void Shift(int offset);

	/// 比較
	int Compare(int offset, const CSampleArray &dst, int dst_offset, int len);
	/// 比較
	int Compare(int offset, const uint8_t *dat, int len);
	/// 比較
	int CompareRead(int offset, const uint8_t *dat, int len);
	/// パターンに一致するデータ位置をさがす
	int Find(int offset, const uint8_t *ptn, int len);
	/// パターンに一致するデータ位置をさがす
	int FindRead(int offset, const uint8_t *ptn, int len);
	/// 値が指定した範囲で同じデータか
	bool SameAsRead(int offset, int len, const CSampleData &dat);

	/// spos位置をさがす
	int FindSPos(int offset, int spos);
	/// 末尾からspos位置をさがす
	int FindRevSPos(int offset, int spos);

};

/// サンプルデータ配列を文字列にする
class CSampleString
{
private:
	char *m_str;
	int   m_len;

	CSampleString();
	CSampleString(const CSampleString &src);

public:
	CSampleString(const CSampleArray &data, int offset, int len);
	~CSampleString();
	const char *Get() const;
	int Length() const;
};

/// サンプルデータ配列をバイト列にする
class CSampleBytes
{
private:
	uint8_t *m_str;
	int      m_len;

	CSampleBytes();
	CSampleBytes(const CSampleBytes &src);

public:
	CSampleBytes(const CSampleArray &data, int offset, int len);
	~CSampleBytes();
	const uint8_t *Get() const;
	int Length() const;
};

/// サンプルデータリスト
class CSampleList : public std::vector<CSampleData>
{
private:
	CSampleList(const CSampleList &src);

public:
	CSampleList();
	CSampleList(size_t size);
};

//

/// 波形解析用バッファ
class WaveData : public CSampleArray
{
private:
	WaveData(const WaveData &src);

public:
	WaveData(int init_size = DATA_ARRAY_SIZE);
};

//

/// 搬送波バッファ
class CarrierData : public CSampleArray
{
private:
	CarrierData(const CarrierData &src);

public:
	CarrierData(int init_size = DATA_ARRAY_SIZE);
};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_DATAS_H_ */
