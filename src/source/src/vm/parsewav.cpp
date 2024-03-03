/// @file parsewav.cpp
///
/// @brief waveデータ <-> 搬送波データ <-> シリアルデータ　変換クラス
///
/// @author Sasaji
/// @date   2011.7.1
///
#define _USE_MATH_DEFINES 1
#include <math.h>
#include "parsewav.h"
#include "../utility.h"

namespace PARSEWAV
{

/// @brief コンストラクタ
///
ParseWav::ParseWav()
{
	//
	wave_data = new WaveData(256);
	wave_correct_data = new WaveData(256);
	carrier_data = new CarrierData(64);

	wave_parser.SetParameter(param);
	carrier_parser.SetParameter(param);

	infile_type = 0;

	err_num = 0;

	// wavファイルヘッダ(出力用)
	outwav.Init(48000, 8, 1);

#ifdef _DEBUG
	if (param.GetDebugMode()) {
		gLogFile.Open(_T("parsewav.log"));
	}
#endif
}

/// @brief デストラクタ
///
ParseWav::~ParseWav()
{
	gLogFile.Close();

	delete carrier_data;
	delete wave_correct_data;
	delete wave_data;
}

/// @brief エラーメッセージ一覧
const _TCHAR *ParseWav::c_errmsgs[] = {
	_T(""),
	_T("File not found."),
	_T("This is not the wav file which is formatted linear PCM."),
	_T("This is not the frequency between 11025 and 48000Hz."),
	_T("Cannot write to file."),
	NULL
};

/// @brief エラーメッセージを出力
///
/// @return エラーメッセージへのポインタ
///
///
///
_TCHAR* ParseWav::Errmsg()
{
	static _TCHAR str[1024];

	if (err_num >= 0 && err_num <= 4) {
		UTILITY::tcscpy(str, sizeof(str)/sizeof(_TCHAR), c_errmsgs[err_num]);
	} else {
		UTILITY::stprintf(str, sizeof(str)/sizeof(_TCHAR), _T("Unknown error: %d"), err_num);
	}
	return str;
}

/// @brief ファイルを開く
///
/// @param[in] file         ファイル
/// @param[in] file_type    ファイル種類 0:waveファイル 1:l3cファイル
/// @param[in] def_rate_num 新規時のサンプルレート番号
/// @param[in] def_bits_num 新規時のサンプルビット番号(0 or 1)
/// @param[in] def_ch       新規時のチャンネル
/// @return    0:正常 1:ファイルなし 2..4:エラーあり
int ParseWav::OpenFile(FILEIO &file, int file_type, int def_rate_num, int def_bits_num, int def_ch)
{
	infile.Attach(file);

	return CheckFileFormat(file, file_type, def_rate_num, def_bits_num, def_ch);
}

/// @brief ファイルを閉じる
void ParseWav::CloseFile()
{
	infile.Detach();
}

/// @brief 入力ファイルのフォーマットをチェックする
///
/// @param[in] file         入力ファイル
/// @param[in] file_type    ファイル種類 0:waveファイル 1:l3cファイル
/// @param[in] def_rate_num 新規時のサンプルレート番号
/// @param[in] def_bits_num 新規時のサンプルビット番号(0 or 1)
/// @param[in] def_ch       新規時のチャンネル
/// @return    0:正常 1:ファイルなし 2..4:エラーあり
int ParseWav::CheckFileFormat(FILEIO &file, int file_type, int def_rate_num, int def_bits_num, int def_ch)
{
	int rc = 0;

	infile_type = file_type;

	// check wave header
	if (file_type == 0) {
		rc = wave_parser.CheckWaveFormat(infile, inwav.GetHead(), inwav.GetFmtChank(), inwav.GetDataChank(), conv);
		if (rc >= 0) {
			err_num = 0;
		} else if (rc == -1) {
			// new file
			err_num = -rc;
			inwav.Init(Parameter::GetSampleRate(def_rate_num), def_bits_num != 0 ? 16 : 8, def_ch);
			infile.SampleNum(0);
		} else {
			err_num = -rc;
		}
	} else if (file_type == 1) {
		rc = carrier_parser.CheckL3CFormat(infile);
		err_num = 0;
	}

	return err_num;
}

/// @brief 初期化
///
/// @param[in] file      入力ファイル
/// @param[in] baud_rate ボーレート
/// @param[in] w_onelen  サウンド出力用バッファサイズ
/// @return 0
int ParseWav::InitData(FILEIO &file, int baud_rate, int w_onelen)
{
	wave_data->Init();
	wave_correct_data->Init();
	carrier_data->Init();

	// waveファイルのヘッダを出力
	outwav.Init(param.GetSampleRate(), param.GetSampleBitsPos() != 0 ? 16 : 8, 1);

	conv.InitConvSampleData(DATA_ARRAY_SIZE + 4);
	snd.InitConvSampleData(w_onelen);

	SetStartPhase(0);

	wave_parser.Init(inwav);
	carrier_parser.Init();

	if (infile_type == 0) {
		infile_samples_onebit = inwav.GetSampleRate() / baud_rate;
		infile_samples_const  = inwav.GetSampleRate() / 600;
		dft.Init(wave_parser.GetLamda().samples[param.GetFskSpeed()], param.GetCorrectType(), param.GetCorrectAmp(0), param.GetCorrectAmp(1));
	} else {
		infile_samples_onebit = (4800 << param.GetFskSpeed()) / baud_rate;
		infile_samples_const  = 8;
	}
	infile_samples_conebit = (4800 << param.GetFskSpeed()) / baud_rate;

	return 0;
}

///
///
///
///
///
void ParseWav::ClearData()
{
	wave_data->Init();
	wave_correct_data->Init();
	carrier_data->Init();

	wave_parser.Init(inwav);
	carrier_parser.Init();
}

///
///
///
///
///
void ParseWav::SetBaudAndFrequency(int baud_rate, int spd)
{
	param.SetBaud(spd & 3);
	param.SetFskSpeed((spd & 4) >> 2);

	infile_samples_onebit = inwav.GetSampleRate() / baud_rate;

	wave_parser.Init(inwav);

	if (infile_type == 0) {
		dft.Init(wave_parser.GetLamda().samples[param.GetFskSpeed()], param.GetCorrectType(), param.GetCorrectAmp(0), param.GetCorrectAmp(1));
	}
}

///
///
///
///
///
void ParseWav::SetParseParamerers(bool half, bool reverse, int corr_type, int corr_amp0, int corr_amp1)
{
	param.SetHalfWave(half);
	param.SetReverseWave(reverse);
	param.SetCorrectType(corr_type);
	param.SetCorrectAmp(0, corr_amp0);
	param.SetCorrectAmp(1, corr_amp1);

	if (infile_type == 0) {
		dft.Init(wave_parser.GetLamda().samples[param.GetFskSpeed()], param.GetCorrectType(), param.GetCorrectAmp(0), param.GetCorrectAmp(1));
	}
}

///
///
/// @param[in] dir 0:再生 1:早送り -1:巻き戻し
///
///
void ParseWav::SetStartPhase(int dir)
{
	switch(dir) {
	case 1:
		if (infile_type == 1) {
			// .l3c
			phase1 = PHASE1_L3C_FAST_FORWARD;
			start_phase = PHASE1_L3C_FAST_FORWARD;
		} else {
			// .wav
			phase1 = PHASE1_WAV_FAST_FORWARD;
			start_phase = PHASE1_WAV_FAST_FORWARD;
		}
		break;
	case -1:
		if (infile_type == 1) {
			// .l3c
			phase1 = PHASE1_L3C_REWIND;
			start_phase = PHASE1_L3C_REWIND;
		} else {
			// .wav
			phase1 = PHASE1_WAV_REWIND;
			start_phase = PHASE1_WAV_REWIND;
		}
		break;
	default:
		if (infile_type == 1) {
			// .l3c
			phase1 = PHASE1_GET_L3C_SAMPLE;
			start_phase = PHASE1_GET_L3C_SAMPLE;
		} else {
			// .wav
			phase1 = PHASE1_GET_WAV_SAMPLE;
			start_phase = PHASE1_GET_WAV_SAMPLE;
		}
	}
}

/// @brief waveファイルのヘッダをバッファに出力する
///
/// @param[out] buffer バッファ
/// @return ヘッダ長さ
///
int ParseWav::InitFileHeader(uint8_t *buffer)
{
	return outwav.Out(buffer);
}

/// @brief waveファイルにファイルサイズを書き込む
///
/// @param[in] fp 入力ファイル
///
///
void ParseWav::SetFileHeader(FILEIO *fp)
{
	WaveFormat::OutSize(fp);
}

/// @brief waveサンプルデータから搬送波(1200/2400Hz)を解析し
/// @brief ビットデータに変換する。(FSK)
///
/// @param[in]  fsk_spd 1:倍速FSK
/// @param[in]  start_phase 開始フェーズ
/// @param[out] w_onedata waveバッファ
/// @param[in,out] w_onelen waveバッファ長さ/書き込んだ長さ
/// @return     データ終わり true
bool ParseWav::decode_phase1(int fsk_spd, enum_phase start_phase, char *w_onedata, int *w_onelen)
{
	int rc = 0;
	bool last = false;
	bool supp = false;
	int w_onepos = 0;
	int w_sum, wc_sum;
	WaveData *wn_data;

	if (infile_type == 0 && param.GetCorrectType() > 0) {
		wn_data = wave_correct_data;
	} else {
		wn_data = wave_data;
	}

	while(phase1 > PHASE_NONE && last == false) {
		switch(phase1) {
			case PHASE1_GET_WAV_SAMPLE:
				// WAVファイル読み込み
				inwav_dirpos = wave_parser.GetWaveSample(wave_data, supp ? (infile_samples_onebit / infile_samples_conebit) : infile_samples_onebit, param.GetReverseWave());
				phase1 = PHASE1_CORRECT_WAVE;
				break;
			case PHASE1_CORRECT_WAVE:
				// WAVサンプルデータを補正する
				if (infile_type == 0 && param.GetCorrectType() > 0) {
					dft.Calcrate(wave_data, wave_correct_data);
				}
				// wavファイル変換
				if (w_onedata) {
					w_onepos = (int)snd.OutConvSampleData(*wn_data, inwav.GetSampleRate()
						,(uint8_t *)w_onedata, sndwav.GetSampleRate(), sndwav.GetSampleBits(), *w_onelen, w_onepos);
				}
				// データ最後まで読んだ場合
				if (param.GetCorrectType() > 0) {
					if (wave_data->IsLastData() && wave_data->IsTail((int)wave_parser.GetLamda().samples[1] + 2)) {
						wave_correct_data->LastData(true);
					}
				}
				phase1 = PHASE1_DECODE_TO_CARRIER;
				break;
			case PHASE1_DECODE_TO_CARRIER:
				// L3Cへ変換
				rc = wave_parser.DecodeToCarrier(fsk_spd, wn_data, carrier_data);
				if (rc & 0x10) {
					// cバッファがいっぱいになったら吐き出す
					phase1 = PHASE1_SHIFT_BUFFER;
				} else if ((rc & 0x03) == 0x03) {
					// 最後のデータ
					carrier_data->LastData(true);
					phase1 = PHASE_NONE;
				} else if ((rc & 0x03) == 0x01) {
					// wバッファがいっぱい
					if (!supp && carrier_data->RemainLength() < infile_samples_conebit) {
						// cバッファのデータ不足
						supp = true;
						wn_data->SetStartPos(wn_data->GetWritePos());
						phase1 = PHASE1_GET_WAV_SAMPLE;
					} else {
						phase1 = PHASE1_SHIFT_BUFFER;
					}
				}
				break;
			case PHASE1_WAV_FAST_FORWARD:
				// F.F.
				inwav_dirpos = wave_parser.SkipWaveSample(infile_samples_const);
				if (w_onedata) {
					w_onepos = 0;
				}

				phase1 = start_phase;
				last = true;
				break;
			case PHASE1_WAV_REWIND:
				// Rewind
				inwav_dirpos = - wave_parser.SkipWaveSample(-infile_samples_const);
				if (w_onedata) {
					w_onepos = 0;
				}

				phase1 = start_phase;
				last = true;
				break;
			case PHASE1_GET_L3C_SAMPLE:
				// L3Cファイル読み込み
				inwav_dirpos = carrier_parser.GetL3CSample(carrier_data, infile_samples_onebit);

				phase1 = start_phase;
				last = true;
				break;
			case PHASE1_L3C_FAST_FORWARD:
				// F.F.
				inwav_dirpos = carrier_parser.SkipL3CSample(infile_samples_const);

				phase1 = start_phase;
				last = true;
				break;
			case PHASE1_L3C_REWIND:
				// Rewind
				inwav_dirpos = - carrier_parser.SkipL3CSample(-infile_samples_const);

				phase1 = start_phase;
				last = true;
				break;
			case PHASE1_SHIFT_BUFFER:
				// 残りをバッファの最初にコピー→次のターンで解析をするため
				w_sum = wave_data->GetReadPos() - (int)wave_parser.GetLamda().samples[1] - 2;
				if (param.GetCorrectType() > 0) {
					wc_sum = wave_correct_data->GetReadPos() - 2;
					w_sum = (w_sum > wc_sum ? wc_sum : w_sum);
				}
				if (w_sum > 0) {
					wave_data->Shift(w_sum);
					wave_correct_data->Shift(w_sum);
				}

				phase1 = start_phase;
				last = true;
				break;
			default:
				break;
		}
	}
	*w_onelen = w_onepos;
	return carrier_data->IsLastData();
}

/// @brief 搬送波(1200/2400Hz)データからシリアルデータに変換
/// ここでボーレートによってシリアルデータが変わってくる
/// @param[out] s_data    シリアルデータ
/// @param[out] s_datalen シリアルデータ長さ
/// @param[out] c_onedata NULL or シリアルデータ1ビット分の搬送波
/// @param[out] c_onelen  NULL or シリアルデータ1ビット分の搬送波の長さ
/// @return フェーズ番号
int ParseWav::decode_phase2(uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen)
{
	int rc = 0;
	int step = 0;
	int baud = param.GetBaud();

	carrier_parser.Decode(carrier_data, baud, step, s_data, s_datalen, c_onedata, c_onelen);
	if (carrier_data->IsLastData() && carrier_data->IsTail()) {
		rc = -1;
	}
	if (infile.SamplePos() >= infile.SampleNum()) {
		rc = -1;
	}

	// バッファの末尾に近づいたら
	// 残りの未解析データをバッファの先頭にコピーして
	// 次のターンで解析を行う。
	if (carrier_data->IsTail(8)) {
		carrier_data->Shift();
	}

	return rc;
}

/// @brief l3b -> l3c シリアルデータを搬送波ビットデータに変換する
///
/// @param[in] data シリアルデータ（1ビット分）
/// @param[out] c_data 搬送波データ
/// @return bufferに書き込んだサイズ
int ParseWav::encode_phase2(uint8_t data, CarrierData *c_data)
{
	return carrier_parser.EncodeToCarrier(data, c_data);
}

/// @brief シリアルデータを搬送波ビットデータに変換しさらにwaveサンプルデータに変換する
///
/// @param[in] data シリアルデータ（1ビット分）
/// @param[out] buffer waveデータ
/// @param[in] buffer_len waveデータバッファ長さ
/// @param[out] c_len  搬送波データの長さ
/// @return bufferに書き込んだサイズ
int ParseWav::encode_phase1(uint8_t data, uint8_t *buffer, int buffer_len, int *c_len)
{
	int len = 0;
	int pos = 0;
	uint8_t wav_samples[10];

	len = encode_phase2(data, carrier_data);
	if (c_len != NULL) {
		*c_len = len;
	}
	carrier_data->Shift();

	// convert to wave sample data
	wave_data->Clear();
	for(int j=0; j < len; j++) {
		pos = wave_parser.EncodeToWave(carrier_data, wav_samples, 10);
		wave_data->AddString(wav_samples, pos, 0);
	}
	// サンプリングレート変換
	pos = (int)conv.OutConvSampleData(*wave_data, 48000
						, buffer, outwav.GetSampleRate(), outwav.GetSampleBits(), buffer_len, 0);

	return pos;
}

/// @brief ファイルからデータを読みシリアルデータ1bitに変換して出力する
/// @param[in]  dir       データ進行方向 0:再生 / 1:早送り / -1:巻き戻し
/// @param[out] s_data    シリアルデータ 1-2bit
/// @param[out] s_datalen シリアルデータ長さ
/// @param[out] w_onedata NULL or waveデータ
/// @param[out] w_onelen  NULL or waveデータの長さ
/// @param[out] c_onedata NULL or シリアルデータ1-2ビット分の搬送波
/// @param[out] c_onelen  NULL or シリアルデータ1-2ビット分の搬送波の長さ
/// @return 長さ / データ終わり: 0
int ParseWav::GetData(int dir, uint8_t *s_data, int &s_datalen, char *w_onedata, int *w_onelen, char *c_onedata, int *c_onelen)
{
	int retry = 1;

	if (dir >= 0 && infile.IsEndPos(1)) {
		if (w_onelen) *w_onelen = 0;
		if (c_onelen) *c_onelen = 0;
		return 0;
	} else if (dir < 0 && infile.IsFirstPos(1)) {
		if (w_onelen) *w_onelen = 0;
		if (c_onelen) *c_onelen = 0;
		return 0;
	}

	do {
		decode_phase1(param.GetFskSpeed(), start_phase, w_onedata, w_onelen);
		if (dir == 0) decode_phase2(s_data, s_datalen, c_onedata, c_onelen);
		retry--;
	} while (s_data[0] == '?' && retry > 0);

	if (s_data[0] == '?') {
		s_data[0] = '1';
	}

	return inwav_dirpos;
}

/// @brief シリアルデータ1bitを変換してバッファに出力する
///
/// @param[in]  data       入力データ
/// @param[out] data_len   入力データ長さ
/// @param[out] buffer     変換後のデータ
/// @param[in]  buffer_len 変換後のデータ長さ
/// @param[out] c_len      c_dataに変換した時の長さ
/// @return bufferに書き込んだサイズ
int ParseWav::SetData(uint8_t data, int &data_len, uint8_t *buffer, int buffer_len, int *c_len)
{
	int len = 0;
	if (infile_type == 1) {
		// l3cデータに変換
		if (data_len >= 88 && (data & 1) == 0) {
			// cr + lf
			buffer[0]='\r';
			buffer[1]='\n';
			buffer += 2;
			len += 2;
			data_len = 0;
		}
		carrier_data->Clear();
		len += encode_phase2(data, carrier_data);
		for(int i=0; i<carrier_data->GetWritePos(); i++) {
			*buffer = carrier_data->At(i).Data();
			buffer++;
		}
		if (c_len != NULL) {
			*c_len = len;
		}
	} else {
		// waveサンプルデータに変換
		len += encode_phase1(data, buffer, buffer_len, c_len);
	}
	infile.SamplePos(infile.SamplePos() + len);
	if (infile.SampleNum() < infile.SamplePos()) {
		infile.SampleNum(infile.SamplePos());
	}
	return len;
}

/// 音声出力用のフォーマットパラメータを設定
///
///
///
///
void ParseWav::SetFormatForSound(int rate, int bits, int ch)
{
	sndwav.Init(rate, bits, ch);
}

}; /* namespace PARSEWAV */
