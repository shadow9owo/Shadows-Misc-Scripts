#pragma once

#include <Windows.h>

extern HINSTANCE instance;
extern HWND mainwin;
extern HBITMAP bgbitmap;
extern DWORD edival;
extern HANDLE prochandle;
extern LPVOID edi;
extern BOOL hooked;
extern char input[256];
extern char score[256];

extern HWND zumahandle;
extern HWND zumaoverlayhandle;

extern int zumawidth,zumaheight; //i am not dealing with structs atm
extern HINSTANCE zumaoverlayinstance;