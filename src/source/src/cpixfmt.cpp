/** @file cpixfmt.cpp

	@author Sasaji
	@date   2019.03.01

	@brief pixel format
*/

#include "cpixfmt.h"

CPixelFormat::CPixelFormat()
{
#if defined(USE_WIN)
	PresetBGRA();
#elif defined(USE_SDL) || defined(USE_SDL2)
	PresetRGBA();
#elif defined(USE_WX) || defined(USE_WX2)
#if defined(_WIN32)
	// windows DIB
	PresetBGRA();
#elif defined(__APPLE__) && defined(__MACH__)
	PresetARGB();
#else
	PresetRGBA();
#endif
#else
	PresetRGBA();
#endif
}

CPixelFormat::CPixelFormat(FormatId id)
{
	Preset(id);
}

CPixelFormat::~CPixelFormat()
{
}

bool CPixelFormat::operator==(const CPixelFormat &val)
{
	return (Rshift = val.Rshift && Gshift == val.Gshift && Bshift == val.Bshift && Ashift == val.Ashift);
}

void CPixelFormat::PresetRGBA()
{
	// OpenGL RGBA masks
	Set(0, 1, 2, 3);
}

void CPixelFormat::PresetBGRA()
{
	// BGRA
	Set(2, 1, 0, 3);
}

void CPixelFormat::PresetARGB()
{
	// ARGB
	Set(1, 2, 3, 0);
}

void CPixelFormat::PresetABGR()
{
	// ABGR
	Set(3, 2, 1, 0);
}

void CPixelFormat::PresetRBGA()
{
	// RBGA masks
	Set(0, 2, 1, 3);
}

void CPixelFormat::PresetRGAB()
{
	// RGAB masks
	Set(0, 1, 3, 2);
}

void CPixelFormat::PresetBARG()
{
	// OpenGL  masks
	Set(0, 2, 3, 1);
}

void CPixelFormat::Preset(FormatId id)
{
	switch(id) {
	case RGBA32:
		PresetRGBA();
		break;
	case BGRA32:
		PresetBGRA();
		break;
	case ARGB32:
		PresetARGB();
		break;
	case ABGR32:
		PresetABGR();
		break;
	case RBGA32:
		PresetRBGA();
		break;
	case RGAB32:
		PresetRGAB();
		break;
	case BARG32:
		PresetBARG();
		break;
	default:
		break;
	}
}

bool CPixelFormat::Set(uint8_t r_pos, uint8_t g_pos, uint8_t b_pos, uint8_t a_pos)
{
	if (r_pos > 3 || g_pos > 3 || b_pos > 3 || a_pos > 3) return false;

#ifdef USE_BIG_ENDIAN
	r_pos = (3 - r_pos);
	g_pos = (3 - g_pos);
	b_pos = (3 - b_pos);
	a_pos = (3 - a_pos);
#endif

#if defined(_RGB888)
	Rshift = (r_pos * 8);
	Gshift = (g_pos * 8);
	Bshift = (b_pos * 8);
	Ashift = (a_pos * 8);
	Rmask = (0xff << Rshift);
	Gmask = (0xff << Gshift);
	Bmask = (0xff << Bshift);
	Amask = (0xff << Ashift);
#elif defined(_RGB565)
	// Unsupported
	uint8_t  s_map[] = {0, 5, 11, 0};
	uint32_t m_map[] = {0x001f, 0x07e0, 0xf800, 0x0000};
	Rshift = s_map[r_pos];
	Gshift = s_map[g_pos];
	Bshift = s_map[b_pos];
	Ashift = s_map[a_pos];
	Rmask = m_map[r_pos];
	Gmask = m_map[g_pos];
	Bmask = m_map[b_pos];
	Amask = m_map[a_pos];
#elif defined(_RGB555)
	// Unsupported
	uint8_t  s_map[] = {0, 5, 10, 0};
	uint32_t m_map[] = {0x001f, 0x03e0, 0x7c00, 0x0000};
	Rshift = s_map[r_pos];
	Gshift = s_map[g_pos];
	Bshift = s_map[b_pos];
	Ashift = s_map[a_pos];
	Rmask = m_map[r_pos];
	Gmask = m_map[g_pos];
	Bmask = m_map[b_pos];
	Amask = m_map[a_pos];
#endif

	return true;
}

void CPixelFormat::Copy(uint8_t r_shift, uint8_t g_shift, uint8_t b_shift, uint8_t a_shift, uint32_t r_mask, uint32_t g_mask, uint32_t b_mask, uint32_t a_mask)
{
	Rshift = r_shift;
	Gshift = g_shift;
	Bshift = b_shift;
	Ashift = a_shift;
	Rmask = r_mask;
	Gmask = g_mask;
	Bmask = b_mask;
	Amask = a_mask;
}

void CPixelFormat::Get(scrntype pixel, uint8_t &r, uint8_t &g, uint8_t &b) const
{
	r = (uint8_t)((pixel & Rmask) >> Rshift);
	g = (uint8_t)((pixel & Gmask) >> Gshift);
	b = (uint8_t)((pixel & Bmask) >> Bshift);
}

