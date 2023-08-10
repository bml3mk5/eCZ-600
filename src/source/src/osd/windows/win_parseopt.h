/** @file win_parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#ifndef WIN_PARSE_OPT_H
#define WIN_PARSE_OPT_H

#include "../../common.h"
#include "../../cchar.h"
#include "../../vm/vm_defs.h"
#include "../parseopt.h"
#include <windows.h>

/// @brief parse command line options
class CParseOptions : public CParseOptionsBase
{
private:
	int get_options(LPTSTR szCmdLine);

	bool get_module_file_name(_TCHAR *path, int size);

	CParseOptions();
	CParseOptions(const CParseOptions &);

public:
	CParseOptions(LPTSTR szCmdLine);
	~CParseOptions();
};

#endif /* WIN_PARSE_OPT_H */