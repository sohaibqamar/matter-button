#pragma once

struct AppEvent;

typedef void (*event_callback_fn)(AppEvent event);

enum class AppEventType: unsigned char {
    None = 0,
    ButtonChanged
};

struct AppEvent {
    AppEventType type;
    event_callback_fn callback;

    union {
        struct {
            // Status returned from GPIO read
            // 0 = pressed, 1 = released, other values are errors
            int status;
        } button_changed;
    };
};