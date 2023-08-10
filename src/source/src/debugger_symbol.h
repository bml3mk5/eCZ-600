/** @file debugger_symbol.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger symbol name ]
*/

#ifndef DEBUGGER_SYMBOL_H
#define DEBUGGER_SYMBOL_H

#include "common.h"
#include "vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "cchar.h"
#include "cptrlist.h"

#define DEBUGGER_SYMBOLS_SIZE		16
#define DEBUGGER_SYMBOLS_TMP_SIZE	32

/**
	@brief Symbol name for disassembler
*/
class Symbol
{
private:
	uint32_t addr;
	_TCHAR   name[DEBUGGER_SYMBOLS_SIZE];

public:
	Symbol();
	Symbol(uint32_t addr_, const _TCHAR *name_);
	~Symbol();
	void Clear();
	void Set(uint32_t addr_, const _TCHAR *name_);

	uint32_t Addr() const { return addr; }
	const _TCHAR *Name() const { return name; }
};

/**
	@brief Symbol table for disassembler
*/
class SymbolTable : public CPtrList<Symbol>
{
private:
	Symbol tmp_symbol;

	const Symbol *FindCyc(uint32_t addr, int start, int end, int depth) const;

public:
	SymbolTable();
	~SymbolTable();

	void Add(Symbol *src);
	const Symbol *Add(uint32_t addr, const _TCHAR *name);

	const Symbol *Find(uint32_t addr) const;
	const Symbol *DeleteByAddr(uint32_t addr);
};

/**
	@brief Manage symbol table for disassembler
*/
class DEBUGGER_SYMBOLS
{
private:
	SymbolTable table;

	_TCHAR tmp_name[8][DEBUGGER_SYMBOLS_TMP_SIZE];
	unsigned int tmp_index;

	const Symbol *find(uint32_t addr) const;

public:
	DEBUGGER_SYMBOLS();
	virtual ~DEBUGGER_SYMBOLS();

	const Symbol *add_symbol(uint32_t addr, const _TCHAR *name);
	void release_symbols();
	const Symbol *release_symbol(uint32_t addr);
	const Symbol *get_symbol(int index) const;

	const _TCHAR *get_symbol_name(int index) const;
	const _TCHAR *find_symbol_name(uint32_t addr) const;
	const _TCHAR *get_label_or_symbol(const _TCHAR *label, uint32_t addr);
	const _TCHAR *get_value_or_symbol(const _TCHAR *format, uint32_t addr);
	const _TCHAR *get_value_and_symbol(const _TCHAR *format, uint32_t addr);

	void cat_label_or_symbol(_TCHAR *buffer, size_t buffer_len, const _TCHAR *prefix, const _TCHAR *label, uint32_t addr);
	void cat_value_or_symbol(_TCHAR *buffer, size_t buffer_len, const _TCHAR *prefix, const _TCHAR *format, uint32_t addr);

//	const symbol_t *get_first_symbol() const { return first_symbol; }
};

#endif /* USE_DEBUGGER */

#endif /* DEBUGGER_SYMBOL_H */
