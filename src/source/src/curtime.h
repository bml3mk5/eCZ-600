/** @file curtime.h

	@author Sasaji
	@date   2018.01.01

	@brief current time
*/

#ifndef _CUR_TIME_H_
#define _CUR_TIME_H_

#include "common.h"
#include <time.h>
#include "fileio.h"

/**
	@brief Current time for RTC
*/
class CurTime
{
private:
#ifdef USE_ALWAYS_HOST_TIME
	int64_t epoch_relative;
	int64_t epoch_hold;
#else
	time_t epoch;
#endif
	int time_temp[7];	// YY/MM/DD HH/MI/SS WK
	bool initialized;

public:
	CurTime();
	~CurTime();

	void Increment();

	void GetHostTime();
	void GetCurrTime();
	void CommitTime();

	int GetYear();
	int GetMonth();
	int GetDay();
	int GetHour();
	int GetMin();
	int GetSec();
	int GetDayOfWeek();
	void SetYear(int value);
	void SetMonth(int value);
	void SetDay(int value);
	void SetHour(int value);
	void SetMin(int value);
	void SetSec(int value);
	void SetDayOfWeek(int value);

	void StoreHoldTime();
	void ReleaseHoldTime();

	void SaveState(FILEIO *fio, size_t *size);
	bool LoadState(FILEIO *fio);
};

#endif /* _CUR_TIME_H_ */
