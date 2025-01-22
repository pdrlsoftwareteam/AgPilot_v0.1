#pragma once

#include <AG_HAL/AG_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_LINUX

#include <AG_HAL/I2CDevice.h>
#include <AG_HAL/SPIDevice.h>
#include <Filter/Filter.h>
#include <Filter/LowPassFilter2p.h>

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"

class AG_InertialSensor_RST : public AG_InertialSensor_Backend
{
public:
    AG_InertialSensor_RST(AG_InertialSensor &imu,
                              AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_gyro,
                              AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_accel,
                              enum Rotation rotation_a,
                              enum Rotation rotation_g);

    virtual ~AG_InertialSensor_RST();

    // probe the sensor on SPI bus
    static AG_InertialSensor_Backend *probe(AG_InertialSensor &imu,
                                              AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_gyro,
                                              AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev_accel,
                                              enum Rotation rotation_a,
                                              enum Rotation rotation_g);

    /* update accel and gyro state */
    bool update() override;

    void start(void) override;

private:
    bool _init_sensor();
    bool _init_gyro();
    bool _init_accel();
    void gyro_measure();
    void accel_measure();

    AG_HAL::OwnPtr<AG_HAL::SPIDevice> _dev_gyro;//i3g4250d
    AG_HAL::OwnPtr<AG_HAL::SPIDevice> _dev_accel;//iis328dq

    float _gyro_scale;
    float _accel_scale;

    // gyro and accel instances
    uint8_t _gyro_instance;
    uint8_t _accel_instance;
    enum Rotation _rotation_g;
    enum Rotation _rotation_a;
};
#endif
