/** @file win_d3d.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ Direct3D ]
*/

#include "win_d3d.h"

#ifdef USE_DIRECT3D

#include "win_csurface.h"

#define VERTEX_IN(st, vx, vy, vz, vu, vv) { \
	(st).x = (FLOAT)(vx); \
	(st).y = (FLOAT)(vy); \
	(st).z = (FLOAT)(vz); \
	(st).rhw = (FLOAT)1.0; \
	(st).u = (FLOAT)(vu); \
	(st).v = (FLOAT)(vv); \
}

#define VERTEX_XYIN(st, vx, vy) { \
	(st).x = (FLOAT)(vx); \
	(st).y = (FLOAT)(vy); \
}

#define VERTEX_UVIN(st, vu, vv) { \
	(st).u = (FLOAT)(vu); \
	(st).v = (FLOAT)(vv); \
}

#define VERTEX_LIMIT(vl, vt, vr, vb, ll, tt, rr, bb) \
	float vl = (float)(ll) - (float)0.5; \
	float vt = (float)(tt) - (float)0.5; \
	float vr = (float)(rr) - (float)0.5; \
	float vb = (float)(bb) - (float)0.5;

// ===========================================================================

CD3DSurface::CD3DSurface()
{
	pSurface = NULL;
}
CD3DSurface::~CD3DSurface()
{
	ReleaseD3DSurface();
}

HRESULT CD3DSurface::CreateD3DSurface(PDIRECT3DDEVICE9 pD3Device, int w, int h)
{
	return pD3Device->CreateOffscreenPlainSurface(w, h, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pSurface, NULL);
}

HRESULT CD3DSurface::CreateD3DMemorySurface(PDIRECT3DDEVICE9 pD3Device, int w, int h)
{
	return pD3Device->CreateOffscreenPlainSurface(w, h, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
}

void CD3DSurface::ReleaseD3DSurface()
{
	if (pSurface) {
		pSurface->Release();
		pSurface = NULL;
	}
}

PDIRECT3DSURFACE9 CD3DSurface::GetD3DSurface()
{
	return pSurface;
}

int CD3DSurface::GetD3DSurfaceWidth() const
{
	if (pSurface) {
		D3DSURFACE_DESC desc;
		pSurface->GetDesc(&desc);
		return (int)desc.Width;
	} else {
		return 0;
	}
}

int CD3DSurface::GetD3DSurfaceHeight() const
{
	if (pSurface) {
		D3DSURFACE_DESC desc;
		pSurface->GetDesc(&desc);
		return (int)desc.Height;
	} else {
		return 0;
	}
}

// ===========================================================================

CD3DTexture::CD3DTexture()
{
	pTexture = NULL;
	m_first = true;
}
CD3DTexture::~CD3DTexture()
{
	ReleaseD3DTexture();
}

HRESULT CD3DTexture::CreateD3DTexture(PDIRECT3DDEVICE9 pD3Device, int w, int h)
{
	int tw = w / 16;
	tw = ((tw + 1) & ~1) * 16;
	int th = h;
	if (th < 32) th = 32;
	HRESULT hre = pD3Device->CreateTexture(tw, th, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL);

	// set vertex
	if (m_first) {
		VERTEX_LIMIT(vl, vt, vr, vb, 0, 0, w, h)
		VERTEX_IN(m_vertex[0], vl, vt, 0, 0, 0);
		VERTEX_IN(m_vertex[1], vr, vt, 0, (float)w/tw, 0);
		VERTEX_IN(m_vertex[2], vr, vb, 0, (float)w/tw, (float)h/th);
		VERTEX_IN(m_vertex[3], vl, vb, 0, 0, (float)h/th);
	}
	m_first = false;
	return hre;
}

void CD3DTexture::ReleaseD3DTexture()
{
	if (pTexture) {
		pTexture->Release();
		pTexture = NULL;
	}
}

HRESULT CD3DTexture::DrawD3DTexture(PDIRECT3DDEVICE9 pD3Device)
{
	HRESULT hre = pD3Device->SetTexture(0, pTexture);

	if (hre == D3D_OK) {
//		pD3Device->SetTextureStageState(0,	D3DTSS_ALPHAARG1	, D3DTA_TEXTURE	);
//		pD3Device->SetTextureStageState(0,	D3DTSS_ALPHAOP	, D3DTOP_SELECTARG1	);

		hre = pD3Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, m_vertex, sizeof(d3d_vertex_t));
	}
	return hre;
}

