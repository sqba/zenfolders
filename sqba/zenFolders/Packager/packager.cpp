#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#define roundoffs(a,b,r) (((BYTE *) (b) - (BYTE *) (a) + ((r) - 1)) & ~((r) - 1))
#define roundpos(a,b,r) (((BYTE *) (a)) + roundoffs(a,b,r))

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


int AddFile(LPCTSTR lpInstallerFileName, LPCTSTR lpResourceFileName, int resourceId)
{
	HANDLE	hFile;
	DWORD	dwFileSize, dwBytesRead;
	LPBYTE	lpBuffer;
	int		result = 0;
	
	hFile = CreateFile(
		lpResourceFileName,
		GENERIC_READ, 
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (INVALID_HANDLE_VALUE != hFile)
	{
		dwFileSize = GetFileSize(hFile, NULL);
		
		lpBuffer = new BYTE[dwFileSize];
		
		if (ReadFile(hFile, lpBuffer, dwFileSize, &dwBytesRead, NULL) != FALSE)
		{
			HANDLE hResource;
			
			hResource = BeginUpdateResource(lpInstallerFileName, FALSE);
			if (NULL != hResource)
			{
				if (UpdateResource(
					hResource, 
					RT_RCDATA, 
					MAKEINTRESOURCE(resourceId), 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPVOID) lpBuffer, 
					dwFileSize) != FALSE)
				{
					EndUpdateResource(hResource, FALSE);
					result = 1;
					printf("Successfully added file '%s' with resource id %d to '%s'\n", lpResourceFileName, resourceId, lpInstallerFileName);
				}
				else
					printf("UpdateResource failed\n");
			}
			else
				printf("BeginUpdateResource(%s) failed\n", lpInstallerFileName);
		}
		else
			printf("ReadFile(%s) failed\n", lpResourceFileName);
		
		delete [] lpBuffer;        
		
		CloseHandle(hFile);
	}
	else
		printf("Failed to open file %s\n", lpResourceFileName);

	return result;
}
/*
DWORD GetVersionInfo(LPCTSTR lpszFileName)
{
	DWORD dwSize, dwHandle, dwResult = 0;
	dwSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
	if (0 < dwSize)
	{
		LPBYTE lpBuffer = new BYTE[dwSize];
		if (GetFileVersionInfo((LPTSTR)lpszFileName, 0, dwSize, lpBuffer) != FALSE)
		{
			// 'point to' the start of the version information block
			VS_VERSIONINFO *pVerInfo = (VS_VERSIONINFO *) lpBuffer;
			
			// the fixed section starts right after the 'VS_VERSION_INFO' string
			LPBYTE pOffsetBytes = (BYTE *) &pVerInfo->szKey[wcslen(pVerInfo->szKey) + 1];
			
			VS_FIXEDFILEINFO *pFixedInfo = (VS_FIXEDFILEINFO *) roundpos(pVerInfo, pOffsetBytes, 4);

			delete lpBuffer;
		}
	}
	return dwResult;
}
*/
void SetVersionFromDLL(LPCTSTR lpszInstallerFileName, LPCTSTR lpszDLLFileName)
{
	DWORD dwHandle, dwResult = 0;
	
	// determine the size of the resource information
	DWORD dwSizeDLL = GetFileVersionInfoSize((LPTSTR)lpszDLLFileName, &dwHandle);
	if (0 < dwSizeDLL)
	{
		LPBYTE lpBufferDLL = new BYTE[dwSizeDLL];
		
		if (GetFileVersionInfo((LPTSTR)lpszDLLFileName, 0, dwSizeDLL, lpBufferDLL) != FALSE)
		{
			// 'point to' the start of the version information block
			VS_VERSIONINFO *pVerInfoDLL = (VS_VERSIONINFO *) lpBufferDLL;
			
			// the fixed section starts right after the 'VS_VERSION_INFO' string
			LPBYTE pOffsetBytesDLL = (BYTE *) &pVerInfoDLL->szKey[wcslen(pVerInfoDLL->szKey) + 1];
			
			VS_FIXEDFILEINFO *pFixedInfoDLL = (VS_FIXEDFILEINFO *) roundpos(pVerInfoDLL, pOffsetBytesDLL, 4);


			DWORD dwSizeEXE = GetFileVersionInfoSize((LPTSTR)lpszDLLFileName, &dwHandle);
			if (0 < dwSizeEXE)
			{
				LPBYTE lpBufferEXE = new BYTE[dwSizeEXE];
				
				if (GetFileVersionInfo((LPTSTR)lpszInstallerFileName, 0, dwSizeEXE, lpBufferEXE) != FALSE)
				{
					// these macros help to align on r-byte boundaries (thanks Ted Peck)
					
					// 'point to' the start of the version information block
					VS_VERSIONINFO *pVerInfoEXE = (VS_VERSIONINFO *) lpBufferEXE;
					
					// the fixed section starts right after the 'VS_VERSION_INFO' string
					LPBYTE pOffsetBytesEXE = (BYTE *) &pVerInfoEXE->szKey[wcslen(pVerInfoEXE->szKey) + 1];
					
					VS_FIXEDFILEINFO *pFixedInfoEXE = (VS_FIXEDFILEINFO *) roundpos(pVerInfoEXE, pOffsetBytesEXE, 4);

					pFixedInfoEXE->dwFileVersionMS    = pFixedInfoDLL->dwFileVersionMS;
					pFixedInfoEXE->dwFileVersionLS    = pFixedInfoDLL->dwFileVersionLS;
					pFixedInfoEXE->dwProductVersionMS = pFixedInfoDLL->dwProductVersionMS;
					pFixedInfoEXE->dwProductVersionLS = pFixedInfoDLL->dwProductVersionLS;

					// increment the numbers!
					/*pFixedInfo->dwFileVersionMS    = pFixedInfo->dwFileVersionMS + 0x00010001;
					pFixedInfo->dwFileVersionLS    = pFixedInfo->dwFileVersionLS + 0x00010001;
					pFixedInfo->dwProductVersionMS = pFixedInfo->dwProductVersionMS + 0x00010001;
					pFixedInfo->dwProductVersionLS = pFixedInfo->dwProductVersionLS + 0x00010001;*/
					
					HANDLE hResource = BeginUpdateResource(lpszInstallerFileName, FALSE);
					if (NULL != hResource)
					{
						UINT uTemp;
						
						// get the language information
						if (VerQueryValue(lpBufferEXE, _T("\\VarFileInfo\\Translation"), (LPVOID *) &lpTranslate, &uTemp) != FALSE)
						{
							// could probably just use LANG_NEUTRAL/SUBLANG_NEUTRAL
							if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), lpTranslate->wLanguage, lpBufferEXE, dwSizeEXE) != FALSE)
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

				delete [] lpBufferEXE;
			}
			else
				dwResult = GetLastError();
		}
		else
			dwResult = GetLastError();
		
		delete [] lpBufferDLL;
	}
	else
		dwResult = GetLastError();
	
	//if (0 != dwResult)
	//	wprintf(_T("Operation was not successful.  Result = %lu\n"), dwResult);
}

int main(int argc, char* argv[])
{
	int result = 0;

	if(argc == 4)
	{
		result = AddFile(argv[1], argv[2], atoi(argv[3]));
		char *ext = strrchr(argv[2], '.');
		if(ext && (0 == strcmpi(ext, ".dll")))
			SetVersionFromDLL(argv[1], argv[2]);
	}

	return result;
}
