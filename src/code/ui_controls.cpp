#include "ui_controls.h"
#include "ui_helpers.h"

namespace ui {

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

    HWND labelDll = CreateWindowExW(0, L"STATIC", L"DLL to inject",  WS_CHILD | WS_VISIBLE, 20, 80, 200, 20, window, nullptr, instance, nullptr);

    state->dllEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 20, 102, 360, 24, window, reinterpret_cast<HMENU>(kDllEditId), instance, nullptr);

    HWND browseButton = CreateWindowExW(0, L"BUTTON", L"Browse...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 100, 110, 28, window, reinterpret_cast<HMENU>(kBrowseButtonId), instance, nullptr);

    HWND injectButton = CreateWindowExW(0, L"BUTTON", L"Inject", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 40, 110, 28, window, reinterpret_cast<HMENU>(kInjectButtonId), instance, nullptr);

    state->unloadButton = CreateWindowExW(0, L"BUTTON", L"Unload", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 390, 70, 110, 28, window, reinterpret_cast<HMENU>(kUnloadButtonId), instance, nullptr);

    state->logEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 20, 150, 480, 180,window, reinterpret_cast<HMENU>(kLogEditId), instance, nullptr);

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

}