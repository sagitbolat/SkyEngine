/* TODO: THIS IS NOT THE FINAL PLATFORM LAYER.
   * > To-do list:
     * >> Saved game locations
     * >> Getting a handle to our own executable file
     * >> Asset loading path
     * >> Threading (launch a thread)
     * >> Raw Input (support for multi-keyboard input)
     * >> Sleep/timeBeginPeriod
     * >> ClipCursor() (for multi-monitor support)
     * >> Fullscreen support
     * >> WM_SETCURSOR (control cursor visibility)
     * >> QueryCancelAutoplay
     * >> WM_ACTIVATEAPP (for when we are not the active app)
     * >> Blit speed improvements (BitBlt)
     * >> Hardware acceleration (OpenGL or Direct3D)
     * >> GetKeyboardLayout (international keyboard like polish or french layout)
     * >>>> OTHER
 */

// SECTION: Includes and global defines.
#include <windows.h>
#include <stdint.h>

#include <xinput.h>
#include <dsound.h>

// TODO: Remove this at some point and implement trig functions myself.
#include <math.h>

#define PI32 3.14159265
#include "skyengine.cpp"


// SECTION: param structs.

// NOTE: RECT operations
struct Win32WindowDimensions {
    int Width;
    int Height;
};
static Win32WindowDimensions Win32GetWindowDimensions(HWND hwnd) {
    RECT window_rect = { };
    GetClientRect(hwnd, &window_rect);
    int width = window_rect.right - window_rect.left;
    int height = window_rect.bottom - window_rect.top;
    return(Win32WindowDimensions{ width, height });
}

// NOTE: Windows bitmap buffer.
struct Win32BitmapBuffer {
    BITMAPINFO  Info;
    void*       Memory;
    int         Width;
    int         Height;
    int         Pitch;
};

// NOTE: SoundParamStruct
struct Win32SoundOutput {
    int     samples_per_second;
    int     tone_hz;
    int16_t toneVolume;
    uint32_t runningSampleIndex;
    int     wavePeriod;
    int     bytesPerSample;
    int     secondaryBufferSize;
    float   t_sine;
    int     latency_sample_count;
};



// SECTION: Constants
const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";
const int BUFFER_WIDTH = 1280;
const int BUFFER_HEIGHT = 720;



// SECTION: Globals
// TODO: Remove later.
LPDIRECTSOUNDBUFFER globalSecondaryBuffer;
static bool Running;
static Win32BitmapBuffer globalBackbuffer = { };



