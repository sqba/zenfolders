#include <windows.h>
#include <stdio.h>


int AddFile(LPCTSTR lpInstallerFileName, LPCTSTR lpResourceFileName, int resourceId)
{
	HANDLE	hFile;
	DWORD	dwFileSize, dwBytesRead;
	LPBYTE	lpBuffer;
	int		result = 0;
	
	hFile = CreateFile(
		lpResourceFileName,
		GENERIC_READ, 
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (INVALID_HANDLE_VALUE != hFile)
	{
		dwFileSize = GetFileSize(hFile, NULL);
		
		lpBuffer = new BYTE[dwFileSize];
		
		if (ReadFile(hFile, lpBuffer, dwFileSize, &dwBytesRead, NULL) != FALSE)
		{
			HANDLE hResource;
			
			hResource = BeginUpdateResource(lpInstallerFileName, FALSE);
			if (NULL != hResource)
			{
				if (UpdateResource(
					hResource, 
					RT_RCDATA, 
					MAKEINTRESOURCE(resourceId), 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
					(LPVOID) lpBuffer, 
					dwFileSize) != FALSE)
				{
					EndUpdateResource(hResource, FALSE);
					result = 1;
					printf("Successfully added file '%s' with resource id %d to '%s'\n", lpResourceFileName, resourceId, lpInstallerFileName);
				}
				else
					printf("UpdateResource failed\n");
			}
			else
				printf("BeginUpdateResource(%s) failed\n", lpInstallerFileName);
		}
		else
			printf("ReadFile(%s) failed\n", lpResourceFileName);
		
		delete [] lpBuffer;        
		
		CloseHandle(hFile);
	}
	else
		printf("Failed to open file %s\n", lpResourceFileName);

	return result;
}

int main(int argc, char* argv[])
{
	int result = 0;

	if(argc == 4)
		result = AddFile(argv[1], argv[2], atoi(argv[3]));

	return result;
}
