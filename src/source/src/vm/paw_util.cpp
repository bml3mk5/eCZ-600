/// @file paw_util.cpp
///
/// @author Sasaji
/// @date   2017.12.01
///

#include <string.h>
#include "paw_util.h"
#include "../fileio.h"


namespace PARSEWAV
{

/// @brief コンストラクタ
///
Util::Util()
{
	src_buf = NULL;
	new_buf = NULL;
	InitConvSampleData(1);
}

/// @brief デストラクタ
///
Util::~Util()
{
	delete[] new_buf;
	delete[] src_buf;
}

/// @brief waveサンプリングデータのレートとビット変換用の変数初期化
///
void Util::InitConvSampleData(int buffer_size)
{
	divide = 0;
	surplus = 0;
	remain_buf_len = 0;
	memset(remain_buf, 0, sizeof(remain_buf));

	buf_size = buffer_size;

	delete [] src_buf;
	src_buf = new int16_t[buf_size];
	delete [] new_buf;
	new_buf = new int16_t[buf_size];
}

/// @brief waveサンプリングデータのレートとビットを変換してファイルに出力
///
/// @param[in] in_data      サンプリングデータ
/// @param[in] in_rate      bufのサンプルレート
/// @param[out] outbuf 出力データ
/// @param[in] outrate outbufのサンプルレート
/// @param[in] outbits outbufのサンプルビット(8 or 16)
/// @param[in] outlen  outbuf長さ
/// @param[in] outpos  outbufに書き出す最初の位置
/// @return outpos + 書き出したバイト数
size_t Util::OutConvSampleData(WaveData &in_data, int in_rate
						,uint8_t *outbuf, int outrate, int outbits, size_t outlen, size_t outpos)
{
	CSampleBytes in_buf(in_data, in_data.GetStartPos(), in_data.Length());
	in_data.SetStartPos(in_data.GetWritePos());
	return OutConvSampleData(in_buf.Get(), in_rate, 8, 1, in_buf.Length(), outbuf, outrate, outbits, outlen, outpos);
}

/// @brief waveサンプリングデータのレートとビットを変換してファイルに出力
///
/// @param[in] in_buf  入力データ
/// @param[in] in_rate in_bufのサンプルレート
/// @param[in] in_bits in_bufのサンプルビット(8 or 16)
/// @param[in] in_ch   in_bufのチャンネル数(1 or 2)
/// @param[in] in_len  in_buf長さ
/// @param[out] outbuf 出力データ
/// @param[in] outrate outbufのサンプルレート
/// @param[in] outbits outbufのサンプルビット(8 or 16)
/// @param[in] outlen  outbuf長さ
/// @param[in] outpos  outbufに書き出す最初の位置
/// @return outpos + 書き出したバイト数
size_t Util::OutConvSampleData(const void *in_buf, uint32_t in_rate, int in_bits, int in_ch, size_t in_len
							,  uint8_t *outbuf, uint32_t outrate, int outbits, size_t outlen, size_t outpos)
{
	size_t src_len = 0;
	size_t pos = 0;
	int new_data = 0;
	size_t new_len = 0;
	size_t i = 0;

	memset(src_buf, 0, buf_size * sizeof(int16_t));
	memset(new_buf, 0, buf_size * sizeof(int16_t));
	for (i=0; i<(size_t)remain_buf_len; i++) {
		src_buf[i]=remain_buf[i];
	}
	src_len = remain_buf_len;

	for(i=0; i<in_len; i+=in_ch) {
		if (in_bits == 8) {
			// 8ビット
			src_buf[src_len]=(int16_t)*((uint8_t *)in_buf + i);
		} else if (in_bits == 16) {
			// 16ビット
			src_buf[src_len]=(int16_t)*((int16_t *)in_buf + i);
		}
		src_len++;
	}

	memset(remain_buf, 0, sizeof(remain_buf));
	remain_buf_len = 0;

	if (in_rate > outrate) {
		// rate 下げる場合
		pos = 0;
		while (pos < src_len) {
			divide = (in_rate + surplus) / outrate;
			if (pos + divide + 2 >= src_len) {
				break;
			}
			int n_surplus = (in_rate + surplus) % outrate;

			// 平均化
			new_data = 0;
			if (surplus >= 0) {
				new_data += (int)((double)src_buf[pos] * (double)((int)outrate - surplus) / (double)outrate);
				pos++;
			}
			for(i = 1; i < (size_t)divide; i++) {
				new_data += src_buf[pos];
				pos++;
			}
			if (n_surplus > 0) {
				new_data += (int)((double)src_buf[pos] * (double)n_surplus / (double)outrate);
			}
			new_data = (int)((double)new_data * (double)outrate / (double)in_rate);

			surplus = n_surplus;
			new_buf[new_len] = (int16_t)new_data;
			new_len++;
		}
		if (pos < src_len) {
			// 余ったデータは次回にまわす
			for(i=pos; i<src_len; i++) {
				remain_buf[i-pos]=src_buf[i];
			}
			remain_buf_len = (int)(src_len - pos);
		}
	} else if (in_rate < outrate) {
		// rate 上げる場合
		pos = 0;
		while (pos < src_len - 1) {
			divide = (outrate + surplus) / in_rate;
			surplus = (outrate + surplus) % in_rate;
			// サンプル間を線形で補完
			for(i=0; i<(size_t)divide; i++) {
				new_data = (int)src_buf[pos] + (int)i * ((int)src_buf[pos+1] - (int)src_buf[pos]) / divide;
				new_buf[new_len] = (int16_t)new_data;
				new_len++;
				// バッファがいっぱいなのでファイルに出す
				if (new_len >= outlen - 1) {
					outpos = out_sample_buffer(new_buf, new_len, in_bits, outbuf, outlen, outpos, outbits);
					memset(new_buf, 0, sizeof(int16_t) * outlen);
					new_len = 0;
				}
			}
			pos++;
		}
		// 最後のデータを次に回す
		remain_buf[0]=src_buf[src_len-1];
		remain_buf_len = 1;
	} else {
		// 同じレート
		memcpy(new_buf, src_buf, src_len * sizeof(int16_t));
		new_len = src_len;
	}

	outpos = out_sample_buffer(new_buf, new_len, in_bits, outbuf, outlen, outpos, outbits);

	return outpos;
}

/// @brief waveファイルをサンプリングレート/サンプルビットを変換してバッファに読み込む
///
/// @param[in] file    入力wavファイルポインタ
/// @param[in] in_fmt  入力wavファイルのfmtチャンク
/// @param[in] in_len  入力wavファイルのデータ部の長さ
/// @param[out] outbuf 出力データ
/// @param[in] outrate outbufのサンプルレート
/// @param[in] outbits outbufのサンプルビット(8 or 16)
/// @param[in] outlen  outbufの長さ
/// @return 書き出したバイト数
size_t Util::ReadWavData(FILEIO &file, wav_fmt_chank_t *in_fmt, size_t in_len, uint8_t *outbuf, uint32_t outrate, int outbits, size_t outlen)
{
	uint8_t buf[32];
	size_t outpos = 0;
	size_t len = 0;
	size_t siz = 0;

	InitConvSampleData((int)outlen);
	while ((siz = file.Fread(buf, sizeof(uint8_t), 32)) > 0 && len < in_len && outpos < outlen) {
		outpos = OutConvSampleData(buf, in_fmt->sample_rate, in_fmt->sample_bits, in_fmt->channels, siz * 8 / in_fmt->sample_bits
									, outbuf, outrate, outbits, outlen, outpos);
		len += siz;
	}
	return outpos;
}

/// @brief waveサンプリングデータのビット変換
///
/// @param[in] in_buf 入力データ
/// @param[in] in_len  in_buf長さ
/// @param[in] in_bits in_bufのサンプルビット(8 or 16)
/// @param[out] outbuf 出力データ
/// @param[in] outlen  outbuf長さ
/// @param[in] outpos  outbufに書き出す最初の位置
/// @param[in] outbits outbufのサンプルビット(8 or 16)
/// @return outpos + 書き出したバイト数
size_t Util::out_sample_buffer(int16_t *in_buf, size_t in_len, int in_bits, uint8_t *outbuf, size_t outlen, size_t outpos, int outbits)
{
	int16_t new_data = 0;
	size_t i = 0;

	// 出力
	for(i=0; i<in_len; i++) {
		if (in_bits > outbits) {
			// ビット 下げる場合
			new_data = (in_buf[i] / 256) + 128;
		} else if (in_bits < outbits) {
			// ビット 上げる場合
			new_data = (in_buf[i] - 128) * 256;
		} else {
			new_data = in_buf[i];
		}
		if (outbits == 16) {
			outbuf[outpos++] = (new_data & 0xff);
			outbuf[outpos++] = ((new_data >> 8) & 0xff);
		} else {
			outbuf[outpos++] = (new_data & 0xff);
		}
		if (outpos >= outlen) {
			break;
		}
	}
	return outpos;
}

/// @brief WAVEファイルのフォーマットをチェックする
///
/// @param[in]   file   入力ファイル
/// @param[out]  format waveフォーマット
/// @param[out]  data_len サンプル数
/// @return 0>=:データチャンク開始位置  -1:ファイルなし(空) -2: PCMでない -3:対応レートでない
int Util::CheckWavFormat(FILEIO &file, WaveFormat &format, size_t *data_len)
{
	return CheckWavFormat(file, format.GetHead(), format.GetFmtChank(), format.GetDataChank(), data_len);
}

/// @brief WAVEファイルのフォーマットをチェックする
///
/// @param[in]   file   入力ファイル
/// @param[out]  head   waveヘッダ
/// @param[out]  fmt    waveフォーマットタイプ
/// @param[out]  data   waveデータ
/// @param[out]  data_len サンプル数
/// @return 0>=:データチャンク開始位置  -1:ファイルなし(空) -2: PCMでない -3:対応レートでない
int Util::CheckWavFormat(FILEIO &file, wav_header_t *head, wav_fmt_chank_t *fmt, wav_data_chank_t *data, size_t *data_len)
{
	char buf[10];
	long offset = 0;
	long fpos_data = 0;
	wav_unknown_chank_t unk;

	memset(head, 0, sizeof(wav_header_t));
	memset(fmt, 0, sizeof(wav_fmt_chank_t));
	memset(data, 0, sizeof(wav_data_chank_t));
	if (data_len != NULL) *data_len = 0;

	offset = (long)file.Fread(head, sizeof(wav_header_t), 1);
	if (offset == 0) {
		// empty file
		return -1;
	}

	if(memcmp(head->RIFF,"RIFF",4) != 0 || memcmp(head->WAVE,"WAVE",4) != 0) {
		// this is not wave format !!!
		return -2;
	}

	for (int i=0; i<10; i++) {
		file.Fread(buf, sizeof(char), 4);
		if (memcmp(buf, "fmt ", 4) == 0) {
			// fmt chank
			file.Fseek(-4, FILEIO::SEEKCUR);
			file.Fread(fmt, sizeof(wav_fmt_chank_t), 1);
			if (fmt->format_id != 1) {
				// this is not pcm format !!!
				return -2;
			}

			// 11025 - 48000Hz
			if (fmt->sample_rate < 11025 || 48000 < fmt->sample_rate) {
				return -3;
			}

			if (fpos_data != 0) break;

			offset = 8 + fmt->fmt_size - sizeof(wav_fmt_chank_t);

		} else if (memcmp(buf, "data", 4) == 0) {
			// data chank
			file.Fseek(-4, FILEIO::SEEKCUR);
			file.Fread(data, sizeof(wav_data_chank_t), 1);
			fpos_data = file.Ftell();

			if (fmt->format_id != 0) break;

			offset = data->data_len;

		} else {
			// unknown chank
			file.Fseek(-4, FILEIO::SEEKCUR);
			file.Fread(&unk, sizeof(wav_unknown_chank_t), 1);

			offset = unk.len;
		}
		file.Fseek(offset, FILEIO::SEEKCUR);
	}

	if (fpos_data == 0 || fmt->format_id != 1) {
		// this is not pcm format !!!
		return -2;
	}

	file.Fseek(fpos_data, FILEIO::SEEKSET);

	if (data_len != NULL) {
		*data_len = (data->data_len) / fmt->channels;
		if(fmt->sample_bits == 16) {
			*data_len /= 2;
		}
	}

	return (int)fpos_data;
}

}; /* namespace PARSEWAV */
