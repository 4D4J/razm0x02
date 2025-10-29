import { useState, useEffect } from "react";
import { getCurrentWindow } from "@tauri-apps/api/window";
import { invoke } from "@tauri-apps/api/core";
import { open } from "@tauri-apps/plugin-dialog";
import "./App.css";

export default function App() {
  const [processTarget, setProcessTarget] = useState("")
  const [dllPath, setDllPath] = useState("")
  const [isAdmin, setIsAdmin] = useState(false)
  const [logs, setLogs] = useState<Array<{ time: string; message: string; type: "info" | "success" | "error" }>>([
    { time: new Date().toLocaleTimeString(), message: "DLL Injector initialized", type: "info" },
  ])

  useEffect(() => {
    // Check admin rights on startup
    invoke<boolean>("is_admin").then((result) => {
      setIsAdmin(result)
      addLog(result ? "Running with administrator privileges" : "Running without administrator privileges", result ? "success" : "error")
    }).catch(() => {
      setIsAdmin(false)
    })
  }, [])

  const addLog = (message: string, type: "info" | "success" | "error" = "info") => {
    const time = new Date().toLocaleTimeString()
    setLogs((prev) => [...prev, { time, message, type }])
  }

  const handleClose = () => {
    getCurrentWindow().close()
  }

  const handleMinimize = () => {
    getCurrentWindow().minimize()
  }

  const handleBrowse = async () => {
    const selected = await open({
      multiple: false,
      filters: [{
        name: 'DLL Files',
        extensions: ['dll']
      }]
    });
    
    if (selected && typeof selected === 'string') {
      setDllPath(selected)
      addLog(`DLL selected: ${selected}`, "info")
    }
  }

  const handleInject = async () => {
    if (!processTarget) {
      addLog("Error: No target process specified", "error")
      return
    }
    if (!dllPath) {
      addLog("Error: No DLL path specified", "error")
      return
    }
    
    // Parse process ID
    const pid = parseInt(processTarget)
    if (isNaN(pid)) {
      addLog(`Error: Invalid process ID. Please enter a numeric PID.`, "error")
      return
    }

    addLog(`Attempting injection into process: ${processTarget}`, "info")
    
    try {
      const result = await invoke<{ success: boolean; module_handle: string; message: string }>(
        "inject_dll_command",
        { processId: pid, dllPath: dllPath }
      )
      
      if (result.success) {
        addLog(result.message, "success")
        addLog(`Module handle: ${result.module_handle}`, "info")
      } else {
        addLog(result.message, "error")
      }
    } catch (error) {
      addLog(`Injection failed: ${error}`, "error")
    }
  }

  const handleUnload = async () => {
    if (!processTarget) {
      addLog("Error: No target process specified", "error")
      return
    }
    if (!dllPath) {
      addLog("Error: No DLL path specified", "error")
      return
    }
    
    // Parse process ID
    const pid = parseInt(processTarget)
    if (isNaN(pid)) {
      addLog(`Error: Invalid process ID. Please enter a numeric PID.`, "error")
      return
    }

    addLog(`Attempting to unload DLL from process: ${processTarget}`, "info")
    
    try {
      const result = await invoke<{ success: boolean; module_handle: string; message: string }>(
        "eject_dll_command",
        { processId: pid, dllPath: dllPath }
      )
      
      if (result.success) {
        addLog(result.message, "success")
      } else {
        addLog(result.message, "error")
      }
    } catch (error) {
      addLog(`Ejection failed: ${error}`, "error")
    }
  }

  return (
    <div className="app-container">
      <div className="card">
        <div className="titlebar" data-tauri-drag-region>
          <div className="titlebar-left">
            <div className={`admin-indicator ${isAdmin ? 'admin' : 'not-admin'}`} title={isAdmin ? "Administrator" : "Not Administrator"}></div>
            <div className="titlebar-title">DLL Injector</div>
          </div>
          <div className="titlebar-buttons">
            <button className="titlebar-button" onClick={handleMinimize}>
              ─
            </button>
            <button className="titlebar-button titlebar-close" onClick={handleClose}>
              ✕
            </button>
          </div>
        </div>

        <div className="input-section">
          <div className="input-group">
            <label htmlFor="process">Target Process</label>
            <div className="input-row">
              <input
                id="process"
                type="text"
                placeholder="Process ID or Name (e.g., 1234 or notepad.exe)"
                value={processTarget}
                onChange={(e) => setProcessTarget(e.target.value)}
              />
              <button className="btn btn-primary" onClick={handleInject}>
                ▶ Inject
              </button>
              <button className="btn btn-danger" onClick={handleUnload}>
                ✕ Unload
              </button>
            </div>
          </div>

          <div className="input-group">
            <label htmlFor="dll-path">DLL Path</label>
            <div className="input-row">
              <input
                id="dll-path"
                type="text"
                placeholder="Path to DLL file"
                value={dllPath}
                onChange={(e) => setDllPath(e.target.value)}
              />
              <button className="btn btn-secondary" onClick={handleBrowse}>
                Browse
              </button>
            </div>
          </div>
        </div>

        <div className="logs-section">
          <label>Console Logs</label>
          <div className="logs">
            {logs.map((log, index) => (
              <div key={index} className={`log-entry log-${log.type}`}>
                <span className="log-time">[{log.time}]</span>
                <span className="log-message">{log.message}</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  )
}
