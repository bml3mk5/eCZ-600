/** @file win_filebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ file box ]
*/

#include "win_filebox.h"
#include "win_gui.h"
#include "../../common.h"
#include "../../cchar.h"
#include "../../utility.h"

namespace GUI_WIN
{

FileBox::FileBox(HWND parent_window)
{
	hWnd = parent_window;
	memset(selected_file, 0, sizeof(selected_file));
}

FileBox::~FileBox()
{
}

bool FileBox::Show(const CMsg::Id *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *path)
{
	_TCHAR fil[_MAX_PATH];

	fil[0] = _T('\0');
	if (filter) {
		set_file_filter(filter, fil);
	}
	return show_main(fil, title, dir, ext, save, path);
}

bool FileBox::Show(const char *filter, const _TCHAR *title, const _TCHAR *dir, bool save, _TCHAR *path)
{
	_TCHAR fil[_MAX_PATH];
	_TCHAR ext[16];

	fil[0] = _T('\0');
	if (filter) {
		set_file_filter(filter, save, fil, ext);
	}
	return show_main(fil, title, dir, ext, save, path);
}

bool FileBox::show_main(const _TCHAR *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *path)
{
	_TCHAR fle[_MAX_PATH] = _T("");
	_TCHAR app[_MAX_PATH];

	OPENFILENAME OpenFileName;
	int rc = 0;

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	CTchar ctitle(title);
	OpenFileName.lpstrTitle = ctitle.Get();
	if (filter != NULL && filter[0]) {
		OpenFileName.lpstrFilter = filter;
	}
	if(ext != NULL && ext[0]) {
		OpenFileName.lpstrDefExt = ext;
	}
	if(dir != NULL && dir[0]) {
		UTILITY::tcscpy(app, _MAX_PATH, dir);
		OpenFileName.lpstrInitialDir = app;
	} else if (path != NULL && path[0]) {
		UTILITY::get_dir_and_basename(path, app, fle);
		OpenFileName.lpstrInitialDir = app;
	} else {
		GetModuleFileName(NULL, app, _MAX_PATH);
		UTILITY::get_parent_dir(app);
		OpenFileName.lpstrInitialDir = app;
	}
	OpenFileName.lpstrFile = fle;
	OpenFileName.nMaxFile = _MAX_PATH;

	if (save) {
		OpenFileName.Flags = OFN_OVERWRITEPROMPT;
		rc = GetSaveFileName(&OpenFileName);
	} else {
		OpenFileName.Flags = 0;
		rc = GetOpenFileName(&OpenFileName);
	}
	flags = OpenFileName.Flags;

	if (rc) {
		UTILITY::tcsncpy(selected_file, _MAX_PATH, OpenFileName.lpstrFile, _MAX_PATH);
		if (path) {
			UTILITY::tcscpy(path, _MAX_PATH, selected_file);
		}
		return true;
	}
	return false;
}

const _TCHAR *FileBox::GetPathM() const
{
#if defined(USE_UTF8_ON_MBCS)
	static _TCHAR tfile[_MAX_PATH];
	UTILITY::conv_from_native_path(selected_file, tfile, _MAX_PATH);
#else
	const _TCHAR *tfile = selected_file;
#endif
	return tfile;
}

void FileBox::set_file_filter(const CMsg::Id *filter, _TCHAR *fil_str)
{
	if (filter) {
		int pos = 0;
		fil_str[0] = _T('\0');
		for(int i=0; filter[i] != 0 && filter[i] != CMsg::End; i++) {
			_TCHAR filter1[_MAX_PATH];
			_TCHAR subext[_MAX_PATH];
			UTILITY::conv_to_api_string(CMSGV(filter[i]), _MAX_PATH, filter1, _MAX_PATH);
			UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, filter1);
			pos += (int)_tcslen(filter1);
			pos++;
			if (pos >= _MAX_PATH) break;
			fil_str[pos] = _T('\0');
			if (UTILITY::substr_in_bracket(filter1, subext) <= 0) continue;

			UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, subext);
			pos += (int)_tcslen(subext);
			pos++;
			if (pos >= _MAX_PATH) break;
			fil_str[pos] = _T('\0');
		}
	}
}

