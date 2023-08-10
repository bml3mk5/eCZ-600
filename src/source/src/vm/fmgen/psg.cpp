/// ---------------------------------------------------------------------------
/// @file psg.cpp
///	PSG Sound Implementation
///	Copyright (C) cisc 1997, 1999.
/// ---------------------------------------------------------------------------
///	$Id: psg.cpp,v 1.10 2002/05/15 21:38:01 cisc Exp $

#include "headers.h"
#include "misc.h"
#include "psg.h"
// for AY-3-8190/8192
#include "../vm_defs.h"

#include "../../fileio.h"

// ---------------------------------------------------------------------------
///	コンストラクタ
///
/// @param[in] type: 0:SSG  0x80:PSG(AY-3-819x)
PSG::PSG(int type)
{
	// テーブル初期化
	for(int i = 0; i < noisetablesize; i++) {
		noisetable[i] = 0;
	}
	for(int i = 0; i < 32; i++) {
		EmitTableL[i] = -1;
#ifdef USE_FMGEN_STEREO
		EmitTableR[i] = -1;
#endif
	}
	for(int i = 0; i < 16; i++) {
		for(int j = 0; j < 64; j++) {
			enveloptable_l[i][j] = 0;
#ifdef USE_FMGEN_STEREO
			enveloptable_r[i][j] = 0;
#endif
		}
	}
	SetVolume(0, 0, type);
	MakeNoiseTable();
	Reset();
	mask = 0x3f;
	envelop_l = enveloptable_l[0]; // temporary fix
#ifdef USE_FMGEN_STEREO
	envelop_r = enveloptable_r[0]; // temporary fix
#endif
}

///	デストラクタ
PSG::~PSG()
{

}

// ---------------------------------------------------------------------------
///	PSG を初期化する(RESET)
///
void PSG::Reset()
{
	for (int i=0; i<14; i++)
		SetReg(i, 0);
	SetReg(7, 0xff);
	SetReg(14, 0xff);
	SetReg(15, 0xff);
}

// ---------------------------------------------------------------------------
///	クロック周波数の設定
///
/// @param[in] clock: 動作クロック
/// @param[in] rate: サンプリングレート
void PSG::SetClock(int clock, int rate)
{
	tperiodbase = int((1 << toneshift ) / 4.0 * clock / rate);
	eperiodbase = int((1 << envshift  ) / 4.0 * clock / rate);
	nperiodbase = int((1 << noiseshift) / 4.0 * clock / rate);

	// 各データの更新
	int tmp;
	tmp = ((reg[0] + reg[1] * 256) & 0xfff);
	speriod[0] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
	tmp = ((reg[2] + reg[3] * 256) & 0xfff);
	speriod[1] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
	tmp = ((reg[4] + reg[5] * 256) & 0xfff);
	speriod[2] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
	tmp = reg[6] & 0x1f;
	nperiod = tmp ? nperiodbase / tmp : ((device_type & 0x80) ? nperiodbase / 32 : nperiodbase);
	tmp = ((reg[11] + reg[12] * 256) & 0xffff);
	eperiod = tmp ? eperiodbase / tmp : ((device_type & 0x82) == 0x82 ? eperiodbase / 65536 : eperiodbase * 2);
}

// ---------------------------------------------------------------------------
///	ノイズテーブルを作成する
///
void PSG::MakeNoiseTable()
{
	if (!noisetable[0])
	{
		int noise = 14321;
		for (int i=0; i<noisetablesize; i++)
		{
			int n = 0;
			for (int j=0; j<32; j++)
			{
				n = n * 2 + (noise & 1);
				noise = (noise >> 1) | (((noise << 14) ^ (noise << 16)) & 0x10000);
			}
			noisetable[i] = n;
		}
	}
}

