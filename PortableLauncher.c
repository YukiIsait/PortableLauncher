#include <Windows.h>
#include <Shlwapi.h>

static BOOL SetAppDataToCurrentDirectory() {
    // Get the current module path
    WCHAR path[MAX_PATH];
    DWORD size = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (size == 0 || size == MAX_PATH) {
        return FALSE;
    }
    // Remove the filename
    if (PathRemoveFileSpecW(path) == 0) {
        return FALSE;
    }
    // Set the APPDATA environment variable
    if (SetEnvironmentVariableW(L"APPDATA", path) == 0) {
        return FALSE;
    }
    // Set the LOCALAPPDATA environment variable
    if (SetEnvironmentVariableW(L"LOCALAPPDATA", path) == 0) {
        return FALSE;
    }
    return TRUE;
}

static BOOL LaunchProcess(LPCWSTR args) {
    // Get the current module path
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
    // Cleanup
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    HeapFree(heapHandle, 0, commandLine);
    return TRUE;
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
            return 1;
    }
    // Set the AppData and LocalAppData location
    if (!SetAppDataToCurrentDirectory()) {
        return 1;
    }
    // Launch the process
    if (!LaunchProcess(args)) {
        return 1;
    }
    return 0;
}

DWORD EntryPoint() {
    // Get command line arguments
    LPWSTR commandLine = GetCommandLineW();
    if (!*commandLine) {
        return Main(0, &commandLine);
    }
    // Split arguments into an array
    INT argumentCount;
    LPWSTR* argumentVector = CommandLineToArgvW(commandLine, &argumentCount);
    if (!argumentCount) {
        return Main(0, &commandLine);
    }
    // Call the main function
    INT exitCode = Main(argumentCount, argumentVector);
    // Cleanup
    LocalFree(argumentVector);
    return exitCode;
}
