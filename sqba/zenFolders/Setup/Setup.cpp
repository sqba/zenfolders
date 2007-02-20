// Setup.cpp : Defines the entry point for the application.
//

//#include <Regstr.h>
//#include <shlobj.h>
#include "stdafx.h"
#include "resource.h"


#define RESOURCE_ID_DLL		104
#define FILENAME_DLL		"zenFolders.dll"
#define RESOURCE_ID_XML		105
#define FILENAME_XML		"zenFolders.xml"
#define FOLDER_NAME			TEXT("zenFolders")
#define FILENAME_EXE		"setup.exe"


typedef HRESULT  (__stdcall *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);
#define CSIDL_PROGRAM_FILES 38


typedef HRESULT (*DllRegisterServer)(void);
typedef HRESULT (*DllUnregisterServer)(void);


HINSTANCE	g_hInstance;
HWND		g_hDialog;
TCHAR		g_szDestinationPath[MAX_PATH] = {0};
//TCHAR		g_szLastMessage[100] = {0};


bool GetProgramFilesPath(LPTSTR pszPath)
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

bool ExtractResourceToFile(int resourceId, LPCTSTR lpResourceFileName)
{
	HMODULE	hLibrary;
	HRSRC	hResource;
	HGLOBAL	hResourceLoaded;
	LPBYTE	lpBuffer;
	bool	result = false;
	TCHAR	szMessage[MAX_PATH] = {0};
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, g_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, lpResourceFileName);

	wsprintf(szMessage, "Extracting %s: ", szPath);
	::SetDlgItemText(g_hDialog, IDC_MESSAGE, szMessage);
	
	hLibrary = g_hInstance;
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
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				
				if (INVALID_HANDLE_VALUE != hFile)
				{
					::WriteFile(hFile, lpBuffer, dwFileSize, &dwBytesWritten, NULL);
					
					::CloseHandle(hFile);

					result = true;

					lstrcat(szMessage, TEXT("OK"));
				}
				else
					lstrcat(szMessage, TEXT("Failed to extract resource"));
			}
			else
				lstrcat(szMessage, TEXT("Failed to lock resource"));
		}  
		else
			lstrcat(szMessage, TEXT("Failed to load resource"));
	}
	else
		lstrcat(szMessage, TEXT("Resource not found"));

	::SetDlgItemText(g_hDialog, IDC_MESSAGE, szMessage);

	return result;
}



BOOL CreateUninstal()
{
	HKEY  hKey;
	LONG  lResult;
	DWORD dwDisp;

//REGSTR_PATH_UNINSTALL

	TCHAR szSrcPath[MAX_PATH] = {0};
	DWORD dwRet = ::GetCurrentDirectory(MAX_PATH, szSrcPath);
	strcat(szSrcPath, "\\");
	strcat(szSrcPath, FILENAME_EXE);

	TCHAR szDstPath[MAX_PATH] = {0};
	lstrcpy(szDstPath, g_szDestinationPath);
	strcat(szDstPath, "\\");
	strcat(szDstPath, FILENAME_EXE);

	if( !::CopyFile(szSrcPath, szDstPath, FALSE) )
	{
		::SetDlgItemText(g_hDialog, IDC_MESSAGE, TEXT("Failed to copy setup."));
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
		::SetDlgItemText(g_hDialog, IDC_MESSAGE, TEXT("RegCreateKeyEx failed"));
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
		::SetDlgItemText(g_hDialog, IDC_MESSAGE, TEXT("RegSetValueEx failed"));
		return FALSE;
	}
	
	return TRUE;
}

bool RegisterActiveX()
{
	bool	result = FALSE;
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, g_szDestinationPath);
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

