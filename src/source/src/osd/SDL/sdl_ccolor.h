/** @file sdl_ccolor.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01 -

	@brief [ ccolor ]
*/

#ifndef SDL_CCOLOR_H
#define SDL_CCOLOR_H

#include "../../common.h"
#include <SDL.h>

class CPixelFormat;

/**
	@brief manage color
*/
class CColor
{
private:
	SDL_Color m_color;

public:
	CColor();
	CColor(const CColor &src);
	~CColor();

	void Set(const CPixelFormat &format, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff);
	const SDL_Color &Get() const;
	Uint32 Map(const CPixelFormat &format) const;
};

#endif /* SDL_CCOLOR_H */
