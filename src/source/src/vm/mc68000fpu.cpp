/** @file mc68000fpu.cpp

	Skelton for retropc emulator

	@par Origin MAME 0.239,  0.152 / Musashi
	@author Sasaji
	@date   2022.01.01-

	@brief [ MC68000 FPU ]
*/

#include "vm_defs.h"
#include "mc68000_consts.h"

#ifdef USE_MC68000FPU

#include <math.h>
#include <stdio.h>
#include "mc68000.h"
#include "../logging.h"
#include "softfloat/softfloat.h"

#define DOUBLE_INFINITY                 U64(0x7ff0000000000000)
#define DOUBLE_EXPONENT                 U64(0x7ff0000000000000)
#define DOUBLE_MANTISSA                 U64(0x000fffffffffffff)

extern flag floatx80_is_nan( floatx80 a );

// masks for packed dwords, positive k-factor
const uint32_t MC68000BASE::pkmask2[18] =
{
	0xffffffff, 0, 0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t MC68000BASE::pkmask3[18] =
{
	0xffffffff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
};

double MC68000BASE::fx80_to_double(floatx80 fx)
{
	uint64_t d;
	double *foo;

	foo = (double *)&d;

	d = floatx80_to_float64(fx);

	return *foo;
}

floatx80 MC68000BASE::double_to_fx80(double in)
{
	uint64_t *d;

	d = (uint64_t *)&in;

	return float64_to_floatx80(*d);
}

floatx80 MC68000BASE::load_extended_float80(uint32_t ea)
{
	uint32_t d1,d2;
	uint16_t d3;
	floatx80 fp;

	d3 = read_16(ea);
	d1 = read_32(ea+4);
	d2 = read_32(ea+8);

	fp.high = d3;
	fp.low = ((uint64_t)d1<<32) | (d2 & 0xffffffff);

	return fp;
}

void MC68000BASE::store_extended_float80(uint32_t ea, floatx80 fpr)
{
	write_16(ea+0, fpr.high);
	write_16(ea+2, 0);
	write_32(ea+4, (fpr.low>>32)&0xffffffff);
	write_32(ea+8, fpr.low&0xffffffff);
}

floatx80 MC68000BASE::load_pack_float80(uint32_t ea)
{
	uint32_t dw1, dw2, dw3;
	floatx80 result;
	double tmp;
	char str[128], *ch;

	dw1 = read_32(ea);
	dw2 = read_32(ea+4);
	dw3 = read_32(ea+8);

	ch = &str[0];
	if (dw1 & 0x80000000)   // mantissa sign
	{
		*ch++ = '-';
	}
	*ch++ = (char)((dw1 & 0xf) + '0');
	*ch++ = '.';
	*ch++ = (char)(((dw2 >> 28) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 16) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 12) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 8)  & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 4)  & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 0)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 28) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 16) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 12) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 8)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 4)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 0)  & 0xf) + '0');
	*ch++ = 'E';
	if (dw1 & 0x40000000)   // exponent sign
	{
		*ch++ = '-';
	}
	*ch++ = (char)(((dw1 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw1 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw1 >> 16) & 0xf) + '0');
	*ch = '\0';

	sscanf(str, "%le", &tmp);

	result = double_to_fx80(tmp);

	return result;
}

void MC68000BASE::store_pack_float80(uint32_t ea, int k, floatx80 fpr)
{
	uint32_t dw1, dw2, dw3;
	char str[128], *ch;
	int i, j, exp;

	dw1 = dw2 = dw3 = 0;
	ch = &str[0];

	sprintf(str, "%.16e", fx80_to_double(fpr));

	if (*ch == '-')
	{
		ch++;
		dw1 = 0x80000000;
	}

	if (*ch == '+')
	{
		ch++;
	}

	dw1 |= (*ch++ - '0');

	if (*ch == '.')
	{
		ch++;
	}

	// handle negative k-factor here
	if ((k <= 0) && (k >= -13))
	{
		exp = 0;
		for (i = 0; i < 3; i++)
		{
			if (ch[18+i] >= '0' && ch[18+i] <= '9')
			{
				exp = (exp << 4) | (ch[18+i] - '0');
			}
		}

		if (ch[17] == '-')
		{
			exp = -exp;
		}

		k = -k;
		// last digit is (k + exponent - 1)
		k += (exp - 1);

		// round up the last significant mantissa digit
		if (ch[k+1] >= '5')
		{
			ch[k]++;
		}

		// zero out the rest of the mantissa digits
		for (j = (k+1); j < 16; j++)
		{
			ch[j] = '0';
		}

		// now zero out K to avoid tripping the positive K detection below
		k = 0;
	}

	// crack 8 digits of the mantissa
	for (i = 0; i < 8; i++)
	{
		dw2 <<= 4;
		if (*ch >= '0' && *ch <= '9')
		{
			dw2 |= *ch++ - '0';
		}
	}

	// next 8 digits of the mantissa
	for (i = 0; i < 8; i++)
	{
		dw3 <<= 4;
		if (*ch >= '0' && *ch <= '9')
		dw3 |= *ch++ - '0';
	}

	// handle masking if k is positive
	if (k >= 1)
	{
		if (k <= 17)
		{
			dw2 &= pkmask2[k];
			dw3 &= pkmask3[k];
		}
		else
		{
			dw2 &= pkmask2[17];
			dw3 &= pkmask3[17];
//          m_fpcr |=  (need to set OPERR bit)
		}
	}

	// finally, crack the exponent
	if (*ch == 'e' || *ch == 'E')
	{
		ch++;
		if (*ch == '-')
		{
			ch++;
			dw1 |= 0x40000000;
		}

		if (*ch == '+')
		{
			ch++;
		}

		j = 0;
		for (i = 0; i < 3; i++)
		{
			if (*ch >= '0' && *ch <= '9')
			{
				j = (j << 4) | (*ch++ - '0');
			}
		}

		dw1 |= (j << 16);
	}

	write_32(ea, dw1);
	write_32(ea+4, dw2);
	write_32(ea+8, dw3);
}

