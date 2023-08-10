/// @file paw_parsecar.h
///
/// @author Sasaji
/// @date   2019.08.01
///


#ifndef _PARSEWAV_PARSECAR_H_
#define _PARSEWAV_PARSECAR_H_

#include "../common.h"
#include "paw_parse.h"
//#include "errorinfo.h"
#include "paw_datas.h"
#include "paw_param.h"
#include "paw_file.h"


namespace PARSEWAV 
{

/// 搬送波データ解析用クラス
class CarrierParser : public ParserBase
{
private:
	//
	int phase;

	int frip;

	/// 2400ボーエンコード時のフリップ有無
	int baud24_frip;

	/// 出力位置
	uint32_t prev_data;
	int prev_width;
//	uint8_t over_buf[128];
	int over_pos;

	int CalcL3CSize(InputFile &file);

	void SetOneCarrierData(CarrierData *c_data, char *c_onedata, int len, int *c_onelen);

	int FindStartCarrierBit(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen);
	int DecodeToSerial(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen);

public:
	CarrierParser();

	void ClearResult();
	void Init();

	int CheckL3CFormat(InputFile &file);

	int GetL3CSample(CarrierData *c_data, int size);

	int SkipL3CSample(int dir);

	int Decode(CarrierData *c_data, int baud, int &step, uint8_t *s_data, int &s_datalen, char *c_onedata, int *c_onelen);

	int EncodeToCarrier(uint8_t s_data, CarrierData *c_data);

	double GetSampleRate();

};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_PARSECAR_H_ */
