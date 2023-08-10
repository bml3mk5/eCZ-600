/** @file win_folderbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ folder box ]
*/

#include "win_folderbox.h"
#include "../../cchar.h"
#include "../../loadlibrary.h"
#include "../../emu.h"
#include "../../utility.h"

#ifndef PCIDLIST_ABSOLUTE
#define PCIDLIST_ABSOLUTE LPCITEMIDLIST
#endif

extern EMU *emu;

static HRESULT (WINAPI *F_SHCreateItemFromParsingName)(PCWSTR, IBindCtx*, REFIID, void**) = NULL;

namespace GUI_WIN
{

FolderBox::FolderBox(HWND parent_window)
{
	hWnd = parent_window;
}

FolderBox::~FolderBox()
{
}

bool FolderBox::Show(const _TCHAR *title, _TCHAR *path, size_t len)
{
#if !defined(__MINGW32__)
	HMODULE hShell = NULL;
	DWORD ver = GetVersion();

	if ( LOWORD(ver) >= 6 ) {	// Vista or later
		int n = 0;
		while(F_SHCreateItemFromParsingName == NULL && n == 0) {
			n++;
			LOAD_LIB(hShell, _T("shell32"), 0);
			GET_ADDR(F_SHCreateItemFromParsingName, HRESULT (WINAPI *)(PCWSTR, IBindCtx*, REFIID, void**), hShell, _T("SHCreateItemFromParsingName"));
		}
	}
	if (F_SHCreateItemFromParsingName) {
		return ShowIFileDialog(title, path, len);
	} else
#endif /* !__MINGW32__ */
	{
		return ShowSHBrowseForFolder(title, path);
	}
}

int CALLBACK FolderBox::ProcSHBrowseForFolder(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return 0;
}

bool FolderBox::ShowSHBrowseForFolder(const _TCHAR *title, _TCHAR *path)
{
	BROWSEINFO  binfo;
	PCIDLIST_ABSOLUTE idlist;

	memset(&binfo, 0, sizeof(BROWSEINFO));
	binfo.hwndOwner=hWnd;
	binfo.pidlRoot=NULL;
	binfo.pszDisplayName=path;
	binfo.lpszTitle=title;
	binfo.ulFlags=BIF_RETURNONLYFSDIRS;
	binfo.lpfn=&ProcSHBrowseForFolder;
	binfo.lParam=(LPARAM)path;
	binfo.iImage=(int)NULL;

	idlist=SHBrowseForFolder(&binfo);
	if (idlist != NULL) {
		SHGetPathFromIDList(idlist, path);
		CoTaskMemFree((LPVOID)idlist);
		return true;
	}
	return false;
}

bool FolderBox::ShowIFileDialog(const _TCHAR *title, _TCHAR *path, size_t len)
{
    HRESULT hr = S_FALSE;
#if !defined(__MINGW32__)
	IFileOpenDialog *fileDialog = NULL;
	IShellItem *folder;
	FILEOPENDIALOGOPTIONS options;

	do {
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));
		if (FAILED(hr)) break;

		hr = fileDialog->GetOptions(&options);
		if (FAILED(hr)) break;
		hr = fileDialog->SetOptions(options | FOS_PICKFOLDERS);
		if (FAILED(hr)) break;

		CTchar ctitle(title);
		hr = fileDialog->SetTitle(ctitle.GetW());

		CTchar cpath(path);
		F_SHCreateItemFromParsingName(cpath.GetW(), NULL, IID_PPV_ARGS(&folder));
		fileDialog->SetFolder(folder);

		hr = fileDialog->Show(hWnd);
		if (FAILED(hr)) break;

		hr = fileDialog->GetResult(&folder);
		if (FAILED(hr)) break;

		LPOLESTR pathOLE = NULL;
		hr = folder->GetDisplayName(SIGDN_FILESYSPATH, &pathOLE);
		if (FAILED(hr)) break;

		CTchar npath(pathOLE);
		UTILITY::tcscpy(path, len, npath.Get());
		CoTaskMemFree(pathOLE);

		fileDialog->Release();

	} while(0);
#endif /* !__MINGW32__ */
	return (hr == S_OK);
}

}; /* namespace GUI_WIN */
