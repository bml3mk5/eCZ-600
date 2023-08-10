/** @file png_rec_video.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.10 -

 @brief [ record video using libpng ]
 */

#ifndef PNG_RECORD_VIDEO_H
#define PNG_RECORD_VIDEO_H

#include "../../rec_video_defs.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_LIBPNG)

#include "../../common.h"

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Save PNG picture using libpng
*/
class PNG_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;

public:
	PNG_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~PNG_REC_VIDEO();

	bool Capture(int type, CSurface *surface, const char *file_name);

};

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_LIBPNG */

#endif /* PNG_RECORD_VIDEO_H */
