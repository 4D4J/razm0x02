#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace injector {

using Logger = std::function<void(const std::wstring&)>;

HMODULE FindModuleHandle(DWORD processId, const std::wstring& moduleReference, const Logger& logger);

bool EjectDLL(DWORD processId, HMODULE moduleHandle, const Logger& logger);

}
