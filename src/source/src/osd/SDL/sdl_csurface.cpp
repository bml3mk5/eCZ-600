/** @file sdl_csurface.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.01.03 -

	@brief [ csurface ]
*/
#include "sdl_csurface.h"
//#include "sdl_utils.h"

// ----------------------------------------------------------------------------

CSDLSurface::CSDLSurface()
{
	suf = NULL;
}
CSDLSurface::CSDLSurface(long width, long height)
{
	suf = NULL;

	this->Create(width, height);
}
CSDLSurface::CSDLSurface(long width, long height, const CPixelFormat &pixel_format)
{
	suf = NULL;

	this->Create(width, height, pixel_format);
}
CSDLSurface::CSDLSurface(long width, long height, SDL_PixelFormat *pixel_format)
{
	suf = NULL;

	this->Create(width, height, pixel_format);
}
CSDLSurface::CSDLSurface(long width, long height, CPixelFormat::FormatId force_format)
{
	suf = NULL;

	this->Create(width, height, force_format);
}
CSDLSurface::~CSDLSurface()
{
	Release();
}

// Create surface
bool CSDLSurface::Create(long width, long height)
{
	return Create(width, height, CPixelFormat());
}

// Create surface
bool CSDLSurface::Create(long width, long height, const CPixelFormat &pixel_format)
{
	SDL_PixelFormat format;
	pixel_format.ConvTo(format);

	return Create(width, height, &format);
}

// Create surface
bool CSDLSurface::Create(long width, long height, SDL_PixelFormat *pixel_format)
{
	SDL_PixelFormat format;
	format = *pixel_format;

	suf = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)width, (int)height
	  , format.BitsPerPixel, format.Rmask, format.Gmask, format.Bmask, format.Amask);

	return (suf != NULL);
}

// Create surface
bool CSDLSurface::Create(long width, long height, CPixelFormat::FormatId force_format)
{
	SDL_PixelFormat format;
	CPixelFormat tmp(force_format);
	tmp.ConvTo(format);

	return Create(width, height, &format);
}

bool CSDLSurface::Create(const VmRectWH &srcrect)
{
	return Create(srcrect.w, srcrect.h);
}

bool CSDLSurface::Create(const VmRectWH &srcrect, const CPixelFormat &srcformat)
{
	return Create(srcrect.w, srcrect.h, srcformat);
}

bool CSDLSurface::Create(const VmRectWH &srcrect, CPixelFormat::FormatId force_format)
{
	return Create(srcrect.w, srcrect.h, force_format);
}

bool CSDLSurface::Create(const VmRectWH &srcrect, CSDLSurface &srcsurface, CPixelFormat::FormatId force_format)
{
	bool valid = Create(srcrect.w, srcrect.h, force_format);
	if (valid) {
		srcsurface.Blit(srcrect, *this, srcrect);
	}
	return valid;
}

bool CSDLSurface::Create(CSDLSurface &srcsurface, const CPixelFormat &pixel_format)
{
	bool valid = Create(srcsurface.Width(), srcsurface.Height(), pixel_format);
	if (valid) {
		srcsurface.Blit(*this);
	}
	return valid;
}

// release surface
void CSDLSurface::Release()
{
	if (suf) {
		SDL_FreeSurface(suf);
		suf = NULL;
	}
}

SDL_Surface *CSDLSurface::Get()
{
	return suf;
}

scrntype *CSDLSurface::GetBuffer()
{
	return (scrntype *)suf->pixels;
}

scrntype *CSDLSurface::GetBuffer(int y)
{
	scrntype *p = (scrntype *)suf->pixels;
	p += (suf->w * y);
	return p;
}

int CSDLSurface::GetBufferSize()
{
	return suf->w * suf->h * suf->format->BytesPerPixel;
}

bool CSDLSurface::IsEnable()
{
	return (suf != NULL);
}

int CSDLSurface::Width()
{
	return suf->w;
}

int CSDLSurface::Height()
{
	return suf->h;
}

int CSDLSurface::BitsPerPixel()
{
	return suf->format->BitsPerPixel;
}

int CSDLSurface::BytesPerPixel()
{
	return suf->format->BytesPerPixel;
}

int CSDLSurface::BytesPerLine()
{
	return suf->pitch;
}

SDL_PixelFormat *CSDLSurface::GetNativePixelFormat()
{
	return suf->format;
}

