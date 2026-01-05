#pragma once
#include <ShlObj_core.h>

BOOL FS_GetKnownFolderPath(
    PWSTR* path,
    REFKNOWNFOLDERID fid
);

BOOL FS_GetCurrentUserPath(PWSTR* path);

BOOL FS_QueryFilesWithExt(
    LPCWSTR   dir,
    LPCWSTR   exts,
    BOOL      skipProtected,
    LPWSTR**  outFiles,
    SIZE_T*   outFileCount
);
