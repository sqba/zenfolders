// string.cpp: implementation of the CString class.
//
//////////////////////////////////////////////////////////////////////

#include "string.h"


int CString::WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
	*pLocal = 0;
	
#ifdef UNICODE
	lstrcpyn(pLocal, pWide, dwChars);
#else
	WideCharToMultiByte(CP_ACP, 0, pWide, -1, pLocal, dwChars, NULL, NULL);
#endif
	
	return lstrlen(pLocal);
}

int CString::LocalToWideChar(LPWSTR pWide, LPTSTR pLocal, DWORD dwChars)
{
	*pWide = 0;
	
#ifdef UNICODE
	lstrcpyn(pWide, pLocal, dwChars);
#else
	MultiByteToWideChar( CP_ACP, 
		0, 
		pLocal, 
		-1, 
		pWide, 
		dwChars); 
#endif
	
	return lstrlenW(pWide);
}

BOOL CString::AddBackslash(LPTSTR lpszString)
{
	if(*lpszString && *(lpszString + lstrlen(lpszString) - 1) != '\\')
	{
		lstrcat(lpszString, TEXT("\\"));
		return TRUE;
	}
	return FALSE;
}

UINT CString::GuidToString(GUID guid, LPTSTR lpszGUID, UINT uSize)
{
	LPWSTR pwsz;
	UINT result = 0;

	//get the CLSID in string form
	StringFromIID(guid, &pwsz);
	
	if(pwsz)
	{
		result = CString::WideCharToLocal(lpszGUID, pwsz, uSize);
		
		//free the string
		LPMALLOC pMalloc;
		CoGetMalloc(1, &pMalloc);
		if(pMalloc)
		{
			pMalloc->Free(pwsz);
			pMalloc->Release();
		}
	}
	return result;
}

int CString::LocalToAnsi(LPSTR pAnsi, LPCTSTR pLocal, DWORD dwChars)
{
	*pAnsi = 0;
	
#ifdef UNICODE
	WideCharToMultiByte( CP_ACP, 
		0, 
		pLocal, 
		-1, 
		pAnsi, 
		dwChars, 
		NULL, 
		NULL);
#else
	lstrcpyn(pAnsi, pLocal, dwChars);
#endif
	
	return lstrlenA(pAnsi);
}

VOID CString::SmartAppendBackslash(LPTSTR pszPath)
{
	if(*(pszPath + lstrlen(pszPath) - 1) != '\\')
		lstrcat(pszPath, TEXT("\\"));
}

int CString::AnsiToLocal(LPTSTR pLocal, LPSTR pAnsi, DWORD dwChars)
{
	*pLocal = 0;
	
#ifdef UNICODE
	MultiByteToWideChar( CP_ACP, 
		0, 
		pAnsi, 
		-1, 
		pLocal, 
		dwChars); 
#else
	lstrcpyn(pLocal, pAnsi, dwChars);
#endif
	
	return lstrlen(pLocal);
}

LPCTSTR CString::GetExtension(LPCTSTR pszPath)
{
	return strrchr(pszPath, '.');
}

LPCTSTR CString::GetFilename(LPCTSTR pszPath)
{
	return strrchr(pszPath, '\\');
}
