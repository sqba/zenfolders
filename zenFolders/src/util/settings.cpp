//#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include "settings.h"
//#include <Shlwapi.h> // Shlwapi.lib
#include "registry.h"
#include "shellnse.h"
#include "string.h"
#include "../pidlmgr.h"
#include "../guid.h"


extern LPPIDLMGR		g_pPidlMgr;
extern HINSTANCE		g_hInst;


bool CSettings::GetPath(TCHAR *lpszPath, DWORD dwSize)
{
	DWORD dwRet = ::GetCurrentDirectory(MAX_PATH, lpszPath);
	if(dwRet > 0)
	{
		::GetModuleFileName(g_hInst, lpszPath, MAX_PATH);
		LPSTR p = strrchr(lpszPath, '\\');
		if(p)
		{
			*(++p) = 0;
			return true;
		}
	}
	return false;
}

bool CSettings::GetXmlFilePath(TCHAR *lpszPath, DWORD dwSize)
{
	if( GetPath(lpszPath, dwSize) )
	{
		strcat(lpszPath, ZENFOLDERS_XML);
		return true;
	}
	return false;
}

bool CSettings::SaveGlobalSettings(void)
{
	DWORD dwArray[2] = {0};
	dwArray[0] = g_nColumn1;
	dwArray[1] = g_nColumn2;

	return CRegistry::SaveValue(
		MAIN_KEY_STRING,
		VALUE_STRING,
		(LPBYTE)dwArray,
		sizeof(dwArray));
}

bool CSettings::GetGlobalSettings(void)
{
	DWORD dwArray[2] = {0};

	//set up the default data
	g_nColumn1 = INITIAL_COLUMN_SIZE;
	g_nColumn2 = INITIAL_COLUMN_SIZE;
	
	if(!CRegistry::GetValue(
		MAIN_KEY_STRING,
		VALUE_STRING,
		(LPBYTE)dwArray,
		sizeof(dwArray)))
		return false;

	g_nColumn1	= dwArray[0];
	g_nColumn2	= dwArray[1];

	g_lCookie = GetGoogleCookie();

	if(g_nColumn1 < 0)
		g_nColumn1 = INITIAL_COLUMN_SIZE;
	if(g_nColumn2 < 0)
		g_nColumn2 = INITIAL_COLUMN_SIZE;

	return true;
}

long CSettings::GetGoogleCookie()
{
	long cookie = 0;
	TCHAR szPath[MAX_PATH] = {0};
	if( GetPath(szPath, MAX_PATH) )
	{
		strcat(szPath, TEXT("cookie"));
		FILE *fp = fopen(szPath, "r");
		if(fp)
		{
			fscanf(fp, "%d", &cookie);
			fclose(fp);
		}
		else
		{
			_RPTF0(_CRT_WARN, "GetGoogleCookie failed\n");
		}
	}
	return cookie;
/*
	int cookie = 0;
	CRegistry::GetValueGlobal(
		MAIN_KEY_STRING,
		COOKIE_STRING,
		(LPBYTE)&cookie,
		sizeof(int));
	return cookie;
*/
}

void CSettings::SetGoogleCookie(long cookie)
{
	TCHAR szPath[MAX_PATH] = {0};
	if( GetPath(szPath, MAX_PATH) )
	{
		strcat(szPath, TEXT("cookie"));
		FILE *fp = fopen(szPath, "w");
		if(fp)
		{
			fprintf(fp, "%d", cookie);
			fclose(fp);
		}
		else
		{
			_RPTF0(_CRT_WARN, "SetGoogleCookie failed\n");
		}
	}

//	CRegistry::SaveIntGlobal(MAIN_KEY_STRING, COOKIE_STRING, cookie);
}

UINT CSettings::GetRootName(LPCITEMIDLIST pidl, LPTSTR lpszName, UINT len)
{
/*
	CShellNSE::GetName(CLSID_ShellFolderNameSpace, lpszName, len);
	return lstrlen(lpszName);
*/
/*
    HRESULT hr;

	IShellFolder *pDesktop;
    hr = ::SHGetDesktopFolder(&pDesktop);
    if(FAILED(hr))
        return 0;

	LPITEMIDLIST pidlNext = CPidlManager::GetNextItem(pidl);

	//LPPIDLDATA pData1 = CPidlManager::GetDataPointer(pidl);
	//LPPIDLDATA pData2 = CPidlManager::GetDataPointer(pidlNext);

	//LPITEMIDLIST pidlNextNext = CPidlManager::GetNextItem(pidlNext);
	//LPPIDLDATA pData3 = CPidlManager::GetDataPointer(pidlNextNext);

	LPITEMIDLIST pidlRoot = g_pPidlMgr->Copy(pidlNext);
//	pidlRoot->mkid.cb = 0;

	STRRET strret;
	strret.uType = STRRET_WSTR;
	hr = pDesktop->GetDisplayNameOf(pidlRoot, SHGDN_NORMAL, &strret);
	if(FAILED(hr))
	{
		g_pPidlMgr->Delete(pidlRoot);
		return 0;
	}

//	hr = ::StrRetToBufW(&strret, pidl, lpszName, len);
//	hr = ::StrRetToStr(&strret, pidl, lpszName);

	CString::WideCharToLocal(lpszName, strret.pOleStr, len);

	g_pPidlMgr->Delete(pidlRoot);
   
	if (STRRET_WSTR == strret.uType)
	{
		LPMALLOC pMalloc = NULL;
		if (SUCCEEDED(hr = ::SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free((LPVOID)strret.pOleStr);
			pMalloc->Release();
		}
	}
*/

	lstrcpy(lpszName, SHELLEX_NAME);
	//lstrcpy(lpszName, TEXT("Searches"));

	return lstrlen(lpszName);
}
