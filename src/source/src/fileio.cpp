/** @file fileio.cpp

	Skelton for retropc emulator

	@author Takeda.Toshiya
	@date   2006.08.18 -

	@brief [ file i/o ]
*/

#if defined(_WIN32) && !defined(NO_USE_WINAPI)
#include "depend.h"
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#if defined(_WIN32) && defined(_MSC_VER)
#include <Share.h>
#endif
#if !defined(USE_WIN)
#include <sys/stat.h>
#endif
#include "fileio.h"
#include "utility.h"

const char *FILEIO::c_fopen_mode[] = {
	"",
	"rb",
	"wb",
	"r+b",
	"w+b",
	"r",
	"w",
	"r+",
	"w+",
	NULL
};

FILEIO::FILEIO()
{
	fp = NULL;
}

FILEIO::~FILEIO()
{
	Fclose();
}

/// @brief Get attribute of the specified file
///
/// @param[in] filename: file path
/// @param[out] flags: (nullable) extended status
/// @return attribute
int FILEIO::GetAttribute(const _TCHAR *filename, int *flags)
{
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
#if defined(USE_UTF8_ON_MBCS)
	// convert UTF-8 to MBCS string
	_TCHAR tfilename[_MAX_PATH];
	UTILITY::conv_to_native_path(filename, tfilename, _MAX_PATH);
#else
	const _TCHAR *tfilename = filename;
#endif
#if defined(USE_WIN)
	// use windows API
	DWORD attr = GetFileAttributes(tfilename);
	if(attr == -1) {
		return -1;
	}
	return (int)attr;
#else
	struct _stat fs;

	if (_tstat(tfilename, &fs)) {
		return -1;
	}
	return (int)fs.st_mode;
#endif
#else
#if defined(_UNICODE)
	// convert wchar_t to char
	char tfilename[_MAX_PATH];
	UTILITY::cconv_from_native_path(filename, tfilename, _MAX_PATH);
#else
	const char *tfilename = filename;
#endif

	struct stat fs;

	if (stat(tfilename, &fs)) {
		return -1;
	}
#if defined(__APPLE__) && defined(__MACH__)
	if (flags) {
		*flags = (int)fs.st_flags;
	}
#endif
	return (int)fs.st_mode;
#endif
}