// ---------------------------------------------------------------------------
///	出力テーブルを作成
///	素直にテーブルで持ったほうが省スペース。
///
/// @param[in] volume_l: 左音量
/// @param[in] volume_r: 右音量
/// @param[in] type: 0: SSG  0x8x: PSG(AY-3-819x)
void PSG::SetVolume(int volume_l, int volume_r, int type)
{
	device_type = type;

	double base_l = 0x4000 / 3.0 * pow(10.0, volume_l / 40.0);
#ifdef USE_FMGEN_STEREO
	double base_r = 0x4000 / 3.0 * pow(10.0, volume_r / 40.0);
#endif
	if (device_type) {
		// AY-3-8190/8192 (PSG): 16step
		double base = base_l;
		for (int i=31; i>=3; i-=2)
		{
			EmitTableL[i] = EmitTableL[i-1] = int(base_l);
#ifdef USE_FMGEN_STEREO
			EmitTableR[i] = EmitTableR[i-1] = int(base_r);
#endif
			if (device_type & 1) {
				if (i >= 17) {
					base_l /= 1.14;
					base = base_l;
				} else {
					// linear
					base_l = base * (i-2) / 15;
				}
			} else {
				// default
				base_l /= 1.189207115;
				base_l /= 1.189207115;
			}
#ifdef USE_FMGEN_STEREO
			base_r = base_l;
#endif
		}
	} else {
		// YM2203 (SSG): 32step
		for (int i=31; i>=2; i--)
		{
			EmitTableL[i] = int(base_l);
#ifdef USE_FMGEN_STEREO
			EmitTableR[i] = int(base_r);
#endif
			base_l /= 1.189207115;
#ifdef USE_FMGEN_STEREO
			base_r = base_l;
#endif
		}
	}
	EmitTableL[1] = 0;
	EmitTableL[0] = 0;
#ifdef USE_FMGEN_STEREO
	EmitTableR[1] = 0;
	EmitTableR[0] = 0;
#endif
	MakeEnvelopTable();

	SetChannelMask(~mask);
}

void PSG::SetChannelMask(int c)
{
	mask = ~c;
	for (int i=0; i<3; i++)
	{
		olevel_l[i] = mask & (1 << i) ? EmitTableL[(reg[8+i] & 15) * 2 + 1] : 0;
#ifdef USE_FMGEN_STEREO
		olevel_r[i] = mask & (1 << i) ? EmitTableR[(reg[8+i] & 15) * 2 + 1] : 0;
#endif
	}
}

// ---------------------------------------------------------------------------
///	エンベロープ波形テーブル
///
void PSG::MakeEnvelopTable()
{
	// 0 lo  1 up 2 down 3 hi
	static uint8_t table1[16*2] =
	{
		2,0, 2,0, 2,0, 2,0, 1,0, 1,0, 1,0, 1,0,
		2,2, 2,0, 2,1, 2,3, 1,1, 1,3, 1,2, 1,0,
	};
	static uint8_t table2[4] = {  0,  0, 31, 31 };
	static uint8_t table3[4] = {  0,  1, static_cast<uint8_t>(-1),  0 };

	uint32_t* ptr_l = enveloptable_l[0];
#ifdef USE_FMGEN_STEREO
	uint32_t* ptr_r = enveloptable_r[0];
#endif

	for (int i=0; i<16*2; i++)
	{
		uint8_t v = table2[table1[i]];

		for (int j=0; j<32; j++)
		{
			*ptr_l++ = EmitTableL[v];
#ifdef USE_FMGEN_STEREO
			*ptr_r++ = EmitTableR[v];
#endif
			v += table3[table1[i]];
		}
	}
}

// ---------------------------------------------------------------------------
///	PSG のレジスタに値をセットする
///
///	@param[in] regnum:		レジスタの番号 (0 - 15)
///	@param[in] data:		セットする値
///
void PSG::SetReg(uint32_t regnum, uint8_t data)
{
	if (regnum < 0x10)
	{
		reg[regnum] = data;
		switch (regnum)
		{
			int tmp;

		case 0:		// ChA Fine Tune
		case 1:		// ChA Coarse Tune
			tmp = ((reg[0] + reg[1] * 256) & 0xfff);
			speriod[0] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
			break;

		case 2:		// ChB Fine Tune
		case 3:		// ChB Coarse Tune
			tmp = ((reg[2] + reg[3] * 256) & 0xfff);
			speriod[1] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
			break;

		case 4:		// ChC Fine Tune
		case 5:		// ChC Coarse Tune
			tmp = ((reg[4] + reg[5] * 256) & 0xfff);
			speriod[2] = tmp ? tperiodbase / tmp : ((device_type & 0x82) == 0x82 ? tperiodbase / 4096 : tperiodbase);
			break;

		case 6:		// Noise generator control
			data &= 0x1f;
			nperiod = data ? nperiodbase / data : ((device_type & 0x80) ? nperiodbase / 32 : nperiodbase);
			break;

		case 8:
			olevel_l[0] = mask & 1 ? EmitTableL[(data & 15) * 2 + 1] : 0;
#ifdef USE_FMGEN_STEREO
			olevel_r[0] = mask & 1 ? EmitTableR[(data & 15) * 2 + 1] : 0;
#endif
			break;

		case 9:
			olevel_l[1] = mask & 2 ? EmitTableL[(data & 15) * 2 + 1] : 0;
#ifdef USE_FMGEN_STEREO
			olevel_r[1] = mask & 2 ? EmitTableR[(data & 15) * 2 + 1] : 0;
#endif
			break;

		case 10:
			olevel_l[2] = mask & 4 ? EmitTableL[(data & 15) * 2 + 1] : 0;
#ifdef USE_FMGEN_STEREO
			olevel_r[2] = mask & 4 ? EmitTableR[(data & 15) * 2 + 1] : 0;
#endif
			break;

		case 11:	// Envelop period
		case 12:
			tmp = ((reg[11] + reg[12] * 256) & 0xffff);
			eperiod = tmp ? eperiodbase / tmp : ((device_type & 0x82) == 0x82 ? eperiodbase / 65536 : eperiodbase * 2);
			break;

		case 13:	// Envelop shape
			ecount = 0;
			envelop_l = enveloptable_l[data & 15];
#ifdef USE_FMGEN_STEREO
			envelop_r = enveloptable_r[data & 15];
#endif
			break;
		}
	}
}

