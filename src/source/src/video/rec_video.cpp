/** @file rec_video.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ record video ]
*/

#include "../rec_video_defs.h"
#if defined(USE_WIN)
#include <Windows.h>
#elif defined(USE_WX) || defined(USE_WX2)
#include <wx/wx.h>
#elif defined(USE_QT)
#include <QPainter>
#endif
#include "rec_video.h"
#include "rec_common.h"
#include "../emu.h"
#include "../config.h"
#include "../utility.h"
#include "../utils.h"
#include "../csurface.h"

#ifdef USE_REC_VIDEO
const int fps[6] = {60, 30, 20, 15, 12, 10};

#ifdef USE_REC_VIDEO_VFW
#include "vfw/vfw_rec_video.h"
#endif
#ifdef USE_REC_VIDEO_MMF
#include "mmf/mmf_rec_video.h"
#endif
#ifdef USE_REC_VIDEO_QTKIT
#include "qtkit/qt_rec_video.h"
#endif
#ifdef USE_REC_VIDEO_FFMPEG
#include "ffmpeg/ffm_rec_video.h"
#endif
#ifdef USE_REC_VIDEO_AVKIT
#include "avkit/avk_rec_video.h"
#endif
#endif /* USE_REC_VIDEO */

#ifdef USE_CAPTURE_SCREEN_PNG
#ifdef USE_CAP_SCREEN_WIN
#include "windows/win_rec_video.h"
#endif
#ifdef USE_CAP_SCREEN_COCOA
#include "cocoa/cocoa_rec_video.h"
#endif
#ifdef USE_CAP_SCREEN_LIBPNG
#include "libpng/png_rec_video.h"
#endif
#endif /* USE_CAPTURE_SCREEN_PNG */

REC_VIDEO::REC_VIDEO(EMU *new_emu)
{
	emu = new_emu;
	now_recording = false;
	rec_type = 0;
	rec_fps = 0;
	rec_count = 0;
	rec_denomi = 1;
	rec_fps_no = -1;
	memset(rec_path, 0, sizeof(rec_path));

	rec_surface = new CSurface();

#ifdef USE_REC_VIDEO
#ifdef USE_REC_VIDEO_VFW
	vfwvideo = new VFW_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_REC_VIDEO_MMF
	mmfvideo = new MMF_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_REC_VIDEO_QTKIT
	qtvideo = new QT_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	ffmvideo = new FFM_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_REC_VIDEO_AVKIT
	avkvideo = new AVK_REC_VIDEO(new_emu, this);
#endif
#endif /* USE_REC_VIDEO */

#ifdef USE_CAPTURE_SCREEN_PNG
#ifdef USE_CAP_SCREEN_WIN
	winvideo = new WIN_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_CAP_SCREEN_COCOA
	cocvideo = new COCOA_REC_VIDEO(new_emu, this);
#endif
#ifdef USE_CAP_SCREEN_LIBPNG
	pngvideo = new PNG_REC_VIDEO(new_emu, this);
#endif
#endif /* USE_CAPTURE_SCREEN_PNG */
}

REC_VIDEO::~REC_VIDEO()
{
#ifdef USE_REC_VIDEO
#ifdef USE_REC_VIDEO_FFMPEG
	delete ffmvideo;
#endif
#ifdef USE_REC_VIDEO_MMF
	delete mmfvideo;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	delete qtvideo;
#endif
#ifdef USE_REC_VIDEO_VFW
	delete vfwvideo;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	delete avkvideo;
#endif
#endif /* USE_REC_VIDEO */

#ifdef USE_CAPTURE_SCREEN_PNG
#ifdef USE_CAP_SCREEN_WIN
	delete winvideo;
#endif
#ifdef USE_CAP_SCREEN_COCOA
	delete cocvideo;
#endif
#ifdef USE_CAP_SCREEN_LIBPNG
	delete pngvideo;
#endif
#endif /* USE_CAPTURE_SCREEN_PNG */
	delete rec_surface;
}

bool REC_VIDEO::IsEnabled(int type)
{
	bool rc = false;
#ifdef USE_REC_VIDEO
	switch(type) {
#ifdef USE_REC_VIDEO_VFW
	case RECORD_VIDEO_TYPE_VFW:
		rc = true;
		break;
#endif
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		rc = mmfvideo->IsEnabled();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		rc = qtvideo->IsEnabled();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		rc = ffmvideo->IsEnabled();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		rc = avkvideo->IsEnabled();
		break;
#endif
	default:
		break;
	}
#endif
	return rc;
}

