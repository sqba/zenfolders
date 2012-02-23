#ifndef STRING_H
#define STRING_H

#include <windows.h>

class CString  
{
public:
	static int WideCharToLocal(LPTSTR, LPWSTR, DWORD);
	static int LocalToWideChar(LPWSTR, LPTSTR, DWORD);
	static bool AddBackslash(LPTSTR);
	static UINT GuidToString(GUID, LPTSTR, UINT);

	static int LocalToAnsi(LPSTR, LPCTSTR, DWORD);
	static void SmartAppendBackslash(LPTSTR);
	static int AnsiToLocal(LPTSTR, LPSTR, DWORD);

	static LPCTSTR GetExtension(LPCTSTR);
	static LPCTSTR GetFilename(LPCTSTR);

public:
	CString();
	virtual ~CString();

	void operator = (LPCTSTR);
	operator LPTSTR();

protected:
	TCHAR *m_pszString;
};

class CPath : public CString
{
public:
	LPCTSTR GetExtension();
//	LPCTSTR GetFilename();
};

#endif // STRING_H
