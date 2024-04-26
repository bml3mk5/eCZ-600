/** @file gui_keybinddata.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2015.5.12

	@brief [ keybind data for gui ]
*/

#include "gui_keybinddata.h"
#include "../clocale.h"
#include "../utility.h"
#include "../keycode.h"

const struct KeybindData::st_type KeybindData::cTypes[] = {
	{ DEVTYPE_KEYBOARD,	VM_TYPE_KEYASSIGN,	FLAG_DENY_DUPLICATE },
#if defined(USE_JOYSTICK)
	{ DEVTYPE_JOYPAD,	VM_TYPE_KEYASSIGN,	0 },
#if defined(USE_PIAJOYSTICK)
# if !defined(USE_PIAJOYSTICKBIT)
	{ DEVTYPE_JOYPAD,	VM_TYPE_PIOJOYASSIGN,	0 },
#else
	{ DEVTYPE_JOYPAD,	VM_TYPE_PIOBITASSIGN,	0 },
# endif
#endif
#endif
#ifdef USE_KEY2JOYSTICK
# if !defined(USE_PIAJOYSTICKBIT)
	{ DEVTYPE_KEYBOARD,	VM_TYPE_PIOJOYASSIGN,	0 },
#else
	{ DEVTYPE_KEYBOARD,	VM_TYPE_PIOBITASSIGN,	0 },
# endif
#endif
	{ -1, -1, 0 },
}; 

KeybindData::KeybindData()
{
	m_max_tabs = TABS_MAX;
	m_tab_num = 0;
	m_devtype = DEVTYPE_KEYBOARD;
	m_vm_type = VM_TYPE_KEYASSIGN;
	m_flags = FLAG_DENY_DUPLICATE;
	p_joy_mask = NULL;
}

KeybindData::~KeybindData()
{
}

void KeybindData::Init(EMU *emu, int new_tabnum)
{
	Init(emu, new_tabnum, cTypes[new_tabnum].devtype, cTypes[new_tabnum].vm_type, cTypes[new_tabnum].flags);
}

void KeybindData::Init(EMU *emu, int new_tabnum, int new_devtype, int new_vmtype, int new_flags)
{
	m_tab_num = new_tabnum;
	m_devtype = new_devtype;
	m_vm_type = new_vmtype;
	m_flags = new_flags;

	// get parameters
	memset(table, 0, sizeof(table));
	memset(row2idx_map, 0, sizeof(row2idx_map));
	vmkey_map_size = 0;
	for(int i=0; i<KEY_STATUS_SIZE; i++) {
		vkkey2rowcol_map[i].row = -1;
		vkkey2rowcol_map[i].col = -1;
	}

	vkkey_defmap = NULL;
	vkkey_defmap_rows = 0;
	vkkey_defmap_cols = 0;

	vkkey_map = NULL;
	memset(vkkey_preset_map, 0, sizeof(vkkey_preset_map));

	// set initial data
	int presets = gKeybind.GetVkKeyPresetSize();

	SetVmKeyMap(gKeybind.GetVmKeyMap(m_tab_num), gKeybind.GetVmKeyMapSize(m_tab_num));

	SetVkKeyDefMap(gKeybind.GetVkKeyDefMap(m_tab_num), gKeybind.GetVkKeyMapSize(m_tab_num));

	SetVkKeyMap(gKeybind.GetVkKeyMap(m_tab_num));

	for(int j=0; j<presets; j++) {
		SetVkKeyPresetMap(gKeybind.GetVkKeyPresetMap(m_tab_num, j), j);
	}

	// set disable key
	m_max_rows = 0;
	for(int idx=0; idx<KBCTRL_MAX_LINES; idx++) {
		if (table[idx].vm_keycode >= vkkey_defmap_rows) {
			table[idx].flags &= ~CODE_TABLE_FLAG_ENABLE;
		}

		if (table[idx].flags & CODE_TABLE_FLAG_ENABLE) {
			table[idx].row = m_max_rows;
			row2idx_map[m_max_rows] = idx;
			m_max_rows++;
		} else {
			table[idx].row = -1;
		}

		uint32_t code = 0;
		for(int col=0; col<vkkey_defmap_cols; col++) {
			table[idx].cols[col].col = col;
			if (table[idx].flags & CODE_TABLE_FLAG_ENABLE) {
				code = vkkey_map[table[idx].vm_keycode].d[col];
			}
			if (m_devtype == DEVTYPE_JOYPAD) {
				SetVkJoyCode(&table[idx],col,code,code,NULL);
			} else {
				SetVkKeyCode(&table[idx],col,code,NULL);
			}
		}
	}
}

/// @brief Set key map for virtual machine
///
/// @param[in] vmKeyMap : array of key map
/// @param[in] size     : size of key map
void KeybindData::SetVmKeyMap(const uint16_t *vmKeyMap, int size)
{
	if (m_tab_num < 0 || m_tab_num >= m_max_tabs) return;

	for(int i=0; i<KBCTRL_MAX_LINES && i<size; i++) {
		SetVmKeyCode(i,vmKeyMap[i]);
	}
	vmkey_map_size = (size > KBCTRL_MAX_LINES) ? KBCTRL_MAX_LINES : size;
}

/// @brief Set code for virtual machine
///
/// @param[in] idx  : index of key map
/// @param[in] code : key code
void KeybindData::SetVmKey(int idx, uint16_t code)
{
	if (m_tab_num < 0 || m_tab_num >= m_max_tabs) return;

	//	vmkey_map[idx] = code;
	SetVmKeyCode(idx,code);
}

