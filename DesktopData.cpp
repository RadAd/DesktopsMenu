#include "pch.h"
#include "DesktopData.h"
#include "ComUtils.h"
#include <winstring.h>

void Init(DesktopData& ws)
{
    CComPtr<IServiceProvider> pServiceProvider;
    CHECK_HR(pServiceProvider.CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER));

    CHECK_HR(pServiceProvider->QueryService(CLSID_VirtualDesktopManagerInternal, &ws.pVirtualDesktopManagerInternal));
    CHECK_HR(pServiceProvider->QueryService(CLSID_VirtualDesktopPinnedApps, &ws.pVirtualDesktopPinnedApps));
    CHECK_HR(pServiceProvider->QueryService(__uuidof(IApplicationViewCollection), &ws.pApplicationViewCollection));
}

CComPtr<Win10::IVirtualDesktop> GetDesktop(const DesktopData* ws, AdjacentDesktop uDirection)
{
    CComPtr<Win10::IVirtualDesktop> pDesktop;

    CComPtr<Win10::IVirtualDesktop> pCurrentDesktop;
    CHECK_HR_RET(ws->pVirtualDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop), pDesktop);

    if (FAILED(ws->pVirtualDesktopManagerInternal->GetAdjacentDesktop(pCurrentDesktop, uDirection, &pDesktop)))
        return pDesktop;

    return pDesktop;
}

CComPtr<Win10::IVirtualDesktop> GetDesktop(const DesktopData* ws, int dn)
{
    CComPtr<Win10::IVirtualDesktop> pDesktop;

    CComPtr<IObjectArray> pDesktopArray;
    if (ws->pVirtualDesktopManagerInternal && SUCCEEDED(ws->pVirtualDesktopManagerInternal->GetDesktops(&pDesktopArray)))
    {
        if (FAILED(pDesktopArray->GetAt(dn, IID_PPV_ARGS(&pDesktop))))
            return pDesktop;
    }

    return pDesktop;
}

LRESULT OnDesktopPin(const DesktopData* ws, const HWND hWndSrc, const Message msg)
{
    const LRESULT ret = msg == Message::Query ? MF_DISABLED : FALSE;
    try
    {
        CHECK_HR_RET(ws->pApplicationViewCollection->RefreshCollection(), ret);

        CComPtr<IApplicationView> pView;
        CHECK_HR_RET(ws->pApplicationViewCollection->GetViewForHwnd(hWndSrc, &pView), ret);

        switch (msg)
        {
        case Message::Query:
        {
            BOOL bPinned = FALSE;
            CHECK_HR_RET(ws->pVirtualDesktopPinnedApps->IsViewPinned(pView, &bPinned), ret);

            return MF_ENABLED | (bPinned ? MF_CHECKED : MF_UNCHECKED);
        }
        case Message::Select:
        {
            BOOL bPinned = FALSE;
            CHECK_HR_RET(ws->pVirtualDesktopPinnedApps->IsViewPinned(pView, &bPinned), ret);

            if (!bPinned)
            {
                CHECK_HR_RET(ws->pVirtualDesktopPinnedApps->PinView(pView), ret);
            }
            else
            {
                CHECK_HR_RET(ws->pVirtualDesktopPinnedApps->UnpinView(pView), ret);
            }

            return TRUE;
        }
        default:
            return ret;
        }
    }
    catch (...)
    {
        ReportError(_T("FAILED Exception"));
        return ret;
    }
}

