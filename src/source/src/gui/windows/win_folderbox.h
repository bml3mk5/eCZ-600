/** @file win_folderbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ folder box ]
*/

#ifndef WIN_FOLDERBOX_H
#define WIN_FOLDERBOX_H

#include <Windows.h>
#include <Shlobj.h>
#include <tchar.h>

namespace GUI_WIN
{

/**
	@brief Folder dialog box
*/
class FolderBox
{
private:
	HWND hWnd;

	static int CALLBACK ProcSHBrowseForFolder(HWND, UINT, LPARAM, LPARAM);

public:
	FolderBox(HWND parent_window);
	~FolderBox();
	bool Show(const _TCHAR *title, _TCHAR *path, size_t len);
	bool ShowSHBrowseForFolder(const _TCHAR *title, _TCHAR *path);
	bool ShowIFileDialog(const _TCHAR *title, _TCHAR *path, size_t len);
};

}; /* namespace GUI_WIN */

#endif /* WIN_FOLDERBOX_H */
