/** @file logging.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.02.18 -

	@brief [ logging ]
*/

#include "../logging.h"
#include "../vm/vm.h"
#include "../fileio.h"
#include "../cmutex.h"
#include "../utility.h"
#include "../clocale.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#elif defined(USE_WX) || defined(USE_WX2)
#include <wx/datetime.h>
#elif defined(USE_QT)
#include "qt/qt_utils.h"
#include <QString>
#include <QDateTime>
#include <QVector>
#endif

#define MESSAGE_BUFFER 1024

Logging::Logging()
{
#ifdef _LOG_FILE
	fplog = NULL;
#endif
	recv = NULL;
	mux = new CMutex();
}

Logging::Logging(const _TCHAR *path)
{
#ifdef _LOG_FILE
	fplog = NULL;
#endif
	recv = NULL;
	mux = new CMutex();

	open_logfile(path);
}

Logging::~Logging()
{
	close_logfile();

	delete mux;
}

/// Open a log file.
///
/// @param[in] path: file path
void Logging::open_logfile(const _TCHAR *path)
{
#ifdef _LOG_CONSOLE_WINPRT
	AllocConsole();
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle("Debug Log");
#endif
#ifdef _LOG_FILE
	// get application path
	m_log_path.Clear();
	fplog = new FILEIO();
	if (fplog) {
		_TCHAR file_path[_MAX_PATH];
		UTILITY::concat(file_path, _MAX_PATH, path, _T(CONFIG_NAME), _T(".log"), NULL);
		if (!fplog->Fopen(file_path, FILEIO::READ_WRITE_NEW_ASCII)) {
			delete fplog;
			fplog = NULL;
		} else {
			m_log_path.Set(file_path);
		}
	}
#endif
}

/// Close a log file.
void Logging::close_logfile()
{
#ifdef _LOG_CONSOLE_WINPRT
	FreeConsole();
#endif
#ifdef _LOG_FILE
	mux->lock();
	if(fplog) {
		delete fplog;
		fplog = NULL;
	}
	m_log_path.Clear();
	mux->unlock();
#endif
}

