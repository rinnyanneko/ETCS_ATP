/*
 * Taiwan Railway ATP Components
 * Copyright (C) 2024  Taiwan Railway Administration
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _TRA_COMPONENTS_H
#define _TRA_COMPONENTS_H
#include "component.h"
#include "color.h"
#include <string>

class TRASignalIndicator : public Component
{
public:
    enum SignalAspect {
        RED_STOP,
        YELLOW_CAUTION, 
        GREEN_CLEAR,
        FLASHING_YELLOW,
        NO_SIGNAL
    };
    
private:
    SignalAspect current_aspect = NO_SIGNAL;
    bool flashing = false;
    int64_t flash_timer = 0;
    
public:
    TRASignalIndicator(float sx, float sy);
    void setAspect(SignalAspect aspect);
    SignalAspect getAspect() const { return current_aspect; }
    void paint() override;
    void update();
};

class TRASpeedometer : public Component  
{
private:
    int max_speed = 130;
    int current_speed = 0;
    int target_speed = 0;
    int warning_speed = 0;
    bool overspeed_warning = false;
    
public:
    TRASpeedometer(float sx, float sy);
    void setSpeed(int speed);
    void setTargetSpeed(int target);
    void setWarningSpeed(int warning);
    void setMaxSpeed(int max);
    void paint() override;
    void drawSpeedDial();
    void drawSpeedNeedle();
    void drawSpeedText();
};

class TRAStatusPanel : public Component
{
private:
    bool atp_active = false;
    bool brake_applied = false;
    bool door_closed = true;
    bool traction_cut = false;
    bool emergency_brake = false;
    bool overspeed = false;
    std::string status_message = "";
    
public:
    TRAStatusPanel(float sx, float sy);
    void setATPStatus(bool active);
    void setBrakeStatus(bool applied);
    void setDoorStatus(bool closed);
    void setTractionStatus(bool cut);
    void setEmergencyBrake(bool active);
    void setOverspeedStatus(bool overspeed);
    void setStatusMessage(const std::string& message);
    void paint() override;
    void drawStatusLights();
    void drawStatusText();
};

class TRADistanceBar : public Component
{
private:
    float distance_to_target = 0;
    float max_distance = 1000;
    bool target_active = false;
    
public:
    TRADistanceBar(float sx, float sy);
    void setDistance(float distance);
    void setMaxDistance(float max_dist);
    void setTargetActive(bool active);
    void paint() override;
};

class TRAControlButtons : public Component
{
private:
    struct ButtonInfo {
        std::string label;
        bool enabled;
        bool pressed;
        std::function<void()> action;
        Color color;
    };
    
    std::vector<ButtonInfo> buttons;
    
public:
    TRAControlButtons(float sx, float sy);
    void addButton(const std::string& label, std::function<void()> action, Color color = TRA_Panel);
    void setButtonEnabled(int index, bool enabled);
    void paint() override;
    bool handleTouch(float x, float y);
};

class TRAMessageArea : public Component
{
private:
    std::vector<std::string> messages;
    int max_messages = 5;
    Color message_color = TRA_Text;
    
public:
    TRAMessageArea(float sx, float sy);
    void addMessage(const std::string& message, Color color = TRA_Text);
    void clearMessages();
    void paint() override;
};

// 全局台鐵ATP組件實例
extern TRASignalIndicator traSignalIndicator;
extern TRASpeedometer traSpeedometer;
extern TRAStatusPanel traStatusPanel;
extern TRADistanceBar traDistanceBar;
extern TRAControlButtons traControlButtons;
extern TRAMessageArea traMessageArea;

// 台鐵ATP配置變數
extern bool tra_atp_mode;
extern int tra_max_speed;
extern bool tra_sound_enabled;
extern std::string tra_language;

// 台鐵ATP功能函數
void initTRAComponents();
void updateTRADisplay();
void handleTRAInput(float x, float y);
void setTRAMode(bool enabled);

#endif