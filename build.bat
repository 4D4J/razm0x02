@echo off
REM  DLL Injector 
echo.
echo  DLL INJECTOR
echo.


set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo [ERROR] Visual Studio introuvable
    echo Installez Visual Studio 2022 Build Tools
    pause
    exit /b 1
)


for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo [ERROR] Visual Studio Build Tools non trouvés
    exit /b 1
)

set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars32.bat"

if not exist "%VCVARS%" (
    echo [ERROR] vcvars32.bat introuvable
    exit /b 1
)

echo Visual Studio: %VS_PATH%
echo Initialisation de l'environnement de build...
echo.

call "%VCVARS%" >nul 2>&1

if not exist "build" mkdir build
if not exist "dll-injector-ui\src-tauri\build" mkdir dll-injector-ui\src-tauri\build

echo ========================================
echo  COMPILATION 1/2: EXE Principal
echo ========================================
echo.

cl.exe /EHsc /O2 /W3 /std:c++17 /DUNICODE /D_UNICODE /nologo ^
    /I src\header ^
    src\code\injector_main.cpp ^
    src\code\process_finder.cpp ^
    src\code\dll_injector.cpp ^
    src\code\dll_ejector.cpp ^
    src\code\ui_controls.cpp ^
    src\code\ui_handlers.cpp ^
    src\code\ui_helpers.cpp ^
    /Fe:build\injector.exe ^
    /link kernel32.lib user32.lib advapi32.lib comctl32.lib gdi32.lib comdlg32.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation EXE echouee
    exit /b 1
)

del *.obj >nul 2>&1

echo.
echo  COMPILATION EXE REUSSIE !
echo.
echo EXE cree: build\injector.exe
for %%A in (build\injector.exe) do echo Taille: %%~zA bytes
echo.

echo ========================================
echo  COMPILATION 2/2: Wrapper DLL pour Tauri
echo ========================================
echo.

cl.exe /LD /EHsc /O2 /W3 /std:c++17 /DUNICODE /D_UNICODE /nologo ^
    /I src\header ^
    /I dll-injector-ui\src-tauri\cpp ^
    dll-injector-ui\src-tauri\cpp\injector_wrapper.cpp ^
    src\code\process_finder.cpp ^
    src\code\dll_injector.cpp ^
    src\code\dll_ejector.cpp ^
    /Fe:dll-injector-ui\src-tauri\build\injector.dll ^
    /link kernel32.lib user32.lib advapi32.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation DLL echouee
    exit /b 1
)

del *.obj >nul 2>&1
del *.exp >nul 2>&1
del *.lib >nul 2>&1

echo.
echo  COMPILATION DLL REUSSIE !
echo.
echo DLL creee: dll-injector-ui\src-tauri\build\injector.dll
for %%A in (dll-injector-ui\src-tauri\build\injector.dll) do echo Taille: %%~zA bytes
echo.

echo ========================================
echo  COMPILATION TERMINEE AVEC SUCCES !
echo ========================================
echo.

