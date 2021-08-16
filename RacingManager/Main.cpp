#include "RacingManager.h"
#include "AppException.h"
#include <windows.h>

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance, _In_ PSTR cmdLine, _In_ int showCmd)
{
    int result = 0;
    try
    {
        RacingManager racingManager(hInstance);
        racingManager.Initialize();
        result = racingManager.Run();
    }
    catch(const AppException& ex)
    {
        MessageBox(nullptr, ex.GetErrorMsgWide(), L"Error", MB_OK);
    }

    return result;
}