/// @param[in] filter : extension string separeted by ";"  ex. "foo;bar;baz"
/// @param[in] save
/// @param[out] fil_str : filter string
/// @param[out] ext : first extension
void FileBox::set_file_filter(const char *filter, bool save, _TCHAR *fil_str, _TCHAR *ext)
{
	ext[0] = _T('\0');
	fil_str[0] = _T('\0');

	if (!filter) return;

	int npos = 0;
	int nlen = (int)strlen(filter);
	char word[8];
	int word_len;

	_TCHAR subexts[_MAX_PATH];
	subexts[0] = _T('\0');
	int ext_nums = 0;

	int pos = 0;
	UTILITY::strcpy(word, 8, "*.");
	if (!save) {
		// load dialog
		do {
			word_len = 0;
			npos = UTILITY::get_token(filter, npos, nlen, &word[2], (int)sizeof(word)-2, ';', &word_len);
			if (word_len > 0) {
#ifdef _UNICODE
				_TCHAR word1[8];
				UTILITY::conv_mbs_to_wcs(word, 8, filter1, 8);
				UTILITY::tcscat(subexts, _MAX_PATH, word1);
#else
				UTILITY::tcscat(subexts, _MAX_PATH, word);
#endif
				if (ext_nums == 0) {
					UTILITY::tcscpy(ext, 8, &subexts[2]);
				}
				if (npos >= 0) {
					UTILITY::tcscat(subexts, _MAX_PATH, _T(";"));
				}
				ext_nums++;
			}
		} while (npos >= 0);

		// "Supported Files (*.foo;*.bar;*.baz)"
		UTILITY::tcscpy(fil_str, _MAX_PATH, CMSGM(Supported_Files));
		UTILITY::tcscat(fil_str, _MAX_PATH, _T(" ("));
		UTILITY::tcscat(fil_str, _MAX_PATH, subexts);
		UTILITY::tcscat(fil_str, _MAX_PATH, _T(")"));
		pos = (int)_tcslen(fil_str);
		fil_str[pos] = _T('\0');
		pos++;
		UTILITY::tcscpy(&fil_str[pos], _MAX_PATH - pos, subexts);
		pos += (int)_tcslen(&fil_str[pos]);
		fil_str[pos] = _T('\0');
		pos++;

	} else {
		// save dialog
		// make filter for each extension
		do {
			word_len = 0;
			npos = UTILITY::get_token(filter, npos, nlen, &word[2], (int)sizeof(word)-2, ';', &word_len);
			if (word_len > 0) {
#ifdef _UNICODE
				_TCHAR word1[8];
				UTILITY::conv_mbs_to_wcs(word, 8, filter1, 8);
				UTILITY::tcscpy(subexts, _MAX_PATH, word1);
#else
				UTILITY::tcscpy(subexts, _MAX_PATH, word);
#endif
				if (ext_nums == 0) {
					UTILITY::tcscpy(ext, 8, &subexts[2]);
				}
				// "foo File (*.foo)"
				UTILITY::tcscpy(&fil_str[pos], _MAX_PATH - pos, &subexts[2]);
				UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, _T(" "));
				UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, CMSGM(File));
				UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, _T(" ("));
				UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, subexts);
				UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, _T(")"));
				pos += (int)_tcslen(&fil_str[pos]);
				fil_str[pos] = _T('\0');
				pos++;
				UTILITY::tcscpy(&fil_str[pos], _MAX_PATH - pos, subexts);
				pos += (int)_tcslen(&fil_str[pos]);
				fil_str[pos] = _T('\0');
				pos++;
				ext_nums++;
			}
		} while (npos >= 0);
	}

	// "All Files (*.*)"
	UTILITY::tcscpy(&fil_str[pos], _MAX_PATH - pos, CMSGM(All_Files));
	UTILITY::tcscat(&fil_str[pos], _MAX_PATH - pos, _T(" (*.*)"));
	pos += (int)_tcslen(&fil_str[pos]);
	fil_str[pos] = _T('\0');
	pos++;
	UTILITY::tcscpy(&fil_str[pos], _MAX_PATH - pos, _T("*.*"));
	pos += (int)_tcslen(&fil_str[pos]);
	fil_str[pos] = _T('\0');
	pos++;
	fil_str[pos] = _T('\0');
	pos++;
}

}; /* namespace GUI_WIN */
