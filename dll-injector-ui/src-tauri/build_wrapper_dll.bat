@echo off
echo Building injector wrapper DLL...

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Compile the wrapper DLL
cl.exe /LD /EHsc /std:c++17 ^
    /I "../../src/header" ^
    /I "cpp" ^
    cpp/injector_wrapper.cpp ^
    ../../src/code/dll_injector.cpp ^
    ../../src/code/dll_ejector.cpp ^
    ../../src/code/process_finder.cpp ^
    /Fe:build/injector.dll ^
    /link /DEF:NONE

if %ERRORLEVEL% EQU 0 (
    echo.
    echo DLL SUCCESSFULLY BUILT!
    echo Output: build\injector.dll
    dir build\injector.dll | findstr /C:"injector.dll"
) else (
    echo.
    echo BUILD FAILED!
    exit /b 1
)
