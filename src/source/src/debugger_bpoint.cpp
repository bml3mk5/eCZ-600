/** @file debugger_bpoint.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.10.20

	@brief [ debugger break point ]
*/

#include "debugger_bpoint.h"

#ifdef USE_DEBUGGER

#include "vm/device.h"
#include "utility.h"

#define MAX_BREAKPOINT_ITEMS	64

//

BreakPoint::BreakPoint()
{
	Clear();
}

BreakPoint::BreakPoint(uint32_t addr_, uint32_t mask_, int len_, int status_)
{
	Set(addr_, mask_, len_, status_);
}

BreakPoint::BreakPoint(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_)
{
	Set(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_);
}

BreakPoint::~BreakPoint()
{
}

void BreakPoint::Clear()
{
	addr = 0;
	mask = 0;
	len  = 0;	// range 0 - addr + len
	status = 0;	// 0 = none, 1 = enabled, -1 = disabled
	*regname = _T('\0');
	regptr = NULL;
	reglen = 0;
	regval = 0;
	regmask = 0;
}

void BreakPoint::Set(uint32_t addr_, uint32_t mask_, int len_, int status_)
{
	addr = addr_;
	mask = mask_;
	len  = len_;
	status = status_;
	*regname = _T('\0');
	regptr = NULL;
	reglen = 0;
	regval = 0;
	regmask = 0;
}

void BreakPoint::Set(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_)
{
	addr = addr_;
	mask = mask_;
	len  = len_;
	status = status_;
	if (regname_) {
		UTILITY::tcscpy(regname, sizeof(regname) / sizeof(regname[0]), regname_);
	} else {
		*regname = _T('\0');
	}
	regptr = regptr_;
	reglen = reglen_;
	regval = regval_;
	regmask = regmask_;
}

bool BreakPoint::MatchRegVal() const
{
	if (!regptr) return true;

	switch(reglen) {
	case 1:
		return (*reinterpret_cast<const uint8_t *>(regptr) == (regval & 0xff));
	case 2:
		return (*reinterpret_cast<const uint16_t *>(regptr) == (regval & 0xffff));
	case 4:
		return (*reinterpret_cast<const uint32_t *>(regptr) == regval);
	default:
		return false;
	}
}

bool BreakPoint::MatchValue(uint32_t val) const
{
	return (reglen == 0 || ((regval & regmask) == (val & regmask)));
}

void BreakPoint::Expression(_TCHAR *buffer, size_t buffer_len) const
{
	if (!regptr) return;

	_TCHAR fmt[8];
	switch(reglen) {
	case 1:
		UTILITY::tcscpy(fmt, 8, _T("%02X"));
		break;
	case 2:
		UTILITY::tcscpy(fmt, 8, _T("%04X"));
		break;
	default:
		UTILITY::tcscpy(fmt, 8, _T("%08X"));
		break;
	}

	UTILITY::tcscat(buffer, buffer_len, _T(" (exp.: "));
	UTILITY::tcscat(buffer, buffer_len, regname);
	UTILITY::tcscat(buffer, buffer_len, _T(" = "));
	UTILITY::sntprintf(buffer, buffer_len, fmt, regval);
	UTILITY::tcscat(buffer, buffer_len, _T(")"));
}

void BreakPoint::RegValue(_TCHAR *buffer, size_t buffer_len) const
{
	if (!reglen) return;

	UTILITY::tcscat(buffer, buffer_len, _T(" (value: "));
	UTILITY::sntprintf(buffer, buffer_len, _T("%02X"), regval);
	if (regmask != ~0) UTILITY::sntprintf(buffer, buffer_len, _T(" & %02X"), regmask);
	UTILITY::tcscat(buffer, buffer_len, _T(")"));
}

//

BreakPointTable::BreakPointTable()
	: CPtrList<BreakPoint>()
{
}

BreakPointTable::~BreakPointTable()
{
}

