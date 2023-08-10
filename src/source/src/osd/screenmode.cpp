/** @file screenmode.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/


#include "screenmode.h"
#include <algorithm>
#include "../vm/vm_defs.h"

//

CDisplayDevice::CDisplayDevice()
{
	disp_no = -1;
	RECT_IN(re, 0, 0, 0, 0);
}

CDisplayDevice::~CDisplayDevice()
{
}

//

CVideoMode::CVideoMode()
{
	width = 0;
	height = 0;
}

CVideoMode::~CVideoMode()
{
}

void CVideoMode::Set(int width_, int height_)
{
	width = width_;
	height = height_;
}

bool CVideoMode::Match(int width_, int height_) const
{
	return (width == width_ && height == height_);
}

CVideoModes::CVideoModes()
	: CPtrList<CVideoMode>()
{
}

bool CVideoModes::greater(const CVideoMode *a, const CVideoMode *b)
{
	int rc = 0;
	int aa = a->width * a->height;
	int bb = b->width * b->height;
//	if (!rc) rc = (a->disp_no - b->disp_no);
	if (!rc) rc = (bb - aa);
	return (rc < 0);
}

void CVideoModes::Sort()
{
	std::sort(items.begin(), items.end(), greater);
}

int CVideoModes::Find(int width_, int height_) const
{
	for(int i=0; i<Count(); i++) {
		if (Item(i)->Match(width_, height_)) {
			return i;
		}
	}
	return -1;
}

bool CVideoModes::IsValidSize(int width_, int height_)
{
	if(width_ < MIN_WINDOW_WIDTH || height_ < MIN_WINDOW_HEIGHT || width_ < 640 || height_ < 480) {
		return false;
	}
	if(((width_ * 30 / height_) != 40)
	&& ((width_ * 40 / height_) != 50)
	&& ((width_ * 90 / height_) != 160)
	&& ((width_ * 50 / height_) != 80)) {
		return false;
	}
	if (width_ <= 1280 && (width_ != 640 && width_ != 800 && width_ != 1024 && width_ != 1280)) {
		return false;
	}
	return true;
}

//

ScreenModeBase::ScreenModeBase()
{
	major_bits_per_pixel = _RGB888;
}

ScreenModeBase::~ScreenModeBase()
{
}

int ScreenModeBase::CountMode(int disp_no) const
{
	if (disp_no < 0 || disp_no >= disp_devices.Count()) return 0;

	return disp_devices[disp_no]->modes.Count();
}

const CVideoMode *ScreenModeBase::GetMode(int disp_no, int num) const
{
	if (disp_no < 0 || disp_no >= disp_devices.Count()) return NULL;
	if (num < 0 || num >= disp_devices[disp_no]->modes.Count()) return NULL;
	return disp_devices[disp_no]->modes[num];
}

int ScreenModeBase::FindMode(int disp_no, int width, int height) const
{
	if (disp_no < 0 || disp_no >= disp_devices.Count()) return -1;
	return disp_devices[disp_no]->modes.Find(width, height);
}

int ScreenModeBase::CountDisp() const
{
	return disp_devices.Count();
}

const CDisplayDevice *ScreenModeBase::GetDisp(int num) const
{
	if (0 <= num && num < disp_devices.Count()) {
		return disp_devices[num];
	}
	return NULL;
}

void ScreenModeBase::GetDispSize(int num, int *width, int *height, int *bits_per_pixel) const
{
	if (width) *width = disp_devices[num]->re.w;
	if (height) *height = disp_devices[num]->re.h;
	if (bits_per_pixel) *bits_per_pixel = major_bits_per_pixel;
}

int ScreenModeBase::WithinDisp(int x, int y) const
{
	int match = -1;
	for (int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		const VmRectWH *re = &disp_devices[disp_no]->re;

		if ((re->x <= x && x < (re->x + re->w - 96))
		 && (re->y <= y && y < (re->y + re->h - 96))) {
			match = disp_no;
			break;
		}
	}
	return match;
}

void ScreenModeBase::GetVirtualDispSize(int *width, int *height, int *bits_per_pixel) const
{
	int left = 0x7fffffff;
	int top = 0x7fffffff;
	int right = 0x80000000;
	int bottom = 0x80000000;
	for (int disp_no = 0; disp_no < disp_devices.Count(); disp_no++) {
		const VmRectWH *re = &disp_devices[disp_no]->re;
		if (re->x < left) left = re->x;
		if (re->y < top) top = re->y;
		if ((re->x + re->w - 1) > right) right = (re->x + re->w - 1);
		if ((re->y + re->h - 1) > bottom) bottom = (re->y + re->h - 1);
	}
	if (width) {
		*width = right - left + 1;
		if (*width < 0) *width = 0;
	}
	if (height) {
		*height = bottom - top + 1;
		if (*height < 0) *height = 0;
	}
	if (bits_per_pixel) *bits_per_pixel = major_bits_per_pixel;
}
