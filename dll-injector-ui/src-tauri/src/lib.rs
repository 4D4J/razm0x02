mod injector_ffi;

use std::os::windows::ffi::OsStrExt;
use std::ffi::OsStr;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
struct InjectionResponse {
    success: bool,
    module_handle: String,
    message: String,
}

// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[tauri::command]
fn inject_dll_command(process_id: u32, dll_path: String) -> InjectionResponse {
    // Validate DLL path exists
    if !std::path::Path::new(&dll_path).exists() {
        return InjectionResponse {
            success: false,
            module_handle: String::new(),
            message: format!("DLL file not found: {}", dll_path),
        };
    }

    let dll_path_wide: Vec<u16> = OsStr::new(&dll_path)
        .encode_wide()
        .chain(std::iter::once(0))
        .collect();

    unsafe {
        let result = injector_ffi::inject_dll(process_id, dll_path_wide.as_ptr());
        
        if result.success {
            InjectionResponse {
                success: true,
                module_handle: format!("{:?}", result.module_handle),
                message: format!("Successfully injected DLL into process {}", process_id),
            }
        } else {
            InjectionResponse {
                success: false,
                module_handle: String::new(),
                message: "Failed to inject DLL. Check if the process exists and you have admin rights.".to_string(),
            }
        }
    }
}

#[tauri::command]
fn eject_dll_command(process_id: u32, dll_path: String) -> InjectionResponse {
    let dll_path_wide: Vec<u16> = OsStr::new(&dll_path)
        .encode_wide()
        .chain(std::iter::once(0))
        .collect();

    unsafe {
        // Find the module first
        let module_handle = injector_ffi::find_module(process_id, dll_path_wide.as_ptr());
        
        if module_handle.is_null() {
            return InjectionResponse {
                success: false,
                module_handle: String::new(),
                message: "DLL not found in target process".to_string(),
            };
        }

        let success = injector_ffi::eject_dll(process_id, module_handle);
        
        if success {
            InjectionResponse {
                success: true,
                module_handle: String::new(),
                message: format!("Successfully ejected DLL from process {}", process_id),
            }
        } else {
            InjectionResponse {
                success: false,
                module_handle: String::new(),
                message: "Failed to eject DLL".to_string(),
            }
        }
    }
}

#[tauri::command]
fn is_admin() -> bool {
    #[cfg(windows)]
    {
        use winapi::um::processthreadsapi::{GetCurrentProcess, OpenProcessToken};
        use winapi::um::securitybaseapi::GetTokenInformation;
        use winapi::um::winnt::{TokenElevation, TOKEN_ELEVATION, TOKEN_QUERY};
        use std::mem;

        unsafe {
            let mut token_handle = std::ptr::null_mut();
            if OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token_handle) == 0 {
                return false;
            }

            let mut elevation: TOKEN_ELEVATION = mem::zeroed();
            let mut return_length = 0;
            
            if GetTokenInformation(
                token_handle,
                TokenElevation,
                &mut elevation as *mut _ as *mut _,
                mem::size_of::<TOKEN_ELEVATION>() as u32,
                &mut return_length,
            ) == 0 {
                return false;
            }

            elevation.TokenIsElevated != 0
        }
    }
    
    #[cfg(not(windows))]
    {
        false
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![greet, is_admin, inject_dll_command, eject_dll_command])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
