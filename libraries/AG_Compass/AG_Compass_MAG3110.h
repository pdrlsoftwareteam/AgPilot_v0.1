#pragma once

#include "AG_Compass_config.h"

#if AP_COMPASS_MAG3110_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_Math/AG_Math.h>

#include "AG_Compass.h"
#include "AG_Compass_Backend.h"


#ifndef HAL_MAG3110_I2C_ADDR 
 #define HAL_MAG3110_I2C_ADDR     0x0E
#endif

class AG_Compass_MAG3110 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                                     enum Rotation rotation);

    static constexpr const char *name = "MAG3110";

    void read() override;

    ~AG_Compass_MAG3110() { }

private:
    AG_Compass_MAG3110(AG_HAL::OwnPtr<AG_HAL::Device> dev);

    bool init(enum Rotation rotation);

    bool _read_sample();

    bool _hardware_init();
    void _update();

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;

    int32_t _mag_x;
    int32_t _mag_y;
    int32_t _mag_z;

    uint8_t _compass_instance;
    bool _initialised;
};

#endif  // AP_COMPASS_MAG3110_ENABLED
