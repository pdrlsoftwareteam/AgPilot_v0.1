#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_SPL06_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_HAL/utility/OwnPtr.h>

#ifndef HAL_BARO_SPL06_I2C_ADDR
 #define HAL_BARO_SPL06_I2C_ADDR  (0x76)
#endif
#ifndef HAL_BARO_SPL06_I2C_ADDR2
 #define HAL_BARO_SPL06_I2C_ADDR2 (0x77)
#endif

class AG_Baro_SPL06 : public AG_Baro_Backend
{
public:
    AG_Baro_SPL06(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

    /* AG_Baro public interface: */
    void update() override;

    static AG_Baro_Backend *probe(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

private:

    bool _init(void);
    void _timer(void);
    void _update_temperature(int32_t);
    void _update_pressure(int32_t);

    int32_t raw_value_scale_factor(uint8_t);

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;

    int8_t _timer_counter;
    uint8_t _instance;
    float _temp_raw;
    float _pressure_sum;
    uint32_t _pressure_count;
    float _temperature;

    // Internal calibration registers
    int32_t _c00, _c10;
    int16_t _c0, _c1, _c01, _c11, _c20, _c21, _c30;
};

#endif  // AP_BARO_SPL06_ENABLED