/// @brief Set code for virtual machine
///
/// @param[in] idx  : index of key map
/// @param[in] code : key code
/// @return true
bool KeybindData::SetVmKeyCode(int idx, uint16_t code)
{
	if (idx < 0 || idx >= KBCTRL_MAX_LINES) {
		return false;
	}
	if ((code & 0xff) == 0xff) {
		table[idx].flags &= ~CODE_TABLE_FLAG_ENABLE;
	} else {
		table[idx].flags |= CODE_TABLE_FLAG_ENABLE;
	}

	table[idx].vm_keycode = (code & 0xff);

	if (code & 0xff00) {
		table[idx].flags |= (code & 0xff00);
	}

	return true;
}

/// @brief Set key map for host machine
///
/// @param[in] vkKeyMap : array of key map
void KeybindData::SetVkKeyMap(uint32_key_assign_t *vkKeyMap)
{
	if (m_tab_num < 0 || m_tab_num >= m_max_tabs) return;

	vkkey_map = vkKeyMap;
}

/// @brief Set default key map for host machine
///
/// @param[in] vkKeyDefMap : array of key map
/// @param[in] rows        : rows of key map
void KeybindData::SetVkKeyDefMap(const uint32_key_assign_t *vkKeyDefMap, int rows)
{
	if (m_tab_num < 0 || m_tab_num >= m_max_tabs) return;

	vkkey_defmap = vkKeyDefMap;
	vkkey_defmap_rows = rows;
	vkkey_defmap_cols = (int)(sizeof(uint32_key_assign_t)/sizeof(uint32_t));
}

/// @brief Set preset key map for host machine
///
/// @param[in] vkKeyMap : array of key map
/// @param[in] idx      : preset number
void KeybindData::SetVkKeyPresetMap(uint32_key_assign_t *vkKeyMap, int idx)
{
	if (m_tab_num < 0 || m_tab_num >= m_max_tabs) return;

	if (idx < 4) {
		vkkey_preset_map[idx] = vkKeyMap;
	}
}

/// @brief Set key code for host machine
///
/// @param[in] tbl    : row table in key map
/// @param[in] col    : column in key map
/// @param[in] code   : key code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkKeyCode(codetable_t *tbl, int col, uint32_t code, _TCHAR *label)
{
	if (label != NULL) GetVkKeyLabel(code,label,true);

	codecols_t *obj = &tbl->cols[col];
	obj->vk_prev_keycode = obj->vk_keycode;

	vkkey2rowcol_map[code].row = tbl->row;
	vkkey2rowcol_map[code].col = obj->col;
	obj->vk_keycode = code;
	return true;
}

/// @brief Set key code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[in] code   : key code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkKeyCode(int row, int col, uint32_t code, _TCHAR *label)
{
	int idx = row2idx_map[row];
	return SetVkKeyCode(&table[idx],col,code,label);
}

/// @brief Clear key code for host machine
///
/// @param[in] tbl    : row table in key map
/// @param[in] col    : column in key map
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::ClearVkKeyCode(codetable_t *tbl, int col, _TCHAR *label)
{
	if (label != NULL) GetVkKeyLabel(0,label,true);

	codecols_t *obj = &tbl->cols[col];
	vkkey2rowcol_map[obj->vk_keycode].row = -1;
	vkkey2rowcol_map[obj->vk_keycode].col = -1;
	obj->vk_prev_keycode = obj->vk_keycode;
	obj->vk_keycode = 0;
	return true;
}

/// @brief Clear key code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::ClearVkKeyCode(int row, int col, _TCHAR *label)
{
	int idx = row2idx_map[row];
	return ClearVkKeyCode(&table[idx],col,label);
}

/// @brief Clear cell by vk key code
///
/// @param[in] code   : key code
/// @param[out] label : label string of specified code
/// @param[out] row   : row of specified code
/// @param[out] col   : col of specified code
/// @return true / false
bool KeybindData::ClearCellByVkKeyCode(uint32_t code, _TCHAR *label, int *row, int *col)
{
	int nrow = vkkey2rowcol_map[code].row;
	int ncol = vkkey2rowcol_map[code].col;
	if (nrow >= 0 && ncol >= 0) {
		if (row) *row = nrow;
		if (col) *col = ncol;
		return ClearVkKeyCode(nrow, ncol, label);
	}
	if (row) *row = -1;
	if (col) *col = -1;
	return false;
}

/// @brief Set joystick code for host machine
///
/// @param[in] tbl    : row table in key map
/// @param[in] col    : column in key map
/// @param[in] code0  : joystick code
/// @param[in] code1  : joystick code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkJoyCode(codetable_t *tbl, int col, uint32_t code0, uint32_t code1, _TCHAR *label)
{
	bool rc = true;
	if (label != NULL) {
		if (tbl->flags & CODE_TABLE_FLAG_JOYANA) {
			rc = GetVkJoyAnLabel(code0,label,true);
		} else {
			rc = GetVkJoyLabel(code0,label,true);
		}
	}
	if (rc == true) {
		codecols_t *obj = &tbl->cols[col];
		obj->vk_prev_keycode = obj->vk_keycode;
		if (tbl->flags & CODE_TABLE_FLAG_JOYANA) {
			obj->vk_keycode = code0;
		} else {
			obj->vk_keycode = code0;
		}
	}
	return true;
}

/// @brief Set joystick code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : column in key map
/// @param[in] code0  : joystick code
/// @param[in] code1  : joystick code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkJoyCode(int row, int col, uint32_t code0, uint32_t code1, _TCHAR *label)
{
	int idx = row2idx_map[row];
	return SetVkJoyCode(&table[idx],col,code0,code1,label);
}

/// @brief Clear joystick code for host machine
///
/// @param[in] tbl    : row table in key map
/// @param[in] col    : column in key map
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::ClearVkJoyCode(codetable_t *tbl, int col, _TCHAR *label)
{
	bool rc = true;
	if (label != NULL) {
		rc = GetVkJoyLabel(0,label,true);
	}
	if (rc == true) {
		codecols_t *obj = &tbl->cols[col];
		obj->vk_prev_keycode = obj->vk_keycode;
		obj->vk_keycode = 0;
	}
	return true;
}

