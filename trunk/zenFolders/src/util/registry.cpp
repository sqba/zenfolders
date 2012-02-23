// registry.cpp: implementation of the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "registry.h"


bool CRegistry::SaveValue(LPCTSTR lpKeyName,
						  LPCTSTR lpValueName,
						  LPBYTE lpData,
						  DWORD dwSize,
						  DWORD dwType)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	
	lResult = ::RegCreateKeyEx(
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
		return false;
	
	lResult = ::RegSetValueEx(
		hKey,
		lpValueName,
		0,
		dwType,
		lpData,
		dwSize);
	
	::RegCloseKey(hKey);
	
	return (lResult == ERROR_SUCCESS);
}

bool CRegistry::SaveValue(LPCTSTR lpKeyName,
						  LPCTSTR lpValueName,
						  LPBYTE lpData,
						  DWORD dwSize)
{
	return SaveValue(
		lpKeyName,
		lpValueName,
		lpData,
		dwSize,
		REG_BINARY);
}

bool CRegistry::SaveString(LPCTSTR lpKeyName,
						   LPCTSTR lpValueName,
						   LPCTSTR lpData,
						   DWORD dwSize)
{
	return SaveValue(
		lpKeyName,
		lpValueName,
		(LPBYTE)lpData,
		dwSize,
		REG_SZ);
}

bool CRegistry::SaveValueGlobal(LPCTSTR lpKeyName,
								LPCTSTR lpValueName,
								LPBYTE lpData,
								DWORD dwSize,
								DWORD dwType)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	
	lResult = ::RegCreateKeyEx(
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
		return false;
	
	//save the last printer selected
	lResult = ::RegSetValueEx(
		hKey,
		lpValueName,
		0,
		dwType,
		lpData,
		dwSize);
	
	::RegCloseKey(hKey);
	
	return (lResult == ERROR_SUCCESS);
}

bool CRegistry::SaveIntGlobal(LPCTSTR lpKeyName,
							  LPCTSTR lpValueName,
							  INT value)
{
	return SaveValueGlobal(
		lpKeyName,
		lpValueName,
		(LPBYTE)&value,
		sizeof(INT),
		REG_DWORD);
}

bool CRegistry::GetValue(LPCTSTR lpKeyName,
						 LPCTSTR lpValueName,
						 LPBYTE lpData,
						 DWORD dwSize)
{
	return GetValue(
		HKEY_CURRENT_USER,
		lpKeyName,
		lpValueName,
		lpData,
		dwSize);
}

bool CRegistry::GetValueGlobal(LPCTSTR lpKeyName,
							   LPCTSTR lpValueName,
							   LPBYTE lpData,
							   DWORD dwSize)
{
	return GetValue(
		HKEY_LOCAL_MACHINE,
		lpKeyName,
		lpValueName,
		lpData,
		dwSize);
}

bool CRegistry::GetValue(HKEY hKey,
						 LPCTSTR lpKeyName,
						 LPCTSTR lpValueName,
						 LPBYTE lpData,
						 DWORD dwSize)
{
	HKEY  hKeyTmp;
	LONG  lResult;
	DWORD dwType;
	
	lResult = ::RegOpenKeyEx(
		hKey,
		lpKeyName,
		0,
		KEY_ALL_ACCESS,
		&hKeyTmp);
	
	if(lResult != ERROR_SUCCESS)
		return false;
	
	lResult = ::RegQueryValueEx(
		hKeyTmp,
		lpValueName,
		NULL,
		&dwType,
		lpData,
		&dwSize);
	
	::RegCloseKey(hKeyTmp);
	
	return (lResult == ERROR_SUCCESS);
}