/// @brief Does the specified file exist?
///
/// @param[in] filename: file path
/// @return true/false
bool FILEIO::IsFileExists(const _TCHAR *filename)
{
	int attr = GetAttribute(filename, NULL);
#if defined(_WIN32)
#if defined(USE_WIN)
	return (attr >= 0 && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	return (attr >= 0 && (attr & _S_IFREG) != 0);
#endif
#else
	return (attr >= 0 && (attr & S_IFREG) != 0);
#endif
}

/// @brief Is the specified file write protected?
///
/// @param[in] filename: file path
/// @return true/false
bool FILEIO::IsFileProtected(const _TCHAR *filename)
{
	int flags = 0;
	int attr = GetAttribute(filename, &flags);
#if defined(_WIN32)
#if defined(USE_WIN)
	// use windows API
	return (attr >= 0 && (attr & FILE_ATTRIBUTE_READONLY) != 0);
#else
	return (attr >= 0 && (attr & _S_IWRITE) == 0);
#endif
#else
#if defined(__APPLE__) && defined(__MACH__)
	return (attr >= 0 && ((attr & S_IWUSR) == 0 || (flags & 2) != 0));
#else
	return (attr >= 0 && (attr & S_IWUSR) == 0);
#endif
#endif
}

/// @brief Does the specified directory exist?
///
/// @param[in] dirname: directory path
/// @return true/false
bool FILEIO::IsDirExists(const _TCHAR *dirname)
{
	int attr = GetAttribute(dirname, NULL);
#if defined(_WIN32)
#if defined(USE_WIN)
	return (attr >= 0 && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
	return (attr >= 0 && (attr & _S_IFDIR) != 0);
#endif
#else
	return (attr >= 0 && (attr & S_IFDIR) != 0);
#endif
}

/// @brief Remove the specified file
///
/// @param[in] filename: file path
void FILEIO::RemoveFile(const _TCHAR *filename)
{
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
#if defined(USE_UTF8_ON_MBCS)
	// convert UTF-8 to MBCS string
	_TCHAR tfilename[_MAX_PATH];
	UTILITY::conv_to_native_path(filename, tfilename, _MAX_PATH);
#else
	const _TCHAR *tfilename = filename;
#endif
#if defined(USE_WIN)
	// use windows API
	DeleteFile(tfilename);
//	_tremove(tfilename);	// not supported on wince
#else
	_tunlink(tfilename);
#endif
#else
#if defined(_UNICODE)
	// convert wchar_t to char
	char tfilename[_MAX_PATH];
	UTILITY::cconv_from_native_path(filename, tfilename, _MAX_PATH);
#else
	const char *tfilename = filename;
#endif
	unlink(tfilename);
#endif
}

/// @brief Open the specified file
///
/// @param[in] filename: file path
/// @param[in] mode: file mode
/// @return true/false
bool FILEIO::Fopen(const _TCHAR *filename, FILEIO_MODES mode)
{
	Fclose();

#if defined(_WIN32) || !defined(_UNICODE)
# if defined(USE_UTF8_ON_MBCS)
	// convert UTF-8 to MBCS string (on SDL)
	_TCHAR tfilename[_MAX_PATH];
	UTILITY::conv_to_native_path(filename, tfilename, _MAX_PATH);
# else
	const _TCHAR *tfilename = filename;
# endif
# ifdef _UNICODE
	// using UNICODE on Windows
	wchar_t fopen_mode[4];
	UTILITY::conv_mbs_to_wcs(c_fopen_mode[mode], 4, fopen_mode, 4);
# else
	const char *fopen_mode = c_fopen_mode[mode];
# endif
# ifdef _MSC_VER
	// for Visual C++
//	return ((_tfopen_s(&fp, tfilename, fopen_mode)) == 0);
	return ((fp = _tfsopen(tfilename, fopen_mode, _SH_DENYNO)) != NULL);
# else
	return ((fp = _tfopen(tfilename, fopen_mode)) != NULL);
# endif
#else
	// convert wchar_t to char
	char tfilename[_MAX_PATH];
	UTILITY::cconv_from_native_path(filename, tfilename, _MAX_PATH);
	return ((fp = fopen(tfilename, c_fopen_mode[mode])) != NULL);
#endif
	return false;
}

/// @brief Close the opened file
void FILEIO::Fclose()
{
	if(fp) {
		fclose(fp);
	}
	fp = NULL;
}

/// @brief Return file size of the opened file
/// @return size
uint32_t FILEIO::FileLength()
{
	long pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return (uint32_t)len;
}

/*
#define GET_VALUE(type) \
	uint8_t buffer[sizeof(type)]; \
	fread(buffer, sizeof(buffer), 1, fp); \
	return *(type *)buffer
*/

#define GET_VALUE(type) \
	type value; \
	fread((void *)&value, sizeof(value), 1, fp); \
	return value

#define PUT_VALUE(type, v) \
	return (fwrite(&v, sizeof(type), 1, fp) * sizeof(type))

/// @brief Get the bool(1byte) value from the opened file
/// @return true when value is 1
bool FILEIO::FgetBool()
{
	return (fgetc(fp) == 1);
}

/// @brief Put the bool(1byte) value to the opened file
/// Set 1 when true
/// @param[in] val: true/false
/// @return 1 (byte)
size_t FILEIO::FputBool(bool val)
{
	fputc(val ? 1 : 0, fp);
	return 1;
}

/// @brief Get the 1byte unsigned value from the opened file
/// @return 1byte value
uint8_t FILEIO::FgetUint8()
{
	GET_VALUE(uint8_t);
}

/// @brief Put the 1byte unsigned value to the opened file
/// @param[in] val: 1byte unsigned value
/// @return 1 (byte)
size_t FILEIO::FputUint8(uint8_t val)
{
	PUT_VALUE(uint8_t, val);
}

#if 0
/// @brief Get the 2bytes unsigned value from the opened file
/// @return 2bytes unsigned value
uint16_t FILEIO::FgetUint16()
{
	GET_VALUE(uint16_t);
}

/// @brief Put the 2bytes unsigned value to the opened file
/// @param[in] val: 2bytes unsigned value
/// @return 2 (bytes)
size_t FILEIO::FputUint16(uint16_t val)
{
	PUT_VALUE(uint16_t, val);
}

/// @brief Get the 4bytes unsigned value from the opened file
/// @return 4bytes unsigned value
uint32_t FILEIO::FgetUint32()
{
	GET_VALUE(uint32_t);
}

/// @brief Put the 4bytes unsigned value to the opened file
/// @param[in] val: 4bytes unsigned value
/// @return 4 (bytes)
size_t FILEIO::FputUint32(uint32_t val)
{
	PUT_VALUE(uint32_t, val);
}

/// @brief Get the 8bytes unsigned value from the opened file
/// @return 8bytes unsigned value
uint64_t FILEIO::FgetUint64()
{
	GET_VALUE(uint64_t);
}

/// @brief Put the 8bytes unsigned value to the opened file
/// @param[in] val: 8bytes unsigned value
/// @return 8 (bytes)
size_t FILEIO::FputUint64(uint64_t val)
{
	PUT_VALUE(uint64_t, val);
}
#endif

/// @brief Get the 1byte signed value from the opened file
/// @return 1byte signed value
int8_t FILEIO::FgetInt8()
{
	GET_VALUE(int8_t);
}

/// @brief Put the 1byte signed value to the opened file
/// @param[in] val: 1byte signed value
/// @return 1 (byte)
size_t FILEIO::FputInt8(int8_t val)
{
	PUT_VALUE(int8_t, val);
}

#if 0
/// @brief Get the 2bytes signed value from the opened file
/// @return 2bytes signed value
int16_t FILEIO::FgetInt16()
{
	GET_VALUE(int16_t);
}

/// @brief Put the 2bytes signed value to the opened file
/// @param[in] val: 2bytes signed value
/// @return 2 (bytes)
void FILEIO::FputInt16(int16_t val)
{
	PUT_VALUE(int16_t, val);
}

/// @brief Get the 4bytes signed value from the opened file
/// @return 4bytes signed value
int32_t FILEIO::FgetInt32()
{
	GET_VALUE(int32_t);
}

/// @brief Put the 4bytes signed value to the opened file
/// @param[in] val: 4bytes signed value
/// @return 4 (bytes)
void FILEIO::FputInt32(int32_t val)
{
	PUT_VALUE(int32_t, val);
}

/// @brief Get the 8bytes signed value from the opened file
/// @return 8bytes signed value
int64_t FILEIO::FgetInt64()
{
	GET_VALUE(int64_t);
}

/// @brief Put the 8bytes signed value to the opened file
/// @param[in] val: 8bytes signed value
/// @return 8 (bytes)
void FILEIO::FputInt64(int64_t val)
{
	PUT_VALUE(int64_t, val);
}
#endif

/// @brief Get the float value from the opened file
/// @return float value
float FILEIO::FgetFloat_LE()
{
	pair32_t tmp;
	tmp.u32 = FgetUint32_LE();
	return tmp.fl;
}

/// @brief Put the float value to the opened file
/// @param[in] val: float value
/// @return 4 (bytes)
size_t FILEIO::FputFloat_LE(float val)
{
	pair32_t tmp;
	tmp.fl = val;
	return FputUint32_LE(tmp.u32);
}

/// @brief Get the double value from the opened file
/// @return double value
double FILEIO::FgetDouble_LE()
{
	pair64_t tmp;
	tmp.u64 = FgetUint64_LE();
	return tmp.db;
}

/// @brief Put the double value to the opened file
/// @param[in] val: double value
/// @return 8 (bytes)
size_t FILEIO::FputDouble_LE(double val)
{
	pair64_t tmp;
	tmp.db = val;
	return FputUint64_LE(tmp.u64);
}

/// @brief Get the 2bytes unsigned value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 2bytes unsigned value
uint16_t FILEIO::FgetUint16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.u16;
}

/// @brief Put the 2bytes unsigned value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 2bytes unsigned value
/// @return 2 (bytes)
size_t FILEIO::FputUint16_LE(uint16_t val)
{
	pair16_t tmp;
	size_t sz = 0;
	tmp.u16 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	return sz;
}

/// @brief Get the 4bytes unsigned value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 4bytes unsigned value
uint32_t FILEIO::FgetUint32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.u32;
}