LRESULT OnDesktopMove(const DesktopData* ws, const HWND hWndSrc, const Message msg, CComPtr<Win10::IVirtualDesktop> pDesktop)
{
    const LRESULT ret = msg == Message::Query ? MF_DISABLED : FALSE;
    try
    {
        CHECK_HR_RET(ws->pApplicationViewCollection->RefreshCollection(), ret);

        CComPtr<IApplicationView> pView;
        CHECK_HR_RET(ws->pApplicationViewCollection->GetViewForHwnd(hWndSrc, &pView), ret);

        switch (msg)
        {
        case Message::Query:
        {
            BOOL bCanMove = FALSE;
            CHECK_HR_RET(ws->pVirtualDesktopManagerInternal->CanViewMoveDesktops(pView, &bCanMove), ret);

            if (bCanMove)
            {
                if (!pDesktop)
                    return MF_DISABLED;

                CComPtr<Win10::IVirtualDesktop> pCurrentDesktop;
                CHECK_HR_RET(ws->pVirtualDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop), ret);

                if (pCurrentDesktop.IsEqualObject(pDesktop))
                    return MF_ENABLED | MF_CHECKED;
                else
                    return MF_ENABLED;
            }
            else
                return MF_DISABLED;
        }
        case Message::Select:
        {
            if (!pDesktop)
                return FALSE;

            CHECK_HR_RET(ws->pVirtualDesktopManagerInternal->MoveViewToDesktop(pView, pDesktop), ret);

            return TRUE;
        }
        default:
            return ret;
        }
    }
    catch (...)
    {
        ReportError(_T("FAILED Exception"));
        return ret;
    }
}

BOOL OnDesktopSwitch(const DesktopData* ws, CComPtr<Win10::IVirtualDesktop> pDesktop)
{
    const BOOL ret = FALSE;
    try
    {
        if (!pDesktop)
            return FALSE;

        CHECK_HR_RET(ws->pVirtualDesktopManagerInternal->SwitchDesktop(pDesktop), ret);

        return TRUE;
    }
    catch (...)
    {
        ReportError(_T("FAILED Exception"));
        return ret;
    }
}

int GetDesktopNames(const DesktopData* ws, LPTSTR text, const int size)
{
    text[0] = _T('\0');

    CComPtr<IObjectArray> pDesktopArray;
    if (ws->pVirtualDesktopManagerInternal && SUCCEEDED(ws->pVirtualDesktopManagerInternal->GetDesktops(&pDesktopArray)))
    {
        int dn = 0;
        for (CComPtr<Win10::IVirtualDesktop2> pDesktop : ObjectArrayRange<Win10::IVirtualDesktop2>(pDesktopArray))
        {
            ++dn;

            HSTRING s = NULL;
            CHECK_HR_RET(pDesktop->GetName(&s), 0);

            if (text[0] != _T('\0'))
                wcscat_s(text, size, _T("|"));
            if (s == nullptr)
            {
                size_t len = wcslen(text);
                swprintf(text + len, size - len, _T("Desktop %d"), dn);
            }
            else
            {
                wcscat_s(text, size, WindowsGetStringRawBuffer(s, nullptr));

                WindowsDeleteString(s);
            }
        }
    }
    return (int) wcslen(text);
}

std::vector<std::wstring> GetDesktopNames(const DesktopData* ws, int* current)
{
    std::vector<std::wstring> names;
    CComPtr<Win10::IVirtualDesktop> pCurrentDesktop;
    ws->pVirtualDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop);
    CComPtr<IObjectArray> pDesktopArray;
    if (ws->pVirtualDesktopManagerInternal && SUCCEEDED(ws->pVirtualDesktopManagerInternal->GetDesktops(&pDesktopArray)))
    {
        int dn = 0;
        for (CComPtr<Win10::IVirtualDesktop2> pDesktop : ObjectArrayRange<Win10::IVirtualDesktop2>(pDesktopArray))
        {
            ++dn;

            if (pCurrentDesktop.IsEqualObject(pDesktop))
                *current = dn;

            HSTRING s = NULL;
            CHECK_HR_RET(pDesktop->GetName(&s), std::vector<std::wstring>());

            if (s == nullptr)
            {
                wchar_t n[100];
                swprintf(n, ARRAYSIZE(n), _T("Desktop %d"), dn);
                names.push_back(n);
            }
            else
            {
                names.push_back(WindowsGetStringRawBuffer(s, nullptr));

                WindowsDeleteString(s);
            }
        }
    }
    return names;
}
