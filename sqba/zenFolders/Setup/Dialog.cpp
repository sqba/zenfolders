// Dialog.cpp: implementation of the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#include <malloc.h>
#include "dialog.h"
#include "resource.h"


#define RESOURCE_ID_DLL	104
#define FILENAME_DLL	"zenFolders.dll"
#define RESOURCE_ID_XML	105
#define FILENAME_XML	"zenFolders.xml"
#define FOLDER_NAME		"zenFolders"
#define UNINSTALL_KEY	"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\zenFolders"


//REGSTR_PATH_UNINSTALL


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



typedef HRESULT  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PROGRAM_FILES 38


typedef HRESULT (*DllRegisterServer)(void);
typedef HRESULT (*DllUnregisterServer)(void);
//typedef void (*DllSetPath)(LPCTSTR, int);
typedef bool (*DllIsRegistered)();


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogProc(HWND hwnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
{
	CDialog *pDlg = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		pDlg = (CDialog*)lParam;
		return pDlg->OnInit(hwnd);

	case WM_COMMAND:
		pDlg = (CDialog*)::GetWindowLong(hwnd, DWL_USER);
		return pDlg->OnCommand(LOWORD (wParam), HIWORD (wParam));

	case WM_DESTROY:
		::PostQuitMessage(0);
		return TRUE;

	case WM_CLOSE:
		pDlg = (CDialog*)::GetWindowLong(hwnd, DWL_USER);
		return pDlg->OnClose();
	}
	return FALSE;
}

CDialog::CDialog(HINSTANCE hInstance, int nCmdShow, bool bUnInstall)
{
//	m_bCleanup		= bUnInstall;
	m_bCleanup		= false;
	m_hInstance		= hInstance;
	m_bUnInstall	= bUnInstall;
	m_bNewVersion	= false;

	GetProgramFilesPath(m_szDestinationPath);
	lstrcat(m_szDestinationPath, "\\");
	lstrcat(m_szDestinationPath, FOLDER_NAME);

	::GetTempPath(MAX_PATH, m_szTempPath);
	::GetModuleFileName(NULL, m_szModulePath, MAX_PATH);

	HWND hwnd = ::CreateDialogParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_MAIN), 
		0, 
		DialogProc,
		(LPARAM)this);
	
	if (!hwnd)
	{
		char buf[100];
		wsprintf(buf, "Error x%x", ::GetLastError());
		::MessageBox(0, buf, "CreateDialog", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	::SetWindowLong(hwnd, DWL_USER, (LONG)this);
	
	::ShowWindow(hwnd, nCmdShow) ;

	m_hwnd = hwnd;

	//::SetFocus( GetDlgItem(m_hDlg, IDC_FOLDER_NAME) );
}

CDialog::~CDialog()
{

}

BOOL CDialog::OnInit(HWND hwnd)
{
	TCHAR szCaption[MAX_PATH] = {0};

	m_hwnd = hwnd;

	if( IsAlreadyInstalled() )
	{
		if( IsSameVersion() )
			m_bUnInstall = true;
		else
			m_bNewVersion = true;
	}

	// Set dialog icon, big and small
	::SendMessage(hwnd, WM_SETICON, ICON_BIG,
		(LPARAM)LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	::SendMessage(hwnd, WM_SETICON, ICON_SMALL,
		(LPARAM)LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN)));

	// Set dialog caption
	int caption = m_bUnInstall ? IDS_CAPTION_UNINSTALL : IDS_CAPTION_INSTALL;
	::LoadString(m_hInstance, caption, szCaption, sizeof(szCaption));
	::SetWindowText(hwnd, szCaption);

	DisplayVersion();

	// Set dialog message
	if(m_bUnInstall)
		SetMessage( IDS_BODY_UNINSTALL );
	else
		SetMessage( IDS_BODY_INSTALL, m_szDestinationPath );

	return TRUE;
}

BOOL CDialog::OnClose()
{
	::DestroyWindow(m_hwnd);
	return TRUE;
}

BOOL CDialog::OnCommand(int id, int code)
{
	switch(id)
	{
	case IDOK:
		if(m_bNewVersion)
		{
			if( Uninstall() )
				return Install();
			else
			{
				// Write something to RunOnce registry key
				// Keep in mind that Uninstall() already did!
				SetMessage( IDS_RESTARTBEFOREUPDATE );
			}
		}
		else if(m_bUnInstall)
			return Uninstall();
		else
			return Install();
		break;

	case IDCANCEL:
		if(m_bUnInstall)
			CreateUninstall(TRUE);
		::PostQuitMessage(1);
		break;
	}
	return FALSE;
}

