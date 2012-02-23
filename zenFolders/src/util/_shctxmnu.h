#ifndef SHCTXMNU_H
#define SHCTXMNU_H

//#define WINVER 0x500

#include <windows.h>
#include <shlobj.h>

class CShellContextMenu  
{
public:
	CShellContextMenu();
	virtual ~CShellContextMenu();
	void SetObjects (IShellFolder*, LPITEMIDLIST);
	void SetObjects (IShellFolder*, LPITEMIDLIST*, int);
	void SetObjects (LPITEMIDLIST);
//	CMenu * GetMenu ();
//	void SetObjects (CString strObject);
//	void SetObjects (CStringArray &strArray);
//	UINT ShowContextMenu (CWnd* pWnd, CPoint pt);

private:
	int GetPIDLCount(LPCITEMIDLIST);
	void InvokeCommand(LPCONTEXTMENU, UINT);
	void FreePIDLArray(LPITEMIDLIST*);
	BOOL GetContextMenu(void**, int&);
	UINT GetPIDLSize(LPCITEMIDLIST);
	LPBYTE GetPIDLPos(LPCITEMIDLIST, int);
	HRESULT SHBindToParentEx(LPCITEMIDLIST, REFIID, VOID**, LPCITEMIDLIST*);
	LPITEMIDLIST CopyPIDL(LPCITEMIDLIST, int cb = -1);

private:
	static LRESULT CALLBACK HookWndProc (HWND, UINT, WPARAM, LPARAM);

private:
	int				nItems;
	BOOL			bDelete;
//	CMenu			*m_Menu;
	IShellFolder	*m_psfFolder;
	LPITEMIDLIST	*m_pidlArray;	
};

#endif // SHCTXMNU_H
