#include <windows.h>
#include <commdlg.h>
#include <string>
#include <sstream>

// header
#include "injector_engine.h"
#include "injector_utils.h"

namespace
{
    namespace utils = injector::utils;
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

    std::wstring GetControlText(HWND control) {
        if (!control) {
            return L"";
        }

        const int length = GetWindowTextLengthW(control);
        if (length <= 0) {
            return L"";
        }

        std::wstring buffer(static_cast<size_t>(length) + 1, L'\0');
        const int written = GetWindowTextW(control, buffer.data(), length + 1);
        if (written <= 0) {
            return L"";
        }

        buffer.resize(static_cast<size_t>(written));
        return buffer;
    }

    void AppendLog(HWND logEdit, const std::wstring& message) {
        if (!logEdit) {
            return;
        }

        const int textLength = GetWindowTextLengthW(logEdit);
        SendMessageW(logEdit, EM_SETSEL, textLength, textLength);

        std::wstring line = message;
        if (!line.empty() && line.back() != L'\n') {
            line.append(L"\r\n");
        } else if (line.empty()) {
            line.assign(L"\r\n");
        }

        SendMessageW(logEdit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
        SendMessageW(logEdit, EM_SCROLLCARET, 0, 0);
    }

    bool IsRunningAsAdmin() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = nullptr;
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            if (!CheckTokenMembership(nullptr, adminGroup, &isAdmin)) {
                isAdmin = FALSE;
            }
        }

        if (adminGroup) {
            FreeSid(adminGroup);
        }

