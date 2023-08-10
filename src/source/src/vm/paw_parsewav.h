/// @file paw_parsewav.h
///
/// @author Sasaji
/// @date   2019.08.01
///


#ifndef _PARSEWAV_PARSEWAV_H_
#define _PARSEWAV_PARSEWAV_H_

#include "../common.h"
#include <stdio.h>
#include "paw_parse.h"
#include "paw_datas.h"
#include "paw_format.h"
#include "paw_file.h"
#include "paw_util.h"


namespace PARSEWAV 
{

/// wavファイル解析のパラメータ
class lamda_t
{
public:
	double samples[3];	// 1波のサンプル数 0:1200Hz 1:2400Hz 2:4800Hz
	double us_delta;	// 1サンプルの長さ(us)
	double us_range[3];	// 0:1200Hz 1:2400Hz 2:4800Hz
	double us[3];		// 0:1200Hz 1:2400Hz 2:4800Hz
	double us_min[3];
	double us_max[3];
	double us_avg[3];
	double us_mid[2];	// 0:1800Hz 1:3600Hz
	double us_mid_avg[2];
	double us_limit[2];	// 0:9600Hz 1:19200Hz
public:
	lamda_t();
	void clear();
};

/// 解析用のワーク
class parse_carrier_t
{
public:
	double x0_prev;
	int    wav_prev;
	int    sample_cnt;
	int    odd;
	int    carr_prev;
public:
	parse_carrier_t();
	void clear();
};

class PrevCross
{
public:
	int	spos;
public:
	PrevCross();
	void Clear();
	int SPos() const { return spos; }
	void SPos(int val) { spos = val; }
};

/// WAVデータ解析用クラス
class WaveParser : public ParserBase
{
private:
	/// 解析用のワーク
	parse_carrier_t st_pa_carr;
	/// １つ前の交点
	PrevCross prev_cross;

	/// wavファイル解析のパラメータ
	lamda_t st_lamda;

	/// Wavファイルのフォーマット情報
	WaveFormat *inwav;

public:
	WaveParser();

	void Clear();
	void ClearResult();
	void Init(WaveFormat &inwav_);

	int CheckWaveFormat(InputFile &file, wav_header_t *head, wav_fmt_chank_t *fmt, wav_data_chank_t *data, Util& conv);

	int GetWaveSample(int blk_size, bool reverse);
	int GetWaveSample(WaveData *w_data, int size, bool reverse);

	int SkipWaveSample(int dir);

	int DecodeToCarrier(int fsk_spd, WaveData *w_data, CarrierData *c_data);

	int EncodeToWave(CarrierData *c_data, uint8_t *w_data, int len);

	const lamda_t &GetLamda() const { return st_lamda; }

};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_PARSEWAV_H_ */
