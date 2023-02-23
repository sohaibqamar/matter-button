#pragma once

#include "zephyr/drivers/gpio.h"

typedef void (*button_interrupt_fn)(int status);

namespace button {
    // Initializes the button
    int init();

    // Read a value from the button
    // Returns an integer indicating the status of the GPIO read
    // 0 = pressed, 1 = released, other values are errors
    int read();

    // Enables the button interrupt with the specified callback
    // Can only be run once
    int enable_interrupt(button_interrupt_fn interrupt_handler);
}