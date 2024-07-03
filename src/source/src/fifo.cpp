/** @file fifo.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.01-

	@brief [ fifo buffer ]

	@note Original author is Takeda.Toshiya
*/

#include "fifo.h"


//

FIFOBYTE::FIFOBYTE()
	: FIFOBase<uint8_t>()
{
}

FIFOBYTE::FIFOBYTE(int s)
	: FIFOBase<uint8_t>(s)
{
}

/// load from buffer
void FIFOBYTE::load_from(const uint8_t *buffer, int read_pos, int write_pos, int count)
{
	memcpy(buf, buffer, siz * sizeof(uint8_t));
	load_pos_from(read_pos, write_pos, count);
}

/// save to buffer
void FIFOBYTE::save_to(uint8_t *buffer, int &read_pos, int &write_pos, int &count) const
{
	memcpy(buffer, buf, siz * sizeof(uint8_t));
	save_pos_to(read_pos, write_pos, count);
}


//

FIFOCHAR::FIFOCHAR()
	: FIFOBase<char>()
{
}

FIFOCHAR::FIFOCHAR(int s)
	: FIFOBase<char>(s)
{
}

/// load from buffer
void FIFOCHAR::load_from(const char *buffer, int read_pos, int write_pos, int count)
{
	memcpy(buf, buffer, siz * sizeof(char));
	load_pos_from(read_pos, write_pos, count);
}

/// save to buffer
void FIFOCHAR::save_to(char *buffer, int &read_pos, int &write_pos, int &count) const
{
	memcpy(buffer, buf, siz * sizeof(char));
	save_pos_to(read_pos, write_pos, count);
}

//

FIFOWORD::FIFOWORD()
	: FIFOBase<uint16_t>()
{
}

FIFOWORD::FIFOWORD(int s)
	: FIFOBase<uint16_t>(s)
{
}

/// load from buffer
void FIFOWORD::load_from(const uint16_t *buffer, int read_pos, int write_pos, int count)
{
	for(int i=0; i<siz; i++) buf[i] = Uint16_LE(buffer[i]);
	load_pos_from(read_pos, write_pos, count);
}

/// save to buffer
void FIFOWORD::save_to(uint16_t *buffer, int &read_pos, int &write_pos, int &count) const
{
	for(int i=0; i<siz; i++) buffer[i] = Uint16_LE(buf[i]);
	save_pos_to(read_pos, write_pos, count);
}

//

FIFOINT::FIFOINT()
	: FIFOBase<int>()
{
}

FIFOINT::FIFOINT(int s)
	: FIFOBase<int>(s)
{
}

/// load from buffer
void FIFOINT::load_from(const int *buffer, int read_pos, int write_pos, int count)
{
	for(int i=0; i<siz; i++) buf[i] = Int32_LE(buffer[i]);
	load_pos_from(read_pos, write_pos, count);
}

/// save to buffer
void FIFOINT::save_to(int *buffer, int &read_pos, int &write_pos, int &count) const
{
	for(int i=0; i<siz; i++) buffer[i] = Int32_LE(buf[i]);
	save_pos_to(read_pos, write_pos, count);
}
