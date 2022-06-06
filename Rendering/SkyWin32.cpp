#include <windows.h>

const wchar_t* CLASS_NAME = L"SkyApp";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool InitWindow(HINSTANCE hInstance, int nCmdShow);
bool ProcessMessages();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Perform application initialization:
    if (!InitWindow(hInstance, nCmdShow))
    {
        return FALSE;
    }


    //TODO: This is just an example gameloop.
    bool running = true;

    while (running) {
        
        if (!ProcessMessages()) {
            running = false;
        }

        //DO RENDERING HERE

        Sleep(10);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEXW wndClass = {};
    wndClass.lpszClassName - CLASS_NAME;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO); // Set Icon
    wndClass.hCursor = LoadCursor(NULL, IDC_PIN); // Set cursor
    wndClass.lpfnWndProc = WndProc;

    RegisterClassExW(&wndClass);

    //  This creates a style for the window
    //  WS_CAPTION      Adds a name to the window.
    //  WS_MINIMIZEBOX  Adds a top bar with the minimize and close buttons
    //  WS_SYSMENU      Displays the minimize bar.
    DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

    //Creates window dimensions
    const int WIDTH = 640;
    const int HEIGHT = 480;
    RECT rect;
    rect.left = 250;
    rect.top = 250;
    rect.right = rect.left + WIDTH;
    rect.bottom = rect.top + HEIGHT;
    AdjustWindowRect(&rect, style, false);

    HWND hWnd = CreateWindowW(
        CLASS_NAME,
        L"TITLE",
        style,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hWnd, nCmdShow);

    return true;
}

bool ProcessMessages() {
    MSG message = {};

    while (PeekMessage(&message, nullptr, 0u, 0u, PM_REMOVE)) {
        
        if (message.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return true;
}

