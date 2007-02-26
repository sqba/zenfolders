#include "stdafx.h"

struct VS_VERSIONINFO
{
    WORD                wLength;
    WORD                wValueLength;
    WORD                wType;
    WCHAR               szKey[1];
    WORD                wPadding1[1];
    VS_FIXEDFILEINFO    Value;
    WORD                wPadding2[1];
    WORD                wChildren[1];
};

struct
{
    WORD wLanguage;
    WORD wCodePage;
} *lpTranslate;

void main( void )
{
    VS_VERSIONINFO      *pVerInfo;
    LPBYTE              pOffsetBytes;
    VS_FIXEDFILEINFO    *pFixedInfo;
    LPCTSTR             lpszFile = _T("c:\\winnt\\system32\\atl.dll");
    DWORD               dwHandle,
                        dwSize,
                        dwResult = 0;

    
    // determine the size of the resource information
    dwSize = GetFileVersionInfoSize(lpszFile, &dwHandle);
    if (0 < dwSize)
    {
        LPBYTE lpBuffer = new BYTE[dwSize];
        
        if (GetFileVersionInfo(lpszFile, 0, dwSize, lpBuffer) != FALSE)
        {
            // these macros help to align on r-byte boundaries (thanks Ted Peck)
            #define roundoffs(a,b,r) (((BYTE *) (b) - (BYTE *) (a) + ((r) - 1)) & ~((r) - 1))
            #define roundpos(a,b,r) (((BYTE *) (a)) + roundoffs(a,b,r))
            
            // 'point to' the start of the version information block
            pVerInfo = (VS_VERSIONINFO *) lpBuffer;
            
            // the fixed section starts right after the 'VS_VERSION_INFO' string
            pOffsetBytes = (BYTE *) &pVerInfo->szKey[_tcslen(pVerInfo->szKey) + 1];

            pFixedInfo = (VS_FIXEDFILEINFO *) roundpos(pVerInfo, pOffsetBytes, 4);

            // increment the numbers!
            pFixedInfo->dwFileVersionMS    = pFixedInfo->dwFileVersionMS + 0x00010001;
            pFixedInfo->dwFileVersionLS    = pFixedInfo->dwFileVersionLS + 0x00010001;
            pFixedInfo->dwProductVersionMS = pFixedInfo->dwProductVersionMS + 0x00010001;
            pFixedInfo->dwProductVersionLS = pFixedInfo->dwProductVersionLS + 0x00010001;

            HANDLE hResource = BeginUpdateResource(lpszFile, FALSE);
            if (NULL != hResource)
            {
                UINT uTemp;

                // get the language information
                if (VerQueryValue(lpBuffer, _T("\\VarFileInfo\\Translation"), (LPVOID *) &lpTranslate, &uTemp) != FALSE)
                {
                    // could probably just use LANG_NEUTRAL/SUBLANG_NEUTRAL
                    if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), lpTranslate->wLanguage, lpBuffer, dwSize) != FALSE)
                    {
                        if (EndUpdateResource(hResource, FALSE) == FALSE)
                            dwResult = GetLastError();
                    }
                    else
                        dwResult = GetLastError();
                }
            }
            else
                dwResult = GetLastError();
        }
        else
            dwResult = GetLastError();

        delete [] lpBuffer;
    }
    else
        dwResult = GetLastError();

    if (0 != dwResult)
        wprintf(_T("Operation was not successful.  Result = %lu\n"), dwResult);
}
