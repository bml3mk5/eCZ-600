/** @file windowmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ window mode ]
*/

#include "windowmode.h"
#include "../emu.h"
#include <algorithm>
#include <stdlib.h>
//#include "../main.h"

static const VmSize window_proposal[] = {
	{640,400},{640,480},{800,600},{1024,768},{0,0}
};

CWindowMode::CWindowMode()
{
	power = 10;
	width = 0;
	height = 0;
}

CWindowMode::CWindowMode(int power_, int width_, int height_)
{
	Set(power_, width_, height_);
}

CWindowMode::~CWindowMode()
{
}

void CWindowMode::Set(int power_, int width_, int height_)
{
	power = power_;
	width = width_ * power_ / 10;
	height = height_ * power_ / 10;
}

bool CWindowMode::Match(int power_, int width_, int height_) const
{
	return (power == power_
		&& width == (width_ * power_ / 10)
		&& height == (height_ * power_ / 10));
}

//

CWindowModes::CWindowModes()
	: CPtrList<CWindowMode>()
{
}

bool CWindowModes::greater(const CWindowMode *a, const CWindowMode *b)
{
	int rc = 0;
	if (!rc) rc = (a->width - b->width);
	if (!rc) rc = (a->height - b->height);
	if (!rc) rc = (a->power - b->power);
	return (rc < 0);
}

void CWindowModes::Sort()
{
	std::sort(items.begin(), items.end(), greater);
}

int CWindowModes::Find(int power_, int width_, int height_) const
{
	for(int i=0; i<Count(); i++) {
		if (Item(i)->Match(power_, width_, height_)) {
			return i;
		}
	}
	return -1;
}

//

WindowMode::WindowMode()
{
}

WindowMode::~WindowMode()
{
}

/// enumerate screen mode for window
/// calcurate magnify range
/// @param [in] max_width
/// @param [in] max_height
void WindowMode::Enum(int max_width, int max_height)
{
	// enumerate screen mode for window
	int w,h;
	bool minsize = false;
	bool maxsize = false;

	for (int i=0; window_proposal[i].w != 0 && window_modes.Count() < WINDOW_MODE_MAX; i++) {
		w = window_proposal[i].w;
		h = window_proposal[i].h;
		// minimum window size?
		if (w < MIN_WINDOW_WIDTH || h < MIN_WINDOW_HEIGHT) {
			continue;
		}
		if (!minsize) {
			w = MIN_WINDOW_WIDTH;
			h = MIN_WINDOW_HEIGHT;
			i--;
			minsize = true;
		}
		if (w > MAX_WINDOW_WIDTH || h > MAX_WINDOW_HEIGHT) {
			if (!maxsize) {
				w = MAX_WINDOW_WIDTH;
				h = MAX_WINDOW_HEIGHT;
				maxsize = true;
				i--;
			} else {
				break;
			}
		}
		for (int pow = 10; pow <= 30 && window_modes.Count() < WINDOW_MODE_MAX; pow+=5) {
			if ((w*pow/10) <= max_width && (h*pow/10) <= max_height) {
				int found = window_modes.Find(pow, w, h);
				if (found < 0) {
					window_modes.Add(new CWindowMode(pow, w, h));
					logging->out_debugf(_T("window_mode:%d %dx%d x%.1f"),window_modes.Count(), w * pow / 10, h * pow / 10, (double)pow / 10.0);
				}
			} else {
				break;
			}
		}
	}
	window_modes.Sort();
}

const CWindowMode *WindowMode::Get(int num) const
{
	if (0 <= num && num < window_modes.Count()) {
		return window_modes[num];
	}
	return NULL;
}

int WindowMode::Find(int width, int height) const
{
	return window_modes.Find(10, width, height);
}
