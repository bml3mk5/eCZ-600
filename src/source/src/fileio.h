/** @file fileio.h

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@brief [ file i/o ]
*/

#ifndef FILEIO_H
#define FILEIO_H

#include "common.h"
#include <stdio.h>

/**
	@brief File I/O
*/
class FILEIO
{
public:
	/// @brief modes on FILEIO::Fopen()
	enum FILEIO_MODES {
		READ_BINARY				= 1,
		WRITE_BINARY			= 2,
		READ_WRITE_BINARY		= 3,
		READ_WRITE_NEW_BINARY	= 4,
		READ_ASCII				= 5,
		WRITE_ASCII				= 6,
		READ_WRITE_ASCII		= 7,
		READ_WRITE_NEW_ASCII	= 8,
	};
	/// @brief seek origin on FILEIO::Fseek()
	enum FILEIO_SEEK_ORIGIN {
		SEEKSET	= 0,
		SEEKCUR	= 1,
		SEEKEND	= 2,
	};
	/// @brief fopen mode table
	static const char *c_fopen_mode[];

private:
	FILE* fp;

	static int GetAttribute(const _TCHAR *filename, int *flags);

public:
	FILEIO();
	~FILEIO();

	static bool IsFileExists(const _TCHAR *filename);
	static bool IsFileProtected(const _TCHAR *filename);
	static bool IsDirExists(const _TCHAR *dirname);
	static void RemoveFile(const _TCHAR *filename);

	bool Fopen(const _TCHAR *filename, FILEIO_MODES mode);
	void Fclose();
	bool IsOpened() const { return (fp != NULL); }
	uint32_t FileLength();

	bool FgetBool();
	size_t FputBool(bool val);
	uint8_t FgetUint8();
	size_t FputUint8(uint8_t val);
#if 0
	uint16_t FgetUint16();
	void FputUint16(uint16_t val);
	uint32_t FgetUint32();
	void FputUint32(uint32_t val);
	uint64_t FgetUint64();
	void FputUint64(uint64_t val);
#endif
	int8_t FgetInt8();
	size_t FputInt8(int8_t val);
#if 0
	int16_t FgetInt16();
	void FputInt16(int16_t val);
	int32_t FgetInt32();
	void FputInt32(int32_t val);
	int64_t FgetInt64();
	void FputInt64(int64_t val);
#endif
	float FgetFloat_LE();
	size_t FputFloat_LE(float val);
	double FgetDouble_LE();
	size_t FputDouble_LE(double val);

	uint16_t FgetUint16_LE();
	size_t FputUint16_LE(uint16_t val);
	uint32_t FgetUint32_LE();
	size_t FputUint32_LE(uint32_t val);
	uint64_t FgetUint64_LE();
	size_t FputUint64_LE(uint64_t val);
	int16_t FgetInt16_LE();
	size_t FputInt16_LE(int16_t val);
	int32_t FgetInt32_LE();
	size_t FputInt32_LE(int32_t val);
	int64_t FgetInt64_LE();
	size_t FputInt64_LE(int64_t val);

	uint16_t FgetUint16_BE();
	size_t FputUint16_BE(uint16_t val);
	uint32_t FgetUint32_BE();
	size_t FputUint32_BE(uint32_t val);
	uint64_t FgetUint64_BE();
	size_t FputUint64_BE(uint64_t val);
	int16_t FgetInt16_BE();
	size_t FputInt16_BE(int16_t val);
	int32_t FgetInt32_BE();
	size_t FputInt32_BE(int32_t val);
	int64_t FgetInt64_BE();
	size_t FputInt64_BE(int64_t val);

	int Fgetc();
	int Fputc(int c);
	char    *Fgets(char *buffer, int max_size);
#ifdef _UNICODE
	wchar_t *Fgets(wchar_t *buffer, int max_size);
#endif
	int Fputs(const char *buffer);
#ifdef _UNICODE
	int Fputs(const wchar_t *buffer);
#endif
	int Fsets(int c, int max_size);
	int Fprintf(const char *format, ...);
#ifdef _UNICODE
	int Fprintf(const wchar_t *format, ...);
#endif
	size_t Fread(void* buffer, size_t size, size_t count);
	size_t Fwrite(const void* buffer, size_t size, size_t count);
	size_t FwriteWithSize(const void* buffer, size_t size, size_t count);
	uint32_t Fseek(long offset, FILEIO_SEEK_ORIGIN origin);
	uint32_t Ftell();
	int Flush();
	int Ftruncate(size_t size);
	FILE *GetFile() { return fp; }
	void SetFile(FILE *fp_) { fp = fp_; }
};

#endif /* FILEIO_H */
