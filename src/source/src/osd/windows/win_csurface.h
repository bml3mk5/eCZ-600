/** @file win_csurface.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2013.10.30 -

	@brief [ csurface ]
*/

#ifndef WIN_CSURFACE_H
#define WIN_CSURFACE_H

#include <windows.h>
#include "../../common.h"
#include "../../cpixfmt.h"
#include "../../main.h"

/**
	@brief manage DIB section
*/
class CSurface
{
protected:
	HDC       hMainDC;
	HANDLE    hMainBuf;
	HBITMAP   hMainBmp;
	scrntype *pMainBuf;

	bool      enable;

	bool Create(HDC hdc, long width, long height, HANDLE *hBuf, HBITMAP *hBmp, scrntype **pBuf, HDC *hdcDib);
	void Release(HANDLE *hBuf, HBITMAP *hBmp, HDC *hdcDib);

public:
	CSurface();
	CSurface(long width, long height);
	CSurface(long width, long height, const CPixelFormat &pixel_format);
	virtual ~CSurface();

	bool Create(long width, long height);
	bool Create(long width, long height, const CPixelFormat &pixel_format);
	bool Create(const VmRectWH &srcrect);
	bool Create(const VmRectWH &srcrect, const CPixelFormat &pixel_format);
	void Release();

	scrntype *GetBuffer();
	scrntype *GetBuffer(int y);
	void UngetBuffer();
	int GetBufferSize();

	bool IsEnable();

	int Width();
	int Height();

	HDC GetDC() { return hMainDC; }
	HANDLE GetHHeader() { return hMainBuf; }
	HBITMAP GetBitmap() { return hMainBmp; }
	LPBITMAPINFOHEADER GetHeader();
	DWORD GetHeaderSize();

	bool Lock();
	void Unlock();

	bool Blit(CSurface &dst);
	bool Blit(CSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

	bool StretchBlit(const VmRectWH &src_re, CSurface &dst, const VmRectWH &dst_re);

};

#endif /* WIN_CSURFACE_H */
