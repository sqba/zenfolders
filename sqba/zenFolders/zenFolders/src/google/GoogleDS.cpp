// GoogleDS.cpp: implementation of the CGoogleDS class.
//
//////////////////////////////////////////////////////////////////////

#include <crtdbg.h>

#include "GoogleDS.h"
#include "../util/settings.h"




// returned when a crawler component tries to register for an already handled extension
#define E_EXTENSION_REGISTERED           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0001)

// specifies that the component must register before performing any operations
#define E_COMPONENT_NOT_REGISTERED       MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0002)

// returned when a component tries to use an inexistent schema
#define E_NO_SUCH_SCHEMA                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0003)

// returned when a component tries to use an inexistent property
#define E_NO_SUCH_PROPERTY               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0004)

// specifies that the component has been disabled by the user
#define E_COMPONENT_DISABLED             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0005)

// specifies that the component is already registered
#define E_COMPONENT_ALREADY_REGISTERED   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0006)

// specifies that indexing is paused and any sent events will be ignored
#define S_INDEXING_PAUSED                MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x0007)

// specifies that the event was rejected because of data size limits
#define E_EVENT_TOO_LARGE                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0008)

// specifies that Google Desktop is not running
#define E_SERVICE_NOT_RUNNING            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0009)

// specifies that an event has invalid flags
#define E_INVALID_EVENT_FLAGS            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x000A)

// specifies that the component is prohibited by the user's group policy
#define E_COMPONENT_PROHIBITED           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x000B)

// specifies that the historical event sent could not be delivered immediately 
// and a delay is recommended before sending again
#define E_SEND_DELAYED                   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x000C)

// specifies that the provided property has been truncated due to size limits
#define S_PROPERTY_TRUNCATED             MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x000D)

// specifies that the provided property is too large and cannot be accepted
#define E_PROPERTY_TOO_LARGE             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x000E)

// returned when a property is not set
#define E_PROPERTY_NOT_SET               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x000F)

// returned when the Google Destop system is exiting.
#define E_SERVICE_IS_EXITING             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0010)

// returned when the application that made the call is exiting
#define E_APPLICATION_IS_EXITING         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0011)

// specifies that the event could not be delivered and a re-send is required
#define E_RETRY_SEND                     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0012)

// specifies that the event could not be delivered because required
// resources were not available and the send operation timed out
#define E_SEND_TIMEOUT                   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0013)

// specifies that the gadget registration was cancelled by the user
#define E_REGISTRATION_CANCELLED_BY_USER MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0014)

// specifies that the search has been locked by the user
#define E_SEARCH_LOCKED                  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0015)













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

	HRESULT hr;

START:
	if(lstrlen(pFolderData->szCategory) > 0)
	{
		_bstr_t category(pFolderData->szCategory);
		pResults = CGoogleDS::Query(query, category, ranking, maxResults, &hr);
	}
	else
	{
		//pResults = CGoogleDS::Query(query, NULL, ranking, maxResults);
		_bstr_t category(TEXT("file"));
		pResults = CGoogleDS::Query(query, category, ranking, maxResults, &hr);
	}

	if(NULL == pResults)
	{
		// Just try to register again
		if(hr == E_COMPONENT_NOT_REGISTERED)
		{
			UnregisterPlugin(); // Just in case somebody changed the cookie in the registry
			if(CGoogleDS::RegisterPlugin())
				goto START;
		}
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

		CSettings::SetGoogleCookie(g_lCookie);

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
		_RPTF0(_CRT_WARN, "Exception in RegisterPlugin()!\n");
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
											   int maxCount,
											   HRESULT *hr)
{
	IGoogleDesktopQueryAPIPtr spQuery;
	IGoogleDesktopQueryResultSet *pResults = NULL;

	*hr = spQuery.CreateInstance("GoogleDesktop.QueryAPI");
	if( FAILED(&hr) )
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

	*hr = spQuery->raw_Query(g_lCookie, bstrQuery, vtCategory, vtRanking, &pResults);

	if( FAILED(&hr) )
	{
		_RPTF0(_CRT_WARN, "Query failed!\n");
		return NULL;
	}

	if(NULL == pResults)
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
												 int maxCount,
												 HRESULT *hr)
{
	IGoogleDesktopQueryPtr ptrQuery;
	IGoogleDesktopQueryAPIPtr spQuery;
	IGoogleDesktopQueryResultSet *pResults;

	*hr = spQuery.CreateInstance("GoogleDesktop.QueryAPI");
	if( FAILED(&hr) )
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

	*hr = spQuery->raw_QueryEx(g_lCookie, bstrQuery, vtCategory, vtRanking, &ptrQuery);

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


const TCHAR* CGoogleDS::GDErrorToString(HRESULT hr)
{
	switch (hr) {
	case E_EXTENSION_REGISTERED:
		return TEXT("A component tried to register for an already handled extension");
		
	case E_COMPONENT_NOT_REGISTERED:
		return TEXT("The component must register before performing any operations");
		
	case E_NO_SUCH_SCHEMA:
		return TEXT("Schema name not found");
		
	case E_NO_SUCH_PROPERTY:
		return TEXT("Property name not found");
		
	case E_COMPONENT_DISABLED:
		return TEXT("The component has been disabled by the user");
		
	case E_COMPONENT_ALREADY_REGISTERED:
		return TEXT("The component is already registered");
		
	case S_INDEXING_PAUSED:
		return TEXT("Indexing is paused and any sent events will be ignored");
		
	case E_EVENT_TOO_LARGE:
		return TEXT("The event was rejected because of data size limits");
		
	case E_SERVICE_NOT_RUNNING:
		return TEXT("Google Desktop is not running");
		
	case E_INVALID_EVENT_FLAGS:
		return TEXT("An event has invalid flags");
		
	case E_COMPONENT_PROHIBITED:
		return TEXT("The component is prohibited by the user's group policy");
		
	case E_SEND_DELAYED:
		return TEXT("The historical event sent could not be delivered immediately. A delay is recommended before sending again");
		
	case S_PROPERTY_TRUNCATED:
		return TEXT("The provided property has been truncated due to size limits");
		
	case E_PROPERTY_TOO_LARGE:
		return TEXT("The provided property is too large and cannot be accepted");
		
	case E_PROPERTY_NOT_SET:
		return TEXT("Property has not been set");
		
	case E_SERVICE_IS_EXITING:
		return TEXT("Google Destop is exiting");
		
	case E_APPLICATION_IS_EXITING:
		return TEXT("The application that made the call is exiting");
		
	case E_RETRY_SEND:
		return TEXT("The event could not be delivered and a re-send is required");
	}
	
	return TEXT("<unknown error>");
}
