/// ---------------------------------------------------------------------------
/// @file opna.h
///	OPN/A/B interface with ADPCM support
///	Copyright (C) cisc 1998, 2003.
/// ---------------------------------------------------------------------------
///	$Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include "../../depend.h"
#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"
#include "../vm_defs.h"

#define BUILD_OPN

#ifdef HAS_YM2608
#define BUILD_OPNA
#endif
#ifdef HAS_YM2610
#define BUILD_OPNB
#endif

/// ---------------------------------------------------------------------------
///	@class FM::OPN
///	@brief OPN/OPNA に良く似た音を生成する音源ユニット
///
///	interface:
///	bool Init(uint32_t clock, uint32_t rate, bool, const _TCHAR* path);
///		初期化．このクラスを使用する前にかならず呼んでおくこと．
///		OPNA の場合はこの関数でリズムサンプルを読み込む
///
///		clock:	OPN/OPNA/OPNB のクロック周波数(Hz)
///
///		rate:	生成する PCM の標本周波数(Hz)
///
///		path:	リズムサンプルのパス(OPNA のみ有効)
///				省略時はカレントディレクトリから読み込む
///				文字列の末尾には '\' や '/' などをつけること
///
///		返り値	初期化に成功すれば true
///
///	bool LoadRhythmSample(const _TCHAR* path)
///		(OPNA ONLY)
///		Rhythm サンプルを読み直す．
///		path は Init の path と同じ．
///
///	bool SetRate(uint32_t clock, uint32_t rate, bool)
///		クロックや PCM レートを変更する
///		引数等は Init を参照のこと．
///
///	void Mix(FM_SAMPLETYPE* dest, int nsamples)
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
///	uint32_t GetReg(uint32_t reg)
///		音源のレジスタ reg の内容を読み出す
///		読み込むことが出来るレジスタは PSG, ADPCM の一部，ID(0xff) とか
///
///	uint32_t ReadStatus()/ReadStatusEx()
///		音源のステータスレジスタを読み出す
///		ReadStatusEx は拡張ステータスレジスタの読み出し(OPNA)
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
///		タイマーが停止している場合は ULONG_MAX を返す… と思う
///
///	void SetVolumeFM(int db_l, int db_r)/SetVolumePSG(int db_l, int db_r) ...
///		各音源の音量を＋－方向に調節する．標準値は 0.
///		単位は約 1/2 dB，有効範囲の上限は 20 (10dB)
///
namespace FM
{
	// -----------------------------------------------------------------
	///	OPN Base
	class OPNBase : public Timer
	{
	public:
		OPNBase();
		virtual ~OPNBase() {}

		bool	Init(uint32_t c, uint32_t r);
		virtual void Reset();
		bool	ReadIRQ();

		void	SetVolumeFM(int db_l, int db_r);
		void	SetVolumePSG(int db_l, int db_r, int ptn);
		void	SetLPFCutoff(uint32_t freq) {}	// obsolete

	protected:
		void	SetParameter(Channel4* ch, uint32_t addr, uint32_t data);
		void	SetPrescaler(uint32_t p);
		void	RebuildTimeTable();
		void	Intr(bool value);

		void SaveState(void *f, size_t *size);
		bool LoadState(void *f);

		int		fmvolume_l;
#ifdef USE_FMGEN_STEREO
		int		fmvolume_r;
#endif

		uint32_t	clock;				// OPN クロック
		uint32_t	rate;				// FM 音源合成レート
		uint32_t	psgrate;			// FMGen  出力レート
		uint32_t	status;
		bool	interrupt;
		Channel4* csmch;


#if defined(BUILD_OPNA) || defined (BUILD_OPNB)
		static  uint32_t lfotable[8];
#endif

	private:
		void	TimerA();
		uint8_t	prescale;

	protected:
		Chip	chip;
		PSG		psg;
	};

#if defined(BUILD_OPNA) || defined(BUILD_OPNB)
	// -----------------------------------------------------------------
	///	OPN2 Base
	class OPNABase : public OPNBase
	{
	public:
		OPNABase();
		virtual ~OPNABase();

		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx();
		void	SetChannelMask(uint32_t mask);

	private:
		void	MakeTable2();

