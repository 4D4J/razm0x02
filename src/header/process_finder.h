#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace injector {

using Logger = std::function<void(const std::wstring&)>;

DWORD FindProcessByName(const std::wstring& processName);

bool ResolveTargetProcess(const std::wstring& rawTarget, DWORD& processId, const Logger& logger);

}
