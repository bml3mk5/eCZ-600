/** @file parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.14 -

	@brief [ parse options ]
*/

#include "parseopt.h"
#include "../gui/gui.h"
#include "../config.h"
#include "../utility.h"

CParseOptionsBase::CParseOptionsBase()
{
#ifdef USE_DEBUGGER
	debugger_imm_start = 0;
#endif
	memset(tmp_path_1, 0, sizeof(tmp_path_1));
	memset(tmp_path_2, 0, sizeof(tmp_path_2));
}

CParseOptionsBase::~CParseOptionsBase()
{
	release_buffers();
}

void CParseOptionsBase::allocate_buffers()
{
	tape_file = new CTchar();
#ifdef USE_FD1
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		disk_file[i] = new CTchar();
	}
#endif
#ifdef USE_HD1
	for(int i=0; i<USE_SASI_HARD_DISKS; i++) {
		sasi_hard_disk_file[i] = new CTchar();
	}
#endif
#ifdef USE_HD17
	for(int i=0; i<USE_SCSI_HARD_DISKS; i++) {
		scsi_hard_disk_file[i] = new CTchar();
	}
#endif
	state_file = new CTchar();
	autokey_file = new CTchar();
	reckey_file = new CTchar();
}

void CParseOptionsBase::release_buffers()
{
	delete tape_file; tape_file = NULL;
#ifdef USE_FD1
	for(int i=0; i<USE_FLOPPY_DISKS; i++) {
		delete disk_file[i]; disk_file[i] = NULL;
	}
#endif
#ifdef USE_HD1
	for(int i=0; i<USE_SASI_HARD_DISKS; i++) {
		delete sasi_hard_disk_file[i]; sasi_hard_disk_file[i] = NULL;
	}
#endif
#ifdef USE_HD17
	for(int i=0; i<USE_SCSI_HARD_DISKS; i++) {
		delete scsi_hard_disk_file[i]; scsi_hard_disk_file[i] = NULL;
	}
#endif
	delete state_file; state_file = NULL;
	delete autokey_file; autokey_file = NULL;
	delete reckey_file; reckey_file = NULL;
}

const _TCHAR *CParseOptionsBase::get_app_path()
{
	return app_path.Get();
}

const _TCHAR *CParseOptionsBase::get_app_name()
{
	return app_name.Get();
}

const _TCHAR *CParseOptionsBase::get_ini_path()
{
	return ini_path.Get();
}

const _TCHAR *CParseOptionsBase::get_ini_file()
{
	return ini_file.Get();
}

const _TCHAR *CParseOptionsBase::get_res_path()
{
	return res_path.Get();
}

void CParseOptionsBase::print_usage()
{
	_TCHAR buf[_MAX_PATH];
	UTILITY::stprintf(buf, _MAX_PATH, _T("Usage: %s "), app_name.Get());
	UTILITY::tcscat(buf, _MAX_PATH, _T("[-h] [-i <ini_file>]"));
#ifdef USE_DATAREC
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-t <tape_file>]"));
#endif
#ifdef USE_FD1
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-d <disk_file>]"));
#endif
#ifdef USE_HD1
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-x <hard_disk_file>]"));
#endif
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-s <state_file>]"));
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-a <autokey_file>]"));
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-k <recordkey_file>]"));
#ifdef USE_DEBUGGER
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [-z]"));
#endif
	UTILITY::tcscat(buf, _MAX_PATH, _T(" [<support_file> ...]"));
	_fputts(buf, stdout);
}

void CParseOptionsBase::set_application_path_and_name(const _TCHAR *arg0)
{
	if (get_module_file_name(tmp_path_1, _MAX_PATH)) {
		UTILITY::get_dir_and_basename(tmp_path_1, tmp_path_2);
	} else {
		if (arg0) {
			UTILITY::get_dir_and_basename(arg0, tmp_path_1, tmp_path_2);
		}
	}
	UTILITY::chomp_crlf(tmp_path_1);
	UTILITY::chomp_crlf(tmp_path_2);
	app_path.SetM(tmp_path_1);
	app_name.SetM(tmp_path_2);
}

