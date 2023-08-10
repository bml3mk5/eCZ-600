/** @file sdl_vkeyboardbase.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.09.22 -

	@brief [ virtual keyboard ]
*/

#ifndef SDL_VKEYBOARD_BASE_H
#define SDL_VKEYBOARD_BASE_H

#include "../../common.h"
#include "../vkeyboardbase.h"

class FIFOINT;

namespace Vkbd {

/**
	@brief VKeyboard Base for SDL
*/
class OSDBase : public Base
{
protected:
	bool load_bitmap(const _TCHAR *res_path);
	void unload_bitmap();
	bool create_surface();
	bool create_bitmap(const _TCHAR *res_path, const _TCHAR *bmp_file, CBitmap **suf);

//	void update_parts(const Pos_t *, const Hori_t *, bool);
//	void mouse_up(const Pos_t *, const Hori_t *);

//	inline void set_pressed_info(PressedInfo_t *, short, short, short, short, short, short);

//	virtual bool update_status_one(short, bool);

//	virtual void need_update_led(short, LedStat_t &);
	virtual void need_update_window_base(PressedInfo_t *, bool);
	virtual void update_window() {}

	virtual void fill_rect(SDL_Rect *);
	virtual bool blit_surface(CBitmap *, SDL_Rect &, SDL_Rect &);

public:
	OSDBase();
	virtual ~OSDBase();

//	virtual void SetStatusBufferPtr(uint8_t *, int, uint8_t);
//	virtual void SetHistoryBufferPtr(FIFOINT *);

	virtual void Show(bool = true) {}
//	virtual void Close();

//	virtual void MouseDown(int, int);
//	virtual void MouseUp();

//	virtual bool UpdateStatus(uint32_t);
};

} /* namespace Vkbd */

#if defined(USE_WX) || defined(USE_WX2)
#include "../../gui/wxwidgets/wx_vkeyboard.h"
#elif defined(_WIN32)
#include "../../gui/windows/win_vkeyboard.h"
#elif defined(__MACH__) && defined(__APPLE__)
#include "../../gui/cocoa/cocoa_vkeyboard.h"
#elif defined(SDL_VIDEO_DRIVER_X11) && defined(USE_X11_VKEYBOARD)
#include "../../gui/gtk_x11/x11_vkeyboard.h"
#elif defined(USE_GTK_VKEYBOARD)
#include "../../gui/gtk_x11/gtk_vkeyboard.h"
#else
namespace Vkbd {

class VKeyboard : public OSDBase
{
public:
	virtual bool Create(const _TCHAR *) { return false; }
};

} /* namespace Vkbd */
#endif

#endif /* SDL_VKEYBOARD_BASE_H */

