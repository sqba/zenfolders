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

public: // Member functions
	void Delete(LPITEMIDLIST);
	LPITEMIDLIST Copy(LPCITEMIDLIST);
	LPITEMIDLIST Concatenate(LPCITEMIDLIST, LPCITEMIDLIST);
	LPITEMIDLIST CreateFileFromSearch(IGoogleDesktopQueryResultItem*);
	LPITEMIDLIST CreateFileFromNode(MSXML2::IXMLDOMNodePtr);
	LPITEMIDLIST CreateFileFromPath(LPCTSTR);
	LPITEMIDLIST CreateFolder(MSXML2::IXMLDOMNodePtr);
	LPITEMIDLIST CreateSubFolder(LPCTSTR);
	LPITEMIDLIST CreateFolderLink(MSXML2::IXMLDOMNodePtr);
	LPITEMIDLIST GetFSPidl(LPCITEMIDLIST);
	LPITEMIDLIST GetFSPidl(LPCTSTR);

public: // Static functions
	static bool IsRoot(LPCITEMIDLIST);
	static bool IsFile(LPCITEMIDLIST);
	static bool IsFolder(LPCITEMIDLIST);
	static bool IsFolderLink(LPCITEMIDLIST);
	static bool IsSubFolder(LPCITEMIDLIST);
	static bool IsOurPidl(LPCITEMIDLIST);
	static bool HasSubFolders(LPCITEMIDLIST);
	static bool Equal(LPCITEMIDLIST, LPCITEMIDLIST);
	static UINT GetSize(LPCITEMIDLIST);
	static DWORD GetItemName(LPCITEMIDLIST, LPTSTR, USHORT);
	static DWORD GetPidlPath(LPCITEMIDLIST, LPTSTR, DWORD);
	static HRESULT CompareIDs(LPCITEMIDLIST, LPCITEMIDLIST);
	static PIDLTYPE GetType(LPCITEMIDLIST);
	static LPPIDLDATA GetDataPointer(LPCITEMIDLIST);
	static LPITEMIDLIST GetNextItem(LPCITEMIDLIST);
	static LPITEMIDLIST GetLastItem(LPCITEMIDLIST);

#ifdef _DEBUG
public: // Debugging functions
	static void dbgTracePidlPath(LPCTSTR, LPCITEMIDLIST);
	static void dbgTracePidlData(LPCTSTR, LPCITEMIDLIST);
#endif	// _DEBUG

private:
	LPITEMIDLIST Create(PIDLTYPE, LPVOID, USHORT);
	bool SetTitle(IGoogleDesktopQueryResultItem *pItem, LPPIDLDATA pData);

private:
	LPMALLOC		m_pMalloc;
	IShellFolder	*m_psfDesktop;
};

typedef CPidlManager FAR *LPPIDLMGR;


#ifdef _DEBUG
// Debugging macros
#define TRACE_PIDL_PATH(a, b)	CPidlManager::dbgTracePidlPath(TEXT(a), b)
#define TRACE_PIDL_DATA(a, b)	CPidlManager::dbgTracePidlData(TEXT(a), b)
#else	// _DEBUG
#define TRACE_PIDL_PATH(a, b)
#define TRACE_PIDL_DATA(a, b)
#endif	// _DEBUG


class CPidl  
{
public:
	CPidl();
	CPidl(LPCITEMIDLIST);
	virtual ~CPidl();

	void operator = (LPCITEMIDLIST);
	bool operator == (LPCITEMIDLIST);
	bool operator != (LPCITEMIDLIST);
	LPITEMIDLIST operator + (CPidl);
	LPITEMIDLIST operator + (LPCITEMIDLIST);
	LPITEMIDLIST operator += (LPCITEMIDLIST);
	operator LPVOID();
	operator LPPIDLDATA();
	operator LPITEMIDLIST();

	bool IsRoot();
	bool IsFile();
	bool IsFolder();
	bool IsFolderLink();
	bool IsSubFolder();

	DWORD GetName(LPTSTR, USHORT);
	DWORD GetPath(LPTSTR, DWORD);
	DWORD GetFSPath(LPTSTR, DWORD);

	LPITEMIDLIST GetRelative();
	MSXML2::IXMLDOMNodePtr GetNode();
	LPPIDLDATA GetData();

private:
	LPITEMIDLIST m_pidl;
};


#endif // PIDLMGR_H
