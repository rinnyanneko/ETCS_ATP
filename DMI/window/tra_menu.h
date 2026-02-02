/*
 * Taiwan Railway ATP Menu System
 * Copyright (C) 2024  Taiwan Railway Administration
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _TRA_MENU_H
#define _TRA_MENU_H
#include "window.h"
#include "../graphics/text_button.h"
#include "../graphics/tra_components.h"

class TRAMainMenu : public window
{
private:
    TextButton systemInfoButton;
    TextButton settingsButton;
    TextButton diagnosticsButton;
    TextButton testModeButton;
    TextButton exitButton;
    
public:
    TRAMainMenu();
    void construct() override;
    void showSystemInfo();
    void showSettings();
    void showDiagnostics();
    void enterTestMode();
    void exitMenu();
};

class TRASettingsMenu : public window
{
private:
    TextButton languageButton;
    TextButton soundButton;
    TextButton displayButton;
    TextButton systemButton;
    TextButton backButton;
    
public:
    TRASettingsMenu();
    void construct() override;
    void showLanguageSettings();
    void showSoundSettings();
    void showDisplaySettings();
    void showSystemSettings();
    void goBack();
};

class TRADiagnosticsWindow : public window
{
private:
    TRAMessageArea diagnosticMessages;
    TextButton runTestButton;
    TextButton clearButton;
    TextButton backButton;
    
public:
    TRADiagnosticsWindow();
    void construct() override;
    void runDiagnostics();
    void clearMessages();
    void goBack();
    void addDiagnosticMessage(const std::string& message, Color color = TRA_Text);
};

// 全局菜單實例
extern TRAMainMenu traMainMenu;
extern TRASettingsMenu traSettingsMenu;
extern TRADiagnosticsWindow traDiagnosticsWindow;

// 菜單管理函數
void showTRAMainMenu();
void hideTRAMainMenu();
void showTRASettingsMenu();
void showTRADiagnosticsWindow();

#endif