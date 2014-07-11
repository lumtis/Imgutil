#include <Windows.h>
#include <wincrypt.h>

// Can be changed
// It must be the same as hiddenCreator
#define BLOB "\x8\x2\0\0\x1h\0\0\x10\0\0\0š?\x18æú«Ø¹,š5P´†uÇ"
#define BLOB_SIZE 28
#define BLOB_BLOCK_SIZE 1120

BOOL getKey(HCRYPTKEY* pKey);
BOOL decryptBuf(HCRYPTKEY key, char* buf, long* bufLen);