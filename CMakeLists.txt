cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(button)

include($ENV{MATTER_BASE}/config/nrfconnect/app/enable-gnu-std.cmake)
include($ENV{MATTER_BASE}/src/app/chip_data_model.cmake)

target_include_directories(app PRIVATE include matter)

target_sources(app PRIVATE
    src/main.cpp
    src/app_task.cpp
    src/button.cpp
    src/reset_switch.cpp
    src/battery.cpp
    matter/zap-generated/IMClusterCommandHandler.cpp
    matter/zap-generated/callback-stub.cpp
)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/matter/button.zap
)
