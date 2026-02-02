/*
 * Taiwan Railway ATP Menu System Implementation
 * Copyright (C) 2024  Taiwan Railway Administration
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "tra_menu.h"
#include "../language/language.h"
#include "../graphics/display.h"
#include "platform_runtime.h"

// 全局菜單實例
TRAMainMenu traMainMenu;
TRASettingsMenu traSettingsMenu;
TRADiagnosticsWindow traDiagnosticsWindow;

// TRAMainMenu Implementation
TRAMainMenu::TRAMainMenu() : window()
{
    systemInfoButton = TextButton(get_text("System Info"), 150, 40, [this]() { showSystemInfo(); });
    settingsButton = TextButton(get_text("Settings"), 150, 40, [this]() { showSettings(); });
    diagnosticsButton = TextButton(get_text("Diagnostics"), 150, 40, [this]() { showDiagnostics(); });
    testModeButton = TextButton(get_text("Test Mode"), 150, 40, [this]() { enterTestMode(); });
    exitButton = TextButton(get_text("Exit"), 150, 40, [this]() { exitMenu(); });
}

void TRAMainMenu::construct()
{
    clearLayout();
    
    // 設置菜單背景顏色
    bounds = {{200, 150, 400, 300}};
    
    // 添加菜單按鈕
    addToLayout(&systemInfoButton, new RelativeAlignment(nullptr, 50, 50));
    addToLayout(&settingsButton, new ConsecutiveAlignment(&systemInfoButton, DOWN, 0));
    addToLayout(&diagnosticsButton, new ConsecutiveAlignment(&settingsButton, DOWN, 0));
    addToLayout(&testModeButton, new ConsecutiveAlignment(&diagnosticsButton, DOWN, 0));
    addToLayout(&exitButton, new ConsecutiveAlignment(&testModeButton, DOWN, 0));
}

void TRAMainMenu::showSystemInfo()
{
    extern TRAMessageArea traMessageArea;
    traMessageArea.clearMessages();
    traMessageArea.addMessage("=== " + get_text("System Info") + " ===", TRA_Active);
    traMessageArea.addMessage(get_text("TRA ATP System") + " v1.0", TRA_Text);
    traMessageArea.addMessage(get_text("Taiwan Railway") + " ATP", TRA_Text);
    traMessageArea.addMessage(get_text("System Ready"), TRA_Green);
    
    extern bool tra_atp_mode;
    extern int tra_max_speed;
    extern std::string tra_language;
    
    traMessageArea.addMessage("ATP: " + (tra_atp_mode ? get_text("ATP Active") : get_text("ATP Inactive")), 
                             tra_atp_mode ? TRA_Green : TRA_Red);
    traMessageArea.addMessage(get_text("Max Speed") + ": " + std::to_string(tra_max_speed) + " " + get_text("km/h"), TRA_Text);
    traMessageArea.addMessage(get_text("Language") + ": " + tra_language, TRA_Text);
    
    hideTRAMainMenu();
}

void TRAMainMenu::showSettings()
{
    hideTRAMainMenu();
    showTRASettingsMenu();
}

void TRAMainMenu::showDiagnostics()
{
    hideTRAMainMenu();
    showTRADiagnosticsWindow();
}

void TRAMainMenu::enterTestMode()
{
    extern TRAMessageArea traMessageArea;
    traMessageArea.addMessage(get_text("Test Mode") + " " + get_text("ATP Active"), TRA_Yellow);
    
    // 執行系統測試
    extern TRAStatusPanel traStatusPanel;
    traStatusPanel.setStatusMessage(get_text("System Test Required"));
    
    hideTRAMainMenu();
}

void TRAMainMenu::exitMenu()
{
    hideTRAMainMenu();
}

// TRASettingsMenu Implementation
TRASettingsMenu::TRASettingsMenu() : window()
{
    languageButton = TextButton(get_text("Language"), 150, 40, [this]() { showLanguageSettings(); });
    soundButton = TextButton(get_text("Sound Settings"), 150, 40, [this]() { showSoundSettings(); });
    displayButton = TextButton(get_text("Display Settings"), 150, 40, [this]() { showDisplaySettings(); });
    systemButton = TextButton(get_text("System"), 150, 40, [this]() { showSystemSettings(); });
    backButton = TextButton(get_text("Back"), 150, 40, [this]() { goBack(); });
}

void TRASettingsMenu::construct()
{
    clearLayout();
    
    bounds = {{200, 100, 400, 400}};
    
    addToLayout(&languageButton, new RelativeAlignment(nullptr, 50, 50));
    addToLayout(&soundButton, new ConsecutiveAlignment(&languageButton, DOWN, 0));
    addToLayout(&displayButton, new ConsecutiveAlignment(&soundButton, DOWN, 0));
    addToLayout(&systemButton, new ConsecutiveAlignment(&displayButton, DOWN, 0));
    addToLayout(&backButton, new ConsecutiveAlignment(&systemButton, DOWN, 0));
}

void TRASettingsMenu::showLanguageSettings()
{
    extern TRAMessageArea traMessageArea;
    extern std::string tra_language;
    
    traMessageArea.clearMessages();
    traMessageArea.addMessage("=== " + get_text("Language") + " ===", TRA_Active);
    traMessageArea.addMessage(get_text("Current") + ": " + tra_language, TRA_Text);
    traMessageArea.addMessage("1. 繁體中文 (zh_TW)", TRA_Text);
    traMessageArea.addMessage("2. English (en)", TRA_Text);
    traMessageArea.addMessage("3. 简体中文 (zh_CN)", TRA_Text);
}

void TRASettingsMenu::showSoundSettings()
{
    extern TRAMessageArea traMessageArea;
    extern bool tra_sound_enabled;
    
    traMessageArea.clearMessages();
    traMessageArea.addMessage("=== " + get_text("Sound Settings") + " ===", TRA_Active);
    traMessageArea.addMessage(get_text("Sound") + ": " + (tra_sound_enabled ? "ON" : "OFF"), 
                             tra_sound_enabled ? TRA_Green : TRA_Red);
    traMessageArea.addMessage("ATP " + get_text("Warning") + ": ON", TRA_Text);
    traMessageArea.addMessage(get_text("Button") + " " + get_text("Sound") + ": ON", TRA_Text);
}

void TRASettingsMenu::showDisplaySettings()
{
    extern TRAMessageArea traMessageArea;
    
    traMessageArea.clearMessages();
    traMessageArea.addMessage("=== " + get_text("Display Settings") + " ===", TRA_Active);
    traMessageArea.addMessage(get_text("Resolution") + ": 800x600", TRA_Text);
    traMessageArea.addMessage(get_text("Brightness") + ": 80%", TRA_Text);
    traMessageArea.addMessage(get_text("Theme") + ": TRA Standard", TRA_Text);
}

void TRASettingsMenu::showSystemSettings()
{
    extern TRAMessageArea traMessageArea;
    extern int tra_max_speed;
    
    traMessageArea.clearMessages();
    traMessageArea.addMessage("=== " + get_text("System") + " ===", TRA_Active);
    traMessageArea.addMessage(get_text("Max Speed") + ": " + std::to_string(tra_max_speed) + " " + get_text("km/h"), TRA_Text);
    traMessageArea.addMessage("ATP " + get_text("Version") + ": 1.0", TRA_Text);
    traMessageArea.addMessage(get_text("Last Update") + ": 2024-01-01", TRA_Text);
}

void TRASettingsMenu::goBack()
{
    extern std::list<window*> active_windows;
    
    // 移除設定菜單
    active_windows.remove(this);
    
    // 顯示主菜單
    showTRAMainMenu();
}

// TRADiagnosticsWindow Implementation
TRADiagnosticsWindow::TRADiagnosticsWindow() : window(), diagnosticMessages(600, 250)
{
    runTestButton = TextButton(get_text("Run Test"), 120, 40, [this]() { runDiagnostics(); });
    clearButton = TextButton(get_text("Clear"), 120, 40, [this]() { clearMessages(); });
    backButton = TextButton(get_text("Back"), 120, 40, [this]() { goBack(); });
}

void TRADiagnosticsWindow::construct()
{
    clearLayout();
    
    bounds = {{100, 50, 600, 500}};
    
    addToLayout(&diagnosticMessages, new RelativeAlignment(nullptr, 20, 20));
    addToLayout(&runTestButton, new RelativeAlignment(nullptr, 20, 300));
    addToLayout(&clearButton, new ConsecutiveAlignment(&runTestButton, RIGHT, 0));
    addToLayout(&backButton, new ConsecutiveAlignment(&clearButton, RIGHT, 0));
}

void TRADiagnosticsWindow::runDiagnostics()
{
    diagnosticMessages.clearMessages();
    diagnosticMessages.addMessage("=== " + get_text("Diagnostics") + " ===", TRA_Active);
    diagnosticMessages.addMessage(get_text("System Test") + "...", TRA_Text);
    
    // 模擬診斷測試
    diagnosticMessages.addMessage("ATP " + get_text("System") + ": OK", TRA_Green);
    diagnosticMessages.addMessage(get_text("Speed") + " " + get_text("Sensor") + ": OK", TRA_Green);
    diagnosticMessages.addMessage(get_text("Brake") + " " + get_text("System") + ": OK", TRA_Green);
    diagnosticMessages.addMessage(get_text("Communication") + ": OK", TRA_Green);
    diagnosticMessages.addMessage(get_text("Signal") + " " + get_text("System") + ": OK", TRA_Green);
    
    // 檢查系統狀態
    extern TRAStatusPanel traStatusPanel;
    extern TRASpeedometer traSpeedometer;
    extern TRASignalIndicator traSignalIndicator;
    
    diagnosticMessages.addMessage("", TRA_Text);
    diagnosticMessages.addMessage(get_text("System Status") + ":", TRA_Active);
    diagnosticMessages.addMessage("ATP: " + get_text("Normal Operation"), TRA_Green);
    diagnosticMessages.addMessage(get_text("All systems operational"), TRA_Green);
    
    traStatusPanel.setStatusMessage(get_text("Diagnostics") + " " + get_text("Complete"));
}

void TRADiagnosticsWindow::clearMessages()
{
    diagnosticMessages.clearMessages();
}

void TRADiagnosticsWindow::goBack()
{
    extern std::list<window*> active_windows;
    active_windows.remove(this);
    showTRAMainMenu();
}

void TRADiagnosticsWindow::addDiagnosticMessage(const std::string& message, Color color)
{
    diagnosticMessages.addMessage(message, color);
}

// 全局菜單管理函數
void showTRAMainMenu()
{
    extern std::list<window*> active_windows;
    
    traMainMenu.construct();
    active_windows.push_front(&traMainMenu);
}

void hideTRAMainMenu()
{
    extern std::list<window*> active_windows;
    active_windows.remove(&traMainMenu);
}

void showTRASettingsMenu()
{
    extern std::list<window*> active_windows;
    
    traSettingsMenu.construct();
    active_windows.push_front(&traSettingsMenu);
}

void showTRADiagnosticsWindow()
{
    extern std::list<window*> active_windows;
    
    traDiagnosticsWindow.construct();
    active_windows.push_front(&traDiagnosticsWindow);
}