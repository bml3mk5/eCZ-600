/** @file win_rec_video.cpp

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.03 -

 @brief [ record video using GDI+ ]
 */

#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#endif
#include "win_rec_video.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_WIN)

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#endif

using namespace Gdiplus;

#include "../../emu.h"
#include "../rec_video.h"
#include "../../csurface.h"

WIN_REC_VIDEO::WIN_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;

    GdiplusStartupInput gdiplusStartupInput;
	gdiplusStartupInput.GdiplusVersion = 1;
	gdiplusStartupInput.DebugEventCallback = NULL;
	gdiplusStartupInput.SuppressBackgroundThread = FALSE;
	gdiplusStartupInput.SuppressExternalCodecs = FALSE;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

WIN_REC_VIDEO::~WIN_REC_VIDEO()
{
    GdiplusShutdown(gdiplusToken);
}

bool WIN_REC_VIDEO::Capture(int type, CSurface *surface, const wchar_t *file_name)
{
	// save PNG file
	CLSID clsid;
	if (GetEncoder(L"PNG", clsid) < 0) {
		return false;
	}

	// create bitmap object
	int stride = 4 * surface->Width();
	PixelFormat format = PixelFormat32bppRGB;
	Bitmap bmp(surface->Width(), surface->Height(), stride, format, (BYTE *)surface->GetBuffer());

	// write to file
	return (bmp.Save(file_name, &clsid, NULL) == Ok);
}

int WIN_REC_VIDEO::GetEncoder(const wchar_t *desc, CLSID &clsid)
{
	Status sts = Ok;
	UINT num = 0, size = 0, match = -1;

	sts = GetImageEncodersSize(&num, &size);
	if (sts != Ok || size == 0) {
		return -1;
	}
	ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)malloc(size);
	if (pImageCodecInfo == NULL) {
		return -1;
	}
	sts = GetImageEncoders(num, size, pImageCodecInfo);
	if (sts == Ok) {
		for(UINT j = 0; j < num; j++) {
			if (wcscmp(pImageCodecInfo[j].FormatDescription, desc) == 0) {
				clsid = pImageCodecInfo[j].Clsid;
				match = j;
				break;
			}
		}
	}
    free(pImageCodecInfo);
    return match;
}

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_WIN */

