/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2011, Tom Hunter
**
**  Name: window_win32.c
**
**  Description:
**      Simulate CDC 6612 or CC545 console display on MS Windows.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License version 3 as
**  published by the Free Software Foundation.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License version 3 for more details.
**
**  You should have received a copy of the GNU General Public License
**  version 3 along with this program in file "license-gpl-3.0.txt".
**  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "resource.h"
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize    5000
// useful for more stable screen shots
// #define ListSize            10000

#define TIMER_ID    1

/*
**  Text view: a read-only multi-line EDIT control that mirrors the
**  screens as plain text so that screen readers can read the console.
**  Small-font characters are 8 dots apart horizontally and lines are
**  10 dots apart vertically, giving a 64 x 52 character grid per screen.
*/
#define IDC_TEXTVIEW       1001
#define TextRows           52
#define TextCols           64
#define TextLineLen        (TextCols + 2)       /* including CR/LF */
#define TextBufSize        (TextRows * TextLineLen + 1)
#define TextLabelHeight    20

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct dispList
    {
    u16 xPos;                       /* horizontal position */
    u16 yPos;                       /* vertical position */
    u8  fontSize;                   /* size of font */
    u8  ch;                         /* character to be displayed */
    } DispList;

typedef enum displaymode
    {
    ModeLeft, ModeCenter, ModeRight
    } DisplayMode;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void windowThread(void);
ATOM windowRegisterClass(HINSTANCE hInstance);
static BOOL windowCreate(void);
static void windowClipboard(HWND hWnd);
static LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
void windowDisplay(HWND hWnd);
static void windowTextViewToggle(HWND hWnd);
static void windowTextViewLayout(HWND hWnd);
static void windowTextViewUpdate(void);
static void windowTextViewSet(int s, char *text, int lines);
static void windowTextViewMessage(char *msg);
static LRESULT CALLBACK windowTextViewProcedure(HWND, UINT, WPARAM, LPARAM);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8          currentFont;
static i16         currentX      = -1;
static i16         currentY      = -1;
static bool        displayActive = FALSE;
static DispList    display[ListSize];
static u32         listEnd;
static HANDLE      hThread;
static HWND        hWnd;
static HFONT       hSmallFont            = 0;
static HFONT       hMediumFont           = 0;
static HFONT       hLargeFont            = 0;
static HPEN        hPen                  = 0;
static HINSTANCE   hInstance             = 0;
static char        *lpClipToKeyboard     = NULL;
static char        *lpClipToKeyboardPtr  = NULL;
static u8          clipToKeyboardDelay   = 0;
static DisplayMode displayMode           = ModeCenter;
static bool        displayModeNeedsErase = FALSE;
static BOOL        shifted               = FALSE;
static HWND        hEdit[2]              = { NULL, NULL };
static HWND        hLabel[2]             = { NULL, NULL };
static HFONT       hEditFont             = 0;
static WNDPROC     editProc              = NULL;
static bool        textViewActive        = FALSE;
static char        textGrid[2][TextRows][TextCols];
static char        textBuf[TextBufSize];
static char        textPrev[2][TextBufSize];
static int         textPrevLines[2] = { -1, -1 };


/*--------------------------------------------------------------------------
**  Purpose:        Create WIN32 thread which will deal with all windows
**                  functions.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowInit(void)
    {
    DWORD dwThreadId;
    int   thPriority = 0;

    /*
    **  Create display list pool.
    */
    listEnd = 0;

    /*
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);

    /*
    **  Create windowing thread.
    */
    hThread = CreateThread(
        NULL,                                       // no security attribute
        0,                                          // default stack size
        (LPTHREAD_START_ROUTINE)windowThread,
        (LPVOID)NULL,                               // thread parameter
        0,                                          // not suspended
        &dwThreadId);                               // returns thread ID

    if (hThread == NULL)
        {
        MessageBox(NULL, "Operator Window Thread Creation Failed.", "dtCyber/window_win32", MB_OK + MB_ICONERROR);
        exit(1);
        }

    thPriority = GetThreadPriority(hThread);
    if (!SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL))
        {
        if (MessageBox(NULL, "Could Not Set Thread Priority. Continue?", "dtCyber/window_win32",
                       MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON1) == MB_DEFBUTTON2)
            {
            exit(1);
            }
        }

    displayActive = TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set font size.
