#pragma once

#include "Win10Desktops.h"

struct DesktopData
{
	CComPtr<IVirtualDesktopManagerInternal> pVirtualDesktopManagerInternal;
	CComPtr<IVirtualDesktopPinnedApps> pVirtualDesktopPinnedApps;
	CComPtr<IApplicationViewCollection> pApplicationViewCollection;
};

void Init(DesktopData& ws);
CComPtr<IVirtualDesktop> GetDesktop(const DesktopData* ws, AdjacentDesktop uDirection);
CComPtr<IVirtualDesktop> GetDesktop(const DesktopData* ws, int dn);
LRESULT OnDesktopPin(const DesktopData* ws, const HWND hWndSrc, const Message msg);
LRESULT OnDesktopMove(const DesktopData* ws, const HWND hWndSrc, const Message msg, CComPtr<IVirtualDesktop> pDesktop);
int GetDesktopNames(const DesktopData* ws, LPTSTR text, const int size);