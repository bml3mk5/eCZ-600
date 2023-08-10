/** @file win_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "win_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../utility.h"

CParseOptions::CParseOptions()
	: CParseOptionsBase()
{
}

CParseOptions::CParseOptions(const CParseOptions &src)
	: CParseOptionsBase(src)
{
}

CParseOptions::CParseOptions(LPTSTR szCmdLine)
	: CParseOptionsBase()
{
	get_options(szCmdLine);
}

CParseOptions::~CParseOptions()
{
}

/**
 *	get and parse options
 */
int CParseOptions::get_options(LPTSTR szCmdLine)
{
	_TCHAR *optarg[10];
	int     optargc = 0;

	// allocate buffer
	allocate_buffers();

	// split options
//	bool quoting = false;
	_TCHAR cmd_line[_MAX_PATH];
	UTILITY::tcscpy(cmd_line, _MAX_PATH, (const _TCHAR *)szCmdLine);
	optargc = UTILITY::get_parameters(cmd_line, _MAX_PATH, optarg, 10);

	// set application path and name
	set_application_path_and_name(NULL);

	// parse options
	int optind = 0;
	_TCHAR opt = _T('\0');
	bool finished = false;
	for(; optind < optargc; optind++) {
		get_option(optarg[optind], optind, opt);
		finished = set_file_by_option(opt, optarg[optind]);
		opt = _T('\0');
		if (finished) break;
	}
	for(; optind < optargc && optarg[optind][0] != _T('\0'); optind++) {
		check_supported_file_by_extension(optarg[optind]);
	}

	// check ini file
	remake_ini_file_path();

	return 0;
}

bool CParseOptions::get_module_file_name(_TCHAR *path, int size)
{
	return (::GetModuleFileName(NULL, path, size) > 0);
}