/// @brief Put the 4bytes unsigned value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 4bytes unsigned value
/// @return 4 (bytes)
size_t FILEIO::FputUint32_LE(uint32_t val)
{
	pair32_t tmp;
	size_t sz = 0;
	tmp.u32 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h3);
	return sz;
}

/// @brief Get the 8bytes unsigned value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 8bytes unsigned value
uint64_t FILEIO::FgetUint64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.u64;
}

/// @brief Put the 8bytes unsigned value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 8bytes unsigned value
/// @return 8 (bytes)
size_t FILEIO::FputUint64_LE(uint64_t val)
{
	pair64_t tmp;
	size_t sz = 0;
	tmp.u64 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h4);
	sz += FputUint8(tmp.b.h5);
	sz += FputUint8(tmp.b.h6);
	sz += FputUint8(tmp.b.h7);
	return sz;
}

/// @brief Get the 2bytes signed value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 2bytes signed value
int16_t FILEIO::FgetInt16_LE()
{
	pair16_t tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.s16;
}

/// @brief Put the 2bytes signed value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 2bytes signed value
/// @return 2 (bytes)
size_t FILEIO::FputInt16_LE(int16_t val)
{
	pair16_t tmp;
	size_t sz = 0;
	tmp.s16 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	return sz;
}

