#include "skyengine.h"
// TODO: Remove this at some point and implement trig functions myself.
#include <math.h>

#define PI32 3.14159265f
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



// NOTE: DEBUG: this is testing code
static void GameOutputSound(GameSoundBuffer* sound_buffer) {
    static float t_sine;
    int16_t tone_volume = 1000;
    int tone_hz = 256;

    int wave_period = sound_buffer->samples_per_second/tone_hz;

    int16_t* sample_out = sound_buffer->samples;
    for(int sample_index = 0; sample_index < sound_buffer->sample_count; ++sample_index) {
        float sine_value = sinf(t_sine);
        int16_t sample_value = (int16_t)(sine_value * tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        t_sine += 2.0f * PI32 * 1.0f / (float)wave_period;
    }

}


static void GameUpdateAndRender(GameBitmapBuffer* graphics_buffer, GameSoundBuffer* sound_buffer) {
    int x_offset = 0;
    int y_offset = 0;
    // TODO: Allow sample offsets here for more robust platform options.
    GameOutputSound(sound_buffer);
    RenderWeirdGradient(graphics_buffer, x_offset, y_offset);
}
