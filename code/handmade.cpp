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
            uint8 Green = (uint8)(X + BlueOffset);
            uint8 Blue = (uint8)(Y + GreenOffset);

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

static void GameUpdateAndRender(game_memory *Memory,
                                game_input *Input,
                                game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        char *Filename = __FILE__;

        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if (File.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }

        GameState->ToneHz = 256;

        Memory->IsInitialized = true;
    }

    game_controller_input *Input0 = &Input->Controllers[0];
    if (Input0->IsAnalog)
    {
        GameState->BlueOffset += (int)(4.0f * (Input0->EndX));
        GameState->GreenOffset += (int)(4.0f * (Input0->EndY));
        GameState->ToneHz = 256 + (int)(128.0f * (Input0->EndY));

    }
    else
    {

    }

    if (Input0->Down.EndedDown)
    {
        GameState->GreenOffset += 1;
    }
    GameOutputSound(SoundBuffer, GameState->ToneHz);
    RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
}