#include "skyengine.h"

// NOTE: DEBUG: this is testing code.
static void RenderWeirdGradient(GameBitmapBuffer* buffer, int xOffset, int yOffset) {
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


static void GameUpdateAndRender(GameBitmapBuffer* buffer) {
    int xOffset = 0;
    int yOffset = 0;
    RenderWeirdGradient(buffer, xOffset, yOffset);
}
