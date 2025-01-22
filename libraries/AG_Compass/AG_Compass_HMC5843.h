#pragma once

#include "AG_Compass_config.h"

#if AP_COMPASS_HMC5843_ENABLED

#ifndef HAL_COMPASS_HMC5843_I2C_ADDR
#define HAL_COMPASS_HMC5843_I2C_ADDR 0x1E
#endif

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_HAL/Device.h>

#include "AG_Compass_Backend.h"

class AuxiliaryBus;
class AuxiliaryBusSlave;
class AG_InertialSensor;
class AP_HMC5843_BusDriver;

class AG_Compass_HMC5843 : public AG_Compass_Backend
{
public:
    static AG_Compass_Backend *probe(AG_HAL::OwnPtr<AG_HAL::Device> dev,
                                     bool force_external,
                                     enum Rotation rotation);

    static AG_Compass_Backend *probe_mpu6000(enum Rotation rotation);

    static constexpr const char *name = "HMC5843";

    virtual ~AG_Compass_HMC5843();

    void read() override;

private:
    AG_Compass_HMC5843(AP_HMC5843_BusDriver *bus,
                       bool force_external, enum Rotation rotation);

    bool init();
    bool _check_whoami();
    bool _calibrate();
    bool _setup_sampling_mode();

    void _timer();

    /* Read a single sample */
    bool _read_sample();

    // ask for a new sample
    void _take_sample();

    AP_HMC5843_BusDriver *_bus;

    Vector3f _scaling;
    float _gain_scale;

    int16_t _mag_x;
    int16_t _mag_y;
    int16_t _mag_z;

    uint8_t _compass_instance;

    enum Rotation _rotation;
    
    bool _initialised:1;
    bool _force_external:1;
};

class AP_HMC5843_BusDriver
{
public:
    virtual ~AP_HMC5843_BusDriver() { }

    virtual bool block_read(uint8_t reg, uint8_t *buf, uint32_t size) = 0;
    virtual bool register_read(uint8_t reg, uint8_t *val) = 0;
    virtual bool register_write(uint8_t reg, uint8_t val) = 0;

    virtual AG_HAL::Semaphore *get_semaphore() = 0;

    virtual bool configure() { return true; }
    virtual bool start_measurements() { return true; }

    virtual AG_HAL::Device::PeriodicHandle register_periodic_callback(uint32_t, AG_HAL::Device::PeriodicCb) = 0;

    // set device type within a device class
    virtual void set_device_type(uint8_t devtype) = 0;

    // return 24 bit bus identifier
    virtual uint32_t get_bus_id(void) const = 0;

    virtual void set_retries(uint8_t retries) {}
};

class AP_HMC5843_BusDriver_HALDevice : public AP_HMC5843_BusDriver
{
public:
    AP_HMC5843_BusDriver_HALDevice(AG_HAL::OwnPtr<AG_HAL::Device> dev);

    bool block_read(uint8_t reg, uint8_t *buf, uint32_t size) override;
    bool register_read(uint8_t reg, uint8_t *val) override;
    bool register_write(uint8_t reg, uint8_t val) override;

    AG_HAL::Semaphore *get_semaphore() override;

    AG_HAL::Device::PeriodicHandle register_periodic_callback(uint32_t period_usec, AG_HAL::Device::PeriodicCb) override;

    // set device type within a device class
    void set_device_type(uint8_t devtype) override {
        _dev->set_device_type(devtype);
    }

    // return 24 bit bus identifier
    uint32_t get_bus_id(void) const override {
        return _dev->get_bus_id();
    }

    void set_retries(uint8_t retries) override {
        return _dev->set_retries(retries);
    }
    
private:
    AG_HAL::OwnPtr<AG_HAL::Device> _dev;
};

class AP_HMC5843_BusDriver_Auxiliary : public AP_HMC5843_BusDriver
{
public:
    AP_HMC5843_BusDriver_Auxiliary(AG_InertialSensor &ins, uint8_t backend_id,
                                   uint8_t addr);
    virtual ~AP_HMC5843_BusDriver_Auxiliary();

    bool block_read(uint8_t reg, uint8_t *buf, uint32_t size) override;
    bool register_read(uint8_t reg, uint8_t *val) override;
    bool register_write(uint8_t reg, uint8_t val) override;

    AG_HAL::Semaphore *get_semaphore() override;

    bool configure() override;
    bool start_measurements() override;

    AG_HAL::Device::PeriodicHandle register_periodic_callback(uint32_t period_usec, AG_HAL::Device::PeriodicCb) override;

    // set device type within a device class
    void set_device_type(uint8_t devtype) override;

    // return 24 bit bus identifier
    uint32_t get_bus_id(void) const override;
    
private:
    AuxiliaryBus *_bus;
    AuxiliaryBusSlave *_slave;
    bool _started;
};

#endif // AP_COMPASS_HMC5843_ENABLED