	protected:
		bool	Init(uint32_t c, uint32_t r, bool);
		bool	SetRate(uint32_t c, uint32_t r, bool);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		void	SetADPCMBReg(uint32_t reg, uint32_t data);
		uint32_t	GetReg(uint32_t addr);

		void SaveState(void *f, size_t *size);
		bool LoadState(void *f);

	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);

		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(Sample* dest, uint32_t count);

		void	WriteRAM(uint32_t data);
		uint32_t	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint32_t);

	// FM 音源関係
		uint8_t	pan[6];
		uint8_t	fnum2[9];

		uint8_t	reg22;
		uint32_t	reg29;		///< OPNA only?

		uint32_t	stmask;
		uint32_t	statusnext;

		uint32_t	lfocount;
		uint32_t	lfodcount;

		uint32_t	fnum[6];
		uint32_t	fnum3[3];

	// ADPCM 関係
		uint8_t*	adpcmbuf;		///< ADPCM RAM
		uint32_t	adpcmmask;		///< メモリアドレスに対するビットマスク
		uint32_t	adpcmnotice;	///< ADPCM 再生終了時にたつビット
		uint32_t	startaddr;		///< Start address
		uint32_t	stopaddr;		///< Stop address
		uint32_t	memaddr;		///< 再生中アドレス
		uint32_t	limitaddr;		///< Limit address/mask
		int		adpcmlevel;		///< ADPCM 音量
		int		adpcmvolume_l;
		int		adpcmvol_l;
		uint32_t	deltan;			///< ⊿N
		int		adplc;			///< 周波数変換用変数
		int		adpld;			///< 周波数変換用変数差分値
		uint32_t	adplbase;		///< adpld の元
		int		adpcmx;			///< ADPCM 合成用 x
		int		adpcmd;			///< ADPCM 合成用 ⊿
		int		adpcmout_l;		///< ADPCM 合成後の出力
		int		apout0_l;			///< out(t-2)+out(t-1)
		int		apout1_l;			///< out(t-1)+out(t)

#ifdef USE_FMGEN_STEREO
		int		adpcmvolume_r;
		int		adpcmvol_r;
		int		adpcmout_r;		///< ADPCM 合成後の出力
		int		apout0_r;			///< out(t-2)+out(t-1)
		int		apout1_r;			///< out(t-1)+out(t)
#endif

		uint32_t	adpcmreadbuf;	///< ADPCM リード用バッファ
		bool	adpcmplay;		///< ADPCM 再生中
		int8_t	granuality;
		bool	adpcmmask_;

		uint8_t	control1;		///< ADPCM コントロールレジスタ１
		uint8_t	control2;		///< ADPCM コントロールレジスタ２
		uint8_t	adpcmreg[8];	///< ADPCM レジスタの一部分

		int		rhythmmask_;

		Channel4 ch[6];

		static void	BuildLFOTable();
		static int amtable[FM_LFOENTS];
		static int pmtable[FM_LFOENTS];
		static int32_t tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};
#endif /* BUILD_OPNA || BUILD_OPNB */

	// -----------------------------------------------------------------
	///	YM2203(OPN)
	class OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}

		bool	Init(uint32_t c, uint32_t r, bool=false, const char* =0);
		bool	SetRate(uint32_t c, uint32_t r, bool=false);

		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx() { return 0xff; }

		void	SetChannelMask(uint32_t mask);

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		void SaveState(void *f, size_t *size);
		bool LoadState(void *f);

	private:
		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);

		uint32_t	fnum[3];
		uint32_t	fnum3[3];
		uint8_t	fnum2[6];

		Channel4 ch[3];
	};

