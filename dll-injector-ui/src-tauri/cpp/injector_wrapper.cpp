#include "injector_wrapper.h"
#include "../../src/header/dll_injector.h"
#include "../../src/header/dll_ejector.h"
#include <string>

// Simple logger that does nothing (we'll handle logging in Rust/JS)
static void null_logger(const std::wstring&) {}

extern "C" {

InjectionResult inject_dll(uint32_t process_id, const wchar_t* dll_path) {
    InjectionResult result = {false, nullptr};
    
    if (!dll_path) {
        return result;
    }

    std::wstring dll_path_str(dll_path);
    auto inject_result = injector::InjectDLL(process_id, dll_path_str, null_logger);
    
    result.success = inject_result.success;
    result.module_handle = inject_result.moduleHandle;
    
    return result;
}

bool eject_dll(uint32_t process_id, void* module_handle) {
    if (!module_handle) {
        return false;
    }
    
    return injector::EjectDLL(process_id, static_cast<HMODULE>(module_handle), null_logger);
}

void* find_module(uint32_t process_id, const wchar_t* module_reference) {
    if (!module_reference) {
        return nullptr;
    }
    
    std::wstring module_ref_str(module_reference);
    return injector::FindModuleHandle(process_id, module_ref_str, null_logger);
}

}
