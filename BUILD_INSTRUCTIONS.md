# 台鐵ATP系統編譯指南

## 快速開始

### Windows 用戶 (推薦方法)

1. **安裝前置需求**
   - Visual Studio 2019+ (包含 C++ 開發工具)
   - CMake 3.14+
   - Git

2. **測試配置**
   ```cmd
   test_cmake_config.bat
   ```

3. **自動編譯**
   ```cmd
   build_tra_atp.bat
   ```

4. **運行系統**
   ```cmd
   cd build\DMI\Release
   dmi.exe
   ```

### Linux 用戶

1. **安裝依賴**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libsdl2-dev libsdl2-ttf-dev cmake build-essential
   
   # CentOS/RHEL
   sudo yum install SDL2-devel SDL2_ttf-devel cmake gcc-c++
   ```

2. **編譯**
   ```bash
   ./build_tra_atp.sh
   ```

3. **運行**
   ```bash
   cd build/DMI
   ./dmi
   ```

## 詳細說明

### 系統需求

- **作業系統**: Windows 10+, Linux (Ubuntu 18.04+)
- **編譯器**: Visual Studio 2019+ (Windows), GCC 7+ (Linux)
- **CMake**: 3.14 或更新版本
- **記憶體**: 至少 4GB RAM
- **硬碟空間**: 至少 2GB 可用空間

### 依賴庫

- **SDL2**: 2.0.12+ (圖形和輸入處理)
- **SDL2_ttf**: 2.0.15+ (字型渲染)
- **nlohmann/json**: 3.7+ (JSON 解析，已包含在專案中)

### 編譯選項

- `SIMRAIL=ON`: 啟用 SimRail 整合
- `TRA_ATP_MODE=ON`: 啟用台鐵 ATP 模式
- `ETCS_VENDORED=OFF`: 使用系統依賴庫
- `RADIO_CFM=OFF`: 停用無線電功能 (避免 c-ares 依賴)
- `DEBUG_VERBOSE=ON`: 啟用詳細除錯訊息

### 手動編譯步驟

1. **創建編譯目錄**
   ```cmd
   mkdir build
   cd build
   ```

2. **配置 CMake**
   ```cmd
   cmake .. -DSIMRAIL=ON -DTRA_ATP_MODE=ON -DETCS_VENDORED=OFF -DRADIO_CFM=OFF
   ```

3. **編譯**
   ```cmd
   cmake --build . --config Release
   ```

### 故障排除

#### CMake 配置失敗

**問題**: `CMake Error: The source directory does not contain a CMakeLists.txt file`
**解決**: 確保在 ETCS 根目錄執行命令

**問題**: `Could not find SDL2`
**解決**: 
- Windows: 安裝 vcpkg 並執行 `vcpkg install sdl2:x64-windows`
- Linux: 安裝 `libsdl2-dev`

**問題**: `c-ares not found`
**解決**: 使用 `-DRADIO_CFM=OFF` 選項

#### 編譯錯誤

**問題**: `C++17 features not supported`
**解決**: 更新編譯器版本

**問題**: `undefined reference to SDL_*`
**解決**: 確認 SDL2 開發庫已正確安裝

#### 運行時錯誤

**問題**: `DLL not found`
**解決**: 確保 SDL2.dll 和 SDL2_ttf.dll 在執行目錄

**問題**: `Failed to initialize graphics`
**解決**: 檢查顯示卡驅動程式

### 配置文件

編譯完成後，系統會使用以下配置文件：

- `config.json`: 主要系統配置
- `DMI/settings.ini`: 顯示設定
- `DMI/tra_atp_layout.json`: ATP 介面佈局
- `locales/dmi/zh_TW.po`: 繁體中文語言包

### 開發模式

如果要進行開發：

1. **使用 Debug 模式**
   ```cmd
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMRAIL=ON -DTRA_ATP_MODE=ON
   ```

2. **啟用詳細除錯**
   ```cmd
   cmake .. -DDEBUG_VERBOSE=ON
   ```

3. **使用 IDE**
   - Visual Studio: 開啟 `.sln` 文件
   - VS Code: 安裝 CMake 擴展

### 測試

編譯完成後可以執行以下測試：

1. **系統自檢**
   - 啟動 DMI 後進入診斷選單
   - 執行系統測試

2. **SimRail 連接測試**
   - 確保 SimRail 正在運行
   - 檢查 API 連接狀態

3. **ATP 功能測試**
   - 測試速度監督
   - 測試信號保護
   - 測試煞車功能

### 效能調整

- **降低 CPU 使用率**: 調整更新頻率
- **減少記憶體使用**: 限制事件歷史大小
- **改善回應速度**: 使用 Release 模式編譯

### 支援

如果遇到問題：

1. 查看 `README_TRA_ATP.md` 詳細文件
2. 檢查 `INSTALL_DEPENDENCIES_WINDOWS.md` 安裝指南
3. 查看編譯輸出的錯誤訊息
4. 確認所有前置需求已滿足

### 版本資訊

- **台鐵 ATP 版本**: 1.0
- **ETCS 基線**: Baseline 3
- **支援的 SimRail 版本**: 最新版本
- **最後更新**: 2026年1月

---

**注意**: 本系統僅供模擬和教育用途，不得用於實際鐵路運營。