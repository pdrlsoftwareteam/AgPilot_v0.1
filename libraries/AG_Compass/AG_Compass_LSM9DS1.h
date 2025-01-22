#pragma once

#include "AG_Compass_config.h"

#if AP_COMPASS_LSM9DS1_ENABLED

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>
#include <AG_Math/AG_Math.h>

#include "AG_Compass_Backend.h"

class AG_Compass_LSM9DS1 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                                     enum Rotation rotation);

    static constexpr const char *name = "LSM9DS1";

    void read() override;

    virtual ~AG_Compass_LSM9DS1() {}

private:
    AG_Compass_LSM9DS1(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                       enum Rotation rotation);
    bool init();
    bool _check_id(void);
    bool _configure(void);
    bool _set_scale(void);
    void _update(void);

    uint8_t _register_read(uint8_t reg);
    void _register_write(uint8_t reg, uint8_t val);
    void _register_modify(uint8_t reg, uint8_t clearbits, uint8_t setbits);
    bool _block_read(uint8_t reg, uint8_t *buf, uint32_t size);
    void _dump_registers();

    AG_HAL::OwnPtr<AG_HAL::Device> _dev;
    uint8_t _compass_instance;
    float _scaling;
    enum Rotation _rotation;
};

#endif
