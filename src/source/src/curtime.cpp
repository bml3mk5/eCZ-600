/** @file curtime.cpp

	@author Sasaji
	@date   2018.01.01

	@brief current time
*/

#include "curtime.h"

CurTime::CurTime()
{
#ifdef USE_ALWAYS_HOST_TIME
	epoch_relative = 0;
	epoch_hold = 0;
#else
	epoch = 0;
#endif
	memset(time_temp, 0, sizeof(time_temp));
	initialized = false;
}

CurTime::~CurTime()
{
}

void CurTime::Increment()
{
#ifndef USE_ALWAYS_HOST_TIME
	epoch++;
#endif
}

void CurTime::GetHostTime()
{
#ifdef USE_ALWAYS_HOST_TIME
	GetCurrTime();
#else
	epoch = time(NULL);
#endif
}

void CurTime::GetCurrTime()
{
#ifdef USE_ALWAYS_HOST_TIME
	time_t epoch_curr = time(NULL);
	epoch_curr = (time_t)((int64_t)epoch_curr + epoch_relative);
	struct tm *local = localtime(&epoch_curr);
#else
	struct tm *local = localtime(&epoch);
#endif
	time_temp[0] = local->tm_year + 1900;
	time_temp[1] = local->tm_mon + 1;
	time_temp[2] = local->tm_mday;
	time_temp[3] = local->tm_hour;
	time_temp[4] = local->tm_min;
	time_temp[5] = local->tm_sec;
	time_temp[6] = local->tm_wday;
	initialized = true;
}

void CurTime::CommitTime()
{
	struct tm local;
	memset(&local, 0, sizeof(local));
	// year 1970-2069
	if (time_temp[0] < 70) time_temp[0] += 2000;
	else if (time_temp[0] < 100) time_temp[0] += 1900;
	local.tm_year = time_temp[0] - 1900;
	local.tm_mon = time_temp[1] - 1;
	local.tm_mday = time_temp[2];
	local.tm_hour = time_temp[3];
	local.tm_min = time_temp[4];
	local.tm_sec = time_temp[5];
//	local.tm_wday = time_temp[6];
	local.tm_isdst = -1;

	time_t epoch_temp = mktime(&local);
#ifdef USE_ALWAYS_HOST_TIME
	time_t epoch_curr = time(NULL);
	epoch_relative = (int64_t)(epoch_temp - epoch_curr);
#else
	epoch = epoch_temp;
#endif
}

int CurTime::GetYear()
{
	return time_temp[0];
}
int CurTime::GetMonth()
{
	return time_temp[1];
}
int CurTime::GetDay()
{
	return time_temp[2];
}
int CurTime::GetHour()
{
	return time_temp[3];
}
int CurTime::GetMin()
{
	return time_temp[4];
}
int CurTime::GetSec()
{
	return time_temp[5];
}
int CurTime::GetDayOfWeek()
{
	return time_temp[6];
}

void CurTime::SetYear(int value)
{
	time_temp[0] = value;
}
void CurTime::SetMonth(int value)
{
	time_temp[1] = value;
}
void CurTime::SetDay(int value)
{
	time_temp[2] = value;
}
void CurTime::SetHour(int value)
{
	time_temp[3] = value;
}
void CurTime::SetMin(int value)
{
	time_temp[4] = value;
}
void CurTime::SetSec(int value)
{
	time_temp[5] = value;
}
void CurTime::SetDayOfWeek(int value)
{
	time_temp[6] = value;
}

void CurTime::StoreHoldTime()
{
#ifdef USE_ALWAYS_HOST_TIME
	time_t epoch_curr = time(NULL);
	epoch_hold = ((int64_t)epoch_curr + epoch_relative);
#endif
}

void CurTime::ReleaseHoldTime()
{
#ifdef USE_ALWAYS_HOST_TIME
	time_t epoch_curr = time(NULL);
	epoch_relative = (epoch_hold - (int64_t)epoch_curr);
	epoch_hold = 0;
#endif
}

#define STATE_VERSION	1

void CurTime::SaveState(FILEIO *fio, size_t *size)
{
	fio->FputUint32_LE(STATE_VERSION);
	fio->FputInt32_LE(initialized ? 1: 0);

#ifdef USE_ALWAYS_HOST_TIME
	int64_t epoch_absolute = (int64_t)time(NULL);
	epoch_absolute = epoch_absolute + epoch_relative;
	fio->FputInt64_LE(epoch_absolute);
	fio->FputInt64_LE(epoch_hold);
#else
	fio->FputInt64_LE(epoch);
	fio->FputInt64_LE(0);	// dummy
#endif

	fio->FputInt32_LE(0);	// dummy
	fio->FputInt32_LE(0);	// dummy

	if (size) *size += 32;
}

bool CurTime::LoadState(FILEIO *fio)
{
	if(fio->FgetUint32_LE() != STATE_VERSION) {
		return false;
	}
	initialized = (fio->FgetInt32_LE() != 0);

#ifdef USE_ALWAYS_HOST_TIME
	int64_t epoch_absolute = fio->FgetInt64_LE();
	epoch_hold = fio->FgetInt64_LE();

	int64_t epoch_curr = (int64_t)time(NULL);
	epoch_relative = epoch_absolute - epoch_curr;
#else
	epoch = fio->FgetInt64_LE();
	fio->FgetInt64_LE();	// dummy
#endif

	fio->FgetInt32_LE();	// dummy
	fio->FgetInt32_LE();	// dummy

	return true;
}
