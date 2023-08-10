/** @file win_vkeyboardbase.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.09.22 -

	@brief [ virtual keyboard ]
*/

#ifndef WIN_VKEYBOARD_BASE_H
#define WIN_VKEYBOARD_BASE_H

#include "../../common.h"
#include "../vkeyboardbase.h"

class FIFOINT;

namespace Vkbd {

/**
	@brief VKeyboard Base for Windows
*/
class OSDBase : public Base
{
protected:
	HBRUSH      hBrush;

	void load_bitmap();
	void unload_bitmap();
	bool create_surface();

//	void update_parts(const Pos_t *, const Hori_t *, bool);
//	void mouse_up(const Pos_t *, const Hori_t *);

//	inline void set_pressed_info(PressedInfo_t *, short, short, short, short, short, short);

//	virtual bool update_status_one(short, bool);

//	virtual void need_update_led(short, LedStat_t &);
	virtual void need_update_window_base(PressedInfo_t *, bool);
	virtual void update_window() {}

	virtual void fill_rect(Rect_t *);
	virtual bool blit_surface(CBitmap *, short, short, Rect_t *);

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

#if defined(_WIN32)
#include "../../gui/windows/win_vkeyboard.h"
#endif

#endif /* WIN_VKEYBOARD_BASE_H */

