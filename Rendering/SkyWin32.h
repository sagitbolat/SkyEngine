#pragma once

struct Win32BitmapBuffer;

//  CONSTANTS
const int BUFFER_WIDTH = 1280;
const int BUFFER_HEIGHT = 720;

//	FUNCTIONS
static HWND Win32InitWindow(HINSTANCE hInstance, int nCmdShow, HWND* hwnd);
static void Win32ProcessMessageQueue();
static void Win32UpdateWindow(HWND* hwnd);
