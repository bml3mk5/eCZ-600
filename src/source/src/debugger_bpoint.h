/** @file debugger_bpoint.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger break point ]
*/

#ifndef DEBUGGER_BPOINT_H
#define DEBUGGER_BPOINT_H

#include "common.h"
#include "vm/vm_defs.h"

#ifdef USE_DEBUGGER

#include "vm/debugger_base.h"
#include "cchar.h"
#include "cptrlist.h"

#define MAX_BREAK_POINTS	8

/**
	@brief Break Point for Debugger
*/
class BreakPoint
{
private:
	uint32_t addr;
	uint32_t mask;
	int len;	///< range 0 - addr + len
	int status;	///< 0 = none, 1 = enabled, -1 = disabled
	_TCHAR regname[8];
	const void *regptr;
	int   reglen;
	uint32_t regval;
	uint32_t regmask;

public:
	BreakPoint();
	BreakPoint(uint32_t addr_, uint32_t mask_, int len_, int status_);
	BreakPoint(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_);
	~BreakPoint();
	void Clear();
	void Set(uint32_t addr_, uint32_t mask_, int len_, int status_);
	void Set(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_);

	bool MatchRegVal() const;
	bool MatchValue(uint32_t val) const;
	void Expression(_TCHAR *buffer, size_t buffer_len) const;
	void RegValue(_TCHAR *buffer, size_t buffer_len) const;

	void Addr(uint32_t addr_) { addr = addr_; }
	uint32_t Addr(void) const { return addr; }
	void Mask(uint32_t mask_) { mask = mask_; }
	uint32_t Mask(void) const { return mask; }
	void Len(int len_) { len = len_; }
	int Len(void) const { return len; }
	void Status(int status_) { status = status_; }
	int Status(void) const { return status; }
};

/**
	@brief Break Point List
*/
class BreakPointTable : public CPtrList<BreakPoint>
{
private:
	BreakPoint *IsExist(uint32_t addr_, uint32_t mask_, int len_, int *add_index_);
	int ModifyExistItem(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &bp_, int *add_index_);
	int ModifyDeletedItem(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &bp_, int *add_index_);

public:
	BreakPointTable();
	~BreakPointTable();

	void Add(BreakPoint *value);
	int  Add(uint32_t addr_, uint32_t mask_, int len_, int status_, BreakPoint * &new_bp_, int *add_index_);
	int  Add(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &new_bp_, int *add_index_);

};

/**
	@brief Break Point Management Class for Debugger
*/
class BreakPoints
{
public:
	enum en_break_point_type {
		BP_FETCH_OP = 0,
		BP_FETCH_OP_PH,
		BP_READ_MEMORY,
		BP_READ_MEMORY_PH,
		BP_WRITE_MEMORY,
		BP_WRITE_MEMORY_PH,
		BP_INPUT_IO,
		BP_OUTPUT_IO,
		BP_INTERRUPT,
		BP_EXCEPTION,
		BP_BASIC_NUMBER,
		BP_END
	};
	enum en_err_num {
		ERR_ALREADY_EXISTS = -1,
		ERR_NO_ITEM_EXISTS = -2,
		ERR_TOO_MANY_ITEMS = -3,
		ERR_OUT_OF_RANGE = -4,
	};
	enum en_intr_masks {
		INTR_OFF = 0,
		INTR_ON = 1,
		INTR_CHG = 2,
	};

private:
	BreakPointTable table;
	BreakPoint *tbl_ena[MAX_BREAK_POINTS];		///< enable points
	BreakPoint *tbl_store[MAX_BREAK_POINTS];
	int tbl_ena_count;
	int tbl_store_count;

	en_break_point_type type;
	const BreakPoint *hit_bp;
//	uint32_t hit_addr;
	uint32_t cur_addr;

	int  Enable(BreakPoint *item);
	int  Disable(BreakPoint *item);
	bool IsExist(BreakPoint *item) const;
	void Shift(int ena_index);
	void Unshift(int ena_index);

