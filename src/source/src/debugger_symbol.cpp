/** @file debugger_symbol.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger symbol name ]
*/

#include "debugger_symbol.h"

#ifdef USE_DEBUGGER

#include "utility.h"

//

Symbol::Symbol()
{
	Clear();
}

Symbol::Symbol(uint32_t addr_, const _TCHAR *name_)
{
	Set(addr_, name_);
}

Symbol::~Symbol()
{
}

void Symbol::Clear()
{
	addr = 0;
	memset(name, 0, sizeof(name));
}

void Symbol::Set(uint32_t addr_, const _TCHAR *name_)
{
	addr = addr_;
	UTILITY::tcscpy(name, sizeof(name) / sizeof(name[0]), name_);
}

//

SymbolTable::SymbolTable()
	: CPtrList<Symbol>()
{
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::Add(Symbol *src)
{
	CPtrList<Symbol>::Add(src);
}

const Symbol *SymbolTable::Add(uint32_t addr, const _TCHAR *name)
{
	Symbol *new_item = NULL;
	// sort by addr
	int i=0;
	for(; i<Count(); i++) {
		Symbol *item = Item(i);
		if (item->Addr() == addr) {
			// already exists
			// override
			item->Set(addr, name);
			return item;
		} else if (item->Addr() > addr) {
			new_item = new Symbol(addr, name);
			Insert(i, new_item);
			return new_item;
		}
	}
	if (i == Count()) {
		new_item = new Symbol(addr, name);
		Add(new_item);
	}
	return new_item;
}

const Symbol *SymbolTable::FindCyc(uint32_t addr, int start, int end, int depth) const
{
	int idx = ((end - start) >> 1) + start;

	const Symbol *itm = Item(idx);
	if (itm->Addr() == addr) return itm;

	if (start == end) return NULL;

	if (itm->Addr() > addr) {
		return (start <= idx - 1 ? FindCyc(addr, start, idx - 1, depth + 1) : NULL); 
	} else {
		return (idx + 1 <= end ? FindCyc(addr, idx + 1, end, depth + 1) : NULL); 
	}
}

const Symbol *SymbolTable::Find(uint32_t addr) const
{
	if (Count() <= 0) return NULL;

	return FindCyc(addr, 0, Count() - 1, 0);
}

const Symbol *SymbolTable::DeleteByAddr(uint32_t addr)
{
	for(int i=0; i<Count(); i++) {
		const Symbol *itm = Item(i);
		if (itm->Addr() == addr) {
			tmp_symbol = *itm;
			Delete(i);
			return &tmp_symbol;
		}
	}
	return NULL;
}

//

DEBUGGER_SYMBOLS::DEBUGGER_SYMBOLS()
{
	for(int i=0; i<8; i++) {
		tmp_name[i][0] = _T('\0');
	}
	tmp_index = 0;
}

DEBUGGER_SYMBOLS::~DEBUGGER_SYMBOLS()
{
	release_symbols();
}

const Symbol *DEBUGGER_SYMBOLS::add_symbol(uint32_t addr, const _TCHAR *name)
{
	return table.Add(addr, name);
}

void DEBUGGER_SYMBOLS::release_symbols()
{
	table.Clear();
}

const Symbol *DEBUGGER_SYMBOLS::find(uint32_t addr) const
{
	return table.Find(addr);
}

const Symbol *DEBUGGER_SYMBOLS::release_symbol(uint32_t addr)
{
	return table.DeleteByAddr(addr);
}

const Symbol *DEBUGGER_SYMBOLS::get_symbol(int index) const
{
	if (index < 0 || index >= table.Count()) return NULL;

	return table.Item(index);
}

const _TCHAR *DEBUGGER_SYMBOLS::get_symbol_name(int index) const
{
	if (index < 0 || index >= table.Count()) return NULL;

	return table.Item(index)->Name();
}

const _TCHAR *DEBUGGER_SYMBOLS::find_symbol_name(uint32_t addr) const
{
	const Symbol *symbol = find(addr);
	if (symbol) {
		return symbol->Name();
	}
	return NULL;
}

const _TCHAR *DEBUGGER_SYMBOLS::get_label_or_symbol(const _TCHAR *label, uint32_t addr)
{
	const _TCHAR *name = find_symbol_name(addr);
	if (name) {
		return name;
	} else {
		return label;
	}
}

const _TCHAR *DEBUGGER_SYMBOLS::get_value_or_symbol(const _TCHAR *format, uint32_t addr)
{
	const _TCHAR *name = find_symbol_name(addr);
	if (name) {
		return name;
	} else {
		unsigned int index = tmp_index;
		tmp_index = ((tmp_index + 1) & 7);
		UTILITY::stprintf(tmp_name[index], DEBUGGER_SYMBOLS_SIZE, format, addr);
		return tmp_name[index];
	}
}

const _TCHAR *DEBUGGER_SYMBOLS::get_value_and_symbol(const _TCHAR *format, uint32_t addr)
{
	unsigned int index = tmp_index;
	tmp_index = ((tmp_index + 1) & 7);

	UTILITY::stprintf(tmp_name[index], DEBUGGER_SYMBOLS_SIZE, format, addr);

	const _TCHAR *name = find_symbol_name(addr);
	if (name) {
		_TCHAR temp[DEBUGGER_SYMBOLS_SIZE];
		UTILITY::concat(temp, DEBUGGER_SYMBOLS_SIZE, _T(" ;"), name, NULL);
		UTILITY::tcscat(tmp_name[index], DEBUGGER_SYMBOLS_SIZE, temp);
	}
	return tmp_name[index];
}

/// @note this function is such as strcat
void DEBUGGER_SYMBOLS::cat_label_or_symbol(_TCHAR *buffer, size_t buffer_len, const _TCHAR *prefix, const _TCHAR *label, uint32_t addr)
{
	if (prefix) UTILITY::tcscat(buffer, buffer_len, prefix);

	const _TCHAR *name = find_symbol_name(addr);
	UTILITY::tcscat(buffer, buffer_len, name ? name : label);
}

/// @note this function is such as strcat
void DEBUGGER_SYMBOLS::cat_value_or_symbol(_TCHAR *buffer, size_t buffer_len, const _TCHAR *prefix, const _TCHAR *format, uint32_t addr)
{
	if (prefix) UTILITY::tcscat(buffer, buffer_len, prefix);

	const _TCHAR *name = find_symbol_name(addr);
	if (name) {
		UTILITY::tcscat(buffer, buffer_len, name);
	} else {
		UTILITY::sntprintf(buffer, buffer_len, format, addr);
	}
}

#endif /* USE_DEBUGGER */
