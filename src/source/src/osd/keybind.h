/** @file keybind.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.02.11

	@brief [ keybind pool ]
*/

#ifndef KEYBIND_H
#define KEYBIND_H

#include "../common.h"
#include "../vm/vm_defs.h"

enum enCodeTableFlags {
	CODE_TABLE_FLAG_ENABLE = 0x0001,
	CODE_TABLE_FLAG_JOYBTN = 0x0100,
	CODE_TABLE_FLAG_JOYKEY = 0x0800,
	CODE_TABLE_FLAG_JOYANA = 0x1000,
};

/// @brief template of array of <TYPE *> 
template <class TYPE>
class KeybindMap
{
protected:
	int   m_size;
	TYPE *p_map;

public:
	KeybindMap();
	virtual ~KeybindMap();
	void Set(const TYPE *map, int size);
	void SetMap(const TYPE *map);
	void SetMap(TYPE *map);
	void SetSize(int size);
	const TYPE *GetMap() const;
	TYPE *GetMap();
	int GetSize() const;
};

/// @brief constructor
template <class TYPE>
KeybindMap<TYPE>::KeybindMap()
{
	m_size = 0;
	p_map = NULL;
}

/// @brief destructor
template <class TYPE>
KeybindMap<TYPE>::~KeybindMap()
{
}

/// @brief set reference to an array
/// @param[in] map : array
/// @param[in] size : number of items in array
template <class TYPE>
void KeybindMap<TYPE>::Set(const TYPE *map, int size)
{
	m_size = size;
	p_map = (TYPE *)map;
}

/// @brief set reference to an array
/// @param[in] map : array
template <class TYPE>
void KeybindMap<TYPE>::SetMap(const TYPE *map)
{
	p_map = (TYPE *)map;
}

/// @brief set reference to an array
/// @param[in] map : array
template <class TYPE>
void KeybindMap<TYPE>::SetMap(TYPE *map)
{
	p_map = map;
}

/// @brief set size of an array
/// @param[in] size : number of items in array
template <class TYPE>
void KeybindMap<TYPE>::SetSize(int size)
{
	m_size = size;
}

/// @brief get reference to the array
/// @return the array
template <class TYPE>
const TYPE *KeybindMap<TYPE>::GetMap() const
{
	return (const TYPE *)p_map;
}

/// @brief get reference to the array
/// @return the array
template <class TYPE>
TYPE *KeybindMap<TYPE>::GetMap()
{
	return p_map;
}

/// @brief get number of items in the array
/// @return number of items in the array
template <class TYPE>
int KeybindMap<TYPE>::GetSize() const
{
	return m_size;
}

//

/// @brief for store key assigning data 
typedef struct st_uint32_assign {
	uint32_t d[KEYBIND_ASSIGN];
} uint32_key_assign_t;

//

/// @brief array of uint16_t
class KeybindMap16 : public KeybindMap<uint16_t>
{
};

/// @brief array of uint32_key_assign_t
class KeybindMap32A : public KeybindMap<uint32_key_assign_t>
{
};

/// @brief VM key map table
class KeybindVmKeyMap : public KeybindMap16
{
};

/// @brief VK key map table
class KeybindVkKeyMap
{
protected:
	KeybindMap32A m_defmap;	///< default mapping
	KeybindMap32A m_curmap; ///< current mapping
	KeybindMap32A m_preset[KEYBIND_PRESETS];	///< preset

public:
	KeybindVkKeyMap();
	virtual ~KeybindVkKeyMap();
	void SetSize(int size);
	void SetDefMap(const uint32_key_assign_t *map);
	void SetMap(uint32_key_assign_t *map);
	void SetPresetMap(int num, uint32_key_assign_t *map);
	int GetSize() const;
	const uint32_key_assign_t *GetDefMap() const;
	uint32_key_assign_t *GetMap();
	uint32_key_assign_t *GetPresetMap(int num);
};

/// @brief keybind one set
class KeybindKeySet
{
protected:
	KeybindVmKeyMap m_vm;
	KeybindVkKeyMap m_vk;

public:
	KeybindKeySet();
	virtual ~KeybindKeySet();
	void SetVmKeyMap(const uint16_t *map, int size);
	const uint16_t *GetVmKeyMap() const;
	int GetVmKeyMapSize() const;
//	int GetVkKeyAssignSize() const;
	void SetVkKeySize(int size);
	void SetVkKeyDefMap(const uint32_key_assign_t *map);
	void SetVkKeyMap(uint32_key_assign_t *map);
	void SetVkKeyPresetMap(int num, uint32_key_assign_t *map);
	int GetVkKeyMapSize() const;
	const uint32_key_assign_t *GetVkKeyDefMap() const;
	uint32_key_assign_t *GetVkKeyMap();
	uint32_key_assign_t *GetVkKeyPresetMap(int num);
};

/// @brief keybind mapping pool
/// @note 0: key2key 1: joy2key 2: joy2joy 3: key2joy
class Keybind
{
protected:
	KeybindKeySet m_set[4];

public:
	Keybind();
	virtual ~Keybind();
	void SetVmKeyMap(int idx, const uint16_t *map, int size);
	const uint16_t *GetVmKeyMap(int idx) const;
	int GetVmKeyMapSize(int idx) const;
//	int GetVkKeyAssignSize() const;
	void SetVkKeySize(int idx, int size);
	void SetVkKeyDefMap(int idx, const uint32_key_assign_t *map);
	void SetVkKeyMap(int idx, uint32_key_assign_t *map);
	void SetVkKeyPresetMap(int idx, int num, uint32_key_assign_t *map);
	int GetVkKeyMapSize(int idx) const;
	const uint32_key_assign_t *GetVkKeyDefMap(int idx) const;
	uint32_key_assign_t *GetVkKeyMap(int idx);
	int GetVkKeyPresetSize() const { return KEYBIND_PRESETS; }
	uint32_key_assign_t *GetVkKeyPresetMap(int idx, int num);
};

/// @brief instance
extern Keybind gKeybind;

#endif /* KEYBIND_H */