	const BreakPoint *FindFetchCyc(uint32_t addr, int len, int start, int end, int depth);
	const BreakPoint *FindMemoryCyc(uint32_t addr, int len, uint32_t value, int start, int end, int depth);
	const BreakPoint *FindIOCyc(uint32_t addr, int len, uint32_t value, int start, int end, int depth);

public:
	BreakPoints();
	~BreakPoints();

	int  Add(uint32_t addr, uint32_t mask, int len, BreakPoint * &new_bp, int *add_index = NULL);
	int  Add(uint32_t addr, uint32_t mask, int len, const _TCHAR *regname, const void *regptr, int reglen, uint32_t regval, uint32_t regmask, BreakPoint * &new_bp, int *add_index = NULL);
	bool Delete(int table_index);
	int  Enable(int table_index);
	int  Disable(int table_index);

	void DeleteAll();
	bool EnableAll();
	bool DisableAll();

	void Store(uint32_t addr, uint32_t mask, int len, int *add_index = NULL);
	void Store();
	void Restore();

	const BreakPoint *TableItem(int table_index) const;
	const BreakPoint *EnableItem(int ena_index) const;

	const BreakPoint *FindFetch(uint32_t addr, int len);
	const BreakPoint *FindMemory(uint32_t addr, int len, uint32_t value);
	const BreakPoint *FindIO(uint32_t addr, int len, uint32_t value);
	const BreakPoint *FindInterrupt(uint32_t addr, uint32_t mask);
	const BreakPoint *FindException(uint32_t addr, uint32_t vector);
	const BreakPoint *FindBASIC(DEVICE *d_mem, uint32_t addr);

//	bool Hit() const { return hit; }
//	void Hit(bool val) { hit = val; }
//	int HitIndex() const { return hit_idx; }
//	void HitIndex(int val) { hit_idx = val; }

//	uint32_t HitAddr() const { return hit_addr; }
//	void HitAddr(uint32_t val) { hit_addr = val; }

	const BreakPoint *Hit() const { return hit_bp; }
	void Hit(const BreakPoint *val) { hit_bp = val; }

	en_break_point_type Type() const { return type; }
	void Type(en_break_point_type val) { type = val; }
};

/**
	@brief Break Point Checker on VM
*/
class DEBUGGER_BPOINTS : public DEBUGGER_BASE
{
protected:
	BreakPoints bps[BreakPoints::BP_END];
	BreakPoints tps[BreakPoints::BP_END];

	bool m_now_breakpoint;
	bool m_now_tracepoint;
	bool m_now_basicreason;
	uint32_t m_stored_mask;

public:
	void find_fetch_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length);
	void find_mem_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length, uint32_t value);
	void find_io_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length, uint32_t value);
	void find_intr_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, uint32_t mask);
	void find_exception_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, uint32_t vector);
	void find_basic_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, DEVICE *d_mem, uint32_t addr, uint32_t data);

public:
	DEBUGGER_BPOINTS(VM* parent_vm, EMU* parent_emu, const char *identifier);
	virtual ~DEBUGGER_BPOINTS();

	void store_break_points(uint32_t addr, uint32_t mask, int len, int *add_index = NULL);
	void restore_break_points();

	BreakPoints::en_break_point_type hit_break_point() const;
	BreakPoints::en_break_point_type hit_trace_point() const;
	void reset_break_point();
	void reset_trace_point();

	BreakPoints *get_bps(BreakPoints::en_break_point_type type) { return &bps[type]; }
	BreakPoints *get_tps(BreakPoints::en_break_point_type type) { return &tps[type]; }

	bool now_breakpoint() const { return m_now_breakpoint; }
	bool now_tracepoint() const { return m_now_tracepoint; }
	bool now_basicreason() const { return m_now_basicreason; }
	bool now_breaktracepoint() const { return m_now_breakpoint | m_now_tracepoint; }
	uint32_t get_stored_mask() const { return m_stored_mask; }
};

#endif /* USE_DEBUGGER */

#endif /* DEBUGGER_BPOINT_H */
