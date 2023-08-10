/// @file paw_param.cpp
///
/// @brief waveデータ <-> 搬送波データ <-> シリアルデータ　変換用パラメータ
///
/// @author Sasaji
/// @date   2011.7.1
///

#include "paw_param.h"


namespace PARSEWAV
{

const int c_sample_rate[] = {
	11025,
	22050,
	44100,
	48000,
	0
};

Parameter::Parameter()
{
	Clear();
}

Parameter::~Parameter()
{
}

void Parameter::Clear()
{
	// パラメータ初期化
	fsk_speed = 0;

	for(int i=0; i<3; i++) {
		freq[i] = (1200 << i);
	}
	range[0] = 25;
	range[1] = 50;
	reverse = false;

	half_wave = true;
	correct_type = 0;
	correct_amp[0] = 1000;
	correct_amp[1] = 1000;

	baud = 0;
	word_select = 0x04;

	debug_log = 0;

	sample_rate = 3;	// 48000
	sample_bits = 0;	// 8bit
}

int Parameter::GetSampleRate(int val)
{
	if (val >= 0 && val < 4) {
		return c_sample_rate[val];
	} else {
		return c_sample_rate[3];
	}
}

int Parameter::GetSampleRate(void) const
{
	return GetSampleRate(sample_rate);
}

int Parameter::GetBaseFreq(void) const
{
	return 1200 * (fsk_speed + 1);
}

}; /* namespace PARSEWAV */
