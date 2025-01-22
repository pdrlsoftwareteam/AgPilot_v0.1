#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_BMP085_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/I2CDevice.h>
#include <AG_HAL/utility/OwnPtr.h>
#include <Filter/Filter.h>

#ifndef HAL_BARO_BMP085_I2C_ADDR
#define HAL_BARO_BMP085_I2C_ADDR        (0x77)
#endif

class AG_Baro_BMP085 : public AG_Baro_Backend {
public:
    AG_Baro_BMP085(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

    /* AG_Baro public interface: */
    void update() override;

    static AG_Baro_Backend *probe(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);


private:
    bool _init();

    void _cmd_read_pressure();
    void _cmd_read_temp();
    bool _read_pressure();
    void _read_temp();
    void _calculate();
    bool _data_ready();

    void _timer(void);

    uint16_t _read_prom_word(uint8_t word);
    bool     _read_prom(uint16_t *prom);


    AG_HAL::OwnPtr<AG_HAL::Device> _dev;
    AG_HAL::DigitalSource *_eoc;

    uint8_t _instance;
    bool _has_sample;

    // Boards with no EOC pin: use times instead
    uint32_t _last_press_read_command_time;
    uint32_t _last_temp_read_command_time;

    // State machine
    uint8_t _state;

    // Internal calibration registers
    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;

    int32_t _raw_pressure;
    int32_t _raw_temp;
    int32_t _temp;
    AverageIntegralFilter<int32_t, int32_t, 10> _pressure_filter;

    uint8_t _vers;
    uint8_t _type;
};

#endif  // AP_BARO_BMP085_ENABLED
