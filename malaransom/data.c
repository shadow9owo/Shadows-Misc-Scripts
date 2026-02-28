#include "data.h"

HINSTANCE instance;
HWND mainwin;
HBITMAP bgbitmap = NULL;
DWORD edival = 0;
HANDLE prochandle;
LPVOID edi;
BOOL hooked = FALSE;
char input[256];
char score[256];

HWND zumahandle;
HWND zumaoverlayhandle;

int zumawidth,zumaheight;
HINSTANCE zumaoverlayinstance;