// FileViewer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FileViewer.h"
#include "FileMonitor.h"

FileMonitor g_FileMonitor;

#define MAX_LOADSTRING 100
#define IDC_MAIN_EDIT	101

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hMainWnd = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                MonitorFile(LPCWSTR filename);

std::wstring OpenFileDialog(HWND hwnd)
{
    OPENFILENAME ofn;       // common dialog box structure
    WCHAR szFile[MAX_PATH]; // buffer for file name
    std::wstring filename;

                            // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 

    if (GetOpenFileName(&ofn) == TRUE)
    {
        filename = ofn.lpstrFile;
    }


    return filename;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FILEVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FILEVIEWER));

    if (lpCmdLine[0] != 0)
    {
        MonitorFile(lpCmdLine);
    }

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILEVIEWER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FILEVIEWER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   hMainWnd = hWnd;
   RegisterHotKey(hMainWnd, IDM_EXIT, NULL, VK_ESCAPE);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_OPEN:
            {
                std::wstring sFile = OpenFileDialog(hWnd);
                if (!sFile.empty())
                {
                    MonitorFile(sFile.c_str());
                }
                break;
            }
            case IDM_ABOUT:
            {
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
                SetWindowText(hEdit, L"Dialog closed");
            }
            break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SIZE:
    {
        HWND hEdit;
        RECT rcClient;

        GetClientRect(hWnd, &rcClient);

        hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
        SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;
    //case WM_PAINT:
    //    {
    //        PAINTSTRUCT ps;
    //        HDC hdc = BeginPaint(hWnd, &ps);
    //        // TODO: Add any drawing code that uses hdc here...
    //        EndPaint(hWnd, &ps);
    //    }
    //    break;
    case WM_CREATE:
    {
        //HFONT hfDefault;
        HWND hEdit;

        //TODO Create own Edit control to work with big text sizes
        //Use ANSI control version
        hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 100, 100, hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
        if (hEdit == NULL)
            MessageBox(hWnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
        else
            Edit_SetReadOnly(hEdit, TRUE);

        //hfDefault = GetStockObject(DEFAULT_GUI_FONT);
        //SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
    }
    break;
    //case WM_CHAR:
    //{
    //    if (wParam == VK_ESCAPE)
    //        DestroyWindow(hWnd);
    //    else
    //        return DefWindowProc(hWnd, message, wParam, lParam);
    //}
    //break;
    case WM_HOTKEY:
        if (wParam == IDM_EXIT)
            DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

BOOL MonitorFile(LPCWSTR filename)
{
    BOOL bRet = TRUE;

    HWND hEdit = GetDlgItem(hMainWnd, IDC_MAIN_EDIT);
    _ASSERT(hEdit);

    if (g_FileMonitor.StartMonitor(filename, hEdit))
    {
        SetWindowText(hMainWnd, filename);
    }

    return bRet;
}



