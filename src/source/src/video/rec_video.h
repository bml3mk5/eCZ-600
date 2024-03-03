/** @file rec_video.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video ]
*/

#ifndef RECORD_VIDEO_H
#define RECORD_VIDEO_H

#include "../rec_video_defs.h"
#include "../common.h"
#include "../cchar.h"
#include "../msgs.h"

#ifdef USE_REC_VIDEO_VFW
class VFW_REC_VIDEO;
#endif
#ifdef USE_REC_VIDEO_MMF
class MMF_REC_VIDEO;
#endif
#ifdef USE_REC_VIDEO_QTKIT
class QT_REC_VIDEO;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
class FFM_REC_VIDEO;
#endif
#ifdef USE_REC_VIDEO_AVKIT
class AVK_REC_VIDEO;
#endif
#ifdef USE_CAP_SCREEN_WIN
class WIN_REC_VIDEO;
#endif
#ifdef USE_CAP_SCREEN_COCOA
class COCOA_REC_VIDEO;
#endif
#ifdef USE_CAP_SCREEN_LIBPNG
class PNG_REC_VIDEO;
#endif

enum en_record_video_type {
	RECORD_VIDEO_TYPE_VFW = 1,
	RECORD_VIDEO_TYPE_QTKIT,
	RECORD_VIDEO_TYPE_FFMPEG,
	RECORD_VIDEO_TYPE_AVKIT,
	RECORD_VIDEO_TYPE_MMF,
	CAPTURE_SCREEN_TYPE = 11,
	SAVE_IMAGE_TYPE,
	RECORD_VIDEO_TYPE_UNKNOWN
};

class EMU;
class CSurface;
class CPixelFormat;

/**
	@brief Record video
*/
class REC_VIDEO
{
private:
	EMU *emu;
	bool now_recording;
	int rec_type;
	int rec_fps;
	int rec_count;
	int rec_denomi;
	int rec_fps_no;
	_TCHAR rec_path[_MAX_PATH];

	VmRectWH m_srcrect;
	CSurface *rec_surface;

#ifdef USE_REC_VIDEO_VFW
	VFW_REC_VIDEO *vfwvideo;
#endif
#ifdef USE_REC_VIDEO_MMF
	MMF_REC_VIDEO *mmfvideo;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	QT_REC_VIDEO *qtvideo;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	FFM_REC_VIDEO *ffmvideo;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	AVK_REC_VIDEO *avkvideo;
#endif
#ifdef USE_CAP_SCREEN_WIN
	WIN_REC_VIDEO *winvideo;
#endif
#ifdef USE_CAP_SCREEN_COCOA
	COCOA_REC_VIDEO *cocvideo;
#endif
#ifdef USE_CAP_SCREEN_LIBPNG
	PNG_REC_VIDEO *pngvideo;
#endif

	bool CreateSurfaceForBMP(const VmRectWH &dstrect, const CSurface &srcsurface, CSurface &dstsurface);
	bool CreateSurfaceForPNG(const VmRectWH &dstrect, const CSurface &srcsurface, CSurface &dstsurface);

	bool SaveBMP(CSurface *surface, CTchar &file_name);
	bool SavePNG(int type, CSurface *surface, CTchar &file_name);

public:
	REC_VIDEO(EMU *new_emu);
	~REC_VIDEO();

	/// true:OK false:ERROR
	bool Start(int type, int fps_no, const VmRectWH &srcrect, CSurface *srcsurface, bool show_dialog);
	void Stop();
	bool Restart();
	bool Record(const VmRectWH &srcrect, CSurface *srcsurface, const VmRectWH &dstrect);
	bool IsRecordFrame();
	bool NowRecording() {
		return now_recording;
	}
	bool *GetNowRecordingPtr() {
		return &now_recording;
	}
	int  GetFpsNum() const { return rec_fps_no; }
	bool Capture(int type, const VmRectWH &srcrect, CSurface *srcsurface, const VmRectWH &dstrect, const _TCHAR *prefix = NULL, const _TCHAR *postfix = NULL);
	void CreateFileName(_TCHAR *file_path, const char *extension, const _TCHAR *prefix = NULL, const _TCHAR *postfix = NULL);

	bool IsEnabled(int type);
	const _TCHAR **GetCodecList(int type);
	const CMsg::Id *GetQualityList(int type);

	const VmRectWH &GetSrcRect() const { return m_srcrect; }

};

#endif /* RECORD_VIDEO_H */