// SECTION: Forward function declarations
LRESULT CALLBACK            Win32WindowProc         (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static  Win32BitmapBuffer   Win32ResizeDIBSection   (Win32BitmapBuffer buffer, int width, int height);
static  void                Win32CopyBufferToWindow (HDC DeviceContext, int windowWidth, int windowHeight, Win32BitmapBuffer buffer);
static  void                Win32LoadXInput         ();
static  void                Win32PollXInput         (Win32SoundOutput*);
static  void                Win32InitDirectSound    (HWND hwnd, int32_t samples_per_second, int32_t bufferSize);
static  void                WriteSineWave           (Win32SoundOutput* soundOutput); 
static  void                Win32FillSoundBuffer    (Win32SoundOutput* soundOutput, DWORD byteToLock, DWORD bytesToWrite);
static  bool                Win32InitWindow         (HINSTANCE hInstance, int nCmdShow, HWND* hwnd);
static  void                Win32ProcessMessageQueue();
static  void                Win32UpdateWindow       (HWND* hwnd);



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



int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    HWND hwnd = { };

    // NOTE: Inits the window
    Win32InitWindow(hInstance, nCmdShow, &hwnd);
    
    // NOTE: Loads XInput
    Win32LoadXInput();
    
    // NOTE: Loads sound buffer output defaults
    Win32SoundOutput soundOutput = { };
    soundOutput.samples_per_second = 48000; 
    soundOutput.tone_hz = 256;
    soundOutput.toneVolume = 500;
    soundOutput.runningSampleIndex = 0;
    soundOutput.wavePeriod = soundOutput.samples_per_second / soundOutput.tone_hz;
    soundOutput.bytesPerSample = sizeof(int16_t) * 2;
    soundOutput.secondaryBufferSize = soundOutput.samples_per_second * soundOutput.bytesPerSample;
    soundOutput.latency_sample_count = soundOutput.samples_per_second / 15; // NOTE: our latency is 1/15th of a second.


    // NOTE: Loads DirectSound
    Win32InitDirectSound(hwnd, soundOutput.samples_per_second, soundOutput.secondaryBufferSize);
    Win32FillSoundBuffer(&soundOutput, 0, soundOutput.latency_sample_count * soundOutput.bytesPerSample);
    globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

//    bool soundIsPlaying = false;
    
    // NOTE: Run the game loop.
    // TODO: Replace later with a better loop.
    Running = true;
    int xOffset = 0;
    int yOffset = 0;
    
    // NOTE: This is for the Square wave sound.
    // TODO: Remove later
    uint32_t runningSampleIndex = 0;

    while (Running) {
        
        // NOTE: Process Window messages 
        Win32ProcessMessageQueue();
        
        // NOTE: Poll Xinput events. Should this be done more frequently?
        // TODO: XInputGetState stalls if the controller is not plugged in so later on, 
        //       change it to only poll active controllers.
        Win32PollXInput(&soundOutput);

        // TODO: This is for debugging purposes.
        // NOTE: Vibrates the first controller.
        XINPUT_VIBRATION vibration = { };
        vibration.wLeftMotorSpeed  = 3000;
        vibration.wRightMotorSpeed = 3000;
        XInputSetState(0, &vibration);

        // NOTE: Gamespecific
        GameBitmapBuffer buffer  = { };
        buffer.Memory = globalBackbuffer.Memory;
        buffer.Width = globalBackbuffer.Width;
        buffer.Height = globalBackbuffer.Height;
        buffer.Pitch = globalBackbuffer.Pitch;
        GameUpdateAndRender(&buffer);
        
        // NOTE: Directsound output sine wave
        // TODO: Delete this when playing actual game sound.
        WriteSineWave(&soundOutput);

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

static void Win32PollXInput(Win32SoundOutput* soundOutput) {
    for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
        XINPUT_STATE controller_state = { };
        if (XInputGetState(controllerIndex, &controller_state) == ERROR_SUCCESS) {
            // NOTE: This controller is plugged in.
            // TODO: See if controller_state.dwPacketNumber increments too fast.

            // NOTE: get the gamepad struct from controller_state
            XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

            // NOTE: Get inputs.
            bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool a_button = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool b_button = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool x_button = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);

            int16_t stick_left_x = pad->sThumbLX;
            int16_t stick_left_y = pad->sThumbLY;

            // TODO: This is only for debugging purposes.
            /*if (up || y_button) { ++yOffset; }
            if (down || a_button) { --yOffset; }
            if (left || x_button) { ++xOffset; }
            if (right || b_button) { --xOffset; }*/
            
            // DEBUG: change sound pitch on button press
            if (left_shoulder) {
                // NOTE: D#
                soundOutput->tone_hz = 622;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
            if (right_shoulder) {
                // NOTE: E
                soundOutput->tone_hz = 659;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
            if (x_button) {
                // NOTE: B
                soundOutput->tone_hz = 494;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
            if (y_button) {
                // NOTE: D
                soundOutput->tone_hz = 587;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
            if (b_button) {
                // NOTE: C
                soundOutput->tone_hz = 523;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
            if (a_button) {
                // NOTE: A
                soundOutput->tone_hz = 440;
                soundOutput->wavePeriod = soundOutput->samples_per_second / soundOutput->tone_hz;
            }
        }
        else {
            // TODO: Handle unavailable controller if needed. Ex: tell user that controller is unplugged.
        }
    }
}



// TODO: change return type to bool to check for failures.
static void Win32InitDirectSound(HWND hwnd, int32_t samples_per_second, int32_t bufferSize) {
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
    waveFormat.nSamplesPerSec = samples_per_second;
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
    
    const int bytes_per_pixel = 4;
    int bitmapMemorySize = (width * height) * bytes_per_pixel;
    buffer.Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Clear screen to black?
    
    buffer.Pitch  = buffer.Width * bytes_per_pixel;
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



static void Win32FillSoundBuffer(Win32SoundOutput* soundOutput, DWORD byteToLock, DWORD bytesToWrite) {
    // NOTE: The two buffer regions.
    VOID* region1;
    DWORD region1Size;
    VOID* region2;
    DWORD region2Size;
    
    if ((globalSecondaryBuffer->Lock(
            byteToLock,
            bytesToWrite,
            &region1, &region1Size,
            &region2, &region2Size,
            0)) != DS_OK) {
        // NOTE: Locking buffer did not succeed.
        // TODO: Diagnostic
        // OutputDebugString(L"DEBUG: Locking buffer did not succeed. \n");
    } 
    
    // TODO: assert that region sizes are valid.
    
    DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
    int16_t* sampleOut = (int16_t*)region1;
    for(DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
        float sineValue = sinf(soundOutput->t_sine);
        int16_t sampleValue = (int16_t)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        soundOutput->t_sine += 2.0f * PI32 * (float)1.0f / (float)soundOutput->wavePeriod;
        
        ++(soundOutput->runningSampleIndex);
    }

    DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
    sampleOut = (int16_t*)region2;
    for(DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
        float sineValue = sinf(soundOutput->t_sine);
        int16_t sampleValue = (int16_t)(sineValue * soundOutput->toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
        soundOutput->t_sine += 2.0f * PI32 * (float)1.0f / (float)soundOutput->wavePeriod;
        
        ++(soundOutput->runningSampleIndex);
    }

    if (!SUCCEEDED(globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size))) {
        // NOTE: Unlocking buffer did not succeed.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Unlocking buffer did not succeed \n");
    }
}



// NOTE: This is testing code.
static void WriteSineWave(Win32SoundOutput* soundOutput) {

    // NOTE: Direct Sound output tests.

    int halfSquareWavePeriod = soundOutput->wavePeriod / 2;

    DWORD playCursor;
    DWORD writeCursor;

    if (!SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
        // NOTE: Current buffer did not exist.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Current buffer did not exist \n");
    }
    
    DWORD byteToLock = (soundOutput->runningSampleIndex * soundOutput->bytesPerSample) % soundOutput->secondaryBufferSize;
    
    DWORD targetCursor = (playCursor + (soundOutput->latency_sample_count * soundOutput->bytesPerSample)) % soundOutput->secondaryBufferSize;
    DWORD bytesToWrite;

    // TODO: We should probably not write the length of the whole buffer in order to lower the latency.
    if (byteToLock > targetCursor) {
        bytesToWrite = (soundOutput->secondaryBufferSize - byteToLock);        
        bytesToWrite += targetCursor;
    } else {
        bytesToWrite = targetCursor - byteToLock;
    }
    Win32FillSoundBuffer(soundOutput, byteToLock, bytesToWrite);
}
