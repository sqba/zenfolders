#ifndef CFGXML_H
#define CFGXML_H

#include <windows.h>
#include "pidlmgr.h"

#import <msxml3.dll> //no_namespace
//using namespace MSXML2;

//typedef MSXML2::IXMLDOMNodePtr		DomNodePtr;
//typedef MSXML2::IXMLDOMElementPtr	DomElemPtr;

class CConfigXML  
{
public:
	CConfigXML();
	virtual ~CConfigXML();

	bool Save();
	bool SaveFolder(LPCITEMIDLIST, LPCITEMIDLIST);
	bool SaveFolder(LPCITEMIDLIST);
	bool DeleteFolder(LPCITEMIDLIST);
	bool MoveFolder(LPCITEMIDLIST, LPCITEMIDLIST);
	bool HasSubFolders(LPCITEMIDLIST);

	MSXML2::IXMLDOMNodePtr CreateFolder(LPCTSTR, MSXML2::IXMLDOMNodePtr);
	MSXML2::IXMLDOMNodePtr GetSubfolder(MSXML2::IXMLDOMNodePtr, LPCTSTR);
	MSXML2::IXMLDOMNodePtr GetNode(LPCITEMIDLIST);

	MSXML2::IXMLDOMNodePtr CreateFolderLink(LPCTSTR, MSXML2::IXMLDOMNodePtr);
	MSXML2::IXMLDOMNodePtr CreateFileLink(LPCTSTR, MSXML2::IXMLDOMNodePtr);

	static bool GetFolderInfo(LPPIDLDATA, MSXML2::IXMLDOMNodePtr);
	static bool SetFolderInfo(MSXML2::IXMLDOMNodePtr, LPPIDLDATA);
	static bool GetFolderLinkInfo(LPPIDLDATA, MSXML2::IXMLDOMNodePtr);

	static bool SetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, LPCTSTR, USHORT);
	static bool SetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, UINT);
	static UINT GetNodeName(MSXML2::IXMLDOMNodePtr, LPTSTR, USHORT);
	static UINT GetNodeValue(MSXML2::IXMLDOMNodePtr, LPTSTR, USHORT);
	static UINT GetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, LPTSTR, USHORT);
	static UINT GetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, UINT);

	MSXML2::IXMLDOMNodeListPtr GetFolders(MSXML2::IXMLDOMNodePtr);
	MSXML2::IXMLDOMNodeListPtr GetFolderLinks(MSXML2::IXMLDOMNodePtr);
	MSXML2::IXMLDOMNodeListPtr GetFileLinks(MSXML2::IXMLDOMNodePtr);

private:
	bool Load();
	USHORT EscapeXML(LPTSTR, USHORT);
	USHORT GetNodeXPath(LPCITEMIDLIST, LPTSTR, USHORT);

private:
	MSXML2::IXMLDOMDocumentPtr	m_pXmlDoc;
	TCHAR m_szFullPath[MAX_PATH*2];
};

typedef CConfigXML FAR *LPCONFIGXML;

#endif   //CFGXML_H
