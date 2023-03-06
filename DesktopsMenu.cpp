#include "pch.h"
#include "Hooks.h"
#include "resource.h"
#include "DesktopData.h"

#ifdef _WIN64
const LPCTSTR g_lpstrTitle = _T("Desktops Menu (x64)");
const LPCTSTR g_lpstrClass = _T("DESKTOPSMENU64");
#else
const LPCTSTR g_lpstrTitle = _T("Desktops Menu (x86)");
const LPCTSTR g_lpstrClass = _T("DESKTOPSMENU32");
#endif
#define WM_TRAY								(WM_USER + 0x0200)

HMENU LoadPopupMenu(HINSTANCE hInstance, DWORD id)
{
    const HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(id));
    const HMENU hPopupMenu = GetSubMenu(hMenu, 0);
    VALIDATEE(RemoveMenu(hMenu, 0, MF_BYPOSITION));
    VALIDATEE(DestroyMenu(hMenu));
    return hPopupMenu;
}

UINT MenuFindByCommand(HMENU hMenu, UINT id)
{
    for (int nPos = 0; nPos < GetMenuItemCount(hMenu); ++nPos)
    {
        if (GetMenuItemID(hMenu, nPos) == id)
            return nPos;
    }
    return -1;
}

void InsertDesktopMenu(const HMENU hMenuSwitch, const UINT id, const std::vector<std::wstring>& names)
{
    const UINT pos = MenuFindByCommand(hMenuSwitch, id);
    VALIDATE(pos >= 0, _T("MenuFindByCommand not found"));
    DeleteMenu(hMenuSwitch, pos, MF_BYPOSITION);

    UINT dn = 0;
    for (const std::wstring& n : names)
    {
        TCHAR buf[1024];
        wsprintf(buf, L"&%d. %s", (dn + 1), n.c_str());
        InsertMenu(hMenuSwitch, pos + dn, MF_STRING, id + ((UINT_PTR) dn << 4), buf);
        ++dn;
    }
}

enum
{
    HK_PIN,
    HK_SWITCH,
};

struct HookDeleter
{
    typedef HHOOK pointer;
    void operator()(HHOOK handle)
    {
        if (handle != NULL)
            UnhookWindowsHookEx(handle);
    }
};
typedef std::unique_ptr<HHOOK, HookDeleter> unique_hook_t;