/// @brief Get the 4bytes signed value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 4bytes signed value
int32_t FILEIO::FgetInt32_LE()
{
	pair32_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.s32;
}

/// @brief Put the 4bytes signed value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 4bytes signed value
/// @return 4 (bytes)
size_t FILEIO::FputInt32_LE(int32_t val)
{
	pair32_t tmp;
	size_t sz = 0;
	tmp.s32 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h3);
	return sz;
}

/// @brief Get the 8bytes signed value from the opened file
/// @note Recorded value in the file is little endian.
/// @return 8bytes signed value
int64_t FILEIO::FgetInt64_LE()
{
	pair64_t tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.s64;
}

/// @brief Put the 8bytes signed value to the opened file
/// @note Specified value is written in little endian.
/// @param[in] val: 8bytes signed value
/// @return 8 (bytes)
size_t FILEIO::FputInt64_LE(int64_t val)
{
	pair64_t tmp;
	size_t sz = 0;
	tmp.s64 = val;
	sz += FputUint8(tmp.b.l);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h4);
	sz += FputUint8(tmp.b.h5);
	sz += FputUint8(tmp.b.h6);
	sz += FputUint8(tmp.b.h7);
	return sz;
}

/// @brief Get the 2bytes unsigned value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 2bytes unsigned value
uint16_t FILEIO::FgetUint16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.u16;
}

/// @brief Put the 2bytes unsigned value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 2bytes unsigned value
/// @return 2 (bytes)
size_t FILEIO::FputUint16_BE(uint16_t val)
{
	pair16_t tmp;
	size_t sz = 0;
	tmp.u16 = val;
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get the 4bytes unsigned value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 4bytes unsigned value
uint32_t FILEIO::FgetUint32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u32;
}

