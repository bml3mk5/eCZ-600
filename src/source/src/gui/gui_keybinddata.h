/** @file gui_keybinddata.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.5.12

	@brief [ keybind data for gui ]
*/

#ifndef GUI_KEYBINDDATA_H
#define GUI_KEYBINDDATA_H

#include "../common.h"
#include "../vm/vm.h"
#include "../emu.h"
#include "../osd/keybind.h"

#define KBLABEL_MAXLEN	100

#define KBCTRL_MAX_LINES 128
#define KBCTRL_MAX_COLS  2

/// keycode on a column in keybind data
typedef struct codecols_st {
	int     col;
	uint32_t  vk_keycode;
	uint32_t  vk_prev_keycode;
} codecols_t;

/// keycodes per row in keybind data
typedef struct codetable_st {
	int        row;
	codecols_t cols[KBCTRL_MAX_COLS];
	uint16_t   vm_keycode;
	uint16_t   flags;
} codetable_t;

typedef struct rowcol_st {
	int row;
	int col;
} rowcol_t;

typedef struct key_labels_st {
	int id;
	const _TCHAR *label;
} key_labels_t;

extern const key_labels_t cVmJoyLabels[];

enum enVmJoyLabels {
	VM_JOY_LABEL_UP = 1,
	VM_JOY_LABEL_UPRIGHT,
	VM_JOY_LABEL_RIGHT,
	VM_JOY_LABEL_DOWNRIGHT,
	VM_JOY_LABEL_DOWN,
	VM_JOY_LABEL_DOWNLEFT,
	VM_JOY_LABEL_LEFT,
	VM_JOY_LABEL_UPLEFT,
	VM_JOY_LABEL_YA_UP,
	VM_JOY_LABEL_YA_UPRIGHT,
	VM_JOY_LABEL_YA_RIGHT,
	VM_JOY_LABEL_YA_DOWNRIGHT,
	VM_JOY_LABEL_YA_DOWN,
	VM_JOY_LABEL_YA_DOWNLEFT,
	VM_JOY_LABEL_YA_LEFT,
	VM_JOY_LABEL_YA_UPLEFT,
	VM_JOY_LABEL_ALLOWS_END,
	VM_JOY_LABEL_BUTTON_A,
	VM_JOY_LABEL_BUTTON_B,
	VM_JOY_LABEL_BUTTON_C,
	VM_JOY_LABEL_BUTTON_D,
	VM_JOY_LABEL_BUTTON_E1,
	VM_JOY_LABEL_BUTTON_E2,
	VM_JOY_LABEL_BUTTON_L,
	VM_JOY_LABEL_BUTTON_R,
	VM_JOY_LABEL_BUTTON_SELECT,
	VM_JOY_LABEL_BUTTON_START,
	VM_JOY_LABEL_BUTTONS_END,
	VM_JOY_LABEL_ESCAPE,
	VM_JOY_LABEL_PAUSE,
	VM_JOY_LABEL_KEYS_END,
	VM_JOY_LABEL_LEFT_ANALOG_X,
	VM_JOY_LABEL_LEFT_ANALOG_Y,
	VM_JOY_LABEL_RIGHT_ANALOG_X,
	VM_JOY_LABEL_RIGHT_ANALOG_Y,
	VM_JOY_LABELS_MAX
};

class EMU;

/// keybind data for GUI
class KeybindData
{
protected:
	int m_max_tabs;
	int m_max_rows;

	codetable_t table[KBCTRL_MAX_LINES];
	int     vmkey_map_size;
	int		row2idx_map[KBCTRL_MAX_LINES];
	/// assign key code uniquely
	rowcol_t vkkey2rowcol_map[KEY_STATUS_SIZE];

	const uint32_key_assign_t *vkkey_defmap;
	int     vkkey_defmap_rows;
	int     vkkey_defmap_cols;

	uint32_key_assign_t *vkkey_map;
	uint32_key_assign_t *vkkey_preset_map[KEYBIND_PRESETS];

