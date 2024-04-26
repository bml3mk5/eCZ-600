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

//

FIFOCHAR::FIFOCHAR()
	: FIFOBase<char>()
{
}

FIFOCHAR::FIFOCHAR(int s)
	: FIFOBase<char>(s)
{
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
