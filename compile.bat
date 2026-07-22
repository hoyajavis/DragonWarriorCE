@echo off
setlocal

echo =========================================
echo  Dragon Warrior Build Script
echo =========================================
echo.

:: Detect CE Toolchain Environment
set CEDEV_PATH=
if exist "C:\CEdev\cedev.bat" set CEDEV_PATH=C:\CEdev
if exist "D:\CEdev\cedev.bat" set CEDEV_PATH=D:\CEdev
if not "%CEDEV%"=="" set CEDEV_PATH=%CEDEV%

if "%CEDEV_PATH%"=="" (
    echo [ERROR] CE C Toolchain not found!
    echo Please install it from https://github.com/CE-Programming/toolchain
    echo to C:\CEdev or D:\CEdev.
    exit /b 1
)

echo [INFO] Found CE Toolchain at %CEDEV_PATH%
set "CEDEV=%CEDEV_PATH%"
set "PATH=%CEDEV%\bin;%PATH%"

echo.
echo [STEP 1] Compiling Graphics (make gfx)...
make gfx
if %errorlevel% neq 0 (
    echo [ERROR] Graphics compilation failed!
    exit /b 1
)

echo.
echo [STEP 2] Building Project (make)...
make
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo =========================================
echo  SUCCESS! 
echo  Your game is ready: bin\PYDW.8xp
echo =========================================