INT_PTR CALLBACK AboutDlg(const HWND hDlg, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        SendMessage(GetDlgItem(hDlg, IDC_ABOUT_APPICON), STM_SETICON, (WPARAM) LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_MAIN)), 0);

        TCHAR	FileName[1024];
        GetModuleFileName(g_hDllInstance, FileName, 1024);

        DWORD	Dummy;
        DWORD	Size = GetFileVersionInfoSize(FileName, &Dummy);

        if (Size > 0)
        {
            void* Info = malloc(Size);
            if (Info != nullptr)
            {
                // VS_VERSION_INFO   VS_VERSIONINFO  VS_FIXEDFILEINFO

                //Dummy = 0;
                GetFileVersionInfo(FileName, Dummy, Size, Info);

                TCHAR* String;
                UINT	Length;
                VerQueryValue(Info, TEXT("\\StringFileInfo\\0c0904b0\\FileVersion"), (LPVOID*) &String, &Length);
                SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_VERSION), String);
                VerQueryValue(Info, TEXT("\\StringFileInfo\\0c0904b0\\ProductName"), (LPVOID*) &String, &Length);
                SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_PRODUCT), String);

                free(Info);
            }
        }
        else
        {
            SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_VERSION), TEXT("Unknown"));
        }

        return TRUE;
    }
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case IDC_ABOUT_WEBSITE:
            case IDC_ABOUT_MAIL:
            {
                TCHAR	Url[1024];
                GetWindowText((HWND) lParam, Url, 1024);
                ShellExecute(hDlg, TEXT("open"), Url, NULL, NULL, SW_SHOW);
                return TRUE;
            }
            break;
            case IDCANCEL:
            case IDOK:
            {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;
            }
        }
    }
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    static UINT s_uTaskbarRestart = 0;
    if (s_uTaskbarRestart != 0 && uMsg == s_uTaskbarRestart)
    {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_TRAY;
        wcscpy_s(nid.szTip, g_lpstrTitle);
        //LoadIconMetric(g_hDllInstance, MAKEINTRESOURCE(IDI_MAIN), LIM_SMALL, &(nid.hIcon));
        nid.hIcon = (HICON) GetClassLongPtr(hWnd, GCLP_HICON);
        VALIDATEE(Shell_NotifyIcon(NIM_ADD, &nid));

        DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        Init(*ws);

        return TRUE;
    }
    switch (uMsg)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT cs = (LPCREATESTRUCT) lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) cs->lpCreateParams);

        VALIDATEE(RegisterHotKey(hWnd, HK_PIN, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, _T('P')));
        VALIDATEE(RegisterHotKey(hWnd, HK_SWITCH, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, _T('D')));

        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_TRAY;
        wcscpy_s(nid.szTip, g_lpstrTitle);
        //LoadIconMetric(g_hDllInstance, MAKEINTRESOURCE(IDI_MAIN), LIM_SMALL, &(nid.hIcon));
        nid.hIcon = (HICON) GetClassLongPtr(hWnd, GCLP_HICON);
        VALIDATEE(Shell_NotifyIcon(NIM_ADD, &nid));

        s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_DESTROY:
    {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uFlags = NIF_MESSAGE;
        nid.uCallbackMessage = WM_TRAY;
        VALIDATEE(Shell_NotifyIcon(NIM_DELETE, &nid));

        VALIDATEE(UnregisterHotKey(hWnd, HK_PIN));
        VALIDATEE(UnregisterHotKey(hWnd, HK_SWITCH));

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_CLOSE:
        PostQuitMessage(0);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    case WM_COPYDATA:
    {
        const HWND hWndSrc = (HWND) wParam;
        const COPYDATASTRUCT* cds = (COPYDATASTRUCT*) lParam;
        if (cds->dwData == CD_DESKTOPS && cds->cbData == sizeof(MessageStruct))
        {
            const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            const MessageStruct* ms = (MessageStruct*) cds->lpData;
            switch (ms->type)
            {
            case SC_PIN: return OnDesktopPin(ws, hWndSrc, ms->msg);
            case SC_MOVE_PREV: return OnDesktopMove(ws, hWndSrc, ms->msg, GetDesktop(ws, LeftDirection));
            case SC_MOVE_NEXT: return OnDesktopMove(ws, hWndSrc, ms->msg, GetDesktop(ws, RightDirection));
            default:
                if (ms->type >= SC_MOVE_DESKTOP && ms->type < (SC_MOVE_DESKTOP + 100))
                    return OnDesktopMove(ws, hWndSrc, ms->msg, GetDesktop(ws, (ms->type - SC_MOVE_DESKTOP) >> 4));
                else
                    return TRUE;
            }
        }
        else
            return FALSE;
    }

    case WM_GETTEXT:
    {
        int nSize = static_cast<int>(wParam);
        const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (ws != nullptr && nSize == 1024)	// Size 1024 means it comes from HookProc
            return GetDesktopNames(ws, (TCHAR*) lParam, nSize);
        else
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_MAIN_ABOUT:
            DialogBox(g_hDllInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlg);
            return FALSE;

        case ID_MAIN_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return FALSE;

        case ID_SWITCH_PREVIOUS:
        {
            const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            VALIDATEE(OnDesktopSwitch(ws, GetDesktop(ws, LeftDirection)));
            return FALSE;
        }

        case ID_SWITCH_NEXT:
        {
            const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            VALIDATEE(OnDesktopSwitch(ws, GetDesktop(ws, RightDirection)));
            return FALSE;
        }

        default:
            if (LOWORD(wParam) >= ID_SWITCH_DESKTOPS && LOWORD(wParam) < (ID_SWITCH_DESKTOPS + 100))
            {
                const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                VALIDATEE(OnDesktopSwitch(ws, GetDesktop(ws, (LOWORD(wParam) - ID_SWITCH_DESKTOPS) >> 4)));
                return FALSE;
            }
            else
                return TRUE;
        }

    case WM_HOTKEY:
        switch (wParam)
        {
        case HK_PIN:
        {
            HWND hWndFG = GetForegroundWindow();
            const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            OnDesktopPin(ws, hWndFG, Message::Select);
            return TRUE;
        }
        case HK_SWITCH:
        {
            const DesktopData* ws = (DesktopData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

            const HMENU hMenuSwitch = LoadPopupMenu(g_hDllInstance, IDR_SWITCH);
            InsertDesktopMenu(hMenuSwitch, ID_SWITCH_DESKTOPS, GetDesktopNames(ws));
            // TODO Radio check current desktop

            const HWND hWndFG = GetForegroundWindow();
            HMONITOR hMonitor = MonitorFromWindow(hWndFG, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(MONITORINFO) };
            VALIDATEE(GetMonitorInfo(hMonitor, &mi));

            VALIDATEE(SetForegroundWindow(hWnd));
            POINT pt = { (mi.rcWork.left + mi.rcWork.right) / 2, (mi.rcWork.top + mi.rcWork.bottom) / 2 };
            VALIDATEE(TrackPopupMenu(hMenuSwitch, TPM_CENTERALIGN | TPM_VCENTERALIGN, pt.x, pt.y, 0, hWnd, nullptr));

            DestroyMenu(hMenuSwitch);

            return TRUE;
        }
        default:
            return FALSE;
        }

    case WM_TRAY:
        if (lParam == WM_RBUTTONUP)
        {
            VALIDATEE(SetForegroundWindow(hWnd));
            POINT pt;
            VALIDATEE(GetCursorPos(&pt));
            const HMENU hMenu = LoadPopupMenu(g_hDllInstance, IDR_MAIN);
            VALIDATEE(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hWnd, nullptr));
            DestroyMenu(hMenu);
        }
        return TRUE;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

struct CoInit
{
    HRESULT Init(DWORD dwCoInit)
    {
        return CoInitializeEx(nullptr, dwCoInit);
    }

    ~CoInit()
    {
        CoUninitialize();
    }
};

extern "C"
{

#ifdef _UNICODE
#define DesktopsMenu DesktopsMenuW

void __declspec(dllexport) CALLBACK DesktopsMenuA(const HWND hParentWnd, const HINSTANCE hInstance, const LPCSTR lpszCmdLine, const int nCmdShow)
{
    MessageBox(hParentWnd, _T("TODO translate"), g_lpstrTitle, MB_ICONWARNING | MB_OK);
}

#else
#define DesktopsMenu DesktopsMenuA

void __declspec(dllexport) CALLBACK DesktopsMenuW(const HWND hParentWnd, const HINSTANCE hInstance, const LPCWSTR lpszCmdLine, const int nCmdShow)
{
    MessageBox(hParentWnd, _T("TODO translate"), lpstrTitle, MB_ICONWARNING | MB_OK);
}
#endif

void __declspec(dllexport) CALLBACK DesktopsMenu(const HWND hParentWnd, const HINSTANCE hInstance, const LPCTSTR lpszCmdLine, const int nCmdShow)
{
    g_hMsgWnd = hParentWnd;

    CoInit coinit;
    CHECK_HR(coinit.Init(COINIT_MULTITHREADED));

    DesktopData ws;
    Init(ws);

    const unique_hook_t hHookCall(SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hDllInstance, 0));
    CHECK_MSG(hHookCall != NULL, _T("Error SetWindowsHookEx WH_CALLWNDPROC"));
    const unique_hook_t hHookGet(SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hDllInstance, 0));
    CHECK_MSG(hHookGet != NULL, _T("Error SetWindowsHookEx WH_GETMESSAGE"));

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_MAIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName = g_lpstrClass;
    RegisterClass(&wc);

    CREATESTRUCT cs = {};
    cs.lpszClass = wc.lpszClassName;
    cs.lpszName = g_lpstrTitle;
    cs.style = WS_OVERLAPPEDWINDOW;
    cs.dwExStyle = WS_EX_TOPMOST;
    cs.x = CW_USEDEFAULT;
    cs.x = CW_USEDEFAULT;
    cs.cx = CW_USEDEFAULT;
    cs.cy = CW_USEDEFAULT;
    cs.hwndParent = hParentWnd;
    cs.lpCreateParams = &ws;
    HWND hDMWnd = CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
    CHECK_MSG(hDMWnd != NULL, _T("Error CreateWindowEx"));

    g_hMsgWnd = hDMWnd;
    //ShowWindow(hDMWnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

}
