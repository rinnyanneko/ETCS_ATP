//SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Taiwan Railway ATP Components Implementation
 *
 * Copyright (C) 2026 rinnyanneko
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "tra_components.h"
#include "drawing.h"
#include "platform_runtime.h"
#include <cmath>
#include <algorithm>

// 全局台鐵ATP組件實例
TRASignalIndicator traSignalIndicator(100, 100);
TRASpeedometer traSpeedometer(200, 200);
TRAStatusPanel traStatusPanel(300, 150);
TRADistanceBar traDistanceBar(700, 30);
TRAControlButtons traControlButtons(700, 60);
TRAMessageArea traMessageArea(700, 200);

// 台鐵ATP配置變數
bool tra_atp_mode = true;
int tra_max_speed = 130;
bool tra_sound_enabled = true;
std::string tra_language = "zh_TW";

// TRASignalIndicator Implementation
TRASignalIndicator::TRASignalIndicator(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Background;
    fgColor = TRA_Text;
}

void TRASignalIndicator::setAspect(SignalAspect aspect)
{
    current_aspect = aspect;
    flashing = (aspect == FLASHING_YELLOW);
    flash_timer = platform->get_timer();
}

void TRASignalIndicator::update()
{
    if (flashing) {
        int64_t current_time = platform->get_timer();
        if (current_time - flash_timer > 500) { // 500ms flash interval
            flash_timer = current_time;
        }
    }
}

void TRASignalIndicator::paint()
{
    Component::paint();
    
    // Draw signal background
    drawRectangle(0, 0, sx, sy, TRA_Panel);
    drawRectangle(0, 0, sx, sy, TRA_Border, LEFT | UP);
    
    // Draw signal light
    float center_x = sx / 2;
    float center_y = sy / 2;
    float radius = std::min(sx, sy) / 3;
    
    Color signal_color = TRA_Inactive;
    bool show_light = true;
    
    switch (current_aspect) {
        case RED_STOP:
            signal_color = TRA_Red;
            break;
        case YELLOW_CAUTION:
            signal_color = TRA_Yellow;
            break;
        case GREEN_CLEAR:
            signal_color = TRA_Green;
            break;
        case FLASHING_YELLOW:
            signal_color = TRA_Yellow;
            show_light = ((platform->get_timer() - flash_timer) % 1000) < 500;
            break;
        case NO_SIGNAL:
            signal_color = TRA_Inactive;
            break;
    }
    
    if (show_light) {
        drawCircle(radius, center_x, center_y);
        // Add inner glow effect
        drawCircle(radius * 0.8f, center_x, center_y);
    }
}

// TRASpeedometer Implementation
TRASpeedometer::TRASpeedometer(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Background;
    fgColor = TRA_Text;
}

void TRASpeedometer::setSpeed(int speed)
{
    current_speed = std::max(0, std::min(speed, max_speed));
    overspeed_warning = (current_speed > warning_speed && warning_speed > 0);
}

void TRASpeedometer::setTargetSpeed(int target)
{
    target_speed = std::max(0, std::min(target, max_speed));
}

void TRASpeedometer::setWarningSpeed(int warning)
{
    warning_speed = std::max(0, std::min(warning, max_speed));
}

void TRASpeedometer::setMaxSpeed(int max)
{
    max_speed = std::max(1, max);
}

void TRASpeedometer::paint()
{
    Component::paint();
    drawSpeedDial();
    drawSpeedNeedle();
    drawSpeedText();
}

void TRASpeedometer::drawSpeedDial()
{
    float center_x = sx / 2;
    float center_y = sy / 2;
    float outer_radius = std::min(sx, sy) / 2 - 10;
    float inner_radius = outer_radius - 20;
    
    // Draw outer circle
    drawCircle(outer_radius, center_x, center_y);
    
    // Draw speed markings
    for (int speed = 0; speed <= max_speed; speed += 10) {
        float angle = -135 + (270.0f * speed / max_speed); // -135° to +135°
        float rad = angle * M_PI / 180.0f;
        
        float x1 = center_x + outer_radius * cos(rad);
        float y1 = center_y + outer_radius * sin(rad);
        float x2 = center_x + inner_radius * cos(rad);
        float y2 = center_y + inner_radius * sin(rad);
        
        Color mark_color = TRA_Text;
        if (speed > warning_speed && warning_speed > 0) {
            mark_color = TRA_Red;
        }
        
        drawLine(x1, y1, x2, y2, mark_color);
        
        // Draw speed numbers
        if (speed % 20 == 0) {
            // TODO: Add text rendering for speed numbers
        }
    }
    
    // Draw target speed marker
    if (target_speed > 0) {
        float angle = -135 + (270.0f * target_speed / max_speed);
        float rad = angle * M_PI / 180.0f;
        
        float x = center_x + (outer_radius + 5) * cos(rad);
        float y = center_y + (outer_radius + 5) * sin(rad);
        
        // Draw triangle marker
        float triangle_size = 8;
        float vx[] = {x, x - triangle_size, x + triangle_size};
        float vy[] = {y - triangle_size, y + triangle_size, y + triangle_size};
        drawConvexPolygon(vx, vy, 3);
    }
}