const _TCHAR **REC_VIDEO::GetCodecList(int type)
{
	const _TCHAR **list = NULL;
#ifdef USE_REC_VIDEO
	switch(type) {
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		list = mmfvideo->GetCodecList();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		list = qtvideo->GetCodecList();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		list = ffmvideo->GetCodecList();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		list = avkvideo->GetCodecList();
		break;
#endif
	default:
		break;
	}
#endif
	return list;
}

const CMsg::Id *REC_VIDEO::GetQualityList(int type)
{
	const CMsg::Id *list = NULL;
#ifdef USE_REC_VIDEO
	switch(type) {
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		list = mmfvideo->GetQualityList();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		list = qtvideo->GetQualityList();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		list = ffmvideo->GetQualityList();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		list = avkvideo->GetQualityList();
		break;
#endif
	default:
		break;
	}
#endif
	return list;
}

bool REC_VIDEO::Start(int type, int fps_no, const VmRectWH &srcrect, CSurface *srcsurface, bool show_dialog)
{
#ifdef USE_REC_VIDEO
	if (type > 0) {
		rec_type = type;
	}
	if(fps_no >= 0) {
		rec_fps_no = fps_no;
		rec_fps = fps[fps_no];
		rec_count = 0;
		rec_denomi = 60 / fps[fps_no];
	}

	if (rec_type <= 0 || rec_fps_no < 0 || rec_fps <= 0) {
		rec_fps_no = -1;
		return false;
	}

	gRecCommon.CreateFileName(true, rec_path, sizeof(rec_path) / sizeof(rec_path[0]), NULL);

	switch(rec_type) {
#ifdef USE_REC_VIDEO_VFW
	case RECORD_VIDEO_TYPE_VFW:
		rec_surface->Release();
		if (!rec_surface->Create(srcrect, CPixelFormat::BGRA32)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
		now_recording = vfwvideo->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), rec_fps, &srcrect, rec_surface, show_dialog);
		break;
#endif
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		rec_surface->Release();
		if (!rec_surface->Create(srcrect, CPixelFormat::RGBA32)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
		now_recording = mmfvideo->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), rec_fps, &srcrect, rec_surface, show_dialog);
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT: {
		rec_surface->Release();
		if (!rec_surface->Create(srcrect, CPixelFormat::RGBA32)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
		now_recording = qtvideo->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), rec_fps, rec_surface, show_dialog);
	}	break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		rec_surface->Release();
#if defined(USE_WIN)
		// height set minus, so create the flipped surface (top line first)
		if (!rec_surface->Create(srcrect.w, -srcrect.h)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
#else
		if (!rec_surface->Create(srcrect, CPixelFormat::RGBA32)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
#endif
		now_recording = ffmvideo->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), rec_fps, &srcrect, rec_surface, show_dialog);
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT: {
		rec_surface->Release();
		if (!rec_surface->Create(srcrect, CPixelFormat::BGRA32)) {
			logging->out_log(LOG_ERROR, _T("Cannot create surface for recording video."));
			return false;
		}
		now_recording = avkvideo->Start(rec_path, sizeof(rec_path) / sizeof(rec_path[0]), rec_fps, rec_surface, show_dialog);
	}	break;
#endif
	default:
		break;
	}
#endif
	m_srcrect = srcrect;
	if (!now_recording) {
		rec_fps_no = -1;
	}
	return now_recording;
}

void REC_VIDEO::Stop()
{
	if (!now_recording) return;
#ifdef USE_REC_VIDEO
	switch(rec_type) {
#ifdef USE_REC_VIDEO_VFW
	case RECORD_VIDEO_TYPE_VFW:
		vfwvideo->Stop();
		break;
#endif
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		mmfvideo->Stop();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		qtvideo->Stop();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		ffmvideo->Stop();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		avkvideo->Stop();
		break;
#endif
	default:
		break;
	}
	rec_surface->Release();
#endif
	now_recording = false;
	rec_fps_no = -1;
}

bool REC_VIDEO::Restart()
{
	if (!now_recording) return false;
#ifdef USE_REC_VIDEO
	switch(rec_type) {
#ifdef USE_REC_VIDEO_VFW
	case RECORD_VIDEO_TYPE_VFW:
		now_recording = vfwvideo->Restart();
		break;
#endif
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
		now_recording = mmfvideo->Restart();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		now_recording = qtvideo->Restart();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
		now_recording = ffmvideo->Restart();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		now_recording = avkvideo->Restart();
		break;
#endif
	default:
		break;
	}
#endif
	return now_recording;
}