/// path of log file
const _TCHAR *Logging::get_log_path() const
{
#ifdef _LOG_FILE
	return m_log_path.Get();
#else
	return NULL;
#endif
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] buffer: formatted message
void Logging::out_log_real(int level, const char* buffer)
{
#if defined(_UNICODE) && !defined(_MSC_VER)
	// always use wide character
	CWchar wbuffer(buffer);

	out_log_real(level, wbuffer.Get());
#else

	const char *levelstr;

	switch(level) {
	case LOG_WARN:
		levelstr = "WARN";
		break;
	case LOG_ERROR:
		levelstr = "ERROR";
		break;
	case LOG_DEBUG:
		levelstr = "DEBUG";
		break;
	default:
		levelstr = "INFO";
		break;
	}

	mux->lock();
#ifdef _LOG_CONSOLE_WINPRT
	DWORD dwWritten;
	WriteConsoleA(hConsole, buffer, strlen(buffer), &dwWritten, NULL);
#endif
#ifdef _LOG_STD_CONSOLE
	fprintf(stderr, "%s: %s\n", levelstr, buffer);
	fflush(stderr);
#endif

	if (recv) recv->send_log_message(level, levelstr, buffer);

#ifdef _LOG_FILE
	if(fplog) {
#if defined(USE_WIN)
		SYSTEMTIME st;
		GetLocalTime(&st);
		char nbuffer[MESSAGE_BUFFER];
		UTILITY::conv_mbs_to_utf8(buffer, MESSAGE_BUFFER, nbuffer, MESSAGE_BUFFER);
		fplog->Fprintf("%04d-%02d-%02d %02d:%02d:%02d.%03d"
			, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
		);
#elif defined(USE_SDL) || defined(USE_SDL2)
		const char *nbuffer = buffer;
		fplog->Fprintf("%10d", SDL_GetTicks());
#elif defined(USE_WX) || defined(USE_WX2)
		const char *nbuffer = buffer;
		wxDateTime dt = wxDateTime::Now();
		fplog->Fprintf("%04d-%02d-%02d %02d:%02d:%02d.%03d"
			, dt.GetYear(), (int)dt.GetMonth() + 1, (int)dt.GetDay(), (int)dt.GetHour(), (int)dt.GetMinute(), (int)dt.GetSecond(), (int)dt.GetMillisecond()
		);
#elif defined(USE_QT)
		const char *nbuffer = buffer;
		QDateTime dt = QDateTime::currentDateTime();
		fplog->Fputs(dt.toString("yyyy-MM-dd HH:mm:ss.zzz").toUtf8().data());
#endif
		fplog->Fprintf(" %-5s: ", levelstr);
		fplog->Fputs(nbuffer);
		fplog->Fputs("\n");
		fplog->Flush();
	}
#endif
	mux->unlock();
#endif /* _UNICODE */
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ap: values
void Logging::out_logv(int level, const char* format, va_list ap)
{
	char buffer[MESSAGE_BUFFER];

//	if (ap) {
#if defined(USE_QT)
		UTILITY::strcpy(buffer, MESSAGE_BUFFER, QString::vasprintf(format, ap).toUtf8().data());
#elif (!defined(USE_WIN) && defined(_WIN32) && !defined(__MINGW32__))
		UTILITY::vsprintf_utf8(buffer, MESSAGE_BUFFER, format, ap);
#else
		UTILITY::vsprintf(buffer, MESSAGE_BUFFER, format, ap);
#endif
//	} else {
//		UTILITY::strcpy(buffer, MESSAGE_BUFFER, format);
//	}
	out_log_real(level, buffer);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_logf(int level, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	out_logv(level, format, ap);
	va_end(ap);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
/// @param[in] ...: messages
/// @attention Last argument must be set NULL.
void Logging::out_logc(int level, const char* msg, ...)
{
	va_list ap;
	char dst[MESSAGE_BUFFER];

	va_start(ap, msg);
	UTILITY::concatv(dst, MESSAGE_BUFFER, msg, ap);
	out_log_real(level, dst);
	va_end(ap);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
void Logging::out_log(int level, const char* msg)
{
	out_log_real(level, msg);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ap: values
void Logging::out_logv_x(int level, const char* format, va_list ap)
{
    out_logv(level, clocale->GetText(format), ap);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_logf_x(int level, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
    out_logv(level, clocale->GetText(format), ap);
	va_end(ap);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
void Logging::out_log_x(int level, const char* msg)
{
    out_log_real(level, clocale->GetText(msg));
}

/// Output a message to a file. Id is converted to translated message.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] id: message id (CMsg)
/// @param[in] ap: values
void Logging::out_logv_x(int level, CMsg::Id id, va_list ap)
{
	out_logv(level, gMessages.Get(id), ap);
}

/// Output a message to a file. Id is converted to translated message.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] id: message id (CMsg)
/// @param[in] ...: values
void Logging::out_logf_x(int level, CMsg::Id id, ...)
{
	va_list ap;

	va_start(ap, id);
	out_logv_x(level, id, ap);
	va_end(ap);
}

/// Output a message to a file. Id is converted to translated message.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] id: message id (CMsg)
void Logging::out_log_x(int level, CMsg::Id id)
{
	out_log_real(level, gMessages.Get(id));
}

/// Output a debug message to a file.
///
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_debugf(const char* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;

	va_start(ap, format);
	out_logv(LOG_DEBUG, format, ap);
	va_end(ap);
#endif
}

/// Output a debug message to a file.
///
/// @param[in] msg: message string
void Logging::out_debug(const char* msg)
{
#ifdef _DEBUG_LOG
	out_log(LOG_DEBUG, msg);
#endif
}

/// Output system error message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] err_num: error number
/// @param[in] premsg: prefix of message
void Logging::out_syserrlog(int level, int err_num, const char* premsg)
{
	char msg[MESSAGE_BUFFER];
	UTILITY::strcpy(msg, MESSAGE_BUFFER, premsg);
#if defined(USE_WIN)
	UTILITY::strcat(msg, MESSAGE_BUFFER, " ");
	int len = (int)strlen(msg);
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		err_num, LANGIDFROMLCID(clocale->GetLcid()), &msg[len], MESSAGE_BUFFER - len, NULL) == 0) {
		UTILITY::sprintf(&msg[len], MESSAGE_BUFFER, "%d", err_num);
	}
	for(int i=(int)strlen(msg)-1; i>=0; i--) {
		if (msg[i] != ' ' && msg[i] != '\t' && msg[i] != '\r' && msg[i] != '\n') break;
		msg[i]='\0';
	}
#endif
	out_log_real(level, msg);
}

/// Get log to buffer
///
/// @param[out] buffer
/// @param[in] buffer_size
int Logging::get_log(char *buffer, int buffer_size)
{
	int line = 0;

#ifdef _LOG_FILE
	if(!fplog) return line;

#if defined(_WIN32) || defined(__MINGW32__)
	char smsg[MESSAGE_BUFFER + 2];
#if defined(USE_SDL) || defined(USE_SDL2)
	char dmsg[MESSAGE_BUFFER + 2];
#else
	char *dmsg = smsg;
#endif
#else
	char smsg[MESSAGE_BUFFER + 2];
	char *dmsg = smsg;
#endif

	mux->lock();

	int pos = (int)fplog->Ftell();
	if (pos < buffer_size) {
		pos = 0;
	} else {
		pos -= buffer_size;
	}

	fplog->Fseek(pos, FILEIO::SEEKSET);
	if (pos > 0) fplog->Fgets(smsg, MESSAGE_BUFFER);

	for(line = 0; ; line++) {
		if (!fplog->Fgets(smsg, MESSAGE_BUFFER)) {
			break;
		}
		UTILITY::rtrim(smsg, "\r\n");

#if defined(_WIN32) || defined(__MINGW32__)
		UTILITY::strcat(smsg, MESSAGE_BUFFER + 2, "\r\n");
#if defined(USE_SDL) || defined(USE_SDL2)
		UTILITY::conv_utf8_to_mbs(smsg, MESSAGE_BUFFER + 2, dmsg, MESSAGE_BUFFER + 2); 
#endif
#else
		UTILITY::strcat(smsg, MESSAGE_BUFFER + 2, "\n");
#endif
		UTILITY::strcat(buffer, buffer_size, dmsg);
		line++;
	}
	fplog->Fseek(0, FILEIO::SEEKEND);

	mux->unlock();
#endif

	return line;
}

#ifdef _UNICODE

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] buffer: formatted message
void Logging::out_log_real(int level, const wchar_t* buffer)
{
	const wchar_t *levelstr;

	switch(level) {
	case LOG_WARN:
		levelstr = L"WARN";
		break;
	case LOG_ERROR:
		levelstr = L"ERROR";
		break;
	case LOG_DEBUG:
		levelstr = L"DEBUG";
		break;
	default:
		levelstr = L"INFO";
		break;
	}

	mux->lock();
#ifdef _LOG_CONSOLE_WINPRT
	DWORD dwWritten;
	WriteConsoleW(hConsole, buffer, wcslen(buffer), &dwWritten, NULL);
#endif
#ifdef _LOG_CONSOLE_STDERR
	wprintf(stderr, L"%ls: %ls\n", levelstr, buffer);
	fflush(stderr);
#endif

	if (recv) recv->send_log_message(level, levelstr, buffer);

#ifdef _LOG_FILE
	if(fplog) {
#if defined(USE_WIN)
		SYSTEMTIME st;
		GetLocalTime(&st);
		fplog->Fprintf(L"%04d-%02d-%02d %02d:%02d:%02d.%03d"
			, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
		);
#elif defined(USE_SDL) || defined(USE_SDL2)
		fplog->Fprintf(L"%10d", SDL_GetTicks());
#elif defined(USE_WX) || defined(USE_WX2)
		wxDateTime dt = wxDateTime::Now();
		fplog->Fprintf(L"%04d-%02d-%02d %02d:%02d:%02d.%03d"
			, dt.GetYear(), (int)dt.GetMonth() + 1, (int)dt.GetDay(), (int)dt.GetHour(), (int)dt.GetMinute(), (int)dt.GetSecond(), (int)dt.GetMillisecond()
		);
#elif defined(USE_QT)
		QDateTime dt = QDateTime::currentDateTime();
		fplog->Fputs(dt.toString("yyyy-MM-dd HH:mm:ss.zzz").toUtf8().data());
#endif
		fplog->Fprintf(L" %-5ls: ", levelstr);
#if defined(_WIN32) && !defined(USE_QT)
		char cbuffer[MESSAGE_BUFFER];
		UTILITY::conv_wcs_to_utf8(buffer, MESSAGE_BUFFER, cbuffer, MESSAGE_BUFFER);
		fplog->Fputs(cbuffer);
#else
		fplog->Fputs(buffer);
#endif
		fplog->Fputs(L"\n");
		fplog->Flush();
	}
#endif
	mux->unlock();
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ap: values
void Logging::out_logv(int level, const wchar_t* format, va_list ap)
{
	wchar_t buffer[MESSAGE_BUFFER];

//	if (ap) {
#if defined(USE_QT)
		QString::vasprintf(format, ap).toWCharArray(buffer);
#else
		UTILITY::vstprintf(buffer, MESSAGE_BUFFER, format, ap);
#endif
//	} else {
//		wcscpy(buffer, format);
//	}
	out_log_real(level, buffer);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_logf(int level, const wchar_t* format, ...)
{
	va_list ap;

	va_start(ap, format);
	out_logv(level, format, ap);
	va_end(ap);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
/// @param[in] ...: messages
/// @attention Last argument must be set NULL.
void Logging::out_logc(int level, const wchar_t* msg, ...)
{
	va_list ap;
	wchar_t dst[MESSAGE_BUFFER];

	va_start(ap, msg);
	UTILITY::concatv(dst, MESSAGE_BUFFER, msg, ap);
	out_log_real(level, dst);
	va_end(ap);
}

/// Output a message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
void Logging::out_log(int level, const wchar_t* msg)
{
	out_log_real(level, msg);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ap: values
void Logging::out_logv_x(int level, const wchar_t* format, va_list ap)
{
	out_logv(level, wgettext(format), ap);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_logf_x(int level, const wchar_t* format, ...)
{
	va_list ap;

	va_start(ap, format);
	out_logv(level, wgettext(format), ap);
	va_end(ap);
}

/// Output a message to a file. Message is translated by locale setting.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] msg: message string
void Logging::out_log_x(int level, const wchar_t* msg)
{
	out_log_real(level, wgettext(msg));
}

/// Output a debug message to a file.
///
/// @param[in] format: formatted string like printf()
/// @param[in] ...: values
void Logging::out_debugf(const wchar_t* format, ...)
{
#ifdef _DEBUG_LOG
	va_list ap;

	va_start(ap, format);
	out_logv(LOG_DEBUG, format, ap);
	va_end(ap);
#endif
}

/// Output a debug message to a file.
///
/// @param[in] msg: message string
void Logging::out_debug(const wchar_t* msg)
{
#ifdef _DEBUG_LOG
	out_log(LOG_DEBUG, msg);
#endif
}

/// Output system error message to a file.
///
/// @param[in] level: @ref LoggingLevel
/// @param[in] err_num: error number
/// @param[in] premsg: prefix of message
void Logging::out_syserrlog(int level, int err_num, const wchar_t* premsg)
{
	wchar_t msg[MESSAGE_BUFFER];
	wcscpy(msg, premsg);
#if defined(USE_WIN)
	wcscat(msg, L" ");
	int len = (int)wcslen(msg);
	if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		err_num, LANGIDFROMLCID(clocale->GetLcid()), &msg[len], MESSAGE_BUFFER - len, NULL) == 0) {
		wsprintf(&msg[len], L"%d", err_num);
	}
	for(int i=(int)wcslen(msg)-1; i>=0; i--) {
		if (msg[i] != L' ' && msg[i] != L'\t' && msg[i] != L'\r' && msg[i] != L'\n') break;
		msg[i]=L'\0';
	}
#endif
	out_log_real(level, msg);
}

/// Get log to buffer
///
/// @param[out] buffer
/// @param[in] buffer_size
int Logging::get_log(wchar_t *buffer, int buffer_size)
{
	int line = 0;

#ifdef _LOG_FILE
	if(!fplog) return line;

	char smsg[MESSAGE_BUFFER + 2];
	wchar_t dmsg[MESSAGE_BUFFER + 2];

	mux->lock();

	int pos = (int)fplog->Ftell();
	if (pos < buffer_size) {
		pos = 0;
	} else {
		pos -= buffer_size;
	}

	fplog->Fseek(pos, FILEIO::SEEKSET);
	if (pos > 0) fplog->Fgets(smsg, MESSAGE_BUFFER);

	for(line = 0; ; line++) {
		if (!fplog->Fgets(smsg, MESSAGE_BUFFER)) {
			break;
		}
		UTILITY::rtrim(smsg, "\r\n");

		UTILITY::strcat(smsg, MESSAGE_BUFFER + 2, "\r\n");
		UTILITY::conv_utf8_to_wcs(smsg, MESSAGE_BUFFER + 2, dmsg, MESSAGE_BUFFER + 2);
		UTILITY::wcscat(buffer, buffer_size, dmsg);
		line++;
	}
	fplog->Fseek(0, FILEIO::SEEKEND);

	mux->unlock();
#endif

	return line;
}
#endif /* _UNICODE */

/// set in main
Logging *logging = NULL;
