


#include "SkyWin32.h"
#include <stdint.h>

#include <xinput.h>
#include <dsound.h>


// NOTE: RECT operations
struct Win32WindowDimensions {
    int Width;
    int Height;
};
static Win32WindowDimensions Win32GetWindowDimensions(HWND hwnd) {
    RECT windowRect = { };
    GetClientRect(hwnd, &windowRect);
    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;
    return(Win32WindowDimensions{ width, height });
}

// SECTION: Constants
const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";

// SECTION: Globals
LPDIRECTSOUNDBUFFER globalSecondaryBuffer;

// SECTION: Forward function declarations
LRESULT CALLBACK            Win32WindowProc         (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static  Win32BitmapBuffer   Win32ResizeDIBSection   (Win32BitmapBuffer buffer, int width, int height);
static  void                Win32CopyBufferToWindow (HDC DeviceContext, int windowWidth, int windowHeight, Win32BitmapBuffer buffer);
static  void                RenderWeirdGradient     (Win32BitmapBuffer* buffer, int xOffset, int yOffset);
static  void                Win32LoadXInput         ();
static  void                Win32InitDirectSound    (HWND hwnd, int32_t samplesPerSecond, int32_t bufferSize);
static  void                WriteSquareWave         (int runningSampleIndex, int squareWavePeriod, int bytesPerSample, int secondaryBufferSize, int volume);                

// SECTION: XInput library function declaration macros
// NOTE: XInputGetState 
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
    return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_
//NOTE: XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// SECTION: DirectSound library function declration macros
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);



// TODO: the wWinMain is temporary. Move to different file
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    HWND hwnd = { };

    // NOTE: Inits the window
    Win32InitWindow(hInstance, nCmdShow, &hwnd);
    
    // NOTE: Loads XInput
    Win32LoadXInput();
    
    // NOTE: Loads DirectSound
    int samplesPerSecond = 48000;
    int bytesPerSample = sizeof(int16_t) * 2;
    int secondaryBufferSize = samplesPerSecond * bytesPerSample;
    Win32InitDirectSound(hwnd, samplesPerSecond, secondaryBufferSize);
    bool soundIsPlaying = false;

    // NOTE: Run the game loop.
    // TODO: Replace later with a better loop.
    Running = true;
    int xOffset = 0;
    int yOffset = 0;
    
    // NOTE: This is for the Square wave sound.
    // TODO: Remove later
    uint32_t runningSampleIndex = 0;
    int hz = 256;
    int volume = 100;
    int squareWavePeriod = samplesPerSecond / hz;


    while (Running) {
        // NOTE: Process Window messages 
        Win32ProcessMessageQueue();
        
        
        // TODO: Move this out of gameloop into its own function?
        // NOTE: Poll Xinput events. Should this be done more frequently?
            // TODO: XInputGetState stalls if the controller is not plugged in so later on, change it to only poll active controllers.
        for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
            XINPUT_STATE controller_state = { };
            if(XInputGetState(controllerIndex, &controller_state) == ERROR_SUCCESS) {
                // NOTE: This controller is plugged in.
                // TODO: See if controller_state.dwPacketNumber increments too fast.

                XINPUT_GAMEPAD* pad = &controller_state.Gamepad;
                
                bool up             = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                bool down           = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                bool left           = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                bool right          = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                bool start          = (pad->wButtons & XINPUT_GAMEPAD_START);
                bool back           = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                bool left_shoulder  = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                bool a_button       = (pad->wButtons & XINPUT_GAMEPAD_A);
                bool b_button       = (pad->wButtons & XINPUT_GAMEPAD_B);
                bool x_button       = (pad->wButtons & XINPUT_GAMEPAD_X);
                bool y_button       = (pad->wButtons & XINPUT_GAMEPAD_Y);
                
                int16_t stick_left_x = pad->sThumbLX;
                int16_t stick_left_y = pad->sThumbLY;
                
                // TODO: This is only for debugging purposes.
                if (up    || y_button) { ++yOffset; }
                if (down  || a_button) { --yOffset; }
                if (left  || x_button) { ++xOffset; }
                if (right || b_button) { --xOffset; }
            } else {
                // TODO: Handle unavailable controller if needed. Ex: tell user that controller is unplugged.
            } 
            // TODO: This is for debugging purposes.
            // NOTE: Vibrates the first controller.
            XINPUT_VIBRATION vibration = { };
            // CODE: vibration.wLeftMotorSpeed  = 60000;
            // CODE: vibration.wRightMotorSpeed = 60000;
            XInputSetState(0, &vibration);
        }

        // NOTE: Draw gradient
        // TODO: Delete this when rendering an actual game.
        RenderWeirdGradient(&globalBackbuffer, xOffset, yOffset);
        
        // NOTE: Directsound output square wave
        // TODO: Delete this when playing actual game sound.
        WriteSquareWave(runningSampleIndex, squareWavePeriod, bytesPerSample, secondaryBufferSize, volume);

        // NOTE: if sound isn't playing, start playing.
        if (!soundIsPlaying) {
            globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            soundIsPlaying = true;
        }

        // NOTE: Update window graphics. 
        Win32UpdateWindow(&hwnd);
    }
    return 0;
}



