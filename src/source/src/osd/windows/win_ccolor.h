/** @file win_ccolor.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#ifndef _WIN_CCOLOR_H_
#define _WIN_CCOLOR_H_

#include <windows.h>

class CPixelFormat;

/**
	@brief manage color
*/
class CColor
{
private:
	COLORREF m_color;

public:
	CColor();
	CColor(const CColor &src);
	~CColor();

	void Set(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff);
	void Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff);
	const COLORREF &Get() const;
};

#endif
