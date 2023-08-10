/** @file sdl_timer.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.2.21

	@brief [ sdl timer ]

	@note
	This code is based on win32_timer.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "sdl_emu.h"
#include <time.h>

void EMU_OSD::update_timer()
{
//	GetLocalTime(&sTime);
	time_t t;

	time(&t);
	struct tm *tim = localtime(&t);
	memcpy(&sTime, tim, sizeof(struct tm));
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

	time[0] = sTime.tm_year + 1900;
	time[1] = sTime.tm_mon + 1;
	time[2] = sTime.tm_mday;
	time[3] = sTime.tm_wday;
	time[4] = sTime.tm_hour;
	time[5] = sTime.tm_min;
	time[6] = sTime.tm_sec;
	time[7] = 0;	// TODO
}

