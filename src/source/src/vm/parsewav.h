/// @file parsewav.h
///
/// @brief waveデータ <-> 搬送波データ <-> シリアルデータ　変換クラス
///
/// @author Sasaji
/// @date   2011.7.1
///

#ifndef _PARSEWAV_H_
#define _PARSEWAV_H_

#include "../common.h"
#include "../fileio.h"
#include "paw_param.h"
#include "paw_datas.h"
#include "paw_format.h"
#include "paw_parsewav.h"
#include "paw_parsecar.h"
#include "paw_util.h"
#include "paw_dft.h"

/// @namespace PARSEWAV
/// @brief データレコーダの変調波(FSK)を解析してシリアルデータに変換するモジュール群
namespace PARSEWAV
{

/// @brief データレコーダの変調波(FSK)を解析してシリアルデータに変換するクラス
///
/// waveファイル形式で保存されたデータレコーダの変調波(FSK)1200Hzと2400Hzの波を
/// 解析して搬送波データとさらにシリアルデータを取り出す。
/// 逆にシリアルデータから変調波のwaveファイルを作成する。
class ParseWav
{
private:
	InputFile infile;
	Parameter param;
	Util conv;
	Util snd;
	Dft  dft;

	int infile_type;

	enum enum_phase {
		PHASE_NONE = -1,
		PHASE1_GET_WAV_SAMPLE = 0,
		PHASE1_CORRECT_WAVE = 1,
		PHASE1_DECODE_TO_CARRIER = 2,
		PHASE1_WAV_FAST_FORWARD = 4,
		PHASE1_WAV_REWIND = 8,
		PHASE1_GET_L3C_SAMPLE = 11,
		PHASE1_L3C_FAST_FORWARD = 14,
		PHASE1_L3C_REWIND = 18,
		PHASE1_SHIFT_BUFFER = 21,
	};

	enum_phase phase1;
	enum_phase start_phase;

	WaveParser wave_parser;
	CarrierParser carrier_parser;

	WaveData *wave_data;
	WaveData *wave_correct_data;
	CarrierData *carrier_data;

	int err_num;

	WaveFormat inwav;
	WaveFormat outwav;
	WaveFormat sndwav;

	int infile_samples_onebit;
	int infile_samples_conebit;
	int infile_samples_const;
	int inwav_dirpos;

	bool  decode_phase1(int fsk_spd, enum_phase start_phase, char *w_onedata, int *w_onelen);
	int   decode_phase2(uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen);

	int   encode_phase2(uint8_t data, CarrierData *c_data);
	int   encode_phase1(uint8_t data, uint8_t *buffer, int buffer_len, int *c_len);

public:
	// prototype define
	ParseWav();
	~ParseWav();

	int   OpenFile(FILEIO &file, int file_type, int def_rate_num = 0, int def_bits_num = 0, int def_ch = 1);
	int   CheckFileFormat(FILEIO &file, int file_type, int def_rate_num, int def_bits_num, int def_ch);
	void  CloseFile();

	int   InitData(FILEIO &file, int baud_rate, int w_onelen);
	void  ClearData();
	int   GetData(int dir, uint8_t *s_data, int &s_datalen, char *w_onedata, int *w_onelen, char *c_onedata, int *c_onelen);
	int   SetData(uint8_t data, int &data_len, uint8_t *buffer, int buffer_len, int *c_len);
	int   InitFileHeader(uint8_t *buffer);
	void  SetFileHeader(FILEIO *fp);

	_TCHAR* Errmsg();

	const Parameter &GetParam() const { return param; }
	Parameter &GetParam() { return param; }

	void SetSampleRatePos(int value) { param.SetSampleRatePos(value); }
	void SetSampleBitsPos(int value) { param.SetSampleBitsPos(value); }
	void SetBaudAndFrequency(int baud_rate, int spd);
	void SetParseParamerers(bool half, bool reverse, int corr_type, int corr_amp0, int corr_amp1);
	void SetStartPhase(int type);

	int GetSampleRatePos(void) const { return param.GetSampleRatePos(); }
	int GetSampleBitsPos(void) const { return param.GetSampleBitsPos(); }
	int GetSamplePos(void) const { return infile.SamplePos(); }

	void SetFormatForSound(int rate, int bits, int ch);
};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_H_ */