// ---------------------------------------------------------------------------
///
///
inline void PSG::StoreSample(Sample& dest, int32_t data)
{
	if (sizeof(Sample) == 2)
		dest = (Sample) Limit(dest + data, 0x7fff, -0x8000);
	else
		dest += data;
}

// ---------------------------------------------------------------------------
///	PCM データを吐き出す(2ch)
///
///	@param[in,out] dest:		PCM データを展開するポインタ
///	@param[in]     nsamples:	展開する PCM のサンプル数
///
void PSG::Mix(Sample* dest, int nsamples)
{
	uint8_t chenable[3], nenable[3];
	uint8_t r7 = ~reg[7];

	if ((r7 & 0x3f) | ((reg[8] | reg[9] | reg[10]) & 0x1f))
	{
		chenable[0] = (r7 & 0x01) && (speriod[0] <= (1 << toneshift));
		chenable[1] = (r7 & 0x02) && (speriod[1] <= (1 << toneshift));
		chenable[2] = (r7 & 0x04) && (speriod[2] <= (1 << toneshift));
		nenable[0]  = (r7 >> 3) & 1;
		nenable[1]  = (r7 >> 4) & 1;
		nenable[2]  = (r7 >> 5) & 1;

		int noise;
		int sample_l;
		uint32_t env_l;
		uint32_t* p1_l = ((mask & 1) && (reg[ 8] & 0x10)) ? &env_l : &olevel_l[0];
		uint32_t* p2_l = ((mask & 2) && (reg[ 9] & 0x10)) ? &env_l : &olevel_l[1];
		uint32_t* p3_l = ((mask & 4) && (reg[10] & 0x10)) ? &env_l : &olevel_l[2];
#ifdef USE_FMGEN_STEREO
		int sample_r;
		uint32_t env_r;
		uint32_t* p1_r = ((mask & 1) && (reg[ 8] & 0x10)) ? &env_r : &olevel_r[0];
		uint32_t* p2_r = ((mask & 2) && (reg[ 9] & 0x10)) ? &env_r : &olevel_r[1];
		uint32_t* p3_r = ((mask & 4) && (reg[10] & 0x10)) ? &env_r : &olevel_r[2];
#endif
		#define SCOUNT(ch)	(scount[ch] >> (toneshift+oversampling))

		if (p1_l != &env_l && p2_l != &env_l && p3_l != &env_l)
		{
			// エンベロープ無し
			if ((r7 & 0x38) == 0)
			{
				// ノイズ無し
				for (int i=0; i<nsamples; i++)
				{
					sample_l = 0;
#ifdef USE_FMGEN_STEREO
					sample_r = 0;
#endif
					for (int j=0; j < (1 << oversampling); j++)
					{
						int x, y, z;
						x = (SCOUNT(0) & chenable[0]) - 1;
						sample_l += (olevel_l[0] + x) ^ x;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[0] + x) ^ x;
#endif
						scount[0] += speriod[0];
						y = (SCOUNT(1) & chenable[1]) - 1;
						sample_l += (olevel_l[1] + y) ^ y;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[1] + y) ^ y;
#endif
						scount[1] += speriod[1];
						z = (SCOUNT(2) & chenable[2]) - 1;
						sample_l += (olevel_l[2] + z) ^ z;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[2] + z) ^ z;
#endif
						scount[2] += speriod[2];
					}
					sample_l /= (1 << oversampling);
					StoreSample(dest[0], sample_l);
#ifdef USE_FMGEN_STEREO
					sample_r /= (1 << oversampling);
					StoreSample(dest[1], sample_r);
#else
					StoreSample(dest[1], sample_l);
#endif
					dest += 2;
				}
			}
			else
			{
				// ノイズ有り
				for (int i=0; i<nsamples; i++)
				{
					sample_l = 0;
#ifdef USE_FMGEN_STEREO
					sample_r = 0;
#endif
					for (int j=0; j < (1 << oversampling); j++)
					{
#ifdef _M_IX86
						noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)] 
							>> (ncount >> (noiseshift+oversampling+1));
#else
						noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)]
							>> (ncount >> (noiseshift+oversampling+1) & 31);
