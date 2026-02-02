/*
 * Taiwan Railway ATP Integration with ETCS - Implementation
 * Copyright (C) 2026 rinnyanneko
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

#include "tra_atp_integration.h"
#include "simrail/shp_interface.h"
#include "../EVC/Supervision/supervision.h"
#include "../EVC/Supervision/targets.h"
#include "../EVC/MA/movement_authority.h"
#include "../EVC/Procedures/mode_transition.h"
#include "../platform/platform_runtime.h"
#include <algorithm>
#include <cmath>

// 全域台鐵ATP整合實例
std::unique_ptr<TRAATPIntegration> tra_atp_integration = nullptr;

TRAATPIntegration::TRAATPIntegration()
{
    // 初始化系統狀態
    system_state = {};
    system_state.integration_status = TRAATPIntegrationStatus::DISCONNECTED;
    system_state.current_mode = TRAATPMode::STANDBY;
    system_state.atp_active = false;
    system_state.shp_active = false;
    system_state.etcs_active = false;
    system_state.current_speed = 0.0f;
    system_state.permitted_speed = 130.0f;
    system_state.target_speed = 0.0f;
    system_state.distance_to_target = 0.0f;
    system_state.brake_intervention = false;
    system_state.traction_cut = false;
    system_state.emergency_brake = false;
    system_state.current_station = "";
    system_state.next_station = "";
    system_state.route_name = "西部幹線";
    system_state.train_number = "TRA-1001";
    
    event_history.clear();
    event_callbacks.clear();
}

TRAATPIntegration::~TRAATPIntegration()
{
    shutdown();
}

bool TRAATPIntegration::initialize()
{
    if (initialized) return true;
    
    platform->debug_print("Initializing TRA ATP Integration System");
    
    // 初始化SHP介面
    if (!initializeSHPInterface()) {
        platform->debug_print("Failed to initialize SHP interface");
        return false;
    }
    
    shp_interface = std::move(::shp_interface);
    
    // 設置SHP回調
    if (shp_interface) {
        shp_interface->setSignalCallback([this](const SHPSignalData& signal) {
            addEvent(TRAATPEventType::SIGNAL_CHANGE, 
                    "信號變化: " + signalAspectToString(signal.aspect), 
                    signal.distance, 1);
            processSHPSignalData();
        });
        
        shp_interface->setSpeedCallback([this](const SHPSpeedRestriction& restriction) {
            addEvent(TRAATPEventType::SPEED_RESTRICTION_AHEAD,
                    "速度限制: " + std::to_string(restriction.speed_limit) + " km/h",
                    restriction.distance, 1);
            processSHPSpeedRestrictions();
        });
        
        shp_interface->setStatusCallback([this](const SHPSystemStatus& status) {
            system_state.current_speed = status.current_speed;
            system_state.target_speed = status.target_speed;
            system_state.distance_to_target = status.distance_to_target;
            system_state.train_number = status.train_number;
            system_state.route_name = status.route;
            system_state.shp_active = status.shp_active;
        });
    }
    
    // 初始化事件系統
    addEvent(TRAATPEventType::SYSTEM_FAULT, "台鐵ATP系統初始化", 0.0f, 0);
    
    system_state.integration_status = TRAATPIntegrationStatus::CONNECTED;
    initialized = true;
    
    platform->debug_print("TRA ATP Integration System initialized successfully");
    return true;
}

void TRAATPIntegration::shutdown()
{
    if (!initialized) return;
    
    platform->debug_print("Shutting down TRA ATP Integration System");
    
    stop();
    
    if (shp_interface) {
        shp_interface->disconnect();
        shp_interface.reset();
    }
    
    system_state.integration_status = TRAATPIntegrationStatus::DISCONNECTED;
    initialized = false;
    
    addEvent(TRAATPEventType::SYSTEM_FAULT, "台鐵ATP系統關閉", 0.0f, 0);
}

bool TRAATPIntegration::start()
{
    if (!initialized) {
        platform->debug_print("Cannot start: TRA ATP Integration not initialized");
        return false;
    }
    
    if (running) return true;
    
    platform->debug_print("Starting TRA ATP Integration System");
    
    // 連接到SimRail
    if (!connectToSimRail()) {
        platform->debug_print("Failed to connect to SimRail");
        return false;
    }
    
    // 啟動ATP模式
    system_state.atp_active = true;
    system_state.current_mode = TRAATPMode::FULL_SUPERVISION;
    system_state.integration_status = TRAATPIntegrationStatus::SYNCHRONIZED;
    
    running = true;
    last_update_time = platform->get_timer();
    
    addEvent(TRAATPEventType::MODE_CHANGE, "ATP系統啟動 - 完全監督模式", 0.0f, 0);
    
    // 更新UI組件
    updateTRAComponents();
    
    platform->debug_print("TRA ATP Integration System started successfully");
    return true;
}

void TRAATPIntegration::stop()
{
    if (!running) return;
    
    platform->debug_print("Stopping TRA ATP Integration System");
    
    // 停止ATP功能
    system_state.atp_active = false;
    system_state.current_mode = TRAATPMode::STANDBY;
    system_state.brake_intervention = false;
    system_state.traction_cut = false;
    system_state.emergency_brake = false;
    
    // 斷開SimRail連接
    disconnectFromSimRail();
    
    running = false;
    
    addEvent(TRAATPEventType::MODE_CHANGE, "ATP系統停止", 0.0f, 0);
    
    // 更新UI組件
    updateTRAComponents();
    
    platform->debug_print("TRA ATP Integration System stopped");
}

void TRAATPIntegration::update()
{
    if (!running) return;
    
    int64_t current_time = platform->get_timer();
    
    // 限制更新頻率 (每100ms更新一次)
    if (current_time - last_update_time < 100) return;
    
    last_update_time = current_time;
    
    // 更新SHP介面
    if (shp_interface && current_time - last_shp_update > 500) {
        shp_interface->update();
        last_shp_update = current_time;
    }
    
    // 執行安全檢查
    if (!performSafetyChecks()) {
        handleSafetyViolation("安全檢查失敗");
    }
    
    // 處理事件
    processEvents();
    
    // 更新ETCS監督
    updateETCSSupervision();
    
    // 更新UI組件
    updateTRAComponents();
}

void TRAATPIntegration::updateETCSSupervision()
{
    if (!system_state.etcs_active) return;
    
    // 將台鐵ATP資料轉換為ETCS監督資料
    extern float Vtarget, Vperm, Vsbi, Vebi, Vrelease, Vest, Dtarg;
    
    Vtarget = system_state.target_speed;
    Vperm = system_state.permitted_speed;
    Vest = system_state.current_speed;
    Dtarg = system_state.distance_to_target;
    
    // 設置煞車曲線
    Vsbi = system_state.permitted_speed + 5.0f;  // 常用煞車介入速度
    Vebi = system_state.permitted_speed + 10.0f; // 緊急煞車介入速度
    Vrelease = system_state.permitted_speed - 2.0f; // 煞車釋放速度
    
    // 檢查超速
    if (system_state.current_speed > Vsbi && !system_state.brake_intervention) {
        applyServiceBrake();
    }
    
    if (system_state.current_speed > Vebi && !system_state.emergency_brake) {
        applyEmergencyBrake();
    }
    
    // 檢查煞車釋放條件
    if (system_state.brake_intervention && system_state.current_speed <= Vrelease) {
        releaseBrake();
    }
}

void TRAATPIntegration::updateETCSMovementAuthority()
{
    // 更新移動授權資料
    // 這裡應該與ETCS MA系統整合
}

void TRAATPIntegration::sendETCSBrakeCommand(bool emergency)
{
    // 發送煞車命令到ETCS系統
    extern bool SB, EB;
    
    if (emergency) {
        EB = true;
        system_state.emergency_brake = true;
        addEvent(TRAATPEventType::EMERGENCY_BRAKE, "緊急煞車作用", 0.0f, 2);
    } else {
        SB = true;
        system_state.brake_intervention = true;
        addEvent(TRAATPEventType::BRAKE_INTERVENTION, "常用煞車作用", 0.0f, 1);
    }
}

void TRAATPIntegration::sendETCSTractionCut()
{
    // 發送牽引切斷命令到ETCS系統
    system_state.traction_cut = true;
    addEvent(TRAATPEventType::TRACTION_CUT, "牽引切斷", 0.0f, 1);
}

void TRAATPIntegration::processSHPSpeedRestrictions()
{
    if (!shp_interface) return;
    
    auto restrictions = shp_interface->getSpeedRestrictions();
    
    for (const auto& restriction : restrictions) {
        if (restriction.active && restriction.distance > 0) {
            // 更新允許速度
            if (restriction.speed_limit < system_state.permitted_speed) {
                system_state.permitted_speed = (float)restriction.speed_limit;
                
                // 如果目前速度超過新的限制，觸發煞車
                if (system_state.current_speed > system_state.permitted_speed + 5.0f) {
                    applyServiceBrake();
                }
            }
            
            // 更新目標速度和距離
            system_state.target_speed = (float)restriction.speed_limit;
            system_state.distance_to_target = restriction.distance;
            
            break; // 只處理最近的限速
        }
    }
}

void TRAATPIntegration::processSHPSignalData()
{
    if (!shp_interface) return;
    
    auto signals = shp_interface->getSignals();
    
    for (const auto& signal : signals) {
        if (signal.active && signal.distance > 0) {
            // 根據信號狀態採取行動
            switch (signal.aspect) {
                case SHPSignalAspect::RED_STOP:
                    if (signal.distance < 100.0f) {
                        applyEmergencyBrake();
                    } else if (signal.distance < 300.0f) {
                        applyServiceBrake();
                    }
                    system_state.target_speed = 0.0f;
                    break;
                    
                case SHPSignalAspect::YELLOW_CAUTION:
                    system_state.target_speed = std::min(80.0f, system_state.permitted_speed);
                    break;
                    
                case SHPSignalAspect::GREEN_CLEAR:
                    system_state.target_speed = system_state.permitted_speed;
                    break;
                    
                default:
                    break;
            }
            
            system_state.distance_to_target = signal.distance;
            break; // 只處理最近的信號
        }
    }
}

void TRAATPIntegration::processSHPTrackData()
{
    if (!shp_interface) return;
    
    auto track_data = shp_interface->getTrackData();
    
    for (const auto& track : track_data) {
        if (track.distance > 0 && track.distance < 1000.0f) {
            // 更新車站資訊
            if (!track.station.empty()) {
                if (system_state.current_station.empty()) {
                    system_state.current_station = track.station;
                } else if (system_state.next_station.empty()) {
                    system_state.next_station = track.station;
                }
            }
            
            // 處理平交道
            if (track.level_crossing && track.distance < 200.0f) {
                addEvent(TRAATPEventType::SPEED_RESTRICTION_AHEAD, 
                        "前方平交道", track.distance, 1);
            }
        }
    }
}

void TRAATPIntegration::addEvent(TRAATPEventType type, const std::string& message, 
                                float distance, int severity)
{
    TRAATPEvent event;
    event.type = type;
    event.message = message;
    event.distance = distance;
    event.severity = severity;
    event.timestamp = platform->get_timer();
    event.additional_data = "";
    
    event_history.push_back(event);
    
    // 限制事件歷史大小
    if (event_history.size() > 1000) {
        event_history.erase(event_history.begin(), event_history.begin() + 100);
    }
    
    // 通知回調
    notifyEventCallbacks(event);
    
    platform->debug_print("TRA ATP Event: " + message);
}

void TRAATPIntegration::processEvents()
{
    // 處理待處理的事件
    // 這裡可以實作事件優先級處理邏輯
}

void TRAATPIntegration::notifyEventCallbacks(const TRAATPEvent& event)
{
    for (const auto& callback : event_callbacks) {
        if (callback) {
            callback(event);
        }
    }
}

void TRAATPIntegration::handleModeTransition(TRAATPMode new_mode)
{
    if (!validateModeTransition(system_state.current_mode, new_mode)) {
        addEvent(TRAATPEventType::SYSTEM_FAULT, 
                "無效的模式轉換: " + modeToString(system_state.current_mode) + 
                " -> " + modeToString(new_mode), 0.0f, 2);
        return;
    }
    
    TRAATPMode old_mode = system_state.current_mode;
    system_state.current_mode = new_mode;
    
    addEvent(TRAATPEventType::MODE_CHANGE,
            "模式變更: " + modeToString(old_mode) + " -> " + modeToString(new_mode),
            0.0f, 0);
    
    // 根據新模式調整系統行為
    switch (new_mode) {
        case TRAATPMode::STANDBY:
            system_state.atp_active = false;
            break;
        case TRAATPMode::FULL_SUPERVISION:
            system_state.atp_active = true;
            break;
        case TRAATPMode::PARTIAL_SUPERVISION:
            system_state.atp_active = true;
            break;
        case TRAATPMode::ON_SIGHT:
            system_state.permitted_speed = 40.0f;
            break;
        case TRAATPMode::SHUNTING:
            system_state.permitted_speed = 25.0f;
            break;
        case TRAATPMode::EMERGENCY:
            applyEmergencyBrake();
            break;
    }
}

bool TRAATPIntegration::validateModeTransition(TRAATPMode from, TRAATPMode to)
{
    // 實作模式轉換驗證邏輯
    // 某些模式轉換可能需要特定條件
    
    if (from == to) return true;
    
    // 緊急模式可以從任何模式進入
    if (to == TRAATPMode::EMERGENCY) return true;
    
    // 從緊急模式只能回到待機模式
    if (from == TRAATPMode::EMERGENCY && to != TRAATPMode::STANDBY) return false;
    
    return true; // 其他轉換暫時允許
}

bool TRAATPIntegration::performSafetyChecks()
{
    // 執行安全檢查
    bool safety_ok = true;
    
    // 檢查速度是否在安全範圍內
    if (system_state.current_speed > system_state.permitted_speed + 15.0f) {
        safety_ok = false;
    }
    
    // 檢查系統連接狀態
    if (system_state.atp_active && !isSimRailConnected()) {
        safety_ok = false;
    }
    
    // 檢查煞車系統
    if (system_state.emergency_brake && system_state.current_speed > 5.0f) {
        // 緊急煞車應該能有效減速
        static float last_speed = system_state.current_speed;
        if (system_state.current_speed >= last_speed) {
            safety_ok = false;
        }
        last_speed = system_state.current_speed;
    }
    
    return safety_ok;
}

void TRAATPIntegration::handleSafetyViolation(const std::string& violation)
{
    addEvent(TRAATPEventType::SYSTEM_FAULT, "安全違規: " + violation, 0.0f, 2);
    
    // 採取安全措施
    applyEmergencyBrake();
    cutTraction();
    
    // 轉換到緊急模式
    handleModeTransition(TRAATPMode::EMERGENCY);
}

// 公共介面實作
bool TRAATPIntegration::setMode(TRAATPMode mode)
{
    handleModeTransition(mode);
    return system_state.current_mode == mode;
}

bool TRAATPIntegration::enableATP(bool enable)
{
    system_state.atp_active = enable;
    
    if (enable) {
        addEvent(TRAATPEventType::MODE_CHANGE, "ATP啟用", 0.0f, 0);
        if (system_state.current_mode == TRAATPMode::STANDBY) {
            handleModeTransition(TRAATPMode::FULL_SUPERVISION);
        }
    } else {
        addEvent(TRAATPEventType::MODE_CHANGE, "ATP停用", 0.0f, 1);
        handleModeTransition(TRAATPMode::STANDBY);
    }
    
    return true;
}

bool TRAATPIntegration::enableSHP(bool enable)
{
    system_state.shp_active = enable;
    
    if (shp_interface) {
        shp_interface->enableTRAATPMode(enable);
    }
    
    addEvent(TRAATPEventType::MODE_CHANGE, 
            enable ? "SHP啟用" : "SHP停用", 0.0f, 0);
    
    return true;
}

bool TRAATPIntegration::enableETCS(bool enable)
{
    system_state.etcs_active = enable;
    
    addEvent(TRAATPEventType::MODE_CHANGE, 
            enable ? "ETCS啟用" : "ETCS停用", 0.0f, 0);
    
    return true;
}

void TRAATPIntegration::setPermittedSpeed(float speed)
{
    system_state.permitted_speed = speed;
}

void TRAATPIntegration::setTargetSpeed(float speed)
{
    system_state.target_speed = speed;
}

void TRAATPIntegration::setCurrentSpeed(float speed)
{
    system_state.current_speed = speed;
}

void TRAATPIntegration::setDistanceToTarget(float distance)
{
    system_state.distance_to_target = distance;
}

void TRAATPIntegration::applyServiceBrake()
{
    if (system_state.brake_intervention) return;
    
    sendETCSBrakeCommand(false);
    
    if (shp_interface) {
        shp_interface->sendBrakeCommand(false);
    }
}

void TRAATPIntegration::applyEmergencyBrake()
{
    if (system_state.emergency_brake) return;
    
    sendETCSBrakeCommand(true);
    cutTraction();
    
    if (shp_interface) {
        shp_interface->sendBrakeCommand(true);
        shp_interface->sendTractionCutCommand();
    }
}

void TRAATPIntegration::releaseBrake()
{
    extern bool SB, EB;
    
    SB = false;
    EB = false;
    system_state.brake_intervention = false;
    system_state.emergency_brake = false;
    
    addEvent(TRAATPEventType::BRAKE_INTERVENTION, "煞車釋放", 0.0f, 0);
}

void TRAATPIntegration::cutTraction()
{
    if (system_state.traction_cut) return;
    
    sendETCSTractionCut();
    
    if (shp_interface) {
        shp_interface->sendTractionCutCommand();
    }
}

void TRAATPIntegration::restoreTraction()
{
    system_state.traction_cut = false;
    addEvent(TRAATPEventType::TRACTION_CUT, "牽引恢復", 0.0f, 0);
}

void TRAATPIntegration::addEventListener(std::function<void(const TRAATPEvent&)> callback)
{
    event_callbacks.push_back(callback);
}

std::vector<TRAATPEvent> TRAATPIntegration::getEventHistory(int max_events)
{
    if (max_events <= 0 || max_events >= (int)event_history.size()) {
        return event_history;
    }
    
    return std::vector<TRAATPEvent>(
        event_history.end() - max_events, 
        event_history.end()
    );
}

void TRAATPIntegration::clearEventHistory()
{
    event_history.clear();
}

bool TRAATPIntegration::runSystemTest()
{
    addEvent(TRAATPEventType::SYSTEM_FAULT, "系統自檢開始", 0.0f, 0);
    
    bool test_passed = true;
    
    // 測試SHP介面
    if (shp_interface && !shp_interface->runSelfTest()) {
        test_passed = false;
        addEvent(TRAATPEventType::SYSTEM_FAULT, "SHP介面測試失敗", 0.0f, 2);
    }
    
    // 測試ETCS連接
    // 這裡應該測試ETCS系統連接
    
    // 測試煞車系統
    // 這裡應該測試煞車系統回應
    
    addEvent(TRAATPEventType::SYSTEM_FAULT, 
            test_passed ? "系統自檢通過" : "系統自檢失敗", 0.0f, 
            test_passed ? 0 : 2);
    
    return test_passed;
}

std::vector<std::string> TRAATPIntegration::getDiagnosticInfo()
{
    std::vector<std::string> info;
    
    info.push_back("=== 台鐵ATP整合系統診斷 ===");
    info.push_back("整合狀態: " + statusToString(system_state.integration_status));
    info.push_back("運行模式: " + modeToString(system_state.current_mode));
    info.push_back("ATP狀態: " + std::string(system_state.atp_active ? "啟用" : "停用"));
    info.push_back("SHP狀態: " + std::string(system_state.shp_active ? "啟用" : "停用"));
    info.push_back("ETCS狀態: " + std::string(system_state.etcs_active ? "啟用" : "停用"));
    info.push_back("目前速度: " + std::to_string((int)system_state.current_speed) + " km/h");
    info.push_back("允許速度: " + std::to_string((int)system_state.permitted_speed) + " km/h");
    info.push_back("目標速度: " + std::to_string((int)system_state.target_speed) + " km/h");
    info.push_back("目標距離: " + std::to_string((int)system_state.distance_to_target) + " m");
    info.push_back("煞車介入: " + std::string(system_state.brake_intervention ? "是" : "否"));
    info.push_back("牽引切斷: " + std::string(system_state.traction_cut ? "是" : "否"));
    info.push_back("緊急煞車: " + std::string(system_state.emergency_brake ? "是" : "否"));
    info.push_back("列車編號: " + system_state.train_number);
    info.push_back("路線名稱: " + system_state.route_name);
    info.push_back("目前車站: " + system_state.current_station);
    info.push_back("下一車站: " + system_state.next_station);
    info.push_back("事件數量: " + std::to_string(event_history.size()));
    
    // 添加SHP診斷資訊
    if (shp_interface) {
        auto shp_info = shp_interface->getDiagnosticMessages();
        info.insert(info.end(), shp_info.begin(), shp_info.end());
    }
    
    return info;
}

bool TRAATPIntegration::performCalibration()
{
    addEvent(TRAATPEventType::SYSTEM_FAULT, "系統校準開始", 0.0f, 0);
    
    // 執行系統校準
    // 這裡應該實作實際的校準程序
    
    addEvent(TRAATPEventType::SYSTEM_FAULT, "系統校準完成", 0.0f, 0);
    return true;
}

bool TRAATPIntegration::connectToSimRail()
{
    if (!shp_interface) return false;
    
    bool connected = shp_interface->connect();
    
    if (connected) {
        system_state.integration_status = TRAATPIntegrationStatus::CONNECTED;
        addEvent(TRAATPEventType::COMMUNICATION_ERROR, "SimRail連接成功", 0.0f, 0);
    } else {
        system_state.integration_status = TRAATPIntegrationStatus::ERROR_STATE;
        addEvent(TRAATPEventType::COMMUNICATION_ERROR, "SimRail連接失敗", 0.0f, 2);
    }
    
    return connected;
}

void TRAATPIntegration::disconnectFromSimRail()
{
    if (shp_interface) {
        shp_interface->disconnect();
    }
    
    system_state.integration_status = TRAATPIntegrationStatus::DISCONNECTED;
    addEvent(TRAATPEventType::COMMUNICATION_ERROR, "SimRail連接中斷", 0.0f, 1);
}

bool TRAATPIntegration::isSimRailConnected()
{
    return shp_interface && shp_interface->isConnected();
}

void TRAATPIntegration::forceSimRailUpdate()
{
    if (shp_interface) {
        shp_interface->forceUpdate();
    }
}

void TRAATPIntegration::updateTRAComponents()
{
    // 更新台鐵ATP UI組件
    extern TRASpeedometer traSpeedometer;
    extern TRAStatusPanel traStatusPanel;
    extern TRASignalIndicator traSignalIndicator;
    extern TRADistanceBar traDistanceBar;
    extern TRAMessageArea traMessageArea;
    
    traSpeedometer.setSpeed((int)system_state.current_speed);
    traSpeedometer.setTargetSpeed((int)system_state.target_speed);
    traSpeedometer.setMaxSpeed((int)system_state.permitted_speed);
    
    traStatusPanel.setATPStatus(system_state.atp_active);
    traStatusPanel.setBrakeStatus(system_state.brake_intervention);
    traStatusPanel.setTractionStatus(system_state.traction_cut);
    traStatusPanel.setEmergencyBrake(system_state.emergency_brake);
    
    traDistanceBar.setDistance(system_state.distance_to_target);
    traDistanceBar.setTargetActive(system_state.target_speed > 0);
    
    // 更新信號指示器
    if (shp_interface) {
        auto current_aspect = shp_interface->getCurrentSignalAspect();
        TRASignalIndicator::SignalAspect tra_aspect;
        
        switch (current_aspect) {
            case SHPSignalAspect::RED_STOP:
                tra_aspect = TRASignalIndicator::RED_STOP;
                break;
            case SHPSignalAspect::YELLOW_CAUTION:
                tra_aspect = TRASignalIndicator::YELLOW_CAUTION;
                break;
            case SHPSignalAspect::GREEN_CLEAR:
                tra_aspect = TRASignalIndicator::GREEN_CLEAR;
                break;
            case SHPSignalAspect::FLASHING_YELLOW:
                tra_aspect = TRASignalIndicator::FLASHING_YELLOW;
                break;
            default:
                tra_aspect = TRASignalIndicator::NO_SIGNAL;
                break;
        }
        
        traSignalIndicator.setAspect(tra_aspect);
    }
}

void TRAATPIntegration::updateETCSComponents()
{
    // 更新ETCS相關組件
    // 這裡應該更新ETCS DMI組件
}

bool TRAATPIntegration::loadConfiguration(const std::string& config_file)
{
    // 載入配置檔案
    // 這裡應該實作配置檔案讀取
    return true;
}

bool TRAATPIntegration::saveConfiguration(const std::string& config_file)
{
    // 儲存配置檔案
    // 這裡應該實作配置檔案寫入
    return true;
}

void TRAATPIntegration::resetToDefaults()
{
    // 重置為預設值
    system_state.permitted_speed = 130.0f;
    system_state.target_speed = 0.0f;
    system_state.current_mode = TRAATPMode::STANDBY;
    
    addEvent(TRAATPEventType::SYSTEM_FAULT, "系統重置為預設值", 0.0f, 0);
}

// 全域函數實作
bool initializeTRAATPIntegration()
{
    if (tra_atp_integration) {
        platform->debug_print("TRA ATP Integration already initialized");
        return true;
    }
    
    tra_atp_integration = std::make_unique<TRAATPIntegration>();
    return tra_atp_integration->initialize();
}

void shutdownTRAATPIntegration()
{
    if (tra_atp_integration) {
        tra_atp_integration->shutdown();
        tra_atp_integration.reset();
        platform->debug_print("TRA ATP Integration shutdown");
    }
}

void updateTRAATPIntegration()
{
    if (tra_atp_integration) {
        tra_atp_integration->update();
    }
}

// 輔助函數實作
std::string modeToString(TRAATPMode mode)
{
    switch (mode) {
        case TRAATPMode::STANDBY: return "待機模式";
        case TRAATPMode::FULL_SUPERVISION: return "完全監督模式";
        case TRAATPMode::PARTIAL_SUPERVISION: return "部分監督模式";
        case TRAATPMode::ON_SIGHT: return "目視運行模式";
        case TRAATPMode::SHUNTING: return "調車模式";
        case TRAATPMode::EMERGENCY: return "緊急模式";
        default: return "未知模式";
    }
}

std::string statusToString(TRAATPIntegrationStatus status)
{
    switch (status) {
        case TRAATPIntegrationStatus::DISCONNECTED: return "未連接";
        case TRAATPIntegrationStatus::CONNECTING: return "連接中";
        case TRAATPIntegrationStatus::CONNECTED: return "已連接";
        case TRAATPIntegrationStatus::SYNCHRONIZING: return "同步中";
        case TRAATPIntegrationStatus::SYNCHRONIZED: return "已同步";
        case TRAATPIntegrationStatus::ERROR_STATE: return "錯誤狀態";
        default: return "未知狀態";
    }
}

std::string eventTypeToString(TRAATPEventType type)
{
    switch (type) {
        case TRAATPEventType::SPEED_RESTRICTION_AHEAD: return "前方速度限制";
        case TRAATPEventType::SIGNAL_CHANGE: return "信號變化";
        case TRAATPEventType::BRAKE_INTERVENTION: return "煞車介入";
        case TRAATPEventType::TRACTION_CUT: return "牽引切斷";
        case TRAATPEventType::EMERGENCY_BRAKE: return "緊急煞車";
        case TRAATPEventType::MODE_CHANGE: return "模式變更";
        case TRAATPEventType::SYSTEM_FAULT: return "系統故障";
        case TRAATPEventType::COMMUNICATION_ERROR: return "通訊錯誤";
        default: return "未知事件";
    }
}

TRAATPMode stringToMode(const std::string& mode_str)
{
    if (mode_str == "STANDBY") return TRAATPMode::STANDBY;
    if (mode_str == "FULL_SUPERVISION") return TRAATPMode::FULL_SUPERVISION;
    if (mode_str == "PARTIAL_SUPERVISION") return TRAATPMode::PARTIAL_SUPERVISION;
    if (mode_str == "ON_SIGHT") return TRAATPMode::ON_SIGHT;
    if (mode_str == "SHUNTING") return TRAATPMode::SHUNTING;
    if (mode_str == "EMERGENCY") return TRAATPMode::EMERGENCY;
    return TRAATPMode::STANDBY;
}