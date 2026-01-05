#include "crypto.h"
#include <bcrypt.h>
#include <winternl.h>

BOOL CT_GenerateKeyIVPair(
	BYTE*  keyBuf,
	SIZE_T keySize,
	BYTE*  ivBuf,
	SIZE_T ivSize
) {
	if (!keyBuf || keySize != AES_KEY_SIZE ||
		!ivBuf || ivSize != AES_BLOCK_SIZE
	) {
		return FALSE;
	}

	if (BCryptGenRandom(
		NULL, 
		keyBuf, 
		AES_KEY_SIZE, BCRYPT_USE_SYSTEM_PREFERRED_RNG
	) != STATUS_SUCCESS) {
		return FALSE;
	}

	if (BCryptGenRandom(
		NULL, 
		ivBuf, 
		AES_BLOCK_SIZE, BCRYPT_USE_SYSTEM_PREFERRED_RNG
	) != STATUS_SUCCESS) {
		return FALSE;
	}

	return TRUE;
}

CryptoError CT_EncryptFile(
	LPCWSTR      inFile,
	LPCWSTR      outFile,
	BYTE*		 keyBuf,
	SIZE_T       keySize,
	BYTE*		 ivBuf,
	SIZE_T       ivSize
) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	DWORD cbKeyObj = 0, cbData = 0, cbOut = 0;
	PBYTE keyObj = NULL;

	if (!inFile || !outFile ||
		!keyBuf || keySize != AES_KEY_SIZE ||
		!ivBuf || ivSize != AES_BLOCK_SIZE
		) {
		return CT_INVALID_PARAM;
	}

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0)) {
		return CT_ALG_PROV_OPEN_FAIL;
	}

	if (BCryptSetProperty(
		hAlg,
		BCRYPT_CHAINING_MODE,
		(PUCHAR)BCRYPT_CHAIN_MODE_CBC,
		sizeof(BCRYPT_CHAIN_MODE_CBC),
		0
	) != STATUS_SUCCESS) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_ALG_PROV_OPEN_FAIL;
	}

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PUCHAR)&cbKeyObj,
		sizeof(DWORD),
		&cbData,
		0
	) != STATUS_SUCCESS) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_ALG_KEY_OBJ_SIZE_FETCH_FAIL;
	}

	keyObj = malloc(cbKeyObj);
	if (!keyObj) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_KEY_OBJ_ALLOC_FAIL;
	}

	if (BCryptGenerateSymmetricKey(
		hAlg, &hKey,
		keyObj, cbKeyObj,
		(PUCHAR)keyBuf, (ULONG)keySize,
		0
	) != STATUS_SUCCESS) {
		free(keyObj);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_KEY_GEN_FAIL;
	}

	HANDLE hFileIn = CreateFileW(
		inFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	HANDLE hFileOut = CreateFileW(
		outFile,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFileIn == INVALID_HANDLE_VALUE || hFileOut == INVALID_HANDLE_VALUE) {
		if (hFileIn != INVALID_HANDLE_VALUE)  CloseHandle(hFileIn);
		if (hFileOut != INVALID_HANDLE_VALUE) CloseHandle(hFileOut);

		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		free(keyObj);

		return CT_FILE_OPEN_FAIL;
	}

	BYTE* inBuf = (BYTE*)malloc(CRYPTO_CHUNK_BUFFER_SIZE);
	BYTE* outBuf = (BYTE*)malloc(CRYPTO_CHUNK_BUFFER_SIZE + AES_BLOCK_SIZE);
	BYTE* iv = (BYTE*)malloc(ivSize);

	if (!inBuf || !outBuf || !iv) {
		if (inBuf)  free(inBuf);
		if (outBuf) free(outBuf);
		if (iv)     free(iv);

		CloseHandle(hFileIn);
		CloseHandle(hFileOut);
		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		free(keyObj);

		return CT_MEMORY_ALLOC_FAIL;
	}

	memcpy(iv, ivBuf, ivSize);

	DWORD bytesRead = 0;
	while (ReadFile(hFileIn, inBuf, CRYPTO_CHUNK_BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
		if (BCryptEncrypt(
			hKey,
			inBuf, bytesRead,
			NULL,
			iv, (ULONG)ivSize,
			outBuf, bytesRead + AES_BLOCK_SIZE,
			&cbOut,
			BCRYPT_BLOCK_PADDING
		) != STATUS_SUCCESS) {
			free(inBuf);
			free(outBuf);
			free(iv);

			CloseHandle(hFileIn);
			CloseHandle(hFileOut);
			BCryptDestroyKey(hKey);
			BCryptCloseAlgorithmProvider(hAlg, 0);
			free(keyObj);

			return CT_CRYPT_FAIL;
		}

		DWORD bytesWritten = 0;
		if (!WriteFile(hFileOut, outBuf, cbOut, &bytesWritten, NULL)) {
			free(inBuf);
			free(outBuf);
			free(iv);

			CloseHandle(hFileIn);
			CloseHandle(hFileOut);
			BCryptDestroyKey(hKey);
			BCryptCloseAlgorithmProvider(hAlg, 0);
			free(keyObj);

			return CT_CRYPT_FAIL;
		}
	}

	free(inBuf);
	free(outBuf);
	free(iv);

	CloseHandle(hFileIn);
	CloseHandle(hFileOut);
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	free(keyObj);

	return CT_OK;
}

CryptoError CT_DecryptFile(
	LPCWSTR      inFile,
	LPCWSTR      outFile,
	BYTE*		 keyBuf,
	SIZE_T       keySize,
	BYTE*		 ivBuf,
	SIZE_T       ivSize
) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	DWORD cbKeyObj = 0, cbData = 0, cbOut = 0;
	PBYTE keyObj = NULL;

	if (!inFile || !outFile ||
		!keyBuf || keySize != AES_KEY_SIZE ||
		!ivBuf || ivSize != AES_BLOCK_SIZE
		) {
		return CT_INVALID_PARAM;
	}

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0)) {
		return CT_ALG_PROV_OPEN_FAIL;
	}

	if (BCryptSetProperty(
		hAlg,
		BCRYPT_CHAINING_MODE,
		(PUCHAR)BCRYPT_CHAIN_MODE_CBC,
		sizeof(BCRYPT_CHAIN_MODE_CBC),
		0
	) != STATUS_SUCCESS) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_ALG_PROV_OPEN_FAIL;
	}

	if (BCryptGetProperty(
		hAlg,
		BCRYPT_OBJECT_LENGTH,
		(PUCHAR)&cbKeyObj,
		sizeof(DWORD),
		&cbData,
		0
	) != STATUS_SUCCESS) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_ALG_KEY_OBJ_SIZE_FETCH_FAIL;
	}

	keyObj = malloc(cbKeyObj);
	if (!keyObj) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_KEY_OBJ_ALLOC_FAIL;
	}

	if (BCryptGenerateSymmetricKey(
		hAlg, &hKey,
		keyObj, cbKeyObj,
		(PUCHAR)keyBuf, (ULONG)keySize,
		0
	) != STATUS_SUCCESS) {
		free(keyObj);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return CT_KEY_GEN_FAIL;
	}

	HANDLE hFileIn = CreateFileW(
		inFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	HANDLE hFileOut = CreateFileW(
		outFile,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFileIn == INVALID_HANDLE_VALUE || hFileOut == INVALID_HANDLE_VALUE) {
		if (hFileIn != INVALID_HANDLE_VALUE)  CloseHandle(hFileIn);
		if (hFileOut != INVALID_HANDLE_VALUE) CloseHandle(hFileOut);

		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		free(keyObj);

		return CT_FILE_OPEN_FAIL;
	}

	BYTE* inBuf = (BYTE*)malloc(CRYPTO_CHUNK_BUFFER_SIZE);
	BYTE* outBuf = (BYTE*)malloc(CRYPTO_CHUNK_BUFFER_SIZE);
	BYTE* iv = (BYTE*)malloc(ivSize);

	if (!inBuf || !outBuf || !iv) {
		if (inBuf)  free(inBuf);
		if (outBuf) free(outBuf);
		if (iv)     free(iv);

		CloseHandle(hFileIn);
		CloseHandle(hFileOut);
		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		free(keyObj);

		return CT_MEMORY_ALLOC_FAIL;
	}

	memcpy(iv, ivBuf, ivSize);

	DWORD bytesRead = 0;
	while (ReadFile(hFileIn, inBuf, CRYPTO_CHUNK_BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
		if (BCryptDecrypt(
			hKey,
			inBuf, bytesRead,
			NULL,
			iv, (ULONG)ivSize,
			outBuf, bytesRead,
			&cbOut,
			BCRYPT_BLOCK_PADDING
		) != STATUS_SUCCESS) {
			free(inBuf);
			free(outBuf);
			free(iv);

			CloseHandle(hFileIn);
			CloseHandle(hFileOut);
			BCryptDestroyKey(hKey);
			BCryptCloseAlgorithmProvider(hAlg, 0);
			free(keyObj);

			return CT_CRYPT_FAIL;
		}

		DWORD bytesWritten = 0;
		if (!WriteFile(hFileOut, outBuf, cbOut, &bytesWritten, NULL)) {
			free(inBuf);
			free(outBuf);
			free(iv);

			CloseHandle(hFileIn);
			CloseHandle(hFileOut);
			BCryptDestroyKey(hKey);
			BCryptCloseAlgorithmProvider(hAlg, 0);
			free(keyObj);

			return CT_CRYPT_FAIL;
		}
	}

	free(inBuf);
	free(outBuf);
	free(iv);

	CloseHandle(hFileIn);
	CloseHandle(hFileOut);
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	free(keyObj);

	return CT_OK;
}
