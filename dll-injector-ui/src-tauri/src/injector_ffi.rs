use std::ffi::c_void;

#[repr(C)]
pub struct InjectionResult {
    pub success: bool,
    pub module_handle: *mut c_void,
}

// Load DLL dynamically at runtime
use std::sync::OnceLock;
use libloading::{Library, Symbol};

static INJECTOR_LIB: OnceLock<Library> = OnceLock::new();

fn get_library() -> &'static Library {
    INJECTOR_LIB.get_or_init(|| unsafe {
        // Try multiple locations for the DLL
        let dll_path = if let Ok(exe_dir) = std::env::current_exe() {
            // Production: DLL is bundled next to the EXE
            exe_dir.parent()
                .map(|p| p.join("injector.dll"))
                .filter(|p| p.exists())
                .or_else(|| {
                    // Development: DLL is in build directory
                    std::env::current_dir()
                        .ok()
                        .map(|p| p.join("build").join("injector.dll"))
                        .filter(|p| p.exists())
                })
        } else {
            // Fallback to build directory
            std::env::current_dir()
                .ok()
                .map(|p| p.join("build").join("injector.dll"))
                .filter(|p| p.exists())
        };
        
        let dll_path = dll_path.expect(
            "Failed to find injector.dll - make sure to run build.bat first"
        );
        
        Library::new(&dll_path)
            .unwrap_or_else(|e| panic!("Failed to load injector.dll from {:?}: {}", dll_path, e))
    })
}

pub unsafe fn inject_dll(process_id: u32, dll_path: *const u16) -> InjectionResult {
    type InjectFn = unsafe extern "C" fn(u32, *const u16) -> InjectionResult;
    
    let lib = get_library();
    let inject_fn: Symbol<InjectFn> = lib.get(b"inject_dll")
        .expect("Failed to find inject_dll function");
    
    inject_fn(process_id, dll_path)
}

pub unsafe fn eject_dll(process_id: u32, module_handle: *mut c_void) -> bool {
    type EjectFn = unsafe extern "C" fn(u32, *mut c_void) -> bool;
    
    let lib = get_library();
    let eject_fn: Symbol<EjectFn> = lib.get(b"eject_dll")
        .expect("Failed to find eject_dll function");
    
    eject_fn(process_id, module_handle)
}

pub unsafe fn find_module(process_id: u32, module_reference: *const u16) -> *mut c_void {
    type FindModuleFn = unsafe extern "C" fn(u32, *const u16) -> *mut c_void;
    
    let lib = get_library();
    let find_fn: Symbol<FindModuleFn> = lib.get(b"find_module")
        .expect("Failed to find find_module function");
    
    find_fn(process_id, module_reference)
}