	uint32_t *p_joy_mask;

private:
	void Init(EMU *emu, int new_tabnum, int new_devtype, int new_vmtype, int new_flags);

public:
	int m_tab_num;
	enum en_devtypes {
		DEVTYPE_KEYBOARD = 0,
		DEVTYPE_JOYPAD,
	};
	int m_devtype;	// 1:joypad 0:keyboard
	enum en_vm_types {
		VM_TYPE_KEYASSIGN = 0,
		VM_TYPE_PIOJOYASSIGN,
		VM_TYPE_PIOBITASSIGN,
	};
	int m_vm_type;	// 2:PIA(SKIPPER) 1:PIA(S1) 0:key
	enum en_flags {
		FLAG_DENY_DUPLICATE = 0x0001,
	};
	int m_flags;

	enum en_tabs {
		TAB_KEY2KEY = 0,
#ifdef USE_JOYSTICK
		TAB_JOY2KEY,
#if defined(USE_PIAJOYSTICK) || defined(USE_PIAJOYSTICKBIT)
		TAB_JOY2JOY,
#endif
#endif
#ifdef USE_KEY2JOYSTICK
		TAB_KEY2JOY,
#endif
		TABS_MAX,
	};

	struct st_type {
		int devtype;
		int vm_type;
		int flags;
	};
	static const struct st_type cTypes[];

	KeybindData();
	virtual ~KeybindData();

	virtual void Init(EMU *emu, int new_tabnum);

	virtual void SetVmKeyMap(const uint16_t *vmKeyMap, int size);
	virtual void SetVmKey(int idx, uint16_t code);
	virtual bool SetVmKeyCode(int idx, uint16_t code);
	virtual void SetVkKeyMap(uint32_key_assign_t *vkKeyMap);
	virtual void SetVkKeyDefMap(const uint32_key_assign_t *vkKeyDefMap, int rows);
	virtual void SetVkKeyPresetMap(uint32_key_assign_t *vkKeyMap, int idx);
	virtual bool SetVkKeyCode(codetable_t *tbl, int col, uint32_t code, _TCHAR *label);
	virtual bool SetVkKeyCode(int row, int col, uint32_t code, _TCHAR *label);
	virtual bool ClearVkKeyCode(codetable_t *tbl, int col, _TCHAR *label);
	virtual bool ClearVkKeyCode(int row, int col, _TCHAR *label);
	virtual bool ClearCellByVkKeyCode(uint32_t code, _TCHAR *label, int *row = NULL, int *col = NULL);
	virtual bool SetVkJoyCode(codetable_t *tbl, int col, uint32_t code0, uint32_t code1, _TCHAR *label);
	virtual bool SetVkJoyCode(int row, int col, uint32_t code0, uint32_t code1, _TCHAR *label);
	virtual bool ClearVkJoyCode(codetable_t *tbl, int col, _TCHAR *label);
	virtual bool ClearVkJoyCode(int row, int col, _TCHAR *label);
	virtual bool SetVkCode(int row, int col, uint32_t code, _TCHAR *label);
	virtual bool ClearVkCode(int row, int col, _TCHAR *label);

	virtual uint32_t GetCombi();
	virtual void SetCombi(uint32_t value);

	virtual void LoadDefaultPreset();
	virtual void LoadPreset(int idx);
	virtual void SavePreset(int idx);

	virtual void SetData();

	void SetJoyMask(uint32_t *mask) { p_joy_mask = mask; }

//	void SetNumberOfTabs(int tabs) { m_max_tabs = tabs; }
	int GetNumberOfTabs() const { return m_max_tabs; }

	int GetNumberOfRows() const { return m_max_rows; }
	int GetNumberOfColumns() const { return vkkey_defmap_cols; }
	virtual const _TCHAR *GetCellString(int row, int col);
	virtual bool GetCellString(int row, int col, _TCHAR *label);
	virtual bool IsEnable(int index);

	virtual bool FindRowColFromVkKey(uint32_t code, int *row, int *col);
	virtual int FindRowFromVmKey(uint32_t code);
	virtual int FindColFromVkKey(int row, uint32_t code);

	static bool GetVmKeyLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVmJoyLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVmJoyBitLabel(int code, _TCHAR *label, bool translate = false);
	static bool GetVkKeyLabel(uint32_t code, _TCHAR *label, bool translate = false);
	static bool GetVkJoyLabel(uint32_t code, _TCHAR *label, bool translate = false);
	static bool GetVkJoyAnLabel(uint32_t code, _TCHAR *label, bool translate = false);
};

#endif /* GUI_KEYBINDDATA_H */
