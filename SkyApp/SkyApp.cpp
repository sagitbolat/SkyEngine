
#include "SkyWin32.h"
#include <stdint.h>


// Forward Function Definitions
static void RenderWeirdGradient(Win32BitmapBuffer buffer, int xOffset, int yOffset);

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

    while (Running) {
        Win32ProcessMessageQueue();

        //draw gradient
        RenderWeirdGradient(globalBackbuffer, xOffset, yOffset);

        Win32UpdateWindow(&hwnd);

        ++xOffset;
        ++yOffset;
    }
    return 0;
}

static void RenderWeirdGradient(Win32BitmapBuffer buffer, int xOffset, int yOffset) {
    // Drawing Logic.

    uint8_t* row = (uint8_t*)buffer.Memory;

    for (int y = 0; y < buffer.Height; ++y) {

        uint32_t* pixel = (uint32_t*)row;

        for (int x = 0; x < buffer.Width; ++x) {

            uint8_t blue    = (uint8_t)(x + xOffset);       //blue channel
            uint8_t green   = (uint8_t)(0);                 //green channel
            uint8_t red     = (uint8_t)(y + yOffset);       //red channel


            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer.Pitch;
    }
}

