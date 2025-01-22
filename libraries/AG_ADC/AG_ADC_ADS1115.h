#pragma once

#include <inttypes.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>

struct adc_report_s
{
    uint8_t id;
    float data;
};

class AG_ADC_ADS1115
{
public:
    AG_ADC_ADS1115();
    ~AG_ADC_ADS1115();

    bool init();
    size_t read(adc_report_s *report, size_t length) const;

    uint8_t get_channels_number() const
    {
        return _channels_number;
    }

private:
    static const uint8_t _channels_number;

    AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev;

    uint16_t            _gain;
    int                 _channel_to_read;
    adc_report_s        *_samples;

    void _update();
    bool _start_conversion(uint8_t channel);

    float _convert_register_data_to_mv(int16_t word) const;
};
