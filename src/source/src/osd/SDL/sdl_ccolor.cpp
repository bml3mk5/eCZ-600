/** @file sdl_ccolor.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#include "sdl_ccolor.h"
#include "../../cpixfmt.h"

CColor::CColor()
{
	memset(&m_color, 0, sizeof(m_color));
}

CColor::CColor(const CColor &src)
{
	m_color = src.m_color;
}

CColor::~CColor()
{
}

void CColor::Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	m_color.r = red;
	m_color.g = green;
	m_color.b = blue;
#if defined(USE_SDL)
	m_color.unused = 0xff;
#elif defined(USE_SDL2)
	m_color.a = alpha;
#endif
}

const SDL_Color &CColor::Get() const
{
	return m_color;
}

Uint32 CColor::Map(const CPixelFormat &format) const
{
	return format.Map(m_color.r, m_color.g, m_color.b);
}
