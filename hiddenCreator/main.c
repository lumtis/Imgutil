#include <Windows.h>
#include <wincrypt.h>
#include "stdio.h"

#define ENT_BEGIN "entity\n"
#define ENT_BEGIN_SZ 7
#define ENT_ENDLINE "\n"
#define ENT_ENDLINE_SZ 1
#define ENT_END "entityend\n"
#define ENT_END_SZ 10


// can be changed
#define BLOB "\x8\x2\0\0\x1h\0\0\x10\0\0\0š?\x18æú«Ø¹,š5P´†uÇ"
#define BLOB_SIZE 28

#define HIDDEN_MAX_SZ 20000000
#define FILE_MAX_NB 20

#define BLOB_BLOCK_SIZE 1120

LPCSTR name = "Container";


void dumpBuf(char* buf, ULONG bufLen, char* name)
{
	FILE* dump;

	dump = fopen(name, "wb");
	if(dump == NULL)
		printf("cannot create a dump file\n");
	else
	{
		fwrite(buf, sizeof(BYTE), bufLen, dump);
		fclose(dump);
	}
}



void quit(char* str)
{
	printf(str);
	printf("%x\n", GetLastError());
	system("pause");
	exit(0);
}


// Get file size
ULONG getFileLen(FILE* f)
{
	ULONG ret;

	if(f == NULL)
		return 0;

	fseek(f, 0L, SEEK_END);
	ret = ftell(f);

	fseek(f, 0L, SEEK_SET);

	return ret;
}


// Add module in the paypack
BOOL addFileToHidden(char* name, char* h, PULONG hCurrentLen)
{
	FILE* f;
	ULONG fLen;

	// Add signature first
	memcpy(h + (*hCurrentLen), ENT_BEGIN, ENT_BEGIN_SZ);
	(*hCurrentLen) += ENT_BEGIN_SZ;

	// Add name with null terminator
	memcpy(h + (*hCurrentLen), name, strlen(name));
	(*hCurrentLen) += strlen(name);
	memcpy(h + (*hCurrentLen), ENT_ENDLINE, ENT_ENDLINE_SZ);
	(*hCurrentLen) += ENT_ENDLINE_SZ;


	// Get the module
	f = fopen(name, "rb");
	if(f == NULL)
		quit("one or more file cannot be opened\n");

	// Get the size of the module
	fLen = getFileLen(f);
	if(fLen == 0)
		quit("error while reading length of the file\n");
	if((*hCurrentLen) + fLen >= HIDDEN_MAX_SZ)
		quit("size of the file too high\n");

	// Write content
	if(fread(h + (*hCurrentLen), sizeof(BYTE), fLen, f) != fLen)
		quit("one file has not been entirely written in paypack");
	(*hCurrentLen) += fLen;

	fclose(f);


	// Add terminator signature
	memcpy(h + (*hCurrentLen), ENT_END, ENT_END_SZ);
	(*hCurrentLen) += ENT_END_SZ;

	return TRUE;
}


// Get key for encryption
BOOL getKey(HCRYPTKEY* pKey)
{
	BYTE blob[BLOB_SIZE];
	HCRYPTPROV prov;

	// Get blob
	memcpy(blob, BLOB, BLOB_SIZE);

	// Get context
	if(CryptAcquireContext(&prov, name, NULL, PROV_RSA_FULL, NULL) == FALSE)
	{
		if(CryptAcquireContext(&prov, name, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET) == FALSE)
			quit("cannot acquire context\n");
	}


	// Key import
	if(CryptImportKey(prov, blob, BLOB_SIZE, NULL, NULL, pKey) == FALSE)
		quit("cannot import key\n");

	return TRUE;
}


// Buffer encryption
BOOL encryptBuf(HCRYPTKEY key, char* buf, PULONG bufLen)
{
	long dataToEncrypt = *bufLen;
	long blockToEncryptSize = BLOB_BLOCK_SIZE;
	char blockToEncrypt[BLOB_BLOCK_SIZE+1] = "";
	long i = 0;
	HCRYPTKEY dupKey;

	// Encrypt buffer block by block
	while(dataToEncrypt > BLOB_BLOCK_SIZE)
	{
		if(CryptDuplicateKey(key, NULL, NULL, &dupKey) == FALSE)
			return FALSE;

		memcpy(blockToEncrypt, buf+i, BLOB_BLOCK_SIZE);
		memcpy(blockToEncrypt+BLOB_BLOCK_SIZE, "\0", 1);

		if(CryptEncrypt(dupKey, NULL, TRUE, NULL, (BYTE*)(blockToEncrypt), (DWORD*)&blockToEncryptSize, blockToEncryptSize) == FALSE)
			return FALSE;
		if(blockToEncryptSize != BLOB_BLOCK_SIZE)
			return FALSE;

		memcpy(buf+i, blockToEncrypt, BLOB_BLOCK_SIZE);

		i += BLOB_BLOCK_SIZE;
		dataToEncrypt -= BLOB_BLOCK_SIZE;

		if(CryptDestroyKey(dupKey) == FALSE)
			return FALSE;
	}
	
	// End enryption
	memcpy(blockToEncrypt, buf+i, dataToEncrypt);
	memcpy(blockToEncrypt+dataToEncrypt, "\0", 1);

	if(CryptEncrypt(key, NULL, TRUE, NULL, (BYTE*)(blockToEncrypt), (DWORD*)&dataToEncrypt, blockToEncryptSize) == FALSE)
		return FALSE;

	memcpy(buf+i, blockToEncrypt, dataToEncrypt);

	CryptDestroyKey(key);
	
	return TRUE;
}



int main(int argc, char* argv[])
{
	int i = 0;
	HCRYPTKEY key;
	char* h;
	ULONG hLen = 0;
	FILE* image;
	ULONG imageSize;

	printf("program started\n");

	// Checking for at least one module
	if(argc < 3)
		quit("too few parameter\n");

	// Paypack memory allocation
	h = (char*)malloc(HIDDEN_MAX_SZ);

	// Walk each module
	for(i=2; i<argc; i++)
	{
		if(i > FILE_MAX_NB-3)
			quit("max file number has been reached\n");

		if(addFileToHidden(argv[i], h, &hLen) == FALSE)
			quit("one file cannot be added\n");
	}

	// dump file creation to check hidden
	dumpBuf(h, hLen, "hidden");

	getKey(&key);

	// Hidden encryption
	if(encryptBuf(key, h, &hLen) == FALSE)
		quit("error while encrypting the paypack\n");
	
	// dump file of the encrypted hidden
	dumpBuf(h, hLen, "hidden_crypt");

	// Open image file and add the encrypted hidden at the end
	image = fopen(argv[1], "ab");
	if(image == NULL)
		quit("cannot read the image file\n");
	imageSize = getFileLen(image);
	if(imageSize == 0)
		quit("the image file is empty\n");

	if(fwrite(h, sizeof(BYTE), hLen, image) == 0)
		quit("failed to write the hidden into the image\n");

	printf("the image now contains the hidden");

	fclose(image);
	free(h);
	return 1;
}