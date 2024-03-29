#include "pch.h"
#include "Hooks.h"
#include "resource.h"
#include <shlobj_core.h>
#include <vector>
#include <string>

HMENU LoadPopupMenu(HINSTANCE hInstance, DWORD id);
UINT MenuFindByCommand(HMENU hMenu, UINT id);
void InsertDesktopMenu(const HMENU hMenuSwitch, const UINT id, const std::vector<std::wstring>& names);

std::vector<std::wstring> GetDesktopNames(const HWND hDesktopsWnd)
{
    std::vector<std::wstring> dtnames;

    TCHAR names[1024] = _T("");
    //GetWindowText(hDesktopsWnd, names, ARRAYSIZE(names));
    SendMessage(hDesktopsWnd, WM_GETTEXT, ARRAYSIZE(names), (LPARAM) names);
    TCHAR* n = names;
    while (TCHAR* e = wcschr(n, _T('|')))
    {
        *e = _T('\0');
        dtnames.emplace_back(n);
        n = e + 1;
    }
    dtnames.emplace_back(n);

    return dtnames;
}

LRESULT SendDesktopMessage(const HWND hDesktopsWnd, const HWND hWnd, const UINT type, const Message msg)
{
    const MessageStruct ms = { type, msg };
    COPYDATASTRUCT cds = { CD_DESKTOPS, sizeof(ms), (PVOID) &ms };
    return SendMessage(hDesktopsWnd, WM_COPYDATA, (WPARAM) hWnd, (LPARAM) &cds);
}

void HookProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    switch (message)
    {
    case WM_INITMENUPOPUP:
    {
        const HMENU hMenu = (HMENU) wParam;
        const HMENU hMenuSystem = GetSystemMenu(hWnd, FALSE);
        if (hMenu == hMenuSystem)
        {
            const HWND hDesktopsWnd = FindWindow(g_lpstrClass, nullptr);

            if (MenuFindByCommand(hMenuSystem, SC_PIN) == -1)
            {
                const HMENU hMenuDesktop = LoadPopupMenu(g_hDllInstance, IDR_SYSTEM);
                InsertDesktopMenu(GetSubMenu(hMenuDesktop, 1), SC_MOVE_DESKTOP, GetDesktopNames(hDesktopsWnd));

                Shell_MergeMenus(hMenuSystem, hMenuDesktop, MenuFindByCommand(hMenuSystem, SC_CLOSE), 0, 0xFFFF, MM_ADDSEPARATOR);

                DestroyMenu(hMenuDesktop);
            }

            for (UINT type : { SC_PIN, SC_MOVE_PREV, SC_MOVE_NEXT })
            {
                const LRESULT result = SendDesktopMessage(hDesktopsWnd, hWnd, type, Message::Query);
                EnableMenuItem(hMenuSystem, type, MF_BYCOMMAND | static_cast<UINT>(result & MF_DISABLED));
                CheckMenuItem(hMenuSystem, type, MF_BYCOMMAND | static_cast<UINT>(result & MF_CHECKED));
            }
            UINT radio = -1;
            UINT max = SC_MOVE_DESKTOP + 0x90;
            for (UINT type = SC_MOVE_DESKTOP + 0; type <= max; type += 0x10)
            {
                const LRESULT result = SendDesktopMessage(hDesktopsWnd, hWnd, type, Message::Query);
                if (EnableMenuItem(hMenuSystem, type, MF_BYCOMMAND | static_cast<UINT>(result & MF_DISABLED)) < 0)
                {
                    max = type - 1;
                    break;
                }
                if (result & MF_CHECKED)
                    radio = type;
            }
            if (radio != (UINT) -1)
                CheckMenuRadioItem(hMenuSystem, SC_MOVE_DESKTOP, max, radio, MF_BYCOMMAND);
        }
        break;
    }

    case WM_SYSCOMMAND:
        switch (((UINT) wParam & 0xFFF0))
        {
        case SC_PIN:
        case SC_MOVE_PREV:
        case SC_MOVE_NEXT:
        case SC_MOVE_DESKTOP + 0x00:
        case SC_MOVE_DESKTOP + 0x10:
        case SC_MOVE_DESKTOP + 0x20:
        case SC_MOVE_DESKTOP + 0x30:
        case SC_MOVE_DESKTOP + 0x40:
        case SC_MOVE_DESKTOP + 0x50:
        case SC_MOVE_DESKTOP + 0x60:
        case SC_MOVE_DESKTOP + 0x70:
        case SC_MOVE_DESKTOP + 0x80:
        case SC_MOVE_DESKTOP + 0x90:
        {
            const HWND hDesktopsWnd = FindWindow(g_lpstrClass, nullptr);
            SendDesktopMessage(hDesktopsWnd, hWnd, (UINT) wParam, Message::Select);
            break;
        }
        }
        break;
    }
}

LRESULT CALLBACK CallWndProc(_In_ const int nCode, _In_ const WPARAM wParam, _In_ const LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        const CWPSTRUCT* sMsg = (CWPSTRUCT*) lParam;
        HookProc(sMsg->hwnd, sMsg->message, sMsg->wParam, sMsg->lParam);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(_In_ const int nCode, _In_ const WPARAM wParam, _In_ const LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == PM_REMOVE)
        {
            const MSG* pMsg = (MSG*) lParam;
            HookProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
