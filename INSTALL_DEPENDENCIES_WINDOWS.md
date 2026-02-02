# Windows 依賴安裝指南

## 前置需求

### 1. 安裝 Visual Studio
- 下載並安裝 Visual Studio 2019 或更新版本
- 確保安裝 "C++ 桌面開發" 工作負載
- 包含 CMake 工具

### 2. 安裝 CMake (如果 Visual Studio 沒有包含)
- 前往 https://cmake.org/download/
- 下載並安裝 CMake 3.14 或更新版本
- 安裝時選擇 "Add CMake to system PATH"

### 3. 安裝 Git (如果尚未安裝)
- 前往 https://git-scm.com/download/win
- 下載並安裝 Git for Windows

## 方法一：使用 vcpkg (推薦)

### 1. 安裝 vcpkg
```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### 2. 安裝依賴庫
```cmd
C:\vcpkg\vcpkg install sdl2:x64-windows
C:\vcpkg\vcpkg install sdl2-ttf:x64-windows
C:\vcpkg\vcpkg install nlohmann-json:x64-windows
```

### 3. 編譯台鐵ATP系統
```cmd
cd C:\path\to\ETCS
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DSIMRAIL=ON -DTRA_ATP_MODE=ON
cmake --build . --config Release
```

## 方法二：使用預編譯腳本 (最簡單)

1. 確保已安裝 Visual Studio 和 CMake
2. 雙擊運行 `build_tra_atp.bat`
3. 腳本會自動處理依賴和編譯

## 方法三：手動安裝依賴

### 1. 下載 SDL2
- 前往 https://www.libsdl.org/download-2.0.php
- 下載 "Development Libraries" (SDL2-devel-2.x.x-VC.zip)
- 解壓到 `C:\SDL2`

### 2. 下載 SDL2_ttf
- 前往 https://www.libsdl.org/projects/SDL_ttf/
- 下載 "Development Libraries" (SDL2_ttf-devel-2.x.x-VC.zip)
- 解壓到 `C:\SDL2_ttf`

### 3. 編譯
```cmd
cd C:\path\to\ETCS
mkdir build
cd build
cmake .. -DSDL2_DIR=C:\SDL2 -DSDL2_TTF_DIR=C:\SDL2_ttf -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DETCS_VENDORED=OFF
cmake --build . --config Release
```

## 運行台鐵ATP系統

編譯成功後：
```cmd
cd DMI\Release
dmi.exe
```

或者從 Debug 目錄：
```cmd
cd DMI\Debug
dmi.exe
```

## 配置台鐵ATP

1. 確保 `config.json` 包含 TRA_ATP 配置
2. 設置 `tra_atp_mode=1` 在 `DMI/settings.ini`
3. 系統會自動載入台鐵ATP介面

## 故障排除

### CMake 找不到
- 確認 CMake 已安裝並加入 PATH
- 嘗試使用 Visual Studio Developer Command Prompt
- 重新啟動命令提示字元

### 找不到 SDL2
- 確認 SDL2 已正確安裝
- 檢查路徑設置
- 嘗試使用 vcpkg 方法

### 編譯錯誤
- 確認 Visual Studio 支援 C++17
- 檢查所有依賴庫版本
- 查看詳細錯誤訊息並搜尋解決方案

### 運行時錯誤
- 確認所有 DLL 文件在執行目錄
- 檢查 `settings.ini` 配置
- 查看控制台輸出的錯誤訊息

## 依賴庫版本要求

- **SDL2**: 2.0.12 或更新版本
- **SDL2_ttf**: 2.0.15 或更新版本
- **CMake**: 3.14 或更新版本
- **Visual Studio**: 2019 或更新版本 (支援 C++17)
- **Windows**: Windows 10 或更新版本

## 開發環境設置

如果要進行開發：

1. 安裝 Visual Studio Code (可選)
2. 安裝 C++ 擴展
3. 配置 CMake 工具
4. 設置調試配置

## 技術支援

如果遇到問題：

1. 檢查 `README_TRA_ATP.md` 的詳細說明
2. 查看編譯輸出的錯誤訊息
3. 確認所有前置需求已滿足
4. 嘗試不同的安裝方法