#ifndef PIDL_H
#define PIDL_H

#include <shlobj.h>

#define MAX_QUERY	200
#define MAX_NAME	50

typedef enum _PIDLTYPE
{
	PT_UNKNOWN		= 0x00000000,
	PT_FOLDER		= 0x00000001,
	PT_FILE			= 0x00000002,
	PT_FOLDER_LINK	= 0x00000003,
	PT_SUB_FOLDER	= 0x00000004
} PIDLTYPE;
/*
typedef enum _PIDLTYPE
{
	PT_UNKNOWN	= 0x0000,
	PT_FILE		= 0x0020,
	PT_FOLDER	= 0x0040,
	PT_SEARCH	= 0x0080,
	PT_FILESYS	= 0x0100
} PIDLTYPE;
Combinations:
	1. PT_FILE
	2. PT_FOLDER | PT_SEARCH
	3. PT_FOLDER | PT_FILESYS
*/
typedef struct _SEARCHDATA
{
	TCHAR	szQuery[MAX_QUERY];
	TCHAR	szCategory[20];
	UINT	ranking;
	UINT	maxResults;
	LONG	viewStyle;
} SEARCHDATA, FAR *LPSEARCHDATA;

typedef struct _FILEDATA
{
	TCHAR			szPath[MAX_PATH];
	LPITEMIDLIST	pidlFS;
} FILEDATA, FAR *LPFILEDATA;

typedef struct _PIDLDATA
{
	PIDLTYPE	type;
	TCHAR		szName[MAX_NAME];
	union
	{
		SEARCHDATA	searchData;
		FILEDATA	fileData;
	};
} PIDLDATA, FAR *LPPIDLDATA;

#endif // PIDL_H
