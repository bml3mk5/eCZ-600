/// ---------------------------------------------------------------------------
/// @file opm.h
///	OPM-like Sound Generator
///	Copyright (C) cisc 1998, 2003.
/// ---------------------------------------------------------------------------
///	$Id: opm.h,v 1.14 2003/06/07 08:25:53 cisc Exp $

#ifndef FM_OPM_H
#define FM_OPM_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"
#include "../vm_defs.h"

#ifdef HAS_YM2151
#define BUILD_OPM
#endif

#ifdef BUILD_OPM
/// ---------------------------------------------------------------------------
///	@class FM::OPM
///	@brief OPM に良く似た(?)音を生成する音源ユニット
///
///	interface:
///	bool Init(uint32_t clock, uint32_t rate, bool);
///		初期化．このクラスを使用する前にかならず呼んでおくこと．
///		注意: 線形補完モードは廃止されました
///
///		clock:	OPM のクロック周波数(Hz)
///
///		rate:	生成する PCM の標本周波数(Hz)
///
///
///		返値	初期化に成功すれば true
///
///	bool SetRate(uint32_t clock, uint32_t rate, bool)
///		クロックや PCM レートを変更する
///		引数等は Init と同様．
///
///	void Mix(Sample* dest, int nsamples)
///		Stereo PCM データを nsamples 分合成し， dest で始まる配列に
///		加える(加算する)
///		・dest には sample*2 個分の領域が必要
///		・格納形式は L, R, L, R... となる．
///		・あくまで加算なので，あらかじめ配列をゼロクリアする必要がある
///		・FM_SAMPLETYPE が short 型の場合クリッピングが行われる.
///		・この関数は音源内部のタイマーとは独立している．
///		  Timer は Count と GetNextEvent で操作する必要がある．
///
///	void Reset()
///		音源をリセット(初期化)する
///
///	void SetReg(uint32_t reg, uint32_t data)
///		音源のレジスタ reg に data を書き込む
///
///	uint32_t ReadStatus()
///		音源のステータスレジスタを読み出す
///		busy フラグは常に 0
///
///	bool ReadIRQ()
///		IRQ 出力を読み出す
///
///	bool Count(uint32_t t)
///		音源のタイマーを t [clock] 進める．
///		音源の内部状態に変化があった時(timer オーバーフロー)
///		true を返す
///
///	uint32_t GetNextEvent()
///		音源のタイマーのどちらかがオーバーフローするまでに必要な
///		時間[clock]を返す
///		タイマーが停止している場合は 0 を返す．
///
///	void SetVolume(int db_l, int db_r)
///		各音源の音量を＋－方向に調節する．標準値は 0.
///		単位は約 1/2 dB，有効範囲の上限は 20 (10dB)
///
namespace FM
{
	// ---------------------------------------------------------------
	///	YM2151(OPM)
	class OPM : public Timer
	{
	public:
		OPM();
		~OPM() {}

		bool	Init(uint32_t c, uint32_t r, bool=false);
		bool	SetRate(uint32_t c, uint32_t r, bool);
		void	SetLPFCutoff(uint32_t freq);
		void	Reset();
		bool	ReadIRQ();

		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }

		void 	Mix(Sample* buffer, int nsamples);

		void	SetVolume(int db_l, int db_r);
		void	SetChannelMask(uint32_t mask);

		void SaveState(void *f, size_t *size);
		bool LoadState(void *f);

	private:
		void	Intr(bool value);

	private:
		enum
		{
			OPM_LFOENTS = 512,
		};

		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		void	SetParameter(uint32_t addr, uint32_t data);
		void	TimerA();
		void	RebuildTimeTable();
		void	MixSub(int activech, ISample**);
		void	MixSubL(int activech, ISample**);
		void	LFO();
		uint32_t	Noise();

		int		fmvolume_l;
#ifdef USE_FMGEN_STEREO
		int		fmvolume_r;
#endif

		uint32_t	clock;
		uint32_t	rate;
		uint32_t	pcmrate;

		uint32_t	pmd;
		uint32_t	amd;
		uint32_t	lfocount;
		uint32_t	lfodcount;

		uint32_t	lfo_count_;
		uint32_t	lfo_count_diff_;
		uint32_t	lfo_step_;
		uint32_t	lfo_count_prev_;

		uint32_t	lfowaveform;
		uint32_t	rateratio;
		uint32_t	noise;
		int32_t		noisecount;
		uint32_t	noisedelta;

		bool	interpolation;
		uint8_t	lfofreq;
		uint8_t	status;
		bool	interrupt;
		uint8_t	reg01;

		uint8_t	kc[8];
		uint8_t	kf[8];
		uint8_t	pan[8];

		Channel4 ch[8];
		Chip	chip;

		static void	BuildLFOTable();
		static int amtable[4][OPM_LFOENTS];
		static int pmtable[4][OPM_LFOENTS];

	public:
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

	};
}
#endif /* BUILD_OPM */

#endif // FM_OPM_H