#endif
						ncount += nperiod;

						int x, y, z;
						x = ((SCOUNT(0) & chenable[0]) | (nenable[0] & noise)) - 1;		// 0 or -1
						sample_l += (olevel_l[0] + x) ^ x;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[0] + x) ^ x;
#endif
						scount[0] += speriod[0];
						y = ((SCOUNT(1) & chenable[1]) | (nenable[1] & noise)) - 1;
						sample_l += (olevel_l[1] + y) ^ y;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[1] + y) ^ y;
#endif
						scount[1] += speriod[1];
						z = ((SCOUNT(2) & chenable[2]) | (nenable[2] & noise)) - 1;
						sample_l += (olevel_l[2] + z) ^ z;
#ifdef USE_FMGEN_STEREO
						sample_r += (olevel_r[2] + z) ^ z;
#endif
						scount[2] += speriod[2];
					}
					sample_l /= (1 << oversampling);
					StoreSample(dest[0], sample_l);
#ifdef USE_FMGEN_STEREO
					sample_r /= (1 << oversampling);
					StoreSample(dest[1], sample_r);
#else
					StoreSample(dest[1], sample_l);
#endif
					dest += 2;
				}
			}

			// エンベロープの計算をさぼった帳尻あわせ
			ecount = (ecount >> 8) + (eperiod >> (8-oversampling)) * nsamples;
			if (ecount >= (1 << (envshift+6+oversampling-8)))
			{
				if ((reg[0x0d] & 0x0b) != 0x0a)
					ecount |= (1 << (envshift+5+oversampling-8));
				ecount &= (1 << (envshift+6+oversampling-8)) - 1;
			}
			ecount <<= 8;
		}
		else
		{
			// エンベロープあり
			for (int i=0; i<nsamples; i++)
			{
				sample_l = 0;
#ifdef USE_FMGEN_STEREO
				sample_r = 0;
#endif
				for (int j=0; j < (1 << oversampling); j++)
				{
					env_l = envelop_l[ecount >> (envshift+oversampling)];
#ifdef USE_FMGEN_STEREO
					env_r = envelop_r[ecount >> (envshift+oversampling)];
#endif
					ecount += eperiod;
					if (ecount >= (1 << (envshift+6+oversampling)))
					{
						if ((reg[0x0d] & 0x0b) != 0x0a)
							ecount |= (1 << (envshift+5+oversampling));
						ecount &= (1 << (envshift+6+oversampling)) - 1;
					}
#ifdef _M_IX86
					noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)]
						>> (ncount >> (noiseshift+oversampling+1));
#else
					noise = noisetable[(ncount >> (noiseshift+oversampling+6)) & (noisetablesize-1)]
						>> (ncount >> (noiseshift+oversampling+1) & 31);
#endif
					ncount += nperiod;

					int x, y, z;
					x = ((SCOUNT(0) & chenable[0]) | (nenable[0] & noise)) - 1;		// 0 or -1
					sample_l += (*p1_l + x) ^ x;
#ifdef USE_FMGEN_STEREO
					sample_r += (*p1_r + x) ^ x;
#endif
					scount[0] += speriod[0];
					y = ((SCOUNT(1) & chenable[1]) | (nenable[1] & noise)) - 1;
					sample_l += (*p2_l + y) ^ y;
#ifdef USE_FMGEN_STEREO
					sample_r += (*p2_r + y) ^ y;
#endif
					scount[1] += speriod[1];
					z = ((SCOUNT(2) & chenable[2]) | (nenable[2] & noise)) - 1;
					sample_l += (*p3_l + z) ^ z;
#ifdef USE_FMGEN_STEREO
					sample_r += (*p3_r + z) ^ z;
#endif
					scount[2] += speriod[2];
				}
				sample_l /= (1 << oversampling);
				StoreSample(dest[0], sample_l);
