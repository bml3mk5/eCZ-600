/** @file sdl_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "sdl_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../utility.h"
//#include "sdl_main.h"
#include "../../depend.h"
//#ifndef _MSC_VER
//#include <getopt.h>
//#endif

CParseOptions::CParseOptions()
	: CParseOptionsBase()
{
}

CParseOptions::CParseOptions(const CParseOptions &src)
	: CParseOptionsBase(src)
{
}

CParseOptions::CParseOptions(int ac, char *av[])
	: CParseOptionsBase()
{
	get_options(ac, av);
}

CParseOptions::~CParseOptions()
{
}

/**
 *	get and parse options
 */
int CParseOptions::get_options(int ac, char *av[])
{
#ifdef _DEBUG
	for(int i=0; i<ac; i++) {
		printf("%d %s\n",i,av[i]);
	}
#endif

	// allocate buffer
	allocate_buffers();

	// set application path and name
	_TCHAR **w_av;
#ifdef _UNICODE
	w_av = new _TCHAR*[ac];
	for(int i=0; i<ac; i++) {
		w_av[i] = new _TCHAR[_MAX_PATH];
		UTILITY::conv_mbs_to_wcs(av[i], (int)strlen(av[i]), w_av[i], _MAX_PATH);
	}
#else
	w_av = av;
#endif
	set_application_path_and_name(w_av[0]);

	// resource path
	UTILITY::tcsncat(tmp_path_1, _MAX_PATH, _T(RESDIR), _MAX_PATH);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	res_path.SetM(tmp_path_2);

	int optind = 1;
#if defined(__APPLE__) && defined(__MACH__)
	// When mac, app_path set upper app folder (ex. foo/bar/baz.app/../)
	UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, app_path.Get(), _MAX_PATH);
	UTILITY::add_path_separator(tmp_path_1);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	UTILITY::get_ancestor_dir(tmp_path_2, 3);
	app_path.Set(tmp_path_2);
	// Xcode set debug options
	if (ac >= 3 && strstr(av[1],"-NSDocument")) {
		optind += 2;
	} else if (ac >= 2 && strstr(av[1],"-psn")) {
		optind += 1;
	}
#endif

#if 0
	int opt = 0;
#ifdef USE_DEBUGGER
	while ((opt = getopt(ac, av, "hzp:a:d:x:i:s:t:k:")) != -1) {
#else
	while ((opt = getopt(ac, av, "hp:a:d:x:i:s:t:k:")) != -1) {
#endif
		///
		set_file_by_option(opt, optarg);
	}
#else
	// parse options
	_TCHAR opt = _T('\0');
	bool finished = false;
	for(; optind < ac; optind++) {
		get_option(w_av[optind], optind, opt);
		finished = set_file_by_option(opt, w_av[optind]);
		opt = _T('\0');
		if (finished) break;
	}
#endif
	for (;optind < ac && av[optind][0] != '\0'; optind++) {
		check_supported_file_by_extension(w_av[optind]);
	}

	// check ini file
	remake_ini_file_path();

#ifdef _UNICODE
	for(int i=0; i<ac; i++) {
		delete w_av[i];
	}
	delete w_av;
#endif

	return 0;
}

bool CParseOptions::get_module_file_name(_TCHAR *path, int size)
{
#if defined(_WIN32)
	return (::GetModuleFileName(NULL, path, size) > 0);
#else
	return false;
#endif
}
