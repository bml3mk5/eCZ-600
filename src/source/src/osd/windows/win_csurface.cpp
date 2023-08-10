/** @file win_csurface.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2013.10.30 -

	@brief [ csurface ]
*/
#include "win_csurface.h"

CSurface::CSurface()
{
	hMainDC = NULL;
	hMainBuf = NULL;
	hMainBmp = NULL;
	pMainBuf = NULL;

	enable = false;
}
CSurface::CSurface(long width, long height)
{
	hMainDC = NULL;
	hMainBuf = NULL;
	hMainBmp = NULL;
	pMainBuf = NULL;

	enable = false;

	Create(width, height);
}
CSurface::CSurface(long width, long height, const CPixelFormat &pixel_format)
{
	hMainDC = NULL;
	hMainBuf = NULL;
	hMainBmp = NULL;
	pMainBuf = NULL;

	enable = false;

	Create(width, height, pixel_format);
}
CSurface::~CSurface()
{
	if (enable) {
		Release();
	}
}

// Create surface
bool CSurface::Create(HDC hdc, long width, long height, HANDLE *hBuf, HBITMAP *hBmp, scrntype **pBuf, HDC *hdcDib)
{
	LPBITMAPINFO bmi;

	if (*hBuf != NULL) {
		Release(hBuf, hBmp, hdcDib);
	}

	*hBuf = GlobalAlloc(GPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (*hBuf == NULL) {
		enable = false;
		return false;
	}

	bmi = (LPBITMAPINFO)(*hBuf);

	memset(&(bmi->bmiHeader), 0, sizeof(BITMAPINFOHEADER));

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = height;
	bmi->bmiHeader.biPlanes = 1;
#if defined(_RGB555)
	bmi->bmiHeader.biBitCount = 16;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB565)
	bmi->bmiHeader.biBitCount = 16;
	bmi->bmiHeader.biCompression = BI_BITFIELDS;
	LPDWORD lpBf = (LPDWORD)*lpDib->bmiColors;
	lpBf[0] = 0x1f << 11;
	lpBf[1] = 0x3f << 5;
	lpBf[2] = 0x1f << 0;
	bmi->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB888)
	bmi->bmiHeader.biBitCount = 32;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = width * height * 4;
#endif

	*hBmp = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, (VOID **)pBuf, NULL, 0);
	if (*hBmp == NULL) {
		enable = false;
		return false;
	}
	*hdcDib = CreateCompatibleDC(hdc);
	SelectObject(*hdcDib, *hBmp);

	enable = true;
	return true;
}
bool CSurface::Create(long width, long height)
{
	bool rc = false;
	HDC hdc = ::GetDC(hMainWindow);
	rc = Create(hdc, width, height, &hMainBuf, &hMainBmp, &pMainBuf, &hMainDC);
	::ReleaseDC(hMainWindow, hdc);
	return rc;
}
bool CSurface::Create(long width, long height, const CPixelFormat &UNUSED_PARAM(srcformat))
{
	return Create(width, height);
}
bool CSurface::Create(const VmRectWH &srcrect)
{
	return Create(srcrect.w, srcrect.h);
}
bool CSurface::Create(const VmRectWH &srcrect, const CPixelFormat &UNUSED_PARAM(srcformat))
{
	return Create(srcrect.w, srcrect.h);
}

// release surface
void CSurface::Release(HANDLE *hBuf, HBITMAP *hBmp, HDC *hdcDib)
{
	if (*hdcDib) {
		DeleteDC(*hdcDib);
		*hdcDib = NULL;
	}
	if (*hBmp) {
		DeleteObject(*hBmp);
		*hBmp = NULL;
	}
	if (*hBuf) {
		GlobalFree(*hBuf);
		*hBuf = NULL;
	}
	enable = false;
}

void CSurface::Release()
{
	Release(&hMainBuf, &hMainBmp, &hMainDC);
}

scrntype *CSurface::GetBuffer()
{
	return (scrntype *)pMainBuf;
}

scrntype *CSurface::GetBuffer(int y)
{
	scrntype *p = (scrntype *)pMainBuf;
	p += (Width() * y);
	return p;
}

int CSurface::GetBufferSize()
{
	if (!hMainBuf) return 0;
	LPBITMAPINFO bmi = (LPBITMAPINFO)hMainBuf;
	return (int)bmi->bmiHeader.biSizeImage;
}

bool CSurface::IsEnable()
{
	return enable;
}

int CSurface::Width()
{
	LPBITMAPINFO bmi = (LPBITMAPINFO)hMainBuf;
	return (int)(bmi->bmiHeader.biWidth);
}

int CSurface::Height()
{
	LPBITMAPINFO bmi = (LPBITMAPINFO)hMainBuf;
	return (int)abs(bmi->bmiHeader.biHeight);
}

LPBITMAPINFOHEADER CSurface::GetHeader()
{
	if (!hMainBuf) return NULL;
	LPBITMAPINFO bmi = (LPBITMAPINFO)hMainBuf;
	return &bmi->bmiHeader;
}

DWORD CSurface::GetHeaderSize()
{
	if (!hMainBuf) return 0;
	LPBITMAPINFO bmi = (LPBITMAPINFO)hMainBuf;
	return bmi->bmiHeader.biSize + bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
}

bool CSurface::Lock()
{
	return true;
}

void CSurface::Unlock()
{
}

bool CSurface::Blit(CSurface &dst)
{
	return (::BitBlt(dst.GetDC(), 0, 0, dst.Width(), dst.Height(), GetDC(), 0, 0, SRCCOPY) == TRUE);
}

bool CSurface::Blit(CSurface &dst, const VmRectWH &dst_re)
{
	return (::BitBlt(dst.GetDC(), dst_re.x, dst_re.y, dst_re.w, dst_re.h, GetDC(), 0, 0, SRCCOPY) == TRUE);
}

bool CSurface::Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	return (::BitBlt(dst.GetDC(), dst_re.x, dst_re.y, dst_re.w, dst_re.h, GetDC(), src_re.x, src_re.y, SRCCOPY) == TRUE);
}

bool CSurface::StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re)
{
	return (::StretchBlt(dst.GetDC(), dst_re.x, dst_re.y, dst_re.w, dst_re.h, GetDC(), src_re.x, src_re.y, src_re.w, src_re.h, SRCCOPY) == TRUE);
}
