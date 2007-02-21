// Dialog.cpp: implementation of the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "dialog.h"
#include "resource.h"


#define RESOURCE_ID_DLL		104
#define FILENAME_DLL		"zenFolders.dll"
#define RESOURCE_ID_XML		105
#define FILENAME_XML		"zenFolders.xml"
#define FOLDER_NAME			TEXT("zenFolders")


typedef HRESULT  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PROGRAM_FILES 38


typedef HRESULT (*DllRegisterServer)(void);
typedef HRESULT (*DllUnregisterServer)(void);


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
		return pDlg->OnInit();

	case WM_COMMAND:
		pDlg = (CDialog*)::GetWindowLong(hwnd, DWL_USER);
		return pDlg->OnCommand(LOWORD (wParam), HIWORD (wParam));

	case WM_DESTROY:
		::PostQuitMessage(0);
		return TRUE;

	case WM_CLOSE:
		pDlg = (CDialog*)lParam;
		return pDlg->OnClose();
	}
	return FALSE;
}

CDialog::CDialog(HINSTANCE hInstance, int nCmdShow, bool bUnInstall)
{
	m_bCleanup = false;
	m_hInstance = hInstance;
	m_bUnInstall = bUnInstall;

	GetProgramFilesPath(m_szDestinationPath);
	lstrcat(m_szDestinationPath, "\\");
	lstrcat(m_szDestinationPath, FOLDER_NAME);

	::GetTempPath(MAX_PATH, m_szTempPath);
	::GetModuleFileName(NULL, m_szModulePath, MAX_PATH);

	m_hwnd = ::CreateDialog(
		hInstance, 
		MAKEINTRESOURCE(IDD_MAIN), 
		0, 
		DialogProc);
	
	if (!m_hwnd)
	{
		char buf[100];
		wsprintf(buf, "Error x%x", ::GetLastError());
		::MessageBox(0, buf, "CreateDialog", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	::SetWindowLong(m_hwnd, DWL_USER, (LONG)this);
	
	::ShowWindow(m_hwnd, nCmdShow) ;

	//::SetFocus( GetDlgItem(m_hDlg, IDC_FOLDER_NAME) );

	if(bUnInstall)
	{
		SetMessage( TEXT("Remove zenFolders?") );
	}
	else
	{
		TCHAR szMessage[MAX_PATH] = {0};
		wsprintf(szMessage, TEXT("zenFolders will be installed in '%s'"), m_szDestinationPath);
		SetMessage( szMessage );
	}
}

CDialog::~CDialog()
{

}

BOOL CDialog::OnInit()
{
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
		//::EndDialog(g_hDialog, IDCANCEL);
		//::DestroyWindow(g_hDialog);
		if(m_bUnInstall)
			CreateUninstall();
		::PostQuitMessage(1);
		break;
	}
	return FALSE;
}

BOOL CDialog::OnClose()
{
	::DestroyWindow(m_hwnd);
	return TRUE;
}

void CDialog::SetMessage(LPCTSTR pszMessage)
{
	::SetDlgItemText(m_hwnd, IDC_MESSAGE, pszMessage);
}
/*
void CDialog::HideOkButton()
{
	HWND hwndOK = ::GetDlgItem(m_hwnd, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;
}

void CDialog::SetCancelButtonText(LPCTSTR pszCaption)
{
	::SetDlgItemText(m_hwnd, IDCANCEL, pszCaption);
}
*/
void CDialog::Finish()
{
	HWND hwndOK = ::GetDlgItem(m_hwnd, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;

	::SetDlgItemText(m_hwnd, IDCANCEL, TEXT("Close"));
}

BOOL CDialog::Uninstall()
{
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_DLL);

	HMODULE hLibrary = LoadLibrary(szPath);
	if (NULL != hLibrary)
	{
		DllUnregisterServer func;
		func = (DllUnregisterServer)GetProcAddress(hLibrary, TEXT("DllUnregisterServer"));
		if(func != NULL)
		{
			HRESULT hr = func();
			FreeLibrary( hLibrary );
			if( SUCCEEDED(hr) )
			{
				if( ::DeleteFile(szPath) )
				{
					RegDeleteKey(
						HKEY_LOCAL_MACHINE,
						TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\zenFolders"));

					m_bCleanup = true; // Tell main to call CleanUp()

					SetMessage( TEXT("zenFolders successfully uninstalled.\nThank you for using zenFolders.\nBye!") );
				}
				else
				{
					SetMessage( TEXT("Failed to remove all files.\nPlease restart and try uninstall again.\nSorry for the inconvenience...") );
				}
			}
		}
		FreeLibrary( hLibrary );
	}

	m_bUnInstall = false;

	//HideOkButton();
	//SetCancelButtonText( TEXT("Close") );
	Finish();

	return TRUE;
}

BOOL CDialog::Install()
{
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	BOOL b = ::CreateDirectory(szPath, NULL);

	if( ExtractResourceToFile(RESOURCE_ID_DLL, FILENAME_DLL, true) )
	{
		ExtractResourceToFile(RESOURCE_ID_XML, FILENAME_XML, false);

		if( RegisterActiveX() )
		{
			if( CreateUninstall() )
			{
				SetMessage( TEXT("zenFolders succesfully installed.") );
			}

			//HideOkButton();
			//SetCancelButtonText( TEXT("Close") );
			Finish();

			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDialog::CreateUninstall()
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;

//REGSTR_PATH_UNINSTALL

	TCHAR szSrcPath[MAX_PATH] = {0};
//	DWORD dwRet = ::GetCurrentDirectory(MAX_PATH, szSrcPath);
//	strcat(szSrcPath, "\\");
	strcat(szSrcPath, m_szModulePath);

	TCHAR szDstPath[MAX_PATH] = {0};
	LPCTSTR pszModuleName = strrchr(m_szModulePath, '\\') + 1;
	lstrcpy(szDstPath, m_szDestinationPath);
	strcat(szDstPath, "\\");
	strcat(szDstPath, pszModuleName);

	if( !::CopyFile(szSrcPath, szDstPath, FALSE) )
	{
		SetMessage( TEXT("Failed to copy setup.") );
		return FALSE;
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
		SetMessage( TEXT("RegCreateKeyEx failed.") );
		return FALSE;
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
		SetMessage( TEXT("RegSetValueEx failed.") );
		return FALSE;
	}
	
	return TRUE;
}

bool CDialog::RegisterActiveX()
{
	bool	result = FALSE;
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, m_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_DLL);

	HMODULE hLibrary = ::LoadLibrary(szPath);
	if (NULL != hLibrary)
	{
		DllRegisterServer func;
		func = (DllRegisterServer)::GetProcAddress(hLibrary, TEXT("DllRegisterServer"));
		if(func != NULL)
		{
			result = SUCCEEDED( func() );
		}
		FreeLibrary( hLibrary );
	}

	return result;
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
