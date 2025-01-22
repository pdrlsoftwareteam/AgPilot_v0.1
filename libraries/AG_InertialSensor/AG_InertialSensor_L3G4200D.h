
#pragma once

#include <AG_HAL/AG_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_LINUX

#include <AG_HAL/I2CDevice.h>
#include <Filter/Filter.h>
#include <Filter/LowPassFilter2p.h>

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"

class AG_InertialSensor_L3G4200D : public AG_InertialSensor_Backend
{
public:
    AG_InertialSensor_L3G4200D(AG_InertialSensor &imu,
                                            AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev_gyro,
                                            AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev_accel);
    
    
    
    virtual ~AG_InertialSensor_L3G4200D();

    // probe the sensor on I2C bus
    static AG_InertialSensor_Backend *probe(AG_InertialSensor &imu,
                                            AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev_gyro,
                                            AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev_accel);

   
   
    /* update accel and gyro state */
    bool update() override;

    void start(void) override;

    
private:
    bool _accel_init();
    bool _gyro_init();
    bool _init_sensor();
    void _accumulate_gyro();
    void _accumulate_accel();
    
    AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev_gyro;
    AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev_accel;

    void _set_filter_frequency(uint8_t filter_hz);

    // Low Pass filters for gyro and accel 
    LowPassFilter2pVector3f _accel_filter;
    LowPassFilter2pVector3f _gyro_filter;

    enum Rotation _rotation; 

    // gyro and accel instances
    uint8_t _gyro_instance;
    uint8_t _accel_instance;
};
#endif // __AP_INERTIAL_SENSOR_L3G4200D2_H__