/// @file paw_format.cpp
///
/// @brief wav format
///
/// @author Sasaji
/// @date   2017.12.01
///

#include <string.h>
#include "paw_format.h"
#include "../fileio.h"


namespace PARSEWAV
{

WaveFormat::WaveFormat()
{
	this->Init(48000, 8, 1);
}

WaveFormat::WaveFormat(int rate, int bits, int channels)
{
	this->Init(rate, bits, channels);
}

WaveFormat::~WaveFormat()
{
}

void WaveFormat::Init(int rate, int bits, int channels)
{
	// wav file header
	memcpy(head.RIFF, "RIFF", 4);
	head.file_len = 0;
	memcpy(head.WAVE, "WAVE", 4);

	memcpy(fmt.fmt, "fmt ", 4);
	fmt.format_id = 1;	// Linear PCM
	fmt.channels = (uint16_t)channels;
	fmt.sample_rate = (uint32_t)rate;
	fmt.data_speed = (uint32_t)(rate * channels);
	fmt.block_size = (uint16_t)(bits * channels / 8);
	fmt.sample_bits = bits;
	fmt.fmt_size = 16;

	memcpy(data.data, "data", 4);
	data.data_len = 0;
}

int WaveFormat::Out(uint8_t *buffer)
{
	int len = 0;
	int len_all = 0;
	len = sizeof(head);
	memcpy(buffer + len_all, &head, len);
	len_all += len;
	len = sizeof(fmt);
	memcpy(buffer + len_all, &fmt, len);
	len_all += len;
	len = sizeof(data);
	memcpy(buffer + len_all, &data, len);
	len_all += len;

	return len_all;
}

void WaveFormat::OutSize(FILEIO *fp)
{
	uint32_t file_size = 0;
	uint32_t riff_size = 0;
	uint32_t data_size = 0;

	fp->Fseek(0, FILEIO::SEEKEND);
	file_size = fp->Ftell();

	riff_size = file_size - 8;

	fp->Fseek(4, FILEIO::SEEKSET);
	fp->Fwrite((uint32_t *)&riff_size, sizeof(uint32_t), 1);

	data_size = file_size - 44;

	fp->Fseek(40, FILEIO::SEEKSET);
	fp->Fwrite((uint32_t *)&data_size, sizeof(uint32_t), 1);
}

}; /* namespace PARSEWAV */
