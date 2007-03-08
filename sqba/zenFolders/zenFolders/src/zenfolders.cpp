
#include <olectl.h>
#include <stdio.h>
#include <tchar.h>
#include <crtdbg.h>

#include "zenfolders.h"

#include "util/settings.h"
#include "util/shellnse.h"
#include "util/utilities.h"
#include "util/registry.h"

#include "google/googleds.h"

#include "viewlist.h"
#include "icons.h"
#include "clsfact.h"
#include "guid.h"
#include "cfgxml.h"
#include "pidlmgr.h"
#include "dlglist.h"


HINSTANCE	g_hInst			= NULL;
UINT		g_DllRefCount	= 0;
CIcons		*g_pIcons		= NULL;
CViewList	*g_pViewList	= NULL;
CConfigXML	*g_pConfigXML	= NULL;
LPPIDLMGR	g_pPidlMgr		= NULL;
CDialogList	*g_pDialogList	= NULL;

UINT		g_references[ENUM_CLASSES_LENGTH] = {0};

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		TESTHR( CoInitialize(NULL) ); 
		g_hInst = hInstance;
		CSettings::GetGlobalSettings();
		g_pConfigXML = new CConfigXML();
		g_pIcons = new CIcons(hInstance);
		g_pViewList = new CViewList();
		g_pPidlMgr = new CPidlManager();
		g_pDialogList = new CDialogList();
		break;

	case DLL_PROCESS_DETACH:
		_RPTF0(_CRT_WARN, "DLL_PROCESS_DETACH\n");
		CSettings::SaveGlobalSettings();
		if(g_pIcons)
			delete g_pIcons;
		if(g_pViewList)
			delete g_pViewList;
		if(g_pConfigXML)
		{
			try{
				delete g_pConfigXML;
			}catch(...){
				_RPTF0(_CRT_WARN, "Error deleting configxml\n");
			}
		}
		if(g_pPidlMgr)
			delete g_pPidlMgr;
		if(g_pDialogList)
			delete g_pDialogList;
		CoUninitialize();
		//_RPTF0(_CRT_WARN, "Unloaded\n");
		break;
	}

	return TRUE;
}

LPCTSTR GetClassName(int i)
{
	switch(i)
	{
	case CDROPSOURCE:
		return "CDropSource";
	case CSHELLFOLDER:
		return "CShellFolder";
	case CSHELLVIEW:
		return "CShellView";
	case CENUMFORMATETC:
		return "CEnumFormatEtc";
	case CCLASSFACTORY:
		return "CClassFactory";
	case CCONTEXTMENU:
		return "CContextMenu";
	case CDATAOBJECT:
		return "CDataObject";
	case CDROPTARGET:
		return "CDropTarget";
	case CENUMIDLIST:
		return "CEnumIDList";
	case CEXTRACTICON:
		return "CExtractIcon";
	case CQUERYINFO:
		return "CQueryInfo";
	case CPIDLMANAGER:
		return "CPidlManager";
	}
	return NULL;
}

STDAPI DllCanUnloadNow(void)
{
	if(g_DllRefCount > 0)
	{
		_RPTF1(_CRT_WARN, "RefCount=%d\n", g_DllRefCount);
		for(int i=0; i<ENUM_CLASSES_LENGTH; i++)
		{
			if(g_references[i] > 0)
			{
				LPCTSTR lpszClass = GetClassName(i);
				_RPTF2(_CRT_WARN, "%s has %d references\n", lpszClass, g_references[i]);
			}
		}
		return S_FALSE;
	}
	return S_OK;
	//return ((g_DllRefCount > 0) ? S_FALSE : S_OK);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;
	
	if(IsEqualCLSID(rclsid, CLSID_ShellFolderProperties))
	{
		_RPTF0(_CRT_WARN, "CLSID_ShellFolderProperties\n");
		return CLASS_E_CLASSNOTAVAILABLE;
	}
	
	if(!IsEqualCLSID(rclsid, CLSID_ShellFolderNameSpace))
		return CLASS_E_CLASSNOTAVAILABLE;
	
	CClassFactory *pClassFactory = new CClassFactory();
	if(NULL == pClassFactory)
		return E_OUTOFMEMORY;
	
	HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);
	
	pClassFactory->Release();
	
	return hResult;
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr;

	/*if( !CSettings::SaveCurrentPath() )
	{
		_RPTF0(_CRT_WARN, "SaveCurrentPath failed\n");
		return E_FAIL;
	}*/

	if( !CGoogleDS::RegisterPlugin() )
	{
		_RPTF0(_CRT_WARN, "RegisterPlugin failed\n");
		//return E_FAIL;
	}

	hr = CShellNSE::RegisterServer(g_hInst, CLSID_ShellFolderNameSpace);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "RegisterServer failed\n");
		return E_FAIL;
	}

	hr = CShellNSE::RegisterProperties(g_hInst, CLSID_ShellFolderNameSpace, CLSID_ShellFolderProperties);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "RegisterServer failed\n");
		return E_FAIL;
	}

	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr;

	hr = CShellNSE::UnregisterProperties(g_hInst, CLSID_ShellFolderProperties);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "UnregisterProperties failed\n");
		return FALSE;
	}

	hr = CShellNSE::UnregisterServer(g_hInst, CLSID_ShellFolderNameSpace);
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "UnregisterServer failed\n");
		return E_FAIL;
	}

	if( !CGoogleDS::UnregisterPlugin() )
	{
		_RPTF0(_CRT_WARN, "CGoogleDS::UnregisterPlugin() failed\n");
	}

	::CoFreeUnusedLibraries();

	return hr;
}

void gAddRef(int cls)
{
	if(ENUM_CLASSES_LENGTH <= cls)
	{
		_RPTF0(_CRT_ERROR, "gAddRef\n");
	}
	g_DllRefCount++;
	g_references[cls]++;
}

void gRelease(int cls)
{
	if(ENUM_CLASSES_LENGTH <= cls)
	{
		_RPTF0(_CRT_ERROR, "gRelease\n");
	}
	g_DllRefCount--;
	g_references[cls]--;
}
/*
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) void DllSetPath(LPCTSTR lpszPath, int len)
{
	TCHAR szPath[MAX_PATH] = {0};
	lstrcpy(szPath, lpszPath);
	LPSTR p = strrchr(szPath, '\\');
	*p = 0;
	strcat(szPath, "\\");
	strcat(szPath, ZENFOLDERS_XML);
	_RPTF1(_CRT_WARN, "szPath: \n", szPath);
	CRegistry::SaveString(
		MAIN_KEY_STRING,
		TEXT("Path"),
		szPath,
		len);
}
#ifdef __cplusplus
}
#endif
*/
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) bool DllIsRegistered()
{
	return (CShellNSE::IsRegistered(g_hInst, CLSID_ShellFolderProperties) ? TRUE : FALSE);
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) int DllDebugTest(void)
{
	_RPTF0(_CRT_WARN, "DllDebugTest\n");
	return S_FALSE;
}
#ifdef __cplusplus
}
#endif
