/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <platform/CHIPDeviceLayer.h>
#include "app_event.hpp"

class AppTask {
public:
	static AppTask& shared() {
		static AppTask app_task;
		return app_task;
	};

    bool initialized() const;
    int init();

    int start();

private:
    static void on_chip_event(const chip::DeviceLayer::ChipDeviceEvent* event, intptr_t arg);
    static void on_button_interrupt(int status);
    static void on_reset();

    static void on_button_change(AppEvent event);

    static void set_button(bool pressed);

    bool _initialized = false;
};