        return isAdmin == TRUE;
    }

    std::wstring OpenDllFileDialog(HWND owner) {
        wchar_t fileBuffer[MAX_PATH] = L"";
        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = L"Dynamic libraries (*.dll)\0*.dll\0All files (*.*)\0*.*\0";
        ofn.lpstrFile = fileBuffer;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
        ofn.lpstrDefExt = L"dll";

        if (GetOpenFileNameW(&ofn)) {
            return fileBuffer;
        }

        return L"";
    }

    UIControls* GetState(HWND window) {
        return reinterpret_cast<UIControls*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    void InitializeControls(HWND window, HINSTANCE instance) {
        auto* state = new UIControls();
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

        HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        state->font = font;

    HWND labelTarget = CreateWindowExW(0, L"STATIC", L"Target (name or PID)", WS_CHILD | WS_VISIBLE, 20, 20, 200, 20, window, nullptr, instance, nullptr);

        state->targetEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 20, 42, 360, 24, window, reinterpret_cast<HMENU>(kTargetEditId), instance, nullptr);

    HWND labelDll = CreateWindowExW(0, L"STATIC", L"DLL to inject", WS_CHILD | WS_VISIBLE, 20, 80, 200, 20, window, nullptr, instance, nullptr);

        state->dllEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 20, 102, 360, 24, window, reinterpret_cast<HMENU>(kDllEditId), instance, nullptr);

    HWND browseButton = CreateWindowExW(0, L"BUTTON", L"Browse...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 100, 110, 28, window, reinterpret_cast<HMENU>(kBrowseButtonId), instance, nullptr);

    HWND injectButton = CreateWindowExW(0, L"BUTTON", L"Inject", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 40, 110, 28, window, reinterpret_cast<HMENU>(kInjectButtonId), instance, nullptr);

    state->unloadButton = CreateWindowExW(0, L"BUTTON", L"Unload", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 70, 110, 28, window, reinterpret_cast<HMENU>(kUnloadButtonId), instance, nullptr);

        state->logEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 20, 150, 480, 180, window, reinterpret_cast<HMENU>(kLogEditId), instance, nullptr);

        SendMessageW(labelTarget, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(labelDll, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(state->targetEdit, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(state->dllEdit, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(browseButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(injectButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(state->unloadButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(state->logEdit, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

        AppendLog(state->logEdit, L"Ready.");
        if (IsRunningAsAdmin()) {
            AppendLog(state->logEdit, L"Administrator rights detected.");
        } else {
            AppendLog(state->logEdit, L"Warning: run as administrator for best results.");
        }

        SendMessageW(state->logEdit, EM_SETLIMITTEXT, 256 * 1024, 0);
        SetFocus(state->targetEdit);
    }

    void DestroyControls(HWND window) {
        auto* state = GetState(window);
        if (!state) {
            return;
        }

        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
        delete state;
    }

    void HandleBrowse(HWND window) {
        auto* state = GetState(window);
        if (!state) {
            return;
        }

        const std::wstring selected = OpenDllFileDialog(window);
        if (!selected.empty()) {
            SetWindowTextW(state->dllEdit, selected.c_str());
        }
    }

    void HandleInject(HWND window) {
        auto* state = GetState(window);
        if (!state) {
            return;
        }

        const std::wstring rawTarget = utils::Trim(GetControlText(state->targetEdit));
        const std::wstring dllPath = utils::Trim(GetControlText(state->dllEdit));

        if (rawTarget.empty()) {
            MessageBoxW(window, L"Please specify a process name or PID.", L"Information", MB_ICONINFORMATION);
            return;
        }

        if (dllPath.empty()) {
            MessageBoxW(window, L"Please select the DLL to inject.", L"Information", MB_ICONINFORMATION);
            return;
        }

        AppendLog(state->logEdit, L"---");
        AppendLog(state->logEdit, L"Target: " + rawTarget);
        AppendLog(state->logEdit, L"DLL: " + dllPath);

        const injector::Logger logFn = [state](const std::wstring& entry) {
            AppendLog(state->logEdit, entry);
        };

        DWORD processId = 0;
        if (!injector::ResolveTargetProcess(rawTarget, processId, logFn)) {
            MessageBoxW(window, L"Process not found.", L"Error", MB_ICONERROR);
            return;
        }

        AppendLog(state->logEdit, L"Starting injection...");

        const injector::InjectionResult result = injector::InjectDLL(processId, dllPath, logFn);

        if (result.success) {
            AppendLog(state->logEdit, L"Injection completed successfully.");
            if (result.moduleHandle) {
                std::wstringstream ss;
                ss << L"Module handle: 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(result.moduleHandle);
                AppendLog(state->logEdit, ss.str());
            }
            MessageBoxW(window, L"DLL injected successfully.", L"Success", MB_ICONINFORMATION);
        } else {
            AppendLog(state->logEdit, L"Injection failed.");
            MessageBoxW(window, L"Unable to inject the DLL. Check the log for details.", L"Failure", MB_ICONERROR);
        }
    }

    void HandleUnload(HWND window) {
        auto* state = GetState(window);
        if (!state) {
            return;
        }

        const std::wstring rawTarget = utils::Trim(GetControlText(state->targetEdit));
        const std::wstring dllPath = utils::Trim(GetControlText(state->dllEdit));

        if (rawTarget.empty() || dllPath.empty()) {
            MessageBoxW(window, L"Provide the target and DLL first.", L"Information", MB_ICONINFORMATION);
            return;
        }

        AppendLog(state->logEdit, L"---");
        AppendLog(state->logEdit, L"Requested unload for: " + dllPath);

        const injector::Logger logFn = [state](const std::wstring& entry) {
            AppendLog(state->logEdit, entry);
        };

        DWORD processId = 0;
        if (!injector::ResolveTargetProcess(rawTarget, processId, logFn)) {
            MessageBoxW(window, L"Process not found.", L"Error", MB_ICONERROR);
            return;
        }

        const HMODULE moduleHandle = injector::FindModuleHandle(processId, dllPath, logFn);
        if (!moduleHandle) {
            MessageBoxW(window, L"Module not found in target process.", L"Error", MB_ICONERROR);
            return;
        }

        std::wstringstream ss;
        ss << L"Found module handle 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(moduleHandle);
        AppendLog(state->logEdit, ss.str());
        AppendLog(state->logEdit, L"Starting unload...");

        const bool success = injector::EjectDLL(processId, moduleHandle, logFn);

        if (success) {
            AppendLog(state->logEdit, L"Unload completed successfully.");
            MessageBoxW(window, L"DLL unloaded.", L"Success", MB_ICONINFORMATION);
        } else {
            AppendLog(state->logEdit, L"Unload failed.");
            MessageBoxW(window, L"Unable to unload the DLL. Check the log for details.", L"Failure", MB_ICONERROR);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            const auto* createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
            InitializeControls(window, createStruct->hInstance);
            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case kBrowseButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        HandleBrowse(window);
                    }
                    return 0;
                case kInjectButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        HandleInject(window);
                    }
                    return 0;
                case kUnloadButtonId:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        HandleUnload(window);
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
            DestroyControls(window);
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

    HWND window = CreateWindowExW(0, className, L"Universal DLL Injector", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 540, 380, nullptr, nullptr, instance, nullptr);

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
