/// ---------------------------------------------------------------------------
/// @file opna.cpp
///	OPN/A/B interface with ADPCM support
///	Copyright (C) cisc 1998, 2001.
/// ---------------------------------------------------------------------------
///	$Id: opna.cpp,v 1.70 2004/02/06 13:13:39 cisc Exp $

#include "headers.h"
#include "misc.h"
#include "opna.h"
#include "fmgeninl.h"

#include "../../fileio.h"
#include "../../logging.h"


///	TOFIX:
///	 OPN ch3 が常にPrepareの対象となってしまう障害


// ---------------------------------------------------------------------------
///	OPNA: ADPCM データの格納方式の違い (8bit/1bit) をエミュレートしない
///	このオプションを有効にすると ADPCM メモリへのアクセス(特に 8bit モード)が
///	多少軽くなるかも
///
//#define NO_BITTYPE_EMULATION

//#ifdef BUILD_OPNA
//#include "file.h"
//#endif

namespace FM
{

// ---------------------------------------------------------------------------
///	OPNBase

#if defined(BUILD_OPNA) || defined (BUILD_OPNB)
uint32_t	OPNBase::lfotable[8];			// OPNA/B 用
#endif

OPNBase::OPNBase()
{
	prescale = 0;
}

///	パラメータセット
void OPNBase::SetParameter(Channel4* ch, uint32_t addr, uint32_t data)
{
	const static uint32_t slottable[4] = { 0, 2, 1, 3 };
	const static uint8_t sltable[16] =
	{
		  0,   4,   8,  12,  16,  20,  24,  28,
		 32,  36,  40,  44,  48,  52,  56, 124,
	};

	if ((addr & 3) < 3)
	{
		uint32_t slot = slottable[(addr >> 2) & 3];
		Operator* op = &ch->op[slot];

		switch ((addr >> 4) & 15)
		{
		case 3:	// 30-3E DT/MULTI
			op->SetDT((data >> 4) & 0x07);
			op->SetMULTI(data & 0x0f);
			break;

		case 4: // 40-4E TL
			op->SetTL(data & 0x7f, ((regtc & 0xc0) == 0x80) && (csmch == ch));
			break;

		case 5: // 50-5E KS/AR
			op->SetKS((data >> 6) & 3);
			op->SetAR((data & 0x1f) * 2);
			break;

		case 6: // 60-6E DR/AMON
			op->SetDR((data & 0x1f) * 2);
			op->SetAMON((data & 0x80) != 0);
			break;

		case 7: // 70-7E SR
			op->SetSR((data & 0x1f) * 2);
			break;

		case 8:	// 80-8E SL/RR
			op->SetSL(sltable[(data >> 4) & 15]);
			op->SetRR((data & 0x0f) * 4 + 2);
			break;

		case 9: // 90-9E SSG-EC
			op->SetSSGEC(data & 0x0f);
			break;
		}
	}
}

///	リセット
void OPNBase::Reset()
{
	status = 0;
	interrupt = false;
	SetPrescaler(0);
	Timer::Reset();
	psg.Reset();
}

///	割り込み信号の取得
bool OPNBase::ReadIRQ()
{
	return interrupt;
}

///	プリスケーラ設定
void OPNBase::SetPrescaler(uint32_t p)
{
	static const char table[3][2] = { { 6, 4 }, { 3, 2 }, { 2, 1 } };
#if defined(BUILD_OPNA) || defined (BUILD_OPNB)
	static const uint8_t table2[8] = { 108,  77,  71,  67,  62,  44,  8,  5 };
#endif
	// 512
	if (prescale != p)
	{
		prescale = p;
		assert(0 <= prescale && prescale < 3);

		uint32_t fmclock = clock / table[p][0] / 12;

		rate = psgrate;

		// 合成周波数と出力周波数の比
		assert(fmclock < (0x80000000 >> FM_RATIOBITS));
		uint32_t ratio = ((fmclock << FM_RATIOBITS) + rate/2) / rate;

		SetTimerPrescaler(table[p][0] * 12);
//		MakeTimeTable(ratio);
		chip.SetRatio(ratio);
		psg.SetClock(clock / table[p][1], psgrate);

#if defined(BUILD_OPNA) || defined (BUILD_OPNB)
		for (int i=0; i<8; i++)
		{
			lfotable[i] = (ratio << (2+FM_LFOCBITS-FM_RATIOBITS)) / table2[i];
		}
#endif
	}
}

///	初期化
bool OPNBase::Init(uint32_t c, uint32_t r)
{
	clock = c;
	psgrate = r;

	return true;
}

///	音量設定
void OPNBase::SetVolumeFM(int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	if (db_l > -192)
		fmvolume_l = int(16384.0 * pow(10.0, db_l / 40.0));
	else
		fmvolume_l = 0;

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	if (db_r > -192)
		fmvolume_r = int(16384.0 * pow(10.0, db_r / 40.0));
	else
		fmvolume_r = 0;
#endif
}

///	タイマー時間処理
void OPNBase::TimerA()
{
	if ((regtc & 0xc0) == 0x80)
	{
		csmch->KeyControl(0x00);
		csmch->KeyControl(0x0f);
	}
}

///	割り込み信号の設定
void OPNBase::Intr(bool value)
{
	interrupt = value;
}

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define OPN_BASE_STATE_VERSION	2

///	ステートセーブ
void OPNBase::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(OPN_BASE_STATE_VERSION);	// 4 bytes

	sz += state_fio->FputInt32_LE(fmvolume_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(fmvolume_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(fmvolume_l);	// 4 bytes
#endif
	sz += state_fio->FputUint32_LE(clock);	// 4 bytes

	sz += state_fio->FputUint32_LE(rate);	// 4 bytes
	sz += state_fio->FputUint32_LE(psgrate);	// 4 bytes
	sz += state_fio->FputUint32_LE(status);	// 4 bytes
	sz += state_fio->FputBool(interrupt);	// 1 byte
	sz += state_fio->FputUint8(prescale);	// 1 byte
	sz += state_fio->FputInt8(0);	// 1 byte dummy
	sz += state_fio->FputInt8(0);	// 1 byte dummy

	Timer::SaveState(f, size);
	chip.SaveState(f, size);
	psg.SaveState(f, size);

	if (size) *size += sz; // 32	// 16 bytes bound
}

///	ステートロード
bool OPNBase::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != OPN_BASE_STATE_VERSION) {
		return false;
	}
	fmvolume_l = state_fio->FgetInt32_LE();	// 4 bytes
#ifdef USE_FMGEN_STEREO
	fmvolume_r = state_fio->FgetInt32_LE();	// 4 bytes
#else
	state_fio->FgetInt32_LE();	// 4 bytes skip
#endif
	clock = state_fio->FgetUint32_LE();	// 4 bytes
	rate = state_fio->FgetUint32_LE();	// 4 bytes
	psgrate = state_fio->FgetUint32_LE();	// 4 bytes
	status = state_fio->FgetUint32_LE();	// 4 bytes
	interrupt = state_fio->FgetBool();	// 1 byte
	prescale = state_fio->FgetUint8();	// 1 byte
	state_fio->FgetInt8();	// 1 byte dummy
	state_fio->FgetInt8();	// 1 byte dummy

