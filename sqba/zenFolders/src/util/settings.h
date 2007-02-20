#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>
#include <shlobj.h>

#define MAIN_KEY_STRING		(TEXT("Software\\zenFolders"))
#define VALUE_STRING		(TEXT("Display Settings"))
#define INITIAL_COLUMN_SIZE	100
#define DEFAULT_MAX_RESULTS	100

#define ZENFOLDERS_XML		(TEXT("zenFolders.xml"))
#define SHELLEX_NAME		"zenFolders"
#define ROOT_LOCATION		"Desktop"
//#define ROOT_LOCATION		"MyComputer"

#define CONSOLE_PLUGIN_GUID		L"{79B7408F-601A-4214-B9FD-F1A05F0B1859}"
#define CONSOLE_TITLE			L"zenFolders"
#define CONSOLE_DESCRIPTION		L"Your files. When and where you need them."

#define DOWNLOAD_PAGE		"http://zenFolders.googlepages.com/install"

extern int  g_nColumn1;
extern int  g_nColumn2;
extern BOOL g_bViewKeys;
extern int  g_lCookie;

class CSettings  
{
public:
	static BOOL SaveCurrentPath();
	static BOOL GetXmlFilePath(TCHAR*, DWORD);
	static BOOL SaveGlobalSettings(void);
	static BOOL GetGlobalSettings(void);
	static UINT GetRootName(LPCITEMIDLIST, LPTSTR, UINT);
};

#endif // SETTINGS_H
