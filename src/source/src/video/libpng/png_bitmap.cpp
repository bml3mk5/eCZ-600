/** @file png_bitmap.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#include <png.h>
#include "png_bitmap.h"

CBitmap::CBitmap()
	: CBitmapBase()
{
}

CBitmap::CBitmap(const _TCHAR *file_name, CPixelFormat *format)
	: CBitmapBase()
{
	this->Load(file_name, format);
}

CBitmap::~CBitmap()
{
}

bool CBitmap::Load(const _TCHAR *file_name, CPixelFormat *format)
{
	FILE       *fp;
	png_structp png_ptr;
	png_infop   info_ptr;
	png_infop   end_info;

	fp = fopen(file_name, "rb");
	if (!fp) {
		return false;
	}
	png_byte sig[10];
	int pos = 8;
	size_t len = fread(sig, sizeof(png_byte), pos, fp);
	if (len < (size_t)pos && png_sig_cmp(sig, 0, pos) != 0) {
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, pos);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width;
	png_uint_32 height;
	int         bit_depth;
	int         color_type;
	png_get_IHDR(png_ptr, info_ptr,
		&width, &height, &bit_depth, &color_type,
		NULL, NULL, NULL);
//	int channel = png_get_channels(png_ptr, info_ptr);

//	printf("w:%d h:%d ch:%d\n",width,height,channel);

	bool enable = Create(width, height, CPixelFormat::RGBA32);
	if (enable) {
		// set transform
		png_set_expand(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		int rowbytes = 4 * width;

		png_bytep rowdata = (png_bytep)malloc(rowbytes + 1);
		png_bytep dspdata = (png_bytep)malloc(rowbytes + 1);
		uint8_t *pixels = (uint8_t *)suf->pixels;
		for(int row=0; row<(int)height; row++) {
			png_read_row(png_ptr, rowdata ,dspdata);
			memcpy(&pixels[rowbytes * row], dspdata, rowbytes);
		}
		free(dspdata);
		free(rowdata);
	}
	png_read_end(png_ptr, info_ptr);

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(fp);

	return enable;
}