	if(!Timer::LoadState(f)) {
		return false;
	}
	if(!chip.LoadState(f)) {
		return false;
	}
	if(!psg.LoadState(f)) {
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
//	YM2203
//
#ifdef BUILD_OPN

OPN::OPN()
{
	SetVolumeFM(0, 0);
	SetVolumePSG(0, 0, 0);

	csmch = &ch[2];

	for (int i=0; i<3; i++)
	{
		ch[i].SetChip(&chip);
		ch[i].SetType(typeN);
	}
}

///	初期化
bool OPN::Init(uint32_t c, uint32_t r, bool ip, const char*)
{
	if (!SetRate(c, r, ip))
		return false;

	Reset();

	SetVolumeFM(0, 0);
	SetVolumePSG(0, 0, 0);
	SetChannelMask(0);
	return true;
}

///	サンプリングレート変更
bool OPN::SetRate(uint32_t c, uint32_t r, bool)
{
	OPNBase::Init(c, r);
	RebuildTimeTable();
	return true;
}


///	リセット
void OPN::Reset()
{
	int i;
	for (i=0x20; i<0x28; i++) SetReg(i, 0);
	for (i=0x30; i<0xc0; i++) SetReg(i, 0);
	OPNBase::Reset();
	ch[0].Reset();
	ch[1].Reset();
	ch[2].Reset();
}


///	レジスタ読み込み
uint32_t OPN::GetReg(uint32_t addr)
{
	if (addr < 0x10)
		return psg.GetReg(addr);
	else
		return 0;
}


///	レジスタアレイにデータを設定
void OPN::SetReg(uint32_t addr, uint32_t data)
{
//	LOG2("reg[%.2x] <- %.2x\n", addr, data);
	if (addr >= 0x100)
		return;

	int c = addr & 3;
	switch (addr)
	{
	case  0: case  1: case  2: case  3: case  4: case  5: case  6: case  7:
	case  8: case  9: case 10: case 11: case 12: case 13: case 14: case 15:
		psg.SetReg(addr, data);
		break;

	case 0x24: case 0x25:
		SetTimerA(addr, data);
		break;

	case 0x26:
		SetTimerB(data);
		break;

	case 0x27:
		SetTimerControl(data);
		break;

	case 0x28:		// Key On/Off
		if ((data & 3) < 3)
			ch[data & 3].KeyControl(data >> 4);
		break;

	case 0x2d: case 0x2e: case 0x2f:
		SetPrescaler(addr-0x2d);
		break;

	// F-Number
	case 0xa0: case 0xa1: case 0xa2:
		fnum[c] = data + fnum2[c] * 0x100;
		break;

	case 0xa4: case 0xa5: case 0xa6:
		fnum2[c] = uint8_t(data);
		break;

	case 0xa8: case 0xa9: case 0xaa:
		fnum3[c] = data + fnum2[c+3] * 0x100;
		break;

	case 0xac: case 0xad: case 0xae:
		fnum2[c+3] = uint8_t(data);
		break;

	case 0xb0:	case 0xb1:  case 0xb2:
		ch[c].SetFB((data >> 3) & 7);
		ch[c].SetAlgorithm(data & 7);
		break;

	default:
		if (c < 3)
		{
			if ((addr & 0xf0) == 0x60)
				data &= 0x1f;
			OPNBase::SetParameter(&ch[c], addr, data);
		}
		break;
	}
}

///	ステータスフラグ設定
void OPN::SetStatus(uint32_t bits)
{
	if (!(status & bits))
	{
		status |= bits;
		Intr(true);
	}
}

void OPN::ResetStatus(uint32_t bit)
{
	status &= ~bit;
	if (!status)
		Intr(false);
}

///	マスク設定
void OPN::SetChannelMask(uint32_t mask)
{
	for (int i=0; i<3; i++)
		ch[i].Mute(!!(mask & (1 << i)));
	psg.SetChannelMask(mask >> 6);
}


///	合成(2ch)
///	@param[in,out] buffer:		合成先
///	@param[in] nsamples:	合成サンプル数
///
void OPN::Mix(Sample* buffer, int nsamples)
{
#define IStoSampleL(s)	((Limit(s, 0x7fff, -0x8000) * fmvolume_l) >> 14)
#ifdef USE_FMGEN_STEREO
#define IStoSampleR(s)	((Limit(s, 0x7fff, -0x8000) * fmvolume_r) >> 14)
#endif

	psg.Mix(buffer, nsamples);

	// skip mixing FM wave if PSG only
	if (fmvolume_l == 0
#ifdef USE_FMGEN_STEREO
		&& fmvolume_r == 0
#endif
	) return;

	// Set F-Number
	ch[0].SetFNum(fnum[0]);
	ch[1].SetFNum(fnum[1]);
	if (!(regtc & 0xc0))
		ch[2].SetFNum(fnum[2]);
	else
	{	// 効果音
		ch[2].op[0].SetFNum(fnum3[1]);
		ch[2].op[1].SetFNum(fnum3[2]);
		ch[2].op[2].SetFNum(fnum3[0]);
		ch[2].op[3].SetFNum(fnum[2]);
	}

	int actch = (((ch[2].Prepare() << 2) | ch[1].Prepare()) << 2) | ch[0].Prepare();
	if (actch & 0x15)
	{
		Sample* limit = buffer + nsamples * 2;
		for (Sample* dest = buffer; dest < limit; dest+=2)
		{
			ISample s = 0;
			ISample s_l;
#ifdef USE_FMGEN_STEREO
			ISample s_r;
#endif
			if (actch & 0x01) s  = ch[0].Calc();
			if (actch & 0x04) s += ch[1].Calc();
			if (actch & 0x10) s += ch[2].Calc();
			s_l = IStoSampleL(s);
			StoreSample(dest[0], s_l);
#ifdef USE_FMGEN_STEREO
			s_r = IStoSampleR(s);
			StoreSample(dest[1], s_r);
#else
			StoreSample(dest[1], s_l);
#endif
		}
	}
#undef IStoSampleL
#ifdef USE_FMGEN_STEREO
#undef IStoSampleR
#endif
}

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define OPN_STATE_VERSION	1

///	ステートセーブ
void OPN::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(OPN_STATE_VERSION);	// 4 bytes

	for(int i = 0; i < 3; i++) {		// 12 bytes
		sz += state_fio->FputUint32_LE(fnum[i]);
	}
	for(int i = 0; i < 3; i++) {		// 12 bytes
		sz += state_fio->FputUint32_LE(fnum3[i]);
	}
	for(int i = 0; i < 6; i++) {		// 6 bytes
		sz += state_fio->FputUint8(fnum2[i]);
	}
	for(int i = 0; i < 14; i++) {		// 14 bytes dummy
		sz += state_fio->FputInt8(0);
	}
	for(int i = 0; i < 3; i++) {
		ch[i].SaveState(f, size);
	}

	OPNBase::SaveState(f, size);

	if (size) *size += sz;	// 48	// 16 bytes bound
}

///	ステートロード
bool OPN::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != OPN_STATE_VERSION) {
		return false;
	}
	for(int i = 0; i < 3; i++) {		// 12 bytes
		fnum[i] = state_fio->FgetUint32_LE();
	}
	for(int i = 0; i < 3; i++) {		// 12 bytes
		fnum3[i] = state_fio->FgetUint32_LE();
	}
	for(int i = 0; i < 6; i++) {		// 6 bytes
		fnum2[i] = state_fio->FgetUint8();
	}
	for(int i = 0; i < 14; i++) {		// 14 bytes dummy
		state_fio->FgetInt8();
	}
	for(int i = 0; i < 3; i++) {
		if(!ch[i].LoadState(f)) {
			return false;
		}
	}
	if(!OPNBase::LoadState(f)) {
		return false;
	}
	return true;
}

#endif // BUILD_OPN

// ---------------------------------------------------------------------------
//	YM2608/2610 common part
// ---------------------------------------------------------------------------

#if defined(BUILD_OPNA) || defined(BUILD_OPNB)

int OPNABase::amtable[FM_LFOENTS] = { -1, };
int OPNABase::pmtable[FM_LFOENTS];

int32_t OPNABase::tltable[FM_TLENTS+FM_TLPOS];
bool OPNABase::tablehasmade = false;

OPNABase::OPNABase()
{
	adpcmbuf = 0;
	memaddr = 0;
	startaddr = 0;
	deltan = 256;

	adpcmvol_l = 0;
#ifdef USE_FMGEN_STEREO
	adpcmvol_r = 0;
#endif
	control2 = 0;

	MakeTable2();
	BuildLFOTable();
	for (int i=0; i<6; i++)
	{
		ch[i].SetChip(&chip);
		ch[i].SetType(typeN);
	}
}

OPNABase::~OPNABase()
{
}

// ---------------------------------------------------------------------------
///	初期化
///
bool OPNABase::Init(uint32_t c, uint32_t r, bool)
{
	RebuildTimeTable();

	Reset();

	SetVolumeFM(0, 0);
	SetVolumePSG(0, 0, 0);
	SetChannelMask(0);
	return true;
}

// ---------------------------------------------------------------------------
///	テーブル作成
///
void OPNABase::MakeTable2()
{
	if (!tablehasmade)
	{
		for (int i=-FM_TLPOS; i<FM_TLENTS; i++)
		{
			tltable[i+FM_TLPOS] = uint32_t(65536. * pow(2.0, i * -16. / FM_TLENTS))-1;
		}

		tablehasmade = true;
	}
}

// ---------------------------------------------------------------------------
///	リセット
///
void OPNABase::Reset()
{
	int i;

	OPNBase::Reset();
	for (i=0x20; i<0x28; i++) SetReg(i, 0);
	for (i=0x30; i<0xc0; i++) SetReg(i, 0);
	for (i=0x130; i<0x1c0; i++) SetReg(i, 0);
	for (i=0x100; i<0x110; i++) SetReg(i, 0);
	for (i=0x10; i<0x20; i++) SetReg(i, 0);
	for (i=0; i<6; i++)
	{
		pan[i] = 3;
		ch[i].Reset();
	}

	stmask = ~0x1c;
	statusnext = 0;
	memaddr = 0;
	adpcmlevel = 0;
	adpcmd = 127;
	adpcmx = 0;
	adpcmreadbuf = 0;
	apout0_l = apout1_l = adpcmout_l = 0;
#ifdef USE_FMGEN_STEREO
	apout0_r = apout1_r = adpcmout_r = 0;
#endif
	lfocount = 0;
	adpcmplay = false;
	adplc = 0;
	adpld = 0x100;
	status = 0;
	UpdateStatus();
}

// ---------------------------------------------------------------------------
///	サンプリングレート変更
///
bool OPNABase::SetRate(uint32_t c, uint32_t r, bool)
{
	c /= 2;		// 従来版との互換性を重視したけりゃコメントアウトしよう

	OPNBase::Init(c, r);

	adplbase = int(8192. * (clock/72.) / r);
	adpld = deltan * adplbase >> 16;

	RebuildTimeTable();

	lfodcount = reg22 & 0x08 ? lfotable[reg22 & 7] : 0;
	return true;
}


// ---------------------------------------------------------------------------
///	チャンネルマスクの設定
///
void OPNABase::SetChannelMask(uint32_t mask)
{
	for (int i=0; i<6; i++)
		ch[i].Mute(!!(mask & (1 << i)));
	psg.SetChannelMask(mask >> 6);
	adpcmmask_ = (mask & (1 << 9)) != 0;
	rhythmmask_ = (mask >> 10) & ((1 << 6) - 1);
}

