/** @file parseopt.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.14 -

	@brief [ parse options ]
*/

#ifndef PARSE_OPT_H
#define PARSE_OPT_H

#include "../common.h"
#include "../cchar.h"
#include "../vm/vm_defs.h"

class GUI;

/// @brief parse command line options
class CParseOptionsBase
{
protected:
	CTchar app_path;
	CTchar app_name;
	CTchar ini_path;
	CTchar ini_file;
	CTchar res_path;

	CTchar *tape_file;
#ifdef USE_FD1
	CTchar *disk_file[USE_FLOPPY_DISKS];
#endif
#ifdef USE_HD1
	CTchar *sasi_hard_disk_file[USE_SASI_HARD_DISKS];
#endif
#ifdef USE_HD17
	CTchar *scsi_hard_disk_file[USE_SCSI_HARD_DISKS];
#endif
	CTchar *state_file;
	CTchar *autokey_file;
	CTchar *reckey_file;
#ifdef USE_DEBUGGER
	int debugger_imm_start;
#endif

	_TCHAR tmp_path_1[_MAX_PATH];
	_TCHAR tmp_path_2[_MAX_PATH];

	CParseOptionsBase(const CParseOptionsBase &) {}
	CParseOptionsBase &operator=(const CParseOptionsBase &) { return *this; }

	void allocate_buffers();
	void release_buffers();

	void print_usage();

	virtual bool get_module_file_name(_TCHAR *path, int size) = 0;

	void set_application_path_and_name(const _TCHAR *arg0);

	bool get_option(const _TCHAR *arg, int &idx, _TCHAR &opt);

	bool set_file_by_option(_TCHAR opt, const _TCHAR *arg);
	int check_supported_file_by_extension(const _TCHAR *path);

	void remake_ini_file_path();

public:
	CParseOptionsBase();
	virtual ~CParseOptionsBase();

	void open_recent_file(GUI *gui);

	const _TCHAR *get_app_path();
	const _TCHAR *get_app_name();
	const _TCHAR *get_ini_path();
	const _TCHAR *get_ini_file();
	const _TCHAR *get_res_path();
};

#endif /* PARSE_OPT_H */
