#if !defined(SKYENGINE_H)

#include <stdint.h>

#if DEBUG
#define Assert(expression) if(!(expression)) {*(int*)0 = 0;}
#else
#define Assert(expression)
#endif


#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))


// TODO: Services that the platform layer provides to the game:


// NOTE: Services that the game provides to the platform layer:
    // TODO: Maybe seperate sound on another thread and update async.

// NOTE: We meed timing, controller/keyboard input, bitmap buffer for graphics, sound buffer.


// NOTE: Platform independant bitmap buffer.
struct GameBitmapBuffer {
    void*       Memory;
    int         Width;
    int         Height;
    int         Pitch;
};

// NOTE: Platform independant sound buffer.
struct GameSoundBuffer {
    int samples_per_second;
    int sample_count;
    int16_t* samples;

};

// SECTION: Platform independant game input

// NOTE: Individual button state
struct GameButtonState {
    int half_transition_count;
    bool ended_down;
};
// NOTE: Individual controller data
struct GameControllerInput {
    
    bool is_analog;

    float start_x;
    float start_y;

    float min_x;
    float min_y;
    
    float max_x;
    float max_y;

    float end_x;
    float end_y;
    
    union {
        GameButtonState button_states[6];
        struct {
            GameButtonState up;
            GameButtonState down;
            GameButtonState left;
            GameButtonState right;
            GameButtonState left_shoulder;
            GameButtonState right_shoulder;
        };
        struct {
            GameButtonState y_button;
            GameButtonState a_button;
            GameButtonState x_button;
            GameButtonState b_button;
            GameButtonState left_shoulder;
            GameButtonState right_shoulder;
        };
    };
};
// NOTE: input data for 4 controllers
struct GameInput {
    GameControllerInput controllers[4];
};


// SECTION: Memory. This is the persistant memory that
// has to be passed from the platform layer to the game
// or vice versa.
struct GameMemory {
    bool is_initialized;
    uint64_t permanent_storage_size;
    void* permanent_storage;

    uint64_t transient_storage_size;
    void* transient_storage;
};

// NOTE: The game state representation.
struct GameState {
    int tone_hz;
    int x_offset;
    int y_offset;
};



static void GameUpdateAndRender(GameMemory* memory, GameBitmapBuffer* graphics_buffer, GameSoundBuffer* sound_buffer, GameInput* input);


#define SKYENGINE_H
#endif
