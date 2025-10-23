#pragma once

#include <windows.h>
#include <string>

namespace ui {

std::wstring GetControlText(HWND control);

void AppendLog(HWND logEdit, const std::wstring& message);

bool IsRunningAsAdmin();

std::wstring OpenDllFileDialog(HWND owner);

}