bool CParseOptionsBase::get_option(const _TCHAR *arg, int &idx, _TCHAR &opt)
{
	if (_tcschr(_T("-/"), arg[0]) != NULL) {
		opt = arg[1];
		if (_tcschr(_T("itdxsak"), opt) != NULL) {
			idx++;
			return true;
		}
#ifdef USE_DEBUGGER
		if (_tcschr(_T("hHz"), opt) != NULL) {
#else
		if (_tcschr(_T("hH"), opt) != NULL) {
#endif
			return true;
		}
	}
	return false;
}

bool CParseOptionsBase::set_file_by_option(_TCHAR opt, const _TCHAR *arg)
{
	bool finished = false;
	switch(opt) {
	case _T('i'):
		ini_file.SetM(arg);
		break;
	case _T('d'):
#ifdef USE_FD1
		for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
			if (disk_file[drv]->Length() <= 0) {
				disk_file[drv]->SetM(arg);
				break;
			}
		}
#endif
		break;
	case _T('x'):
#ifdef USE_HD1
		for(int idx=0; idx<USE_SASI_HARD_DISKS; idx++) {
			if (sasi_hard_disk_file[idx]->Length() <= 0) {
				sasi_hard_disk_file[idx]->SetM(arg);
				break;
			}
		}
#endif
		break;
	case _T('w'):
#ifdef USE_HD17
		for(int idx=0; idx<USE_SCSI_HARD_DISKS; idx++) {
			if (scsi_hard_disk_file[idx]->Length() <= 0) {
				scsi_hard_disk_file[idx]->SetM(arg);
				break;
			}
		}
#endif
		break;
	case _T('t'):
		tape_file->SetM(arg);
		break;
	case _T('s'):
		state_file->SetM(arg);
		break;
	case _T('a'):
		autokey_file->SetM(arg);
		break;
	case _T('k'):
		reckey_file->SetM(arg);
		break;
#ifdef USE_DEBUGGER
	case _T('z'):
		debugger_imm_start = 1;
		break;
#endif
	case _T('h'):
	case _T('H'):
	case _T('?'):
		print_usage();
		exit(0);
		break;
	default:
		finished = true;
		break;
	}
	return finished;
}

int CParseOptionsBase::check_supported_file_by_extension(const _TCHAR *path)
{
	int rc = GUI::CheckSupportedFile(path);
	switch(rc) {
	case GUI_BASE::FILE_TYPE_DATAREC:
		// tape file
		tape_file->SetM(path);
		break;
	case GUI_BASE::FILE_TYPE_FLOPPY:
#ifdef USE_FD1
		for(int drv=0; drv<USE_FLOPPY_DISKS; drv++) {
			if (disk_file[drv]->Length() <= 0) {
				disk_file[drv]->SetM(path);
				break;
			}
		}
#endif
		break;
	case GUI_BASE::FILE_TYPE_SASI_HARD_DISK:
#ifdef USE_HD1
		for(int idx=0; idx<USE_SASI_HARD_DISKS; idx++) {
			if (sasi_hard_disk_file[idx]->Length() <= 0) {
				sasi_hard_disk_file[idx]->SetM(path);
				break;
			}
		}
#endif
	case GUI_BASE::FILE_TYPE_SCSI_HARD_DISK:
#ifdef USE_HD17
		for(int idx=0; idx<USE_SCSI_HARD_DISKS; idx++) {
			if (scsi_hard_disk_file[idx]->Length() <= 0) {
				scsi_hard_disk_file[idx]->SetM(path);
				break;
			}
		}
#endif
		break;
	case GUI_BASE::FILE_TYPE_STATE:
		state_file->SetM(path);
		break;
	case GUI_BASE::FILE_TYPE_AUTO_KEY:
		autokey_file->SetM(path);
		break;
	case GUI_BASE::FILE_TYPE_INITIALIZE:
		ini_file.SetM(path);
		break;
	case GUI_BASE::FILE_TYPE_KEY_RECORD:
		reckey_file->SetM(path);
		break;
	default:
		break;
	}
	return rc;
}

void CParseOptionsBase::remake_ini_file_path()
{
	if (ini_file.Length() > 0) {
		// specified ini file 
		if (UTILITY::check_file_extension(ini_file.Get(), _T("ini"))) {
			_TCHAR ini_name[_MAX_PATH];
			// path1: path existing ini file
			UTILITY::get_dir_and_basename(ini_file.Get(), tmp_path_1, ini_name);
		}
		UTILITY::add_path_separator(tmp_path_1);
	} else {
		// default
		UTILITY::tcscpy(tmp_path_1, _MAX_PATH, app_path.Get());
		UTILITY::tcscpy(tmp_path_2, _MAX_PATH, tmp_path_1);
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(CONFIG_NAME));
		UTILITY::tcscat(tmp_path_2, _MAX_PATH, _T(".ini"));
		ini_file.Set(tmp_path_2);
	}
	ini_path.Set(tmp_path_1);
}