floatx80 MC68000BASE::propagateFloatx80NaNOneArg(floatx80 a)
{
	if (floatx80_is_signaling_nan(a))
		float_raise(float_flag_invalid);

	a.low |= 0xC000000000000000U;

	return a;
}

void MC68000BASE::normalizeFloatx80Subnormal(uint64_t aSig, int32_t *zExpPtr, uint64_t *zSigPtr)
{
	int shiftCount = countLeadingZeros64(aSig);
	*zSigPtr = aSig << shiftCount;
	*zExpPtr = 1 - shiftCount;
}

floatx80 MC68000BASE::getman(floatx80 src)
{
	const flag sign = (src.high >> 15);
	int32_t exp = (src.high & 0x7fff);
	uint64_t signific = src.low;

	if (exp == 0x7fff)
	{
		if ((uint64_t)(signific << 1))
		{
			return propagateFloatx80NaNOneArg(src);
		}
		else
		{
			return packFloatx80(0, 0xffff, 0xffffffffffffffffU);
		}
	}

	if (exp == 0)
	{
		if (signific == 0)
		{
			return packFloatx80(sign, 0, 0);
		}
		normalizeFloatx80Subnormal(signific, &exp, &signific);
	}

	return packFloatx80(sign, 0x3fff, signific);
}

void MC68000BASE::SET_CONDITION_CODES(floatx80 reg)
{
//  uint64_t *regi;

//  regi = (uint64_t *)&reg;

	m_fpsr &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.high & 0x8000)
	{
		m_fpsr |= FPCC_N;
	}

	// zero flag
	if (((reg.high & 0x7fff) == 0) && ((reg.low<<1) == 0))
	{
		m_fpsr |= FPCC_Z;
	}

	// infinity flag
	if (((reg.high & 0x7fff) == 0x7fff) && ((reg.low<<1) == 0))
	{
		m_fpsr |= FPCC_I;
	}

	// NaN flag
	if (floatx80_is_nan(reg))
	{
		m_fpsr |= FPCC_NAN;
	}
}

int MC68000BASE::TEST_CONDITION(int condition)
{
	int n = (m_fpsr & FPCC_N) != 0;
	int z = (m_fpsr & FPCC_Z) != 0;
	int nan = (m_fpsr & FPCC_NAN) != 0;
	int r = 0;
	switch (condition)
	{
		case 0x10:
		case 0x00:      return 0;                   // False

		case 0x11:
		case 0x01:      return (z);                 // Equal

		case 0x12:
		case 0x02:      return (!(nan || z || n));          // Greater Than

		case 0x13:
		case 0x03:      return (z || !(nan || n));          // Greater or Equal

		case 0x14:
		case 0x04:      return (n && !(nan || z));          // Less Than

		case 0x15:
		case 0x05:      return (z || (n && !nan));          // Less Than or Equal

		case 0x16:
		case 0x06:      return !nan && !z;

		case 0x17:
		case 0x07:      return !nan;

		case 0x18:
		case 0x08:      return nan;

		case 0x19:
		case 0x09:      return nan || z;

		case 0x1a:
		case 0x0a:      return (nan || !(n || z));          // Not Less Than or Equal

		case 0x1b:
		case 0x0b:      return (nan || z || !n);            // Not Less Than

		case 0x1c:
		case 0x0c:      return (nan || (n && !z));          // Not Greater or Equal Than

		case 0x1d:
		case 0x0d:      return (nan || z || n);             // Not Greater Than

		case 0x1e:
		case 0x0e:      return (!z);                    // Not Equal

		case 0x1f:
		case 0x0f:      return 1;                   // True

		default:        logging->out_logf(LOG_ERROR, "M68kFPU: test_condition: unhandled condition %02X\n", condition);
	}

	return r;
}



floatx80 MC68000BASE::READ_EA_FPE(int mode, int reg, uint32 di_mode_ea)
{
	floatx80 fpr;

	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			fpr = load_extended_float80(ea);
			break;
		}

		case 3:     // (An)+
		{
			uint32_t ea = REG_A()[reg];
			REG_A()[reg] += 12;
			fpr = load_extended_float80(ea);
			break;
		}
		case 4:     // -(An)
		{
			uint32_t ea = REG_A()[reg]-12;
			REG_A()[reg] -= 12;
			fpr = load_extended_float80(ea);
			break;
		}
		case 5:     // (d16, An)
		{
			fpr = load_extended_float80(di_mode_ea);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			fpr = load_extended_float80(di_mode_ea);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 2: // (d16, PC)
					{
						uint32_t ea = EA_PCDI_32();
						fpr = load_extended_float80(ea);
					}
					break;

				case 3: // (d16,PC,Dx.w)
					{
						uint32_t ea = EA_PCIX_32();
						fpr = load_extended_float80(ea);
					}
					break;

				default:
					logging->out_logf(LOG_ERROR, "M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC());
					break;
			}
		}
		break;

		default:    logging->out_logf(LOG_ERROR, "M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC()); break;
	}

	return fpr;
}