BreakPoint *BreakPointTable::IsExist(uint32_t addr_, uint32_t mask_, int len_, int *add_index_)
{
	for(int i=0; i<Count(); i++) {
		BreakPoint *bp = Item(i);
		if (bp->Status() != 0 && bp->Addr() == addr_ && bp->Mask() == mask_ && bp->Len() == len_) {
			if (add_index_) *add_index_ = i;
			return bp;
		}
	}
	return NULL;
}

/// @return 1:Update item  -1:No item exists
int BreakPointTable::ModifyExistItem(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &bp_, int *add_index_)
{
	int rc = -1;
	bp_ = IsExist(addr_, mask_, len_, add_index_);
	if (bp_) {
		// update
		bp_->Set(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_);
		rc = 1;
	}
	return rc;
}

/// @return 0:Add item  -1:No item exists
int BreakPointTable::ModifyDeletedItem(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &bp_, int *add_index_)
{
	for(int i=0; i<Count(); i++) {
		BreakPoint *bp = Item(i);
		if (bp->Status() == 0) {
			bp->Set(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_);
			if (add_index_) *add_index_ = i;
			bp_ = bp;
			return 0;
		}
	}
	return -1;
}

void BreakPointTable::Add(BreakPoint *value)
{
	CPtrList<BreakPoint>::Add(value);
}

/// @return 0:Add item  1:Update item
int BreakPointTable::Add(uint32_t addr_, uint32_t mask_, int len_, int status_, BreakPoint * &new_bp_, int *add_index_)
{
	int rc = ModifyExistItem(addr_, mask_, len_, status_, NULL, NULL, 0, 0, 0, new_bp_, add_index_);
	if (rc < 0) {
		rc = ModifyDeletedItem(addr_, mask_, len_, status_, NULL, NULL, 0, 0, 0, new_bp_, add_index_);
	}
	if (rc < 0) {
		if (Count() < MAX_BREAKPOINT_ITEMS) {
			new_bp_ = new BreakPoint(addr_, mask_, len_, status_);
			Add(new_bp_);
			if (add_index_) *add_index_ = Count() - 1;
			rc = 0;
		}
	}
	return rc;
}

/// @return 0:Add item  1:Update item
int BreakPointTable::Add(uint32_t addr_, uint32_t mask_, int len_, int status_, const _TCHAR *regname_, const void *regptr_, int reglen_, uint32_t regval_, uint32_t regmask_, BreakPoint * &new_bp_, int *add_index_)
{
	int rc = ModifyExistItem(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_, new_bp_, add_index_);
	if (rc < 0) {
		rc = ModifyDeletedItem(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_, new_bp_, add_index_);
	}
	if (rc < 0) {
		if (Count() < MAX_BREAKPOINT_ITEMS) {
			new_bp_ = new BreakPoint(addr_, mask_, len_, status_, regname_, regptr_, reglen_, regval_, regmask_);
			Add(new_bp_);
			if (add_index_) *add_index_ = Count() - 1;
			rc = 0;
		}
	}
	return rc;
}

//

BreakPoints::BreakPoints()
{
	for(int i=0; i<MAX_BREAK_POINTS; i++) {
		tbl_ena[i] = NULL;
		tbl_store[i] = NULL;
	}
	tbl_ena_count = 0;
	tbl_store_count = 0;

	hit_bp = NULL;
	cur_addr = ~0;
}

BreakPoints::~BreakPoints()
{
}

/// @return 0:Add item  1:Update item  -1:Cannot add
int BreakPoints::Add(uint32_t addr, uint32_t mask, int len, BreakPoint * &new_bp, int *add_index)
{
	int rc = table.Add(addr, mask, len, 1, new_bp, add_index);
	if (rc < 0) return rc;
	// set disable flag
	new_bp->Status(-1);
	// enable
	Enable(new_bp);
	return rc;
}

/// @return 0:Add item  1:Update item  -1:Cannot add
int BreakPoints::Add(uint32_t addr, uint32_t mask, int len, const _TCHAR *regname, const void *regptr, int reglen, uint32_t regval, uint32_t regmask, BreakPoint * &new_bp, int *add_index)
{
	int rc = table.Add(addr, mask, len, 1, regname, regptr, reglen, regval, regmask, new_bp, add_index);
	if (rc < 0) return rc;
	// set disable flag
	new_bp->Status(-1);
	// enable
	Enable(new_bp);
	return rc;
}

