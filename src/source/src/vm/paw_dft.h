/// @file paw_dft.h
///
/// @author Sasaji
/// @date   2017.12.01
///


#ifndef _PARSEWAV_DFT_H_
#define _PARSEWAV_DFT_H_

#include "../common.h"
#include "paw_datas.h"


namespace PARSEWAV
{

/// @brief フーリエ変換クラス
class Dft
{
private:
	double samples;		// 1200/2400Hzのサンプル数
	double h[2][100];	// 0:1200Hz 1:2400Hz

	double amp[2];

	int a_max;
	int a_min;

public:
	Dft();
	~Dft();

	void Init(double samples, int type, int amp0, int amp1);
	int  Calcrate(WaveData *w_data, WaveData *wc_data);

	int GetAmpMax() { return a_max; }
	int GetAmpMin() { return a_min; }
};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_DFT_H_ */
