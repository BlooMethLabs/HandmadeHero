#include "handmade.h"

static void RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            // Spreads grandient across whole window
            // uint8 Green = (( ((double)X) / Buffer.Width) * 256) + XOffset;
            // uint8 Blue = (( ((double)Y) / Buffer.Height) * 256) + YOffset;
            uint8 Green = X + BlueOffset;
            uint8 Blue = Y + GreenOffset;

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}

static void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    static real32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        // real32 t = 2.0f * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
        real32 SineValue = sinf(tSine);
        /*
        char SineString[256];
        sprintf(SineString, "%.5f\n", SineValue);
        OutputDebugStringA(SineString);
        */
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        // SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
        tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
}

static void GameUpdateAndRender(game_offscreen_buffer *Buffer,
                                int BlueOffset,
                                int GreenOffset,
                                game_sound_output_buffer *SoundBuffer,
                                int ToneHz)
{
    GameOutputSound(SoundBuffer, ToneHz);
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}