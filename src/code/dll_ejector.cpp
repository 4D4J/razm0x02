#include "dll_ejector.h"
#include "injector_utils.h"
#include <tlhelp32.h>
#include <sstream>

namespace injector {

HMODULE FindModuleHandle(DWORD processId, const std::wstring& moduleReference, const Logger& logger) {
    const std::wstring targetFull = utils::ToLower(moduleReference);
    const std::wstring targetName = utils::ToLower(utils::ExtractFileName(moduleReference));

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Unable to enumerate modules (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        return nullptr;
    }

    MODULEENTRY32W moduleEntry = {};
    moduleEntry.dwSize = sizeof(moduleEntry);

    HMODULE result = nullptr;

    if (Module32FirstW(snapshot, &moduleEntry)) {
        do {
            const std::wstring currentName = utils::ToLower(moduleEntry.szModule);
            const std::wstring currentPath = utils::ToLower(moduleEntry.szExePath);

            if ((!targetName.empty() && currentName == targetName) ||
                (!targetFull.empty() && currentPath == targetFull)) {
                result = moduleEntry.hModule;
                break;
            }
        } while (Module32NextW(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);

    if (!result && logger) {
        logger(L"Module not found in target process.");
    }

    return result;
}

bool EjectDLL(DWORD processId, HMODULE moduleHandle, const Logger& logger) {
    if (!moduleHandle) {
        return false;
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"Opening process for unload (PID: " << processId << L")...";
        logger(ss.str());
    }

    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION,
                                 FALSE, processId);
    if (!process) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Failed to open process (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        return false;
    }

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    FARPROC freeLibrary = kernel32 ? GetProcAddress(kernel32, "FreeLibrary") : nullptr;
    if (!freeLibrary) {
        if (logger) {
            logger(L"FreeLibrary not found.");
        }
        CloseHandle(process);
        return false;
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"FreeLibrary located at 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(freeLibrary);
        logger(ss.str());
    }

    if (logger) {
        logger(L"Creating remote thread for unload...");
    }

    HANDLE remoteThread = CreateRemoteThread(process, nullptr, 0,
                                             reinterpret_cast<LPTHREAD_START_ROUTINE>(freeLibrary),
                                             moduleHandle, 0, nullptr);
    if (!remoteThread) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Remote thread creation failed (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        CloseHandle(process);
        return false;
    }

    const DWORD waitResult = WaitForSingleObject(remoteThread, 5000);
    if (waitResult == WAIT_TIMEOUT && logger) {
        logger(L"Unload wait timed out after 5s.");
    }

    DWORD exitCode = 0;
    GetExitCodeThread(remoteThread, &exitCode);

    CloseHandle(remoteThread);
    CloseHandle(process);

    if (exitCode == 0) {
        if (logger) {
            logger(L"FreeLibrary returned 0. Unload failed.");
        }
        return false;
    }

    if (logger) {
        logger(L"DLL unloaded successfully.");
    }

    return true;
}

} 