/// @brief Put the 4bytes unsigned value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 4bytes unsigned value
/// @return 4 (bytes)
size_t FILEIO::FputUint32_BE(uint32_t val)
{
	pair32_t tmp;
	size_t sz = 0;
	tmp.u32 = val;
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get the 8bytes unsigned value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 8bytes unsigned value
uint64_t FILEIO::FgetUint64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u64;
}

/// @brief Put the 8bytes unsigned value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 8bytes unsigned value
/// @return 8 (bytes)
size_t FILEIO::FputUint64_BE(uint64_t val)
{
	pair64_t tmp;
	size_t sz = 0;
	tmp.u64 = val;
	sz += FputUint8(tmp.b.h7);
	sz += FputUint8(tmp.b.h6);
	sz += FputUint8(tmp.b.h5);
	sz += FputUint8(tmp.b.h4);
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get the 2bytes signed value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 2bytes signed value
int16_t FILEIO::FgetInt16_BE()
{
	pair16_t tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.s16;
}

/// @brief Put the 2bytes signed value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 2bytes signed value
/// @return 2 (bytes)
size_t FILEIO::FputInt16_BE(int16_t val)
{
	pair16_t tmp;
	size_t sz = 0;
	tmp.s16 = val;
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get the 4bytes signed value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 4bytes signed value
int32_t FILEIO::FgetInt32_BE()
{
	pair32_t tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s32;
}

/// @brief Put the 4bytes signed value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 4bytes signed value
/// @return 4 (bytes)
size_t FILEIO::FputInt32_BE(int32_t val)
{
	pair32_t tmp;
	size_t sz = 0;
	tmp.s32 = val;
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get the 8bytes signed value from the opened file
/// @note Recorded value in the file is big endian.
/// @return 8bytes signed value
int64_t FILEIO::FgetInt64_BE()
{
	pair64_t tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s64;
}

/// @brief Put the 8bytes signed value to the opened file
/// @note Specified value is written in big endian.
/// @param[in] val: 8bytes signed value
/// @return 8 (bytes)
size_t FILEIO::FputInt64_BE(int64_t val)
{
	pair64_t tmp;
	size_t sz = 0;
	tmp.s64 = val;
	sz += FputUint8(tmp.b.h7);
	sz += FputUint8(tmp.b.h6);
	sz += FputUint8(tmp.b.h5);
	sz += FputUint8(tmp.b.h4);
	sz += FputUint8(tmp.b.h3);
	sz += FputUint8(tmp.b.h2);
	sz += FputUint8(tmp.b.h);
	sz += FputUint8(tmp.b.l);
	return sz;
}

/// @brief Get one character from the opened file
/// @return a character
int FILEIO::Fgetc()
{
	return fgetc(fp);
}

/// @brief Put one character to the opened file
/// @param[in] c: a character
/// @return a character
int FILEIO::Fputc(int c)
{
	return fputc(c, fp);
}

/// @brief Get a string from the opened file
/// @param[out] buffer: a string
/// @param[in] max_size: a string buffer size
/// @return pointer to a string
char *FILEIO::Fgets(char *buffer, int max_size)
{
	return fgets(buffer, max_size, fp);
}

#ifdef _UNICODE
/// @brief Get a string from the opened file
/// @param[out] buffer: a string
/// @param[in] max_size: a string buffer size
/// @return pointer to a string
wchar_t *FILEIO::Fgets(wchar_t *buffer, int max_size)
{
	return fgetws(buffer, max_size, fp);
}
#endif

/// @brief Put a string to the opened file
/// @param[in] buffer: a string
/// @return -1 when error
int FILEIO::Fputs(const char *buffer)
{
	return fputs(buffer, fp);
}

#ifdef _UNICODE
/// @brief Put a string to the opened file
/// @param[in] buffer: a string
/// @return -1 when error
int FILEIO::Fputs(const wchar_t *buffer)
{
	return fputws(buffer, fp);
}
#endif

/// @brief Fill a character to the opened file
/// @param[in] c: a character
/// @param[in] max_size: fill size
/// @return filled size
int FILEIO::Fsets(int c, int max_size)
{
#if 0
	int len = 0;
	while(len < max_size && fputc(c, fp) != EOF) {
		len++;
	}
	return len;
#else
	uint8_t buf[256];
	memset(buf, c, sizeof(buf));
	int remain = max_size;
	while(remain > 0) {
		int len = remain > (int)sizeof(buf) ? (int)sizeof(buf) : remain;
		len = (int)fwrite(buf, sizeof(uint8_t), len, fp);
		if (len == 0) break;
		remain -= len;
	}
	return max_size - remain;
#endif
}

/// @brief Print the formatted string to the opened file
/// @param[in] format: a string like printf()
/// @param[in] ...: arguments
/// @return number of arguments which processed
int FILEIO::Fprintf(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	int len = vfprintf(fp, format, ap);
	va_end(ap);

	return len;
}

#ifdef _UNICODE
/// @brief Print the formatted string to the opened file
/// @param[in] format: a string like printf()
/// @param[in] ...: arguments
/// @return number of arguments which processed
int FILEIO::Fprintf(const wchar_t *format, ...)
{
	va_list ap;

	va_start(ap, format);
	int len = UTILITY::vftprintf(fp, format, ap);
	va_end(ap);

	return len;
}
#endif

/// @brief Read some data from the opened file
/// @param[out] buffer: a buffer
/// @param[in] size: one data size
/// @param[in] count: number of data
/// @return number of read data
/// @note Buffer size must be (size * count) bytes at least.
size_t FILEIO::Fread(void* buffer, size_t size, size_t count)
{
	return fread(buffer, size, count, fp);
}

/// @brief Write some data from the opened file
/// @param[in] buffer: a buffer
/// @param[in] size: one data size
/// @param[in] count: number of data
/// @return number of wrote data
size_t FILEIO::Fwrite(const void* buffer, size_t size, size_t count)
{
	return fwrite(buffer, size, count, fp);
}

/// @brief Write some data from the opened file
/// @param[in] buffer: a buffer
/// @param[in] size: one data size
/// @param[in] count: number of data
/// @return wrote size (= data size * count)(bytes)
size_t FILEIO::FwriteWithSize(const void* buffer, size_t size, size_t count)
{
	return fwrite(buffer, size, count, fp) * size;
}

/// @brief Seek the opened file
/// @param[in] offset: number of byte from origin 
/// @param[in] origin: based position used in offset
/// @return 0 when success
uint32_t FILEIO::Fseek(long offset, FILEIO_SEEK_ORIGIN origin)
{
	switch(origin) {
	case FILEIO::SEEKCUR:
		return fseek(fp, offset, SEEK_CUR);
	case FILEIO::SEEKEND:
		return fseek(fp, offset, SEEK_END);
	case FILEIO::SEEKSET:
		return fseek(fp, offset, SEEK_SET);
	}
	return 0xFFFFFFFF;
}

/// @brief Tell the position of the opened file
/// @return position
uint32_t FILEIO::Ftell()
{
	return (uint32_t)ftell(fp);
}

/// @brief Flush cached data of the opened file
/// @return 0 when success
int FILEIO::Flush()
{
	return (int)fflush(fp);
}

/// @brief Truncate or expand size of the opened file
/// @param[in] size: truncate or expand size
/// @return 0 when success
int FILEIO::Ftruncate(size_t size)
{
#if defined(_WIN32) && !defined(NO_USE_WINAPI)
	int fd = _fileno(fp);
	return _chsize(fd, (long)size);
//	SetEndOfFile(hFile);
#else
	int fd = fileno(fp);
	return ftruncate(fd, (off_t)size);
#endif
}