CPixelFormat CSDLSurface::GetPixelFormat() const
{
	CPixelFormat tmp;
	tmp.ConvFrom(*suf->format);
	return tmp;
}

bool CSDLSurface::Lock()
{
	return (suf != NULL && SDL_LockSurface(suf) == 0);
}

void CSDLSurface::Unlock()
{
	if (suf) SDL_UnlockSurface(suf);
}

bool CSDLSurface::Blit(CSDLSurface &dst)
{
	return (SDL_BlitSurface(suf, NULL, dst.Get(), NULL) >= 0);
}

bool CSDLSurface::Blit(CSDLSurface &dst, const VmRectWH &dst_re)
{
	SDL_Rect dst_sre;
	CP_RECT(dst_re, dst_sre);
	return (SDL_BlitSurface(suf, NULL, dst.Get(), &dst_sre) >= 0);
}

bool CSDLSurface::Blit(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re)
{
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
	return (SDL_BlitSurface(suf, &src_sre, dst.Get(), &dst_sre) >= 0);
}

bool CSDLSurface::Blit(CSDLSurface &dst, SDL_Rect &dst_re)
{
	return (SDL_BlitSurface(suf, NULL, dst.Get(), &dst_re) >= 0);
}

bool CSDLSurface::Blit(SDL_Rect &src_re, CSDLSurface &dst, SDL_Rect &dst_re)
{
#if 0
	SDL_Rect sre, dre;
	if (src_re != NULL) {
		CP_RECT(*src_re, sre);
	} else {
		sre.x = 0; sre.y = 0;
		sre.w = suf->w; sre.h = suf->h;
	}
	if (dst_re != NULL) {
		CP_RECT(*dst_re, dre);
	} else {
		dre.x = 0; dre.y = 0;
		dre.w = dst.suf->w; dre.h = dst.suf->h;
	}
	SDL_UTILS::copy_surface(suf, &sre, dst.suf, &dre); 
	return 0;
#else
	return (SDL_BlitSurface(suf, &src_re, dst.Get(), &dst_re) >= 0);
#endif
}

bool CSDLSurface::Blit(SDL_Surface &dst)
{
	return (SDL_BlitSurface(suf, NULL, &dst, NULL) >= 0);
}

bool CSDLSurface::Blit(SDL_Surface &dst, const VmRectWH &dst_re)
{
	SDL_Rect dst_sre;
	CP_RECT(dst_re, dst_sre);
	return (SDL_BlitSurface(suf, NULL, &dst, &dst_sre) >= 0);
}

bool CSDLSurface::Blit(const VmRectWH &src_re, SDL_Surface &dst, const VmRectWH &dst_re)
{
#if 0
	SDL_Rect sre, dre;
	if (src_re != NULL) {
		CP_RECT(*src_re, sre);
	} else {
		sre.x = 0; sre.y = 0;
		sre.w = suf->w; sre.h = suf->h;
	}
	if (dst_re != NULL) {
		CP_RECT(*dst_re, dre);
	} else {
		dre.x = 0; dre.y = 0;
		dre.w = dst->w; dre.h = dst->h;
	}
	SDL_UTILS::copy_surface(suf, &sre, dst, &dre); 
	return 0;
#else
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
	return (SDL_BlitSurface(suf, &src_sre, &dst, &dst_sre) >= 0);
#endif
}

bool CSDLSurface::Blit(SDL_Surface &dst, SDL_Rect &dst_re)
{
	return (SDL_BlitSurface(suf, NULL, &dst, &dst_re) >= 0);
}

/// copy surface
/// @param[in] src_re : area to copy from src (cannot null)
/// @param[out] dst
/// @param[in] dst_re : area to copy on dst (cannot null)
bool CSDLSurface::Blit(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re)
{
#if 0
	SDL_Rect sre, dre;
	if (src_re != NULL) {
		CP_RECT(*src_re, sre);
	} else {
		sre.x = 0; sre.y = 0;
		sre.w = suf->w; sre.h = suf->h;
	}
	if (dst_re != NULL) {
		CP_RECT(*dst_re, dre);
	} else {
		dre.x = 0; dre.y = 0;
		dre.w = dst->w; dre.h = dst->h;
	}
	SDL_UTILS::copy_surface(suf, &sre, dst, &dre); 
	return 0;
#else
	return (SDL_BlitSurface(suf, &src_re, &dst, &dst_re) >= 0);
#endif
}

