#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_BMP280_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_HAL/utility/OwnPtr.h>

#ifndef HAL_BARO_BMP280_I2C_ADDR
 #define HAL_BARO_BMP280_I2C_ADDR  (0x76)
#endif
#ifndef HAL_BARO_BMP280_I2C_ADDR2
 #define HAL_BARO_BMP280_I2C_ADDR2 (0x77)
#endif

class AG_Baro_BMP280 : public AG_Baro_Backend
{
public:
    AG_Baro_BMP280(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

    /* AG_Baro public interface: */
    void update() override;

    static AG_Baro_Backend *probe(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

private:

    bool _init(void);
    void _timer(void);
    void _update_temperature(int32_t);
    void _update_pressure(int32_t);

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;

    uint8_t _instance;
    int32_t _t_fine;
    float _pressure_sum;
    uint32_t _pressure_count;
    float _temperature;

    // Internal calibration registers
    int16_t _t2, _t3, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9;
    uint16_t _t1, _p1;
};

#endif  // AP_BARO_BMP280_ENABLED
