#include "dll_injector.h"
#include <sstream>

namespace injector {

InjectionResult InjectDLL(DWORD processId, const std::wstring& dllPath, const Logger& logger) {
    if (logger) {
        logger(L"Checking DLL path...");
    }

    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (logger) {
            logger(L"DLL not found: " + dllPath);
        }
        return {};
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"Opening process (PID: " << processId << L")...";
        logger(ss.str());
    }

    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                                     PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                                 FALSE, processId);

    if (!process) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Failed to open process (code: " << GetLastError() << L")";
            logger(ss.str());
            logger(L"Try running as administrator if needed.");
        }
        return {};
    }

    if (logger) {
        logger(L"Process opened.");
    }

    const SIZE_T dllPathSize = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID remoteMemory = VirtualAllocEx(process, nullptr, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMemory) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Remote allocation failed (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        CloseHandle(process);
        return {};
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"Remote memory reserved at 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(remoteMemory);
        logger(ss.str());
    }

    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(process, remoteMemory, dllPath.c_str(), dllPathSize, &bytesWritten)) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Failed to write DLL path (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return {};
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"DLL path written (" << (bytesWritten / sizeof(wchar_t)) << L" characters).";
        logger(ss.str());
    }

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    FARPROC loadLibrary = kernel32 ? GetProcAddress(kernel32, "LoadLibraryW") : nullptr;

    if (!loadLibrary) {
        if (logger) {
            logger(L"LoadLibraryW not found.");
        }
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return {};
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"LoadLibraryW located at 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(loadLibrary);
        logger(ss.str());
        logger(L"Creating remote thread...");
    }

    HANDLE remoteThread = CreateRemoteThread(process, nullptr, 0,
                                             reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibrary),
                                             remoteMemory, 0, nullptr);

    if (!remoteThread) {
        if (logger) {
            std::wstringstream ss;
            ss << L"Remote thread creation failed (code: " << GetLastError() << L")";
            logger(ss.str());
        }
        VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(process);
        return {};
    }

    if (logger) {
        logger(L"Remote thread created, waiting for result...");
    }

    const DWORD waitResult = WaitForSingleObject(remoteThread, 5000);
    if (waitResult == WAIT_TIMEOUT && logger) {
        logger(L"Remote thread wait timed out after 5s.");
    }

    DWORD exitCode = 0;
    GetExitCodeThread(remoteThread, &exitCode);

    InjectionResult result;

    if (exitCode == 0) {
        if (logger) {
            logger(L"LoadLibraryW returned NULL. Injection most likely failed.");
        }
    } else if (logger) {
        std::wstringstream ss;
        ss << L"DLL loaded at 0x" << std::hex << std::uppercase << exitCode;
        logger(ss.str());
        result.success = true;
        result.moduleHandle = reinterpret_cast<HMODULE>(exitCode);
    }

    CloseHandle(remoteThread);
    VirtualFreeEx(process, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(process);

    return result;
}

}
