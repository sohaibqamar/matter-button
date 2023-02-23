#include "app_task.hpp"


#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include "app_event.hpp"
#include "button.hpp"
#include "reset_switch.hpp"
#include "battery.hpp"
#include "util.hpp"

LOG_MODULE_DECLARE(app);

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

constexpr chip::EndpointId button_endpoint = 1;

K_MSGQ_DEFINE(app_event_queue, sizeof(AppEvent), 10, alignof(AppEvent));

int AppTask::init() {
    LOG_INF("Initializing...");

    // Hardware initialization
    int err = button::init();
    CHECK_SUCCESS("initializing button", err)

    err = reset_switch::init();
    CHECK_SUCCESS("initializing reset switch", err)

    err = battery::init();
    CHECK_SUCCESS("initializing battery management", err)

    // Matter initialization
    CHIP_ERROR chip_err = chip::Platform::MemoryInit();
    CHECK_CHIP_SUCCESS("initializing memory", chip_err)

    chip_err = PlatformMgr().InitChipStack();
    CHECK_CHIP_SUCCESS("initializing Matter stack", chip_err)

    chip_err = ThreadStackMgr().InitThreadStack();
    CHECK_CHIP_SUCCESS("initializing Thread stack", chip_err)

    chip_err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SynchronizedSleepyEndDevice);
    CHECK_CHIP_SUCCESS("setting Thread device type", chip_err)

    // TODO: Proper credentials
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // TODO: Proper parameter storage
    static chip::CommonCaseDeviceServerInitParams init_params;
    init_params.InitializeStaticResourcesBeforeServerInit();

    chip_err = chip::Server::GetInstance().Init(init_params);
    CHECK_CHIP_SUCCESS("initializing server", chip_err)

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag::kBLE);

    chip_err = PlatformMgr().AddEventHandler(on_chip_event, 0);
    CHECK_CHIP_SUCCESS("adding CHIP event handler", chip_err)

    chip_err = PlatformMgr().StartEventLoopTask();
    CHECK_CHIP_SUCCESS("starting event loop task", chip_err)

    _initialized = true;
    return 0;
}

int AppTask::start() {
    if (!_initialized) {
        LOG_ERR("Application is not initialized");
        return 1;
    }

    int status = button::enable_interrupt(on_button_interrupt);
    CHECK_SUCCESS("enabling button interrupt", status)

    status = button::read();

    if (status != 0 && status != 1) {
        LOG_ERR("Invalid button status on initial read: %i", status);
    } else {
        set_button(status == 0);
    }

    status = reset_switch::enable_interrupt(on_reset);
    CHECK_SUCCESS("enabling reset switch interrupt", status)

    auto percentage = battery::get_percentage();
    CHECK_STATUS("getting battery percentage", percentage, < 0);
    LOG_INF("Current battery percentage: %d%%", percentage / 2);

    AppEvent event {};

    while (true) {
        status = k_msgq_get(&app_event_queue, &event, K_FOREVER);

        if (status != 0) {
            LOG_WRN("App event queue returned invalid status: %i", status);
            continue;
        }

        if (event.callback) {
            event.callback(event);
        } else {
            LOG_WRN("App event does not have a callback");
        }
    }
}

void AppTask::on_chip_event(const chip::DeviceLayer::ChipDeviceEvent* event, intptr_t /* arg */) {
    LOG_INF("CHIP event: %u", event->Type);
}

void AppTask::on_button_interrupt(int status) {
    LOG_INF("Button status: %d", status);
    AppEvent event {
        .type = AppEventType::ButtonChanged,
        .callback = on_button_change,
        .button_changed = {
            .status = status
        }
    };

    auto _status = k_msgq_put(&app_event_queue, &event, K_NO_WAIT);

    if (_status != 0) {
        LOG_ERR("Failed to post ButtonChanged event: %i", _status);
    }
}

void AppTask::on_reset() {
    LOG_INF("Performing factory reset");
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::on_button_change(AppEvent event) {
    if (event.type != AppEventType::ButtonChanged) return;

    auto status = event.button_changed.status;

    if (status != 0 && status != 1) {
        LOG_ERR("Invalid status in ButtonChanged event: %i", status);
        return;
    }

    set_button(status == 0);
}

void AppTask::set_button(bool pressed) {
    SystemLayer().ScheduleLambda([pressed] {
        auto status = chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(button_endpoint, pressed);

        if (pressed) {
            chip::app::Clusters::SwitchServer::Instance().OnInitialPress(button_endpoint, 1);
        } else {
            chip::app::Clusters::SwitchServer::Instance().OnShortRelease(button_endpoint, 1);
        }

        if (status != EMBER_ZCL_STATUS_SUCCESS) {
            LOG_ERR("Failed to update button cluster: %i", status);
        }
    });
}

bool AppTask::initialized() const { return _initialized; }
