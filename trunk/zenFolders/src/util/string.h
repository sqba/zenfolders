#ifndef STRING_H
#define STRING_H

#include <windows.h>

class CString  
{
public:
	static int WideCharToLocal(LPTSTR, LPWSTR, DWORD);
	static int LocalToWideChar(LPWSTR, LPTSTR, DWORD);
	static BOOL AddBackslash(LPTSTR);
	static UINT GuidToString(GUID, LPTSTR, UINT);

	static int LocalToAnsi(LPSTR, LPCTSTR, DWORD);
	static VOID SmartAppendBackslash(LPTSTR);
	static int AnsiToLocal(LPTSTR, LPSTR, DWORD);

	static LPCTSTR GetExtension(LPCTSTR);
	static LPCTSTR GetFilename(LPCTSTR);
};

#endif // STRING_H
