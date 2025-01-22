#include <AG_HAL/AG_HAL.h>
#include "AG_InertialSensor_ExternalAHRS.h"
#include <AG_ExternalAHRS/AG_ExternalAHRS.h>
#include <stdio.h>

#if HAL_EXTERNAL_AHRS_ENABLED

const extern AG_HAL::HAL& hal;

AG_InertialSensor_ExternalAHRS::AG_InertialSensor_ExternalAHRS(AG_InertialSensor &imu, uint8_t _serial_port) :
    AG_InertialSensor_Backend(imu),
    serial_port(_serial_port)
{
}

void AG_InertialSensor_ExternalAHRS::handle_external(const AG_ExternalAHRS::ins_data_message_t &pkt)
{
    if (!started) {
        return;
    }
    Vector3f accel = pkt.accel;
    Vector3f gyro = pkt.gyro;

    _rotate_and_correct_accel(accel_instance, accel);
    _notify_new_accel_raw_sample(accel_instance, accel, AG_HAL::micros64());

    _publish_temperature(accel_instance, pkt.temperature);

    _notify_new_gyro_sensor_rate_sample(gyro_instance, gyro);
    _rotate_and_correct_gyro(gyro_instance, gyro);
    _notify_new_gyro_raw_sample(gyro_instance, gyro, AG_HAL::micros64());
}

bool AG_InertialSensor_ExternalAHRS::update(void)
{
    if (started) {
        update_accel(accel_instance);
        update_gyro(gyro_instance);
    }
    return started;
}

void AG_InertialSensor_ExternalAHRS::start()
{
    const float rate = AP::externalAHRS().get_IMU_rate();
    if (_imu.register_gyro(gyro_instance, rate,
                           AG_HAL::Device::make_bus_id(AG_HAL::Device::BUS_TYPE_SERIAL, serial_port, 1, DEVTYPE_SERIAL)) &&
        _imu.register_accel(accel_instance, rate,
                            AG_HAL::Device::make_bus_id(AG_HAL::Device::BUS_TYPE_SERIAL, serial_port, 2, DEVTYPE_SERIAL))) {
        started = true;
    }
}

void AG_InertialSensor_ExternalAHRS::accumulate()
{
    AP::externalAHRS().update();
}

// get a startup banner to output to the GCS
bool AG_InertialSensor_ExternalAHRS::get_output_banner(char* banner, uint8_t banner_len)
{
    const char* name = AP::externalAHRS().get_name();
    snprintf(banner, banner_len, "IMU%u: External: %s %0.0fHz",
             gyro_instance,
             (name != nullptr) ? name : "",
              AP::externalAHRS().get_IMU_rate());
    return true;
}

#endif // HAL_EXTERNAL_AHRS_ENABLED