void Install()
{
	TCHAR	szPath[MAX_PATH] = {0};

	lstrcpy(szPath, g_szDestinationPath);
	BOOL b = ::CreateDirectory(szPath, NULL);

	if( ExtractResourceToFile(RESOURCE_ID_DLL, FILENAME_DLL) )
	{
		ExtractResourceToFile(RESOURCE_ID_XML, FILENAME_XML);

		if( RegisterActiveX() )
		{
			if( CreateUninstal() )
				::SetDlgItemText(g_hDialog, IDC_MESSAGE, TEXT("zenFolders succesfully installed.\n"));

			HWND hwndOK = ::GetDlgItem(g_hDialog, IDOK);
			::ShowWindow(hwndOK, SW_HIDE) ;
			::SetDlgItemText(g_hDialog, IDCANCEL, TEXT("Close"));
		}
	}
}

void Uninstall()
{
//	::MessageBox(0, TEXT("zenFolders successfully uninstalled.\nThank you for using zenFolders.\nBye!"), "zenFolders", MB_ICONINFORMATION | MB_OK);

	TCHAR	szPath[MAX_PATH] = {0};

	BOOL b;

	lstrcpy(szPath, g_szDestinationPath);
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
				b = ::DeleteFile(szPath);

				memset(szPath, 0, sizeof(szPath));
				lstrcpy(szPath, g_szDestinationPath);
				lstrcat(szPath, "\\");
				lstrcat(szPath, FILENAME_EXE);
				b = ::DeleteFile(szPath);

				//b = ::RemoveDirectory(g_szDestinationPath);
			}
		}
		else
			FreeLibrary( hLibrary );
	}

	::SetDlgItemText(g_hDialog, IDC_MESSAGE, TEXT("zenFolders successfully uninstalled.\nThank you for using zenFolders.\nBye!"));

	HWND hwndOK = GetDlgItem(g_hDialog, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;
	::SetDlgItemText(g_hDialog, IDCANCEL, TEXT("Close"));

	/*memset(szPath, 0, sizeof(szPath));
	lstrcpy(szPath, g_szDestinationPath);
	lstrcat(szPath, "\\");
	lstrcat(szPath, FILENAME_XML);
	::DeleteFile(szPath);*/


	//BOOL b = ::DeleteFile();
	//b = RemoveDirectory();
}

BOOL OnCommand(int id, int code)
{
	switch(id)
	{
	case IDOK:
		Install();
		return TRUE;
		break;
	case IDCANCEL:
		//::EndDialog(g_hDialog, IDCANCEL);
		//::DestroyWindow(g_hDialog);
		::PostQuitMessage(1);
		break;
	}
	return FALSE;
}

BOOL CALLBACK DialogProc(HWND hwnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		return OnCommand(LOWORD (wParam), HIWORD (wParam));
	case WM_DESTROY:
		::PostQuitMessage(0);
		return TRUE;
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		return TRUE;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	GetProgramFilesPath(g_szDestinationPath);
	lstrcat(g_szDestinationPath, "\\");
	lstrcat(g_szDestinationPath, FOLDER_NAME);

	g_hInstance = hInstance;
	
	g_hDialog = ::CreateDialog(
		hInstance, 
		MAKEINTRESOURCE(IDD_MAIN), 
		0, 
		DialogProc);
	
	if (!g_hDialog)
	{
		char buf[100];
		wsprintf(buf, "Error x%x", GetLastError ());
		::MessageBox(0, buf, "CreateDialog", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	
	::ShowWindow(g_hDialog, nCmdShow) ;

	if(0 == lstrcmpi(lpCmdLine, TEXT("uninstall")))
	{
		Uninstall();
	}
	else
	{
		TCHAR szMessage[MAX_PATH] = {0};
		wsprintf(szMessage, TEXT("zenFolders will be installed in '%s'"), g_szDestinationPath);
		::SetDlgItemText(g_hDialog, IDC_MESSAGE, szMessage);
	}
	
	MSG  msg;
	int status;
	while ((status = GetMessage (& msg, 0, 0, 0)) != 0)
	{
		if (status == -1)
			return -1;
		if (!::IsDialogMessage (g_hDialog, & msg))
		{
			::TranslateMessage ( & msg );
			::DispatchMessage ( & msg );
		}
	}

	return msg.wParam;
	return 0;
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