/// @brief Clear joystick code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::ClearVkJoyCode(int row, int col, _TCHAR *label)
{
	int idx = row2idx_map[row];
	return ClearVkJoyCode(&table[idx],col,label);
}

/// @brief Set code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[in] code   : code
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::SetVkCode(int row, int col, uint32_t code, _TCHAR *label)
{
	if (m_devtype == DEVTYPE_JOYPAD) {
		return SetVkJoyCode(row, col, code, code, label);
	} else {
		return SetVkKeyCode(row, col, code, label);
	}
}

/// @brief Clear code for host machine
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[out] label : label string of specified code
/// @return true
bool KeybindData::ClearVkCode(int row, int col, _TCHAR *label)
{
	if (m_devtype == DEVTYPE_JOYPAD) {
		return ClearVkJoyCode(row, col, label);
	} else {
		return ClearVkKeyCode(row, col, label);
	}
}

/// @brief get combination flag
///
/// @return value
uint32_t KeybindData::GetCombi()
{
	return vkkey_map[vkkey_defmap_rows - 1].d[0];
//	return *(vkkey_map + (vkkey_defmap_rows - 1) * vkkey_defmap_cols);
}

/// @brief set combination flag
///
/// @param[in] value
void KeybindData::SetCombi(uint32_t value)
{
	vkkey_map[vkkey_defmap_rows - 1].d[0] = value;
//	*(vkkey_map + (vkkey_defmap_rows - 1) * vkkey_defmap_cols) = value;
}

/// @brief get cell string
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @return label string
const _TCHAR *KeybindData::GetCellString(int row, int col)
{
	static _TCHAR label[128];
	GetCellString(row, col, label);
	return label;
}

/// @brief get cell string
///
/// @param[in] row    : row in key map
/// @param[in] col    : col in key map
/// @param[out] label : string
/// @return true / false
bool KeybindData::GetCellString(int row, int col, _TCHAR *label)
{
	bool rc = false;
	int idx = row2idx_map[row];
	if(col < 0) {
		if (m_vm_type == VM_TYPE_PIOBITASSIGN) rc = GetVmJoyBitLabel(idx, label, true);
		else if (m_vm_type == VM_TYPE_PIOJOYASSIGN) rc = GetVmJoyLabel(idx, label, true);
		else rc = GetVmKeyLabel(idx, label, true);
	} else {
		codetable_t *tbl = &table[idx];
		uint32_t code = tbl->cols[col].vk_keycode;
		if (m_devtype == DEVTYPE_JOYPAD) {
			if (tbl->flags & CODE_TABLE_FLAG_JOYANA) {
				rc = GetVkJoyAnLabel(code,label,true);
			} else {
				rc = GetVkJoyLabel(code,label,true);
			}
		} else {
			rc = GetVkKeyLabel(code,label,true);
		}
	}
	return rc;
}

/// @brief table is valid ?
///
/// @param[in] index : table number
bool KeybindData::IsEnable(int index)
{
	return (table[index].flags & CODE_TABLE_FLAG_ENABLE) != 0;
}

/// @brief find row and col in table from vk key code
///
/// @param[in] code
/// @param[out] row
/// @param[out] col
/// @return true if found
bool KeybindData::FindRowColFromVkKey(uint32_t code, int *row, int *col)
{
	bool found = false;
	for(int idx = 0; idx<KBCTRL_MAX_LINES; idx++) {
		if (!(table[idx].flags & CODE_TABLE_FLAG_ENABLE)) continue;
		for(int pos = 0; pos<KBCTRL_MAX_COLS; pos++) {
			if (table[idx].cols[pos].vk_keycode == code) {
				found = true;
				if (row) *row = table[idx].row;
				if (col) *col = table[idx].cols[pos].col;
				break;
			}
		}
	}
	return found;
}

/// @brief find row in table from vm key code
///
/// @param[in] code
/// @return row / -1 if not found
int KeybindData::FindRowFromVmKey(uint32_t code)
{
	int found = -1;
	for(int idx = 0; idx<KBCTRL_MAX_LINES; idx++) {
		if (!(table[idx].flags & CODE_TABLE_FLAG_ENABLE)) continue;
		if (table[idx].vm_keycode == code) {
			found = table[idx].row;
			break;
		}
	}
	return found;
}

/// @brief find col in table from vk key code
///
/// @param[in] row
/// @param[in] code
/// @return row / -1 if not found
int KeybindData::FindColFromVkKey(int row, uint32_t code)
{
	int found = -1;
	if (row < 0 || row >= m_max_rows) return found;
	int idx = row2idx_map[row];
	for(int pos = 0; pos<KBCTRL_MAX_COLS; pos++) {
		if (table[idx].cols[pos].vk_keycode == code) {
			found = pos;
			break;
		}
	}
	return found;
}

/// @brief load default preset
void KeybindData::LoadDefaultPreset()
{
	LoadPreset(-1);
}

/// @brief load preset
///
/// @param[in] idx : preset number
void KeybindData::LoadPreset(int idx)
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	const uint32_key_assign_t *dst;
	uint32_t code;

	if (0 <= idx && idx <= KEYBIND_PRESETS) {
		dst = vkkey_preset_map[idx];
	} else {
		dst = vkkey_defmap;
	}

	for(int row=0; row<rows; row++) {
		if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
			for(int col=0; col<cols; col++) {
				code = dst[table[row].vm_keycode].d[col];
				if (m_devtype == DEVTYPE_JOYPAD) {
					SetVkJoyCode(&table[row],col,code,code,NULL);
				} else {
					SetVkKeyCode(&table[row],col,code,NULL);
				}
			}
		}
	}
	if (m_devtype == DEVTYPE_JOYPAD && (m_vm_type == VM_TYPE_KEYASSIGN || m_vm_type == VM_TYPE_PIOBITASSIGN)) {
		SetCombi(dst[vkkey_defmap_rows - 1].d[0]);
	}
}

