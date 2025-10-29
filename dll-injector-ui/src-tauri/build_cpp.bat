@echo off
echo Building C++ injector library...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo [ERROR] Visual Studio not found
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo [ERROR] Visual Studio Build Tools not found
    exit /b 1
)

set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS%" (
    echo [ERROR] vcvars64.bat not found
    exit /b 1
)

call "%VCVARS%" >nul 2>&1

if not exist "target\release" mkdir target\release

echo Compiling C++ wrapper...

cl.exe /EHsc /O2 /W3 /std:c++17 /DUNICODE /D_UNICODE /nologo ^
    /I ..\..\src\header ^
    /I cpp ^
    cpp\injector_wrapper.cpp ^
    ..\..\src\code\dll_injector.cpp ^
    ..\..\src\code\dll_ejector.cpp ^
    ..\..\src\code\process_finder.cpp ^
    /LD ^
    /Fe:target\release\injector.dll ^
    /link kernel32.lib advapi32.lib

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed
    exit /b 1
)

del *.obj *.exp *.lib >nul 2>&1

echo [SUCCESS] Library created: target\release\injector.dll
