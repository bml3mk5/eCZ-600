/// @file paw_util.h
///
/// @author Sasaji
/// @date   2011.12.01
///


#ifndef _PARSEWAV_UTIL_H_
#define _PARSEWAV_UTIL_H_

#include "../common.h"
#include "paw_datas.h"
#include "paw_format.h"

class FILEIO;

namespace PARSEWAV
{

/// @brief WAVファイルを扱うクラス
class Util
{
private:
	int remain_buf[100];
	int remain_buf_len;
	int divide;
	int surplus;

	int buf_size;
	int16_t *src_buf;
	int16_t *new_buf;

	size_t out_sample_buffer(int16_t *in_buf, size_t in_len, int in_bsiz, uint8_t *outbuf, size_t outlen, size_t outpos, int outbsiz);

public:
	Util();
	~Util();

	void   InitConvSampleData(int buffer_size);
	size_t OutConvSampleData(WaveData &in_data, int in_rate
						,uint8_t *outbuf, int outrate, int outbits, size_t outlen, size_t outpos);
	size_t OutConvSampleData(const void *in_buf, uint32_t in_rate, int in_bits, int in_ch, size_t in_len
						, uint8_t *outbuf, uint32_t outrate, int outbits, size_t outlen, size_t outpos);
	size_t ReadWavData(FILEIO &file, wav_fmt_chank_t *in_fmt, size_t in_len, uint8_t *outbuf, uint32_t outrate, int outbits, size_t outlen);

	static int CheckWavFormat(FILEIO &file, wav_header_t *head, wav_fmt_chank_t *fmt, wav_data_chank_t *data, size_t *data_len = NULL);
	static int CheckWavFormat(FILEIO &file, WaveFormat &format, size_t *data_len = NULL);

};

}; /* namespace PARSEWAV */

#endif /* _PARSEWAV_UTIL_H_ */
