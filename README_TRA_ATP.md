# 台鐵ATP系統整合 (Taiwan Railway ATP Integration)

# VIBE CODED(Kiro) 不保證效果

## 概述

本專案將原有的ETCS DMI系統改造為台鐵ATP (Automatic Train Protection) 系統，並整合SimRail的SHP (Samoczynne Hamowanie Pociągu) 區段限速和信號資料，實現與ETCS相同的監督功能。

## 主要功能

### 1. 台鐵ATP用戶介面
- **速度表**: 顯示目前速度、目標速度、限制速度
- **信號指示器**: 顯示前方信號狀態（紅、黃、綠、閃黃）
- **狀態面板**: 顯示ATP狀態、煞車狀態、車門狀態、牽引狀態
- **距離條**: 顯示到目標點的距離
- **訊息區域**: 顯示系統訊息和警告
- **控制按鈕**: 確認、重置、超馳、緊急等功能

### 2. SimRail SHP整合
- **速度限制**: 從SimRail獲取區段限速資料
- **信號資料**: 獲取信號機狀態和距離資訊
- **軌道資料**: 獲取軌道元素（車站、平交道、隧道等）
- **列車狀態**: 獲取列車運行狀態和系統資訊

### 3. ETCS整合
- **監督功能**: 將SHP資料轉換為ETCS監督曲線
- **煞車控制**: 自動煞車介入和釋放
- **牽引控制**: 牽引切斷和恢復
- **移動授權**: 整合ETCS移動授權系統

### 4. 安全功能
- **超速保護**: 自動監督速度限制
- **信號保護**: 根據信號狀態自動煞車
- **緊急煞車**: 危險情況下自動緊急煞車
- **故障安全**: 系統故障時自動採取安全措施

## 系統架構

```
台鐵ATP系統
├── UI層 (DMI/graphics/tra_components.*)
│   ├── TRASpeedometer (速度表)
│   ├── TRASignalIndicator (信號指示器)
│   ├── TRAStatusPanel (狀態面板)
│   ├── TRADistanceBar (距離條)
│   ├── TRAControlButtons (控制按鈕)
│   └── TRAMessageArea (訊息區域)
├── 整合層 (DMI/tra_atp_integration.*)
│   ├── TRAATPIntegration (主整合類)
│   ├── 模式管理
│   ├── 事件處理
│   └── 安全檢查
├── SHP介面層 (DMI/simrail/shp_interface.*)
│   ├── SHPInterface (SHP介面類)
│   ├── 資料解析
│   └── ETCS轉換
├── SimRail API層 (DMI/simrail/simrail_api.*)
│   ├── SimRailAPI (API介面類)
│   ├── HTTP/WebSocket通訊
│   └── 資料快取
└── 配置層
    ├── config.json (系統配置)
    ├── tra_atp_layout.json (UI佈局)
    └── zh_TW.po (繁體中文本地化)
```

## 安裝和配置

### 1. 編譯要求
- CMake 3.10+
- C++17編譯器
- SDL2開發庫
- nlohmann/json庫

### 2. 編譯步驟
```bash
mkdir build
cd build
cmake ..
make
```

### 3. 配置檔案

#### config.json
```json
{
    "TRA_ATP": {
        "SpeedDial": 130,
        "STMLayout": "tra_atp_layout.json",
        "SoftKeys": true,
        "Language": "zh_TW",
        "MaxSpeed": 130,
        "SoundEnabled": true
    }
}
```

#### settings.ini
```ini
width=800
height=600
tra_atp_mode=1
tra_language=zh_TW
tra_sound_enabled=1
```

### 4. 啟動系統
```bash
./dmi
```

系統會自動檢測配置並啟動台鐵ATP模式。

## 使用說明

### 1. 系統啟動
- 系統啟動後會自動初始化台鐵ATP組件
- 嘗試連接到SimRail API (預設localhost:8080)
- 顯示台鐵ATP用戶介面

### 2. 操作介面
- **速度表**: 顯示目前速度和限制
- **信號燈**: 顯示前方信號狀態
- **狀態燈**: 顯示各系統狀態
- **控制按鈕**: 點擊執行相應功能
- **訊息區**: 查看系統訊息

### 3. 功能按鈕
- **確認**: 確認警告訊息
- **重置**: 重置系統狀態
- **超馳**: 暫時超馳限制（謹慎使用）
- **緊急**: 緊急煞車

### 4. 菜單系統
- 點擊右上角菜單按鈮進入設定
- 可查看系統資訊、診斷、設定等

## SimRail整合

### 1. API端點
系統會連接到SimRail API獲取以下資料：
- `/api/train` - 列車狀態
- `/api/signals` - 信號資料
- `/api/track` - 軌道資料
- `/api/system` - 系統狀態

### 2. 資料格式
參考 `DMI/simrail/simrail_api.h` 中的資料結構定義。

### 3. 自動更新
系統每500ms更新一次SimRail資料，確保即時性。

## 故障排除

### 1. 常見問題

#### 無法連接SimRail
- 檢查SimRail是否運行
- 確認API端點和埠號正確
- 檢查防火牆設定

#### 介面顯示異常
- 檢查解析度設定
- 確認字型檔案存在
- 檢查tra_atp_mode設定

#### 功能無回應
- 檢查系統日誌
- 執行系統診斷
- 重新啟動系統

### 2. 日誌和診斷
- 系統日誌會顯示在控制台
- 使用診斷功能檢查系統狀態
- 檢查事件歷史記錄

## 開發資訊

### 1. 主要檔案
- `DMI/graphics/tra_components.*` - UI組件
- `DMI/tra_atp_integration.*` - 主整合系統
- `DMI/simrail/shp_interface.*` - SHP介面
- `DMI/simrail/simrail_api.*` - SimRail API
- `DMI/window/tra_menu.*` - 菜單系統

### 2. 擴展開發
- 新增UI組件：繼承Component類
- 新增功能：修改TRAATPIntegration類
- 新增API：修改SimRailAPI類
- 新增語言：編輯.po檔案

### 3. 測試
- 使用內建診斷功能
- 檢查系統自檢結果
- 監控事件日誌

## 授權

本專案使用GNU General Public License v3.0授權。
詳見LICENSE檔案。

## 貢獻

歡迎提交Issue和Pull Request。
請遵循現有的程式碼風格和註釋規範。

---

**注意**: 本系統僅供模擬和教育用途，不得用於實際鐵路運營。