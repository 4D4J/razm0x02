#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace injector {

using Logger = std::function<void(const std::wstring&)>;

struct InjectionResult {
    bool success = false;
    HMODULE moduleHandle = nullptr;
};

InjectionResult InjectDLL(DWORD processId, const std::wstring& dllPath, const Logger& logger);

} 
