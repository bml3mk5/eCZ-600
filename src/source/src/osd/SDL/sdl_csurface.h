/** @file sdl_csurface.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.03 -

	@brief [ csurface ]
*/

#ifndef SDL_CSURFACE_H
#define SDL_CSURFACE_H

#include "../../common.h"
#include "../../cpixfmt.h"
#include <SDL.h>

#ifdef USE_GTK
#include <cairo/cairo.h>
#endif

#define CP_RECT(src, dst) { \
	(dst).x = (src).x; \
	(dst).y = (src).y; \
	(dst).w = (src).w; \
	(dst).h = (src).h; \
}

// ----------------------------------------------------------------------------

class CSurface;

/**
	@brief manage CSDLSurface
*/
class CSDLSurface
{
protected:
	SDL_Surface *suf;

public:
	CSDLSurface();
	CSDLSurface(long width, long height);
	CSDLSurface(long width, long height, const CPixelFormat &pixel_format);
	CSDLSurface(long width, long height, SDL_PixelFormat *pixel_format);
	CSDLSurface(long width, long height, CPixelFormat::FormatId force_format);
	virtual ~CSDLSurface();

	virtual bool Create(long width, long height);
	virtual bool Create(long width, long height, const CPixelFormat &pixel_format);
	virtual bool Create(long width, long height, SDL_PixelFormat *pixel_format);
	virtual bool Create(long width, long height, CPixelFormat::FormatId force_format);
	virtual bool Create(const VmRectWH &srcrect);
	virtual bool Create(const VmRectWH &srcrect, const CPixelFormat &pixel_format);
	virtual bool Create(const VmRectWH &srcrect, CPixelFormat::FormatId force_format);
	virtual bool Create(const VmRectWH &srcrect, CSDLSurface &srcsurface, CPixelFormat::FormatId force_format = CPixelFormat::NONE);
	virtual bool Create(CSDLSurface &srcsurface, const CPixelFormat &pixel_format);

	virtual void Release();

	scrntype *GetBuffer();
	scrntype *GetBuffer(int y);
	void UngetBuffer();
	int GetBufferSize();

	bool IsEnable();

	int Width();
	int Height();

	int BitsPerPixel();
	int BytesPerPixel();
	int BytesPerLine();

	SDL_Surface *Get();
	SDL_PixelFormat *GetNativePixelFormat();
	CPixelFormat GetPixelFormat() const;

	bool Lock();
	void Unlock();

	bool Blit(CSDLSurface &dst);
	bool Blit(CSDLSurface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re);
	bool Blit(CSDLSurface &dst, SDL_Rect &dst_re);
	bool Blit(SDL_Rect &src_re, CSDLSurface &dst, SDL_Rect &dst_re);
	bool Blit(SDL_Surface &dst);
	bool Blit(SDL_Surface &dst, const VmRectWH &dst_re);
	bool Blit(const VmRectWH &src_re, SDL_Surface &dst, const VmRectWH &dst_re);
	bool Blit(SDL_Surface &dst, SDL_Rect &dst_re);
	bool Blit(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re);

	bool StretchBlit(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re);
	bool StretchBlit(const VmRectWH &src_re, SDL_Surface &dst, const VmRectWH &dst_re);
	bool StretchBlit(SDL_Rect &src_re, CSDLSurface &dst, SDL_Rect &dst_re);
	bool StretchBlit(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re);

	bool BlitFlipped(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re);
	bool BlitFlipped(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re);

	bool StretchBlitFlipped(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re);
	bool StretchBlitFlipped(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re);

	void Fill(scrntype data, scrntype mask);

#ifdef USE_SDL2
	static bool Render(SDL_Renderer &renderer, SDL_Texture &texture, VmRectWH &srcrect, VmRectWH &dstrect);
#endif
};

// ----------------------------------------------------------------------------

/**
	@brief manage CSurface
*/
class CSurface : public CSDLSurface
{
public:
	CSurface();
	CSurface(long width, long height);
	CSurface(long width, long height, const CPixelFormat &pixel_format);
	CSurface(long width, long height, SDL_PixelFormat *pixel_format);
	CSurface(long width, long height, CPixelFormat::FormatId force_format);
};

#ifdef USE_GTK
/**
	@brief manage CCairoSurface
*/
class CCairoSurface
{
protected:
	cairo_surface_t *p_cas;

public:
	CCairoSurface();
	CCairoSurface(CSurface &buf, long width, long height);
	CCairoSurface(CSurface &buf, const VmRectWH &srcrect);
	virtual ~CCairoSurface();

	bool CreateC(CSurface &buf, long width, long height);
	bool CreateC(CSurface &buf, const VmRectWH &srcrect);
	bool CreateC(CSurface &buf, long width, long height, cairo_format_t format);
	bool CreateC(CSurface &buf, const VmRectWH &srcrect, cairo_format_t format);

	void ReleaseC();

	bool BlitC(cairo_t *dst);
	bool BlitC(cairo_t *dst, const VmRectWH &dst_re);
	bool BlitC(cairo_t *dst, const SDL_Rect &dst_re);
	bool BlitC(SDL_Rect &src_re, cairo_t *dst, SDL_Rect &dst_re);

	bool StretchBlitC(const VmRectWH &src_re, cairo_t *dst, const VmRectWH &dst_re);
};
#endif /* USE_GTK */

// ----------------------------------------------------------------------------

#if defined(USE_SDL2) || defined(USE_WX2)
/**
	@brief manage CTexture
*/
class CTexture
{
protected:
	SDL_Renderer *m_renderer;
	SDL_Texture *m_tex;
	scrntype *m_buf;
	int m_w;
	int m_h;

public:
	CTexture();
	CTexture(SDL_Renderer *renderer, long width, long height);
	CTexture(SDL_Renderer *renderer, long width, long height, Uint32 format);
	virtual ~CTexture();

	bool Create(SDL_Renderer *renderer, long width, long height, Uint32 format = 0);

	void Release();

	scrntype *GetBuffer();
	scrntype *GetBuffer(int y);

	bool IsEnable();

	int Width();
	int Height();

	SDL_Texture *Get();
	SDL_Renderer *Renderer();

	bool Lock();
	void Unlock();

	void Fill(scrntype data);
};
#endif /* defined(USE_SDL2) || defined(USE_WX2) */

#endif /* SDL_CSURFACE_H */
