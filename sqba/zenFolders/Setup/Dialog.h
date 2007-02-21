// Dialog.h: interface for the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_)
#define AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

class CDialog  
{
public:
	CDialog(HINSTANCE, int, bool);
	virtual ~CDialog();

	BOOL OnInit(HWND);
	BOOL OnClose();
	BOOL OnCommand(int, int);

	void CleanUp();

private:
	BOOL Uninstall();
	BOOL Install();
	BOOL CreateUninstall(BOOL);
	bool RegisterActiveX(LPCTSTR);
	bool UnRegisterActiveX(LPCTSTR);
	bool ExtractResourceToFile(int, LPCTSTR, bool);
	void SetMessage(LPCTSTR);
	void Finish();
	bool DirectoryExists(LPCTSTR);
	bool GetProgramFilesPath(LPTSTR);

public:
	HWND m_hwnd;
	bool m_bCleanup;

private:
	HINSTANCE	m_hInstance;
	bool		m_bUnInstall;
	TCHAR		m_szDestinationPath[MAX_PATH];
	TCHAR		m_szModulePath[MAX_PATH];
	TCHAR		m_szTempPath[MAX_PATH];
};

#endif // !defined(AFX_DIALOG_H__564EC7EA_AAAF_486F_9B1B_6FAF1642CE15__INCLUDED_)
