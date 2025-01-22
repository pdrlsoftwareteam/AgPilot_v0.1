#pragma once

#define LSM9DS0_DEBUG 0

#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/SPIDevice.h>

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"
#include <Filter/LowPassFilter2p.h>

class AG_InertialSensor_LSM9DS0 : public AG_InertialSensor_Backend
{
public:
    virtual ~AG_InertialSensor_LSM9DS0() { }
    void start(void) override;
    bool update() override;

    static AG_InertialSensor_Backend *probe(AG_InertialSensor &imu,
                                            AG_HAL::OwnPtr<AG_HAL::Device> dev_gyro,
                                            AG_HAL::OwnPtr<AG_HAL::Device> dev_accel,
                                            enum Rotation rotation_a,
                                            enum Rotation rotation_g,
                                            enum Rotation rotation_gH);

private:
    AG_InertialSensor_LSM9DS0(AG_InertialSensor &imu,
                              AG_HAL::OwnPtr<AG_HAL::Device> dev_gyro,
                              AG_HAL::OwnPtr<AG_HAL::Device> dev_accel,
                              int drdy_pin_num_a, int drdy_pin_num_b,
                              enum Rotation rotation_a,
                              enum Rotation rotation_g,
                              enum Rotation rotation_gH);

    struct PACKED sensor_raw_data {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    enum gyro_scale {
        G_SCALE_245DPS = 0,
        G_SCALE_500DPS,
        G_SCALE_2000DPS,
    };

    enum accel_scale {
        A_SCALE_2G = 0,
        A_SCALE_4G,
        A_SCALE_6G,
        A_SCALE_8G,
        A_SCALE_16G,
    };

    bool _accel_data_ready();
    bool _gyro_data_ready();

    void _poll_data();

    bool _init_sensor();
    bool _hardware_init();

    void _gyro_init();
    void _accel_init();

    void _gyro_disable_i2c();
    void _accel_disable_i2c();

    void _set_gyro_scale(gyro_scale scale);
    void _set_accel_scale(accel_scale scale);

    uint8_t _register_read_xm(uint8_t reg);
    uint8_t _register_read_g(uint8_t reg);
    void _register_write_xm(uint8_t reg, uint8_t val, bool checked=false);
    void _register_write_g(uint8_t reg, uint8_t val, bool checked=false);

    void _read_data_transaction_a();
    void _read_data_transaction_g();

#if LSM9DS0_DEBUG
    void        _dump_registers();
#endif

    AG_HAL::OwnPtr<AG_HAL::Device> _dev_gyro;
    AG_HAL::OwnPtr<AG_HAL::Device> _dev_accel;
    AG_HAL::Semaphore *_spi_sem;

    /*
     * If data-ready GPIO pins numbers are not defined (i.e. any negative
     * value), the fallback approach used is to check if there's new data ready
     * by reading the status register. It is *strongly* recommended to use
     * data-ready GPIO pins for performance reasons.
     */
    AG_HAL::DigitalSource * _drdy_pin_a;
    AG_HAL::DigitalSource * _drdy_pin_g;
    float _gyro_scale;
    float _accel_scale;
    int _drdy_pin_num_a;
    int _drdy_pin_num_g;
    uint8_t _gyro_instance;
    uint8_t _accel_instance;
    float _temperature;
    uint8_t _temp_counter;
    LowPassFilter2pFloat _temp_filter;

    // gyro whoami
    uint8_t whoami_g;
    
    /*
      for boards that have a separate LSM303D and L3GD20 there can be
      different rotations for each
     */
    enum Rotation _rotation_a;
    enum Rotation _rotation_g;  // for L3GD20
    enum Rotation _rotation_gH; // for L3GD20H
};
