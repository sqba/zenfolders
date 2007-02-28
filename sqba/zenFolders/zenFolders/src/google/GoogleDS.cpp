// GoogleDS.cpp: implementation of the CGoogleDS class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>

#include "GoogleDS.h"
#include "../util/registry.h"
#include "../util/settings.h"

int  g_lCookie;

IGoogleDesktopQueryResultSet *CGoogleDS::Query(LPPIDLDATA pData,
											   BOOL bHasSubFolders)
{
	IGoogleDesktopQueryResultSet *pResults = NULL;
	LPFOLDERDATA pFolderData = &pData->folderData;
	UINT ranking	= pFolderData->ranking;
	UINT maxResults	= pFolderData->maxResults;
	_bstr_t query(pFolderData->szQuery);

	if(0 == lstrlen(pFolderData->szQuery))
	{
		if(!bHasSubFolders)
			query = pData->szName;
		else
			return NULL;
	}

	if(lstrlen(pFolderData->szCategory) > 0)
	{
		_bstr_t category(pFolderData->szCategory);
		pResults = CGoogleDS::Query(query, category, ranking, maxResults);
	}
	else
	{
		//pResults = CGoogleDS::Query(query, NULL, ranking, maxResults);
		_bstr_t category(TEXT("file"));
		pResults = CGoogleDS::Query(query, category, ranking, maxResults);
	}

	if(NULL == pResults)
	{
		// Just try to register again
		if(	!CGoogleDS::IsInstalled() )
			CGoogleDS::RegisterPlugin();
	}

	return pResults;
}

HRESULT CGoogleDS::AddPairToSafeArray(int index,
									  const OLECHAR *val,
									  SAFEARRAY FAR* psa)
{
	long ix[1];
	ix[0] = index;
	_variant_t var(::SysAllocString(val));
	HRESULT hr = ::SafeArrayPutElement(psa, ix, &var);
	return hr;
}

BOOL CGoogleDS::RegisterPlugin()
{
//	_RPTF0(_CRT_WARN, "RegisterPlugin\n");
	try
	{
		HRESULT hr;

		// Instantiate the Google Desktop registrar component by creating
		// an ActiveX registration object.
		IGoogleDesktopRegistrarPtr spRegistrar;
		hr = spRegistrar.CreateInstance("GoogleDesktop.Registrar");
		if( FAILED(hr) )
		{
			_RPTF0(_CRT_WARN, "CreateInstance('GoogleDesktop.Registrar') failed!\n");
			return FALSE;
		}

		_bstr_t guid( CONSOLE_PLUGIN_GUID );

		// Component description is 6 strings
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = 6;
		
		// Component description is 6 strings
		SAFEARRAY *psa = ::SafeArrayCreate(VT_VARIANT, 1, rgsabound);
		AddPairToSafeArray(0, L"Title", psa);
		AddPairToSafeArray(1, CONSOLE_TITLE, psa);
		AddPairToSafeArray(2, L"Description", psa);
		AddPairToSafeArray(3, CONSOLE_DESCRIPTION, psa);
		AddPairToSafeArray(4, L"Icon", psa);
		AddPairToSafeArray(5, L",1", psa);

		// Wrap description array in variant
		VARIANT description;
		VariantInit(&description);
		description.vt= VT_ARRAY | VT_VARIANT;
		description.parray = psa;
		
		// and register
		hr = spRegistrar->StartComponentRegistration(guid, description);
		if (FAILED(hr))
		{
			_RPTF0(_CRT_WARN, "StartComponentRegistration failed!\n");
			return FALSE;
		}

		// Ask for the specific registration object that gives us Query API access.
		_bstr_t progid("GoogleDesktop.QueryRegistration");
		IGoogleDesktopRegisterQueryPluginPtr spRegistration;
		hr = spRegistrar->raw_GetRegistrationInterface(progid,
			reinterpret_cast<IUnknown**>(&spRegistration));
		if (FAILED(hr))
		{
			_RPTF0(_CRT_WARN, "raw_GetRegistrationInterface failed!\n");
			return FALSE;
		}

		// Now ask that registration object to give us a cookie representing
		// our Query API access.  Note the read_only flag argument here;
		// if false, then we're asking for read-write access instead of
		// just read-only access.
		g_lCookie = spRegistration->RegisterPlugin(guid, TRUE);

		CRegistry::SaveIntGlobal(MAIN_KEY_STRING, COOKIE_STRING, g_lCookie);

		_RPTF1(_CRT_WARN, "RegisterPlugin returned %d", g_lCookie);

		hr = spRegistrar->FinishComponentRegistration();
		if (FAILED(hr))
		{
			_RPTF0(_CRT_WARN, "FinishComponentRegistration failed!\n");
			return FALSE;
		}

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

BOOL CGoogleDS::UnregisterPlugin()
{
	try
	{
		HRESULT hr;

		// Instantiate the Google Desktop registrar component by creating
		// an ActiveX registration object.
		IGoogleDesktopRegistrarPtr spRegistrar;
		hr = spRegistrar.CreateInstance("GoogleDesktop.Registrar");
		if( FAILED(hr) )
		{
			_RPTF0(_CRT_WARN, "CreateInstance('GoogleDesktop.Registrar') failed!\n");
			return FALSE;
		}

		_bstr_t guid( CONSOLE_PLUGIN_GUID );
		hr = spRegistrar->UnregisterComponent(guid);
		if (FAILED(hr))
		{
			_RPTF0(_CRT_WARN, "UnregisterComponent failed!\n");
			return FALSE;
		}

		g_lCookie = 0;

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

IGoogleDesktopQueryResultSet *CGoogleDS::Query(const OLECHAR *query,
											   const OLECHAR *category,
											   int ranking,
											   int maxCount)
{
	HRESULT hr;
	IGoogleDesktopQueryAPIPtr spQuery;
	IGoogleDesktopQueryResultSet *pResults = NULL;

	hr = spQuery.CreateInstance("GoogleDesktop.QueryAPI");
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "Failed to create GoogleDesktop.QueryAPI!\n");
		return NULL;
	}

	_bstr_t bstrQuery(::SysAllocString(query));
	_variant_t vtCategory = vtMissing;
	_variant_t vtRanking((BYTE)ranking);

	if(NULL != category)
	{
		_variant_t vtTemp(::SysAllocString(category));
		vtCategory = vtTemp;
	}

	hr = spQuery->raw_Query(g_lCookie, bstrQuery, vtCategory, vtRanking, &pResults);

	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "Query failed!\n");
		return NULL;
	}

	while((GD_QRS_COMPLETE != pResults->Getreadystate()) &&
		(GD_QRS_FAILED == pResults->Getreadystate()))
	{
		if((maxCount > 0) && (pResults->Getavailable_count()>=maxCount))
			break;
	}

	return pResults;
}