/// @brief save preset
///
/// @param[in] idx : preset number
void KeybindData::SavePreset(int idx)
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	uint32_key_assign_t *dst;
	uint32_t code;

	if (0 <= idx && idx <= KEYBIND_PRESETS) {
		dst = vkkey_preset_map[idx];
	} else {
		return;
	}

	for(int row=0; row<rows; row++) {
		if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
			for(int col=0; col<cols; col++) {
				code = table[row].cols[col].vk_keycode;
				dst[table[row].vm_keycode].d[col] = code;
			}
		}
	}
	if (m_devtype == DEVTYPE_JOYPAD && (m_vm_type == VM_TYPE_KEYASSIGN || m_vm_type == VM_TYPE_PIOBITASSIGN)) {
		dst[vkkey_defmap_rows - 1].d[0] = GetCombi();
	}
}

/// @brief set data to table
void KeybindData::SetData()
{
	int rows = vmkey_map_size;
	int cols = vkkey_defmap_cols;
	uint32_key_assign_t *dst = vkkey_map;
	uint32_t code = 0;

	for(int row=0; row<rows; row++) {
		if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
			for(int col=0; col<cols; col++) {
				code = table[row].cols[col].vk_keycode;
				dst[table[row].vm_keycode].d[col] = code;
			}
		}
	}
	switch(m_devtype) {
	case DEVTYPE_KEYBOARD:
		switch(m_vm_type) {
		case VM_TYPE_KEYASSIGN:
			// make keyboard to keycode mapping
			emu->clear_vm_key_map();
			for(int row=0; row<rows; row++) {
				if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
					int vm_code = table[row].vm_keycode;
					for(int col=0; col<cols; col++) {
						code = table[row].cols[col].vk_keycode;
						emu->set_vm_key_map(code, vm_code);
					}
				}
			}
			break;
		case VM_TYPE_PIOJOYASSIGN:
		case VM_TYPE_PIOBITASSIGN:
			// make keyboard to joyport mapping
			emu->clear_key2joy_map();
			for(int row=0, bidx = -1; row<rows; row++) {
				if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
					uint32_t joy_code = (table[row].vm_keycode & 0xff);
					for(int col=0; col<cols; col++) {
						code = table[row].cols[col].vk_keycode;
						if (table[row].flags & CODE_TABLE_FLAG_JOYBTN) {
							if (bidx < 0) bidx = joy_code;
							emu->set_key2joy_map(code, col, (joy_code - bidx) | 0x80000000);
						} else {
							emu->set_key2joy_map(code, col, joy_code);
						}
					}
				}
			}
			break;
		default:
			break;
		}
		break;
	case DEVTYPE_JOYPAD:
		switch(m_vm_type) {
		case VM_TYPE_PIOJOYASSIGN:
		case VM_TYPE_PIOBITASSIGN:
			// make joypad to joyport mapping
			emu->clear_joy2joy_map();
			emu->clear_joy2joyk_map();
			for(int row=0, anaidx = -1; row<rows; row++) {
				if (table[row].flags & CODE_TABLE_FLAG_ENABLE) {
					uint8_t idx = (table[row].vm_keycode & 0xff);
					for(int col=0; col<cols; col++) {
						code = table[row].cols[col].vk_keycode;
						if (table[row].flags & CODE_TABLE_FLAG_JOYANA) {
							if (anaidx < 0) anaidx = idx;
							emu->set_joy2joy_ana_map(col, idx - anaidx, code);
						} else if (table[row].flags & CODE_TABLE_FLAG_JOYKEY) {
							emu->set_joy2joyk_map(col, idx, code);
						} else {
							emu->set_joy2joy_map(col, idx, code);
						}
					}
				}
			}
			break;
		default:
			break;
		}
	default:
		break;
	}
}

// ----------------------------------------------------------------------

