#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_LPS2XH_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_HAL/utility/OwnPtr.h>
#include <AG_Math/AG_Math.h>

#define HAL_BARO_LPS25H_I2C_BUS 0

#ifndef HAL_BARO_LPS25H_I2C_ADDR
# define HAL_BARO_LPS25H_I2C_ADDR 0x5D
#endif


class AG_Baro_LPS2XH : public AG_Baro_Backend
{
public:
    enum LPS2XH_TYPE {
        BARO_LPS22H = 0,
        BARO_LPS25H = 1,
    };

    AG_Baro_LPS2XH(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);

    /* AG_Baro public interface: */
    void update() override;

    static AG_Baro_Backend *probe(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev);
    static AG_Baro_Backend *probe_InvensenseIMU(AG_Baro &baro, AG_HAL::OwnPtr<AG_HAL::Device> dev, uint8_t imu_address);

private:
    virtual ~AG_Baro_LPS2XH(void) {};

    bool _init(void);
    void _timer(void);
    void _update_temperature(void);
    void _update_pressure(void);
    bool _imu_i2c_init(uint8_t imu_address);

    bool _check_whoami(void);

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;

    uint8_t _instance;
    float _pressure_sum;
    uint32_t _pressure_count;
    float _temperature;

    uint32_t CallTime = 0;

    enum LPS2XH_TYPE _lps2xh_type;
};

#endif  // AP_BARO_LPS2XH_ENABLED
