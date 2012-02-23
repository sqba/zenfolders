#ifndef SHELLNSE_H
#define SHELLNSE_H

#include <objbase.h>
#include <olectl.h>
#include <shlobj.h>


typedef struct {
	HKEY  hRootKey;
	LPTSTR lpszSubKey;
	LPTSTR lpszValueName;
	LPTSTR lpszData;
} REGSTRUCT, FAR *LPREGSTRUCT;


class CShellNSE  
{
public:
	static HRESULT RegisterServer(HINSTANCE, GUID);
	static HRESULT UnregisterServer(HINSTANCE, GUID);

	static HRESULT RegisterProperties(HINSTANCE, GUID, GUID);
	static HRESULT UnregisterProperties(HINSTANCE, GUID);

	static bool IsRegistered(HINSTANCE, GUID);

	static int GetName(GUID, LPCTSTR, int);

private:
	static HRESULT CreateSubKey(LPCTSTR, LPREGSTRUCT, LPCTSTR);
	static HRESULT CreateSubKeys(LPCTSTR, LPCTSTR, REGSTRUCT*);
//	static HRESULT RemoveSubKeys(LPCTSTR, REGSTRUCT*);
};

#endif // SHELLNSE_H
