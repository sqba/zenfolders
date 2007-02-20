// browser.cpp: implementation of the CWebBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <comdef.h>
#include <comutil.h>
//#include <exdisp.h>
//#include <Mshtml.h>
#include "browser.h"
//#include "util/string.h"


#define NOTIMPLEMENTED __asm{ int 3 }; return E_NOTIMPL


char CWebBrowser::szClassName[] = "IEPlaceholder";


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebBrowser::CWebBrowser(HINSTANCE hInstance, HWND hWnd): m_ObjRefCount(1)
{
	m_pWebObject	= NULL;
	m_pWebBrowser	= NULL;

	m_Site.host		= this;
	m_Frame.host	= this;

	m_hWndParent	= hWnd;

	OleInitialize(NULL);

	WNDCLASSEX wc = 
	{
		sizeof(wc),
			0, //style,
			WindowProc,
			0,0,
			hInstance,
			LoadIcon(NULL,IDI_APPLICATION),
			LoadCursor(NULL,IDC_ARROW),
			(HBRUSH)(COLOR_WINDOW+1),
			NULL,
			szClassName,
			LoadIcon(NULL,IDI_APPLICATION)
	};
	RegisterClassEx(&wc);

	DWORD dwStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD;// | WS_BORDER;
	m_hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE, //0,
		szClassName,
		NULL,
		dwStyle,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		m_hWndParent,
		NULL,
		hInstance,
		this);
}

CWebBrowser::~CWebBrowser()
{
	OleUninitialize();
}

HRESULT CWebBrowser::Navigate(LPCTSTR lpszURL,
							  DWORD dwFlags /* = 0 */,
							  LPCTSTR lpszTargetFrameName /* = NULL */,
							  LPCTSTR lpszHeaders /* = NULL */,
							  LPVOID lpvPostData /* = NULL */,
							  DWORD dwPostDataLen /* = 0 */)
{
	return m_pWebBrowser->Navigate2(
		&_variant_t(lpszURL),
		&_variant_t((long)(navNoReadFromCache | navNoHistory | navNoWriteToCache)), //&vtMissing , //&vtEmpty,//&vWorkaround,
		&vtMissing,
		&vtMissing,
		&vtMissing);
}

BOOL CWebBrowser::Move(int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint);

	RECT rect;
	GetClientRect(m_hWnd, &rect);

	HRESULT hr;

	if(m_pWebObject)
		hr = m_pWebObject->DoVerb(OLEIVERB_SHOW, NULL, &m_Site, -1, m_hWnd, &rect);

	if(m_pWebBrowser)
	{
		hr = m_pWebBrowser->put_Left(0);
		hr = m_pWebBrowser->put_Top(0);
		hr = m_pWebBrowser->put_Width(rect.right);
		hr = m_pWebBrowser->put_Height(rect.bottom);
	}

	if(m_pOIPObject)
	{
		HWND hWnd;
		hr = m_pOIPObject->GetWindow(&hWnd);
		::MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

		/*SCROLLINFO si;
		si.fMask = SIF_RANGE;
		si.cbSize = sizeof(SCROLLINFO);
		si.nMin = si.nMax = 0;
		::SetScrollInfo(hWnd, SB_CTL, &si, TRUE);*/
	}

	return TRUE;
}






HWND CWebBrowser::operator = (CWebBrowser* rhs)
{
	return m_hWnd;
}

void CWebBrowser::CreateEmbeddedWebControl(void)
{
	HRESULT hr;

	::OleCreate(
		CLSID_WebBrowser,
		IID_IOleObject,
		OLERENDER_DRAW,
		0,
		&m_Site,
		&m_Storage,
		(void**)&m_pWebObject);

	m_pWebObject->SetHostNames(L"Web Host", L"Web View");

	// I have no idea why this is necessary.
	// remark it out and everything works perfectly.
	OleSetContainedObject(m_pWebObject, TRUE);

	RECT rect;
	GetClientRect(m_hWnd, &rect);

	hr = m_pWebObject->DoVerb(OLEIVERB_SHOW, NULL, &m_Site, -1, m_hWnd, &rect);

	hr = m_pWebObject->QueryInterface(IID_IWebBrowser2, (void**)&m_pWebBrowser);

	hr = m_pWebObject->QueryInterface(IID_IOleInPlaceObject, (void**)&m_pOIPObject);

	VARIANT vURL;
	vURL.vt = VT_BSTR;
	vURL.bstrVal = SysAllocString(L"about:blank");
	VARIANT ve1, ve2, ve3, ve4;
	ve1.vt = VT_EMPTY;
	ve2.vt = VT_EMPTY;
	ve3.vt = VT_EMPTY;
	ve4.vt = VT_EMPTY;

	m_pWebBrowser->put_Left(0);
	m_pWebBrowser->put_Top(0);
	m_pWebBrowser->put_Width(rect.right);
	m_pWebBrowser->put_Height(rect.bottom);

	m_pWebBrowser->put_Visible(VARIANT_TRUE);

	hr = m_pWebBrowser->Navigate2(&vURL, &ve1, &ve2, &ve3, &ve4);

//	m_pWebBrowser->put_Visible(VARIANT_TRUE);
//	m_pWebBrowser->put_Resizable(VARIANT_FALSE);

//	IHTMLwindow2 *p;
//	hr = m_pWebObject->QueryInterface(IID_IHTMLwindow2, (void**)&p);

/*
	IDispatch *pDisp;
	hr = m_pWebBrowser->get_Document(&pDisp);
	IHTMLDocument2 *pDoc;
	hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
	IHTMLElement *pBody;
	pDoc->get_body(&pBody);
	pBody->setAttribute(_bstr_t(L"scroll"), VARIANT_FALSE, 0);
*/

	VariantClear(&vURL);

	m_pWebBrowser->Release();
}

