#pragma once

#include <windows.h>

struct Win32BitmapBuffer {
    BITMAPINFO  Info;
    void*       Memory;
    int         Width;
    int         Height;
    int         Pitch;
    int         BytesPerPixel;
};

//  CONSTANTS
const int BUFFER_WIDTH = 1280;
const int BUFFER_HEIGHT = 720;

// TODO: This is global for now.
static bool Running;
static Win32BitmapBuffer globalBackbuffer = { };

//	FUNCTIONS
bool Win32InitWindow(HINSTANCE hInstance, int nCmdShow, HWND* hwnd);
void Win32ProcessMessageQueue();
void Win32UpdateWindow(HWND* hwnd);