bool Win32InitWindow(HINSTANCE hInstance, int nCmdShow, HWND* hwnd) {
    globalBackbuffer = Win32ResizeDIBSection(globalBackbuffer, BUFFER_WIDTH, BUFFER_HEIGHT);

    WNDCLASS wc = { };

    wc.lpfnWndProc = Win32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;      // TODO: THIS MIGHT BE NOT NECESSARY
    //wc.hIcon = TODO: Set the icon


    // NOTE: Registers window class.
    if (!RegisterClass(&wc)) return(0);      // TODO:  Log Windowclass register failure.

    // NOTE: Creates the window.
    *hwnd = CreateWindowEx(
        0,              // NOTE: Optional window styles.
        CLASS_NAME,     // NOTE: Window class
        L"SkyApp",      // NOTE: Window text

        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // NOTE: Window style

        CW_USEDEFAULT, CW_USEDEFAULT,       // NOTE: Position 
        CW_USEDEFAULT, CW_USEDEFAULT,      // NOTE: Size

        NULL,           // NOTE: Parent window    
        NULL,           // NOTE: Menu
        hInstance,      // NOTE: Instance handle
        NULL            // NOTE: Additional application data
    );

    if (*hwnd == NULL) return(0);     // TODO: Log windowhandler not defined.

    ShowWindow(*hwnd, nCmdShow);

    return true;
}

void Win32ProcessMessageQueue() {
    // NOTE: Processes Message Queue
    MSG message = { };
    while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void Win32UpdateWindow(HWND *hwnd) {
    HDC DeviceContext = GetDC(*hwnd);
    Win32WindowDimensions windowDimensions = Win32GetWindowDimensions(*hwnd);
    Win32CopyBufferToWindow(DeviceContext, windowDimensions.Width, windowDimensions.Height, globalBackbuffer);
    ReleaseDC(*hwnd, DeviceContext);
}



static void Win32LoadXInput() {
    // NOTE: Load XInput 1.4
    HMODULE XInputLibrary = LoadLibrary(L"xinput1_4.dll");

    // NOTE: if XInput 1.4 unavailable, load XInput 1.3 instead.
    if (!XInputLibrary) { XInputLibrary = LoadLibrary(L"xinput1_3.dll"); }
    
    // NOTE: Set the GetState and SetState function pointers to point at the library functions.
    if (XInputLibrary) {
        XInputGetState = (x_input_get_state*) GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state*) GetProcAddress(XInputLibrary, "XInputSetState");
    } 
}



// TODO: change return type to bool to check for failures.
static void Win32InitDirectSound(HWND hwnd, int32_t samplesPerSecond, int32_t bufferSize) {
    // SECTION: Load the library
    
    HMODULE directSoundLibrary = LoadLibrary(L"dsound.dll");
    
    if (!directSoundLibrary) { 
        // NOTE: Direct Sound Library failed to load.
        // TODO: Diagnostic
        return;
    }


    // SECTION: Get a DirectSound Object
    
    direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(directSoundLibrary, "DirectSoundCreate");        
   
    // NOTE: Direct sound object pointer 
    LPDIRECTSOUND directSoundObj;
    if (!DirectSoundCreate) {
        // NOTE: DirectSoundCreate failed to initialize.
        // TODO: Diagnostic
        return;
    }
    if (!SUCCEEDED(DirectSoundCreate(0, &directSoundObj, 0))) {    // TODO: Check if this boolean is correct.
        // NOTE: Direct Sound Object failed to create.
        // TODO: Diagnostic
        return;
    }
    if(!SUCCEEDED(directSoundObj->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
        // NOTE: Failed to set format of audio.
        // TODO: Diagnostic
        return;
    }
    

    // SECTION: Initalize Wave Format for the audio.

    WAVEFORMATEX waveFormat = { };
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = samplesPerSecond;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;


    // SECTION: Create a primary buffer

    DSBUFFERDESC bufferDescription = { };
    bufferDescription.dwSize = sizeof(bufferDescription);
    // TODO: DSBCAPS_GLOBALFOCUS ?
    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

    LPDIRECTSOUNDBUFFER primaryBuffer;

    if(!SUCCEEDED(directSoundObj->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0))) {
        // NOTE: Failed to create primary buffer.
        // TODO: Diagnostic
        return;
    }
    
    if (!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
        // NOTE: Failed to set format of the primary buffer.
        // TODO: Diagnostic 
        return;
    }
    

    // SECTION: Create a secondary buffer
    
    bufferDescription = { };
    bufferDescription.dwSize = sizeof(bufferDescription);
    bufferDescription.dwFlags = 0;
    bufferDescription.dwBufferBytes = bufferSize;
    bufferDescription.lpwfxFormat = &waveFormat;  
    if(!SUCCEEDED(directSoundObj->CreateSoundBuffer(&bufferDescription, &globalSecondaryBuffer, 0))) {
        // NOTE: Failed to create secondary buffer.
        // TODO: Diagnostic.
        return;
    }
}




LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT Result = { };
    switch (uMsg) {
        case WM_SYSKEYDOWN: {
            uint32_t VKCode = wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);     
            bool isDown  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.

            //NOTE: Makes the window close if alt f4 is pressed.
            //NOTE: bool altKeyWasDown = ((lParam & (1 << 29)) != 0);
            if (VKCode == VK_F4){
                Running = false; // TODO: Make the game pop up with an "Are you sure you want to Quit?" message
            }
        } break;
        case WM_SYSKEYUP: {
            uint32_t VKCode = wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);     
            bool isDown  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        case WM_KEYDOWN: {
            uint32_t VKCode = wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);     
            bool isDown  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        case WM_KEYUP: {
            uint32_t VKCode = wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);     
            bool isDown  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        
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
        0, 0, windowWidth, windowHeight,    //NOTE: This is the buffer we are drawing to.
        0, 0, buffer.Width, buffer.Height,  //NOTE: This is the buffer we are dwaring from
        buffer.Memory,
        &buffer.Info,
        DIB_RGB_COLORS,             // NOTE: Specifies the type of bitmap, in this case RGB color. Can also be set to DIB_PAL_COLORS.
        SRCCOPY                     // NOTE: A bit-wise operation that specifies that we are copying from one bitmap to another. 
    );
}



static void RenderWeirdGradient(Win32BitmapBuffer* buffer, int xOffset, int yOffset) {
    // NOTE: Drawing Logic.

    uint8_t* row = (uint8_t*)buffer->Memory;

    for (int y = 0; y < buffer->Height; ++y) {

        uint32_t* pixel = (uint32_t*)row;

        for (int x = 0; x < buffer->Width; ++x) {

            uint8_t blue = (uint8_t)(x + xOffset);       // NOTE: Blue channel
            uint8_t green = (uint8_t)(0);                 // NOTE: Green channel
            uint8_t red = (uint8_t)(y + yOffset);       // NOTE: Red channel


            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer->Pitch;
    }
}



static void WriteSquareWave(int runningSampleIndex, int squareWavePeriod, int bytesPerSample, int secondaryBufferSize, int volume) {
   
    // NOTE: Direct Sound output tests.
    
    int halfSquareWavePeriod = squareWavePeriod / 2;

    DWORD playCursor;
    DWORD writeCursor;

    if (!SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
        // NOTE: Current buffer did not exist.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Current buffer did not exist \n");
    }
    
    DWORD byteToLock = runningSampleIndex * bytesPerSample % secondaryBufferSize;
    DWORD bytesToWrite; 
    if (byteToLock == playCursor) {
        bytesToWrite = secondaryBufferSize;
    } else if (byteToLock > playCursor) {
        bytesToWrite = (secondaryBufferSize - byteToLock);        
        bytesToWrite += playCursor;
    } else {
        bytesToWrite = playCursor - byteToLock;
    }

    // NOTE: The two buffer regions.
    VOID* region1;
    DWORD region1Size;
    VOID* region2;
    DWORD region2Size;
    
    if (!SUCCEEDED(globalSecondaryBuffer->Lock(
            byteToLock,
            bytesToWrite,
            &region1, &region1Size,
            &region2, &region2Size,
            0))) {
        // NOTE: Locking buffer did not succeed.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Locking buffer did not succeed. \n");
    } 
    
    // TODO: assert that region sizes are valid.
    
    DWORD region1SampleCount = region1Size/bytesPerSample;
    int16_t* sampleOut = (int16_t*)region1;
    for(DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
        int16_t sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? volume : -volume;
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
    }

    DWORD region2SampleCount = region2Size/bytesPerSample;
    sampleOut = (int16_t*)region2;
    for(DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
        int16_t sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? volume : -volume;
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
    }

    if (!SUCCEEDED(globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size))) {
        // NOTE: Unlocking buffer did not succeed.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Unlocking buffer did not succeed \n");
    }
}