floatx80 MC68000BASE::READ_EA_PACK(int ea)
{
	floatx80 fpr;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea = REG_A()[reg];
			fpr = load_pack_float80(ea);
			break;
		}

		case 3:     // (An)+
		{
			uint32_t ea = REG_A()[reg];
			REG_A()[reg] += 12;
			fpr = load_pack_float80(ea);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 3: // (d16,PC,Dx.w)
					{
						uint32_t ea = EA_PCIX_32();
						fpr = load_pack_float80(ea);
					}
					break;

				default:
					logging->out_logf(LOG_ERROR, "M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC());
					break;
			}
		}
		break;

		default:    logging->out_logf(LOG_ERROR, "M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC()); break;
	}

	return fpr;
}


void MC68000BASE::WRITE_EA_FPE(int mode, int reg, floatx80 fpr, uint32 di_mode_ea)
{
	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			break;
		}

		case 3:     // (An)+
		{
			uint32_t ea;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			uint32_t ea;
			REG_A()[reg] -= 12;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			break;
		}

		case 5:     // (d16,An)
		{
			// EA_AY_DI_32() should not be done here because fmovem would increase
			// PC each time, reading incorrect displacement & advancing PC too much.
			store_extended_float80(di_mode_ea, fpr);
			break;
		}

		case 6: // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			store_extended_float80(ea, fpr);
			break;
		}

#if 0
		case 7:
		{
			switch (reg)
			{
				default:    logging->out_logf(LOG_ERROR, "M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X", mode, reg, REG_PC());
			}
		}
#endif
		default:    logging->out_logf(LOG_ERROR, "M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X", mode, reg, REG_PC());
	}
}

void MC68000BASE::WRITE_EA_PACK(int ea, int k, floatx80 fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			uint32_t ea;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			break;
		}

		case 3:     // (An)+
		{
			uint32_t ea;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			uint32_t ea;
			REG_A()[reg] -= 12;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			break;
		}

#if 0
		case 7:
		{
			switch (reg)
			{
				default:    logging->out_logf(LOG_ERROR, "M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X", mode, reg, REG_PC());
			}
		}
#endif
		default:    logging->out_logf(LOG_ERROR, "M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X", mode, reg, REG_PC());
	}
}