bool CDialog::RegisterActiveX(LPCTSTR lpszPath)
{
	bool result = FALSE;

	HMODULE hLibrary = ::LoadLibrary(lpszPath);
	if (NULL != hLibrary)
	{
		DllRegisterServer func;
		func = (DllRegisterServer)::GetProcAddress(hLibrary, TEXT("DllRegisterServer"));
		if(func != NULL)
		{
			result = SUCCEEDED( func() );
			/*if(result)
			{
				DllSetPath func2;
				func2 = (DllSetPath)::GetProcAddress(hLibrary, TEXT("DllSetPath"));
				if(func2)
				{
					func2(lpszPath, lstrlen(lpszPath)*sizeof(TCHAR));
				}
			}*/
		}
		FreeLibrary( hLibrary );
	}

	return result;
}

bool CDialog::UnRegisterActiveX(LPCTSTR lpszPath)
{
	bool result = FALSE;

	HMODULE hLibrary = ::LoadLibrary(lpszPath);
	if (NULL != hLibrary)
	{
		DllUnregisterServer func;
		func = (DllUnregisterServer)::GetProcAddress(hLibrary, TEXT("DllUnregisterServer"));
		if(func != NULL)
		{
			result = SUCCEEDED( func() );
		}
		FreeLibrary( hLibrary );
	}

	return result;
}

BOOL CDialog::Install()
{
	BOOL result = FALSE;
	TCHAR szMessage[MAX_PATH] = {0};
	TCHAR szPath[MAX_PATH] = {0};
	int msg = IDS_INSTALL_SUCCESS;

	SetMessage( TEXT("") );

	lstrcpy(szPath, m_szDestinationPath);

	if( Exists(szPath) || ::CreateDirectory(szPath, NULL) )
	{
		if( ExtractResourceToFile(RESOURCE_ID_DLL, FILENAME_DLL, true) )
		{
			ExtractResourceToFile(RESOURCE_ID_XML, FILENAME_XML, false);

			lstrcat(szPath, "\\");
			lstrcat(szPath, FILENAME_DLL);

			HWND hwndCancel = ::GetDlgItem(m_hwnd, IDCANCEL);
			::ShowWindow(hwndCancel, SW_HIDE) ;

			if( RegisterActiveX(szPath) )
			{
				if( CreateUninstall(FALSE) )
					msg = IDS_INSTALL_SUCCESS;
				else
				{
				}

				result = TRUE;
			}
			else
				msg = IDS_REGISTERACTIVEXFAILED;
		}
		else
			msg = IDS_INSTALL_FAILED;

		AppendMessage( msg, NULL, true );
	}
	else
	{
		AppendMessage( IDS_CREATEFOLDERFAILED, szPath, true );
	}

	Finish();

	return result;
}

BOOL CDialog::Uninstall()
{
	BOOL result = TRUE;
	TCHAR szPath[MAX_PATH] = {0};
	TCHAR szMessage[MAX_PATH] = {0};

	SetMessage( TEXT("") );

	int msg = IDS_UNINSTALL_RESTART;

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_DLL);

	if( UnRegisterActiveX(szPath) )
	{
		RegDeleteKey( HKEY_LOCAL_MACHINE, UNINSTALL_KEY);
		if( ::DeleteFile(szPath) )
		{
			m_bCleanup = true; // Tell main to call CleanUp()
			msg = IDS_UNINSTALL_SUCCESS;
		}
		else
		{
			// Write something to RunOnce registry key
		}
	}
	else
	{
		msg = IDS_UNREGISTERACTIVEXFAILED;
		result = FALSE;
	}

	m_bUnInstall = false;

	AppendMessage( msg, NULL, true );

	Finish();

	return result;
}

