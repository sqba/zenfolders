
#include "strutils.h"

int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
	*pLocal = 0;
	
#ifdef UNICODE
	lstrcpyn(pLocal, pWide, dwChars);
#else
	WideCharToMultiByte(CP_ACP, 0, pWide, -1, pLocal, dwChars, NULL, NULL);
#endif
	
	return lstrlen(pLocal);
}

int LocalToWideChar(LPWSTR pWide, LPTSTR pLocal, DWORD dwChars)
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

BOOL AddBackslash(LPTSTR lpszString)
{
	if(*lpszString && *(lpszString + lstrlen(lpszString) - 1) != '\\')
	{
		lstrcat(lpszString, TEXT("\\"));
		return TRUE;
	}
	return FALSE;
}