// ---------------------------------------------------------------------------
///	レジスタアレイにデータを設定
///
void OPNABase::SetReg(uint32_t addr, uint32_t data)
{
	int	c = addr & 3;
	switch (addr)
	{
		uint32_t modified;

	// Timer -----------------------------------------------------------------
		case 0x24: case 0x25:
			SetTimerA(addr, data);
			break;

		case 0x26:
			SetTimerB(data);
			break;

		case 0x27:
			SetTimerControl(data);
			break;

	// Misc ------------------------------------------------------------------
	case 0x28:		// Key On/Off
		if ((data & 3) < 3)
		{
			c = (data & 3) + (data & 4 ? 3 : 0);
			ch[c].KeyControl(data >> 4);
		}
		break;

	// Status Mask -----------------------------------------------------------
	case 0x29:
		reg29 = data;
//		UpdateStatus(); //?
		break;

	// Prescaler -------------------------------------------------------------
	case 0x2d: case 0x2e: case 0x2f:
		SetPrescaler(addr-0x2d);
		break;

	// F-Number --------------------------------------------------------------
	case 0x1a0:	case 0x1a1: case 0x1a2:
		c += 3;
	case 0xa0:	case 0xa1: case 0xa2:
		fnum[c] = data + fnum2[c] * 0x100;
		ch[c].SetFNum(fnum[c]);
		break;

	case 0x1a4:	case 0x1a5: case 0x1a6:
		c += 3;
	case 0xa4 : case 0xa5: case 0xa6:
		fnum2[c] = uint8_t(data);
		break;

	case 0xa8:	case 0xa9: case 0xaa:
		fnum3[c] = data + fnum2[c+6] * 0x100;
		break;

	case 0xac : case 0xad: case 0xae:
		fnum2[c+6] = uint8_t(data);
		break;

	// Algorithm -------------------------------------------------------------

	case 0x1b0:	case 0x1b1:  case 0x1b2:
		c += 3;
	case 0xb0:	case 0xb1:  case 0xb2:
		ch[c].SetFB((data >> 3) & 7);
		ch[c].SetAlgorithm(data & 7);
		break;

	case 0x1b4: case 0x1b5: case 0x1b6:
		c += 3;
	case 0xb4: case 0xb5: case 0xb6:
		pan[c] = (data >> 6) & 3;
		ch[c].SetMS(data);
		break;

	// LFO -------------------------------------------------------------------
	case 0x22:
		modified = reg22 ^ data;
		reg22 = data;
		if (modified & 0x8)
			lfocount = 0;
		lfodcount = reg22 & 8 ? lfotable[reg22 & 7] : 0;
		break;

	// PSG -------------------------------------------------------------------
	case  0: case  1: case  2: case  3: case  4: case  5: case  6: case  7:
	case  8: case  9: case 10: case 11: case 12: case 13: case 14: case 15:
		psg.SetReg(addr, data);
		break;

	// 音色 ------------------------------------------------------------------
	default:
		if (c < 3)
		{
			if (addr & 0x100)
				c += 3;
			OPNBase::SetParameter(&ch[c], addr, data);
		}
		break;
	}
}

// ---------------------------------------------------------------------------
///	ADPCM B
///
void OPNABase::SetADPCMBReg(uint32_t addr, uint32_t data)
{
	switch (addr)
	{
	case 0x00:		// Control Register 1
		if ((data & 0x80) && !adpcmplay)
		{
			adpcmplay = true;
			memaddr = startaddr;
			adpcmx = 0, adpcmd = 127;
			adplc = 0;
		}
		if (data & 1)
		{
			adpcmplay = false;
		}
		control1 = data;
		break;

	case 0x01:		// Control Register 2
		control2 = data;
		granuality = control2 & 2 ? 1 : 4;
		break;

	case 0x02:		// Start Address L
	case 0x03:		// Start Address H
		adpcmreg[addr - 0x02 + 0] = data;
		startaddr = (adpcmreg[1]*256+adpcmreg[0]) << 6;
		memaddr = startaddr;
//		LOG1("  startaddr %.6x", startaddr);
		break;

	case 0x04:		// Stop Address L
	case 0x05:		// Stop Address H
		adpcmreg[addr - 0x04 + 2] = data;
		stopaddr = (adpcmreg[3]*256+adpcmreg[2] + 1) << 6;
//		LOG1("  stopaddr %.6x", stopaddr);
		break;

	case 0x08:		// ADPCM data
		if ((control1 & 0x60) == 0x60)
		{
//			LOG2("  Wr [0x%.5x] = %.2x", memaddr, data);
			WriteRAM(data);
		}
		break;

	case 0x09:		// delta-N L
	case 0x0a:		// delta-N H
		adpcmreg[addr - 0x09 + 4] = data;
		deltan = adpcmreg[5]*256+adpcmreg[4];
		deltan = Max(256, deltan);
		adpld = deltan * adplbase >> 16;
		break;

	case 0x0b:		// Level Control
		adpcmlevel = data; 
		adpcmvolume_l = (adpcmvol_l * adpcmlevel) >> 12;
#ifdef USE_FMGEN_STEREO
		adpcmvolume_r = (adpcmvol_r * adpcmlevel) >> 12;
#endif
		break;

	case 0x0c:		// Limit Address L
	case 0x0d:		// Limit Address H
		adpcmreg[addr - 0x0c + 6] = data;
		limitaddr = (adpcmreg[7]*256+adpcmreg[6] + 1) << 6;
//		LOG1("  limitaddr %.6x", limitaddr);
		break;

	case 0x10:		// Flag Control
		if (data & 0x80)
		{
			// for Firecracker Music collection (Hi-speed PCM loader)
			status &= 0x03;
			UpdateStatus();
		}
		else
		{
			stmask = ~(data & 0x1f);
//			UpdateStatus();					//???
		}
		break;
	}
}


// ---------------------------------------------------------------------------
///	レジスタ取得
///
uint32_t OPNA::GetReg(uint32_t addr)
{
	if (addr < 0x10)
		return psg.GetReg(addr);

	if (addr == 0x108)
	{
//		LOG1("%d:reg[108] ->   ", Diag::GetCPUTick());

		uint32_t data = adpcmreadbuf & 0xff;
		adpcmreadbuf >>= 8;
		if ((control1 & 0x60) == 0x20)
		{
			adpcmreadbuf |= ReadRAM() << 8;
//			LOG2("Rd [0x%.6x:%.2x] ", memaddr, adpcmreadbuf >> 8);
		}
//		LOG0("%.2x\n");
		return data;
	}

	if (addr == 0xff)
		return 1;

	return 0;
}




// ---------------------------------------------------------------------------
///	ステータスフラグ設定
///
void OPNABase::SetStatus(uint32_t bits)
{
	if (!(status & bits))
	{
//		LOG2("SetStatus(%.2x %.2x)\n", bits, stmask);
		status |= bits & stmask;
		UpdateStatus();
	}
//	else
//		LOG1("SetStatus(%.2x) - ignored\n", bits);
}

void OPNABase::ResetStatus(uint32_t bits)
{
	status &= ~bits;
//	LOG1("ResetStatus(%.2x)\n", bits);
	UpdateStatus();
}

inline void OPNABase::UpdateStatus()
{
//	LOG2("%d:INT = %d\n", Diag::GetCPUTick(), (status & stmask & reg29) != 0);
	Intr((status & stmask & reg29) != 0);
}

