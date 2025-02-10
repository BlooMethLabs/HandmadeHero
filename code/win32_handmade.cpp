#include <windows.h>

LRESULT CALLBACK
MainWindowCallback(HWND Window,
                   UINT Message,
                   WPARAM WParam,
                   LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("Size\n");
        } break;
        case WM_DESTROY:
        {
            OutputDebugStringA("Destroy\n");
        } break;
        case WM_CLOSE:
        {
            OutputDebugStringA("Close\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("Activate app\n");
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            static DWORD Operation = WHITENESS;
            Operation == WHITENESS ? Operation = BLACKNESS : Operation = WHITENESS;
            PatBlt(DeviceContext, X, Y, Width, Height, Operation);
            // BOOL EndPaint(
            //   [in] HWND              hWnd,
            //   [in] const PAINTSTRUCT *lpPaint
            // );
        }
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
    WNDCLASSA WindowClass = {};

    // WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass))
    {
        HWND WindowHandle = CreateWindowExA(0,
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
        if(WindowHandle)
        {
            MSG Message;
            for(;;)
            {
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
                if (MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else
                {
                    break;
                }
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