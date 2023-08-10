/** @file common.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@brief [ common function ]
*/

#include "common.h"

/// swap endian
uint16_t swap16(uint16_t x) {
	return ((x << 8) | (x >> 8));
}

/// swap endian
uint32_t swap32(uint32_t x) {
	return ((x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24));
}

/// swap endian
uint64_t swap64(uint64_t x) {
	return ((x << 56) | ((x & 0xff00) << 40) | ((x & 0xff0000) << 24) | ((x & 0xff000000) << 8)
		 | ((x & 0xff00000000) >> 8) | ((x & 0xff0000000000) >> 24) | ((x & 0xff0000000000) >> 40) | (x >> 56));
}

/// treat specified buffer pointer as uint16_t value
uint16_t conv_to_uint16_le(uint8_t *src)
{
	return Uint16_LE(*((uint16_t *)src));
}

/// treat specified buffer pointer as uint32_t value
uint32_t conv_to_uint32_le(uint8_t *src)
{
	return Uint32_LE(*((uint32_t *)src));
}

/// treat specified buffer pointer as int16_t value
int16_t conv_to_int16_le(uint8_t *src)
{
	return Int16_LE(*((int16_t *)src));
}

/// treat specified buffer pointer as int32_t value
int32_t conv_to_int32_le(uint8_t *src)
{
	return Int32_LE(*((int32_t *)src));
}

/// store uint16_t value to specified buffer pointer
void conv_from_uint16_le(uint8_t *dst, uint16_t val)
{
	*((uint16_t *)dst) = Uint16_LE(val);
}

/// store uint32_t value to specified buffer pointer
void conv_from_uint32_le(uint8_t *dst, uint32_t val)
{
	*((uint32_t *)dst) = Uint32_LE(val);
}

/// store int16_t value to specified buffer pointer
void conv_from_int16_le(uint8_t *dst, int16_t val)
{
	*((int16_t *)dst) = Int16_LE(val);
}

/// store int32_t value to specified buffer pointer
void conv_from_int32_le(uint8_t *dst, int32_t val)
{
	*((int32_t *)dst) = Int32_LE(val);
}
