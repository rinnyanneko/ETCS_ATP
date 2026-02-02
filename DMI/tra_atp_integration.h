/*
 * Taiwan Railway ATP Integration with ETCS
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
#ifndef _TRA_ATP_INTEGRATION_H
#define _TRA_ATP_INTEGRATION_H

#include "simrail/shp_interface.h"
#include "graphics/tra_components.h"
#include "../EVC/Supervision/supervision.h"
#include "../EVC/MA/movement_authority.h"
#include <memory>
#include <vector>
#include <functional>

// 台鐵ATP與ETCS整合狀態
enum class TRAATPIntegrationStatus {
    DISCONNECTED,       // 未連接
    CONNECTING,         // 連接中
    CONNECTED,          // 已連接
    SYNCHRONIZING,      // 同步中
    SYNCHRONIZED,       // 已同步
    ERROR_STATE         // 錯誤狀態
};

// 台鐵ATP運行模式
enum class TRAATPMode {
    STANDBY,            // 待機模式
    FULL_SUPERVISION,   // 完全監督模式
    PARTIAL_SUPERVISION,// 部分監督模式
    ON_SIGHT,          // 目視運行模式
    SHUNTING,          // 調車模式
    EMERGENCY          // 緊急模式
};

// 台鐵ATP系統狀態
struct TRAATPSystemState {
    TRAATPIntegrationStatus integration_status;
    TRAATPMode current_mode;
    bool atp_active;
    bool shp_active;
    bool etcs_active;
    float current_speed;
    float permitted_speed;
    float target_speed;
    float distance_to_target;
    bool brake_intervention;
    bool traction_cut;
    bool emergency_brake;
    std::string current_station;
    std::string next_station;
    std::string route_name;
    std::string train_number;
};

// 台鐵ATP事件類型
enum class TRAATPEventType {
    SPEED_RESTRICTION_AHEAD,
    SIGNAL_CHANGE,
    BRAKE_INTERVENTION,
    TRACTION_CUT,
    EMERGENCY_BRAKE,
    MODE_CHANGE,
    SYSTEM_FAULT,
    COMMUNICATION_ERROR
};

// 台鐵ATP事件資料
struct TRAATPEvent {
    TRAATPEventType type;
    std::string message;
    float distance;
    int severity; // 0=info, 1=warning, 2=critical
    int64_t timestamp;
    std::string additional_data;
};

class TRAATPIntegration {
private:
    std::unique_ptr<SHPInterface> shp_interface;
    TRAATPSystemState system_state;
    std::vector<TRAATPEvent> event_history;
    std::vector<std::function<void(const TRAATPEvent&)>> event_callbacks;
    
    // 內部狀態
    bool initialized = false;
    bool running = false;
    int64_t last_update_time = 0;
    int64_t last_shp_update = 0;
    
    // ETCS整合
    void updateETCSSupervision();
    void updateETCSMovementAuthority();
    void sendETCSBrakeCommand(bool emergency);
    void sendETCSTractionCut();
    
    // SHP資料處理
    void processSHPSpeedRestrictions();
    void processSHPSignalData();
    void processSHPTrackData();
    
    // 事件處理
    void addEvent(TRAATPEventType type, const std::string& message, 
                  float distance = 0.0f, int severity = 0);
    void processEvents();
    void notifyEventCallbacks(const TRAATPEvent& event);
    
    // 模式管理
    void handleModeTransition(TRAATPMode new_mode);
    bool validateModeTransition(TRAATPMode from, TRAATPMode to);
    
    // 安全檢查
    bool performSafetyChecks();
    void handleSafetyViolation(const std::string& violation);
    
public:
    TRAATPIntegration();
    ~TRAATPIntegration();
    
    // 初始化和控制
    bool initialize();
    void shutdown();
    bool start();
    void stop();
    void update();
    
    // 狀態查詢
    bool isInitialized() const { return initialized; }
    bool isRunning() const { return running; }
    const TRAATPSystemState& getSystemState() const { return system_state; }
    TRAATPIntegrationStatus getIntegrationStatus() const { return system_state.integration_status; }
    TRAATPMode getCurrentMode() const { return system_state.current_mode; }
    
    // 模式控制
    bool setMode(TRAATPMode mode);
    bool enableATP(bool enable);
    bool enableSHP(bool enable);
    bool enableETCS(bool enable);
    
    // 速度和距離控制
    void setPermittedSpeed(float speed);
    void setTargetSpeed(float speed);
    void setCurrentSpeed(float speed);
    void setDistanceToTarget(float distance);
    
    // 煞車和牽引控制
    void applyServiceBrake();
    void applyEmergencyBrake();
    void releaseBrake();
    void cutTraction();
    void restoreTraction();
    
    // 事件管理
    void addEventListener(std::function<void(const TRAATPEvent&)> callback);
    std::vector<TRAATPEvent> getEventHistory(int max_events = 100);
    void clearEventHistory();
    
    // 診斷和測試
    bool runSystemTest();
    std::vector<std::string> getDiagnosticInfo();
    bool performCalibration();
    
    // SimRail整合
    bool connectToSimRail();
    void disconnectFromSimRail();
    bool isSimRailConnected();
    void forceSimRailUpdate();
    
    // UI更新
    void updateTRAComponents();
    void updateETCSComponents();
    
    // 配置管理
    bool loadConfiguration(const std::string& config_file);
    bool saveConfiguration(const std::string& config_file);
    void resetToDefaults();
};

// 全局台鐵ATP整合實例
extern std::unique_ptr<TRAATPIntegration> tra_atp_integration;

// 全局函數
bool initializeTRAATPIntegration();
void shutdownTRAATPIntegration();
void updateTRAATPIntegration();

// 輔助函數
std::string modeToString(TRAATPMode mode);
std::string statusToString(TRAATPIntegrationStatus status);
std::string eventTypeToString(TRAATPEventType type);
TRAATPMode stringToMode(const std::string& mode_str);

// 回調函數類型定義
using TRAATPEventCallback = std::function<void(const TRAATPEvent&)>;
using TRAATPModeChangeCallback = std::function<void(TRAATPMode old_mode, TRAATPMode new_mode)>;
using TRAATPStatusChangeCallback = std::function<void(TRAATPIntegrationStatus old_status, TRAATPIntegrationStatus new_status)>;

#endif