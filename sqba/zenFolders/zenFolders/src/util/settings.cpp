//#include <windows.h>
#include "settings.h"
//#include <Shlwapi.h> // Shlwapi.lib
#include "registry.h"
#include "shellnse.h"
#include "string.h"
#include "../pidlmgr.h"
#include "../guid.h"


extern LPPIDLMGR		g_pPidlMgr;


BOOL CSettings::SaveCurrentPath()
{
	TCHAR szFullPath[MAX_PATH];
	DWORD dwRet = ::GetCurrentDirectory(MAX_PATH, szFullPath);
//	::GetModuleFileName(NULL, szFullPath, MAX_PATH);
//	LPSTR p = strrchr(szFullPath, '\\');
//	*p = 0;
//	_RPTF1(_CRT_WARN, "szFullPath: \n", szFullPath);
	strcat(szFullPath, "\\");
	strcat(szFullPath, ZENFOLDERS_XML);
//	_RPTF1(_CRT_WARN, "szFullPath = %s\n", szFullPath);

	return CRegistry::SaveString(
		MAIN_KEY_STRING,
		TEXT("Path"),
		szFullPath,
		sizeof(szFullPath));
}

BOOL CSettings::GetXmlFilePath(TCHAR *lpszPath, DWORD dwSize)
{
	return CRegistry::GetValue(MAIN_KEY_STRING, TEXT("Path"), (LPBYTE)lpszPath, dwSize);
}

BOOL CSettings::SaveGlobalSettings(void)
{
	DWORD dwArray[4];
	dwArray[0] = g_nColumn1;
	dwArray[1] = g_nColumn2;
	dwArray[2] = 0;//g_bViewKeys;
//	dwArray[3] = g_lCookie;

//	CRegistry::SaveIntGlobal(MAIN_KEY_STRING, COOKIE_STRING, g_lCookie);

	return CRegistry::SaveValue(
		MAIN_KEY_STRING,
		VALUE_STRING,
		(LPBYTE)dwArray,
		sizeof(dwArray));
/*
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	
	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		MAIN_KEY_STRING,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL, 
		&hKey,
		&dwDisp);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//create an array to put our data in
	DWORD dwArray[4];
	dwArray[0] = g_nColumn1;
	dwArray[1] = g_nColumn2;
	dwArray[2] = g_bViewKeys;
//	dwArray[3] = g_bShowIDW;
	
	//save the last printer selected
	lResult = RegSetValueEx(
		hKey,
		VALUE_STRING,
		0,
		REG_BINARY,
		(LPBYTE)dwArray,
		sizeof(dwArray));
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
*/
}

BOOL CSettings::GetGlobalSettings(void)
{
	DWORD dwArray[4];
	DWORD dwSize = sizeof(dwArray);

	//set up the default data
	g_nColumn1 = INITIAL_COLUMN_SIZE;
	g_nColumn2 = INITIAL_COLUMN_SIZE;
//	g_bViewKeys = TRUE;
//	g_bShowIDW = FALSE;
	
	if(!CRegistry::GetValue(MAIN_KEY_STRING,
		VALUE_STRING,
		(LPBYTE)dwArray,
		dwSize))
		return FALSE;

	g_nColumn1	= dwArray[0];
	g_nColumn2	= dwArray[1];
//	g_bViewKeys	= dwArray[2];
//	g_lCookie	= dwArray[3];

	g_lCookie = GetGoogleCookie();

	if(g_nColumn1 < 0)
		g_nColumn1 = INITIAL_COLUMN_SIZE;
	if(g_nColumn2 < 0)
		g_nColumn2 = INITIAL_COLUMN_SIZE;

	return TRUE;

/*
	HKEY  hKey;
	LONG  lResult;
	
	//set up the default data
	g_nColumn1 = INITIAL_COLUMN_SIZE;
	g_nColumn2 = INITIAL_COLUMN_SIZE;
	g_bViewKeys = TRUE;
//	g_bShowIDW = FALSE;
	
	lResult = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		MAIN_KEY_STRING,
		0,
		KEY_ALL_ACCESS,
		&hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//create an array to put our data in
	DWORD dwArray[4];
	DWORD dwType;
	DWORD dwSize = sizeof(dwArray);
	
	//save the last printer selected
	lResult = RegQueryValueEx(
		hKey,
		VALUE_STRING,
		NULL,
		&dwType,
		(LPBYTE)dwArray,
		&dwSize);
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	g_nColumn1 = dwArray[0];
	g_nColumn2 = dwArray[1];
	g_bViewKeys = dwArray[2];
//	g_bShowIDW = dwArray[3];
	
	return TRUE;
*/
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

int CSettings::GetGoogleCookie()
{
	int cookie = 0;
	CRegistry::GetValueGlobal(
		MAIN_KEY_STRING,
		COOKIE_STRING,
		(LPBYTE)&cookie,
		sizeof(int));
	return cookie;
}

void CSettings::SetGoogleCookie(int cookie)
{
	CRegistry::SaveIntGlobal(MAIN_KEY_STRING, COOKIE_STRING, cookie);
}
