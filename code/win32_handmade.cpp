#include <math.h>
#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// Syntax errors for these:
typedef float real32;
typedef double real64;

// #include "handmade.h"
#include "handmade.cpp"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

// TODO: this is a global for now.

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// Alias functions with stubs
// XInputGetState stub
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state_func_type);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state_func_type *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState stub
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state_func_type);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state_func_type *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// DirectSoundCreate
#define DIRECT_SOUND_CREATE(name) \
    HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

void * PlatformLoadFile(char *FileName)
{
    // NOTE(casey): Implements the Win32 file loading
    return(0);
}

internal void Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("XInput1_4.dll");
    if (!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("XInput9_1_0.dll");
    }
    if (!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("XInput1_3.dll");
    }
    // TODO: log lib loaded
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state_func_type *)GetProcAddress(XInputLibrary, "XInputGetState");
        if (!XInputGetState)
        {
            XInputGetState = XInputGetStateStub;
        }
        XInputSetState = (x_input_set_state_func_type *)GetProcAddress(XInputLibrary, "XInputSetState");
        if (!XInputSetState)
        {
            XInputSetState = XInputSetStateStub;
        }
    }
}

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    // TODO: log lib loaded

    if (DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,
                                                                &PrimaryBuffer,
                                                                0)))
                {
                    if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        //TODO: Log error and exit?
                        OutputDebugStringA("Succeeded in setting wave format for primary buffer.\n");
                    }
                    else
                    {
                        //TODO: Log error and exit?
                        OutputDebugStringA("Failed to set wave format for primary buffer.\n");
                    }
                }
                else
                {
                    //TODO: Log error and exit?
                    OutputDebugStringA("Failed to create Primary buffer.\n");
                }
            }
            else
            {
                //TODO: Log error and exit?
                OutputDebugStringA("Failed to set cooperative level for direct sound.\n");
            }

            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,
                                                         &GlobalSecondaryBuffer,
                                                         0)))
            {
                OutputDebugStringA("Succeeded in creating secondary sound buffer\n");
            }
        }
        else
        {

        }
    }
    else
    {
        // TODO: log lib loaded
    }
}

static win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

static void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    // BitmapInfo.bmiHeader.biSizeImage = 0;
    // BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    // BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    // BitmapInfo.bmiHeader.biClrUsed = 0;
    // BitmapInfo.bmiHeader.biClrImportant = 0;
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * BytesPerPixel;

    // TODO: Probably want to clear this to black
}

static void Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
                                       int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  //   X, Y, Width, Height,
                  //   X, Y, Width, Height,
                  0, 0, Width, Height,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

static LRESULT CALLBACK Win32MainWindowCallback(HWND Window,
                   UINT Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        // case WM_SIZE:
        // {
        //     // OutputDebugStringA("Size\n");
        // } break;
        case WM_DESTROY:
        {
            GlobalRunning = false;
            OutputDebugStringA("Destroy\n");
        } break;
        case WM_CLOSE:
        {
            GlobalRunning = false;
            OutputDebugStringA("Close\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("Activate app\n");
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            // Bit 30 indicates if the key was down before this message was sent.
            // 0=Key has just been pressed down. 1=Key was already down.
            bool32 WasDown = (LParam & (1 << 30)) != 0;
            bool32 IsDown = (LParam & (1 << 31)) == 0; // Bit 31 indicates if the key is down. 1=Down, 0=UP
            bool32 IsAlt = (LParam & (1 << 29)) != 0;
            if (WasDown == IsDown)
            {
                break;
            }
            switch(VKCode)
            {
                case 'W':
                {
                } break;
                case 'A':
                {
                } break;
                case 'S':
                {
                } break;
                case 'D':
                {
                } break;
                case 'Q':
                {
                } break;
                case 'E':
                {
                } break;
                case VK_SHIFT:
                {
                } break;
                case VK_CONTROL:
                {
                } break;
                case VK_ESCAPE:
                {
                } break;
                case VK_SPACE:
                {
                } break;
                case VK_LEFT:
                {
                } break;
                case VK_RIGHT:
                {
                } break;
                case VK_UP:
                {
                } break;
                case VK_DOWN:
                {
                } break;
                case VK_F4:
                {
                    if (IsDown && IsAlt)
                    {
                        GlobalRunning = false;
                    }
                } break;
                default:
                {

                }
            }
        } break;
        // Casey says we don't want to windows to handle syskeys
        // Result = DefWindowProc(Window, Message, WParam, LParam);
        // case WM_SETCURSOR:
        // {
        //     SetCursor(0);
        // } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            // int X = Paint.rcPaint.left;
            // int Y = Paint.rcPaint.top;
            // int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            // int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
                                       WindowDimension.Width, WindowDimension.Height);
            EndPaint(Window, &Paint);
            // static DWORD Operation = WHITENESS;
            // Operation == WHITENESS ? Operation = BLACKNESS : Operation = WHITENESS;
            // PatBlt(DeviceContext, X, Y, Width, Height, Operation);
        } break;
        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}