bool CSDLSurface::StretchBlit(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re)
{
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
#ifndef USE_SDL2
	return StretchBlit(src_sre, *dst.suf, dst_sre);
#else
	return (SDL_BlitScaled(suf, &src_sre, dst.suf, &dst_sre) >= 0);
#endif
}

bool CSDLSurface::StretchBlit(const VmRectWH &src_re, SDL_Surface &dst, const VmRectWH &dst_re)
{
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
#ifndef USE_SDL2
	return StretchBlit(src_sre, dst, dst_sre);
#else
	return (SDL_BlitScaled(suf, &src_sre, &dst, &dst_sre) >= 0);
#endif
}

bool CSDLSurface::StretchBlit(SDL_Rect &src_re, CSDLSurface &dst, SDL_Rect &dst_re)
{
#ifndef USE_SDL2
	return StretchBlit(src_re, *dst.suf, dst_re);
#else
	return (SDL_BlitScaled(suf, &src_re, dst.suf, &dst_re) >= 0);
#endif
}

/// zoom surface
/// @param[in] src_re : area to copy from src (cannot null)
/// @param[out] dst
/// @param[in] dst_re : area to copy on dst (cannot null)
/// @note no error check. so you must specify valid rect.
bool CSDLSurface::StretchBlit(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re)
{
	scrntype *pSrc;
	scrntype *pDst;
	Uint8 r, g, b, a;

	int sright = (src_re.x + src_re.w);
	int sbottom = (src_re.y + src_re.h);
	int dright = (dst_re.x + dst_re.w);
	int dbottom = (dst_re.y + dst_re.h);
	int x_mag = (src_re.w << 12) / dst_re.w;
	int y_mag = (src_re.h << 12) / dst_re.h;
	int sx_m = 0;
	int sy_m = 0;

	SDL_LockSurface(&dst);
	SDL_LockSurface(suf);
	sy_m = (src_re.y << 12);
	for(int sy=src_re.y, dy = dst_re.y; sy < sbottom && dy < dbottom; dy++) {
		pSrc = (scrntype *)suf->pixels + sy * (suf->w);
		pDst = (scrntype *)dst.pixels + dy * (dst.w);
		sx_m = (src_re.x << 12);
		for(int sx=src_re.x, dx = dst_re.x; sx < sright && dx < dright; dx++) {
			r = ((pSrc[sx] & suf->format->Rmask) >> (suf->format->Rshift)) & 0xff;
			g = ((pSrc[sx] & suf->format->Gmask) >> (suf->format->Gshift)) & 0xff;
			b = ((pSrc[sx] & suf->format->Bmask) >> (suf->format->Bshift)) & 0xff;
			a = ((pSrc[sx] & suf->format->Amask) >> (suf->format->Ashift)) & 0xff;
			pDst[dx] =
			((r << dst.format->Rshift) & dst.format->Rmask)
			| ((g << dst.format->Gshift) & dst.format->Gmask)
			| ((b << dst.format->Bshift) & dst.format->Bmask)
			| ((a << dst.format->Ashift) & dst.format->Amask);
			sx_m+=x_mag;
			sx = (sx_m >> 12);
		}
		sy_m+=y_mag;
		sy = (sy_m >> 12);
	}
	SDL_UnlockSurface(suf);
	SDL_UnlockSurface(&dst);

	return true;
}

bool CSDLSurface::BlitFlipped(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re)
{
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
	return BlitFlipped(src_sre, *dst.suf, dst_sre);
}

