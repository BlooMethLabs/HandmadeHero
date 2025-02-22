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

static void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}