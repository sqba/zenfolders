#ifndef CFGXML_H
#define CFGXML_H

#include <windows.h>
#include "pidlmgr.h"

#import <msxml3.dll> //no_namespace
//using namespace MSXML2;

class CConfigXML  
{
public:
	CConfigXML();
	virtual ~CConfigXML();

	BOOL Save();
	BOOL SaveFolder(LPCITEMIDLIST, LPCITEMIDLIST);
	BOOL SaveFolder(LPCITEMIDLIST);
	BOOL DeleteFolder(LPCITEMIDLIST);
	BOOL MoveFolder(LPCITEMIDLIST, LPCITEMIDLIST);
	static BOOL GetFolderInfo(LPPIDLDATA, MSXML2::IXMLDOMNodePtr);
	static BOOL SetFolderInfo(MSXML2::IXMLDOMNodePtr, LPPIDLDATA);

	MSXML2::IXMLDOMNodePtr CreateFolder(LPCTSTR, MSXML2::IXMLDOMNodePtr);
	MSXML2::IXMLDOMNodePtr GetSubfolder(MSXML2::IXMLDOMNodePtr, LPCTSTR);
	MSXML2::IXMLDOMNodePtr GetRoot();
	MSXML2::IXMLDOMNodePtr GetNode(LPCITEMIDLIST);

private:
	BOOL Load();

	USHORT EscapeXML(LPTSTR, USHORT);

	USHORT GetNodeXPath(LPCITEMIDLIST, LPTSTR, USHORT);
//	void CreateAttribute(MSXML2::IXMLDOMNode*, LPCTSTR, LPCTSTR);
//	MSXML2::IXMLDOMNodePtr CreateChild(MSXML2::IXMLDOMNode*, LPCTSTR);

	static BOOL SetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, LPCTSTR, USHORT);
	static BOOL SetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, UINT);
	static UINT GetNodeName(MSXML2::IXMLDOMNodePtr, LPTSTR, USHORT);
	static UINT GetNodeValue(MSXML2::IXMLDOMNodePtr, LPTSTR, USHORT);
	static UINT GetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, LPTSTR, USHORT);
	static UINT GetNodeAttribute(MSXML2::IXMLDOMNodePtr, LPCTSTR, UINT);

private:
	MSXML2::IXMLDOMDocumentPtr	m_pXmlDoc;
	TCHAR m_szFullPath[MAX_PATH*2];
};

typedef CConfigXML FAR *LPCONFIGXML;

#endif   //CFGXML_H
