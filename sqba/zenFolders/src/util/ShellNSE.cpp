// ShellNSE.cpp: implementation of the CShellNSE class.
//
//////////////////////////////////////////////////////////////////////

#include "shellnse.h"
#include "string.h"
#include "registry.h"
#include "settings.h"

#define STR_CLSID_KEY		"CLSID\\%s"
#define STR_INPROCSRV_KEY	"CLSID\\%s\\InprocServer32"
#define STR_ICON_KEY		"CLSID\\%s\\DefaultIcon"
#define STR_PROPERTIES_KEY	"CLSID\\%s\\ShellEx\\PropertySheetHandlers\\FolderProperties"

#define STR_NAMESPACE_KEY	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\%s\\NameSpace\\%s"
#define STR_SHELLEX_KEY		"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"
#define STR_SHELLFOLDER_KEY	"CLSID\\%s\\ShellFolder"

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


//register the CLSID entries
REGSTRUCT ClsidEntries[] =
{
	HKEY_CLASSES_ROOT,	TEXT(STR_CLSID_KEY),		NULL,					TEXT( SHELLEX_NAME ),
	HKEY_CLASSES_ROOT,	TEXT(STR_INPROCSRV_KEY),	NULL,					TEXT("%s"),
	HKEY_CLASSES_ROOT,	TEXT(STR_INPROCSRV_KEY),	TEXT("ThreadingModel"),	TEXT("Apartment"),
	HKEY_CLASSES_ROOT,	TEXT(STR_INPROCSRV_KEY),	TEXT("LoadWithoutCOM"),	TEXT(""),
	HKEY_CLASSES_ROOT,	TEXT(STR_ICON_KEY),			NULL,					TEXT("%s,0"),
	NULL, NULL, NULL, NULL
};

REGSTRUCT PropertyClsidEntries[] =
{
	HKEY_CLASSES_ROOT,	TEXT(STR_INPROCSRV_KEY),	NULL,					TEXT("%s"),
	HKEY_CLASSES_ROOT,	TEXT(STR_INPROCSRV_KEY),	TEXT("ThreadingModel"),	TEXT("Apartment"),
	NULL, NULL, NULL, NULL
};

REGSTRUCT regProperties =
{
	HKEY_CLASSES_ROOT,
	TEXT(STR_PROPERTIES_KEY),
	NULL,
	TEXT("%s")
};


HRESULT CShellNSE::RegisterServer(HINSTANCE hInst, GUID guid)
{
	HKEY     hKey;
	LRESULT  lResult;
	DWORD    dwDisp;
	TCHAR    szSubKey[MAX_PATH];
	TCHAR    szCLSID[MAX_PATH];
	TCHAR    szModule[MAX_PATH];

	CString::GuidToString(guid, szCLSID, ARRAYSIZE(szCLSID));
	
	//get this DLL's path and file name
	::GetModuleFileName(hInst, szModule, ARRAYSIZE(szModule));
	
	CreateSubKeys(szModule, szCLSID, ClsidEntries);
	
	//Register the default flags for the folder.
	
	wsprintf(szSubKey, TEXT(STR_SHELLFOLDER_KEY), szCLSID);
	
	lResult = RegCreateKeyEx(
		HKEY_CLASSES_ROOT,
		szSubKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&hKey,
		&dwDisp);
	
	if(NOERROR == lResult)
	{
		DWORD dwData =
			SFGAO_FOLDER
			| SFGAO_HASSUBFOLDER
			| SFGAO_FILESYSTEM;
			//| SFGAO_CANRENAME;
			//| SFGAO_HASPROPSHEET;
		
#ifdef SFGAO_BROWSABLE
		dwData |= SFGAO_BROWSABLE;
#endif
		
		lResult = RegSetValueEx(hKey, TEXT("Attributes"), 0, REG_BINARY, (LPBYTE)&dwData, sizeof(dwData));
		
		lResult = RegSetValueEx(hKey, TEXT("WantsFORPARSING"), 0, REG_SZ, (LPBYTE)(TEXT("")), 0);

		RegCloseKey(hKey);
	}
	else
		return SELFREG_E_CLASS;
	
	//Register the name space extension
	
	wsprintf(szSubKey, TEXT(STR_NAMESPACE_KEY), TEXT(ROOT_LOCATION), szCLSID);
	
	lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);
	
	if(NOERROR == lResult)
	{
		TCHAR szData[MAX_PATH];
		
		//Create the value string.
		lstrcpy(szData, TEXT( SHELLEX_NAME ));
		
		lResult = RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szData, (lstrlen(szData) + 1) * sizeof(TCHAR));
		
		RegCloseKey(hKey);
	}
	else
		return SELFREG_E_CLASS;
	
	//If running on NT, register the extension as approved.
	OSVERSIONINFO  osvi;
	
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	
	if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
	{
		lstrcpy( szSubKey, TEXT(STR_SHELLEX_KEY));
		
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);
		
		if(NOERROR == lResult)
		{
			TCHAR szData[MAX_PATH];
			
			//Create the value string.
			lstrcpy(szData, TEXT( SHELLEX_NAME ));
			
			lResult = RegSetValueEx(hKey, szCLSID, 0, REG_SZ, (LPBYTE)szData, (lstrlen(szData) + 1) * sizeof(TCHAR));
			
			RegCloseKey(hKey);
		}
		else
			return SELFREG_E_CLASS;
	}
	
	//Refresh Desktop
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	return S_OK;
}

