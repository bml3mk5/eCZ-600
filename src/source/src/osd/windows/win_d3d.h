/** @file win_d3d.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ Direct3D ]
*/

#ifndef WIN_D3D_H
#define WIN_D3D_H

#include "../../vm/vm_defs.h"

#ifdef USE_DIRECT3D

#include <windows.h>
#include "../../common.h"

//#define DIRECT3D_VERSION 0x900
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#ifdef USE_DIRECT3DX
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#endif

typedef struct st_d3d_vertex {
	float x,y,z,rhw;
	float u,v;
} d3d_vertex_t;

class CSurface;

/**
	@brief one surface
*/
class CD3DSurface
{
protected:
	PDIRECT3DSURFACE9 pSurface;

public:
	CD3DSurface();
	virtual ~CD3DSurface();

	HRESULT CreateD3DSurface(PDIRECT3DDEVICE9 pD3Device, int w, int h);
	HRESULT CreateD3DMemorySurface(PDIRECT3DDEVICE9 pD3Device, int w, int h);
	void ReleaseD3DSurface();

	PDIRECT3DSURFACE9 GetD3DSurface();
	int GetD3DSurfaceWidth() const;
	int GetD3DSurfaceHeight() const;
};

/**
	@brief one texture with one square polygon
*/
class CD3DTexture
{
protected:
	PDIRECT3DTEXTURE9 pTexture;

	d3d_vertex_t m_vertex[4];

	bool m_first;

public:
	CD3DTexture();
	virtual ~CD3DTexture();

	HRESULT CreateD3DTexture(PDIRECT3DDEVICE9 pD3Device, int w, int h);
	void ReleaseD3DTexture();

	HRESULT DrawD3DTexture(PDIRECT3DDEVICE9 pD3Device);

	bool CopyD3DTextureFrom(CSurface *suf);
	bool CopyD3DTextureFrom(CSurface *suf, int suf_top, int suf_h);

	void SetD3DTexturePosition(RECT &re);
	void SetD3DTexturePositionUv(RECT &re);
	void SetD3DTexturePosition(VmRectWH &re);
	void SetD3DTexturePosition(int x, int y, int w, int h);
	void SetD3DTexturePositionUV(float u, float v, float uw, float vh);

	PDIRECT3DTEXTURE9 GetD3DTexture();
	int GetD3DTextureWidth() const;
	int GetD3DTextureHeight() const;
};

#endif /* USE_DIRECT3D */

#endif /* WIN_D3D_H */
