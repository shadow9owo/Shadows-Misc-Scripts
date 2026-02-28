#pragma once
#include <windows.h>

// NT def
#define STATUS_SUCCESS   0x00000000

// AES def 
#define AES_KEY_SIZE     32
#define AES_BLOCK_SIZE   16

// Crypto def
#define CRYPTO_CHUNK_BUFFER_SIZE 0x10000

typedef enum CryptoError {
	CT_OK,
	CT_INVALID_PARAM,
	CT_ALG_PROV_OPEN_FAIL,
	CT_ALG_KEY_OBJ_SIZE_FETCH_FAIL,
	CT_KEY_OBJ_ALLOC_FAIL,
	CT_KEY_GEN_FAIL,
	CT_FILE_OPEN_FAIL,
	CT_MEMORY_ALLOC_FAIL,
	CT_IN_FILE_SIZE_FETCH_FAIL,
	CT_CRYPT_FAIL
} CryptoError;

BOOL CT_GenerateKeyIVPair(
	BYTE*		 keyBuf,
	SIZE_T		 keySize,
	BYTE*		 ivBuf,
	SIZE_T		 ivSize
);

CryptoError CT_EncryptFile(
	LPCWSTR		 inFile,
	LPCWSTR		 outFile,
	BYTE*		 keyBuf,
	SIZE_T		 keySize,
	BYTE*		 ivBuf,
	SIZE_T		 ivSize
);

CryptoError CT_DecryptFile(
	LPCWSTR      inFile,
	LPCWSTR      outFile,
	BYTE*		 keyBuf,
	SIZE_T       keySize,
	BYTE*		 ivBuf,
	SIZE_T       ivSize
);