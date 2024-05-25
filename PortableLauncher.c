#include <Windows.h>
#include <Shlwapi.h>

static BOOL SetAppDataToCurrentDirectory() {
    WCHAR path[MAX_PATH];
    DWORD size = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (size == 0 || size == MAX_PATH) {
        return FALSE;
    }
    if (PathRemoveFileSpecW(path) == 0) {
        return FALSE;
    }
    if (SetEnvironmentVariableW(L"APPDATA", path) == 0) {
        return FALSE;
    }
    if (SetEnvironmentVariableW(L"LOCALAPPDATA", path) == 0) {
        return FALSE;
    }
    return TRUE;
}

static BOOL LaunchProcess(LPCWSTR args) {
    WCHAR path[MAX_PATH];
    DWORD size = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (size == 0 || size == MAX_PATH) {
        return FALSE;
    }
    // Remove the EXE extension
    PathRemoveExtensionW(path);
    // Check if the secondary extension is LAUNCHER
    LPWSTR extension = PathFindExtensionW(path) + 1;
    if (lstrcmpiW(extension, L"launcher") != 0) {
        return FALSE;
    }
    // Replace LAUNCHER with the EXE extension
    lstrcpyW(extension, L"exe");
    // Allocate memory for the command line
    HANDLE heapHandle = GetProcessHeap();
    if (!heapHandle) {
        return FALSE;
    }
    SIZE_T pathSize = lstrlenW(path);
    SIZE_T argsSize = lstrlenW(args);
    SIZE_T allocatedSize = pathSize + argsSize + 1;
    if (argsSize) {
        allocatedSize++;
    }
    LPWSTR commandLine = (LPWSTR) HeapAlloc(heapHandle, 0, allocatedSize * sizeof(WCHAR));
    if (!commandLine) {
        return FALSE;
    }
    // Combine the application name and command line
    lstrcpyW(commandLine, path);
    if (argsSize) {
        lstrcpyW(commandLine + pathSize + 1, args);
        commandLine[pathSize] = 0x20; // Add a space
    }
    // Create the process
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);
    if (!CreateProcessW(path, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return FALSE;
    }
#ifdef _CONSOLE
    // If a console program is launched, wait for it to exit
    WaitForSingleObject(pi.hProcess, INFINITE);
#endif
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    HeapFree(heapHandle, 0, commandLine);
    return TRUE;
}

static void ShowError(LPCWSTR text, LPCWSTR caption) {
#ifdef _CONSOLE
    // Allocate memory for the message
    HANDLE heapHandle = GetProcessHeap();
    if (!heapHandle) {
        return;
    }
    SIZE_T textSize = lstrlenW(text);
    SIZE_T captionSize = lstrlenW(caption);
    SIZE_T allocatedSize = textSize + captionSize + 3;
    LPWSTR message = (LPWSTR) HeapAlloc(heapHandle, 0, allocatedSize * sizeof(WCHAR));
    if (!message) {
        return;
    }
    // Combine the caption and text into the message
    lstrcpyW(message, caption);
    lstrcpyW(message + captionSize + 2, text);
    message[captionSize] = 0x3A;     // Add a colon
    message[captionSize + 1] = 0x20; // Add a space
    // Output the message to the console
    HANDLE console = GetStdHandle(STD_ERROR_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        return;
    }
    WriteConsoleW(console, message, (DWORD) (allocatedSize - 1), NULL, NULL);
    HeapFree(heapHandle, 0, message);
#else
    MessageBoxW(NULL, text, caption, MB_ICONERROR);
#endif
}

static INT Main(INT argumentCount, LPWSTR* argumentVector) {
    // Use the first argument as the command line
    LPCWSTR args;
    switch (argumentCount) {
        case 0:
        case 1:
            args = NULL;
            break;
        case 2:
            args = argumentVector[1];
            break;
        default:
            ShowError(L"Too many arguments provided to the program.", L"Argument Error");
            return 1;
    }
    if (!SetAppDataToCurrentDirectory()) {
        ShowError(L"Failed to set the current directory as AppData or LocalAppData.", L"Environment Variable Error");
        return 1;
    }
    if (!LaunchProcess(args)) {
        ShowError(L"Failed to launch the process.", L"Process Launch Error");
        return 1;
    }
    return 0;
}

DWORD EntryPoint() {
    LPWSTR commandLine = GetCommandLineW();
    if (!*commandLine) {
        return Main(0, &commandLine);
    }
    INT argumentCount;
    LPWSTR* argumentVector = CommandLineToArgvW(commandLine, &argumentCount);
    if (!argumentCount) {
        return Main(0, &commandLine);
    }
    INT exitCode = Main(argumentCount, argumentVector);
    LocalFree(argumentVector);
    return exitCode;
}