/**
 *	open recent file
 */
void CParseOptionsBase::open_recent_file(GUI *gui)
{
#ifdef USE_FD1
	_TCHAR path[_MAX_PATH];
#endif

	if (gui) {
#ifdef USE_DATAREC
		if (tape_file->Length() > 0) {
			gui->PostEtLoadDataRecMessage(tape_file->Get());
		}
#endif
#ifdef USE_FD1
		// auto open recent file
		for(int drv=(USE_FLOPPY_DISKS-1); drv>=0; drv--) {
			path[0] = _T('\0');
			int bank_num = 0;
			if (disk_file[drv]->Length() > 0 && disk_file[drv]->MatchString(path) != 0) {
				// specified file in the option
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, disk_file[drv]->Get());
			} else if ((pConfig->mount_fdd & (1 << drv)) != 0 && pConfig->GetRecentFloppyDiskPathCount(drv) > 0 && pConfig->GetRecentFloppyDiskPathLength(drv, 0) > 0) {
				// recent file
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, pConfig->GetRecentFloppyDiskPathString(drv, 0));
				bank_num = pConfig->GetRecentFloppyDiskPathNumber(drv, 0);
			}
			if (path[0] != _T('\0')) {
				gui->PostEtOpenFloppyMessage(drv, path, bank_num, 0, false);
			}
		}
#endif
#ifdef USE_HD1
		// auto open recent file
		for(int drv=(USE_SASI_HARD_DISKS-1); drv>=0; drv--) {
			int idx = drv;
			path[0] = _T('\0');
			if (sasi_hard_disk_file[idx]->Length() > 0 && sasi_hard_disk_file[idx]->MatchString(path) != 0) {
				// specified file in the option
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, sasi_hard_disk_file[idx]->Get());
			} else if ((pConfig->mount_hdd & (1 << drv)) != 0 && pConfig->GetRecentHardDiskPathCount(drv) > 0 && pConfig->GetRecentHardDiskPathLength(drv, 0) > 0) {
				// recent file
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, pConfig->GetRecentHardDiskPathString(drv, 0));
//				bank_num = pConfig->recent_hard_disk_path[drv][0]->num;
			}
			if (path[0] != _T('\0')) {
				gui->PostEtOpenHardDiskMessage(drv, path, 0);
			}
		}
#endif
#ifdef USE_HD17
		// auto open recent file
		for(int drv=(MAX_SASI_HARD_DISKS+USE_SCSI_HARD_DISKS-1); drv>=MAX_SASI_HARD_DISKS; drv--) {
			int idx = TO_SCSI_DRIVE(drv);
			path[0] = _T('\0');
			if (scsi_hard_disk_file[idx]->Length() > 0 && scsi_hard_disk_file[idx]->MatchString(path) != 0) {
				// specified file in the option
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, scsi_hard_disk_file[idx]->Get());
			} else if ((pConfig->mount_hdd & (1 << drv)) != 0 && pConfig->GetRecentHardDiskPathCount(drv) > 0 && pConfig->GetRecentHardDiskPathLength(drv, 0) > 0) {
				// recent file
				path[0] = _T('\0');
				UTILITY::tcscpy(path, _MAX_PATH, pConfig->GetRecentHardDiskPathString(drv, 0));
//				bank_num = pConfig->recent_hard_disk_path[drv][0]->num;
			}
			if (path[0] != _T('\0')) {
				gui->PostEtOpenHardDiskMessage(drv, path, 0);
			}
		}
#endif
		if (state_file->Length() > 0) {
			gui->PostEtLoadStatusMessage(state_file->Get());
		}
		if (autokey_file->Length() > 0) {
			gui->PostEtLoadAutoKeyMessage(autokey_file->Get());
		}
		if (reckey_file->Length() > 0) {
			gui->PostEtLoadRecKeyMessage(reckey_file->Get());
		}
#ifdef USE_DEBUGGER
		if (debugger_imm_start || pConfig->debugger_imm_start) {
			gui->PostMtOpenDebugger();
		}
#endif
	}

	// release buffer
	release_buffers();
}