void CPixelFormat::Get(scrntype pixel, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
{
	r = (uint8_t)((pixel & Rmask) >> Rshift);
	g = (uint8_t)((pixel & Gmask) >> Gshift);
	b = (uint8_t)((pixel & Bmask) >> Bshift);
	a = (uint8_t)((pixel & Amask) >> Ashift);
}

void CPixelFormat::Get(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift) const
{
	if (r_mask) *r_mask = Rmask;
	if (g_mask) *g_mask = Gmask;
	if (b_mask) *b_mask = Bmask;
	if (r_shift) *r_shift = Rshift;
	if (g_shift) *g_shift = Gshift;
	if (b_shift) *b_shift = Bshift;
}

void CPixelFormat::Get(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint32_t *a_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift, uint8_t *a_shift) const
{
	if (r_mask) *r_mask = Rmask;
	if (g_mask) *g_mask = Gmask;
	if (b_mask) *b_mask = Bmask;
	if (a_mask) *a_mask = Amask;
	if (r_shift) *r_shift = Rshift;
	if (g_shift) *g_shift = Gshift;
	if (b_shift) *b_shift = Bshift;
	if (a_shift) *a_shift = Ashift;

	// calcrate alpha channel
	if (Amask == 0) {
		uint32_t am = ~(Rmask | Gmask | Bmask);
		if (a_mask) {
			*a_mask = am;
		}
		if (a_shift) {
			for(int i=0; i<32; i++) {
				if (am & (1 << i)) {
					*a_shift = i;
					break;
				}
			}
		}
	}
}

scrntype CPixelFormat::Map(uint8_t r, uint8_t g, uint8_t b) const
{
	scrntype pixel = 0;
	pixel |= ((scrntype)r << Rshift);
	pixel |= ((scrntype)g << Gshift);
	pixel |= ((scrntype)b << Bshift);
	pixel |= ((scrntype)0xff << Ashift);
	return pixel;
}

scrntype CPixelFormat::Map(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const
{
	scrntype pixel = 0;
	pixel |= ((scrntype)r << Rshift);
	pixel |= ((scrntype)g << Gshift);
	pixel |= ((scrntype)b << Bshift);
	pixel |= ((scrntype)a << Ashift);
	return pixel;
}

int CPixelFormat::BitsPerPixel() const
{
	return (int)sizeof(scrntype) * 8; 
}

int CPixelFormat::BytesPerPixel() const
{
	return (int)sizeof(scrntype); 
}

#if defined(USE_SDL) || defined(USE_SDL2)
void CPixelFormat::ConvTo(SDL_PixelFormat &fmt) const
{
	fmt.palette = NULL;
	fmt.BytesPerPixel =  (Uint8)BytesPerPixel();
	fmt.BitsPerPixel = (Uint8)BitsPerPixel();
	fmt.Rshift = Rshift;
	fmt.Gshift = Gshift;
	fmt.Bshift = Bshift;
	fmt.Ashift = Ashift;
	fmt.Rmask = Rmask;
	fmt.Gmask = Gmask;
	fmt.Bmask = Bmask;
	fmt.Amask = Amask;
	fmt.Rloss = 0;
	fmt.Gloss = 0;
	fmt.Bloss = 0;
	fmt.Aloss = 0;
#if defined(USE_SDL)
	// disable alpha channel on SDL1
	fmt.Ashift = 0;
	fmt.Amask = 0;
	fmt.Aloss = 8;
#endif
}
void CPixelFormat::ConvFrom(const SDL_PixelFormat &fmt)
{
	Rshift = fmt.Rshift;
	Gshift = fmt.Gshift;
	Bshift = fmt.Bshift;
	Ashift = fmt.Ashift;
	Rmask = fmt.Rmask;
	Gmask = fmt.Gmask;
	Bmask = fmt.Bmask;
	Amask = fmt.Amask;
	if (Amask == 0) {
		Amask = ~(Rmask | Gmask | Bmask);
		for(int i=0; i<32; i++) {
			if (Amask & (1 << i)) {
				Ashift = i;
				break;
			}
		}
	}
}
#if defined(USE_SDL2)
void CPixelFormat::ConvFrom(Uint32 num)
{
	SDL_PixelFormat *fmt = SDL_AllocFormat(num);
	ConvFrom(*fmt);
	SDL_FreeFormat(fmt);
}
#endif
#elif defined(USE_QT)
void CPixelFormat::ConvTo(QImage::Format &fmt) const
{
	int r, g, b, a;
#if defined(_RGB888)
	r = (int)Rshift / 8;
	g = (int)Gshift / 8;
	b = (int)Bshift / 8;
	a = (int)Ashift / 8;
#endif
	if (r == 0 && g == 1 && b == 2 && a == 3) fmt = QImage::Format_RGBA8888;
	else if (r == 2 && g == 1 && b == 0 && a == 3) fmt = QImage::Format_ARGB32;
}
void CPixelFormat::ConvFrom(const QImage::Format &fmt)
{
	switch(fmt) {
	case QImage::Format_RGBA8888:
	case QImage::Format_RGBX8888:
		PresetRGBA();
		break;
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
		PresetBGRA();
		break;
	default:
		PresetRGBA();
		break;
	}
}
#endif
