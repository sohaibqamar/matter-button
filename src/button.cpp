#include "button.hpp"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "util.hpp"


LOG_MODULE_DECLARE(app);

namespace button {

    namespace {
        const struct device* _device = DEVICE_DT_GET(DT_NODELABEL(gpio0));

        struct gpio_callback _callback;
        button_interrupt_fn _interrupt_handler;

        // State machine
        enum button_state_t {
            BUTTON_RELEASED,
            BUTTON_WAIT_DOUBLE_PRESS_OR_LONG_PRESS,
            BUTTON_WAIT_LONG_PRESS
        };

        enum button_event_t{
            BUTTON_SINGLE_PRESSED,
            BUTTON_DOUBLE_PRESSED,
            BUTTON_LONG_PRESSED
        };

        button_state_t _button_state = BUTTON_RELEASED;

         // Timers
        K_TIMER_DEFINE(_double_press_timer, nullptr, nullptr);
        K_TIMER_DEFINE(_long_press_timer, nullptr, nullptr);

        // Timer callback functions
        void double_press_timer_callback(k_timer* timer_id) {
            LOG_INF("Double press timer expired");
            _button_state = BUTTON_RELEASED;
            _interrupt_handler(BUTTON_SINGLE_PRESSED);
        }

        void long_press_timer_callback(k_timer* timer_id) {
            LOG_INF("Long press timer expired");
            _button_state = BUTTON_RELEASED;
            _interrupt_handler(BUTTON_LONG_PRESSED);
        }

        // Timestamp of the last button press
        uint32_t _last_button_press_time = 0;

        // Number of button presses
        int _num_button_presses = 0;

        /*void gpio_handler(const struct device* port, struct gpio_callback* callback, gpio_port_pins_t pins) {
            LOG_INF("*****************************\n");
            LOG_INF("*****************************\n");
            LOG_INF("\n\n");
            
            // This call should not be delayed
            int value = read();
            _interrupt_handler(value);
        }

        void gpio_handler(const struct device* port, struct gpio_callback* callback, gpio_port_pins_t pins) {
            int value = read();
            uint32_t current_time = k_uptime_get_32();

            if (value == 0) {
                // Button is pressed
                if (_button_state == BUTTON_RELEASED) {
                    // Button was just pressed
                    _button_state = BUTTON_PRESSED;
                    _last_button_press_time = current_time;
                    //LOG_INF("Button pressed");
                } else if (_button_state == BUTTON_PRESSED) {
                    // Button was already pressed
                    if (current_time - _last_button_press_time >= 1000) {
                        // Button has been held down for more than 1 second, trigger long press
                        _button_state = BUTTON_LONG_PRESSED;
                        _interrupt_handler(BUTTON_LONG_PRESSED);
                        LOG_INF("Button long press");
                    }
                } else if (_button_state == BUTTON_LONG_PRESSED) {
                    // Button was already long pressed
                    // Do nothing
                } else if (_button_state == BUTTON_DOUBLE_PRESSED) {
                    // Button was already double pressed
                    // Do nothing
                }
            } else {
                // Button is released
                if (_button_state == BUTTON_PRESSED) {
                    // Button was just released
                    if (current_time - _last_button_press_time >= 250) {
                        // Button was released after less than 250ms, trigger double press
                        _num_button_presses += 1;
                        if (_num_button_presses == 2) {
                            _button_state = BUTTON_DOUBLE_PRESSED;
                            _interrupt_handler(BUTTON_DOUBLE_PRESSED);
                            _num_button_presses = 0;
                            LOG_INF("Button double press");
                        } else {
                            // Wait for another button press
                            _last_button_press_time = current_time;
                        }
                    } else {
                        // Button was released after more than 250ms, trigger single press
                        _button_state = BUTTON_RELEASED;
                        _interrupt_handler(BUTTON_RELEASED);
                        LOG_INF("Button single press");
                    }
                } else if (_button_state == BUTTON_LONG_PRESSED) {
                    // Button was just released after a long press
                    _button_state = BUTTON_RELEASED;
                    _interrupt_handler(BUTTON_RELEASED);
                    //LOG_INF("Button long press released");
                } else if (_button_state == BUTTON_DOUBLE_PRESSED) {
                    // Button was just released after a double press
                    _button_state = BUTTON_RELEASED;
                    _interrupt_handler(BUTTON_RELEASED);
                    //LOG_INF("Button double press released");
                }
            }
        }*/
/*
        // GPIO interrupt handler
        void gpio_handler(const struct device* port, struct gpio_callback* callback, gpio_port_pins_t pins) {
            if (pins & BIT(CONFIG_BUTTON_GPIO)) {
                int value = read();
                switch (_button_state) {
                    case BUTTON_RELEASED:
                        if (value == 0) {
                            _button_state = BUTTON_WAIT_PRESS;
                            k_timer_start(&_single_press_timer, K_MSEC(CONFIG_BUTTON_SINGLE_PRESS_TIME), K_NO_WAIT);
                            LOG_INF("Button pressed, waiting for release or double press");
                        }
                        break;
                    case BUTTON_WAIT_PRESS:
                        if (value == 1) {
                            k_timer_stop(&_single_press_timer);
                            _button_state = BUTTON_RELEASED;
                            _interrupt_handler(BUTTON_SINGLE_PRESSED);
                            LOG_INF("Button single pressed");
                        } else if (k_timer_status_get(&_single_press_timer) > 0) {
                            _button_state = BUTTON_WAIT_DOUBLE_PRESS;
                            k_timer_start(&_double_press_timer, K_MSEC(CONFIG_BUTTON_DOUBLE_PRESS_TIME), K_NO_WAIT);
                            LOG_INF("Button pressed again, waiting for release or triple press");
                        }
                        break;
                    case BUTTON_WAIT_DOUBLE_PRESS:
                        if (value == 1) {
                            k_timer_stop(&_double_press_timer);
                            _button_state = BUTTON_RELEASED;
                            _interrupt_handler(BUTTON_DOUBLE_PRESSED);
                            LOG_INF("Button double pressed");
                        } else if (k_timer_status_get(&_double_press_timer) > 0) {
                            _button_state = BUTTON_WAIT_LONG_PRESS;
                            k_timer_start(&_long_press_timer, K_MSEC(CONFIG_BUTTON_LONG_PRESS_TIME), K_NO_WAIT);
                            LOG_INF("Button pressed again, waiting for long press");
                        }
                        break;
                    case BUTTON_WAIT_LONG_PRESS:
                        if (value == 1) {
                            k_timer_stop(&_long_press_timer);
                            _button_state = BUTTON_RELEASED;
                            _interrupt_handler(BUTTON_LONG_PRESSED);
                            LOG_INF("Button long pressed");
                        }
                        break;
                }
            }
        }
*/

