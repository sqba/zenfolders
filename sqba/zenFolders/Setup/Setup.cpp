// Setup.cpp : Defines the entry point for the application.
//

//#include <regstr.h>
//#include <shlobj.h>
#include <windows.h>
//#include <shellapi.h>
//#include <shlobj.h>
//#include <shlwapi.h>
#include <malloc.h>
#include "resource.h"
#include "dialog.h"


void CleanUp()
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
			strlen(modulename) * 2 + strlen(temppath) + 20) ;
		
		wsprintf(bat, templ, modulename, modulename, temppath) ;
		
		::WriteFile(hf, bat, strlen(bat), &len, NULL) ;
		::CloseHandle(hf) ;
		
		::ShellExecute(NULL, "open", temppath, NULL, NULL, SW_HIDE);
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	bool bUninstall = (0 == lstrcmpi(lpCmdLine, TEXT("uninstall")));

	CDialog *pDialog = new CDialog(hInstance, nCmdShow, bUninstall);

	MSG  msg;
	int status;
	while ((status = GetMessage (& msg, 0, 0, 0)) != 0)
	{
		if (status == -1)
		{
			if(bUninstall && pDialog->m_bCleanup)
			{
				CleanUp();
				// ubiti explorer, mamojebac zna da lokuje dll
			}
			delete pDialog;
			return -1;
		}
		if (!::IsDialogMessage (pDialog->m_hwnd, & msg))
		{
			::TranslateMessage ( & msg );
			::DispatchMessage ( & msg );
		}
	}

	if(bUninstall && pDialog->m_bCleanup)
	{
		CleanUp();
		// ubiti explorer, mamojebac zna da lokuje dll
	}

	delete pDialog;

	return msg.wParam;
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




