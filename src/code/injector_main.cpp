
#include <windows.h>
#include "ui_controls.h"
#include "ui_handlers.h"

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const auto* createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
            ui::InitializeControls(window, createStruct->hInstance);
            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ui::kBrowseButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        ui::HandleBrowse(window);
                    }
                    return 0;
                case ui::kInjectButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        ui::HandleInject(window);
                    }
                    return 0;
                case ui::kUnloadButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        ui::HandleUnload(window);
                    }
                    return 0;
                default:
                    break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_NCDESTROY:
            ui::DestroyControls(window);
            break;
        default:
            break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    const wchar_t className[] = L"UniversalDllInjectorWindow";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = className;

    if (!RegisterClassExW(&windowClass)) {
        MessageBoxW(nullptr, L"Failed to register the main window class.", L"Error", MB_ICONERROR);
        return 0;
    }

    HWND window = CreateWindowExW(0, className, L"Universal DLL Injector",
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 540, 380,
                                  nullptr, nullptr, instance, nullptr);

    if (!window) {
        MessageBoxW(nullptr, L"Failed to create the main window.", L"Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