         // GPIO interrupt handler
        void gpio_handler(const struct device* port, struct gpio_callback* callback, gpio_port_pins_t pins) {
            int value = read();
            if (value == 0) {
                if (_button_state == BUTTON_RELEASED) {
                    _button_state = BUTTON_WAIT_DOUBLE_PRESS_OR_LONG_PRESS;
                    //k_timer_start(&_double_press_timer, K_MSEC(CONFIG_BUTTON_DOUBLE_PRESS_TIME), K_NO_WAIT);
                    k_timer_start(&_double_press_timer, K_MSEC(250), K_NO_WAIT);
                    LOG_INF("*********Button pressed, waiting for double press or long press");
                } else if (_button_state == BUTTON_WAIT_DOUBLE_PRESS_OR_LONG_PRESS) {
                    _button_state = BUTTON_WAIT_LONG_PRESS;
                    k_timer_stop(&_double_press_timer);
                    //k_timer_start(&_long_press_timer, K_MSEC(CONFIG_BUTTON_LONG_PRESS_TIME), K_NO_WAIT);
                    k_timer_start(&_long_press_timer, K_MSEC(1000), K_NO_WAIT);
                    LOG_INF("*********Button pressed twice, waiting for long press");
                }
            } else {
                if (_button_state == BUTTON_WAIT_LONG_PRESS) {
                    k_timer_stop(&_long_press_timer);
                    _button_state = BUTTON_RELEASED;
                    _interrupt_handler(BUTTON_LONG_PRESSED);
                    LOG_INF("*********Button long pressed");
                } else if (_button_state == BUTTON_WAIT_DOUBLE_PRESS_OR_LONG_PRESS) {
                    k_timer_stop(&_double_press_timer);
                    _button_state = BUTTON_RELEASED;
                    _interrupt_handler(BUTTON_SINGLE_PRESSED);
                    LOG_INF("*********Button single pressed");
                }
            }
        }






    }

    int init() {
        //LOG_DBG("Initializing button GPIO: %i", CONFIG_BUTTON_GPIO);
        LOG_INF("Initializing button GPIO: %i", CONFIG_BUTTON_GPIO);

        int status = gpio_pin_configure(_device, CONFIG_BUTTON_GPIO, GPIO_INPUT);// | GPIO_PULL_UP);

        CHECK_SUCCESS("initializing button GPIO", status)

        return status;
    }

    int read() {
        //LOG_DBG("Reading button GPIO: %i", CONFIG_BUTTON_GPIO);
        //LOG_INF("Reading button GPIO: %i", CONFIG_BUTTON_GPIO);

        int status = gpio_pin_get(_device, CONFIG_BUTTON_GPIO);

        //CHECK_STATUS("reading button GPIO", status, < 0)

        return status;
    }

    int enable_interrupt(button_interrupt_fn interrupt_handler) {
        //LOG_DBG("Enabling button interrupt");
        LOG_INF("Enabling button interrupt");

        if (!interrupt_handler) {
            LOG_ERR("Invalid parameter for button enable interrupt: interrupt_handler is null");
            return 1;
        }

        if (_interrupt_handler) {
            LOG_ERR("Button interrupt already configured");
            return 1;
        }

        _interrupt_handler = interrupt_handler;

        int status = gpio_pin_interrupt_configure(_device, CONFIG_BUTTON_GPIO, GPIO_INT_EDGE_BOTH);

        CHECK_SUCCESS("configuring button GPIO interrupt", status)

        gpio_init_callback(&_callback, gpio_handler, BIT(CONFIG_BUTTON_GPIO));
        status = gpio_add_callback(_device, &_callback);

        CHECK_SUCCESS("adding button GPIO callback", status)

         // Start the timers
        k_timer_init(&_double_press_timer, double_press_timer_callback, nullptr);
        k_timer_init(&_long_press_timer, long_press_timer_callback, nullptr);

        return status;
    }
}