/** @file mc68000mmu.h

	Skelton for retropc emulator

	@par Origin  MAME 0.239, 0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@note This is a part of MC68000BASE class, so included in mc68000.h.

	@brief [ mc68000 MMU ]
*/

#ifndef MC68000MMU_H
#define MC68000MMU_H

#include "../common.h"
#include "vm_defs.h"
#include "mc68000_consts.h"

#ifdef USE_MC68000MMU

// MMU status register bit definitions

#if 0
#define MMULOG logging->out_debugf
#else
#define MMULOG(...)
#endif

//#define MACHINE_SIDE_EFFECTS_ENABLED() (!machine().side_effects_disabled())
#define MACHINE_SIDE_EFFECTS_ENABLED() (true)

// MMU SR register fields
enum en_mmu_sr {
	M68K_MMU_SR_BUS_ERROR       = 0x8000,
	M68K_MMU_SR_SUPERVISOR_ONLY = 0x2000,
	M68K_MMU_SR_WRITE_PROTECT   = 0x0800,
	M68K_MMU_SR_INVALID         = 0x0400,
	M68K_MMU_SR_MODIFIED        = 0x0200,
	M68K_MMU_SR_TRANSPARENT     = 0x0040,
};

// MMU translation table descriptor field definitions
enum en_mmu_df {
	M68K_MMU_DF_DT              = 0x00000003,
	M68K_MMU_DF_DT_INVALID      = 0x00000000,
	M68K_MMU_DF_DT_PAGE         = 0x00000001,
	M68K_MMU_DF_DT_TABLE_4BYTE  = 0x00000002,
	M68K_MMU_DF_DT_TABLE_8BYTE  = 0x00000003,
	M68K_MMU_DF_WP              = 0x00000004,
	M68K_MMU_DF_USED            = 0x00000008,
	M68K_MMU_DF_MODIFIED        = 0x00000010,
	M68K_MMU_DF_CI              = 0x00000040,
	M68K_MMU_DF_SUPERVISOR      = 0x00000100,
	M68K_MMU_DF_ADDR_MASK       = 0xfffffff0,
	M68K_MMU_DF_IND_ADDR_MASK   = 0xfffffffc,
};

// MMU ATC Fields
enum en_mmu_atc {
	M68K_MMU_ATC_BUSERROR       = 0x08000000,
	M68K_MMU_ATC_CACHE_IN       = 0x04000000,
	M68K_MMU_ATC_WRITE_PR       = 0x02000000,
	M68K_MMU_ATC_MODIFIED       = 0x01000000,
	M68K_MMU_ATC_MASK           = 0x00ffffff,
	M68K_MMU_ATC_SHIFT          = 8,
	M68K_MMU_ATC_VALID          = 0x08000000,
};

// MMU Translation Control register
enum en_mmu_tc {
	M68K_MMU_TC_SRE             = 0x02000000,
	M68K_MMU_TC_FCL             = 0x01000000,
};

// TT register
enum en_mmu_tt {
	M68K_MMU_TT_ENABLE          = 0x8000,
};

/** decodes the effective address */
uint32_t DECODE_EA_32(int ea);

void pmmu_set_buserror(uint32_t addr_in);

/// pmmu_atc_add: adds this address to the ATC
void pmmu_atc_add(uint32_t logical, uint32_t physical, uint32_t fc, const int rw);

/// pmmu_atc_flush: flush entire ATC
/// 7fff0003 001ffd10 80f05750 is what should load
void pmmu_atc_flush();

void pmmu_atc_flush_fc_ea(const uint16_t modes);

