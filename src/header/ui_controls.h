#pragma once

#include <windows.h>

namespace ui {

constexpr int kTargetEditId = 1001;
constexpr int kDllEditId = 1002;
constexpr int kBrowseButtonId = 1003;
constexpr int kInjectButtonId = 1004;
constexpr int kUnloadButtonId = 1005;
constexpr int kLogEditId = 1006;

struct UIControls {
    HWND targetEdit = nullptr;
    HWND dllEdit = nullptr;
    HWND unloadButton = nullptr;
    HWND logEdit = nullptr;
    HFONT font = nullptr;
};

UIControls* GetState(HWND window);

void InitializeControls(HWND window, HINSTANCE instance);

void DestroyControls(HWND window);

}
