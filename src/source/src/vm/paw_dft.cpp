/// @file paw_dft.cpp
///
/// @author Sasaji
/// @date   2017.12.01
///

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "paw_dft.h"
#include "paw_file.h"


namespace PARSEWAV
{

/// @brief コンストラクタ
///
Dft::Dft()
{
	samples = 0;
	memset(h, 0, sizeof(h));
	amp[0] = 0.0;
	amp[1] = 0.0;
}

/// @brief デストラクタ
///
Dft::~Dft()
{
}

/// @brief 補正用dftの係数を作成
/// @param[in] samples 1200Hzのサンプル数
/// @param[in] type 1:cos 2:sin
/// @param[in] amp0 1200Hz波の振幅
/// @param[in] amp1 2400Hz波の振幅
///
void Dft::Init(double samples, int type, int amp0, int amp1)
{
	int half[2];
	half[0] = (int)(samples / 2.0);
	half[1] = (int)(samples / 4.0);
	int n, k;

	for(k = 0; k < 2; k++) {
		for(n = 0; n < (int)(samples * 2); n++) {
			double rag = -2.0 * M_PI * (k + 1) * (n - half[k]) / samples;
			if (type == 1) {
				h[k][n] = cos(rag);
			} else {
				h[k][n] = sin(rag);
			}
		}
	}

	amp[0] = amp0;
	amp[1] = amp1;

	this->samples = samples;
	a_max = 0;
	a_min = 0;

	return;
}

/// @brief フーリエ変換
/// 1200Hz, 2400Hz,4800Hzの正弦波or余弦波を合成して補正する
///
/// @param[in] w_data    元の波形データ
/// @param[out] wc_data  補正後の波形データ
/// @return 変換したデータ数
int Dft::Calcrate(WaveData *w_data, WaveData *wc_data)
{
	int k;
	int n, n_min, n_max;
	int p;
	int wc;
	double dat;
	int fdata[2];

	int half[2];
	half[0] = (int)(samples / 2.0);
	half[1] = (int)(samples / 4.0);

//	int w_rpos = w_data->GetReadPos();
//	int wc_wpos = wc_data->GetWritePos();
	for (p = w_data->GetReadPos(); p < (w_data->GetWritePos() - half[0]); p++) {
		for (k = 0; k < 2; k++) {
			fdata[k] = 0;

			dat = 0.0;
			n_min = p - half[k];
			if (n_min < 0) n_min = 0;
			n_max = p + half[k];
			for (n = n_min; n < n_max; n++) {
				dat += (double)((int)w_data->At(n).Data() - 128) * h[k][n - n_min];
			}
			int idat = (int)(dat * amp[k] / samples);
			fdata[k] = idat;
		}
		wc = (fdata[0] + fdata[1]) / 1000;
		if (a_max < wc) a_max = wc;
		if (a_min > wc) a_min = wc;
		if (wc >= 128) wc = 127;
		if (wc < -128) wc = -128;

		wc_data->Add((uint8_t)(wc + 128), w_data->GetRead().SPos());
		w_data->IncreaseReadPos();
		if (wc_data->IsFull()) {
			break;
		}
	}

#if 0
	if (gLogFile.IsOpened()) {
		wc = wc_wpos;
		for (p = w_rpos; p < w_data->GetReadPos(); p++) {
			gLogFile.Fprintf("%d,%d,%d\n"
				, w_data->At(p).SPos()
				, ((int)w_data->At(p).Data() - 128)
				, ((int)wc_data->At(wc).Data() - 128)
			);
			wc++;
		}
	}
#endif

	return wc_data->GetWritePos();
}

}; /* namespace PARSEWAV */
