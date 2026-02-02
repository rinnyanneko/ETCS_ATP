/*
 * SimRail API Interface for Taiwan Railway ATP
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
#ifndef _SIMRAIL_API_H
#define _SIMRAIL_API_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

// SimRail API連接狀態
enum class SimRailConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

// SimRail列車資料
struct SimRailTrainData {
    std::string train_number;
    std::string route;
    float current_speed;        // km/h
    float max_speed;           // km/h
    float distance_traveled;   // km
    float total_distance;      // km
    bool doors_open;
    bool pantograph_up;
    bool main_breaker_on;
    bool traction_active;
    std::string current_station;
    std::string next_station;
    float distance_to_next_station; // km
};

// SimRail信號資料
struct SimRailSignalData {
    std::string signal_id;
    int aspect;                // 0=Red, 1=Yellow, 2=Green, etc.
    float distance;            // m
    int speed_limit;           // km/h after signal
    bool approach_control;
    std::string signal_name;
};

// SimRail軌道資料
struct SimRailTrackData {
    float distance;            // m
    int speed_limit;           // km/h
    bool temporary_limit;
    std::string limit_reason;
    std::string track_section;
    bool level_crossing;
    bool station_area;
    std::string station_name;
};

// SimRail系統狀態
struct SimRailSystemStatus {
    bool game_running;
    bool train_selected;
    bool shp_active;
    bool sifa_active;
    float simulation_time;     // seconds since start
    std::string weather;
    float visibility;          // km
    bool emergency_brake;
    bool service_brake;
    float brake_pressure;      // bar
};

class SimRailAPI {
private:
    SimRailConnectionStatus connection_status;
    std::string api_endpoint;
    int api_port;
    bool auto_reconnect;
    int reconnect_interval; // ms
    int64_t last_connection_attempt;
    
    // 資料快取
    SimRailTrainData cached_train_data;
    std::vector<SimRailSignalData> cached_signals;
    std::vector<SimRailTrackData> cached_track_data;
    SimRailSystemStatus cached_system_status;
    
    // 回調函數
    std::function<void(const SimRailTrainData&)> train_data_callback;
    std::function<void(const std::vector<SimRailSignalData>&)> signal_data_callback;
    std::function<void(const std::vector<SimRailTrackData>&)> track_data_callback;
    std::function<void(const SimRailSystemStatus&)> system_status_callback;
    std::function<void(SimRailConnectionStatus)> connection_callback;
    
    // 內部方法
    bool establishConnection();
    void closeConnection();
    bool sendRequest(const std::string& endpoint, std::string& response);
    bool parseTrainData(const std::string& json_data, SimRailTrainData& train_data);
    bool parseSignalData(const std::string& json_data, std::vector<SimRailSignalData>& signals);
    bool parseTrackData(const std::string& json_data, std::vector<SimRailTrackData>& track_data);
    bool parseSystemStatus(const std::string& json_data, SimRailSystemStatus& status);
    
public:
    SimRailAPI();
    ~SimRailAPI();
    
    // 連接管理
    bool connect(const std::string& endpoint = "localhost", int port = 8080);
    void disconnect();
    bool isConnected() const { return connection_status == SimRailConnectionStatus::CONNECTED; }
    SimRailConnectionStatus getConnectionStatus() const { return connection_status; }
    
    // 資料獲取
    bool updateTrainData();
    bool updateSignalData();
    bool updateTrackData();
    bool updateSystemStatus();
    bool updateAllData();
    
    // 資料存取
    const SimRailTrainData& getTrainData() const { return cached_train_data; }
    const std::vector<SimRailSignalData>& getSignalData() const { return cached_signals; }
    const std::vector<SimRailTrackData>& getTrackData() const { return cached_track_data; }
    const SimRailSystemStatus& getSystemStatus() const { return cached_system_status; }
    
    // 控制命令
    bool sendBrakeCommand(bool emergency = false);
    bool sendTractionCommand(bool enable);
    bool sendHornCommand();
    bool sendPantographCommand(bool raise);
    bool sendMainBreakerCommand(bool on);
    bool sendDoorCommand(bool open);
    
    // 回調設定
    void setTrainDataCallback(std::function<void(const SimRailTrainData&)> callback);
    void setSignalDataCallback(std::function<void(const std::vector<SimRailSignalData>&)> callback);
    void setTrackDataCallback(std::function<void(const std::vector<SimRailTrackData>&)> callback);
    void setSystemStatusCallback(std::function<void(const SimRailSystemStatus&)> callback);
    void setConnectionCallback(std::function<void(SimRailConnectionStatus)> callback);
    
    // 配置
    void setAutoReconnect(bool enable) { auto_reconnect = enable; }
    void setReconnectInterval(int interval_ms) { reconnect_interval = interval_ms; }
    
    // 診斷
    std::vector<std::string> getDiagnosticInfo();
    bool testConnection();
    void clearCache();
};

// 全域SimRail API實例
extern std::unique_ptr<SimRailAPI> simrail_api;

// 全域函數
bool initializeSimRailAPI();
void shutdownSimRailAPI();
bool connectToSimRail(const std::string& endpoint = "localhost", int port = 8080);
void disconnectFromSimRail();
bool isSimRailConnected();

// 輔助函數
std::string connectionStatusToString(SimRailConnectionStatus status);
std::string signalAspectToString(int aspect);
int stringToSignalAspect(const std::string& aspect_str);

#endif