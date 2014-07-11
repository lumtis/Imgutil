#include "crypt.h"

LPCSTR name = "Container";


BOOL getKey(HCRYPTKEY* pKey)
{
	BYTE blob[BLOB_SIZE];
	HCRYPTPROV prov;

	// Predefined blob
	memcpy(blob, BLOB, BLOB_SIZE);

	if(CryptAcquireContextA(&prov, name, NULL, PROV_RSA_FULL, NULL) == FALSE)
	{
		if(CryptAcquireContextA(&prov, name, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET) == FALSE)
			return FALSE;
	}

	if(CryptImportKey(prov, blob, BLOB_SIZE, NULL, NULL, pKey) == FALSE)
		return FALSE;

	return TRUE;
}



// Buffer decryption
BOOL decryptBuf(HCRYPTKEY key, char* buf, long* bufLen)
{
	long dataToDecrypt = *bufLen;
	long blockToDecryptSize = BLOB_BLOCK_SIZE;
	char blockToDecrypt[BLOB_BLOCK_SIZE+1] = "";
	long i = 0;
	HCRYPTKEY dupKey;
	
	// Decrypt buffer block by block
	while(dataToDecrypt > BLOB_BLOCK_SIZE)
	{
		if(CryptDuplicateKey(key, NULL, NULL, &dupKey) == FALSE)
			return FALSE;

		memcpy(blockToDecrypt, buf+i, BLOB_BLOCK_SIZE);
		memcpy(blockToDecrypt+BLOB_BLOCK_SIZE, "\0", 1);

		if(CryptDecrypt(dupKey, NULL, TRUE, NULL, (BYTE*)(blockToDecrypt), (DWORD*)&blockToDecryptSize) == FALSE)
			return FALSE;
		if(blockToDecryptSize != BLOB_BLOCK_SIZE)
			return FALSE;

		memcpy(buf+i, blockToDecrypt, BLOB_BLOCK_SIZE);

		i += BLOB_BLOCK_SIZE;
		dataToDecrypt -= BLOB_BLOCK_SIZE;

		if(CryptDestroyKey(dupKey) == FALSE)
			return FALSE;
	}
	
	// End decryption
	memcpy(blockToDecrypt, buf+i, dataToDecrypt);
	memcpy(blockToDecrypt+dataToDecrypt, "\0", 1);

	if(CryptDecrypt(key, NULL, TRUE, NULL, (BYTE*)(blockToDecrypt), (DWORD*)&dataToDecrypt) == FALSE)
		return FALSE;

	memcpy(buf+i, blockToDecrypt, dataToDecrypt);

	CryptDestroyKey(key);
	
	return TRUE;
}