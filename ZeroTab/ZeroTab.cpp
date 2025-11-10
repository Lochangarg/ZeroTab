#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <algorithm>
#include <vector>

// Core Windows processes that must never be killed
std::vector<std::wstring> systemProcesses = {
    L"system",
    L"wininit.exe",
    L"winlogon.exe",
    L"csrss.exe",
    L"services.exe",
    L"svchost.exe",
    L"lsass.exe",
    L"smss.exe",
    L"dwm.exe",
    L"fontdrvhost.exe"
};

std::wstring toLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    return s;
}

bool isSystemProcess(const std::wstring& name) {
    std::wstring lower = toLower(name);
    for (auto& p : systemProcesses)
        if (lower == p) return true;
    return false;
}

void closeAllUserApps() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe{ sizeof(pe) };
    if (Process32FirstW(snap, &pe)) {
        do {
            std::wstring exeName = pe.szExeFile;

            // Skip system processes and this program itself
            if (isSystemProcess(exeName) || pe.th32ProcessID == GetCurrentProcessId())
                continue;

            HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (h) {
                TerminateProcess(h, 0);
                CloseHandle(h);
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 0x5A)) { // Ctrl + Alt + Z
        MessageBox(NULL, L"Failed to register hotkey. Run as Administrator.", L"ZeroTab", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1)
            closeAllUserApps();
    }

    UnregisterHotKey(NULL, 1);
    return 0;
}