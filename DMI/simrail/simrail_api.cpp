/*
 * SimRail API Interface for Taiwan Railway ATP - Implementation
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

#include "simrail_api.h"
#include "../../platform/platform_runtime.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <chrono>

using json = nlohmann::json;

// 全域SimRail API實例
std::unique_ptr<SimRailAPI> simrail_api = nullptr;

SimRailAPI::SimRailAPI()
{
    connection_status = SimRailConnectionStatus::DISCONNECTED;
    api_endpoint = "localhost";
    api_port = 8080;
    auto_reconnect = true;
    reconnect_interval = 5000; // 5 seconds
    last_connection_attempt = 0;
    
    // 初始化快取資料
    cached_train_data = {};
    cached_signals.clear();
    cached_track_data.clear();
    cached_system_status = {};
}

SimRailAPI::~SimRailAPI()
{
    disconnect();
}

bool SimRailAPI::connect(const std::string& endpoint, int port)
{
    api_endpoint = endpoint;
    api_port = port;
    
    platform->debug_print("Connecting to SimRail API at " + endpoint + ":" + std::to_string(port));
    
    connection_status = SimRailConnectionStatus::CONNECTING;
    
    if (connection_callback) {
        connection_callback(connection_status);
    }
    
    if (establishConnection()) {
        connection_status = SimRailConnectionStatus::CONNECTED;
        platform->debug_print("Successfully connected to SimRail API");
        
        if (connection_callback) {
            connection_callback(connection_status);
        }
        
        return true;
    } else {
        connection_status = SimRailConnectionStatus::ERROR;
        platform->debug_print("Failed to connect to SimRail API");
        
        if (connection_callback) {
            connection_callback(connection_status);
        }
        
        return false;
    }
}

void SimRailAPI::disconnect()
{
    if (connection_status == SimRailConnectionStatus::DISCONNECTED) return;
    
    platform->debug_print("Disconnecting from SimRail API");
    
    closeConnection();
    connection_status = SimRailConnectionStatus::DISCONNECTED;
    
    if (connection_callback) {
        connection_callback(connection_status);
    }
}

bool SimRailAPI::establishConnection()
{
    // 實際的SimRail API連接邏輯
    // 這裡應該實作HTTP/WebSocket連接到SimRail
    // 目前使用模擬連接
    
    last_connection_attempt = platform->get_timer();
    
    // 模擬連接延遲
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 模擬連接成功
    return true;
}

void SimRailAPI::closeConnection()
{
    // 關閉實際連接
    // 清空快取
    clearCache();
}

bool SimRailAPI::sendRequest(const std::string& endpoint, std::string& response)
{
    if (connection_status != SimRailConnectionStatus::CONNECTED) {
        return false;
    }
    
    // 實際的HTTP請求邏輯
    // 這裡應該實作HTTP GET/POST請求到SimRail API
    // 目前返回模擬資料
    
    if (endpoint == "/api/train") {
        response = R"({
            "train_number": "TRA-1001",
            "route": "西部幹線",
            "current_speed": 85.5,
            "max_speed": 130,
            "distance_traveled": 15.2,
            "total_distance": 45.8,
            "doors_open": false,
            "pantograph_up": true,
            "main_breaker_on": true,
            "traction_active": true,
            "current_station": "台北",
            "next_station": "板橋",
            "distance_to_next_station": 3.2
        })";
    } else if (endpoint == "/api/signals") {
        response = R"({
            "signals": [
                {
                    "signal_id": "S001",
                    "aspect": 2,
                    "distance": 450.0,
                    "speed_limit": 110,
                    "approach_control": false,
                    "signal_name": "台北出發"
                },
                {
                    "signal_id": "S002",
                    "aspect": 1,
                    "distance": 1200.0,
                    "speed_limit": 80,
                    "approach_control": true,
                    "signal_name": "板橋進站"
                }
            ]
        })";
    } else if (endpoint == "/api/track") {
        response = R"({
            "track_data": [
                {
                    "distance": 200.0,
                    "speed_limit": 110,
                    "temporary_limit": false,
                    "limit_reason": "",
                    "track_section": "1A",
                    "level_crossing": false,
                    "station_area": false,
                    "station_name": ""
                },
                {
                    "distance": 800.0,
                    "speed_limit": 60,
                    "temporary_limit": true,
                    "limit_reason": "施工限速",
                    "track_section": "1B",
                    "level_crossing": true,
                    "station_area": false,
                    "station_name": ""
                },
                {
                    "distance": 1500.0,
                    "speed_limit": 40,
                    "temporary_limit": false,
                    "limit_reason": "",
                    "track_section": "2A",
                    "level_crossing": false,
                    "station_area": true,
                    "station_name": "板橋"
                }
            ]
        })";
    } else if (endpoint == "/api/system") {
        response = R"({
            "game_running": true,
            "train_selected": true,
            "shp_active": true,
            "sifa_active": true,
            "simulation_time": 3600.5,
            "weather": "晴天",
            "visibility": 10.0,
            "emergency_brake": false,
            "service_brake": false,
            "brake_pressure": 0.0
        })";
    } else {
        return false;
    }
    
    return true;
}

bool SimRailAPI::updateTrainData()
{
    std::string response;
    if (!sendRequest("/api/train", response)) {
        return false;
    }
    
    if (parseTrainData(response, cached_train_data)) {
        if (train_data_callback) {
            train_data_callback(cached_train_data);
        }
        return true;
    }
    
    return false;
}

bool SimRailAPI::updateSignalData()
{
    std::string response;
    if (!sendRequest("/api/signals", response)) {
        return false;
    }
    
    if (parseSignalData(response, cached_signals)) {
        if (signal_data_callback) {
            signal_data_callback(cached_signals);
        }
        return true;
    }
    
    return false;
}

bool SimRailAPI::updateTrackData()
{
    std::string response;
    if (!sendRequest("/api/track", response)) {
        return false;
    }
    
    if (parseTrackData(response, cached_track_data)) {
        if (track_data_callback) {
            track_data_callback(cached_track_data);
        }
        return true;
    }
    
    return false;
}

bool SimRailAPI::updateSystemStatus()
{
    std::string response;
    if (!sendRequest("/api/system", response)) {
        return false;
    }
    
    if (parseSystemStatus(response, cached_system_status)) {
        if (system_status_callback) {
            system_status_callback(cached_system_status);
        }
        return true;
    }
    
    return false;
}

bool SimRailAPI::updateAllData()
{
    bool success = true;
    
    success &= updateTrainData();
    success &= updateSignalData();
    success &= updateTrackData();
    success &= updateSystemStatus();
    
    return success;
}

bool SimRailAPI::parseTrainData(const std::string& json_data, SimRailTrainData& train_data)
{
    try {
        json j = json::parse(json_data);
        
        train_data.train_number = j.value("train_number", "");
        train_data.route = j.value("route", "");
        train_data.current_speed = j.value("current_speed", 0.0f);
        train_data.max_speed = j.value("max_speed", 130.0f);
        train_data.distance_traveled = j.value("distance_traveled", 0.0f);
        train_data.total_distance = j.value("total_distance", 0.0f);
        train_data.doors_open = j.value("doors_open", false);
        train_data.pantograph_up = j.value("pantograph_up", false);
        train_data.main_breaker_on = j.value("main_breaker_on", false);
        train_data.traction_active = j.value("traction_active", false);
        train_data.current_station = j.value("current_station", "");
        train_data.next_station = j.value("next_station", "");
        train_data.distance_to_next_station = j.value("distance_to_next_station", 0.0f);
        
        return true;
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse train data: " + std::string(e.what()));
        return false;
    }
}

bool SimRailAPI::parseSignalData(const std::string& json_data, std::vector<SimRailSignalData>& signals)
{
    try {
        json j = json::parse(json_data);
        signals.clear();
        
        if (j.contains("signals") && j["signals"].is_array()) {
            for (const auto& signal_json : j["signals"]) {
                SimRailSignalData signal;
                signal.signal_id = signal_json.value("signal_id", "");
                signal.aspect = signal_json.value("aspect", 0);
                signal.distance = signal_json.value("distance", 0.0f);
                signal.speed_limit = signal_json.value("speed_limit", 130);
                signal.approach_control = signal_json.value("approach_control", false);
                signal.signal_name = signal_json.value("signal_name", "");
                
                signals.push_back(signal);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse signal data: " + std::string(e.what()));
        return false;
    }
}

bool SimRailAPI::parseTrackData(const std::string& json_data, std::vector<SimRailTrackData>& track_data)
{
    try {
        json j = json::parse(json_data);
        track_data.clear();
        
        if (j.contains("track_data") && j["track_data"].is_array()) {
            for (const auto& track_json : j["track_data"]) {
                SimRailTrackData track;
                track.distance = track_json.value("distance", 0.0f);
                track.speed_limit = track_json.value("speed_limit", 130);
                track.temporary_limit = track_json.value("temporary_limit", false);
                track.limit_reason = track_json.value("limit_reason", "");
                track.track_section = track_json.value("track_section", "");
                track.level_crossing = track_json.value("level_crossing", false);
                track.station_area = track_json.value("station_area", false);
                track.station_name = track_json.value("station_name", "");
                
                track_data.push_back(track);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse track data: " + std::string(e.what()));
        return false;
    }
}

bool SimRailAPI::parseSystemStatus(const std::string& json_data, SimRailSystemStatus& status)
{
    try {
        json j = json::parse(json_data);
        
        status.game_running = j.value("game_running", false);
        status.train_selected = j.value("train_selected", false);
        status.shp_active = j.value("shp_active", false);
        status.sifa_active = j.value("sifa_active", false);
        status.simulation_time = j.value("simulation_time", 0.0f);
        status.weather = j.value("weather", "");
        status.visibility = j.value("visibility", 10.0f);
        status.emergency_brake = j.value("emergency_brake", false);
        status.service_brake = j.value("service_brake", false);
        status.brake_pressure = j.value("brake_pressure", 0.0f);
        
        return true;
    } catch (const std::exception& e) {
        platform->debug_print("Failed to parse system status: " + std::string(e.what()));
        return false;
    }
}

bool SimRailAPI::sendBrakeCommand(bool emergency)
{
    // 發送煞車命令到SimRail
    std::string endpoint = emergency ? "/api/emergency_brake" : "/api/service_brake";
    std::string response;
    
    // 這裡應該實作POST請求
    platform->debug_print("Sending brake command: " + std::string(emergency ? "emergency" : "service"));
    
    return true; // 模擬成功
}

bool SimRailAPI::sendTractionCommand(bool enable)
{
    // 發送牽引控制命令到SimRail
    platform->debug_print("Sending traction command: " + std::string(enable ? "enable" : "disable"));
    
    return true; // 模擬成功
}

bool SimRailAPI::sendHornCommand()
{
    // 發送鳴笛命令到SimRail
    platform->debug_print("Sending horn command");
    
    return true; // 模擬成功
}

bool SimRailAPI::sendPantographCommand(bool raise)
{
    // 發送集電弓控制命令到SimRail
    platform->debug_print("Sending pantograph command: " + std::string(raise ? "raise" : "lower"));
    
    return true; // 模擬成功
}

bool SimRailAPI::sendMainBreakerCommand(bool on)
{
    // 發送主斷路器控制命令到SimRail
    platform->debug_print("Sending main breaker command: " + std::string(on ? "on" : "off"));
    
    return true; // 模擬成功
}

bool SimRailAPI::sendDoorCommand(bool open)
{
    // 發送車門控制命令到SimRail
    platform->debug_print("Sending door command: " + std::string(open ? "open" : "close"));
    
    return true; // 模擬成功
}

void SimRailAPI::setTrainDataCallback(std::function<void(const SimRailTrainData&)> callback)
{
    train_data_callback = callback;
}

void SimRailAPI::setSignalDataCallback(std::function<void(const std::vector<SimRailSignalData>&)> callback)
{
    signal_data_callback = callback;
}

void SimRailAPI::setTrackDataCallback(std::function<void(const std::vector<SimRailTrackData>&)> callback)
{
    track_data_callback = callback;
}

void SimRailAPI::setSystemStatusCallback(std::function<void(const SimRailSystemStatus&)> callback)
{
    system_status_callback = callback;
}

void SimRailAPI::setConnectionCallback(std::function<void(SimRailConnectionStatus)> callback)
{
    connection_callback = callback;
}

std::vector<std::string> SimRailAPI::getDiagnosticInfo()
{
    std::vector<std::string> info;
    
    info.push_back("=== SimRail API診斷 ===");
    info.push_back("連接狀態: " + connectionStatusToString(connection_status));
    info.push_back("API端點: " + api_endpoint + ":" + std::to_string(api_port));
    info.push_back("自動重連: " + std::string(auto_reconnect ? "啟用" : "停用"));
    info.push_back("重連間隔: " + std::to_string(reconnect_interval) + "ms");
    
    if (connection_status == SimRailConnectionStatus::CONNECTED) {
        info.push_back("列車編號: " + cached_train_data.train_number);
        info.push_back("路線: " + cached_train_data.route);
        info.push_back("目前速度: " + std::to_string(cached_train_data.current_speed) + " km/h");
        info.push_back("信號數量: " + std::to_string(cached_signals.size()));
        info.push_back("軌道資料數量: " + std::to_string(cached_track_data.size()));
        info.push_back("遊戲運行: " + std::string(cached_system_status.game_running ? "是" : "否"));
        info.push_back("SHP啟用: " + std::string(cached_system_status.shp_active ? "是" : "否"));
    }
    
    return info;
}

bool SimRailAPI::testConnection()
{
    if (connection_status != SimRailConnectionStatus::CONNECTED) {
        return false;
    }
    
    std::string response;
    return sendRequest("/api/ping", response);
}

void SimRailAPI::clearCache()
{
    cached_train_data = {};
    cached_signals.clear();
    cached_track_data.clear();
    cached_system_status = {};
}

// 全域函數實作
bool initializeSimRailAPI()
{
    if (simrail_api) {
        platform->debug_print("SimRail API already initialized");
        return true;
    }
    
    simrail_api = std::make_unique<SimRailAPI>();
    platform->debug_print("SimRail API initialized");
    return true;
}

void shutdownSimRailAPI()
{
    if (simrail_api) {
        simrail_api->disconnect();
        simrail_api.reset();
        platform->debug_print("SimRail API shutdown");
    }
}

bool connectToSimRail(const std::string& endpoint, int port)
{
    if (!simrail_api) {
        if (!initializeSimRailAPI()) {
            return false;
        }
    }
    
    return simrail_api->connect(endpoint, port);
}

void disconnectFromSimRail()
{
    if (simrail_api) {
        simrail_api->disconnect();
    }
}

bool isSimRailConnected()
{
    return simrail_api && simrail_api->isConnected();
}

// 輔助函數實作
std::string connectionStatusToString(SimRailConnectionStatus status)
{
    switch (status) {
        case SimRailConnectionStatus::DISCONNECTED: return "未連接";
        case SimRailConnectionStatus::CONNECTING: return "連接中";
        case SimRailConnectionStatus::CONNECTED: return "已連接";
        case SimRailConnectionStatus::ERROR: return "錯誤";
        default: return "未知";
    }
}

std::string signalAspectToString(int aspect)
{
    switch (aspect) {
        case 0: return "紅燈停車";
        case 1: return "黃燈注意";
        case 2: return "綠燈正常";
        case 3: return "雙黃燈";
        case 4: return "綠黃燈";
        case 5: return "閃黃燈";
        default: return "未知信號";
    }
}

int stringToSignalAspect(const std::string& aspect_str)
{
    if (aspect_str == "RED" || aspect_str == "紅燈") return 0;
    if (aspect_str == "YELLOW" || aspect_str == "黃燈") return 1;
    if (aspect_str == "GREEN" || aspect_str == "綠燈") return 2;
    if (aspect_str == "YELLOW_YELLOW" || aspect_str == "雙黃燈") return 3;
    if (aspect_str == "GREEN_YELLOW" || aspect_str == "綠黃燈") return 4;
    if (aspect_str == "FLASHING_YELLOW" || aspect_str == "閃黃燈") return 5;
    return -1; // 未知
}