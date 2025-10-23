#include "process_finder.h"
#include "injector_utils.h"
#include <tlhelp32.h>
#include <sstream>

namespace injector {

DWORD FindProcessByName(const std::wstring& processName) {
    const std::wstring target = utils::ToLower(processName);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            std::wstring current = utils::ToLower(entry.szExeFile);
            if (current == target) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

bool ResolveTargetProcess(const std::wstring& rawTarget, DWORD& processId, const Logger& logger) {
    if (utils::TryParseProcessId(rawTarget, processId)) {
        if (logger) {
            std::wstringstream ss;
            ss << L"PID mode detected: " << processId;
            logger(ss.str());
        }
        return true;
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"Searching process by name: " << rawTarget;
        logger(ss.str());
    }

    processId = FindProcessByName(rawTarget);
    if (processId == 0) {
        if (logger) {
            logger(L"Process not found.");
        }
        return false;
    }

    if (logger) {
        std::wstringstream ss;
        ss << L"Process found (PID: " << processId << L").";
        logger(ss.str());
    }
    return true;
}

} 