/// copy surface (flipped top and bottom)
/// @param[in] src_re : area to copy from src (cannot null)
/// @param[out] dst
/// @param[in] dst_re : area to copy on dst (cannot null)
/// @note no error check. so you must specify valid rect.
bool CSDLSurface::BlitFlipped(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re)
{
	scrntype *pSrc;
	scrntype *pDst;
	Uint32 r, g, b, a;

	int sright = (src_re.x + src_re.w);
	int sbottom = (src_re.y + src_re.h);
	int dright = (dst_re.x + dst_re.w);
	int dbottom = (dst_re.y + dst_re.h);

	SDL_LockSurface(&dst);
	SDL_LockSurface(suf);
	for(int sy=src_re.y, dy = dbottom - 1; sy < sbottom && dy >= dst_re.y ; sy++, dy--) {
		pSrc = (scrntype *)suf->pixels + sy * (suf->w);
		pDst = (scrntype *)dst.pixels + dy * (dst.w);
		for(int sx=src_re.x, dx = dst_re.x; sx < sright && dx < dright; sx++, dx++) {
			r = (pSrc[sx] & suf->format->Rmask) >> (suf->format->Rshift);
			g = (pSrc[sx] & suf->format->Gmask) >> (suf->format->Gshift);
			b = (pSrc[sx] & suf->format->Bmask) >> (suf->format->Bshift);
			a = (pSrc[sx] & suf->format->Amask) >> (suf->format->Ashift);
			pDst[dx] = (r << dst.format->Rshift)
			| (g << dst.format->Gshift)
			| (b << dst.format->Bshift)
			| (a << dst.format->Ashift);
		}
	}
	SDL_UnlockSurface(suf);
	SDL_UnlockSurface(&dst);

	return true;
}

bool CSDLSurface::StretchBlitFlipped(const VmRectWH &src_re, CSDLSurface &dst, const VmRectWH &dst_re)
{
	SDL_Rect src_sre, dst_sre;
	CP_RECT(src_re, src_sre);
	CP_RECT(dst_re, dst_sre);
	return StretchBlitFlipped(src_sre, *dst.suf, dst_sre);
}

/// zoom surface (flipped top and bottom)
/// @param[in] src_re : area to copy from src (cannot null)
/// @param[out] dst
/// @param[in] dst_re : area to copy on dst (cannot null)
/// @note no error check. so you must specify valid rect.
bool CSDLSurface::StretchBlitFlipped(SDL_Rect &src_re, SDL_Surface &dst, SDL_Rect &dst_re)
{
	scrntype *pSrc;
	scrntype *pDst;
	Uint32 r, g, b, a;

	int sright = (src_re.x + src_re.w);
	int sbottom = (src_re.y + src_re.h);
	int dright = (dst_re.x + dst_re.w);
	int dbottom = (dst_re.y + dst_re.h);
	int x_mag = (src_re.w << 12) / dst_re.w;
	int y_mag = (src_re.h << 12) / dst_re.h;
	int sx_m = 0;
	int sy_m = 0;

	SDL_LockSurface(&dst);
	SDL_LockSurface(suf);
	sy_m = (src_re.y << 12);
	for(int sy=src_re.y, dy = dbottom - 1; sy < sbottom && dy >= dst_re.y; dy--) {
		pSrc = (scrntype *)suf->pixels + sy * (suf->w);
		pDst = (scrntype *)dst.pixels + dy * (dst.w);
		sx_m = (src_re.x << 12);
		for(int sx=src_re.x, dx = dst_re.x; sx < sright && dx < dright; dx++) {
			r = (pSrc[sx] & suf->format->Rmask) >> (suf->format->Rshift);
			g = (pSrc[sx] & suf->format->Gmask) >> (suf->format->Gshift);
			b = (pSrc[sx] & suf->format->Bmask) >> (suf->format->Bshift);
			a = (pSrc[sx] & suf->format->Amask) >> (suf->format->Ashift);
			pDst[dx] = (r << dst.format->Rshift)
			| (g << dst.format->Gshift)
			| (b << dst.format->Bshift)
			| (a << dst.format->Ashift);
			sx_m+=x_mag;
			sx = (sx_m >> 12);
		}
		sy_m+=y_mag;
		sy = (sy_m >> 12);
	}
	SDL_UnlockSurface(suf);
	SDL_UnlockSurface(&dst);

	return true;
}

/// fill data on surface
/// @param[in] data : pixel data to fill
/// @param[in] mask : mask data
/// @note no error check. so you must specify valid rect.
void CSDLSurface::Fill(scrntype data, scrntype mask)
{
	scrntype *pSrc;
	SDL_LockSurface(suf);
	for(int sy=0; sy < suf->h; sy++) {
		pSrc = (scrntype *)suf->pixels + sy * (suf->w);
		for(int sx=0; sx < suf->w; sx++) {
			*pSrc = ((data & mask) | (*pSrc & ~mask));
			pSrc++;
		}
	}
	SDL_UnlockSurface(suf);
}

