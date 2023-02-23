#include "battery.hpp"

#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include "util.hpp"

LOG_MODULE_DECLARE(app);

namespace battery {

    namespace {
        const struct device* _device = DEVICE_DT_GET(DT_NODELABEL(adc));

        const struct adc_channel_cfg _adc_config {
            .gain = ADC_GAIN_1_6,
            .reference = ADC_REF_INTERNAL,
            .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
            .channel_id = 0,
            .differential = 0,
            .input_positive = SAADC_CH_PSELP_PSELP_VDD
        };
    }

    int init() {
        LOG_DBG("Initializing battery management");

        int status = adc_channel_setup(_device, &_adc_config);

        CHECK_SUCCESS("initializing battery ADC", status)

        return status;
    }

    int get_percentage() {
        int sample = 0;

        struct adc_sequence sequence {
            .channels = BIT(0),
            .buffer = &sample,
            .buffer_size = sizeof(sample),
            .resolution = 14,
            .oversampling = 4,
            .calibrate = true
        };

        int status = adc_read(_device, &sequence);
        CHECK_SUCCESS("measuring battery ADC", status);

        auto ref = adc_ref_internal(_device);

        LOG_DBG("Battery raw: %u, reference: %u", sample, ref);

        adc_raw_to_millivolts(ref, _adc_config.gain, sequence.resolution, &sample);

        LOG_DBG("Battery mV: %d", sample);

        // Clip from 2V to 3V
        sample = sample <= 3000 ? (sample >= 2000 ? sample : 2000) : 3000;

        // Discharge curve for CR2430 shows near linear line to 2.5V for the first 80%, dropping faster to 2V for the last 20%
        // This results in the following equations:
        // Above 2500, percentage = sample * 80/500 - 380
        // Below 2500, percentage = sample * 20/500 - 80

        return sample >= 2500 ? sample * 80 / 500 - 380 : sample * 20/500 - 80;
    }

}