bool REC_VIDEO::Record(const VmRectWH &srcrect, CSurface *srcsurface, const VmRectWH &dstrect)
{
	if (!now_recording) return false;

#ifdef USE_REC_VIDEO
	// copy surface
	VmRectWH re = { 0, 0, dstrect.w, dstrect.h };

	switch(rec_type) {
#ifdef USE_REC_VIDEO_VFW
	case RECORD_VIDEO_TYPE_VFW:
#if defined(USE_WIN)
		srcsurface->Blit(srcrect, *rec_surface, re);
#elif defined(USE_WX) || defined(USE_WX2)
		srcsurface->BlitWithoutAlpha(srcrect, *rec_surface, re);
#elif defined(USE_SDL) || defined(USE_SDL2)
		srcsurface->BlitFlipped(srcrect, *rec_surface, re);
#endif
//		emu->mix_ledbox(rec_surface);
		now_recording = vfwvideo->Record();
		break;
#endif
#ifdef USE_REC_VIDEO_MMF
	case RECORD_VIDEO_TYPE_MMF:
#if defined(USE_WIN)
		srcsurface->Blit(srcrect, *rec_surface, re);
#elif defined(USE_WX) || defined(USE_WX2)
		srcsurface->BlitWithoutAlpha(srcrect, *rec_surface, re);
#elif defined(USE_SDL) || defined(USE_SDL2)
		srcsurface->BlitFlipped(srcrect, *rec_surface, re);
#endif
//		emu->mix_ledbox(rec_surface);
		now_recording = mmfvideo->Record();
		break;
#endif
#ifdef USE_REC_VIDEO_QTKIT
	case RECORD_VIDEO_TYPE_QTKIT:
		srcsurface->Blit(srcrect, *rec_surface, re);
//		emu->mix_ledbox(rec_surface);
		now_recording = qtvideo->Record();
		break;
#endif
#ifdef USE_REC_VIDEO_FFMPEG
	case RECORD_VIDEO_TYPE_FFMPEG:
#if defined(USE_WX) || defined(USE_WX2)
		srcsurface->BlitFlippedWithoutAlpha(srcrect, *rec_surface, re);
#else
		srcsurface->Blit(srcrect, *rec_surface, re);
#endif
//		emu->mix_ledbox(rec_surface);
		now_recording = ffmvideo->Record();
		break;
#endif
#ifdef USE_REC_VIDEO_AVKIT
	case RECORD_VIDEO_TYPE_AVKIT:
		srcsurface->Blit(srcrect, *rec_surface, re);
//		emu->mix_ledbox(rec_surface);
		now_recording = avkvideo->Record();
		break;
#endif
	default:
		break;
	}
#endif
	return now_recording;
}

bool REC_VIDEO::IsRecordFrame()
{
	if (!now_recording) return false;

	bool rc = (rec_count == 0);
	rec_count = ((rec_count + 1) % rec_denomi);
	return rc;
}

static const char *capture_file_ext[] = {
	"bmp",
	"png",
	NULL
};

bool REC_VIDEO::Capture(int type, const VmRectWH &srcrect, CSurface *srcsurface, const VmRectWH &dstrect, const _TCHAR *prefix, const _TCHAR *postfix)
{
	if (srcsurface == NULL) {
		return false;
	}

	bool rc = false;
	CSurface bmpsurface;
	_TCHAR file_path[_MAX_PATH];
	VmRectWH re = { 0, 0, dstrect.w, dstrect.h };

#ifndef USE_CAPTURE_SCREEN_PNG
	pConfig->capture_type = 0;
#endif
	if (1 < pConfig->capture_type) {
		pConfig->capture_type = 0;
	}

	// add file name
	gRecCommon.CreateFileName(true, file_path, _MAX_PATH, capture_file_ext[pConfig->capture_type], prefix, postfix);

	// create surface for capture
	switch(pConfig->capture_type) {
	case 1:	// PNG
		rc = CreateSurfaceForPNG(dstrect, *srcsurface, bmpsurface);
		break;
	default: // BMP
		rc = CreateSurfaceForBMP(dstrect, *srcsurface, bmpsurface);
		break;
	}
	if (!rc) {
		logging->out_log(LOG_ERROR,_T("Capture screen failed. Cannot create surface."));
		return false;
	}

	// copy surface
	if (srcrect.w == dstrect.w && srcrect.h == dstrect.h) {
#if defined(USE_WX) || defined(USE_WX2)
		srcsurface->BlitWithoutAlpha(srcrect, bmpsurface, re);
#else
		srcsurface->Blit(srcrect, bmpsurface, re);
#endif
	} else {
		srcsurface->StretchBlit(srcrect, bmpsurface, re);
	}

	CTchar cfile_path(file_path);

	switch(pConfig->capture_type) {
	case 1:	// PNG
		// save surface
		rc = SavePNG(type, &bmpsurface, cfile_path);
		break;
	default: // BMP
		// save surface
		rc = SaveBMP(&bmpsurface, cfile_path);
		break;
	}
	if (rc) {
		if (type == CAPTURE_SCREEN_TYPE) {
			logging->out_log_x(LOG_INFO, CMsg::Screen_was_saved_successfully);
		}
	} else {
		logging->out_log(LOG_ERROR, _T("Cannot save screen captured file."));
	}

	return rc;
}