bool CD3DTexture::CopyD3DTextureFrom(CSurface *suf)
{
	int dst_w = GetD3DTextureWidth();

	D3DLOCKED_RECT pLockedRect;
	HRESULT hre = pTexture->LockRect(0, &pLockedRect, NULL, 0);
	if (hre != D3D_OK) {
		return false;
	}
	// copy to main context
	int src_w = suf->Width();
	int src_h = suf->Height();
	scrntype *src = suf->GetBuffer(src_h - 1);
	scrntype *dst = (scrntype *)pLockedRect.pBits;
	for(int y=0; y<src_h; y++) {
		for(int x=0; x<src_w; x++) {
			dst[x] = src[x];
		}
		src -= src_w;
		dst += dst_w;
	}
	pTexture->UnlockRect(0);

	return true;
}

bool CD3DTexture::CopyD3DTextureFrom(CSurface *suf, int suf_top, int suf_h)
{
	int dst_w = GetD3DTextureWidth();

	D3DLOCKED_RECT pLockedRect;
	HRESULT hre = pTexture->LockRect(0, &pLockedRect, NULL, 0);
	if (hre != D3D_OK) {
		return false;
	}
	// copy to main context
	int src_w = suf->Width();
	int src_h = suf->Height();
	scrntype *src = suf->GetBuffer(src_h - suf_top - 1);
	scrntype *dst = (scrntype *)pLockedRect.pBits;
	for(int y=0; y<suf_h; y++) {
		for(int x=0; x<src_w; x++) {
			dst[x] = src[x];
		}
		src -= src_w;
		dst += dst_w;
	}
	pTexture->UnlockRect(0);

	return true;
}

void CD3DTexture::SetD3DTexturePosition(RECT &re)
{
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, re.left, re.top, re.right, re.bottom)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
}

void CD3DTexture::SetD3DTexturePositionUv(RECT &re)
{
	// set vertex
	int tex_w = GetD3DTextureWidth();
	int tex_h = GetD3DTextureHeight();
	float u = (float)(re.right - re.left) / tex_w;
	float v = (float)(re.bottom - re.top) / tex_h;
	VERTEX_LIMIT(vl, vt, vr, vb, re.left, re.top, re.right, re.bottom)
	VERTEX_IN(m_vertex[0], vl, vt, 0, 0, 0);
	VERTEX_IN(m_vertex[1], vr, vt, 0, u, 0);
	VERTEX_IN(m_vertex[2], vr, vb, 0, u, v);
	VERTEX_IN(m_vertex[3], vl, vb, 0, 0, v);
}

void CD3DTexture::SetD3DTexturePosition(VmRectWH &re)
{
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, re.x, re.y, re.x + re.w, re.y + re.h)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
}

void CD3DTexture::SetD3DTexturePosition(int x, int y, int w, int h)
{
	// set vertex
	VERTEX_LIMIT(vl, vt, vr, vb, x, y, x + w, y + h)
	VERTEX_XYIN(m_vertex[0], vl, vt);
	VERTEX_XYIN(m_vertex[1], vr, vt);
	VERTEX_XYIN(m_vertex[2], vr, vb);
	VERTEX_XYIN(m_vertex[3], vl, vb);
}

void CD3DTexture::SetD3DTexturePositionUV(float u, float v, float uw, float vh)
{
	VERTEX_UVIN(m_vertex[0], u, v);
	VERTEX_UVIN(m_vertex[1], u + uw, v);
	VERTEX_UVIN(m_vertex[2], u + uw, v + vh);
	VERTEX_UVIN(m_vertex[3], u, v + vh);
}

PDIRECT3DTEXTURE9 CD3DTexture::GetD3DTexture()
{
	return pTexture;
}

int CD3DTexture::GetD3DTextureWidth() const
{
	if (pTexture) {
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		return (int)desc.Width;
	} else {
		return 0;
	}
}

int CD3DTexture::GetD3DTextureHeight() const
{
	if (pTexture) {
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		return (int)desc.Height;
	} else {
		return 0;
	}
}

#endif /* USE_DIRECT3D */
