#pragma once

#include <functional>
#include <string>
#include <windows.h>

namespace injector {

using Logger = std::function<void(const std::wstring&)>;

struct InjectionResult {
    bool success = false;
    HMODULE moduleHandle = nullptr;
};

bool ResolveTargetProcess(const std::wstring& rawTarget, DWORD& processId, const Logger& logger);
HMODULE FindModuleHandle(DWORD processId, const std::wstring& moduleReference, const Logger& logger);
InjectionResult InjectDLL(DWORD processId, const std::wstring& dllPath, const Logger& logger);
bool EjectDLL(DWORD processId, HMODULE moduleHandle, const Logger& logger);

}
