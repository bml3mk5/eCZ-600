/** @file cpixfmt.h

	@author Sasaji
	@date   2019.03.01

	@brief pixel format
*/

#ifndef CPIXFMT_H
#define CPIXFMT_H

#include "common.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#elif defined(USE_QT)
#include <QImage>
#endif

/**
	@brief R,G,B,A position on 32bit color
*/
class CPixelFormat
{
public:
	uint8_t  Rshift;
	uint8_t  Gshift;
	uint8_t  Bshift;
	uint8_t  Ashift;
	uint32_t Rmask;
	uint32_t Gmask;
	uint32_t Bmask;
	uint32_t Amask;

	enum FormatId {
		NONE = 0,
		RGBA32,	// openGL
		BGRA32,
		ARGB32,
		ABGR32,
		RBGA32,
		RGAB32,
		BARG32,
		RGB24,
		BGR24
	};

public:
	CPixelFormat();
	CPixelFormat(FormatId id);
	~CPixelFormat();

	bool operator==(const CPixelFormat &val);

	void PresetRGBA();
	void PresetBGRA();
	void PresetARGB();
	void PresetABGR();
	void PresetRBGA();
	void PresetRGAB();
	void PresetBARG();
	void Preset(FormatId id);

	bool Set(uint8_t r_pos, uint8_t g_pos, uint8_t b_pos, uint8_t a_pos); 
	void Copy(uint8_t r_shift, uint8_t g_shift, uint8_t b_shift, uint8_t a_shift, uint32_t r_mask, uint32_t g_mask, uint32_t b_mask, uint32_t a_mask);

	void Get(scrntype pixel, uint8_t &r, uint8_t &g, uint8_t &b) const;
	void Get(scrntype pixel, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const;

	void Get(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift) const;
	void Get(uint32_t *r_mask, uint32_t *g_mask, uint32_t *b_mask, uint32_t *a_mask, uint8_t *r_shift, uint8_t *g_shift, uint8_t *b_shift, uint8_t *a_shift) const;

	scrntype Map(uint8_t r, uint8_t g, uint8_t b) const;
	scrntype Map(uint8_t r, uint8_t g, uint8_t b, uint8_t a) const;

	int BitsPerPixel() const;
	int BytesPerPixel() const;

#if defined(USE_SDL) || defined(USE_SDL2)
	void ConvTo(SDL_PixelFormat &fmt) const;
	void ConvFrom(const SDL_PixelFormat &fmt);
#if defined(USE_SDL2)
	void ConvFrom(Uint32 num);
#endif
#elif defined(USE_QT)
	void ConvTo(QImage::Format &fmt) const;
	void ConvFrom(const QImage::Format &fmt);
#endif
};

#endif /* CPIXFMT_H */
