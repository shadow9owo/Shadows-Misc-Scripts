#pragma once
#include <Windows.h>
#include "crypto.h"

BOOL EC_DeriveBase64Key(
	CHAR*   outStr,
	SIZE_T  outStrSize,
	BYTE*	key,
	SIZE_T  keyLen,
	BYTE*   iv,
	SIZE_T  ivLen
);

BOOL EC_UnpackBase64Key(
	LPCSTR  encKey,
	BYTE* keyOut,
	SIZE_T* keyLenOut,
	BYTE* ivOut,
	SIZE_T* ivLenOut
);