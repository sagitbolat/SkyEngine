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
    int16_t tone_volume;
    uint32_t running_sample_index;
    int     wave_period;
    int     bytes_per_sample;
    int     secondary_buffer_size;
    float   t_sine;
    int     latency_sample_count;
};



// SECTION: Constants
const wchar_t CLASS_NAME[] = L"SkyEngineWindowClass";
const int BUFFER_WIDTH = 1280;
const int BUFFER_HEIGHT = 720;



// SECTION: Globals
// TODO: Remove later.
LPDIRECTSOUNDBUFFER global_secondary_buffer;
static bool running;
static Win32BitmapBuffer global_back_buffer = { };



// SECTION: Forward function declarations
LRESULT CALLBACK            Win32WindowProc         (HWND hwnd, UINT windows_message, WPARAM wParam, LPARAM lParam);
static  Win32BitmapBuffer   Win32ResizeDIBSection   (Win32BitmapBuffer buffer, int width, int height);
static  void                Win32CopyBufferToWindow (HDC device_context, int window_width, int window_height, Win32BitmapBuffer buffer);
static  void                Win32LoadXInput         ();
static  void                Win32PollXInput         (Win32SoundOutput*);
static  void                Win32InitDirectSound    (HWND hwnd, int32_t samples_per_second, int32_t buffer_size);
static  void                WriteSineWave           (Win32SoundOutput* sound_output); 
static  void                Win32FillSoundBuffer    (Win32SoundOutput* sound_output, DWORD byte_to_lock, DWORD bytes_to_write);
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
    Win32SoundOutput sound_output = { };
    sound_output.samples_per_second = 48000; 
    sound_output.tone_hz = 256;
    sound_output.tone_volume = 500;
    sound_output.running_sample_index = 0;
    sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
    sound_output.bytes_per_sample = sizeof(int16_t) * 2;
    sound_output.secondary_buffer_size = sound_output.samples_per_second * sound_output.bytes_per_sample;
    sound_output.latency_sample_count = sound_output.samples_per_second / 15; // NOTE: our latency is 1/15th of a second.


    // NOTE: Loads DirectSound
    Win32InitDirectSound(hwnd, sound_output.samples_per_second, sound_output.secondary_buffer_size);
    Win32FillSoundBuffer(&sound_output, 0, sound_output.latency_sample_count * sound_output.bytes_per_sample);
    global_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

//    bool soundIsPlaying = false;
    
    // NOTE: Run the game loop.
    // TODO: Replace later with a better loop.
    running = true;
    int x_offset = 0;
    int y_offset = 0;
    
    // NOTE: This is for the Square wave sound.
    // TODO: Remove later
    uint32_t running_sample_index = 0;

    while (running) {
        
        // NOTE: Process Window messages 
        Win32ProcessMessageQueue();
        
        // NOTE: Poll Xinput events. Should this be done more frequently?
        // TODO: XInputGetState stalls if the controller is not plugged in so later on, 
        //       change it to only poll active controllers.
        Win32PollXInput(&sound_output);

        // TODO: This is for debugging purposes.
        // NOTE: Vibrates the first controller.
        XINPUT_VIBRATION vibration = { };
        vibration.wLeftMotorSpeed  = 3000;
        vibration.wRightMotorSpeed = 3000;
        XInputSetState(0, &vibration);

        // NOTE: Gamespecific
        GameBitmapBuffer buffer  = { };
        buffer.Memory = global_back_buffer.Memory;
        buffer.Width = global_back_buffer.Width;
        buffer.Height = global_back_buffer.Height;
        buffer.Pitch = global_back_buffer.Pitch;
        GameUpdateAndRender(&buffer);
        
        // NOTE: Directsound output sine wave
        // TODO: Delete this when playing actual game sound.
        WriteSineWave(&sound_output);

        // NOTE: Update window graphics. 
        Win32UpdateWindow(&hwnd);
    }
    return 0;
}