template<bool ptest>
bool pmmu_atc_lookup(const uint32_t addr_in, uint32_t fc, const int rw,
					 uint32_t& addr_out)
{
	MMULOG("%s: LOOKUP addr_in=%08x, fc=%d, ptest=%d\n", __func__, addr_in, fc, ptest);
	const int ps = (m_mmu_tc >> 20) & 0xf;
	const uint32_t atc_tag = M68K_MMU_ATC_VALID | ((fc & 7) << 24) | ((addr_in >> ps) << (ps - 8));

	for (int i = 0; i < MMU_ATC_ENTRIES; i++)
	{

		if (m_mmu_atc_tag[i] != atc_tag)
		{
			continue;
		}

		const uint32_t atc_data = m_mmu_atc_data[i];

		if (!ptest && !rw)
		{
			// According to MC86030UM:
			// "If the M bit is clear and a write access to this logical
			// address is attempted, the MC68030 aborts the access and initiates a table
			// search, setting the M bit in the page descriptor, invalidating the old ATC
			// entry, and creating a new entry with the M bit set.
			if (!(atc_data & M68K_MMU_ATC_MODIFIED))
			{
				m_mmu_atc_tag[i] = 0;
				continue;
			}
		}

		m_mmu_tmp_sr = 0;
		if (atc_data & M68K_MMU_ATC_MODIFIED)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_MODIFIED;
		}

		if (atc_data & M68K_MMU_ATC_WRITE_PR)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_WRITE_PROTECT;
		}

		if (atc_data & M68K_MMU_ATC_BUSERROR)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_BUS_ERROR|M68K_MMU_SR_INVALID;
		}
		addr_out = (atc_data << 8) | (addr_in & ~(~0 << ps));
		MMULOG("%s: addr_in=%08x, addr_out=%08x, MMU SR %04x\n",
				__func__, addr_in, addr_out, m_mmu_tmp_sr);
		return true;
	}
	MMULOG("%s: lookup failed\n", __func__);
	if (ptest)
	{
		m_mmu_tmp_sr = M68K_MMU_SR_INVALID;
	}
	return false;
}

bool pmmu_match_tt(const uint32_t addr_in, const uint32_t fc, const uint32_t tt, const int rw);

void update_descriptor(const uint32_t tptr, const uint32_t fc, const int type, const uint32_t entry, const int rw);

template<bool _long>
void update_sr(const int type, const uint32_t tbl_entry, uint32_t fc)
{
	if (!MACHINE_SIDE_EFFECTS_ENABLED())
	{
		return;
	}

	switch(type)
	{
	case M68K_MMU_DF_DT_INVALID:
		// Invalid has no flags
		break;

	case M68K_MMU_DF_DT_PAGE:
		if (tbl_entry & M68K_MMU_DF_MODIFIED)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_MODIFIED;
		}
#if __cplusplus >= 201703L
		[[fallthrough]];
#endif
	case M68K_MMU_DF_DT_TABLE_4BYTE:
#if __cplusplus >= 201703L
		[[fallthrough]];
#endif
	case M68K_MMU_DF_DT_TABLE_8BYTE:

		if (tbl_entry & M68K_MMU_DF_WP)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_WRITE_PROTECT;
		}

		if (_long && !(fc & 4) && (tbl_entry & M68K_MMU_DF_SUPERVISOR))
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_SUPERVISOR_ONLY;
		}
		break;
	default:
		break;
	}
}

