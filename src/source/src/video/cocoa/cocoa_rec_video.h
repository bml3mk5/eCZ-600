/** @file cocoa_rec_video.h

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.03 -

 @brief [ record video using Cocoa ]
 */

#ifndef _COCOA_RECORD_VIDEO_H_
#define _COCOA_RECORD_VIDEO_H_

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif
#include "../../rec_video_defs.h"
#include "../../common.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_COCOA)

class EMU;
class REC_VIDEO;
class CSurface;

/**
	@brief Save PNG picture using Cocoa
*/
class COCOA_REC_VIDEO
{
private:
	EMU *emu;
	REC_VIDEO *vid;

public:
	COCOA_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid);
	~COCOA_REC_VIDEO();

	bool Capture(int type, CSurface *surface, const char *file_name);

};

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_COCOA */

#endif /* _COCOA_RECORD_VIDEO_H_ */
