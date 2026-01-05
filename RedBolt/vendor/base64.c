/***********************************************************
* Base64 library implementation                            *
* @author Ahmed Elzoughby                                  *
* @date July 23, 2017                                      *
***********************************************************/

#include "base64.h"
#include <stdlib.h>
#include <memory.h>

// Change: typo
char base64_map[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };


// Change: Made a binary safe version
char* base64_encode(BYTE* data, size_t len) {
    int i, j;
    char* encoded = malloc((len + 2) / 3 * 4 + 1);
    if (!encoded) return NULL;

    for (i = 0, j = 0; i < len;) {
        int octet_a = i < len ? data[i++] : 0;
        int octet_b = i < len ? data[i++] : 0;
        int octet_c = i < len ? data[i++] : 0;

        int triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded[j++] = base64_map[(triple >> 18) & 0x3F];
        encoded[j++] = base64_map[(triple >> 12) & 0x3F];
        encoded[j++] = (i - 2) <= len ? base64_map[(triple >> 6) & 0x3F] : '=';
        encoded[j++] = (i - 1) <= len ? base64_map[triple & 0x3F] : '=';
    }

    encoded[j] = '\0';
    return encoded;
}

int base64_char_to_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;  // padding '=' or invalid
}

// Change: Made a binary safe version
BYTE* base64_decode(LPCSTR encoded, SIZE_T* outLen) {
    size_t len = 0;
    int i = 0;
    size_t enc_len = strlen(encoded);

    BYTE* decoded = malloc(enc_len / 4 * 3);
    if (!decoded) return NULL;

    int buf[4];
    int buf_count = 0;
    int j = 0;

    for (i = 0; i < enc_len; i++) {
        if (encoded[i] == '=') buf[buf_count++] = 0;
        else {
            int val = base64_char_to_val(encoded[i]);
            if (val == -1) continue; // skip invalid chars
            buf[buf_count++] = val;
        }

        if (buf_count == 4) {
            decoded[j++] = (buf[0] << 2) | (buf[1] >> 4);
            if (encoded[i - 1] != '=') decoded[j++] = (buf[1] << 4) | (buf[2] >> 2);
            if (encoded[i] != '=') decoded[j++] = (buf[2] << 6) | buf[3];
            buf_count = 0;
        }
    }

    *outLen = j;
    return decoded;
}
