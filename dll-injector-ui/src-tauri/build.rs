fn main() {
    // No need to link the DLL, we'll load it dynamically at runtime
    tauri_build::build()
}
