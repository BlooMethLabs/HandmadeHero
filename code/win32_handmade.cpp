#include <windows.h>
#include <stdint.h>
#include <xinput.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// TODO: this is a global for now.
static bool Running;

// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state_func_type);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return 0;
}
static x_input_get_state_func_type *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state_func_type);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return 0;
}
static x_input_set_state_func_type *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static void Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("XInput1_3.dll");
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state_func_type *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state_func_type *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};
static win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
    int Width;
    int Height;
};

static win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

static void RenderWeirdGradient(win32_offscreen_buffer Buffer, uint8 XOffset, uint8 YOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {
            // Spreads grandient across whole window
            // uint8 Green = (( ((double)X) / Buffer.Width) * 256) + XOffset;
            // uint8 Blue = (( ((double)Y) / Buffer.Height) * 256) + YOffset;
            uint8 Green = X + XOffset;
            uint8 Blue = Y + YOffset;

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer.Pitch;
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
    int BitmapMemorySize = Width * Height * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // TODO: Probably want to clear this to black

    Buffer->Pitch = Width * BytesPerPixel;
}

static void Win32DisplayBufferInWindow(HDC DeviceContext, win32_offscreen_buffer Buffer, int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  //   X, Y, Width, Height,
                  //   X, Y, Width, Height,
                  0, 0, Width, Height,
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory,
                  &Buffer.Info,
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
        case WM_SIZE:
        {
            // OutputDebugStringA("Size\n");
        } break;
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
            bool WasDown = (LParam & (1 << 30)) != 0;
            bool IsDown = (LParam & (1 << 31)) == 0;
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
                    OutputDebugStringA("Esc: ");
                    if (WasDown) OutputDebugStringA("WasDown ");
                    if (IsDown) OutputDebugStringA("IsDown\n");
                    OutputDebugStringA("\n");
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
                default:
                {

                }
            }
        } break;
        Result = DefWindowProc(Window, Message, WParam, LParam);
        case WM_SETCURSOR:
        {
            SetCursor(0);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, GlobalBackBuffer, X, Y, WindowDimension.Width, WindowDimension.Height);
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

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CmdLine,
                     int ShowCode)
{
    Win32LoadXInput();
    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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
            Running = true;
            uint8 XOffset = 0;
            uint8 YOffset = 0;
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

                DWORD dwResult;    
                for (DWORD ControllerIndex=0; ControllerIndex< XUSER_MAX_COUNT; ControllerIndex++ )
                {
                    XINPUT_STATE ControllerState;
                    ZeroMemory( &ControllerState, sizeof(XINPUT_STATE) );

                    dwResult = XInputGetState( ControllerIndex, &ControllerState );

                    if( dwResult != ERROR_SUCCESS )
                    {
                        continue;
                    }
                    XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                    bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                    bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                    bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                    bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                    bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                    bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                    bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                    bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                    bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                    bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                    bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                    bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                    int16 LStickX = Pad->sThumbLX;
                    int16 LStickY = Pad->sThumbLY;
                    int16 RStickX = Pad->sThumbRX;
                    int16 RStickY = Pad->sThumbRY;
                    ++XOffset;
                    if (AButton)
                        ++YOffset;
                }

                // XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 60000;
                // Vibration.wRightMotorSpeed = 6000;
                // XInputSetState(0, &Vibration);

                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, GlobalBackBuffer, 0, 0, WindowDimension.Width, WindowDimension.Height);
                ReleaseDC(Window, DeviceContext);
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