struct win32_sound_output
{
    int SamplesPerSecond;
    int ToneHz;
    int16 ToneVolume;
    uint32 RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

internal void Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                        &Region1, &Region1Size,
                                        &Region2, &Region2Size,
                                        0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                                   game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                        &Region1, &Region1Size,
                                        &Region2, &Region2Size,
                                        0)))
    {
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCode)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32LoadXInput();
    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
                                            WindowClass.lpszClassName,
                                            "Handmade Hero",
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            0,
                                            0,
                                            Instance,
                                            0);
        if(Window)
        {
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0;
            int YOffset = 0;

            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.ToneVolume = 3000;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;

            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);

            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;

            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
                                                   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            uint64 LastCycleCount = __rdtsc();
            while (GlobalRunning)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                for (DWORD ControllerIndex=0; ControllerIndex< XUSER_MAX_COUNT; ++ControllerIndex )
                {
                    XINPUT_STATE ControllerState;
                    // ZeroMemory( &ControllerState, sizeof(XINPUT_STATE) );

                    DWORD Result = XInputGetState( ControllerIndex, &ControllerState);
                    if (Result == ERROR_SUCCESS)
                    {
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 LStickX = Pad->sThumbLX;
                        int16 LStickY = Pad->sThumbLY;
                        int16 RStickX = Pad->sThumbRX;
                        int16 RStickY = Pad->sThumbRY;

                        XOffset += LStickX / 4096;
                        YOffset += LStickY / 4096;
                        SoundOutput.ToneHz = 512 + (int)(256.0f * ((real32)LStickY / 30000.0f));
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
                    }
                }

                // XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 60000;
                // Vibration.wRightMotorSpeed = 6000;
                // XInputSetState(0, &Vibration);

                DWORD ByteToLock = 0;
                DWORD TargetCursor = 0;
                DWORD BytesToWrite = 0;
                DWORD PlayCursor = 0;
                DWORD WriteCursor = 0;
                bool32 SoundIsValid = false;

                // Sound Test
                if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
                         SoundOutput.SecondaryBufferSize);
                    TargetCursor =
                        ((PlayCursor +
                        (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) %
                        SoundOutput.SecondaryBufferSize);
                    if (ByteToLock > TargetCursor)
                    {
                        BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }
                
                    SoundIsValid = true;
                }
                else
                {
                    OutputDebugStringA("Failed to get current position of secondary buffer.\n");
                }

                game_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;

                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackBuffer.Memory;
                Buffer.Width = GlobalBackBuffer.Width;
                Buffer.Height = GlobalBackBuffer.Height;
                Buffer.Pitch = GlobalBackBuffer.Pitch;

                GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);

                if (SoundIsValid)
                {
                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                }

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
                                           Dimension.Width, Dimension.Height);
            
                uint64 EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                real64 MSPerFrame = (((1000.0f * (real64)CounterElapsed) / (real64)PerfCountFrequency));
                real64 FPS = (real64)PerfCountFrequency / (real64)CounterElapsed;
                real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));

#if 0
                char Buffer[256];
                sprintf(Buffer, "%.02fms/f,  %.02ff/s,  %.02fmc/f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(Buffer);
#endif

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }
    return 0;
}