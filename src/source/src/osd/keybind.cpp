/** @file keybind.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.11

	@brief [ keybind pool ]
*/


#include "keybind.h"

//

/// constructor
KeybindVkKeyMap::KeybindVkKeyMap()
{
}

/// destructor
KeybindVkKeyMap::~KeybindVkKeyMap()
{
}

/// set size of each map
/// @param[in] size : number of items
void KeybindVkKeyMap::SetSize(int size)
{
	m_defmap.SetSize(size);
	m_curmap.SetSize(size);
	for(int preset=0; preset<KEYBIND_PRESETS; preset++) {
		m_preset[preset].SetSize(size);
	}
}

/// set reference of default map
/// @param[in] map : default mapping
void KeybindVkKeyMap::SetDefMap(const uint32_key_assign_t *map)
{
	m_defmap.SetMap(map);
}

/// set reference of current map
/// @param[in] map : current mapping
void KeybindVkKeyMap::SetMap(uint32_key_assign_t *map)
{
	m_curmap.SetMap(map);
}

/// set reference of preset map
/// @param[in] num : preset number
/// @param[in] map : preset mapping
void KeybindVkKeyMap::SetPresetMap(int num, uint32_key_assign_t *map)
{
	m_preset[num].SetMap(map);
}

/// @brief get number of items in the map
/// @return number of items
int KeybindVkKeyMap::GetSize() const
{
	return m_defmap.GetSize();
}

/// get reference of default map
/// @return default mapping
const uint32_key_assign_t *KeybindVkKeyMap::GetDefMap() const
{
	return m_defmap.GetMap();
}

/// get reference of current map
/// @return current mapping
uint32_key_assign_t *KeybindVkKeyMap::GetMap()
{
	return m_curmap.GetMap();
}

/// get reference of preset map
/// @param[in] num : preset number
/// @return preset mapping
uint32_key_assign_t *KeybindVkKeyMap::GetPresetMap(int num)
{
	return m_preset[num].GetMap();
}

//

/// constructor
KeybindKeySet::KeybindKeySet()
{
}

/// destructor
KeybindKeySet::~KeybindKeySet()
{
}

/// set reference of vm key map
/// @param[in] map : mapping data
/// @param[in] size : number of items
void KeybindKeySet::SetVmKeyMap(const uint16_t *map, int size)
{
	m_vm.Set(map, size);
}

/// get reference of vm key map
/// @return mapping data
const uint16_t *KeybindKeySet::GetVmKeyMap() const
{
	return m_vm.GetMap();
}

/// get number of items in vm key map
/// @return number of item
int KeybindKeySet::GetVmKeyMapSize() const
{
	return m_vm.GetSize();
}

#if 0
int KeybindKeySet::GetVkKeyAssignSize() const
{
	return m_vk.GetAssignSize();
}
#endif

/// set number of items in vk(host) key map
/// @param[in] size : number of items
void KeybindKeySet::SetVkKeySize(int size)
{
	m_vk.SetSize(size);
}

/// set reference of vk(host) key default map
/// @param[in] map : mapping data
void KeybindKeySet::SetVkKeyDefMap(const uint32_key_assign_t *map)
{
	m_vk.SetDefMap(map);
}

/// set reference of vk(host) key map
/// @param[in] map : mapping data
void KeybindKeySet::SetVkKeyMap(uint32_key_assign_t *map)
{
	m_vk.SetMap(map);
}

/// set reference of vk(host) preset map
/// @param[in] num : preset number
/// @param[in] map : preset mapping
void KeybindKeySet::SetVkKeyPresetMap(int num, uint32_key_assign_t *map)
{
	m_vk.SetPresetMap(num, map);
}

/// get number of items in vk(host) key map
/// @return number of items
int KeybindKeySet::GetVkKeyMapSize() const
{
	return m_vk.GetSize();
}

/// get reference of vk(host) key default map
/// @return mapping data
const uint32_key_assign_t *KeybindKeySet::GetVkKeyDefMap() const
{
	return m_vk.GetDefMap();
}

/// get reference of vk(host) key map
/// @return mapping data
uint32_key_assign_t *KeybindKeySet::GetVkKeyMap()
{
	return m_vk.GetMap();
}

/// get reference of vk(host) preset map
/// @param[in] num : preset number
/// @return mapping data
uint32_key_assign_t *KeybindKeySet::GetVkKeyPresetMap(int num)
{
	return m_vk.GetPresetMap(num);
}

//

/// constructor
Keybind::Keybind()
{
}

/// destructor
Keybind::~Keybind()
{
}

/// set reference of vm scan code map
/// @param[in] idx : tab index number
/// @param[in] map : mapping data
/// @param[in] size : number of items
void Keybind::SetVmKeyMap(int idx, const uint16_t *map, int size)
{
	m_set[idx].SetVmKeyMap(map, size);
}

/// get reference of vm scan code map
/// @param[in] idx : tab index number
/// @return mapping data
const uint16_t *Keybind::GetVmKeyMap(int idx) const
{
	return m_set[idx].GetVmKeyMap();
}

/// get number of items in vm scan code map
/// @param[in] idx : tab index number
/// @return number of items
int Keybind::GetVmKeyMapSize(int idx) const
{
	return m_set[idx].GetVmKeyMapSize();
}

#if 0
int Keybind::GetVkKeyAssignSize() const
{
	return m_set[0].GetVkKeyAssignSize();
}
#endif

/// set number of items in vk(host) key map
/// @param[in] idx : tab index number
/// @param[in] size : number of items
void Keybind::SetVkKeySize(int idx, int size)
{
	m_set[idx].SetVkKeySize(size);
}

/// set reference of vk(host) key default map
/// @param[in] idx : tab index number
/// @param[in] map : mapping data
void Keybind::SetVkKeyDefMap(int idx, const uint32_key_assign_t *map)
{
	m_set[idx].SetVkKeyDefMap(map);
}

/// set reference of vk(host) key map
/// @param[in] idx : tab index number
/// @param[in] map : mapping data
void Keybind::SetVkKeyMap(int idx, uint32_key_assign_t *map)
{
	m_set[idx].SetVkKeyMap(map);
}

/// set reference of vk(host) preset map
/// @param[in] idx : tab index number
/// @param[in] num : preset number
/// @param[in] map : preset mapping
void Keybind::SetVkKeyPresetMap(int idx, int num, uint32_key_assign_t *map)
{
	m_set[idx].SetVkKeyPresetMap(num, map);
}

/// get number of items in vk(host) key map
/// @param[in] idx : tab index number
/// @return number of items
int Keybind::GetVkKeyMapSize(int idx) const
{
	return m_set[idx].GetVkKeyMapSize();
}

/// get reference of vk(host) key default map
/// @param[in] idx : tab index number
/// @return mapping data
const uint32_key_assign_t *Keybind::GetVkKeyDefMap(int idx) const
{
	return m_set[idx].GetVkKeyDefMap();
}

/// get reference of vk(host) key map
/// @param[in] idx : tab index number
/// @return mapping data
uint32_key_assign_t *Keybind::GetVkKeyMap(int idx)
{
	return m_set[idx].GetVkKeyMap();
}

/// get reference of vk(host) preset map
/// @param[in] idx : tab index number
/// @param[in] num : preset number
/// @return mapping data
uint32_key_assign_t *Keybind::GetVkKeyPresetMap(int idx, int num)
{
	return m_set[idx].GetVkKeyPresetMap(num);
}

// instance

Keybind gKeybind;