#ifdef BUILD_OPNA
	// -----------------------------------------------------------------
	///	YM2608(OPNA)
	class OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();

		bool	Init(uint32_t c, uint32_t r, bool  = false, const _TCHAR* const* rhythmpaths=0);
		bool	LoadRhythmSample(const _TCHAR* const*);

		bool	SetRate(uint32_t c, uint32_t r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);

		void	SetVolumeADPCM(int db_l, int db_r);
		void	SetVolumeRhythmTotal(int db_l, int db_r);
		void	SetVolumeRhythm(int index, int db_l, int db_r);

		uint8_t*	GetADPCMBuffer() { return adpcmbuf; }

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		void SaveState(void *f, size_t *size);
		bool LoadState(void *f);

	private:
		struct Rhythm
		{
			uint8_t	pan;		///< ぱん
			int8_t	level;		///< おんりょう
			int		volume_l;		///< おんりょうせってい
#ifdef USE_FMGEN_STEREO
			int		volume_r;		///< おんりょうせってい
#endif
			int16_t*	sample;		///< さんぷる
			uint32_t	size;		///< さいず
			uint32_t	pos;		///< いち
			uint32_t	step;		///< すてっぷち
			uint32_t	rate;		///< さんぷるのれーと
		};

		void	RhythmMix(Sample* buffer, uint32_t count);

	// リズム音源関係
		bool    loaded_rhythm;
		Rhythm	rhythm[6];
		int8_t	rhythmtl;		///< リズム全体の音量
		int		rhythmtvol_l;
#ifdef USE_FMGEN_STEREO
		int		rhythmtvol_r;
#endif
		uint8_t	rhythmkey;		///< リズムのキー
	};
#endif /* BUILD_OPNA */

#ifdef BUILD_OPNB
	// -----------------------------------------------------------------
	///	YM2610/B(OPNB)
	class OPNB : public OPNABase
	{
	public:
		OPNB();
		virtual ~OPNB();

		bool	Init(uint32_t c, uint32_t r, bool = false,
					 uint8_t *_adpcma = 0, int _adpcma_size = 0,
					 uint8_t *_adpcmb = 0, int _adpcmb_size = 0);

		bool	SetRate(uint32_t c, uint32_t r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatusEx();

		void	SetVolumeADPCMATotal(int db_l, int db_r);
		void	SetVolumeADPCMA(int index, int db_l, int db_r);
		void	SetVolumeADPCMB(int db_l, int db_r);

//		void	SetChannelMask(uint32_t mask);

	private:
		struct ADPCMA
		{
			uint8_t	pan;		///< ぱん
			int8_t	level;		///< おんりょう
			int		volume_l;		///< おんりょうせってい
#ifdef USE_FMGEN_STEREO
			int		volume_r;		///< おんりょうせってい
#endif
			uint32_t	pos;		///< いち
			uint32_t	step;		///< すてっぷち

			uint32_t	start;		///< 開始
			uint32_t	stop;		///< 終了
			uint32_t	nibble;		///< 次の 4 bit
			int		adpcmx;		///< 変換用
			int		adpcmd;		///< 変換用
		};

		int		DecodeADPCMASample(uint32_t);
		void	ADPCMAMix(Sample* buffer, uint32_t count);
		static void InitADPCMATable();

	// ADPCMA 関係
		uint8_t*	adpcmabuf;		///< ADPCMA ROM
		int		adpcmasize;
		ADPCMA	adpcma[6];
		int8_t	adpcmatl;		///< ADPCMA 全体の音量
		int		adpcmatvol_l;
#ifdef USE_FMGEN_STEREO
		int		adpcmatvol_r;
#endif
		uint8_t	adpcmakey;		///< ADPCMA のキー
		int		adpcmastep;
		uint8_t	adpcmareg[32];

		static int jedi_table[(48+1)*16];

		Channel4 ch[6];
	};
#endif /* BUILD_OPNB */

#ifdef BUILD_OPN2
	// -----------------------------------------------------------------
	///	YM2612/3438(OPN2)
	class OPN2 : public OPNBase
	{
	public:
		OPN2();
		virtual ~OPN2() {}

		bool	Init(uint32_t c, uint32_t r, bool=false, const char* =0);
		bool	SetRate(uint32_t c, uint32_t r, bool);

		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx() { return 0xff; }

		void	SetChannelMask(uint32_t mask);

	private:
		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);

		uint32_t	fnum[3];
		uint32_t	fnum3[3];
		uint8_t	fnum2[6];

	// 線形補間用ワーク
		int32_t	mixc, mixc1;

		Channel4 ch[3];
	};
#endif /* BUILD_OPN2 */
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = -1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db_l, int db_r, int ptn)
{
	psg.SetVolume(db_l, db_r, ptn);
}

#endif // FM_OPNA_H
