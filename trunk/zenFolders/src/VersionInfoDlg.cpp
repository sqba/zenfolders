// VersionInfoDlg.cpp: implementation of the CVersionInfoDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "VersionInfoDlg.h"
#include "resource.h"
#include "util\settings.h"


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


extern HINSTANCE		g_hInst;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVersionInfoDlg::CVersionInfoDlg(CShellFolder *pParent)
{
	m_hDlg = NULL;
	m_pParent = pParent;
}

CVersionInfoDlg::~CVersionInfoDlg()
{
	if(m_hDlg)
		::DestroyWindow(m_hDlg);
}



BOOL CALLBACK CVersionInfoDlg::DlgProc(HWND hDlg,
									   UINT message,
									   WPARAM wParam,
									   LPARAM lParam)
{
	CVersionInfoDlg *pDlg = NULL;

	switch (message)
	{
	case WM_INITDIALOG :
		pDlg = (CVersionInfoDlg*)lParam;
		pDlg->DisplayVersion(hDlg);
		pDlg->DisplayPath(hDlg);
		break;

	case WM_COMMAND :
		switch (LOWORD (wParam))
		{
		case IDOK:
		case IDCANCEL:
			::EndDialog(hDlg, 0) ;
		}
		break;

	case WM_CLOSE :
		::EndDialog(hDlg, 0) ;
	}

	return FALSE;
}

void CVersionInfoDlg::Show()
{
	m_hDlg = ::CreateDialogParam(
		g_hInst,
		MAKEINTRESOURCE(IDD_ZENFOLDERS_INFO), 
		NULL,
		DlgProc,
		(LPARAM)this);

	::SetWindowLong(m_hDlg, DWL_USER, (LONG)this);

	::ShowWindow(m_hDlg, SW_SHOW) ;

}

void CVersionInfoDlg::DisplayPath(HWND hDlg)
{
	TCHAR szMessage[256] = {0};
	TCHAR szPath[MAX_PATH] = {0};
	CSettings::GetXmlFilePath(szPath, ARRAYSIZE(szPath));
	//*strrchr(szPath, '\\') = 0;
	::SetDlgItemText(hDlg, IDC_PATH, szPath);
}

void CVersionInfoDlg::DisplayVersion(HWND hDlg)
{
	TCHAR szFullPath[MAX_PATH] = {0};
	::GetModuleFileName(g_hInst, szFullPath, MAX_PATH);

	DWORD dwVerInfoSize;
	DWORD dwVerHnd=0;
	dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if(dwVerInfoSize)
	{
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)::GlobalLock(hMem);
		::GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		UINT uVersionLen = 0;
		LPSTR lpVersion = NULL;
		TCHAR szGetName[256];
//		lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\");	 
//		lstrcat(szGetName, TEXT("ProductVersion"));
		lstrcpy(szGetName, "\\StringFileInfo\\040904b0\\ProductVersion");	 
		BOOL bRetCode = ::VerQueryValue(
			(LPVOID)lpstrVffInfo,
			(LPSTR)szGetName,
			(LPVOID*)&lpVersion,
			(UINT *)&uVersionLen);

		if(bRetCode)
			::SetDlgItemText(hDlg, IDC_VERSION, lpVersion);

		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
}