/// array of vm key label
const key_labels_t sVmKeyLabels[] = {
	// 0x00
	{ -1, _T("") },
	{ CMsg::Power_Switch, _T("Power Switch") },
	{ CMsg::Reset_Switch, _T("Reset Switch") },
	{ CMsg::Interrupt_Switch, _T("Interrupt Switch") },
	{ CMsg::Bracket_Mouse_OnOff, _T("(Mouse On/Off)") },
	{ CMsg::Bracket_Pause, _T("(Pause)") },
	// 0x06
#if defined(__APPLE__) && defined(__MACH__)
	{ -1, _T("(option)") },
#else
	{ -1, _T("(Alt)") },
#endif
	// 0x07
	{ -1, _T("INS") },
	{ -1, _T("DEL") },
	{ -1, _T("BS") },
	{ -1, _T("TAB") },
	{ -1, _T("BREAK") },
	{ -1, _T("HOME") },
	{ -1, _T("CLR") },
	{ -1, _T("RETURN") },
	{ -1, _T("ENTER") },
	// 0x10
	{ -1, _T("SHIFT") },
	{ -1, _T("CTRL") },
	{ -1, _T("CAPS LOCK") },
	{ CMsg::KANA, _T("KANA") },
	{ CMsg::ROMAJI, _T("ROMA-JI") },
	{ CMsg::CODE_INPUT, _T("CODE INPUT") },
	{ CMsg::HIRAGANA, _T("HIRAGANA") },
	{ CMsg::ZENKAKU, _T("ZENKAKU") },
	{ CMsg::KIGOU_INPUT, _T("KIGOU INPUT") },
	{ CMsg::TOUROKU, _T("TOUROKU") },
	{ -1, _T("COPY") },
	{ -1, _T("ESC") },
	{ CMsg::Allow_RIGHT, _T("RIGHT") },
	{ CMsg::Allow_LEFT, _T("LEFT") },
	{ CMsg::Allow_UP, _T("UP") },
	{ CMsg::Allow_DOWN, _T("DOWN") },
	// 0x20
	{ CMsg::SPACE, _T("SPACE") },
	{ -1, _T("ROLL UP") },
	{ -1, _T("ROLL DOWN") },
	{ -1, _T("UNDO") },
	{ -1, _T("HELP") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("XF1") },
	{ -1, _T("XF2") },
	{ -1, _T("XF3") },
	{ -1, _T("XF4") },
	{ -1, _T("XF5") },
	{ -1, _T("OPT.1") },
	{ -1, _T("OPT.2") },
	// 0x30
	{ -1, _T("0") },
	{ -1, _T("1") },
	{ -1, _T("2") },
	{ -1, _T("3") },
	{ -1, _T("4") },
	{ -1, _T("5") },
	{ -1, _T("6") },
	{ -1, _T("7") },
	{ -1, _T("8") },
	{ -1, _T("9") },
	{ -1, _T(": *") },
	{ -1, _T("; +") },
	{ -1, _T(", <") },
	{ -1, _T("- =") },
	{ -1, _T(". >") },
	{ -1, _T("/ ?") },
	// 0x40
	{ -1,_T(" @ ") },
	{ -1, _T("A") },
	{ -1, _T("B") },
	{ -1, _T("C") },
	{ -1, _T("D") },
	{ -1, _T("E") },
	{ -1, _T("F") },
	{ -1, _T("G") },
	{ -1, _T("H") },
	{ -1, _T("I") },
	{ -1, _T("J") },
	{ -1, _T("K") },
	{ -1, _T("L") },
	{ -1, _T("M") },
	{ -1, _T("N") },
	{ -1, _T("O") },
	// 0x50
	{ -1, _T("P") },
	{ -1, _T("Q") },
	{ -1, _T("R") },
	{ -1, _T("S") },
	{ -1, _T("T") },
	{ -1, _T("U") },
	{ -1, _T("V") },
	{ -1, _T("W") },
	{ -1, _T("X") },
	{ -1, _T("Y") },
	{ -1, _T("Z") },
	{ -1,  _T(" [ ") },
	{ -1,  _T(" \\ ") },
	{ -1,  _T(" ] ") },
	{ -1,  _T(" ^ ") },
	{ -1,  _T(" _ ") },
	// 0x60 - 0x69  num 0 - num 9
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	{ -1, _T("") },
	// 0x6a
	{ CMsg::num_Multiply, _T("num *") },
	{ CMsg::num_Plus, _T("num +") },
	{ CMsg::num_Equal, _T("num =") },
	{ CMsg::num_Minus, _T("num -") },
	{ CMsg::num_Comma, _T("num ,") },
	{ CMsg::num_Point, _T("num .") },
	// 0x70
	{ CMsg::num_Devide, _T("num /") },
	// 0x71 - 0x7f  F1 - F15
	{ -1, NULL }
};

/// @brief get key label for virtual machine
///
/// @param[in]  code      : key code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmKeyLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;

	if (label) {
		label[0] = '\0';
	}

	if (0x30 <= code && code <= 0x39) {
		// 0 - 9
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c") ,code);
	} else if (0x41 <= code && code <= 0x5a) {
		// A - Z
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c"), code);
	} else if (0x60 <= code && code <= 0x69) {
		// num 0 - num 9
		fp = gMessages.Get(CMsg::num_VCHAR, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code - 0x30);
	} else if (0x71 <= code && code <= 0x7f) {
		// F1 - F15
		fp = gMessages.Get(CMsg::F_VDIGIT, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code - 0x70);
	} else if (0 <= code && code < 0x71) {
		CMsg::Id id = (CMsg::Id)sVmKeyLabels[code].id;
		if (id > 0 && translate) {
			p = gMessages.Get(id);
		} else {
			p = sVmKeyLabels[code].label;
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	} else {
		UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("0x%02x") ,code);
		known_key = false;
	}

	return known_key;
}

/// array of vm joystick label
const key_labels_t cVmJoyLabels[] = {
	{ -1, _T("") },
	{ CMsg::up,         _T("up") },
	{ CMsg::up_right,   _T("up+right") },
	{ CMsg::right,      _T("right") },
	{ CMsg::down_right, _T("down+right") },
	{ CMsg::down,       _T("down") },
	{ CMsg::down_left,  _T("down+left") },
	{ CMsg::left,       _T("left") },
	// 8
	{ CMsg::up_left,    _T("up+left") },
	{ CMsg::YA_up,         _T("YA up") },
	{ CMsg::YA_up_right,   _T("YA up+right") },
	{ CMsg::YA_right,      _T("YA right") },
	{ CMsg::YA_down_right, _T("YA down+right") },
	{ CMsg::YA_down,       _T("YA down") },
	{ CMsg::YA_down_left,  _T("YA down+left") },
	{ CMsg::YA_left,       _T("YA left") },
	// 16
	{ CMsg::YA_up_left,    _T("YA up+left") },
	{ CMsg::Null,       _T("?") },
	{ CMsg::button_A,	_T("button A") },
	{ CMsg::button_B,	_T("button B") },
	{ CMsg::button_C,	_T("button C") },
	{ CMsg::button_D_X,	_T("button D, X") },
	{ CMsg::button_E1_Y,_T("button E1, Y") },
	{ CMsg::button_E2_Z,_T("button E2, Z") },
	// 24
	{ CMsg::button_L,	_T("button L") },
	{ CMsg::button_R,	_T("button R") },
	{ CMsg::select,		_T("select") },
	{ CMsg::start,		_T("start") },
	{ CMsg::Null,       _T("?") },
	{ -1,                   _T("ESC") },
	{ CMsg::Bracket_Pause,  _T("(Pause)") },
	{ CMsg::Null,           _T("?") },
	// 32
	{ CMsg::Left_Analog_X,	_T("Left Analog X") },
	{ CMsg::Left_Analog_Y,	_T("Left Analog Y") },
	{ CMsg::Right_Analog_X,	_T("Right Analog X") },
	{ CMsg::Right_Analog_Y,	_T("Right Analog Y") },
	{ -1, NULL }
};

