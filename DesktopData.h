#pragma once

#include "Win10Desktops.h"

#include <vector>
#include <string>

struct DesktopData
{
    CComPtr<Win10::IVirtualDesktopManagerInternal> pVirtualDesktopManagerInternal;
    CComPtr<IVirtualDesktopPinnedApps> pVirtualDesktopPinnedApps;
    CComPtr<IApplicationViewCollection> pApplicationViewCollection;
};

void Init(DesktopData& ws);
CComPtr<Win10::IVirtualDesktop> GetDesktop(const DesktopData* ws, AdjacentDesktop uDirection);
CComPtr<Win10::IVirtualDesktop> GetDesktop(const DesktopData* ws, int dn);
LRESULT OnDesktopPin(const DesktopData* ws, const HWND hWndSrc, const Message msg);
LRESULT OnDesktopMove(const DesktopData* ws, const HWND hWndSrc, const Message msg, CComPtr<Win10::IVirtualDesktop> pDesktop);
BOOL OnDesktopSwitch(const DesktopData* ws, CComPtr<Win10::IVirtualDesktop> pDesktop);
int GetDesktopNames(const DesktopData* ws, LPTSTR text, const int size);
std::vector<std::wstring> GetDesktopNames(const DesktopData* ws);
