#include "skyengine.h"

// NOTE: DEBUG: this is testing code.
static void RenderWeirdGradient(GameBitmapBuffer* buffer, int x_offset, int y_offset) {
    // NOTE: Drawing Logic.

    uint8_t* row = (uint8_t*)buffer->Memory;

    for (int y = 0; y < buffer->Height; ++y) {

        uint32_t* pixel = (uint32_t*)row;

        for (int x = 0; x < buffer->Width; ++x) {

            uint8_t blue = (uint8_t)(x + x_offset);       // NOTE: Blue channel
            uint8_t green = (uint8_t)(0);                 // NOTE: Green channel
            uint8_t red = (uint8_t)(y + y_offset);       // NOTE: Red channel


            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer->Pitch;
    }
}


static void GameUpdateAndRender(GameBitmapBuffer* buffer) {
    int x_offset = 0;
    int y_offset = 0;
    RenderWeirdGradient(buffer, x_offset, y_offset);
}
