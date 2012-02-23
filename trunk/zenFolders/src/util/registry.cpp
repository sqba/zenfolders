// registry.cpp: implementation of the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "registry.h"

/*
DWORD CRegistry::GetKeyName(HKEY hKeyRoot,
							LPCTSTR pszSubKey,
							DWORD dwIndex,
							LPTSTR pszOut,
							DWORD dwSize)
{
	HKEY     hKey;
	LONG     lResult;
	FILETIME ft;
	
	if(!pszOut)
		return 0;
	
	if(!pszSubKey)
		pszSubKey = TEXT("");
	
	//open the specified key
	lResult = RegOpenKeyEx(
		hKeyRoot,
		pszSubKey,
		0,
		KEY_ENUMERATE_SUB_KEYS,
		&hKey);
	
	if(ERROR_SUCCESS != lResult)
		return 0;
	
	//try to get the specified subkey
	lResult = RegEnumKeyEx(
		hKey,
		dwIndex,
		pszOut,
		&dwSize,
		NULL,
		NULL,
		NULL,
		&ft);
	
	RegCloseKey(hKey);
	
	if(ERROR_SUCCESS == lResult)
		return dwSize;
	
	return 0;
}

BOOL CRegistry::GetValueName(HKEY hKeyRoot, 
				  LPCTSTR pszSubKey, 
				  DWORD dwIndex, 
				  LPTSTR pszOut, 
				  DWORD dwSize)
{
	HKEY     hKey;
	LONG     lResult;
	DWORD    dwType;
	
	if(!pszOut)
		return FALSE;
	
	if(!pszSubKey)
		pszSubKey = TEXT("");
	
	//open the specified key
	lResult = RegOpenKeyEx( hKeyRoot,
		pszSubKey,
		0,
		KEY_QUERY_VALUE,
		&hKey);
	
	if(ERROR_SUCCESS != lResult)
		return FALSE;
	
	//try to get the specified subkey
	lResult = RegEnumValue( hKey,
		dwIndex,
		pszOut,
		&dwSize,
		NULL,
		&dwType,
		NULL,
		NULL);
	
	RegCloseKey(hKey);
	
	return BOOL(ERROR_SUCCESS == lResult);
}

DWORD CRegistry::GetRootKeyText(HKEY hKeyRoot, LPTSTR lpszOut, DWORD dwOutSize)
{
	*lpszOut = 0;
	
	if(hKeyRoot == HKEY_CLASSES_ROOT)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_CLASSES_ROOT"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_CURRENT_USER)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_CURRENT_USER"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_LOCAL_MACHINE)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_LOCAL_MACHINE"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_USERS)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_USERS"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_PERFORMANCE_DATA)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_PERFORMANCE_DATA"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_CURRENT_CONFIG)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_CURRENT_CONFIG"), dwOutSize);
	}
	
	else if(hKeyRoot == HKEY_DYN_DATA)
	{
		lstrcpyn(lpszOut, TEXT("HKEY_DYN_DATA"), dwOutSize);
	}
	
	return lstrlen(lpszOut) + 1;
}

BOOL CRegistry::RootKeyExists(HKEY hKeyRoot)
{
	LONG  lResult;
	HKEY  hKey;
	
	//open the specified key
	lResult = RegOpenKeyEx(
		hKeyRoot,
		NULL,
		0,
		KEY_ENUMERATE_SUB_KEYS,
		&hKey);
	
	if(ERROR_SUCCESS != lResult)
		return FALSE;
	
	RegCloseKey(hKey);
	
	return TRUE;
}

LPTSTR CRegistry::GetNextRegElement(LPTSTR pszNext, LPTSTR pszOut, DWORD dwOut)
{
	if(!pszNext || !*pszNext)
		return NULL;
	
	LPTSTR   pszTail = pszNext;
	
	while(*pszTail && (*pszTail != '\\'))
	{
		pszTail++;
	}
	
	DWORD dwCopy = ((LPBYTE)pszTail - (LPBYTE)pszNext)/sizeof(TCHAR) + 1;
	
	lstrcpyn(pszOut, pszNext, min(dwOut, dwCopy));
	
	if(*pszTail)
		pszTail++;
	
	return pszTail;
}
*/
BOOL CRegistry::SaveValue(LPCTSTR lpKeyName,
						  LPCTSTR lpValueName,
						  LPBYTE lpData,
						  DWORD dwSize,
						  DWORD dwType)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	
	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		lpKeyName,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL, 
		&hKey,
		&dwDisp);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//save the last printer selected
	lResult = RegSetValueEx(
		hKey,
		lpValueName,
		0,
		dwType,
		lpData,
		dwSize);
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
}

