// Dialog.cpp: implementation of the CDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "dialog.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogProc2(HWND hwnd,
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

CDialog::CDialog(HINSTANCE hInstance, int nCmdShow)
{
	m_hwnd = ::CreateDialog(
		hInstance, 
		MAKEINTRESOURCE(IDD_MAIN), 
		0, 
		DialogProc2);
	
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
		/*if(g_bUninstall)
			Uninstall();
		else
			Install();*/
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

BOOL CDialog::OnClose()
{
	::DestroyWindow(m_hwnd);
	return TRUE;
}

void CDialog::SetMessage(LPCTSTR pszMessage)
{
	::SetDlgItemText(m_hwnd, IDC_MESSAGE, pszMessage);
}

void CDialog::HideOkButton()
{
	HWND hwndOK = ::GetDlgItem(m_hwnd, IDOK);
	::ShowWindow(hwndOK, SW_HIDE) ;
}

void CDialog::SetCancelButtonText(LPCTSTR pszCaption)
{
	::SetDlgItemText(m_hwnd, IDCANCEL, pszCaption);
}