// ---------------------------------------------------------------------------
///	ADPCM RAM への書込み操作
///
void OPNABase::WriteRAM(uint32_t data)
{
#ifndef NO_BITTYPE_EMULATION
	if (!(control2 & 2))
	{
		// 1 bit mode
		adpcmbuf[(memaddr >> 4) & 0x3ffff] = data;
		memaddr += 16;
	}
	else
	{
		// 8 bit mode
		uint8_t* p = &adpcmbuf[(memaddr >> 4) & 0x7fff];
		uint32_t bank = (memaddr >> 1) & 7;
		uint8_t mask = 1 << bank;
		data <<= bank;

		p[0x00000] = (p[0x00000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x08000] = (p[0x08000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x10000] = (p[0x10000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x18000] = (p[0x18000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x20000] = (p[0x20000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x28000] = (p[0x28000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x30000] = (p[0x30000] & ~mask) | (uint8_t(data) & mask); data >>= 1;
		p[0x38000] = (p[0x38000] & ~mask) | (uint8_t(data) & mask);
		memaddr += 2;
	}
#else
	adpcmbuf[(memaddr >> granuality) & 0x3ffff] = data;
	memaddr += 1 << granuality;
#endif

	if (memaddr == stopaddr)
	{
		SetStatus(4);
		statusnext = 0x04;	// EOS
		memaddr &= 0x3fffff;
	}
	if (memaddr == limitaddr)
	{
//		LOG1("Limit ! (%.8x)\n", limitaddr);
		memaddr = 0;
	}
	SetStatus(8);
}

// ---------------------------------------------------------------------------
///	ADPCM RAM からの読み込み操作
///
uint32_t OPNABase::ReadRAM()
{
	uint32_t data;
#ifndef NO_BITTYPE_EMULATION
	if (!(control2 & 2))
	{
		// 1 bit mode
		data = adpcmbuf[(memaddr >> 4) & 0x3ffff];
		memaddr += 16;
	}
	else
	{
		// 8 bit mode
		uint8_t* p = &adpcmbuf[(memaddr >> 4) & 0x7fff];
		uint32_t bank = (memaddr >> 1) & 7;
		uint8_t mask = 1 << bank;

		data =            (p[0x38000] & mask);
		data = data * 2 + (p[0x30000] & mask);
		data = data * 2 + (p[0x28000] & mask);
		data = data * 2 + (p[0x20000] & mask);
		data = data * 2 + (p[0x18000] & mask);
		data = data * 2 + (p[0x10000] & mask);
		data = data * 2 + (p[0x08000] & mask);
		data = data * 2 + (p[0x00000] & mask);
		data >>= bank;
		memaddr += 2;
	}
#else
	data = adpcmbuf[(memaddr >> granuality) & 0x3ffff];
	memaddr += 1 << granuality;
#endif
	if (memaddr == stopaddr)
	{
		SetStatus(4);
		statusnext = 0x04;	// EOS
		memaddr &= 0x3fffff;
	}
	if (memaddr == limitaddr)
	{
//		LOG1("Limit ! (%.8x)\n", limitaddr);
		memaddr = 0;
	}
	if (memaddr < stopaddr)
		SetStatus(8);
	return data;
}


inline int OPNABase::DecodeADPCMBSample(uint32_t data)
{
	static const int table1[16] =
	{
		  1,   3,   5,   7,   9,  11,  13,  15,
		 -1,  -3,  -5,  -7,  -9, -11, -13, -15,
	};
	static const int table2[16] =
	{
		 57,  57,  57,  57,  77, 102, 128, 153,
		 57,  57,  57,  57,  77, 102, 128, 153,
	};
	adpcmx = Limit(adpcmx + table1[data] * adpcmd / 8, 32767, -32768);
	adpcmd = Limit(adpcmd * table2[data] / 64, 24576, 127);
	return adpcmx;
}


// ---------------------------------------------------------------------------
///	ADPCM RAM からの nibble 読み込み及び ADPCM 展開
///
int OPNABase::ReadRAMN()
{
	uint32_t data;
	if (granuality > 0)
	{
#ifndef NO_BITTYPE_EMULATION
		if (!(control2 & 2))
		{
			data = adpcmbuf[(memaddr >> 4) & 0x3ffff];
			memaddr += 8;
			if (memaddr & 8)
				return DecodeADPCMBSample(data >> 4);
			data &= 0x0f;
		}
		else
		{
			uint8_t* p = &adpcmbuf[(memaddr >> 4) & 0x7fff] + ((~memaddr & 1) << 17);
			uint32_t bank = (memaddr >> 1) & 7;
			uint8_t mask = 1 << bank;

			data =            (p[0x18000] & mask);
			data = data * 2 + (p[0x10000] & mask);
			data = data * 2 + (p[0x08000] & mask);
			data = data * 2 + (p[0x00000] & mask);
			data >>= bank;
			memaddr ++;
			if (memaddr & 1)
				return DecodeADPCMBSample(data);
		}
#else
		data = adpcmbuf[(memaddr >> granuality) & adpcmmask];
		memaddr += 1 << (granuality-1);
		if (memaddr & (1 << (granuality-1)))
			return DecodeADPCMBSample(data >> 4);
		data &= 0x0f;
#endif
	}
	else
	{
		data = adpcmbuf[(memaddr >> 1) & adpcmmask];
		++memaddr;
		if (memaddr & 1)
			return DecodeADPCMBSample(data >> 4);
		data &= 0x0f;
	}

	DecodeADPCMBSample(data);

	// check
	if (memaddr == stopaddr)
	{
		if (control1 & 0x10)
		{
			memaddr = startaddr;
			data = adpcmx;
			adpcmx = 0, adpcmd = 127;
			// for PC-8801FA/MA shop demonstration
			SetStatus(adpcmnotice);
			return data;
		}
		else
		{
			memaddr &= adpcmmask;	//0x3fffff;
			SetStatus(adpcmnotice);
			adpcmplay = false;
		}
	}

	if (memaddr == limitaddr)
		memaddr = 0;

	return adpcmx;
}

// ---------------------------------------------------------------------------
///	拡張ステータスを読みこむ
///
uint32_t OPNABase::ReadStatusEx()
{
	uint32_t r = ((status | 8) & stmask) | (adpcmplay ? 0x20 : 0);
	status |= statusnext;
	statusnext = 0;
	return r;
}

// ---------------------------------------------------------------------------
///	ADPCM 展開
///
inline void OPNABase::DecodeADPCMB()
{
	apout0_l = apout1_l;
#ifdef USE_FMGEN_STEREO
	apout0_r = apout1_r;
#endif
	int ram = ReadRAMN();
	int s_l = (ram * adpcmvolume_l) >> 13;
	apout1_l = adpcmout_l + s_l;
	adpcmout_l = s_l;
#ifdef USE_FMGEN_STEREO
	int s_r = (ram * adpcmvolume_r) >> 13;
	apout1_r = adpcmout_r + s_r;
	adpcmout_r = s_r;
#endif
}

// ---------------------------------------------------------------------------
///	ADPCM 合成
///
void OPNABase::ADPCMBMix(Sample* dest, uint32_t count)
{
	uint32_t mask_l = control2 & 0x80 ? -1 : 0;
#ifdef USE_FMGEN_STEREO
	uint32_t mask_r = control2 & 0x40 ? -1 : 0;
#endif
	if (adpcmmask_)
	{
		mask_l = 0;
#ifdef USE_FMGEN_STEREO
		mask_r = 0;
#endif
	}

	if (adpcmplay)
	{
//		LOG2("ADPCM Play: %d   DeltaN: %d\n", adpld, deltan);
		if (adpld <= 8192)		// fplay < fsamp
		{
			for (; count>0; count--)
			{
				if (adplc < 0)
				{
					adplc += 8192;
					DecodeADPCMB();
					if (!adpcmplay)
						break;
				}
				int s_l = (adplc * apout0_l + (8192-adplc) * apout1_l) >> 13;
				StoreSample(dest[0], s_l & mask_l);
#ifdef USE_FMGEN_STEREO
				int s_r = (adplc * apout0_r + (8192-adplc) * apout1_r) >> 13;
				StoreSample(dest[1], s_r & mask_r);
#else
				StoreSample(dest[1], s_l & mask_l);
#endif
				dest += 2;
				adplc -= adpld;
			}
			for (; count>0 && (apout0_l
#ifdef USE_FMGEN_STEREO
				|| apout0_r
#endif
				); count--)
			{
				if (adplc < 0)
				{
					apout0_l = apout1_l, apout1_l = 0;
#ifdef USE_FMGEN_STEREO
					apout0_r = apout1_r, apout1_r = 0;
#endif
					adplc += 8192;
				}
				int s_l = (adplc * apout1_l) >> 13;
				StoreSample(dest[0], s_l & mask_l);
#ifdef USE_FMGEN_STEREO
				int s_r = (adplc * apout1_r) >> 13;
				StoreSample(dest[1], s_r & mask_r);
#else
				StoreSample(dest[1], s_l & mask_l);
#endif
				dest += 2;
				adplc -= adpld;
			}
		}
		else	// fplay > fsamp	(adpld = fplay/famp*8192)
		{
			int t = (-8192*8192)/adpld;
			for (; count>0; count--)
			{
				int s_l = apout0_l * (8192+adplc);
#ifdef USE_FMGEN_STEREO
				int s_r = apout0_r * (8192+adplc);
#endif
				while (adplc < 0)
				{
					DecodeADPCMB();
					if (!adpcmplay)
						goto stop;
					s_l -= apout0_l * Max(adplc, t);
#ifdef USE_FMGEN_STEREO
					s_r -= apout0_r * Max(adplc, t);
#endif
					adplc -= t;
				}
				adplc -= 8192;
				s_l >>= 13;
				StoreSample(dest[0], s_l & mask_l);
#ifdef USE_FMGEN_STEREO
				s_r >>= 13;
				StoreSample(dest[1], s_r & mask_r);
#else
				StoreSample(dest[1], s_l & mask_l);
#endif
				dest += 2;
			}
stop:
			;
		}
	}
	if (!adpcmplay)
	{
		apout0_l = apout1_l = adpcmout_l = 0;
#ifdef USE_FMGEN_STEREO
		apout0_r = apout1_r = adpcmout_r = 0;
#endif
		adplc = 0;
	}
}

// ---------------------------------------------------------------------------
///	合成
///	@param[in,out] buffer:		合成先
///	@param[in] nsamples:	合成サンプル数
///
void OPNABase::FMMix(Sample* buffer, int nsamples)
{
	if (fmvolume_l > 0
#ifdef USE_FMGEN_STEREO
		|| fmvolume_r > 0
#endif
	)
	{
		// 準備
		// Set F-Number
		if (!(regtc & 0xc0))
// FNum is set on OPNABase::SetReg
//			csmch->SetFNum(fnum[csmch-ch]);
			;
		else
		{
			// 効果音モード
			csmch->op[0].SetFNum(fnum3[1]);	csmch->op[1].SetFNum(fnum3[2]);
			csmch->op[2].SetFNum(fnum3[0]);	csmch->op[3].SetFNum(fnum[2]);
		}

		int act = (((ch[2].Prepare() << 2) | ch[1].Prepare()) << 2) | ch[0].Prepare();
		if (reg29 & 0x80)
			act |= (ch[3].Prepare() | ((ch[4].Prepare() | (ch[5].Prepare() << 2)) << 2)) << 6;
		if (!(reg22 & 0x08))
			act &= 0x555;

		if (act & 0x555)
		{
			Mix6(buffer, nsamples, act);
		}
	}
}

// ---------------------------------------------------------------------------

void OPNABase::MixSubSL(int activech, ISample** dest)
{
	if (activech & 0x001) (*dest[0]  = ch[0].CalcL());
	if (activech & 0x004) (*dest[1] += ch[1].CalcL());
	if (activech & 0x010) (*dest[2] += ch[2].CalcL());
	if (activech & 0x040) (*dest[3] += ch[3].CalcL());
	if (activech & 0x100) (*dest[4] += ch[4].CalcL());
	if (activech & 0x400) (*dest[5] += ch[5].CalcL());
}

inline void OPNABase::MixSubS(int activech, ISample** dest)
{
	if (activech & 0x001) (*dest[0]  = ch[0].Calc());
	if (activech & 0x004) (*dest[1] += ch[1].Calc());
	if (activech & 0x010) (*dest[2] += ch[2].Calc());
	if (activech & 0x040) (*dest[3] += ch[3].Calc());
	if (activech & 0x100) (*dest[4] += ch[4].Calc());
	if (activech & 0x400) (*dest[5] += ch[5].Calc());
}

// ---------------------------------------------------------------------------

void OPNABase::BuildLFOTable()
{
	if (amtable[0] == -1)
	{
		for (int c=0; c<256; c++)
		{
			int v;
			if (c < 0x40)		v = c * 2 + 0x80;
			else if (c < 0xc0)	v = 0x7f - (c - 0x40) * 2 + 0x80;
			else				v = (c - 0xc0) * 2;
			pmtable[c] = v;

			if (c < 0x80)		v = 0xff - c * 2;
			else				v = (c - 0x80) * 2;
			amtable[c] = v & ~3;
		}
	}
}

// ---------------------------------------------------------------------------

inline void OPNABase::LFO()
{
//	LOG3("%4d - %8d, %8d\n", c, lfocount, lfodcount);

//	Operator::SetPML(pmtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
//	Operator::SetAML(amtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
	chip.SetPML(pmtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
	chip.SetAML(amtable[(lfocount >> (FM_LFOCBITS+1)) & 0xff]);
	lfocount += lfodcount;
}

// ---------------------------------------------------------------------------
#define IStoSampleL(s)	((Limit(s, 0x7fff, -0x8000) * fmvolume_l) >> 14)
#ifdef USE_FMGEN_STEREO
#define IStoSampleR(s)	((Limit(s, 0x7fff, -0x8000) * fmvolume_r) >> 14)
#endif

///	合成
///	@param[in,out] buffer:		合成先
///	@param[in] nsamples:	合成サンプル数
///	@param[in] activech:	有効なチャンネル
///
void OPNABase::Mix6(Sample* buffer, int nsamples, int activech)
{
	// Mix
	ISample ibuf[4];
	ISample* idest[6];
	idest[0] = &ibuf[pan[0]];
	idest[1] = &ibuf[pan[1]];
	idest[2] = &ibuf[pan[2]];
	idest[3] = &ibuf[pan[3]];
	idest[4] = &ibuf[pan[4]];
	idest[5] = &ibuf[pan[5]];

	Sample* limit = buffer + nsamples * 2;
	for (Sample* dest = buffer; dest < limit; dest+=2)
	{
		ibuf[1] = ibuf[2] = ibuf[3] = 0;
		if (activech & 0xaaa)
			LFO(), MixSubSL(activech, idest);
		else
			MixSubS(activech, idest);
		StoreSample(dest[0], IStoSampleL(ibuf[2] + ibuf[3]));
#ifdef USE_FMGEN_STEREO
		StoreSample(dest[1], IStoSampleR(ibuf[1] + ibuf[3]));
#else
		StoreSample(dest[1], IStoSampleL(ibuf[1] + ibuf[3]));
#endif
	}
}

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define OPNA_BASE_STATE_VERSION	2

///	ステートセーブ
void OPNABase::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(OPNA_BASE_STATE_VERSION);	// 4 bytes

	OPNBase::SaveState(f, size);
	sz += state_fio->FwriteWithSize(pan, sizeof(pan), 1);	// 6 bytes
	sz += state_fio->FwriteWithSize(fnum2, sizeof(fnum2), 1);	// 9 bytes
	sz += state_fio->FputUint8(reg22);	// 1 byte

	sz += state_fio->FputUint32_LE(reg29);	// 4 bytes
	sz += state_fio->FputUint32_LE(stmask);	// 4 bytes
	sz += state_fio->FputUint32_LE(statusnext);	// 4 bytes

	sz += state_fio->FputUint32_LE(lfocount);	// 4 bytes
	sz += state_fio->FputUint32_LE(lfodcount);	// 4 bytes
	for (int i = 0; i < 6; i++) {		// 24 bytes
		sz += state_fio->FputUint32_LE(fnum[i]);
	}

	for (int i = 0; i < 3; i++) {		// 12 bytes
		sz += state_fio->FputUint32_LE(fnum3[i]);
	}
	sz += state_fio->FputUint32_LE(adpcmmask);	// 4 bytes

	sz += state_fio->FwriteWithSize(adpcmbuf, 0x40000, 1);	// 262144 bytes

	sz += state_fio->FputUint32_LE(adpcmnotice);	// 4 bytes
	sz += state_fio->FputUint32_LE(startaddr);	// 4 bytes
	sz += state_fio->FputUint32_LE(stopaddr);	// 4 bytes
	sz += state_fio->FputUint32_LE(memaddr);	// 4 bytes

	sz += state_fio->FputUint32_LE(limitaddr);	// 4 bytes
	sz += state_fio->FputInt32_LE(adpcmlevel);	// 4 bytes
	sz += state_fio->FputInt32_LE(adpcmvolume_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(adpcmvolume_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(adpcmvolume_l);	// 4 bytes
#endif
	sz += state_fio->FputInt32_LE(adpcmvol_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(adpcmvol_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(adpcmvol_l);	// 4 bytes
#endif
	sz += state_fio->FputUint32_LE(deltan);	// 4 bytes
	sz += state_fio->FputInt32_LE(adplc);	// 4 bytes

	sz += state_fio->FputInt32_LE(adpld);	// 4 bytes
	sz += state_fio->FputUint32_LE(adplbase);	// 4 bytes
	sz += state_fio->FputInt32_LE(adpcmx);	// 4 bytes
	sz += state_fio->FputInt32_LE(adpcmd);	// 4 bytes

	sz += state_fio->FputInt32_LE(adpcmout_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(adpcmout_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(adpcmout_l);	// 4 bytes
#endif
	sz += state_fio->FputInt32_LE(apout0_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(apout0_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(apout0_l);	// 4 bytes
#endif

	sz += state_fio->FputInt32_LE(apout1_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(apout1_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(apout1_l);	// 4 bytes
#endif
	sz += state_fio->FputUint32_LE(adpcmreadbuf);	// 4 bytes

	sz += state_fio->FputBool(adpcmplay);	// 1 byte
	sz += state_fio->FputInt8(granuality);	// 1 byte
	sz += state_fio->FputBool(adpcmmask_);	// 1 byte
	sz += state_fio->FputUint8(control1);	// 1 byte

	sz += state_fio->FputUint8(control2);	// 1 byte
	sz += state_fio->FputInt8(0);	// 1 byte dummy
	sz += state_fio->FputInt8(0);	// 1 byte dummy
	sz += state_fio->FputInt8(0);	// 1 byte dummy

	sz += state_fio->FwriteWithSize(adpcmreg, sizeof(adpcmreg), 1);	// 8 bytes

	sz += state_fio->FputInt32_LE(rhythmmask_);	// 4 bytes

	for(int i = 0; i < 6; i++) {
		ch[i].SaveState(f, size);
	}

	if (size) *size += sz; // 262144 + 188
}

///	ステートロード
bool OPNABase::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != OPNA_BASE_STATE_VERSION) {
		return false;
	}
	if(!OPNBase::LoadState(f)) {
		return false;
	}
	state_fio->Fread(pan, sizeof(pan), 1);
	state_fio->Fread(fnum2, sizeof(fnum2), 1);
	reg22 = state_fio->FgetUint8();
	reg29 = state_fio->FgetUint32_LE();
	stmask = state_fio->FgetUint32_LE();
	statusnext = state_fio->FgetUint32_LE();
	lfocount = state_fio->FgetUint32_LE();
	lfodcount = state_fio->FgetUint32_LE();
	state_fio->Fread(fnum, sizeof(fnum), 1);
	state_fio->Fread(fnum3, sizeof(fnum3), 1);
	state_fio->Fread(adpcmbuf, 0x40000, 1);
	adpcmmask = state_fio->FgetUint32_LE();
	adpcmnotice = state_fio->FgetUint32_LE();
	startaddr = state_fio->FgetUint32_LE();
	stopaddr = state_fio->FgetUint32_LE();
	memaddr = state_fio->FgetUint32_LE();
	limitaddr = state_fio->FgetUint32_LE();
	adpcmlevel = state_fio->FgetInt32_LE();
	adpcmvolume_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	adpcmvolume_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif
	adpcmvol_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	adpcmvol_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif
	deltan = state_fio->FgetUint32_LE();
	adplc = state_fio->FgetInt32_LE();
	adpld = state_fio->FgetInt32_LE();
	adplbase = state_fio->FgetUint32_LE();
	adpcmx = state_fio->FgetInt32_LE();
	adpcmd = state_fio->FgetInt32_LE();
	adpcmout_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	adpcmout_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif
	apout0_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	apout0_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif
	apout1_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	apout1_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif
	adpcmreadbuf = state_fio->FgetUint32_LE();
	adpcmplay = state_fio->FgetBool();
	granuality = state_fio->FgetInt8();
	adpcmmask_ = state_fio->FgetBool();
	control1 = state_fio->FgetUint8();
	control2 = state_fio->FgetUint8();
	state_fio->FgetInt8();	// 1 byte dummy
	state_fio->FgetInt8();	// 1 byte dummy
	state_fio->FgetInt8();	// 1 byte dummy
	state_fio->Fread(adpcmreg, sizeof(adpcmreg), 1);
	rhythmmask_ = state_fio->FgetInt32_LE();
	for(int i = 0; i < 6; i++) {
		if(!ch[i].LoadState(f)) {
			return false;
		}
	}
	return true;
}

#endif // defined(BUILD_OPNA) || defined(BUILD_OPNB)

// ---------------------------------------------------------------------------
//	YM2608(OPNA)
// ---------------------------------------------------------------------------

#ifdef BUILD_OPNA

// ---------------------------------------------------------------------------
///	構築
///
OPNA::OPNA()
{
	loaded_rhythm = false;
	for (int i=0; i<6; i++)
	{
		rhythm[i].sample = 0;
		rhythm[i].pos = 0;
		rhythm[i].size = 0;
		rhythm[i].volume_l = 0;
#ifdef USE_FMGEN_STEREO
		rhythm[i].volume_r = 0;
#endif
		rhythm[i].level = 0;
		rhythm[i].pan = 0;
	}
	rhythmtvol_l = 0;
#ifdef USE_FMGEN_STEREO
	rhythmtvol_r = 0;
#endif
	adpcmmask = 0x3ffff;
	adpcmnotice = 4;
	csmch = &ch[2];
}

// ---------------------------------------------------------------------------

OPNA::~OPNA()
{
	delete[] adpcmbuf;
	for (int i=0; i<6; i++)
		delete[] rhythm[i].sample;
}



// ---------------------------------------------------------------------------
///	初期化
///
bool OPNA::Init(uint32_t c, uint32_t r, bool ipflag, const _TCHAR* const* paths)
{
	rate = 8000;
	LoadRhythmSample(paths);

	if (!adpcmbuf)
		adpcmbuf = new uint8_t[0x40000];
	if (!adpcmbuf)
		return false;

	if (!SetRate(c, r, ipflag))
		return false;
	if (!OPNABase::Init(c, r, ipflag))
		return false;

	Reset();

	SetVolumeADPCM(0, 0);
	SetVolumeRhythmTotal(0, 0);
	for (int i=0; i<6; i++)
		SetVolumeRhythm(i, 0, 0);
	return true;
}

// ---------------------------------------------------------------------------
///	リセット
///
void OPNA::Reset()
{
	reg29 = 0x1f;
	rhythmkey = 0;
	rhythmtl = 0;
	limitaddr = 0x3ffff;
	OPNABase::Reset();
}

// ---------------------------------------------------------------------------
///	サンプリングレート変更
///
bool OPNA::SetRate(uint32_t c, uint32_t r, bool ipflag)
{
	if (!OPNABase::SetRate(c, r, ipflag))
		return false;

	for (int i=0; i<6; i++)
	{
		rhythm[i].step = rhythm[i].rate * 1024 / r;
	}
	return true;
}


// ---------------------------------------------------------------------------
///	リズム音を読みこむ
/// @param[in] paths: サンプリングファイルのあるフォルダリスト
///
bool OPNA::LoadRhythmSample(const _TCHAR* const* paths)
{
#ifdef HAS_YM2608
	static const _TCHAR* rhythmname[6][2] =
	{
		{_T("BD"), 0}, {_T("SD"), 0}, {_T("TOP"), 0}, {_T("HH"), 0}, {_T("TOM"), 0}, {_T("RIM"), _T("RYM")}
	};

	if(loaded_rhythm) {
		return loaded_rhythm;
	}

	int i;
	for (i=0; i<6; i++)
		rhythm[i].pos = ~0;

	_TCHAR buf[_MAX_PATH];
	_TCHAR fname[_MAX_PATH];
	for (i=0; i<6; i++)
	{
		FILEIO file;
		uint32_t fsize;
		bool exist = false;
		for(int n=0; !exist; n++) {
			const _TCHAR *path = paths ? paths[n] : 0;
			if (!path && n != 0) break;
			for(int j=0; j<2; j++) {
				if (!rhythmname[i][j]) break;
				_tcsncpy(fname, _T("2608_"), _MAX_PATH);
				_tcsncat(fname, rhythmname[i][j], _MAX_PATH);
				_tcsncat(fname, _T(".WAV"), _MAX_PATH);
				buf[0] = _T('\0');
				if (path)
					_tcsncpy(buf, path, _MAX_PATH);
				_tcsncat(buf, fname, _MAX_PATH);

				if (file.Fopen(buf, FILEIO::READ_BINARY)) {
					exist = true;
					break;
				}
			}
		}
		if (!exist) break;

		struct
		{
			uint32_t chunksize;
			uint16_t tag;
			uint16_t nch;
			uint32_t rate;
			uint32_t avgbytes;
			uint16_t align;
			uint16_t bps;
			uint16_t size;
		} whdr;

		file.Fseek(0x10, FILEIO::SEEKSET);
		file.Fread(&whdr, sizeof(whdr), 1);

		uint8_t subchunkname[4];
		fsize = 4 + whdr.chunksize - sizeof(whdr);
		do
		{
			file.Fseek(fsize, FILEIO::SEEKCUR);
			file.Fread(&subchunkname, 4, 1);
			file.Fread(&fsize, 4, 1);
		} while (memcmp("data", subchunkname, 4));

		fsize /= 2;
		if (fsize >= 0x100000 || whdr.tag != 1 || whdr.nch != 1)
			break;
		fsize = Max(fsize, (1<<31)/1024);

		delete rhythm[i].sample;
		rhythm[i].sample = new int16_t[fsize];
		if (!rhythm[i].sample)
			break;

		file.Fread(rhythm[i].sample, fsize * 2, 1);

		rhythm[i].rate = whdr.rate;
		rhythm[i].step = rhythm[i].rate * 1024 / rate;
		rhythm[i].pos = rhythm[i].size = fsize * 1024;

		logging->out_logf_x(LOG_INFO, CMsg::VSTR_was_loaded, fname);
	}
	if (i != 6)
	{
		logging->out_logf_x(LOG_WARN, CMsg::VSTR_couldn_t_be_loaded, fname);
		for (i=0; i<6; i++)
		{
			delete[] rhythm[i].sample;
			rhythm[i].sample = 0;
		}
		loaded_rhythm = false;
	}
	loaded_rhythm = true;
	return loaded_rhythm;
#else
	return false;
#endif
}



// ---------------------------------------------------------------------------
///	レジスタアレイにデータを設定
///
void OPNA::SetReg(uint32_t addr, uint32_t data)
{
	addr &= 0x1ff;

	switch (addr)
	{
	case 0x29:
		reg29 = data;
//		UpdateStatus(); //?
		break;

	// Rhythm ----------------------------------------------------------------
	case 0x10:			// DM/KEYON
		if (!(data & 0x80))  // KEY ON
		{
			rhythmkey |= data & 0x3f;
			if (data & 0x01) rhythm[0].pos = 0;
			if (data & 0x02) rhythm[1].pos = 0;
			if (data & 0x04) rhythm[2].pos = 0;
			if (data & 0x08) rhythm[3].pos = 0;
			if (data & 0x10) rhythm[4].pos = 0;
			if (data & 0x20) rhythm[5].pos = 0;
		}
		else
		{					// DUMP
			rhythmkey &= ~data;
		}
		break;

	case 0x11:
		rhythmtl = ~data & 63;
		break;

	case 0x18: 		// Bass Drum
	case 0x19:		// Snare Drum
	case 0x1a:		// Top Cymbal
	case 0x1b:		// Hihat
	case 0x1c:		// Tom-tom
	case 0x1d:		// Rim shot
		rhythm[addr & 7].pan   = (data >> 6) & 3;
		rhythm[addr & 7].level = ~data & 31;
		break;

	case 0x100: case 0x101:
	case 0x102: case 0x103:
	case 0x104: case 0x105:
	case 0x108:	case 0x109:
	case 0x10a:	case 0x10b:
	case 0x10c:	case 0x10d:
	case 0x110:
		OPNABase::SetADPCMBReg(addr - 0x100, data);
		break;

	case 0x0127:
		// for PC-8801FA/MA shop demonstration
		if ((control1 & 0x10) && (status & adpcmnotice)) {
			ResetStatus(adpcmnotice);
		}
		break;

	default:
		OPNABase::SetReg(addr, data);
		break;
	}
}


// ---------------------------------------------------------------------------
///	リズム合成
///
void OPNA::RhythmMix(Sample* buffer, uint32_t count)
{
	if ((rhythmtvol_l < 128
#ifdef USE_FMGEN_STEREO
		|| rhythmtvol_r < 128
#endif
		) && rhythm[0].sample && (rhythmkey & 0x3f))
	{
		Sample* limit = buffer + count * 2;
		for (int i=0; i<6; i++)
		{
			Rhythm& r = rhythm[i];
			if ((rhythmkey & (1 << i)) && r.level < 128)
			{
				int db_l = Limit(rhythmtl+rhythmtvol_l+r.level+r.volume_l, 127, -31);
				int vol_l = tltable[FM_TLPOS+(db_l << (FM_TLBITS-7))] >> 4;
				int mask_l = -((r.pan >> 1) & 1);
#ifdef USE_FMGEN_STEREO
				int db_r = Limit(rhythmtl+rhythmtvol_r+r.level+r.volume_r, 127, -31);
				int vol_r = tltable[FM_TLPOS+(db_r << (FM_TLBITS-7))] >> 4;
				int mask_r = -(r.pan & 1);
#endif

				if (rhythmmask_ & (1 << i))
				{
					mask_l = 0;
#ifdef USE_FMGEN_STEREO
					mask_r = 0;
#endif
				}

				for (Sample* dest = buffer; dest<limit && r.pos < r.size; dest+=2)
				{
					int sample_l = (r.sample[r.pos / 1024] * vol_l) >> 12;
#ifdef USE_FMGEN_STEREO
					int sample_r = (r.sample[r.pos / 1024] * vol_r) >> 12;
#endif
					r.pos += r.step;
					StoreSample(dest[0], sample_l & mask_l);
#ifdef USE_FMGEN_STEREO
					StoreSample(dest[1], sample_r & mask_r);
#else
					StoreSample(dest[1], sample_l & mask_l);
#endif
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
///	音量設定
///
void OPNA::SetVolumeRhythmTotal(int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	rhythmtvol_l = -(db_l * 2 / 3);

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	rhythmtvol_r = -(db_r * 2 / 3);
#endif
}

void OPNA::SetVolumeRhythm(int index, int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	rhythm[index].volume_l = -(db_l * 2 / 3);

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	rhythm[index].volume_r = -(db_r * 2 / 3);
#endif
}

void OPNA::SetVolumeADPCM(int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	if (db_l > -192)
		adpcmvol_l = int(65536.0 * pow(10.0, db_l / 40.0));
	else
		adpcmvol_l = 0;

	adpcmvolume_l = (adpcmvol_l * adpcmlevel) >> 12;

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	if (db_r > -192)
		adpcmvol_r = int(65536.0 * pow(10.0, db_r / 40.0));
	else
		adpcmvol_r = 0;

	adpcmvolume_r = (adpcmvol_r * adpcmlevel) >> 12;
#endif
}

// ---------------------------------------------------------------------------
///	合成
///	@param[in,out] buffer:		合成先
///	@param[in] nsamples:	合成サンプル数
///
void OPNA::Mix(Sample* buffer, int nsamples)
{
	FMMix(buffer, nsamples);
	psg.Mix(buffer, nsamples);
	ADPCMBMix(buffer, nsamples);
	RhythmMix(buffer, nsamples);
}

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define OPNA_STATE_VERSION	2

///	ステートセーブ
void OPNA::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(OPNA_STATE_VERSION);	// 4 bytes

	OPNABase::SaveState(f, size);
	for(int i = 0; i < 6; i++) {	// 6*6 = 36 bytes
		sz += state_fio->FputUint8(rhythm[i].pan);
		sz += state_fio->FputInt8(rhythm[i].level);
		sz += state_fio->FputUint32_LE(rhythm[i].pos);
	}
	sz += state_fio->FputInt8(rhythmtl);	// 1 byte
	sz += state_fio->FputUint8(rhythmkey);	// 1 byte
	sz += state_fio->FputInt8(0);	// 1 byte dummy
	sz += state_fio->FputInt8(0);	// 1 byte dummy

	sz += state_fio->FputInt32_LE(rhythmtvol_l);	// 4 bytes
#ifdef USE_FMGEN_STEREO
	sz += state_fio->FputInt32_LE(rhythmtvol_r);	// 4 bytes
#else
	sz += state_fio->FputInt32_LE(rhythmtvol_l);	// 4 bytes
#endif

	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy
	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy
	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy

	if (size) *size += sz;	// 64	// 16 bytes bound
}

///	ステートロード
bool OPNA::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != OPNA_STATE_VERSION) {
		return false;
	}
	if(!OPNABase::LoadState(f)) {
		return false;
	}
	for(int i = 0; i < 6; i++) {
		rhythm[i].pan = state_fio->FgetUint8();
		rhythm[i].level = state_fio->FgetInt8();
		rhythm[i].pos = state_fio->FgetUint32_LE();
	}
	rhythmtl = state_fio->FgetInt8();
	rhythmkey = state_fio->FgetUint8();
	state_fio->FgetInt8();	// 1 byte dummy
	state_fio->FgetInt8();	// 1 byte dummy

	rhythmtvol_l = state_fio->FgetInt32_LE();
#ifdef USE_FMGEN_STEREO
	rhythmtvol_r = state_fio->FgetInt32_LE();
#else
	state_fio->FgetInt32_LE();
#endif

	state_fio->FgetInt32_LE();	// 4 bytes dummy
	state_fio->FgetInt32_LE();	// 4 bytes dummy
	state_fio->FgetInt32_LE();	// 4 bytes dummy
	return true;
}

#endif // BUILD_OPNA

// ---------------------------------------------------------------------------
//	YM2610(OPNB)
// ---------------------------------------------------------------------------

#ifdef BUILD_OPNB

// ---------------------------------------------------------------------------
///	構築
///
OPNB::OPNB()
{
	adpcmabuf = 0;
	adpcmasize = 0;
	for (int i=0; i<6; i++)
	{
		adpcma[i].pan = 0;
		adpcma[i].level = 0;
		adpcma[i].volume_l = 0;
#ifdef USE_FMGEN_STEREO
		adpcma[i].volume_r = 0;
#endif
		adpcma[i].pos = 0;
		adpcma[i].step = 0;
		adpcma[i].start = 0;
		adpcma[i].stop = 0;
		adpcma[i].adpcmx = 0;
		adpcma[i].adpcmd = 0;
	}
	adpcmatl = 0;
	adpcmakey = 0;
	adpcmatvol_l = 0;
#ifdef USE_FMGEN_STEREO
	adpcmatvol_r = 0;
#endif
	adpcmmask = 0;
	adpcmnotice = 0x8000;
	granuality = -1;
	csmch = &ch[2];

	InitADPCMATable();
}

OPNB::~OPNB()
{
}

// ---------------------------------------------------------------------------
///	初期化
///
bool OPNB::Init(uint32_t c, uint32_t r, bool ipflag,
				uint8_t *_adpcma, int _adpcma_size,
				uint8_t *_adpcmb, int _adpcmb_size)
{
	int i;
	if (!SetRate(c, r, ipflag))
		return false;
	if (!OPNABase::Init(c, r, ipflag))
		return false;

	adpcmabuf = _adpcma;
	adpcmasize = _adpcma_size;
	adpcmbuf = _adpcmb;

	for (i=0; i<=24; i++)		// max 16M bytes
	{
		if (_adpcmb_size <= (1 << i))
		{
			adpcmmask = (1 << i) - 1;
			break;
		}
	}

//	adpcmmask = _adpcmb_size - 1;
	limitaddr = adpcmmask;

	Reset();

	SetVolumeFM(0, 0);
	SetVolumePSG(0, 0, 0);
	SetVolumeADPCMB(0, 0);
	SetVolumeADPCMATotal(0, 0);
	for (i=0; i<6; i++)
		SetVolumeADPCMA(i, 0, 0);
	SetChannelMask(0);
	return true;
}

// ---------------------------------------------------------------------------
///	リセット
///
void OPNB::Reset()
{
	OPNABase::Reset();

	stmask = ~0;
	adpcmakey = 0;
	reg29 = ~0;

	for (int i=0; i<6; i++)
	{
		adpcma[i].pan = 0;
		adpcma[i].level = 0;
		adpcma[i].volume_l = 0;
#ifdef USE_FMGEN_STEREO
		adpcma[i].volume_r = 0;
#endif
		adpcma[i].pos = 0;
		adpcma[i].step = 0;
		adpcma[i].start = 0;
		adpcma[i].stop = 0;
		adpcma[i].adpcmx = 0;
		adpcma[i].adpcmd = 0;
	}
}

// ---------------------------------------------------------------------------
///	サンプリングレート変更
///
bool OPNB::SetRate(uint32_t c, uint32_t r, bool ipflag)
{
	if (!OPNABase::SetRate(c, r, ipflag))
		return false;

	adpcmastep = int(double(c) / 54 * 8192 / r);
	return true;
}

// ---------------------------------------------------------------------------
///	レジスタアレイにデータを設定
///
void OPNB::SetReg(uint32_t addr, uint32_t data)
{
	addr &= 0x1ff;

	switch (addr)
	{
	// omitted registers
	case 0x29:
	case 0x2d: case 0x2e: case 0x2f:
		break;

	// ADPCM A ---------------------------------------------------------------
	case 0x100:			// DM/KEYON
		if (!(data & 0x80))  // KEY ON
		{
			adpcmakey |= data & 0x3f;
			for (int c=0; c<6; c++)
			{
				if (data & (1<<c))
				{
					ResetStatus(0x100 << c);
					adpcma[c].pos = adpcma[c].start;
//					adpcma[c].step = 0x10000 - adpcma[c].step;
					adpcma[c].step = 0;
					adpcma[c].adpcmx = 0;
					adpcma[c].adpcmd = 0;
					adpcma[c].nibble = 0;
				}
			}
		}
		else
		{					// DUMP
			adpcmakey &= ~data;
		}
		break;

	case 0x101:
		adpcmatl = ~data & 63;
		break;

	case 0x108:	case 0x109:	case 0x10a:
	case 0x10b: case 0x10c:	case 0x10d:
		adpcma[addr & 7].pan   = (data >> 6) & 3;
		adpcma[addr & 7].level = ~data & 31;
		break;

	case 0x110: case 0x111: case 0x112:	// START ADDRESS (L)
	case 0x113: case 0x114:	case 0x115:
	case 0x118: case 0x119: case 0x11a:	// START ADDRESS (H)
	case 0x11b: case 0x11c: case 0x11d:
		adpcmareg[addr - 0x110] = data;
		adpcma[addr & 7].pos = adpcma[addr & 7].start =
			(adpcmareg[(addr&7)+8]*256+adpcmareg[addr&7]) << 9;
		break;

	case 0x120: case 0x121: case 0x122:	// END ADDRESS (L)
	case 0x123: case 0x124: case 0x125:
	case 0x128: case 0x129: case 0x12a:	// END ADDRESS (H)
	case 0x12b: case 0x12c: case 0x12d:
		adpcmareg[addr - 0x110] = data;
		adpcma[addr & 7].stop =
			(adpcmareg[(addr&7)+24]*256+adpcmareg[(addr&7)+16] + 1) << 9;
		break;

	// ADPCMB -----------------------------------------------------------------
	case 0x10:
		if ((data & 0x80) && !adpcmplay)
		{
			adpcmplay = true;
			memaddr = startaddr;
			adpcmx = 0, adpcmd = 127;
			adplc = 0;
		}
		if (data & 1)
			adpcmplay = false;
		control1 = data & 0x91;
		break;


	case 0x11:		// Control Register 2
		control2 = data & 0xc0;
		break;

	case 0x12:		// Start Address L
	case 0x13:		// Start Address H
		adpcmreg[addr - 0x12 + 0] = data;
		startaddr = (adpcmreg[1]*256+adpcmreg[0]) << 9;
		memaddr = startaddr;
		break;

	case 0x14:		// Stop Address L
	case 0x15:		// Stop Address H
		adpcmreg[addr - 0x14 + 2] = data;
		stopaddr = (adpcmreg[3]*256+adpcmreg[2] + 1) << 9;
//		LOG1("  stopaddr %.6x", stopaddr);
		break;

	case 0x19:		// delta-N L
	case 0x1a:		// delta-N H
		adpcmreg[addr - 0x19 + 4] = data;
		deltan = adpcmreg[5]*256+adpcmreg[4];
		deltan = Max(256, deltan);
		adpld = deltan * adplbase >> 16;
		break;

	case 0x1b:		// Level Control
		adpcmlevel = data; 
		adpcmvolume_l = (adpcmvol_l * adpcmlevel) >> 12;
#ifdef USE_FMGEN_STEREO
		adpcmvolume_r = (adpcmvol_r * adpcmlevel) >> 12;
#endif
		break;

	case 0x1c:		// Flag Control
		stmask = ~((data & 0xbf) << 8);
		status &= stmask;
		UpdateStatus();
		break;

	default:
		OPNABase::SetReg(addr, data);
		break;
	}
//	LOG0("\n");
}

// ---------------------------------------------------------------------------
///	レジスタ取得
///
uint32_t OPNB::GetReg(uint32_t addr)
{
	if (addr < 0x10)
		return psg.GetReg(addr);

	return 0;
}

// ---------------------------------------------------------------------------
///	拡張ステータスを読みこむ
///
uint32_t OPNB::ReadStatusEx()
{
	return (status & stmask) >> 8;
}

// ---------------------------------------------------------------------------
///	YM2610
///
int OPNB::jedi_table[(48+1)*16];

void OPNB::InitADPCMATable()
{
	const static int8_t table2[] =
	{
		 1,  3,  5,  7,  9, 11, 13, 15,
		-1, -3, -5, -7, -9,-11,-13,-15,
	};

	for (int i=0; i<=48; i++)
	{
		int s = int(16.0 * pow (1.1, i) * 3);
		for (int j=0; j<16; j++)
		{
			jedi_table[i*16+j] = s * table2[j] / 8;
		}
	}
}

// ---------------------------------------------------------------------------
///	ADPCMA 合成
///
void OPNB::ADPCMAMix(Sample* buffer, uint32_t count)
{
	const static int decode_tableA1[16] =
	{
		-1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16,
		-1*16, -1*16, -1*16, -1*16, 2*16, 5*16, 7*16, 9*16
	};

	if ((adpcmatvol_l < 128
#ifdef USE_FMGEN_STEREO
		|| adpcmatvol_r < 128
#endif
		) && (adpcmakey & 0x3f))
	{
		Sample* limit = buffer + count * 2;
		for (int i=0; i<6; i++)
		{
			ADPCMA& r = adpcma[i];
			if ((adpcmakey & (1 << i)) && r.level < 128)
			{
				uint32_t mask_l = r.pan & 2 ? -1 : 0;
				uint32_t mask_r = r.pan & 1 ? -1 : 0;
				if (rhythmmask_ & (1 << i))
				{
					mask_l = mask_r = 0;
				}

				int db_l = Limit(adpcmatl+adpcmatvol_l+r.level+r.volume_l, 127, -31);
				int vol_l = tltable[FM_TLPOS+(db_l << (FM_TLBITS-7))] >> 4;
#ifdef USE_FMGEN_STEREO
				int db_r = Limit(adpcmatl+adpcmatvol_r+r.level+r.volume_r, 127, -31);
				int vol_r = tltable[FM_TLPOS+(db_r << (FM_TLBITS-7))] >> 4;
#endif

				Sample* dest = buffer;
				for ( ; dest<limit; dest+=2)
				{
					r.step += adpcmastep;
					if (r.pos >= r.stop)
					{
						SetStatus(0x100 << i);
						adpcmakey &= ~(1<<i);
						break;
					}

					for (; r.step > 0x10000; r.step -= 0x10000)
					{
						int data;
						if (!(r.pos & 1))
						{
							r.nibble = adpcmabuf[r.pos>>1];
							data = r.nibble >> 4;
						}
						else
						{
							data = r.nibble & 0x0f;
						}
						r.pos++;

						r.adpcmx += jedi_table[r.adpcmd + data];
						r.adpcmx = Limit(r.adpcmx, 2048*3-1, -2048*3);
						r.adpcmd += decode_tableA1[data];
						r.adpcmd = Limit(r.adpcmd, 48*16, 0);
					}
					int sample_l = (r.adpcmx * vol_l) >> 10;
					StoreSample(dest[0], sample_l & mask_l);
#ifdef USE_FMGEN_STEREO
					int sample_r = (r.adpcmx * vol_r) >> 10;
					StoreSample(dest[1], sample_r & mask_r);
#else
					StoreSample(dest[1], sample_l & mask_l);
#endif
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
///	音量設定
///
void OPNB::SetVolumeADPCMATotal(int db_l, int db_r)
{
	db_l = Min(db_l, 20);
	db_r = Min(db_r, 20);

	adpcmatvol_l = -(db_l * 2 / 3);

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	adpcmatvol_r = -(db_r * 2 / 3);
#endif
}

void OPNB::SetVolumeADPCMA(int index, int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	adpcma[index].volume_l = -(db_l * 2 / 3);

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	adpcma[index].volume_r = -(db_r * 2 / 3);
#endif
}

void OPNB::SetVolumeADPCMB(int db_l, int db_r)
{
	db_l = Min(db_l, 20);

	if (db_l > -192)
		adpcmvol_l = int(65536.0 * pow(10.0, db_l / 40.0));
	else
		adpcmvol_l = 0;

#ifdef USE_FMGEN_STEREO
	db_r = Min(db_r, 20);

	if (db_r > -192)
		adpcmvol_r = int(65536.0 * pow(10.0, db_r / 40.0));
	else
		adpcmvol_r = 0;
#endif
}

// ---------------------------------------------------------------------------
///	合成
///	@param[in,out] buffer:		合成先
///	@param[in] nsamples:	合成サンプル数
///
void OPNB::Mix(Sample* buffer, int nsamples)
{
	FMMix(buffer, nsamples);
	psg.Mix(buffer, nsamples);
	ADPCMBMix(buffer, nsamples);
	ADPCMAMix(buffer, nsamples);
}

#endif // BUILD_OPNB

}	// namespace FM
