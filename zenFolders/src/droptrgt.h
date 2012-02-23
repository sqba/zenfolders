#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include "shlfldr.h"
#include "pidlmgr.h"

class FAR CDropTarget : public IDropTarget
{
public:
	CDropTarget(CShellFolder*);
	~CDropTarget();

public:
	//IUnknown methods
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	//IDropTarget methods
	STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

private:
	BOOL QueryDrop(DWORD, LPDWORD);
	DWORD GetDropEffectFromKeyState(DWORD);
	BOOL DoPrivateDrop(HGLOBAL, BOOL);
	BOOL DoHDrop(HDROP, BOOL);
	LPITEMIDLIST* AllocPidlTable(DWORD);
	void FreePidlTable(LPITEMIDLIST*);

private:
	CShellFolder	*m_psfParent;
	IMalloc			*m_pMalloc;
	ULONG			m_ObjRefCount;  
	BOOL			m_fAcceptFmt;
	UINT			m_cfPrivateData;
};
