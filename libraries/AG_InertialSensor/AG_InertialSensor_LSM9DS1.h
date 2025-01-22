#pragma once

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/SPIDevice.h>

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"

/* enable debug to see a register dump on startup */
#define LSM9DS1_DEBUG 0

class AG_InertialSensor_LSM9DS1 : public AG_InertialSensor_Backend
{
public:
    virtual ~AG_InertialSensor_LSM9DS1() { }
    void start(void) override;
    bool update() override;

    static AG_InertialSensor_Backend *probe(AG_InertialSensor &imu,
                                            AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev,
                                            enum Rotation rotation);
private:
    AG_InertialSensor_LSM9DS1(AG_InertialSensor &imu,
                              AG_HAL::OwnPtr<AG_HAL::SPIDevice> dev,
                              int drdy_pin_num_xg,
                              enum Rotation rotation);

    static AG_InertialSensor_Backend *detect(AG_InertialSensor &imu);

    struct PACKED sensor_raw_data {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    enum accel_scale {
        A_SCALE_2G = 0,
        A_SCALE_4G,
        A_SCALE_8G,
        A_SCALE_16G
    };

    void _poll_data();
    void _fifo_reset();

    bool _init_sensor();
    bool _hardware_init();

    void _gyro_init();
    void _accel_init();

    void _set_gyro_scale();
    void _set_accel_scale(accel_scale scale);

    uint8_t _register_read(uint8_t reg);
    void _register_write(uint8_t reg, uint8_t val, bool checked=false);

    void _read_data_transaction_x(uint16_t samples);
    void _read_data_transaction_g(uint16_t samples);

    #if LSM9DS1_DEBUG
    void        _dump_registers();
    #endif

    AG_HAL::OwnPtr<AG_HAL::SPIDevice> _dev;
    AG_HAL::Semaphore *_spi_sem;
    AG_HAL::DigitalSource * _drdy_pin_xg;
    float _gyro_scale;
    float _accel_scale;
    int _drdy_pin_num_xg;
    uint8_t _gyro_instance;
    uint8_t _accel_instance;
    enum Rotation _rotation;
};