#ifdef USE_SDL2
/// copy texture to renderer
/// @param[in] renderer
/// @param[in] texture
/// @param[in] srcrect : area to copy from texture
/// @param[in] dstrect : area to copy on renderer
bool CSDLSurface::Render(SDL_Renderer &renderer, SDL_Texture &texture, VmRectWH &srcrect, VmRectWH &dstrect)
{
	SDL_Rect s_reSrc, s_reDst;
	CP_RECT(srcrect, s_reSrc);
	CP_RECT(dstrect, s_reDst);
	return (SDL_RenderCopy(&renderer, &texture, &s_reSrc, &s_reDst) >= 0);
}
#endif

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

CSurface::CSurface()
	: CSDLSurface()
{
}
CSurface::CSurface(long width, long height)
	: CSDLSurface(width, height)
{
}
CSurface::CSurface(long width, long height, const CPixelFormat &pixel_format)
	: CSDLSurface(width, height, pixel_format)
{
}
CSurface::CSurface(long width, long height, SDL_PixelFormat *pixel_format)
	: CSDLSurface(width, height, pixel_format)
{
}
CSurface::CSurface(long width, long height, CPixelFormat::FormatId force_format)
	: CSDLSurface(width, height, force_format)
{
}

// ----------------------------------------------------------------------------

#ifdef USE_GTK
CCairoSurface::CCairoSurface()
{
	p_cas = NULL;
}
CCairoSurface::CCairoSurface(CSurface &buf, long width, long height)
{
	p_cas = NULL;
	CreateC(buf, width, height);
}
CCairoSurface::CCairoSurface(CSurface &buf, const VmRectWH &srcrect)
{
	p_cas = NULL;
	CreateC(buf, srcrect);
}
CCairoSurface::~CCairoSurface()
{
	ReleaseC();
}

bool CCairoSurface::CreateC(CSurface &buf, long width, long height)
{
	return CreateC(buf, width, height, CAIRO_FORMAT_RGB24);
}

bool CCairoSurface::CreateC(CSurface &buf, const VmRectWH &srcrect)
{
	return CreateC(buf, srcrect, CAIRO_FORMAT_RGB24);
}

bool CCairoSurface::CreateC(CSurface &buf, long width, long height, cairo_format_t format)
{
	ReleaseC();

	p_cas = cairo_image_surface_create_for_data(
		(unsigned char *)buf.GetBuffer()
		, format
		, width, height
		, cairo_format_stride_for_width(format, buf.Width())
	);
	return (p_cas != NULL);
}

bool CCairoSurface::CreateC(CSurface &buf, const VmRectWH &srcrect, cairo_format_t format)
{
	return CreateC(buf, srcrect.w, srcrect.h, format);
}

// release surface
void CCairoSurface::ReleaseC()
{
	if (p_cas) {
		cairo_surface_finish(p_cas);
		p_cas = NULL;
	}
}

bool CCairoSurface::BlitC(cairo_t *dst)
{
	if (!p_cas) return false;
	cairo_set_source_surface(dst, p_cas, 0.0, 0.0);
	return true;
}

bool CCairoSurface::BlitC(cairo_t *dst, const VmRectWH &dst_re)
{
	if (!p_cas) return false;
	cairo_matrix_t m, bm;
	cairo_get_matrix(dst, &m);
	bm = m;
	m.xx = 1.0;
	m.xy = 0.0;
	m.yx = 0.0;
	m.yy = 1.0;
	m.x0 += (double)dst_re.x;
	m.y0 += (double)dst_re.y;
	cairo_set_matrix(dst, &m);
	cairo_set_source_surface(dst, p_cas, 0.0, 0.0);
	cairo_set_matrix(dst, &bm);
	return true;
}

bool CCairoSurface::BlitC(cairo_t *dst, const SDL_Rect &dst_re)
{
	if (!p_cas) return false;
	cairo_matrix_t m, bm;
	cairo_get_matrix(dst, &m);
	bm = m;
	m.xx = 1.0;
	m.xy = 0.0;
	m.yx = 0.0;
	m.yy = 1.0;
	m.x0 += (double)dst_re.x;
	m.y0 += (double)dst_re.y;
	cairo_set_matrix(dst, &m);
	cairo_set_source_surface(dst, p_cas, 0.0, 0.0);
	cairo_set_matrix(dst, &bm);
	return true;
}

