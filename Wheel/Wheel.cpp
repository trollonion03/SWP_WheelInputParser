#define _CRT_SECURE_NO_WARNINGS
#include <dinput.h>
#include <iostream>

// DirectInput Device Object
LPDIRECTINPUTDEVICE8W g_pDIWheelDevice = nullptr;
LPDIRECTINPUT8 g_pDI = nullptr;

// Input buffer
char g_szInputBuffer[256];

// Initialize DirectInput Device
bool InitDirectInput(HWND hWnd)
{
    // Initialize DirectInput
    if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8W, (void**)&g_pDI, nullptr)))
    {
        return false;
    }


    // Initialize DirectInput Device
    if (FAILED(g_pDI->CreateDevice(GUID_SysMouse, &g_pDIWheelDevice, nullptr)))
    {
        return false;
    }

    // Set DirectInput Device Data Format
    if (FAILED(g_pDIWheelDevice->SetDataFormat(&c_dfDIMouse)))
    {
        return false;
    }

    // Set DirectInput Device Cooperation Level
    if (FAILED(g_pDIWheelDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
    {
        return false;
    }

    // Acquire DirectInput Device
    if (FAILED(g_pDIWheelDevice->Acquire()))
    {
        return false;
    }

    return true;
}

// Read Input from DirectInput Device
void ReadInput()
{
    DWORD dwElements = 256;
    HRESULT hr = g_pDIWheelDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), (LPDIDEVICEOBJECTDATA)&g_szInputBuffer, &dwElements, 0);

    if (SUCCEEDED(hr))
    {
        // Process Input
        for (DWORD i = 0; i < dwElements; i++)
        {
            DIDEVICEOBJECTDATA* pData = (DIDEVICEOBJECTDATA*)&g_szInputBuffer[i];

            // Handle Input
            switch (pData->dwOfs)
            {
            case DIMOFS_X:
                // Handle X Axis Input
                std::cout << "X Axis Input: " << pData->dwData << std::endl;
                break;

            case DIMOFS_Y:
                // Handle Y Axis Input
                std::cout << "Y Axis Input: " << pData->dwData << std::endl;
                break;

            case DIMOFS_Z:
                // Handle Z Axis Input
                std::cout << "Z Axis Input: " << pData->dwData << std::endl;
                break;

                // Add more cases to handle additional inputs as needed

            default:
                break;
            }
        }
    }
}

int main()
{
    // Allocate Console
    AllocConsole();

    // Redirect Standard Input and Output to Console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);

    // Create Window (or use an existing one)
    HWND hWnd = CreateWindowW(
        L"STATIC",      // Window class name
        L"My Window",   // Window title
        WS_OVERLAPPEDWINDOW,    // Window style
        CW_USEDEFAULT,  // X position
        CW_USEDEFAULT,  // Y position
        640,            // Width
        480,            // Height
        nullptr,        // Parent window handle
        nullptr,        // Menu handle
        nullptr,        // Instance handle
        nullptr         // Additional application data
    );

    // Initialize DirectInput Device
    if (!InitDirectInput(hWnd))
    {
        return 0;
    }

    // Main Game Loop
    while (true)
    {
        // Read Input from DirectInput Device
        ReadInput();

        // Process Input and Update Game State
        // ...
    }

    // Release DirectInput Device
    g_pDIWheelDevice->Unacquire();
    g_pDIWheelDevice->Release();

    return 0;
}