@echo off
echo ========================================
echo Taiwan Railway ATP System Builder
echo ========================================

REM 檢查是否在正確的目錄
if not exist "DMI\CMakeLists.txt" (
    echo Error: Please run this script from the ETCS root directory
    echo Current directory should contain DMI and EVC folders
    pause
    exit /b 1
)

REM 檢查 CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake not found in PATH
    echo Please install CMake and add it to your PATH
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM 檢查 Visual Studio
where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Warning: Visual Studio compiler not found in PATH
    echo Please run this script from Visual Studio Developer Command Prompt
    echo Or install Visual Studio with C++ development tools
    echo.
    echo Trying to find Visual Studio...
    
    REM 嘗試找到 Visual Studio
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        echo Found Visual Studio 2019 Community
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        echo Found Visual Studio 2022 Community
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    ) else (
        echo Visual Studio not found automatically
        echo Please install Visual Studio or run from Developer Command Prompt
        pause
        exit /b 1
    )
)

echo.
echo Checking for dependencies...

REM 檢查是否有 vcpkg
where vcpkg >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found vcpkg, installing dependencies...
    vcpkg install sdl2:x64-windows sdl2-ttf:x64-windows nlohmann-json:x64-windows
    set USE_VCPKG=1
) else (
    echo vcpkg not found, will try to use system libraries
    set USE_VCPKG=0
)

echo.
echo Creating build directory...
if exist build rmdir /s /q build
mkdir build
cd build

echo.
echo Configuring CMake for Taiwan Railway ATP...

if %USE_VCPKG%==1 (
    echo Using vcpkg toolchain...
    cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DCMAKE_BUILD_TYPE=Release
) else (
    echo Using system libraries...
    cmake .. -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DETCS_VENDORED=OFF -DRADIO_CFM=OFF -DCMAKE_BUILD_TYPE=Release
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed!
    echo.
    echo Possible solutions:
    echo 1. Install SDL2 and SDL2_ttf development libraries
    echo 2. Use vcpkg: vcpkg install sdl2:x64-windows sdl2-ttf:x64-windows
    echo 3. Check INSTALL_DEPENDENCIES_WINDOWS.md for detailed instructions
    echo.
    pause
    exit /b 1
)

echo.
echo Building Taiwan Railway ATP System...
cmake --build . --config Release --parallel

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build Successful!
    echo ========================================
    echo.
    echo Taiwan Railway ATP system has been built successfully.
    echo.
    echo To run the system:
    echo   cd DMI\Release
    echo   dmi.exe
    echo.
    echo Configuration files:
    echo   - config.json (system configuration)
    echo   - DMI\settings.ini (display settings)
    echo   - DMI\tra_atp_layout.json (ATP layout)
    echo.
    echo For more information, see README_TRA_ATP.md
    echo.
) else (
    echo.
    echo ========================================
    echo Build Failed!
    echo ========================================
    echo.
    echo Please check the error messages above.
    echo Common issues:
    echo 1. Missing dependencies (SDL2, SDL2_ttf)
    echo 2. Compiler not found (install Visual Studio)
    echo 3. CMake version too old (need 3.14+)
    echo.
    echo See INSTALL_DEPENDENCIES_WINDOWS.md for help
    echo.
)

pause