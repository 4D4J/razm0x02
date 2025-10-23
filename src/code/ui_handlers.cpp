#include "ui_handlers.h"
#include "ui_controls.h"
#include "ui_helpers.h"
#include "injector_engine.h"
#include "injector_utils.h"
#include <sstream>

namespace ui {

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

    const std::wstring rawTarget = injector::utils::Trim(GetControlText(state->targetEdit));
    const std::wstring dllPath = injector::utils::Trim(GetControlText(state->dllEdit));

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

    const std::wstring rawTarget = injector::utils::Trim(GetControlText(state->targetEdit));
    const std::wstring dllPath = injector::utils::Trim(GetControlText(state->dllEdit));

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
