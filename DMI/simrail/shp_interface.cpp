/*
 * SimRail SHP Interface for Taiwan Railway ATP - Implementation
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

#include "shp_interface.h"
#include "../graphics/tra_components.h"
#include "../../EVC/Supervision/supervision.h"
#include "../../EVC/Packets/messages.h"
#include "../../platform/platform_runtime.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>

using json = nlohmann::json;

// 全局SHP介面實例
std::unique_ptr<SHPInterface> shp_interface = nullptr;

// SHPInterface Implementation
SHPInterface::SHPInterface() 
{
    system_status = {};
    system_status.connected = false;
    system_status.shp_active = false;
    system_status.atp_mode = false;
    system_status.current_speed = 0.0f;
    system_status.target_speed = 0.0f;
    system_status.distance_to_target = 0.0f;
    system_status.train_number = "";
    system_status.route = "";
}

SHPInterface::~SHPInterface()
{
    disconnect();
}

bool SHPInterface::initialize()
{
    if (initialized) return true;
    
    platform->debug_print("Initializing SHP Interface for TRA ATP");
    
    // 初始化SimRail連接
    if (!connect()) {
        platform->debug_print("Failed to connect to SimRail");
        return false;
    }
    
    // 設置預設回調
    setSignalCallback([this](const SHPSignalData& signal) {
        handleTRASignalChange(SHPSignalAspect::UNKNOWN, signal.aspect);
        sendSignalDataToETCS(signal);
    });
    
    setSpeedCallback([this](const SHPSpeedRestriction& restriction) {
        sendSpeedRestrictionToETCS(restriction);
    });
    
    setStatusCallback([this](const SHPSystemStatus& status) {
        // 更新台鐵ATP組件
        extern TRASpeedometer traSpeedometer;
        extern TRAStatusPanel traStatusPanel;
        extern TRASignalIndicator traSignalIndicator;
        
        traSpeedometer.setSpeed((int)status.current_speed);
        traSpeedometer.setTargetSpeed((int)status.target_speed);
        traStatusPanel.setATPStatus(status.atp_mode);
    });
    
    initialized = true;
    platform->debug_print("SHP Interface initialized successfully");
    return true;
}

bool SHPInterface::connect()
{
    // 嘗試連接到SimRail
    // 這裡應該實作實際的SimRail API連接
    // 目前使用模擬資料
    
    platform->debug_print("Connecting to SimRail...");
    
    // 模擬連接成功
    connected = true;
    system_status.connected = true;
    system_status.shp_active = true;
    system_status.atp_mode = true;
    system_status.train_number = "TRA-1001";
    system_status.route = "西部幹線";
    
    // 初始化模擬資料
    speed_restrictions.clear();
    signals.clear();
    track_data.clear();
    
    // 添加一些模擬的速度限制
    speed_restrictions.push_back({500.0f, 80, 85, false, "彎道限速", true});
    speed_restrictions.push_back({1200.0f, 60, 65, true, "施工限速", true});
    speed_restrictions.push_back({2000.0f, 110, 115, false, "正常限速", true});
    
    // 添加一些模擬的信號資料
    signals.push_back({300.0f, SHPSignalAspect::GREEN_CLEAR, 110, "S001", false, true});
    signals.push_back({800.0f, SHPSignalAspect::YELLOW_CAUTION, 80, "S002", true, true});
    signals.push_back({1500.0f, SHPSignalAspect::RED_STOP, 0, "S003", false, true});
    
    // 添加一些模擬的軌道資料
    track_data.push_back({100.0f, "1A", "台北", 1, false, false, false});
    track_data.push_back({600.0f, "1B", "", 0, true, false, false});
    track_data.push_back({1100.0f, "2A", "板橋", 2, false, false, true});
    
    platform->debug_print("Connected to SimRail successfully");
    return true;
}

void SHPInterface::disconnect()
{
    if (!connected) return;
    
    platform->debug_print("Disconnecting from SimRail");
    
    connected = false;
    system_status.connected = false;
    system_status.shp_active = false;
    
    // 清空資料
    speed_restrictions.clear();
    signals.clear();
    track_data.clear();
}

void SHPInterface::update()
{
    if (!connected) return;
    
    processSimRailData();
    updateSpeedRestrictions();
    updateSignalData();
    updateTrackData();
    
    // 觸發狀態回調
    if (status_callback) {
        status_callback(system_status);
    }
}

void SHPInterface::forceUpdate()
{
    if (!connected) return;
    
    platform->debug_print("Force updating SHP data from SimRail");
    update();
}

void SHPInterface::processSimRailData()
{
    // 這裡應該實作從SimRail API獲取實際資料的邏輯
    // 目前使用模擬資料更新
    
    // 模擬速度變化
    static float sim_speed = 0.0f;
    static int speed_direction = 1;
    
    sim_speed += speed_direction * 2.0f;
    if (sim_speed >= 120.0f) speed_direction = -1;
    if (sim_speed <= 0.0f) speed_direction = 1;
    
    system_status.current_speed = sim_speed;
    
    // 更新目標速度基於下一個速度限制
    auto* next_restriction = getNextSpeedRestriction();
    if (next_restriction) {
        system_status.target_speed = (float)next_restriction->speed_limit;
        system_status.distance_to_target = next_restriction->distance;
    }
}

void SHPInterface::updateSpeedRestrictions()
{
    // 更新速度限制距離（模擬列車移動）
    for (auto& restriction : speed_restrictions) {
        if (restriction.active) {
            restriction.distance -= system_status.current_speed * 0.01f; // 簡化計算
            
            if (restriction.distance <= 0) {
                restriction.active = false;
                
                // 觸發速度回調
                if (speed_callback) {
                    speed_callback(restriction);
                }
            }
        }
    }
    
    // 移除已過的限速
    speed_restrictions.erase(
        std::remove_if(speed_restrictions.begin(), speed_restrictions.end(),
                      [](const SHPSpeedRestriction& r) { return !r.active && r.distance < -100.0f; }),
        speed_restrictions.end()
    );
}

void SHPInterface::updateSignalData()
{
    // 更新信號距離
    for (auto& signal : signals) {
        if (signal.active) {
            signal.distance -= system_status.current_speed * 0.01f;
            
            if (signal.distance <= 0) {
                // 通過信號機
                signal.active = false;
                
                // 觸發信號回調
                if (signal_callback) {
                    signal_callback(signal);
                }
            }
        }
    }
    
    // 移除已過的信號
    signals.erase(
        std::remove_if(signals.begin(), signals.end(),
                      [](const SHPSignalData& s) { return !s.active && s.distance < -50.0f; }),
        signals.end()
    );
}

void SHPInterface::updateTrackData()
{
    // 更新軌道元素距離
    for (auto& track : track_data) {
        track.distance -= system_status.current_speed * 0.01f;
    }
    
    // 移除已過的軌道元素
    track_data.erase(
        std::remove_if(track_data.begin(), track_data.end(),
                      [](const SHPTrackData& t) { return t.distance < -100.0f; }),
        track_data.end()
    );
}

SHPSpeedRestriction* SHPInterface::getNextSpeedRestriction()
{
    auto it = std::min_element(speed_restrictions.begin(), speed_restrictions.end(),
                              [](const SHPSpeedRestriction& a, const SHPSpeedRestriction& b) {
                                  return a.active && a.distance < b.distance;
                              });
    
    return (it != speed_restrictions.end() && it->active) ? &(*it) : nullptr;
}

SHPSignalData* SHPInterface::getNextSignal()
{
    auto it = std::min_element(signals.begin(), signals.end(),
                              [](const SHPSignalData& a, const SHPSignalData& b) {
                                  return a.active && a.distance < b.distance;
                              });
    
    return (it != signals.end() && it->active) ? &(*it) : nullptr;
}

SHPTrackData* SHPInterface::getNextTrackElement()
{
    auto it = std::min_element(track_data.begin(), track_data.end(),
                              [](const SHPTrackData& a, const SHPTrackData& b) {
                                  return a.distance < b.distance;
                              });
    
    return (it != track_data.end()) ? &(*it) : nullptr;
}

int SHPInterface::getCurrentSpeedLimit()
{
    auto* restriction = getNextSpeedRestriction();
    return restriction ? restriction->speed_limit : 130; // 預設最高速度
}

SHPSignalAspect SHPInterface::getCurrentSignalAspect()
{
    auto* signal = getNextSignal();
    return signal ? signal->aspect : SHPSignalAspect::UNKNOWN;
}

void SHPInterface::setSignalCallback(std::function<void(const SHPSignalData&)> callback)
{
    signal_callback = callback;
}

void SHPInterface::setSpeedCallback(std::function<void(const SHPSpeedRestriction&)> callback)
{
    speed_callback = callback;
}

void SHPInterface::setStatusCallback(std::function<void(const SHPSystemStatus&)> callback)
{
    status_callback = callback;
}

void SHPInterface::sendSpeedRestrictionToETCS(const SHPSpeedRestriction& restriction)
{
    // 將SHP速度限制轉換為ETCS格式並發送
    platform->debug_print("Sending speed restriction to ETCS: " + 
                          std::to_string(restriction.speed_limit) + " km/h at " + 
                          std::to_string(restriction.distance) + "m");
    
    // 這裡應該實作實際的ETCS介面
    // 目前更新台鐵ATP組件
    extern TRASpeedometer traSpeedometer;
    extern TRADistanceBar traDistanceBar;
    extern TRAMessageArea traMessageArea;
    
    traSpeedometer.setTargetSpeed(restriction.speed_limit);
    traSpeedometer.setWarningSpeed(restriction.warning_speed);
    traDistanceBar.setDistance(restriction.distance);
    traDistanceBar.setTargetActive(true);
    
    std::string message = "速度限制: " + std::to_string(restriction.speed_limit) + " km/h";
    if (restriction.temporary) {
        message += " (臨時)";
    }
    if (!restriction.reason.empty()) {
        message += " - " + restriction.reason;
    }
    
    traMessageArea.addMessage(message, restriction.temporary ? TRA_Orange : TRA_Yellow);
}

void SHPInterface::sendSignalDataToETCS(const SHPSignalData& signal)
{
    // 將SHP信號資料轉換為ETCS格式並發送
    platform->debug_print("Sending signal data to ETCS: " + 
                          signalAspectToString(signal.aspect) + " at " + 
                          std::to_string(signal.distance) + "m");
    
    // 更新台鐵ATP組件
    extern TRASignalIndicator traSignalIndicator;
    extern TRAMessageArea traMessageArea;
    
    // 轉換SHP信號到TRA信號
    TRASignalIndicator::SignalAspect tra_aspect;
    switch (signal.aspect) {
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
    
    std::string message = "信號 " + signal.signal_id + ": " + signalAspectToString(signal.aspect);
    Color message_color = signalAspectToColor(signal.aspect);
    traMessageArea.addMessage(message, message_color);
    
    // 如果是停車信號，觸發煞車
    if (signal.aspect == SHPSignalAspect::RED_STOP && signal.distance < 100.0f) {
        sendBrakeCommand(false);
    }
}

void SHPInterface::sendBrakeCommand(bool emergency)
{
    platform->debug_print(emergency ? "Sending emergency brake command" : "Sending service brake command");
    
    // 更新台鐵ATP組件
    extern TRAStatusPanel traStatusPanel;
    extern TRAMessageArea traMessageArea;
    
    traStatusPanel.setBrakeStatus(true);
    if (emergency) {
        traStatusPanel.setEmergencyBrake(true);
        traMessageArea.addMessage("緊急煞車作用", TRA_Red);
    } else {
        traMessageArea.addMessage("常用煞車作用", TRA_Orange);
    }
    
    // 這裡應該實作實際的煞車控制介面
}

void SHPInterface::sendTractionCutCommand()
{
    platform->debug_print("Sending traction cut command");
    
    // 更新台鐵ATP組件
    extern TRAStatusPanel traStatusPanel;
    extern TRAMessageArea traMessageArea;
    
    traStatusPanel.setTractionStatus(true);
    traMessageArea.addMessage("牽引切斷", TRA_Yellow);
    
    // 這裡應該實作實際的牽引控制介面
}

void SHPInterface::enableTRAATPMode(bool enable)
{
    system_status.atp_mode = enable;
    
    extern TRAStatusPanel traStatusPanel;
    extern TRAMessageArea traMessageArea;
    
    traStatusPanel.setATPStatus(enable);
    
    if (enable) {
        traMessageArea.addMessage("台鐵ATP模式啟動", TRA_Green);
        platform->debug_print("TRA ATP mode enabled");
    } else {
        traMessageArea.addMessage("台鐵ATP模式關閉", TRA_Yellow);
        platform->debug_print("TRA ATP mode disabled");
    }
}

void SHPInterface::setTRASpeedProfile(const std::vector<SHPSpeedRestriction>& profile)
{
    speed_restrictions = profile;
    
    extern TRAMessageArea traMessageArea;
    traMessageArea.addMessage("速度設定檔已更新 (" + std::to_string(profile.size()) + " 個限制)", TRA_Blue);
    
    platform->debug_print("TRA speed profile updated with " + std::to_string(profile.size()) + " restrictions");
}

void SHPInterface::handleTRASignalChange(SHPSignalAspect old_aspect, SHPSignalAspect new_aspect)
{
    if (old_aspect == new_aspect) return;
    
    extern TRAMessageArea traMessageArea;
    
    std::string message = "信號變化: " + signalAspectToString(old_aspect) + " → " + signalAspectToString(new_aspect);
    Color message_color = signalAspectToColor(new_aspect);
    
    traMessageArea.addMessage(message, message_color);
    
    // 根據信號變化採取行動
    switch (new_aspect) {
        case SHPSignalAspect::RED_STOP:
            sendBrakeCommand(false);
            break;
        case SHPSignalAspect::YELLOW_CAUTION:
            // 準備減速
            break;
        case SHPSignalAspect::GREEN_CLEAR:
            // 可以正常行駛
            break;
        default:
            break;
    }
    
    platform->debug_print("Signal changed from " + signalAspectToString(old_aspect) + 
                          " to " + signalAspectToString(new_aspect));
}

bool SHPInterface::runSelfTest()
{
    platform->debug_print("Running SHP interface self test");
    
    bool test_passed = true;
    
    // 測試連接
    if (!connected) {
        platform->debug_print("Self test failed: Not connected to SimRail");
        test_passed = false;
    }
    
    // 測試資料完整性
    if (speed_restrictions.empty()) {
        platform->debug_print("Self test warning: No speed restrictions available");
    }
    
    if (signals.empty()) {
        platform->debug_print("Self test warning: No signal data available");
    }
    
    // 測試回調函數
    if (!signal_callback || !speed_callback || !status_callback) {
        platform->debug_print("Self test failed: Missing callback functions");
        test_passed = false;
    }
    
    platform->debug_print("SHP interface self test " + std::string(test_passed ? "passed" : "failed"));
    return test_passed;
}

std::vector<std::string> SHPInterface::getDiagnosticMessages()
{
    std::vector<std::string> messages;
    
    messages.push_back("=== SHP介面診斷 ===");
    messages.push_back("連接狀態: " + std::string(connected ? "已連接" : "未連接"));
    messages.push_back("SHP狀態: " + std::string(system_status.shp_active ? "啟用" : "停用"));
    messages.push_back("ATP模式: " + std::string(system_status.atp_mode ? "啟用" : "停用"));
    messages.push_back("列車編號: " + system_status.train_number);
    messages.push_back("路線: " + system_status.route);
    messages.push_back("目前速度: " + std::to_string((int)system_status.current_speed) + " km/h");
    messages.push_back("目標速度: " + std::to_string((int)system_status.target_speed) + " km/h");
    messages.push_back("速度限制數量: " + std::to_string(speed_restrictions.size()));
    messages.push_back("信號數量: " + std::to_string(signals.size()));
    messages.push_back("軌道元素數量: " + std::to_string(track_data.size()));
    
    return messages;
}

void SHPInterface::clearDiagnosticMessages()
{
    // 清空診斷訊息（如果有緩存的話）
}

// 輔助函數實作
std::string signalAspectToString(SHPSignalAspect aspect)
{
    switch (aspect) {
        case SHPSignalAspect::RED_STOP: return "紅燈停車";
        case SHPSignalAspect::YELLOW_CAUTION: return "黃燈注意";
        case SHPSignalAspect::GREEN_CLEAR: return "綠燈正常";
        case SHPSignalAspect::YELLOW_YELLOW: return "雙黃燈";
        case SHPSignalAspect::GREEN_YELLOW: return "綠黃燈";
        case SHPSignalAspect::FLASHING_YELLOW: return "閃黃燈";
        case SHPSignalAspect::UNKNOWN: return "未知";
        default: return "無效";
    }
}

Color signalAspectToColor(SHPSignalAspect aspect)
{
    switch (aspect) {
        case SHPSignalAspect::RED_STOP: return TRA_Red;
        case SHPSignalAspect::YELLOW_CAUTION: return TRA_Yellow;
        case SHPSignalAspect::GREEN_CLEAR: return TRA_Green;
        case SHPSignalAspect::YELLOW_YELLOW: return TRA_Yellow;
        case SHPSignalAspect::GREEN_YELLOW: return TRA_Green;
        case SHPSignalAspect::FLASHING_YELLOW: return TRA_Orange;
        case SHPSignalAspect::UNKNOWN: return TRA_Inactive;
        default: return TRA_Text;
    }
}

SHPSignalAspect stringToSignalAspect(const std::string& aspect_str)
{
    if (aspect_str == "RED" || aspect_str == "0") return SHPSignalAspect::RED_STOP;
    if (aspect_str == "YELLOW" || aspect_str == "1") return SHPSignalAspect::YELLOW_CAUTION;
    if (aspect_str == "GREEN" || aspect_str == "2") return SHPSignalAspect::GREEN_CLEAR;
    if (aspect_str == "YELLOW_YELLOW" || aspect_str == "3") return SHPSignalAspect::YELLOW_YELLOW;
    if (aspect_str == "GREEN_YELLOW" || aspect_str == "4") return SHPSignalAspect::GREEN_YELLOW;
    if (aspect_str == "FLASHING_YELLOW" || aspect_str == "5") return SHPSignalAspect::FLASHING_YELLOW;
    return SHPSignalAspect::UNKNOWN;
}

// 全局函數實作
bool initializeSHPInterface()
{
    if (shp_interface) {
        platform->debug_print("SHP interface already initialized");
        return true;
    }
    
    shp_interface = std::make_unique<SHPInterface>();
    return shp_interface->initialize();
}

void shutdownSHPInterface()
{
    if (shp_interface) {
        shp_interface->disconnect();
        shp_interface.reset();
        platform->debug_print("SHP interface shutdown");
    }
}

// SimRail資料解析函數實作
SHPSystemStatus parseSimRailSystemStatus(const std::string& data)
{
    SHPSystemStatus status = {};
    
    try {
        json j = json::parse(data);
        
        status.connected = j.value("connected", false);
        status.shp_active = j.value("shp_active", false);
        status.atp_mode = j.value("atp_mode", false);
        status.current_speed = j.value("current_speed", 0.0f);
        status.target_speed = j.value("target_speed", 0.0f);
        status.distance_to_target = j.value("distance_to_target", 0.0f);
        status.train_number = j.value("train_number", "");
        status.route = j.value("route", "");
        
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse SimRail system status: " + std::string(e.what()));
    }
    
    return status;
}

std::vector<SHPSpeedRestriction> parseSimRailSpeedData(const std::string& data)
{
    std::vector<SHPSpeedRestriction> restrictions;
    
    try {
        json j = json::parse(data);
        
        if (j.contains("speed_restrictions") && j["speed_restrictions"].is_array()) {
            for (const auto& item : j["speed_restrictions"]) {
                SHPSpeedRestriction restriction;
                restriction.distance = item.value("distance", 0.0f);
                restriction.speed_limit = item.value("speed_limit", 130);
                restriction.warning_speed = item.value("warning_speed", restriction.speed_limit + 5);
                restriction.temporary = item.value("temporary", false);
                restriction.reason = item.value("reason", "");
                restriction.active = item.value("active", true);
                
                restrictions.push_back(restriction);
            }
        }
        
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse SimRail speed data: " + std::string(e.what()));
    }
    
    return restrictions;
}

std::vector<SHPSignalData> parseSimRailSignalData(const std::string& data)
{
    std::vector<SHPSignalData> signals;
    
    try {
        json j = json::parse(data);
        
        if (j.contains("signals") && j["signals"].is_array()) {
            for (const auto& item : j["signals"]) {
                SHPSignalData signal;
                signal.distance = item.value("distance", 0.0f);
                signal.aspect = stringToSignalAspect(item.value("aspect", "UNKNOWN"));
                signal.speed_limit = item.value("speed_limit", 130);
                signal.signal_id = item.value("signal_id", "");
                signal.approach_control = item.value("approach_control", false);
                signal.active = item.value("active", true);
                
                signals.push_back(signal);
            }
        }
        
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse SimRail signal data: " + std::string(e.what()));
    }
    
    return signals;
}

std::vector<SHPTrackData> parseSimRailTrackData(const std::string& data)
{
    std::vector<SHPTrackData> track_elements;
    
    try {
        json j = json::parse(data);
        
        if (j.contains("track_data") && j["track_data"].is_array()) {
            for (const auto& item : j["track_data"]) {
                SHPTrackData track;
                track.distance = item.value("distance", 0.0f);
                track.track_id = item.value("track_id", "");
                track.station = item.value("station", "");
                track.platform = item.value("platform", 0);
                track.level_crossing = item.value("level_crossing", false);
                track.tunnel = item.value("tunnel", false);
                track.bridge = item.value("bridge", false);
                
                track_elements.push_back(track);
            }
        }
        
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse SimRail track data: " + std::string(e.what()));
    }
    
    return track_elements;
}