bool BreakPoints::Delete(int table_index)
{
	Disable(table_index);
	if (table_index < 0 || table_index >= table.Count()) {
		return false;
	}
//	table.Delete(table_index);
	// clear data in item
	// *not* delete
	table.Item(table_index)->Clear();
	return true;
}

/// @return 0 / ERR_OUT_OF_RANGE
int BreakPoints::Enable(int table_index)
{
	if (table_index < 0 || table_index >= table.Count()) {
		return ERR_OUT_OF_RANGE;
	}
	return Enable(table[table_index]);
}

/// @return 0 / ERR_ALREADY_EXISTS / ERR_TOO_MANY_ITEMS
int BreakPoints::Enable(BreakPoint *item)
{
	if (tbl_ena_count >= MAX_BREAK_POINTS) {
		return ERR_TOO_MANY_ITEMS;
	}

	// add enable table (sort by addr)
	item->Status(1);

	// already registered ?
	int i=0;
	if (IsExist(item)) {
		return ERR_ALREADY_EXISTS;
	}

	i=0;
	switch(type) {
	case BP_FETCH_OP:
	case BP_FETCH_OP_PH:
	case BP_READ_MEMORY:
	case BP_READ_MEMORY_PH:
	case BP_WRITE_MEMORY:
	case BP_WRITE_MEMORY_PH:
	case BP_INPUT_IO:
	case BP_OUTPUT_IO:
		for(; i<tbl_ena_count; i++) {
			BreakPoint *it = tbl_ena[i];
			if ((it->Addr() > item->Addr())
			 || (it->Addr() == item->Addr() && it->Len() > item->Len())) {
				// shift
				Shift(i);
				// insert
				tbl_ena[i] = item;
				return 0;
			}
		}
		break;
	default:
		for(; i<tbl_ena_count; i++) {
			BreakPoint *it = tbl_ena[i];
			if ( (it->Addr() > item->Addr())
			|| (it->Addr() == item->Addr() && it->Mask() > item->Mask())
			|| (it->Addr() == item->Addr() && it->Mask() == item->Mask() && it->Len() > item->Len()) ) {
				// shift
				Shift(i);
				// insert
				tbl_ena[i] = item;
				return 0;
			}
		}
		break;
	}
	if (i == tbl_ena_count) {
		// add last
		tbl_ena[i] = item;
		tbl_ena_count++;
	}
	return 0;
}

/// @return 0 / ERR_OUT_OF_RANGE
int BreakPoints::Disable(int table_index)
{
	if (table_index < 0 || table_index >= table.Count()) {
		return ERR_OUT_OF_RANGE;
	}
	return Disable(table[table_index]);
}

/// @return 0 / ERR_NO_ITEM_EXISTS / ERR_ALREADY_EXISTS
int BreakPoints::Disable(BreakPoint *item)
{
	if (tbl_ena_count <= 0) {
		return ERR_NO_ITEM_EXISTS;
	}

	// delete from enable table
	int i=0;
	for(; i<tbl_ena_count; i++) {
		BreakPoint *it = tbl_ena[i];
		if (it == item) {
			item->Status(-1);
			Unshift(i);
			return 0;
		}
	}
	return ERR_ALREADY_EXISTS;
}

void BreakPoints::DeleteAll()
{
	for(int i=0; i<table.Count(); i++) {
		Delete(i);
	}
}

bool BreakPoints::EnableAll()
{
	bool enabled = false;
	for(int i=0; i<table.Count(); i++) {
		int rc = Enable(i);
		enabled = (enabled || rc == 0 || rc == ERR_ALREADY_EXISTS);
	}
	return enabled;
}

bool BreakPoints::DisableAll()
{
	for(int i=0; i<table.Count(); i++) {
		Disable(i);
	}
	return true;
}

bool BreakPoints::IsExist(BreakPoint *item) const
{
	for(int i=0; i<tbl_ena_count; i++) {
		BreakPoint *it = tbl_ena[i];
		if (it == item) return true;
	}
	return false;
}

