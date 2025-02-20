#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

// Syntax errors for these:
typedef float real32;
typedef double real64;

#define Pi32 3.1415926535f

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

static bool32 Running;
static win32_offscreen_buffer GlobalBackBuffer;
static LPDIRECTSOUNDBUFFER SecondaryBuffer;

// Alias functions with stubs
// XInputGetState stub
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state_func_type);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state_func_type *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState stub
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state_func_type);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state_func_type *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// DirectSoundCreate
// #define DIRECT_SOUND_CREATE(name) \
//     HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef HRESULT WINAPI direct_sound_create_func_type(LPCGUID pcGuidDevice,
                                                     LPDIRECTSOUND *ppDS,
                                                     LPUNKNOWN pUnkOuter);

static void Win32LoadXInput(void)
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

static void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    // TODO: log lib loaded

    if (DSoundLibrary)
    {
        direct_sound_create_func_type *DirectSoundCreate = (direct_sound_create_func_type *)
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
                DSBUFFERDESC PrimaryBufferDescription = {};
                PrimaryBufferDescription.dwSize = sizeof(PrimaryBufferDescription);
                PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription,
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
                                                              &SecondaryBuffer,
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

static void RenderWeirdGradient(win32_offscreen_buffer *Buffer, uint8 XOffset, uint8 YOffset)
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
            uint8 Green = X + XOffset;
            uint8 Blue = Y + YOffset;

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
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
            Running = false;
            OutputDebugStringA("Destroy\n");
        } break;
        case WM_CLOSE:
        {
            Running = false;
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
                        Running = false;
                    }
                } break;
                default:
                {

                }
            }
        } break;
        // Casey says we don't want to windows to handle syskeys
        // Result = DefWindowProc(Window, Message, WParam, LParam);
        case WM_SETCURSOR:
        {
            SetCursor(0);
        } break;
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
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}

struct win32_sound_output
{
    int SamplesPerSecond = 48000;
    int ToneHz = 256;
    int16 ToneVolume = 3000;
    uint32 RunningSampleIndex = 0;
    int WavePeriod = SamplesPerSecond / ToneHz;
    int BytesPerSample = sizeof(int16)*2;
    int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;
    real32 tSine = 0;
    int LatencySampleCount = SamplesPerSecond / 15;
};

void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if (SUCCEEDED(SecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                        &Region1, &Region1Size,
                                        &Region2, &Region2Size,
                                        0)))
    {
        DWORD RegionSampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16 *SampleOut = (int16 *)Region1;
        for (int i = 0; i < 2; ++i)
        {
            for (DWORD SampleIndex = 0; SampleIndex < RegionSampleCount; ++SampleIndex)
            {
                real32 t = 2.0f * Pi32 * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
                real32 SineValue = sinf(SoundOutput->tSine);
                int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
                *SampleOut++ = SampleValue;
                *SampleOut++ = SampleValue;

                SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
                ++SoundOutput->RunningSampleIndex;
            }
            RegionSampleCount = Region2Size / SoundOutput->BytesPerSample;
            SampleOut = (int16 *)Region2;
        }

        SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
    else
    {
        OutputDebugStringA("Failed to lock secondary buffer.\n");
    }
}

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCode)
{
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

            uint8 XOffset = 0;
            uint8 YOffset = 0;

            win32_sound_output SoundOutput;

            Win32InitDSound(Window, 48000, SoundOutput.SecondaryBufferSize);
            Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);

            SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            Running = true;
            while (Running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                for (DWORD ControllerIndex=0; ControllerIndex< XUSER_MAX_COUNT; ControllerIndex++ )
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

                        XOffset += LStickX / 2048;
                        YOffset += LStickY / 2048;
                        SoundOutput.ToneHz = 512 + (int)(256.0f * ((real32)LStickY / 30000.0f));
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
                    }
                }

                // XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 60000;
                // Vibration.wRightMotorSpeed = 6000;
                // XInputSetState(0, &Vibration);

                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                // Sound Test
                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    DWORD ByteToLock =
                        (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    DWORD TargetCursor =
                        ((PlayCursor +
                        (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) %
                        SoundOutput.SecondaryBufferSize);
                    DWORD BytesToWrite;
                    if (ByteToLock > TargetCursor)
                    {
                        BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }

                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
                }
                else
                {
                    OutputDebugStringA("Failed to get current position of secondary buffer.\n");
                }

                win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext,
                                           WindowDimension.Width, WindowDimension.Height);
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