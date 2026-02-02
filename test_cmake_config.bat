@echo off
echo Testing CMake Configuration for Taiwan Railway ATP...

REM 檢查基本工具
echo Checking tools...

where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] CMake not found
    goto :end
) else (
    echo [OK] CMake found
)

where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [WARN] Visual Studio compiler not in PATH
) else (
    echo [OK] Visual Studio compiler found
)

REM 檢查文件結構
echo.
echo Checking project structure...

if not exist "DMI\CMakeLists.txt" (
    echo [FAIL] DMI\CMakeLists.txt not found
    goto :end
) else (
    echo [OK] DMI\CMakeLists.txt found
)

if not exist "EVC\CMakeLists.txt" (
    echo [FAIL] EVC\CMakeLists.txt not found
    goto :end
) else (
    echo [OK] EVC\CMakeLists.txt found
)

if not exist "CMakeLists.txt" (
    echo [FAIL] Root CMakeLists.txt not found
    goto :end
) else (
    echo [OK] Root CMakeLists.txt found
)

REM 檢查台鐵ATP文件
echo.
echo Checking Taiwan Railway ATP files...

if not exist "DMI\graphics\tra_components.h" (
    echo [FAIL] TRA components header not found
    goto :end
) else (
    echo [OK] TRA components found
)

if not exist "DMI\tra_atp_layout.json" (
    echo [WARN] TRA ATP layout file not found
) else (
    echo [OK] TRA ATP layout found
)

if not exist "config.json" (
    echo [WARN] config.json not found
) else (
    echo [OK] config.json found
)

REM 測試CMake配置
echo.
echo Testing CMake configuration...

if exist test-build rmdir /s /q test-build
mkdir test-build
cd test-build

echo Running: cmake .. -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DETCS_VENDORED=OFF -DRADIO_CFM=OFF
cmake .. -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DETCS_VENDORED=OFF -DRADIO_CFM=OFF >cmake_output.txt 2>&1

if %ERRORLEVEL% EQU 0 (
    echo [OK] CMake configuration successful
    echo.
    echo Configuration summary:
    findstr "Taiwan Railway ATP Configuration" cmake_output.txt
    findstr "SIMRAIL:" cmake_output.txt
    findstr "TRA_ATP_MODE:" cmake_output.txt
    findstr "RADIO_CFM:" cmake_output.txt
) else (
    echo [FAIL] CMake configuration failed
    echo.
    echo Error output:
    type cmake_output.txt
)

cd ..
rmdir /s /q test-build

:end
echo.
echo Test completed. 
if %ERRORLEVEL% EQU 0 (
    echo You can now run build_tra_atp.bat to build the system.
) else (
    echo Please fix the issues above before building.
    echo See INSTALL_DEPENDENCIES_WINDOWS.md for help.
)
pause