BOOL CRegistry::SaveValue(LPCTSTR lpKeyName,
						  LPCTSTR lpValueName,
						  LPBYTE lpData,
						  DWORD dwSize)
{
	return SaveValue(lpKeyName, lpValueName, lpData, dwSize, REG_BINARY);
}

BOOL CRegistry::SaveString(LPCTSTR lpKeyName,
						   LPCTSTR lpValueName,
						   LPCTSTR lpData,
						   DWORD dwSize)
{
	return SaveValue(lpKeyName, lpValueName, (LPBYTE)lpData, dwSize, REG_SZ);
}

BOOL CRegistry::SaveValueGlobal(LPCTSTR lpKeyName,
								LPCTSTR lpValueName,
								LPBYTE lpData,
								DWORD dwSize,
								DWORD dwType)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	
	lResult = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		lpKeyName,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL, 
		&hKey,
		&dwDisp);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//save the last printer selected
	lResult = RegSetValueEx(
		hKey,
		lpValueName,
		0,
		dwType,
		lpData,
		dwSize);
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
}

BOOL CRegistry::SaveIntGlobal(LPCTSTR lpKeyName,
							  LPCTSTR lpValueName,
							  INT value)
{
	return SaveValueGlobal(lpKeyName, lpValueName, (LPBYTE)&value, sizeof(INT), REG_DWORD);
}

BOOL CRegistry::GetValue(LPCTSTR lpKeyName,
						 LPCTSTR lpValueName,
						 LPBYTE lpData,
						 DWORD dwSize)
{
/*
	HKEY  hKey;
	LONG  lResult;
	DWORD dwType;
	
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, lpKeyName, 0, KEY_ALL_ACCESS, &hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//save the last printer selected
	lResult = RegQueryValueEx(hKey, lpValueName, NULL, &dwType, lpData, &dwSize);
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
*/
	return GetValue(HKEY_CURRENT_USER, lpKeyName, lpValueName, lpData, dwSize);
}

BOOL CRegistry::GetValueGlobal(LPCTSTR lpKeyName,
							   LPCTSTR lpValueName,
							   LPBYTE lpData,
							   DWORD dwSize)
{
/*
	HKEY  hKey;
	LONG  lResult;
	DWORD dwType;
	
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, lpKeyName, 0, KEY_ALL_ACCESS, &hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//save the last printer selected
	lResult = RegQueryValueEx(hKey, lpValueName, NULL, &dwType, lpData, &dwSize);
	
	RegCloseKey(hKey);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
*/
	return GetValue(HKEY_LOCAL_MACHINE, lpKeyName, lpValueName, lpData, dwSize);
}

BOOL CRegistry::GetValue(HKEY hKey,
						 LPCTSTR lpKeyName,
						 LPCTSTR lpValueName,
						 LPBYTE lpData,
						 DWORD dwSize)
{
	HKEY  hKeyTmp;
	LONG  lResult;
	DWORD dwType;
	
	lResult = RegOpenKeyEx(hKey, lpKeyName, 0, KEY_ALL_ACCESS, &hKeyTmp);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	//save the last printer selected
	lResult = RegQueryValueEx(hKeyTmp, lpValueName, NULL, &dwType, lpData, &dwSize);
	
	RegCloseKey(hKeyTmp);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	return TRUE;
}
