#ifndef REGISTRY_H
#define REGISTRY_H

#include <windows.h>
#include <regstr.h>

class CRegistry  
{
public:
	static bool SaveValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static bool GetValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static bool GetValue(HKEY, LPCTSTR, LPCTSTR, LPBYTE, DWORD);

	static bool SaveString(LPCTSTR, LPCTSTR, LPCTSTR, DWORD);

	static bool GetValueGlobal(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
	static bool SaveIntGlobal(LPCTSTR, LPCTSTR, INT);

private:
	static bool SaveValue(LPCTSTR, LPCTSTR, LPBYTE, DWORD, DWORD);
	static bool SaveValueGlobal(LPCTSTR, LPCTSTR, LPBYTE, DWORD, DWORD);
};

#endif // REGISTRY_H