HRESULT CShellNSE::UnregisterServer(HINSTANCE hInst, GUID guid)
{
	int      i;
	TCHAR    szSubKey[MAX_PATH];
	TCHAR    szCLSID[MAX_PATH];
	TCHAR    szModule[MAX_PATH];

	CString::GuidToString(guid, szCLSID, ARRAYSIZE(szCLSID));

	//get this DLL's path and file name
	GetModuleFileName(hInst, szModule, ARRAYSIZE(szModule));
	
	for(i = 0; ClsidEntries[i].hRootKey; i++)
	{
		//Create the sub key string.
		wsprintf(szSubKey, ClsidEntries[i].lpszSubKey, szCLSID);
		RegDeleteKey(ClsidEntries[i].hRootKey, szSubKey);
	}
	
	//Unregister the default flags for the folder.
	wsprintf(szSubKey, TEXT(STR_SHELLFOLDER_KEY), szCLSID);
	RegDeleteKey(HKEY_CLASSES_ROOT, szSubKey);
	
	//Unregister the name space extension
	
	//Remove the sub key string.
	wsprintf(szSubKey, TEXT(STR_NAMESPACE_KEY), TEXT(ROOT_LOCATION), szCLSID);
	
	RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	
	//If running on NT, unregister the extension as approved.
	OSVERSIONINFO  osvi;
	
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	
	if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
	{
		lstrcpy( szSubKey, TEXT(STR_SHELLEX_KEY));
		RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	}

	//Refresh Desktop
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

//	::SHChangeNotify( SHCNE_RMDIR , SHCNF_IDLIST | SHCNF_FLUSH, _Module.m_pidlNSFROOT, NULL);

	return S_OK;
}

HRESULT CShellNSE::RegisterProperties(HINSTANCE hInst, GUID guid, GUID guidProperties)
{
	HRESULT hr;
	TCHAR szCLSID[MAX_PATH];
	TCHAR szCLSID_Properties[MAX_PATH];
	TCHAR szModule[MAX_PATH];

	CString::GuidToString(guid, szCLSID, ARRAYSIZE(szCLSID));

	CString::GuidToString(guidProperties, szCLSID_Properties, ARRAYSIZE(szCLSID_Properties));

	//get this DLL's path and file name
	::GetModuleFileName(hInst, szModule, ARRAYSIZE(szModule));

	CreateSubKeys(szModule, szCLSID_Properties, PropertyClsidEntries);

	hr = CreateSubKey(szCLSID, &regProperties, szCLSID_Properties);

	return hr;
}

HRESULT CShellNSE::UnregisterProperties(HINSTANCE hInst, GUID guid)
{
	TCHAR szCLSID[MAX_PATH];
	TCHAR szSubKey[MAX_PATH];
	CString::GuidToString(guid, szCLSID, ARRAYSIZE(szCLSID));

	wsprintf(szSubKey, TEXT("CLSID\\%s\\ShellEx\\PropertySheetHandlers\\FolderProperties"), szCLSID);
	RegDeleteKey(regProperties.hRootKey, szSubKey);

	wsprintf(szSubKey, TEXT("CLSID\\%s\\ShellEx\\PropertySheetHandlers"), szCLSID);
	RegDeleteKey(regProperties.hRootKey, szSubKey);

	wsprintf(szSubKey, TEXT("CLSID\\%s\\ShellEx"), szCLSID);
	RegDeleteKey(regProperties.hRootKey, szSubKey);

	UnregisterServer(hInst, guid);

	return S_OK;
}
	
HRESULT CShellNSE::CreateSubKey(LPCTSTR lpszCLSID,
								LPREGSTRUCT lpRegStruct,
								LPCTSTR lpszValue)
{
	HKEY     hKey;
	LRESULT  lResult;
	DWORD    dwDisp;
	TCHAR    szSubKey[MAX_PATH];

	//Create the sub key string.
	wsprintf(szSubKey, lpRegStruct->lpszSubKey, lpszCLSID);
	
	lResult = RegCreateKeyEx(
		lpRegStruct->hRootKey,
		szSubKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&hKey,
		&dwDisp);
	
	if(NOERROR == lResult)
	{
		TCHAR szData[MAX_PATH];
		
		//if necessary, create the value string
		wsprintf(szData, lpRegStruct->lpszData, lpszValue);
		
		lResult = RegSetValueEx(
			hKey,
			lpRegStruct->lpszValueName,
			0,
			REG_SZ,
			(LPBYTE)szData,
			(lstrlen(szData) + 1) * sizeof(TCHAR));
		
		RegCloseKey(hKey);
	}
	else
		return SELFREG_E_CLASS;

	return S_OK;
}

HRESULT CShellNSE::CreateSubKeys(LPCTSTR lpszModule,
								 LPCTSTR lpszCLSID,
								 REGSTRUCT *pEntries)
{
	for(int i = 0; ClsidEntries[i].hRootKey; i++)
	{
		CreateSubKey(lpszCLSID, &ClsidEntries[i], lpszModule);
	}

	return S_OK;
}
/*
HRESULT CShellNSE::RemoveSubKeys(LPCTSTR lpszCLSID, REGSTRUCT *pEntries)
{
	TCHAR szSubKey[MAX_PATH];

	for(int i = 0; ClsidEntries[i].hRootKey; i++)
	{
		//Create the sub key string.
		wsprintf(szSubKey, ClsidEntries[i].lpszSubKey, lpszCLSID);
		RegDeleteKey(ClsidEntries[i].hRootKey, szSubKey);
	}
}
*/
int CShellNSE::GetName(GUID guid, LPCTSTR lpszName, int size)
{
	TCHAR    szCLSID[MAX_PATH];
	CString::GuidToString(guid, szCLSID, ARRAYSIZE(szCLSID));

	TCHAR szKeyName[MAX_PATH] = {0};
	wsprintf(szKeyName, TEXT(STR_CLSID_KEY), szCLSID);

	CRegistry::GetValue(HKEY_CLASSES_ROOT, szKeyName, NULL, (LPBYTE)lpszName, size);

	return 0;
}