void BreakPoints::Shift(int ena_index)
{
	for(int i=(tbl_ena_count < MAX_BREAK_POINTS ? tbl_ena_count-1 : tbl_ena_count-2); i>=ena_index; i--) {
		tbl_ena[i + 1] = tbl_ena[i];
	}
	if (tbl_ena_count < MAX_BREAK_POINTS) tbl_ena_count++;
}

void BreakPoints::Unshift(int ena_index)
{
	for(int i=ena_index; i<(tbl_ena_count-1); i++) {
		tbl_ena[i] = tbl_ena[i + 1];
	}
	if (tbl_ena_count > 0) tbl_ena_count--;
}

const BreakPoint *BreakPoints::TableItem(int table_index) const
{
	if (table_index < 0 || table_index >= table.Count()) {
		return NULL;
	}
	return table.Item(table_index);
}

const BreakPoint *BreakPoints::EnableItem(int ena_index) const
{
	if (ena_index < 0 || ena_index >= tbl_ena_count) {
		return NULL;
	}
	return tbl_ena[ena_index];
}

void BreakPoints::Store(uint32_t addr, uint32_t mask, int len, int *add_index)
{
	Store();
	BreakPoint *bp;
	Add(addr, mask, len, bp, add_index);
}

void BreakPoints::Store()
{
	// disable current table
	for(int i=0; i<tbl_ena_count; i++) {
		tbl_ena[i]->Status(-1);
	}
	// exchange to store table
	for(int i=0; i<MAX_BREAK_POINTS; i++) {
		tbl_store[i] = tbl_ena[i];
		tbl_ena[i] = NULL;
	}
	tbl_store_count = tbl_ena_count;
	tbl_ena_count = 0;
}

void BreakPoints::Restore()
{
	// disable current table
	for(int i=0; i<tbl_ena_count; i++) {
		tbl_ena[i]->Status(-1);
	}
	// exchange to store table
	for(int i=0; i<MAX_BREAK_POINTS; i++) {
		tbl_ena[i] = tbl_store[i];
		tbl_store[i] = NULL;
	}
	tbl_ena_count = tbl_store_count;
	tbl_store_count = 0;

	// enable current table
	for(int i=0; i<tbl_ena_count; i++) {
		tbl_ena[i]->Status(1);
	}
}

const BreakPoint *BreakPoints::FindFetchCyc(uint32_t addr, int len, int start, int end, int depth)
{
	int idx = ((end - start) >> 1) + start;

	const BreakPoint *bp = EnableItem(idx);
	addr &= bp->Mask();
	if(addr >= bp->Addr() && addr < (bp->Addr() + bp->Len() + len)) {
		if (bp->MatchRegVal()) {
			Hit(bp);
			return bp;
		} else {
			return NULL;
		}
	}

	if (start == end) return NULL;

	if (bp->Addr() > addr) {
		return (start <= idx - 1 ? FindFetchCyc(addr, len, start, idx - 1, depth + 1) : NULL); 
	} else {
		return (idx + 1 <= end ? FindFetchCyc(addr, len, idx + 1, end, depth + 1) : NULL); 
	}
}

const BreakPoint *BreakPoints::FindMemoryCyc(uint32_t addr, int len, uint32_t value, int start, int end, int depth)
{
	int idx = ((end - start) >> 1) + start;

	const BreakPoint *bp = EnableItem(idx);
	addr &= bp->Mask();
	if(addr >= bp->Addr() && addr < (bp->Addr() + bp->Len() + len)) {
		if (bp->MatchValue(value)) {
			Hit(bp);
			return bp;
		} else {
			return NULL;
		}
	}

	if (start == end) return NULL;

	if (bp->Addr() > addr) {
		return (start <= idx - 1 ? FindMemoryCyc(addr, len, value, start, idx - 1, depth + 1) : NULL); 
	} else {
		return (idx + 1 <= end ? FindMemoryCyc(addr, len, value, idx + 1, end, depth + 1) : NULL); 
	}
}

