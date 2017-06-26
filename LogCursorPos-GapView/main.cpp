
#include "helpers.h"

void DrawBordersForDirectionToCursor(HDC hdc, HWND hwnd, const WindowRenderData & rd, RECT* prcClient)
{
    RECT rcBorder;
    int iBorderWidth = 10;
    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);

    static HBRUSH hbrWhite = CreateSolidBrush(RGB(255, 255, 255));
    static HBRUSH hbrBlack = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH* phbr = (GetRValue(rd.rgbText) == 0) ? &hbrBlack : &hbrWhite;

    // Highlite each side of the client area if the cursor is on that side of the window
    if (rd.ptLast.x < rcWindow.left) {
        rcBorder = *prcClient;
        rcBorder.right = prcClient->left + iBorderWidth;
        FillRect(hdc, &rcBorder, *phbr);
    }

    if (rd.ptLast.x > rcWindow.right) {
        rcBorder = *prcClient;
        rcBorder.left = prcClient->right - iBorderWidth;
        FillRect(hdc, &rcBorder, *phbr);
    }

    if (rd.ptLast.y < rcWindow.top) {
        rcBorder = *prcClient;
        rcBorder.bottom = prcClient->top + iBorderWidth;
        FillRect(hdc, &rcBorder, *phbr);
    }

    if (rd.ptLast.y > rcWindow.bottom) {
        rcBorder = *prcClient;
        rcBorder.top = prcClient->bottom - iBorderWidth;
        FillRect(hdc, &rcBorder, *phbr);
    }

    // Bump in rcClient by double the border for the text
    prcClient->top += 2 * iBorderWidth;
    prcClient->left += 2 * iBorderWidth;
    prcClient->right -= 2 * iBorderWidth;
    prcClient->bottom -= 2 * iBorderWidth;
}

void DrawStats(HDC hdc, HWND hwnd, const WindowRenderData & rd, RECT rcClient)
{
    // Set up text mode and font
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, rd.rgbText);
    const int txtSize = 30;
    static HFONT hfont = CreateFont(
        txtSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
    SelectFont(hdc, hfont);

    // Create buffers, and butt-ugly macros to make it f-in readable...
    WCHAR buf[100];
    RECT rcTxt = rcClient;
#define STR (LPWSTR)&buf
#define PRINT_TEXT(prc, DT_flags, ...) \
    wsprintf(STR, __VA_ARGS__); \
    DrawText(hdc, STR, wcslen(STR), prc, DT_flags);
#define PRINT_CENTER(...) \
    PRINT_TEXT(&rcTxt, DT_CENTER | DT_VCENTER | DT_SINGLELINE, __VA_ARGS__)
#define PRINT_LEFT( ...) \
    PRINT_TEXT(&rcTxt, DT_LEFT, __VA_ARGS__); \
    rcTxt.top += txtSize;

    // Draw Text
    PRINT_LEFT(L"avg : %i", rd.avgSpeed);
    PRINT_LEFT(L"%%: %i", rd.percentageOfGoal);
    PRINT_LEFT(L"pos: %i x %i", rd.ptLast.x, rd.ptLast.y);
}

void Draw(HDC hdc, HWND hwnd)
{
    WindowRenderData rd;
    if (!GetRenderDataForWindow(hwnd, &rd)) {
        return;
    }

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    FillRect(hdc, &rcClient, rd.hbrBackground);
    
    DrawBordersForDirectionToCursor(hdc, hwnd, rd, &rcClient);
    DrawStats(hdc, hwnd, rd, rcClient);
}

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
#define IDT_TIMER 10101
    static bool bFreeze = false;

    switch (message)
    {
    case WM_CREATE:
        AddWindowToMap(hwnd);
        SetTimer(hwnd, IDT_TIMER, TIMER_DELAY, NULL);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        Draw(BeginPaint(hwnd, &ps), hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_TIMER:
        if (wParam == IDT_TIMER && !bFreeze) {
            POINT pt;
            GetCursorPos(&pt);
            AddPoint(hwnd, pt);
            InvalidateRect(hwnd, NULL, TRUE); // hack alert
        }
        break;
    case WM_CHAR:
        if (wParam == VK_SPACE) {
            bFreeze = !bFreeze;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);        
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    LPCWSTR szWndClass = L"class",
            szWndTitle = L"Logical Cursor Pos Test";

    // Register the window class
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = szWndClass;
    if (!RegisterClassEx(&wcex)) {
        return 1;
    }

    // Create three windows
    for (int i = 0; i < 3; i++)
    {
        HWND hwnd = CreateWindowEx(0,
            szWndClass,
            szWndTitle,
            WS_OVERLAPPEDWINDOW /*WS_POPUPWINDOW*/,
            50 + (20 * i), 50 + (20 * i), 400, 200,
            NULL, nullptr, hInstance, nullptr);

        if (!hwnd) {
            return 1;
        }

        ShowWindow(hwnd, SW_SHOW);
    }
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


