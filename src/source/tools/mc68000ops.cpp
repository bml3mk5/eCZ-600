/** @file mc68000ops.cpp

	Skelton for retropc emulator

	@par Origin MAME 0.239,0.152 / Musashi

	@author Sasaji
	@date   2022.01.01-

	@brief [ MC68000ops ]

	@attention Generated source, edits will be lost.  Run m68kmake.py instead
*/

#include "mc68000.h"
#include "../logging.h"

void MC68000BASE::xa000_1010_071234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_1010);


}
void MC68000BASE::xf000_1111_071234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_1111);


}
void MC68000BASE::xf200_040fpu0_l_234f()
{
#if defined(USE_MC68000FPU)
	if(m_has_fpu) {
		m68040_fpu_op0();
	} else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xf300_040fpu1_l_234f()
{
#if defined(USE_MC68000FPU)
	if(m_has_fpu) {
		m68040_fpu_op1();
	} else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xc100_abcd_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void MC68000BASE::xcf08_abcd_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xc10f_abcd_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xcf0f_abcd_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xc108_abcd_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xd000_add_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd010_add_b_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd018_add_b_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd01f_add_b_pi7_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd020_add_b_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd027_add_b_pd7_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd028_add_b_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd030_add_b_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd038_add_b_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd039_add_b_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd03a_add_b_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd03b_add_b_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd03c_add_b_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd040_add_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd048_add_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd050_add_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd058_add_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd060_add_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd068_add_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd070_add_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd078_add_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd079_add_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd07a_add_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd07b_add_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd07c_add_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::xd080_add_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd088_add_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = AY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd090_add_l_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd098_add_l_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0a0_add_l_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0a8_add_l_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0b0_add_l_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0b8_add_l_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0b9_add_l_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0ba_add_l_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0bb_add_l_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd0bc_add_l_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::xd110_add_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd118_add_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd11f_add_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd120_add_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd127_add_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd128_add_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd130_add_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd138_add_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd139_add_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xd150_add_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd158_add_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd160_add_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd168_add_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd170_add_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd178_add_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd179_add_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xd190_add_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd198_add_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd1a0_add_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd1a8_add_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd1b0_add_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd1b8_add_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd1b9_add_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd0c0_adda_w_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY()));


}
void MC68000BASE::xd0c8_adda_w_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY()));


}
void MC68000BASE::xd0d0_adda_w_ai_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0d8_adda_w_pi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0e0_adda_w_pd_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0e8_adda_w_di_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0f0_adda_w_ix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0f8_adda_w_aw_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0f9_adda_w_al_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0fa_adda_w_pcdi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0fb_adda_w_pcix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd0fc_adda_w_i_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1c0_adda_l_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY());


}
void MC68000BASE::xd1c8_adda_l_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY());


}
void MC68000BASE::xd1d0_adda_l_ai_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1d8_adda_l_pi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1e0_adda_l_pd_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1e8_adda_l_di_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1f0_adda_l_ix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1f8_adda_l_aw_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1f9_adda_l_al_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1fa_adda_l_pcdi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1fb_adda_l_pcix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::xd1fc_adda_l_i_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void MC68000BASE::x0600_addi_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x0610_addi_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0618_addi_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x061f_addi_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0620_addi_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0627_addi_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0628_addi_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0630_addi_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0638_addi_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0639_addi_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0640_addi_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x0650_addi_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0658_addi_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0660_addi_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0668_addi_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0670_addi_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0678_addi_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0679_addi_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0680_addi_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x0690_addi_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x0698_addi_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x06a0_addi_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x06a8_addi_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x06b0_addi_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x06b8_addi_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x06b9_addi_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x5000_addq_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x5010_addq_b_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5018_addq_b_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x501f_addq_b_pi7_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5020_addq_b_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5027_addq_b_pd7_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5028_addq_b_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5030_addq_b_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5038_addq_b_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5039_addq_b_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5040_addq_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x5048_addq_w_071234fc()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);


}
void MC68000BASE::x5050_addq_w_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5058_addq_w_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5060_addq_w_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5068_addq_w_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5070_addq_w_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5078_addq_w_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5079_addq_w_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5080_addq_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x5088_addq_l_071234fc()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);


}
void MC68000BASE::x5090_addq_l_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x5098_addq_l_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x50a0_addq_l_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x50a8_addq_l_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x50b0_addq_l_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x50b8_addq_l_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x50b9_addq_l_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::xd100_addx_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void MC68000BASE::xd140_addx_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void MC68000BASE::xd180_addx_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void MC68000BASE::xdf08_addx_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xd10f_addx_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xdf0f_addx_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xd108_addx_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::xd148_addx_w_071234fc()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t ea  = EA_AX_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::xd188_addx_l_071234fc()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t ea  = EA_AX_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::xc000_and_b_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (DY() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc010_and_b_ai_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_AI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc018_and_b_pi_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc01f_and_b_pi7_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc020_and_b_pd_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc027_and_b_pd7_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc028_and_b_di_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_DI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc030_and_b_ix_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_IX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc038_and_b_aw_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AW_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc039_and_b_al_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AL_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc03a_and_b_pcdi_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCDI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc03b_and_b_pcix_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCIX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc03c_and_b_i_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc040_and_w_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (DY() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc050_and_w_ai_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_AI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc058_and_w_pi_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc060_and_w_pd_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PD_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc068_and_w_di_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_DI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc070_and_w_ix_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_IX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc078_and_w_aw_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AW_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc079_and_w_al_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AL_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc07a_and_w_pcdi_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCDI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc07b_and_w_pcix_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCIX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc07c_and_w_i_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc080_and_l_071234fc()
{
	m_not_z_flag = DX() &= DY();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc090_and_l_ai_071234fc()
{
	m_not_z_flag = DX() &= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc098_and_l_pi_071234fc()
{
	m_not_z_flag = DX() &= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0a0_and_l_pd_071234fc()
{
	m_not_z_flag = DX() &= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0a8_and_l_di_071234fc()
{
	m_not_z_flag = DX() &= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0b0_and_l_ix_071234fc()
{
	m_not_z_flag = DX() &= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0b8_and_l_aw_071234fc()
{
	m_not_z_flag = DX() &= OPER_AW_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0b9_and_l_al_071234fc()
{
	m_not_z_flag = DX() &= OPER_AL_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0ba_and_l_pcdi_071234fc()
{
	m_not_z_flag = DX() &= OPER_PCDI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0bb_and_l_pcix_071234fc()
{
	m_not_z_flag = DX() &= OPER_PCIX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc0bc_and_l_i_071234fc()
{
	m_not_z_flag = DX() &= OPER_I_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xc110_and_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc118_and_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc11f_and_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc120_and_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc127_and_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc128_and_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc130_and_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc138_and_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc139_and_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = DX() & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::xc150_and_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc158_and_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc160_and_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc168_and_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc170_and_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc178_and_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc179_and_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = DX() & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::xc190_and_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc198_and_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc1a0_and_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc1a8_and_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc1b0_and_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc1b8_and_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::xc1b9_and_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x0200_andi_b_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DY() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0210_andi_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0218_andi_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x021f_andi_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0220_andi_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0227_andi_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0228_andi_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0230_andi_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0238_andi_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0239_andi_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = src & read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_8(ea, res);


}
void MC68000BASE::x0240_andi_w_071234fc()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DY() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0250_andi_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0258_andi_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0260_andi_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0268_andi_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0270_andi_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0278_andi_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0279_andi_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = src & read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_16(ea, res);


}
void MC68000BASE::x0280_andi_l_071234fc()
{
	m_not_z_flag = DY() &= (OPER_I_32());

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0290_andi_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x0298_andi_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x02a0_andi_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x02a8_andi_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x02b0_andi_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x02b8_andi_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x02b9_andi_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src & read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	write_32(ea, res);


}
void MC68000BASE::x023c_andi_w_071234fc()
{
	uint32_t src = OPER_I_8();
	if (CPU_TYPE_IS_000()) {
		/* dummy read */
		read_dummy_imm_16();
	}
	set_ccr(get_ccr() & src);


}
void MC68000BASE::x027c_andi_w_071234fc()
{
	if(m_s_flag) {
		uint32_t src = OPER_I_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if (CPU_TYPE_IS_000()) {
			/* dummy read */
			read_dummy_imm_16();
		}
		set_sr(get_sr() & src);
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::xe000_asr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void MC68000BASE::xe040_asr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void MC68000BASE::xe080_asr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void MC68000BASE::xe020_asr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 8) {
			if(GET_MSB_8(src))
				res |= m68ki_shift_8_table[shift];

			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

			m_x_flag = m_c_flag = src << (9-shift);
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;

		} else if(GET_MSB_8(src)) {
			*r_dst |= 0xff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;

		} else {
			*r_dst &= 0xffffff00;
			m_c_flag = CFLAG_CLEAR;
			m_x_flag = XFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe060_asr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 16) {
			if(GET_MSB_16(src))
				res |= m68ki_shift_16_table[shift];

			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;

		} else if(GET_MSB_16(src)) {
			*r_dst |= 0xffff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;

		} else {
			*r_dst &= 0xffff0000;
			m_c_flag = CFLAG_CLEAR;
			m_x_flag = XFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe0a0_asr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 32) {
			if(GET_MSB_32(src))
				res |= m68ki_shift_32_table[shift];

			*r_dst = res;

			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;

		} else if(GET_MSB_32(src)) {
			*r_dst = 0xffffffff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;

		} else {
			*r_dst = 0;
			m_c_flag = CFLAG_CLEAR;
			m_x_flag = XFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe0d0_asr_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0d8_asr_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0e0_asr_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0e8_asr_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0f0_asr_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0f8_asr_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe0f9_asr_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void MC68000BASE::xe100_asl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_x_flag = m_c_flag = src << shift;
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	src &= m68ki_shift_8_table[shift + 1];
	m_v_flag = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;


}
void MC68000BASE::xe140_asl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;


}
void MC68000BASE::xe180_asl_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;


}
void MC68000BASE::xe120_asl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 8) {
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			src &= m68ki_shift_8_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_8_table[shift + 1]))<<7;

		} else {
			*r_dst &= 0xffffff00;
			m_x_flag = m_c_flag = ((shift == 8 ? src & 1 : 0))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = (!(src == 0))<<7;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe160_asl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 16) {
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_x_flag = m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			src &= m68ki_shift_16_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
		} else {
			*r_dst &= 0xffff0000;
			m_x_flag = m_c_flag = ((shift == 16 ? src & 1 : 0))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = (!(src == 0))<<7;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe1a0_asl_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 32) {
			*r_dst = res;
			m_x_flag = m_c_flag = (src >> (32 - shift)) << 8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			src &= m68ki_shift_32_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
		} else {
			*r_dst = 0;
			m_x_flag = m_c_flag = ((shift == 32 ? src & 1 : 0))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = (!(src == 0))<<7;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe1d0_asl_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1d8_asl_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1e0_asl_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1e8_asl_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1f0_asl_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1f8_asl_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::xe1f9_asl_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void MC68000BASE::x6200_bhi_b_071234fc()
{
	if(COND_HI()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6300_bls_b_071234fc()
{
	if(COND_LS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6400_bcc_b_071234fc()
{
	if(COND_CC()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6500_bcs_b_071234fc()
{
	if(COND_CS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6600_bne_b_071234fc()
{
	if(COND_NE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6700_beq_b_071234fc()
{
	if(COND_EQ()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6800_bvc_b_071234fc()
{
	if(COND_VC()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6900_bvs_b_071234fc()
{
	if(COND_VS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6a00_bpl_b_071234fc()
{
	if(COND_PL()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6b00_bmi_b_071234fc()
{
	if(COND_MI()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6c00_bge_b_071234fc()
{
	if(COND_GE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6d00_blt_b_071234fc()
{
	if(COND_LT()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6e00_bgt_b_071234fc()
{
	if(COND_GT()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6f00_ble_b_071234fc()
{
	if(COND_LE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));

	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6200_bhi_w_071234fc()
{
	uint32_t offset;
	if(COND_HI()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6300_bls_w_071234fc()
{
	uint32_t offset;
	if(COND_LS()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6400_bcc_w_071234fc()
{
	uint32_t offset;
	if(COND_CC()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6500_bcs_w_071234fc()
{
	uint32_t offset;
	if(COND_CS()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6600_bne_w_071234fc()
{
	uint32_t offset;
	if(COND_NE()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6700_beq_w_071234fc()
{
	uint32_t offset;
	if(COND_EQ()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6800_bvc_w_071234fc()
{
	uint32_t offset;
	if(COND_VC()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6900_bvs_w_071234fc()
{
	uint32_t offset;
	if(COND_VS()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6a00_bpl_w_071234fc()
{
	uint32_t offset;
	if(COND_PL()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6b00_bmi_w_071234fc()
{
	uint32_t offset;
	if(COND_MI()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6c00_bge_w_071234fc()
{
	uint32_t offset;
	if(COND_GE()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6d00_blt_w_071234fc()
{
	uint32_t offset;
	if(COND_LT()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6e00_bgt_w_071234fc()
{
	uint32_t offset;
	if(COND_GT()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x6f00_ble_w_071234fc()
{
	uint32_t offset;
	if(COND_LE()) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);

	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_bcc_notake_w);
	}


}
void MC68000BASE::x62ff_bhi_l_071()
{
	if(COND_HI()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x63ff_bls_l_071()
{
	if(COND_LS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x64ff_bcc_l_071()
{
	if(COND_CC()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x65ff_bcs_l_071()
{
	if(COND_CS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x66ff_bne_l_071()
{
	if(COND_NE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x67ff_beq_l_071()
{
	if(COND_EQ()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x68ff_bvc_l_071()
{
	if(COND_VC()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x69ff_bvs_l_071()
{
	if(COND_VS()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6aff_bpl_l_071()
{
	if(COND_PL()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6bff_bmi_l_071()
{
	if(COND_MI()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6cff_bge_l_071()
{
	if(COND_GE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6dff_blt_l_071()
{
	if(COND_LT()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6eff_bgt_l_071()
{
	if(COND_GT()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x6fff_ble_l_071()
{
	if(COND_LE()) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_8(MASK_OUT_ABOVE_8(m_ir));
	} else {
		non_branch_8(MASK_OUT_ABOVE_8(m_ir));
		SET_ICOUNT(m_cyc_bcc_notake_b);
	}


}
void MC68000BASE::x62ff_bhi_l_234fc()
{
	if(COND_HI()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x63ff_bls_l_234fc()
{
	if(COND_LS()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x64ff_bcc_l_234fc()
{
	if(COND_CC()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x65ff_bcs_l_234fc()
{
	if(COND_CS()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x66ff_bne_l_234fc()
{
	if(COND_NE()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x67ff_beq_l_234fc()
{
	if(COND_EQ()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x68ff_bvc_l_234fc()
{
	if(COND_VC()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x69ff_bvs_l_234fc()
{
	if(COND_VS()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6aff_bpl_l_234fc()
{
	if(COND_PL()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6bff_bmi_l_234fc()
{
	if(COND_MI()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6cff_bge_l_234fc()
{
	if(COND_GE()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6dff_blt_l_234fc()
{
	if(COND_LT()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6eff_bgt_l_234fc()
{
	if(COND_GT()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x6fff_ble_l_234fc()
{
	if(COND_LE()) {
		uint32_t offset = OPER_I_16();
		offset <<= 16;
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 4;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset);

	} else {
		uint32_t offset = OPER_I_32();
//		m_pc += 4;
		non_branch_32(offset);
	}


}
void MC68000BASE::x0140_bchg_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;


}
void MC68000BASE::x0150_bchg_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0158_bchg_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x015f_bchg_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0160_bchg_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0167_bchg_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0168_bchg_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0170_bchg_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0178_bchg_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0179_bchg_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0840_bchg_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;


}
void MC68000BASE::x0850_bchg_b_ai_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0858_bchg_b_pi_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x085f_bchg_b_pi7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0860_bchg_b_pd_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0867_bchg_b_pd7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0868_bchg_b_di_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0870_bchg_b_ix_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0878_bchg_b_aw_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0879_bchg_b_al_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src ^ mask);


}
void MC68000BASE::x0180_bclr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;


}
void MC68000BASE::x0190_bclr_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x0198_bclr_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x019f_bclr_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01a0_bclr_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01a7_bclr_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01a8_bclr_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01b0_bclr_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01b8_bclr_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x01b9_bclr_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x0880_bclr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;


}
void MC68000BASE::x0890_bclr_b_ai_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x0898_bclr_b_pi_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x089f_bclr_b_pi7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08a0_bclr_b_pd_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08a7_bclr_b_pd7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08a8_bclr_b_di_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08b0_bclr_b_ix_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08b8_bclr_b_aw_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::x08b9_bclr_b_al_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src & ~mask);


}
void MC68000BASE::xeac0_bfchg_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t* data = &DY();
	uint64_t mask;


	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask = ROR_32(mask, offset);

	m_n_flag = NFLAG_32(*data<<offset);
	m_not_z_flag = *data & mask;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	*data ^= mask;


}
void MC68000BASE::xead0_bfchg_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long ^ mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte ^ mask_byte);
	}


}
void MC68000BASE::xeae8_bfchg_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long ^ mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte ^ mask_byte);
	}


}
void MC68000BASE::xeaf0_bfchg_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long ^ mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte ^ mask_byte);
	}


}
void MC68000BASE::xeaf8_bfchg_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long ^ mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte ^ mask_byte);
	}


}
void MC68000BASE::xeaf9_bfchg_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long ^ mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte ^ mask_byte);
	}


}
void MC68000BASE::xecc0_bfclr_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t* data = &DY();
	uint64_t mask;

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask = ROR_32(mask, offset);

	m_n_flag = NFLAG_32(*data<<offset);
	m_not_z_flag = *data & mask;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	*data &= ~mask;


}
void MC68000BASE::xecd0_bfclr_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long & ~mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte & ~mask_byte);
	}


}
void MC68000BASE::xece8_bfclr_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long & ~mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte & ~mask_byte);
	}


}
void MC68000BASE::xecf0_bfclr_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long & ~mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte & ~mask_byte);
	}


}
void MC68000BASE::xecf8_bfclr_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long & ~mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte & ~mask_byte);
	}


}
void MC68000BASE::xecf9_bfclr_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long & ~mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte & ~mask_byte);
	}


}
void MC68000BASE::xebc0_bfexts_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint64_t data = DY();

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	data = ROL_32(data, offset);
	m_n_flag = NFLAG_32(data);
	data = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = MASK_OUT_ABOVE_32(data);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2>>12)&7] = MASK_OUT_ABOVE_32(data);


}
void MC68000BASE::xebd0_bfexts_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebe8_bfexts_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebf0_bfexts_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebf8_bfexts_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebf9_bfexts_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebfa_bfexts_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_PCDI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xebfb_bfexts_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_PCIX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);

	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  = MAKE_INT_32(data) >> (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9c0_bfextu_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint64_t data = DY();

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	data = ROL_32(data, offset);
	m_n_flag = NFLAG_32(data);
	data >>= 32 - width;

	m_not_z_flag = MASK_OUT_ABOVE_32(data);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2>>12)&7] = MASK_OUT_ABOVE_32(data);


}
void MC68000BASE::xe9d0_bfextu_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9e8_bfextu_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9f0_bfextu_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9f8_bfextu_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9f9_bfextu_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9fa_bfextu_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_PCDI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xe9fb_bfextu_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t data;
	uint32_t ea = EA_PCIX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<offset);

	if((offset+width) > 32)
		data |= (read_8(ea+4) << offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	REG_D()[(word2 >> 12) & 7] = data;


}
void MC68000BASE::xedc0_bfffo_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint64_t data = DY();
	uint32_t bit;

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	data = ROL_32(data, offset);
	m_n_flag = NFLAG_32(data);
	data >>= 32 - width;

	m_not_z_flag = MASK_OUT_ABOVE_32(data);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedd0_bfffo_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xede8_bfffo_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedf0_bfffo_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedf8_bfffo_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedf9_bfffo_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedfa_bfffo_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_PCDI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xedfb_bfffo_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	int32_t local_offset;
	uint32_t width = word2;
	uint32_t data;
	uint32_t bit;
	uint32_t ea = EA_PCIX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	local_offset = offset % 8;
	if(local_offset < 0) {
		local_offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	data = (offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	data = MASK_OUT_ABOVE_32(data<<local_offset);

	if((local_offset+width) > 32)
		data |= (read_8(ea+4) << local_offset) >> 8;

	m_n_flag = NFLAG_32(data);
	data  >>= (32 - width);

	m_not_z_flag = data;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
		offset++;

	REG_D()[(word2>>12)&7] = offset;


}
void MC68000BASE::xefc0_bfins_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t* data = &DY();
	uint64_t mask;
	uint64_t insert = REG_D()[(word2>>12)&7];

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask = ROR_32(mask, offset);

	insert = MASK_OUT_ABOVE_32(insert << (32 - width));
	m_n_flag = NFLAG_32(insert);
	m_not_z_flag = MASK_OUT_ABOVE_32(insert);
	insert = ROR_32(insert, offset);

	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	*data &= ~mask;
	*data |= insert;


}
void MC68000BASE::xefd0_bfins_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t insert_base = REG_D()[(word2>>12)&7];
	uint32_t insert_long;
	uint32_t insert_byte;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
	m_n_flag = NFLAG_32(insert_base);
	m_not_z_flag = insert_base;
	insert_long = insert_base >> offset;

	data_long = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) < 8) {
		write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
	} else if((width + offset) < 16) {
		write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
	} else {
		write_32(ea, (data_long & ~mask_long) | insert_long);
	}

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (insert_byte & mask_byte);
		write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
	}


}
void MC68000BASE::xefe8_bfins_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t insert_base = REG_D()[(word2>>12)&7];
	uint32_t insert_long;
	uint32_t insert_byte;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
	m_n_flag = NFLAG_32(insert_base);
	m_not_z_flag = insert_base;
	insert_long = insert_base >> offset;

	data_long = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) < 8) {
		write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
	} else if((width + offset) < 16) {
		write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
	} else {
		write_32(ea, (data_long & ~mask_long) | insert_long);
	}

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (insert_byte & mask_byte);
		write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
	}


}
void MC68000BASE::xeff0_bfins_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t insert_base = REG_D()[(word2>>12)&7];
	uint32_t insert_long;
	uint32_t insert_byte;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
	m_n_flag = NFLAG_32(insert_base);
	m_not_z_flag = insert_base;
	insert_long = insert_base >> offset;

	data_long = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) < 8) {
		write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
	} else if((width + offset) < 16) {
		write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
	} else {
		write_32(ea, (data_long & ~mask_long) | insert_long);
	}

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (insert_byte & mask_byte);
		write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
	}


}
void MC68000BASE::xeff8_bfins_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t insert_base = REG_D()[(word2>>12)&7];
	uint32_t insert_long;
	uint32_t insert_byte;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
	m_n_flag = NFLAG_32(insert_base);
	m_not_z_flag = insert_base;
	insert_long = insert_base >> offset;

	data_long = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) < 8) {
		write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
	} else if((width + offset) < 16) {
		write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
	} else {
		write_32(ea, (data_long & ~mask_long) | insert_long);
	}

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (insert_byte & mask_byte);
		write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
	}


}
void MC68000BASE::xeff9_bfins_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t insert_base = REG_D()[(word2>>12)&7];
	uint32_t insert_long;
	uint32_t insert_byte;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	if(BIT_B(word2)) {
		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0) {
			offset += 8;
			ea--;
		}
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
	m_n_flag = NFLAG_32(insert_base);
	m_not_z_flag = insert_base;
	insert_long = insert_base >> offset;

	data_long = (offset+width) < 8 ? (read_8(ea) << 24) :
			(offset+width) < 16 ? (read_16(ea) << 16) : read_32(ea);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) < 8) {
		write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
	} else if((width + offset) < 16) {
		write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
	} else {
		write_32(ea, (data_long & ~mask_long) | insert_long);
	}

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (insert_byte & mask_byte);
		write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
	}


}
void MC68000BASE::xeec0_bfset_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t* data = &DY();
	uint64_t mask;

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask = ROR_32(mask, offset);

	m_n_flag = NFLAG_32(*data<<offset);
	m_not_z_flag = *data & mask;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	*data |= mask;


}
void MC68000BASE::xeed0_bfset_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long | mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte | mask_byte);
	}


}
void MC68000BASE::xeee8_bfset_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long | mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte | mask_byte);
	}


}
void MC68000BASE::xeef0_bfset_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long | mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte | mask_byte);
	}


}
void MC68000BASE::xeef8_bfset_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long | mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte | mask_byte);
	}


}
void MC68000BASE::xeef9_bfset_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = NFLAG_32(data_long << offset);
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	write_32(ea, data_long | mask_long);

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
		write_8(ea+4, data_byte | mask_byte);
	}


}
void MC68000BASE::xe8c0_bftst_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t* data = &DY();
	uint64_t mask;

	if(BIT_B(word2))
		offset = REG_D()[offset&7];
	if(BIT_5(word2))
		width = REG_D()[width&7];

	offset &= 31;
	width = ((width-1) & 31) + 1;

	mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask = ROR_32(mask, offset);

	m_n_flag = NFLAG_32(*data<<offset);
	m_not_z_flag = *data & mask;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xe8d0_bftst_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_AI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8e8_bftst_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_DI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8f0_bftst_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AY_IX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8f8_bftst_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AW_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8f9_bftst_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_AL_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8fa_bftst_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_PCDI_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::xe8fb_bftst_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t offset = (word2>>6)&31;
	uint32_t width = word2;
	uint32_t mask_base;
	uint32_t data_long;
	uint32_t mask_long;
	uint32_t data_byte = 0;
	uint32_t mask_byte = 0;
	uint32_t ea = EA_PCIX_8();

	if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
	if(BIT_5(word2))
		width = REG_D()[width&7];

	/* Offset is signed so we have to use ugly math =( */
	ea += offset / 8;
	offset %= 8;
	if(offset < 0) {
		offset += 8;
		ea--;
	}
	width = ((width-1) & 31) + 1;

	mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
	mask_long = mask_base >> offset;

	data_long = read_32(ea);
	m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
	m_not_z_flag = data_long & mask_long;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	if((width + offset) > 32) {
		mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
		data_byte = read_8(ea+4);
		m_not_z_flag |= (data_byte & mask_byte);
	}


}
void MC68000BASE::x4848_bkpt_1()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_BREAKPOINT);

}
void MC68000BASE::x4848_bkpt_234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_BREAKPOINT);


}
void MC68000BASE::x6000_bra_b_071234fc()
{
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	branch_8(MASK_OUT_ABOVE_8(m_ir));


}
void MC68000BASE::x6000_bra_w_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t offset = OPER_I_16();
	m_pc -= 2;
	trace_t0();              /* auto-disable (see m68kcpu.h) */
	branch_16(offset);


}
void MC68000BASE::x60ff_bra_l_234fc()
{
	uint32_t offset = OPER_I_16();
	offset <<= 16;
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	offset = OPER_I_16();
	m_pc -= 4;
	trace_t0();              /* auto-disable (see m68kcpu.h) */
	branch_32(offset);


}
void MC68000BASE::x01c0_bset_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;


}
void MC68000BASE::x01d0_bset_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01d8_bset_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01df_bset_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01e0_bset_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01e7_bset_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01e8_bset_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01f0_bset_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01f8_bset_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x01f9_bset_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08c0_bset_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;


}
void MC68000BASE::x08d0_bset_b_ai_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08d8_bset_b_pi_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08df_bset_b_pi7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08e0_bset_b_pd_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08e7_bset_b_pd7_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08e8_bset_b_di_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08f0_bset_b_ix_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08f8_bset_b_aw_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x08f9_bset_b_al_071234fc()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);

	m_not_z_flag = src & mask;
	write_8(ea, src | mask);


}
void MC68000BASE::x6100_bsr_b_071234fc()
{
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	branch_8(MASK_OUT_ABOVE_8(m_ir));


}
void MC68000BASE::x6100_bsr_w_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t offset = OPER_I_16();
	trace_t0();              /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	m_pc -= 2;
	branch_16(offset);


}
void MC68000BASE::x61ff_bsr_l_234fc()
{
	uint32_t offset = OPER_I_16();
	offset <<= 16;
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	offset = OPER_I_16();
	trace_t0();              /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	m_pc -= 4;
	branch_32(offset);


}
void MC68000BASE::x0100_btst_l_071234fc()
{
	m_not_z_flag = DY() & (1 << (DX() & 0x1f));


}
void MC68000BASE::x0110_btst_b_ai_071234fc()
{
	m_not_z_flag = OPER_AY_AI_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0118_btst_b_pi_071234fc()
{
	m_not_z_flag = OPER_AY_PI_8() & (1 << (DX() & 7));


}
void MC68000BASE::x011f_btst_b_pi7_071234fc()
{
	m_not_z_flag = OPER_A7_PI_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0120_btst_b_pd_071234fc()
{
	m_not_z_flag = OPER_AY_PD_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0127_btst_b_pd7_071234fc()
{
	m_not_z_flag = OPER_A7_PD_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0128_btst_b_di_071234fc()
{
	m_not_z_flag = OPER_AY_DI_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0130_btst_b_ix_071234fc()
{
	m_not_z_flag = OPER_AY_IX_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0138_btst_b_aw_071234fc()
{
	m_not_z_flag = OPER_AW_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0139_btst_b_al_071234fc()
{
	m_not_z_flag = OPER_AL_8() & (1 << (DX() & 7));


}
void MC68000BASE::x013a_btst_b_pcdi_071234fc()
{
	m_not_z_flag = OPER_PCDI_8() & (1 << (DX() & 7));


}
void MC68000BASE::x013b_btst_b_pcix_071234fc()
{
	m_not_z_flag = OPER_PCIX_8() & (1 << (DX() & 7));


}
void MC68000BASE::x013c_btst_b_i_071234fc()
{
	m_not_z_flag = OPER_I_8() & (1 << (DX() & 7));


}
void MC68000BASE::x0800_btst_l_071234fc()
{
	m_not_z_flag = DY() & (1 << (OPER_I_8() & 0x1f));


}
void MC68000BASE::x0810_btst_b_ai_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_AI_8() & (1 << bit);


}
void MC68000BASE::x0818_btst_b_pi_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PI_8() & (1 << bit);


}
void MC68000BASE::x081f_btst_b_pi7_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PI_8() & (1 << bit);


}
void MC68000BASE::x0820_btst_b_pd_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PD_8() & (1 << bit);


}
void MC68000BASE::x0827_btst_b_pd7_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PD_8() & (1 << bit);


}
void MC68000BASE::x0828_btst_b_di_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_DI_8() & (1 << bit);


}
void MC68000BASE::x0830_btst_b_ix_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_IX_8() & (1 << bit);


}
void MC68000BASE::x0838_btst_b_aw_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AW_8() & (1 << bit);


}
void MC68000BASE::x0839_btst_b_al_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AL_8() & (1 << bit);


}
void MC68000BASE::x083a_btst_b_pcdi_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCDI_8() & (1 << bit);


}
void MC68000BASE::x083b_btst_b_pcix_071234fc()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCIX_8() & (1 << bit);


}
void MC68000BASE::x06d0_callm_l_ai_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_AY_AI_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06e8_callm_l_di_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_AY_DI_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06f0_callm_l_ix_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_AY_IX_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06f8_callm_l_aw_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_AW_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06f9_callm_l_al_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_AL_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06fa_callm_l_pcdi_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_PCDI_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x06fb_callm_l_pcix_2f()
{
	/* note: watch out for pcrelative modes */
	uint32_t ea = EA_PCIX_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_pc += 2;
	(void)ea;   /* just to avoid an 'unused variable' warning */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (callm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x0ad0_cas_b_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_AI_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ad8_cas_b_pi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PI_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0adf_cas_b_pi7_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_A7_PI_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ae0_cas_b_pd_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PD_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ae7_cas_b_pd7_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_A7_PD_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ae8_cas_b_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_DI_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0af0_cas_b_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_IX_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0af8_cas_b_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AW_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0af9_cas_b_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AL_8();
	uint32_t dest = read_8(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(*compare, dest, res);
	m_c_flag = CFLAG_8(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_8(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0cd0_cas_w_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0cd8_cas_w_pi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ce0_cas_w_pd_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ce8_cas_w_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0cf0_cas_w_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0cf8_cas_w_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0cf9_cas_w_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t dest = read_16(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(*compare, dest, res);
	m_c_flag = CFLAG_16(res);

	if(COND_NE())
		*compare = MASK_OUT_BELOW_16(*compare) | dest;
	else {
		SET_ICOUNT(3);
		write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
	}


}
void MC68000BASE::x0ed0_cas_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_AI_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ed8_cas_l_pi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PI_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ee0_cas_l_pd_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_PD_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ee8_cas_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_DI_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ef0_cas_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AY_IX_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ef8_cas_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AW_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0ef9_cas_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint32_t ea = EA_AL_32();
	uint32_t dest = read_32(ea);
	uint32_t* compare = &REG_D()[word2 & 7];
	uint32_t res = dest - *compare;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(*compare, dest, res);
	m_c_flag = CFLAG_SUB_32(*compare, dest, res);

	if(COND_NE())
		*compare = dest;
	else {
		SET_ICOUNT(3);
		write_32(ea, REG_D()[(word2 >> 6) & 7]);
	}


}
void MC68000BASE::x0cfc_cas2_w_234fc()
{
	uint32_t word2 = OPER_I_32();
	uint32_t* compare1 = &REG_D()[(word2 >> 16) & 7];
	uint32_t ea1 = REG_DA()[(word2 >> 28) & 15];
	uint32_t dest1 = read_16(ea1);
	uint32_t res1 = dest1 - MASK_OUT_ABOVE_16(*compare1);
	uint32_t* compare2 = &REG_D()[word2 & 7];
	uint32_t ea2 = REG_DA()[(word2 >> 12) & 15];
	uint32_t dest2 = read_16(ea2);
	uint32_t res2;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_16(res1);
	m_not_z_flag = MASK_OUT_ABOVE_16(res1);
	m_v_flag = VFLAG_SUB_16(*compare1, dest1, res1);
	m_c_flag = CFLAG_16(res1);

	if(COND_EQ()) {
		res2 = dest2 - MASK_OUT_ABOVE_16(*compare2);

		m_n_flag = NFLAG_16(res2);
		m_not_z_flag = MASK_OUT_ABOVE_16(res2);
		m_v_flag = VFLAG_SUB_16(*compare2, dest2, res2);
		m_c_flag = CFLAG_16(res2);

		if(COND_EQ()) {
			SET_ICOUNT(3);
			write_16(ea1, REG_D()[(word2 >> 22) & 7]);
			write_16(ea2, REG_D()[(word2 >> 6) & 7]);
			goto done;
		}
	}
	*compare1 = BIT_1F(word2) ? MAKE_INT_16(dest1) : MASK_OUT_BELOW_16(*compare1) | dest1;
	*compare2 = BIT_F(word2) ? MAKE_INT_16(dest2) : MASK_OUT_BELOW_16(*compare2) | dest2;
	done: ;


}
void MC68000BASE::x0efc_cas2_l_234fc()
{
	uint32_t word2 = OPER_I_32();
	uint32_t* compare1 = &REG_D()[(word2 >> 16) & 7];
	uint32_t ea1 = REG_DA()[(word2 >> 28) & 15];
	uint32_t dest1 = read_32(ea1);
	uint32_t res1 = dest1 - *compare1;
	uint32_t* compare2 = &REG_D()[word2 & 7];
	uint32_t ea2 = REG_DA()[(word2 >> 12) & 15];
	uint32_t dest2 = read_32(ea2);
	uint32_t res2;

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	m_n_flag = NFLAG_32(res1);
	m_not_z_flag = MASK_OUT_ABOVE_32(res1);
	m_v_flag = VFLAG_SUB_32(*compare1, dest1, res1);
	m_c_flag = CFLAG_SUB_32(*compare1, dest1, res1);

	if(COND_EQ()) {
		res2 = dest2 - *compare2;

		m_n_flag = NFLAG_32(res2);
		m_not_z_flag = MASK_OUT_ABOVE_32(res2);
		m_v_flag = VFLAG_SUB_32(*compare2, dest2, res2);
		m_c_flag = CFLAG_SUB_32(*compare2, dest2, res2);

		if(COND_EQ()) {
			SET_ICOUNT(3);
			write_32(ea1, REG_D()[(word2 >> 22) & 7]);
			write_32(ea2, REG_D()[(word2 >> 6) & 7]);
			goto done;
		}
	}
	*compare1 = dest1;
	*compare2 = dest2;
	done: ;


}
void MC68000BASE::x4180_chk_w_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(DY());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound)
	{
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4190_chk_w_ai_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_AI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4198_chk_w_pi_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_PI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41a0_chk_w_pd_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_PD_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41a8_chk_w_di_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_DI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41b0_chk_w_ix_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_IX_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41b8_chk_w_aw_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AW_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41b9_chk_w_al_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AL_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41ba_chk_w_pcdi_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_PCDI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41bb_chk_w_pcix_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_PCIX_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x41bc_chk_w_i_071234fc()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_I_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4100_chk_l_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(DY());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4110_chk_l_ai_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AY_AI_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4118_chk_l_pi_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AY_PI_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4120_chk_l_pd_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AY_PD_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4128_chk_l_di_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AY_DI_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4130_chk_l_ix_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AY_IX_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4138_chk_l_aw_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AW_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x4139_chk_l_al_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_AL_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x413a_chk_l_pcdi_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_PCDI_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x413b_chk_l_pcix_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_PCIX_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x413c_chk_l_i_234fc()
{
	int32_t src = MAKE_INT_32(DX());
	int32_t bound = MAKE_INT_32(OPER_I_32());

	m_not_z_flag = ZFLAG_32(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src < 0 || src > bound) {
		m_n_flag = (src < 0)<<7;
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);
	}


}
void MC68000BASE::x00fa_chk2cmp2_b_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_PCDI_8();
	int32_t lower_bound = read_naerr_8(ea);
	int32_t upper_bound = read_naerr_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00fb_chk2cmp2_b_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_PCIX_8();
	int32_t lower_bound = read_naerr_8(ea);
	int32_t upper_bound = read_naerr_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00d0_chk2cmp2_b_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_AY_AI_8();
	int32_t lower_bound = read_8(ea);
	int32_t upper_bound = read_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00e8_chk2cmp2_b_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_AY_DI_8();
	int32_t lower_bound = read_8(ea);
	int32_t upper_bound = read_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00f0_chk2cmp2_b_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_AY_IX_8();
	int32_t lower_bound = read_8(ea);
	int32_t upper_bound = read_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00f8_chk2cmp2_b_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_AW_8();
	int32_t lower_bound = read_8(ea);
	int32_t upper_bound = read_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x00f9_chk2cmp2_b_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xff;

	uint32_t ea = EA_AL_8();
	int32_t lower_bound = read_8(ea);
	int32_t upper_bound = read_8(ea + 1);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80) {
		lower_bound = (int32_t)(int8_t)lower_bound;
		upper_bound = (int32_t)(int8_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int8_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02fa_chk2cmp2_w_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_PCDI_16();
	int32_t lower_bound = read_naerr_16(ea);
	int32_t upper_bound = read_naerr_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02fb_chk2cmp2_w_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_PCIX_16();
	int32_t lower_bound = read_naerr_16(ea);
	int32_t upper_bound = read_naerr_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02d0_chk2cmp2_w_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_AY_AI_16();
	int32_t lower_bound = read_16(ea);
	int32_t upper_bound = read_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02e8_chk2cmp2_w_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_AY_DI_16();
	int32_t lower_bound = read_16(ea);
	int32_t upper_bound = read_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02f0_chk2cmp2_w_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_AY_IX_16();
	int32_t lower_bound = read_16(ea);
	int32_t upper_bound = read_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02f8_chk2cmp2_w_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_AW_16();
	int32_t lower_bound = read_16(ea);
	int32_t upper_bound = read_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x02f9_chk2cmp2_w_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
	if(!BIT_F(word2))
		compare &= 0xffff;

	uint32_t ea = EA_AL_16();
	int32_t lower_bound = read_16(ea);
	int32_t upper_bound = read_16(ea + 2);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x8000) {
		lower_bound = (int32_t)(int16_t)lower_bound;
		upper_bound = (int32_t)(int16_t)upper_bound;

		if(!BIT_F(word2))
			compare = (int32_t)(int16_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04fa_chk2cmp2_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_PCDI_32();
	int64_t lower_bound = read_naerr_32(ea);
	int64_t upper_bound = read_naerr_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04fb_chk2cmp2_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_PCIX_32();
	int64_t lower_bound = read_naerr_32(ea);
	int64_t upper_bound = read_naerr_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04d0_chk2cmp2_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_AY_AI_32();
	int64_t lower_bound = read_32(ea);
	int64_t upper_bound = read_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04e8_chk2cmp2_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_AY_DI_32();
	int64_t lower_bound = read_32(ea);
	int64_t upper_bound = read_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04f0_chk2cmp2_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_AY_IX_32();
	int64_t lower_bound = read_32(ea);
	int64_t upper_bound = read_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04f8_chk2cmp2_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_AW_32();
	int64_t lower_bound = read_32(ea);
	int64_t upper_bound = read_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x04f9_chk2cmp2_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	int64_t compare = REG_DA()[(word2 >> 12) & 15];
	uint32_t ea = EA_AL_32();
	int64_t lower_bound = read_32(ea);
	int64_t upper_bound = read_32(ea + 4);

	// for signed compare, the arithmetically smaller value is the lower bound
	if (lower_bound & 0x80000000) {
		lower_bound = (int64_t)(int32_t)lower_bound;
		upper_bound = (int64_t)(int32_t)upper_bound;
		compare = (int64_t)(int32_t)compare;
	}

	m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
	m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

	if(COND_CS() && BIT_B(word2))
		m_proc_mode = (PROC_MODE_EXCEPTION_CHK);


}
void MC68000BASE::x4200_clr_b_071234fc()
{
	DY() &= 0xffffff00;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4210_clr_b_ai_0()
{
	uint32_t ea = EA_AY_AI_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4218_clr_b_pi_0()
{
	uint32_t ea = EA_AY_PI_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x421f_clr_b_pi7_0()
{
	uint32_t ea = EA_A7_PI_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4220_clr_b_pd_0()
{
	uint32_t ea = EA_AY_PD_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4227_clr_b_pd7_0()
{
	uint32_t ea = EA_A7_PD_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4228_clr_b_di_0()
{
	uint32_t ea = EA_AY_DI_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4230_clr_b_ix_0()
{
	uint32_t ea = EA_AY_IX_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4238_clr_b_aw_0()
{
	uint32_t ea = EA_AW_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4239_clr_b_al_0()
{
	uint32_t ea = EA_AL_8();

	read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4210_clr_b_ai_71234fc()
{
	uint32_t ea = EA_AY_AI_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4218_clr_b_pi_71234fc()
{
	uint32_t ea = EA_AY_PI_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x421f_clr_b_pi7_71234fc()
{
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4220_clr_b_pd_71234fc()
{
	uint32_t ea = EA_AY_PD_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4227_clr_b_pd7_71234fc()
{
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4228_clr_b_di_71234fc()
{
	uint32_t ea = EA_AY_DI_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4230_clr_b_ix_71234fc()
{
	uint32_t ea = EA_AY_IX_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4238_clr_b_aw_71234fc()
{
	uint32_t ea = EA_AW_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4239_clr_b_al_71234fc()
{
	uint32_t ea = EA_AL_8();

	write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4240_clr_w_071234fc()
{
	DY() &= 0xffff0000;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4250_clr_w_ai_0()
{
	uint32_t ea = EA_AY_AI_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4258_clr_w_pi_0()
{
	uint32_t ea = EA_AY_PI_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4260_clr_w_pd_0()
{
	uint32_t ea = EA_AY_PD_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4268_clr_w_di_0()
{
	uint32_t ea = EA_AY_DI_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4270_clr_w_ix_0()
{
	uint32_t ea = EA_AY_IX_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4278_clr_w_aw_0()
{
	uint32_t ea = EA_AW_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4279_clr_w_al_0()
{
	uint32_t ea = EA_AL_16();

	read_16(ea);  /* the 68000 does a dummy read, the value is discarded */

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4250_clr_w_ai_71234fc()
{
	uint32_t ea = EA_AY_AI_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4258_clr_w_pi_71234fc()
{
	uint32_t ea = EA_AY_PI_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4260_clr_w_pd_71234fc()
{
	uint32_t ea = EA_AY_PD_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4268_clr_w_di_71234fc()
{
	uint32_t ea = EA_AY_DI_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4270_clr_w_ix_71234fc()
{
	uint32_t ea = EA_AY_IX_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4278_clr_w_aw_71234fc()
{
	uint32_t ea = EA_AW_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4279_clr_w_al_71234fc()
{
	uint32_t ea = EA_AL_16();

	write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4280_clr_l_071234fc()
{
	DY() = 0;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4290_clr_l_ai_0()
{
	uint32_t ea = EA_AY_AI_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4298_clr_l_pi_0()
{
	uint32_t ea = EA_AY_PI_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42a0_clr_l_pd_0()
{
	uint32_t ea = EA_AY_PD_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42a8_clr_l_di_0()
{
	uint32_t ea = EA_AY_DI_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b0_clr_l_ix_0()
{
	uint32_t ea = EA_AY_IX_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b8_clr_l_aw_0()
{
	uint32_t ea = EA_AW_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b9_clr_l_al_0()
{
	uint32_t ea = EA_AL_32();

	read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4290_clr_l_ai_71234fc()
{
	uint32_t ea = EA_AY_AI_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x4298_clr_l_pi_71234fc()
{
	uint32_t ea = EA_AY_PI_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42a0_clr_l_pd_71234fc()
{
	uint32_t ea = EA_AY_PD_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42a8_clr_l_di_71234fc()
{
	uint32_t ea = EA_AY_DI_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b0_clr_l_ix_71234fc()
{
	uint32_t ea = EA_AY_IX_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b8_clr_l_aw_71234fc()
{
	uint32_t ea = EA_AW_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::x42b9_clr_l_al_71234fc()
{
	uint32_t ea = EA_AL_32();

	write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void MC68000BASE::xb000_cmp_b_071234fc()
{
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb010_cmp_b_ai_071234fc()
{
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb018_cmp_b_pi_071234fc()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb01f_cmp_b_pi7_071234fc()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb020_cmp_b_pd_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb027_cmp_b_pd7_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb028_cmp_b_di_071234fc()
{
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb030_cmp_b_ix_071234fc()
{
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb038_cmp_b_aw_071234fc()
{
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb039_cmp_b_al_071234fc()
{
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb03a_cmp_b_pcdi_071234fc()
{
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb03b_cmp_b_pcix_071234fc()
{
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb03c_cmp_b_i_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb040_cmp_w_071234fc()
{
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb048_cmp_w_071234fc()
{
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb050_cmp_w_ai_071234fc()
{
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb058_cmp_w_pi_071234fc()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb060_cmp_w_pd_071234fc()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb068_cmp_w_di_071234fc()
{
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb070_cmp_w_ix_071234fc()
{
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb078_cmp_w_aw_071234fc()
{
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb079_cmp_w_al_071234fc()
{
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb07a_cmp_w_pcdi_071234fc()
{
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb07b_cmp_w_pcix_071234fc()
{
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb07c_cmp_w_i_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb080_cmp_l_071234fc()
{
	uint32_t src = DY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb088_cmp_l_071234fc()
{
	uint32_t src = AY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb090_cmp_l_ai_071234fc()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb098_cmp_l_pi_071234fc()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0a0_cmp_l_pd_071234fc()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0a8_cmp_l_di_071234fc()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0b0_cmp_l_ix_071234fc()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0b8_cmp_l_aw_071234fc()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0b9_cmp_l_al_071234fc()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0ba_cmp_l_pcdi_071234fc()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0bb_cmp_l_pcix_071234fc()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0bc_cmp_l_i_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0c0_cmpa_w_071234fc()
{
	uint32_t src = MAKE_INT_16(DY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0c8_cmpa_w_071234fc()
{
	uint32_t src = MAKE_INT_16(AY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0d0_cmpa_w_ai_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0d8_cmpa_w_pi_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0e0_cmpa_w_pd_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0e8_cmpa_w_di_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0f0_cmpa_w_ix_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0f8_cmpa_w_aw_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AW_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0f9_cmpa_w_al_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_AL_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0fa_cmpa_w_pcdi_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0fb_cmpa_w_pcix_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb0fc_cmpa_w_i_071234fc()
{
	uint32_t src = MAKE_INT_16(OPER_I_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1c0_cmpa_l_071234fc()
{
	uint32_t src = DY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1c8_cmpa_l_071234fc()
{
	uint32_t src = AY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1d0_cmpa_l_ai_071234fc()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1d8_cmpa_l_pi_071234fc()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1e0_cmpa_l_pd_071234fc()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1e8_cmpa_l_di_071234fc()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1f0_cmpa_l_ix_071234fc()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1f8_cmpa_l_aw_071234fc()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1f9_cmpa_l_al_071234fc()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1fa_cmpa_l_pcdi_071234fc()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1fb_cmpa_l_pcix_071234fc()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xb1fc_cmpa_l_i_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0c00_cmpi_b_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c10_cmpi_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_AI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c18_cmpi_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c1f_cmpi_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c20_cmpi_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c27_cmpi_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c28_cmpi_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_DI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c30_cmpi_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_IX_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c38_cmpi_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AW_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c39_cmpi_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AL_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c3a_cmpi_b_234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_PCDI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c3b_cmpi_b_234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_PCIX_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::x0c40_cmpi_w_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c50_cmpi_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_AI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c58_cmpi_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c60_cmpi_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PD_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c68_cmpi_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_DI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c70_cmpi_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_IX_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c78_cmpi_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AW_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c79_cmpi_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AL_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c7a_cmpi_w_234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_PCDI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c7b_cmpi_w_234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_PCIX_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::x0c80_cmpi_l_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = DY();
	uint32_t res = dst - src;

//	if (!m_cmpild_instr_callback.isnull())
//		(m_cmpild_instr_callback)(m_ir & 7, src);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0c90_cmpi_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_AI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0c98_cmpi_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0ca0_cmpi_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PD_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0ca8_cmpi_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_DI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0cb0_cmpi_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_IX_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0cb8_cmpi_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AW_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0cb9_cmpi_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AL_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0cba_cmpi_l_234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_PCDI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::x0cbb_cmpi_l_234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_PCIX_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xbf08_cmpm_b_071234fc()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb10f_cmpm_b_071234fc()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xbf0f_cmpm_b_071234fc()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb108_cmpm_b_071234fc()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void MC68000BASE::xb148_cmpm_w_071234fc()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = OPER_AX_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void MC68000BASE::xb188_cmpm_l_071234fc()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = OPER_AX_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void MC68000BASE::xf080_cpbcc_l_23()
{
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cpbcc)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::xf048_cpdbcc_l_23()
{
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cpdbcc)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::xf000_cpgen_l_23()
{
	bool has_fpu_or_pmmu = false;
#if defined(USE_MC68000FPU)
	has_fpu_or_pmmu |= m_has_fpu;
#endif
#if defined(USE_MC68000MMU)
	has_fpu_or_pmmu |= m_has_pmmu;
#endif
	if(has_fpu_or_pmmu)
	{
		logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cpgen)\n",
						get_class_name(), m_ppc, m_ir);
	} else
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xf040_cpscc_l_23()
{
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cpscc)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::xf078_cptrapcc_l_23()
{
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cptrapcc)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::xf278_ftrapcc_l_23()
{
#if defined(USE_MC68000FPU)
	if(m_has_fpu)
	{
		m68881_ftrap();
	} else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}

}
void MC68000BASE::x50c8_dbt_w_071234fc()
{
	m_pc += 2;


}
void MC68000BASE::x51c8_dbf_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
	uint32_t offset;
	if(res != 0xffff) {
		m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
		offset = OPER_I_16();
		m_pc -= 2;
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset);
		SET_ICOUNT(m_cyc_dbcc_f_noexp);
	} else {
		offset = OPER_I_16();
//		m_pc += 2;
		non_branch_16(offset);
		SET_ICOUNT(m_cyc_dbcc_f_exp);
	}


}
void MC68000BASE::x52c8_dbhi_w_071234fc()
{
	if(COND_NOT_HI()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x53c8_dbls_w_071234fc()
{
	if(COND_NOT_LS()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x54c8_dbcc_w_071234fc()
{
	if(COND_NOT_CC()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x55c8_dbcs_w_071234fc()
{
	if(COND_NOT_CS()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x56c8_dbne_w_071234fc()
{
	if(COND_NOT_NE()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x57c8_dbeq_w_071234fc()
{
	if(COND_NOT_EQ()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x58c8_dbvc_w_071234fc()
{
	if(COND_NOT_VC()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x59c8_dbvs_w_071234fc()
{
	if(COND_NOT_VS()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5ac8_dbpl_w_071234fc()
{
	if(COND_NOT_PL()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5bc8_dbmi_w_071234fc()
{
	if(COND_NOT_MI()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5cc8_dbge_w_071234fc()
{
	if(COND_NOT_GE()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5dc8_dblt_w_071234fc()
{
	if(COND_NOT_LT()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5ec8_dbgt_w_071234fc()
{
	if(COND_NOT_GT()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5fc8_dble_w_071234fc()
{
	if(COND_NOT_LE()) {
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		uint32_t offset;
		if(res != 0xffff) {
			m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
			offset = OPER_I_16();
			m_pc -= 2;
			trace_t0();              /* auto-disable (see m68kcpu.h) */
			branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_noexp);
		} else {
			offset = OPER_I_16();
//			m_pc += 2;
			non_branch_16(offset);
			SET_ICOUNT(m_cyc_dbcc_f_exp);
		}
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x81c0_divs_w_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(DY());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}

}
void MC68000BASE::x81d0_divs_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_AI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81d8_divs_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_PI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81e0_divs_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_PD_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81e8_divs_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_DI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81f0_divs_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_IX_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81f8_divs_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AW_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81f9_divs_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AL_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81fa_divs_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_PCDI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81fb_divs_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_PCIX_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x81fc_divs_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_I_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1) {
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
		} else {
			quotient = MAKE_INT_32(*r_dst) / src;
			remainder = MAKE_INT_32(*r_dst) % src;

			if(quotient == MAKE_INT_16(quotient)) {
				m_not_z_flag = quotient;
				m_n_flag = NFLAG_16(quotient);
				m_v_flag = VFLAG_CLEAR;
				*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			} else
				m_v_flag = VFLAG_SET;
		}
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80c0_divu_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80d0_divu_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80d8_divu_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80e0_divu_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80e8_divu_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80f0_divu_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80f8_divu_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80f9_divu_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80fa_divu_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80fb_divu_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x80fc_divu_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();

	if(src != 0) {
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000) {
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
		} else
			m_v_flag = VFLAG_SET;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c40_divl_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor   = DY();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}

		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c50_divl_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AY_AI_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c58_divl_l_pi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AY_PI_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c60_divl_l_pd_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AY_PD_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c68_divl_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AY_DI_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c70_divl_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AY_IX_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c78_divl_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AW_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c79_divl_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_AL_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c7a_divl_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_PCDI_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c7b_divl_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_PCIX_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::x4c7c_divl_l_i_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t divisor = OPER_I_32();
	uint64_t dividend  = 0;
	uint64_t quotient  = 0;
	uint64_t remainder = 0;

	if(divisor != 0) {
		if(BIT_A(word2)) {   /* 64 bit */
			dividend = REG_D()[word2 & 7];
			dividend <<= 32;
			dividend |= REG_D()[(word2 >> 12) & 7];

			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
				if((int64_t)quotient != (int64_t)((int32_t)quotient)) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				if(quotient > 0xffffffff) {
					m_v_flag = VFLAG_SET;
					goto done;
				}
				remainder = dividend % divisor;
			}
		} else {    /* 32 bit */
			dividend = REG_D()[(word2 >> 12) & 7];
			if(BIT_B(word2)) {       /* signed */
				quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
				remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
			} else {                 /* unsigned */
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
		}
		REG_D()[word2 & 7] = MASK_OUT_ABOVE_32(remainder);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(quotient);

		m_n_flag = NFLAG_32(quotient);
		m_not_z_flag = MASK_OUT_ABOVE_32(quotient);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		done: ;
	} else {
		RESET_ICOUNT(m_cyc_instruction[m_ir]);
		m_proc_mode = (PROC_MODE_EXCEPTION_ZERO_DIVIDE);
	}


}
void MC68000BASE::xb100_eor_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= MASK_OUT_ABOVE_8(DX()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb110_eor_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb118_eor_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb11f_eor_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb120_eor_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb127_eor_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb128_eor_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb130_eor_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb138_eor_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb139_eor_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb140_eor_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= MASK_OUT_ABOVE_16(DX()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb150_eor_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb158_eor_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb160_eor_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb168_eor_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb170_eor_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb178_eor_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb179_eor_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb180_eor_l_071234fc()
{
	uint32_t res = DY() ^= DX();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb190_eor_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb198_eor_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb1a0_eor_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb1a8_eor_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb1b0_eor_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb1b8_eor_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xb1b9_eor_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a00_eori_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= OPER_I_8());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a10_eori_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a18_eori_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a1f_eori_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a20_eori_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a27_eori_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a28_eori_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a30_eori_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a38_eori_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a39_eori_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = src ^ read_8(ea);

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a40_eori_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a50_eori_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a58_eori_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a60_eori_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a68_eori_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a70_eori_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a78_eori_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a79_eori_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = src ^ read_16(ea);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a80_eori_l_071234fc()
{
	uint32_t res = DY() ^= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a90_eori_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a98_eori_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0aa0_eori_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0aa8_eori_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0ab0_eori_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0ab8_eori_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0ab9_eori_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src ^ read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0a3c_eori_w_071234fc()
{
	uint32_t src = OPER_I_8();
	if (CPU_TYPE_IS_000()) {
		/* dummy read */
		read_dummy_imm_16();
	}
	set_ccr(get_ccr() ^ src);


}
void MC68000BASE::x0a7c_eori_w_071234fc()
{
	if(m_s_flag) {
		uint32_t src = OPER_I_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if (CPU_TYPE_IS_000()) {
			/* dummy read */
			read_dummy_imm_16();
		}
		set_sr(get_sr() ^ src);
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::xc140_exg_l_071234fc()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &DY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void MC68000BASE::xc148_exg_l_071234fc()
{
	uint32_t* reg_a = &AX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void MC68000BASE::xc188_exg_l_071234fc()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void MC68000BASE::x4880_ext_w_071234fc()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

	m_n_flag = NFLAG_16(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x48c0_ext_l_071234fc()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x49c0_extb_l_234fc()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xffffff00 : 0);

	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4afc_illegal_071234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);


}
void MC68000BASE::x4ed0_jmp_l_ai_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_AY_AI_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4ee8_jmp_l_di_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_AY_DI_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4ef0_jmp_l_ix_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_AY_IX_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4ef8_jmp_l_aw_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_AW_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4ef9_jmp_l_al_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_AL_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4efa_jmp_l_pcdi_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_PCDI_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4efb_jmp_l_pcix_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	jump(EA_PCIX_32());
	m_pref_flags &= ~PREF_IGNORE;
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4e90_jsr_l_ai_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_AY_AI_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4ea8_jsr_l_di_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_AY_DI_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4eb0_jsr_l_ix_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_AY_IX_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4eb8_jsr_l_aw_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_AW_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4eb9_jsr_l_al_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_AL_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4eba_jsr_l_pcdi_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_PCDI_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x4ebb_jsr_l_pcix_071234fc()
{
	m_pref_flags |= PREF_IGNORE; /* no update prefetch queue */
	uint32_t ea = EA_PCIX_32();
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	push_32(m_pc);
	jump(ea);
	m_pref_flags &= ~PREF_IGNORE;


}
void MC68000BASE::x41d0_lea_l_ai_071234fc()
{
	AX() = EA_AY_AI_32();


}
void MC68000BASE::x41e8_lea_l_di_071234fc()
{
	AX() = EA_AY_DI_32();


}
void MC68000BASE::x41f0_lea_l_ix_071234fc()
{
	AX() = EA_AY_IX_32();


}
void MC68000BASE::x41f8_lea_l_aw_071234fc()
{
	AX() = EA_AW_32();


}
void MC68000BASE::x41f9_lea_l_al_071234fc()
{
	AX() = EA_AL_32();


}
void MC68000BASE::x41fa_lea_l_pcdi_071234fc()
{
	AX() = EA_PCDI_32();


}
void MC68000BASE::x41fb_lea_l_pcix_071234fc()
{
	AX() = EA_PCIX_32();


}
void MC68000BASE::x4e57_link_w_071234fc()
{
	REG_SP() -= 4;
	write_32(REG_SP(), REG_SP());
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + MAKE_INT_16(OPER_I_16())));


}
void MC68000BASE::x4e50_link_w_071234fc()
{
	uint32_t* r_dst = &AY();

	push_32(*r_dst);
	*r_dst = REG_SP();
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + MAKE_INT_16(OPER_I_16())));


}
void MC68000BASE::x480f_link_l_234fc()
{
	REG_SP() -= 4;
	write_32(REG_SP(), REG_SP());
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + OPER_I_32()));


}
void MC68000BASE::x4808_link_l_234fc()
{
	uint32_t* r_dst = &AY();

	push_32(*r_dst);
	*r_dst = REG_SP();
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + OPER_I_32()));


}
void MC68000BASE::xe008_lsr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe048_lsr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe088_lsr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe028_lsr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift <= 8) {
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << (9-shift);
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;

		} else {
			*r_dst &= 0xffffff00;
			m_x_flag = XFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe068_lsr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift <= 16) {
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			*r_dst &= 0xffff0000;
			m_x_flag = XFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}

}
void MC68000BASE::xe0a8_lsr_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 32) {
			*r_dst = res;
			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			*r_dst = 0;
			m_x_flag = m_c_flag = (shift == 32 ? GET_MSB_32(src)>>23 : 0);
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe2d0_lsr_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2d8_lsr_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2e0_lsr_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2e8_lsr_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2f0_lsr_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2f8_lsr_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe2f9_lsr_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = src >> 1;

	write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe108_lsl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << shift;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe148_lsl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe188_lsl_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe128_lsl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift <= 8) {
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			*r_dst &= 0xffffff00;
			m_x_flag = XFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe168_lsl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift <= 16) {
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_x_flag = m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			*r_dst &= 0xffff0000;
			m_x_flag = XFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe1a8_lsl_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0) {
		SET_ICOUNT(shift * m_cyc_shift);

		if(shift < 32) {
			*r_dst = res;
			m_x_flag = m_c_flag = (src >> (32 - shift)) << 8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			*r_dst = 0;
			m_x_flag = m_c_flag = ((shift == 32 ? src & 1 : 0))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = ZFLAG_SET;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe3d0_lsl_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3d8_lsl_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3e0_lsl_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3e8_lsl_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3f0_lsl_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3f8_lsl_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe3f9_lsl_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x1000_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1010_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1018_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x101f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1020_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1027_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1028_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1030_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1038_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1039_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x103a_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x103b_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x103c_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1080_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1090_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1098_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x109f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10a0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10a7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10a8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10b0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10b8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10b9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10ba_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10bb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10bc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_AI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ec0_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10c0_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ed0_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ed8_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1edf_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ee0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ee7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ee8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ef0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ef8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1ef9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1efa_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1efb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1efc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10d0_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10d8_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10df_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10e0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10e7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10e8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10f0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10f8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10f9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10fa_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10fb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x10fc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f00_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1100_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f10_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f18_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f1f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f20_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f27_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f28_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f30_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f38_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f39_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f3a_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f3b_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1f3c_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1110_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1118_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x111f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1120_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1127_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1128_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1130_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1138_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1139_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x113a_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x113b_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x113c_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PD_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1140_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1150_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1158_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x115f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1160_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1167_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1168_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1170_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1178_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1179_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x117a_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x117b_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x117c_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_DI_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1180_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1190_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x1198_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x119f_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11a0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11a7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11a8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11b0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11b8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11b9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11ba_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11bb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11bc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_IX_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11c0_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11d0_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11d8_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11df_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11e0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11e7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11e8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11f0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11f8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11f9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11fa_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11fb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x11fc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AW_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13c0_move_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13d0_move_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13d8_move_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13df_move_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13e0_move_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13e7_move_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13e8_move_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13f0_move_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13f8_move_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13f9_move_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13fa_move_b_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13fb_move_b_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x13fc_move_b_i_071234fc()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AL_8();

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3000_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3008_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3010_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3018_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3020_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3028_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3030_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3038_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3039_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x303a_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x303b_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x303c_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3080_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3088_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3090_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3098_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30a0_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30a8_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30b0_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30b8_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30b9_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30ba_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30bb_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30bc_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_AI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30c0_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30c8_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30d0_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30d8_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30e0_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30e8_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30f0_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30f8_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30f9_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30fa_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30fb_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x30fc_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3100_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3108_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3110_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3118_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3120_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3128_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3130_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3138_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3139_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x313a_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x313b_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x313c_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PD_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3140_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3148_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3150_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3158_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3160_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3168_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3170_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3178_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3179_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x317a_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x317b_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x317c_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_DI_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3180_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3188_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3190_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3198_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31a0_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31a8_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31b0_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31b8_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31b9_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31ba_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31bb_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31bc_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_IX_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31c0_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31c8_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31d0_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31d8_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31e0_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31e8_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31f0_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31f8_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31f9_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31fa_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31fb_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x31fc_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AW_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33c0_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33c8_move_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33d0_move_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33d8_move_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33e0_move_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33e8_move_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33f0_move_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33f8_move_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33f9_move_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33fa_move_w_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33fb_move_w_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x33fc_move_w_i_071234fc()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AL_16();

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2000_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2008_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2010_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2018_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2020_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2028_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2030_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2038_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2039_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x203a_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x203b_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x203c_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2080_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2088_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2090_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2098_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20a0_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20a8_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20b0_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20b8_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20b9_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20ba_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20bb_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20bc_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_AI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20c0_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20c8_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20d0_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20d8_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20e0_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20e8_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20f0_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20f8_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20f9_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20fa_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20fb_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x20fc_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_PI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2100_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2108_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2110_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2118_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2120_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2128_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2130_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2138_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2139_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x213a_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x213b_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x213c_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_PD_32();

	write_16(ea+2, res & 0xFFFF );
	write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2140_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2148_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2150_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2158_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2160_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2168_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2170_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2178_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2179_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x217a_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x217b_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x217c_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_DI_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2180_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2188_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2190_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x2198_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21a0_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21a8_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21b0_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21b8_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21b9_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21ba_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21bb_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21bc_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_IX_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21c0_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21c8_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21d0_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21d8_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21e0_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21e8_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21f0_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21f8_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21f9_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21fa_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21fb_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x21fc_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AW_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23c0_move_l_071234fc()
{
	uint32_t res = DY();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23c8_move_l_071234fc()
{
	uint32_t res = AY();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23d0_move_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23d8_move_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23e0_move_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23e8_move_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23f0_move_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23f8_move_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23f9_move_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23fa_move_l_pcdi_071234fc()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23fb_move_l_pcix_071234fc()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x23fc_move_l_i_071234fc()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AL_32();

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x3040_movea_w_071234fc()
{
	AX() = MAKE_INT_16(DY());


}
void MC68000BASE::x3048_movea_w_071234fc()
{
	AX() = MAKE_INT_16(AY());


}
void MC68000BASE::x3050_movea_w_ai_071234fc()
{
	AX() = MAKE_INT_16(OPER_AY_AI_16());


}
void MC68000BASE::x3058_movea_w_pi_071234fc()
{
	AX() = MAKE_INT_16(OPER_AY_PI_16());


}
void MC68000BASE::x3060_movea_w_pd_071234fc()
{
	AX() = MAKE_INT_16(OPER_AY_PD_16());


}
void MC68000BASE::x3068_movea_w_di_071234fc()
{
	AX() = MAKE_INT_16(OPER_AY_DI_16());


}
void MC68000BASE::x3070_movea_w_ix_071234fc()
{
	AX() = MAKE_INT_16(OPER_AY_IX_16());


}
void MC68000BASE::x3078_movea_w_aw_071234fc()
{
	AX() = MAKE_INT_16(OPER_AW_16());


}
void MC68000BASE::x3079_movea_w_al_071234fc()
{
	AX() = MAKE_INT_16(OPER_AL_16());


}
void MC68000BASE::x307a_movea_w_pcdi_071234fc()
{
	AX() = MAKE_INT_16(OPER_PCDI_16());


}
void MC68000BASE::x307b_movea_w_pcix_071234fc()
{
	AX() = MAKE_INT_16(OPER_PCIX_16());


}
void MC68000BASE::x307c_movea_w_i_071234fc()
{
	AX() = MAKE_INT_16(OPER_I_16());


}
void MC68000BASE::x2040_movea_l_071234fc()
{
	AX() = DY();


}
void MC68000BASE::x2048_movea_l_071234fc()
{
	AX() = AY();


}
void MC68000BASE::x2050_movea_l_ai_071234fc()
{
	AX() = OPER_AY_AI_32();


}
void MC68000BASE::x2058_movea_l_pi_071234fc()
{
	AX() = OPER_AY_PI_32();


}
void MC68000BASE::x2060_movea_l_pd_071234fc()
{
	AX() = OPER_AY_PD_32();


}
void MC68000BASE::x2068_movea_l_di_071234fc()
{
	AX() = OPER_AY_DI_32();


}
void MC68000BASE::x2070_movea_l_ix_071234fc()
{
	AX() = OPER_AY_IX_32();


}
void MC68000BASE::x2078_movea_l_aw_071234fc()
{
	AX() = OPER_AW_32();


}
void MC68000BASE::x2079_movea_l_al_071234fc()
{
	AX() = OPER_AL_32();


}
void MC68000BASE::x207a_movea_l_pcdi_071234fc()
{
	AX() = OPER_PCDI_32();


}
void MC68000BASE::x207b_movea_l_pcix_071234fc()
{
	AX() = OPER_PCIX_32();


}
void MC68000BASE::x207c_movea_l_i_071234fc()
{
	AX() = OPER_I_32();


}
void MC68000BASE::x42c0_move_w_1234fc()
{
	DY() = MASK_OUT_BELOW_16(DY()) | get_ccr();


}
void MC68000BASE::x42d0_move_w_ai_1234fc()
{
	write_16(EA_AY_AI_16(), get_ccr());


}
void MC68000BASE::x42d8_move_w_pi_1234fc()
{
	write_16(EA_AY_PI_16(), get_ccr());


}
void MC68000BASE::x42e0_move_w_pd_1234fc()
{
	write_16(EA_AY_PD_16(), get_ccr());


}
void MC68000BASE::x42e8_move_w_di_1234fc()
{
	write_16(EA_AY_DI_16(), get_ccr());


}
void MC68000BASE::x42f0_move_w_ix_1234fc()
{
	write_16(EA_AY_IX_16(), get_ccr());


}
void MC68000BASE::x42f8_move_w_aw_1234fc()
{
	write_16(EA_AW_16(), get_ccr());


}
void MC68000BASE::x42f9_move_w_al_1234fc()
{
	write_16(EA_AL_16(), get_ccr());


}
void MC68000BASE::x44c0_move_w_071234fc()
{
	set_ccr(DY());


}
void MC68000BASE::x44d0_move_w_ai_071234fc()
{
	set_ccr(OPER_AY_AI_16());


}
void MC68000BASE::x44d8_move_w_pi_071234fc()
{
	set_ccr(OPER_AY_PI_16());


}
void MC68000BASE::x44e0_move_w_pd_071234fc()
{
	set_ccr(OPER_AY_PD_16());


}
void MC68000BASE::x44e8_move_w_di_071234fc()
{
	set_ccr(OPER_AY_DI_16());


}
void MC68000BASE::x44f0_move_w_ix_071234fc()
{
	set_ccr(OPER_AY_IX_16());


}
void MC68000BASE::x44f8_move_w_aw_071234fc()
{
	set_ccr(OPER_AW_16());


}
void MC68000BASE::x44f9_move_w_al_071234fc()
{
	set_ccr(OPER_AL_16());


}
void MC68000BASE::x44fa_move_w_pcdi_071234fc()
{
	set_ccr(OPER_PCDI_16());


}
void MC68000BASE::x44fb_move_w_pcix_071234fc()
{
	set_ccr(OPER_PCIX_16());


}
void MC68000BASE::x44fc_move_w_i_071234fc()
{
	set_ccr(OPER_I_16());


}
void MC68000BASE::x40c0_move_w_07()
{
	DY() = MASK_OUT_BELOW_16(DY()) | get_sr();


}
void MC68000BASE::x40c0_move_w_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		DY() = MASK_OUT_BELOW_16(DY()) | get_sr();
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40d0_move_w_ai_07()
{
	uint32_t ea = EA_AY_AI_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40d8_move_w_pi_07()
{
	uint32_t ea = EA_AY_PI_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40e0_move_w_pd_07()
{
	uint32_t ea = EA_AY_PD_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40e8_move_w_di_07()
{
	uint32_t ea = EA_AY_DI_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40f0_move_w_ix_07()
{
	uint32_t ea = EA_AY_IX_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40f8_move_w_aw_07()
{
	uint32_t ea = EA_AW_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40f9_move_w_al_07()
{
	uint32_t ea = EA_AL_16();
	write_16(ea, get_sr());


}
void MC68000BASE::x40d0_move_w_ai_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AY_AI_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40d8_move_w_pi_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AY_PI_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40e0_move_w_pd_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AY_PD_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40e8_move_w_di_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AY_DI_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40f0_move_w_ix_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AY_IX_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40f8_move_w_aw_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AW_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x40f9_move_w_al_1234fc()
{
	if(m_s_flag) { /* NS990408 */
		uint32_t ea = EA_AL_16();
		write_16(ea, get_sr());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46c0_move_w_071234fc()
{
	if(m_s_flag) {
		set_sr(DY());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46d0_move_w_ai_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AY_AI_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46d8_move_w_pi_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AY_PI_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46e0_move_w_pd_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AY_PD_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46e8_move_w_di_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AY_DI_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46f0_move_w_ix_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AY_IX_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46f8_move_w_aw_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AW_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46f9_move_w_al_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_AL_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46fa_move_w_pcdi_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_PCDI_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46fb_move_w_pcix_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_PCIX_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x46fc_move_w_i_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_I_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		set_sr(new_sr);

	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e68_move_l_071234fc()
{
	if(m_s_flag) {
		AY() = (REG_USP());
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e60_move_l_071234fc()
{
	if(m_s_flag) {
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		REG_USP() = AY();
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7a_movec_l_1()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch(word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			REG_DA()[(word2 >> 12) & 15] = m_sfc;
			break;
		case 0x001:            /* DFC */
			REG_DA()[(word2 >> 12) & 15] = m_dfc;
			break;
#endif
		case 0x800:            /* USP */
			REG_DA()[(word2 >> 12) & 15] = REG_USP();
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			REG_DA()[(word2 >> 12) & 15] = m_vbr;
			break;
#endif
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7a_movec_l_23f()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch(word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			REG_DA()[(word2 >> 12) & 15] = m_sfc;
			break;
		case 0x001:            /* DFC */
			REG_DA()[(word2 >> 12) & 15] = m_dfc;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			REG_DA()[(word2 >> 12) & 15] = m_cacr;
			break;
#endif
		case 0x800:            /* USP */
			REG_DA()[(word2 >> 12) & 15] = REG_USP();
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			REG_DA()[(word2 >> 12) & 15] = m_vbr;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			REG_DA()[(word2 >> 12) & 15] = m_caar;
			break;
#endif
		case 0x803:            /* MSP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_SP() : REG_MSP();
			break;
		case 0x804:            /* ISP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_ISP() : REG_SP();
			break;
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7a_movec_l_4()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch(word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			REG_DA()[(word2 >> 12) & 15] = m_sfc;
			break;
		case 0x001:            /* DFC */
			REG_DA()[(word2 >> 12) & 15] = m_dfc;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			REG_DA()[(word2 >> 12) & 15] = m_cacr;
			break;
#endif
		case 0x800:            /* USP */
			REG_DA()[(word2 >> 12) & 15] = REG_USP();
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			REG_DA()[(word2 >> 12) & 15] = m_vbr;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			REG_DA()[(word2 >> 12) & 15] = m_caar;
			break;
#endif
		case 0x803:            /* MSP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_SP() : REG_MSP();
			break;
		case 0x804:            /* ISP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_ISP() : REG_SP();
			break;
#if defined(USE_MC68000MMU)
		case 0x003:             /* TC */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_tc;
			break;
		case 0x004:             /* ITT0 (040+, ACR0 on ColdFire) */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_itt0;
			break;
		case 0x005:             /* ITT1 (040+, ACR1 on ColdFire) */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_itt1;
			break;
		case 0x006:             /* DTT0 */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_dtt0;
			break;
		case 0x007:             /* DTT1 */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_dtt1;
			break;
		case 0x805:             /* MMUSR */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_sr_040;
			break;
		case 0x806:             /* URP */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_urp_aptr;
			break;
		case 0x807:             /* SRP */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_srp_aptr;
			break;
#endif
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7a_movec_l_c()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch(word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			REG_DA()[(word2 >> 12) & 15] = m_sfc;
			break;
		case 0x001:            /* DFC */
			REG_DA()[(word2 >> 12) & 15] = m_dfc;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			REG_DA()[(word2 >> 12) & 15] = m_cacr;
			break;
#endif
		case 0x800:            /* USP */
			REG_DA()[(word2 >> 12) & 15] = REG_USP();
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			REG_DA()[(word2 >> 12) & 15] = m_vbr;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			REG_DA()[(word2 >> 12) & 15] = m_caar;
			break;
#endif
		case 0x803:            /* MSP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_SP() : REG_MSP();
			break;
		case 0x804:            /* ISP */
			REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_ISP() : REG_SP();
			break;
#if defined(USE_MC68000MMU)
		case 0x004:             /* ITT0 (040+, ACR0 on ColdFire) */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_acr0;
			break;
		case 0x005:             /* ITT1 (040+, ACR1 on ColdFire) */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_acr1;
			break;
		case 0x006:             /* DTT0 */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_acr2;
			break;
		case 0x007:             /* DTT1 */
			REG_DA()[(word2 >> 12) & 15] = m_mmu_acr3;
			break;
#endif
		case 0xc00: // ROMBAR0
			/* TODO */
			break;
		case 0xc01: // ROMBAR1
			/* TODO */
			break;
		case 0xc04: // RAMBAR0
			/* TODO */
			break;
		case 0xc05: // RAMBAR1
			/* TODO */
			break;
		case 0xc0c: // MPCR
			/* TODO */
			break;
		case 0xc0d: // EDRAMBAR
			/* TODO */
			break;
		case 0xc0e: // SECMBAR
			/* TODO */
			break;
		case 0xc0f: // MBAR
			/* TODO */
			break;
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7b_movec_l_1()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch (word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
		case 0x001:            /* DFC */
			m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
#endif
		case 0x800:            /* USP */
			REG_USP() = REG_DA()[(word2 >> 12) & 15];
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			m_vbr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7b_movec_l_2f()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch (word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
		case 0x001:            /* DFC */
			m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			/* 68030 can write all bits except 5-7, 040 can write all */
			m_cacr = REG_DA()[(word2 >> 12) & 15] & 0x0f;

			if (m_cacr & (M68K_CACR_CI | M68K_CACR_CEI)) {
				ic_clear();
			}
			break;
#endif
		case 0x800:            /* USP */
			REG_USP() = REG_DA()[(word2 >> 12) & 15];
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			m_vbr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			m_caar = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		case 0x803:            /* MSP */
			/* we are in supervisor mode so just check for M flag */
			if(!m_m_flag) {
				REG_MSP() = REG_DA()[(word2 >> 12) & 15];
			} else {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			}
			break;
		case 0x804:            /* ISP */
			if(!m_m_flag) {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			} else {
				REG_ISP() = REG_DA()[(word2 >> 12) & 15];
			}
			break;
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7b_movec_l_3()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch (word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
		case 0x001:            /* DFC */
			m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			/* 68030 can write all bits except 5-7, 040 can write all */
			m_cacr = REG_DA()[(word2 >> 12) & 15] & 0xff1f;

			if (m_cacr & (M68K_CACR_CI | M68K_CACR_CEI)) {
				ic_clear();
			}
			break;
#endif
		case 0x800:            /* USP */
			REG_USP() = REG_DA()[(word2 >> 12) & 15];
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			m_vbr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			m_caar = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		case 0x803:            /* MSP */
			/* we are in supervisor mode so just check for M flag */
			if(!m_m_flag) {
				REG_MSP() = REG_DA()[(word2 >> 12) & 15];
			} else {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			}
			break;
		case 0x804:            /* ISP */
			if(!m_m_flag) {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			} else {
				REG_ISP() = REG_DA()[(word2 >> 12) & 15];
			}
			break;
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7b_movec_l_4()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch (word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
		case 0x001:            /* DFC */
			m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			/* 68030 can write all bits except 5-7, 040 can write all */
			m_cacr = REG_DA()[(word2 >> 12) & 15];
			if (m_cacr & (M68K_CACR_CI | M68K_CACR_CEI)) {
				ic_clear();
			}
			break;
#endif
		case 0x800:            /* USP */
			REG_USP() = REG_DA()[(word2 >> 12) & 15];
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			m_vbr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			m_caar = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		case 0x803:            /* MSP */
			/* we are in supervisor mode so just check for M flag */
			if(!m_m_flag) {
				REG_MSP() = REG_DA()[(word2 >> 12) & 15];
			} else {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			}
			break;
		case 0x804:            /* ISP */
			if(!m_m_flag) {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			} else {
				REG_ISP() = REG_DA()[(word2 >> 12) & 15];
			}
			break;
#if defined(USE_MC68000MMU)
		case 0x003:         /* TC */
			m_mmu_tc = REG_DA()[(word2 >> 12) & 15];

			if (m_mmu_tc & 0x8000)
			{
				m_pmmu_enabled = 1;
			}
			else
			{
				m_pmmu_enabled = 0;
			}
			break;
		case 0x004:         /* ITT0 */
			m_mmu_itt0 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x005:         /* ITT1 */
			m_mmu_itt1 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x006:         /* DTT0 */
			m_mmu_dtt0 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x007:         /* DTT1 */
			m_mmu_dtt1 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x805:         /* MMUSR */
			m_mmu_sr_040 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x806:         /* URP */
			m_mmu_urp_aptr = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x807:         /* SRP */
			m_mmu_srp_aptr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e7b_movec_l_c()
{
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();

		trace_t0();          /* auto-disable (see m68kcpu.h) */
		switch (word2 & 0xfff) {
#if defined(USE_MC68000VBR)
		case 0x000:            /* SFC */
			m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
		case 0x001:            /* DFC */
			m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x002:            /* CACR */
			/* 68030 can write all bits except 5-7, 040 can write all */
			m_cacr = REG_DA()[(word2 >> 12) & 15] & 0x0f;

			if (m_cacr & (M68K_CACR_CI | M68K_CACR_CEI)) {
				ic_clear();
			}
			break;
#endif
		case 0x800:            /* USP */
			REG_USP() = REG_DA()[(word2 >> 12) & 15];
			break;
#if defined(USE_MC68000VBR)
		case 0x801:            /* VBR */
			m_vbr = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
#if defined(USE_MC68000CACHE)
		case 0x802:            /* CAAR */
			m_caar = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		case 0x803:            /* MSP */
			/* we are in supervisor mode so just check for M flag */
			if(!m_m_flag) {
				REG_MSP() = REG_DA()[(word2 >> 12) & 15];
			} else {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			}
			break;
		case 0x804:            /* ISP */
			if(!m_m_flag) {
				REG_SP() = (REG_DA()[(word2 >> 12) & 15]);
			} else {
				REG_ISP() = REG_DA()[(word2 >> 12) & 15];
			}
			break;
#if defined(USE_MC68000MMU)
		case 0x004:         /* ITT0 */
			m_mmu_acr0 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x005:         /* ITT1 */
			m_mmu_acr1 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x006:         /* DTT0 */
			m_mmu_acr2 = REG_DA()[(word2 >> 12) & 15];
			break;
		case 0x007:         /* DTT1 */
			m_mmu_acr3 = REG_DA()[(word2 >> 12) & 15];
			break;
#endif
		case 0xc00: // ROMBAR0
			/* TODO */
			break;
		case 0xc01: // ROMBAR1
			/* TODO */
			break;
		case 0xc04: // RAMBAR0
			/* TODO */
			break;
		case 0xc05: // RAMBAR1
			/* TODO */
			break;
		case 0xc0c: // MPCR
			/* TODO */
			break;
		case 0xc0d: // EDRAMBAR
			/* TODO */
			break;
		case 0xc0e: // SECMBAR
			/* TODO */
			break;
		case 0xc0f: // MBAR
			/* TODO */
			break;
		default:
			m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x48a0_movem_w_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			ea -= 2;
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[15-i]));
			count++;
		}
	AY() = (ea);

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4890_movem_w_ai_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x48a8_movem_w_di_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x48b0_movem_w_ix_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x48b8_movem_w_aw_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x48b9_movem_w_al_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x48e0_movem_l_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			ea -= 4;
			write_stack_32_rev(ea, REG_DA()[15-i]);
			count++;
		}
	AY() = (ea);

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x48d0_movem_l_ai_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x48e8_movem_l_di_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x48f0_movem_l_ix_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x48f8_movem_l_aw_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x48f9_movem_l_al_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			write_stack_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4c98_movem_w_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}
	AY() = (ea);

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cba_movem_w_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCDI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cbb_movem_w_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCIX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4c90_movem_w_ai_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4ca8_movem_w_di_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cb0_movem_w_ix_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cb8_movem_w_aw_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cb9_movem_w_al_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(read_stack_16(ea)));
			ea += 2;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_w);


}
void MC68000BASE::x4cd8_movem_l_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}
	AY() = (ea);

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cfa_movem_l_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCDI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cfb_movem_l_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCIX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cd0_movem_l_ai_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4ce8_movem_l_di_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cf0_movem_l_ix_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cf8_movem_l_aw_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x4cf9_movem_l_al_071234fc()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i)) {
			REG_DA()[i] = read_stack_32(ea);
			ea += 4;
			count++;
		}

	SUBST_ICOUNT(count * m_cyc_movem_l);


}
void MC68000BASE::x0188_movep_w_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = DX();

	write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
	write_8(ea += 2, MASK_OUT_ABOVE_8(src));


}
void MC68000BASE::x01c8_movep_l_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();

	write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
	write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
	write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
	write_8(ea += 2, MASK_OUT_ABOVE_8(src));


}
void MC68000BASE::x0108_movep_w_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | ((read_8(ea) << 8) + read_8(ea + 2));


}
void MC68000BASE::x0148_movep_l_071234fc()
{
	uint32_t ea = EA_AY_DI_32();

	DX() = (read_8(ea) << 24) + (read_8(ea + 2) << 16)
		+ (read_8(ea + 4) << 8) + read_8(ea + 6);


}
void MC68000BASE::x0e10_moves_b_ai_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e18_moves_b_pi_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e1f_moves_b_pi7_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e20_moves_b_pd_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e27_moves_b_pd7_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PD_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e28_moves_b_di_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e30_moves_b_ix_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e38_moves_b_aw_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e39_moves_b_al_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e10_moves_b_ai_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e18_moves_b_pi_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e1f_moves_b_pi7_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e20_moves_b_pd_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e27_moves_b_pd7_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PD_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e28_moves_b_di_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e30_moves_b_ix_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e38_moves_b_aw_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e39_moves_b_al_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag)
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_8();

		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {             /* Register to memory */
			set_fc(m_dfc);
			write_8(ea, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {      /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(read_8(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | read_8(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e50_moves_w_ai_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e58_moves_w_pi_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e60_moves_w_pd_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e68_moves_w_di_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e70_moves_w_ix_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e78_moves_w_aw_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e79_moves_w_al_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e50_moves_w_ai_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e58_moves_w_pi_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e60_moves_w_pd_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e68_moves_w_di_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e70_moves_w_ix_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e78_moves_w_aw_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e79_moves_w_al_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_16();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
		} else if(BIT_F(word2)) {    /* Memory to address register */
			set_fc(m_sfc);
			REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(read_16(ea));
			SET_ICOUNT(2);
		} else {
			/* Memory to data register */
			set_fc(m_sfc);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | read_16(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e90_moves_l_ai_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e98_moves_l_pi_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0ea0_moves_l_pd_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0ea8_moves_l_di_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb0_moves_l_ix_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb8_moves_l_aw_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb9_moves_l_al_134fc()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e90_moves_l_ai_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0e98_moves_l_pi_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0ea0_moves_l_pd_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0ea8_moves_l_di_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb0_moves_l_ix_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb8_moves_l_aw_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x0eb9_moves_l_al_2()
{
#if defined(USE_MC68000VBR)
	if(m_s_flag) {
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_32();

		trace_t0();            /* auto-disable (see m68kcpu.h) */
		if(BIT_B(word2)) {           /* Register to memory */
			set_fc(m_dfc);
			write_32(ea, REG_DA()[(word2 >> 12) & 15]);
			SET_ICOUNT(2);
		} else {
			/* Memory to register */
			set_fc(m_sfc);
			REG_DA()[(word2 >> 12) & 15] = read_32(ea);
			SET_ICOUNT(2);
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}
#else
	m_proc_mode = (PROC_MODE_EXCEPTION_ILLEGAL_INSTRUCTION);
#endif


}
void MC68000BASE::x7000_moveq_l_071234fc()
{
	uint32_t res = DX() = MAKE_INT_8(MASK_OUT_ABOVE_8(m_ir));

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xf620_move16_l_4fc()
{
	uint16_t w2 = OPER_I_16();
	int ax = m_ir & 7;
	int ay = (w2 >> 12) & 7;
	write_32(REG_A()[ay],    read_32(REG_A()[ax]));
	write_32(REG_A()[ay]+4,  read_32(REG_A()[ax]+4));
	write_32(REG_A()[ay]+8,  read_32(REG_A()[ax]+8));
	write_32(REG_A()[ay]+12, read_32(REG_A()[ax]+12));

	REG_A()[ax] += 16;
	REG_A()[ay] += 16;


}
void MC68000BASE::xc1c0_muls_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(DY()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1d0_muls_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_AI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1d8_muls_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1e0_muls_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PD_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1e8_muls_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_DI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1f0_muls_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_IX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1f8_muls_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AW_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1f9_muls_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AL_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1fa_muls_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCDI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1fb_muls_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCIX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc1fc_muls_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_I_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0c0_mulu_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_16(DY()) * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0d0_mulu_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_AI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0d8_mulu_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0e0_mulu_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PD_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0e8_mulu_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_DI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0f0_mulu_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_IX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0f8_mulu_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AW_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0f9_mulu_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AL_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0fa_mulu_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCDI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0fb_mulu_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCIX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::xc0fc_mulu_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_I_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4c00_mull_l_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = DY();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);

	done: ;


}
void MC68000BASE::x4c10_mull_l_ai_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AY_AI_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c18_mull_l_pi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AY_PI_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c20_mull_l_pd_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AY_PD_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c28_mull_l_di_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AY_DI_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c30_mull_l_ix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AY_IX_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c38_mull_l_aw_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AW_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c39_mull_l_al_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_AL_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c3a_mull_l_pcdi_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_PCDI_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c3b_mull_l_pcix_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_PCIX_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4c3c_mull_l_i_234fc()
{
	uint32_t word2 = OPER_I_16();
	uint64_t src = OPER_I_32();
	uint64_t dst = REG_D()[(word2 >> 12) & 7];
	uint64_t res;

	m_c_flag = CFLAG_CLEAR;

	if(BIT_B(word2)) {               /* signed */
		res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
		if(!BIT_A(word2)) {
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = ((int64_t)res != (int32_t)res)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			goto done;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		goto done;
	}

	res = src * dst;
	if(!BIT_A(word2)) {
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_v_flag = (res > 0xffffffff)<<7;
		REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
		goto done;
	}
	m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
	m_n_flag = NFLAG_64(res);
	m_v_flag = VFLAG_CLEAR;
	REG_D()[word2 & 7] = (res >> 32);
	REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
	done: ;

}
void MC68000BASE::x4800_nbcd_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4810_nbcd_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4818_nbcd_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x481f_nbcd_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4820_nbcd_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4827_nbcd_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4828_nbcd_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4830_nbcd_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4838_nbcd_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4839_nbcd_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);
	uint32_t res = (uint32_t)(-(int32_t)dst - (int32_t)XFLAG_1());

	if(res != 0) {
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	} else {
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void MC68000BASE::x4400_neg_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_8(*r_dst);

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = *r_dst & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x4410_neg_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4418_neg_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x441f_neg_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4420_neg_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4427_neg_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4428_neg_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4430_neg_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4438_neg_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4439_neg_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x4440_neg_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_16(*r_dst);

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (*r_dst & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x4450_neg_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4458_neg_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4460_neg_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4468_neg_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4470_neg_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4478_neg_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4479_neg_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x4480_neg_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - *r_dst;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(*r_dst, 0, res);
	m_v_flag = (*r_dst & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x4490_neg_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x4498_neg_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x44a0_neg_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x44a8_neg_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x44b0_neg_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x44b8_neg_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x44b9_neg_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x4000_negx_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_8(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = *r_dst & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void MC68000BASE::x4010_negx_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4018_negx_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x401f_negx_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4020_negx_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4027_negx_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4028_negx_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4030_negx_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4038_negx_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4039_negx_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x4040_negx_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_16(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (*r_dst & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void MC68000BASE::x4050_negx_w_ai_071234fc()
{
	uint32_t ea  = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4058_negx_w_pi_071234fc()
{
	uint32_t ea  = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4060_negx_w_pd_071234fc()
{
	uint32_t ea  = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4068_negx_w_di_071234fc()
{
	uint32_t ea  = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4070_negx_w_ix_071234fc()
{
	uint32_t ea  = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4078_negx_w_aw_071234fc()
{
	uint32_t ea  = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4079_negx_w_al_071234fc()
{
	uint32_t ea  = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x4080_negx_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_32(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(*r_dst, 0, res);
	m_v_flag = (*r_dst & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void MC68000BASE::x4090_negx_l_ai_071234fc()
{
	uint32_t ea  = EA_AY_AI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x4098_negx_l_pi_071234fc()
{
	uint32_t ea  = EA_AY_PI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x40a0_negx_l_pd_071234fc()
{
	uint32_t ea  = EA_AY_PD_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x40a8_negx_l_di_071234fc()
{
	uint32_t ea  = EA_AY_DI_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x40b0_negx_l_ix_071234fc()
{
	uint32_t ea  = EA_AY_IX_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x40b8_negx_l_aw_071234fc()
{
	uint32_t ea  = EA_AW_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x40b9_negx_l_al_071234fc()
{
	uint32_t ea  = EA_AL_32();
	uint32_t src = read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x4e71_nop_071234fc()
{
	trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void MC68000BASE::x4600_not_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_8(~*r_dst);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4610_not_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4618_not_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x461f_not_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4620_not_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4627_not_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4628_not_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4630_not_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4638_not_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4639_not_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(~read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4640_not_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_16(~*r_dst);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4650_not_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4658_not_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4660_not_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4668_not_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4670_not_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4678_not_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4679_not_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(~read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4680_not_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4690_not_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4698_not_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x46a0_not_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x46a8_not_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x46b0_not_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x46b8_not_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x46b9_not_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = MASK_OUT_ABOVE_32(~read_32(ea));

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8000_or_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= MASK_OUT_ABOVE_8(DY())));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8010_or_b_ai_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_AI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8018_or_b_pi_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x801f_or_b_pi7_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8020_or_b_pd_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8027_or_b_pd7_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8028_or_b_di_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_DI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8030_or_b_ix_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_IX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8038_or_b_aw_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AW_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8039_or_b_al_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AL_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x803a_or_b_pcdi_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCDI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x803b_or_b_pcix_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCIX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x803c_or_b_i_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8040_or_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= MASK_OUT_ABOVE_16(DY())));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8050_or_w_ai_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_AI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8058_or_w_pi_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8060_or_w_pd_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PD_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8068_or_w_di_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_DI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8070_or_w_ix_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_IX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8078_or_w_aw_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AW_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8079_or_w_al_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AL_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x807a_or_w_pcdi_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCDI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x807b_or_w_pcix_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCIX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x807c_or_w_i_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_I_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8080_or_l_071234fc()
{
	uint32_t res = DX() |= DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8090_or_l_ai_071234fc()
{
	uint32_t res = DX() |= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8098_or_l_pi_071234fc()
{
	uint32_t res = DX() |= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80a0_or_l_pd_071234fc()
{
	uint32_t res = DX() |= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80a8_or_l_di_071234fc()
{
	uint32_t res = DX() |= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80b0_or_l_ix_071234fc()
{
	uint32_t res = DX() |= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80b8_or_l_aw_071234fc()
{
	uint32_t res = DX() |= OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80b9_or_l_al_071234fc()
{
	uint32_t res = DX() |= OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80ba_or_l_pcdi_071234fc()
{
	uint32_t res = DX() |= OPER_PCDI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80bb_or_l_pcix_071234fc()
{
	uint32_t res = DX() |= OPER_PCIX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x80bc_or_l_i_071234fc()
{
	uint32_t res = DX() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8110_or_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8118_or_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x811f_or_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8120_or_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8127_or_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8128_or_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8130_or_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8138_or_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8139_or_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8150_or_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8158_or_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8160_or_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8168_or_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8170_or_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8178_or_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8179_or_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8190_or_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x8198_or_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x81a0_or_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x81a8_or_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x81b0_or_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x81b8_or_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x81b9_or_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0000_ori_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8((DY() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0010_ori_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0018_ori_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x001f_ori_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0020_ori_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0027_ori_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0028_ori_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0030_ori_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0038_ori_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0039_ori_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | read_8(ea));

	write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0040_ori_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() |= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0050_ori_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0058_ori_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0060_ori_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0068_ori_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0070_ori_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0078_ori_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0079_ori_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | read_16(ea));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0080_ori_l_071234fc()
{
	uint32_t res = DY() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0090_ori_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x0098_ori_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x00a0_ori_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x00a8_ori_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x00b0_ori_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x00b8_ori_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x00b9_ori_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src | read_32(ea);

	write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x003c_ori_w_071234fc()
{
	uint32_t src = OPER_I_8();
	if (CPU_TYPE_IS_000()) {
		/* dummy read */
		read_dummy_imm_16();
	}
	set_ccr(get_ccr() | src);


}
void MC68000BASE::x007c_ori_w_071234fc()
{
	if(m_s_flag) {
		uint32_t src = OPER_I_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		if (CPU_TYPE_IS_000()) {
			/* dummy read */
			read_dummy_imm_16();
		}
		set_sr(get_sr() | src);
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}

}
void MC68000BASE::x8140_pack_w_234fc()
{
	/* Note: DX() and DY() are reversed in Motorola's docs */
	uint32_t src = DY() + OPER_I_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | ((src >> 4) & 0x00f0) | (src & 0x000f);


}
void MC68000BASE::x8f48_pack_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t ea_src = EA_AY_PD_8();
	uint32_t src = read_8(ea_src);
	ea_src = EA_AY_PD_8();
	src = ((src << 8) | read_8(ea_src)) + OPER_I_16();

	write_8(EA_A7_PD_8(), ((src >> 8) & 0x000f) | ((src<<4) & 0x00f0));


}
void MC68000BASE::x814f_pack_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t ea_src = EA_A7_PD_8();
	uint32_t src = read_8(ea_src);
	ea_src = EA_A7_PD_8();
	src = (src | (read_8(ea_src) << 8)) + OPER_I_16();

	write_8(EA_AX_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x00f));


}
void MC68000BASE::x8f4f_pack_w_234fc()
{
	uint32_t ea_src = EA_A7_PD_8();
	uint32_t src = read_8(ea_src);
	ea_src = EA_A7_PD_8();
	src = (src | (read_8(ea_src) << 8)) + OPER_I_16();

	write_8(EA_A7_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x000f));


}
void MC68000BASE::x8148_pack_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t ea_src = EA_AY_PD_8();
	uint32_t src = read_8(ea_src);
	ea_src = EA_AY_PD_8();
	src = (src | (read_8(ea_src) << 8)) + OPER_I_16();

	write_8(EA_AX_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x000f));


}
void MC68000BASE::x4850_pea_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();

	push_32(ea);


}
void MC68000BASE::x4868_pea_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();

	push_32(ea);


}
void MC68000BASE::x4870_pea_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();

	push_32(ea);


}
void MC68000BASE::x4878_pea_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();

	push_32(ea);


}
void MC68000BASE::x4879_pea_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();

	push_32(ea);


}
void MC68000BASE::x487a_pea_l_pcdi_071234fc()
{
	uint32_t ea = EA_PCDI_32();

	push_32(ea);


}
void MC68000BASE::x487b_pea_l_pcix_071234fc()
{
	uint32_t ea = EA_PCIX_32();

	push_32(ea);


}
void MC68000BASE::xf518_pflusha_l_4fc()
{
#if defined(USE_MC68000MMU)
	if(m_has_pmmu) {
		logging->out_logf(LOG_ERROR, "68040: unhandled PFLUSHA (ir=%04x)\n", m_ir);
	} else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xf510_pflushan_l_4fc()
{
#if defined(USE_MC68000MMU)
	if(m_has_pmmu) {
		logging->out_logf(LOG_ERROR, "68040: unhandled PFLUSHAN (ir=%04x)\n", m_ir);
	} else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xf000_pmmu_l_234fc()
{
#if defined(USE_MC68000MMU)
	if (m_has_pmmu)
	{
		m68851_mmu_ops();
	}
	else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::xf548_ptest_l_4()
{
#if defined(USE_MC68000MMU)
	if(m_has_pmmu)
	{
		logging->out_logf(LOG_ERROR, "68040: unhandled PTEST\n");
	}
	else
#endif
	{
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
	}


}
void MC68000BASE::x4e70_reset_071234fc()
{
	if(m_s_flag) {
//		if(!m_reset_instr_callback.isnull())
//			(m_reset_instr_callback)(1);
		reset_peripherals();
		SET_ICOUNT(m_cyc_reset);
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}

}
void MC68000BASE::xe018_ror_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0)
		SET_ICOUNT(orig_shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-orig_shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe058_ror_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe098_ror_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = MASK_OUT_ABOVE_32(src << (9-shift));
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe038_ror_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_c_flag = src << (8-((shift-1)&7));
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe078_ror_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_c_flag = (src >> ((shift - 1) & 15)) << 8;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe0b8_ror_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		*r_dst = res;
		m_c_flag = MASK_OUT_ABOVE_32((src >> ((shift - 1) & 31)) << 8);
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = MASK_OUT_ABOVE_32(src);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe6d0_ror_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6d8_ror_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6e0_ror_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6e8_ror_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6f0_ror_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6f8_ror_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe6f9_ror_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_16(src, 1);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe118_rol_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0)
		SET_ICOUNT(orig_shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << orig_shift;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe158_rol_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_16(src, shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe198_rol_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = MASK_OUT_ABOVE_32(src >> (24-shift));
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe138_rol_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		if(shift != 0) {
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			m_c_flag = (src & 1)<<8;
			m_n_flag = NFLAG_8(src);
			m_not_z_flag = src;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe178_rol_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		if(shift != 0) {
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
		} else {
			m_c_flag = (src & 1)<<8;
			m_n_flag = NFLAG_16(src);
			m_not_z_flag = src;
			m_v_flag = VFLAG_CLEAR;
		}
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe1b8_rol_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(orig_shift != 0) {
		SET_ICOUNT(orig_shift * m_cyc_shift);

		*r_dst = res;

		m_c_flag = MASK_OUT_ABOVE_32((src >> ((32 - shift) & 0x1f)) << 8);
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_32(src);
		m_not_z_flag = MASK_OUT_ABOVE_32(src);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe7d0_rol_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7d8_rol_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7e0_rol_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7e8_rol_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7f0_rol_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7f8_rol_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe7f9_rol_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe010_roxr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe050_roxr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe090_roxr_l_071234fc()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	res = ROR_33_64(res, shift);

	m_c_flag = m_x_flag = MASK_OUT_ABOVE_32(res >> 24);
	res = MASK_OUT_ABOVE_32(res);

	*r_dst =  MASK_OUT_ABOVE_32(res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe030_roxr_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0) {
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROR_9(src | (XFLAG_1() << 8), shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_8(*r_dst);
		m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe070_roxr_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROR_17(src | (XFLAG_1() << 16), shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_16(*r_dst);
		m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe0b0_roxr_l_071234fc()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0) {
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROR_33_64(res, shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = MASK_OUT_ABOVE_32(res >> 24);
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_32(*r_dst);
		m_not_z_flag = *r_dst;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe4d0_roxr_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4d8_roxr_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4e0_roxr_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4e8_roxr_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4f0_roxr_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4f8_roxr_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe4f9_roxr_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe110_roxl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe150_roxl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe190_roxl_l_071234fc()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		SET_ICOUNT(shift * m_cyc_shift);

	res = ROL_33_64(res, shift);

	m_c_flag = m_x_flag = MASK_OUT_ABOVE_32(res >> 24);
	res = MASK_OUT_ABOVE_32(res);

	*r_dst = MASK_OUT_ABOVE_32(res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe130_roxl_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;


	if(orig_shift != 0) {
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROL_9(src | (XFLAG_1() << 8), shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_8(*r_dst);
		m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe170_roxl_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0) {
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROL_17(src | (XFLAG_1() << 16), shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_16(*r_dst);
		m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe1b0_roxl_l_071234fc()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0) {
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROL_33_64(res, shift);

		SET_ICOUNT(orig_shift * m_cyc_shift);

		m_c_flag = m_x_flag = MASK_OUT_ABOVE_32(res >> 24);
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = MASK_OUT_ABOVE_32(res);
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_CLEAR;
	} else {
		m_c_flag = m_x_flag;
		m_n_flag = NFLAG_32(*r_dst);
		m_not_z_flag = *r_dst;
		m_v_flag = VFLAG_CLEAR;
	}


}
void MC68000BASE::xe5d0_roxl_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5d8_roxl_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5e0_roxl_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5e8_roxl_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5f0_roxl_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5f8_roxl_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::xe5f9_roxl_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4e74_rtd_l_1234fc()
{
	uint32_t new_pc = pull_32();

	trace_t0();              /* auto-disable (see m68kcpu.h) */
	REG_SP() = (MASK_OUT_ABOVE_32(REG_SP() + MAKE_INT_16(OPER_I_16())));
	jump(new_pc);


}
void MC68000BASE::x4e73_rte_l_0()
{
	if(m_s_flag) {
		uint32_t new_sr;
		uint32_t new_pc;

//		if (!m_rte_instr_callback.isnull())
//			(m_rte_instr_callback)(1);
		trace_t0();              /* auto-disable (see m68kcpu.h) */

		new_sr = pull_16();
		new_pc = pull_32();
		jump(new_pc);
		set_sr(new_sr);

		m_instr_mode = INSTRUCTION_YES;
		m_run_mode = RUN_MODE_NORMAL;
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x4e73_rte_l_71()
{
	if(m_s_flag) {
		uint32_t new_sr;
		uint32_t new_pc;
		uint32_t format_word;

//		if (!m_rte_instr_callback.isnull())
//			(m_rte_instr_callback)(1);
		trace_t0();              /* auto-disable (see m68kcpu.h) */

		format_word = read_16(REG_SP()+6) >> 12;
		if(format_word == 0) {
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* format word */
			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
		} else {
			if (format_word == 0x8) /* type 1000 stack frame */
			{
				new_sr = pull_16();
				new_pc = pull_32();
				fake_pull_16();   /* format word */
				jump(new_pc);
				set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				fake_pull_16();  /* special status */
				fake_pull_32();  /* fault address */
				fake_pull_32();  /* reserved and data output buffer */
				fake_pull_32();  /* reserved and data input buffer */
				fake_pull_32();  /* reserved and instruction input buffer */
				fake_pull_32();  /* 8 dwords of CPU specific undocumented data */
				fake_pull_32();
				fake_pull_32();
				fake_pull_32();
				fake_pull_32();
				fake_pull_32();
				fake_pull_32();
				fake_pull_32();
			}
			else
			{
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				/* Not handling bus fault (9) */
				m_proc_mode = (PROC_MODE_EXCEPTION_FORMAT_ERROR);
			}
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}

}
void MC68000BASE::x4e73_rte_l_234fc()
{
	if(m_s_flag) {
		uint32_t new_sr;
		uint32_t new_pc;
		uint32_t format_word;

//		if (!m_rte_instr_callback.isnull())
//			(m_rte_instr_callback)(1);
		trace_t0();              /* auto-disable (see m68kcpu.h) */

	rte_loop:
		format_word = read_16(REG_SP()+6) >> 12;
		switch(format_word) {
		case 0: /* Normal */
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* format word */
			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			break;

		case 1: /* Throwaway */
			new_sr = pull_16();
			fake_pull_32();   /* program counter */
			fake_pull_16();   /* format word */
			set_sr_noint(new_sr);
			goto rte_loop;

		case 2: /* Trap */
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* format word */
			fake_pull_32();   /* address */
			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			break;

		case 7: /* 68040 access error */
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* $06: format word */
			fake_pull_32();   /* $08: effective address */
			fake_pull_16();   /* $0c: special status word */
			fake_pull_16();   /* $0e: wb3s */
			fake_pull_16();   /* $10: wb2s */
			fake_pull_16();   /* $12: wb1s */
			fake_pull_32();   /* $14: data fault address */
			fake_pull_32();   /* $18: wb3a */
			fake_pull_32();   /* $1c: wb3d */
			fake_pull_32();   /* $20: wb2a */
			fake_pull_32();   /* $24: wb2d */
			fake_pull_32();   /* $28: wb1a */
			fake_pull_32();   /* $2c: wb1d/pd0 */
			fake_pull_32();   /* $30: pd1 */
			fake_pull_32();   /* $34: pd2 */
			fake_pull_32();   /* $38: pd3 */
			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			break;

		case 0x0a: /* Bus Error at instruction boundary */
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* $06: format word */
			fake_pull_16();   /* $08: internal register */
			fake_pull_16();   /* $0a: special status word */
			fake_pull_16();   /* $0c: instruction pipe stage c */
			fake_pull_16();   /* $0e: instruction pipe stage b */
			fake_pull_32();   /* $10: data fault address */
			fake_pull_32();   /* $14: internal registers */
			fake_pull_32();   /* $18: data output buffer */
			fake_pull_32();   /* $1c: internal registers */

			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			break;

		case 0x0b: /* Bus Error - Instruction Execution in Progress */
			new_sr = pull_16();
			new_pc = pull_32();
			fake_pull_16();   /* $06: format word */
			fake_pull_16();   /* $08: internal register */
			fake_pull_16();   /* $0a: special status word */
			fake_pull_16();   /* $0c: instruction pipe stage c */
			fake_pull_16();   /* $0e: instruction pipe stage b */
			fake_pull_32();   /* $10: data fault address */
			fake_pull_32();   /* $14: internal registers */
			fake_pull_32();   /* $18: data output buffer */
			fake_pull_32();   /* $1c: internal registers */
			fake_pull_32();   /* $20:  */
			fake_pull_32();   /* $24: stage B address */
			fake_pull_32();   /* $28:  */
			fake_pull_32();   /* $2c: data input buffer */
			fake_pull_32();   /* $30:  */
			fake_pull_16();   /* $34:  */
			fake_pull_16();   /* $36: version #, internal information */
			fake_pull_32();   /* $38:  */
			fake_pull_32();   /* $3c:  */
			fake_pull_32();   /* $40:  */
			fake_pull_32();   /* $44:  */
			fake_pull_32();   /* $48:  */
			fake_pull_32();   /* $4c:  */
			fake_pull_32();   /* $50:  */
			fake_pull_32();   /* $54:  */
			fake_pull_32();   /* $58:  */

			jump(new_pc);
			set_sr(new_sr);
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			break;

		default:
			/* Not handling long or short bus fault */
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			m_proc_mode = (PROC_MODE_EXCEPTION_FORMAT_ERROR);
			break;
		}
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x06c0_rtm_l_234fc()
{
	trace_t0();              /* auto-disable (see m68kcpu.h) */
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (rtm)\n",
					get_class_name(), m_ppc, m_ir);


}
void MC68000BASE::x4e77_rtr_l_071234fc()
{
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	set_ccr(pull_16());
	jump(pull_32());


}
void MC68000BASE::x4e75_rts_l_071234fc()
{
	trace_t0();                  /* auto-disable (see m68kcpu.h) */
	jump(pull_32());


}
void MC68000BASE::x8100_sbcd_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff) {
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	} else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void MC68000BASE::x8f08_sbcd_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff) {
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	} else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x810f_sbcd_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff) {
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	} else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x8f0f_sbcd_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff) {
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	} else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x8108_sbcd_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff) {
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	} else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x50c0_st_b_071234fc()
{
	DY() |= 0xff;


}
void MC68000BASE::x50d0_st_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), 0xff);


}
void MC68000BASE::x50d8_st_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), 0xff);


}
void MC68000BASE::x50df_st_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), 0xff);


}
void MC68000BASE::x50e0_st_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), 0xff);


}
void MC68000BASE::x50e7_st_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), 0xff);


}
void MC68000BASE::x50e8_st_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), 0xff);


}
void MC68000BASE::x50f0_st_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), 0xff);


}
void MC68000BASE::x50f8_st_b_aw_071234fc()
{
	write_8(EA_AW_8(), 0xff);


}
void MC68000BASE::x50f9_st_b_al_071234fc()
{
	write_8(EA_AL_8(), 0xff);


}
void MC68000BASE::x51c0_sf_b_071234fc()
{
	DY() &= 0xffffff00;


}
void MC68000BASE::x51d0_sf_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), 0);


}
void MC68000BASE::x51d8_sf_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), 0);


}
void MC68000BASE::x51df_sf_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), 0);


}
void MC68000BASE::x51e0_sf_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), 0);


}
void MC68000BASE::x51e7_sf_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), 0);


}
void MC68000BASE::x51e8_sf_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), 0);


}
void MC68000BASE::x51f0_sf_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), 0);


}
void MC68000BASE::x51f8_sf_b_aw_071234fc()
{
	write_8(EA_AW_8(), 0);


}
void MC68000BASE::x51f9_sf_b_al_071234fc()
{
	write_8(EA_AL_8(), 0);


}
void MC68000BASE::x52c0_shi_b_071234fc()
{
	if(COND_HI()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x53c0_sls_b_071234fc()
{
	if(COND_LS()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x54c0_scc_b_071234fc()
{
	if(COND_CC()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x55c0_scs_b_071234fc()
{
	if(COND_CS()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x56c0_sne_b_071234fc()
{
	if(COND_NE()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x57c0_seq_b_071234fc()
{
	if(COND_EQ()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x58c0_svc_b_071234fc()
{
	if(COND_VC()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x59c0_svs_b_071234fc()
{
	if(COND_VS()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5ac0_spl_b_071234fc()
{
	if(COND_PL()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5bc0_smi_b_071234fc()
{
	if(COND_MI()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5cc0_sge_b_071234fc()
{
	if(COND_GE()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5dc0_slt_b_071234fc()
{
	if(COND_LT()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5ec0_sgt_b_071234fc()
{
	if(COND_GT()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x5fc0_sle_b_071234fc()
{
	if(COND_LE()) {
		DY() |= 0xff;
		SET_ICOUNT(m_cyc_scc_r_true);

	} else {
		DY() &= 0xffffff00;
	}


}
void MC68000BASE::x52d0_shi_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52d8_shi_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52df_shi_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52e0_shi_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52e7_shi_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52e8_shi_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52f0_shi_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52f8_shi_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x52f9_shi_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_HI() ? 0xff : 0);


}
void MC68000BASE::x53d0_sls_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53d8_sls_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53df_sls_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53e0_sls_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53e7_sls_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53e8_sls_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53f0_sls_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53f8_sls_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x53f9_sls_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_LS() ? 0xff : 0);


}
void MC68000BASE::x54d0_scc_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54d8_scc_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54df_scc_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54e0_scc_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54e7_scc_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54e8_scc_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54f0_scc_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54f8_scc_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x54f9_scc_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_CC() ? 0xff : 0);


}
void MC68000BASE::x55d0_scs_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55d8_scs_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55df_scs_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55e0_scs_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55e7_scs_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55e8_scs_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55f0_scs_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55f8_scs_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x55f9_scs_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_CS() ? 0xff : 0);


}
void MC68000BASE::x56d0_sne_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56d8_sne_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56df_sne_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56e0_sne_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56e7_sne_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56e8_sne_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56f0_sne_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56f8_sne_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x56f9_sne_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_NE() ? 0xff : 0);


}
void MC68000BASE::x57d0_seq_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57d8_seq_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57df_seq_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57e0_seq_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57e7_seq_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57e8_seq_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57f0_seq_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57f8_seq_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x57f9_seq_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_EQ() ? 0xff : 0);


}
void MC68000BASE::x58d0_svc_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58d8_svc_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58df_svc_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58e0_svc_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58e7_svc_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58e8_svc_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58f0_svc_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58f8_svc_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x58f9_svc_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_VC() ? 0xff : 0);


}
void MC68000BASE::x59d0_svs_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59d8_svs_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59df_svs_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59e0_svs_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59e7_svs_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59e8_svs_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59f0_svs_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59f8_svs_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x59f9_svs_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_VS() ? 0xff : 0);


}
void MC68000BASE::x5ad0_spl_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5ad8_spl_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5adf_spl_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5ae0_spl_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5ae7_spl_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5ae8_spl_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5af0_spl_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5af8_spl_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5af9_spl_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_PL() ? 0xff : 0);


}
void MC68000BASE::x5bd0_smi_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5bd8_smi_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5bdf_smi_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5be0_smi_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5be7_smi_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5be8_smi_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5bf0_smi_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5bf8_smi_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5bf9_smi_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_MI() ? 0xff : 0);


}
void MC68000BASE::x5cd0_sge_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5cd8_sge_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5cdf_sge_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5ce0_sge_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5ce7_sge_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5ce8_sge_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5cf0_sge_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5cf8_sge_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5cf9_sge_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_GE() ? 0xff : 0);


}
void MC68000BASE::x5dd0_slt_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5dd8_slt_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5ddf_slt_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5de0_slt_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5de7_slt_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5de8_slt_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5df0_slt_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5df8_slt_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5df9_slt_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_LT() ? 0xff : 0);


}
void MC68000BASE::x5ed0_sgt_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ed8_sgt_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5edf_sgt_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ee0_sgt_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ee7_sgt_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ee8_sgt_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ef0_sgt_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ef8_sgt_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5ef9_sgt_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_GT() ? 0xff : 0);


}
void MC68000BASE::x5fd0_sle_b_ai_071234fc()
{
	write_8(EA_AY_AI_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5fd8_sle_b_pi_071234fc()
{
	write_8(EA_AY_PI_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5fdf_sle_b_pi7_071234fc()
{
	write_8(EA_A7_PI_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5fe0_sle_b_pd_071234fc()
{
	write_8(EA_AY_PD_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5fe7_sle_b_pd7_071234fc()
{
	write_8(EA_A7_PD_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5fe8_sle_b_di_071234fc()
{
	write_8(EA_AY_DI_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5ff0_sle_b_ix_071234fc()
{
	write_8(EA_AY_IX_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5ff8_sle_b_aw_071234fc()
{
	write_8(EA_AW_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x5ff9_sle_b_al_071234fc()
{
	write_8(EA_AL_8(), COND_LE() ? 0xff : 0);


}
void MC68000BASE::x4e72_stop_071234fc()
{
	if(m_s_flag) {
		uint32_t new_sr = OPER_I_16();
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_stopped |= STOP_LEVEL_STOP;
		m_signals |= SIG_MASK_M68K_STOPBYSW;
		set_sr(new_sr);
//		m_icount = 0;
	} else {
		m_proc_mode = (PROC_MODE_EXCEPTION_PRIVILEGE_VIOLATION);
	}


}
void MC68000BASE::x9000_sub_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9010_sub_b_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9018_sub_b_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x901f_sub_b_pi7_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9020_sub_b_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9027_sub_b_pd7_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9028_sub_b_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9030_sub_b_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9038_sub_b_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9039_sub_b_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x903a_sub_b_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x903b_sub_b_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x903c_sub_b_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9040_sub_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9048_sub_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9050_sub_w_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9058_sub_w_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9060_sub_w_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9068_sub_w_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9070_sub_w_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9078_sub_w_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9079_sub_w_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x907a_sub_w_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x907b_sub_w_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x907c_sub_w_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x9080_sub_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x9088_sub_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = AY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x9090_sub_l_ai_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x9098_sub_l_pi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90a0_sub_l_pd_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90a8_sub_l_di_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90b0_sub_l_ix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90b8_sub_l_aw_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90b9_sub_l_al_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90ba_sub_l_pcdi_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90bb_sub_l_pcix_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x90bc_sub_l_i_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x9110_sub_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9118_sub_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x911f_sub_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9120_sub_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9127_sub_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9128_sub_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9130_sub_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9138_sub_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9139_sub_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x9150_sub_w_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9158_sub_w_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9160_sub_w_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9168_sub_w_di_071234fc()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9170_sub_w_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9178_sub_w_aw_071234fc()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9179_sub_w_al_071234fc()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x9190_sub_l_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x9198_sub_l_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x91a0_sub_l_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x91a8_sub_l_di_071234fc()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x91b0_sub_l_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x91b8_sub_l_aw_071234fc()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x91b9_sub_l_al_071234fc()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = DX();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x90c0_suba_w_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY()));


}
void MC68000BASE::x90c8_suba_w_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY()));


}
void MC68000BASE::x90d0_suba_w_ai_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90d8_suba_w_pi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90e0_suba_w_pd_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90e8_suba_w_di_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90f0_suba_w_ix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90f8_suba_w_aw_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90f9_suba_w_al_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90fa_suba_w_pcdi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90fb_suba_w_pcix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x90fc_suba_w_i_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91c0_suba_l_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - DY());


}
void MC68000BASE::x91c8_suba_l_071234fc()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - AY());


}
void MC68000BASE::x91d0_suba_l_ai_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91d8_suba_l_pi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91e0_suba_l_pd_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91e8_suba_l_di_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91f0_suba_l_ix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91f8_suba_l_aw_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91f9_suba_l_al_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91fa_suba_l_pcdi_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91fb_suba_l_pcix_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x91fc_suba_l_i_071234fc()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void MC68000BASE::x0400_subi_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x0410_subi_b_ai_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0418_subi_b_pi_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x041f_subi_b_pi7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0420_subi_b_pd_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0427_subi_b_pd7_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0428_subi_b_di_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0430_subi_b_ix_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0438_subi_b_aw_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0439_subi_b_al_071234fc()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x0440_subi_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x0450_subi_w_ai_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0458_subi_w_pi_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0460_subi_w_pd_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0468_subi_w_di_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0470_subi_w_ix_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0478_subi_w_aw_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0479_subi_w_al_071234fc()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x0480_subi_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x0490_subi_l_ai_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x0498_subi_l_pi_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x04a0_subi_l_pd_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x04a8_subi_l_di_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x04b0_subi_l_ix_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x04b8_subi_l_aw_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x04b9_subi_l_al_071234fc()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x5100_subq_b_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x5110_subq_b_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5118_subq_b_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x511f_subq_b_pi7_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5120_subq_b_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5127_subq_b_pd7_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5128_subq_b_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5130_subq_b_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5138_subq_b_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5139_subq_b_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	write_8(ea, m_not_z_flag);


}
void MC68000BASE::x5140_subq_w_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void MC68000BASE::x5148_subq_w_071234fc()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));


}
void MC68000BASE::x5150_subq_w_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5158_subq_w_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5160_subq_w_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5168_subq_w_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5170_subq_w_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5178_subq_w_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5179_subq_w_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	write_16(ea, m_not_z_flag);


}
void MC68000BASE::x5180_subq_l_071234fc()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	*r_dst = m_not_z_flag;


}
void MC68000BASE::x5188_subq_l_071234fc()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));


}
void MC68000BASE::x5190_subq_l_ai_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x5198_subq_l_pi_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x51a0_subq_l_pd_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x51a8_subq_l_di_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x51b0_subq_l_ix_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x51b8_subq_l_aw_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x51b9_subq_l_al_071234fc()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	write_32(ea, m_not_z_flag);


}
void MC68000BASE::x9100_subx_b_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void MC68000BASE::x9140_subx_w_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void MC68000BASE::x9180_subx_l_071234fc()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void MC68000BASE::x9f08_subx_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x910f_subx_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x9f0f_subx_b_071234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x9108_subx_b_071234fc()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	write_8(ea, res);


}
void MC68000BASE::x9148_subx_w_071234fc()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t ea  = EA_AX_PD_16();
	uint32_t dst = read_16(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	write_16(ea, res);


}
void MC68000BASE::x9188_subx_l_071234fc()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t ea  = EA_AX_PD_32();
	uint32_t dst = read_32(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	write_32(ea, res);


}
void MC68000BASE::x4840_swap_l_071234fc()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_32(*r_dst<<16);
	*r_dst = (*r_dst>>16) | m_not_z_flag;

	m_not_z_flag = *r_dst;
	m_n_flag = NFLAG_32(*r_dst);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void MC68000BASE::x4ac0_tas_b_071234fc()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
	m_n_flag = NFLAG_8(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	*r_dst |= 0x80;


}
void MC68000BASE::x4ad0_tas_b_ai_071234fc()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4ad8_tas_b_pi_071234fc()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4adf_tas_b_pi7_071234fc()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4ae0_tas_b_pd_071234fc()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4ae7_tas_b_pd7_071234fc()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4ae8_tas_b_di_071234fc()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4af0_tas_b_ix_071234fc()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4af8_tas_b_aw_071234fc()
{
	uint32_t ea = EA_AW_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4af9_tas_b_al_071234fc()
{
	uint32_t ea = EA_AL_8();
	uint32_t dst = read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
//	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
//		(m_tas_write_callback)(ea, dst | 0x80);
//	else
		write_8(ea, dst | 0x80);


}
void MC68000BASE::x4e40_trap_071234fc()
{
	/* Trap#n stacks exception frame type 0 */
	m_proc_mode = (PROC_MODE_EXCEPTION_TRAPN | ((m_ir & 0xf) << 8));    /* HJB 990403 */


}
void MC68000BASE::x50fc_trapt_234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x50fa_trapt_w_234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x50fb_trapt_l_234fc()
{
	m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x51fc_trapf_234fc()
{


}
void MC68000BASE::x51fa_trapf_w_234fc()
{
	m_pc += 2;


}
void MC68000BASE::x51fb_trapf_l_234fc()
{
	m_pc += 4;


}
void MC68000BASE::x52fc_traphi_234fc()
{
	if(COND_HI())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x53fc_trapls_234fc()
{
	if(COND_LS())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x54fc_trapcc_234fc()
{
	if(COND_CC())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x55fc_trapcs_234fc()
{
	if(COND_CS())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x56fc_trapne_234fc()
{
	if(COND_NE())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x57fc_trapeq_234fc()
{
	if(COND_EQ())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x58fc_trapvc_234fc()
{
	if(COND_VC())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x59fc_trapvs_234fc()
{
	if(COND_VS())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5afc_trappl_234fc()
{
	if(COND_PL())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5bfc_trapmi_234fc()
{
	if(COND_MI())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5cfc_trapge_234fc()
{
	if(COND_GE())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5dfc_traplt_234fc()
{
	if(COND_LT())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5efc_trapgt_234fc()
{
	if(COND_GT())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x5ffc_traple_234fc()
{
	if(COND_LE())
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */


}
void MC68000BASE::x52fa_traphi_w_234fc()
{
	if(COND_HI()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x53fa_trapls_w_234fc()
{
	if(COND_LS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x54fa_trapcc_w_234fc()
{
	if(COND_CC()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x55fa_trapcs_w_234fc()
{
	if(COND_CS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x56fa_trapne_w_234fc()
{
	if(COND_NE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x57fa_trapeq_w_234fc()
{
	if(COND_EQ()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x58fa_trapvc_w_234fc()
{
	if(COND_VC()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x59fa_trapvs_w_234fc()
{
	if(COND_VS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5afa_trappl_w_234fc()
{
	if(COND_PL()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5bfa_trapmi_w_234fc()
{
	if(COND_MI()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5cfa_trapge_w_234fc()
{
	if(COND_GE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5dfa_traplt_w_234fc()
{
	if(COND_LT()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5efa_trapgt_w_234fc()
{
	if(COND_GT()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x5ffa_traple_w_234fc()
{
	if(COND_LE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 2;
	}


}
void MC68000BASE::x52fb_traphi_l_234fc()
{
	if(COND_HI()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x53fb_trapls_l_234fc()
{
	if(COND_LS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x54fb_trapcc_l_234fc()
{
	if(COND_CC()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x55fb_trapcs_l_234fc()
{
	if(COND_CS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x56fb_trapne_l_234fc()
{
	if(COND_NE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x57fb_trapeq_l_234fc()
{
	if(COND_EQ()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x58fb_trapvc_l_234fc()
{
	if(COND_VC()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x59fb_trapvs_l_234fc()
{
	if(COND_VS()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5afb_trappl_l_234fc()
{
	if(COND_PL()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5bfb_trapmi_l_234fc()
{
	if(COND_MI()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5cfb_trapge_l_234fc()
{
	if(COND_GE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5dfb_traplt_l_234fc()
{
	if(COND_LT()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5efb_trapgt_l_234fc()
{
	if(COND_GT()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x5ffb_traple_l_234fc()
{
	if(COND_LE()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	} else {
		m_pc += 4;
	}


}
void MC68000BASE::x4e76_trapv_071234fc()
{
	if(!COND_VC()) {
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);  /* HJB 990403 */
	}


}
void MC68000BASE::x4a00_tst_b_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a10_tst_b_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a18_tst_b_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a1f_tst_b_pi7_071234fc()
{
	uint32_t res = OPER_A7_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a20_tst_b_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a27_tst_b_pd7_071234fc()
{
	uint32_t res = OPER_A7_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a28_tst_b_di_071234fc()
{
	uint32_t res = OPER_AY_DI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a30_tst_b_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a38_tst_b_aw_071234fc()
{
	uint32_t res = OPER_AW_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a39_tst_b_al_071234fc()
{
	uint32_t res = OPER_AL_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a3a_tst_b_234fc()
{
	uint32_t res = OPER_PCDI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a3b_tst_b_234fc()
{
	uint32_t res = OPER_PCIX_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a3c_tst_b_234fc()
{
	uint32_t res = OPER_I_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a40_tst_w_071234fc()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a48_tst_w_234fc()
{
	uint32_t res = MAKE_INT_16(AY());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a50_tst_w_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a58_tst_w_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a60_tst_w_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a68_tst_w_di_071234fc()
{
	uint32_t res = OPER_AY_DI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a70_tst_w_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a78_tst_w_aw_071234fc()
{
	uint32_t res = OPER_AW_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a79_tst_w_al_071234fc()
{
	uint32_t res = OPER_AL_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a7a_tst_w_234fc()
{
	uint32_t res = OPER_PCDI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a7b_tst_w_234fc()
{
	uint32_t res = OPER_PCIX_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a7c_tst_w_234fc()
{
	uint32_t res = OPER_I_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a80_tst_l_071234fc()
{
	uint32_t res = DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a88_tst_l_234fc()
{
	uint32_t res = AY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a90_tst_l_ai_071234fc()
{
	uint32_t res = OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4a98_tst_l_pi_071234fc()
{
	uint32_t res = OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4aa0_tst_l_pd_071234fc()
{
	uint32_t res = OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4aa8_tst_l_di_071234fc()
{
	uint32_t res = OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4ab0_tst_l_ix_071234fc()
{
	uint32_t res = OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4ab8_tst_l_aw_071234fc()
{
	uint32_t res = OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4ab9_tst_l_al_071234fc()
{
	uint32_t res = OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4aba_tst_l_234fc()
{
	uint32_t res = OPER_PCDI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4abb_tst_l_234fc()
{
	uint32_t res = OPER_PCIX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4abc_tst_l_234fc()
{
	uint32_t res = OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void MC68000BASE::x4e5f_unlk_l_071234fc()
{
	REG_SP() = (read_32(REG_SP()));


}
void MC68000BASE::x4e58_unlk_l_071234fc()
{
	uint32_t* r_dst = &AY();

	REG_SP() = *r_dst;
	*r_dst = pull_32();


}
void MC68000BASE::x8180_unpk_w_234fc()
{
	/* Note: DX() and DY() are reversed in Motorola's docs */
	uint32_t src = DY();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | (((((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16()) & 0xffff);


}
void MC68000BASE::x8f88_unpk_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea_dst;

	src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
		ea_dst = EA_A7_PD_8();
	write_8(ea_dst, src & 0xff);
	ea_dst = EA_A7_PD_8();
	write_8(ea_dst, (src >> 8) & 0xff);


}
void MC68000BASE::x818f_unpk_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea_dst;

	src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
	ea_dst = EA_AX_PD_8();
	write_8(ea_dst, src & 0xff);
	ea_dst = EA_AX_PD_8();
	write_8(ea_dst, (src >> 8) & 0xff);


}
void MC68000BASE::x8f8f_unpk_w_234fc()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea_dst;

	src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
	ea_dst = EA_A7_PD_8();
	write_8(ea_dst, src & 0xff);
	ea_dst = EA_A7_PD_8();
	write_8(ea_dst, (src >> 8) & 0xff);


}
void MC68000BASE::x8188_unpk_w_234fc()
{
	/* Note: AX and AY are reversed in Motorola's docs */
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea_dst;

	src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
	ea_dst = EA_AX_PD_8();
	write_8(ea_dst, src & 0xff);
	ea_dst = EA_AX_PD_8();
	write_8(ea_dst, (src >> 8) & 0xff);


}
void MC68000BASE::xf400_cinv_l_4()
{
	uint16_t ir = m_ir;
	uint8_t cache = (ir >> 6) & 3;
	//  uint8_t scope = (ir >> 3) & 3;
	//  logging->out_logf(LOG_ERROR, "68040 %s: pc=%08x ir=%04x cache=%d scope=%d register=%d\n", ir & 0x0020 ? "cpush" : "cinv", m_ppc, ir, cache, scope, ir & 7);
	switch (cache) {
	case 1:
		// TODO: data cache
		break;
	case 2:
	case 3:
		// we invalidate/push the whole instruction cache
		ic_clear();
		break;
	default:
		m_proc_mode = (PROC_MODE_EXCEPTION_1111);
		break;
	}


}
void MC68000BASE::xf420_cpush_l_4()
{
	logging->out_logf(LOG_ERROR, "%s at %08x: called unimplemented instruction %04x (cpush)\n",
					get_class_name(), m_ppc, m_ir);


}
const MC68000BASE::opcode_handler_ptr MC68000BASE::m68k_handler_table[] =
{

	&MC68000BASE::xa000_1010_071234fc,
	&MC68000BASE::xf000_1111_071234fc,
	&MC68000BASE::x7000_moveq_l_071234fc,
	&MC68000BASE::xf080_cpbcc_l_23,
	&MC68000BASE::xf000_cpgen_l_23,
	&MC68000BASE::xf040_cpscc_l_23,
	&MC68000BASE::xf000_pmmu_l_234fc,
	&MC68000BASE::x6000_bra_b_071234fc,
	&MC68000BASE::x6100_bsr_b_071234fc,
	&MC68000BASE::x6200_bhi_b_071234fc,
	&MC68000BASE::x6300_bls_b_071234fc,
	&MC68000BASE::x6400_bcc_b_071234fc,
	&MC68000BASE::x6500_bcs_b_071234fc,
	&MC68000BASE::x6600_bne_b_071234fc,
	&MC68000BASE::x6700_beq_b_071234fc,
	&MC68000BASE::x6800_bvc_b_071234fc,
	&MC68000BASE::x6900_bvs_b_071234fc,
	&MC68000BASE::x6a00_bpl_b_071234fc,
	&MC68000BASE::x6b00_bmi_b_071234fc,
	&MC68000BASE::x6c00_bge_b_071234fc,
	&MC68000BASE::x6d00_blt_b_071234fc,
	&MC68000BASE::x6e00_bgt_b_071234fc,
	&MC68000BASE::x6f00_ble_b_071234fc,
	&MC68000BASE::xf200_040fpu0_l_234f,
	&MC68000BASE::xf300_040fpu1_l_234f,
	&MC68000BASE::xf400_cinv_l_4,
	&MC68000BASE::xf420_cpush_l_4,
	&MC68000BASE::x0100_btst_l_071234fc,
	&MC68000BASE::x0108_movep_w_071234fc,
	&MC68000BASE::x0110_btst_b_ai_071234fc,
	&MC68000BASE::x0118_btst_b_pi_071234fc,
	&MC68000BASE::x0120_btst_b_pd_071234fc,
	&MC68000BASE::x0128_btst_b_di_071234fc,
	&MC68000BASE::x0130_btst_b_ix_071234fc,
	&MC68000BASE::x0140_bchg_l_071234fc,
	&MC68000BASE::x0148_movep_l_071234fc,
	&MC68000BASE::x0150_bchg_b_ai_071234fc,
	&MC68000BASE::x0158_bchg_b_pi_071234fc,
	&MC68000BASE::x0160_bchg_b_pd_071234fc,
	&MC68000BASE::x0168_bchg_b_di_071234fc,
	&MC68000BASE::x0170_bchg_b_ix_071234fc,
	&MC68000BASE::x0180_bclr_l_071234fc,
	&MC68000BASE::x0188_movep_w_071234fc,
	&MC68000BASE::x0190_bclr_b_ai_071234fc,
	&MC68000BASE::x0198_bclr_b_pi_071234fc,
	&MC68000BASE::x01a0_bclr_b_pd_071234fc,
	&MC68000BASE::x01a8_bclr_b_di_071234fc,
	&MC68000BASE::x01b0_bclr_b_ix_071234fc,
	&MC68000BASE::x01c0_bset_l_071234fc,
	&MC68000BASE::x01c8_movep_l_071234fc,
	&MC68000BASE::x01d0_bset_b_ai_071234fc,
	&MC68000BASE::x01d8_bset_b_pi_071234fc,
	&MC68000BASE::x01e0_bset_b_pd_071234fc,
	&MC68000BASE::x01e8_bset_b_di_071234fc,
	&MC68000BASE::x01f0_bset_b_ix_071234fc,
	&MC68000BASE::x1000_move_b_071234fc,
	&MC68000BASE::x1010_move_b_ai_071234fc,
	&MC68000BASE::x1018_move_b_pi_071234fc,
	&MC68000BASE::x1020_move_b_pd_071234fc,
	&MC68000BASE::x1028_move_b_di_071234fc,
	&MC68000BASE::x1030_move_b_ix_071234fc,
	&MC68000BASE::x1080_move_b_071234fc,
	&MC68000BASE::x1090_move_b_ai_071234fc,
	&MC68000BASE::x1098_move_b_pi_071234fc,
	&MC68000BASE::x10a0_move_b_pd_071234fc,
	&MC68000BASE::x10a8_move_b_di_071234fc,
	&MC68000BASE::x10b0_move_b_ix_071234fc,
	&MC68000BASE::x10c0_move_b_071234fc,
	&MC68000BASE::x10d0_move_b_ai_071234fc,
	&MC68000BASE::x10d8_move_b_pi_071234fc,
	&MC68000BASE::x10e0_move_b_pd_071234fc,
	&MC68000BASE::x10e8_move_b_di_071234fc,
	&MC68000BASE::x10f0_move_b_ix_071234fc,
	&MC68000BASE::x1100_move_b_071234fc,
	&MC68000BASE::x1110_move_b_ai_071234fc,
	&MC68000BASE::x1118_move_b_pi_071234fc,
	&MC68000BASE::x1120_move_b_pd_071234fc,
	&MC68000BASE::x1128_move_b_di_071234fc,
	&MC68000BASE::x1130_move_b_ix_071234fc,
	&MC68000BASE::x1140_move_b_071234fc,
	&MC68000BASE::x1150_move_b_ai_071234fc,
	&MC68000BASE::x1158_move_b_pi_071234fc,
	&MC68000BASE::x1160_move_b_pd_071234fc,
	&MC68000BASE::x1168_move_b_di_071234fc,
	&MC68000BASE::x1170_move_b_ix_071234fc,
	&MC68000BASE::x1180_move_b_071234fc,
	&MC68000BASE::x1190_move_b_ai_071234fc,
	&MC68000BASE::x1198_move_b_pi_071234fc,
	&MC68000BASE::x11a0_move_b_pd_071234fc,
	&MC68000BASE::x11a8_move_b_di_071234fc,
	&MC68000BASE::x11b0_move_b_ix_071234fc,
	&MC68000BASE::x2000_move_l_071234fc,
	&MC68000BASE::x2008_move_l_071234fc,
	&MC68000BASE::x2010_move_l_ai_071234fc,
	&MC68000BASE::x2018_move_l_pi_071234fc,
	&MC68000BASE::x2020_move_l_pd_071234fc,
	&MC68000BASE::x2028_move_l_di_071234fc,
	&MC68000BASE::x2030_move_l_ix_071234fc,
	&MC68000BASE::x2040_movea_l_071234fc,
	&MC68000BASE::x2048_movea_l_071234fc,
	&MC68000BASE::x2050_movea_l_ai_071234fc,
	&MC68000BASE::x2058_movea_l_pi_071234fc,
	&MC68000BASE::x2060_movea_l_pd_071234fc,
	&MC68000BASE::x2068_movea_l_di_071234fc,
	&MC68000BASE::x2070_movea_l_ix_071234fc,
	&MC68000BASE::x2080_move_l_071234fc,
	&MC68000BASE::x2088_move_l_071234fc,
	&MC68000BASE::x2090_move_l_ai_071234fc,
	&MC68000BASE::x2098_move_l_pi_071234fc,
	&MC68000BASE::x20a0_move_l_pd_071234fc,
	&MC68000BASE::x20a8_move_l_di_071234fc,
	&MC68000BASE::x20b0_move_l_ix_071234fc,
	&MC68000BASE::x20c0_move_l_071234fc,
	&MC68000BASE::x20c8_move_l_071234fc,
	&MC68000BASE::x20d0_move_l_ai_071234fc,
	&MC68000BASE::x20d8_move_l_pi_071234fc,
	&MC68000BASE::x20e0_move_l_pd_071234fc,
	&MC68000BASE::x20e8_move_l_di_071234fc,
	&MC68000BASE::x20f0_move_l_ix_071234fc,
	&MC68000BASE::x2100_move_l_071234fc,
	&MC68000BASE::x2108_move_l_071234fc,
	&MC68000BASE::x2110_move_l_ai_071234fc,
	&MC68000BASE::x2118_move_l_pi_071234fc,
	&MC68000BASE::x2120_move_l_pd_071234fc,
	&MC68000BASE::x2128_move_l_di_071234fc,
	&MC68000BASE::x2130_move_l_ix_071234fc,
	&MC68000BASE::x2140_move_l_071234fc,
	&MC68000BASE::x2148_move_l_071234fc,
	&MC68000BASE::x2150_move_l_ai_071234fc,
	&MC68000BASE::x2158_move_l_pi_071234fc,
	&MC68000BASE::x2160_move_l_pd_071234fc,
	&MC68000BASE::x2168_move_l_di_071234fc,
	&MC68000BASE::x2170_move_l_ix_071234fc,
	&MC68000BASE::x2180_move_l_071234fc,
	&MC68000BASE::x2188_move_l_071234fc,
	&MC68000BASE::x2190_move_l_ai_071234fc,
	&MC68000BASE::x2198_move_l_pi_071234fc,
	&MC68000BASE::x21a0_move_l_pd_071234fc,
	&MC68000BASE::x21a8_move_l_di_071234fc,
	&MC68000BASE::x21b0_move_l_ix_071234fc,
	&MC68000BASE::x3000_move_w_071234fc,
	&MC68000BASE::x3008_move_w_071234fc,
	&MC68000BASE::x3010_move_w_ai_071234fc,
	&MC68000BASE::x3018_move_w_pi_071234fc,
	&MC68000BASE::x3020_move_w_pd_071234fc,
	&MC68000BASE::x3028_move_w_di_071234fc,
	&MC68000BASE::x3030_move_w_ix_071234fc,
	&MC68000BASE::x3040_movea_w_071234fc,
	&MC68000BASE::x3048_movea_w_071234fc,
	&MC68000BASE::x3050_movea_w_ai_071234fc,
	&MC68000BASE::x3058_movea_w_pi_071234fc,
	&MC68000BASE::x3060_movea_w_pd_071234fc,
	&MC68000BASE::x3068_movea_w_di_071234fc,
	&MC68000BASE::x3070_movea_w_ix_071234fc,
	&MC68000BASE::x3080_move_w_071234fc,
	&MC68000BASE::x3088_move_w_071234fc,
	&MC68000BASE::x3090_move_w_ai_071234fc,
	&MC68000BASE::x3098_move_w_pi_071234fc,
	&MC68000BASE::x30a0_move_w_pd_071234fc,
	&MC68000BASE::x30a8_move_w_di_071234fc,
	&MC68000BASE::x30b0_move_w_ix_071234fc,
	&MC68000BASE::x30c0_move_w_071234fc,
	&MC68000BASE::x30c8_move_w_071234fc,
	&MC68000BASE::x30d0_move_w_ai_071234fc,
	&MC68000BASE::x30d8_move_w_pi_071234fc,
	&MC68000BASE::x30e0_move_w_pd_071234fc,
	&MC68000BASE::x30e8_move_w_di_071234fc,
	&MC68000BASE::x30f0_move_w_ix_071234fc,
	&MC68000BASE::x3100_move_w_071234fc,
	&MC68000BASE::x3108_move_w_071234fc,
	&MC68000BASE::x3110_move_w_ai_071234fc,
	&MC68000BASE::x3118_move_w_pi_071234fc,
	&MC68000BASE::x3120_move_w_pd_071234fc,
	&MC68000BASE::x3128_move_w_di_071234fc,
	&MC68000BASE::x3130_move_w_ix_071234fc,
	&MC68000BASE::x3140_move_w_071234fc,
	&MC68000BASE::x3148_move_w_071234fc,
	&MC68000BASE::x3150_move_w_ai_071234fc,
	&MC68000BASE::x3158_move_w_pi_071234fc,
	&MC68000BASE::x3160_move_w_pd_071234fc,
	&MC68000BASE::x3168_move_w_di_071234fc,
	&MC68000BASE::x3170_move_w_ix_071234fc,
	&MC68000BASE::x3180_move_w_071234fc,
	&MC68000BASE::x3188_move_w_071234fc,
	&MC68000BASE::x3190_move_w_ai_071234fc,
	&MC68000BASE::x3198_move_w_pi_071234fc,
	&MC68000BASE::x31a0_move_w_pd_071234fc,
	&MC68000BASE::x31a8_move_w_di_071234fc,
	&MC68000BASE::x31b0_move_w_ix_071234fc,
	&MC68000BASE::x4100_chk_l_234fc,
	&MC68000BASE::x4110_chk_l_ai_234fc,
	&MC68000BASE::x4118_chk_l_pi_234fc,
	&MC68000BASE::x4120_chk_l_pd_234fc,
	&MC68000BASE::x4128_chk_l_di_234fc,
	&MC68000BASE::x4130_chk_l_ix_234fc,
	&MC68000BASE::x4180_chk_w_071234fc,
	&MC68000BASE::x4190_chk_w_ai_071234fc,
	&MC68000BASE::x4198_chk_w_pi_071234fc,
	&MC68000BASE::x41a0_chk_w_pd_071234fc,
	&MC68000BASE::x41a8_chk_w_di_071234fc,
	&MC68000BASE::x41b0_chk_w_ix_071234fc,
	&MC68000BASE::x41d0_lea_l_ai_071234fc,
	&MC68000BASE::x41e8_lea_l_di_071234fc,
	&MC68000BASE::x41f0_lea_l_ix_071234fc,
	&MC68000BASE::x5000_addq_b_071234fc,
	&MC68000BASE::x5010_addq_b_ai_071234fc,
	&MC68000BASE::x5018_addq_b_pi_071234fc,
	&MC68000BASE::x5020_addq_b_pd_071234fc,
	&MC68000BASE::x5028_addq_b_di_071234fc,
	&MC68000BASE::x5030_addq_b_ix_071234fc,
	&MC68000BASE::x5040_addq_w_071234fc,
	&MC68000BASE::x5048_addq_w_071234fc,
	&MC68000BASE::x5050_addq_w_ai_071234fc,
	&MC68000BASE::x5058_addq_w_pi_071234fc,
	&MC68000BASE::x5060_addq_w_pd_071234fc,
	&MC68000BASE::x5068_addq_w_di_071234fc,
	&MC68000BASE::x5070_addq_w_ix_071234fc,
	&MC68000BASE::x5080_addq_l_071234fc,
	&MC68000BASE::x5088_addq_l_071234fc,
	&MC68000BASE::x5090_addq_l_ai_071234fc,
	&MC68000BASE::x5098_addq_l_pi_071234fc,
	&MC68000BASE::x50a0_addq_l_pd_071234fc,
	&MC68000BASE::x50a8_addq_l_di_071234fc,
	&MC68000BASE::x50b0_addq_l_ix_071234fc,
	&MC68000BASE::x5100_subq_b_071234fc,
	&MC68000BASE::x5110_subq_b_ai_071234fc,
	&MC68000BASE::x5118_subq_b_pi_071234fc,
	&MC68000BASE::x5120_subq_b_pd_071234fc,
	&MC68000BASE::x5128_subq_b_di_071234fc,
	&MC68000BASE::x5130_subq_b_ix_071234fc,
	&MC68000BASE::x5140_subq_w_071234fc,
	&MC68000BASE::x5148_subq_w_071234fc,
	&MC68000BASE::x5150_subq_w_ai_071234fc,
	&MC68000BASE::x5158_subq_w_pi_071234fc,
	&MC68000BASE::x5160_subq_w_pd_071234fc,
	&MC68000BASE::x5168_subq_w_di_071234fc,
	&MC68000BASE::x5170_subq_w_ix_071234fc,
	&MC68000BASE::x5180_subq_l_071234fc,
	&MC68000BASE::x5188_subq_l_071234fc,
	&MC68000BASE::x5190_subq_l_ai_071234fc,
	&MC68000BASE::x5198_subq_l_pi_071234fc,
	&MC68000BASE::x51a0_subq_l_pd_071234fc,
	&MC68000BASE::x51a8_subq_l_di_071234fc,
	&MC68000BASE::x51b0_subq_l_ix_071234fc,
	&MC68000BASE::x8000_or_b_071234fc,
	&MC68000BASE::x8010_or_b_ai_071234fc,
	&MC68000BASE::x8018_or_b_pi_071234fc,
	&MC68000BASE::x8020_or_b_pd_071234fc,
	&MC68000BASE::x8028_or_b_di_071234fc,
	&MC68000BASE::x8030_or_b_ix_071234fc,
	&MC68000BASE::x8040_or_w_071234fc,
	&MC68000BASE::x8050_or_w_ai_071234fc,
	&MC68000BASE::x8058_or_w_pi_071234fc,
	&MC68000BASE::x8060_or_w_pd_071234fc,
	&MC68000BASE::x8068_or_w_di_071234fc,
	&MC68000BASE::x8070_or_w_ix_071234fc,
	&MC68000BASE::x8080_or_l_071234fc,
	&MC68000BASE::x8090_or_l_ai_071234fc,
	&MC68000BASE::x8098_or_l_pi_071234fc,
	&MC68000BASE::x80a0_or_l_pd_071234fc,
	&MC68000BASE::x80a8_or_l_di_071234fc,
	&MC68000BASE::x80b0_or_l_ix_071234fc,
	&MC68000BASE::x80c0_divu_w_071234fc,
	&MC68000BASE::x80d0_divu_w_ai_071234fc,
	&MC68000BASE::x80d8_divu_w_pi_071234fc,
	&MC68000BASE::x80e0_divu_w_pd_071234fc,
	&MC68000BASE::x80e8_divu_w_di_071234fc,
	&MC68000BASE::x80f0_divu_w_ix_071234fc,
	&MC68000BASE::x8100_sbcd_b_071234fc,
	&MC68000BASE::x8108_sbcd_b_071234fc,
	&MC68000BASE::x8110_or_b_ai_071234fc,
	&MC68000BASE::x8118_or_b_pi_071234fc,
	&MC68000BASE::x8120_or_b_pd_071234fc,
	&MC68000BASE::x8128_or_b_di_071234fc,
	&MC68000BASE::x8130_or_b_ix_071234fc,
	&MC68000BASE::x8140_pack_w_234fc,
	&MC68000BASE::x8148_pack_w_234fc,
	&MC68000BASE::x8150_or_w_ai_071234fc,
	&MC68000BASE::x8158_or_w_pi_071234fc,
	&MC68000BASE::x8160_or_w_pd_071234fc,
	&MC68000BASE::x8168_or_w_di_071234fc,
	&MC68000BASE::x8170_or_w_ix_071234fc,
	&MC68000BASE::x8180_unpk_w_234fc,
	&MC68000BASE::x8188_unpk_w_234fc,
	&MC68000BASE::x8190_or_l_ai_071234fc,
	&MC68000BASE::x8198_or_l_pi_071234fc,
	&MC68000BASE::x81a0_or_l_pd_071234fc,
	&MC68000BASE::x81a8_or_l_di_071234fc,
	&MC68000BASE::x81b0_or_l_ix_071234fc,
	&MC68000BASE::x81c0_divs_w_071234fc,
	&MC68000BASE::x81d0_divs_w_ai_071234fc,
	&MC68000BASE::x81d8_divs_w_pi_071234fc,
	&MC68000BASE::x81e0_divs_w_pd_071234fc,
	&MC68000BASE::x81e8_divs_w_di_071234fc,
	&MC68000BASE::x81f0_divs_w_ix_071234fc,
	&MC68000BASE::x9000_sub_b_071234fc,
	&MC68000BASE::x9010_sub_b_ai_071234fc,
	&MC68000BASE::x9018_sub_b_pi_071234fc,
	&MC68000BASE::x9020_sub_b_pd_071234fc,
	&MC68000BASE::x9028_sub_b_di_071234fc,
	&MC68000BASE::x9030_sub_b_ix_071234fc,
	&MC68000BASE::x9040_sub_w_071234fc,
	&MC68000BASE::x9048_sub_w_071234fc,
	&MC68000BASE::x9050_sub_w_ai_071234fc,
	&MC68000BASE::x9058_sub_w_pi_071234fc,
	&MC68000BASE::x9060_sub_w_pd_071234fc,
	&MC68000BASE::x9068_sub_w_di_071234fc,
	&MC68000BASE::x9070_sub_w_ix_071234fc,
	&MC68000BASE::x9080_sub_l_071234fc,
	&MC68000BASE::x9088_sub_l_071234fc,
	&MC68000BASE::x9090_sub_l_ai_071234fc,
	&MC68000BASE::x9098_sub_l_pi_071234fc,
	&MC68000BASE::x90a0_sub_l_pd_071234fc,
	&MC68000BASE::x90a8_sub_l_di_071234fc,
	&MC68000BASE::x90b0_sub_l_ix_071234fc,
	&MC68000BASE::x90c0_suba_w_071234fc,
	&MC68000BASE::x90c8_suba_w_071234fc,
	&MC68000BASE::x90d0_suba_w_ai_071234fc,
	&MC68000BASE::x90d8_suba_w_pi_071234fc,
	&MC68000BASE::x90e0_suba_w_pd_071234fc,
	&MC68000BASE::x90e8_suba_w_di_071234fc,
	&MC68000BASE::x90f0_suba_w_ix_071234fc,
	&MC68000BASE::x9100_subx_b_071234fc,
	&MC68000BASE::x9108_subx_b_071234fc,
	&MC68000BASE::x9110_sub_b_ai_071234fc,
	&MC68000BASE::x9118_sub_b_pi_071234fc,
	&MC68000BASE::x9120_sub_b_pd_071234fc,
	&MC68000BASE::x9128_sub_b_di_071234fc,
	&MC68000BASE::x9130_sub_b_ix_071234fc,
	&MC68000BASE::x9140_subx_w_071234fc,
	&MC68000BASE::x9148_subx_w_071234fc,
	&MC68000BASE::x9150_sub_w_ai_071234fc,
	&MC68000BASE::x9158_sub_w_pi_071234fc,
	&MC68000BASE::x9160_sub_w_pd_071234fc,
	&MC68000BASE::x9168_sub_w_di_071234fc,
	&MC68000BASE::x9170_sub_w_ix_071234fc,
	&MC68000BASE::x9180_subx_l_071234fc,
	&MC68000BASE::x9188_subx_l_071234fc,
	&MC68000BASE::x9190_sub_l_ai_071234fc,
	&MC68000BASE::x9198_sub_l_pi_071234fc,
	&MC68000BASE::x91a0_sub_l_pd_071234fc,
	&MC68000BASE::x91a8_sub_l_di_071234fc,
	&MC68000BASE::x91b0_sub_l_ix_071234fc,
	&MC68000BASE::x91c0_suba_l_071234fc,
	&MC68000BASE::x91c8_suba_l_071234fc,
	&MC68000BASE::x91d0_suba_l_ai_071234fc,
	&MC68000BASE::x91d8_suba_l_pi_071234fc,
	&MC68000BASE::x91e0_suba_l_pd_071234fc,
	&MC68000BASE::x91e8_suba_l_di_071234fc,
	&MC68000BASE::x91f0_suba_l_ix_071234fc,
	&MC68000BASE::xb000_cmp_b_071234fc,
	&MC68000BASE::xb010_cmp_b_ai_071234fc,
	&MC68000BASE::xb018_cmp_b_pi_071234fc,
	&MC68000BASE::xb020_cmp_b_pd_071234fc,
	&MC68000BASE::xb028_cmp_b_di_071234fc,
	&MC68000BASE::xb030_cmp_b_ix_071234fc,
	&MC68000BASE::xb040_cmp_w_071234fc,
	&MC68000BASE::xb048_cmp_w_071234fc,
	&MC68000BASE::xb050_cmp_w_ai_071234fc,
	&MC68000BASE::xb058_cmp_w_pi_071234fc,
	&MC68000BASE::xb060_cmp_w_pd_071234fc,
	&MC68000BASE::xb068_cmp_w_di_071234fc,
	&MC68000BASE::xb070_cmp_w_ix_071234fc,
	&MC68000BASE::xb080_cmp_l_071234fc,
	&MC68000BASE::xb088_cmp_l_071234fc,
	&MC68000BASE::xb090_cmp_l_ai_071234fc,
	&MC68000BASE::xb098_cmp_l_pi_071234fc,
	&MC68000BASE::xb0a0_cmp_l_pd_071234fc,
	&MC68000BASE::xb0a8_cmp_l_di_071234fc,
	&MC68000BASE::xb0b0_cmp_l_ix_071234fc,
	&MC68000BASE::xb0c0_cmpa_w_071234fc,
	&MC68000BASE::xb0c8_cmpa_w_071234fc,
	&MC68000BASE::xb0d0_cmpa_w_ai_071234fc,
	&MC68000BASE::xb0d8_cmpa_w_pi_071234fc,
	&MC68000BASE::xb0e0_cmpa_w_pd_071234fc,
	&MC68000BASE::xb0e8_cmpa_w_di_071234fc,
	&MC68000BASE::xb0f0_cmpa_w_ix_071234fc,
	&MC68000BASE::xb100_eor_b_071234fc,
	&MC68000BASE::xb108_cmpm_b_071234fc,
	&MC68000BASE::xb110_eor_b_ai_071234fc,
	&MC68000BASE::xb118_eor_b_pi_071234fc,
	&MC68000BASE::xb120_eor_b_pd_071234fc,
	&MC68000BASE::xb128_eor_b_di_071234fc,
	&MC68000BASE::xb130_eor_b_ix_071234fc,
	&MC68000BASE::xb140_eor_w_071234fc,
	&MC68000BASE::xb148_cmpm_w_071234fc,
	&MC68000BASE::xb150_eor_w_ai_071234fc,
	&MC68000BASE::xb158_eor_w_pi_071234fc,
	&MC68000BASE::xb160_eor_w_pd_071234fc,
	&MC68000BASE::xb168_eor_w_di_071234fc,
	&MC68000BASE::xb170_eor_w_ix_071234fc,
	&MC68000BASE::xb180_eor_l_071234fc,
	&MC68000BASE::xb188_cmpm_l_071234fc,
	&MC68000BASE::xb190_eor_l_ai_071234fc,
	&MC68000BASE::xb198_eor_l_pi_071234fc,
	&MC68000BASE::xb1a0_eor_l_pd_071234fc,
	&MC68000BASE::xb1a8_eor_l_di_071234fc,
	&MC68000BASE::xb1b0_eor_l_ix_071234fc,
	&MC68000BASE::xb1c0_cmpa_l_071234fc,
	&MC68000BASE::xb1c8_cmpa_l_071234fc,
	&MC68000BASE::xb1d0_cmpa_l_ai_071234fc,
	&MC68000BASE::xb1d8_cmpa_l_pi_071234fc,
	&MC68000BASE::xb1e0_cmpa_l_pd_071234fc,
	&MC68000BASE::xb1e8_cmpa_l_di_071234fc,
	&MC68000BASE::xb1f0_cmpa_l_ix_071234fc,
	&MC68000BASE::xc000_and_b_071234fc,
	&MC68000BASE::xc010_and_b_ai_071234fc,
	&MC68000BASE::xc018_and_b_pi_071234fc,
	&MC68000BASE::xc020_and_b_pd_071234fc,
	&MC68000BASE::xc028_and_b_di_071234fc,
	&MC68000BASE::xc030_and_b_ix_071234fc,
	&MC68000BASE::xc040_and_w_071234fc,
	&MC68000BASE::xc050_and_w_ai_071234fc,
	&MC68000BASE::xc058_and_w_pi_071234fc,
	&MC68000BASE::xc060_and_w_pd_071234fc,
	&MC68000BASE::xc068_and_w_di_071234fc,
	&MC68000BASE::xc070_and_w_ix_071234fc,
	&MC68000BASE::xc080_and_l_071234fc,
	&MC68000BASE::xc090_and_l_ai_071234fc,
	&MC68000BASE::xc098_and_l_pi_071234fc,
	&MC68000BASE::xc0a0_and_l_pd_071234fc,
	&MC68000BASE::xc0a8_and_l_di_071234fc,
	&MC68000BASE::xc0b0_and_l_ix_071234fc,
	&MC68000BASE::xc0c0_mulu_w_071234fc,
	&MC68000BASE::xc0d0_mulu_w_ai_071234fc,
	&MC68000BASE::xc0d8_mulu_w_pi_071234fc,
	&MC68000BASE::xc0e0_mulu_w_pd_071234fc,
	&MC68000BASE::xc0e8_mulu_w_di_071234fc,
	&MC68000BASE::xc0f0_mulu_w_ix_071234fc,
	&MC68000BASE::xc100_abcd_b_071234fc,
	&MC68000BASE::xc108_abcd_b_071234fc,
	&MC68000BASE::xc110_and_b_ai_071234fc,
	&MC68000BASE::xc118_and_b_pi_071234fc,
	&MC68000BASE::xc120_and_b_pd_071234fc,
	&MC68000BASE::xc128_and_b_di_071234fc,
	&MC68000BASE::xc130_and_b_ix_071234fc,
	&MC68000BASE::xc140_exg_l_071234fc,
	&MC68000BASE::xc148_exg_l_071234fc,
	&MC68000BASE::xc150_and_w_ai_071234fc,
	&MC68000BASE::xc158_and_w_pi_071234fc,
	&MC68000BASE::xc160_and_w_pd_071234fc,
	&MC68000BASE::xc168_and_w_di_071234fc,
	&MC68000BASE::xc170_and_w_ix_071234fc,
	&MC68000BASE::xc188_exg_l_071234fc,
	&MC68000BASE::xc190_and_l_ai_071234fc,
	&MC68000BASE::xc198_and_l_pi_071234fc,
	&MC68000BASE::xc1a0_and_l_pd_071234fc,
	&MC68000BASE::xc1a8_and_l_di_071234fc,
	&MC68000BASE::xc1b0_and_l_ix_071234fc,
	&MC68000BASE::xc1c0_muls_w_071234fc,
	&MC68000BASE::xc1d0_muls_w_ai_071234fc,
	&MC68000BASE::xc1d8_muls_w_pi_071234fc,
	&MC68000BASE::xc1e0_muls_w_pd_071234fc,
	&MC68000BASE::xc1e8_muls_w_di_071234fc,
	&MC68000BASE::xc1f0_muls_w_ix_071234fc,
	&MC68000BASE::xd000_add_b_071234fc,
	&MC68000BASE::xd010_add_b_ai_071234fc,
	&MC68000BASE::xd018_add_b_pi_071234fc,
	&MC68000BASE::xd020_add_b_pd_071234fc,
	&MC68000BASE::xd028_add_b_di_071234fc,
	&MC68000BASE::xd030_add_b_ix_071234fc,
	&MC68000BASE::xd040_add_w_071234fc,
	&MC68000BASE::xd048_add_w_071234fc,
	&MC68000BASE::xd050_add_w_ai_071234fc,
	&MC68000BASE::xd058_add_w_pi_071234fc,
	&MC68000BASE::xd060_add_w_pd_071234fc,
	&MC68000BASE::xd068_add_w_di_071234fc,
	&MC68000BASE::xd070_add_w_ix_071234fc,
	&MC68000BASE::xd080_add_l_071234fc,
	&MC68000BASE::xd088_add_l_071234fc,
	&MC68000BASE::xd090_add_l_ai_071234fc,
	&MC68000BASE::xd098_add_l_pi_071234fc,
	&MC68000BASE::xd0a0_add_l_pd_071234fc,
	&MC68000BASE::xd0a8_add_l_di_071234fc,
	&MC68000BASE::xd0b0_add_l_ix_071234fc,
	&MC68000BASE::xd0c0_adda_w_071234fc,
	&MC68000BASE::xd0c8_adda_w_071234fc,
	&MC68000BASE::xd0d0_adda_w_ai_071234fc,
	&MC68000BASE::xd0d8_adda_w_pi_071234fc,
	&MC68000BASE::xd0e0_adda_w_pd_071234fc,
	&MC68000BASE::xd0e8_adda_w_di_071234fc,
	&MC68000BASE::xd0f0_adda_w_ix_071234fc,
	&MC68000BASE::xd100_addx_b_071234fc,
	&MC68000BASE::xd108_addx_b_071234fc,
	&MC68000BASE::xd110_add_b_ai_071234fc,
	&MC68000BASE::xd118_add_b_pi_071234fc,
	&MC68000BASE::xd120_add_b_pd_071234fc,
	&MC68000BASE::xd128_add_b_di_071234fc,
	&MC68000BASE::xd130_add_b_ix_071234fc,
	&MC68000BASE::xd140_addx_w_071234fc,
	&MC68000BASE::xd148_addx_w_071234fc,
	&MC68000BASE::xd150_add_w_ai_071234fc,
	&MC68000BASE::xd158_add_w_pi_071234fc,
	&MC68000BASE::xd160_add_w_pd_071234fc,
	&MC68000BASE::xd168_add_w_di_071234fc,
	&MC68000BASE::xd170_add_w_ix_071234fc,
	&MC68000BASE::xd180_addx_l_071234fc,
	&MC68000BASE::xd188_addx_l_071234fc,
	&MC68000BASE::xd190_add_l_ai_071234fc,
	&MC68000BASE::xd198_add_l_pi_071234fc,
	&MC68000BASE::xd1a0_add_l_pd_071234fc,
	&MC68000BASE::xd1a8_add_l_di_071234fc,
	&MC68000BASE::xd1b0_add_l_ix_071234fc,
	&MC68000BASE::xd1c0_adda_l_071234fc,
	&MC68000BASE::xd1c8_adda_l_071234fc,
	&MC68000BASE::xd1d0_adda_l_ai_071234fc,
	&MC68000BASE::xd1d8_adda_l_pi_071234fc,
	&MC68000BASE::xd1e0_adda_l_pd_071234fc,
	&MC68000BASE::xd1e8_adda_l_di_071234fc,
	&MC68000BASE::xd1f0_adda_l_ix_071234fc,
	&MC68000BASE::xe000_asr_b_071234fc,
	&MC68000BASE::xe008_lsr_b_071234fc,
	&MC68000BASE::xe010_roxr_b_071234fc,
	&MC68000BASE::xe018_ror_b_071234fc,
	&MC68000BASE::xe020_asr_b_071234fc,
	&MC68000BASE::xe028_lsr_b_071234fc,
	&MC68000BASE::xe030_roxr_b_071234fc,
	&MC68000BASE::xe038_ror_b_071234fc,
	&MC68000BASE::xe040_asr_w_071234fc,
	&MC68000BASE::xe048_lsr_w_071234fc,
	&MC68000BASE::xe050_roxr_w_071234fc,
	&MC68000BASE::xe058_ror_w_071234fc,
	&MC68000BASE::xe060_asr_w_071234fc,
	&MC68000BASE::xe068_lsr_w_071234fc,
	&MC68000BASE::xe070_roxr_w_071234fc,
	&MC68000BASE::xe078_ror_w_071234fc,
	&MC68000BASE::xe080_asr_l_071234fc,
	&MC68000BASE::xe088_lsr_l_071234fc,
	&MC68000BASE::xe090_roxr_l_071234fc,
	&MC68000BASE::xe098_ror_l_071234fc,
	&MC68000BASE::xe0a0_asr_l_071234fc,
	&MC68000BASE::xe0a8_lsr_l_071234fc,
	&MC68000BASE::xe0b0_roxr_l_071234fc,
	&MC68000BASE::xe0b8_ror_l_071234fc,
	&MC68000BASE::xe100_asl_b_071234fc,
	&MC68000BASE::xe108_lsl_b_071234fc,
	&MC68000BASE::xe110_roxl_b_071234fc,
	&MC68000BASE::xe118_rol_b_071234fc,
	&MC68000BASE::xe120_asl_b_071234fc,
	&MC68000BASE::xe128_lsl_b_071234fc,
	&MC68000BASE::xe130_roxl_b_071234fc,
	&MC68000BASE::xe138_rol_b_071234fc,
	&MC68000BASE::xe140_asl_w_071234fc,
	&MC68000BASE::xe148_lsl_w_071234fc,
	&MC68000BASE::xe150_roxl_w_071234fc,
	&MC68000BASE::xe158_rol_w_071234fc,
	&MC68000BASE::xe160_asl_w_071234fc,
	&MC68000BASE::xe168_lsl_w_071234fc,
	&MC68000BASE::xe170_roxl_w_071234fc,
	&MC68000BASE::xe178_rol_w_071234fc,
	&MC68000BASE::xe180_asl_l_071234fc,
	&MC68000BASE::xe188_lsl_l_071234fc,
	&MC68000BASE::xe190_roxl_l_071234fc,
	&MC68000BASE::xe198_rol_l_071234fc,
	&MC68000BASE::xe1a0_asl_l_071234fc,
	&MC68000BASE::xe1a8_lsl_l_071234fc,
	&MC68000BASE::xe1b0_roxl_l_071234fc,
	&MC68000BASE::xe1b8_rol_l_071234fc,
	&MC68000BASE::xf048_cpdbcc_l_23,
	&MC68000BASE::xf078_cptrapcc_l_23,
	&MC68000BASE::xf548_ptest_l_4,
	&MC68000BASE::x06c0_rtm_l_234fc,
	&MC68000BASE::x4e40_trap_071234fc,
	&MC68000BASE::x011f_btst_b_pi7_071234fc,
	&MC68000BASE::x0127_btst_b_pd7_071234fc,
	&MC68000BASE::x0138_btst_b_aw_071234fc,
	&MC68000BASE::x0139_btst_b_al_071234fc,
	&MC68000BASE::x013a_btst_b_pcdi_071234fc,
	&MC68000BASE::x013b_btst_b_pcix_071234fc,
	&MC68000BASE::x013c_btst_b_i_071234fc,
	&MC68000BASE::x015f_bchg_b_pi7_071234fc,
	&MC68000BASE::x0167_bchg_b_pd7_071234fc,
	&MC68000BASE::x0178_bchg_b_aw_071234fc,
	&MC68000BASE::x0179_bchg_b_al_071234fc,
	&MC68000BASE::x019f_bclr_b_pi7_071234fc,
	&MC68000BASE::x01a7_bclr_b_pd7_071234fc,
	&MC68000BASE::x01b8_bclr_b_aw_071234fc,
	&MC68000BASE::x01b9_bclr_b_al_071234fc,
	&MC68000BASE::x01df_bset_b_pi7_071234fc,
	&MC68000BASE::x01e7_bset_b_pd7_071234fc,
	&MC68000BASE::x01f8_bset_b_aw_071234fc,
	&MC68000BASE::x01f9_bset_b_al_071234fc,
	&MC68000BASE::x101f_move_b_pi7_071234fc,
	&MC68000BASE::x1027_move_b_pd7_071234fc,
	&MC68000BASE::x1038_move_b_aw_071234fc,
	&MC68000BASE::x1039_move_b_al_071234fc,
	&MC68000BASE::x103a_move_b_pcdi_071234fc,
	&MC68000BASE::x103b_move_b_pcix_071234fc,
	&MC68000BASE::x103c_move_b_i_071234fc,
	&MC68000BASE::x109f_move_b_pi7_071234fc,
	&MC68000BASE::x10a7_move_b_pd7_071234fc,
	&MC68000BASE::x10b8_move_b_aw_071234fc,
	&MC68000BASE::x10b9_move_b_al_071234fc,
	&MC68000BASE::x10ba_move_b_pcdi_071234fc,
	&MC68000BASE::x10bb_move_b_pcix_071234fc,
	&MC68000BASE::x10bc_move_b_i_071234fc,
	&MC68000BASE::x10df_move_b_pi7_071234fc,
	&MC68000BASE::x10e7_move_b_pd7_071234fc,
	&MC68000BASE::x10f8_move_b_aw_071234fc,
	&MC68000BASE::x10f9_move_b_al_071234fc,
	&MC68000BASE::x10fa_move_b_pcdi_071234fc,
	&MC68000BASE::x10fb_move_b_pcix_071234fc,
	&MC68000BASE::x10fc_move_b_i_071234fc,
	&MC68000BASE::x111f_move_b_pi7_071234fc,
	&MC68000BASE::x1127_move_b_pd7_071234fc,
	&MC68000BASE::x1138_move_b_aw_071234fc,
	&MC68000BASE::x1139_move_b_al_071234fc,
	&MC68000BASE::x113a_move_b_pcdi_071234fc,
	&MC68000BASE::x113b_move_b_pcix_071234fc,
	&MC68000BASE::x113c_move_b_i_071234fc,
	&MC68000BASE::x115f_move_b_pi7_071234fc,
	&MC68000BASE::x1167_move_b_pd7_071234fc,
	&MC68000BASE::x1178_move_b_aw_071234fc,
	&MC68000BASE::x1179_move_b_al_071234fc,
	&MC68000BASE::x117a_move_b_pcdi_071234fc,
	&MC68000BASE::x117b_move_b_pcix_071234fc,
	&MC68000BASE::x117c_move_b_i_071234fc,
	&MC68000BASE::x119f_move_b_pi7_071234fc,
	&MC68000BASE::x11a7_move_b_pd7_071234fc,
	&MC68000BASE::x11b8_move_b_aw_071234fc,
	&MC68000BASE::x11b9_move_b_al_071234fc,
	&MC68000BASE::x11ba_move_b_pcdi_071234fc,
	&MC68000BASE::x11bb_move_b_pcix_071234fc,
	&MC68000BASE::x11bc_move_b_i_071234fc,
	&MC68000BASE::x2038_move_l_aw_071234fc,
	&MC68000BASE::x2039_move_l_al_071234fc,
	&MC68000BASE::x203a_move_l_pcdi_071234fc,
	&MC68000BASE::x203b_move_l_pcix_071234fc,
	&MC68000BASE::x203c_move_l_i_071234fc,
	&MC68000BASE::x2078_movea_l_aw_071234fc,
	&MC68000BASE::x2079_movea_l_al_071234fc,
	&MC68000BASE::x207a_movea_l_pcdi_071234fc,
	&MC68000BASE::x207b_movea_l_pcix_071234fc,
	&MC68000BASE::x207c_movea_l_i_071234fc,
	&MC68000BASE::x20b8_move_l_aw_071234fc,
	&MC68000BASE::x20b9_move_l_al_071234fc,
	&MC68000BASE::x20ba_move_l_pcdi_071234fc,
	&MC68000BASE::x20bb_move_l_pcix_071234fc,
	&MC68000BASE::x20bc_move_l_i_071234fc,
	&MC68000BASE::x20f8_move_l_aw_071234fc,
	&MC68000BASE::x20f9_move_l_al_071234fc,
	&MC68000BASE::x20fa_move_l_pcdi_071234fc,
	&MC68000BASE::x20fb_move_l_pcix_071234fc,
	&MC68000BASE::x20fc_move_l_i_071234fc,
	&MC68000BASE::x2138_move_l_aw_071234fc,
	&MC68000BASE::x2139_move_l_al_071234fc,
	&MC68000BASE::x213a_move_l_pcdi_071234fc,
	&MC68000BASE::x213b_move_l_pcix_071234fc,
	&MC68000BASE::x213c_move_l_i_071234fc,
	&MC68000BASE::x2178_move_l_aw_071234fc,
	&MC68000BASE::x2179_move_l_al_071234fc,
	&MC68000BASE::x217a_move_l_pcdi_071234fc,
	&MC68000BASE::x217b_move_l_pcix_071234fc,
	&MC68000BASE::x217c_move_l_i_071234fc,
	&MC68000BASE::x21b8_move_l_aw_071234fc,
	&MC68000BASE::x21b9_move_l_al_071234fc,
	&MC68000BASE::x21ba_move_l_pcdi_071234fc,
	&MC68000BASE::x21bb_move_l_pcix_071234fc,
	&MC68000BASE::x21bc_move_l_i_071234fc,
	&MC68000BASE::x3038_move_w_aw_071234fc,
	&MC68000BASE::x3039_move_w_al_071234fc,
	&MC68000BASE::x303a_move_w_pcdi_071234fc,
	&MC68000BASE::x303b_move_w_pcix_071234fc,
	&MC68000BASE::x303c_move_w_i_071234fc,
	&MC68000BASE::x3078_movea_w_aw_071234fc,
	&MC68000BASE::x3079_movea_w_al_071234fc,
	&MC68000BASE::x307a_movea_w_pcdi_071234fc,
	&MC68000BASE::x307b_movea_w_pcix_071234fc,
	&MC68000BASE::x307c_movea_w_i_071234fc,
	&MC68000BASE::x30b8_move_w_aw_071234fc,
	&MC68000BASE::x30b9_move_w_al_071234fc,
	&MC68000BASE::x30ba_move_w_pcdi_071234fc,
	&MC68000BASE::x30bb_move_w_pcix_071234fc,
	&MC68000BASE::x30bc_move_w_i_071234fc,
	&MC68000BASE::x30f8_move_w_aw_071234fc,
	&MC68000BASE::x30f9_move_w_al_071234fc,
	&MC68000BASE::x30fa_move_w_pcdi_071234fc,
	&MC68000BASE::x30fb_move_w_pcix_071234fc,
	&MC68000BASE::x30fc_move_w_i_071234fc,
	&MC68000BASE::x3138_move_w_aw_071234fc,
	&MC68000BASE::x3139_move_w_al_071234fc,
	&MC68000BASE::x313a_move_w_pcdi_071234fc,
	&MC68000BASE::x313b_move_w_pcix_071234fc,
	&MC68000BASE::x313c_move_w_i_071234fc,
	&MC68000BASE::x3178_move_w_aw_071234fc,
	&MC68000BASE::x3179_move_w_al_071234fc,
	&MC68000BASE::x317a_move_w_pcdi_071234fc,
	&MC68000BASE::x317b_move_w_pcix_071234fc,
	&MC68000BASE::x317c_move_w_i_071234fc,
	&MC68000BASE::x31b8_move_w_aw_071234fc,
	&MC68000BASE::x31b9_move_w_al_071234fc,
	&MC68000BASE::x31ba_move_w_pcdi_071234fc,
	&MC68000BASE::x31bb_move_w_pcix_071234fc,
	&MC68000BASE::x31bc_move_w_i_071234fc,
	&MC68000BASE::x4138_chk_l_aw_234fc,
	&MC68000BASE::x4139_chk_l_al_234fc,
	&MC68000BASE::x413a_chk_l_pcdi_234fc,
	&MC68000BASE::x413b_chk_l_pcix_234fc,
	&MC68000BASE::x413c_chk_l_i_234fc,
	&MC68000BASE::x41b8_chk_w_aw_071234fc,
	&MC68000BASE::x41b9_chk_w_al_071234fc,
	&MC68000BASE::x41ba_chk_w_pcdi_071234fc,
	&MC68000BASE::x41bb_chk_w_pcix_071234fc,
	&MC68000BASE::x41bc_chk_w_i_071234fc,
	&MC68000BASE::x41f8_lea_l_aw_071234fc,
	&MC68000BASE::x41f9_lea_l_al_071234fc,
	&MC68000BASE::x41fa_lea_l_pcdi_071234fc,
	&MC68000BASE::x41fb_lea_l_pcix_071234fc,
	&MC68000BASE::x501f_addq_b_pi7_071234fc,
	&MC68000BASE::x5027_addq_b_pd7_071234fc,
	&MC68000BASE::x5038_addq_b_aw_071234fc,
	&MC68000BASE::x5039_addq_b_al_071234fc,
	&MC68000BASE::x5078_addq_w_aw_071234fc,
	&MC68000BASE::x5079_addq_w_al_071234fc,
	&MC68000BASE::x50b8_addq_l_aw_071234fc,
	&MC68000BASE::x50b9_addq_l_al_071234fc,
	&MC68000BASE::x511f_subq_b_pi7_071234fc,
	&MC68000BASE::x5127_subq_b_pd7_071234fc,
	&MC68000BASE::x5138_subq_b_aw_071234fc,
	&MC68000BASE::x5139_subq_b_al_071234fc,
	&MC68000BASE::x5178_subq_w_aw_071234fc,
	&MC68000BASE::x5179_subq_w_al_071234fc,
	&MC68000BASE::x51b8_subq_l_aw_071234fc,
	&MC68000BASE::x51b9_subq_l_al_071234fc,
	&MC68000BASE::x801f_or_b_pi7_071234fc,
	&MC68000BASE::x8027_or_b_pd7_071234fc,
	&MC68000BASE::x8038_or_b_aw_071234fc,
	&MC68000BASE::x8039_or_b_al_071234fc,
	&MC68000BASE::x803a_or_b_pcdi_071234fc,
	&MC68000BASE::x803b_or_b_pcix_071234fc,
	&MC68000BASE::x803c_or_b_i_071234fc,
	&MC68000BASE::x8078_or_w_aw_071234fc,
	&MC68000BASE::x8079_or_w_al_071234fc,
	&MC68000BASE::x807a_or_w_pcdi_071234fc,
	&MC68000BASE::x807b_or_w_pcix_071234fc,
	&MC68000BASE::x807c_or_w_i_071234fc,
	&MC68000BASE::x80b8_or_l_aw_071234fc,
	&MC68000BASE::x80b9_or_l_al_071234fc,
	&MC68000BASE::x80ba_or_l_pcdi_071234fc,
	&MC68000BASE::x80bb_or_l_pcix_071234fc,
	&MC68000BASE::x80bc_or_l_i_071234fc,
	&MC68000BASE::x80f8_divu_w_aw_071234fc,
	&MC68000BASE::x80f9_divu_w_al_071234fc,
	&MC68000BASE::x80fa_divu_w_pcdi_071234fc,
	&MC68000BASE::x80fb_divu_w_pcix_071234fc,
	&MC68000BASE::x80fc_divu_w_i_071234fc,
	&MC68000BASE::x810f_sbcd_b_071234fc,
	&MC68000BASE::x811f_or_b_pi7_071234fc,
	&MC68000BASE::x8127_or_b_pd7_071234fc,
	&MC68000BASE::x8138_or_b_aw_071234fc,
	&MC68000BASE::x8139_or_b_al_071234fc,
	&MC68000BASE::x814f_pack_w_234fc,
	&MC68000BASE::x8178_or_w_aw_071234fc,
	&MC68000BASE::x8179_or_w_al_071234fc,
	&MC68000BASE::x818f_unpk_w_234fc,
	&MC68000BASE::x81b8_or_l_aw_071234fc,
	&MC68000BASE::x81b9_or_l_al_071234fc,
	&MC68000BASE::x81f8_divs_w_aw_071234fc,
	&MC68000BASE::x81f9_divs_w_al_071234fc,
	&MC68000BASE::x81fa_divs_w_pcdi_071234fc,
	&MC68000BASE::x81fb_divs_w_pcix_071234fc,
	&MC68000BASE::x81fc_divs_w_i_071234fc,
	&MC68000BASE::x901f_sub_b_pi7_071234fc,
	&MC68000BASE::x9027_sub_b_pd7_071234fc,
	&MC68000BASE::x9038_sub_b_aw_071234fc,
	&MC68000BASE::x9039_sub_b_al_071234fc,
	&MC68000BASE::x903a_sub_b_pcdi_071234fc,
	&MC68000BASE::x903b_sub_b_pcix_071234fc,
	&MC68000BASE::x903c_sub_b_i_071234fc,
	&MC68000BASE::x9078_sub_w_aw_071234fc,
	&MC68000BASE::x9079_sub_w_al_071234fc,
	&MC68000BASE::x907a_sub_w_pcdi_071234fc,
	&MC68000BASE::x907b_sub_w_pcix_071234fc,
	&MC68000BASE::x907c_sub_w_i_071234fc,
	&MC68000BASE::x90b8_sub_l_aw_071234fc,
	&MC68000BASE::x90b9_sub_l_al_071234fc,
	&MC68000BASE::x90ba_sub_l_pcdi_071234fc,
	&MC68000BASE::x90bb_sub_l_pcix_071234fc,
	&MC68000BASE::x90bc_sub_l_i_071234fc,
	&MC68000BASE::x90f8_suba_w_aw_071234fc,
	&MC68000BASE::x90f9_suba_w_al_071234fc,
	&MC68000BASE::x90fa_suba_w_pcdi_071234fc,
	&MC68000BASE::x90fb_suba_w_pcix_071234fc,
	&MC68000BASE::x90fc_suba_w_i_071234fc,
	&MC68000BASE::x910f_subx_b_071234fc,
	&MC68000BASE::x911f_sub_b_pi7_071234fc,
	&MC68000BASE::x9127_sub_b_pd7_071234fc,
	&MC68000BASE::x9138_sub_b_aw_071234fc,
	&MC68000BASE::x9139_sub_b_al_071234fc,
	&MC68000BASE::x9178_sub_w_aw_071234fc,
	&MC68000BASE::x9179_sub_w_al_071234fc,
	&MC68000BASE::x91b8_sub_l_aw_071234fc,
	&MC68000BASE::x91b9_sub_l_al_071234fc,
	&MC68000BASE::x91f8_suba_l_aw_071234fc,
	&MC68000BASE::x91f9_suba_l_al_071234fc,
	&MC68000BASE::x91fa_suba_l_pcdi_071234fc,
	&MC68000BASE::x91fb_suba_l_pcix_071234fc,
	&MC68000BASE::x91fc_suba_l_i_071234fc,
	&MC68000BASE::xb01f_cmp_b_pi7_071234fc,
	&MC68000BASE::xb027_cmp_b_pd7_071234fc,
	&MC68000BASE::xb038_cmp_b_aw_071234fc,
	&MC68000BASE::xb039_cmp_b_al_071234fc,
	&MC68000BASE::xb03a_cmp_b_pcdi_071234fc,
	&MC68000BASE::xb03b_cmp_b_pcix_071234fc,
	&MC68000BASE::xb03c_cmp_b_i_071234fc,
	&MC68000BASE::xb078_cmp_w_aw_071234fc,
	&MC68000BASE::xb079_cmp_w_al_071234fc,
	&MC68000BASE::xb07a_cmp_w_pcdi_071234fc,
	&MC68000BASE::xb07b_cmp_w_pcix_071234fc,
	&MC68000BASE::xb07c_cmp_w_i_071234fc,
	&MC68000BASE::xb0b8_cmp_l_aw_071234fc,
	&MC68000BASE::xb0b9_cmp_l_al_071234fc,
	&MC68000BASE::xb0ba_cmp_l_pcdi_071234fc,
	&MC68000BASE::xb0bb_cmp_l_pcix_071234fc,
	&MC68000BASE::xb0bc_cmp_l_i_071234fc,
	&MC68000BASE::xb0f8_cmpa_w_aw_071234fc,
	&MC68000BASE::xb0f9_cmpa_w_al_071234fc,
	&MC68000BASE::xb0fa_cmpa_w_pcdi_071234fc,
	&MC68000BASE::xb0fb_cmpa_w_pcix_071234fc,
	&MC68000BASE::xb0fc_cmpa_w_i_071234fc,
	&MC68000BASE::xb10f_cmpm_b_071234fc,
	&MC68000BASE::xb11f_eor_b_pi7_071234fc,
	&MC68000BASE::xb127_eor_b_pd7_071234fc,
	&MC68000BASE::xb138_eor_b_aw_071234fc,
	&MC68000BASE::xb139_eor_b_al_071234fc,
	&MC68000BASE::xb178_eor_w_aw_071234fc,
	&MC68000BASE::xb179_eor_w_al_071234fc,
	&MC68000BASE::xb1b8_eor_l_aw_071234fc,
	&MC68000BASE::xb1b9_eor_l_al_071234fc,
	&MC68000BASE::xb1f8_cmpa_l_aw_071234fc,
	&MC68000BASE::xb1f9_cmpa_l_al_071234fc,
	&MC68000BASE::xb1fa_cmpa_l_pcdi_071234fc,
	&MC68000BASE::xb1fb_cmpa_l_pcix_071234fc,
	&MC68000BASE::xb1fc_cmpa_l_i_071234fc,
	&MC68000BASE::xc01f_and_b_pi7_071234fc,
	&MC68000BASE::xc027_and_b_pd7_071234fc,
	&MC68000BASE::xc038_and_b_aw_071234fc,
	&MC68000BASE::xc039_and_b_al_071234fc,
	&MC68000BASE::xc03a_and_b_pcdi_071234fc,
	&MC68000BASE::xc03b_and_b_pcix_071234fc,
	&MC68000BASE::xc03c_and_b_i_071234fc,
	&MC68000BASE::xc078_and_w_aw_071234fc,
	&MC68000BASE::xc079_and_w_al_071234fc,
	&MC68000BASE::xc07a_and_w_pcdi_071234fc,
	&MC68000BASE::xc07b_and_w_pcix_071234fc,
	&MC68000BASE::xc07c_and_w_i_071234fc,
	&MC68000BASE::xc0b8_and_l_aw_071234fc,
	&MC68000BASE::xc0b9_and_l_al_071234fc,
	&MC68000BASE::xc0ba_and_l_pcdi_071234fc,
	&MC68000BASE::xc0bb_and_l_pcix_071234fc,
	&MC68000BASE::xc0bc_and_l_i_071234fc,
	&MC68000BASE::xc0f8_mulu_w_aw_071234fc,
	&MC68000BASE::xc0f9_mulu_w_al_071234fc,
	&MC68000BASE::xc0fa_mulu_w_pcdi_071234fc,
	&MC68000BASE::xc0fb_mulu_w_pcix_071234fc,
	&MC68000BASE::xc0fc_mulu_w_i_071234fc,
	&MC68000BASE::xc10f_abcd_b_071234fc,
	&MC68000BASE::xc11f_and_b_pi7_071234fc,
	&MC68000BASE::xc127_and_b_pd7_071234fc,
	&MC68000BASE::xc138_and_b_aw_071234fc,
	&MC68000BASE::xc139_and_b_al_071234fc,
	&MC68000BASE::xc178_and_w_aw_071234fc,
	&MC68000BASE::xc179_and_w_al_071234fc,
	&MC68000BASE::xc1b8_and_l_aw_071234fc,
	&MC68000BASE::xc1b9_and_l_al_071234fc,
	&MC68000BASE::xc1f8_muls_w_aw_071234fc,
	&MC68000BASE::xc1f9_muls_w_al_071234fc,
	&MC68000BASE::xc1fa_muls_w_pcdi_071234fc,
	&MC68000BASE::xc1fb_muls_w_pcix_071234fc,
	&MC68000BASE::xc1fc_muls_w_i_071234fc,
	&MC68000BASE::xd01f_add_b_pi7_071234fc,
	&MC68000BASE::xd027_add_b_pd7_071234fc,
	&MC68000BASE::xd038_add_b_aw_071234fc,
	&MC68000BASE::xd039_add_b_al_071234fc,
	&MC68000BASE::xd03a_add_b_pcdi_071234fc,
	&MC68000BASE::xd03b_add_b_pcix_071234fc,
	&MC68000BASE::xd03c_add_b_i_071234fc,
	&MC68000BASE::xd078_add_w_aw_071234fc,
	&MC68000BASE::xd079_add_w_al_071234fc,
	&MC68000BASE::xd07a_add_w_pcdi_071234fc,
	&MC68000BASE::xd07b_add_w_pcix_071234fc,
	&MC68000BASE::xd07c_add_w_i_071234fc,
	&MC68000BASE::xd0b8_add_l_aw_071234fc,
	&MC68000BASE::xd0b9_add_l_al_071234fc,
	&MC68000BASE::xd0ba_add_l_pcdi_071234fc,
	&MC68000BASE::xd0bb_add_l_pcix_071234fc,
	&MC68000BASE::xd0bc_add_l_i_071234fc,
	&MC68000BASE::xd0f8_adda_w_aw_071234fc,
	&MC68000BASE::xd0f9_adda_w_al_071234fc,
	&MC68000BASE::xd0fa_adda_w_pcdi_071234fc,
	&MC68000BASE::xd0fb_adda_w_pcix_071234fc,
	&MC68000BASE::xd0fc_adda_w_i_071234fc,
	&MC68000BASE::xd10f_addx_b_071234fc,
	&MC68000BASE::xd11f_add_b_pi7_071234fc,
	&MC68000BASE::xd127_add_b_pd7_071234fc,
	&MC68000BASE::xd138_add_b_aw_071234fc,
	&MC68000BASE::xd139_add_b_al_071234fc,
	&MC68000BASE::xd178_add_w_aw_071234fc,
	&MC68000BASE::xd179_add_w_al_071234fc,
	&MC68000BASE::xd1b8_add_l_aw_071234fc,
	&MC68000BASE::xd1b9_add_l_al_071234fc,
	&MC68000BASE::xd1f8_adda_l_aw_071234fc,
	&MC68000BASE::xd1f9_adda_l_al_071234fc,
	&MC68000BASE::xd1fa_adda_l_pcdi_071234fc,
	&MC68000BASE::xd1fb_adda_l_pcix_071234fc,
	&MC68000BASE::xd1fc_adda_l_i_071234fc,
	&MC68000BASE::x0000_ori_b_071234fc,
	&MC68000BASE::x0010_ori_b_ai_071234fc,
	&MC68000BASE::x0018_ori_b_pi_071234fc,
	&MC68000BASE::x0020_ori_b_pd_071234fc,
	&MC68000BASE::x0028_ori_b_di_071234fc,
	&MC68000BASE::x0030_ori_b_ix_071234fc,
	&MC68000BASE::x0040_ori_w_071234fc,
	&MC68000BASE::x0050_ori_w_ai_071234fc,
	&MC68000BASE::x0058_ori_w_pi_071234fc,
	&MC68000BASE::x0060_ori_w_pd_071234fc,
	&MC68000BASE::x0068_ori_w_di_071234fc,
	&MC68000BASE::x0070_ori_w_ix_071234fc,
	&MC68000BASE::x0080_ori_l_071234fc,
	&MC68000BASE::x0090_ori_l_ai_071234fc,
	&MC68000BASE::x0098_ori_l_pi_071234fc,
	&MC68000BASE::x00a0_ori_l_pd_071234fc,
	&MC68000BASE::x00a8_ori_l_di_071234fc,
	&MC68000BASE::x00b0_ori_l_ix_071234fc,
	&MC68000BASE::x00d0_chk2cmp2_b_ai_234fc,
	&MC68000BASE::x00e8_chk2cmp2_b_di_234fc,
	&MC68000BASE::x00f0_chk2cmp2_b_ix_234fc,
	&MC68000BASE::x0200_andi_b_071234fc,
	&MC68000BASE::x0210_andi_b_ai_071234fc,
	&MC68000BASE::x0218_andi_b_pi_071234fc,
	&MC68000BASE::x0220_andi_b_pd_071234fc,
	&MC68000BASE::x0228_andi_b_di_071234fc,
	&MC68000BASE::x0230_andi_b_ix_071234fc,
	&MC68000BASE::x0240_andi_w_071234fc,
	&MC68000BASE::x0250_andi_w_ai_071234fc,
	&MC68000BASE::x0258_andi_w_pi_071234fc,
	&MC68000BASE::x0260_andi_w_pd_071234fc,
	&MC68000BASE::x0268_andi_w_di_071234fc,
	&MC68000BASE::x0270_andi_w_ix_071234fc,
	&MC68000BASE::x0280_andi_l_071234fc,
	&MC68000BASE::x0290_andi_l_ai_071234fc,
	&MC68000BASE::x0298_andi_l_pi_071234fc,
	&MC68000BASE::x02a0_andi_l_pd_071234fc,
	&MC68000BASE::x02a8_andi_l_di_071234fc,
	&MC68000BASE::x02b0_andi_l_ix_071234fc,
	&MC68000BASE::x02d0_chk2cmp2_w_ai_234fc,
	&MC68000BASE::x02e8_chk2cmp2_w_di_234fc,
	&MC68000BASE::x02f0_chk2cmp2_w_ix_234fc,
	&MC68000BASE::x0400_subi_b_071234fc,
	&MC68000BASE::x0410_subi_b_ai_071234fc,
	&MC68000BASE::x0418_subi_b_pi_071234fc,
	&MC68000BASE::x0420_subi_b_pd_071234fc,
	&MC68000BASE::x0428_subi_b_di_071234fc,
	&MC68000BASE::x0430_subi_b_ix_071234fc,
	&MC68000BASE::x0440_subi_w_071234fc,
	&MC68000BASE::x0450_subi_w_ai_071234fc,
	&MC68000BASE::x0458_subi_w_pi_071234fc,
	&MC68000BASE::x0460_subi_w_pd_071234fc,
	&MC68000BASE::x0468_subi_w_di_071234fc,
	&MC68000BASE::x0470_subi_w_ix_071234fc,
	&MC68000BASE::x0480_subi_l_071234fc,
	&MC68000BASE::x0490_subi_l_ai_071234fc,
	&MC68000BASE::x0498_subi_l_pi_071234fc,
	&MC68000BASE::x04a0_subi_l_pd_071234fc,
	&MC68000BASE::x04a8_subi_l_di_071234fc,
	&MC68000BASE::x04b0_subi_l_ix_071234fc,
	&MC68000BASE::x04d0_chk2cmp2_l_ai_234fc,
	&MC68000BASE::x04e8_chk2cmp2_l_di_234fc,
	&MC68000BASE::x04f0_chk2cmp2_l_ix_234fc,
	&MC68000BASE::x0600_addi_b_071234fc,
	&MC68000BASE::x0610_addi_b_ai_071234fc,
	&MC68000BASE::x0618_addi_b_pi_071234fc,
	&MC68000BASE::x0620_addi_b_pd_071234fc,
	&MC68000BASE::x0628_addi_b_di_071234fc,
	&MC68000BASE::x0630_addi_b_ix_071234fc,
	&MC68000BASE::x0640_addi_w_071234fc,
	&MC68000BASE::x0650_addi_w_ai_071234fc,
	&MC68000BASE::x0658_addi_w_pi_071234fc,
	&MC68000BASE::x0660_addi_w_pd_071234fc,
	&MC68000BASE::x0668_addi_w_di_071234fc,
	&MC68000BASE::x0670_addi_w_ix_071234fc,
	&MC68000BASE::x0680_addi_l_071234fc,
	&MC68000BASE::x0690_addi_l_ai_071234fc,
	&MC68000BASE::x0698_addi_l_pi_071234fc,
	&MC68000BASE::x06a0_addi_l_pd_071234fc,
	&MC68000BASE::x06a8_addi_l_di_071234fc,
	&MC68000BASE::x06b0_addi_l_ix_071234fc,
	&MC68000BASE::x06d0_callm_l_ai_2f,
	&MC68000BASE::x06e8_callm_l_di_2f,
	&MC68000BASE::x06f0_callm_l_ix_2f,
	&MC68000BASE::x0800_btst_l_071234fc,
	&MC68000BASE::x0810_btst_b_ai_071234fc,
	&MC68000BASE::x0818_btst_b_pi_071234fc,
	&MC68000BASE::x0820_btst_b_pd_071234fc,
	&MC68000BASE::x0828_btst_b_di_071234fc,
	&MC68000BASE::x0830_btst_b_ix_071234fc,
	&MC68000BASE::x0840_bchg_l_071234fc,
	&MC68000BASE::x0850_bchg_b_ai_071234fc,
	&MC68000BASE::x0858_bchg_b_pi_071234fc,
	&MC68000BASE::x0860_bchg_b_pd_071234fc,
	&MC68000BASE::x0868_bchg_b_di_071234fc,
	&MC68000BASE::x0870_bchg_b_ix_071234fc,
	&MC68000BASE::x0880_bclr_l_071234fc,
	&MC68000BASE::x0890_bclr_b_ai_071234fc,
	&MC68000BASE::x0898_bclr_b_pi_071234fc,
	&MC68000BASE::x08a0_bclr_b_pd_071234fc,
	&MC68000BASE::x08a8_bclr_b_di_071234fc,
	&MC68000BASE::x08b0_bclr_b_ix_071234fc,
	&MC68000BASE::x08c0_bset_l_071234fc,
	&MC68000BASE::x08d0_bset_b_ai_071234fc,
	&MC68000BASE::x08d8_bset_b_pi_071234fc,
	&MC68000BASE::x08e0_bset_b_pd_071234fc,
	&MC68000BASE::x08e8_bset_b_di_071234fc,
	&MC68000BASE::x08f0_bset_b_ix_071234fc,
	&MC68000BASE::x0a00_eori_b_071234fc,
	&MC68000BASE::x0a10_eori_b_ai_071234fc,
	&MC68000BASE::x0a18_eori_b_pi_071234fc,
	&MC68000BASE::x0a20_eori_b_pd_071234fc,
	&MC68000BASE::x0a28_eori_b_di_071234fc,
	&MC68000BASE::x0a30_eori_b_ix_071234fc,
	&MC68000BASE::x0a40_eori_w_071234fc,
	&MC68000BASE::x0a50_eori_w_ai_071234fc,
	&MC68000BASE::x0a58_eori_w_pi_071234fc,
	&MC68000BASE::x0a60_eori_w_pd_071234fc,
	&MC68000BASE::x0a68_eori_w_di_071234fc,
	&MC68000BASE::x0a70_eori_w_ix_071234fc,
	&MC68000BASE::x0a80_eori_l_071234fc,
	&MC68000BASE::x0a90_eori_l_ai_071234fc,
	&MC68000BASE::x0a98_eori_l_pi_071234fc,
	&MC68000BASE::x0aa0_eori_l_pd_071234fc,
	&MC68000BASE::x0aa8_eori_l_di_071234fc,
	&MC68000BASE::x0ab0_eori_l_ix_071234fc,
	&MC68000BASE::x0ad0_cas_b_ai_234fc,
	&MC68000BASE::x0ad8_cas_b_pi_234fc,
	&MC68000BASE::x0ae0_cas_b_pd_234fc,
	&MC68000BASE::x0ae8_cas_b_di_234fc,
	&MC68000BASE::x0af0_cas_b_ix_234fc,
	&MC68000BASE::x0c00_cmpi_b_071234fc,
	&MC68000BASE::x0c10_cmpi_b_ai_071234fc,
	&MC68000BASE::x0c18_cmpi_b_pi_071234fc,
	&MC68000BASE::x0c20_cmpi_b_pd_071234fc,
	&MC68000BASE::x0c28_cmpi_b_di_071234fc,
	&MC68000BASE::x0c30_cmpi_b_ix_071234fc,
	&MC68000BASE::x0c40_cmpi_w_071234fc,
	&MC68000BASE::x0c50_cmpi_w_ai_071234fc,
	&MC68000BASE::x0c58_cmpi_w_pi_071234fc,
	&MC68000BASE::x0c60_cmpi_w_pd_071234fc,
	&MC68000BASE::x0c68_cmpi_w_di_071234fc,
	&MC68000BASE::x0c70_cmpi_w_ix_071234fc,
	&MC68000BASE::x0c80_cmpi_l_071234fc,
	&MC68000BASE::x0c90_cmpi_l_ai_071234fc,
	&MC68000BASE::x0c98_cmpi_l_pi_071234fc,
	&MC68000BASE::x0ca0_cmpi_l_pd_071234fc,
	&MC68000BASE::x0ca8_cmpi_l_di_071234fc,
	&MC68000BASE::x0cb0_cmpi_l_ix_071234fc,
	&MC68000BASE::x0cd0_cas_w_ai_234fc,
	&MC68000BASE::x0cd8_cas_w_pi_234fc,
	&MC68000BASE::x0ce0_cas_w_pd_234fc,
	&MC68000BASE::x0ce8_cas_w_di_234fc,
	&MC68000BASE::x0cf0_cas_w_ix_234fc,
	&MC68000BASE::x0e10_moves_b_ai_134fc,
	&MC68000BASE::x0e10_moves_b_ai_2,
	&MC68000BASE::x0e18_moves_b_pi_134fc,
	&MC68000BASE::x0e18_moves_b_pi_2,
	&MC68000BASE::x0e20_moves_b_pd_134fc,
	&MC68000BASE::x0e20_moves_b_pd_2,
	&MC68000BASE::x0e28_moves_b_di_134fc,
	&MC68000BASE::x0e28_moves_b_di_2,
	&MC68000BASE::x0e30_moves_b_ix_134fc,
	&MC68000BASE::x0e30_moves_b_ix_2,
	&MC68000BASE::x0e50_moves_w_ai_134fc,
	&MC68000BASE::x0e50_moves_w_ai_2,
	&MC68000BASE::x0e58_moves_w_pi_134fc,
	&MC68000BASE::x0e58_moves_w_pi_2,
	&MC68000BASE::x0e60_moves_w_pd_134fc,
	&MC68000BASE::x0e60_moves_w_pd_2,
	&MC68000BASE::x0e68_moves_w_di_134fc,
	&MC68000BASE::x0e68_moves_w_di_2,
	&MC68000BASE::x0e70_moves_w_ix_134fc,
	&MC68000BASE::x0e70_moves_w_ix_2,
	&MC68000BASE::x0e90_moves_l_ai_134fc,
	&MC68000BASE::x0e90_moves_l_ai_2,
	&MC68000BASE::x0e98_moves_l_pi_134fc,
	&MC68000BASE::x0e98_moves_l_pi_2,
	&MC68000BASE::x0ea0_moves_l_pd_134fc,
	&MC68000BASE::x0ea0_moves_l_pd_2,
	&MC68000BASE::x0ea8_moves_l_di_134fc,
	&MC68000BASE::x0ea8_moves_l_di_2,
	&MC68000BASE::x0eb0_moves_l_ix_134fc,
	&MC68000BASE::x0eb0_moves_l_ix_2,
	&MC68000BASE::x0ed0_cas_l_ai_234fc,
	&MC68000BASE::x0ed8_cas_l_pi_234fc,
	&MC68000BASE::x0ee0_cas_l_pd_234fc,
	&MC68000BASE::x0ee8_cas_l_di_234fc,
	&MC68000BASE::x0ef0_cas_l_ix_234fc,
	&MC68000BASE::x11c0_move_b_071234fc,
	&MC68000BASE::x11d0_move_b_ai_071234fc,
	&MC68000BASE::x11d8_move_b_pi_071234fc,
	&MC68000BASE::x11e0_move_b_pd_071234fc,
	&MC68000BASE::x11e8_move_b_di_071234fc,
	&MC68000BASE::x11f0_move_b_ix_071234fc,
	&MC68000BASE::x13c0_move_b_071234fc,
	&MC68000BASE::x13d0_move_b_ai_071234fc,
	&MC68000BASE::x13d8_move_b_pi_071234fc,
	&MC68000BASE::x13e0_move_b_pd_071234fc,
	&MC68000BASE::x13e8_move_b_di_071234fc,
	&MC68000BASE::x13f0_move_b_ix_071234fc,
	&MC68000BASE::x1ec0_move_b_071234fc,
	&MC68000BASE::x1ed0_move_b_ai_071234fc,
	&MC68000BASE::x1ed8_move_b_pi_071234fc,
	&MC68000BASE::x1ee0_move_b_pd_071234fc,
	&MC68000BASE::x1ee8_move_b_di_071234fc,
	&MC68000BASE::x1ef0_move_b_ix_071234fc,
	&MC68000BASE::x1f00_move_b_071234fc,
	&MC68000BASE::x1f10_move_b_ai_071234fc,
	&MC68000BASE::x1f18_move_b_pi_071234fc,
	&MC68000BASE::x1f20_move_b_pd_071234fc,
	&MC68000BASE::x1f28_move_b_di_071234fc,
	&MC68000BASE::x1f30_move_b_ix_071234fc,
	&MC68000BASE::x21c0_move_l_071234fc,
	&MC68000BASE::x21c8_move_l_071234fc,
	&MC68000BASE::x21d0_move_l_ai_071234fc,
	&MC68000BASE::x21d8_move_l_pi_071234fc,
	&MC68000BASE::x21e0_move_l_pd_071234fc,
	&MC68000BASE::x21e8_move_l_di_071234fc,
	&MC68000BASE::x21f0_move_l_ix_071234fc,
	&MC68000BASE::x23c0_move_l_071234fc,
	&MC68000BASE::x23c8_move_l_071234fc,
	&MC68000BASE::x23d0_move_l_ai_071234fc,
	&MC68000BASE::x23d8_move_l_pi_071234fc,
	&MC68000BASE::x23e0_move_l_pd_071234fc,
	&MC68000BASE::x23e8_move_l_di_071234fc,
	&MC68000BASE::x23f0_move_l_ix_071234fc,
	&MC68000BASE::x31c0_move_w_071234fc,
	&MC68000BASE::x31c8_move_w_071234fc,
	&MC68000BASE::x31d0_move_w_ai_071234fc,
	&MC68000BASE::x31d8_move_w_pi_071234fc,
	&MC68000BASE::x31e0_move_w_pd_071234fc,
	&MC68000BASE::x31e8_move_w_di_071234fc,
	&MC68000BASE::x31f0_move_w_ix_071234fc,
	&MC68000BASE::x33c0_move_w_071234fc,
	&MC68000BASE::x33c8_move_w_071234fc,
	&MC68000BASE::x33d0_move_w_ai_071234fc,
	&MC68000BASE::x33d8_move_w_pi_071234fc,
	&MC68000BASE::x33e0_move_w_pd_071234fc,
	&MC68000BASE::x33e8_move_w_di_071234fc,
	&MC68000BASE::x33f0_move_w_ix_071234fc,
	&MC68000BASE::x4000_negx_b_071234fc,
	&MC68000BASE::x4010_negx_b_ai_071234fc,
	&MC68000BASE::x4018_negx_b_pi_071234fc,
	&MC68000BASE::x4020_negx_b_pd_071234fc,
	&MC68000BASE::x4028_negx_b_di_071234fc,
	&MC68000BASE::x4030_negx_b_ix_071234fc,
	&MC68000BASE::x4040_negx_w_071234fc,
	&MC68000BASE::x4050_negx_w_ai_071234fc,
	&MC68000BASE::x4058_negx_w_pi_071234fc,
	&MC68000BASE::x4060_negx_w_pd_071234fc,
	&MC68000BASE::x4068_negx_w_di_071234fc,
	&MC68000BASE::x4070_negx_w_ix_071234fc,
	&MC68000BASE::x4080_negx_l_071234fc,
	&MC68000BASE::x4090_negx_l_ai_071234fc,
	&MC68000BASE::x4098_negx_l_pi_071234fc,
	&MC68000BASE::x40a0_negx_l_pd_071234fc,
	&MC68000BASE::x40a8_negx_l_di_071234fc,
	&MC68000BASE::x40b0_negx_l_ix_071234fc,
	&MC68000BASE::x40c0_move_w_07,
	&MC68000BASE::x40c0_move_w_1234fc,
	&MC68000BASE::x40d0_move_w_ai_07,
	&MC68000BASE::x40d0_move_w_ai_1234fc,
	&MC68000BASE::x40d8_move_w_pi_07,
	&MC68000BASE::x40d8_move_w_pi_1234fc,
	&MC68000BASE::x40e0_move_w_pd_07,
	&MC68000BASE::x40e0_move_w_pd_1234fc,
	&MC68000BASE::x40e8_move_w_di_07,
	&MC68000BASE::x40e8_move_w_di_1234fc,
	&MC68000BASE::x40f0_move_w_ix_07,
	&MC68000BASE::x40f0_move_w_ix_1234fc,
	&MC68000BASE::x4200_clr_b_071234fc,
	&MC68000BASE::x4210_clr_b_ai_0,
	&MC68000BASE::x4210_clr_b_ai_71234fc,
	&MC68000BASE::x4218_clr_b_pi_0,
	&MC68000BASE::x4218_clr_b_pi_71234fc,
	&MC68000BASE::x4220_clr_b_pd_0,
	&MC68000BASE::x4220_clr_b_pd_71234fc,
	&MC68000BASE::x4228_clr_b_di_0,
	&MC68000BASE::x4228_clr_b_di_71234fc,
	&MC68000BASE::x4230_clr_b_ix_0,
	&MC68000BASE::x4230_clr_b_ix_71234fc,
	&MC68000BASE::x4240_clr_w_071234fc,
	&MC68000BASE::x4250_clr_w_ai_0,
	&MC68000BASE::x4250_clr_w_ai_71234fc,
	&MC68000BASE::x4258_clr_w_pi_0,
	&MC68000BASE::x4258_clr_w_pi_71234fc,
	&MC68000BASE::x4260_clr_w_pd_0,
	&MC68000BASE::x4260_clr_w_pd_71234fc,
	&MC68000BASE::x4268_clr_w_di_0,
	&MC68000BASE::x4268_clr_w_di_71234fc,
	&MC68000BASE::x4270_clr_w_ix_0,
	&MC68000BASE::x4270_clr_w_ix_71234fc,
	&MC68000BASE::x4280_clr_l_071234fc,
	&MC68000BASE::x4290_clr_l_ai_0,
	&MC68000BASE::x4290_clr_l_ai_71234fc,
	&MC68000BASE::x4298_clr_l_pi_0,
	&MC68000BASE::x4298_clr_l_pi_71234fc,
	&MC68000BASE::x42a0_clr_l_pd_0,
	&MC68000BASE::x42a0_clr_l_pd_71234fc,
	&MC68000BASE::x42a8_clr_l_di_0,
	&MC68000BASE::x42a8_clr_l_di_71234fc,
	&MC68000BASE::x42b0_clr_l_ix_0,
	&MC68000BASE::x42b0_clr_l_ix_71234fc,
	&MC68000BASE::x42c0_move_w_1234fc,
	&MC68000BASE::x42d0_move_w_ai_1234fc,
	&MC68000BASE::x42d8_move_w_pi_1234fc,
	&MC68000BASE::x42e0_move_w_pd_1234fc,
	&MC68000BASE::x42e8_move_w_di_1234fc,
	&MC68000BASE::x42f0_move_w_ix_1234fc,
	&MC68000BASE::x4400_neg_b_071234fc,
	&MC68000BASE::x4410_neg_b_ai_071234fc,
	&MC68000BASE::x4418_neg_b_pi_071234fc,
	&MC68000BASE::x4420_neg_b_pd_071234fc,
	&MC68000BASE::x4428_neg_b_di_071234fc,
	&MC68000BASE::x4430_neg_b_ix_071234fc,
	&MC68000BASE::x4440_neg_w_071234fc,
	&MC68000BASE::x4450_neg_w_ai_071234fc,
	&MC68000BASE::x4458_neg_w_pi_071234fc,
	&MC68000BASE::x4460_neg_w_pd_071234fc,
	&MC68000BASE::x4468_neg_w_di_071234fc,
	&MC68000BASE::x4470_neg_w_ix_071234fc,
	&MC68000BASE::x4480_neg_l_071234fc,
	&MC68000BASE::x4490_neg_l_ai_071234fc,
	&MC68000BASE::x4498_neg_l_pi_071234fc,
	&MC68000BASE::x44a0_neg_l_pd_071234fc,
	&MC68000BASE::x44a8_neg_l_di_071234fc,
	&MC68000BASE::x44b0_neg_l_ix_071234fc,
	&MC68000BASE::x44c0_move_w_071234fc,
	&MC68000BASE::x44d0_move_w_ai_071234fc,
	&MC68000BASE::x44d8_move_w_pi_071234fc,
	&MC68000BASE::x44e0_move_w_pd_071234fc,
	&MC68000BASE::x44e8_move_w_di_071234fc,
	&MC68000BASE::x44f0_move_w_ix_071234fc,
	&MC68000BASE::x4600_not_b_071234fc,
	&MC68000BASE::x4610_not_b_ai_071234fc,
	&MC68000BASE::x4618_not_b_pi_071234fc,
	&MC68000BASE::x4620_not_b_pd_071234fc,
	&MC68000BASE::x4628_not_b_di_071234fc,
	&MC68000BASE::x4630_not_b_ix_071234fc,
	&MC68000BASE::x4640_not_w_071234fc,
	&MC68000BASE::x4650_not_w_ai_071234fc,
	&MC68000BASE::x4658_not_w_pi_071234fc,
	&MC68000BASE::x4660_not_w_pd_071234fc,
	&MC68000BASE::x4668_not_w_di_071234fc,
	&MC68000BASE::x4670_not_w_ix_071234fc,
	&MC68000BASE::x4680_not_l_071234fc,
	&MC68000BASE::x4690_not_l_ai_071234fc,
	&MC68000BASE::x4698_not_l_pi_071234fc,
	&MC68000BASE::x46a0_not_l_pd_071234fc,
	&MC68000BASE::x46a8_not_l_di_071234fc,
	&MC68000BASE::x46b0_not_l_ix_071234fc,
	&MC68000BASE::x46c0_move_w_071234fc,
	&MC68000BASE::x46d0_move_w_ai_071234fc,
	&MC68000BASE::x46d8_move_w_pi_071234fc,
	&MC68000BASE::x46e0_move_w_pd_071234fc,
	&MC68000BASE::x46e8_move_w_di_071234fc,
	&MC68000BASE::x46f0_move_w_ix_071234fc,
	&MC68000BASE::x4800_nbcd_b_071234fc,
	&MC68000BASE::x4808_link_l_234fc,
	&MC68000BASE::x4810_nbcd_b_ai_071234fc,
	&MC68000BASE::x4818_nbcd_b_pi_071234fc,
	&MC68000BASE::x4820_nbcd_b_pd_071234fc,
	&MC68000BASE::x4828_nbcd_b_di_071234fc,
	&MC68000BASE::x4830_nbcd_b_ix_071234fc,
	&MC68000BASE::x4840_swap_l_071234fc,
	&MC68000BASE::x4848_bkpt_1,
	&MC68000BASE::x4848_bkpt_234fc,
	&MC68000BASE::x4850_pea_l_ai_071234fc,
	&MC68000BASE::x4868_pea_l_di_071234fc,
	&MC68000BASE::x4870_pea_l_ix_071234fc,
	&MC68000BASE::x4880_ext_w_071234fc,
	&MC68000BASE::x4890_movem_w_ai_071234fc,
	&MC68000BASE::x48a0_movem_w_071234fc,
	&MC68000BASE::x48a8_movem_w_di_071234fc,
	&MC68000BASE::x48b0_movem_w_ix_071234fc,
	&MC68000BASE::x48c0_ext_l_071234fc,
	&MC68000BASE::x48d0_movem_l_ai_071234fc,
	&MC68000BASE::x48e0_movem_l_071234fc,
	&MC68000BASE::x48e8_movem_l_di_071234fc,
	&MC68000BASE::x48f0_movem_l_ix_071234fc,
	&MC68000BASE::x49c0_extb_l_234fc,
	&MC68000BASE::x4a00_tst_b_071234fc,
	&MC68000BASE::x4a10_tst_b_ai_071234fc,
	&MC68000BASE::x4a18_tst_b_pi_071234fc,
	&MC68000BASE::x4a20_tst_b_pd_071234fc,
	&MC68000BASE::x4a28_tst_b_di_071234fc,
	&MC68000BASE::x4a30_tst_b_ix_071234fc,
	&MC68000BASE::x4a40_tst_w_071234fc,
	&MC68000BASE::x4a48_tst_w_234fc,
	&MC68000BASE::x4a50_tst_w_ai_071234fc,
	&MC68000BASE::x4a58_tst_w_pi_071234fc,
	&MC68000BASE::x4a60_tst_w_pd_071234fc,
	&MC68000BASE::x4a68_tst_w_di_071234fc,
	&MC68000BASE::x4a70_tst_w_ix_071234fc,
	&MC68000BASE::x4a80_tst_l_071234fc,
	&MC68000BASE::x4a88_tst_l_234fc,
	&MC68000BASE::x4a90_tst_l_ai_071234fc,
	&MC68000BASE::x4a98_tst_l_pi_071234fc,
	&MC68000BASE::x4aa0_tst_l_pd_071234fc,
	&MC68000BASE::x4aa8_tst_l_di_071234fc,
	&MC68000BASE::x4ab0_tst_l_ix_071234fc,
	&MC68000BASE::x4ac0_tas_b_071234fc,
	&MC68000BASE::x4ad0_tas_b_ai_071234fc,
	&MC68000BASE::x4ad8_tas_b_pi_071234fc,
	&MC68000BASE::x4ae0_tas_b_pd_071234fc,
	&MC68000BASE::x4ae8_tas_b_di_071234fc,
	&MC68000BASE::x4af0_tas_b_ix_071234fc,
	&MC68000BASE::x4c00_mull_l_234fc,
	&MC68000BASE::x4c10_mull_l_ai_234fc,
	&MC68000BASE::x4c18_mull_l_pi_234fc,
	&MC68000BASE::x4c20_mull_l_pd_234fc,
	&MC68000BASE::x4c28_mull_l_di_234fc,
	&MC68000BASE::x4c30_mull_l_ix_234fc,
	&MC68000BASE::x4c40_divl_l_234fc,
	&MC68000BASE::x4c50_divl_l_ai_234fc,
	&MC68000BASE::x4c58_divl_l_pi_234fc,
	&MC68000BASE::x4c60_divl_l_pd_234fc,
	&MC68000BASE::x4c68_divl_l_di_234fc,
	&MC68000BASE::x4c70_divl_l_ix_234fc,
	&MC68000BASE::x4c90_movem_w_ai_071234fc,
	&MC68000BASE::x4c98_movem_w_071234fc,
	&MC68000BASE::x4ca8_movem_w_di_071234fc,
	&MC68000BASE::x4cb0_movem_w_ix_071234fc,
	&MC68000BASE::x4cd0_movem_l_ai_071234fc,
	&MC68000BASE::x4cd8_movem_l_071234fc,
	&MC68000BASE::x4ce8_movem_l_di_071234fc,
	&MC68000BASE::x4cf0_movem_l_ix_071234fc,
	&MC68000BASE::x4e50_link_w_071234fc,
	&MC68000BASE::x4e58_unlk_l_071234fc,
	&MC68000BASE::x4e60_move_l_071234fc,
	&MC68000BASE::x4e68_move_l_071234fc,
	&MC68000BASE::x4e90_jsr_l_ai_071234fc,
	&MC68000BASE::x4ea8_jsr_l_di_071234fc,
	&MC68000BASE::x4eb0_jsr_l_ix_071234fc,
	&MC68000BASE::x4ed0_jmp_l_ai_071234fc,
	&MC68000BASE::x4ee8_jmp_l_di_071234fc,
	&MC68000BASE::x4ef0_jmp_l_ix_071234fc,
	&MC68000BASE::x50c0_st_b_071234fc,
	&MC68000BASE::x50c8_dbt_w_071234fc,
	&MC68000BASE::x50d0_st_b_ai_071234fc,
	&MC68000BASE::x50d8_st_b_pi_071234fc,
	&MC68000BASE::x50e0_st_b_pd_071234fc,
	&MC68000BASE::x50e8_st_b_di_071234fc,
	&MC68000BASE::x50f0_st_b_ix_071234fc,
	&MC68000BASE::x51c0_sf_b_071234fc,
	&MC68000BASE::x51c8_dbf_w_071234fc,
	&MC68000BASE::x51d0_sf_b_ai_071234fc,
	&MC68000BASE::x51d8_sf_b_pi_071234fc,
	&MC68000BASE::x51e0_sf_b_pd_071234fc,
	&MC68000BASE::x51e8_sf_b_di_071234fc,
	&MC68000BASE::x51f0_sf_b_ix_071234fc,
	&MC68000BASE::x52c0_shi_b_071234fc,
	&MC68000BASE::x52c8_dbhi_w_071234fc,
	&MC68000BASE::x52d0_shi_b_ai_071234fc,
	&MC68000BASE::x52d8_shi_b_pi_071234fc,
	&MC68000BASE::x52e0_shi_b_pd_071234fc,
	&MC68000BASE::x52e8_shi_b_di_071234fc,
	&MC68000BASE::x52f0_shi_b_ix_071234fc,
	&MC68000BASE::x53c0_sls_b_071234fc,
	&MC68000BASE::x53c8_dbls_w_071234fc,
	&MC68000BASE::x53d0_sls_b_ai_071234fc,
	&MC68000BASE::x53d8_sls_b_pi_071234fc,
	&MC68000BASE::x53e0_sls_b_pd_071234fc,
	&MC68000BASE::x53e8_sls_b_di_071234fc,
	&MC68000BASE::x53f0_sls_b_ix_071234fc,
	&MC68000BASE::x54c0_scc_b_071234fc,
	&MC68000BASE::x54c8_dbcc_w_071234fc,
	&MC68000BASE::x54d0_scc_b_ai_071234fc,
	&MC68000BASE::x54d8_scc_b_pi_071234fc,
	&MC68000BASE::x54e0_scc_b_pd_071234fc,
	&MC68000BASE::x54e8_scc_b_di_071234fc,
	&MC68000BASE::x54f0_scc_b_ix_071234fc,
	&MC68000BASE::x55c0_scs_b_071234fc,
	&MC68000BASE::x55c8_dbcs_w_071234fc,
	&MC68000BASE::x55d0_scs_b_ai_071234fc,
	&MC68000BASE::x55d8_scs_b_pi_071234fc,
	&MC68000BASE::x55e0_scs_b_pd_071234fc,
	&MC68000BASE::x55e8_scs_b_di_071234fc,
	&MC68000BASE::x55f0_scs_b_ix_071234fc,
	&MC68000BASE::x56c0_sne_b_071234fc,
	&MC68000BASE::x56c8_dbne_w_071234fc,
	&MC68000BASE::x56d0_sne_b_ai_071234fc,
	&MC68000BASE::x56d8_sne_b_pi_071234fc,
	&MC68000BASE::x56e0_sne_b_pd_071234fc,
	&MC68000BASE::x56e8_sne_b_di_071234fc,
	&MC68000BASE::x56f0_sne_b_ix_071234fc,
	&MC68000BASE::x57c0_seq_b_071234fc,
	&MC68000BASE::x57c8_dbeq_w_071234fc,
	&MC68000BASE::x57d0_seq_b_ai_071234fc,
	&MC68000BASE::x57d8_seq_b_pi_071234fc,
	&MC68000BASE::x57e0_seq_b_pd_071234fc,
	&MC68000BASE::x57e8_seq_b_di_071234fc,
	&MC68000BASE::x57f0_seq_b_ix_071234fc,
	&MC68000BASE::x58c0_svc_b_071234fc,
	&MC68000BASE::x58c8_dbvc_w_071234fc,
	&MC68000BASE::x58d0_svc_b_ai_071234fc,
	&MC68000BASE::x58d8_svc_b_pi_071234fc,
	&MC68000BASE::x58e0_svc_b_pd_071234fc,
	&MC68000BASE::x58e8_svc_b_di_071234fc,
	&MC68000BASE::x58f0_svc_b_ix_071234fc,
	&MC68000BASE::x59c0_svs_b_071234fc,
	&MC68000BASE::x59c8_dbvs_w_071234fc,
	&MC68000BASE::x59d0_svs_b_ai_071234fc,
	&MC68000BASE::x59d8_svs_b_pi_071234fc,
	&MC68000BASE::x59e0_svs_b_pd_071234fc,
	&MC68000BASE::x59e8_svs_b_di_071234fc,
	&MC68000BASE::x59f0_svs_b_ix_071234fc,
	&MC68000BASE::x5ac0_spl_b_071234fc,
	&MC68000BASE::x5ac8_dbpl_w_071234fc,
	&MC68000BASE::x5ad0_spl_b_ai_071234fc,
	&MC68000BASE::x5ad8_spl_b_pi_071234fc,
	&MC68000BASE::x5ae0_spl_b_pd_071234fc,
	&MC68000BASE::x5ae8_spl_b_di_071234fc,
	&MC68000BASE::x5af0_spl_b_ix_071234fc,
	&MC68000BASE::x5bc0_smi_b_071234fc,
	&MC68000BASE::x5bc8_dbmi_w_071234fc,
	&MC68000BASE::x5bd0_smi_b_ai_071234fc,
	&MC68000BASE::x5bd8_smi_b_pi_071234fc,
	&MC68000BASE::x5be0_smi_b_pd_071234fc,
	&MC68000BASE::x5be8_smi_b_di_071234fc,
	&MC68000BASE::x5bf0_smi_b_ix_071234fc,
	&MC68000BASE::x5cc0_sge_b_071234fc,
	&MC68000BASE::x5cc8_dbge_w_071234fc,
	&MC68000BASE::x5cd0_sge_b_ai_071234fc,
	&MC68000BASE::x5cd8_sge_b_pi_071234fc,
	&MC68000BASE::x5ce0_sge_b_pd_071234fc,
	&MC68000BASE::x5ce8_sge_b_di_071234fc,
	&MC68000BASE::x5cf0_sge_b_ix_071234fc,
	&MC68000BASE::x5dc0_slt_b_071234fc,
	&MC68000BASE::x5dc8_dblt_w_071234fc,
	&MC68000BASE::x5dd0_slt_b_ai_071234fc,
	&MC68000BASE::x5dd8_slt_b_pi_071234fc,
	&MC68000BASE::x5de0_slt_b_pd_071234fc,
	&MC68000BASE::x5de8_slt_b_di_071234fc,
	&MC68000BASE::x5df0_slt_b_ix_071234fc,
	&MC68000BASE::x5ec0_sgt_b_071234fc,
	&MC68000BASE::x5ec8_dbgt_w_071234fc,
	&MC68000BASE::x5ed0_sgt_b_ai_071234fc,
	&MC68000BASE::x5ed8_sgt_b_pi_071234fc,
	&MC68000BASE::x5ee0_sgt_b_pd_071234fc,
	&MC68000BASE::x5ee8_sgt_b_di_071234fc,
	&MC68000BASE::x5ef0_sgt_b_ix_071234fc,
	&MC68000BASE::x5fc0_sle_b_071234fc,
	&MC68000BASE::x5fc8_dble_w_071234fc,
	&MC68000BASE::x5fd0_sle_b_ai_071234fc,
	&MC68000BASE::x5fd8_sle_b_pi_071234fc,
	&MC68000BASE::x5fe0_sle_b_pd_071234fc,
	&MC68000BASE::x5fe8_sle_b_di_071234fc,
	&MC68000BASE::x5ff0_sle_b_ix_071234fc,
	&MC68000BASE::x8f08_sbcd_b_071234fc,
	&MC68000BASE::x8f48_pack_w_234fc,
	&MC68000BASE::x8f88_unpk_w_234fc,
	&MC68000BASE::x9f08_subx_b_071234fc,
	&MC68000BASE::xbf08_cmpm_b_071234fc,
	&MC68000BASE::xcf08_abcd_b_071234fc,
	&MC68000BASE::xdf08_addx_b_071234fc,
	&MC68000BASE::xe0d0_asr_w_ai_071234fc,
	&MC68000BASE::xe0d8_asr_w_pi_071234fc,
	&MC68000BASE::xe0e0_asr_w_pd_071234fc,
	&MC68000BASE::xe0e8_asr_w_di_071234fc,
	&MC68000BASE::xe0f0_asr_w_ix_071234fc,
	&MC68000BASE::xe1d0_asl_w_ai_071234fc,
	&MC68000BASE::xe1d8_asl_w_pi_071234fc,
	&MC68000BASE::xe1e0_asl_w_pd_071234fc,
	&MC68000BASE::xe1e8_asl_w_di_071234fc,
	&MC68000BASE::xe1f0_asl_w_ix_071234fc,
	&MC68000BASE::xe2d0_lsr_w_ai_071234fc,
	&MC68000BASE::xe2d8_lsr_w_pi_071234fc,
	&MC68000BASE::xe2e0_lsr_w_pd_071234fc,
	&MC68000BASE::xe2e8_lsr_w_di_071234fc,
	&MC68000BASE::xe2f0_lsr_w_ix_071234fc,
	&MC68000BASE::xe3d0_lsl_w_ai_071234fc,
	&MC68000BASE::xe3d8_lsl_w_pi_071234fc,
	&MC68000BASE::xe3e0_lsl_w_pd_071234fc,
	&MC68000BASE::xe3e8_lsl_w_di_071234fc,
	&MC68000BASE::xe3f0_lsl_w_ix_071234fc,
	&MC68000BASE::xe4d0_roxr_w_ai_071234fc,
	&MC68000BASE::xe4d8_roxr_w_pi_071234fc,
	&MC68000BASE::xe4e0_roxr_w_pd_071234fc,
	&MC68000BASE::xe4e8_roxr_w_di_071234fc,
	&MC68000BASE::xe4f0_roxr_w_ix_071234fc,
	&MC68000BASE::xe5d0_roxl_w_ai_071234fc,
	&MC68000BASE::xe5d8_roxl_w_pi_071234fc,
	&MC68000BASE::xe5e0_roxl_w_pd_071234fc,
	&MC68000BASE::xe5e8_roxl_w_di_071234fc,
	&MC68000BASE::xe5f0_roxl_w_ix_071234fc,
	&MC68000BASE::xe6d0_ror_w_ai_071234fc,
	&MC68000BASE::xe6d8_ror_w_pi_071234fc,
	&MC68000BASE::xe6e0_ror_w_pd_071234fc,
	&MC68000BASE::xe6e8_ror_w_di_071234fc,
	&MC68000BASE::xe6f0_ror_w_ix_071234fc,
	&MC68000BASE::xe7d0_rol_w_ai_071234fc,
	&MC68000BASE::xe7d8_rol_w_pi_071234fc,
	&MC68000BASE::xe7e0_rol_w_pd_071234fc,
	&MC68000BASE::xe7e8_rol_w_di_071234fc,
	&MC68000BASE::xe7f0_rol_w_ix_071234fc,
	&MC68000BASE::xe8c0_bftst_l_234fc,
	&MC68000BASE::xe8d0_bftst_l_ai_234fc,
	&MC68000BASE::xe8e8_bftst_l_di_234fc,
	&MC68000BASE::xe8f0_bftst_l_ix_234fc,
	&MC68000BASE::xe9c0_bfextu_l_234fc,
	&MC68000BASE::xe9d0_bfextu_l_ai_234fc,
	&MC68000BASE::xe9e8_bfextu_l_di_234fc,
	&MC68000BASE::xe9f0_bfextu_l_ix_234fc,
	&MC68000BASE::xeac0_bfchg_l_234fc,
	&MC68000BASE::xead0_bfchg_l_ai_234fc,
	&MC68000BASE::xeae8_bfchg_l_di_234fc,
	&MC68000BASE::xeaf0_bfchg_l_ix_234fc,
	&MC68000BASE::xebc0_bfexts_l_234fc,
	&MC68000BASE::xebd0_bfexts_l_ai_234fc,
	&MC68000BASE::xebe8_bfexts_l_di_234fc,
	&MC68000BASE::xebf0_bfexts_l_ix_234fc,
	&MC68000BASE::xecc0_bfclr_l_234fc,
	&MC68000BASE::xecd0_bfclr_l_ai_234fc,
	&MC68000BASE::xece8_bfclr_l_di_234fc,
	&MC68000BASE::xecf0_bfclr_l_ix_234fc,
	&MC68000BASE::xedc0_bfffo_l_234fc,
	&MC68000BASE::xedd0_bfffo_l_ai_234fc,
	&MC68000BASE::xede8_bfffo_l_di_234fc,
	&MC68000BASE::xedf0_bfffo_l_ix_234fc,
	&MC68000BASE::xeec0_bfset_l_234fc,
	&MC68000BASE::xeed0_bfset_l_ai_234fc,
	&MC68000BASE::xeee8_bfset_l_di_234fc,
	&MC68000BASE::xeef0_bfset_l_ix_234fc,
	&MC68000BASE::xefc0_bfins_l_234fc,
	&MC68000BASE::xefd0_bfins_l_ai_234fc,
	&MC68000BASE::xefe8_bfins_l_di_234fc,
	&MC68000BASE::xeff0_bfins_l_ix_234fc,
	&MC68000BASE::xf278_ftrapcc_l_23,
	&MC68000BASE::xf510_pflushan_l_4fc,
	&MC68000BASE::xf518_pflusha_l_4fc,
	&MC68000BASE::xf620_move16_l_4fc,
	&MC68000BASE::x001f_ori_b_pi7_071234fc,
	&MC68000BASE::x0027_ori_b_pd7_071234fc,
	&MC68000BASE::x0038_ori_b_aw_071234fc,
	&MC68000BASE::x0039_ori_b_al_071234fc,
	&MC68000BASE::x003c_ori_w_071234fc,
	&MC68000BASE::x0078_ori_w_aw_071234fc,
	&MC68000BASE::x0079_ori_w_al_071234fc,
	&MC68000BASE::x007c_ori_w_071234fc,
	&MC68000BASE::x00b8_ori_l_aw_071234fc,
	&MC68000BASE::x00b9_ori_l_al_071234fc,
	&MC68000BASE::x00f8_chk2cmp2_b_aw_234fc,
	&MC68000BASE::x00f9_chk2cmp2_b_al_234fc,
	&MC68000BASE::x00fa_chk2cmp2_b_234fc,
	&MC68000BASE::x00fb_chk2cmp2_b_234fc,
	&MC68000BASE::x021f_andi_b_pi7_071234fc,
	&MC68000BASE::x0227_andi_b_pd7_071234fc,
	&MC68000BASE::x0238_andi_b_aw_071234fc,
	&MC68000BASE::x0239_andi_b_al_071234fc,
	&MC68000BASE::x023c_andi_w_071234fc,
	&MC68000BASE::x0278_andi_w_aw_071234fc,
	&MC68000BASE::x0279_andi_w_al_071234fc,
	&MC68000BASE::x027c_andi_w_071234fc,
	&MC68000BASE::x02b8_andi_l_aw_071234fc,
	&MC68000BASE::x02b9_andi_l_al_071234fc,
	&MC68000BASE::x02f8_chk2cmp2_w_aw_234fc,
	&MC68000BASE::x02f9_chk2cmp2_w_al_234fc,
	&MC68000BASE::x02fa_chk2cmp2_w_234fc,
	&MC68000BASE::x02fb_chk2cmp2_w_234fc,
	&MC68000BASE::x041f_subi_b_pi7_071234fc,
	&MC68000BASE::x0427_subi_b_pd7_071234fc,
	&MC68000BASE::x0438_subi_b_aw_071234fc,
	&MC68000BASE::x0439_subi_b_al_071234fc,
	&MC68000BASE::x0478_subi_w_aw_071234fc,
	&MC68000BASE::x0479_subi_w_al_071234fc,
	&MC68000BASE::x04b8_subi_l_aw_071234fc,
	&MC68000BASE::x04b9_subi_l_al_071234fc,
	&MC68000BASE::x04f8_chk2cmp2_l_aw_234fc,
	&MC68000BASE::x04f9_chk2cmp2_l_al_234fc,
	&MC68000BASE::x04fa_chk2cmp2_l_234fc,
	&MC68000BASE::x04fb_chk2cmp2_l_234fc,
	&MC68000BASE::x061f_addi_b_pi7_071234fc,
	&MC68000BASE::x0627_addi_b_pd7_071234fc,
	&MC68000BASE::x0638_addi_b_aw_071234fc,
	&MC68000BASE::x0639_addi_b_al_071234fc,
	&MC68000BASE::x0678_addi_w_aw_071234fc,
	&MC68000BASE::x0679_addi_w_al_071234fc,
	&MC68000BASE::x06b8_addi_l_aw_071234fc,
	&MC68000BASE::x06b9_addi_l_al_071234fc,
	&MC68000BASE::x06f8_callm_l_aw_2f,
	&MC68000BASE::x06f9_callm_l_al_2f,
	&MC68000BASE::x06fa_callm_l_pcdi_2f,
	&MC68000BASE::x06fb_callm_l_pcix_2f,
	&MC68000BASE::x081f_btst_b_pi7_071234fc,
	&MC68000BASE::x0827_btst_b_pd7_071234fc,
	&MC68000BASE::x0838_btst_b_aw_071234fc,
	&MC68000BASE::x0839_btst_b_al_071234fc,
	&MC68000BASE::x083a_btst_b_pcdi_071234fc,
	&MC68000BASE::x083b_btst_b_pcix_071234fc,
	&MC68000BASE::x085f_bchg_b_pi7_071234fc,
	&MC68000BASE::x0867_bchg_b_pd7_071234fc,
	&MC68000BASE::x0878_bchg_b_aw_071234fc,
	&MC68000BASE::x0879_bchg_b_al_071234fc,
	&MC68000BASE::x089f_bclr_b_pi7_071234fc,
	&MC68000BASE::x08a7_bclr_b_pd7_071234fc,
	&MC68000BASE::x08b8_bclr_b_aw_071234fc,
	&MC68000BASE::x08b9_bclr_b_al_071234fc,
	&MC68000BASE::x08df_bset_b_pi7_071234fc,
	&MC68000BASE::x08e7_bset_b_pd7_071234fc,
	&MC68000BASE::x08f8_bset_b_aw_071234fc,
	&MC68000BASE::x08f9_bset_b_al_071234fc,
	&MC68000BASE::x0a1f_eori_b_pi7_071234fc,
	&MC68000BASE::x0a27_eori_b_pd7_071234fc,
	&MC68000BASE::x0a38_eori_b_aw_071234fc,
	&MC68000BASE::x0a39_eori_b_al_071234fc,
	&MC68000BASE::x0a3c_eori_w_071234fc,
	&MC68000BASE::x0a78_eori_w_aw_071234fc,
	&MC68000BASE::x0a79_eori_w_al_071234fc,
	&MC68000BASE::x0a7c_eori_w_071234fc,
	&MC68000BASE::x0ab8_eori_l_aw_071234fc,
	&MC68000BASE::x0ab9_eori_l_al_071234fc,
	&MC68000BASE::x0adf_cas_b_pi7_234fc,
	&MC68000BASE::x0ae7_cas_b_pd7_234fc,
	&MC68000BASE::x0af8_cas_b_aw_234fc,
	&MC68000BASE::x0af9_cas_b_al_234fc,
	&MC68000BASE::x0c1f_cmpi_b_pi7_071234fc,
	&MC68000BASE::x0c27_cmpi_b_pd7_071234fc,
	&MC68000BASE::x0c38_cmpi_b_aw_071234fc,
	&MC68000BASE::x0c39_cmpi_b_al_071234fc,
	&MC68000BASE::x0c3a_cmpi_b_234fc,
	&MC68000BASE::x0c3b_cmpi_b_234fc,
	&MC68000BASE::x0c78_cmpi_w_aw_071234fc,
	&MC68000BASE::x0c79_cmpi_w_al_071234fc,
	&MC68000BASE::x0c7a_cmpi_w_234fc,
	&MC68000BASE::x0c7b_cmpi_w_234fc,
	&MC68000BASE::x0cb8_cmpi_l_aw_071234fc,
	&MC68000BASE::x0cb9_cmpi_l_al_071234fc,
	&MC68000BASE::x0cba_cmpi_l_234fc,
	&MC68000BASE::x0cbb_cmpi_l_234fc,
	&MC68000BASE::x0cf8_cas_w_aw_234fc,
	&MC68000BASE::x0cf9_cas_w_al_234fc,
	&MC68000BASE::x0cfc_cas2_w_234fc,
	&MC68000BASE::x0e1f_moves_b_pi7_134fc,
	&MC68000BASE::x0e1f_moves_b_pi7_2,
	&MC68000BASE::x0e27_moves_b_pd7_134fc,
	&MC68000BASE::x0e27_moves_b_pd7_2,
	&MC68000BASE::x0e38_moves_b_aw_134fc,
	&MC68000BASE::x0e38_moves_b_aw_2,
	&MC68000BASE::x0e39_moves_b_al_134fc,
	&MC68000BASE::x0e39_moves_b_al_2,
	&MC68000BASE::x0e78_moves_w_aw_134fc,
	&MC68000BASE::x0e78_moves_w_aw_2,
	&MC68000BASE::x0e79_moves_w_al_134fc,
	&MC68000BASE::x0e79_moves_w_al_2,
	&MC68000BASE::x0eb8_moves_l_aw_134fc,
	&MC68000BASE::x0eb8_moves_l_aw_2,
	&MC68000BASE::x0eb9_moves_l_al_134fc,
	&MC68000BASE::x0eb9_moves_l_al_2,
	&MC68000BASE::x0ef8_cas_l_aw_234fc,
	&MC68000BASE::x0ef9_cas_l_al_234fc,
	&MC68000BASE::x0efc_cas2_l_234fc,
	&MC68000BASE::x11df_move_b_pi7_071234fc,
	&MC68000BASE::x11e7_move_b_pd7_071234fc,
	&MC68000BASE::x11f8_move_b_aw_071234fc,
	&MC68000BASE::x11f9_move_b_al_071234fc,
	&MC68000BASE::x11fa_move_b_pcdi_071234fc,
	&MC68000BASE::x11fb_move_b_pcix_071234fc,
	&MC68000BASE::x11fc_move_b_i_071234fc,
	&MC68000BASE::x13df_move_b_pi7_071234fc,
	&MC68000BASE::x13e7_move_b_pd7_071234fc,
	&MC68000BASE::x13f8_move_b_aw_071234fc,
	&MC68000BASE::x13f9_move_b_al_071234fc,
	&MC68000BASE::x13fa_move_b_pcdi_071234fc,
	&MC68000BASE::x13fb_move_b_pcix_071234fc,
	&MC68000BASE::x13fc_move_b_i_071234fc,
	&MC68000BASE::x1edf_move_b_pi7_071234fc,
	&MC68000BASE::x1ee7_move_b_pd7_071234fc,
	&MC68000BASE::x1ef8_move_b_aw_071234fc,
	&MC68000BASE::x1ef9_move_b_al_071234fc,
	&MC68000BASE::x1efa_move_b_pcdi_071234fc,
	&MC68000BASE::x1efb_move_b_pcix_071234fc,
	&MC68000BASE::x1efc_move_b_i_071234fc,
	&MC68000BASE::x1f1f_move_b_pi7_071234fc,
	&MC68000BASE::x1f27_move_b_pd7_071234fc,
	&MC68000BASE::x1f38_move_b_aw_071234fc,
	&MC68000BASE::x1f39_move_b_al_071234fc,
	&MC68000BASE::x1f3a_move_b_pcdi_071234fc,
	&MC68000BASE::x1f3b_move_b_pcix_071234fc,
	&MC68000BASE::x1f3c_move_b_i_071234fc,
	&MC68000BASE::x21f8_move_l_aw_071234fc,
	&MC68000BASE::x21f9_move_l_al_071234fc,
	&MC68000BASE::x21fa_move_l_pcdi_071234fc,
	&MC68000BASE::x21fb_move_l_pcix_071234fc,
	&MC68000BASE::x21fc_move_l_i_071234fc,
	&MC68000BASE::x23f8_move_l_aw_071234fc,
	&MC68000BASE::x23f9_move_l_al_071234fc,
	&MC68000BASE::x23fa_move_l_pcdi_071234fc,
	&MC68000BASE::x23fb_move_l_pcix_071234fc,
	&MC68000BASE::x23fc_move_l_i_071234fc,
	&MC68000BASE::x31f8_move_w_aw_071234fc,
	&MC68000BASE::x31f9_move_w_al_071234fc,
	&MC68000BASE::x31fa_move_w_pcdi_071234fc,
	&MC68000BASE::x31fb_move_w_pcix_071234fc,
	&MC68000BASE::x31fc_move_w_i_071234fc,
	&MC68000BASE::x33f8_move_w_aw_071234fc,
	&MC68000BASE::x33f9_move_w_al_071234fc,
	&MC68000BASE::x33fa_move_w_pcdi_071234fc,
	&MC68000BASE::x33fb_move_w_pcix_071234fc,
	&MC68000BASE::x33fc_move_w_i_071234fc,
	&MC68000BASE::x401f_negx_b_pi7_071234fc,
	&MC68000BASE::x4027_negx_b_pd7_071234fc,
	&MC68000BASE::x4038_negx_b_aw_071234fc,
	&MC68000BASE::x4039_negx_b_al_071234fc,
	&MC68000BASE::x4078_negx_w_aw_071234fc,
	&MC68000BASE::x4079_negx_w_al_071234fc,
	&MC68000BASE::x40b8_negx_l_aw_071234fc,
	&MC68000BASE::x40b9_negx_l_al_071234fc,
	&MC68000BASE::x40f8_move_w_aw_07,
	&MC68000BASE::x40f8_move_w_aw_1234fc,
	&MC68000BASE::x40f9_move_w_al_07,
	&MC68000BASE::x40f9_move_w_al_1234fc,
	&MC68000BASE::x421f_clr_b_pi7_0,
	&MC68000BASE::x421f_clr_b_pi7_71234fc,
	&MC68000BASE::x4227_clr_b_pd7_0,
	&MC68000BASE::x4227_clr_b_pd7_71234fc,
	&MC68000BASE::x4238_clr_b_aw_0,
	&MC68000BASE::x4238_clr_b_aw_71234fc,
	&MC68000BASE::x4239_clr_b_al_0,
	&MC68000BASE::x4239_clr_b_al_71234fc,
	&MC68000BASE::x4278_clr_w_aw_0,
	&MC68000BASE::x4278_clr_w_aw_71234fc,
	&MC68000BASE::x4279_clr_w_al_0,
	&MC68000BASE::x4279_clr_w_al_71234fc,
	&MC68000BASE::x42b8_clr_l_aw_0,
	&MC68000BASE::x42b8_clr_l_aw_71234fc,
	&MC68000BASE::x42b9_clr_l_al_0,
	&MC68000BASE::x42b9_clr_l_al_71234fc,
	&MC68000BASE::x42f8_move_w_aw_1234fc,
	&MC68000BASE::x42f9_move_w_al_1234fc,
	&MC68000BASE::x441f_neg_b_pi7_071234fc,
	&MC68000BASE::x4427_neg_b_pd7_071234fc,
	&MC68000BASE::x4438_neg_b_aw_071234fc,
	&MC68000BASE::x4439_neg_b_al_071234fc,
	&MC68000BASE::x4478_neg_w_aw_071234fc,
	&MC68000BASE::x4479_neg_w_al_071234fc,
	&MC68000BASE::x44b8_neg_l_aw_071234fc,
	&MC68000BASE::x44b9_neg_l_al_071234fc,
	&MC68000BASE::x44f8_move_w_aw_071234fc,
	&MC68000BASE::x44f9_move_w_al_071234fc,
	&MC68000BASE::x44fa_move_w_pcdi_071234fc,
	&MC68000BASE::x44fb_move_w_pcix_071234fc,
	&MC68000BASE::x44fc_move_w_i_071234fc,
	&MC68000BASE::x461f_not_b_pi7_071234fc,
	&MC68000BASE::x4627_not_b_pd7_071234fc,
	&MC68000BASE::x4638_not_b_aw_071234fc,
	&MC68000BASE::x4639_not_b_al_071234fc,
	&MC68000BASE::x4678_not_w_aw_071234fc,
	&MC68000BASE::x4679_not_w_al_071234fc,
	&MC68000BASE::x46b8_not_l_aw_071234fc,
	&MC68000BASE::x46b9_not_l_al_071234fc,
	&MC68000BASE::x46f8_move_w_aw_071234fc,
	&MC68000BASE::x46f9_move_w_al_071234fc,
	&MC68000BASE::x46fa_move_w_pcdi_071234fc,
	&MC68000BASE::x46fb_move_w_pcix_071234fc,
	&MC68000BASE::x46fc_move_w_i_071234fc,
	&MC68000BASE::x480f_link_l_234fc,
	&MC68000BASE::x481f_nbcd_b_pi7_071234fc,
	&MC68000BASE::x4827_nbcd_b_pd7_071234fc,
	&MC68000BASE::x4838_nbcd_b_aw_071234fc,
	&MC68000BASE::x4839_nbcd_b_al_071234fc,
	&MC68000BASE::x4878_pea_l_aw_071234fc,
	&MC68000BASE::x4879_pea_l_al_071234fc,
	&MC68000BASE::x487a_pea_l_pcdi_071234fc,
	&MC68000BASE::x487b_pea_l_pcix_071234fc,
	&MC68000BASE::x48b8_movem_w_aw_071234fc,
	&MC68000BASE::x48b9_movem_w_al_071234fc,
	&MC68000BASE::x48f8_movem_l_aw_071234fc,
	&MC68000BASE::x48f9_movem_l_al_071234fc,
	&MC68000BASE::x4a1f_tst_b_pi7_071234fc,
	&MC68000BASE::x4a27_tst_b_pd7_071234fc,
	&MC68000BASE::x4a38_tst_b_aw_071234fc,
	&MC68000BASE::x4a39_tst_b_al_071234fc,
	&MC68000BASE::x4a3a_tst_b_234fc,
	&MC68000BASE::x4a3b_tst_b_234fc,
	&MC68000BASE::x4a3c_tst_b_234fc,
	&MC68000BASE::x4a78_tst_w_aw_071234fc,
	&MC68000BASE::x4a79_tst_w_al_071234fc,
	&MC68000BASE::x4a7a_tst_w_234fc,
	&MC68000BASE::x4a7b_tst_w_234fc,
	&MC68000BASE::x4a7c_tst_w_234fc,
	&MC68000BASE::x4ab8_tst_l_aw_071234fc,
	&MC68000BASE::x4ab9_tst_l_al_071234fc,
	&MC68000BASE::x4aba_tst_l_234fc,
	&MC68000BASE::x4abb_tst_l_234fc,
	&MC68000BASE::x4abc_tst_l_234fc,
	&MC68000BASE::x4adf_tas_b_pi7_071234fc,
	&MC68000BASE::x4ae7_tas_b_pd7_071234fc,
	&MC68000BASE::x4af8_tas_b_aw_071234fc,
	&MC68000BASE::x4af9_tas_b_al_071234fc,
	&MC68000BASE::x4afc_illegal_071234fc,
	&MC68000BASE::x4c38_mull_l_aw_234fc,
	&MC68000BASE::x4c39_mull_l_al_234fc,
	&MC68000BASE::x4c3a_mull_l_pcdi_234fc,
	&MC68000BASE::x4c3b_mull_l_pcix_234fc,
	&MC68000BASE::x4c3c_mull_l_i_234fc,
	&MC68000BASE::x4c78_divl_l_aw_234fc,
	&MC68000BASE::x4c79_divl_l_al_234fc,
	&MC68000BASE::x4c7a_divl_l_pcdi_234fc,
	&MC68000BASE::x4c7b_divl_l_pcix_234fc,
	&MC68000BASE::x4c7c_divl_l_i_234fc,
	&MC68000BASE::x4cb8_movem_w_aw_071234fc,
	&MC68000BASE::x4cb9_movem_w_al_071234fc,
	&MC68000BASE::x4cba_movem_w_071234fc,
	&MC68000BASE::x4cbb_movem_w_071234fc,
	&MC68000BASE::x4cf8_movem_l_aw_071234fc,
	&MC68000BASE::x4cf9_movem_l_al_071234fc,
	&MC68000BASE::x4cfa_movem_l_071234fc,
	&MC68000BASE::x4cfb_movem_l_071234fc,
	&MC68000BASE::x4e57_link_w_071234fc,
	&MC68000BASE::x4e5f_unlk_l_071234fc,
	&MC68000BASE::x4e70_reset_071234fc,
	&MC68000BASE::x4e71_nop_071234fc,
	&MC68000BASE::x4e72_stop_071234fc,
	&MC68000BASE::x4e73_rte_l_0,
	&MC68000BASE::x4e73_rte_l_71,
	&MC68000BASE::x4e73_rte_l_234fc,
	&MC68000BASE::x4e74_rtd_l_1234fc,
	&MC68000BASE::x4e75_rts_l_071234fc,
	&MC68000BASE::x4e76_trapv_071234fc,
	&MC68000BASE::x4e77_rtr_l_071234fc,
	&MC68000BASE::x4e7a_movec_l_1,
	&MC68000BASE::x4e7a_movec_l_23f,
	&MC68000BASE::x4e7a_movec_l_4,
	&MC68000BASE::x4e7a_movec_l_c,
	&MC68000BASE::x4e7b_movec_l_1,
	&MC68000BASE::x4e7b_movec_l_2f,
	&MC68000BASE::x4e7b_movec_l_3,
	&MC68000BASE::x4e7b_movec_l_4,
	&MC68000BASE::x4e7b_movec_l_c,
	&MC68000BASE::x4eb8_jsr_l_aw_071234fc,
	&MC68000BASE::x4eb9_jsr_l_al_071234fc,
	&MC68000BASE::x4eba_jsr_l_pcdi_071234fc,
	&MC68000BASE::x4ebb_jsr_l_pcix_071234fc,
	&MC68000BASE::x4ef8_jmp_l_aw_071234fc,
	&MC68000BASE::x4ef9_jmp_l_al_071234fc,
	&MC68000BASE::x4efa_jmp_l_pcdi_071234fc,
	&MC68000BASE::x4efb_jmp_l_pcix_071234fc,
	&MC68000BASE::x50df_st_b_pi7_071234fc,
	&MC68000BASE::x50e7_st_b_pd7_071234fc,
	&MC68000BASE::x50f8_st_b_aw_071234fc,
	&MC68000BASE::x50f9_st_b_al_071234fc,
	&MC68000BASE::x50fa_trapt_w_234fc,
	&MC68000BASE::x50fb_trapt_l_234fc,
	&MC68000BASE::x50fc_trapt_234fc,
	&MC68000BASE::x51df_sf_b_pi7_071234fc,
	&MC68000BASE::x51e7_sf_b_pd7_071234fc,
	&MC68000BASE::x51f8_sf_b_aw_071234fc,
	&MC68000BASE::x51f9_sf_b_al_071234fc,
	&MC68000BASE::x51fa_trapf_w_234fc,
	&MC68000BASE::x51fb_trapf_l_234fc,
	&MC68000BASE::x51fc_trapf_234fc,
	&MC68000BASE::x52df_shi_b_pi7_071234fc,
	&MC68000BASE::x52e7_shi_b_pd7_071234fc,
	&MC68000BASE::x52f8_shi_b_aw_071234fc,
	&MC68000BASE::x52f9_shi_b_al_071234fc,
	&MC68000BASE::x52fa_traphi_w_234fc,
	&MC68000BASE::x52fb_traphi_l_234fc,
	&MC68000BASE::x52fc_traphi_234fc,
	&MC68000BASE::x53df_sls_b_pi7_071234fc,
	&MC68000BASE::x53e7_sls_b_pd7_071234fc,
	&MC68000BASE::x53f8_sls_b_aw_071234fc,
	&MC68000BASE::x53f9_sls_b_al_071234fc,
	&MC68000BASE::x53fa_trapls_w_234fc,
	&MC68000BASE::x53fb_trapls_l_234fc,
	&MC68000BASE::x53fc_trapls_234fc,
	&MC68000BASE::x54df_scc_b_pi7_071234fc,
	&MC68000BASE::x54e7_scc_b_pd7_071234fc,
	&MC68000BASE::x54f8_scc_b_aw_071234fc,
	&MC68000BASE::x54f9_scc_b_al_071234fc,
	&MC68000BASE::x54fa_trapcc_w_234fc,
	&MC68000BASE::x54fb_trapcc_l_234fc,
	&MC68000BASE::x54fc_trapcc_234fc,
	&MC68000BASE::x55df_scs_b_pi7_071234fc,
	&MC68000BASE::x55e7_scs_b_pd7_071234fc,
	&MC68000BASE::x55f8_scs_b_aw_071234fc,
	&MC68000BASE::x55f9_scs_b_al_071234fc,
	&MC68000BASE::x55fa_trapcs_w_234fc,
	&MC68000BASE::x55fb_trapcs_l_234fc,
	&MC68000BASE::x55fc_trapcs_234fc,
	&MC68000BASE::x56df_sne_b_pi7_071234fc,
	&MC68000BASE::x56e7_sne_b_pd7_071234fc,
	&MC68000BASE::x56f8_sne_b_aw_071234fc,
	&MC68000BASE::x56f9_sne_b_al_071234fc,
	&MC68000BASE::x56fa_trapne_w_234fc,
	&MC68000BASE::x56fb_trapne_l_234fc,
	&MC68000BASE::x56fc_trapne_234fc,
	&MC68000BASE::x57df_seq_b_pi7_071234fc,
	&MC68000BASE::x57e7_seq_b_pd7_071234fc,
	&MC68000BASE::x57f8_seq_b_aw_071234fc,
	&MC68000BASE::x57f9_seq_b_al_071234fc,
	&MC68000BASE::x57fa_trapeq_w_234fc,
	&MC68000BASE::x57fb_trapeq_l_234fc,
	&MC68000BASE::x57fc_trapeq_234fc,
	&MC68000BASE::x58df_svc_b_pi7_071234fc,
	&MC68000BASE::x58e7_svc_b_pd7_071234fc,
	&MC68000BASE::x58f8_svc_b_aw_071234fc,
	&MC68000BASE::x58f9_svc_b_al_071234fc,
	&MC68000BASE::x58fa_trapvc_w_234fc,
	&MC68000BASE::x58fb_trapvc_l_234fc,
	&MC68000BASE::x58fc_trapvc_234fc,
	&MC68000BASE::x59df_svs_b_pi7_071234fc,
	&MC68000BASE::x59e7_svs_b_pd7_071234fc,
	&MC68000BASE::x59f8_svs_b_aw_071234fc,
	&MC68000BASE::x59f9_svs_b_al_071234fc,
	&MC68000BASE::x59fa_trapvs_w_234fc,
	&MC68000BASE::x59fb_trapvs_l_234fc,
	&MC68000BASE::x59fc_trapvs_234fc,
	&MC68000BASE::x5adf_spl_b_pi7_071234fc,
	&MC68000BASE::x5ae7_spl_b_pd7_071234fc,
	&MC68000BASE::x5af8_spl_b_aw_071234fc,
	&MC68000BASE::x5af9_spl_b_al_071234fc,
	&MC68000BASE::x5afa_trappl_w_234fc,
	&MC68000BASE::x5afb_trappl_l_234fc,
	&MC68000BASE::x5afc_trappl_234fc,
	&MC68000BASE::x5bdf_smi_b_pi7_071234fc,
	&MC68000BASE::x5be7_smi_b_pd7_071234fc,
	&MC68000BASE::x5bf8_smi_b_aw_071234fc,
	&MC68000BASE::x5bf9_smi_b_al_071234fc,
	&MC68000BASE::x5bfa_trapmi_w_234fc,
	&MC68000BASE::x5bfb_trapmi_l_234fc,
	&MC68000BASE::x5bfc_trapmi_234fc,
	&MC68000BASE::x5cdf_sge_b_pi7_071234fc,
	&MC68000BASE::x5ce7_sge_b_pd7_071234fc,
	&MC68000BASE::x5cf8_sge_b_aw_071234fc,
	&MC68000BASE::x5cf9_sge_b_al_071234fc,
	&MC68000BASE::x5cfa_trapge_w_234fc,
	&MC68000BASE::x5cfb_trapge_l_234fc,
	&MC68000BASE::x5cfc_trapge_234fc,
	&MC68000BASE::x5ddf_slt_b_pi7_071234fc,
	&MC68000BASE::x5de7_slt_b_pd7_071234fc,
	&MC68000BASE::x5df8_slt_b_aw_071234fc,
	&MC68000BASE::x5df9_slt_b_al_071234fc,
	&MC68000BASE::x5dfa_traplt_w_234fc,
	&MC68000BASE::x5dfb_traplt_l_234fc,
	&MC68000BASE::x5dfc_traplt_234fc,
	&MC68000BASE::x5edf_sgt_b_pi7_071234fc,
	&MC68000BASE::x5ee7_sgt_b_pd7_071234fc,
	&MC68000BASE::x5ef8_sgt_b_aw_071234fc,
	&MC68000BASE::x5ef9_sgt_b_al_071234fc,
	&MC68000BASE::x5efa_trapgt_w_234fc,
	&MC68000BASE::x5efb_trapgt_l_234fc,
	&MC68000BASE::x5efc_trapgt_234fc,
	&MC68000BASE::x5fdf_sle_b_pi7_071234fc,
	&MC68000BASE::x5fe7_sle_b_pd7_071234fc,
	&MC68000BASE::x5ff8_sle_b_aw_071234fc,
	&MC68000BASE::x5ff9_sle_b_al_071234fc,
	&MC68000BASE::x5ffa_traple_w_234fc,
	&MC68000BASE::x5ffb_traple_l_234fc,
	&MC68000BASE::x5ffc_traple_234fc,
	&MC68000BASE::x6000_bra_w_071234fc,
	&MC68000BASE::x60ff_bra_l_234fc,
	&MC68000BASE::x6100_bsr_w_071234fc,
	&MC68000BASE::x61ff_bsr_l_234fc,
	&MC68000BASE::x6200_bhi_w_071234fc,
	&MC68000BASE::x62ff_bhi_l_071,
	&MC68000BASE::x62ff_bhi_l_234fc,
	&MC68000BASE::x6300_bls_w_071234fc,
	&MC68000BASE::x63ff_bls_l_071,
	&MC68000BASE::x63ff_bls_l_234fc,
	&MC68000BASE::x6400_bcc_w_071234fc,
	&MC68000BASE::x64ff_bcc_l_071,
	&MC68000BASE::x64ff_bcc_l_234fc,
	&MC68000BASE::x6500_bcs_w_071234fc,
	&MC68000BASE::x65ff_bcs_l_071,
	&MC68000BASE::x65ff_bcs_l_234fc,
	&MC68000BASE::x6600_bne_w_071234fc,
	&MC68000BASE::x66ff_bne_l_071,
	&MC68000BASE::x66ff_bne_l_234fc,
	&MC68000BASE::x6700_beq_w_071234fc,
	&MC68000BASE::x67ff_beq_l_071,
	&MC68000BASE::x67ff_beq_l_234fc,
	&MC68000BASE::x6800_bvc_w_071234fc,
	&MC68000BASE::x68ff_bvc_l_071,
	&MC68000BASE::x68ff_bvc_l_234fc,
	&MC68000BASE::x6900_bvs_w_071234fc,
	&MC68000BASE::x69ff_bvs_l_071,
	&MC68000BASE::x69ff_bvs_l_234fc,
	&MC68000BASE::x6a00_bpl_w_071234fc,
	&MC68000BASE::x6aff_bpl_l_071,
	&MC68000BASE::x6aff_bpl_l_234fc,
	&MC68000BASE::x6b00_bmi_w_071234fc,
	&MC68000BASE::x6bff_bmi_l_071,
	&MC68000BASE::x6bff_bmi_l_234fc,
	&MC68000BASE::x6c00_bge_w_071234fc,
	&MC68000BASE::x6cff_bge_l_071,
	&MC68000BASE::x6cff_bge_l_234fc,
	&MC68000BASE::x6d00_blt_w_071234fc,
	&MC68000BASE::x6dff_blt_l_071,
	&MC68000BASE::x6dff_blt_l_234fc,
	&MC68000BASE::x6e00_bgt_w_071234fc,
	&MC68000BASE::x6eff_bgt_l_071,
	&MC68000BASE::x6eff_bgt_l_234fc,
	&MC68000BASE::x6f00_ble_w_071234fc,
	&MC68000BASE::x6fff_ble_l_071,
	&MC68000BASE::x6fff_ble_l_234fc,
	&MC68000BASE::x8f0f_sbcd_b_071234fc,
	&MC68000BASE::x8f4f_pack_w_234fc,
	&MC68000BASE::x8f8f_unpk_w_234fc,
	&MC68000BASE::x9f0f_subx_b_071234fc,
	&MC68000BASE::xbf0f_cmpm_b_071234fc,
	&MC68000BASE::xcf0f_abcd_b_071234fc,
	&MC68000BASE::xdf0f_addx_b_071234fc,
	&MC68000BASE::xe0f8_asr_w_aw_071234fc,
	&MC68000BASE::xe0f9_asr_w_al_071234fc,
	&MC68000BASE::xe1f8_asl_w_aw_071234fc,
	&MC68000BASE::xe1f9_asl_w_al_071234fc,
	&MC68000BASE::xe2f8_lsr_w_aw_071234fc,
	&MC68000BASE::xe2f9_lsr_w_al_071234fc,
	&MC68000BASE::xe3f8_lsl_w_aw_071234fc,
	&MC68000BASE::xe3f9_lsl_w_al_071234fc,
	&MC68000BASE::xe4f8_roxr_w_aw_071234fc,
	&MC68000BASE::xe4f9_roxr_w_al_071234fc,
	&MC68000BASE::xe5f8_roxl_w_aw_071234fc,
	&MC68000BASE::xe5f9_roxl_w_al_071234fc,
	&MC68000BASE::xe6f8_ror_w_aw_071234fc,
	&MC68000BASE::xe6f9_ror_w_al_071234fc,
	&MC68000BASE::xe7f8_rol_w_aw_071234fc,
	&MC68000BASE::xe7f9_rol_w_al_071234fc,
	&MC68000BASE::xe8f8_bftst_l_aw_234fc,
	&MC68000BASE::xe8f9_bftst_l_al_234fc,
	&MC68000BASE::xe8fa_bftst_l_pcdi_234fc,
	&MC68000BASE::xe8fb_bftst_l_pcix_234fc,
	&MC68000BASE::xe9f8_bfextu_l_aw_234fc,
	&MC68000BASE::xe9f9_bfextu_l_al_234fc,
	&MC68000BASE::xe9fa_bfextu_l_pcdi_234fc,
	&MC68000BASE::xe9fb_bfextu_l_pcix_234fc,
	&MC68000BASE::xeaf8_bfchg_l_aw_234fc,
	&MC68000BASE::xeaf9_bfchg_l_al_234fc,
	&MC68000BASE::xebf8_bfexts_l_aw_234fc,
	&MC68000BASE::xebf9_bfexts_l_al_234fc,
	&MC68000BASE::xebfa_bfexts_l_pcdi_234fc,
	&MC68000BASE::xebfb_bfexts_l_pcix_234fc,
	&MC68000BASE::xecf8_bfclr_l_aw_234fc,
	&MC68000BASE::xecf9_bfclr_l_al_234fc,
	&MC68000BASE::xedf8_bfffo_l_aw_234fc,
	&MC68000BASE::xedf9_bfffo_l_al_234fc,
	&MC68000BASE::xedfa_bfffo_l_pcdi_234fc,
	&MC68000BASE::xedfb_bfffo_l_pcix_234fc,
	&MC68000BASE::xeef8_bfset_l_aw_234fc,
	&MC68000BASE::xeef9_bfset_l_al_234fc,
	&MC68000BASE::xeff8_bfins_l_aw_234fc,
	&MC68000BASE::xeff9_bfins_l_al_234fc,
};

const uint16_t MC68000BASE::m68k_state_illegal = 1797;

const MC68000BASE::opcode_handler_struct MC68000BASE::m68k_opcode_table[] =
{

	{ 0xa000, 0xf000, {{  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// 1010
	{ 0xf000, 0xf000, {{  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// 1111
	{ 0x7000, 0xf100, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// moveq_l
	{ 0xf080, 0xf180, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// cpbcc_l
	{ 0xf000, 0xf1c0, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// cpgen_l
	{ 0xf040, 0xf1c0, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// cpscc_l
	{ 0xf000, 0xfe00, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// pmmu_l
	{ 0x6000, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}}},	// bra_b
	{ 0x6100, 0xff00, {{ 18,  2}, { 17,  1}, { 18,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// bsr_b
	{ 0x6200, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bhi_b
	{ 0x6300, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bls_b
	{ 0x6400, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcc_b
	{ 0x6500, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcs_b
	{ 0x6600, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bne_b
	{ 0x6700, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// beq_b
	{ 0x6800, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvc_b
	{ 0x6900, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvs_b
	{ 0x6a00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bpl_b
	{ 0x6b00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bmi_b
	{ 0x6c00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bge_b
	{ 0x6d00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// blt_b
	{ 0x6e00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bgt_b
	{ 0x6f00, 0xff00, {{ 10,  2}, { 13,  5}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// ble_b
	{ 0xf200, 0xff00, {{255,255}, {255,255}, {255,255}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {255,255}}},	// 040fpu0_l
	{ 0xf300, 0xff00, {{255,255}, {255,255}, {255,255}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {255,255}}},	// 040fpu1_l
	{ 0xf400, 0xff20, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, { 16, 16}, {255,255}, {255,255}}},	// cinv_l
	{ 0xf420, 0xff20, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, { 16, 16}, {255,255}, {255,255}}},	// cpush_l
	{ 0x0100, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// btst_l
	{ 0x0108, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// movep_w
	{ 0x0110, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  4,  0}}},	// btst_b_ai
	{ 0x0118, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  4,  0}}},	// btst_b_pi
	{ 0x0120, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  9,  1}, {  9,  1}, {  9,  1}, {  9,  1}, {  4,  0}}},	// btst_b_pd
	{ 0x0128, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  9,  1}, {  9,  1}, {  9,  1}, {  9,  1}, {  4,  0}}},	// btst_b_di
	{ 0x0130, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, { 11,  3}, { 11,  3}, { 11,  3}, { 11,  3}, {  4,  0}}},	// btst_b_ix
	{ 0x0140, 0xf1f8, {{  8,  4}, { 10,  6}, {  8,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bchg_l
	{ 0x0148, 0xf1f8, {{ 24,  0}, { 36, 12}, { 24,  0}, { 18, 18}, { 18, 18}, { 18, 18}, { 18, 18}, { 18, 18}}},	// movep_l
	{ 0x0150, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_ai
	{ 0x0158, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_pi
	{ 0x0160, 0xf1f8, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_pd
	{ 0x0168, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_di
	{ 0x0170, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bchg_b_ix
	{ 0x0180, 0xf1f8, {{ 10,  6}, { 10,  6}, { 10,  6}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bclr_l
	{ 0x0188, 0xf1f8, {{ 16,  0}, { 25,  9}, { 16,  0}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}}},	// movep_w
	{ 0x0190, 0xf1f8, {{ 12,  0}, { 18,  6}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_ai
	{ 0x0198, 0xf1f8, {{ 12,  0}, { 18,  6}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_pi
	{ 0x01a0, 0xf1f8, {{ 14,  2}, { 20,  8}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_pd
	{ 0x01a8, 0xf1f8, {{ 16,  0}, { 22,  6}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_di
	{ 0x01b0, 0xf1f8, {{ 18,  2}, { 24,  8}, { 20,  4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bclr_b_ix
	{ 0x01c0, 0xf1f8, {{  8,  4}, { 10,  6}, {  8,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bset_l
	{ 0x01c8, 0xf1f8, {{ 24,  0}, { 39, 15}, { 24,  0}, { 17, 17}, { 17, 17}, { 17, 17}, { 17, 17}, { 17, 17}}},	// movep_l
	{ 0x01d0, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_ai
	{ 0x01d8, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_pi
	{ 0x01e0, 0xf1f8, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_pd
	{ 0x01e8, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_di
	{ 0x01f0, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bset_b_ix
	{ 0x1000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_b
	{ 0x1010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_b_ai
	{ 0x1018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_b_pi
	{ 0x1020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_b_pd
	{ 0x1028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_b_di
	{ 0x1030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_b_ix
	{ 0x1080, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_b
	{ 0x1090, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_ai
	{ 0x1098, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi
	{ 0x10a0, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd
	{ 0x10a8, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_di
	{ 0x10b0, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_ix
	{ 0x10c0, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_b
	{ 0x10d0, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_ai
	{ 0x10d8, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi
	{ 0x10e0, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd
	{ 0x10e8, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_di
	{ 0x10f0, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_ix
	{ 0x1100, 0xf1f8, {{  8,  0}, { 14,  6}, {  8,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_b
	{ 0x1110, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_ai
	{ 0x1118, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi
	{ 0x1120, 0xf1f8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd
	{ 0x1128, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_di
	{ 0x1130, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_ix
	{ 0x1140, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_b
	{ 0x1150, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_ai
	{ 0x1158, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi
	{ 0x1160, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd
	{ 0x1168, 0xf1f8, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_di
	{ 0x1170, 0xf1f8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_ix
	{ 0x1180, 0xf1f8, {{ 14,  2}, { 21,  9}, { 14,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// move_b
	{ 0x1190, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_b_ai
	{ 0x1198, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_b_pi
	{ 0x11a0, 0xf1f8, {{ 20,  4}, { 27, 11}, { 20,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_b_pd
	{ 0x11a8, 0xf1f8, {{ 22,  2}, { 29,  9}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_b_di
	{ 0x11b0, 0xf1f8, {{ 24,  4}, { 31, 11}, { 24,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_b_ix
	{ 0x2000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_l
	{ 0x2008, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_l
	{ 0x2010, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_l_ai
	{ 0x2018, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_l_pi
	{ 0x2020, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_l_pd
	{ 0x2028, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_l_di
	{ 0x2030, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_l_ix
	{ 0x2040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// movea_l
	{ 0x2048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// movea_l
	{ 0x2050, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_l_ai
	{ 0x2058, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_l_pi
	{ 0x2060, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_l_pd
	{ 0x2068, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_l_di
	{ 0x2070, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// movea_l_ix
	{ 0x2080, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x2088, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x2090, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_ai
	{ 0x2098, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_pi
	{ 0x20a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pd
	{ 0x20a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_di
	{ 0x20b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_ix
	{ 0x20c0, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x20c8, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x20d0, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_ai
	{ 0x20d8, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_pi
	{ 0x20e0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pd
	{ 0x20e8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_di
	{ 0x20f0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_ix
	{ 0x2100, 0xf1f8, {{ 12,  0}, { 18,  6}, { 14,  2}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_l
	{ 0x2108, 0xf1f8, {{ 12,  0}, { 18,  6}, { 14,  2}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_l
	{ 0x2110, 0xf1f8, {{ 20,  0}, { 26,  6}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_ai
	{ 0x2118, 0xf1f8, {{ 20,  0}, { 26,  6}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_pi
	{ 0x2120, 0xf1f8, {{ 22,  2}, { 28,  8}, { 24,  4}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_pd
	{ 0x2128, 0xf1f8, {{ 24,  0}, { 30,  6}, { 26,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_di
	{ 0x2130, 0xf1f8, {{ 26,  2}, { 32,  8}, { 28,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_l_ix
	{ 0x2140, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_l
	{ 0x2148, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_l
	{ 0x2150, 0xf1f8, {{ 24,  0}, { 30,  6}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_ai
	{ 0x2158, 0xf1f8, {{ 24,  0}, { 30,  6}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_pi
	{ 0x2160, 0xf1f8, {{ 26,  2}, { 32,  8}, { 26,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_pd
	{ 0x2168, 0xf1f8, {{ 28,  0}, { 34,  6}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_di
	{ 0x2170, 0xf1f8, {{ 30,  2}, { 36,  8}, { 30,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_l_ix
	{ 0x2180, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// move_l
	{ 0x2188, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// move_l
	{ 0x2190, 0xf1f8, {{ 26,  2}, { 33,  9}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_l_ai
	{ 0x2198, 0xf1f8, {{ 26,  2}, { 33,  9}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_l_pi
	{ 0x21a0, 0xf1f8, {{ 28,  4}, { 35, 11}, { 28,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_l_pd
	{ 0x21a8, 0xf1f8, {{ 30,  2}, { 37,  9}, { 30,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_l_di
	{ 0x21b0, 0xf1f8, {{ 32,  4}, { 39, 11}, { 32,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_l_ix
	{ 0x3000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_w
	{ 0x3008, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_w
	{ 0x3010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_w_ai
	{ 0x3018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_w_pi
	{ 0x3020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_w_pd
	{ 0x3028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_w_di
	{ 0x3030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_w_ix
	{ 0x3040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// movea_w
	{ 0x3048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// movea_w
	{ 0x3050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_w_ai
	{ 0x3058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_w_pi
	{ 0x3060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_w_pd
	{ 0x3068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_w_di
	{ 0x3070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// movea_w_ix
	{ 0x3080, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x3088, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x3090, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_ai
	{ 0x3098, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_pi
	{ 0x30a0, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pd
	{ 0x30a8, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_di
	{ 0x30b0, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_ix
	{ 0x30c0, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x30c8, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x30d0, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_ai
	{ 0x30d8, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_pi
	{ 0x30e0, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pd
	{ 0x30e8, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_di
	{ 0x30f0, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_ix
	{ 0x3100, 0xf1f8, {{  8,  0}, { 14,  6}, {  8,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_w
	{ 0x3108, 0xf1f8, {{  8,  0}, { 14,  6}, {  8,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_w
	{ 0x3110, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_ai
	{ 0x3118, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_pi
	{ 0x3120, 0xf1f8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_pd
	{ 0x3128, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_di
	{ 0x3130, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_w_ix
	{ 0x3140, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_w
	{ 0x3148, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_w
	{ 0x3150, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_ai
	{ 0x3158, 0xf1f8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_pi
	{ 0x3160, 0xf1f8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_pd
	{ 0x3168, 0xf1f8, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_di
	{ 0x3170, 0xf1f8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_w_ix
	{ 0x3180, 0xf1f8, {{ 14,  2}, { 21,  9}, { 14,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// move_w
	{ 0x3188, 0xf1f8, {{ 14,  2}, { 21,  9}, { 14,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// move_w
	{ 0x3190, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_w_ai
	{ 0x3198, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_w_pi
	{ 0x31a0, 0xf1f8, {{ 20,  4}, { 27, 11}, { 20,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_w_pd
	{ 0x31a8, 0xf1f8, {{ 22,  2}, { 29,  9}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_w_di
	{ 0x31b0, 0xf1f8, {{ 24,  4}, { 31, 11}, { 24,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_w_ix
	{ 0x4100, 0xf1f8, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// chk_l
	{ 0x4110, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_l_ai
	{ 0x4118, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_l_pi
	{ 0x4120, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_l_pd
	{ 0x4128, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_l_di
	{ 0x4130, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// chk_l_ix
	{ 0x4180, 0xf1f8, {{ 10,  6}, { 19, 15}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// chk_w
	{ 0x4190, 0xf1f8, {{ 14,  6}, { 23, 15}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_w_ai
	{ 0x4198, 0xf1f8, {{ 14,  6}, { 23, 15}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_w_pi
	{ 0x41a0, 0xf1f8, {{ 16,  8}, { 25, 17}, { 14, 10}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_w_pd
	{ 0x41a8, 0xf1f8, {{ 18,  6}, { 27, 15}, { 16,  8}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_w_di
	{ 0x41b0, 0xf1f8, {{ 20,  8}, { 29, 17}, { 18, 10}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// chk_w_ix
	{ 0x41d0, 0xf1f8, {{  4,  0}, { 11,  7}, {  4,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// lea_l_ai
	{ 0x41e8, 0xf1f8, {{  8,  0}, { 15,  7}, {  8,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// lea_l_di
	{ 0x41f0, 0xf1f8, {{ 12,  4}, { 19, 11}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// lea_l_ix
	{ 0x5000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addq_b
	{ 0x5010, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_b_ai
	{ 0x5018, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_b_pi
	{ 0x5020, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_b_pd
	{ 0x5028, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_b_di
	{ 0x5030, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addq_b_ix
	{ 0x5040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addq_w
	{ 0x5048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addq_w
	{ 0x5050, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_w_ai
	{ 0x5058, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_w_pi
	{ 0x5060, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_w_pd
	{ 0x5068, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_w_di
	{ 0x5070, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addq_w_ix
	{ 0x5080, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addq_l
	{ 0x5088, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addq_l
	{ 0x5090, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_l_ai
	{ 0x5098, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_l_pi
	{ 0x50a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_l_pd
	{ 0x50a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_l_di
	{ 0x50b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addq_l_ix
	{ 0x5100, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subq_b
	{ 0x5110, 0xf1f8, {{ 12,  4}, { 15,  7}, { 12,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_b_ai
	{ 0x5118, 0xf1f8, {{ 12,  4}, { 15,  7}, { 12,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_b_pi
	{ 0x5120, 0xf1f8, {{ 14,  6}, { 17,  9}, { 14,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_b_pd
	{ 0x5128, 0xf1f8, {{ 16,  4}, { 19,  7}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_b_di
	{ 0x5130, 0xf1f8, {{ 18,  6}, { 21,  9}, { 18,  6}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subq_b_ix
	{ 0x5140, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subq_w
	{ 0x5148, 0xf1f8, {{  8,  4}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subq_w
	{ 0x5150, 0xf1f8, {{ 12,  4}, { 15,  7}, { 12,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_w_ai
	{ 0x5158, 0xf1f8, {{ 12,  4}, { 15,  7}, { 12,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_w_pi
	{ 0x5160, 0xf1f8, {{ 14,  6}, { 17,  9}, { 14,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_w_pd
	{ 0x5168, 0xf1f8, {{ 16,  4}, { 19,  7}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_w_di
	{ 0x5170, 0xf1f8, {{ 18,  6}, { 21,  9}, { 18,  6}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subq_w_ix
	{ 0x5180, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subq_l
	{ 0x5188, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subq_l
	{ 0x5190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_l_ai
	{ 0x5198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_l_pi
	{ 0x51a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_l_pd
	{ 0x51a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_l_di
	{ 0x51b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subq_l_ix
	{ 0x8000, 0xf1f8, {{  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// or_b
	{ 0x8010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_b_ai
	{ 0x8018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_b_pi
	{ 0x8020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_b_pd
	{ 0x8028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_b_di
	{ 0x8030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_b_ix
	{ 0x8040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// or_w
	{ 0x8050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_w_ai
	{ 0x8058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_w_pi
	{ 0x8060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_w_pd
	{ 0x8068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_w_di
	{ 0x8070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_w_ix
	{ 0x8080, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// or_l
	{ 0x8090, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_l_ai
	{ 0x8098, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_l_pi
	{ 0x80a0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_l_pd
	{ 0x80a8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_l_di
	{ 0x80b0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_l_ix
	{ 0x80c0, 0xf1f8, {{140,136}, {130,126}, {108,104}, { 44, 44}, { 44, 44}, { 44, 44}, { 44, 44}, { 44, 44}}},	// divu_w
	{ 0x80d0, 0xf1f8, {{144,136}, {134,126}, {112,104}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 44, 44}}},	// divu_w_ai
	{ 0x80d8, 0xf1f8, {{144,136}, {134,126}, {112,104}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 44, 44}}},	// divu_w_pi
	{ 0x80e0, 0xf1f8, {{146,138}, {136,128}, {114,106}, { 49, 45}, { 49, 45}, { 49, 45}, { 49, 45}, { 44, 44}}},	// divu_w_pd
	{ 0x80e8, 0xf1f8, {{148,136}, {138,126}, {116,104}, { 49, 45}, { 49, 45}, { 49, 45}, { 49, 45}, { 44, 44}}},	// divu_w_di
	{ 0x80f0, 0xf1f8, {{150,138}, {140,128}, {118,106}, { 51, 47}, { 51, 47}, { 51, 47}, { 51, 47}, { 44, 44}}},	// divu_w_ix
	{ 0x8100, 0xf1f8, {{  6,  2}, { 10,  6}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// sbcd_b
	{ 0x8108, 0xf1f8, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// sbcd_b
	{ 0x8110, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_b_ai
	{ 0x8118, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_b_pi
	{ 0x8120, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_b_pd
	{ 0x8128, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_b_di
	{ 0x8130, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// or_b_ix
	{ 0x8140, 0xf1f8, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// pack_w
	{ 0x8148, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// pack_w
	{ 0x8150, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_w_ai
	{ 0x8158, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_w_pi
	{ 0x8160, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_w_pd
	{ 0x8168, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_w_di
	{ 0x8170, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// or_w_ix
	{ 0x8180, 0xf1f8, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// unpk_w
	{ 0x8188, 0xf1f8, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// unpk_w
	{ 0x8190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_l_ai
	{ 0x8198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_l_pi
	{ 0x81a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_l_pd
	{ 0x81a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_l_di
	{ 0x81b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// or_l_ix
	{ 0x81c0, 0xf1f8, {{158,154}, {169,165}, {122,118}, { 56, 56}, { 56, 56}, { 56, 56}, { 56, 56}, { 56, 56}}},	// divs_w
	{ 0x81d0, 0xf1f8, {{162,154}, {173,165}, {126,118}, { 60, 56}, { 60, 56}, { 60, 56}, { 60, 56}, { 56, 56}}},	// divs_w_ai
	{ 0x81d8, 0xf1f8, {{162,154}, {173,165}, {126,118}, { 60, 56}, { 60, 56}, { 60, 56}, { 60, 56}, { 56, 56}}},	// divs_w_pi
	{ 0x81e0, 0xf1f8, {{164,156}, {175,167}, {128,120}, { 61, 57}, { 61, 57}, { 61, 57}, { 61, 57}, { 56, 56}}},	// divs_w_pd
	{ 0x81e8, 0xf1f8, {{166,154}, {177,165}, {130,118}, { 61, 57}, { 61, 57}, { 61, 57}, { 61, 57}, { 56, 56}}},	// divs_w_di
	{ 0x81f0, 0xf1f8, {{168,156}, {179,167}, {132,120}, { 63, 59}, { 63, 59}, { 63, 59}, { 63, 59}, { 56, 56}}},	// divs_w_ix
	{ 0x9000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// sub_b
	{ 0x9010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_b_ai
	{ 0x9018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_b_pi
	{ 0x9020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_b_pd
	{ 0x9028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_b_di
	{ 0x9030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_b_ix
	{ 0x9040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// sub_w
	{ 0x9048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// sub_w
	{ 0x9050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_w_ai
	{ 0x9058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_w_pi
	{ 0x9060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_w_pd
	{ 0x9068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_w_di
	{ 0x9070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_w_ix
	{ 0x9080, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// sub_l
	{ 0x9088, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// sub_l
	{ 0x9090, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_l_ai
	{ 0x9098, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_l_pi
	{ 0x90a0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_l_pd
	{ 0x90a8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_l_di
	{ 0x90b0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_l_ix
	{ 0x90c0, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// suba_w
	{ 0x90c8, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// suba_w
	{ 0x90d0, 0xf1f8, {{ 12,  4}, { 11,  3}, { 12,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_w_ai
	{ 0x90d8, 0xf1f8, {{ 12,  4}, { 11,  3}, { 12,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_w_pi
	{ 0x90e0, 0xf1f8, {{ 14,  6}, { 13,  5}, { 14,  6}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_w_pd
	{ 0x90e8, 0xf1f8, {{ 16,  4}, { 15,  3}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_w_di
	{ 0x90f0, 0xf1f8, {{ 18,  6}, { 17,  5}, { 18,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// suba_w_ix
	{ 0x9100, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subx_b
	{ 0x9108, 0xf1f8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_b
	{ 0x9110, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_b_ai
	{ 0x9118, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_b_pi
	{ 0x9120, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_b_pd
	{ 0x9128, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_b_di
	{ 0x9130, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// sub_b_ix
	{ 0x9140, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subx_w
	{ 0x9148, 0xf1f8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_w
	{ 0x9150, 0xf1f8, {{ 12,  0}, { 19,  7}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_w_ai
	{ 0x9158, 0xf1f8, {{ 12,  0}, { 19,  7}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_w_pi
	{ 0x9160, 0xf1f8, {{ 14,  2}, { 21,  9}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_w_pd
	{ 0x9168, 0xf1f8, {{ 16,  0}, { 23,  7}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_w_di
	{ 0x9170, 0xf1f8, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// sub_w_ix
	{ 0x9180, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subx_l
	{ 0x9188, 0xf1f8, {{ 30,  2}, { 40, 12}, { 30,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_l
	{ 0x9190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_l_ai
	{ 0x9198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_l_pi
	{ 0x91a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_l_pd
	{ 0x91a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_l_di
	{ 0x91b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// sub_l_ix
	{ 0x91c0, 0xf1f8, {{  8,  4}, {  9,  5}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// suba_l
	{ 0x91c8, 0xf1f8, {{  8,  4}, {  9,  5}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// suba_l
	{ 0x91d0, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_l_ai
	{ 0x91d8, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_l_pi
	{ 0x91e0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_l_pd
	{ 0x91e8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_l_di
	{ 0x91f0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// suba_l_ix
	{ 0xb000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmp_b
	{ 0xb010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_b_ai
	{ 0xb018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_b_pi
	{ 0xb020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_b_pd
	{ 0xb028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_b_di
	{ 0xb030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_b_ix
	{ 0xb040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmp_w
	{ 0xb048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmp_w
	{ 0xb050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_w_ai
	{ 0xb058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_w_pi
	{ 0xb060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_w_pd
	{ 0xb068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_w_di
	{ 0xb070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_w_ix
	{ 0xb080, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmp_l
	{ 0xb088, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmp_l
	{ 0xb090, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_l_ai
	{ 0xb098, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_l_pi
	{ 0xb0a0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_l_pd
	{ 0xb0a8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_l_di
	{ 0xb0b0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_l_ix
	{ 0xb0c0, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// cmpa_w
	{ 0xb0c8, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// cmpa_w
	{ 0xb0d0, 0xf1f8, {{ 10,  2}, { 11,  3}, { 10,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_w_ai
	{ 0xb0d8, 0xf1f8, {{ 10,  2}, { 11,  3}, { 10,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_w_pi
	{ 0xb0e0, 0xf1f8, {{ 12,  4}, { 13,  5}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_w_pd
	{ 0xb0e8, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_w_di
	{ 0xb0f0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// cmpa_w_ix
	{ 0xb100, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eor_b
	{ 0xb108, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_b
	{ 0xb110, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_b_ai
	{ 0xb118, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_b_pi
	{ 0xb120, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_b_pd
	{ 0xb128, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_b_di
	{ 0xb130, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eor_b_ix
	{ 0xb140, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eor_w
	{ 0xb148, 0xf1f8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_w
	{ 0xb150, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_w_ai
	{ 0xb158, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_w_pi
	{ 0xb160, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_w_pd
	{ 0xb168, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_w_di
	{ 0xb170, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eor_w_ix
	{ 0xb180, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eor_l
	{ 0xb188, 0xf1f8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_l
	{ 0xb190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_l_ai
	{ 0xb198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_l_pi
	{ 0xb1a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_l_pd
	{ 0xb1a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_l_di
	{ 0xb1b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eor_l_ix
	{ 0xb1c0, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// cmpa_l
	{ 0xb1c8, 0xf1f8, {{  6,  2}, {  7,  3}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// cmpa_l
	{ 0xb1d0, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_l_ai
	{ 0xb1d8, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_l_pi
	{ 0xb1e0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_l_pd
	{ 0xb1e8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_l_di
	{ 0xb1f0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// cmpa_l_ix
	{ 0xc000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// and_b
	{ 0xc010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_b_ai
	{ 0xc018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_b_pi
	{ 0xc020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_b_pd
	{ 0xc028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_b_di
	{ 0xc030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_b_ix
	{ 0xc040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// and_w
	{ 0xc050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_w_ai
	{ 0xc058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_w_pi
	{ 0xc060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_w_pd
	{ 0xc068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_w_di
	{ 0xc070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_w_ix
	{ 0xc080, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// and_l
	{ 0xc090, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_l_ai
	{ 0xc098, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_l_pi
	{ 0xc0a0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_l_pd
	{ 0xc0a8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_l_di
	{ 0xc0b0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_l_ix
	{ 0xc0c0, 0xf1f8, {{ 54, 50}, { 76, 72}, { 30, 26}, { 27, 27}, { 27, 27}, { 27, 27}, { 27, 27}, { 27, 27}}},	// mulu_w
	{ 0xc0d0, 0xf1f8, {{ 58, 50}, { 80, 72}, { 34, 26}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// mulu_w_ai
	{ 0xc0d8, 0xf1f8, {{ 58, 50}, { 80, 72}, { 34, 26}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// mulu_w_pi
	{ 0xc0e0, 0xf1f8, {{ 60, 52}, { 82, 74}, { 36, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// mulu_w_pd
	{ 0xc0e8, 0xf1f8, {{ 62, 50}, { 84, 72}, { 38, 26}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// mulu_w_di
	{ 0xc0f0, 0xf1f8, {{ 64, 52}, { 86, 74}, { 40, 28}, { 34, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 27, 27}}},	// mulu_w_ix
	{ 0xc100, 0xf1f8, {{  6,  2}, { 10,  6}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// abcd_b
	{ 0xc108, 0xf1f8, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// abcd_b
	{ 0xc110, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_b_ai
	{ 0xc118, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_b_pi
	{ 0xc120, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_b_pd
	{ 0xc128, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_b_di
	{ 0xc130, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// and_b_ix
	{ 0xc140, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// exg_l
	{ 0xc148, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// exg_l
	{ 0xc150, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_w_ai
	{ 0xc158, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_w_pi
	{ 0xc160, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_w_pd
	{ 0xc168, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_w_di
	{ 0xc170, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// and_w_ix
	{ 0xc188, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// exg_l
	{ 0xc190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_l_ai
	{ 0xc198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_l_pi
	{ 0xc1a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_l_pd
	{ 0xc1a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_l_di
	{ 0xc1b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// and_l_ix
	{ 0xc1c0, 0xf1f8, {{ 54, 50}, { 76, 72}, { 32, 28}, { 27, 27}, { 27, 27}, { 27, 27}, { 27, 27}, { 27, 27}}},	// muls_w
	{ 0xc1d0, 0xf1f8, {{ 58, 50}, { 80, 72}, { 36, 28}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// muls_w_ai
	{ 0xc1d8, 0xf1f8, {{ 58, 50}, { 80, 72}, { 36, 28}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// muls_w_pi
	{ 0xc1e0, 0xf1f8, {{ 60, 52}, { 82, 74}, { 38, 30}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// muls_w_pd
	{ 0xc1e8, 0xf1f8, {{ 62, 50}, { 84, 72}, { 40, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// muls_w_di
	{ 0xc1f0, 0xf1f8, {{ 64, 52}, { 86, 74}, { 42, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 27, 27}}},	// muls_w_ix
	{ 0xd000, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// add_b
	{ 0xd010, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_b_ai
	{ 0xd018, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_b_pi
	{ 0xd020, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_b_pd
	{ 0xd028, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_b_di
	{ 0xd030, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_b_ix
	{ 0xd040, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// add_w
	{ 0xd048, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// add_w
	{ 0xd050, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_w_ai
	{ 0xd058, 0xf1f8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_w_pi
	{ 0xd060, 0xf1f8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_w_pd
	{ 0xd068, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_w_di
	{ 0xd070, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_w_ix
	{ 0xd080, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// add_l
	{ 0xd088, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// add_l
	{ 0xd090, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_l_ai
	{ 0xd098, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_l_pi
	{ 0xd0a0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_l_pd
	{ 0xd0a8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_l_di
	{ 0xd0b0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_l_ix
	{ 0xd0c0, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// adda_w
	{ 0xd0c8, 0xf1f8, {{  8,  4}, {  7,  3}, {  8,  4}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// adda_w
	{ 0xd0d0, 0xf1f8, {{ 12,  4}, { 11,  3}, { 12,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_w_ai
	{ 0xd0d8, 0xf1f8, {{ 12,  4}, { 11,  3}, { 12,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_w_pi
	{ 0xd0e0, 0xf1f8, {{ 14,  6}, { 13,  5}, { 14,  6}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_w_pd
	{ 0xd0e8, 0xf1f8, {{ 16,  4}, { 15,  3}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_w_di
	{ 0xd0f0, 0xf1f8, {{ 18,  6}, { 17,  5}, { 18,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// adda_w_ix
	{ 0xd100, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addx_b
	{ 0xd108, 0xf1f8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_b
	{ 0xd110, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_b_ai
	{ 0xd118, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_b_pi
	{ 0xd120, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_b_pd
	{ 0xd128, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_b_di
	{ 0xd130, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// add_b_ix
	{ 0xd140, 0xf1f8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addx_w
	{ 0xd148, 0xf1f8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_w
	{ 0xd150, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_w_ai
	{ 0xd158, 0xf1f8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_w_pi
	{ 0xd160, 0xf1f8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_w_pd
	{ 0xd168, 0xf1f8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_w_di
	{ 0xd170, 0xf1f8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// add_w_ix
	{ 0xd180, 0xf1f8, {{  8,  4}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addx_l
	{ 0xd188, 0xf1f8, {{ 30,  2}, { 40, 12}, { 30,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_l
	{ 0xd190, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_l_ai
	{ 0xd198, 0xf1f8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_l_pi
	{ 0xd1a0, 0xf1f8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_l_pd
	{ 0xd1a8, 0xf1f8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_l_di
	{ 0xd1b0, 0xf1f8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// add_l_ix
	{ 0xd1c0, 0xf1f8, {{  8,  4}, {  9,  5}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// adda_l
	{ 0xd1c8, 0xf1f8, {{  8,  4}, {  9,  5}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// adda_l
	{ 0xd1d0, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_l_ai
	{ 0xd1d8, 0xf1f8, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_l_pi
	{ 0xd1e0, 0xf1f8, {{ 16,  4}, { 17,  5}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_l_pd
	{ 0xd1e8, 0xf1f8, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_l_di
	{ 0xd1f0, 0xf1f8, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// adda_l_ix
	{ 0xe000, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// asr_b
	{ 0xe008, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsr_b
	{ 0xe010, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_b
	{ 0xe018, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// ror_b
	{ 0xe020, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// asr_b
	{ 0xe028, 0xf1f8, {{  6,  6}, { 13,  9}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// lsr_b
	{ 0xe030, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_b
	{ 0xe038, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// ror_b
	{ 0xe040, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// asr_w
	{ 0xe048, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsr_w
	{ 0xe050, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_w
	{ 0xe058, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// ror_w
	{ 0xe060, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// asr_w
	{ 0xe068, 0xf1f8, {{  6,  6}, { 13,  9}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// lsr_w
	{ 0xe070, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_w
	{ 0xe078, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// ror_w
	{ 0xe080, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// asr_l
	{ 0xe088, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsr_l
	{ 0xe090, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_l
	{ 0xe098, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// ror_l
	{ 0xe0a0, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// asr_l
	{ 0xe0a8, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// lsr_l
	{ 0xe0b0, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxr_l
	{ 0xe0b8, 0xf1f8, {{  8,  4}, { 13, 13}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// ror_l
	{ 0xe100, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// asl_b
	{ 0xe108, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsl_b
	{ 0xe110, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_b
	{ 0xe118, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// rol_b
	{ 0xe120, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// asl_b
	{ 0xe128, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// lsl_b
	{ 0xe130, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_b
	{ 0xe138, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// rol_b
	{ 0xe140, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// asl_w
	{ 0xe148, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsl_w
	{ 0xe150, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_w
	{ 0xe158, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// rol_w
	{ 0xe160, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// asl_w
	{ 0xe168, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// lsl_w
	{ 0xe170, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_w
	{ 0xe178, 0xf1f8, {{  6,  2}, { 13,  9}, {  6,  2}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// rol_w
	{ 0xe180, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// asl_l
	{ 0xe188, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// lsl_l
	{ 0xe190, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_l
	{ 0xe198, 0xf1f8, {{  8,  4}, { 13, 13}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// rol_l
	{ 0xe1a0, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// asl_l
	{ 0xe1a8, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// lsl_l
	{ 0xe1b0, 0xf1f8, {{  8,  4}, { 13,  9}, {  8,  4}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// roxl_l
	{ 0xe1b8, 0xf1f8, {{  8,  4}, { 13, 13}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}}},	// rol_l
	{ 0xf048, 0xf1f8, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// cpdbcc_l
	{ 0xf078, 0xf1f8, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// cptrapcc_l
	{ 0xf548, 0xffd8, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  8,  8}, {255,255}, {255,255}}},	// ptest_l
	{ 0x06c0, 0xfff0, {{255,255}, {255,255}, {255,255}, { 19, 19}, { 19, 19}, { 19, 19}, { 19, 19}, { 19, 19}}},	// rtm_l
	{ 0x4e40, 0xfff0, {{  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// trap
	{ 0x011f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  4,  0}}},	// btst_b_pi7
	{ 0x0127, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  9,  1}, {  9,  1}, {  9,  1}, {  9,  1}, {  4,  0}}},	// btst_b_pd7
	{ 0x0138, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  4,  0}}},	// btst_b_aw
	{ 0x0139, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  8,  0}, {  4,  0}}},	// btst_b_al
	{ 0x013a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  9,  1}, {  9,  1}, {  9,  1}, {  9,  1}, {  4,  0}}},	// btst_b_pcdi
	{ 0x013b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, { 11,  3}, { 11,  3}, { 11,  3}, { 11,  3}, {  4,  0}}},	// btst_b_pcix
	{ 0x013c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6, -2}, {  6, -2}, {  6, -2}, {  6, -2}, {  4,  0}}},	// btst_b_i
	{ 0x015f, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_pi7
	{ 0x0167, 0xf1ff, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_pd7
	{ 0x0178, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_aw
	{ 0x0179, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_al
	{ 0x019f, 0xf1ff, {{ 12,  0}, { 18,  6}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_pi7
	{ 0x01a7, 0xf1ff, {{ 14,  2}, { 20,  8}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_pd7
	{ 0x01b8, 0xf1ff, {{ 16,  0}, { 22,  6}, { 18,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_aw
	{ 0x01b9, 0xf1ff, {{ 20,  0}, { 26,  6}, { 22,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_al
	{ 0x01df, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_pi7
	{ 0x01e7, 0xf1ff, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_pd7
	{ 0x01f8, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_aw
	{ 0x01f9, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_al
	{ 0x101f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_b_pi7
	{ 0x1027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_b_pd7
	{ 0x1038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_b_aw
	{ 0x1039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_b_al
	{ 0x103a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_b_pcdi
	{ 0x103b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_b_pcix
	{ 0x103c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// move_b_i
	{ 0x109f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi7
	{ 0x10a7, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd7
	{ 0x10b8, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_aw
	{ 0x10b9, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_al
	{ 0x10ba, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pcdi
	{ 0x10bb, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_pcix
	{ 0x10bc, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_b_i
	{ 0x10df, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi7
	{ 0x10e7, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd7
	{ 0x10f8, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_aw
	{ 0x10f9, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_al
	{ 0x10fa, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pcdi
	{ 0x10fb, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_pcix
	{ 0x10fc, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_b_i
	{ 0x111f, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi7
	{ 0x1127, 0xf1ff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd7
	{ 0x1138, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_aw
	{ 0x1139, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_al
	{ 0x113a, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pcdi
	{ 0x113b, 0xf1ff, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_pcix
	{ 0x113c, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  5,  5}}},	// move_b_i
	{ 0x115f, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi7
	{ 0x1167, 0xf1ff, {{ 18,  2}, { 24,  8}, { 18,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd7
	{ 0x1178, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_aw
	{ 0x1179, 0xf1ff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_al
	{ 0x117a, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pcdi
	{ 0x117b, 0xf1ff, {{ 22,  2}, { 28,  8}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_pcix
	{ 0x117c, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  5,  5}}},	// move_b_i
	{ 0x119f, 0xf1ff, {{ 18,  2}, { 25,  9}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_b_pi7
	{ 0x11a7, 0xf1ff, {{ 20,  4}, { 27, 11}, { 20,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_b_pd7
	{ 0x11b8, 0xf1ff, {{ 22,  2}, { 29,  9}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_b_aw
	{ 0x11b9, 0xf1ff, {{ 26,  2}, { 33,  9}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_b_al
	{ 0x11ba, 0xf1ff, {{ 22,  2}, { 29,  9}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_b_pcdi
	{ 0x11bb, 0xf1ff, {{ 24,  4}, { 31, 11}, { 24,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_b_pcix
	{ 0x11bc, 0xf1ff, {{ 18,  2}, { 25,  9}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  7,  7}}},	// move_b_i
	{ 0x2038, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_l_aw
	{ 0x2039, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_l_al
	{ 0x203a, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_l_pcdi
	{ 0x203b, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_l_pcix
	{ 0x203c, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_l_i
	{ 0x2078, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_l_aw
	{ 0x2079, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_l_al
	{ 0x207a, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_l_pcdi
	{ 0x207b, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// movea_l_pcix
	{ 0x207c, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_l_i
	{ 0x20b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_aw
	{ 0x20b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_al
	{ 0x20ba, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pcdi
	{ 0x20bb, 0xf1ff, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_pcix
	{ 0x20bc, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_i
	{ 0x20f8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_aw
	{ 0x20f9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_al
	{ 0x20fa, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pcdi
	{ 0x20fb, 0xf1ff, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_pcix
	{ 0x20fc, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_i
	{ 0x2138, 0xf1ff, {{ 24,  0}, { 30,  6}, { 26,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_aw
	{ 0x2139, 0xf1ff, {{ 28,  0}, { 34,  6}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_al
	{ 0x213a, 0xf1ff, {{ 24,  0}, { 30,  6}, { 26,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_pcdi
	{ 0x213b, 0xf1ff, {{ 26,  2}, { 32,  8}, { 28,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_l_pcix
	{ 0x213c, 0xf1ff, {{ 20,  0}, { 26,  6}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_i
	{ 0x2178, 0xf1ff, {{ 28,  0}, { 34,  6}, { 28,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_aw
	{ 0x2179, 0xf1ff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_al
	{ 0x217a, 0xf1ff, {{ 28,  0}, { 34,  6}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_l_pcdi
	{ 0x217b, 0xf1ff, {{ 30,  2}, { 36,  8}, { 30,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_l_pcix
	{ 0x217c, 0xf1ff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_l_i
	{ 0x21b8, 0xf1ff, {{ 30,  2}, { 37,  9}, { 30,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_l_aw
	{ 0x21b9, 0xf1ff, {{ 34,  2}, { 41,  9}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_l_al
	{ 0x21ba, 0xf1ff, {{ 30,  2}, { 37,  9}, { 30,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_l_pcdi
	{ 0x21bb, 0xf1ff, {{ 32,  4}, { 39, 11}, { 32,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_l_pcix
	{ 0x21bc, 0xf1ff, {{ 26,  2}, { 33,  9}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_l_i
	{ 0x3038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_w_aw
	{ 0x3039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// move_w_al
	{ 0x303a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// move_w_pcdi
	{ 0x303b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// move_w_pcix
	{ 0x303c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// move_w_i
	{ 0x3078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_w_aw
	{ 0x3079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// movea_w_al
	{ 0x307a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// movea_w_pcdi
	{ 0x307b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// movea_w_pcix
	{ 0x307c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// movea_w_i
	{ 0x30b8, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_aw
	{ 0x30b9, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_al
	{ 0x30ba, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pcdi
	{ 0x30bb, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_pcix
	{ 0x30bc, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_w_i
	{ 0x30f8, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_aw
	{ 0x30f9, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_al
	{ 0x30fa, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pcdi
	{ 0x30fb, 0xf1ff, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_pcix
	{ 0x30fc, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_w_i
	{ 0x3138, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_aw
	{ 0x3139, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_al
	{ 0x313a, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_pcdi
	{ 0x313b, 0xf1ff, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_w_pcix
	{ 0x313c, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  5,  5}}},	// move_w_i
	{ 0x3178, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_aw
	{ 0x3179, 0xf1ff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_w_al
	{ 0x317a, 0xf1ff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_w_pcdi
	{ 0x317b, 0xf1ff, {{ 22,  2}, { 28,  8}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_w_pcix
	{ 0x317c, 0xf1ff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  5,  5}}},	// move_w_i
	{ 0x31b8, 0xf1ff, {{ 22,  2}, { 29,  9}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_w_aw
	{ 0x31b9, 0xf1ff, {{ 26,  2}, { 33,  9}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// move_w_al
	{ 0x31ba, 0xf1ff, {{ 22,  2}, { 29,  9}, { 22,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// move_w_pcdi
	{ 0x31bb, 0xf1ff, {{ 24,  4}, { 31, 11}, { 24,  4}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// move_w_pcix
	{ 0x31bc, 0xf1ff, {{ 18,  2}, { 25,  9}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  7,  7}}},	// move_w_i
	{ 0x4138, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_l_aw
	{ 0x4139, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_l_al
	{ 0x413a, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_l_pcdi
	{ 0x413b, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// chk_l_pcix
	{ 0x413c, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_l_i
	{ 0x41b8, 0xf1ff, {{ 18,  6}, { 27, 15}, { 16,  8}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_w_aw
	{ 0x41b9, 0xf1ff, {{ 22,  6}, { 31, 15}, { 20,  8}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// chk_w_al
	{ 0x41ba, 0xf1ff, {{ 18,  6}, { 27, 15}, { 16,  8}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// chk_w_pcdi
	{ 0x41bb, 0xf1ff, {{ 20,  8}, { 29, 17}, { 18, 10}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// chk_w_pcix
	{ 0x41bc, 0xf1ff, {{ 14,  6}, { 23, 15}, { 12,  8}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  8,  8}}},	// chk_w_i
	{ 0x41f8, 0xf1ff, {{  8,  0}, { 15,  7}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// lea_l_aw
	{ 0x41f9, 0xf1ff, {{ 12,  0}, { 19,  7}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// lea_l_al
	{ 0x41fa, 0xf1ff, {{  8,  0}, { 15,  7}, {  8,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// lea_l_pcdi
	{ 0x41fb, 0xf1ff, {{ 12,  4}, { 19, 11}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// lea_l_pcix
	{ 0x501f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_b_pi7
	{ 0x5027, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addq_b_pd7
	{ 0x5038, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_b_aw
	{ 0x5039, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_b_al
	{ 0x5078, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_w_aw
	{ 0x5079, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_w_al
	{ 0x50b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_l_aw
	{ 0x50b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addq_l_al
	{ 0x511f, 0xf1ff, {{ 12,  4}, { 15,  7}, { 12,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_b_pi7
	{ 0x5127, 0xf1ff, {{ 14,  6}, { 17,  9}, { 14,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subq_b_pd7
	{ 0x5138, 0xf1ff, {{ 16,  4}, { 19,  7}, { 16,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_b_aw
	{ 0x5139, 0xf1ff, {{ 20,  4}, { 23,  7}, { 20,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_b_al
	{ 0x5178, 0xf1ff, {{ 16,  4}, { 19,  7}, { 16,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_w_aw
	{ 0x5179, 0xf1ff, {{ 20,  4}, { 23,  7}, { 20,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_w_al
	{ 0x51b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_l_aw
	{ 0x51b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subq_l_al
	{ 0x801f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_b_pi7
	{ 0x8027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_b_pd7
	{ 0x8038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_b_aw
	{ 0x8039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_b_al
	{ 0x803a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_b_pcdi
	{ 0x803b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_b_pcix
	{ 0x803c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// or_b_i
	{ 0x8078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_w_aw
	{ 0x8079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_w_al
	{ 0x807a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_w_pcdi
	{ 0x807b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_w_pcix
	{ 0x807c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// or_w_i
	{ 0x80b8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_l_aw
	{ 0x80b9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_l_al
	{ 0x80ba, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// or_l_pcdi
	{ 0x80bb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// or_l_pcix
	{ 0x80bc, 0xf1ff, {{ 16,  4}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// or_l_i
	{ 0x80f8, 0xf1ff, {{148,136}, {138,126}, {116,104}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 44, 44}}},	// divu_w_aw
	{ 0x80f9, 0xf1ff, {{152,136}, {142,126}, {120,104}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 44, 44}}},	// divu_w_al
	{ 0x80fa, 0xf1ff, {{148,136}, {138,126}, {116,104}, { 49, 45}, { 49, 45}, { 49, 45}, { 49, 45}, { 44, 44}}},	// divu_w_pcdi
	{ 0x80fb, 0xf1ff, {{150,138}, {140,128}, {118,106}, { 51, 47}, { 51, 47}, { 51, 47}, { 51, 47}, { 44, 44}}},	// divu_w_pcix
	{ 0x80fc, 0xf1ff, {{144,136}, {134,126}, {112,104}, { 46, 42}, { 46, 42}, { 46, 42}, { 46, 42}, { 44, 44}}},	// divu_w_i
	{ 0x810f, 0xf1ff, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// sbcd_b
	{ 0x811f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_b_pi7
	{ 0x8127, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// or_b_pd7
	{ 0x8138, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_b_aw
	{ 0x8139, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_b_al
	{ 0x814f, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// pack_w
	{ 0x8178, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_w_aw
	{ 0x8179, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_w_al
	{ 0x818f, 0xf1ff, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// unpk_w
	{ 0x81b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_l_aw
	{ 0x81b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// or_l_al
	{ 0x81f8, 0xf1ff, {{166,154}, {177,165}, {130,118}, { 60, 56}, { 60, 56}, { 60, 56}, { 60, 56}, { 56, 56}}},	// divs_w_aw
	{ 0x81f9, 0xf1ff, {{170,154}, {181,165}, {134,118}, { 60, 56}, { 60, 56}, { 60, 56}, { 60, 56}, { 56, 56}}},	// divs_w_al
	{ 0x81fa, 0xf1ff, {{166,154}, {177,165}, {130,118}, { 61, 57}, { 61, 57}, { 61, 57}, { 61, 57}, { 56, 56}}},	// divs_w_pcdi
	{ 0x81fb, 0xf1ff, {{168,156}, {179,167}, {132,120}, { 63, 59}, { 63, 59}, { 63, 59}, { 63, 59}, { 56, 56}}},	// divs_w_pcix
	{ 0x81fc, 0xf1ff, {{162,154}, {173,165}, {126,118}, { 58, 54}, { 58, 54}, { 58, 54}, { 58, 54}, { 56, 56}}},	// divs_w_i
	{ 0x901f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_b_pi7
	{ 0x9027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_b_pd7
	{ 0x9038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_b_aw
	{ 0x9039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_b_al
	{ 0x903a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_b_pcdi
	{ 0x903b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_b_pcix
	{ 0x903c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// sub_b_i
	{ 0x9078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_w_aw
	{ 0x9079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_w_al
	{ 0x907a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_w_pcdi
	{ 0x907b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_w_pcix
	{ 0x907c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// sub_w_i
	{ 0x90b8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_l_aw
	{ 0x90b9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_l_al
	{ 0x90ba, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// sub_l_pcdi
	{ 0x90bb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// sub_l_pcix
	{ 0x90bc, 0xf1ff, {{ 16,  4}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// sub_l_i
	{ 0x90f8, 0xf1ff, {{ 16,  4}, { 15,  3}, { 16,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_w_aw
	{ 0x90f9, 0xf1ff, {{ 20,  4}, { 19,  3}, { 20,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_w_al
	{ 0x90fa, 0xf1ff, {{ 16,  4}, { 15,  3}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_w_pcdi
	{ 0x90fb, 0xf1ff, {{ 18,  6}, { 17,  5}, { 18,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// suba_w_pcix
	{ 0x90fc, 0xf1ff, {{ 12,  4}, { 11,  3}, { 12,  4}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// suba_w_i
	{ 0x910f, 0xf1ff, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_b
	{ 0x911f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_b_pi7
	{ 0x9127, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// sub_b_pd7
	{ 0x9138, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_b_aw
	{ 0x9139, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_b_al
	{ 0x9178, 0xf1ff, {{ 16,  0}, { 23,  7}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_w_aw
	{ 0x9179, 0xf1ff, {{ 20,  0}, { 27,  7}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_w_al
	{ 0x91b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_l_aw
	{ 0x91b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// sub_l_al
	{ 0x91f8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_l_aw
	{ 0x91f9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_l_al
	{ 0x91fa, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// suba_l_pcdi
	{ 0x91fb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// suba_l_pcix
	{ 0x91fc, 0xf1ff, {{ 16,  4}, { 17,  5}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// suba_l_i
	{ 0xb01f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_b_pi7
	{ 0xb027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_b_pd7
	{ 0xb038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_b_aw
	{ 0xb039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_b_al
	{ 0xb03a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_b_pcdi
	{ 0xb03b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_b_pcix
	{ 0xb03c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// cmp_b_i
	{ 0xb078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_w_aw
	{ 0xb079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_w_al
	{ 0xb07a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_w_pcdi
	{ 0xb07b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_w_pcix
	{ 0xb07c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// cmp_w_i
	{ 0xb0b8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_l_aw
	{ 0xb0b9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_l_al
	{ 0xb0ba, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmp_l_pcdi
	{ 0xb0bb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmp_l_pcix
	{ 0xb0bc, 0xf1ff, {{ 14,  2}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmp_l_i
	{ 0xb0f8, 0xf1ff, {{ 14,  2}, { 15,  3}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_w_aw
	{ 0xb0f9, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_w_al
	{ 0xb0fa, 0xf1ff, {{ 14,  2}, { 15,  3}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_w_pcdi
	{ 0xb0fb, 0xf1ff, {{ 16,  4}, { 17,  5}, { 16,  4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// cmpa_w_pcix
	{ 0xb0fc, 0xf1ff, {{ 10,  2}, { 11,  3}, { 10,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// cmpa_w_i
	{ 0xb10f, 0xf1ff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_b
	{ 0xb11f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_b_pi7
	{ 0xb127, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eor_b_pd7
	{ 0xb138, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_b_aw
	{ 0xb139, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_b_al
	{ 0xb178, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_w_aw
	{ 0xb179, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_w_al
	{ 0xb1b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_l_aw
	{ 0xb1b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eor_l_al
	{ 0xb1f8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_l_aw
	{ 0xb1f9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_l_al
	{ 0xb1fa, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// cmpa_l_pcdi
	{ 0xb1fb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// cmpa_l_pcix
	{ 0xb1fc, 0xf1ff, {{ 14,  2}, { 15,  3}, { 14,  2}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// cmpa_l_i
	{ 0xc01f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_b_pi7
	{ 0xc027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_b_pd7
	{ 0xc038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_b_aw
	{ 0xc039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_b_al
	{ 0xc03a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_b_pcdi
	{ 0xc03b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_b_pcix
	{ 0xc03c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// and_b_i
	{ 0xc078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_w_aw
	{ 0xc079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_w_al
	{ 0xc07a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_w_pcdi
	{ 0xc07b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_w_pcix
	{ 0xc07c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// and_w_i
	{ 0xc0b8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_l_aw
	{ 0xc0b9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_l_al
	{ 0xc0ba, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// and_l_pcdi
	{ 0xc0bb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// and_l_pcix
	{ 0xc0bc, 0xf1ff, {{ 16,  4}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// and_l_i
	{ 0xc0f8, 0xf1ff, {{ 62, 50}, { 84, 72}, { 38, 26}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// mulu_w_aw
	{ 0xc0f9, 0xf1ff, {{ 66, 50}, { 88, 72}, { 42, 26}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// mulu_w_al
	{ 0xc0fa, 0xf1ff, {{ 62, 50}, { 84, 72}, { 38, 26}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// mulu_w_pcdi
	{ 0xc0fb, 0xf1ff, {{ 64, 52}, { 86, 74}, { 40, 28}, { 34, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 27, 27}}},	// mulu_w_pcix
	{ 0xc0fc, 0xf1ff, {{ 58, 50}, { 80, 72}, { 34, 26}, { 29, 25}, { 29, 25}, { 29, 25}, { 29, 25}, { 27, 27}}},	// mulu_w_i
	{ 0xc10f, 0xf1ff, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// abcd_b
	{ 0xc11f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_b_pi7
	{ 0xc127, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// and_b_pd7
	{ 0xc138, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_b_aw
	{ 0xc139, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_b_al
	{ 0xc178, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_w_aw
	{ 0xc179, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_w_al
	{ 0xc1b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_l_aw
	{ 0xc1b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// and_l_al
	{ 0xc1f8, 0xf1ff, {{ 62, 50}, { 84, 72}, { 40, 28}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// muls_w_aw
	{ 0xc1f9, 0xf1ff, {{ 66, 50}, { 88, 72}, { 44, 28}, { 31, 27}, { 31, 27}, { 31, 27}, { 31, 27}, { 27, 27}}},	// muls_w_al
	{ 0xc1fa, 0xf1ff, {{ 62, 50}, { 84, 72}, { 40, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 27, 27}}},	// muls_w_pcdi
	{ 0xc1fb, 0xf1ff, {{ 64, 52}, { 86, 74}, { 42, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 34, 30}, { 27, 27}}},	// muls_w_pcix
	{ 0xc1fc, 0xf1ff, {{ 58, 50}, { 80, 72}, { 36, 28}, { 29, 25}, { 29, 25}, { 29, 25}, { 29, 25}, { 27, 27}}},	// muls_w_i
	{ 0xd01f, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_b_pi7
	{ 0xd027, 0xf1ff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_b_pd7
	{ 0xd038, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_b_aw
	{ 0xd039, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_b_al
	{ 0xd03a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_b_pcdi
	{ 0xd03b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_b_pcix
	{ 0xd03c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// add_b_i
	{ 0xd078, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_w_aw
	{ 0xd079, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_w_al
	{ 0xd07a, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_w_pcdi
	{ 0xd07b, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_w_pcix
	{ 0xd07c, 0xf1ff, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// add_w_i
	{ 0xd0b8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_l_aw
	{ 0xd0b9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_l_al
	{ 0xd0ba, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// add_l_pcdi
	{ 0xd0bb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// add_l_pcix
	{ 0xd0bc, 0xf1ff, {{ 16,  4}, { 15,  3}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// add_l_i
	{ 0xd0f8, 0xf1ff, {{ 16,  4}, { 15,  3}, { 16,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_w_aw
	{ 0xd0f9, 0xf1ff, {{ 20,  4}, { 19,  3}, { 20,  4}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_w_al
	{ 0xd0fa, 0xf1ff, {{ 16,  4}, { 15,  3}, { 16,  4}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_w_pcdi
	{ 0xd0fb, 0xf1ff, {{ 18,  6}, { 17,  5}, { 18,  6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// adda_w_pcix
	{ 0xd0fc, 0xf1ff, {{ 12,  4}, { 11,  3}, { 12,  4}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  2,  2}}},	// adda_w_i
	{ 0xd10f, 0xf1ff, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_b
	{ 0xd11f, 0xf1ff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_b_pi7
	{ 0xd127, 0xf1ff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// add_b_pd7
	{ 0xd138, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_b_aw
	{ 0xd139, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_b_al
	{ 0xd178, 0xf1ff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_w_aw
	{ 0xd179, 0xf1ff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_w_al
	{ 0xd1b8, 0xf1ff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_l_aw
	{ 0xd1b9, 0xf1ff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// add_l_al
	{ 0xd1f8, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_l_aw
	{ 0xd1f9, 0xf1ff, {{ 22,  2}, { 23,  3}, { 22,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_l_al
	{ 0xd1fa, 0xf1ff, {{ 18,  2}, { 19,  3}, { 18,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// adda_l_pcdi
	{ 0xd1fb, 0xf1ff, {{ 20,  4}, { 21,  5}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// adda_l_pcix
	{ 0xd1fc, 0xf1ff, {{ 16,  4}, { 17,  5}, { 14,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// adda_l_i
	{ 0x0000, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// ori_b
	{ 0x0010, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_b_ai
	{ 0x0018, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_b_pi
	{ 0x0020, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_b_pd
	{ 0x0028, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_b_di
	{ 0x0030, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// ori_b_ix
	{ 0x0040, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// ori_w
	{ 0x0050, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_w_ai
	{ 0x0058, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_w_pi
	{ 0x0060, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_w_pd
	{ 0x0068, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_w_di
	{ 0x0070, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// ori_w_ix
	{ 0x0080, 0xfff8, {{ 16,  4}, { 18,  6}, { 14,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// ori_l
	{ 0x0090, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_l_ai
	{ 0x0098, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_l_pi
	{ 0x00a0, 0xfff8, {{ 30,  2}, { 36,  8}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_l_pd
	{ 0x00a8, 0xfff8, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_l_di
	{ 0x00b0, 0xfff8, {{ 34,  2}, { 40,  8}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// ori_l_ix
	{ 0x00d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_b_ai
	{ 0x00e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 23, 19}, { 23, 19}, { 23, 19}, { 23, 19}, { 18, 18}}},	// chk2cmp2_b_di
	{ 0x00f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 18, 18}}},	// chk2cmp2_b_ix
	{ 0x0200, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// andi_b
	{ 0x0210, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_b_ai
	{ 0x0218, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_b_pi
	{ 0x0220, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_b_pd
	{ 0x0228, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_b_di
	{ 0x0230, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// andi_b_ix
	{ 0x0240, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// andi_w
	{ 0x0250, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_w_ai
	{ 0x0258, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_w_pi
	{ 0x0260, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_w_pd
	{ 0x0268, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_w_di
	{ 0x0270, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// andi_w_ix
	{ 0x0280, 0xfff8, {{ 14,  2}, { 18,  6}, { 14,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// andi_l
	{ 0x0290, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_l_ai
	{ 0x0298, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_l_pi
	{ 0x02a0, 0xfff8, {{ 30,  2}, { 36,  8}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_l_pd
	{ 0x02a8, 0xfff8, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_l_di
	{ 0x02b0, 0xfff8, {{ 34,  2}, { 40,  8}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// andi_l_ix
	{ 0x02d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_w_ai
	{ 0x02e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 23, 19}, { 23, 19}, { 23, 19}, { 23, 19}, { 18, 18}}},	// chk2cmp2_w_di
	{ 0x02f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 18, 18}}},	// chk2cmp2_w_ix
	{ 0x0400, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subi_b
	{ 0x0410, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_b_ai
	{ 0x0418, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_b_pi
	{ 0x0420, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_b_pd
	{ 0x0428, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_b_di
	{ 0x0430, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subi_b_ix
	{ 0x0440, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subi_w
	{ 0x0450, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_w_ai
	{ 0x0458, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_w_pi
	{ 0x0460, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_w_pd
	{ 0x0468, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_w_di
	{ 0x0470, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subi_w_ix
	{ 0x0480, 0xfff8, {{ 16,  4}, { 18,  6}, { 14,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// subi_l
	{ 0x0490, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_l_ai
	{ 0x0498, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_l_pi
	{ 0x04a0, 0xfff8, {{ 30,  2}, { 36,  8}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_l_pd
	{ 0x04a8, 0xfff8, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_l_di
	{ 0x04b0, 0xfff8, {{ 34,  2}, { 40,  8}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// subi_l_ix
	{ 0x04d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_l_ai
	{ 0x04e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 23, 19}, { 23, 19}, { 23, 19}, { 23, 19}, { 18, 18}}},	// chk2cmp2_l_di
	{ 0x04f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 18, 18}}},	// chk2cmp2_l_ix
	{ 0x0600, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addi_b
	{ 0x0610, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_b_ai
	{ 0x0618, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_b_pi
	{ 0x0620, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_b_pd
	{ 0x0628, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_b_di
	{ 0x0630, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addi_b_ix
	{ 0x0640, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addi_w
	{ 0x0650, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_w_ai
	{ 0x0658, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_w_pi
	{ 0x0660, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_w_pd
	{ 0x0668, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_w_di
	{ 0x0670, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addi_w_ix
	{ 0x0680, 0xfff8, {{ 16,  4}, { 18,  6}, { 14,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// addi_l
	{ 0x0690, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_l_ai
	{ 0x0698, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_l_pi
	{ 0x06a0, 0xfff8, {{ 30,  2}, { 36,  8}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_l_pd
	{ 0x06a8, 0xfff8, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_l_di
	{ 0x06b0, 0xfff8, {{ 34,  2}, { 40,  8}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// addi_l_ix
	{ 0x06d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 64, 60}, {255,255}, {255,255}, { 64, 60}, {255,255}}},	// callm_l_ai
	{ 0x06e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 65, 61}, {255,255}, {255,255}, { 65, 61}, {255,255}}},	// callm_l_di
	{ 0x06f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 67, 63}, {255,255}, {255,255}, { 67, 63}, {255,255}}},	// callm_l_ix
	{ 0x0800, 0xfff8, {{ 10,  2}, { 14,  6}, { 10,  2}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// btst_l
	{ 0x0810, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// btst_b_ai
	{ 0x0818, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// btst_b_pi
	{ 0x0820, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// btst_b_pd
	{ 0x0828, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// btst_b_di
	{ 0x0830, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// btst_b_ix
	{ 0x0840, 0xfff8, {{ 12,  4}, { 17,  9}, { 12,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bchg_l
	{ 0x0850, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_ai
	{ 0x0858, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_pi
	{ 0x0860, 0xfff8, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_pd
	{ 0x0868, 0xfff8, {{ 20,  0}, { 29,  9}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_di
	{ 0x0870, 0xfff8, {{ 22,  2}, { 31, 11}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bchg_b_ix
	{ 0x0880, 0xfff8, {{ 14,  6}, { 17,  9}, { 14,  6}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bclr_l
	{ 0x0890, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_ai
	{ 0x0898, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_pi
	{ 0x08a0, 0xfff8, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_pd
	{ 0x08a8, 0xfff8, {{ 20,  0}, { 29,  9}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_di
	{ 0x08b0, 0xfff8, {{ 22,  2}, { 31, 11}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bclr_b_ix
	{ 0x08c0, 0xfff8, {{ 12,  4}, { 17,  9}, { 12,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// bset_l
	{ 0x08d0, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_ai
	{ 0x08d8, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_pi
	{ 0x08e0, 0xfff8, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_pd
	{ 0x08e8, 0xfff8, {{ 20,  0}, { 29,  9}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_di
	{ 0x08f0, 0xfff8, {{ 22,  2}, { 31, 11}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// bset_b_ix
	{ 0x0a00, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eori_b
	{ 0x0a10, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_b_ai
	{ 0x0a18, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_b_pi
	{ 0x0a20, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_b_pd
	{ 0x0a28, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_b_di
	{ 0x0a30, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eori_b_ix
	{ 0x0a40, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eori_w
	{ 0x0a50, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_w_ai
	{ 0x0a58, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_w_pi
	{ 0x0a60, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_w_pd
	{ 0x0a68, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_w_di
	{ 0x0a70, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eori_w_ix
	{ 0x0a80, 0xfff8, {{ 16,  4}, { 18,  6}, { 14,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// eori_l
	{ 0x0a90, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_l_ai
	{ 0x0a98, 0xfff8, {{ 28,  0}, { 34,  6}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_l_pi
	{ 0x0aa0, 0xfff8, {{ 30,  2}, { 36,  8}, { 30,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_l_pd
	{ 0x0aa8, 0xfff8, {{ 32,  0}, { 38,  6}, { 32,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_l_di
	{ 0x0ab0, 0xfff8, {{ 34,  2}, { 40,  8}, { 34,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// eori_l_ix
	{ 0x0ad0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_b_ai
	{ 0x0ad8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_b_pi
	{ 0x0ae0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_b_pd
	{ 0x0ae8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_b_di
	{ 0x0af0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 12, 12}}},	// cas_b_ix
	{ 0x0c00, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmpi_b
	{ 0x0c10, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_b_ai
	{ 0x0c18, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_b_pi
	{ 0x0c20, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_b_pd
	{ 0x0c28, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_b_di
	{ 0x0c30, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmpi_b_ix
	{ 0x0c40, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmpi_w
	{ 0x0c50, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_w_ai
	{ 0x0c58, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_w_pi
	{ 0x0c60, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_w_pd
	{ 0x0c68, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_w_di
	{ 0x0c70, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmpi_w_ix
	{ 0x0c80, 0xfff8, {{ 14,  2}, { 18,  6}, { 12, 12}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// cmpi_l
	{ 0x0c90, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_l_ai
	{ 0x0c98, 0xfff8, {{ 20,  0}, { 26,  6}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_l_pi
	{ 0x0ca0, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_l_pd
	{ 0x0ca8, 0xfff8, {{ 24,  0}, { 30,  6}, { 24,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_l_di
	{ 0x0cb0, 0xfff8, {{ 26,  2}, { 32,  8}, { 26,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// cmpi_l_ix
	{ 0x0cd0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_w_ai
	{ 0x0cd8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_w_pi
	{ 0x0ce0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_w_pd
	{ 0x0ce8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_w_di
	{ 0x0cf0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 12, 12}}},	// cas_w_ix
	{ 0x0e10, 0xfff8, {{255,255}, {255,255}, { 18,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_b_ai
	{ 0x0e10, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_ai
	{ 0x0e18, 0xfff8, {{255,255}, {255,255}, { 18,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_b_pi
	{ 0x0e18, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_pi
	{ 0x0e20, 0xfff8, {{255,255}, {255,255}, { 20,  4}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_b_pd
	{ 0x0e20, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_pd
	{ 0x0e28, 0xfff8, {{255,255}, {255,255}, { 26, 10}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_b_di
	{ 0x0e28, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_di
	{ 0x0e30, 0xfff8, {{255,255}, {255,255}, { 30, 14}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// moves_b_ix
	{ 0x0e30, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12,  8}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_ix
	{ 0x0e50, 0xfff8, {{255,255}, {255,255}, { 18,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_w_ai
	{ 0x0e50, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_ai
	{ 0x0e58, 0xfff8, {{255,255}, {255,255}, { 18,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_w_pi
	{ 0x0e58, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_pi
	{ 0x0e60, 0xfff8, {{255,255}, {255,255}, { 20,  4}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_w_pd
	{ 0x0e60, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_pd
	{ 0x0e68, 0xfff8, {{255,255}, {255,255}, { 26, 10}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_w_di
	{ 0x0e68, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_di
	{ 0x0e70, 0xfff8, {{255,255}, {255,255}, { 30, 14}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// moves_w_ix
	{ 0x0e70, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12,  8}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_ix
	{ 0x0e90, 0xfff8, {{255,255}, {255,255}, { 22,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_l_ai
	{ 0x0e90, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_ai
	{ 0x0e98, 0xfff8, {{255,255}, {255,255}, { 22,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_l_pi
	{ 0x0e98, 0xfff8, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_pi
	{ 0x0ea0, 0xfff8, {{255,255}, {255,255}, { 28,  8}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_l_pd
	{ 0x0ea0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_pd
	{ 0x0ea8, 0xfff8, {{255,255}, {255,255}, { 32, 12}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_l_di
	{ 0x0ea8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_di
	{ 0x0eb0, 0xfff8, {{255,255}, {255,255}, { 36, 16}, {255,255}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// moves_l_ix
	{ 0x0eb0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12,  8}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_ix
	{ 0x0ed0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_l_ai
	{ 0x0ed8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_l_pi
	{ 0x0ee0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_l_pd
	{ 0x0ee8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_l_di
	{ 0x0ef0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 12, 12}}},	// cas_l_ix
	{ 0x11c0, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_b
	{ 0x11d0, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_ai
	{ 0x11d8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi
	{ 0x11e0, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd
	{ 0x11e8, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_di
	{ 0x11f0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_ix
	{ 0x13c0, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// move_b
	{ 0x13d0, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_b_ai
	{ 0x13d8, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_b_pi
	{ 0x13e0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_b_pd
	{ 0x13e8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_b_di
	{ 0x13f0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_b_ix
	{ 0x1ec0, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_b
	{ 0x1ed0, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_ai
	{ 0x1ed8, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi
	{ 0x1ee0, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd
	{ 0x1ee8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_di
	{ 0x1ef0, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_ix
	{ 0x1f00, 0xfff8, {{  8,  0}, { 14,  6}, {  8,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// move_b
	{ 0x1f10, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_ai
	{ 0x1f18, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi
	{ 0x1f20, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd
	{ 0x1f28, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_di
	{ 0x1f30, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_ix
	{ 0x21c0, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x21c8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_l
	{ 0x21d0, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_ai
	{ 0x21d8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_pi
	{ 0x21e0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pd
	{ 0x21e8, 0xfff8, {{ 28,  0}, { 31,  3}, { 28,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_di
	{ 0x21f0, 0xfff8, {{ 30,  2}, { 33,  5}, { 30,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_ix
	{ 0x23c0, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// move_l
	{ 0x23c8, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// move_l
	{ 0x23d0, 0xfff8, {{ 28,  0}, { 31,  3}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_l_ai
	{ 0x23d8, 0xfff8, {{ 28,  0}, { 31,  3}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_l_pi
	{ 0x23e0, 0xfff8, {{ 30,  2}, { 33,  5}, { 30,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_l_pd
	{ 0x23e8, 0xfff8, {{ 32,  0}, { 35,  3}, { 32,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_l_di
	{ 0x23f0, 0xfff8, {{ 34,  2}, { 37,  5}, { 34,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_l_ix
	{ 0x31c0, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x31c8, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x31d0, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_ai
	{ 0x31d8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_pi
	{ 0x31e0, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pd
	{ 0x31e8, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_di
	{ 0x31f0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_ix
	{ 0x33c0, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// move_w
	{ 0x33c8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// move_w
	{ 0x33d0, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_w_ai
	{ 0x33d8, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_w_pi
	{ 0x33e0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_w_pd
	{ 0x33e8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_w_di
	{ 0x33f0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_w_ix
	{ 0x4000, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// negx_b
	{ 0x4010, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_b_ai
	{ 0x4018, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_b_pi
	{ 0x4020, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_b_pd
	{ 0x4028, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_b_di
	{ 0x4030, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// negx_b_ix
	{ 0x4040, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// negx_w
	{ 0x4050, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_w_ai
	{ 0x4058, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_w_pi
	{ 0x4060, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_w_pd
	{ 0x4068, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_w_di
	{ 0x4070, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// negx_w_ix
	{ 0x4080, 0xfff8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// negx_l
	{ 0x4090, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_l_ai
	{ 0x4098, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_l_pi
	{ 0x40a0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_l_pd
	{ 0x40a8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_l_di
	{ 0x40b0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// negx_l_ix
	{ 0x40c0, 0xfff8, {{  6,  2}, {  7,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w
	{ 0x40c0, 0xfff8, {{255,255}, {255,255}, {  4,  0}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// move_w
	{ 0x40d0, 0xfff8, {{ 12,  0}, { 15,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_ai
	{ 0x40d0, 0xfff8, {{255,255}, {255,255}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, {  8,  0}}},	// move_w_ai
	{ 0x40d8, 0xfff8, {{ 12,  0}, { 15,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_pi
	{ 0x40d8, 0xfff8, {{255,255}, {255,255}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, {  8,  0}}},	// move_w_pi
	{ 0x40e0, 0xfff8, {{ 14,  2}, { 17,  5}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_pd
	{ 0x40e0, 0xfff8, {{255,255}, {255,255}, { 14,  2}, { 13,  1}, { 13,  1}, { 13,  1}, { 13,  1}, {  8,  0}}},	// move_w_pd
	{ 0x40e8, 0xfff8, {{ 16,  0}, { 19,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_di
	{ 0x40e8, 0xfff8, {{255,255}, {255,255}, { 16,  0}, { 13,  1}, { 13,  1}, { 13,  1}, { 13,  1}, {  8,  0}}},	// move_w_di
	{ 0x40f0, 0xfff8, {{ 18,  2}, { 21,  5}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_ix
	{ 0x40f0, 0xfff8, {{255,255}, {255,255}, { 18,  2}, { 15,  3}, { 15,  3}, { 15,  3}, { 15,  3}, {  8,  0}}},	// move_w_ix
	{ 0x4200, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// clr_b
	{ 0x4210, 0xfff8, {{ 12,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_ai
	{ 0x4210, 0xfff8, {{255,255}, { 15,  3}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_b_ai
	{ 0x4218, 0xfff8, {{ 12,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_pi
	{ 0x4218, 0xfff8, {{255,255}, { 15,  3}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_b_pi
	{ 0x4220, 0xfff8, {{ 14,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_pd
	{ 0x4220, 0xfff8, {{255,255}, { 17,  5}, { 10, -2}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// clr_b_pd
	{ 0x4228, 0xfff8, {{ 16,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_di
	{ 0x4228, 0xfff8, {{255,255}, { 19,  3}, { 12, -4}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// clr_b_di
	{ 0x4230, 0xfff8, {{ 18,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_ix
	{ 0x4230, 0xfff8, {{255,255}, { 21,  5}, { 14, -2}, { 11, -1}, { 11, -1}, { 11, -1}, { 11, -1}, {  4, -4}}},	// clr_b_ix
	{ 0x4240, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// clr_w
	{ 0x4250, 0xfff8, {{ 12,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_ai
	{ 0x4250, 0xfff8, {{255,255}, { 15,  3}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_w_ai
	{ 0x4258, 0xfff8, {{ 12,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_pi
	{ 0x4258, 0xfff8, {{255,255}, { 15,  3}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_w_pi
	{ 0x4260, 0xfff8, {{ 14,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_pd
	{ 0x4260, 0xfff8, {{255,255}, { 17,  5}, { 10, -2}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// clr_w_pd
	{ 0x4268, 0xfff8, {{ 16,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_di
	{ 0x4268, 0xfff8, {{255,255}, { 19,  3}, { 12, -4}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// clr_w_di
	{ 0x4270, 0xfff8, {{ 18,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_ix
	{ 0x4270, 0xfff8, {{255,255}, { 21,  5}, { 14, -2}, { 11, -1}, { 11, -1}, { 11, -1}, { 11, -1}, {  4, -4}}},	// clr_w_ix
	{ 0x4280, 0xfff8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// clr_l
	{ 0x4290, 0xfff8, {{ 20,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_ai
	{ 0x4290, 0xfff8, {{255,255}, { 23,  3}, { 12, -8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// clr_l_ai
	{ 0x4298, 0xfff8, {{ 20,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_pi
	{ 0x4298, 0xfff8, {{255,255}, { 23,  3}, { 12, -8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// clr_l_pi
	{ 0x42a0, 0xfff8, {{ 22,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_pd
	{ 0x42a0, 0xfff8, {{255,255}, { 25,  5}, { 14, -6}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// clr_l_pd
	{ 0x42a8, 0xfff8, {{ 24,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_di
	{ 0x42a8, 0xfff8, {{255,255}, { 27,  3}, { 16, -8}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// clr_l_di
	{ 0x42b0, 0xfff8, {{ 26,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_ix
	{ 0x42b0, 0xfff8, {{255,255}, { 29,  5}, { 20, -4}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// clr_l_ix
	{ 0x42c0, 0xfff8, {{255,255}, {255,255}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// move_w
	{ 0x42d0, 0xfff8, {{255,255}, {255,255}, { 12,  0}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// move_w_ai
	{ 0x42d8, 0xfff8, {{255,255}, {255,255}, { 12,  0}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// move_w_pi
	{ 0x42e0, 0xfff8, {{255,255}, {255,255}, { 14,  2}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// move_w_pd
	{ 0x42e8, 0xfff8, {{255,255}, {255,255}, { 16,  0}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// move_w_di
	{ 0x42f0, 0xfff8, {{255,255}, {255,255}, { 18,  2}, { 11, -1}, { 11, -1}, { 11, -1}, { 11, -1}, {  4, -4}}},	// move_w_ix
	{ 0x4400, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// neg_b
	{ 0x4410, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_b_ai
	{ 0x4418, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_b_pi
	{ 0x4420, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_b_pd
	{ 0x4428, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_b_di
	{ 0x4430, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// neg_b_ix
	{ 0x4440, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// neg_w
	{ 0x4450, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_w_ai
	{ 0x4458, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_w_pi
	{ 0x4460, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_w_pd
	{ 0x4468, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_w_di
	{ 0x4470, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// neg_w_ix
	{ 0x4480, 0xfff8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// neg_l
	{ 0x4490, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_l_ai
	{ 0x4498, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_l_pi
	{ 0x44a0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_l_pd
	{ 0x44a8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_l_di
	{ 0x44b0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// neg_l_ix
	{ 0x44c0, 0xfff8, {{ 12,  8}, { 10,  6}, { 12,  8}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move_w
	{ 0x44d0, 0xfff8, {{ 16,  8}, { 14,  6}, { 16,  8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_ai
	{ 0x44d8, 0xfff8, {{ 16,  8}, { 14,  6}, { 16,  8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_pi
	{ 0x44e0, 0xfff8, {{ 18, 10}, { 16,  8}, { 18, 10}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pd
	{ 0x44e8, 0xfff8, {{ 20,  8}, { 18,  6}, { 20,  8}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_di
	{ 0x44f0, 0xfff8, {{ 22, 10}, { 20,  8}, { 22, 10}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_ix
	{ 0x4600, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// not_b
	{ 0x4610, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_b_ai
	{ 0x4618, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_b_pi
	{ 0x4620, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_b_pd
	{ 0x4628, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_b_di
	{ 0x4630, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// not_b_ix
	{ 0x4640, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// not_w
	{ 0x4650, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_w_ai
	{ 0x4658, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_w_pi
	{ 0x4660, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_w_pd
	{ 0x4668, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_w_di
	{ 0x4670, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// not_w_ix
	{ 0x4680, 0xfff8, {{  6,  2}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// not_l
	{ 0x4690, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_l_ai
	{ 0x4698, 0xfff8, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_l_pi
	{ 0x46a0, 0xfff8, {{ 22,  2}, { 25,  5}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_l_pd
	{ 0x46a8, 0xfff8, {{ 24,  0}, { 27,  3}, { 24,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_l_di
	{ 0x46b0, 0xfff8, {{ 26,  2}, { 29,  5}, { 26,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// not_l_ix
	{ 0x46c0, 0xfff8, {{ 12,  4}, { 10,  2}, { 12,  4}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// move_w
	{ 0x46d0, 0xfff8, {{ 16,  4}, { 14,  2}, { 16,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// move_w_ai
	{ 0x46d8, 0xfff8, {{ 16,  4}, { 14,  2}, { 16,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// move_w_pi
	{ 0x46e0, 0xfff8, {{ 18,  6}, { 16,  4}, { 18,  6}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// move_w_pd
	{ 0x46e8, 0xfff8, {{ 20,  4}, { 18,  2}, { 20,  4}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// move_w_di
	{ 0x46f0, 0xfff8, {{ 22,  6}, { 20,  4}, { 22,  6}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// move_w_ix
	{ 0x4800, 0xfff8, {{  6,  2}, { 10,  6}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}}},	// nbcd_b
	{ 0x4808, 0xfff8, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// link_l
	{ 0x4810, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// nbcd_b_ai
	{ 0x4818, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// nbcd_b_pi
	{ 0x4820, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// nbcd_b_pd
	{ 0x4828, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// nbcd_b_di
	{ 0x4830, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// nbcd_b_ix
	{ 0x4840, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// swap_l
	{ 0x4848, 0xfff8, {{255,255}, {255,255}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bkpt
	{ 0x4848, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}}},	// bkpt
	{ 0x4850, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// pea_l_ai
	{ 0x4868, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// pea_l_di
	{ 0x4870, 0xfff8, {{ 26,  6}, { 32, 12}, { 26,  6}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// pea_l_ix
	{ 0x4880, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// ext_w
	{ 0x4890, 0xfff8, {{  8,  0}, { 23, 15}, {  8,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_w_ai
	{ 0x48a0, 0xfff8, {{  8,  0}, { 23, 15}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// movem_w
	{ 0x48a8, 0xfff8, {{ 12,  0}, { 27, 15}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// movem_w_di
	{ 0x48b0, 0xfff8, {{ 14,  2}, { 29, 17}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// movem_w_ix
	{ 0x48c0, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// ext_l
	{ 0x48d0, 0xfff8, {{  8,  0}, { 23, 15}, {  8,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_l_ai
	{ 0x48e0, 0xfff8, {{  8,  0}, { 23, 15}, {  8,  0}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// movem_l
	{ 0x48e8, 0xfff8, {{ 12,  0}, { 27, 15}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// movem_l_di
	{ 0x48f0, 0xfff8, {{ 14,  2}, { 29, 17}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// movem_l_ix
	{ 0x49c0, 0xfff8, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// extb_l
	{ 0x4a00, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// tst_b
	{ 0x4a10, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_b_ai
	{ 0x4a18, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_b_pi
	{ 0x4a20, 0xfff8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_b_pd
	{ 0x4a28, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_b_di
	{ 0x4a30, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// tst_b_ix
	{ 0x4a40, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// tst_w
	{ 0x4a48, 0xfff8, {{255,255}, {255,255}, {255,255}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// tst_w
	{ 0x4a50, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_w_ai
	{ 0x4a58, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_w_pi
	{ 0x4a60, 0xfff8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_w_pd
	{ 0x4a68, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_w_di
	{ 0x4a70, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// tst_w_ix
	{ 0x4a80, 0xfff8, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// tst_l
	{ 0x4a88, 0xfff8, {{255,255}, {255,255}, {255,255}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// tst_l
	{ 0x4a90, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_l_ai
	{ 0x4a98, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_l_pi
	{ 0x4aa0, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_l_pd
	{ 0x4aa8, 0xfff8, {{ 16,  0}, { 19,  3}, { 16,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_l_di
	{ 0x4ab0, 0xfff8, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  2,  2}}},	// tst_l_ix
	{ 0x4ac0, 0xfff8, {{  4,  0}, { 10,  6}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// tas_b
	{ 0x4ad0, 0xfff8, {{ 18,  2}, { 19,  3}, { 18,  2}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// tas_b_ai
	{ 0x4ad8, 0xfff8, {{ 18,  2}, { 19,  3}, { 18,  2}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// tas_b_pi
	{ 0x4ae0, 0xfff8, {{ 20,  4}, { 21,  5}, { 20,  4}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// tas_b_pd
	{ 0x4ae8, 0xfff8, {{ 22,  2}, { 23,  3}, { 22,  2}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// tas_b_di
	{ 0x4af0, 0xfff8, {{ 24,  4}, { 25,  5}, { 24,  4}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 12, 12}}},	// tas_b_ix
	{ 0x4c00, 0xfff8, {{255,255}, {255,255}, {255,255}, { 43, 43}, { 43, 43}, { 43, 43}, { 43, 43}, { 43, 43}}},	// mull_l
	{ 0x4c10, 0xfff8, {{255,255}, {255,255}, {255,255}, { 47, 43}, { 47, 43}, { 47, 43}, { 47, 43}, { 43, 43}}},	// mull_l_ai
	{ 0x4c18, 0xfff8, {{255,255}, {255,255}, {255,255}, { 47, 43}, { 47, 43}, { 47, 43}, { 47, 43}, { 43, 43}}},	// mull_l_pi
	{ 0x4c20, 0xfff8, {{255,255}, {255,255}, {255,255}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 43, 43}}},	// mull_l_pd
	{ 0x4c28, 0xfff8, {{255,255}, {255,255}, {255,255}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 43, 43}}},	// mull_l_di
	{ 0x4c30, 0xfff8, {{255,255}, {255,255}, {255,255}, { 50, 46}, { 50, 46}, { 50, 46}, { 50, 46}, { 43, 43}}},	// mull_l_ix
	{ 0x4c40, 0xfff8, {{255,255}, {255,255}, {255,255}, { 84, 84}, { 84, 84}, { 84, 84}, { 84, 84}, { 84, 84}}},	// divl_l
	{ 0x4c50, 0xfff8, {{255,255}, {255,255}, {255,255}, { 88, 84}, { 88, 84}, { 88, 84}, { 88, 84}, { 84, 84}}},	// divl_l_ai
	{ 0x4c58, 0xfff8, {{255,255}, {255,255}, {255,255}, { 88, 84}, { 88, 84}, { 88, 84}, { 88, 84}, { 84, 84}}},	// divl_l_pi
	{ 0x4c60, 0xfff8, {{255,255}, {255,255}, {255,255}, { 89, 85}, { 89, 85}, { 89, 85}, { 89, 85}, { 84, 84}}},	// divl_l_pd
	{ 0x4c68, 0xfff8, {{255,255}, {255,255}, {255,255}, { 89, 85}, { 89, 85}, { 89, 85}, { 89, 85}, { 84, 84}}},	// divl_l_di
	{ 0x4c70, 0xfff8, {{255,255}, {255,255}, {255,255}, { 91, 87}, { 91, 87}, { 91, 87}, { 91, 87}, { 84, 84}}},	// divl_l_ix
	{ 0x4c90, 0xfff8, {{ 12,  0}, { 26, 14}, { 12,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_w_ai
	{ 0x4c98, 0xfff8, {{ 12,  0}, { 26, 14}, { 12,  0}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// movem_w
	{ 0x4ca8, 0xfff8, {{ 16,  0}, { 30, 14}, { 16,  0}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// movem_w_di
	{ 0x4cb0, 0xfff8, {{ 18,  2}, { 32, 16}, { 18,  2}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// movem_w_ix
	{ 0x4cd0, 0xfff8, {{ 12,  0}, { 26, 14}, { 12,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_l_ai
	{ 0x4cd8, 0xfff8, {{ 12,  0}, { 26, 14}, { 12,  0}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// movem_l
	{ 0x4ce8, 0xfff8, {{ 16,  0}, { 30, 14}, { 16,  0}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// movem_l_di
	{ 0x4cf0, 0xfff8, {{ 18,  2}, { 32, 16}, { 18,  2}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// movem_l_ix
	{ 0x4e50, 0xfff8, {{ 16,  0}, { 25,  9}, { 16,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// link_w
	{ 0x4e58, 0xfff8, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// unlk_l
	{ 0x4e60, 0xfff8, {{  4,  0}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_l
	{ 0x4e68, 0xfff8, {{  4,  0}, {  7,  3}, {  6,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// move_l
	{ 0x4e90, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jsr_l_ai
	{ 0x4ea8, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, {  5,  1}, {  5,  1}, {  5,  1}, {  5,  1}, {  0,  0}}},	// jsr_l_di
	{ 0x4eb0, 0xfff8, {{ 22,  2}, { 28,  8}, { 22,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  0,  0}}},	// jsr_l_ix
	{ 0x4ed0, 0xfff8, {{  8,  0}, { 11,  3}, {  8,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jmp_l_ai
	{ 0x4ee8, 0xfff8, {{ 10,  2}, { 13,  5}, { 10,  2}, {  5,  1}, {  5,  1}, {  5,  1}, {  5,  1}, {  0,  0}}},	// jmp_l_di
	{ 0x4ef0, 0xfff8, {{ 14,  2}, { 17,  5}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  0,  0}}},	// jmp_l_ix
	{ 0x50c0, 0xfff8, {{  6,  2}, { 13,  9}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// st_b
	{ 0x50c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 12,  4}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbt_w
	{ 0x50d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// st_b_ai
	{ 0x50d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// st_b_pi
	{ 0x50e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// st_b_pd
	{ 0x50e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// st_b_di
	{ 0x50f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// st_b_ix
	{ 0x51c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sf_b
	{ 0x51c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// dbf_w
	{ 0x51d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sf_b_ai
	{ 0x51d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sf_b_pi
	{ 0x51e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sf_b_pd
	{ 0x51e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sf_b_di
	{ 0x51f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sf_b_ix
	{ 0x52c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// shi_b
	{ 0x52c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbhi_w
	{ 0x52d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// shi_b_ai
	{ 0x52d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// shi_b_pi
	{ 0x52e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// shi_b_pd
	{ 0x52e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// shi_b_di
	{ 0x52f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// shi_b_ix
	{ 0x53c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sls_b
	{ 0x53c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbls_w
	{ 0x53d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sls_b_ai
	{ 0x53d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sls_b_pi
	{ 0x53e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sls_b_pd
	{ 0x53e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sls_b_di
	{ 0x53f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sls_b_ix
	{ 0x54c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// scc_b
	{ 0x54c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbcc_w
	{ 0x54d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scc_b_ai
	{ 0x54d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scc_b_pi
	{ 0x54e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scc_b_pd
	{ 0x54e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scc_b_di
	{ 0x54f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// scc_b_ix
	{ 0x55c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// scs_b
	{ 0x55c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbcs_w
	{ 0x55d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scs_b_ai
	{ 0x55d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scs_b_pi
	{ 0x55e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scs_b_pd
	{ 0x55e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scs_b_di
	{ 0x55f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// scs_b_ix
	{ 0x56c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sne_b
	{ 0x56c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbne_w
	{ 0x56d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sne_b_ai
	{ 0x56d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sne_b_pi
	{ 0x56e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sne_b_pd
	{ 0x56e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sne_b_di
	{ 0x56f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sne_b_ix
	{ 0x57c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// seq_b
	{ 0x57c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbeq_w
	{ 0x57d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// seq_b_ai
	{ 0x57d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// seq_b_pi
	{ 0x57e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// seq_b_pd
	{ 0x57e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// seq_b_di
	{ 0x57f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// seq_b_ix
	{ 0x58c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// svc_b
	{ 0x58c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbvc_w
	{ 0x58d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svc_b_ai
	{ 0x58d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svc_b_pi
	{ 0x58e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svc_b_pd
	{ 0x58e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svc_b_di
	{ 0x58f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// svc_b_ix
	{ 0x59c0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// svs_b
	{ 0x59c8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbvs_w
	{ 0x59d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svs_b_ai
	{ 0x59d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svs_b_pi
	{ 0x59e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svs_b_pd
	{ 0x59e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svs_b_di
	{ 0x59f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// svs_b_ix
	{ 0x5ac0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// spl_b
	{ 0x5ac8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbpl_w
	{ 0x5ad0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// spl_b_ai
	{ 0x5ad8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// spl_b_pi
	{ 0x5ae0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// spl_b_pd
	{ 0x5ae8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// spl_b_di
	{ 0x5af0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// spl_b_ix
	{ 0x5bc0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// smi_b
	{ 0x5bc8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbmi_w
	{ 0x5bd0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// smi_b_ai
	{ 0x5bd8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// smi_b_pi
	{ 0x5be0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// smi_b_pd
	{ 0x5be8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// smi_b_di
	{ 0x5bf0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// smi_b_ix
	{ 0x5cc0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sge_b
	{ 0x5cc8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbge_w
	{ 0x5cd0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sge_b_ai
	{ 0x5cd8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sge_b_pi
	{ 0x5ce0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sge_b_pd
	{ 0x5ce8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sge_b_di
	{ 0x5cf0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sge_b_ix
	{ 0x5dc0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// slt_b
	{ 0x5dc8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dblt_w
	{ 0x5dd0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// slt_b_ai
	{ 0x5dd8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// slt_b_pi
	{ 0x5de0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// slt_b_pd
	{ 0x5de8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// slt_b_di
	{ 0x5df0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// slt_b_ix
	{ 0x5ec0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sgt_b
	{ 0x5ec8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dbgt_w
	{ 0x5ed0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sgt_b_ai
	{ 0x5ed8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sgt_b_pi
	{ 0x5ee0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sgt_b_pd
	{ 0x5ee8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sgt_b_di
	{ 0x5ef0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sgt_b_ix
	{ 0x5fc0, 0xfff8, {{  4,  0}, { 13,  9}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// sle_b
	{ 0x5fc8, 0xfff8, {{ 12,  4}, { 14,  6}, { 10, 10}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// dble_w
	{ 0x5fd0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sle_b_ai
	{ 0x5fd8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sle_b_pi
	{ 0x5fe0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sle_b_pd
	{ 0x5fe8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sle_b_di
	{ 0x5ff0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// sle_b_ix
	{ 0x8f08, 0xfff8, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// sbcd_b
	{ 0x8f48, 0xfff8, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// pack_w
	{ 0x8f88, 0xfff8, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// unpk_w
	{ 0x9f08, 0xfff8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_b
	{ 0xbf08, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_b
	{ 0xcf08, 0xfff8, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// abcd_b
	{ 0xdf08, 0xfff8, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_b
	{ 0xe0d0, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// asr_w_ai
	{ 0xe0d8, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// asr_w_pi
	{ 0xe0e0, 0xfff8, {{ 14,  6}, { 20, 12}, { 14,  6}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// asr_w_pd
	{ 0xe0e8, 0xfff8, {{ 16,  4}, { 22, 10}, { 16,  4}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// asr_w_di
	{ 0xe0f0, 0xfff8, {{ 18,  6}, { 24, 12}, { 18,  6}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// asr_w_ix
	{ 0xe1d0, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, {  6,  2}}},	// asl_w_ai
	{ 0xe1d8, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, {  6,  2}}},	// asl_w_pi
	{ 0xe1e0, 0xfff8, {{ 14,  6}, { 20, 12}, { 14,  6}, { 11,  3}, { 11,  3}, { 11,  3}, { 11,  3}, {  6,  2}}},	// asl_w_pd
	{ 0xe1e8, 0xfff8, {{ 16,  4}, { 22, 10}, { 16,  4}, { 11,  3}, { 11,  3}, { 11,  3}, { 11,  3}, {  6,  2}}},	// asl_w_di
	{ 0xe1f0, 0xfff8, {{ 18,  6}, { 24, 12}, { 18,  6}, { 13,  5}, { 13,  5}, { 13,  5}, { 13,  5}, {  6,  2}}},	// asl_w_ix
	{ 0xe2d0, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsr_w_ai
	{ 0xe2d8, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsr_w_pi
	{ 0xe2e0, 0xfff8, {{ 14,  6}, { 20, 12}, { 14,  6}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// lsr_w_pd
	{ 0xe2e8, 0xfff8, {{ 16,  4}, { 22, 10}, { 16,  4}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// lsr_w_di
	{ 0xe2f0, 0xfff8, {{ 18,  6}, { 24, 12}, { 18,  6}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// lsr_w_ix
	{ 0xe3d0, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsl_w_ai
	{ 0xe3d8, 0xfff8, {{ 12,  4}, { 18, 10}, { 12,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsl_w_pi
	{ 0xe3e0, 0xfff8, {{ 14,  6}, { 20, 12}, { 14,  6}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// lsl_w_pd
	{ 0xe3e8, 0xfff8, {{ 16,  4}, { 22, 10}, { 16,  4}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// lsl_w_di
	{ 0xe3f0, 0xfff8, {{ 18,  6}, { 24, 12}, { 18,  6}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// lsl_w_ix
	{ 0xe4d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxr_w_ai
	{ 0xe4d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxr_w_pi
	{ 0xe4e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// roxr_w_pd
	{ 0xe4e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// roxr_w_di
	{ 0xe4f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// roxr_w_ix
	{ 0xe5d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxl_w_ai
	{ 0xe5d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxl_w_pi
	{ 0xe5e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// roxl_w_pd
	{ 0xe5e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// roxl_w_di
	{ 0xe5f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// roxl_w_ix
	{ 0xe6d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// ror_w_ai
	{ 0xe6d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// ror_w_pi
	{ 0xe6e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// ror_w_pd
	{ 0xe6e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// ror_w_di
	{ 0xe6f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// ror_w_ix
	{ 0xe7d0, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// rol_w_ai
	{ 0xe7d8, 0xfff8, {{ 12,  0}, { 18,  6}, { 12,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// rol_w_pi
	{ 0xe7e0, 0xfff8, {{ 14,  2}, { 20,  8}, { 14,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// rol_w_pd
	{ 0xe7e8, 0xfff8, {{ 16,  0}, { 22,  6}, { 16,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  7,  7}}},	// rol_w_di
	{ 0xe7f0, 0xfff8, {{ 18,  2}, { 24,  8}, { 18,  2}, { 14, 10}, { 14, 10}, { 14, 10}, { 14, 10}, {  7,  7}}},	// rol_w_ix
	{ 0xe8c0, 0xfff8, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bftst_l
	{ 0xe8d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 13, 13}}},	// bftst_l_ai
	{ 0xe8e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 18, 14}, { 18, 14}, { 18, 14}, { 18, 14}, { 13, 13}}},	// bftst_l_di
	{ 0xe8f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 13, 13}}},	// bftst_l_ix
	{ 0xe9c0, 0xfff8, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// bfextu_l
	{ 0xe9d0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfextu_l_ai
	{ 0xe9e8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 15, 15}}},	// bfextu_l_di
	{ 0xe9f0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 15, 15}}},	// bfextu_l_ix
	{ 0xeac0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// bfchg_l
	{ 0xead0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfchg_l_ai
	{ 0xeae8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 20, 20}}},	// bfchg_l_di
	{ 0xeaf0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 27, 23}, { 27, 23}, { 27, 23}, { 27, 23}, { 20, 20}}},	// bfchg_l_ix
	{ 0xebc0, 0xfff8, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// bfexts_l
	{ 0xebd0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfexts_l_ai
	{ 0xebe8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 15, 15}}},	// bfexts_l_di
	{ 0xebf0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 15, 15}}},	// bfexts_l_ix
	{ 0xecc0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// bfclr_l
	{ 0xecd0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfclr_l_ai
	{ 0xece8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 20, 20}}},	// bfclr_l_di
	{ 0xecf0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 27, 23}, { 27, 23}, { 27, 23}, { 27, 23}, { 20, 20}}},	// bfclr_l_ix
	{ 0xedc0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 18, 18}, { 18, 18}, { 18, 18}, { 18, 18}, { 18, 18}}},	// bfffo_l
	{ 0xedd0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 28, 28}}},	// bfffo_l_ai
	{ 0xede8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 33, 29}, { 33, 29}, { 33, 29}, { 33, 29}, { 28, 28}}},	// bfffo_l_di
	{ 0xedf0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 35, 31}, { 35, 31}, { 35, 31}, { 35, 31}, { 28, 28}}},	// bfffo_l_ix
	{ 0xeec0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// bfset_l
	{ 0xeed0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfset_l_ai
	{ 0xeee8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 25, 21}, { 25, 21}, { 25, 21}, { 25, 21}, { 20, 20}}},	// bfset_l_di
	{ 0xeef0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 27, 23}, { 27, 23}, { 27, 23}, { 27, 23}, { 20, 20}}},	// bfset_l_ix
	{ 0xefc0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}}},	// bfins_l
	{ 0xefd0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 21, 17}, { 21, 17}, { 21, 17}, { 21, 17}, { 17, 17}}},	// bfins_l_ai
	{ 0xefe8, 0xfff8, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 17, 17}}},	// bfins_l_di
	{ 0xeff0, 0xfff8, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 17, 17}}},	// bfins_l_ix
	{ 0xf278, 0xfff8, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {255,255}, {255,255}, {255,255}}},	// ftrapcc_l
	{ 0xf510, 0xfff8, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}}},	// pflushan_l
	{ 0xf518, 0xfff8, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}}},	// pflusha_l
	{ 0xf620, 0xfff8, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}}},	// move16_l
	{ 0x001f, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_b_pi7
	{ 0x0027, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// ori_b_pd7
	{ 0x0038, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_b_aw
	{ 0x0039, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_b_al
	{ 0x003c, 0xffff, {{ 20,  8}, { 14,  6}, { 16,  8}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// ori_w
	{ 0x0078, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_w_aw
	{ 0x0079, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_w_al
	{ 0x007c, 0xffff, {{ 20,  8}, { 14,  6}, { 16,  8}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// ori_w
	{ 0x00b8, 0xffff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_l_aw
	{ 0x00b9, 0xffff, {{ 36,  0}, { 42,  6}, { 36,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// ori_l_al
	{ 0x00f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_b_aw
	{ 0x00f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_b_al
	{ 0x00fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_b
	{ 0x00fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_b
	{ 0x021f, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_b_pi7
	{ 0x0227, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// andi_b_pd7
	{ 0x0238, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_b_aw
	{ 0x0239, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_b_al
	{ 0x023c, 0xffff, {{ 20,  8}, { 14,  6}, { 16, 16}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// andi_w
	{ 0x0278, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_w_aw
	{ 0x0279, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_w_al
	{ 0x027c, 0xffff, {{ 20,  8}, { 14,  6}, { 16, 16}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// andi_w
	{ 0x02b8, 0xffff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_l_aw
	{ 0x02b9, 0xffff, {{ 36,  0}, { 42,  6}, { 36,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// andi_l_al
	{ 0x02f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_w_aw
	{ 0x02f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_w_al
	{ 0x02fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_w
	{ 0x02fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_w
	{ 0x041f, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_b_pi7
	{ 0x0427, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// subi_b_pd7
	{ 0x0438, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_b_aw
	{ 0x0439, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_b_al
	{ 0x0478, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_w_aw
	{ 0x0479, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_w_al
	{ 0x04b8, 0xffff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_l_aw
	{ 0x04b9, 0xffff, {{ 36,  0}, { 42,  6}, { 36,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// subi_l_al
	{ 0x04f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_l_aw
	{ 0x04f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 18, 18}}},	// chk2cmp2_l_al
	{ 0x04fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_l
	{ 0x04fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}, { 23, 23}}},	// chk2cmp2_l
	{ 0x061f, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_b_pi7
	{ 0x0627, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// addi_b_pd7
	{ 0x0638, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_b_aw
	{ 0x0639, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_b_al
	{ 0x0678, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_w_aw
	{ 0x0679, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_w_al
	{ 0x06b8, 0xffff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_l_aw
	{ 0x06b9, 0xffff, {{ 36,  0}, { 42,  6}, { 36,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// addi_l_al
	{ 0x06f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 64, 60}, {255,255}, {255,255}, { 64, 60}, {255,255}}},	// callm_l_aw
	{ 0x06f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 64, 60}, {255,255}, {255,255}, { 64, 60}, {255,255}}},	// callm_l_al
	{ 0x06fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 65, 61}, {255,255}, {255,255}, { 65, 61}, {255,255}}},	// callm_l_pcdi
	{ 0x06fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 67, 63}, {255,255}, {255,255}, { 67, 63}, {255,255}}},	// callm_l_pcix
	{ 0x081f, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// btst_b_pi7
	{ 0x0827, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// btst_b_pd7
	{ 0x0838, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// btst_b_aw
	{ 0x0839, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// btst_b_al
	{ 0x083a, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// btst_b_pcdi
	{ 0x083b, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// btst_b_pcix
	{ 0x085f, 0xffff, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_pi7
	{ 0x0867, 0xffff, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bchg_b_pd7
	{ 0x0878, 0xffff, {{ 20,  0}, { 29,  9}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_aw
	{ 0x0879, 0xffff, {{ 24,  0}, { 33,  9}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bchg_b_al
	{ 0x089f, 0xffff, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_pi7
	{ 0x08a7, 0xffff, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bclr_b_pd7
	{ 0x08b8, 0xffff, {{ 20,  0}, { 29,  9}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_aw
	{ 0x08b9, 0xffff, {{ 24,  0}, { 33,  9}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bclr_b_al
	{ 0x08df, 0xffff, {{ 16,  0}, { 25,  9}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_pi7
	{ 0x08e7, 0xffff, {{ 18,  2}, { 27, 11}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// bset_b_pd7
	{ 0x08f8, 0xffff, {{ 20,  0}, { 29,  9}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_aw
	{ 0x08f9, 0xffff, {{ 24,  0}, { 33,  9}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// bset_b_al
	{ 0x0a1f, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_b_pi7
	{ 0x0a27, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// eori_b_pd7
	{ 0x0a38, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_b_aw
	{ 0x0a39, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_b_al
	{ 0x0a3c, 0xffff, {{ 20,  8}, { 14,  6}, { 16,  8}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// eori_w
	{ 0x0a78, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_w_aw
	{ 0x0a79, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_w_al
	{ 0x0a7c, 0xffff, {{ 20,  8}, { 14,  6}, { 16,  8}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// eori_w
	{ 0x0ab8, 0xffff, {{ 32,  0}, { 38,  6}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_l_aw
	{ 0x0ab9, 0xffff, {{ 36,  0}, { 42,  6}, { 36,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// eori_l_al
	{ 0x0adf, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_b_pi7
	{ 0x0ae7, 0xffff, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// cas_b_pd7
	{ 0x0af8, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_b_aw
	{ 0x0af9, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_b_al
	{ 0x0c1f, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_b_pi7
	{ 0x0c27, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// cmpi_b_pd7
	{ 0x0c38, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_b_aw
	{ 0x0c39, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_b_al
	{ 0x0c3a, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// cmpi_b
	{ 0x0c3b, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpi_b
	{ 0x0c78, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_w_aw
	{ 0x0c79, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_w_al
	{ 0x0c7a, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// cmpi_w
	{ 0x0c7b, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpi_w
	{ 0x0cb8, 0xffff, {{ 24,  0}, { 30,  6}, { 24,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_l_aw
	{ 0x0cb9, 0xffff, {{ 28,  0}, { 34,  6}, { 28,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// cmpi_l_al
	{ 0x0cba, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// cmpi_l
	{ 0x0cbb, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpi_l
	{ 0x0cf8, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_w_aw
	{ 0x0cf9, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_w_al
	{ 0x0cfc, 0xffff, {{255,255}, {255,255}, {255,255}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// cas2_w
	{ 0x0e1f, 0xffff, {{255,255}, {255,255}, { 18,  6}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_b_pi7
	{ 0x0e1f, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_pi7
	{ 0x0e27, 0xffff, {{255,255}, {255,255}, { 20,  4}, {255,255}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// moves_b_pd7
	{ 0x0e27, 0xffff, {{255,255}, {255,255}, {255,255}, { 10,  6}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_pd7
	{ 0x0e38, 0xffff, {{255,255}, {255,255}, { 26, 10}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_b_aw
	{ 0x0e38, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_aw
	{ 0x0e39, 0xffff, {{255,255}, {255,255}, { 30, 10}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_b_al
	{ 0x0e39, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_b_al
	{ 0x0e78, 0xffff, {{255,255}, {255,255}, { 26, 10}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_w_aw
	{ 0x0e78, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_aw
	{ 0x0e79, 0xffff, {{255,255}, {255,255}, { 30, 10}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_w_al
	{ 0x0e79, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_w_al
	{ 0x0eb8, 0xffff, {{255,255}, {255,255}, { 32, 12}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_l_aw
	{ 0x0eb8, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_aw
	{ 0x0eb9, 0xffff, {{255,255}, {255,255}, { 36, 12}, {255,255}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// moves_l_al
	{ 0x0eb9, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  5}, {255,255}, {255,255}, {255,255}, {255,255}}},	// moves_l_al
	{ 0x0ef8, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_l_aw
	{ 0x0ef9, 0xffff, {{255,255}, {255,255}, {255,255}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// cas_l_al
	{ 0x0efc, 0xffff, {{255,255}, {255,255}, {255,255}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// cas2_l
	{ 0x11df, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi7
	{ 0x11e7, 0xffff, {{ 18,  2}, { 21,  5}, { 18,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd7
	{ 0x11f8, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_aw
	{ 0x11f9, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_al
	{ 0x11fa, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pcdi
	{ 0x11fb, 0xffff, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_pcix
	{ 0x11fc, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_b_i
	{ 0x13df, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_b_pi7
	{ 0x13e7, 0xffff, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_b_pd7
	{ 0x13f8, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_b_aw
	{ 0x13f9, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_b_al
	{ 0x13fa, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_b_pcdi
	{ 0x13fb, 0xffff, {{ 26,  2}, { 29,  5}, { 26,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_b_pcix
	{ 0x13fc, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  6,  6}}},	// move_b_i
	{ 0x1edf, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_pi7
	{ 0x1ee7, 0xffff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pd7
	{ 0x1ef8, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_aw
	{ 0x1ef9, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_b_al
	{ 0x1efa, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_b_pcdi
	{ 0x1efb, 0xffff, {{ 18,  2}, { 21,  5}, { 18,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_b_pcix
	{ 0x1efc, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_b_i
	{ 0x1f1f, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_pi7
	{ 0x1f27, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pd7
	{ 0x1f38, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_aw
	{ 0x1f39, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// move_b_al
	{ 0x1f3a, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// move_b_pcdi
	{ 0x1f3b, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// move_b_pcix
	{ 0x1f3c, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  5,  5}}},	// move_b_i
	{ 0x21f8, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_aw
	{ 0x21f9, 0xffff, {{ 32,  0}, { 35,  3}, { 32,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_al
	{ 0x21fa, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_l_pcdi
	{ 0x21fb, 0xffff, {{ 30,  2}, { 33,  5}, { 30,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_l_pcix
	{ 0x21fc, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_l_i
	{ 0x23f8, 0xffff, {{ 32,  0}, { 35,  3}, { 32,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_l_aw
	{ 0x23f9, 0xffff, {{ 36,  0}, { 39,  3}, { 36,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_l_al
	{ 0x23fa, 0xffff, {{ 32,  0}, { 35,  3}, { 32,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_l_pcdi
	{ 0x23fb, 0xffff, {{ 34,  2}, { 37,  5}, { 34,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_l_pcix
	{ 0x23fc, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_l_i
	{ 0x31f8, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_aw
	{ 0x31f9, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_al
	{ 0x31fa, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pcdi
	{ 0x31fb, 0xffff, {{ 22,  2}, { 25,  5}, { 22,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_pcix
	{ 0x31fc, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_w_i
	{ 0x33f8, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_w_aw
	{ 0x33f9, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// move_w_al
	{ 0x33fa, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// move_w_pcdi
	{ 0x33fb, 0xffff, {{ 26,  2}, { 29,  5}, { 26,  2}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  6,  6}}},	// move_w_pcix
	{ 0x33fc, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  6,  6}}},	// move_w_i
	{ 0x401f, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_b_pi7
	{ 0x4027, 0xffff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// negx_b_pd7
	{ 0x4038, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_b_aw
	{ 0x4039, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_b_al
	{ 0x4078, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_w_aw
	{ 0x4079, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_w_al
	{ 0x40b8, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_l_aw
	{ 0x40b9, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// negx_l_al
	{ 0x40f8, 0xffff, {{ 16,  0}, { 19,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_aw
	{ 0x40f8, 0xffff, {{255,255}, {255,255}, { 16,  0}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, {  8,  0}}},	// move_w_aw
	{ 0x40f9, 0xffff, {{ 20,  0}, { 23,  3}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// move_w_al
	{ 0x40f9, 0xffff, {{255,255}, {255,255}, { 20,  0}, { 12,  0}, { 12,  0}, { 12,  0}, { 12,  0}, {  8,  0}}},	// move_w_al
	{ 0x421f, 0xffff, {{ 12,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_pi7
	{ 0x421f, 0xffff, {{255,255}, { 15,  3}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_b_pi7
	{ 0x4227, 0xffff, {{ 14,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_pd7
	{ 0x4227, 0xffff, {{255,255}, { 17,  5}, { 10, -2}, {  9, -3}, {  9, -3}, {  9, -3}, {  9, -3}, {  4, -4}}},	// clr_b_pd7
	{ 0x4238, 0xffff, {{ 16,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_aw
	{ 0x4238, 0xffff, {{255,255}, { 19,  3}, { 12, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_b_aw
	{ 0x4239, 0xffff, {{ 20,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_b_al
	{ 0x4239, 0xffff, {{255,255}, { 23,  3}, { 14, -6}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_b_al
	{ 0x4278, 0xffff, {{ 16,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_aw
	{ 0x4278, 0xffff, {{255,255}, { 19,  3}, { 12, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_w_aw
	{ 0x4279, 0xffff, {{ 20,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_w_al
	{ 0x4279, 0xffff, {{255,255}, { 23,  3}, { 14, -6}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// clr_w_al
	{ 0x42b8, 0xffff, {{ 24,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_aw
	{ 0x42b8, 0xffff, {{255,255}, { 27,  3}, { 16, -8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// clr_l_aw
	{ 0x42b9, 0xffff, {{ 28,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// clr_l_al
	{ 0x42b9, 0xffff, {{255,255}, { 31,  3}, { 20, -8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// clr_l_al
	{ 0x42f8, 0xffff, {{255,255}, {255,255}, { 16,  0}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// move_w_aw
	{ 0x42f9, 0xffff, {{255,255}, {255,255}, { 20,  0}, {  8, -4}, {  8, -4}, {  8, -4}, {  8, -4}, {  4, -4}}},	// move_w_al
	{ 0x441f, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_b_pi7
	{ 0x4427, 0xffff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// neg_b_pd7
	{ 0x4438, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_b_aw
	{ 0x4439, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_b_al
	{ 0x4478, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_w_aw
	{ 0x4479, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_w_al
	{ 0x44b8, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_l_aw
	{ 0x44b9, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// neg_l_al
	{ 0x44f8, 0xffff, {{ 20,  8}, { 18,  6}, { 20,  8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_aw
	{ 0x44f9, 0xffff, {{ 24,  8}, { 22,  6}, { 24,  8}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// move_w_al
	{ 0x44fa, 0xffff, {{ 20,  8}, { 18,  6}, { 20,  8}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// move_w_pcdi
	{ 0x44fb, 0xffff, {{ 22, 10}, { 20,  8}, { 22, 10}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  4,  4}}},	// move_w_pcix
	{ 0x44fc, 0xffff, {{ 16,  8}, { 14,  6}, { 16,  8}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  4,  4}}},	// move_w_i
	{ 0x461f, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_b_pi7
	{ 0x4627, 0xffff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  4,  4}}},	// not_b_pd7
	{ 0x4638, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_b_aw
	{ 0x4639, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_b_al
	{ 0x4678, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_w_aw
	{ 0x4679, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_w_al
	{ 0x46b8, 0xffff, {{ 24,  0}, { 27,  3}, { 24,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_l_aw
	{ 0x46b9, 0xffff, {{ 28,  0}, { 31,  3}, { 28,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// not_l_al
	{ 0x46f8, 0xffff, {{ 20,  4}, { 18,  2}, { 20,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// move_w_aw
	{ 0x46f9, 0xffff, {{ 24,  4}, { 22,  2}, { 24,  4}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// move_w_al
	{ 0x46fa, 0xffff, {{ 20,  4}, { 18,  2}, { 20,  4}, { 13,  9}, { 13,  9}, { 13,  9}, { 13,  9}, {  8,  8}}},	// move_w_pcdi
	{ 0x46fb, 0xffff, {{ 22,  6}, { 20,  4}, { 22,  6}, { 15, 11}, { 15, 11}, { 15, 11}, { 15, 11}, {  8,  8}}},	// move_w_pcix
	{ 0x46fc, 0xffff, {{ 16,  4}, { 14,  2}, { 16,  4}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  8,  8}}},	// move_w_i
	{ 0x480f, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// link_l
	{ 0x481f, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// nbcd_b_pi7
	{ 0x4827, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// nbcd_b_pd7
	{ 0x4838, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// nbcd_b_aw
	{ 0x4839, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// nbcd_b_al
	{ 0x4878, 0xffff, {{ 22,  2}, { 28,  8}, { 22,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// pea_l_aw
	{ 0x4879, 0xffff, {{ 26,  2}, { 32,  8}, { 26,  2}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// pea_l_al
	{ 0x487a, 0xffff, {{ 22,  2}, { 28,  8}, { 22,  2}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  5,  5}}},	// pea_l_pcdi
	{ 0x487b, 0xffff, {{ 26,  6}, { 32, 12}, { 26,  6}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  5,  5}}},	// pea_l_pcix
	{ 0x48b8, 0xffff, {{ 12,  0}, { 27, 15}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_w_aw
	{ 0x48b9, 0xffff, {{ 16,  0}, { 31, 15}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_w_al
	{ 0x48f8, 0xffff, {{ 12,  0}, { 27, 15}, { 12,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_l_aw
	{ 0x48f9, 0xffff, {{ 16,  0}, { 31, 15}, { 16,  0}, {  8,  4}, {  8,  4}, {  8,  4}, {  8,  4}, {  4,  4}}},	// movem_l_al
	{ 0x4a1f, 0xffff, {{  8,  0}, { 11,  3}, {  8,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_b_pi7
	{ 0x4a27, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  2,  2}}},	// tst_b_pd7
	{ 0x4a38, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_b_aw
	{ 0x4a39, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_b_al
	{ 0x4a3a, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// tst_b
	{ 0x4a3b, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// tst_b
	{ 0x4a3c, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// tst_b
	{ 0x4a78, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_w_aw
	{ 0x4a79, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_w_al
	{ 0x4a7a, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// tst_w
	{ 0x4a7b, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// tst_w
	{ 0x4a7c, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// tst_w
	{ 0x4ab8, 0xffff, {{ 16,  0}, { 19,  3}, { 16,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_l_aw
	{ 0x4ab9, 0xffff, {{ 20,  0}, { 23,  3}, { 20,  0}, {  6,  2}, {  6,  2}, {  6,  2}, {  6,  2}, {  2,  2}}},	// tst_l_al
	{ 0x4aba, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// tst_l
	{ 0x4abb, 0xffff, {{255,255}, {255,255}, {255,255}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// tst_l
	{ 0x4abc, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// tst_l
	{ 0x4adf, 0xffff, {{ 18,  2}, { 19,  3}, { 18,  2}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// tas_b_pi7
	{ 0x4ae7, 0xffff, {{ 20,  4}, { 21,  5}, { 20,  4}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 12, 12}}},	// tas_b_pd7
	{ 0x4af8, 0xffff, {{ 22,  2}, { 23,  3}, { 22,  2}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// tas_b_aw
	{ 0x4af9, 0xffff, {{ 26,  2}, { 27,  3}, { 26,  2}, { 16, 12}, { 16, 12}, { 16, 12}, { 16, 12}, { 12, 12}}},	// tas_b_al
	{ 0x4afc, 0xffff, {{  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// illegal
	{ 0x4c38, 0xffff, {{255,255}, {255,255}, {255,255}, { 47, 43}, { 47, 43}, { 47, 43}, { 47, 43}, { 43, 43}}},	// mull_l_aw
	{ 0x4c39, 0xffff, {{255,255}, {255,255}, {255,255}, { 47, 43}, { 47, 43}, { 47, 43}, { 47, 43}, { 43, 43}}},	// mull_l_al
	{ 0x4c3a, 0xffff, {{255,255}, {255,255}, {255,255}, { 48, 44}, { 48, 44}, { 48, 44}, { 48, 44}, { 43, 43}}},	// mull_l_pcdi
	{ 0x4c3b, 0xffff, {{255,255}, {255,255}, {255,255}, { 50, 46}, { 50, 46}, { 50, 46}, { 50, 46}, { 43, 43}}},	// mull_l_pcix
	{ 0x4c3c, 0xffff, {{255,255}, {255,255}, {255,255}, { 47, 43}, { 47, 43}, { 47, 43}, { 47, 43}, { 43, 43}}},	// mull_l_i
	{ 0x4c78, 0xffff, {{255,255}, {255,255}, {255,255}, { 88, 84}, { 88, 84}, { 88, 84}, { 88, 84}, { 84, 84}}},	// divl_l_aw
	{ 0x4c79, 0xffff, {{255,255}, {255,255}, {255,255}, { 88, 84}, { 88, 84}, { 88, 84}, { 88, 84}, { 84, 84}}},	// divl_l_al
	{ 0x4c7a, 0xffff, {{255,255}, {255,255}, {255,255}, { 89, 85}, { 89, 85}, { 89, 85}, { 89, 85}, { 84, 84}}},	// divl_l_pcdi
	{ 0x4c7b, 0xffff, {{255,255}, {255,255}, {255,255}, { 91, 87}, { 91, 87}, { 91, 87}, { 91, 87}, { 84, 84}}},	// divl_l_pcix
	{ 0x4c7c, 0xffff, {{255,255}, {255,255}, {255,255}, { 88, 84}, { 88, 84}, { 88, 84}, { 88, 84}, { 84, 84}}},	// divl_l_i
	{ 0x4cb8, 0xffff, {{ 16,  0}, { 30, 14}, { 16,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_w_aw
	{ 0x4cb9, 0xffff, {{ 20,  0}, { 34, 14}, { 20,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_w_al
	{ 0x4cba, 0xffff, {{ 16,  0}, { 30, 14}, { 16,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// movem_w
	{ 0x4cbb, 0xffff, {{ 18,  2}, { 33, 17}, { 18,  2}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}}},	// movem_w
	{ 0x4cf8, 0xffff, {{ 16,  0}, { 30, 14}, { 16,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_l_aw
	{ 0x4cf9, 0xffff, {{ 20,  0}, { 34, 14}, { 20,  0}, { 12,  8}, { 12,  8}, { 12,  8}, { 12,  8}, {  8,  8}}},	// movem_l_al
	{ 0x4cfa, 0xffff, {{ 16,  0}, { 30, 14}, { 16,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// movem_l
	{ 0x4cfb, 0xffff, {{ 18,  2}, { 33, 17}, { 18,  2}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}, { 11, 11}}},	// movem_l
	{ 0x4e57, 0xffff, {{ 16,  0}, { 25,  9}, { 16,  0}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}, {  5,  5}}},	// link_w
	{ 0x4e5f, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// unlk_l
	{ 0x4e70, 0xffff, {{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}}},	// reset
	{ 0x4e71, 0xffff, {{  4,  0}, {  7,  3}, {  4,  0}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}, {  2,  2}}},	// nop
	{ 0x4e72, 0xffff, {{  4,  0}, { 13,  9}, {  4,  0}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// stop
	{ 0x4e73, 0xffff, {{ 20,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// rte_l
	{ 0x4e73, 0xffff, {{255,255}, { 39, 15}, { 24,  0}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// rte_l
	{ 0x4e73, 0xffff, {{255,255}, {255,255}, {255,255}, { 20, 20}, { 20, 20}, { 20, 20}, { 20, 20}, { 20, 20}}},	// rte_l
	{ 0x4e74, 0xffff, {{255,255}, {255,255}, { 16,  0}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}}},	// rtd_l
	{ 0x4e75, 0xffff, {{ 16,  0}, { 15, -1}, { 16,  0}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}}},	// rts_l
	{ 0x4e76, 0xffff, {{  4,  0}, { 10,  6}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}}},	// trapv
	{ 0x4e77, 0xffff, {{ 20, 12}, { 22, 14}, { 20, 12}, { 14, 14}, { 14, 14}, { 14, 14}, { 14, 14}, { 14, 14}}},	// rtr_l
	{ 0x4e7a, 0xffff, {{255,255}, {255,255}, { 12,  4}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// movec_l
	{ 0x4e7a, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {255,255}, {  6,  6}, {255,255}}},	// movec_l
	{ 0x4e7a, 0xffff, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  6,  6}, {255,255}, {255,255}}},	// movec_l
	{ 0x4e7a, 0xffff, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {  6,  6}}},	// movec_l
	{ 0x4e7b, 0xffff, {{255,255}, {255,255}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// movec_l
	{ 0x4e7b, 0xffff, {{255,255}, {255,255}, {255,255}, { 12, 12}, {255,255}, {255,255}, { 12, 12}, {255,255}}},	// movec_l
	{ 0x4e7b, 0xffff, {{255,255}, {255,255}, {255,255}, {255,255}, { 12, 12}, {255,255}, {255,255}, {255,255}}},	// movec_l
	{ 0x4e7b, 0xffff, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, { 12, 12}, {255,255}, {255,255}}},	// movec_l
	{ 0x4e7b, 0xffff, {{255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}, { 12, 12}}},	// movec_l
	{ 0x4eb8, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jsr_l_aw
	{ 0x4eb9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jsr_l_al
	{ 0x4eba, 0xffff, {{ 18,  2}, { 24,  8}, { 18,  2}, {  5,  1}, {  5,  1}, {  5,  1}, {  5,  1}, {  0,  0}}},	// jsr_l_pcdi
	{ 0x4ebb, 0xffff, {{ 22,  2}, { 28,  8}, { 22,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  0,  0}}},	// jsr_l_pcix
	{ 0x4ef8, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jmp_l_aw
	{ 0x4ef9, 0xffff, {{ 12,  0}, { 15,  3}, { 12,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  4,  0}, {  0,  0}}},	// jmp_l_al
	{ 0x4efa, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {  5,  1}, {  5,  1}, {  5,  1}, {  5,  1}, {  0,  0}}},	// jmp_l_pcdi
	{ 0x4efb, 0xffff, {{ 14,  2}, { 17,  5}, { 14,  2}, {  7,  3}, {  7,  3}, {  7,  3}, {  7,  3}, {  0,  0}}},	// jmp_l_pcix
	{ 0x50df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// st_b_pi7
	{ 0x50e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// st_b_pd7
	{ 0x50f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// st_b_aw
	{ 0x50f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// st_b_al
	{ 0x50fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapt_w
	{ 0x50fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapt_l
	{ 0x50fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapt
	{ 0x51df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sf_b_pi7
	{ 0x51e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sf_b_pd7
	{ 0x51f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sf_b_aw
	{ 0x51f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sf_b_al
	{ 0x51fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapf_w
	{ 0x51fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapf_l
	{ 0x51fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapf
	{ 0x52df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// shi_b_pi7
	{ 0x52e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// shi_b_pd7
	{ 0x52f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// shi_b_aw
	{ 0x52f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// shi_b_al
	{ 0x52fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// traphi_w
	{ 0x52fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// traphi_l
	{ 0x52fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// traphi
	{ 0x53df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sls_b_pi7
	{ 0x53e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sls_b_pd7
	{ 0x53f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sls_b_aw
	{ 0x53f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sls_b_al
	{ 0x53fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapls_w
	{ 0x53fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapls_l
	{ 0x53fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapls
	{ 0x54df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scc_b_pi7
	{ 0x54e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scc_b_pd7
	{ 0x54f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scc_b_aw
	{ 0x54f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scc_b_al
	{ 0x54fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapcc_w
	{ 0x54fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapcc_l
	{ 0x54fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapcc
	{ 0x55df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scs_b_pi7
	{ 0x55e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// scs_b_pd7
	{ 0x55f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scs_b_aw
	{ 0x55f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// scs_b_al
	{ 0x55fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapcs_w
	{ 0x55fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapcs_l
	{ 0x55fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapcs
	{ 0x56df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sne_b_pi7
	{ 0x56e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sne_b_pd7
	{ 0x56f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sne_b_aw
	{ 0x56f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sne_b_al
	{ 0x56fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapne_w
	{ 0x56fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapne_l
	{ 0x56fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapne
	{ 0x57df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// seq_b_pi7
	{ 0x57e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// seq_b_pd7
	{ 0x57f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// seq_b_aw
	{ 0x57f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// seq_b_al
	{ 0x57fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapeq_w
	{ 0x57fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapeq_l
	{ 0x57fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapeq
	{ 0x58df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svc_b_pi7
	{ 0x58e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svc_b_pd7
	{ 0x58f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svc_b_aw
	{ 0x58f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svc_b_al
	{ 0x58fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapvc_w
	{ 0x58fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapvc_l
	{ 0x58fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapvc
	{ 0x59df, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svs_b_pi7
	{ 0x59e7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// svs_b_pd7
	{ 0x59f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svs_b_aw
	{ 0x59f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// svs_b_al
	{ 0x59fa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapvs_w
	{ 0x59fb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapvs_l
	{ 0x59fc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapvs
	{ 0x5adf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// spl_b_pi7
	{ 0x5ae7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// spl_b_pd7
	{ 0x5af8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// spl_b_aw
	{ 0x5af9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// spl_b_al
	{ 0x5afa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trappl_w
	{ 0x5afb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trappl_l
	{ 0x5afc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trappl
	{ 0x5bdf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// smi_b_pi7
	{ 0x5be7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// smi_b_pd7
	{ 0x5bf8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// smi_b_aw
	{ 0x5bf9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// smi_b_al
	{ 0x5bfa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapmi_w
	{ 0x5bfb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapmi_l
	{ 0x5bfc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapmi
	{ 0x5cdf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sge_b_pi7
	{ 0x5ce7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sge_b_pd7
	{ 0x5cf8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sge_b_aw
	{ 0x5cf9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sge_b_al
	{ 0x5cfa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapge_w
	{ 0x5cfb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapge_l
	{ 0x5cfc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapge
	{ 0x5ddf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// slt_b_pi7
	{ 0x5de7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// slt_b_pd7
	{ 0x5df8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// slt_b_aw
	{ 0x5df9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// slt_b_al
	{ 0x5dfa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// traplt_w
	{ 0x5dfb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// traplt_l
	{ 0x5dfc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// traplt
	{ 0x5edf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sgt_b_pi7
	{ 0x5ee7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sgt_b_pd7
	{ 0x5ef8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sgt_b_aw
	{ 0x5ef9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sgt_b_al
	{ 0x5efa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// trapgt_w
	{ 0x5efb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// trapgt_l
	{ 0x5efc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// trapgt
	{ 0x5fdf, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sle_b_pi7
	{ 0x5fe7, 0xffff, {{ 14,  2}, { 20,  8}, { 14,  2}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  6,  6}}},	// sle_b_pd7
	{ 0x5ff8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sle_b_aw
	{ 0x5ff9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 10,  6}, { 10,  6}, { 10,  6}, { 10,  6}, {  6,  6}}},	// sle_b_al
	{ 0x5ffa, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// traple_w
	{ 0x5ffb, 0xffff, {{255,255}, {255,255}, {255,255}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}, {  8,  8}}},	// traple_l
	{ 0x5ffc, 0xffff, {{255,255}, {255,255}, {255,255}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}, {  4,  4}}},	// traple
	{ 0x6000, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}}},	// bra_w
	{ 0x60ff, 0xffff, {{255,255}, {255,255}, {255,255}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}, { 10, 10}}},	// bra_l
	{ 0x6100, 0xffff, {{ 18,  2}, { 22,  6}, { 18,  2}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// bsr_w
	{ 0x61ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}, {  7,  7}}},	// bsr_l
	{ 0x6200, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bhi_w
	{ 0x62ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bhi_l
	{ 0x62ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bhi_l
	{ 0x6300, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bls_w
	{ 0x63ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bls_l
	{ 0x63ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bls_l
	{ 0x6400, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcc_w
	{ 0x64ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bcc_l
	{ 0x64ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcc_l
	{ 0x6500, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcs_w
	{ 0x65ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bcs_l
	{ 0x65ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bcs_l
	{ 0x6600, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bne_w
	{ 0x66ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bne_l
	{ 0x66ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bne_l
	{ 0x6700, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// beq_w
	{ 0x67ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// beq_l
	{ 0x67ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// beq_l
	{ 0x6800, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvc_w
	{ 0x68ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bvc_l
	{ 0x68ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvc_l
	{ 0x6900, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvs_w
	{ 0x69ff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bvs_l
	{ 0x69ff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bvs_l
	{ 0x6a00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bpl_w
	{ 0x6aff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bpl_l
	{ 0x6aff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bpl_l
	{ 0x6b00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bmi_w
	{ 0x6bff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bmi_l
	{ 0x6bff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bmi_l
	{ 0x6c00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bge_w
	{ 0x6cff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bge_l
	{ 0x6cff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bge_l
	{ 0x6d00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// blt_w
	{ 0x6dff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// blt_l
	{ 0x6dff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// blt_l
	{ 0x6e00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bgt_w
	{ 0x6eff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// bgt_l
	{ 0x6eff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// bgt_l
	{ 0x6f00, 0xffff, {{ 10,  2}, { 14,  6}, { 10,  2}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// ble_w
	{ 0x6fff, 0xffff, {{ 10,  2}, { 13,  5}, { 10,  2}, {255,255}, {255,255}, {255,255}, {255,255}, {255,255}}},	// ble_l
	{ 0x6fff, 0xffff, {{255,255}, {255,255}, {255,255}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}, {  6,  6}}},	// ble_l
	{ 0x8f0f, 0xffff, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// sbcd_b
	{ 0x8f4f, 0xffff, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// pack_w
	{ 0x8f8f, 0xffff, {{255,255}, {255,255}, {255,255}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}, { 13, 13}}},	// unpk_w
	{ 0x9f0f, 0xffff, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// subx_b
	{ 0xbf0f, 0xffff, {{ 12,  0}, { 18,  6}, { 12,  0}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}, {  9,  9}}},	// cmpm_b
	{ 0xcf0f, 0xffff, {{ 18,  2}, { 31, 15}, { 18,  2}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}, { 16, 16}}},	// abcd_b
	{ 0xdf0f, 0xffff, {{ 18,  2}, { 28, 12}, { 18,  2}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}, { 12, 12}}},	// addx_b
	{ 0xe0f8, 0xffff, {{ 16,  4}, { 22, 10}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// asr_w_aw
	{ 0xe0f9, 0xffff, {{ 20,  4}, { 26, 10}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// asr_w_al
	{ 0xe1f8, 0xffff, {{ 16,  4}, { 22, 10}, { 16,  4}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, {  6,  2}}},	// asl_w_aw
	{ 0xe1f9, 0xffff, {{ 20,  4}, { 26, 10}, { 20,  4}, { 10,  2}, { 10,  2}, { 10,  2}, { 10,  2}, {  6,  2}}},	// asl_w_al
	{ 0xe2f8, 0xffff, {{ 16,  4}, { 22, 10}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsr_w_aw
	{ 0xe2f9, 0xffff, {{ 20,  4}, { 26, 10}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsr_w_al
	{ 0xe3f8, 0xffff, {{ 16,  4}, { 22, 10}, { 16,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsl_w_aw
	{ 0xe3f9, 0xffff, {{ 20,  4}, { 26, 10}, { 20,  4}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// lsl_w_al
	{ 0xe4f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxr_w_aw
	{ 0xe4f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxr_w_al
	{ 0xe5f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxl_w_aw
	{ 0xe5f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, {  9,  5}, {  9,  5}, {  9,  5}, {  9,  5}, {  5,  5}}},	// roxl_w_al
	{ 0xe6f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// ror_w_aw
	{ 0xe6f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// ror_w_al
	{ 0xe7f8, 0xffff, {{ 16,  0}, { 22,  6}, { 16,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// rol_w_aw
	{ 0xe7f9, 0xffff, {{ 20,  0}, { 26,  6}, { 20,  0}, { 11,  7}, { 11,  7}, { 11,  7}, { 11,  7}, {  7,  7}}},	// rol_w_al
	{ 0xe8f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 13, 13}}},	// bftst_l_aw
	{ 0xe8f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 17, 13}, { 17, 13}, { 17, 13}, { 17, 13}, { 13, 13}}},	// bftst_l_al
	{ 0xe8fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 18, 14}, { 18, 14}, { 18, 14}, { 18, 14}, { 13, 13}}},	// bftst_l_pcdi
	{ 0xe8fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 13, 13}}},	// bftst_l_pcix
	{ 0xe9f8, 0xffff, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfextu_l_aw
	{ 0xe9f9, 0xffff, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfextu_l_al
	{ 0xe9fa, 0xffff, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 15, 15}}},	// bfextu_l_pcdi
	{ 0xe9fb, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 15, 15}}},	// bfextu_l_pcix
	{ 0xeaf8, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfchg_l_aw
	{ 0xeaf9, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfchg_l_al
	{ 0xebf8, 0xffff, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfexts_l_aw
	{ 0xebf9, 0xffff, {{255,255}, {255,255}, {255,255}, { 19, 15}, { 19, 15}, { 19, 15}, { 19, 15}, { 15, 15}}},	// bfexts_l_al
	{ 0xebfa, 0xffff, {{255,255}, {255,255}, {255,255}, { 20, 16}, { 20, 16}, { 20, 16}, { 20, 16}, { 15, 15}}},	// bfexts_l_pcdi
	{ 0xebfb, 0xffff, {{255,255}, {255,255}, {255,255}, { 22, 18}, { 22, 18}, { 22, 18}, { 22, 18}, { 15, 15}}},	// bfexts_l_pcix
	{ 0xecf8, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfclr_l_aw
	{ 0xecf9, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfclr_l_al
	{ 0xedf8, 0xffff, {{255,255}, {255,255}, {255,255}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 28, 28}}},	// bfffo_l_aw
	{ 0xedf9, 0xffff, {{255,255}, {255,255}, {255,255}, { 32, 28}, { 32, 28}, { 32, 28}, { 32, 28}, { 28, 28}}},	// bfffo_l_al
	{ 0xedfa, 0xffff, {{255,255}, {255,255}, {255,255}, { 33, 29}, { 33, 29}, { 33, 29}, { 33, 29}, { 28, 28}}},	// bfffo_l_pcdi
	{ 0xedfb, 0xffff, {{255,255}, {255,255}, {255,255}, { 35, 31}, { 35, 31}, { 35, 31}, { 35, 31}, { 28, 28}}},	// bfffo_l_pcix
	{ 0xeef8, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfset_l_aw
	{ 0xeef9, 0xffff, {{255,255}, {255,255}, {255,255}, { 24, 20}, { 24, 20}, { 24, 20}, { 24, 20}, { 20, 20}}},	// bfset_l_al
	{ 0xeff8, 0xffff, {{255,255}, {255,255}, {255,255}, { 21, 17}, { 21, 17}, { 21, 17}, { 21, 17}, { 17, 17}}},	// bfins_l_aw
	{ 0xeff9, 0xffff, {{255,255}, {255,255}, {255,255}, { 21, 17}, { 21, 17}, { 21, 17}, { 21, 17}, { 17, 17}}},	// bfins_l_al
	{ 0, 0, {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}}
};

