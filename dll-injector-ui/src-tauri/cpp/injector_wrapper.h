#ifndef INJECTOR_WRAPPER_H
#define INJECTOR_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Injection result structure
typedef struct {
    bool success;
    void* module_handle;
} InjectionResult;

// Inject a DLL into a target process
// Returns InjectionResult with success status and module handle
InjectionResult inject_dll(uint32_t process_id, const wchar_t* dll_path);

// Eject/Unload a DLL from a target process
// Returns true if successful, false otherwise
bool eject_dll(uint32_t process_id, void* module_handle);

// Find a module handle in a process by path or name
// Returns module handle (HMODULE) or NULL if not found
void* find_module(uint32_t process_id, const wchar_t* module_reference);

#ifdef __cplusplus
}
#endif

#endif // INJECTOR_WRAPPER_H
