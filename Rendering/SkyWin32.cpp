#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";

    WNDCLASS wc = { };

    wc.lpfnWndProc      = WindowProc;
    wc.hInstance        = hInstance;
    wc.lpszClassName        = CLASS_NAME;
    wc.style        = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;      // TODO: THIS MIGHT BE NOT NECESSARY
    //wc.hIcon = TODO: Set the icon

        
    // Register window class.
    if (!RegisterClass(&wc)) return 1;      // TODO:  Log Windowclass register failure.

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,            // Optional window styles.
        CLASS_NAME,   // Window class
        L"SkyApp",    // Window text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style

        250, 250,   // Position 
        1280, 720,  // Size

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) return 1;     // TODO: Log windowhandler not defined.

    ShowWindow(hwnd, nCmdShow);


    // Run the message loop.
    // TODO: Replace later with a better loop.
    for(;;) {
        MSG message = { };
        BOOL messageResult = GetMessage(&message, 0, 0, 0);

        if (messageResult <= 0) break;
        
        TranslateMessage    (&message);
        DispatchMessage     (&message);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = { };
    switch (uMsg)
    {
        case WM_SIZE: {
            OutputDebugStringA("SIZE\n");
        } break;
        case WM_DESTROY: {
            OutputDebugStringA("DESTROY\n");
            PostQuitMessage(0);
            return 0;
        } break;
        case WM_CLOSE: {
            OutputDebugStringA("CLOSE\n");
        } break;
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("ACTIVATEAPP\n");
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hwnd, &Paint);
            
            int x = Paint.rcPaint.left;
            int y = Paint.rcPaint.top;
            int width = Paint.rcPaint.right - x;
            int height = Paint.rcPaint.bottom - y;
            PatBlt(
                DeviceContext,
                x,
                y,
                width,
                height,
                BLACKNESS
            );

            EndPaint(hwnd, &Paint);
        } break;
        default: {
//            OutputDebugStringA("default\n");
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return Result;
}
