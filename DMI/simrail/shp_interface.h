/*
 * SimRail SHP Interface for Taiwan Railway ATP
 * Copyright (C) 2024  Taiwan Railway Administration
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _SHP_INTERFACE_H
#define _SHP_INTERFACE_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "../graphics/color.h"

// SHP信號狀態
enum class SHPSignalAspect {
    RED_STOP = 0,           // 紅燈停車
    YELLOW_CAUTION = 1,     // 黃燈注意
    GREEN_CLEAR = 2,        // 綠燈正常
    YELLOW_YELLOW = 3,      // 雙黃燈
    GREEN_YELLOW = 4,       // 綠黃燈
    FLASHING_YELLOW = 5,    // 閃黃燈
    UNKNOWN = 99            // 未知狀態
};

// SHP速度限制資料
struct SHPSpeedRestriction {
    float distance;         // 距離 (公尺)
    int speed_limit;        // 速度限制 (km/h)
    int warning_speed;      // 警告速度 (km/h)
    bool temporary;         // 是否為臨時限速
    std::string reason;     // 限速原因
    bool active;            // 是否啟用
};

// SHP信號資料
struct SHPSignalData {
    float distance;         // 信號機距離 (公尺)
    SHPSignalAspect aspect; // 信號顯示
    int speed_limit;        // 信號後速度限制
    std::string signal_id;  // 信號機編號
    bool approach_control;  // 是否有進路控制
    bool active;            // 信號是否有效
};

// SHP軌道資料
struct SHPTrackData {
    float distance;         // 距離 (公尺)
    std::string track_id;   // 軌道編號
    std::string station;    // 車站名稱
    int platform;          // 月台編號
    bool level_crossing;    // 是否有平交道
    bool tunnel;            // 是否在隧道內
    bool bridge;            // 是否在橋樑上
};

// SHP系統狀態
struct SHPSystemStatus {
    bool connected;         // 是否連接到SimRail
    bool shp_active;        // SHP系統是否啟用
    bool atp_mode;          // ATP模式是否啟用
    float current_speed;    // 目前速度
    float target_speed;     // 目標速度
    float distance_to_target; // 到目標距離
    std::string train_number; // 列車編號
    std::string route;      // 路線名稱
};

class SHPInterface {
private:
    bool initialized = false;
    bool connected = false;
    SHPSystemStatus system_status;
    std::vector<SHPSpeedRestriction> speed_restrictions;
    std::vector<SHPSignalData> signals;
    std::vector<SHPTrackData> track_data;
    
    // 回調函數
    std::function<void(const SHPSignalData&)> signal_callback;
    std::function<void(const SHPSpeedRestriction&)> speed_callback;
    std::function<void(const SHPSystemStatus&)> status_callback;
    
    // 內部處理函數
    void processSimRailData();
    void updateSpeedRestrictions();
    void updateSignalData();
    void updateTrackData();
    void sendETCSCommands();
    
public:
    SHPInterface();
    ~SHPInterface();
    
    // 初始化和連接
    bool initialize();
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    
    // 資料更新
    void update();
    void forceUpdate();
    
    // 資料存取
    const SHPSystemStatus& getSystemStatus() const { return system_status; }
    const std::vector<SHPSpeedRestriction>& getSpeedRestrictions() const { return speed_restrictions; }
    const std::vector<SHPSignalData>& getSignals() const { return signals; }
    const std::vector<SHPTrackData>& getTrackData() const { return track_data; }
    
    // 查詢函數
    SHPSpeedRestriction* getNextSpeedRestriction();
    SHPSignalData* getNextSignal();
    SHPTrackData* getNextTrackElement();
    int getCurrentSpeedLimit();
    SHPSignalAspect getCurrentSignalAspect();
    
    // 回調設定
    void setSignalCallback(std::function<void(const SHPSignalData&)> callback);
    void setSpeedCallback(std::function<void(const SHPSpeedRestriction&)> callback);
    void setStatusCallback(std::function<void(const SHPSystemStatus&)> callback);
    
    // ETCS整合
    void sendSpeedRestrictionToETCS(const SHPSpeedRestriction& restriction);
    void sendSignalDataToETCS(const SHPSignalData& signal);
    void sendBrakeCommand(bool emergency = false);
    void sendTractionCutCommand();
    
    // 台鐵ATP特定功能
    void enableTRAATPMode(bool enable);
    void setTRASpeedProfile(const std::vector<SHPSpeedRestriction>& profile);
    void handleTRASignalChange(SHPSignalAspect old_aspect, SHPSignalAspect new_aspect);
    
    // 診斷和測試
    bool runSelfTest();
    std::vector<std::string> getDiagnosticMessages();
    void clearDiagnosticMessages();
};

// 全局SHP介面實例
extern std::unique_ptr<SHPInterface> shp_interface;

// 輔助函數
std::string signalAspectToString(SHPSignalAspect aspect);
Color signalAspectToColor(SHPSignalAspect aspect);
SHPSignalAspect stringToSignalAspect(const std::string& aspect_str);

// 初始化函數
bool initializeSHPInterface();
void shutdownSHPInterface();

// SimRail資料解析函數
SHPSystemStatus parseSimRailSystemStatus(const std::string& data);
std::vector<SHPSpeedRestriction> parseSimRailSpeedData(const std::string& data);
std::vector<SHPSignalData> parseSimRailSignalData(const std::string& data);
std::vector<SHPTrackData> parseSimRailTrackData(const std::string& data);

#endif