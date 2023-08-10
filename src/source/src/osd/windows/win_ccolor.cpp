/** @file win_ccolor.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#include "win_ccolor.h"
#include "../../cpixfmt.h"

CColor::CColor()
{
}

CColor::CColor(const CColor &src)
{
	m_color = src.m_color;
}

CColor::~CColor()
{
}

void CColor::Set(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	m_color = RGB(red, green, blue);
}

void CColor::Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	m_color = RGB(red, green, blue);
}

const COLORREF &CColor::Get() const
{
	return m_color;
}