void CWebBrowser::UnCreateEmbeddedWebControl(void)
{
	if(m_pWebObject)
	{
		m_pWebObject->Close(OLECLOSE_NOSAVE);
		m_pWebObject->Release();
	}
}








ULONG CWebBrowser::AddRef()
{
	return m_ObjRefCount++;
}

ULONG CWebBrowser::Release()
{
	if(--m_ObjRefCount)
		return m_ObjRefCount;
	delete this;
	return 0;
}









BOOL CWebBrowser::HandleMessage(UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT* r)
{
	if(uMsg == WM_DESTROY)
	{
		UnCreateEmbeddedWebControl();
		PostQuitMessage(0);
		return TRUE;
	}
	else if(uMsg == WM_CREATE)
	{
		CreateEmbeddedWebControl();
		return TRUE;
	}
	
	return FALSE;
}

LRESULT CALLBACK CWebBrowser::WindowProc(HWND m_hWnd,
										 UINT uMsg,
										 WPARAM wParam,
										 LPARAM lParam)
{
	CWebBrowser* _this;
	if(uMsg == WM_CREATE && (_this = (CWebBrowser*)(LPCREATESTRUCT(lParam))->lpCreateParams))
	{
		SetWindowLong(m_hWnd,GWL_USERDATA,(long)_this);
		_this->m_hWnd = m_hWnd;
		_this->AddRef();
	}
	else
		_this = (CWebBrowser*)GetWindowLong(m_hWnd,GWL_USERDATA);
	
	LRESULT result = 0;
	BOOL fDoDef = !(_this && _this->HandleMessage(uMsg,wParam,lParam,&result));
	
	if(uMsg == WM_DESTROY)
	{
		SetWindowLong(m_hWnd,GWL_USERDATA,(long)NULL);
		_this->Release();
	}
	
	return fDoDef?DefWindowProc(m_hWnd,uMsg,wParam,lParam):result;
}





















///////////////////////////////////////////////////////////////////////////////


// IUnknown 

STDMETHODIMP CNullStorage::QueryInterface(REFIID riid,void ** ppvObject)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP_(ULONG) CNullStorage::AddRef(void)
{
	return 1;
}

STDMETHODIMP_(ULONG) CNullStorage::Release(void)
{
	return 1;
}


// IStorage
STDMETHODIMP CNullStorage::CreateStream(const WCHAR * pwcsName,
										DWORD grfMode,
										DWORD reserved1,
										DWORD reserved2,
										IStream ** ppstm)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::OpenStream(const WCHAR * pwcsName,
									  void * reserved1,
									  DWORD grfMode,
									  DWORD reserved2,
									  IStream ** ppstm)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::CreateStorage(const WCHAR * pwcsName,
										 DWORD grfMode,
										 DWORD reserved1,
										 DWORD reserved2,
										 IStorage ** ppstg)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::OpenStorage(const WCHAR * pwcsName,
									   IStorage * pstgPriority,
									   DWORD grfMode,
									   SNB snbExclude,
									   DWORD reserved,
									   IStorage ** ppstg)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::CopyTo(DWORD ciidExclude,
								  IID const * rgiidExclude,
								  SNB snbExclude,
								  IStorage * pstgDest)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::MoveElementTo(const OLECHAR * pwcsName,
										 IStorage * pstgDest,
										 const OLECHAR* pwcsNewName,
										 DWORD grfFlags)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::Commit(DWORD grfCommitFlags)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::Revert(void)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::EnumElements(DWORD reserved1,
										void * reserved2,
										DWORD reserved3,
										IEnumSTATSTG ** ppenum)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::DestroyElement(const OLECHAR * pwcsName)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::RenameElement(const WCHAR * pwcsOldName,
										 const WCHAR * pwcsNewName)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::SetElementTimes(const WCHAR * pwcsName,
										   FILETIME const * pctime,
										   FILETIME const * patime,
										   FILETIME const * pmtime)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::SetClass(REFCLSID clsid)
{
	return S_OK;
}