BOOL CDialog::CreateUninstall(BOOL silent)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;
	TCHAR szMessage[MAX_PATH] = {0};

	TCHAR szSrcPath[MAX_PATH] = {0};
	strcat(szSrcPath, m_szModulePath);

	TCHAR szDstPath[MAX_PATH] = {0};
	LPCTSTR pszModuleName = strrchr(m_szModulePath, '\\') + 1;
	lstrcpy(szDstPath, m_szDestinationPath);
	strcat(szDstPath, "\\");
	strcat(szDstPath, pszModuleName);

	if( !::CopyFile(szSrcPath, szDstPath, FALSE) )
	{
		if(!silent)
		{
			::LoadString(m_hInstance, IDS_SETUPCOPYFAILED, szMessage, sizeof(szMessage));
			SetMessage( szMessage );
			return FALSE;
		}
	}

	lResult = ::RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\zenFolders"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL, 
		&hKey,
		&dwDisp);
	
	if(lResult != ERROR_SUCCESS)
	{
		if(!silent)
		{
			::LoadString(m_hInstance, IDS_CREATEUNINSTALLKEYFAILED, szMessage, sizeof(szMessage));
			SetMessage( szMessage );
			return FALSE;
		}
	}
	
	lResult = ::RegSetValueEx(
		hKey,
		TEXT("DisplayName"),
		0,
		REG_SZ,
		(LPBYTE)"zenFolders",
		sizeof(TEXT("zenFolders")));

	strcat(szDstPath, " uninstall");

	lResult = ::RegSetValueEx(
		hKey,
		TEXT("UninstallString"),
		0,
		REG_EXPAND_SZ,
		(LPBYTE)szDstPath,
		sizeof(szDstPath));

	RegCloseKey(hKey);

	if(lResult != ERROR_SUCCESS)
	{
		if(!silent)
		{
			::LoadString(m_hInstance, IDS_SETUNINSTALLKEYFAILED, szMessage, sizeof(szMessage));
			SetMessage( szMessage );
			return FALSE;
		}
	}
	
	return TRUE;
}

