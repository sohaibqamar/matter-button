#include "reset_switch.hpp"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "util.hpp"

LOG_MODULE_DECLARE(app);

namespace reset_switch {

    namespace {
        const struct device* _device = DEVICE_DT_GET(DT_NODELABEL(gpio0));

        struct gpio_callback _callback;
        reset_fn _reset_handler;

        // Timer work handler
        // Called from the system work queue, calling the reset handler
        void work_handler(struct k_work* work) {
            _reset_handler();
        }

        K_WORK_DEFINE(_work, work_handler);

        // Timer handler for reset switch
        // Dispatches a work item to the system work queue, so the reset handler is not running in the ISR
        void timer_handler(struct k_timer* timer) {
            k_work_submit(&_work);
            k_timer_stop(timer);
        }

        K_TIMER_DEFINE(_timer, timer_handler, nullptr);

        // Interrupt handler for reset switch
        // When the switch is pressed, it starts the timer for 5 seconds
        // When the switch is released, it stops the timer
        void gpio_handler(const struct device* port, struct gpio_callback* callback, gpio_port_pins_t pins) {
            // This call should not be delayed
            int value = read();

            if (value == 0) {
                k_timer_start(&_timer, K_SECONDS(5), K_FOREVER);
            } else if (value == 1) {
                k_timer_stop(&_timer);
            }
        }

    }

    int init() {
        LOG_DBG("Initializing reset switch GPIO: %i", CONFIG_RESET_SWITCH_GPIO);

        int status = gpio_pin_configure(_device, CONFIG_RESET_SWITCH_GPIO, GPIO_INPUT);

        CHECK_SUCCESS("initializing reset switch GPIO", status)

        return status;
    }

    int read() {
        LOG_DBG("Reading reset switch GPIO: %i", CONFIG_RESET_SWITCH_GPIO);

        int status = gpio_pin_get(_device, CONFIG_RESET_SWITCH_GPIO);

        CHECK_STATUS("reading reset switch GPIO", status, < 0)

        return status;
    }

    int enable_interrupt(reset_fn reset_handler) {
        LOG_DBG("Enabling reset switch interrupt");

        if (!reset_handler) {
            LOG_ERR("Invalid parameter for reset switch enable interrupt: interrupt_handler is null");
            return 1;
        }

        if (_reset_handler) {
            LOG_ERR("Reset switch interrupt already configured");
            return 1;
        }

        _reset_handler = reset_handler;

        int status = gpio_pin_interrupt_configure(_device, CONFIG_RESET_SWITCH_GPIO, GPIO_INT_EDGE_BOTH);

        CHECK_SUCCESS("configuring reset switch GPIO interrupt", status)

        gpio_init_callback(&_callback, gpio_handler, BIT(CONFIG_RESET_SWITCH_GPIO));
        status = gpio_add_callback(_device, &_callback);

        CHECK_SUCCESS("adding reset switch GPIO callback", status)

        return status;
    }

}