void TRASpeedometer::drawSpeedNeedle()
{
    float center_x = sx / 2;
    float center_y = sy / 2;
    float needle_length = std::min(sx, sy) / 2 - 30;
    
    float angle = -135 + (270.0f * current_speed / max_speed);
    float rad = angle * M_PI / 180.0f;
    
    float end_x = center_x + needle_length * cos(rad);
    float end_y = center_y + needle_length * sin(rad);
    
    Color needle_color = overspeed_warning ? TRA_Red : TRA_Text;
    drawLine(center_x, center_y, end_x, end_y, needle_color);
    
    // Draw needle center
    drawCircle(5, center_x, center_y);
}

void TRASpeedometer::drawSpeedText()
{
    // Draw current speed in center
    // TODO: Add text rendering for current speed display
    
    // Draw speed limit if different from target
    if (warning_speed > 0 && warning_speed != target_speed) {
        // TODO: Add warning speed display
    }
}

// TRAStatusPanel Implementation
TRAStatusPanel::TRAStatusPanel(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Panel;
    fgColor = TRA_Text;
}

void TRAStatusPanel::setATPStatus(bool active)
{
    atp_active = active;
}

void TRAStatusPanel::setBrakeStatus(bool applied)
{
    brake_applied = applied;
}

void TRAStatusPanel::setDoorStatus(bool closed)
{
    door_closed = closed;
}

void TRAStatusPanel::setTractionStatus(bool cut)
{
    traction_cut = cut;
}

void TRAStatusPanel::setEmergencyBrake(bool active)
{
    emergency_brake = active;
}

void TRAStatusPanel::setOverspeedStatus(bool overspeed_status)
{
    overspeed = overspeed_status;
}

void TRAStatusPanel::setStatusMessage(const std::string& message)
{
    status_message = message;
}

void TRAStatusPanel::paint()
{
    Component::paint();
    
    // Draw panel background
    drawRectangle(0, 0, sx, sy, bgColor);
    drawRectangle(0, 0, sx, sy, TRA_Border, LEFT | UP);
    
    drawStatusLights();
    drawStatusText();
}

void TRAStatusPanel::drawStatusLights()
{
    float light_size = 20;
    float spacing = 25;
    float start_x = 10;
    float start_y = 10;
    
    // ATP Status Light
    Color atp_color = atp_active ? TRA_Active : TRA_Inactive;
    drawRectangle(start_x, start_y, light_size, light_size, atp_color);
    
    // Brake Status Light
    Color brake_color = brake_applied ? TRA_Red : TRA_Inactive;
    drawRectangle(start_x + spacing, start_y, light_size, light_size, brake_color);
    
    // Door Status Light
    Color door_color = door_closed ? TRA_Green : TRA_Red;
    drawRectangle(start_x + spacing * 2, start_y, light_size, light_size, door_color);
    
    // Traction Status Light
    Color traction_color = traction_cut ? TRA_Red : TRA_Green;
    drawRectangle(start_x + spacing * 3, start_y, light_size, light_size, traction_color);
    
    // Emergency Brake Light
    if (emergency_brake) {
        Color emergency_color = TRA_Red;
        drawRectangle(start_x + spacing * 4, start_y, light_size, light_size, emergency_color);
    }
    
    // Overspeed Warning Light
    if (overspeed) {
        Color overspeed_color = TRA_Orange;
        drawRectangle(start_x + spacing * 5, start_y, light_size, light_size, overspeed_color);
    }
}

void TRAStatusPanel::drawStatusText()
{
    // TODO: Add text rendering for status labels and messages
    // Labels: "ATP", "煞車", "車門", "牽引", "緊急", "超速"
    // Status message display
}

// TRADistanceBar Implementation
TRADistanceBar::TRADistanceBar(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Background;
    fgColor = TRA_Text;
}

void TRADistanceBar::setDistance(float distance)
{
    distance_to_target = std::max(0.0f, distance);
}

void TRADistanceBar::setMaxDistance(float max_dist)
{
    max_distance = std::max(1.0f, max_dist);
}

void TRADistanceBar::setTargetActive(bool active)
{
    target_active = active;
}

