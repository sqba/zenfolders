#ifndef CONTMENU_H
#define CONTMENU_H

#include <windows.h>
#include <shlobj.h>
#include "shlfldr.h"

class CContextMenu : public IContextMenu
{
public:
	CContextMenu(CShellFolder*, LPCITEMIDLIST*, UINT);
	~CContextMenu();
	
public:
	//IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();
	
	//IContextMenu methods
	STDMETHODIMP GetCommandString(UINT, UINT, LPUINT, LPSTR, UINT);
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
	STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);

private:
	BOOL AllocPidlTable(DWORD);
	void FreePidlTable(void);
	BOOL FillPidlTable(LPCITEMIDLIST*, UINT);
	BOOL CanRenameItems(void);
	LPCTSTR GetFileExtension(LPCITEMIDLIST);
	void AddMenuItem(HMENU, LPTSTR, UINT, UINT=0, BOOL=TRUE, BOOL=FALSE);
	void LoadShellMenu(HMENU, UINT);

	void OnExecute();
	void OnOpenFolder(LPCMINVOKECOMMANDINFO);
	void OnShowProperties();
	void OnCreateExtensionFolder();
	void OnHideExtension();
	void OnCreateNewFolder();
	void OnShowOnlyExtension();
	void OnOpenContainingFolder();
	void OnClearSearch();
	void OnDelete();

private:
	DWORD			m_ObjRefCount;
	LPITEMIDLIST	*m_aPidls;
	IMalloc			*m_pMalloc;
	CShellFolder	*m_pSFParent;
	BOOL			m_fAllFiles;
	LPCONTEXTMENU	m_pContextMenu;
};

#endif// CONTMENU_H