**                  functions.
**
**  Parameters:     Name        Description.
**                  size        font size in points.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetFont(u8 font)
    {
    currentFont = font;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set X coordinate.
**
**  Parameters:     Name        Description.
**                  x           horizontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX(u16 x)
    {
    currentX = x;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set Y coordinate.
**
**  Parameters:     Name        Description.
**                  y           vertical coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY(u16 y)
    {
    currentY = 0777 - y;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Queue characters.
**
**  Parameters:     Name        Description.
**                  ch          character to be queued.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowQueue(u8 ch)
    {
    DispList *elem;

    if ((listEnd >= ListSize)
        || (currentX == -1)
        || (currentY == -1))
        {
        return;
        }

    if (ch != 0)
        {
        elem           = display + listEnd++;
        elem->ch       = ch;
        elem->fontSize = currentFont;
        elem->xPos     = currentX;
        elem->yPos     = currentY;
        }

    currentX += currentFont;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Terminate console window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowTerminate(void)
    {
    if (displayActive)
        {
        SendMessage(hWnd, WM_DESTROY, 0, 0);
        WaitForSingleObject(hThread, INFINITE);
        displayActive = FALSE;
        }
    }

/*
 **--------------------------------------------------------------------------
 **
 **  Private Functions
 **
 **--------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------
**  Purpose:        Windows thread.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowThread(void)
    {
    MSG msg;

    /*
    **  Register the window class.
    */
    windowRegisterClass(hInstance);

    /*
    **  Create the window.
    */
    if (!windowCreate())
        {
        MessageBox(NULL, "(window_win32) window creation failed", "Error", MB_OK);

        return;
        }

    /*
    **  Main message loop.
    */
    while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Register the window class.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
ATOM windowRegisterClass(HINSTANCE hInstance)
    {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style         = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
    wcex.lpfnWndProc   = (WNDPROC)windowProcedure;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_CONSOLE);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = (LPCSTR)IDC_CONSOLE;
    wcex.lpszClassName = "CONSOLE";
    wcex.hIconSm       = LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create the main window.
**
**  Parameters:     Name        Description.
**
**  Returns:        TRUE if successful, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static BOOL windowCreate(void)
    {
    char windowName[132];

    windowName[0] = '\0';
    strcat(windowName, displayName);
    strcat(windowName, " - " DtCyberVersion);
    strcat(windowName, " - " DtCyberBuildDate);

#if CcLargeWin32Screen == 1
    hWnd = CreateWindow(
        "CONSOLE",              // Registered class name
        windowName,             // window name
        WS_OVERLAPPEDWINDOW,    // window style
        CW_USEDEFAULT,          // horizontal position of window
        0,                      // vertical position of window
        widthPX,                // window width
        heightPX,               // window height
        NULL,                   // handle to parent or owner window
        NULL,                   // menu handle or child identifier
        0,                      // handle to application instance
        NULL);                  // window-creation data
#else
    hWnd = CreateWindow(
        "CONSOLE",                                                                    // Registered class name
        windowName,                                                                   // window name
        (WS_OVERLAPPEDWINDOW | WS_EX_COMPOSITED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN), // window style
        CW_USEDEFAULT,                                                                // horizontal position of window
        CW_USEDEFAULT,                                                                // vertical position of window
        widthPX,                                                                      // window width
        heightPX,                                                                     // window height
        NULL,                                                                         // handle to parent or owner window
        NULL,                                                                         // menu handle or child identifier
        0,                                                                            // handle to application instance
        NULL);                                                                        // window-creation data
#endif

    if (!hWnd)
        {
        return FALSE;
        }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    if (textView)
        {
        windowTextViewToggle(hWnd);
        }

    SetTimer(hWnd, TIMER_ID, timerRate, NULL);

    return TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Copy clipboard data to keyboard buffer.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowClipboard(HWND hWnd)
    {
    HANDLE hClipMemory;
    char   *lpClipMemory;

    if (!IsClipboardFormatAvailable(CF_TEXT)
        || !OpenClipboard(hWnd))
        {
        return;
        }

    hClipMemory = GetClipboardData(CF_TEXT);
    if (hClipMemory == NULL)
        {
        CloseClipboard();

        return;
        }

    lpClipToKeyboard = malloc(GlobalSize(hClipMemory));
    if (lpClipToKeyboard != NULL)
        {
        lpClipMemory = GlobalLock(hClipMemory);
        strcpy(lpClipToKeyboard, lpClipMemory);
        GlobalUnlock(hClipMemory);
        lpClipToKeyboardPtr = lpClipToKeyboard;
        }

    CloseClipboard();
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process messages for the main window.
**
**  Parameters:     Name        Description.
**
**  Returns:        LRESULT
**
**------------------------------------------------------------------------*/
static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    int     wmId, wmEvent;
    LOGFONT lfTmp;
    RECT    rt;

    switch (message)
        {
    /*
    **  Process the application menu.
    */
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_ERASEBKGND:
        return (1);

        break;

    case WM_CREATE:
        hPen = CreatePen(PS_SOLID, 1, colorFG);
        if (!hPen)
            {
            MessageBox(GetFocus(),
                       "Unable to get foreground pen",
                       "(window_win32) CreatePen Error",
                       MB_OK);
            }

        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, fontName);
        lfTmp.lfWeight       = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight       = fontHeightSmall;
        hSmallFont           = CreateFontIndirect(&lfTmp);
        if (!hSmallFont)
            {
            MessageBox(GetFocus(),
                       "Unable to get small height font ",
                       "(window_win32) CreateFont Error",
                       MB_OK);
            }

        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, fontName);
        lfTmp.lfWeight       = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight       = fontHeightMedium;
        hMediumFont          = CreateFontIndirect(&lfTmp);
        if (!hMediumFont)
            {
            MessageBox(GetFocus(),
                       "Unable to get medium height font ",
                       "(window_win32) CreateFont Error",
                       MB_OK);
            }

        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, fontName);
        lfTmp.lfWeight       = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight       = fontHeightLarge;
        hLargeFont           = CreateFontIndirect(&lfTmp);
        if (!hLargeFont)
            {
            MessageBox(GetFocus(),
                       "Unable to get large height font ",
                       "(window_win32) CreateFont Error",
                       MB_OK);
            }

        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_DESTROY:
        if (hSmallFont)
            {
            DeleteObject(hSmallFont);
            }
        if (hMediumFont)
            {
            DeleteObject(hMediumFont);
            }
        if (hLargeFont)
            {
            DeleteObject(hLargeFont);
            }
        if (hPen)
            {
            DeleteObject(hPen);
            }
        if (hEditFont)
            {
            DeleteObject(hEditFont);
            }
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        windowTextViewLayout(hWnd);
        break;

    case WM_SETFOCUS:
        if (textViewActive)
            {
            SetFocus(hEdit[displayMode == ModeRight ? RightScreen : LeftScreen]);
            }
        break;

    case WM_TIMER:
        if (lpClipToKeyboard != NULL)
            {
            if (clipToKeyboardDelay == 0)
                {
                ppKeyIn = *lpClipToKeyboardPtr++;
                if (ppKeyIn == 0)
                    {
                    free(lpClipToKeyboard);
                    lpClipToKeyboard    = NULL;
                    lpClipToKeyboardPtr = NULL;
                    }
                else if (ppKeyIn == '\r')
                    {
                    clipToKeyboardDelay = 10;
                    }
                else if (ppKeyIn == '\n')
                    {
                    ppKeyIn = 0;
                    }
                }
            else
                {
                clipToKeyboardDelay -= 1;
                }
            }

        if (textViewActive)
            {
            windowTextViewUpdate();
            break;
            }

        GetClientRect(hWnd, &rt);
        InvalidateRect(hWnd, &rt, TRUE);
        break;

    /*
    **  Paint the main window.
    */
    case WM_PAINT:
        windowDisplay(hWnd);
        break;

        /*
        **  Handle input characters.
        */
#if CcDebug == 1
    case WM_KEYDOWN:
        if (GetKeyState(VK_CONTROL) & 0x8000)
            {
            switch (wParam)
                {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                dumpRunningPpu((u8)(wParam - '0'));
                break;

            case 'C':
            case 'c':
                dumpRunningCpu();
                break;
                }
            }

        break;
#endif

    /*
     * Posted to the window with the keyboard focus when a WM_SYSKEYDOWN message
     * is translated by the TranslateMessage function. It specifies the character
     * code of a system character key that is, a character key that is pressed
     * while the ALT key is down.
     */
    case WM_SYSCHAR:
        switch (wParam)
            {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            traceMask ^= ((u64)1 << (wParam - '0' + (shifted ? 10 : 0)));
            break;

        case 'C':
        case 'c':
            traceMask ^= ((u64)TraceCpu170 << 32) | ((u64)TraceCpu170 << 48);
            break;

        case 'E':
        case 'e':
            traceMask ^= ((u64)TraceExchange << 32) | ((u64)TraceExchange << 48);
            break;

        case 'F':
        case 'f':
            traceMask ^= ((u64)TraceCallFrame << 32) | ((u64)TraceCallFrame << 48);
            break;

        case 'V':
        case 'v':
            traceMask ^= ((u64)TraceCpu180 << 32) | ((u64)TraceCpu180 << 48);
            break;

        case 'W':
        case 'w':
            traceMask ^= ((u64)(TraceCpu180|TraceExchange|TraceBlockOp|TraceCallFrame|TraceConditions) << 32)
                       | ((u64)(TraceCpu180|TraceExchange|TraceBlockOp|TraceCallFrame|TraceConditions) << 48);
            break;

        case 'X':
        case 'x':
            traceMask = 0;
            break;

        case 'Y':
        case 'y':
            traceMask ^= ((u64)TraceConditions << 32) | ((u64)TraceConditions << 48);
            break;

        case 'L':
        case 'l':
        case '[':
            displayMode           = ModeLeft;
            displayModeNeedsErase = TRUE;
            windowTextViewLayout(hWnd);
            break;

        case 'R':
        case 'r':
        case ']':
            displayMode           = ModeRight;
            displayModeNeedsErase = TRUE;
            windowTextViewLayout(hWnd);
            break;

        case 'M':
        case 'm':
        case '\\':
            displayMode = ModeCenter;
            windowTextViewLayout(hWnd);
            break;

        case 'P':
        case 'p':
            windowClipboard(hWnd);
            break;

        case 'T':
        case 't':
            windowTextViewToggle(hWnd);
            break;

        case 's':
        case 'S':
            shifted = !shifted;
            }
        break;

    case WM_CHAR:
        ppKeyIn = (char)wParam;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }

    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display current list.
**
**  Parameters:     Name        Description.
**                  hWnd        window handle.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowDisplay(HWND hWnd)
    {
    static   refreshCount = 0;
    char     str[2]       = " ";
    DispList *curr;
    DispList *end;
    long     oldFont = 0;

    RECT        rect;
    PAINTSTRUCT ps;
    HDC         hdc;
    HBRUSH      hBrush;

    HDC     hdcMem;
    HBITMAP hbmMem, hbmOld;
    HFONT   hfntOld;

    if (textViewActive)
        {
        /*
        **  The text view child covers the client area; just validate.
        */
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);

        return;
        }

    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rect);

    /*
    **  Create a compatible DC.
    */

    hdcMem = CreateCompatibleDC(ps.hdc);

    /*
    **  Create a bitmap big enough for our client rect.
    */
    hbmMem = CreateCompatibleBitmap(ps.hdc,
                                    rect.right - rect.left,
                                    rect.bottom - rect.top);

    /*
    **  Select the bitmap into the off-screen dc.
    */
    hbmOld = SelectObject(hdcMem, hbmMem);

    hBrush = CreateSolidBrush(colorBG);
    FillRect(hdcMem, &rect, hBrush);
    if (displayModeNeedsErase)
        {
        displayModeNeedsErase = FALSE;
        FillRect(ps.hdc, &rect, hBrush);
        }
    DeleteObject(hBrush);

    SetBkMode(hdcMem, TRANSPARENT);
    SetBkColor(hdcMem, colorBG);
    SetTextColor(hdcMem, colorFG);

    hfntOld = SelectObject(hdcMem, hSmallFont);
    oldFont = fontSmall;

#if CcCycleTime
        {
        extern double cycleTime;
        char          buf[80];

//    sprintf(buf, "Cycle time: %.3f", cycleTime);
        sprintf(buf, "Cycle time: %10.3f    NPU Buffers: %5d", cycleTime, npuBipBufCount());
        TextOut(hdcMem, 0, 0, buf, strlen(buf));
        }
#endif

#if CcDebug == 1
        {
        char buf[160];

        /*
        **  Display P registers of PPUs and CPU and current trace mask.
        */
        sprintf(buf, "Refresh: %-10d  PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg: %06o"
                     "   Trace0x: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c %c",
                refreshCount++,
                ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
                ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
                cpus170[0].regP,
                (traceMask >> 0) & 1 ? '0' : '_',
                (traceMask >> 1) & 1 ? '1' : '_',
                (traceMask >> 2) & 1 ? '2' : '_',
                (traceMask >> 3) & 1 ? '3' : '_',
                (traceMask >> 4) & 1 ? '4' : '_',
                (traceMask >> 5) & 1 ? '5' : '_',
                (traceMask >> 6) & 1 ? '6' : '_',
                (traceMask >> 7) & 1 ? '7' : '_',
                (traceMask >> 8) & 1 ? '8' : '_',
                (traceMask >> 9) & 1 ? '9' : '_',
                traceMask & ((u64)TraceCpu170 << 32) ? 'C' : '_',
                traceMask & ((u64)TraceCpu180 << 32) ? 'V' : '_',
                traceMask & ((u64)TraceExchange << 32) ? 'E' : '_',
                traceMask & ((u64)TraceCallFrame << 32) ? 'F' : '_',
                traceMask & ((u64)TraceConditions << 32) ? 'Y' : '_',
                shifted ? ' ' : '<');

        TextOut(hdcMem, 0, 0, buf, strlen(buf));

        if (ppuCount == 20)
            {
            /*
            **  Display P registers of second barrel of PPUs.
            */
            sprintf(buf, "                     PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o                    "
                         "   Trace1x: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c %c",
                    ppu[10].regP, ppu[11].regP, ppu[12].regP, ppu[13].regP, ppu[14].regP,
                    ppu[15].regP, ppu[16].regP, ppu[17].regP, ppu[18].regP, ppu[19].regP,
                    (traceMask >> 10) & 1 ? '0' : '_',
                    (traceMask >> 11) & 1 ? '1' : '_',
                    (traceMask >> 12) & 1 ? '2' : '_',
                    (traceMask >> 13) & 1 ? '3' : '_',
                    (traceMask >> 14) & 1 ? '4' : '_',
                    (traceMask >> 15) & 1 ? '5' : '_',
                    (traceMask >> 16) & 1 ? '6' : '_',
                    (traceMask >> 17) & 1 ? '7' : '_',
                    (traceMask >> 18) & 1 ? '8' : '_',
                    (traceMask >> 19) & 1 ? '9' : '_',
                    (cpuCount > 1) && (traceMask & ((u64)TraceCpu170 << 48)) ? 'C' : '_',
                    (cpuCount > 1) && (traceMask & ((u64)TraceCpu180 << 48)) ? 'V' : '_',
                    (cpuCount > 1) && (traceMask & ((u64)TraceExchange << 48)) ? 'E' : '_',
                    (cpuCount > 1) && (traceMask & ((u64)TraceCallFrame << 48)) ? 'F' : '_',
                    (cpuCount > 1) && (traceMask & ((u64)TraceConditions << 48)) ? 'Y' : '_',
                    shifted ? '<' : ' ');

            TextOut(hdcMem, 0, 12, buf, strlen(buf));
            }
        }
#endif

    if (opPaused)
        {
        static char opMessage[] = "(window_win32) Emulation paused";
        hfntOld = SelectObject(hdcMem, hLargeFont);
        oldFont = fontLarge;
        TextOut(hdcMem, (0 * scaleX) / 10, (256 * scaleY) / 10, opMessage, (int)strlen(opMessage));
        }
    else if (consoleIsRemoteActive())
        {
        static char opMessage[] = "Remote console active";
        hfntOld = SelectObject(hdcMem, hLargeFont);
        oldFont = fontLarge;
        TextOut(hdcMem, (0 * scaleX) / 10, (256 * scaleY) / 10, opMessage, (int)strlen(opMessage));
        }


    SelectObject(hdcMem, hPen);

    curr = display;
    end  = display + listEnd;
    for (curr = display; curr < end; curr++)
        {
        if (oldFont != curr->fontSize)
            {
            oldFont = curr->fontSize;

            if (oldFont == fontSmall)
                {
                SelectObject(hdcMem, hSmallFont);
                }

            if (oldFont == fontMedium)
                {
                SelectObject(hdcMem, hMediumFont);
                }

            if (oldFont == fontLarge)
                {
                SelectObject(hdcMem, hLargeFont);
                }
            }

        if (curr->fontSize == FontDot)
            {
            SetPixel(hdcMem, (curr->xPos * scaleX) / 10, (curr->yPos * scaleY) / 10 + 30, colorFG);
            }
        else
            {
            str[0] = curr->ch;
            TextOut(hdcMem, (curr->xPos * scaleX) / 10, (curr->yPos * scaleY) / 10 + 20, str, 1);
            }
        }

    listEnd  = 0;
    currentX = -1;
    currentY = -1;

    if (hfntOld)
        {
        SelectObject(hdcMem, hfntOld);
        }

    /*
    **  Blit the changes to the screen dc.
    */
    switch (displayMode)
        {
    default:
    case ModeCenter:
        BitBlt(ps.hdc,
               rect.left, rect.top,
               rect.right - rect.left, rect.bottom - rect.top,
               hdcMem,
               0, 0,
               SRCCOPY);
        break;

    case ModeLeft:
        StretchBlt(ps.hdc,
                   rect.left + (rect.right - rect.left) / 2 - 512 * scaleY / 10 / 2, rect.top,
                   512 * scaleY / 10, rect.bottom - rect.top,
                   hdcMem,
                   OffLeftScreen, 0,
                   512 * scaleX / 10 + fontLarge, rect.bottom - rect.top,
                   SRCCOPY);
        break;

    case ModeRight:
        StretchBlt(ps.hdc,
                   rect.left + (rect.right - rect.left) / 2 - 512 * scaleY / 10 / 2, rect.top,
                   512 * scaleY / 10, rect.bottom - rect.top,
                   hdcMem,
                   OffRightScreen, 0,
                   512 * scaleX / 10 + fontLarge, rect.bottom - rect.top,
                   SRCCOPY);
        break;
        }

    /*
    **  Done with off screen bitmap and dc.
    */
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(hWnd, &ps);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Toggle the text view on or off, creating the EDIT
**                  controls and their labels on first use.
**
**  Parameters:     Name        Description.
**                  hWnd        console window handle.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextViewToggle(HWND hWnd)
    {
    static char *labels[2] = { "Left screen", "Right screen" };
    int         s;

    if (hEdit[LeftScreen] == NULL)
        {
        hEditFont = CreateFont(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               FIXED_PITCH | FF_MODERN, "Consolas");

        /*
        **  Each label is created just before its EDIT control so that
        **  screen readers use it as the control's name.
        */
        for (s = 0; s < 2; s++)
            {
            hLabel[s] = CreateWindowEx(0, "STATIC", labels[s], WS_CHILD | SS_LEFT,
                                       0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)(IDC_TEXTVIEW + 2 + s), hInstance, NULL);
            hEdit[s] = CreateWindowEx(0, "EDIT", "",
                                      WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_LEFT,
                                      0, 0, 0, 0, hWnd, (HMENU)(INT_PTR)(IDC_TEXTVIEW + s), hInstance, NULL);
            if ((hLabel[s] == NULL) || (hEdit[s] == NULL))
                {
                MessageBox(hWnd, "Unable to create text view", "(window_win32) CreateWindowEx Error", MB_OK);
                hEdit[LeftScreen] = NULL;

                return;
                }
            if (hEditFont)
                {
                SendMessage(hLabel[s], WM_SETFONT, (WPARAM)hEditFont, 0);
                SendMessage(hEdit[s], WM_SETFONT, (WPARAM)hEditFont, 0);
                }
            SendMessage(hEdit[s], EM_SETLIMITTEXT, 0, 0);

            /*
            **  Subclass the control so keystrokes reach the emulated
            **  keyboard while the caret keys still navigate the text.
            */
            editProc = (WNDPROC)SetWindowLongPtr(hEdit[s], GWLP_WNDPROC, (LONG_PTR)windowTextViewProcedure);
            }

        SetWindowLongPtr(hWnd, GWL_STYLE, GetWindowLongPtr(hWnd, GWL_STYLE) | WS_CLIPCHILDREN);
        }

    textViewActive = !textViewActive;
    if (textViewActive)
        {
        textPrevLines[LeftScreen]  = -1;
        textPrevLines[RightScreen] = -1;
        windowTextViewLayout(hWnd);
        SetFocus(hEdit[displayMode == ModeRight ? RightScreen : LeftScreen]);
        }
    else
        {
        for (s = 0; s < 2; s++)
            {
            ShowWindow(hLabel[s], SW_HIDE);
            ShowWindow(hEdit[s], SW_HIDE);
            }
        SetFocus(hWnd);
        displayModeNeedsErase = TRUE;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Size and show the text view controls for the current
**                  display mode: both screens side by side, or one of
**                  them filling the window.
**
**  Parameters:     Name        Description.
**                  hWnd        console window handle.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextViewLayout(HWND hWnd)
    {
    HWND focus;
    int  height;
    RECT rect;
    int  s;
    bool shown[2];
    int  width;
    int  x;

    if ((hEdit[LeftScreen] == NULL) || !textViewActive)
        {
        return;
        }

    shown[LeftScreen]  = displayMode != ModeRight;
    shown[RightScreen] = displayMode != ModeLeft;

    GetClientRect(hWnd, &rect);
    width  = rect.right - rect.left;
    height = rect.bottom - rect.top;
    if (displayMode == ModeCenter)
        {
        width /= 2;
        }

    x = 0;
    for (s = 0; s < 2; s++)
        {
        if (shown[s])
            {
            if (!IsWindowVisible(hEdit[s]))
                {
                textPrevLines[s] = -1;
                }
            MoveWindow(hLabel[s], x, 0, width, TextLabelHeight, TRUE);
            MoveWindow(hEdit[s], x, TextLabelHeight, width, height - TextLabelHeight, TRUE);
            ShowWindow(hLabel[s], SW_SHOW);
            ShowWindow(hEdit[s], SW_SHOW);
            x += width;
            }
        else
            {
            ShowWindow(hLabel[s], SW_HIDE);
            ShowWindow(hEdit[s], SW_HIDE);
            }
        }

    /*
    **  Do not leave the focus on a hidden control.
    */
    focus = GetFocus();
    if ((focus == hEdit[LeftScreen]) && !shown[LeftScreen])
        {
        SetFocus(hEdit[RightScreen]);
        }
    else if ((focus == hEdit[RightScreen]) && !shown[RightScreen])
        {
        SetFocus(hEdit[LeftScreen]);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Convert the display list into a character grid and
**                  push each screen to its text view control.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextViewUpdate(void)
    {
    int      col;
    DispList *curr;
    DispList *end;
    int      lines;
    int      r;
    int      s;
    int      screen;
    char     *tp;
    int      x;

    if (opPaused)
        {
        listEnd = 0;
        windowTextViewMessage("Emulation paused");

        return;
        }

    if (consoleIsRemoteActive())
        {
        listEnd = 0;
        windowTextViewMessage("Remote console active");

        return;
        }

    if (listEnd == 0)
        {
        /*
        **  Nothing was drawn during this tick; keep the last text.
        */
        return;
        }

    memset(textGrid, ' ', sizeof(textGrid));

    end = display + listEnd;
    for (curr = display; curr < end; curr++)
        {
        if (curr->fontSize == FontDot)
            {
            continue;
            }

        if (curr->xPos >= OffRightScreen)
            {
            screen = RightScreen;
            x      = curr->xPos - OffRightScreen;
            }
        else
            {
            screen = LeftScreen;
            x      = curr->xPos - OffLeftScreen;
            }

        col = x / curr->fontSize;
        r   = curr->yPos / 10;
        if ((x < 0) || (col >= TextCols) || (r >= TextRows)
            || (curr->ch < 0x20) || (curr->ch > 0x7e))
            {
            continue;
            }

        textGrid[screen][r][col] = curr->ch;
        }

    listEnd  = 0;
    currentX = -1;
    currentY = -1;

    /*
    **  Lay each screen out as fixed-width lines, dropping trailing
    **  blank rows, so that a line's offset in the control never
    **  depends on its content.
    */
    for (s = 0; s < 2; s++)
        {
        lines = 0;
        for (r = TextRows - 1; r >= 0 && lines == 0; r--)
            {
            for (col = 0; col < TextCols; col++)
                {
                if (textGrid[s][r][col] != ' ')
                    {
                    lines = r + 1;
                    break;
                    }
                }
            }

        tp = textBuf;
        for (r = 0; r < lines; r++)
            {
            memcpy(tp, textGrid[s][r], TextCols);
            tp   += TextCols;
            *tp++ = '\r';
            *tp++ = '\n';
            }
        if (lines > 0)
            {
            tp -= 2;
            }
        *tp = '\0';

        windowTextViewSet(s, textBuf, lines);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Show a one-line status message in both text views.
**
**  Parameters:     Name        Description.
**                  msg         message text (at most TextCols chars)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextViewMessage(char *msg)
    {
    size_t len = strlen(msg);
    int    s;

    if (len > TextCols)
        {
        len = TextCols;
        }
    memset(textBuf, ' ', TextCols);
    memcpy(textBuf, msg, len);
    textBuf[TextCols] = '\0';
    for (s = 0; s < 2; s++)
        {
        windowTextViewSet(s, textBuf, 1);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Replace a text view's content, patching only the
**                  lines that changed and preserving the caret and
**                  scroll position.
**
**  Parameters:     Name        Description.
**                  s           screen ordinal (LeftScreen/RightScreen)
**                  text        new content, TextLineLen bytes per line
**                  lines       number of lines in text
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextViewSet(int s, char *text, int lines)
    {
    bool  changed      = FALSE;
    int   firstVisible = 0;
    HWND  hw           = hEdit[s];
    int   i;
    int   off;
    char  *prev = textPrev[s];
    char  save;
    DWORD selEnd   = 0;
    DWORD selStart = 0;

    /*
    **  Hidden controls are left alone; they are refreshed in full when
    **  shown again.
    */
    if ((hw == NULL) || !IsWindowVisible(hw))
        {
        return;
        }

    if (lines != textPrevLines[s])
        {
        SendMessage(hw, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
        firstVisible = (int)SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0);
        SetWindowText(hw, text);
        SendMessage(hw, EM_SETSEL, selStart, selEnd);
        SendMessage(hw, EM_LINESCROLL, 0,
                    firstVisible - (int)SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0));
        strcpy(prev, text);
        textPrevLines[s] = lines;

        return;
        }

    for (i = 0; i < lines; i++)
        {
        off = i * TextLineLen;
        if (memcmp(text + off, prev + off, TextCols) == 0)
            {
            continue;
            }

        if (!changed)
            {
            changed = TRUE;
            SendMessage(hw, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
            firstVisible = (int)SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0);
            }

        save = text[off + TextCols];
        text[off + TextCols] = '\0';
        SendMessage(hw, EM_SETSEL, off, off + TextCols);
        SendMessage(hw, EM_REPLACESEL, FALSE, (LPARAM)(text + off));
        text[off + TextCols] = save;
        }

    if (changed)
        {
        SendMessage(hw, EM_SETSEL, selStart, selEnd);
        SendMessage(hw, EM_LINESCROLL, 0,
                    firstVisible - (int)SendMessage(hw, EM_GETFIRSTVISIBLELINE, 0, 0));
        strcpy(prev, text);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Subclass procedure for the text view EDIT controls.
**                  Typed characters go to the emulated keyboard, Tab
**                  moves between the two screens, Alt keys go to the
**                  console window, everything else (caret movement,
**                  copy) is left to the control.
**
**  Parameters:     Name        Description.
**                  hEditWnd    EDIT control handle.
**                  message     window message.
**                  wParam      message parameter.
**                  lParam      message parameter.
**
**  Returns:        LRESULT
**
**------------------------------------------------------------------------*/
static LRESULT CALLBACK windowTextViewProcedure(HWND hEditWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    HWND other;

    switch (message)
        {
    case WM_KEYDOWN:
        if (wParam == VK_TAB)
            {
            other = hEdit[hEditWnd == hEdit[LeftScreen] ? RightScreen : LeftScreen];
            if (IsWindowVisible(other))
                {
                SetFocus(other);
                }

            return 0;
            }
        break;

    case WM_CHAR:
        if ((GetKeyState(VK_CONTROL) & 0x8000) || (wParam == '\t'))
            {
            break;
            }
        ppKeyIn = (char)wParam;

        return 0;

    case WM_SYSCHAR:
        windowProcedure(hWnd, message, wParam, lParam);

        return 0;
        }

    return CallWindowProc(editProc, hEditWnd, message, wParam, lParam);
    }

/*---------------------------  End Of File  ------------------------------*/
