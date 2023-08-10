/** @file vkeyboardbase.h

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.12.21 -

	@brief [ virtual keyboard ]
*/

#ifndef VKEYBOARD_BASE_H
#define VKEYBOARD_BASE_H

#include "../common.h"
#include "../csurface.h"
#include "../cbitmap.h"

class FIFOINT;

namespace Vkbd {

enum enKeyKindIndex {
	KEYKIND_NOANIME = -1,
	KEYKIND_NORMAL = 0,
	KEYKIND_ARRAY = 1,
	KEYKIND_TOGGLE = 2
};

typedef struct {
	short x;
	short w;
	short code;
	short kind;	// 0:normal 1:arraykey 2:togglekey
	short kidx;
	short parts_num; // bitmap parts number
	short led_parts_num;	// led parts number
} Hori_t;

typedef struct {
	short y;
	short h;
	const Hori_t *px;
} Pos_t;

enum enArrayKeysIndex {
	ARRAYKEYS_RETURN = 0,
	ARRAYKEYS_END
};

enum enToggleKeysIndex {
	TOGGLEKEYS_MODE = 0,
	TOGGLEKEYS_SHIFT,
	TOGGLEKEYS_CTRL,
	TOGGLEKEYS_GRAPH,
	TOGGLEKEYS_END
};

enum enBitmapIdsIndex {
	BITMAPIDS_BASE = 0,
	BITMAPIDS_VKEY_LED0,
	BITMAPIDS_VKEY_LED1,
	BITMAPIDS_VKEY_LED2,
	BITMAPIDS_VKEY_LED3,
	BITMAPIDS_END
};

enum enBitmapPartsIndex {
	BITMAPPARTS_HIRAGANA = 0,
	BITMAPPARTS_ZENKAKU,
	BITMAPPARTS_KANA,
	BITMAPPARTS_ROMAJI,
	BITMAPPARTS_CODEIN,
	BITMAPPARTS_INSERT,
	BITMAPPARTS_CAPSLOCK,
	BITMAPPARTS_END
};

typedef struct stBitmap_t {
	short idx;	
	short x;
	short y;
	short w;
	short h;
} Bitmap_t;

enum enLedPartsIndex {
	LEDPARTS_HIRAGANA = 0,
	LEDPARTS_ZENKAKU,
	LEDPARTS_KANA,
	LEDPARTS_ROMAJI,
	LEDPARTS_CODEIN,
	LEDPARTS_INSERT,
	LEDPARTS_CAPSLOCK,
	LEDPARTS_END
};

typedef struct stLedPos_t {
	short parts_num;
	short x;
	short y;
} LedPos_t;

// constant tables
extern const struct stLedPos_t cLedPos[];
extern const struct stBitmap_t cBmpParts[];

/**
	@brief VKeyboard Base
*/
class Base
{
protected:
	typedef struct {
		short left;
		short top;
		short right;
		short bottom;
	} Rect_t;

	typedef struct {
		Rect_t re;
		short parts_num; // bitmap parts number
		short led_bitmap_num;	// bitmap number for led
		short led_parts_num;	// led parts number
	} PressedInfo_t;

	typedef struct {
		short kind;
//		bool pressed;
		short code;
		short array_idx;
//		PressedInfo_t info;
	} PressedKey_t;

	typedef struct {
		short code;
		short nums;	// item nums of arr 
		PressedInfo_t *arr;
	} ArrKeys_t;

	typedef struct {
		ArrKeys_t a;
	} ArrayKeys_t;

	typedef struct {
		ArrKeys_t a;
		bool pressed;
	} ToggleKeys_t;

	int offset_x;
	int offset_y;

	bool prepared;
	bool closed;

	PressedKey_t pressed_key;

//	short pushed_array_key_idx;
	ArrayKeys_t array_keys[ARRAYKEYS_END];

	ToggleKeys_t toggle_keys[TOGGLEKEYS_END];

	typedef struct {
		short vert;
		short hori;
	} PosHoriMap_t;
	PosHoriMap_t *code_rev_map;
	int code_rev_map_size;

//	short noanime_key_code;

	typedef struct {
		bool on;
		short code;
	} LedStat_t;
	LedStat_t led_stat[LEDPARTS_END];

	uint8_t *key_status;
	uint8_t key_status_mask;
	int key_status_size;
	FIFOINT *key_history;
	int led_bright;

	CSurface   *pSurface;
	CBitmap	   *pBitmaps[BITMAPIDS_END];

//	void load_bitmap();
	virtual void unload_bitmap();
	virtual bool create_surface() = 0;

	void update_parts(const Pos_t *, const Hori_t *, bool);

	void set_pressed_info(PressedInfo_t *, short, short, short, short, short, short);

	virtual bool update_status_one(short, bool);

	virtual void need_update_led(short, LedStat_t &);
	virtual void need_update_window(PressedInfo_t *, bool);
	virtual void need_update_window_base(PressedInfo_t *, bool) = 0;
	virtual void update_window() {}

public:
	Base();
	virtual ~Base();

	virtual void SetStatusBufferPtr(uint8_t *, int, uint8_t);
	virtual void SetHistoryBufferPtr(FIFOINT *);

	virtual void Show(bool = true);
	virtual void Close() = 0;
	virtual void CloseBase();

	virtual void MouseDown(int, int);
	virtual void MouseUp();

	virtual bool UpdateStatus(uint32_t);
};

} /* namespace Vkbd */

#endif /* VKEYBOARD_BASE_H */

