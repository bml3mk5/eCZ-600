/// @file paw_parsecar.cpp
///
/// @author Sasaji
/// @date   2019.08.01
///

#include "paw_parsecar.h"
#include "paw_defs.h"
#include "paw_file.h"
//#include "utils.h"


namespace PARSEWAV 
{

// 2400ボーの時はエッジ判定を優先
static const struct st_pattern carrier_edge_pattern[]={
	{ (const uint8_t *)"0010", 4 },		// 01 (2400 baud)
	{ (const uint8_t *)"1101", 4 },		// 01 (2400 baud frip)
	{ (const uint8_t *)"0100", 4 },		// 10 (2400 baud)
	{ (const uint8_t *)"1011", 4 },		// 10 (2400 baud frip)
	{ NULL, 0 }
};

//#define MAX_CARRIER_PATTERN	1
static const struct st_pattern carrier_pattern[4][4]={
	{
		{ (const uint8_t *)"11001100", 8 },	// 0 (600 baud)
		{ (const uint8_t *)"00110011", 8 },	// 0 (600 baud frip)
		{ (const uint8_t *)"10101010", 8 },	// 1 (600 baud)
		{ (const uint8_t *)"01010101", 8 },	// 1 (600 baud frip)
	},{
		{ (const uint8_t *)"1100", 4 },		// 0 (1200 baud)
		{ (const uint8_t *)"0011", 4 },		// 0 (1200 baud frip)
		{ (const uint8_t *)"1010", 4 },		// 1 (1200 baud)
		{ (const uint8_t *)"0101", 4 },		// 1 (1200 baud frip)
	},{
		{ (const uint8_t *)"11", 2 },		// 0 (2400 baud)
		{ (const uint8_t *)"00", 2 },		// 0 (2400 baud frip)
		{ (const uint8_t *)"10", 2 },		// 1 (2400 baud)
		{ (const uint8_t *)"01", 2 },		// 1 (2400 baud frip)
	},{
		{ (const uint8_t *)"1100110011001100", 16 },	// 0 (300 baud)
		{ (const uint8_t *)"0011001100110011", 16 },	// 0 (300 baud frip)
		{ (const uint8_t *)"1010101010101010", 16 },	// 1 (300 baud)
		{ (const uint8_t *)"0101010101010101", 16 },	// 1 (300 baud frip)
	}
};

//

CarrierParser::CarrierParser()
	: ParserBase()
{
	phase = 0;
	frip = 0;
	baud24_frip = 0;
}

void CarrierParser::ClearResult()
{
}

/// @brief 初期化
///
void CarrierParser::Init()
{
	ParserBase::Init();

	phase = 0;
	frip = 0;
	baud24_frip = 0;

	prev_data = 0;
	prev_width = 0;
	over_pos = 0;
}

/// @brief l3cファイル(搬送波ビットデータ)からサイズを計算
int CarrierParser::CalcL3CSize(InputFile &file)
{
	int l;
	int sample_num = 0;
	int file_size = file.FileLength();
	file.Fseek(0, FILEIO::SEEKSET);
	while(file_size > 0) {
		file_size--;

		l = file.Fgetc();
		l &= 0xff;

		if (l == '\r' || l == '\n') {
			continue;
		}
		sample_num++;
	}
	file.SampleNum(sample_num);
	file.Fseek(0, FILEIO::SEEKSET);
	return sample_num;
}

/// @brief l3cファイルのフォーマットをチェック（チェックしていないが）
///
/// @param[in] file 入力ファイル
/// @return 0
///
int CarrierParser::CheckL3CFormat(InputFile &file)
{
	SetInputFile(file);

	CalcL3CSize(file);

	file.SampleRate(GetSampleRate());

	return 0;
}

double CarrierParser::GetSampleRate()
{
	int baud_mag = param->GetFskSpeed() + 1;
	return 4800.0 * baud_mag;
}

/// @brief l3cファイル(搬送波ビットデータ)から１データ読んでバッファに追記
///
/// @param[in,out] c_data 搬送波データ用のバッファ(追記していく)
/// @param[in]     size   読み込む数
/// @return 読み込んだデータの長さ
///
///
int CarrierParser::GetL3CSample(CarrierData *c_data, int size)
{
	int l;

	while(c_data->IsFull() != true && size > 0 && infile->SamplePos() < infile->SampleNum()) {
		l = infile->Fgetc();
		l &= 0xff;

		if (l == '\r' || l == '\n') {
			continue;
		}
		c_data->Add(l, infile->SamplePos());
		infile->IncreaseSamplePos();
		size--;
	}
	if (infile->SamplePos() >= infile->SampleNum()) {
		c_data->LastData(true);
	}
	return c_data->GetWritePos();
}

/// @brief l3cファイル(搬送波ビットデータ)から１サンプルスキップする
///
/// @param[in] dir
/// @return スキップ数
///
int CarrierParser::SkipL3CSample(int dir)
{
	int l;
	int pos = 0;
	if (dir > 0) {
		while(pos < dir && !infile->IsEndPos(1)) {
			l = infile->Fgetc();
			if (l == '\r' || l == '\n') {
				continue;
			}
			infile->IncreaseSamplePos();
			pos++;
		}
	} else if (dir < 0) {
		while(pos > dir && !infile->IsFirstPos(1)) {
			infile->Fseek(-1, FILEIO::SEEKCUR);
			l = infile->Fgetc();
			infile->Fseek(-1, FILEIO::SEEKCUR);
			if (l == '\r' || l == '\n') {
				continue;
			}
			infile->DecreaseSamplePos();
			pos--;
		}
	}

//	sample_ms = (uint32_t)(1000.0 * (double)sample_pos / 4800.0);

	return pos;
}

/// @brief 一致するパターンを探してデコード
int CarrierParser::Decode(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen)
{
	int rc = 0;
	if (phase == 0) {
		rc = FindStartCarrierBit(c_data, baud, step, s_data, s_datalen, c_onedata, c_onelen);
		if (step >= 0) phase = 1;
	} else {
		rc = DecodeToSerial(c_data, baud, step, s_data, s_datalen, c_onedata, c_onelen);
		if (step < 0) phase = 0;
	}
	return rc;
}

/// @brief 一致するパターンを探す
///
/// @param[in]   c_data   搬送波
/// @param[in]   baud     600baud:0 1200baud:1 2400baud:2 300baud:3
/// @param[out]  step     スタート位置 / -1 なし
/// @param[out]  s_data   シリアルデータ
/// @param[out] s_datalen シリアルデータ長さ
/// @param[out] c_onedata 搬送波データ1つ
/// @param[out] c_onelen  搬送波データ1つの長さ
/// @return 0
int CarrierParser::FindStartCarrierBit(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen)
{
	int pos1[4];
	int pos = -1;
	int idx_ptn = (baud & 3);
	int best_num = -1;
	int best_pos = c_data->GetSize();
	int i, n;
	CSampleData samples[4];
	int samples_len;

	samples[0].Data('?');
	samples_len = 1;

	// 2400ボーの時はエッジを探す
	if (idx_ptn == IDX_PTN_2400) {
		for(i=0; i<4; i++) {
			pos1[i]=c_data->FindRead(0, carrier_edge_pattern[i].ptn, carrier_edge_pattern[i].len);
		}
		for(i=0; i<4; i++) {
			if (pos1[i] >= 0 && best_pos > pos1[i]) {
				best_pos = pos1[i];
				best_num = i;
			}
		}
		if (best_num >= 0) {
			// あり
			pos = best_pos + carrier_edge_pattern[best_num].len;

			if ((best_num % 2) == 0) {
				frip = 0;
			} else {
				frip = 1;
			}

			samples_len = 2;
			for(n=0; n<samples_len; n++) {
				samples[n].Set(c_data->GetRead(best_pos + n * 2));
				samples[n].Attr(idx_ptn);
			}
			if ((best_num / 2) == 0) {
				samples[0].Data('0');
				samples[1].Data('1');
			} else {
				samples[0].Data('1');
				samples[1].Data('0');
			}
		}
	}

	// 一致するパターンを探す
	if (best_num < 0) {
		for(i=0; i<4; i++) {
			pos1[i]=c_data->FindRead(0, carrier_pattern[idx_ptn][i].ptn, carrier_pattern[idx_ptn][i].len);
		}
		for(i=0; i<4; i++) {
			if (pos1[i] >= 0 && best_pos > pos1[i]) {
				best_pos = pos1[i];
				best_num = i;
			}
		}
		if (best_num >= 0) {
			// あり
			pos = best_pos + carrier_pattern[idx_ptn][best_num].len;

			if ((best_num % 2) == 0) {
				frip = 0;
			} else {
				frip = 1;
			}

			samples_len = 1;
			samples[0].Set(c_data->GetRead(best_pos));
			samples[0].Attr(idx_ptn);
			if ((best_num / 2) == 0) {
				samples[0].Data('0');
			} else {
				samples[0].Data('1');
			}
		}
	}

	if (pos >= 0) {
		// データ有り
#if 0
		if (gLogFile.IsOpened()) {
			// デバッグログ
			gLogFile.Fprintf("p2 fst c:%12d w_pos:%d pos:%d frip:%d ("
				, samples[0].SPos()
/*				, UTILS::get_time_cstr(infile->CalcrateSampleUSec(samples[0].SPos())) */
				, samples_len /* s_data->GetTotalWritePos() */
				, best_pos, frip);
			CSampleString str(*c_data, c_data->GetReadPos() + best_pos, pos - best_pos);
			gLogFile.Fputs(str.Get());
			gLogFile.Fputs(")\n");
		}
#endif
		for(int i=0; i<samples_len; i++) {
			s_data[i] = samples[i].Data();
		}
		s_data[samples_len] = '\0';
		s_datalen = samples_len;
#if 0
		if (gLogFile.IsOpened()) {
			gLogFile.Fputs("fst: ");
			gLogFile.Fputs((const char *)s_data);
			gLogFile.Fputs("\n");
		}
#endif
		SetOneCarrierData(c_data, c_onedata, pos, c_onelen);
		c_data->AddReadPos(pos);

	} else {
		// データなし
		SetOneCarrierData(c_data, c_onedata, c_data->RemainLength(), c_onelen);
		c_data->SkipReadPos();
	}

	step = pos;

	return 0;
}

/// @brief 搬送波(2400/1200Hz)のデータからシリアルデータを１ビット取り出す
///
/// @param[in]  c_data    搬送波
/// @param[in]  baud      600baud:0 1200baud:1 2400baud:2 300baud:3
/// @param[out] step      スタート位置 / -1 なし
/// @param[out] s_data    シリアルデータ
/// @param[out] s_datalen シリアルデータ長さ
/// @param[out] c_onedata 搬送波データ1つ
/// @param[out] c_onelen  搬送波データ1つの長さ
/// @return 次の位置
int CarrierParser::DecodeToSerial(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen)
{
	int pos = -1;
	int idx_ptn = (baud & 3);
	int len;

	CSampleData sample;
	const uint8_t *ptn;


	len = carrier_pattern[idx_ptn][frip].len;
	ptn = carrier_pattern[idx_ptn][frip].ptn;
	sample.Set(c_data->GetRead());
	sample.Attr(idx_ptn);

	if (c_data->CompareRead(0, ptn, len) == 0) {	// 0
		// 0
		sample.Data('0');

		pos = len;
		if (idx_ptn == IDX_PTN_2400) {
			// 2400ボーで0の場合、常にfripする
			frip = (1 - frip);
		}
	}
	if (pos < 0 && idx_ptn == IDX_PTN_2400) {
		// 2400 ボーのときはfripして再度0を検索
		len = carrier_pattern[idx_ptn][1 - frip].len;
		ptn = carrier_pattern[idx_ptn][1 - frip].ptn;
		if (c_data->CompareRead(0, ptn, len) == 0) {	// 0
			sample.Data('0');

			pos = len;
			if (idx_ptn == IDX_PTN_2400) {
				// 2400ボーで0の場合、常にfripする
				frip = (1 - frip);
			}
		}
	}
	if (pos < 0) {
		len = carrier_pattern[idx_ptn][2 + frip].len;
		ptn = carrier_pattern[idx_ptn][2 + frip].ptn;
		if (c_data->CompareRead(0, ptn, len) == 0)	{ // 1
			// 1
			sample.Data('1');

			pos = len;
		}
	}

	if (pos >= 0) {
		// データ追加
		s_data[0] = sample.Data();
		s_data[1] = '\0';
		s_datalen = 1;
#if 0
		if (gLogFile.IsOpened()) {
			gLogFile.Fputs("dts: ");
			gLogFile.Fputs((const char *)s_data);
			gLogFile.Fputs("\n");
		}
#endif
		SetOneCarrierData(c_data, c_onedata, pos, c_onelen);
		c_data->AddReadPos(pos);

	} else {
		// 一致するパターンがない
		sample.Data('?');
		sample.Flgs(0x80);

		s_data[0] = sample.Data();
		s_data[1] = '\0';
		s_datalen = 1;
#if 0
		if (gLogFile.IsOpened()) {
			gLogFile.Fputs("err: ");
			gLogFile.Fputs((const char *)s_data);
			gLogFile.Fputs("\n");
		}
#endif
		SetOneCarrierData(c_data, c_onedata, c_data->RemainLength(), c_onelen);

		if (c_data->RemainLength() > 0) {
			c_data->IncreaseReadPos();
		}

#if 0
		if (gLogFile.IsOpened()) {
			// デバッグログ
			gLogFile.Fprintf("p2 c2s c:%12d s:%8d: error ("
				, sample.SPos()
				/* , UTILS::get_time_cstr(infile->CalcrateSampleUSec(sample.SPos())) */
				, 1 /* s_data->GetTotalWritePos() */
				);
			CSampleString str(*c_data, c_data->GetReadPos(), c_data->RemainLength() >= len ? len : c_data->RemainLength());
			gLogFile.Fputs(str.Get());
			gLogFile.Fputs(")\n");
		}
#endif
	}	

	step = pos;

	return 0x10000;
}

void CarrierParser::SetOneCarrierData(CarrierData *c_data, char *c_onedata, int len, int *c_onelen)
{
	if (c_onedata != NULL) {
		*c_onelen = len;
		for(int i=0; i<len; i++) {
			*c_onedata = c_data->GetRead(i).Data();
			c_onedata++;
		}
		*c_onedata = '\0';
	}
}

/// @brief シリアルデータを搬送波ビットデータに変換する
///
/// @param[in] s_data シリアルデータ
/// @param[out] c_data 搬送波データ
/// @return 搬送波データに変換した長さ
int CarrierParser::EncodeToCarrier(uint8_t s_data, CarrierData *c_data)
{
	int idx_ptn = (param->GetBaud() & 3);
	int len = 0;
	const st_pattern *pattern;

	if (s_data & 0x01) {
		// 1
		pattern = &carrier_pattern[idx_ptn][2 + baud24_frip];
	} else {
		// 0
		pattern = &carrier_pattern[idx_ptn][baud24_frip];
		if (idx_ptn == IDX_PTN_2400) {
			// 2400ボーの時 fripする
			baud24_frip = (1 - baud24_frip);
		}
	}
	len = c_data->AddString(pattern->ptn, pattern->len, 0);

	return len;
}

}; /* namespace PARSEWAV */
