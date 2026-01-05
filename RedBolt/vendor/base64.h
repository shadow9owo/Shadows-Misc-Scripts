/***********************************************************
* Base64 library                                           *
* @author Ahmed Elzoughby                                  *
* @date July 23, 2017                                      *
* Purpose: encode and decode base64 format                 *
***********************************************************/

// Dont ask me why the author called it base46 its base64
#ifndef BASE46_H
#define BASE46_H

#include <Windows.h>

char* base64_encode(BYTE* data, size_t len);
BYTE* base64_decode(LPCSTR encoded, SIZE_T* outLen);

#endif //BASE46_H