template<bool ptest>
bool pmmu_walk_tables(uint32_t addr_in, int type, uint32_t table, uint32_t fc,
						const int limit, const int rw, uint32_t &addr_out)
{
	int level = 0;
	const uint32_t bits = m_mmu_tc & 0xffff;
	const int pagesize = (m_mmu_tc >> 20) & 0xf;
	const int is = (m_mmu_tc >> 16) & 0xf;
	int bitpos = 12;
	int resolved = 0;
	int pageshift = is;

	addr_in <<= is;

	m_mmu_tablewalk = true;

	if (m_mmu_tc & M68K_MMU_TC_FCL)
	{
		bitpos = 16;
	}

	do
	{
		const int indexbits = (bits >> bitpos) & 0xf;
		const int table_index  = (bitpos == 16) ? fc : (addr_in >> (32 - indexbits));
		bitpos -= 4;
		const bool indirect = (!bitpos || !(bits >> bitpos)) && indexbits;
		uint32_t tbl_entry, tbl_entry2;

		MMULOG("%s: type %d, table %08x, addr_in %08x, indexbits %d, pageshift %d, indirect %d table_index %08x, rw=%d fc=%d\n",
				__func__, type, table, addr_in, indexbits, pageshift, indirect, table_index, rw, fc);

		switch(type)
		{
			case M68K_MMU_DF_DT_INVALID:   // invalid, will cause MMU exception
				m_mmu_tmp_sr = M68K_MMU_SR_INVALID;
				MMULOG("PMMU: DT0 PC=%x (addr_in %08x -> %08x)\n", m_ppc, addr_in, addr_out);
				resolved = 1;
				break;

			case M68K_MMU_DF_DT_PAGE:   // page descriptor, will cause direct mapping
				if (!ptest)
				{
					table &= ~0 << pagesize;
					addr_out = table + (addr_in >> pageshift);
				}
				resolved = 1;
				break;

			case M68K_MMU_DF_DT_TABLE_4BYTE:   // valid 4 byte descriptors
				level++;
				addr_out = table + (table_index << 2);
//				tbl_entry = m_program->read_dword(addr_out);
				tbl_entry = RM32(addr_out, fc);
				type = tbl_entry & M68K_MMU_DF_DT;

				if (indirect && (type == 2 || type == 3))
				{
					level++;
					MMULOG("SHORT INDIRECT DESC: %08x\n", tbl_entry);
					addr_out = tbl_entry & M68K_MMU_DF_IND_ADDR_MASK;
//					tbl_entry = m_program->read_dword(addr_out);
					tbl_entry = RM32(addr_out, fc);
					type = tbl_entry & M68K_MMU_DF_DT;
				}

				MMULOG("SHORT DESC: %08x\n", tbl_entry);
				table = tbl_entry & M68K_MMU_DF_ADDR_MASK;
				if (MACHINE_SIDE_EFFECTS_ENABLED())
				{
					update_sr<0>(type, tbl_entry, fc);
					if (!ptest)
					{
						update_descriptor(addr_out, fc, type, tbl_entry, rw);
					}
				}
				break;

			case M68K_MMU_DF_DT_TABLE_8BYTE:   // valid 8 byte descriptors
				level++;
				addr_out = table + (table_index << 3);
//				tbl_entry = m_program->read_dword(addr_out);
//				tbl_entry2 = m_program->read_dword(addr_out + 4);
				tbl_entry = RM32(addr_out, fc);
				tbl_entry2 = RM32(addr_out + 4, fc);
				type = tbl_entry & M68K_MMU_DF_DT;

				if (indirect && (type == 2 || type == 3))
				{
					level++;
					MMULOG("LONG INDIRECT DESC: %08x%08x\n", tbl_entry, tbl_entry2);
					addr_out = tbl_entry2 & M68K_MMU_DF_IND_ADDR_MASK;
//					tbl_entry = m_program->read_dword(addr_out);
//					tbl_entry2 = m_program->read_dword(addr_out);
					tbl_entry = RM32(addr_out, fc);
					tbl_entry2 = RM32(addr_out, fc);
					type = tbl_entry & M68K_MMU_DF_DT;
				}

				MMULOG("LONG DESC: %08x %08x\n", tbl_entry, tbl_entry2);
				table = tbl_entry2 & M68K_MMU_DF_ADDR_MASK;
				if (MACHINE_SIDE_EFFECTS_ENABLED())
				{
					update_sr<1>(type, tbl_entry, fc);
					if (!ptest)
					{
						update_descriptor(addr_out, fc, type, tbl_entry, rw);
					}
				}
				break;
		}

		if (m_mmu_tmp_sr & M68K_MMU_SR_BUS_ERROR)
		{
			// Bus erorr during page table walking is always fatal
			resolved = 1;
			break;
		}

		if (!ptest && MACHINE_SIDE_EFFECTS_ENABLED())
		{
			if (!rw && (m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
			{
				resolved = 1;
				break;
			}

			if (!(fc & 4) && (m_mmu_tmp_sr & M68K_MMU_SR_SUPERVISOR_ONLY))
			{
				resolved = 1;
				break;
			}

		}
		addr_in <<= indexbits;
		pageshift += indexbits;
	} while(level < limit && !resolved);


	m_mmu_tmp_sr &= 0xfff0;
	m_mmu_tmp_sr |= level;
	MMULOG("MMU SR after walk: %04X\n", m_mmu_tmp_sr);
	m_mmu_tablewalk = false;
	return (resolved != 0);
}

// pmmu_translate_addr_with_fc: perform 68851/68030-style PMMU address translation
template<bool ptest, bool pload>
uint32_t pmmu_translate_addr_with_fc(uint32_t addr_in, uint32_t fc, int rw, const int limit = 7)
{
	uint32_t addr_out = 0;


	MMULOG("%s: addr_in=%08x, fc=%d, ptest=%d, rw=%d, limit=%d\n",
			__func__, addr_in, fc, ptest, rw, limit);
	m_mmu_tmp_sr = 0;

	m_mmu_last_logical_addr = addr_in;

	if (pmmu_match_tt(addr_in, fc, m_mmu_tt0, rw) ||
		pmmu_match_tt(addr_in, fc, m_mmu_tt1, rw) ||
		fc == 7)
	{
		return addr_in;
	}

	if (ptest && limit == 0)
	{
		pmmu_atc_lookup<true>(addr_in, fc, rw, addr_out);
		return addr_out;
	}

	if (!ptest && !pload && pmmu_atc_lookup<false>(addr_in, fc, rw, addr_out))
	{
		if ((m_mmu_tmp_sr & M68K_MMU_SR_BUS_ERROR) || (!rw && (m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT)))
		{
			MMULOG("set atc hit buserror: addr_in=%08x, addr_out=%x, rw=%x, fc=%d, sz=%d\n",
					addr_in, addr_out, m_mmu_tmp_rw, m_mmu_tmp_fc, m_mmu_tmp_sz);
			pmmu_set_buserror(addr_in);
		}
		return addr_out;
	}

	int type;
	uint32_t tbl_addr;
	// if SRP is enabled and we're in supervisor mode, use it
	if ((m_mmu_tc & M68K_MMU_TC_SRE) && (fc & 4))
	{
		tbl_addr = m_mmu_srp_aptr & M68K_MMU_DF_ADDR_MASK;
		type = m_mmu_srp_limit & M68K_MMU_DF_DT;
	}
	else    // else use the CRP
	{
		tbl_addr = m_mmu_crp_aptr & M68K_MMU_DF_ADDR_MASK;
		type = m_mmu_crp_limit & M68K_MMU_DF_DT;
	}

	if (!pmmu_walk_tables<ptest>(addr_in, type, tbl_addr, fc, limit, rw, addr_out))
	{
		logging->out_logf(LOG_ERROR, "Table walk did not resolve\n");
	}

	if (ptest)
	{
		return addr_out;
	}

	if ((m_mmu_tmp_sr & (M68K_MMU_SR_INVALID|M68K_MMU_SR_SUPERVISOR_ONLY)) ||
			((m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT) && !rw))
	{

		if (!pload)
		{
			MMULOG("%s: set buserror (SR %04X)\n", __func__, m_mmu_tmp_sr);
			pmmu_set_buserror(addr_in);
		}
	}

	// it seems like at least the 68030 sets the M bit in the MMU SR
	// if the root descriptor is of PAGE type, so do a logical and
	// between RW and the root type
	if (MACHINE_SIDE_EFFECTS_ENABLED())
	{
		pmmu_atc_add(addr_in, addr_out, fc, rw && type != 1);
	}
	MMULOG("PMMU: [%08x] => [%08x] (SR %04x)\n", addr_in, addr_out, m_mmu_tmp_sr);
	return addr_out;
}


// FC bits: 2 = supervisor, 1 = program, 0 = data
// the 68040 is a subset of the 68851 and 68030 PMMUs - the page table sizes are fixed, there is no early termination, etc, etc.
uint32_t pmmu_translate_addr_with_fc_040(uint32_t addr_in, uint32_t fc, uint8_t ptest);

// pmmu_translate_addr: perform 68851/68030-style PMMU address translation
uint32_t pmmu_translate_addr(uint32_t addr_in, const int rw);

// m68851_mmu_ops: COP 0 MMU opcode handling
int fc_from_modes(const uint16_t modes);

void m68851_pload(const uint32_t ea, const uint16_t modes);

void m68851_ptest(const uint32_t ea, const uint16_t modes);

void m68851_pmove_get(uint32_t ea, uint16_t modes);

void m68851_pmove_put(uint32_t ea, uint16_t modes);

void m68851_pmove(uint32_t ea, uint16_t modes);

void m68851_mmu_ops();

/* Apple HMMU translation is much simpler */
inline uint32_t hmmu_translate_addr(uint32_t addr_in);

public:
int m68851_buserror(uint32_t& addr);

#else /* !USE_MC68000MMU */

#define pmmu_atc_flush()

#endif /* !USE_MC68000MMU */

#endif /* MC68000MMU_H */
