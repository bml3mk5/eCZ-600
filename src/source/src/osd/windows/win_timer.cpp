/** @file win_timer.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.21 -

	@brief [ timer ]
*/

#include "win_emu.h"

void EMU_OSD::update_timer()
{
	GetLocalTime(&sTime);
}

void EMU_OSD::get_timer(int *time, size_t size) const
{
/*
	0	year
	1	month
	2	day
	3	day of week
	4	hour
	5	minute
	6	second
	7	milli seconds
*/
	if (size < 8) return;

	time[0] = sTime.wYear;
	time[1] = sTime.wMonth;
	time[2] = sTime.wDay;
	time[3] = sTime.wDayOfWeek;
	time[4] = sTime.wHour;
	time[5] = sTime.wMinute;
	time[6] = sTime.wSecond;
	time[7] = sTime.wMilliseconds;
}