STDMETHODIMP CNullStorage::SetStateBits(DWORD grfStateBits,DWORD grfMask)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CNullStorage::Stat(STATSTG * pstatstg,DWORD grfStatFlag)
{
	NOTIMPLEMENTED;
}




STDMETHODIMP CMySite::QueryInterface(REFIID riid,void ** ppvObject)
{
	if(riid == IID_IUnknown || riid == IID_IOleClientSite)
		*ppvObject = (IOleClientSite*)this;
	else if(riid == IID_IOleInPlaceSite) // || riid == IID_IOleInPlaceSiteEx || riid == IID_IOleInPlaceSiteWindowless)
		*ppvObject = (IOleInPlaceSite*)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	
	return S_OK;
}

STDMETHODIMP_(ULONG) CMySite::AddRef(void)
{
	return 1;
}

STDMETHODIMP_(ULONG) CMySite::Release(void)
{
	return 1;
}


// IOleClientSite

STDMETHODIMP CMySite::SaveObject()
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::GetMoniker(DWORD dwAssign,
								 DWORD dwWhichMoniker,
								 IMoniker ** ppmk)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::GetContainer(LPOLECONTAINER FAR* ppContainer)
{
	// We are a simple object and don't support a container.
	*ppContainer = NULL;

	return E_NOINTERFACE;
}

STDMETHODIMP CMySite::ShowObject()
{
//	NOTIMPLEMENTED;
	// huh?
	return NOERROR;
}

STDMETHODIMP CMySite::OnShowWindow(BOOL fShow)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::RequestNewObjectLayout()
{
	NOTIMPLEMENTED;
}

// IOleWindow

STDMETHODIMP CMySite::GetWindow(HWND FAR* lphwnd)
{
	*lphwnd = host->m_hWnd;

	return S_OK;
}

STDMETHODIMP CMySite::ContextSensitiveHelp(BOOL fEnterMode)
{
	NOTIMPLEMENTED;
}

// IOleInPlaceSite


STDMETHODIMP CMySite::CanInPlaceActivate()
{
	// Yes we can
	return S_OK;
}

STDMETHODIMP CMySite::OnInPlaceActivate()
{
	// Why disagree.
	return S_OK;
}

STDMETHODIMP CMySite::OnUIActivate()
{
	return S_OK;
}

STDMETHODIMP CMySite::GetWindowContext(LPOLEINPLACEFRAME FAR* ppFrame,
									   LPOLEINPLACEUIWINDOW FAR* ppDoc,
									   LPRECT prcPosRect,
									   LPRECT prcClipRect,
									   LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	*ppFrame = &host->m_Frame;
	*ppDoc = NULL;
	GetClientRect(host->m_hWnd,prcPosRect);
	GetClientRect(host->m_hWnd,prcClipRect);

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = host->m_hWnd;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

STDMETHODIMP CMySite::Scroll(SIZE scrollExtent)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::OnUIDeactivate(BOOL fUndoable)
{
	return S_OK;
}

STDMETHODIMP CMySite::OnInPlaceDeactivate()
{
	return S_OK;
}

STDMETHODIMP CMySite::DiscardUndoState()
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::DeactivateAndUndo()
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMySite::OnPosRectChange(LPCRECT lprcPosRect)
{
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CMyFrame
//


// IUnknown
STDMETHODIMP CMyFrame::QueryInterface(REFIID riid,void ** ppvObject)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP_(ULONG) CMyFrame::AddRef(void)
{
	return 1;
}

STDMETHODIMP_(ULONG) CMyFrame::Release(void)
{
	return 1;
}

// IOleWindow
STDMETHODIMP CMyFrame::GetWindow(HWND FAR* lphwnd)
{
	*lphwnd = this->host->m_hWnd;
	return S_OK;
//  NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
  NOTIMPLEMENTED;
}

// IOleInPlaceUIWindow
STDMETHODIMP CMyFrame::GetBorder(LPRECT lprectBorder)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::SetActiveObject(IOleInPlaceActiveObject *pActiveObject,
									   LPCOLESTR pszObjName)
{
	return S_OK;
}

// IOleInPlaceFrame
STDMETHODIMP CMyFrame::InsertMenus(HMENU hmenuShared,
								   LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::SetMenu(HMENU hmenuShared,
							   HOLEMENU holemenu,
							   HWND hwndActiveObject)
{
	return S_OK;
}

STDMETHODIMP CMyFrame::RemoveMenus(HMENU hmenuShared)
{
	NOTIMPLEMENTED;
}

STDMETHODIMP CMyFrame::SetStatusText(LPCOLESTR pszStatusText)
{
	return S_OK;
}

STDMETHODIMP CMyFrame::EnableModeless(BOOL fEnable)
{
	return S_OK;
}

STDMETHODIMP CMyFrame::TranslateAccelerator(LPMSG lpmsg,WORD wID)
{
	NOTIMPLEMENTED;
}