#ifdef USE_FMGEN_STEREO
				sample_r /= (1 << oversampling);
				StoreSample(dest[1], sample_r);
#else
				StoreSample(dest[1], sample_l);
#endif
				dest += 2;
			}
		}
	}
}

// ---------------------------------------------------------------------------
//	テーブル
//
//uint32_t	PSG::noisetable[noisetablesize] = { 0, };
//int	PSG::EmitTableL[0x20] = { -1, };
//int	PSG::EmitTableR[0x20] = { -1, };
//uint32_t	PSG::enveloptable_l[16][64] = { 0, };
//uint32_t	PSG::enveloptable_r[16][64] = { 0, };

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define PSG_STATE_VERSION	2

///	ステートセーブ
void PSG::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(PSG_STATE_VERSION);	// 4 bytes

	sz += state_fio->FwriteWithSize(reg, sizeof(reg), 1);	// 16 bytes
	sz += state_fio->FputInt32_LE((int)(envelop_l - &enveloptable_l[0][0]));	// 4 bytes
	for (int i = 0; i < 3; i++) {	// 12 bytes
		sz += state_fio->FputUint32_LE(olevel_l[i]);
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
#ifdef USE_FMGEN_STEREO
		sz += state_fio->FputUint32_LE(olevel_r[i]);
#else
		sz += state_fio->FputUint32_LE(olevel_l[i]);
#endif
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
		sz += state_fio->FputUint32_LE(scount[i]);
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
		sz += state_fio->FputUint32_LE(speriod[i]);
	}
	sz += state_fio->FputUint32_LE(ecount);	// 4 bytes
	sz += state_fio->FputUint32_LE(eperiod);	// 4 bytes
	sz += state_fio->FputUint32_LE(ncount);	// 4 bytes
	sz += state_fio->FputUint32_LE(nperiod);	// 4 bytes
	sz += state_fio->FputUint32_LE(tperiodbase);	// 4 bytes
	sz += state_fio->FputUint32_LE(eperiodbase);	// 4 bytes
	sz += state_fio->FputUint32_LE(nperiodbase);	// 4 bytes
	sz += state_fio->FputInt32_LE(mask);	// 4 bytes

	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy
	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy

	if (size) *size += sz;	// 112
}

///	ステートロード
bool PSG::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != PSG_STATE_VERSION) {
		return false;
	}
	state_fio->Fread(reg, sizeof(reg), 1);	// 16 bytes
	int offset = state_fio->FgetInt32_LE();	// 4 bytes
	envelop_l = &enveloptable_l[0][0] + offset;
#ifdef USE_FMGEN_STEREO
	envelop_r = &enveloptable_r[0][0] + offset;
#endif
	for (int i = 0; i < 3; i++) {	// 12 bytes
		olevel_l[i] = state_fio->FgetUint32_LE();
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
#ifdef USE_FMGEN_STEREO
		olevel_r[i] = state_fio->FgetUint32_LE();
#else
		state_fio->FgetUint32_LE();	// skip
#endif
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
		scount[i] = state_fio->FgetUint32_LE();
	}
	for (int i = 0; i < 3; i++) {	// 12 bytes
		speriod[i] = state_fio->FgetUint32_LE();
	}
	ecount = state_fio->FgetUint32_LE();	// 4 bytes
	eperiod = state_fio->FgetUint32_LE();	// 4 bytes
	ncount = state_fio->FgetUint32_LE();	// 4 bytes
	nperiod = state_fio->FgetUint32_LE();	// 4 bytes
	tperiodbase = state_fio->FgetUint32_LE();	// 4 bytes
	eperiodbase = state_fio->FgetUint32_LE();	// 4 bytes
	nperiodbase = state_fio->FgetUint32_LE();	// 4 bytes
	mask = state_fio->FgetInt32_LE();	// 4 bytes

	state_fio->FgetInt32_LE();	// 4 bytes dummy
	state_fio->FgetInt32_LE();	// 4 bytes dummy
	return true;
}
