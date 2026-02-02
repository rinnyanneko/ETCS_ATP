/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "monitor.h"
#include "graphics/drawing.h"
#include "graphics/tra_components.h"
#include "tra_atp_integration.h"
#include "tcp/server.h"
#include "control/control.h"
#include "platform_runtime.h"

void startWindows();
void initialize_stm_windows();

float platform_size_w = 800.0f, platform_size_h = 600.0f;

void on_platform_ready()
{
    platform->on_quit_request().then([](){
        platform->quit();
    }).detach();

    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    
    // 初始化台鐵ATP組件
    extern bool tra_atp_mode;
    if (tra_atp_mode) {
        initTRAComponents();
        setTRAMode(true);
        
        // 初始化台鐵ATP整合系統
        if (initializeTRAATPIntegration()) {
            platform->debug_print("TRA ATP Integration initialized");
            
            // 啟動整合系統
            if (tra_atp_integration) {
                tra_atp_integration->start();
            }
        } else {
            platform->debug_print("Failed to initialize TRA ATP Integration");
        }
    }
    
    startSocket();
    startWindows();
    initialize_stm_windows();
    drawing_start();
}