/// @brief get joystick label for virtual machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmJoyLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;

	*label = _T('\0');

	if (0 <= code && code < VM_JOY_LABELS_MAX) {
		CMsg::Id id = (CMsg::Id)cVmJoyLabels[code].id;
		if (id > CMsg::Null) {
			p = gMessages.Get(id, translate);
		} else {
			p = cVmJoyLabels[code].label;
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	} else if (code >= VM_JOY_LABELS_MAX && code < 30) {
		p = gMessages.Get(CMsg::trigger_VCHAR);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, p, code + 0x41 - VM_JOY_LABELS_MAX);
	} else {
		*label = _T('\0');
		known_key = false;
	}

	return known_key;
}

/// @brief get joystick label for virtual machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVmJoyBitLabel(int code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *fp = NULL;

	*label = _T('\0');

	if (code >= 0x00 && code <= 0x07) {
		fp = gMessages.Get(CMsg::bit_VDIGIT, translate);
		UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code);
	} else {
		*label = _T('\0');
		known_key = false;
	}
	return known_key;
}

/// @brief get key label for host machine
///
/// @param[in]  code      : key code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVkKeyLabel(uint32_t code, _TCHAR *label, bool translate) {
	bool known_key = true;
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;
	int id = -1;

	*label = _T('\0');

	switch(code) {
		case 0:
			p = _T("");
			break;
		case KEYCODE_BACKSPACE:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("delete");
#else
			p = _T("back space");
#endif
			break;
		case KEYCODE_TAB:
			p = _T("tab");
			break;
		case KEYCODE_CLEAR:
			p = _T("clear");
			break;
		case KEYCODE_RETURN:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("return");
#else
			p = _T("enter");
#endif
			break;
		case KEYCODE_PAUSE:
			p = _T("pause");
			break;

		case KEYCODE_ESCAPE:
			p = _T("esc");
			break;
		case KEYCODE_SPACE:
			p = _T("space");
			id = CMsg::space;
			break;

		case KEYCODE_COMMA:
			p = _T(", <");		// JP
//			id = CMsg::comma_smaller;
			break;
		case KEYCODE_MINUS:
			p = _T("- =");		// JP
//			id = CMsg::minus_equal;
			break;
		case KEYCODE_PERIOD:
			p = _T(". >");		// JP
//			id = CMsg::period_greater;
			break;
		case KEYCODE_SLASH:
			p = _T("/ ?");		// JP
//			id = CMsg::slash_question;
			break;

		case KEYCODE_COLON:
			p = _T(": *");		// JP
//			id = CMsg::colon_asterisk;
			break;
		case KEYCODE_SEMICOLON:
			p = _T("; +");		// JP
//			id = CMsg::semicolon_plus;
			break;
		case KEYCODE_AT:
			p = _T("@ `");		// JP
//			id = CMsg::at_quote;
			break;
		case KEYCODE_LBRACKET:
			p = _T("[ {");		// JP
//			id = CMsg::lbracket;
			break;
		case KEYCODE_BACKSLASH:
			p = _T("\\ |");	// JP
//			id = CMsg::backslash_bar;
			break;
		case KEYCODE_RBRACKET:
			p = _T("] }");		// JP
//			id = CMsg::rbracket;
			break;
		case KEYCODE_CARET:
			p = _T("^ ~");		// JP
//			id = CMsg::caret_tilde;
			break;
		case KEYCODE_UNDERSCORE:
			p = _T(" _ ");		// JP
//			id = CMsg::underscore;
			break;
		case KEYCODE_GRAVE:
			p = _T("kanji");	// JP
			id = CMsg::Kanji;
			break;

		case KEYCODE_DELETE:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("delete x");
#else
			p = _T("delete");
#endif
			break;

		case KEYCODE_HENKAN:	// JP
			p = _T("henkan");
			id = CMsg::henkan;
			break;
		case KEYCODE_MUHENKAN:	// JP
			p = _T("muhenkan");
			id = CMsg::muhenkan;
			break;
		case KEYCODE_KATAHIRA:	// JP
			p = _T("katakana");
			id = CMsg::katakana;
			break;
		case KEYCODE_EISU:		// JP
			p = _T("eisu");
			id = CMsg::eisu;
			break;
		case KEYCODE_KANA:		// JP
			p = _T("kana");
			id = CMsg::kana;
			break;

		case KEYCODE_KP_PERIOD:
			p = _T("num .");
			id = CMsg::num_Point;
			break;
		case KEYCODE_KP_DIVIDE:
			p = _T("num /");
			id = CMsg::num_Devide;
			break;
		case KEYCODE_KP_MULTIPLY:
			p = _T("num *");
			id = CMsg::num_Multiply;
			break;
		case KEYCODE_KP_MINUS:
			p = _T("num -");
			id = CMsg::num_Minus;
			break;
		case KEYCODE_KP_PLUS:
			p = _T("num +");
			id = CMsg::num_Plus;
			break;
		case KEYCODE_KP_ENTER:
			p = _T("num enter");
			id = CMsg::num_Enter;
			break;
		case KEYCODE_KP_EQUALS:
			p = _T("num =");
			id = CMsg::num_Equal;
			break;
		case KEYCODE_KP_COMMA:
			p = _T("num ,");
			id = CMsg::num_Comma;
			break;

		case KEYCODE_UP:
			p = _T("up");
			id = CMsg::up;
			break;
		case KEYCODE_DOWN:
			p = _T("down");
			id = CMsg::down;
			break;
		case KEYCODE_RIGHT:
			p = _T("right");
			id = CMsg::right;
			break;
		case KEYCODE_LEFT:
			p = _T("left");
			id = CMsg::left;
			break;
		case KEYCODE_INSERT:
			p = _T("insert");
			break;
		case KEYCODE_HOME:
			p = _T("home");
			break;
		case KEYCODE_END:
			p = _T("end");
			break;
		case KEYCODE_PAGEUP:
			p = _T("page up");
			break;
		case KEYCODE_PAGEDOWN:
			p = _T("page down");
			break;

		case KEYCODE_NUMLOCK:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("clear");
#else
			p = _T("numlock");
#endif
			break;
		case KEYCODE_CAPSLOCK:
			p = _T("caps lock");
			break;
		case KEYCODE_SCROLLLOCK:
			p = _T("scroll lock");
			break;
		case KEYCODE_RSHIFT:
			p = _T("right shift");
			id = CMsg::right_shift;
			break;
		case KEYCODE_LSHIFT:
			p = _T("left shift");
			id = CMsg::left_shift;
			break;
		case KEYCODE_RCTRL:
			p = _T("right ctrl");
			id = CMsg::right_ctrl;
			break;
		case KEYCODE_LCTRL:
			p = _T("left ctrl");
			id = CMsg::left_ctrl;
			break;
		case KEYCODE_RALT:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("right option");
			id = CMsg::right_option;
#else
			p = _T("right alt");
			id = CMsg::right_alt;
#endif
			break;
		case KEYCODE_LALT:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("left option");
			id = CMsg::left_option;
#else
			p = _T("left alt");
			id = CMsg::left_alt;
#endif
			break;
		case KEYCODE_RGUI:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("right command");
			id = CMsg::right_command;
#else
			p = _T("right meta");
			id = CMsg::right_meta;
#endif
			break;
		case KEYCODE_LGUI:
#if defined(__APPLE__) && defined(__MACH__)
			p = _T("left command");
			id = CMsg::left_command;
#else
			p = _T("left meta");
			id = CMsg::left_meta;
#endif
			break;
		case KEYCODE_MODE:
			p = _T("mode");
			break;
		case KEYCODE_HELP:
			p = _T("help");
			break;
		case KEYCODE_SYSREQ:
			p = _T("sys req");
			break;
		case KEYCODE_MENU:
			p = _T("menu");
			break;
		case KEYCODE_RSUPER:
			p = _T("right win");
			id = CMsg::right_win;
			break;
		case KEYCODE_LSUPER:
			p = _T("left win");
			id = CMsg::left_win;
			break;
		case KEYCODE_COMPOSE:
			p = _T("compose");
			break;
		case KEYCODE_PRINT:
			p = _T("print");
			break;
		case KEYCODE_0:
			p = _T("0");
			break;
		case KEYCODE_KP_0:
			p = _T("num 0");
			id = CMsg::num_0;
			break;
		case KEYCODE_FUNCTION:
			p = _T("fn");
			break;
		default:
			if (KEYCODE_1 <= code && code <= KEYCODE_9) {
				// 1 - 9
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%d"), code + 1 - KEYCODE_1);
			} else if (KEYCODE_A <= code && code <= KEYCODE_Z) {
				// A - Z
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("%c"), code + 0x41 - KEYCODE_A);
			} else if (KEYCODE_KP_1 <= code && code <= KEYCODE_KP_9) {
				// num 1 - num 9
				fp = gMessages.Get(CMsg::num_VDIGIT, translate);
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 1 - KEYCODE_KP_1);
			} else if (KEYCODE_F1 <= code && code <= KEYCODE_F12) {
				// F1 - F12
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 1 - KEYCODE_F1);
			} else if (KEYCODE_F13 <= code && code <= KEYCODE_F15) {
				// F13 - F15
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 13 - KEYCODE_F13);
			} else if (KEYCODE_F16 <= code && code <= KEYCODE_F19) {
				// F16 - F19
				fp = _T("F%d");
				UTILITY::stprintf(label, KBLABEL_MAXLEN, fp, code + 16 - KEYCODE_F16);
			} else {
				UTILITY::stprintf(label, KBLABEL_MAXLEN, _T("0x%02x"), code);
				known_key = false;
			}
			break;
	}
	if (p != NULL) {
		if (id > 0) {
			p = gMessages.Get((CMsg::Id)id, translate);
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	}

	return known_key;
}