void TRADistanceBar::paint()
{
    Component::paint();
    
    if (!target_active) return;
    
    // Draw distance bar background
    drawRectangle(0, 0, sx, sy, TRA_Panel);
    drawRectangle(0, 0, sx, sy, TRA_Border, LEFT | UP);
    
    // Calculate bar fill percentage
    float fill_percentage = 1.0f - (distance_to_target / max_distance);
    fill_percentage = std::max(0.0f, std::min(1.0f, fill_percentage));
    
    // Draw filled portion
    float fill_width = sx * fill_percentage;
    Color fill_color = (distance_to_target < 100) ? TRA_Red : 
                      (distance_to_target < 300) ? TRA_Yellow : TRA_Green;
    
    drawRectangle(0, 0, fill_width, sy, fill_color);
    
    // TODO: Add distance text display
}

// TRAControlButtons Implementation
TRAControlButtons::TRAControlButtons(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Background;
    fgColor = TRA_Text;
}

void TRAControlButtons::addButton(const std::string& label, std::function<void()> action, Color color)
{
    buttons.push_back({label, true, false, action, color});
}

void TRAControlButtons::setButtonEnabled(int index, bool enabled)
{
    if (index >= 0 && index < buttons.size()) {
        buttons[index].enabled = enabled;
    }
}

void TRAControlButtons::paint()
{
    Component::paint();
    
    float button_width = sx / std::max(1, (int)buttons.size());
    float button_height = sy;
    
    for (int i = 0; i < buttons.size(); i++) {
        float x = i * button_width;
        Color button_color = buttons[i].enabled ? buttons[i].color : TRA_Inactive;
        
        if (buttons[i].pressed) {
            button_color = TRA_Active;
        }
        
        drawRectangle(x, 0, button_width, button_height, button_color);
        drawRectangle(x, 0, button_width, button_height, TRA_Border, LEFT | UP);
        
        // TODO: Add button label text rendering
    }
}

bool TRAControlButtons::handleTouch(float x, float y)
{
    if (x < 0 || x > sx || y < 0 || y > sy) return false;
    
    float button_width = sx / std::max(1, (int)buttons.size());
    int button_index = (int)(x / button_width);
    
    if (button_index >= 0 && button_index < buttons.size() && buttons[button_index].enabled) {
        buttons[button_index].pressed = true;
        if (buttons[button_index].action) {
            buttons[button_index].action();
        }
        return true;
    }
    
    return false;
}

// TRAMessageArea Implementation
TRAMessageArea::TRAMessageArea(float sx, float sy) : Component(sx, sy)
{
    bgColor = TRA_Background;
    fgColor = TRA_Text;
}

void TRAMessageArea::addMessage(const std::string& message, Color color)
{
    messages.push_back(message);
    message_color = color;
    
    // Keep only the most recent messages
    while (messages.size() > max_messages) {
        messages.erase(messages.begin());
    }
}

void TRAMessageArea::clearMessages()
{
    messages.clear();
}

void TRAMessageArea::paint()
{
    Component::paint();
    
    // Draw message area background
    drawRectangle(0, 0, sx, sy, bgColor);
    drawRectangle(0, 0, sx, sy, TRA_Border, LEFT | UP);
    
    // TODO: Add text rendering for messages
    // Display messages from bottom to top, most recent at bottom
}

// Global functions
void initTRAComponents()
{
    // Initialize all TRA components with default values
    traSignalIndicator.setAspect(TRASignalIndicator::NO_SIGNAL);
    traSpeedometer.setSpeed(0);
    traSpeedometer.setTargetSpeed(0);
    traSpeedometer.setMaxSpeed(tra_max_speed);
    traStatusPanel.setATPStatus(tra_atp_mode);
    traDistanceBar.setTargetActive(false);
    
    // Add control buttons
    traControlButtons.addButton("確認", []() { /* ACK action */ }, TRA_Green);
    traControlButtons.addButton("重置", []() { /* Reset action */ }, TRA_Yellow);
    traControlButtons.addButton("超馳", []() { /* Override action */ }, TRA_Orange);
    traControlButtons.addButton("緊急", []() { /* Emergency action */ }, TRA_Red);
    
    traMessageArea.addMessage("台鐵ATP系統啟動", TRA_Green);
}

void updateTRADisplay()
{
    // Update flashing signals
    traSignalIndicator.update();
    
    // Update component visibility based on mode
    traSignalIndicator.visible = tra_atp_mode;
    traSpeedometer.visible = true;
    traStatusPanel.visible = tra_atp_mode;
    traDistanceBar.visible = tra_atp_mode;
    traControlButtons.visible = tra_atp_mode;
    traMessageArea.visible = tra_atp_mode;
}

void handleTRAInput(float x, float y)
{
    // Handle touch input for TRA components
    if (traControlButtons.visible) {
        traControlButtons.handleTouch(x - traControlButtons.x, y - traControlButtons.y);
    }
}

void setTRAMode(bool enabled)
{
    tra_atp_mode = enabled;
    if (enabled) {
        traMessageArea.addMessage("ATP模式啟動", TRA_Green);
    } else {
        traMessageArea.addMessage("ATP模式關閉", TRA_Yellow);
    }
}