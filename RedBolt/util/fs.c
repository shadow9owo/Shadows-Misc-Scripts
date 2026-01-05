#include "fs.h"
#include <Shlwapi.h>
#include <strsafe.h>

BOOL FS_GetKnownFolderPath(
    PWSTR* path,
    REFKNOWNFOLDERID fid
) {
    if (!path) {
        return FALSE;
    }

    HRESULT res = SHGetKnownFolderPath(
        fid,
        0,
        NULL,
        path
    );

    return res == S_OK;
}

BOOL FS_GetCurrentUserPath(PWSTR* path) {
    return FS_GetKnownFolderPath(path, &FOLDERID_Profile);
}

BOOL FS_QueryFilesWithExt(
    LPCWSTR   dir,
    LPCWSTR   exts,
    BOOL      skipProtected,
    LPWSTR**  outFiles,
    SIZE_T*   outFileCount
) {
    if (!dir || !exts || !outFiles || !outFileCount) {
        return FALSE;
    }

    SIZE_T dirLen = lstrlenW(dir);
    BOOL needSlash = (dirLen > 0 && dir[dirLen - 1] != L'\\' && dir[dirLen - 1] != L'/');

    SIZE_T searchLen = dirLen + (needSlash ? 2 : 1);
    LPWSTR searchPath = (LPWSTR)malloc((searchLen + 1) * sizeof(WCHAR));
    if (!searchPath) {
        return FALSE;
    }

    if (FAILED(StringCchCopyW(searchPath, searchLen + 1, dir))) { 
        free(searchPath); 
        return FALSE; 
    }

    if (needSlash && FAILED(StringCchCatW(searchPath, searchLen + 1, L"\\"))) { 
        free(searchPath);
        return FALSE; 
    }
    
    if (FAILED(StringCchCatW(searchPath, searchLen + 1, L"*"))) { 
        free(searchPath); 
        return FALSE; 
    }

    WIN32_FIND_DATAW file;
    HANDLE hFind = FindFirstFileW(searchPath, &file);
    free(searchPath);
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
        LPCWSTR name = file.cFileName;
        if (lstrcmpW(name, L".") == 0 || lstrcmpW(name, L"..") == 0) continue;

        SIZE_T fullLen = dirLen + (needSlash ? 1 : 0) + lstrlenW(name);
        LPWSTR fullPath = (LPWSTR)malloc((fullLen + 1) * sizeof(WCHAR));
        if (!fullPath) continue;

        if (FAILED(StringCchCopyW(fullPath, fullLen + 1, dir)) ||
            (needSlash && FAILED(StringCchCatW(fullPath, fullLen + 1, L"\\"))) ||
            FAILED(StringCchCatW(fullPath, fullLen + 1, name))) {
            free(fullPath);
            continue;
        }

        if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            FS_QueryFilesWithExt(fullPath, exts, skipProtected, outFiles, outFileCount);
            free(fullPath);
            continue;
        }

        if (skipProtected && (file.dwFileAttributes &
            (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN))) {
            free(fullPath);
            continue;
        }

        LPCWSTR dot = StrRChrW(name, NULL, L'.');
        if (!dot) { free(fullPath); continue; }

        // Check multiple extensions
        BOOL matched = FALSE;
        LPCWSTR extStart = exts;
        while (extStart && *extStart) {
            LPCWSTR sep = StrChrW(extStart, L';');
            SIZE_T extLen = sep ? (size_t)(sep - extStart) : lstrlenW(extStart);
            if (CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, 
                dot, -1, extStart, (int)extLen) == CSTR_EQUAL) {
                matched = TRUE;
                break;
            }
            extStart = sep ? sep + 1 : NULL;
        }

        if (matched) {
            LPWSTR* newList = realloc(*outFiles, (*outFileCount + 1) * sizeof(LPWSTR));
            if (newList) {
                *outFiles = newList;
                (*outFiles)[*outFileCount] = fullPath;
                (*outFileCount)++;
            } else {
                free(fullPath);
            }
        } else {
            free(fullPath);
        }

    } while (FindNextFileW(hFind, &file));

    FindClose(hFind);
    return TRUE;
}
