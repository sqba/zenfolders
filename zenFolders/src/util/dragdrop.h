#ifndef DRAGDROP_H
#define DRAGDROP_H

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <Regstr.h>

#ifndef SFGAO_BROWSABLE
#define SFGAO_BROWSABLE (0)
#endif

#ifndef SHGDN_INCLUDE_NONFILESYS
#define SHGDN_INCLUDE_NONFILESYS (0)
#endif

#ifdef _cplusplus
extern "C" {
#endif   //_cplusplus

typedef struct
{
    BOOL fCut;
    UINT cidl;
    UINT aoffset[1];
} PRIVCLIPDATA, FAR *LPPRIVCLIPDATA;

class CDragAndDrop  
{
public:
	static HGLOBAL CreateHDrop(IShellFolder*, LPITEMIDLIST*, UINT);
	static HGLOBAL CreatePrivateClipboardData(LPITEMIDLIST, LPITEMIDLIST*, UINT, BOOL);
	static HGLOBAL CreateShellIDList(LPITEMIDLIST, LPITEMIDLIST*, UINT);

private:
	static BOOL GetTextFromSTRRET(IMalloc*, LPSTRRET, LPCITEMIDLIST, LPTSTR, DWORD);
};

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#endif   //DRAGDROP_H