IGoogleDesktopQueryResultSet *CGoogleDS::QueryEx(const OLECHAR *query,
												 const OLECHAR *category,
												 int ranking,
												 int maxCount)
{
	HRESULT hr;
	IGoogleDesktopQueryPtr ptrQuery;
	IGoogleDesktopQueryAPIPtr spQuery;
	IGoogleDesktopQueryResultSet *pResults;

	hr = spQuery.CreateInstance("GoogleDesktop.QueryAPI");
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "Failed to create GoogleDesktop.QueryAPI!\n");
		return NULL;
	}

	_bstr_t bstrQuery(::SysAllocString(query));
	_variant_t vtCategory = vtMissing;
	_variant_t vtRanking((BYTE)ranking);

	if(NULL != category)
	{
		_variant_t vtTemp(::SysAllocString(category));
		vtCategory = vtTemp;
	}

	hr = spQuery->raw_QueryEx(g_lCookie, bstrQuery, vtCategory, vtRanking, &ptrQuery);

//	_variant_t async(FALSE);
//	ptrQuery->SetOption("async", async);

	pResults = ptrQuery->Execute();

	while(GD_QRS_COMPLETE != pResults->Getreadystate())
	{
		if((maxCount > 0) && (pResults->Getavailable_count()>=maxCount))
			break;
	}

	return pResults;
}

BOOL CGoogleDS::IsInstalled()
{
	HRESULT hr;

	// Instantiate the Google Desktop registrar component by creating
    // an ActiveX registration object.
	IGoogleDesktopRegistrarPtr spRegistrar;
	hr = spRegistrar.CreateInstance("GoogleDesktop.Registrar");
	if( FAILED(hr) )
	{
		_RPTF0(_CRT_WARN, "CreateInstance('GoogleDesktop.Registrar') failed!\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CGoogleDS::IsRegistered()
{
	HRESULT hr;
	IGoogleDesktopQueryAPIPtr spQuery;

	hr = spQuery.CreateInstance("GoogleDesktop.QueryAPI");
	if( FAILED(hr) )
		return FALSE;

	return TRUE;
}
