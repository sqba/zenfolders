#ifndef PIDLMGR_H
#define PIDLMGR_H

//#import <msxml3.dll>
//#import "msxml3.dll"
//using namespace MSXML2;

#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#include "google/googleds.h"
#include "cfgxml.h"
#include "pidl.h"

#import <msxml3.dll>


class CPidl  ;


class CPidlManager  
{
public:
	CPidlManager();
	virtual ~CPidlManager();

public:
	// Member functions
	void Delete(LPITEMIDLIST);
	LPITEMIDLIST Copy(LPCITEMIDLIST);
	LPITEMIDLIST Concatenate(LPCITEMIDLIST, LPCITEMIDLIST);
	LPITEMIDLIST CreateFile(IGoogleDesktopQueryResultItem*);
	LPITEMIDLIST CreateFolder(MSXML2::IXMLDOMNodePtr);
	LPITEMIDLIST CreateFromPath(LPCITEMIDLIST);
	LPITEMIDLIST GetFSPidl(LPCITEMIDLIST);
	LPITEMIDLIST GetFSPidl(LPCTSTR);

public:
	// Static functions
	static BOOL IsFile(LPCITEMIDLIST);
	static BOOL HasChildNodes(LPCITEMIDLIST);
	static BOOL Equal(LPCITEMIDLIST, LPCITEMIDLIST);
	static UINT GetSize(LPCITEMIDLIST);
	static DWORD GetItemName(LPCITEMIDLIST, LPTSTR, USHORT);
	static DWORD GetPidlPath(LPCITEMIDLIST, LPTSTR, DWORD);
	static HRESULT CompareIDs(LPCITEMIDLIST, LPCITEMIDLIST);
	static LPPIDLDATA GetDataPointer(LPCITEMIDLIST);
	static LPITEMIDLIST GetNextItem(LPCITEMIDLIST);
	static LPITEMIDLIST GetLastItem(LPCITEMIDLIST);
	static BOOL IsOurPidl(LPCITEMIDLIST);

public:
	// Debugging functions
	static void dbgTracePidlPath(LPCTSTR, LPCITEMIDLIST);
	static void dbgTracePidlData(LPCTSTR, LPCITEMIDLIST);
	static void dbgTracePidlPath(LPCTSTR, CPidl*);
	static void dbgTracePidlData(LPCTSTR, CPidl*);

private:
	LPITEMIDLIST Create(PIDLTYPE, LPVOID, USHORT);

private:
	LPMALLOC		m_pMalloc;
	IShellFolder	*m_psfDesktop;
};

typedef CPidlManager FAR *LPPIDLMGR;


#ifdef _DEBUG
#define TRACE_PIDL_PATH(a, b)	CPidlManager::dbgTracePidlPath(TEXT(a), b)
#define TRACE_PIDL_DATA(a, b)	CPidlManager::dbgTracePidlData(TEXT(a), b)
#else
#define TRACE_PIDL_PATH(a, b)
#define TRACE_PIDL_DATA(a, b)
#endif


class CPidl  
{
public:
	CPidl();
	CPidl(LPCITEMIDLIST);
	virtual ~CPidl();

	void operator = (LPCITEMIDLIST);
	LPITEMIDLIST operator + (LPCITEMIDLIST);
	LPITEMIDLIST operator + (CPidl);
	bool operator == (LPCITEMIDLIST);
	bool operator != (LPCITEMIDLIST);
	LPITEMIDLIST operator += (LPCITEMIDLIST);

	LPITEMIDLIST GetRelative();
	LPITEMIDLIST GetFull();

	MSXML2::IXMLDOMNodePtr GetNode();
	LPPIDLDATA GetData();

	BOOL IsFile();
	BOOL IsFolder();
	BOOL IsRoot();

	DWORD GetName(LPTSTR, USHORT);
	DWORD GetPath(LPTSTR, DWORD);
	DWORD GetFSPath(LPTSTR, DWORD);

private:
	LPITEMIDLIST m_pidl;
};


#endif // PIDLMGR_H
