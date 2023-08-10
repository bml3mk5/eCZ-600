﻿/// ---------------------------------------------------------------------------
/// @file fmtimer.cpp
///	FM sound generator common timer module
///	Copyright (C) cisc 1998, 2000.
/// ---------------------------------------------------------------------------
///	$Id: fmtimer.cpp,v 1.1 2000/09/08 13:45:56 cisc Exp $

#include "headers.h"
#include "fmtimer.h"

#include "../../fileio.h"

using namespace FM;

Timer::Timer()
{
	status = 0;
	regtc = 0;
	regta[0] = 0;
	regta[1] = 0;

	timera = timera_count = 0;
	timerb = timerb_count = 0;
	prescaler = 1;
}

// ---------------------------------------------------------------------------
///	タイマー制御
///
void Timer::SetTimerControl(uint32_t data)
{
	uint32_t tmp = regtc ^ data;
	regtc = uint8_t(data);

	if (data & 0x10)
		ResetStatus(1);
	if (data & 0x20)
		ResetStatus(2);

	if (tmp & 0x01)
		timera_count = (data & 1) ? timera * prescaler : 0;
	if (tmp & 0x02)
		timerb_count = (data & 2) ? timerb * prescaler : 0;
}

// ---------------------------------------------------------------------------
///	タイマーA 周期設定
///
void Timer::SetTimerA(uint32_t addr, uint32_t data)
{
	uint32_t tmp;
	regta[addr & 1] = uint8_t(data);
	tmp = (regta[0] << 2) + (regta[1] & 3);
	timera = 1024-tmp;
}

// ---------------------------------------------------------------------------
///	タイマーB 周期設定
///
void Timer::SetTimerB(uint32_t data)
{
	timerb = (256-data) << 4;
}

// ---------------------------------------------------------------------------
///	タイマー時間処理
///
bool Timer::Count(int32_t clock)
{
	bool event = false;

	if (timera_count)
	{
		timera_count -= clock;
		if (timera_count <= 0)
		{
			event = true;
			TimerA();

			while (timera_count <= 0)
				timera_count += timera * prescaler;

			if (regtc & 4)
				SetStatus(1);
		}
	}
	if (timerb_count)
	{
		timerb_count -= clock;
		if (timerb_count <= 0)
		{
			event = true;
			while (timerb_count <= 0)
				timerb_count += timerb * prescaler;

			if (regtc & 8)
				SetStatus(2);
		}
	}
	return event;
}

// ---------------------------------------------------------------------------
///	次にタイマーが発生するまでの時間を求める
///
int32_t Timer::GetNextEvent()
{
	if(timera_count > 0 && timerb_count > 0) {
		return (timera_count < timerb_count ? timera_count : timerb_count);
	} else if(timera_count > 0) {
		return timera_count;
	} else if (timerb_count > 0) {
		return timerb_count;
	}
	return 0;
}

// ---------------------------------------------------------------------------
///	タイマー基準値設定
///
void Timer::SetTimerPrescaler(int32_t p)
{
	prescaler = p;
}

// ---------------------------------------------------------------------------
///	ステートセーブ
///
#define TIMER_STATE_VERSION	1

///	ステートセーブ
void Timer::SaveState(void *f, size_t *size)
{
	FILEIO *state_fio = (FILEIO *)f;
	size_t sz = 0;

	sz += state_fio->FputUint32_LE(TIMER_STATE_VERSION);	// 4 bytes

	sz += state_fio->FputUint8(status);	// 1 byte
	sz += state_fio->FputUint8(regtc);	// 1 byte
	sz += state_fio->FwriteWithSize(regta, sizeof(regta), 1);	// 2 bytes
	sz += state_fio->FputInt32_LE(timera);	// 4 bytes
	sz += state_fio->FputInt32_LE(timera_count);	// 4 bytes

	sz += state_fio->FputInt32_LE(timerb);	// 4 bytes
	sz += state_fio->FputInt32_LE(timerb_count);	// 4 bytes
	sz += state_fio->FputInt32_LE(prescaler);	// 4 bytes
	sz += state_fio->FputInt32_LE(0);	// 4 bytes dummy

	if (size) *size += sz;	// 32	// 16 bytes bound
}

///	ステートロード
bool Timer::LoadState(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;

	if(state_fio->FgetUint32_LE() != TIMER_STATE_VERSION) {
		return false;
	}
	status = state_fio->FgetUint8();	// 1 byte
	regtc = state_fio->FgetUint8();	// 1 byte
	state_fio->Fread(regta, sizeof(regta), 1);	// 2 bytes
	timera = state_fio->FgetInt32_LE();	// 4 bytes
	timera_count = state_fio->FgetInt32_LE();	// 4 bytes
	timerb = state_fio->FgetInt32_LE();	// 4 bytes
	timerb_count = state_fio->FgetInt32_LE();	// 4 bytes
	prescaler = state_fio->FgetInt32_LE();	// 4 bytes
	state_fio->FgetInt32_LE();	// 4 bytes dummy
	return true;
}

