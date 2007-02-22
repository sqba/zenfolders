// Dialog.cpp: implementation of the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#include <malloc.h>
#include "dialog.h"
#include "resource.h"


#define RESOURCE_ID_DLL		104
#define FILENAME_DLL		"zenFolders.dll"
#define RESOURCE_ID_XML		105
#define FILENAME_XML		"zenFolders.xml"
#define FOLDER_NAME			TEXT("zenFolders")
#define UNINSTALL_KEY		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\zenFolders")


//REGSTR_PATH_UNINSTALL


typedef HRESULT  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PROGRAM_FILES 38


typedef HRESULT (*DllRegisterServer)(void);
typedef HRESULT (*DllUnregisterServer)(void);
typedef void (*DllSetPath)(LPCTSTR, int);


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
	m_hwnd = hwnd;

	BOOL result = FALSE;
	TCHAR szMessage[MAX_PATH] = {0};
	TCHAR szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	if( !m_bUnInstall && DirectoryExists(szPath) )
	{
		lstrcat(szPath, "\\");
		lstrcat(szPath, FILENAME_DLL);
		if( DirectoryExists(szPath) )
		{
			m_bUnInstall = true;//!m_bUnInstall;
		}
	}

	if(m_bUnInstall)
	{
		//::LoadString(m_hInstance, IDS_COLUMN1, szString, sizeof(szString));
		::SetWindowText(hwnd, TEXT("zenFolders Removal Wizard"));
		SetMessage( TEXT("Remove zenFolders?") );
	}
	else
	{
		::SetWindowText(hwnd, TEXT("zenFolders Installation Wizard"));
		TCHAR szMessage[MAX_PATH] = {0};
		wsprintf(szMessage, TEXT("zenFolders will be installed in: '%s'"), m_szDestinationPath);
		SetMessage( szMessage );
	}

	::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN)));

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
		if(m_bUnInstall)
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

void CDialog::SetMessage(LPCTSTR pszMessage)
{
	::SetDlgItemText(m_hwnd, IDC_MESSAGE, pszMessage);
}

void CDialog::Finish()
{
	HWND hwndOK = ::GetDlgItem(m_hwnd, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;

	::SetDlgItemText(m_hwnd, IDCANCEL, TEXT("Close"));
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
			if(result)
			{
				DllSetPath func2;
				func2 = (DllSetPath)::GetProcAddress(hLibrary, TEXT("DllSetPath"));
				if(func2)
				{
					func2(lpszPath, lstrlen(lpszPath)*sizeof(TCHAR));
				}
			}
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

bool CDialog::DirectoryExists(LPCTSTR lpszPath)
{
	WIN32_FIND_DATA FindFileData;

	HANDLE hFind = FindFirstFile(lpszPath, &FindFileData);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	return false;
}

BOOL CDialog::Install()
{
	BOOL result = FALSE;
	TCHAR szMessage[MAX_PATH] = {0};
	TCHAR szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	if( DirectoryExists(szPath) || ::CreateDirectory(szPath, NULL) )
	{
		if( ExtractResourceToFile(RESOURCE_ID_DLL, FILENAME_DLL, false) )
		{
			ExtractResourceToFile(RESOURCE_ID_XML, FILENAME_XML, false);

			lstrcat(szPath, "\\");
			lstrcat(szPath, FILENAME_DLL);

			if( RegisterActiveX(szPath) )
			{
				if( CreateUninstall(FALSE) )
				{
					wsprintf(szMessage, TEXT("zenFolders succesfully installed."));
				}

				result = TRUE;
			}
			else
				wsprintf(szMessage, "Failed to register ActiveX control.");
		}
		else
		{
			//if( DirectoryExists(szPath) )
			//return Uninstall();
			wsprintf(szMessage, "Failed to extract file %s: \nProbably previos uninstallation failed.\nPlease restart the computer before installing again.\nThank You", szPath);
		}
	}
	else
		wsprintf(szMessage, "Failed to create folder %s", szPath);

	SetMessage( szMessage );

	Finish();

	return result;
}

BOOL CDialog::Uninstall()
{
	BOOL result = FALSE;
	TCHAR szPath[MAX_PATH] = {0};
	TCHAR szMessage[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_DLL);

	if( UnRegisterActiveX(szPath) )
	{
		if( ::DeleteFile(szPath) )
		{
			RegDeleteKey(
				HKEY_LOCAL_MACHINE,
				UNINSTALL_KEY);

			result = true;

			m_bCleanup = true; // Tell main to call CleanUp()

			wsprintf(szMessage, "zenFolders successfully uninstalled.\nThank you for using zenFolders.\nBye!");
		}
		else
			wsprintf(szMessage, TEXT("Failed to remove all files.\nNothing to worry about.\nPlease restart and try uninstall again.\nSorry for the inconvenience..."));
	}
	else
		wsprintf(szMessage, "Failed to unregister ActiveX control.");

	m_bUnInstall = false;

	SetMessage( szMessage );

	Finish();

	return result;
}

BOOL CDialog::CreateUninstall(BOOL silent)
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;


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
			SetMessage( TEXT("Failed to copy setup.") );
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
			SetMessage( TEXT("RegCreateKeyEx failed.") );
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
			SetMessage( TEXT("RegSetValueEx failed.") );
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
	TCHAR	szMessage[MAX_PATH] = {0};
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, lpResourceFileName);

	wsprintf(szMessage, "Extracting %s: ", szPath);
	SetMessage(szMessage);
	
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

					lstrcat(szMessage, TEXT("OK"));
				}
				else if(bOverwrite)
					wsprintf(szMessage, "Failed to extract %s", szPath);
				else
					wsprintf(szMessage, "Skipped %s", szPath);
			}
			else
				lstrcat(szMessage, TEXT("Failed to lock resource"));
		}  
		else
			wsprintf(szMessage, "Failed to load resource %d", resourceId);
	}
	else
		wsprintf(szMessage, "Resource %d not found", resourceId);

	SetMessage(szMessage);

	if(!result)
	{
		//HideOkButton();
		//SetCancelButtonText( TEXT("Close") );
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



/*
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\zenFolders
 DisplayName
 UninstallString

DisplayName REG_SZ ProductName Display name of application 
UninstallPath REG_EXPAND_SZ N/A Full path to the application's uninstall program 
InstallLocation REG_EXPAND_SZ ARPINSTALLLOCATION Full path where application is located (folder or .exe) 
Publisher REG_SZ Manufacturer Publisher/Developer of application 
VersionMajor DWORD ProductVersion Major version number of application 
VersionMinor DWORD ProductVersion Minor version of application 

*/




