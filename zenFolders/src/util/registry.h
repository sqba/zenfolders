#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>
#include <regstr.h>

class CRegistry  
{
public:
//	static DWORD GetKeyName(HKEY, LPCTSTR, DWORD, LPTSTR, DWORD);
//	static BOOL GetValueName(HKEY, LPCTSTR, DWORD, LPTSTR, DWORD);
//	static DWORD GetRootKeyText(HKEY, LPTSTR, DWORD);
//	static BOOL RootKeyExists(HKEY);
//	static LPTSTR GetNextRegElement(LPTSTR, LPTSTR, DWORD);

	static BOOL SaveValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static BOOL GetValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static BOOL GetValue(HKEY, LPCTSTR, LPCTSTR, LPBYTE, DWORD);

	static BOOL SaveString(LPCTSTR, LPCTSTR, LPCTSTR, DWORD);

	static BOOL GetValueGlobal(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static BOOL SaveIntGlobal(LPCTSTR, LPCTSTR, INT);

private:
	static BOOL SaveValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD, DWORD);
	static BOOL SaveValueGlobal(LPCTSTR, LPCTSTR, LPBYTE, DWORD, DWORD);
};

#endif // REGISTRY_H
