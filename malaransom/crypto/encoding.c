#include <strsafe.h>
#include "encoding.h"
#include "../vendor/base64.h"

BOOL EC_DeriveBase64Key(
	CHAR*   outStr,
	SIZE_T  outStrSize,
	BYTE*   key,
	SIZE_T  keyLen,
	BYTE*   iv,
	SIZE_T  ivLen
) {
	if (!key || keyLen == 0 || !iv || ivLen == 0) {
		return FALSE;
	}

	SIZE_T totalLen = keyLen + ivLen;
	BYTE* combined = malloc(totalLen);
	if (!combined) {
		return FALSE;
	}

	memcpy(combined, key, keyLen);
	memcpy(combined + keyLen, iv, ivLen);

	char* base64Encoded = base64_encode(combined, totalLen);
	HRESULT hr = StringCchCopyA(outStr, outStrSize, base64Encoded);
	if (FAILED(hr)) {
		return FALSE;
	}

	free(base64Encoded);
	free(combined);

	return TRUE;
}

BOOL EC_UnpackBase64Key(
	LPCSTR  encKey,
	BYTE*   keyOut,
	SIZE_T* keyLenOut,
	BYTE*   ivOut,
	SIZE_T* ivLenOut
) {
	SIZE_T decodedLen = 0;
	BYTE* decoded = base64_decode(encKey, &decodedLen);
	if (!decoded) {
		return FALSE;
	}

	if (*keyLenOut + *ivLenOut != decodedLen) {
		free(decoded);
		return FALSE;
	}

	memcpy(keyOut, decoded, *keyLenOut);
	memcpy(ivOut, decoded + *keyLenOut, *ivLenOut);

	free(decoded);

	return TRUE;
}