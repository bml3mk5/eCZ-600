/** @file win_rec_video.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.03 -

 @brief [ record video using GDI+ ]
 */

#ifndef _WIN_RECORD_VIDEO_H_
#define _WIN_RECORD_VIDEO_H_

#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_WIN)

#include <windows.h>

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Save PNG picture using GDI+ on Windows
*/
class WIN_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;
    ULONG_PTR gdiplusToken;

	int GetEncoder(const wchar_t *desc, CLSID &clsid);

public:
	WIN_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~WIN_REC_VIDEO();

	bool Capture(int type, CSurface *surface, const wchar_t *file_name);

};

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_WIN */

#endif /* _WIN_RECORD_VIDEO_H_ */