/// @brief get joystick label for host machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVkJoyLabel(uint32_t code, _TCHAR *label, bool translate) {
	bool known_key = true;
	_TCHAR numstr[8];
	const _TCHAR *p = NULL;
	const _TCHAR *fp = NULL;
	int id = -1;

	*label = _T('\0');

	switch(code) {
		case 0x00:
			p = _T("");
			break;
		case JOYCODE_UP:
			p = _T("Y up");
			id = CMsg::Y_up;
			break;
		case JOYCODE_DOWN:
			p = _T("Y down");
			id = CMsg::Y_down;
			break;
		case JOYCODE_LEFT:
			p = _T("X left");
			id = CMsg::X_left;
			break;
		case JOYCODE_UPLEFT:
			p = _T("YX up+left");
			id = CMsg::YX_up_left;
			break;
		case JOYCODE_DOWNLEFT:
			p = _T("YX down+left");
			id = CMsg::YX_down_left;
			break;
		case JOYCODE_RIGHT:
			p = _T("X right");
			id = CMsg::X_right;
			break;
		case JOYCODE_UPRIGHT:
			p = _T("YX up+right");
			id = CMsg::YX_up_right;
			break;
		case JOYCODE_DOWNRIGHT:
			p = _T("YX down+right");
			id = CMsg::YX_down_right;
			break;
		case JOYCODE_R_UP:
			p = _T("R up");
			id = CMsg::R_up;
			break;
		case JOYCODE_R_DOWN:
			p = _T("R down");
			id = CMsg::R_down;
			break;
		case JOYCODE_Z_LEFT:
			p = _T("Z left");
			id = CMsg::Z_left;
			break;
		case JOYCODE_RZ_UPLEFT:
			p = _T("RZ up+left");
			id = CMsg::RZ_up_left;
			break;
		case JOYCODE_RZ_DOWNLEFT:
			p = _T("RZ down+left");
			id = CMsg::RZ_down_left;
			break;
		case JOYCODE_Z_RIGHT:
			p = _T("Z right");
			id = CMsg::Z_right;
			break;
		case JOYCODE_RZ_UPRIGHT:
			p = _T("RZ up+right");
			id = CMsg::RZ_up_right;
			break;
		case JOYCODE_RZ_DOWNRIGHT:
			p = _T("RZ down+right");
			id = CMsg::RZ_down_right;
			break;
		case JOYCODE_V_UP:
			p = _T("V up");
			id = CMsg::V_up;
			break;
		case JOYCODE_V_DOWN:
			p = _T("V down");
			id = CMsg::V_down;
			break;
		case JOYCODE_U_LEFT:
			p = _T("U left");
			id = CMsg::U_left;
			break;
		case JOYCODE_VU_UPLEFT:
			p = _T("VU up+left");
			id = CMsg::VU_up_left;
			break;
		case JOYCODE_VU_DOWNLEFT:
			p = _T("VU down+left");
			id = CMsg::VU_down_left;
			break;
		case JOYCODE_U_RIGHT:
			p = _T("U right");
			id = CMsg::U_right;
			break;
		case JOYCODE_VU_UPRIGHT:
			p = _T("VU up+right");
			id = CMsg::VU_up_right;
			break;
		case JOYCODE_VU_DOWNRIGHT:
			p = _T("VU down+right");
			id = CMsg::VU_down_right;
			break;
		case JOYCODE_POV_UP:
			p = _T("POV up");
			id = CMsg::POV_up;
			break;
		case JOYCODE_POV_DOWN:
			p = _T("POV down");
			id = CMsg::POV_down;
			break;
		case JOYCODE_POV_LEFT:
			p = _T("POV left");
			id = CMsg::POV_left;
			break;
		case JOYCODE_POV_UPLEFT:
			p = _T("POV up+left");
			id = CMsg::POV_up_left;
			break;
		case JOYCODE_POV_DOWNLEFT:
			p = _T("POV down+left");
			id = CMsg::POV_down_left;
			break;
		case JOYCODE_POV_RIGHT:
			p = _T("POV right");
			id = CMsg::POV_right;
			break;
		case JOYCODE_POV_UPRIGHT:
			p = _T("POV up+right");
			id = CMsg::POV_up_right;
			break;
		case JOYCODE_POV_DOWNRIGHT:
			p = _T("POV down+right");
			id = CMsg::POV_down_right;
			break;
		default:
			// button
			if (code & 0xffff0000) {
				fp = gMessages.Get(CMsg::button, translate);
				UTILITY::tcscpy(label, KBLABEL_MAXLEN, fp);
				for(int i=0; i<16; i++) {
					if (code & (0x10000 << i)) {
						UTILITY::stprintf(numstr, 8, _T(" %d"), i+1);
						UTILITY::tcscat(label, KBLABEL_MAXLEN, numstr);
					}
				}
			} else {
				*label = _T('\0');
				known_key = false;
			}
			break;
	}
	if (p != NULL) {
		if (id > 0) {
			p = gMessages.Get((CMsg::Id)id, translate);
		}
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	}

	return known_key;
}

