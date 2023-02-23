#pragma once

#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

/*
 * Switching from Thread child to router may cause a few second packet stall.
 * Until this is improved in OpenThread we need to increase the retransmission
 * interval to survive the stall.
 */
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (1000_ms32)
