/** @file logging.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.02.18 -

	@brief [ logging ]
*/

#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
//#include "fileio.h"
//#include "cmutex.h"
#include "cchar.h"
#include "msgs.h"

class FILEIO;
class CMutex;

// for log
// #define _LOG_CONSOLE
#define _LOG_FILE

/// @ingroup Enums
/// Logging level (for use Logging class)
enum LoggingLevel {
	LOG_INFO =	0,	///< Information
	LOG_WARN =	1,	///< Warning
	LOG_ERROR =	2,	///< Error
	LOG_DEBUG =	3	///< Debug
};

// for debug
//#define _DEBUG_LOG
#ifdef _DEBUG_LOG
	// output debug log to console
//	#define _DEBUG_CONSOLE
	// output debug log to file
	#define _DEBUG_FILE

	// output cpu debug log
//	#define _CPU_DEBUG_LOG
	// output fdc debug log
//	#define _FDC_DEBUG_LOG
	// output i/o debug log
//	#define _IO_DEBUG_LOG
#endif

/**
	@brief Callback class to receive a log message
*/
class LogMessageReceiver
{
public:
	LogMessageReceiver() {}
	virtual ~LogMessageReceiver() {}

	virtual void send_log_message(int level, const char *levelstr, const char *msg) {}
#ifdef _UNICODE
	virtual void send_log_message(int level, const wchar_t *levelstr, const wchar_t *msg) {}
#endif
};

/**
	@brief Record any messages to file
*/
class Logging
{
private:
#ifdef _LOG_FILE
	FILEIO *fplog;
	CTchar  m_log_path;
#endif
	LogMessageReceiver *recv;
	CMutex *mux;

	void out_log_real(int level, const char* buffer);
#ifdef _UNICODE
	void out_log_real(int level, const wchar_t* buffer);
#endif

public:
	Logging();
	Logging(const _TCHAR *path);
	virtual ~Logging();

	void open_logfile(const _TCHAR *path);
	void close_logfile();

	void set_receiver(LogMessageReceiver *src) { recv = src; }

	const _TCHAR *get_log_path() const;

	// for narrow char
	virtual void out_log(int level, const char* msg);
	virtual void out_logf(int level, const char* format, ...);
	virtual void out_logc(int level, const char* msg, ...);
	virtual void out_logv(int level, const char* format, va_list ap);
	// for narrow char and translate locale
	virtual void out_log_x(int level, const char* msg);
	virtual void out_logf_x(int level, const char* format, ...);
	virtual void out_logv_x(int level, const char* format, va_list ap);
	// for id and translate locale
	virtual void out_log_x(int level, CMsg::Id id);
	virtual void out_logf_x(int level, CMsg::Id id, ...);
	virtual void out_logv_x(int level, CMsg::Id id, va_list ap);
	// debug log
	virtual void out_debug(const char* msg);
	virtual void out_debugf(const char* format, ...);
	// system error log
	virtual void out_syserrlog(int level, int err_num, const char* premsg);

	// get log
	virtual int get_log(char *buffer, int buffer_size);

#ifdef _UNICODE

	virtual void out_log(int level, const wchar_t* msg);
	virtual void out_logf(int level, const wchar_t* format, ...);
	virtual void out_logc(int level, const wchar_t* msg, ...);
	virtual void out_logv(int level, const wchar_t* format, va_list ap);
	// translate locale
	virtual void out_log_x(int level, const wchar_t* msg);
	virtual void out_logf_x(int level, const wchar_t* format, ...);
	virtual void out_logv_x(int level, const wchar_t* format, va_list ap);
	/// debug log
	virtual void out_debug(const wchar_t* msg);
	virtual void out_debugf(const wchar_t* format, ...);
	// system error log
	virtual void out_syserrlog(int level, int err_num, const wchar_t* premsg);

	// get log
	virtual int get_log(wchar_t *buffer, int buffer_size);

#endif

	/// suppress debug log
	void dummyf(const void *format, ...) {}
};

extern Logging *logging;

#endif /* LOGGING_H */