bool CCairoSurface::BlitC(SDL_Rect &src_re, cairo_t *dst, SDL_Rect &dst_re)
{
	if (!p_cas) return false;
	cairo_matrix_t m, bm;
	cairo_get_matrix(dst, &m);
	bm = m;
	m.xx = 1.0;
	m.xy = 0.0;
	m.yx = 0.0;
	m.yy = 1.0;
	m.x0 += (double)dst_re.x;
	m.y0 += (double)dst_re.y;
	cairo_set_matrix(dst, &m);
	cairo_set_source_surface(dst, p_cas, (double)-src_re.x, -(double)src_re.y);
	cairo_set_matrix(dst, &bm);
	return true;
}

bool CCairoSurface::StretchBlitC(const VmRectWH &src_re, cairo_t *dst, const VmRectWH &dst_re)
{
	if (!p_cas) return false;
	cairo_matrix_t m, bm;
	cairo_get_matrix(dst, &m);
	bm = m;
	m.xx = (double)dst_re.w/src_re.w;
	m.xy = 0.0;
	m.yx = 0.0;
	m.yy = (double)dst_re.h/src_re.h;
	m.x0 += (double)dst_re.x;
	m.y0 += (double)dst_re.y;
	cairo_set_matrix(dst, &m);
	cairo_set_source_surface(dst, p_cas, (double)-src_re.x, -(double)src_re.y);
	cairo_set_matrix(dst, &bm);
	return true;
}
#endif /* USE_GTK */

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if defined(USE_SDL2) || defined(USE_WX2)
CTexture::CTexture()
{
	m_renderer = NULL;
	m_tex = NULL;
	m_buf = NULL;
	m_w = 0;
	m_h = 0;
}

CTexture::CTexture(SDL_Renderer *renderer, long width, long height)
{
	m_renderer = NULL;
	m_tex = NULL;
	m_buf = NULL;
	m_w = 0;
	m_h = 0;

	this->Create(renderer, width, height, 0);
}

CTexture::CTexture(SDL_Renderer *renderer, long width, long height, Uint32 format)
{
	m_renderer = NULL;
	m_tex = NULL;
	m_buf = NULL;
	m_w = 0;
	m_h = 0;

	this->Create(renderer, width, height, format);
}

CTexture::~CTexture()
{
	Release();
}

bool CTexture::Create(SDL_Renderer *renderer, long width, long height, Uint32 format)
{
	if (!format) {
		format = SDL_PIXELFORMAT_ARGB8888;
	}
	m_tex = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING,
			(int)width, (int)height);
	if (m_tex != NULL) {
		m_renderer = renderer;
		m_buf = NULL;
		m_w = (int)width;
		m_h = (int)height;
		SDL_SetTextureBlendMode(m_tex, SDL_BLENDMODE_BLEND);
	} else {
		m_renderer = NULL;
		m_buf = NULL;
		m_w = 0;
		m_h = 0;
	}
	return (m_tex != NULL);
}

void CTexture::Release()
{
	if (m_tex) {
		SDL_DestroyTexture(m_tex);
		m_tex = NULL;
		m_buf = NULL;
		m_w = 0;
		m_h = 0;
	}
}

SDL_Texture *CTexture::Get()
{
	return m_tex;
}

SDL_Renderer *CTexture::Renderer()
{
	return m_renderer;
}

scrntype *CTexture::GetBuffer()
{
	return m_buf;
}

scrntype *CTexture::GetBuffer(int y)
{
	return (m_buf + m_w * y);
}

bool CTexture::IsEnable()
{
	return (m_tex != NULL);
}

int CTexture::Width()
{
	return m_w;
}

int CTexture::Height()
{
	return m_h;
}

bool CTexture::Lock()
{
	int p;
	return (m_tex != NULL && SDL_LockTexture(m_tex, NULL, (void **)&m_buf, &p) == 0);
}

void CTexture::Unlock()
{
	if (m_tex) SDL_UnlockTexture(m_tex);
	m_buf = NULL;
}

/// fill data on texture
/// @param[in] data : pixel data to fill
/// @note no error check. so you must specify valid rect.
void CTexture::Fill(scrntype data)
{
	int pitch;
	if (SDL_LockTexture(m_tex, NULL, (void **)&m_buf, &pitch) == 0) {
		for(int sy=0; sy < m_h; sy++) {
			for(int sx=0; sx < m_w; sx++) {
				*m_buf = data;
				m_buf++;
			}
		}
		SDL_UnlockTexture(m_tex);
		m_buf = NULL;
	}
}
#endif /* defined(USE_SDL2) || defined(USE_WX2) */