/// @brief get joystick (analog) label for host machine
///
/// @param[in]  code      : joystick code
/// @param[out] label
/// @param[in]  translate : translate by locale
/// @return true if known key code
bool KeybindData::GetVkJoyAnLabel(uint32_t code, _TCHAR *label, bool translate) {
	bool known_key = true;
//	_TCHAR numstr[8];
	const _TCHAR *p = NULL;
	int id = -1;

	*label = _T('\0');

	switch(code) {
		case JOYCODE_ANA_X:
			id = CMsg::Analog_X;
			break;
		case JOYCODE_ANA_X_REV:
			id = CMsg::Analog_X_Rev;
			break;
		case JOYCODE_ANA_Y:
			id = CMsg::Analog_Y;
			break;
		case JOYCODE_ANA_Y_REV:
			id = CMsg::Analog_Y_Rev;
			break;
		case JOYCODE_ANA_Z:
			id = CMsg::Analog_Z;
			break;
		case JOYCODE_ANA_Z_REV:
			id = CMsg::Analog_Z_Rev;
			break;
		case JOYCODE_ANA_R:
			id = CMsg::Analog_R;
			break;
		case JOYCODE_ANA_R_REV:
			id = CMsg::Analog_R_Rev;
			break;
		case JOYCODE_ANA_U:
			id = CMsg::Analog_U;
			break;
		case JOYCODE_ANA_U_REV:
			id = CMsg::Analog_U_Rev;
			break;
		case JOYCODE_ANA_V:
			id = CMsg::Analog_V;
			break;
		case JOYCODE_ANA_V_REV:
			id = CMsg::Analog_V_Rev;
			break;
		default:
			p = _T("");
			known_key = false;
			break;
	}
	if (id > 0) {
		p = gMessages.Get((CMsg::Id)id, translate);
	}
	if (p != NULL) {
		UTILITY::tcscpy(label, KBLABEL_MAXLEN, p);
	}

	return known_key;
}