bool REC_VIDEO::CreateSurfaceForBMP(const VmRectWH &dstrect, const CSurface &srcsurface, CSurface &dstsurface)
{
	bool rc = false;
#if defined(USE_WIN)
	rc = dstsurface.Create(dstrect.w, dstrect.h);
#elif defined(USE_SDL) || defined(USE_SDL2)
	rc = dstsurface.Create(dstrect, srcsurface.GetPixelFormat());
#elif defined(USE_WX) || defined(USE_WX2)
	rc = dstsurface.Create(dstrect);
#elif defined(USE_QT)
	rc = dstsurface.Create(dstrect, srcsurface.GetPixelFormat());
#endif
	return rc;
}

bool REC_VIDEO::CreateSurfaceForPNG(const VmRectWH &dstrect, const CSurface &srcsurface, CSurface &dstsurface)
{
	bool rc = false;
#if defined(USE_CAP_SCREEN_WIN)
#if defined(USE_WIN)
	rc = dstsurface.Create(dstrect.w, -dstrect.h); // flipped
#else
	// notice that format always use BGRA
	rc = dstsurface.Create(dstrect, CPixelFormat::BGRA32);
#endif
#elif defined(USE_CAP_SCREEN_COCOA)
	// notice that format always use RGBA
	rc = dstsurface.Create(dstrect, CPixelFormat::ARGB32);
#elif defined(USE_CAP_SCREEN_LIBPNG)
	// notice that format always use RGBA
	rc = dstsurface.Create(dstrect, CPixelFormat::RGBA32);
#elif defined(USE_CAP_SCREEN_WX)
	rc = dstsurface.Create(dstrect);
#elif defined(USE_CAP_SCREEN_QT)
	rc = dstsurface.Create(dstrect, srcsurface.GetPixelFormat());
#endif
	return rc;
}

bool REC_VIDEO::SaveBMP(CSurface *surface, CTchar &file_name)
{
#if defined(USE_WIN)
	DWORD dwSize;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BITMAPFILEHEADER bmFileHeader = { (WORD)(TEXT('B') | TEXT('M') << 8) };

	bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmFileHeader.bfSize = bmFileHeader.bfOffBits + surface->GetBufferSize();

	hFile = CreateFile(file_name.Get(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		logging->out_syserrlog(LOG_ERROR,GetLastError(),_T("Capture screen failed."));
		return false;
	}
	WriteFile(hFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwSize, NULL);
	WriteFile(hFile, surface->GetHeader(), sizeof(BITMAPINFOHEADER), &dwSize, NULL);
	WriteFile(hFile, surface->GetBuffer(), surface->GetBufferSize(), &dwSize, NULL);
	CloseHandle(hFile);

	return true;
#elif defined(USE_SDL) || defined(USE_SDL2)
	return (SDL_SaveBMP(surface->Get(), file_name.GetN()) >= 0);
#elif defined(USE_WX) || defined(USE_WX2)
	return surface->Get()->SaveFile(file_name.Get(), wxBITMAP_TYPE_BMP);
#elif defined(USE_QT)
	QString cfile_name(file_name.GetN());
	// Format is determined to extension (suffix) of the filename.
	return (surface->Get()->save(cfile_name));
#else
	return false;
#endif
}

bool REC_VIDEO::SavePNG(int type, CSurface *surface, CTchar &file_name)
{
#if defined(USE_CAP_SCREEN_WIN)
	return winvideo->Capture(type, surface, file_name.GetWM());
#elif defined(USE_CAP_SCREEN_COCOA)
	return cocvideo->Capture(type, surface, file_name.GetN());
#elif defined(USE_CAP_SCREEN_LIBPNG)
	// fill alpha channel
	surface->Fill(0xffffffff, surface->GetPixelFormat().Amask);

	return pngvideo->Capture(type, surface, file_name.GetN());
#elif defined(USE_CAP_SCREEN_WX)
	return surface->Get()->SaveFile(file_name.Get(), wxBITMAP_TYPE_PNG);
#elif defined(USE_CAP_SCREEN_QT)
	QString cfile_name(file_name.GetN());
	// Format is determined to extension (suffix) of the filename.
	return (surface->Get()->save(cfile_name));
#else
	return false;
#endif
}
