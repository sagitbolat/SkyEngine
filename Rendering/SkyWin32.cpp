#include <windows.h>

#include <stdint.h>
#include <Xinput.h>
#include "SkyWin32.h"

struct Win32BitmapBuffer {
    BITMAPINFO  Info;
    void*       Memory;
    int         Width;
    int         Height;
    int         Pitch;
    int         BytesPerPixel;
};

struct Win32WindowDimensions {
    int Width;
    int Height;
};
inline Win32WindowDimensions Win32GetWindowDimensions(HWND hwnd) {
    RECT windowRect = { };
    GetClientRect(hwnd, &windowRect);
    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;
    return(Win32WindowDimensions{ width, height });
}



//  CONSTANTS
const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";

//  GLOBAL VARIABLES 
// TODO: This is global for now.
static bool Running;   // TODO: Move outside engine code when moving the gameloop.
static Win32BitmapBuffer globalBackbuffer = { };

//  FORWARD FUNCTION DECLERATIONS
LRESULT CALLBACK            Win32WindowProc         (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static  Win32BitmapBuffer   Win32ResizeDIBSection   (Win32BitmapBuffer buffer, int width, int height);
static  void                Win32CopyBufferToWindow (HDC DeviceContext, int windowWidth, int windowHeight, Win32BitmapBuffer buffer);
static  void                RenderWeirdGradient     (Win32BitmapBuffer buffer, int xOffset, int yOffset);



//the wWinMain is temporary. Move to different file
int WINAPI wWinMain(
    _In_        HINSTANCE   hInstance,
    _In_opt_    HINSTANCE   hPrevInstance,
    _In_        LPWSTR      lpCmdLine,
    _In_        int         nCmdShow)
{
    HWND hwnd = { };
    Win32InitWindow(hInstance, nCmdShow, &hwnd);

    // Run the game loop.
    // TODO: Replace later with a better loop.
    Running = true;
    int xOffset = 0;
    int yOffset = 0;

    while(Running) {
        Win32ProcessMessageQueue();

        //draw gradient
        RenderWeirdGradient(globalBackbuffer, xOffset, yOffset);

        Win32UpdateWindow(&hwnd);

        ++xOffset;
        ++yOffset;
    }
    return 0;
}



static HWND Win32InitWindow(HINSTANCE hInstance, int nCmdShow, HWND* hwnd) {
    // Register the window class.

    globalBackbuffer = Win32ResizeDIBSection(globalBackbuffer, BUFFER_WIDTH, BUFFER_HEIGHT);

    WNDCLASS wc = { };

    wc.lpfnWndProc = Win32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;      // TODO: THIS MIGHT BE NOT NECESSARY
    //wc.hIcon = TODO: Set the icon


    // Register window class.
    if (!RegisterClass(&wc)) return(0);      // TODO:  Log Windowclass register failure.

    // Create the window.
    *hwnd = CreateWindowEx(
        0,              // Optional window styles.
        CLASS_NAME,     // Window class
        L"SkyApp",      // Window text

        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style

        CW_USEDEFAULT, CW_USEDEFAULT,       // Position 
        CW_USEDEFAULT, CW_USEDEFAULT,      // Size

        NULL,           // Parent window    
        NULL,           // Menu
        hInstance,      // Instance handle
        NULL            // Additional application data
    );

    if (*hwnd == NULL) return(0);     // TODO: Log windowhandler not defined.

    ShowWindow(*hwnd, nCmdShow);// Register the window class.
}

static void Win32ProcessMessageQueue() {
    //Process Message Queue
    MSG message = { };
    while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

static void Win32UpdateWindow(HWND *hwnd) {
    HDC DeviceContext = GetDC(*hwnd);
    Win32WindowDimensions windowDimensions = Win32GetWindowDimensions(*hwnd);
    Win32CopyBufferToWindow(DeviceContext, windowDimensions.Width, windowDimensions.Height, globalBackbuffer);
    ReleaseDC(*hwnd, DeviceContext);
}



LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = { };
    switch (uMsg) {
        case WM_SIZE: {
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
           
            int x       = Paint.rcPaint.left;
            int y       = Paint.rcPaint.top;
            int width   = Paint.rcPaint.right - Paint.rcPaint.left;
            int height  = Paint.rcPaint.bottom - Paint.rcPaint.top;
           
            Win32WindowDimensions windowDimensions = Win32GetWindowDimensions(hwnd);
            Win32CopyBufferToWindow(DeviceContext, windowDimensions.Width, windowDimensions.Height, globalBackbuffer);

            EndPaint(hwnd, &Paint);
        } break;
        default: {
//            OutputDebugStringA("default\n");
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return Result;
}

static Win32BitmapBuffer Win32ResizeDIBSection(Win32BitmapBuffer buffer, int width, int height) {
    // TODO: Maybe dont free first, free after and then free first if it fails.
    
    if (buffer.Memory) {
        VirtualFree(buffer.Memory, 0, MEM_RELEASE);
    }

    buffer.Width = width;
    buffer.Height = height;

    buffer.Info.bmiHeader.biSize             = sizeof(buffer.Info.bmiHeader);
    buffer.Info.bmiHeader.biWidth            = width;
    buffer.Info.bmiHeader.biHeight           = -height;
    buffer.Info.bmiHeader.biPlanes           = 1;
    buffer.Info.bmiHeader.biBitCount         = 32;
    buffer.Info.bmiHeader.biCompression      = BI_RGB;
    // bitmapInfo.bmiHeader.bisizeImage        = 0; 
    // bitmapInfo.bmiHeader.biXPelsPerMeter    = 0; 
    // bitmapInfo.bmiHeader.biYPelsPerMeter    = 0; 
    // bitmapInfo.bmiHeader.biClrUsed          = 0;
    // bitmapInfo.bmiHeader.biClrImport        = 0;
    
    buffer.BytesPerPixel = 4;
    int bitmapMemorySize = (width * height) * buffer.BytesPerPixel;
    buffer.Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Clear screen to black?
    
    buffer.Pitch  = buffer.Width * buffer.BytesPerPixel;
    return (buffer);
}

static void Win32CopyBufferToWindow(HDC DeviceContext, int windowWidth, int windowHeight, Win32BitmapBuffer buffer) {
    // TODO: Correct the aspect ratio

    StretchDIBits(
        DeviceContext,
        /*x, y, width, height,        // This is the buffer we are drawing to.
        x, y, width, height, */       // This is the buffer we are drawing from.
        0, 0, windowWidth, windowHeight,
        0, 0, buffer.Width, buffer.Height,
        buffer.Memory,
        &buffer.Info,
        DIB_RGB_COLORS,             // Specifies the type of bitmap, in this case RGB color. Can also be set to DIB_PAL_COLORS.
        SRCCOPY                     // A bit-wise operation that specifies that we are copying from one bitmap to another. 
    );
}


static void RenderWeirdGradient(Win32BitmapBuffer buffer, int xOffset, int yOffset) {
    // Drawing Logic.
    
    uint8_t* row    = (uint8_t*)buffer.Memory;

    for (int y = 0; y < buffer.Height; ++y) {

        uint32_t* pixel     = (uint32_t*)row;

        for (int x = 0; x < buffer.Width; ++x) {

            uint8_t blue    = (uint8_t)(x + xOffset);       //blue channel
            uint8_t green   = (uint8_t)(0);                 //green channel
            uint8_t red     = (uint8_t)(y + yOffset);       //red channel
            
            
            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer.Pitch;
    }
}
