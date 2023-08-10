/** @file png_rec_video.cpp

 Skelton for retropc emulator

 @author Sasaji
 @date   2016.12.10 -

 @brief [ record video using libpng ]
 */

#include "png_rec_video.h"

#if defined(USE_CAPTURE_SCREEN_PNG) && defined(USE_CAP_SCREEN_LIBPNG)

#include <png.h>
#include "../rec_video.h"
#include "../../csurface.h"

PNG_REC_VIDEO::PNG_REC_VIDEO(EMU *new_emu, REC_VIDEO *new_vid)
{
	emu = new_emu;
	vid = new_vid;
}

PNG_REC_VIDEO::~PNG_REC_VIDEO()
{
}

bool PNG_REC_VIDEO::Capture(int type, CSurface *surface, const char *file_name)
{
	FILE       *fp;
	png_structp png_ptr;
	png_infop   info_ptr;
	int         color_type;
	png_bytep   image;

	fp = fopen(file_name, "wb");
	if (!fp) {
		return false;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		return false;
	}

	image = (png_bytep)surface->GetBuffer();

	png_init_io(png_ptr, fp);

	color_type = PNG_COLOR_TYPE_RGBA;
//	png_set_bgr(png_ptr); // BGR -> RGB convert
//	png_set_invert_alpha(png_ptr); // invert Alpha channel
	png_set_IHDR(png_ptr, info_ptr,
		surface->Width(),
		surface->Height(),
	    8,
		color_type,
		PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);
//	png_write_image(png_ptr, image);
	for(int row=0; row<(surface->Height()); row++) {
		png_write_row(png_ptr, image);
		image += (surface->BytesPerLine());
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return true;
}

#endif /* USE_CAPTURE_SCREEN_PNG && USE_CAP_SCREEN_LIBPNG */

