// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>
#include <atlbase.h>
#include <memory>

extern HINSTANCE g_hDllInstance;
extern const LPCTSTR g_lpstrTitle;
extern const LPCTSTR g_lpstrClass;

#define CD_DESKTOPS							0x0100

enum class Message
{
    Query,
    Select,
};

struct MessageStruct
{
    UINT type;
    Message msg;
};

extern HWND g_hMsgWnd;

void ReportError(LPCTSTR msg);

#define CHECK_HR_MSG(x, msg) if (FAILED(x)) { ReportError(_T("Check failed: ") msg); return; }
#define CHECK_HR(x) CHECK_HR_MSG(x, _CRT_WIDE(#x))
#define CHECK_HR_MSG_RET(x, msg, r) if (FAILED(x)) { ReportError(_T("Check failed: ") msg); return r; }
#define CHECK_HR_RET(x, r) CHECK_HR_MSG_RET(x, _CRT_WIDE(#x), r)
#define CHECK_MSG(x, msg) if (!(x)) { ReportError(msg); return; }
#define CHECK(x) CHECK_MSG(x, _CRT_WIDE(#x))
// No return
#define VALIDATE(x, msg) if (!(x)) { ReportError(msg); }
#define VALIDATEE(x) VALIDATE(x, _T("Error: ") _CRT_WIDE(#x))

#endif //PCH_H