bool Win32InitWindow(HINSTANCE hInstance, int nCmdShow, HWND* hwnd) {
    global_back_buffer = Win32ResizeDIBSection(global_back_buffer, BUFFER_WIDTH, BUFFER_HEIGHT);

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
    HDC device_context = GetDC(*hwnd);
    Win32WindowDimensions window_dimensions = Win32GetWindowDimensions(*hwnd);
    Win32CopyBufferToWindow(device_context, window_dimensions.Width, window_dimensions.Height, global_back_buffer);
    ReleaseDC(*hwnd, device_context);
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

static void Win32PollXInput(Win32SoundOutput* sound_output) {
    for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; ++controller_index) {
        XINPUT_STATE controller_state = { };
        if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
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
            /*if (up || y_button) { ++y_offset; }
            if (down || a_button) { --y_offset; }
            if (left || x_button) { ++x_offset; }
            if (right || b_button) { --x_offset; }*/
            
            // DEBUG: change sound pitch on button press
            if (left_shoulder) {
                // NOTE: D#
                sound_output->tone_hz = 622;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
            if (right_shoulder) {
                // NOTE: E
                sound_output->tone_hz = 659;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
            if (x_button) {
                // NOTE: B
                sound_output->tone_hz = 494;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
            if (y_button) {
                // NOTE: D
                sound_output->tone_hz = 587;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
            if (b_button) {
                // NOTE: C
                sound_output->tone_hz = 523;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
            if (a_button) {
                // NOTE: A
                sound_output->tone_hz = 440;
                sound_output->wave_period = sound_output->samples_per_second / sound_output->tone_hz;
            }
        }
        else {
            // TODO: Handle unavailable controller if needed. Ex: tell user that controller is unplugged.
        }
    }
}



// TODO: change return type to bool to check for failures.
static void Win32InitDirectSound(HWND hwnd, int32_t samples_per_second, int32_t buffer_size) {
    // SECTION: Load the library
    
    HMODULE direct_sound_library = LoadLibrary(L"dsound.dll");
    
    if (!direct_sound_library) { 
        // NOTE: Direct Sound Library failed to load.
        // TODO: Diagnostic
        return;
    }


    // SECTION: Get a DirectSound Object
   
   // NOTE: Get a Direct Sound Object creation function pointer 
    direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(direct_sound_library, "DirectSoundCreate");        
   
    // NOTE: Direct sound object pointer 
    LPDIRECTSOUND direct_sound_object;
    if (!DirectSoundCreate) {
        // NOTE: DirectSoundCreate failed to initialize.
        // TODO: Diagnostic
        return;
    }
    if (!SUCCEEDED(DirectSoundCreate(0, &direct_sound_object, 0))) {    // TODO: Check if this boolean is correct.
        // NOTE: Direct Sound Object failed to create.
        // TODO: Diagnostic
        return;
    }
    if(!SUCCEEDED(direct_sound_object->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
        // NOTE: Failed to set format of audio.
        // TODO: Diagnostic
        return;
    }
    

    // SECTION: Initalize Wave Format for the audio.

    WAVEFORMATEX wave_format = { };
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = 2;
    wave_format.nSamplesPerSec = samples_per_second;
    wave_format.wBitsPerSample = 16;
    wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    wave_format.cbSize = 0;


    // SECTION: Create a primary buffer

    DSBUFFERDESC buffer_description = { };
    buffer_description.dwSize = sizeof(buffer_description);
    // TODO: DSBCAPS_GLOBALFOCUS ?
    buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

    LPDIRECTSOUNDBUFFER primary_buffer;

    if(!SUCCEEDED(direct_sound_object->CreateSoundBuffer(&buffer_description, &primary_buffer, 0))) {
        // NOTE: Failed to create primary buffer.
        // TODO: Diagnostic
        return;
    }
    
    if (!SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
        // NOTE: Failed to set format of the primary buffer.
        // TODO: Diagnostic 
        return;
    }
    

    // SECTION: Create a secondary buffer
    
    buffer_description = { };
    buffer_description.dwSize = sizeof(buffer_description);
    buffer_description.dwFlags = 0;
    buffer_description.dwBufferBytes = buffer_size;
    buffer_description.lpwfxFormat = &wave_format;  
    if(!SUCCEEDED(direct_sound_object->CreateSoundBuffer(&buffer_description, &global_secondary_buffer, 0))) {
        // NOTE: Failed to create secondary buffer.
        // TODO: Diagnostic.
        return;
    }
}




LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT windows_message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = { };
    switch (windows_message) {
        case WM_SYSKEYDOWN: {
            uint32_t vk_code = wParam;
            bool was_down = ((lParam & (1 << 30)) != 0);     
            bool is_down  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.

            //NOTE: Makes the window close if alt f4 is pressed.
            //NOTE: bool altKeyWasDown = ((lParam & (1 << 29)) != 0);
            if (vk_code == VK_F4){
                running = false; // TODO: Make the game pop up with an "Are you sure you want to Quit?" message
            }
        } break;
        case WM_SYSKEYUP: {
            uint32_t vk_code = wParam;
            bool was_down = ((lParam & (1 << 30)) != 0);     
            bool is_down  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        case WM_KEYDOWN: {
            uint32_t vk_code = wParam;
            bool was_down = ((lParam & (1 << 30)) != 0);     
            bool is_down  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        case WM_KEYUP: {
            uint32_t vk_code = wParam;
            bool was_down = ((lParam & (1 << 30)) != 0);     
            bool is_down  = ((lParam & (1 << 30)) == 0); 
            // TODO: Pass the info on key event to the game.
        } break;
        
        case WM_SIZE: {
            
        } break;

        case WM_DESTROY: {
            // TODO: Handle as an error if running is true and recreate window.
            running = false;
            OutputDebugStringA("DESTROY\n");
            return 0;
        } break;

        case WM_CLOSE: {

            // TODO: Give the user an "Are you sure you want to quit?" message.
            running = false;

            OutputDebugStringA("CLOSE\n");
        } break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("ACTIVATEAPP\n");
        } break;

        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC device_context = BeginPaint(hwnd, &Paint);
           
            int x       = Paint.rcPaint.left;
            int y       = Paint.rcPaint.top;
            int width   = Paint.rcPaint.right - Paint.rcPaint.left;
            int height  = Paint.rcPaint.bottom - Paint.rcPaint.top;
           
            Win32WindowDimensions window_dimensions = Win32GetWindowDimensions(hwnd);
            Win32CopyBufferToWindow(device_context, window_dimensions.Width, window_dimensions.Height, global_back_buffer);

            EndPaint(hwnd, &Paint);
        } break;

        default: {
            result = DefWindowProc(hwnd, windows_message, wParam, lParam);
        } break;
    }
    return result;
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
    int bitmap_memory_size = (width * height) * bytes_per_pixel;
    buffer.Memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Clear screen to black?
    
    buffer.Pitch  = buffer.Width * bytes_per_pixel;
    return (buffer);
}

static void Win32CopyBufferToWindow(HDC device_context, int window_width, int window_height, Win32BitmapBuffer buffer) {
    // TODO: Correct the aspect ratio

    StretchDIBits(
        device_context,
        0, 0, window_width, window_height,    //NOTE: This is the buffer we are drawing to.
        0, 0, buffer.Width, buffer.Height,  //NOTE: This is the buffer we are dwaring from
        buffer.Memory,
        &buffer.Info,
        DIB_RGB_COLORS,             // NOTE: Specifies the type of bitmap, in this case RGB color. Can also be set to DIB_PAL_COLORS.
        SRCCOPY                     // NOTE: A bit-wise operation that specifies that we are copying from one bitmap to another. 
    );
}



static void Win32FillSoundBuffer(Win32SoundOutput* sound_output, DWORD byte_to_lock, DWORD bytes_to_write) {
    // NOTE: The two buffer regions.
    VOID* region1;
    DWORD region1_size;
    VOID* region2;
    DWORD region2_size;
    
    if ((global_secondary_buffer->Lock(
            byte_to_lock,
            bytes_to_write,
            &region1, &region1_size,
            &region2, &region2_size,
            0)) != DS_OK) {
        // NOTE: Locking buffer did not succeed.
        // TODO: Diagnostic
        // OutputDebugString(L"DEBUG: Locking buffer did not succeed. \n");
    } 
    
    // TODO: assert that region sizes are valid.
    
    DWORD region1_sample_count = region1_size / sound_output->bytes_per_sample;
    int16_t* sample_out = (int16_t*)region1;
    for(DWORD sample_index = 0; sample_index < region1_sample_count; ++sample_index) {
        float sine_value = sinf(sound_output->t_sine);
        int16_t sample_value = (int16_t)(sine_value * sound_output->tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        sound_output->t_sine += 2.0f * PI32 * (float)1.0f / (float)sound_output->wave_period;
        
        ++(sound_output->running_sample_index);
    }

    DWORD region2_sample_count = region2_size / sound_output->bytes_per_sample;
    sample_out = (int16_t*)region2;
    for(DWORD sample_index = 0; sample_index < region2_sample_count; ++sample_index) {
        float sine_value = sinf(sound_output->t_sine);
        int16_t sample_value = (int16_t)(sine_value * sound_output->tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        sound_output->t_sine += 2.0f * PI32 * (float)1.0f / (float)sound_output->wave_period;
        
        ++(sound_output->running_sample_index);
    }

    if (!SUCCEEDED(global_secondary_buffer->Unlock(region1, region1_size, region2, region2_size))) {
        // NOTE: Unlocking buffer did not succeed.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Unlocking buffer did not succeed \n");
    }
}



// NOTE: This is testing code.
static void WriteSineWave(Win32SoundOutput* sound_output) {

    // NOTE: Direct Sound output tests.

    DWORD play_cursor;
    DWORD write_cursor;

    if (!SUCCEEDED(global_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor))) {
        // NOTE: Current buffer did not exist.
        // TODO: Diagnostic
        OutputDebugString(L"DEBUG: Current buffer did not exist \n");
    }
    
    DWORD byte_to_lock = (sound_output->running_sample_index * sound_output->bytes_per_sample) % sound_output->secondary_buffer_size;
    
    DWORD target_cursor = (play_cursor + (sound_output->latency_sample_count * sound_output->bytes_per_sample)) % sound_output->secondary_buffer_size;
    DWORD bytes_to_write;

    // TODO: We should probably not write the length of the whole buffer in order to lower the latency.
    if (byte_to_lock > target_cursor) {
        bytes_to_write = (sound_output->secondary_buffer_size - byte_to_lock);        
        bytes_to_write += target_cursor;
    } else {
        bytes_to_write = target_cursor - byte_to_lock;
    }
    Win32FillSoundBuffer(sound_output, byte_to_lock, bytes_to_write);
}