const BreakPoint *BreakPoints::FindIOCyc(uint32_t addr, int len, uint32_t value, int start, int end, int depth)
{
	int idx = ((end - start) >> 1) + start;

	const BreakPoint *bp = EnableItem(idx);
	addr &= bp->Mask();
	if(addr >= bp->Addr() && addr < (bp->Addr() + bp->Len() + len)) {
		if (bp->MatchValue(value)) {
			Hit(bp);
			return bp;
		} else {
			return NULL;
		}
	}

	if (start == end) return NULL;

	if (bp->Addr() > addr) {
		return (start <= idx - 1 ? FindIOCyc(addr, len, value, start, idx - 1, depth + 1) : NULL); 
	} else {
		return (idx + 1 <= end ? FindIOCyc(addr, len, value, idx + 1, end, depth + 1) : NULL); 
	}
}

const BreakPoint *BreakPoints::FindFetch(uint32_t addr, int len)
{
	if (tbl_ena_count <= 0) return NULL;
	if (cur_addr == addr) return NULL;
	cur_addr = addr;

	return FindFetchCyc(addr, len, 0, tbl_ena_count - 1, 0);
}

const BreakPoint *BreakPoints::FindMemory(uint32_t addr, int len, uint32_t value)
{
	if (tbl_ena_count <= 0) return NULL;

	return FindMemoryCyc(addr, len, value, 0, tbl_ena_count - 1, 0);
}

const BreakPoint *BreakPoints::FindIO(uint32_t addr, int len, uint32_t value)
{
	if (tbl_ena_count <= 0) return NULL;

	return FindIOCyc(addr, len, value, 0, tbl_ena_count - 1, 0);
}

const BreakPoint *BreakPoints::FindInterrupt(uint32_t addr, uint32_t mask)
{
	const BreakPoint *found = NULL;
	for(int i = 0; i < tbl_ena_count; i++) {
		const BreakPoint *bp = EnableItem(i);
		if(addr == bp->Addr() && (((mask & BreakPoints::INTR_ON) == (bp->Mask() & BreakPoints::INTR_ON)) || ((bp->Mask() & BreakPoints::INTR_CHG) != 0))) {
			Hit(bp);
			found = bp;
			break;
		}
	}
	return found;
}

const BreakPoint *BreakPoints::FindException(uint32_t addr, uint32_t vector)
{
	const BreakPoint *found = NULL;
	for(int i = 0; i < tbl_ena_count; i++) {
		const BreakPoint *bp = EnableItem(i);
		if(addr == bp->Addr() && (vector == bp->Mask() || bp->Mask() == (uint32_t)-1)) {
			Hit(bp);
			found = bp;
			break;
		}
	}
	return found;
}

const BreakPoint *BreakPoints::FindBASIC(DEVICE *d_mem, uint32_t addr)
{
	const BreakPoint *found = NULL;
	for(int i = 0; i < tbl_ena_count; i++) {
		const BreakPoint *bp = EnableItem(i);
		if(addr == bp->Addr()) {
			if (d_mem->debug_basic_check_break_point(bp->Mask(), bp->Len())) {
				Hit(bp);
				found = bp;
				break;
			}
		}
	}
	return found;
}

//

DEBUGGER_BPOINTS::DEBUGGER_BPOINTS(VM* parent_vm, EMU* parent_emu, const char *identifier)
	: DEBUGGER_BASE(parent_vm, parent_emu, identifier)
{
	m_now_breakpoint = false;
	m_now_tracepoint = false;
	m_now_basicreason = false;
	m_stored_mask = 0;

	for(int i=0; i<BreakPoints::BP_END; i++) {
		bps[i].Type((BreakPoints::en_break_point_type)i);
		tps[i].Type((BreakPoints::en_break_point_type)i);
	}
}

DEBUGGER_BPOINTS::~DEBUGGER_BPOINTS()
{
}

