#pragma once

#include "zephyr/drivers/gpio.h"

typedef void (*reset_fn)();

namespace reset_switch {
    // Initializes the reset switch
    int init();

    // Read a value from the reset switch
    // Returns an integer indicating the status of the GPIO read
    // 0 = pressed, 1 = not pressed, other values are errors
    int read();

    // Enables the reset switch interrupt with the specified callback
    // Can only be run once
    int enable_interrupt(reset_fn reset_handler);
}