void MC68000BASE::fpgen_rm_reg(uint16_t w2)
{
	const int ea = m_ir & 0x3f;
	const int rm = (w2 >> 14) & 0x1;
	const int src = (w2 >> 10) & 0x7;
	const int dst = (w2 >>  7) & 0x7;
	const int opmode = w2 & 0x7f;
	floatx80 source;

	// fmovecr #$f, fp0 f200 5c0f

	if (rm)
	{
		switch (src)
		{
			case 0:     // Long-Word Integer
			{
				int32_t d = READ_EA_32(ea);
				source = int32_to_floatx80(d);
				break;
			}
			case 1:     // Single-precision Real
			{
				uint32_t d = READ_EA_32(ea);
				source = float32_to_floatx80(d);
				break;
			}
			case 2:     // Extended-precision Real
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				uint32 di_mode_ea = imode == 5 ? (REG_A()[reg] + MAKE_INT_16(read_imm_16())) : 0;
				source = READ_EA_FPE(imode, reg, di_mode_ea);
				break;
			}
			case 3:     // Packed-decimal Real
			{
				source = READ_EA_PACK(ea);
				break;
			}
			case 4:     // Word Integer
			{
				int16_t d = READ_EA_16(ea);
				source = int32_to_floatx80((int32_t)d);
				break;
			}
			case 5:     // Double-precision Real
			{
				uint64_t d = READ_EA_64(ea);

				source = float64_to_floatx80(d);
				break;
			}
			case 6:     // Byte Integer
			{
				int8_t d = READ_EA_8(ea);
				source = int32_to_floatx80((int32_t)d);
				break;
			}
			case 7:     // FMOVECR load from constant ROM
			{
				switch (w2 & 0x7f)
				{
					case 0x0:   // Pi
						source.high = 0x4000;
						source.low = U64(0xc90fdaa22168c235);
						break;

					case 0xb:   // log10(2)
						source.high = 0x3ffd;
						source.low = U64(0x9a209a84fbcff798);
						break;

					case 0xc:   // e
						source.high = 0x4000;
						source.low = U64(0xadf85458a2bb4a9b);
						break;

					case 0xd:   // log2(e)
						source.high = 0x3fff;
						source.low = U64(0xb8aa3b295c17f0bc);
						break;

					case 0xe:   // log10(e)
						source.high = 0x3ffd;
						source.low = U64(0xde5bd8a937287195);
						break;

					case 0xf:   // 0.0
						source = int32_to_floatx80((int32_t)0);
						break;

					case 0x30:  // ln(2)
						source.high = 0x3ffe;
						source.low = U64(0xb17217f7d1cf79ac);
						break;

					case 0x31:  // ln(10)
						source.high = 0x4000;
						source.low = U64(0x935d8dddaaa8ac17);
						break;

					case 0x32:  // 1 (or 100?  manuals are unclear, but 1 would make more sense)
						source = int32_to_floatx80((int32_t)1);
						break;

					case 0x33:  // 10^1
						source = int32_to_floatx80((int32_t)10);
						break;

					case 0x34:  // 10^2
						source = int32_to_floatx80((int32_t)10*10);
						break;

					case 0x35:  // 10^4
						source = int32_to_floatx80((int32_t)1000*10);
						break;

					case 0x36:  // 1.0e8
						source = int32_to_floatx80((int32_t)10000000*10);
						break;

					case 0x37:  // 1.0e16 - can't get the right precision from int32_t so go "direct" with constants from h/w
						source.high = 0x4034;
						source.low = U64(0x8e1bc9bf04000000);
						break;

					case 0x38:  // 1.0e32
						source.high = 0x4069;
						source.low = U64(0x9dc5ada82b70b59e);
						break;

					case 0x39:  // 1.0e64
						source.high = 0x40d3;
						source.low = U64(0xc2781f49ffcfa6d5);
						break;

					case 0x3a:  // 1.0e128
						source.high = 0x41a8;
						source.low = U64(0x93ba47c980e98ce0);
						break;

					case 0x3b:  // 1.0e256
						source.high = 0x4351;
						source.low = U64(0xaa7eebfb9df9de8e);
						break;

					case 0x3c:  // 1.0e512
						source.high = 0x46a3;
						source.low = U64(0xe319a0aea60e91c7);
						break;

					case 0x3d:  // 1.0e1024
						source.high = 0x4d48;
						source.low = U64(0xc976758681750c17);
						break;

					case 0x3e:  // 1.0e2048
						source.high = 0x5a92;
						source.low = U64(0x9e8b3b5dc53d5de5);
						break;

					case 0x3f:  // 1.0e4096
						source.high = 0x7525;
						source.low = U64(0xc46052028a20979b);
						break;

					default:
						logging->out_logf(LOG_ERROR, "fmove_rm_reg: unknown constant ROM offset %x at %08x\n", w2&0x7f, REG_PC()-4);
						break;
				}

				// handle it right here, the usual opmode bits aren't valid in the FMOVECR case
				m_fpr[dst] = source;
				SET_CONDITION_CODES(m_fpr[dst]);
				SET_ICOUNT(4);
				return;
			}
			default:    logging->out_logf(LOG_ERROR, "fmove_rm_reg: invalid source specifier %x at %08X\n", src, REG_PC()-4);
		}
	}
	else
	{
		source = m_fpr[src];
	}



	switch (opmode)
	{
		case 0x00:      // FMOVE
		{
			m_fpr[dst] = source;
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(4);
			break;
		}
		case 0x01:      // FINT
		{
			int32_t temp;
			temp = floatx80_to_int32(source);
			m_fpr[dst] = int32_to_floatx80(temp);
			SET_CONDITION_CODES(m_fpr[dst]);
			break;
		}
		case 0x03:      // FINTRZ
		{
			int32_t temp;
			temp = floatx80_to_int32_round_to_zero(source);
			m_fpr[dst] = int32_to_floatx80(temp);
			SET_CONDITION_CODES(m_fpr[dst]);
			break;
		}
		case 0x04:      // FSQRT
		{
			m_fpr[dst] = floatx80_sqrt(source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(109);
			break;
		}
		case 0x06:      // FLOGNP1
		{
			m_fpr[dst] = floatx80_flognp1(source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(594); // for MC68881
			break;
		}
		case 0x0a:      // FATAN
		{
			m_fpr[dst] = floatx80_fatan(source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(426); // for MC68881
			break;
		}
		case 0x0e:      // FSIN
		{
			m_fpr[dst] = source;
			floatx80_fsin(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(75);
			break;
		}
		case 0x0f:      // FTAN
		{
			m_fpr[dst] = source;
			floatx80_ftan(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(75);
			break;
		}
		case 0x14:      // FLOGN
		{
			m_fpr[dst] = floatx80_flogn (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(548); // for MC68881
			break;
		}
		case 0x15:      // FLOG10
		{
			m_fpr[dst] = floatx80_flog10 (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(604); // for MC68881
			break;
		}
		case 0x16:      // FLOG2
		{
			m_fpr[dst] = floatx80_flog2 (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(604); // for MC68881
			break;
		}
		case 0x18:      // FABS
		{
			m_fpr[dst] = source;
			m_fpr[dst].high &= 0x7fff;
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(3);
			break;
		}
		case 0x1a:      // FNEG
		{
			m_fpr[dst] = source;
			m_fpr[dst].high ^= 0x8000;
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(3);
			break;
		}
		case 0x1d:      // FCOS
		{
			m_fpr[dst] = source;
			floatx80_fcos(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(75);
			break;
		}
		case 0x1e:      // FGETEXP
		{
			int16_t temp2;

			temp2 = source.high;    // get the exponent
			temp2 -= 0x3fff;    // take off the bias
			m_fpr[dst] = double_to_fx80((double)temp2);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(6);
			break;
		}
		case 0x1f:      // FGETMAN
		{
			m_fpr[dst] = getman(source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(31);
			break;
		}
		case 0x60:      // FSDIVS
		case 0x20:      // FDIV
		{
			m_fpr[dst] = floatx80_div(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(43);
			break;
		}
		case 0x21:      // FMOD
		{
			int8_t const mode = float_rounding_mode;
			float_rounding_mode = float_round_to_zero;
			m_fpr[dst] = floatx80_rem(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			float_rounding_mode = mode;
			SET_ICOUNT(43);   // guess
			break;
		}
		case 0x22:      // FADD
		{
			m_fpr[dst] = floatx80_add(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(9);
			break;
		}
		case 0x63:      // FSMULS (JFF)
		case 0x23:      // FMUL
		{
			m_fpr[dst] = floatx80_mul(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(11);
			break;
		}
		case 0x24:      // FSGLDIV
		{
			float32 a = floatx80_to_float32( m_fpr[dst] );
			float32 b = floatx80_to_float32( source );
			m_fpr[dst] = float32_to_floatx80( float32_div(a, b) );
			SET_ICOUNT(43); //  // ? (value is from FDIV)
			break;
		}
		case 0x25:      // FREM
		{
			int8_t const mode = float_rounding_mode;
			float_rounding_mode = float_round_nearest_even;
			m_fpr[dst] = floatx80_rem(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			float_rounding_mode = mode;
			SET_ICOUNT(43);   // guess
			break;
		}
		case 0x26:      // FSCALE
		{
			m_fpr[dst] = floatx80_scale(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(46);   // (better?) guess
			break;
		}
		case 0x27:      // FSGLMUL
		{
			float32 a = floatx80_to_float32( m_fpr[dst] );
			float32 b = floatx80_to_float32( source );
			m_fpr[dst] = float32_to_floatx80( float32_mul(a, b) );
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(11); // ? (value is from FMUL)
			break;
		}
		case 0x28:      // FSUB
		{
			m_fpr[dst] = floatx80_sub(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			SET_ICOUNT(9);
			break;
		}
		case 0x30:      // FSINCOS
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		{
			m_fpr[dst] = source;
			floatx80_fsin(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);    // condition codes are set for the sine result

			m_fpr[(w2 & 0x7)] = source;
			floatx80_fcos(m_fpr[(w2 & 0x7)]);
			SET_ICOUNT(451);
			break;
		}
		case 0x38:      // FCMP
		{
			floatx80 res;
			res = floatx80_sub(m_fpr[dst], source);
			SET_CONDITION_CODES(res);
			SET_ICOUNT(7);
			break;
		}
		case 0x3a:      // FTST
		{
			floatx80 res;
			res = source;
			SET_CONDITION_CODES(res);
			SET_ICOUNT(7);
			break;
		}

		default:    logging->out_logf(LOG_ERROR, "fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, REG_PPC());
	}
}

void MC68000BASE::fmove_reg_mem(uint16_t w2)
{
	int ea = m_ir & 0x3f;
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	int k = (w2 & 0x7f);

	switch (dst)
	{
		case 0:     // Long-Word Integer
		{
			int32_t d = (int32_t)floatx80_to_int32(m_fpr[src]);
			WRITE_EA_32(ea, d);
			break;
		}
		case 1:     // Single-precision Real
		{
			uint32_t d = floatx80_to_float32(m_fpr[src]);
			WRITE_EA_32(ea, d);
			break;
		}
		case 2:     // Extended-precision Real
		{
			int mode = (ea >> 3) & 0x7;
			int reg = (ea & 0x7);
			uint32_t di_mode_ea = mode == 5 ? (REG_A()[reg] + MAKE_INT_16(read_imm_16())) : 0;

			WRITE_EA_FPE(mode, reg, m_fpr[src], di_mode_ea);
			break;
		}
		case 3:     // Packed-decimal Real with Static K-factor
		{
			// sign-extend k
			k = (k & 0x40) ? (k | 0xffffff80) : (k & 0x7f);
			WRITE_EA_PACK(ea, k, m_fpr[src]);
			break;
		}
		case 4:     // Word Integer
		{
			int32_t value = floatx80_to_int32(m_fpr[src]);
			if (value > 0x7fff || value < -0x8000 )
			{
				m_fpsr |= FPES_OE | FPAE_IOP;
			}
			WRITE_EA_16(ea, (int16_t)value);
			break;
		}
		case 5:     // Double-precision Real
		{
			uint64_t d;

			d = floatx80_to_float64(m_fpr[src]);

			WRITE_EA_64(ea, d);
			break;
		}
		case 6:     // Byte Integer
		{
			int32 value = floatx80_to_int32(m_fpr[src]);
			if (value > 127 || value < -128)
			{
				m_fpsr |= FPES_OE | FPAE_IOP;
			}
			WRITE_EA_8(ea, (int8_t) value);
			break;
		}
		case 7:     // Packed-decimal Real with Dynamic K-factor
		{
			WRITE_EA_PACK(ea, REG_D()[k>>4], m_fpr[src]);
			break;
		}
	}

	SET_ICOUNT(12);
}

void MC68000BASE::fmove_fpcr(uint16_t w2)
{
	int ea = m_ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int regsel = (w2 >> 10) & 0x7;
	int mode = (ea >> 3) & 0x7;

	if ((mode == 5) || (mode == 6))
	{
		uint32_t address = 0xffffffff;    // force a bus error if this doesn't get assigned

		if (mode == 5)
		{
			address = EA_AY_DI_32();
		}
		else if (mode == 6)
		{
			address = EA_AY_IX_32();
		}

		if (dir)    // From system control reg to <ea>
		{
			if (regsel & 4) { write_32(address, m_fpcr); address += 4; }
			if (regsel & 2) { write_32(address, m_fpsr); address += 4; }
			if (regsel & 1) { write_32(address, m_fpiar); address += 4; }
		}
		else        // From <ea> to system control reg
		{
			if (regsel & 4) { m_fpcr = read_32(address); address += 4; }
			if (regsel & 2) { m_fpsr = read_32(address); address += 4; }
			if (regsel & 1) { m_fpiar = read_32(address); address += 4; }
		}
	}
	else
	{
		if (dir)    // From system control reg to <ea>
		{
			if (regsel & 4) WRITE_EA_32(ea, m_fpcr);
			if (regsel & 2) WRITE_EA_32(ea, m_fpsr);
			if (regsel & 1) WRITE_EA_32(ea, m_fpiar);
		}
		else        // From <ea> to system control reg
		{
			if (regsel & 4) m_fpcr = READ_EA_32(ea);
			if (regsel & 2) m_fpsr = READ_EA_32(ea);
			if (regsel & 1) m_fpiar = READ_EA_32(ea);
		}
	}

#if 0
	// FIXME: (2011-12-18 ost)
	// rounding_mode and rounding_precision of softfloat.c should be set according to current fpcr
	// but:  with this code on Apollo the following programs in /systest/fptest will fail:
	// 1. Single Precision Whetstone will return wrong results never the less
	// 2. Vector Test will fault with 00040004: reference to illegal address

	if ((regsel & 4) && dir == 0)
	{
		int rnd = (m_fpcr >> 4) & 3;
		int prec = (m_fpcr >> 6) & 3;

		logerror("m68k_fpsp:fmove_fpcr fpcr=%04x prec=%d rnd=%d\n", m_fpcr, prec, rnd);

#ifdef FLOATX80
		switch (prec)
		{
		case 0: // Extend (X)
			floatx80_rounding_precision = 80;
			break;
		case 1: // Single (S)
			floatx80_rounding_precision = 32;
			break;
		case 2: // Double (D)
			floatx80_rounding_precision = 64;
			break;
		case 3: // Undefined
			floatx80_rounding_precision = 80;
			break;
		}
#endif

		switch (rnd)
		{
		case 0: // To Nearest (RN)
			float_rounding_mode = float_round_nearest_even;
			break;
		case 1: // To Zero (RZ)
			float_rounding_mode = float_round_to_zero;
			break;
		case 2: // To Minus Infinitiy (RM)
			float_rounding_mode = float_round_down;
			break;
		case 3: // To Plus Infinitiy (RP)
			float_rounding_mode = float_round_up;
			break;
		}
	}
#endif

	SET_ICOUNT(10);
}

void MC68000BASE::fmovem(uint16_t w2)
{
	int i;
	int ea = m_ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int mode = (w2 >> 11) & 0x3;
	int reglist = w2 & 0xff;

	if (dir)    // From FP regs to mem
	{
		switch (mode)
		{
			case 1: // Dynamic register list, postincrement or control addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];
				// [[fallthrough]];
			case 0:     // Static register list, predecrement or control addressing mode
			{
				// the "di_mode_ea" parameter kludge is required here else WRITE_EA_FPE would have
				// to call EA_AY_DI_32() (that advances PC & reads displacement) each time
				// when the proper behaviour is 1) read once, 2) increment ea for each matching register
				// this forces to pre-read the mode (named "imode") so we can decide to read displacement, only once
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = imode == 5;
				uint32_t di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(read_imm_16())) : 0;

				if (reglist)
				{
					for (i=0; i < 8; i++)
					{
						if (reglist & (1 << i))
						{
							WRITE_EA_FPE(imode, reg, m_fpr[i], di_mode_ea);
							if (di_mode)
							{
								di_mode_ea += 12;
							}

							SET_ICOUNT(2);
						}
					}
				}
				else if (imode == 6)
					// advance PC if the register list is empty
					EA_AY_IX_32();
				else if (imode == 7)
					logging->out_logf(LOG_ERROR, "m68881: fmovem addressing mode %d unimplemented at 0x%08x\n", imode, m_pc - 4);
				break;
			}

			case 3: // Dynamic register list, postincrement or control addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];
				//[[fallthrough]];
			case 2:     // Static register list, postdecrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = (imode == 5);
				uint32 di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(read_imm_16())) : 0;

				if (reglist)
				{
					for (i=0; i < 8; i++)
					{
						if (reglist & (1 << i))
						{
							WRITE_EA_FPE(imode, reg, m_fpr[7 - i], di_mode_ea);
							if (di_mode)
							{
								di_mode_ea += 12;
							}

							SET_ICOUNT(2);
						}
					}
				}
				else if (imode == 6)
					// advance PC if the register list is empty
					EA_AY_IX_32();
				else if (imode == 7)
					logging->out_logf(LOG_ERROR, "m68881: fmovem addressing mode %d unimplemented at 0x%08x\n", imode, m_pc - 4);
				break;
			}

			default:    logging->out_logf(LOG_ERROR, "M680x0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC()-4);
		}
	}
	else        // From mem to FP regs
	{
		switch (mode)
		{
			case 3: // Dynamic register list, predecrement addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];
				//[[fallthrough]];
			case 2:     // Static register list, postincrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = (imode == 5);
				uint32 di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(read_imm_16())) : 0;

				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						m_fpr[7 - i] = READ_EA_FPE(imode, reg, di_mode_ea);
						if (di_mode)
						{
							di_mode_ea += 12;
						}
						SET_ICOUNT(2);
					}
				}
				break;
			}

			default:    logging->out_logf(LOG_ERROR, "M680x0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC()-4);
		}
	}
}

void MC68000BASE::fscc()
{
	const int mode = (m_ir & 0x38) >> 3;
	const int condition = OPER_I_16() & 0x3f;
	const int v = (TEST_CONDITION(condition) ? 0xff : 0x00);

	switch (mode)
	{
		case 0: // Dx (handled specially because it only changes the low byte of Dx)
			{
				const int reg = m_ir & 7;
				REG_D()[reg] = (REG_D()[reg] & 0xffffff00) | v;
			}
			break;

		default:
			WRITE_EA_8(m_ir & 0x3f, v);
			break;
	}

	SET_ICOUNT(7); // ???
}

void MC68000BASE::fbcc16()
{
	int32_t offset;
	int condition = m_ir & 0x3f;

	offset = (int16_t)(OPER_I_16());

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_16(offset-2);
	}

	SET_ICOUNT(7);
}

void MC68000BASE::fbcc32()
{
	int32_t offset;
	int condition = m_ir & 0x3f;

	offset = OPER_I_32();

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		trace_t0();              /* auto-disable (see m68kcpu.h) */
		branch_32(offset-4);
	}

	SET_ICOUNT(7);
}


void MC68000BASE::m68040_fpu_op0()
{
	m_fpu_just_reset = 0;

	switch ((m_ir >> 6) & 0x3)
	{
		case 0:
		{
			uint16_t w2 = OPER_I_16();
			switch ((w2 >> 13) & 0x7)
			{
				case 0x0:   // FPU ALU FP, FP
				case 0x2:   // FPU ALU ea, FP
				{
					fpgen_rm_reg(w2);
					break;
				}

				case 0x3:   // FMOVE FP, ea
				{
					fmove_reg_mem(w2);
					break;
				}

				case 0x4:   // FMOVEM ea, FPCR
				case 0x5:   // FMOVEM FPCR, ea
				{
					fmove_fpcr(w2);
					break;
				}

				case 0x6:   // FMOVEM ea, list
				case 0x7:   // FMOVEM list, ea
				{
					fmovem(w2);
					break;
				}

				default:    logging->out_logf(LOG_ERROR, "M68kFPU: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, REG_PC()-4);
			}
			break;
		}

		case 1:     // FBcc disp16
		{
			switch ((m_ir >> 3) & 0x7) {
			case 1: // FDBcc
				// TODO:
				break;
			default: // FScc (?)
				fscc();
				return;
			}
			logging->out_logf(LOG_ERROR, "M68kFPU: unimplemented main op %d with mode %d at %08X\n", (m_ir >> 6) & 0x3, (m_ir >> 3) & 0x7, REG_PPC());
		}

		case 2:     // FBcc disp16
		{
			fbcc16();
			break;
		}
		case 3:     // FBcc disp32
		{
			fbcc32();
			break;
		}

		default:    logging->out_logf(LOG_ERROR, "M68kFPU: unimplemented main op %d\n", (m_ir >> 6)   & 0x3);
	}
}

int MC68000BASE::perform_fsave(uint32_t addr, int inc)
{
	if(m_cpu_type & CPU_TYPE_040)
	{
		if(inc)
		{
			write_32(addr, 0x41000000);
			return 4;
		}
		else
		{
			write_32(addr-4, 0x41000000);
			return -4;
		}
	}

	if (inc)
	{
		// 68881 IDLE, version 0x1f
		write_32(addr, 0x1f180000);
		write_32(addr+4, 0);
		write_32(addr+8, 0);
		write_32(addr+12, 0);
		write_32(addr+16, 0);
		write_32(addr+20, 0);
		write_32(addr+24, 0x70000000);
		return 7*4;
	}
	else
	{
		write_32(addr-4, 0x70000000);
		write_32(addr-8, 0);
		write_32(addr-12, 0);
		write_32(addr-16, 0);
		write_32(addr-20, 0);
		write_32(addr-24, 0);
		write_32(addr-28, 0x1f180000);
		return -7*4;
	}
}

// FRESTORE on a NULL frame reboots the FPU - all registers to NaN, the 3 status regs to 0
void MC68000BASE::do_frestore_null()
{
	int i;

	m_fpcr = 0;
	m_fpsr = 0;
	m_fpiar = 0;
	for (i = 0; i < 8; i++)
	{
		m_fpr[i].high = 0x7fff;
		m_fpr[i].low = U64(0xffffffffffffffff);
	}

	// Mac IIci at 408458e6 wants an FSAVE of a just-restored NULL frame to also be NULL
	// The PRM says it's possible to generate a NULL frame, but not how/when/why.  (need the 68881/68882 manual!)
	m_fpu_just_reset = 1;
}

void MC68000BASE::m68040_do_fsave(uint32_t addr, int reg, int inc)
{
	if (m_fpu_just_reset)
	{
			write_32(addr, 0);
	}
	else
	{
		// we normally generate an IDLE frame
		int delta = perform_fsave(addr, inc);
		if(reg != -1)
			REG_A()[reg] += delta;
	}
}

void MC68000BASE::m68040_do_frestore(uint32_t addr, int reg)
{
	bool m40 = ((m_cpu_type & CPU_TYPE_040) != 0);
	uint32_t temp = read_32(addr);

	// check for NULL frame
	if (temp & 0xff000000)
	{
		// we don't handle non-NULL frames
		m_fpu_just_reset = 0;

		if (reg != -1)
		{
			// how about an IDLE frame?
			if (!m40 && ((temp & 0x00ff0000) == 0x00180000))
			{
				REG_A()[reg] += 7*4;
			}
			else if (m40 && ((temp & 0xffff0000) == 0x41000000))
			{
				REG_A()[reg] += 4;
			} // check UNIMP
			else if ((temp & 0x00ff0000) == 0x00380000)
			{
				REG_A()[reg] += 14*4;
			} // check BUSY
			else if ((temp & 0x00ff0000) == 0x00b40000)
			{
				REG_A()[reg] += 45*4;
			}
		}
	}
	else
	{
		do_frestore_null();
	}
}

void MC68000BASE::m68040_fpu_op1()
{
	int ea = m_ir & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	uint32_t addr;

	switch ((m_ir >> 6) & 0x3)
	{
		case 0:     // FSAVE <ea>
		{
			switch (mode)
			{
			case 2: // (An)
				addr = REG_A()[reg];
				m68040_do_fsave(addr, -1, 1);
				break;

			case 3: // (An)+
				addr = EA_AY_PI_32();
				m68040_do_fsave(addr, reg, 1);
				break;

			case 4: // -(An)
				addr = EA_AY_PD_32();
				m68040_do_fsave(addr, reg, 0);
				break;

			case 5: // (D16, An)
				addr = EA_AY_DI_16();
				m68040_do_fsave(addr, -1, 1);
				break;

			case 6: // (An) + (Xn) + d8
				addr = EA_AY_IX_16();
				m68040_do_fsave(addr, -1, 1);
				break;

			case 7: //
				switch (reg)
				{
					case 1:     // (abs32)
					{
						addr = EA_AL_32();
						m68040_do_fsave(addr, -1, 1);
						break;
					}
					case 2:     // (d16, PC)
					{
						addr = EA_PCDI_16();
						m68040_do_fsave(addr, -1, 1);
						break;
					}
					default:
						logging->out_logf(LOG_ERROR, "M68kFPU: FSAVE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC());
				}

				break;

			default:
				logging->out_logf(LOG_ERROR, "M68kFPU: FSAVE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC());
			}
			break;
		}
		break;

		case 1:     // FRESTORE <ea>
		{
			switch (mode)
			{
			case 2: // (An)
				addr = REG_A()[reg];
				m68040_do_frestore(addr, -1);
				break;

			case 3: // (An)+
				addr = EA_AY_PI_32();
				m68040_do_frestore(addr, reg);
				break;

			case 5: // (D16, An)
				addr = EA_AY_DI_16();
				m68040_do_frestore(addr, -1);
				break;

			case 6: // (An) + (Xn) + d8
				addr = EA_AY_IX_16();
				m68040_do_frestore(addr, -1);
				break;

			case 7: //
				switch (reg)
				{
					case 1:     // (abs32)
					{
						addr = EA_AL_32();
						m68040_do_frestore(addr, -1);
						break;
					}
					case 2:     // (d16, PC)
					{
						addr = EA_PCDI_16();
						m68040_do_frestore(addr, -1);
						break;
					}
					default:
						logging->out_logf(LOG_ERROR, "M68kFPU: FRESTORE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC());
				}

				break;

			default:
				logging->out_logf(LOG_ERROR, "M68kFPU: FRESTORE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC());
			}
			break;
		}
		break;

		default:    logging->out_logf(LOG_ERROR, "m68040_fpu_op1: unimplemented op %d at %08X\n", (m_ir >> 6) & 0x3, REG_PC()-2);
	}
}

void MC68000BASE::m68881_ftrap()
{
	uint16_t w2  = OPER_I_16();

	// now check the condition
	if (TEST_CONDITION(w2 & 0x3f))
	{
		// trap here
		m_proc_mode = (PROC_MODE_EXCEPTION_TRAPV);
	}
	else    // fall through, requires eating the operand
	{
		switch (m_ir & 0x7)
		{
			case 2: // word operand
				OPER_I_16();
				break;

			case 3: // long word operand
				OPER_I_32();
				break;

			case 4: // no operand
				break;
		}
	}
}

#endif /* USE_MC68000FPU */
