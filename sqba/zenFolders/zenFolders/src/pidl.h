#ifndef PIDL_H
#define PIDL_H

#include <shlobj.h>

#define MAX_QUERY	200
#define MAX_NAME	50

typedef enum tagPIDLTYPE
{
	PT_FOLDER	= 0x00000000,
	PT_FILE		= 0x00000001
} PIDLTYPE;

typedef struct tagFOLDERDATA
{
	TCHAR	szQuery[MAX_QUERY];
	TCHAR	szCategory[20];
	UINT	ranking;
	UINT	maxResults;
} FOLDERDATA, FAR *LPFOLDERDATA;

typedef struct tagFILEDATA
{
	TCHAR			szPath[MAX_PATH];
	LPITEMIDLIST	pidlFS;
} FILEDATA, FAR *LPFILEDATA;

typedef struct tagPIDLDATA
{
	PIDLTYPE	type;
	TCHAR		szName[MAX_NAME];
	union
	{
		FOLDERDATA	folderData;
		FILEDATA	fileData;
	};
} PIDLDATA, FAR *LPPIDLDATA;

#endif // PIDL_H
