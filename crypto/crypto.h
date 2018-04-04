
#ifndef __CRYPTO_H__
#define __CRYPTO_H__
#include <stdbool.h>
#define BUG1(a,b) { perror("a"); }
#define BUG(a) { perror("a"); }

#define PREAMBLE_SIZE 8			// 4 byte signature + 4 byte subfile type
#define KEY_BYTES 32			// 32 byts = 256 bits of key
#define PREAMBLE 0x01020304		// first 4 bytes of the file
#define FILEKEY_FORMAT 1		// next 4 bytes determine version/format
#define MASTERKEY_FORMAT 2		// format with master key
#define MASTER_KEY_OFFSET (PREAMBLE_SIZE+AES_MAX_IV_SIZE)
#define UNUSED(expr) (void)(expr)

bool EncryptAndWriteFile(HWND hwnd, HANDLE hFile, BYTE *data, DWORD size, DWORD *written);
bool ReadAndDecryptFile(HWND hwnd, HANDLE hFile, DWORD size, void** result, DWORD *resultlen);
bool GetFileKey(HWND hwnd);
void ResetEncryption();
#endif