bool CDialog::ExtractResourceToFile(int resourceId, LPCTSTR lpResourceFileName, bool bOverwrite)
{
	HMODULE	hLibrary;
	HRSRC	hResource;
	HGLOBAL	hResourceLoaded;
	LPBYTE	lpBuffer;
	bool	result = false;
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, lpResourceFileName);

	AppendMessage( IDS_EXTRACTING, szPath, true );
	
	hLibrary = m_hInstance;
	hResource = ::FindResource(hLibrary, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (NULL != hResource)
	{
		hResourceLoaded = ::LoadResource(hLibrary, hResource);
		if (NULL != hResourceLoaded)        
		{
			lpBuffer = (LPBYTE)::LockResource(hResourceLoaded);            
			if (NULL != lpBuffer)            
			{                
				DWORD	dwFileSize, dwBytesWritten;
				HANDLE	hFile;
				
				dwFileSize = ::SizeofResource(hLibrary, hResource);
				
				hFile = ::CreateFile(
					szPath,
					GENERIC_WRITE,
					0,
					NULL,
					(bOverwrite ? CREATE_ALWAYS : CREATE_NEW),
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				
				if (INVALID_HANDLE_VALUE != hFile)
				{
					::WriteFile(hFile, lpBuffer, dwFileSize, &dwBytesWritten, NULL);
					
					::CloseHandle(hFile);

					result = true;

					AppendMessage( IDS_EXTRACTINGOK, NULL, false );
				}
				else if(bOverwrite)
					AppendMessage( IDS_EXTRACTINGFAILED, NULL, false );
				else
					AppendMessage( IDS_EXTRACTINGSKIPPED, NULL, false );
			}
			else
				AppendMessage( IDS_LOCKRESOURCEFAILED, NULL, false );
		}  
		else
			AppendMessage( IDS_LOADRESOURCEFAILED, NULL, false );
	}
	else
		AppendMessage( IDS_RESOURCENOTFOUND, NULL, false );

	if(!result)
	{
		Finish();
	}

	return result;
}

bool CDialog::GetProgramFilesPath(LPTSTR pszPath)
{
	bool result = false;
	HMODULE hModule = LoadLibrary("SHFOLDER.DLL");
	if (hModule != NULL)
	{
		SHGETFOLDERPATH fnShGetFolderPath;
		fnShGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(
			hModule, "SHGetFolderPathA");
		
		if (fnShGetFolderPath != NULL)
		{
			HRESULT hr = fnShGetFolderPath(
				NULL,
				CSIDL_PROGRAM_FILES,
				NULL,
				0,
				pszPath);
			result = (S_OK == hr);
			
		}
		FreeLibrary(hModule);
	}
	return result;
}

void CDialog::CleanUp()
{
	const char tempbatname[] = "_uninsep.bat" ;

	// temporary .bat file
	static char templ[] = 
		":Repeat\r\n"
		"del \"%s\"\r\n"
		"if exist \"%s\" goto Repeat\r\n"
		"del \"%s\"" ;
	
	char modulename[MAX_PATH] ;    // absolute path of calling .exe file
	char temppath[MAX_PATH] ;      // absolute path of temporary .bat file
	char folder[MAX_PATH] ;
	
	::GetTempPath(MAX_PATH, temppath) ;
	strcat(temppath, tempbatname) ;
	
	::GetModuleFileName(NULL, modulename, MAX_PATH) ;
	strcpy (folder, modulename) ;
	char *pb = strrchr(folder, '\\');
	if (pb != NULL)
		*pb = 0 ;

	TCHAR moduleName2[MAX_PATH] = {0};
	lstrcpy(moduleName2, m_szDestinationPath);
	lstrcat(moduleName2, "\\");
	lstrcat(moduleName2, strrchr(modulename, '\\')+1);
	
	HANDLE hf ;
	
	hf = ::CreateFile(
		temppath,
		GENERIC_WRITE,
		0,
		NULL, 
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL) ;
	
	if (hf != INVALID_HANDLE_VALUE)
	{
		DWORD len ;
		char *bat ;
		
		bat = (char*)alloca(strlen(templ) + 
			strlen(moduleName2) * 2 + strlen(temppath) + 20) ;
		
		wsprintf(bat, templ, moduleName2, moduleName2, temppath) ;
		
		::WriteFile(hf, bat, strlen(bat), &len, NULL) ;
		::CloseHandle(hf) ;
		
		::ShellExecute(NULL, "open", temppath, NULL, NULL, SW_HIDE);
	}
}

void CDialog::Finish()
{
	HWND hwndOK = ::GetDlgItem(m_hwnd, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;

	::SetDlgItemText(m_hwnd, IDCANCEL, TEXT("Close"));
	HWND hwndCancel = ::GetDlgItem(m_hwnd, IDCANCEL);
	::ShowWindow(hwndCancel, SW_SHOW) ;
}

bool CDialog::IsAlreadyInstalled()
{
	bool result = false;
	TCHAR szPath[MAX_PATH] = {0};
	lstrcpy(szPath, m_szDestinationPath);

	if( Exists(szPath) )
	{
		lstrcat(szPath, "\\");
		lstrcat(szPath, FILENAME_DLL);
		if( Exists(szPath) )
		{
			// Check if the DLL is registered
			HMODULE hLibrary = ::LoadLibrary(szPath);
			if (NULL != hLibrary)
			{
				DllIsRegistered IsRegistered;
				IsRegistered = (DllIsRegistered)::GetProcAddress(hLibrary, TEXT("DllIsRegistered"));
				if(IsRegistered != NULL)
				{
					result = IsRegistered();
				}
				FreeLibrary( hLibrary );
			}
		}
	}
	return result;
}

bool CDialog::IsSameVersion()
{
	TCHAR szPath[MAX_PATH] = {0};
	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_DLL);

	DWORD a1, b1, c1, d1;
	if( GetVersion(szPath, &a1, &b1, &c1, &d1) )
	{
		DWORD a2, b2, c2, d2;
		if( GetVersion(m_szModulePath, &a2, &b2, &c2, &d2) )
		{
			return ((a1==a2) && (b1==b2) && (c1==c2) && (d1=d2));
		}
	}

	return false;
}

bool CDialog::GetVersion(LPCTSTR lpszFilename, DWORD *a, DWORD *b, DWORD *c, DWORD *d)
{
	DWORD dwHandle, dwResult = 0;
	
	// determine the size of the resource information
	DWORD dwSize = ::GetFileVersionInfoSize((LPTSTR)lpszFilename, &dwHandle);
	if (0 < dwSize)
	{
		LPBYTE lpBuffer = new BYTE[dwSize];
		
		if (::GetFileVersionInfo((LPTSTR)lpszFilename, 0, dwSize, lpBuffer) != FALSE)
		{
			// 'point to' the start of the version information block
			VS_VERSIONINFO *pVerInfo = (VS_VERSIONINFO*)lpBuffer;
			
			// the fixed section starts right after the 'VS_VERSION_INFO' string
			LPBYTE pOffsetBytes = (BYTE*)&pVerInfo->szKey[wcslen(pVerInfo->szKey) + 1];
			
			VS_FIXEDFILEINFO *pFixedInfo = (VS_FIXEDFILEINFO*)roundpos(pVerInfo, pOffsetBytes, 4);

			*a = pFixedInfo->dwFileVersionMS;
			*b = pFixedInfo->dwFileVersionLS;
			*c = pFixedInfo->dwProductVersionMS;
			*d = pFixedInfo->dwProductVersionLS;
		}
		
		delete [] lpBuffer;

		return true;
	}

	return false;
}

bool CDialog::Exists(LPCTSTR lpszPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = ::FindFirstFile(lpszPath, &FindFileData);
	return (hFind != INVALID_HANDLE_VALUE);
}

void CDialog::SetMessage(LPCTSTR pszMessage)
{
	::SetDlgItemText(m_hwnd, IDC_MESSAGE, pszMessage);
}

void CDialog::SetMessage(int msg)
{
	TCHAR szMessage[MAX_PATH] = {0};

	::LoadString(m_hInstance, msg, szMessage, sizeof(szMessage));

	SetMessage( szMessage );
}

void CDialog::SetMessage(int msg, LPCTSTR pszMessage)
{
	TCHAR szTemp[MAX_PATH] = {0};
	TCHAR szMessage[MAX_PATH] = {0};

	::LoadString(m_hInstance, msg, szTemp, sizeof(szTemp));

	wsprintf(szMessage, szTemp, pszMessage);

	SetMessage( szMessage );
}

void CDialog::AppendMessage(int msg, LPCTSTR pszParam, bool bNewLine)
{
	UINT uLen;
	TCHAR szMessage[MAX_PATH] = {0};
	TCHAR szBody[MAX_PATH] = {0};

	uLen = ::GetDlgItemText(
		m_hwnd,
		IDC_MESSAGE,
		szBody,
		sizeof(szBody));

	if(pszParam)
	{
		TCHAR szTemp[MAX_PATH] = {0};
		::LoadString(m_hInstance, msg, szTemp, sizeof(szTemp));
		wsprintf(szMessage, szTemp, pszParam);
	}
	else
	{
		::LoadString(m_hInstance, msg, szMessage, sizeof(szMessage));
	}

	if(bNewLine && (lstrlen(szBody) > 0))
		lstrcat(szBody, "\n");
	lstrcat(szBody, szMessage);

	SetMessage( szBody );
}

#define roundoffs(a,b,r) (((BYTE *) (b) - (BYTE *) (a) + ((r) - 1)) & ~((r) - 1))
#define roundpos(a,b,r) (((BYTE *) (a)) + roundoffs(a,b,r))

void CDialog::DisplayVersion()
{
	HANDLE	hMem;
	LPSTR	lpstrVffInfo;
	DWORD	dwVerHnd=0;
	DWORD	dwVerInfoSize;
	TCHAR	szFullPath[MAX_PATH] = {0};

	if(m_bUnInstall)
	{
		lstrcpy(szFullPath, m_szDestinationPath);
		lstrcat(szFullPath, "\\");
		lstrcat(szFullPath, FILENAME_DLL);
	}
	else
	{
		::GetModuleFileName(m_hInstance, szFullPath, MAX_PATH);
	}

	dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	
	if(dwVerInfoSize)
	{
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)::GlobalLock(hMem);
		::GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		VS_VERSIONINFO *pVerInfo = (VS_VERSIONINFO*)lpstrVffInfo;
		LPBYTE pOffsetBytes = (BYTE*)&pVerInfo->szKey[wcslen(pVerInfo->szKey) + 1];
		VS_FIXEDFILEINFO *pFixedInfo = (VS_FIXEDFILEINFO*) roundpos(pFixedInfo, pOffsetBytes, 4);
		//DWORD a = pFixedInfo->dwFileVersionMS;
		//DWORD b = pFixedInfo->dwFileVersionLS;
		DWORD a = pFixedInfo->dwProductVersionMS;
		DWORD b = pFixedInfo->dwProductVersionLS;
		TCHAR	szVersion[MAX_PATH] = {0};
		wsprintf(szVersion, "%d,%d,%d,%d", HIWORD(a), LOWORD(a), HIWORD(b), LOWORD(b));

		//SetMessage(IDS_VERSION, szVersion);

		TCHAR szFormat[MAX_PATH] = {0};
		TCHAR szMessage[MAX_PATH] = {0};
		::LoadString(m_hInstance, IDS_VERSION, szFormat, sizeof(szFormat));
		wsprintf(szMessage, szFormat, szVersion);
		::SetDlgItemText(m_hwnd, IDC_VERSION, szMessage);

		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
}
