#include <windows.h>

#include <stdint.h>

//  GLOBAL VARIABLES
static bool Running;                // TODO: This is global for now.
static BITMAPINFO bitmapInfo;     
static void *bitmapMemory;          
static int bitmapWidth = { };
static int bitmapHeight = { };


//  FORWARD FUNCTION DECLERATIONS
LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void Win32ResizeDIBSection(int width, int height);
static void Win32UpdateWindow(HDC DeviceContext, RECT* WindowRect, int x, int y, int width, int height);

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";

    WNDCLASS wc = { };

    wc.lpfnWndProc      = Win32WindowProc;
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

    Running = true;

    // Run the message loop.
    // TODO: Replace later with a better loop.
    while(Running) {
        MSG message = { };
        BOOL messageResult = GetMessage(&message, 0, 0, 0);

        if (messageResult <= 0) break;
        
        TranslateMessage    (&message);
        DispatchMessage     (&message);
    }
    return 0;
}

LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = { };
    switch (uMsg) {
        case WM_SIZE: {
            RECT windowSize = { }; 
            GetClientRect(hwnd, &windowSize);
            int width = windowSize.right - windowSize.left;
            int height = windowSize.bottom - windowSize.top;
            Win32ResizeDIBSection(width, height);
        } break;
        case WM_DESTROY: {
            
            // TODO: Handle as an error if Running is true and recreate window.
            Running = false;
            OutputDebugStringA("DESTROY\n");
            return 0;
        } break;
        case WM_CLOSE: {

            // TODO: Give the user an "Are you sure you want to quit?" message.
            Running = false;
            
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
            int width = Paint.rcPaint.right - Paint.rcPaint.left;
            int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
           
            RECT windowSize = { };
            GetClientRect(hwnd, &windowSize);
            Win32UpdateWindow(DeviceContext, &windowSize, x, y, width, height);

            EndPaint(hwnd, &Paint);
        } break;
        default: {
//            OutputDebugStringA("default\n");
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return Result;
}

static void Win32ResizeDIBSection(int width, int height) {
    // TODO: Maybe dont free first, free after and then free first if it fails.
    
    if (bitmapMemory) {
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

    bitmapWidth = width;
    bitmapHeight = height;

    bitmapInfo.bmiHeader.biSize             = sizeof(bitmapInfo.bmiHeader); 
    bitmapInfo.bmiHeader.biWidth            = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight           = -bitmapHeight;
    bitmapInfo.bmiHeader.biPlanes           = 1;
    bitmapInfo.bmiHeader.biBitCount         = 32;
    bitmapInfo.bmiHeader.biCompression      = BI_RGB;
    // bitmapInfo.bmiHeader.bisizeImage        = 0; 
    // bitmapInfo.bmiHeader.biXPelsPerMeter    = 0; 
    // bitmapInfo.bmiHeader.biYPelsPerMeter    = 0; 
    // bitmapInfo.bmiHeader.biClrUsed          = 0;
    // bitmapInfo.bmiHeader.biClrImport        = 0;
    
    int bytesPerPixel = 4;
    int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // Drawing Logic.
    int pitch = width * bytesPerPixel;
    uint8_t *row = (uint8_t *)bitmapMemory;
    for (int y = 0; y < bitmapHeight; ++y) {
        uint8_t *pixel = (uint8_t *)row;
        for (int x = 0; x < bitmapWidth; ++x) {
            //blue channel
            *pixel = 0;
            ++pixel;
            //green channel
            *pixel = 0;
            ++pixel;
            //red channel
            *pixel = 255;
            ++pixel;
            //padding channel. leave at 0
            *pixel = 0;
            ++pixel;
        }
        row += pitch;
    }

}

static void Win32UpdateWindow(HDC DeviceContext, RECT* WindowRect, int x, int y, int width, int height) {
    
    int windowWidth = WindowRect->right - WindowRect->left;
    int windowHeight = WindowRect->bottom - WindowRect->top;

    StretchDIBits(
        DeviceContext,
        /*x, y, width, height,        // This is the buffer we are drawing to.
        x, y, width, height, */       // This is the buffer we are drawing from.
        0, 0, bitmapWidth, bitmapHeight,
        0, 0, windowWidth, windowHeight,
        bitmapMemory,
        &bitmapInfo,
        DIB_RGB_COLORS,             // Specifies the type of bitmap, in this case RGB color. Can also be set to DIB_PAL_COLORS.
        SRCCOPY                     // A bit-wise operation that specifies that we are copying from one bitmap to another. 
    );
}
