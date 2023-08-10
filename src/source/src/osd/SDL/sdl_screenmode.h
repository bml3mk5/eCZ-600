/** @file sdl_screenmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.01

	@brief [ screen mode ]
*/

#ifndef SDL_SCREEN_MODE_H
#define SDL_SCREEN_MODE_H

#include "../../common.h"


/**
	@brief Remain size on each displays
*/
class ScreenMode : public ScreenModeBase
{
public:
	ScreenMode();
	~ScreenMode();

	void Enum();
	void Enum(int desktop_width, int desktop_height, int bits_per_pixel);

	static void GetDesktopSize(int *width, int *height, int *bpp);
};

#endif /* SDL_SCREEN_MODE_H */
