// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

HINSTANCE g_hDllInstance = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hDllInstance = hModule;
        //CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        break;
    case DLL_PROCESS_DETACH:
        //CoUninitialize();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

HWND g_hMsgWnd = NULL;

void ReportError(LPCTSTR msg)
{
#if 0
    MessageBox(g_hMsgWnd, msg, g_lpstrTitle, MB_ICONERROR | MB_OK);
#else
    TCHAR buf[1024];
    swprintf_s(buf, _T("%s\n"), msg);
    OutputDebugString(buf);
#endif
}
