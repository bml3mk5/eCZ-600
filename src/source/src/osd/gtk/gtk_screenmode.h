/** @file gtk_screenmode.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.11

	@brief [ screen mode ]
*/

#ifndef GTK_SCREENMODE_H
#define GTK_SCREENMODE_H

#include "../../common.h"


/// remain size on each displays
class ScreenMode : public ScreenModeBase
{
public:
	ScreenMode();
	~ScreenMode();

	void Enum();
	void Enum(int desktop_width, int desktop_height, int bits_per_pixel);

	static void GetDesktopSize(int *width, int *height, int *bpp);
};

#endif /* GTK_SCREENMODE_H */
