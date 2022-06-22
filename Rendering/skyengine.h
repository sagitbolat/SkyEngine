#if !defined(SKYENGINE_H)

#include <stdint.h>


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

static void GameUpdateAndRender(GameBitmapBuffer* buffer);


#define SKYENGINE_H
#endif