void DEBUGGER_BPOINTS::find_fetch_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length)
{
	if (bps[bp_type].FindFetch(addr, length)) {
		m_now_suspended = m_now_breakpoint = true;
		d_detected = dbg;
	} else if (tps[bp_type].FindFetch(addr, length)) {
		m_now_suspended = m_now_tracepoint = true;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::find_mem_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length, uint32_t value)
{
	if (bps[bp_type].FindMemory(addr, length, value)) {
		m_now_suspended = m_now_breakpoint = true;
		d_detected = dbg;
	} else if (tps[bp_type].FindMemory(addr, length, value)) {
		m_now_suspended = m_now_tracepoint = true;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::find_io_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, int length, uint32_t value)
{
	if (bps[bp_type].FindIO(addr, length, value)) {
		m_now_suspended = m_now_breakpoint = true;
		d_detected = dbg;
	} else if (tps[bp_type].FindIO(addr, length, value)) {
		m_now_suspended = m_now_tracepoint = true;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::find_intr_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, uint32_t mask)
{
	if (bps[bp_type].FindInterrupt(addr, mask)) {
		m_now_suspended = m_now_breakpoint = true;
		m_stored_mask = mask;
		d_detected = dbg;
	} else if (tps[bp_type].FindInterrupt(addr, mask)) {
		m_now_suspended = m_now_tracepoint = true;
		m_stored_mask = mask;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::find_exception_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, uint32_t addr, uint32_t vector)
{
	if (bps[bp_type].FindException(addr, vector)) {
		m_now_suspended = m_now_breakpoint = true;
		m_stored_mask = vector;
		d_detected = dbg;
	} else if (tps[bp_type].FindException(addr, vector)) {
		m_now_suspended = m_now_tracepoint = true;
		m_stored_mask = vector;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::find_basic_break_trace_points(DEBUGGER_BUS_BASE *dbg, BreakPoints::en_break_point_type bp_type, DEVICE *d_mem, uint32_t addr, uint32_t data)
{
	if (bps[bp_type].FindBASIC(d_mem, addr)) {
		m_now_suspended = m_now_basicreason = m_now_breakpoint = true;
		d_detected = dbg;
	} else if (tps[bp_type].FindBASIC(d_mem, addr)) {
		m_now_suspended = m_now_basicreason = m_now_tracepoint = true;
		d_detected = dbg;
	}
}

void DEBUGGER_BPOINTS::store_break_points(uint32_t addr, uint32_t mask, int len, int *add_index)
{
	for(int i=0; i<BreakPoints::BP_END; i++) {
		if (i == BreakPoints::BP_FETCH_OP) {
			bps[i].Store(addr, mask, len, add_index);
		} else {
			bps[i].Store();
		}
		tps[i].Store();
	}
}

void DEBUGGER_BPOINTS::restore_break_points()
{
	for(int i=0; i<BreakPoints::BP_END; i++) {
		bps[i].Restore();
		tps[i].Restore();
	}
}

BreakPoints::en_break_point_type DEBUGGER_BPOINTS::hit_break_point() const
{
	BreakPoints::en_break_point_type hit = BreakPoints::BP_END;
	if (m_now_breakpoint) {
		for(int i = BreakPoints::BP_END - 1; i>=0; i--) {
			if (bps[i].Hit()) {
				hit = (BreakPoints::en_break_point_type)i;
				break;
			}
		}
	}
	return hit;
}

BreakPoints::en_break_point_type DEBUGGER_BPOINTS::hit_trace_point() const
{
	BreakPoints::en_break_point_type hit = BreakPoints::BP_END;
	if (m_now_tracepoint) {
		for(int i = BreakPoints::BP_END - 1; i>=0; i--) {
			if (tps[i].Hit()) {
				hit = (BreakPoints::en_break_point_type)i;
				break;
			}
		}
	}
	return hit;
}

void DEBUGGER_BPOINTS::reset_break_point()
{
	for(int i=0; i<BreakPoints::BP_END; i++) {
		bps[i].Hit(NULL);
	}
	m_now_breakpoint = false;
}

void DEBUGGER_BPOINTS::reset_trace_point()
{
	for(int i=0; i<BreakPoints::BP_END; i++) {
		tps[i].Hit(NULL);
	}
	m_now_tracepoint = false;
}

#endif /* USE_DEBUGGER */
